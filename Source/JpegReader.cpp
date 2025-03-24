#include "JpegReader.h"


//--------------------------------
//	CHUNK READER
//--------------------------------

///<summary>
///Reads next block of rows starting at NextRow.
///Returns resulting bitmap.
///Advances NextRow by num_lines.
///If num_lines is bigger than number of rows left reads all available rows.
///When all rows are already read returns NULL.
///</summary>
ImageBuffer_Byte JpegReader::ReadNextRows(int num_rows) {
	
	//----------------------------------------------------------------------
	// 1 - Arguments check

	//If reading has finished we return unallocated image buffer
	if (_state == ReaderStates::Finished)
		return ImageBuffer_Byte(_image_info.GetHeight(), _image_info.GetWidth(), _image_info.GetLayout(), _image_info.GetBitDepth(), false);

	//If reader has failed before we throw
	if (_state == ReaderStates::Failed) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Jpeg_InitError, "Trying to read a file using failed reader object.");
	}

	//If reader wasn't initialized we throw
	if (_state == ReaderStates::Uninitialized) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Jpeg_InitError, "Trying to read a file using uninitialized reader object.");
	}

	//Simple argument sanity check
	if (num_rows <= 0)
		return ImageBuffer_Byte(_image_info.GetHeight(), _image_info.GetWidth(), _image_info.GetLayout(), _image_info.GetBitDepth(), false);

	//----------------------------------------------------------------------
	// 2 - Remaining rows calculation

	//Calculating num rows actually available to read and advancing the row counter.
	int actual_num_rows = AdvanceRows(num_rows);

	//----------------------------------------------------------------------
	// 3 - Decompressing the image

	//This object will represent the result of file reading.
	ImageBuffer_Byte decompressed_image(
			actual_num_rows,
			_image_info._width,
			_image_info._layout,
			_image_info._bit_depth);

	//Pointer to the beginning of array of rows where decompressed rows will be stored.
	//JSAMPARRAY is an array of row pointers, where rows themselves are arrays of uchar. So, basically JSAMPARRAY = unsigned char**.
	//This pointer is the beginning of ImageBuffer_Byte data array.
	JSAMPARRAY decompressed_data_array = static_cast<JSAMPARRAY>(decompressed_image.GetDataPtr());

	try {
		//Aqcuiring decompressed rows.
		//jpeg_read_scanlines requires array of rows as a buffer since it can read multiple rows at once.
		//We read one row at a time because ImageBufferByte data rows are not allocated continuously.
		for (int row = 0; row < actual_num_rows; row++)
			jpeg_read_scanlines(
				&jpeg_decomp,							//Jpeg image managing object.
				&(decompressed_data_array[row]),		//Buffer for reading current row.
				1										//Number of lines to be read at once.
			);
	}
	catch (codec_fatal_exception e) {
		/* delete decompressed_image; //Deleting the uncompressed image object */ //Archived from the times when the buffer was returned as a pointer
		_state = ReaderStates::Failed;
		CleanUp();
		throw; //rethrowing
	}

	//----------------------------------------------------------------------
	// 4 - Finishing decompressing

	//If we have finished reading the file we close it and deallocate the decompressor
	if (_state == ReaderStates::Finished) {
		//Finalizing reading (not that well described step, may be unnecessary)
		jpeg_finish_decompress(&jpeg_decomp);
		//Releasing the decompressor object memory
		jpeg_destroy_decompress(&jpeg_decomp);
		_is_decompressor_initialized = false;
		//Closing the file
		fclose(_file_handle);
		_is_file_opened = false;
	}

	//If this was first block to read we switch state to continue
	if (_state == ReaderStates::Ready_Start)
		_state = ReaderStates::Ready_Continue;

	//Returning decompressed image object
	return decompressed_image;
}



//--------------------------------
//	WHOLE FILE READING
//--------------------------------

