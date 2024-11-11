#include "JpegWriter.h"



//--------------------------------
//	CHUNK WRITER
//--------------------------------

///<summary>
///Writes next block of rows starting at NextRow.
///Advances NextRow by the height of given image.
///If number of lines in the provided image is bigger than number of rows left writes what is possible.
///</summary>
void JpegWriter::WriteNextRows(ImageBuffer_Byte* image) {
	
	//Aliases
	int buffer_height = image->GetHeight();
	int buffer_width = image->GetWidth();

	//----------------------------------------------------------------------
	// 1 - Arguments check

	// Checking writer state -----------------------------------------------

	//If writing is finished we do nothing
	if (_state == WriterStates::Finished)
		return;

	//If writer has failed we throw
	if (_state == WriterStates::Failed) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Jpeg_InitError, "Trying to write a file using failed writer object.");
	}
	
	//If writer wasn't initialized we throw
	if (_state == WriterStates::Uninitialized) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Jpeg_InitError, "Trying to write a file using uninitialized writer object.");
	}

	// Checking buffer -----------------------------------------------------

	if (image->GetBitPerComponent() != BitDepth::BD_8_BIT) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Jpeg_InitError, "Trying to write image object that is not 8 bit per color component.");
	}

	//Checking if provided image conforms with this writer settings --------

	if (image->GetLayout() != _image_info._layout) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Jpeg_EncodingError, "Trying to write image object incompatible with writer object (layout mismatch).");
	}

	if (image->GetWidth() != _image_info.GetWidth()) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Jpeg_EncodingError, "Trying to write image object incompatible with writer object (width mismatch).");
	}

	//----------------------------------------------------------------------
	// 2 - Remaining rows calculation

	//Checking how many rows are actually available to write until the file ends
	int actual_num_rows = AdvanceRows(buffer_height);

	//----------------------------------------------------------------------
	// 3 - Compressing and writing the image rows
	
	try {
		//Appending image rows to the file

		//jpeg_write_scanlines can write multiple rows in one call.
		//To do that it excepts pointer to the beginning of a data array
		//that contains possibly more than one row. 
		//This array is of type JSAMPARRAY which is unsigned char**.
		//We cannot write multiple lines in this way because they have to be
		//allocated continiously in memory, but ImageBuffer_Byte
		//is not expected to allocated image data in this way.
		//So, we write one line at a time, but still have to pretend
		//it is array of rows of type JSAMPARRAY.

		//Image data alias
		uint8_t** image_data = image->GetData();
		
		for (int row = 0; row < actual_num_rows; row++) {
			//Jpeg compressor accepts JSAMPROW. It is unsigned char*
			//Therefore we write next row:
			JSAMPARRAY rows_to_write = static_cast<JSAMPARRAY>(&(image_data[row]));
			jpeg_write_scanlines(&jpeg_comp, rows_to_write, 1);
		}
	}
	catch (codec_fatal_exception e) {
		_state = WriterStates::Failed;
		CleanUp();
		throw; //Rethrowing
	}

	//If we have finished writing the file we close it and deallocate the compressor
	if (_state == WriterStates::Finished) {
		//Finalizing writing
		jpeg_finish_compress(&jpeg_comp);
		//Releasing the decompressor object memory
		jpeg_destroy_compress(&jpeg_comp);
		_is_compressor_initialized = false;
		//Closing the file
		fclose(_file_handle);
		_is_file_opened = false;
	}

	//If this was first block of writing we switch state to continue
	if (_state == WriterStates::Ready_Start)
		_state = WriterStates::Ready_Continue;
}




//--------------------------------
//	WHOLE FILE WRITING
//--------------------------------

///<summary>
/// Static method. Compresses and writes image file to the given path.
/// Image is expected to be 8 bit per channel and not containing alpha channels.
/// <para>Can throw std::ifstream::failure if failed to open file.</para>
/// <para>Can throw codec_fatal_exception if failed to compress the image.</para>
/// </summary>
/// <param name="file_path">Path to the output file.</param>
/// <param name="image">Image to write.</param>
/// <param name="header">Jpeg header data. Should contain output colorspace.</param>
/// <param name="quality">Jpeg codec compression quality setting. 1-100</param>
/// <param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
void JpegWriter::WriteJPEG(std::filesystem::path file_path, ImageBuffer_Byte* image, JpegHeaderInfo header, int quality, WarningCallbackData warning_callback_data) {
	JpegWriter writer(file_path, header, quality, warning_callback_data);
	writer.WriteNextRows(image);
}




