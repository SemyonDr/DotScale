#include "PngReader.h"



//--------------------------------
//	CHUNK READER
//--------------------------------

///<summary>
///Reads next block of rows starting at NextRow.
///Returns resulting bitmap.
///Advances NextRow by num_lines.
///If num_lines is bigger than number of rows left reads all available rows.
///When all rows are already read returns nullptr.
///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
///</summary>
ImageBuffer_Byte PngReader::ReadNextRows(int num_rows) {

	//----------------------------------------------------------------------
	// 1 - Arguments check

	//Simple argument sanity check
	if (num_rows <= 0)
		return ImageBuffer_Byte(_image_info.GetHeight(), _image_info.GetWidth(), _image_info.GetLayout(), _image_info.GetBitDepth());

	// Checking reader state -----------------------------------------------

	//If reading has finished we return unallocated image
	if (_state == ReaderStates::Finished)
		return ImageBuffer_Byte(_image_info.GetHeight(), _image_info.GetWidth(), _image_info.GetLayout(), _image_info.GetBitDepth());

	//If reader has failed before we throw
	if (_state == ReaderStates::Failed) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Trying to read a file using failed reader object.");
	}

	//If reader wasn't initialized we throw
	if (_state == ReaderStates::Uninitialized) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Trying to read a file using uninitialized reader object.");
	}

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

	//Image data alias
	png_bytepp decompressed_data = reinterpret_cast<png_bytepp>(decompressed_image.GetDataPtr()); 

	//As for writing of this code libpng documentation states
	//that only two possible interlacement schemes exists for png -
	//1) not interlaced - requires to do nothing special
	//2) adam7 - requires to perform 7 passes over the same lines when reading
	int num_passes = 1; // Default - PNG_INTERLACE_NONE
	if (_png_header._png_interlace_type == PNG_INTERLACE_ADAM7)
		//We have to read the image in 7 passes
		num_passes = PNG_INTERLACE_ADAM7_PASSES;
	try {
		for (int pass = 0; pass < num_passes; pass++)
			png_read_rows(
				_png_read_struct_ptr,	//libPNG decompressor structure
				decompressed_data,		//Buffer for storing uncompressed image
				NULL,					//Buffer for storing progressively read image prepared for display (not needed)
				actual_num_rows			//Number of rows to read
			);
	}
	catch (codec_fatal_exception e) {
		/* delete decompressed_image; //Deleting the uncompressed image object */ //Archived from the times when this method returned a pointer
		_state = ReaderStates::Failed;
		CleanUp();
		throw; //rethrowing
	}



	//----------------------------------------------------------------------
	// 4 - Finishing decompressing

	//If we finished reading the file we close it and deallocate the decompressor
	if (_state == ReaderStates::Finished) {
		//Finalizing reading
		png_read_end(
			_png_read_struct_ptr,	//PNG decompressor object
			NULL);					//PNG info object. Refer to page 43 of the manual.
		//Releasing the decompressor object memory
		CleanUp();
	}

	//Returning decompressed image container
	return decompressed_image;
}





//--------------------------------
//	WHOLE FILE READING
//--------------------------------

///<summary>
///Archived method. Whole reading procedure is written for one pass.
///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
///<para>Can throw std::ifstream::failure if failed to open file.</para>
///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
///</summary>
///<param name="file_path">Path to file to read.</param>
///<param name="headerPtr">Writes info to PNG header located at this pointer. If NULL it is ignored.</param>
///<param name="warningCallback">Pointer to function to handle warnings produced by libPng. Can be NULL.</param>
///<param name="warningCallbackArgsPtr">Arguments to be given to warning handler function. Can be NULL.</param>
ImageBuffer_Byte PngReader::ReadPngFile(std::filesystem::path file_path, PngHeaderInfo* headerPtr, WarningCallbackData warning_callback_data) {
	//Creating reader object
	PngReader reader(file_path, warning_callback_data);

	PngHeaderInfo png_header = reader.GetPngHeader();

	//Filling header object for the user
	if (headerPtr != nullptr) {
		headerPtr->_height = png_header.GetHeight();
		headerPtr->_width = png_header.GetWidth();
		headerPtr->_bit_depth = png_header.GetBitDepth();
		headerPtr->_png_color_type = png_header.GetPngColorType();
		headerPtr->_png_interlace_type = png_header.GetPngInterlaceType();
	}

	//Reading all rows of the file
	return reader.ReadNextRows(png_header.GetHeight());
}