///<summary>
///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
///<para>Can throw std::ifstream::failure if failed to open file.</para>
///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
///</summary>
///<param name="file_path">Path to file to read.</param>
///<param name="headerPtr">Writes info to JPEG header located at this pointer. If NULL it is ignored.</param>
///<param name="warningCallback">Pointer to function to handle warnings produced by libJpeg. Can be NULL.</param>
///<param name="warningCallbackArgsPtr">Arguments to be given to warning handler function. Can be NULL.</param>
ImageBuffer_Byte JpegReader::ReadJpegFile(std::filesystem::path file_path, JpegHeaderInfo* headerPtr, WarningCallbackData warning_callback_data) {
	JpegReader reader(file_path, warning_callback_data);
	JpegHeaderInfo header = reader.GetJpegHeader();

	//Returning header content to the user
	if (headerPtr != nullptr) {
		headerPtr->_height = header.GetHeight();
		headerPtr->_width = header.GetWidth();
		headerPtr->_color_space = header.GetColorSpace();
		headerPtr->_num_components = header.GetNumComponents();
	}
	return reader.ReadNextRows(header._height);
}




//--------------------------------
//	PUBLIC CONSTRUCTORS
//--------------------------------

///<summary>
///<para>Opens file pointed by file_path and reads the file header.</para>
///<para>Header is accessible by calling GetCommonHeader() and GetJpegHeader().</para>
///<para>Can throw std::ifstream::failure if failed to open file.</para>
///<para>Can throw codec_fatal_exception if failed to decompress the header.</para>
///</summary>
JpegReader::JpegReader(std::filesystem::path file_path, WarningCallbackData warning_callback_data) : ImageReader(file_path) {
	//----------------------------------------------------------------------
	// 0 - Setting initial state of the reader object

	_state = ReaderStates::Uninitialized;
	_is_file_opened = false;
	_is_decompressor_initialized = false;
	_warning_callback_data.warningCallback = warning_callback_data.warningCallback;
	_warning_callback_data.warningCallbackArgs_ptr = warning_callback_data.warningCallbackArgs_ptr;

	//----------------------------------------------------------------------
	// 1 - Opening the file pointed by file_path

	OpenCFileForBinaryReading(file_path);

	//----------------------------------------------------------------------
	// 2 - Initializing decompressor data structures

	//Assigning error manager to decompressor
	jpeg_decomp.err = jpeg_std_error(&jerr_decomp);
	//Registering jpeg error handler
	jerr_decomp.error_exit = &ErrorExitHandler;
	//Registering warning handler function
	jerr_decomp.emit_message = &WarningHandler;
	//Initializing decompressor object
	jpeg_create_decompress(&jpeg_decomp);

	//Setting opened file as a source for decompressor
	jpeg_stdio_src(&jpeg_decomp, _file_handle);

	//Referencing this warning callback in the decompressor object
	jpeg_decomp.client_data = &_warning_callback_data;

	//Setting state flag
	_is_decompressor_initialized = true;

	//----------------------------------------------------------------------
	// 3 - Reading the image header 

	//Reading JPEG header to get the image info.
	//If error is encountered ErrorExitHandler will be called.
	try {
		jpeg_read_header(&jpeg_decomp, TRUE);
	}
	catch (codec_fatal_exception e) {
		//Closing the file
		fclose(_file_handle); 
		_is_file_opened = false;
		//Deallocating the decompressor
		jpeg_destroy_decompress(&jpeg_decomp);
		_is_decompressor_initialized = false;
		//Rethrowing
		_state = ReaderStates::Failed;
		throw;
	}

	//Filling the info read from the header

	//Jpeg header object
	_jpeg_header._height = jpeg_decomp.image_height;
	_jpeg_header._width = jpeg_decomp.image_width;
	_jpeg_header._num_components = jpeg_decomp.num_components;
	_jpeg_header._color_space = jpeg_decomp.jpeg_color_space;

	//Common header object
	_image_info._height = static_cast<int>(jpeg_decomp.image_height);
	_image_info._width = static_cast<int>(jpeg_decomp.image_width);
	_image_info._bit_depth = BitDepth::BD_8_BIT; //Jpeg can only be 8 bit per channel
	_image_info._layout = JpegLayoutToImageLayout(_jpeg_header._color_space);

	//----------------------------------------------------------------------
	// 4 - Setting the decompressor to the ready state

	//Setting decompressor output colorspace
	//We decompress to grayscale if the file is explicitly grayscale.
	//We convert all other file color spaces to RGB when decompressing.
	if (jpeg_decomp.jpeg_color_space == J_COLOR_SPACE::JCS_GRAYSCALE)
		jpeg_decomp.out_color_space = J_COLOR_SPACE::JCS_GRAYSCALE;
	else
		jpeg_decomp.out_color_space = J_COLOR_SPACE::JCS_RGB;

	//Setting decompression method choosing slow and accurate
	jpeg_decomp.dct_method = J_DCT_METHOD::JDCT_ISLOW;

	//Setting decompressor to ready state
	//Image info will be available after this call
	jpeg_start_decompress(&jpeg_decomp);

	//Now file is opened and decompressor is ready to decompress the image lines
	_state = ReaderStates::Ready_Start;
}