//--------------------------------
//	PUBLIC CONSTRUCTORS
//--------------------------------

///<summary>
///<para>Opens file pointed by file_path and writes the header.</para>
///<para>JPEG format does not support alpha channels. Alpha channels should be mixed in, or removed by the consumer before passing an image object to the writer, or exception will be produced.</para>
///<para>Provided image dimensions take precedence over header ones. Layout actually written to the file should be provided in JpegHeaderInfo.</para>
///<para>Can throw std::ifstream::failure if failed to open file.</para>
///<para>Can throw codec_fatal_exception if failed to write the header.</para>
///</summary>
///<param name="file_path">Path to the file to be written.</param>
///<param name="header">JPEG header describing the file.</param>
///<param name="quality">JPEG compression quality setting.</param>
JpegWriter::JpegWriter(std::filesystem::path file_path, JpegHeaderInfo header, int quality, WarningCallbackData warning_callback_data) : ImageWriter(file_path) {

	//----------------------------------------------------------------------
	// 0 - Setting initial state of the writer object

	_state = WriterStates::Uninitialized;
	_is_file_opened = false;
	_is_compressor_initialized = false;
	_warning_callback_data.warningCallback = warning_callback_data.warningCallback;
	_warning_callback_data.warningCallbackArgs_ptr = warning_callback_data.warningCallbackArgs_ptr;

	//----------------------------------------------------------------------
	// 1 - Arguments check

	//Jpeg quality setting
	if (quality < 1 || quality > 100)
		throw codec_fatal_exception(CodecExceptions::Jpeg_InitError, "Jpeg quality setting is out of range.");

	//JPEG file colorspace
	//Those are the colorspaces actually supported by JPEG files
	J_COLOR_SPACE header_layout = header.GetColorSpace();
	if (header_layout != J_COLOR_SPACE::JCS_GRAYSCALE &&
		header_layout != J_COLOR_SPACE::JCS_RGB &&
		header_layout != J_COLOR_SPACE::JCS_YCCK &&
		header_layout != J_COLOR_SPACE::JCS_CMYK &&
		header_layout != J_COLOR_SPACE::JCS_YCbCr)
		throw codec_fatal_exception(CodecExceptions::Jpeg_InitError, "Invalid JPEG file colorspace requested.");

	//Saving the arguments
	_image_info._height = header.GetHeight();
	_image_info._width = header.GetWidth();
	_image_info._bit_depth = BitDepth::BD_8_BIT;
	//Image buffers with this layout will be expected
	_image_info._layout = JpegLayoutToImageLayout(header_layout); 

	_jpeg_header = header;

	//----------------------------------------------------------------------
	// 2 - Opening the file pointed by file_path

	OpenCFileForBinaryWriting(file_path);

	//----------------------------------------------------------------------
	// 3 - Initializing compressor data structures

	//Assigning error manager to compressor
	jpeg_comp.err = jpeg_std_error(&jerr_comp);
	//Registering jpeg error handler
	jerr_comp.error_exit = &ErrorExitHandler;
	//Registering warning handler function
	jerr_comp.emit_message = WarningHandler;
	//Initializing compressor object
	jpeg_create_compress(&jpeg_comp);

	//Assigning opened file as compression destination
	jpeg_stdio_dest(&jpeg_comp, _file_handle);

	//Referencing this warning callback in the compressor object
	jpeg_comp.client_data = &_warning_callback_data;

	//Setting state flag
	_is_compressor_initialized = true;

	//----------------------------------------------------------------------
	// 4 - Configuring compressor for given image

	//Passing image dimensions
	jpeg_comp.image_height = header._height;
	jpeg_comp.image_width = header._width;

	//Passing image buffer colorspace and number of color components as input for compressor.
	//At this point with jpeg header colorspace check and JpegLayoutToImageLayout output
	//expected layouts may only be RGB and GRAYSCALE
	switch (_image_info._layout) {
	case ImagePixelLayout::G: {
		jpeg_comp.in_color_space = J_COLOR_SPACE::JCS_GRAYSCALE;
		jpeg_comp.input_components = 1;
		break;
	}
	case ImagePixelLayout::RGB:
		jpeg_comp.in_color_space = J_COLOR_SPACE::JCS_RGB;
		jpeg_comp.input_components = 3;
		break;
	}

	//----------------------------------------------------------------------
	// 5 - Setting the compressor to the ready state

	//Applying default settings
	jpeg_set_defaults(&jpeg_comp);

	//Setting the file colorspace according to the JpegHeader object
	jpeg_set_colorspace(&jpeg_comp, header_layout);

	//Setting quality
	jpeg_set_quality(&jpeg_comp, quality, TRUE);

	//Explicitly saying to use slow, but accurate compression method (default)
	jpeg_comp.dct_method = JDCT_ISLOW;			

	//Initializing compression procedure
	jpeg_start_compress(&jpeg_comp, TRUE);

	//File is opened, compressor is ready and the header was written
	_state = WriterStates::Ready_Start;
}