//--------------------------------
//	PUBLIC CONSTRUCTORS
//--------------------------------

///<summary>
///<para>Opens file pointed by file_path and reads the file header.</para>
///<para>Header is accessible by calling GetCommonHeader() and GetPngHeader().</para>
///<para>Can throw std::ifstream::failure if failed to open file.</para>
///<para>Can throw codec_fatal_exception if failed to decompress the header.</para>
///</summary>
PngReader::PngReader(std::filesystem::path file_path, WarningCallbackData warning_callback_data) : ImageReader(file_path) {
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

	//Checking if file is a PNG by reading the signature
	const png_byte file_signature[8] = { 0,0,0,0,0,0,0,0 }; //Buffer for holding the signature
	fread((void*)file_signature, 1, 8, _file_handle);	  //Reading 8 first bytes
	if (png_sig_cmp(file_signature, 0, 8) != 0) { //Checking if first 8 bytes from the buffer constitute png signature (returns 0 if it IS)
		//Closing the file, throwing exception
		fclose(_file_handle);
		_is_file_opened = false;
		_state = ReaderStates::Failed;
		throw codec_fatal_exception(CodecExceptions::Png_DecodingError, "Invalid PNG file signature.");
	}


	//----------------------------------------------------------------------
	// 2 - Initializing decompressor data structures

	//Allocating png structure for reading
	_png_read_struct_ptr = png_create_read_struct(
		PNG_LIBPNG_VER_STRING,								//Library version
		static_cast<png_voidp>(&_warning_callback_data),	//Referencing warning callback data in png structure
		reinterpret_cast<png_error_ptr>(ErrorExitHandler),	//Error handler function
		reinterpret_cast<png_error_ptr>(WarningHandler)		//Warning handler function
	);
	if (_png_read_struct_ptr == NULL) { //Failed to allocate
		//Closing the file, throwing exception
		fclose(_file_handle);
		_is_file_opened = false;
		_state = ReaderStates::Failed;
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Failed to initialize PNG decoder.");
	}

	//Allocating info structure for reading
	_png_info_ptr = png_create_info_struct(_png_read_struct_ptr);
	if (_png_info_ptr == NULL) { //Failed to allocate
		//Closing the file, destroying png object, throwing exception
		png_destroy_read_struct(&_png_read_struct_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(_file_handle);
		_is_file_opened = false;
		_state = ReaderStates::Failed;
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Failed to initialize PNG decoder.");
	}

	//Setting state flag
	_is_decompressor_initialized = true;

	//Initializing reading mechanism
	png_init_io(_png_read_struct_ptr, _file_handle);

	//Telling the png reader that we already have read 8 bytes to check the signature
	png_set_sig_bytes(_png_read_struct_ptr, 8);

	//Telling decoder that we want to ignore all unknown data chunks
	png_set_keep_unknown_chunks(_png_read_struct_ptr, PNG_HANDLE_CHUNK_NEVER, NULL, 0);


	//----------------------------------------------------------------------
	// 3 - Reading the image header 

	//Reading info header of the image
	try {
		png_read_info(_png_read_struct_ptr, _png_info_ptr);
	}
	catch (codec_fatal_exception e) {
		//Removing the structures
		png_destroy_read_struct(&_png_read_struct_ptr, &_png_info_ptr, (png_infopp)NULL);
		_is_decompressor_initialized = false;
		//Closing the file
		fclose(_file_handle);
		_is_file_opened = false;
		//Rethrowing
		_state = ReaderStates::Failed;
		throw;
	}

	//Filling png header data
	_png_header._height = png_get_image_height(_png_read_struct_ptr, _png_info_ptr);
	_png_header._width = png_get_image_width(_png_read_struct_ptr, _png_info_ptr);
	_png_header._bit_depth = png_get_bit_depth(_png_read_struct_ptr, _png_info_ptr);
	_png_header._png_color_type = png_get_color_type(_png_read_struct_ptr, _png_info_ptr);
	_png_header._png_interlace_type = png_get_interlace_type(_png_read_struct_ptr, _png_info_ptr);

	//Filling common header
	_image_info._height = _png_header._height;
	_image_info._width = _png_header._width;
	_image_info._bit_depth = PngBitDepthToImageBitDepth(_png_header._bit_depth);
	_image_info._layout = PngLayoutToImageLayout(_png_header._png_color_type);


	//----------------------------------------------------------------------
	// 4 - Setting the decompressor to a ready state

	//Telling decompressor to apply following transformations when decompressing if needed:

	//Transformation - Bit depth expansion
	//If we have grayscale image with bit depth lower than 8 we tell decompressor to expand the bit depth to 8
	if (_png_header._png_color_type == PNG_COLOR_TYPE_GRAY && _png_header._bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(_png_read_struct_ptr);

	//Transformation - byte order swap
	//16 bit PNG is stored in big endian (network format) - most significant byte first.
	//We ask the decompressor to reverse byte order to little endian (x86 format) - least significant byte first.
	if (_png_header._bit_depth == 16)
		png_set_swap(_png_read_struct_ptr);

	//Transformation - Palette to RGB
	//If PNG colors are on a palette we transform it into RGB
	if (_png_header._png_color_type == PNG_COLOR_TYPE_PALETTE) {
		//Additionally we set transformation to expand bit depth to 8 if it is less than 8
		if (_png_header._bit_depth < 8)
			png_set_expand_gray_1_2_4_to_8(_png_read_struct_ptr);
		png_set_palette_to_rgb(_png_read_struct_ptr);
	}

	//We update the info structure to account for transformations we set above.
	png_read_update_info(_png_read_struct_ptr, _png_info_ptr);

	//Now file is opened and decompressor is ready to decompress the image lines
	_state = ReaderStates::Ready_Start;
}




//--------------------------------
//	PRIVATE METHODS
//--------------------------------

///<summary>
///Closes file and destroys decompressor
///if file is opened and decompressor exists.
///</summary>
void PngReader::CleanUp() {
	//Deallocating decompressor if exists
	if (_is_decompressor_initialized == true) {
		png_destroy_read_struct(&_png_read_struct_ptr, &_png_info_ptr, (png_infopp)NULL);
		_is_decompressor_initialized = false;
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
/// Translates PNG file color type (pixel layout) to corresponding pixel layot used in ImageBuffer objects.
/// </summary>
ImagePixelLayout PngReader::PngLayoutToImageLayout(unsigned char png_file_color_type) {
	switch (png_file_color_type) {
		case PNG_COLOR_TYPE_GRAY:
			return ImagePixelLayout::G;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			return ImagePixelLayout::GA;
		case PNG_COLOR_TYPE_RGB:
			return ImagePixelLayout::RGB;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			return ImagePixelLayout::RGBA;
		case PNG_COLOR_TYPE_PALETTE:
			//We will set transformation to transform palette into RGB
			return ImagePixelLayout::RGB;
		default:
			return ImagePixelLayout::UNDEF;
	}
}

/// <summary>
/// Translates PNG file bit depth to corresponding bit depth parameter used in ImageBuffer objects.
/// </summary>
/// <param name="png_bit_depth"></param>
/// <returns></returns>
BitDepth PngReader::PngBitDepthToImageBitDepth(unsigned int png_bit_depth) {
	if (png_bit_depth <= 8)
	//If bit depth is less than 8 we will set png decoder transformation
	//to expand it to 8bit when decompressing
		return BitDepth::BD_8_BIT;
	else
		return BitDepth::BD_16_BIT;
}




//--------------------------------
//	ARCHIVED METHODS
//--------------------------------

///<summary>
///Archived method. Whole reading procedure is written for one pass.
///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
///<para>Can throw std::ifstream::failure if failed to open file.</para>
///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
///</summary>
///<param name="file_path">Path to file to read.</param>
///<param name="headerPtr">Writes info to PNG header located at this pointer. If NULL it is ignored.</param>
///<param name="warningCallback">Pointer to function to handle warnings produced by libPng. Can be NULL.</param>
///<param name="warningCallbackArgsPtr">Arguments to be given to warning handler function. Can be NULL.</param>
ImageBuffer_Byte* PngReader::ReadPngFile_Archive(std::filesystem::path file_path, PngHeaderInfo* headerPtr, WarningCallbackData warning_callback_data) {
	//----------------------------------------------------------------------
	// 1 - Opening the file pointed by file_path

	//Extracting file path as UTF-8 c-style string to use with c file opener.
	const char* c_file_path = reinterpret_cast<const char*>(file_path.generic_u8string().c_str());
	FILE* file_handle;
	//Trying to open the file in reading binary mode
	errno_t fopen_error = fopen_s(&file_handle, c_file_path, "rb");
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

	//Checking if file is a PNG by reading the signature
	const png_byte file_signature[8] = { 0,0,0,0,0,0,0,0 }; //Buffer for holding the signature
	fread((void*)file_signature, 1, 8, file_handle);	  //Reading 8 first bytes
	if (png_sig_cmp(file_signature, 0, 8) != 0) { //Checking if first 8 bytes from the buffer constitute png signature (returns 0 if it IS)
		//Closing the file, throwing exception
		fclose(file_handle);
		throw codec_fatal_exception(CodecExceptions::Png_DecodingError, "Invalid PNG file signature.");
	}

	//----------------------------------------------------------------------
	// 2 - Initializing decompressor data structures

	//Allocating and initializing png reader structure
	//Note: If error_ptr is not set (NULL) setjump should be set up (see libpng manual).
	//	    setjump is omitted since we use ErrorExitHandler to catch fatal errors.
	//Note: If warn_fn is not set (NULL) warnings will be printed to standard output. 
	//      If callback provided by user in warning_callback_data is NULL 
	//      WarningHandler() will be called and will simply do nothing.
	png_structp png_read_struct_ptr = png_create_read_struct(
		PNG_LIBPNG_VER_STRING,								// Library version
		static_cast<png_voidp>(&warning_callback_data),		// error_ptr - Referencing warning callback and its arguments
															// in reader object (accessible by calling png_get_error_ptr(png_ptr))
		reinterpret_cast<png_error_ptr>(&ErrorExitHandler),	// error_fn - Fatal error handler function
		reinterpret_cast<png_error_ptr>(&WarningHandler)	// warn_fn - Warning handler function
	);
	if (png_read_struct_ptr == NULL) { //Failed to allocate
		//Closing the file, throwing exception
		fclose(file_handle);
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Failed to initialize PNG decoder.");
	}

	//Allocating info structure for reading
	png_infop png_info_ptr = png_create_info_struct(png_read_struct_ptr);
	if (png_info_ptr == NULL) { //Failed to allocate
		//Closing the file, destroying png object, throwing exception
		png_destroy_read_struct(&png_read_struct_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(file_handle);
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Failed to initialize PNG decoder.");
	}

	//Initializing reading mechanism
	png_init_io(png_read_struct_ptr, file_handle);

	//Telling the png reader that we already have read 8 bytes to check the signature
	png_set_sig_bytes(png_read_struct_ptr, 8);

	//Telling decoder that we want to ignore all unknown data chunks
	png_set_keep_unknown_chunks(png_read_struct_ptr, PNG_HANDLE_CHUNK_NEVER, NULL, 0);


	//----------------------------------------------------------------------
	// 3 - Reading the image header 

	//Reading info header of the image
	try {
		png_read_info(png_read_struct_ptr, png_info_ptr);
	}
	catch (codec_fatal_exception e) {
		//Removing the structures
		png_destroy_read_struct(&png_read_struct_ptr, &png_info_ptr, (png_infopp)NULL);
		//Closing the file
		fclose(file_handle);
		//Rethrowing
		throw;
	}

	//Extracting png header data
	PngHeaderInfo png_header(
		png_get_image_height(png_read_struct_ptr, png_info_ptr),
		png_get_image_width(png_read_struct_ptr, png_info_ptr),
		png_get_bit_depth(png_read_struct_ptr, png_info_ptr),
		png_get_color_type(png_read_struct_ptr, png_info_ptr),
		png_get_interlace_type(png_read_struct_ptr, png_info_ptr)
	);

	//Copying header data for the user
	if (headerPtr != NULL) {
		headerPtr->_height = png_get_image_height(png_read_struct_ptr, png_info_ptr);
		headerPtr->_width = png_get_image_width(png_read_struct_ptr, png_info_ptr);
		headerPtr->_bit_depth = png_get_bit_depth(png_read_struct_ptr, png_info_ptr);
		headerPtr->_png_color_type = png_get_color_type(png_read_struct_ptr, png_info_ptr);
		headerPtr->_png_interlace_type = png_get_interlace_type(png_read_struct_ptr, png_info_ptr);
	}

	//----------------------------------------------------------------------
	// 4 - Setting the decompressor to ready state

	//Telling decompressor to apply following transformations when decompressing if needed:

	//Transformation - Bit depth expansion
	//If we have grayscale image with bit depth lower than 8 we tell decompressor to expand the bit depth to 8
	if (png_header._png_color_type == PNG_COLOR_TYPE_GRAY && png_header._bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_read_struct_ptr);

	//Transformation - byte order swap
	//16 bit PNG is stored in big endian (network format) - most significant byte first.
	//We ask the decompressor to reverse byte order to little endian (x86 format) - least significant byte first.
	if (png_header._bit_depth == 16)
		png_set_swap(png_read_struct_ptr);

	//Transformation - Palette to RGB
	//If PNG colors are on a palette we transform it into RGB
	if (png_header._png_color_type == PNG_COLOR_TYPE_PALETTE) {
		//Additionally we set transformation to expand bit depth to 8 if it is less than 8
		if (png_header._bit_depth < 8)
			png_set_expand_gray_1_2_4_to_8(png_read_struct_ptr);
		png_set_palette_to_rgb(png_read_struct_ptr);
	}

	//We update the info structure to account for transformations we set above.
	png_read_update_info(png_read_struct_ptr, png_info_ptr);

	//Now file is opened and decompressor is ready to decompress the image lines

	//----------------------------------------------------------------------
	// 5 - Decompressing the image


	//This object will represent the result of file reading.
	ImageBuffer_Byte* decompressed_image =
		new ImageBuffer_Byte(
			static_cast<int>(png_header._height),
			static_cast<int>(png_header._width),
			PngLayoutToImageLayout(png_header.GetPngColorType()),
			PngBitDepthToImageBitDepth(png_header.GetBitDepth())
		);

	//Image data alias
	png_bytepp decompressed_data = static_cast<png_bytepp>(decompressed_image->GetDataPtr());

	//As for writing of this code libpng documentation states
	//that only two possible interlacement schemes exists for png -
	//1) not interlaced - requires to do nothing special
	//2) adam7 - requires to perform 7 passes over the same lines when reading

	int num_passes = 1; // Default - PNG_INTERLACE_NONE
	if (png_header._png_interlace_type == PNG_INTERLACE_ADAM7)
		//We have to read the image in 7 passes
		num_passes = PNG_INTERLACE_ADAM7_PASSES;

	try {
		for (int pass = 0; pass < num_passes; pass++)
			png_read_rows(
				png_read_struct_ptr,	//libPNG decompressor structure
				decompressed_data,		//Buffer for storing uncompressed image
				NULL,					//Buffer for storing progressively read image prepared for display (not needed)
				png_header._height		//Number of rows to read
			);
	}
	catch (codec_fatal_exception e) {
		delete decompressed_image; //Deleting the uncompressed image object
		//Closing the file, destroying png object, throwing exception
		png_destroy_read_struct(&png_read_struct_ptr, &png_info_ptr, (png_infopp)NULL);
		fclose(file_handle);
		throw; //rethrowing
	}


	//----------------------------------------------------------------------
	// 6 - Finishing decompressing

	//If we finished reading the file we close it and deallocate the decompressor
	//Finalizing reading
	png_read_end(png_read_struct_ptr, NULL);
	//Releasing the decompressor object memory
	png_destroy_read_struct(&png_read_struct_ptr, &png_info_ptr, (png_infopp)NULL);
	//Closing the file
	fclose(file_handle);

	//Returning decompressed image container
	return decompressed_image;
}