//--------------------------------
//	UTILITY METHODS
//--------------------------------

/// <summary>
/// Translates jpeg file colorspace (pixel layout) to appropriate pixel layout used in ImageBuffer objects.
/// </summary>
ImagePixelLayout JpegReader::JpegLayoutToImageLayout(J_COLOR_SPACE jpeg_colorspace) {
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
//	PRIVATE METHODS
//--------------------------------

///<summary>
///Closes file and destroys decompressor
///if file is opened and decompressor exists.
///</summary>
void JpegReader::CleanUp() {
	//Deallocating decompressor if exists
	if (_is_decompressor_initialized == true) {
		jpeg_destroy_decompress(&jpeg_decomp);
		_is_decompressor_initialized = false;
	}
	//Closing file if opened
	if (_is_file_opened == true) {
		fclose(_file_handle);
		_is_file_opened = false;
	}
}


///<summary>
///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
///<para>Can throw std::ifstream::failure if failed to open file.</para>
///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
///</summary>
///<param name="file_path">Path to file to read.</param>
///<param name="headerPtr">Writes info to JPEG header located at this pointer. If NULL it is ignored.</param>
///<param name="warningCallback">Pointer to function to handle warnings produced by libJpeg. Can be NULL.</param>
///<param name="warningCallbackArgsPtr">Arguments to be given to warning handler function. Can be NULL.</param>
ImageBuffer_Byte* JpegReader::ReadJpegFile_Archive(std::filesystem::path file_path, JpegHeaderInfo* headerPtr, void (*warningCallback)(std::string, void*), void* warningCallbackArgsPtr) {

	//----------------------------------------------------------------------
	// 1 - Opening the file pointed by file_path

	//Extracting file path as UTF-8 c-style string to use with c file opener.
	const char* c_file_path = reinterpret_cast<const char*>(file_path.generic_u8string().c_str());
	FILE* file_handle;
	//Trying to open the file in reading binary mode
	errno_t fopen_error = fopen_s(&file_handle, c_file_path, "rb");
	//Checking if failed to open the file
	if (fopen_error != 0) {
		throw std::ifstream::failure("Failed to open file."); //Throwing exception
	}

	//----------------------------------------------------------------------
	// 2 - Initializing decompressor data structures

	//Ddeclaring libjpeg decompressor
	struct jpeg_decompress_struct decomp;
	//Declaring libjpeg error manager
	struct jpeg_error_mgr jerr;

	//Assigning error manager to decompressor
	decomp.err = jpeg_std_error(&jerr);
	//Registering jpeg error handler
	jerr.error_exit = &ErrorExitHandler;
	//Registering warning handler function if it is provided
	WarningCallbackData warning_callback_data;
	if (warningCallback != NULL) {
		//Referencing warning callback and its arguments in decompressor object
		warning_callback_data.warningCallback = warningCallback;
		warning_callback_data.warningCallbackArgs_ptr = warningCallbackArgsPtr;
		decomp.client_data = &warning_callback_data;

		//Registering warning callback
		jerr.emit_message = &WarningHandler;
	}

	//Initializing decompressor object
	jpeg_create_decompress(&decomp);

	//Setting opened file as a source for decompressor
	jpeg_stdio_src(&decomp, file_handle);

	//----------------------------------------------------------------------
	// 3 - Reading the image header 

	//Reading JPEG header to get the image info.
	//If error is encountered ErrorExitHandler will be called.
	try {
		jpeg_read_header(&decomp, TRUE);
	}
	catch (codec_fatal_exception e) {
		//Deleting warning callback data
		if (warningCallback != NULL)
			delete decomp.client_data;
		//Closing the file
		fclose(file_handle);
		//Deallocating the decompressor
		jpeg_destroy_decompress(&decomp);
		//Rethrowing
		throw;
	}

	//Writing header info into header object
	if (headerPtr != NULL) {
		headerPtr->_color_space = decomp.jpeg_color_space;
		headerPtr->_height = decomp.image_height;
		headerPtr->_width = decomp.image_width;
		headerPtr->_num_components = decomp.num_components;
	}

	//We try to decode jpeg to RGB color space in all cases when it is not explicitly grayscale
	ImagePixelLayout decompressed_layout;
	switch (decomp.jpeg_color_space) {
		case J_COLOR_SPACE::JCS_GRAYSCALE:
			decompressed_layout = ImagePixelLayout::G;
			break;

		case J_COLOR_SPACE::JCS_RGB:
			decompressed_layout = ImagePixelLayout::RGB;
			break;

		default:
			decompressed_layout = ImagePixelLayout::RGB;
			break;
	}

	//----------------------------------------------------------------------
	// 4 - Setting the decompressor to the ready state

	//Setting decompressor output colorspace
	//We decompress to grayscale if the file is explicitly grayscale.
	//We convert all other file color spaces to RGB when decompressing.
	if (decomp.jpeg_color_space == J_COLOR_SPACE::JCS_GRAYSCALE)
		decomp.out_color_space = J_COLOR_SPACE::JCS_GRAYSCALE;
	else
		decomp.out_color_space = J_COLOR_SPACE::JCS_RGB;

	//Setting decompression method chosing slow and accurate
	decomp.dct_method = J_DCT_METHOD::JDCT_ISLOW;

	//Setting decompressor to ready state
	//Image info will be available after this call
	jpeg_start_decompress(&decomp);

	//----------------------------------------------------------------------
	// 5 - Decompressing the image

	//This object will represent the result of file reading.
	ImageBuffer_Byte* decompressed_image =
		new ImageBuffer_Byte(
			decomp.image_height,
			decomp.image_width,
			decompressed_layout,
			BitDepth::BD_8_BIT);

	//Pointer to the beginning of array of rows where decompressed rows will be stored.
	//JSAMPARRAY is an array of row pointers, where rows themselves are arrays of uchar. So, basically JSAMPARRAY = unsigned char**.
	//This pointer is the beginning of ImageBuffer_Byte data array.
	JSAMPARRAY decompressed_data_array = static_cast<JSAMPARRAY>(decompressed_image->GetDataPtr());

	try {
		//Aqcuiring decompressed rows.
		//jpeg_read_scanlines requires array of rows as a buffer since it can read multiple rows at once.
		//We read one row at a time because ImageBufferByte data rows are not allocated continuously.
		for (unsigned int row = 0; row < decomp.image_height; row++)
			jpeg_read_scanlines(
				&decomp,								//Jpeg image managing object.
				&(decompressed_data_array[row]),		//Buffer for reading current row.
				1										//Number of lines to be read at once.
			);
	}
	catch (codec_fatal_exception e) {
		//Deleting warning callback data
		if (warningCallback != NULL)
			delete decomp.client_data;
		//Closing the file
		fclose(file_handle);
		//Deallocating the decompressor
		jpeg_destroy_decompress(&decomp);
		//Rethrowing
		throw;
	}


	//----------------------------------------------------------------------
	// 6 - Finishing decompressing

	//Finalizing reading (not that well described step, may be unnecessary)
	jpeg_finish_decompress(&decomp);
	//Deleting warning callback data
	if (warningCallback != NULL)
		delete decomp.client_data;
	//Releasing the decompressor object memory
	jpeg_destroy_decompress(&decomp);
	//Closing the file
	fclose(file_handle);

	//Returning the result
	return decompressed_image;
}