//--------------------------------
//	PRIVATE METHODS
//--------------------------------

///<summary>
///Closes file and destroys compressor
///if file is opened and compressor exists.
///</summary>
void JpegWriter::CleanUp() {
	//Deallocating decompressor if exists
	if (_is_compressor_initialized == true) {
		jpeg_destroy_compress(&jpeg_comp);
		_is_compressor_initialized = false;
	}
	//Closing file if opened
	if (_is_file_opened == true) {
		fclose(_file_handle);
		_is_file_opened = false;
	}
}




//--------------------------------
//	UTILITY METHODS
//--------------------------------

/// <summary>
/// Translates jpeg file colorspace (pixel layout) to appropriate pixel layout used in ImageBuffer objects.
/// </summary>
ImagePixelLayout JpegWriter::JpegLayoutToImageLayout(J_COLOR_SPACE jpeg_colorspace) {
	//We try to decode jpeg to RGB color space in all cases when it is not explicitly grayscale
	switch (jpeg_colorspace)
	{
		case J_COLOR_SPACE::JCS_GRAYSCALE:
			return ImagePixelLayout::G;

		case J_COLOR_SPACE::JCS_RGB:
			return ImagePixelLayout::RGB;

		default:
			return ImagePixelLayout::RGB;
	}
}



//--------------------------------
//	ARCHIVED METHODS
//--------------------------------

///<summary>
/// Archived method. Whole reading procedure is written for one pass.
///Static method. Compresses and writes image file to the given path.
///Image is expected to be 8 bit per channel and not containing alpha channels.
///<para>Can throw std::ifstream::failure if failed to open file.</para>
///<para>Can throw codec_fatal_exception if failed to compress the image.</para>
///</summary>
///<param name="file_path">Path to the output file.</param>
///<param name="image">Image to write.</param>
///<param name="header">Jpeg header data. Should contain valid output colorspace.</param>
///<param name="quality">Jpeg codec compression quality setting. 1-100</param>
///<param name="warning_callback_ptr">Pointer to the warning callback function. Can be NULL.</param>
///<param name="warning_callback_args_ptr">Pointer to the warning callback function arguments. Can be NULL.</param>
void JpegWriter::WriteJPEG_Archive(std::filesystem::path file_path, ImageBuffer_Byte* image, JpegHeaderInfo header, int quality, void (*warning_callback_ptr)(std::string, void*), void* warning_callback_args_ptr) {
	//----------------------------------------------------------------------
	// 0 - Argument check

	if (quality < 1 || quality > 100)
		throw codec_fatal_exception(CodecExceptions::Jpeg_EncodingError, "Jpeg quality setting is out of range.");

	//Checking if we can write the image as a JPEG
	if (image->GetLayout() == ImagePixelLayout::GA || image->GetLayout() == ImagePixelLayout::RGBA)
		throw codec_fatal_exception(CodecExceptions::Jpeg_InitError, "JPEG format does not support alpha channels.");

	if (image->GetBitPerComponent() == BitDepth::BD_16_BIT)
		throw codec_fatal_exception(CodecExceptions::Jpeg_EncodingError, "JPEG writer does not support 16 bit per channel images.");

	if (image->GetHeight() != header._height || image->GetWidth() != header.GetWidth())
		throw codec_fatal_exception(CodecExceptions::Jpeg_EncodingError, "Dimensions mismatch between provided header and provided image.");

	//----------------------------------------------------------------------
	// 1 - Opening the file pointed by file_path

	//Extracting file path as UTF-8 c-style string to use with c file opener.
	FILE* file_handle;
	const char* c_file_path = reinterpret_cast<const char*>(file_path.generic_u8string().c_str());
	//Trying to open the file in reading binary mode
	errno_t fopen_error = fopen_s(&file_handle, c_file_path, "wb");
	//Checking if failed to open the file
	if (fopen_error != 0) {
		//Constructing error message string
		char err_msg_buffer[256];
		errno_t strerror_error = strerror_s(err_msg_buffer, 256, fopen_error);
		//Throwing exception	
		if (strerror_error == 0) //Message string constructed
			throw std::ifstream::failure(std::string(err_msg_buffer));
		else
			throw std::ifstream::failure("");
	}

	//----------------------------------------------------------------------
	// 2 - Initializing compressor data structures

	//Declaring libjpeg compressor structure
	struct jpeg_compress_struct jpeg_comp;
	//Declaring libjpeg error manager structure
	struct jpeg_error_mgr jerr_comp;

	//Assigning error manager to compressor
	jpeg_comp.err = jpeg_std_error(&jerr_comp);
	//Registering jpeg error handler
	jerr_comp.error_exit = &ErrorExitHandler;

	//Referencing warnings callback in the compressor object if the callback is provided
	WarningCallbackData warning_callback_data;
	if (warning_callback_ptr != NULL) {
		//Combining user provided callback function and arguments into one structure
		warning_callback_data.warningCallback = warning_callback_ptr;
		warning_callback_data.warningCallbackArgs_ptr = warning_callback_args_ptr;

		//Referencing actual user callback in compressor object so it can be called by warning handler
		jpeg_comp.client_data = &warning_callback_data;

		//Registering warning handler function which will call to user provided callback
		jerr_comp.emit_message = &WarningHandler;
	}

	//Initializing compressor object
	jpeg_create_compress(&jpeg_comp);

	//Assigning opened file as compression destination
	jpeg_stdio_dest(&jpeg_comp, file_handle);

	//----------------------------------------------------------------------
	// 3 - Configuring compressor for given image

	//Passing image info to managing object
	//(note signed to unsigned cast)
	jpeg_comp.image_height = static_cast<JDIMENSION>(image->GetHeight());
	jpeg_comp.image_width = static_cast<JDIMENSION>(image->GetWidth());

	//Passing layout of raw image data
	switch (image->GetLayout()) {
		case ImagePixelLayout::G:
			jpeg_comp.input_components = 1;
			jpeg_comp.in_color_space = J_COLOR_SPACE::JCS_GRAYSCALE;
			break;

		case ImagePixelLayout::RGB:
			jpeg_comp.input_components = 3;
			jpeg_comp.in_color_space = J_COLOR_SPACE::JCS_RGB;
			break;
	}

	//----------------------------------------------------------------------
	// 4 - Setting the compressor to the ready state

	//Applying default settings
	jpeg_set_defaults(&jpeg_comp);

	//Setting the output file colorspace according to the JpegHeader object
	jpeg_set_colorspace(&jpeg_comp, header.GetColorSpace());

	//Setting quality
	jpeg_set_quality(&jpeg_comp, quality, TRUE);

	//Explicitly saying to use slow, but accurate compression method (default)
	jpeg_comp.dct_method = JDCT_ISLOW;

	//----------------------------------------------------------------------
	// 5 - Compressing and writing the image

	//Initializing compression procedure
	jpeg_start_compress(&jpeg_comp, TRUE);

	//Image data alias
	JSAMPARRAY uncomressed_data = static_cast<JSAMPARRAY>(image->GetData());

	//Compressing rows and writing them to file
	try {
		jpeg_write_scanlines(&jpeg_comp, uncomressed_data, jpeg_comp.image_height);
	}
	catch (codec_fatal_exception e) {
		//Releasing the compressor object
		jpeg_destroy_compress(&jpeg_comp);
		//Closing the file
		fclose(file_handle);
		//Rethrowing
		throw;
	}

	//----------------------------------------------------------------------
	// 6 - Cleaning

	//Finishing writing and resetting the managing object
	jpeg_finish_compress(&jpeg_comp);

	//Releasing the compressor object
	jpeg_destroy_compress(&jpeg_comp);

	//Closing the file
	fclose(file_handle);
}