#include "PngWriter.h"


//--------------------------------
//	PUBLIC METHODS
//--------------------------------

/// <summary>
/// <para>Static method. Compresses and writes image file to the given path.</para>
/// <para>Buffer parameters should match the header.</para>
/// <para>Can throw std::ifstream::failure if failed to open file.</para>
/// <para>Can throw codec_fatal_exception if failed to compress the image.</para>
/// </summary>
/// <param name="file_path">Path to the output file.</param>
/// <param name="image">Image to write.</param>
/// <param name="header">PNG header data. Should contain valid output colorspace.</param>
/// <param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
void PngWriter::WritePng(std::filesystem::path file_path, ImageBuffer_Byte* image, PngHeaderInfo header, WarningCallbackData warning_callback_data) {
	PngWriter writer(file_path, header, warning_callback_data);
	writer.WriteNextRows(image);
}



///<summary>
	///Writes next block of rows starting at NextRow.
	///Advances NextRow by the height of given image.
	///If number of lines in the provided image is bigger than number of rows left writes what is possible.
	///</summary>
void PngWriter::WriteNextRows(ImageBuffer_Byte* image) {

	//Aliases
	unsigned int header_bit_depth = _png_header.GetBitDepth();
	unsigned int header_layout = _png_header.GetPngColorType();
	unsigned char header_interlacing = _png_header.GetPngInterlaceType();
	unsigned int header_height = _png_header.GetHeight();

	unsigned int buffer_layout = ImageLayoutToPngLayout(image->GetLayout());
	unsigned int buffer_bit_depth = ImageBitDepthToPngBitDepth(image->GetBitPerComponent());
	unsigned int buffer_height = static_cast<unsigned int>(image->GetHeight());

	//----------------------------------------------------------------------
	// 1 - Arguments check

	// Checking writer state -----------------------------------------------

	//If writing is finished we do nothing
	if (_state == WriterStates::Finished)
		return;

	//If writer has failed we throw
	if (_state == WriterStates::Failed) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Trying to write a file using failed writer object.");
	}

	//If writer wasn't initialized we throw
	if (_state == WriterStates::Uninitialized) {
		CleanUp();
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Trying to write a file using uninitialized writer object.");
	}

	// Checking buffer -----------------------------------------------------

	if (image->GetLayout() == ImagePixelLayout::UNDEF)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Cannot write an image with undefined layout.");

	if (image->GetBitPerComponent() == BitDepth::BD_32_BIT)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "PNG file does not support bit depth of 32 bit per channel.");

	//Checking if provided image conforms with this writer settings --------

	if (header_layout != PNG_COLOR_TYPE_PALETTE) {
		if (buffer_layout != header_layout)
			throw codec_fatal_exception(CodecExceptions::Png_InitError, "Layout mismatch bitween provided header and provided image.");
	}
	else {
		if (buffer_layout != PNG_COLOR_TYPE_GRAY
			&& buffer_layout != PNG_COLOR_TYPE_GA)
			throw codec_fatal_exception(CodecExceptions::Png_InitError, "Paletted PNG mode only accepts grayscale and grayscale-alpha image buffers as input.");
	}

	if (image->GetWidth() != _png_header.GetWidth())
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Width mismatch between provided header and provided image.");

	if (header_bit_depth < 8) {
		if (buffer_bit_depth != 8)
			throw codec_fatal_exception(CodecExceptions::Png_InitError, "To write png with bit depth less than 8 provide 8 bit image buffer.");
	}
	else {
		if (buffer_bit_depth != header_bit_depth)
			throw codec_fatal_exception(CodecExceptions::Png_InitError, "Bit depth set in the header do not match provided image.");
	}

	//----------------------------------------------------------------------
	// 2 - Remaining rows calculation

	//Calculating num rows actually available to write and advancing the row counter.
	int actual_num_rows = AdvanceRows(static_cast<int>(buffer_height));

	//----------------------------------------------------------------------
	// 3 - Compressing and writing the image

	ImageBuffer_Byte* bitcrushed_image = nullptr;
	if (_is_low_depth_grayscale)
		bitcrushed_image = CrushBitDepth(image, header_bit_depth);

	//Data alias for writing
	png_bytepp image_data;
	if (_is_low_depth_grayscale)
		image_data = static_cast<png_bytepp>(bitcrushed_image->GetData());
	else
		image_data = static_cast<png_bytepp>(image->GetData());

	//Appending image buffer rows to the file
	try {
		png_write_rows(png_ptr, image_data, actual_num_rows);
	}
	catch (codec_fatal_exception e) {
		_state = WriterStates::Failed;
		CleanUp();
		throw;
	}

	//----------------------------------------------------------------------
	// 4 - Cleaning
	// 
	//If we have finished writing the file we close it and deallocate the compressor
	if (_state == WriterStates::Finished) {
		//Finishing the writing
		png_write_end(png_ptr, info_ptr);

		//Deallocating the structures
		png_destroy_write_struct(&png_ptr, &info_ptr);

		//Closing the file
		fclose(_file_handle);
	}

	//Deallocating bitcrushed image
	if (_is_low_depth_grayscale)
		delete bitcrushed_image;

	//If this was first block of writing we switch state to continue
	if (_state == WriterStates::Ready_Start)
		_state = WriterStates::Ready_Continue;
}




//--------------------------------
//	PUBLIC CONSTRUCTORS
//--------------------------------


///<summary>
///<para>Opens file pointed by file_path and writes the header.</para>
///<para>Can throw std::ifstream::failure if failed to open file.</para>
///<para>Can throw codec_fatal_exception if failed to write the header.</para>
///</summary>
///<param name="file_path">Path to the file to be written.</param>
///<param name="header">PNG header describing the file.</param>
///<param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
PngWriter::PngWriter(std::filesystem::path file_path, PngHeaderInfo header, WarningCallbackData warning_callback_data) : ImageWriter(file_path) {
	//----------------------------------------------------------------------
	// 0 - Setting initial state of the writer object

	_state = WriterStates::Uninitialized;
	_is_file_opened = false;
	_is_compressor_initialized = false;

	// Saving arguments
	_png_header = header;
	_warning_callback_data.warningCallback = warning_callback_data.warningCallback;
	_warning_callback_data.warningCallbackArgs_ptr = warning_callback_data.warningCallbackArgs_ptr;

	//----------------------------------------------------------------------
	// 1 - Arguments check

	//Aliases
	unsigned int header_bit_depth = _png_header.GetBitDepth();
	unsigned int header_layout = _png_header.GetPngColorType();
	unsigned char header_interlacing = _png_header.GetPngInterlaceType();

	// If we need to perform bit crush on ImageBuffers before writing them
	if (header_bit_depth < 8 && header_layout != PNG_COLOR_TYPE_GRAY)
		_is_low_depth_grayscale = true;

	// Checking header parameters ------------------------------------

	if (header_bit_depth != 1
		&& header_bit_depth != 2
		&& header_bit_depth != 4
		&& header_bit_depth != 8
		&& header_bit_depth != 16)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Invalid bit depth is provided.");

	if (header_layout != PNG_COLOR_TYPE_GRAY
		&& header_layout != PNG_COLOR_TYPE_GA
		&& header_layout != PNG_COLOR_TYPE_RGB
		&& header_layout != PNG_COLOR_TYPE_RGBA
		&& header_layout != PNG_COLOR_TYPE_PALETTE)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Invalid parameter for color type.");

	if (header_interlacing != PNG_INTERLACE_NONE
		&& header_interlacing != PNG_INTERLACE_ADAM7)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Invalid parameter for interlacing type.");

	if (header_interlacing == PNG_INTERLACE_ADAM7)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Cannot write interlaced image in chunks.");

	if (header_layout == PNG_COLOR_TYPE_PALETTE)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Currently paletted PNG is not supported.");

	if (header_layout == PNG_COLOR_TYPE_PALETTE && header_bit_depth == 16)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Paletted PNG does not support 16 bit.");

	if (header_bit_depth < 8 && header_layout != PNG_COLOR_TYPE_GRAY && header_layout != PNG_COLOR_TYPE_PALETTE)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Bit depth of less than 8 bit is only acceptable for grayscale and palletted images.");

	// Saving image parameters for future reference ------------------------
	_image_info._height = static_cast<int>(header.GetHeight());
	_image_info._width = static_cast<int>(header.GetWidth());
	_image_info._layout = PngLayoutToImageLayout(header_layout);
	_image_info._bit_depth = PngBitDepthToImageBitDepth(header_bit_depth);


	//----------------------------------------------------------------------
	// 2 - Opening the file pointed by file_path for writing

	//Opening file in C mode for binary writing
	OpenCFileForBinaryWriting(file_path);

	//----------------------------------------------------------------------
	// 3 - Initializing compressor data structures

	//Allocating and initializing png writer structure
	//Note: If error_ptr is not set (NULL) setjump should be set up (see libpng manual).
	//	    setjump is omitted since we use ErrorExitHandler to catch fatal errors.
	//Note: If warn_fn is not set (NULL) warnings will be printed to standard output. 
	//      If callback provided by user in warning_callback_data is NULL 
	//      WarningHandler() will be called and will simply do nothing.
	png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,								// Library version
		static_cast<png_voidp>(&_warning_callback_data),	// error_ptr - Referencing warning callback and its arguments
															// in writer object (accessible by calling png_get_error_ptr(png_ptr))
		reinterpret_cast<png_error_ptr>(&ErrorExitHandler),	// error_fn - Fatal error handler function
		reinterpret_cast<png_error_ptr>(&WarningHandler)	// warn_fn - Warning handler function
	);
	if (png_ptr == NULL) { //Failed to allocate
		//Closing the file, throwing exception
		fclose(_file_handle);
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Failed to initialize PNG encoder.");
	}

	//Allocating info structure for writing
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) { //Failed to allocate
		//Closing the file, destroying png object, throwing exception
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(_file_handle);
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Failed to initialize PNG encoder.");
	}

	//Initializing writing mechanism
	try {
		png_init_io(png_ptr, _file_handle);
	}
	catch (codec_fatal_exception e) {
		//Closing the file, destroying png objects, rethrowing exception
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(_file_handle);
		throw;
	}

	//Setting state flag
	_is_compressor_initialized = true;

	//----------------------------------------------------------------------
	// 4 - Configuring compressor for given image

	//Providing encoder with basic info about the image and basic comression settings
	png_set_IHDR(
		png_ptr,	//PNG structure
		info_ptr,	//Info structure
		_png_header.GetWidth(),		//Image width in pixels
		_png_header.GetHeight(),		//Image height in pixels
		header_bit_depth,		//Per color bit depth
		header_layout,			//Data layout
		header_interlacing,		//Write file as interlaced or not
		PNG_COMPRESSION_TYPE_DEFAULT,	//Compression type - must be default
		PNG_FILTER_TYPE_DEFAULT			//Filter method - for non-embedded png stream must be default
	);

	//Writing the header
	try {
		png_write_info(png_ptr, info_ptr);
	}
	catch (codec_fatal_exception e) {
		//Closing the file, destroying png objects, rethrowing exception
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(_file_handle);
		throw;
	}

	//----------------------------------------------------------------------
	// 5 - Setting the compressor to the ready state

	//If the image is 16 bit per channel each channel bytes stored in little-endian format (least signigicant byte first).
	//PNG format stores bytes in big-endian format (most significant byte first).
	//So, we have to swap byte order.
	if (header_bit_depth == 16)
		png_set_swap(png_ptr);

	//If header is set to less than 8 bit and the layout is grayscale
	//we have to perform bit crush
	if (_is_low_depth_grayscale)
		//This setting tells the compressor that each byte holds bitcrushed value for one grayscale pixel
		png_set_packing(png_ptr);

	//File is opened, compressor is ready and the header was written
	_state = WriterStates::Ready_Start;
}





//--------------------------------
//	UTILITY METHODS
//--------------------------------

/// <summary>
/// Takes in an image with grayscale layout and outputs
/// grayscale image with given bit depth - 1, 2 or 4 bit per pixel.
/// One byte per value is used. To write PNG correctly set png_set_packing(png_ptr).
/// </summary>
/// <param name="image">Input image buffer assumed to be grayscale 8 bit per pixel.</param>
/// <param name="bit_depth">Desired bit depth. Possible values are 1, 2, 4.</param>
ImageBuffer_Byte* PngWriter::CrushBitDepth(ImageBuffer_Byte * src_image, int bit_depth) {
	//Aliases
	int height = src_image->GetHeight();
	int width = src_image->GetWidth();

	//Result buffer
	ImageBuffer_Byte* trg_image = new ImageBuffer_Byte(height, width, ImagePixelLayout::G, BitDepth::BD_8_BIT);

	//Data alias
	uint8_t** src_data = src_image->GetData();
	uint8_t** trg_data = trg_image->GetData();

	int divisor = 1;

	switch (bit_depth)
	{
		case 4:
			divisor = 16;
			break;
		case 2:
			divisor = 64;
			break;
		case 1:
			divisor = 128;
			break;
	}

	for (int row = 0; row < height; row++)
		for (int px = 0; px < width; px++)
			trg_data[row][px] = static_cast<uint8_t>(src_data[row][px] / divisor);

	return trg_image;
}


/// <summary>
/// Translates Pixel Layout of ImageBuffer to png file layout.
/// </summary>
unsigned int PngWriter::ImageLayoutToPngLayout(ImagePixelLayout layout) {
	switch (layout)
	{
		case UNDEF:
			return -1;
		case G:
			return PNG_COLOR_TYPE_GRAY;
		case GA:
			return PNG_COLOR_TYPE_GA;
		case RGB:
			return PNG_COLOR_TYPE_RGB;
		case RGBA:
			return PNG_COLOR_TYPE_RGBA;
	}

	return -1;
}

/// <summary>
/// Translates PNG file color type (pixel layout) to corresponding pixel layot used in ImageBuffer objects.
/// </summary>
ImagePixelLayout PngWriter::PngLayoutToImageLayout(unsigned char png_file_color_type) {
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
			//We will set transformation to transform RGB into Palette
			return ImagePixelLayout::RGB;
		default:
			return ImagePixelLayout::UNDEF;
	}
}



/// <summary>
/// Translates Bit Depth of ImageBuffer to png bit depth.
/// </summary>
unsigned int PngWriter::ImageBitDepthToPngBitDepth(BitDepth bit_depth) {
	switch (bit_depth)
	{
		case BD_8_BIT:
			return 8;
		case BD_16_BIT:
			return 16;
		case BD_32_BIT:
			return 32;
	}
	return 0;
}

/// <summary>
/// Translates PNG file bit depth to corresponding bit depth parameter used in ImageBuffer objects.
/// </summary>
/// <param name="png_bit_depth"></param>
/// <returns></returns>
BitDepth PngWriter::PngBitDepthToImageBitDepth(unsigned int png_bit_depth) {
	if (png_bit_depth <= 8)
	//If bit depth is less than 8 we will set png encoder transformation
	//to trim it from 8bit when decompressing
		return BitDepth::BD_8_BIT;
	else
		return BitDepth::BD_16_BIT;
}


//--------------------------------
//	PRIVATE METHODS
//--------------------------------

///<summary>
///Closes file and destroys compressor object
///if file is opened and compressor exists.
///</summary>
void PngWriter::CleanUp() {
	//Deallocating decompressor if exists
	if (_is_compressor_initialized == true) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		_is_compressor_initialized = false;
	}
	//Closing file if opened
	if (_is_file_opened == true) {
		fclose(_file_handle);
		_is_file_opened = false;
	}
}

//--------------------------------
//	ARCHIVED METHODS
//--------------------------------


/// <summary>
///Archived method. Whole reading procedure is written for one pass.
/// <para>Static method. Compresses and writes image file to the given path.</para>
/// <para>Buffer parameters should match the header.</para>
/// <para>Can throw std::ifstream::failure if failed to open file.</para>
/// <para>Can throw codec_fatal_exception if failed to compress the image.</para>
/// </summary>
/// <param name="file_path">Path to the output file.</param>
/// <param name="image">Image to write.</param>
/// <param name="header">PNG header data. Should contain valid output colorspace.</param>
/// <param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
void PngWriter::WritePng_Archive(std::filesystem::path file_path, ImageBuffer_Byte* image, PngHeaderInfo header, WarningCallbackData warning_callback_data) {
	//Currently we do not support palette.
	//To support it palettising algorithms should be added to ImageBuffer.
	//And a way to read and store a palette. Research needed.

	//Primary source of arguments for writing is the header and the buffer is checked to match the header.

	//Aliases
	unsigned int header_bit_depth = header.GetBitDepth();
	unsigned int header_layout = header.GetPngColorType();
	unsigned char header_interlacing = header.GetPngInterlaceType();

	unsigned int buffer_layout = ImageLayoutToPngLayout(image->GetLayout());
	unsigned int buffer_bit_depth = ImageBitDepthToPngBitDepth(image->GetBitPerComponent());

	//Flag that shows if file is grayscale and less than 8 bit per pixel
	//Because of image buffers have minimum bit depth 8 bit per pixel 
	//additional manipulations will be required
	//and the flag will be used to indicate the need to perform them
	bool bitcrush = false;
	if (header_bit_depth < 8 && header_layout != PNG_COLOR_TYPE_GRAY)
		bitcrush = true;

	//----------------------------------------------------------------------
	// 0 - Arguments check

	// 0.a - Checking buffer parameters
	if (image->GetLayout() == ImagePixelLayout::UNDEF)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Cannot write an image with undefined layout.");

	if (image->GetBitPerComponent() == BitDepth::BD_32_BIT)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "PNG file does not support bit depth of 32 bit per channel.");

	// 0.b - Checking header parameters

	if (header_bit_depth != 1
		&& header_bit_depth != 2
		&& header_bit_depth != 4
		&& header_bit_depth != 8
		&& header_bit_depth != 16)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Invalid bit depth is provided.");

	if (header_layout != PNG_COLOR_TYPE_GRAY
		&& header_layout != PNG_COLOR_TYPE_GA
		&& header_layout != PNG_COLOR_TYPE_RGB
		&& header_layout != PNG_COLOR_TYPE_RGBA
		&& header_layout != PNG_COLOR_TYPE_PALETTE)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Invalid parameter for color type.");

	if (header_interlacing != PNG_INTERLACE_NONE
		&& header_interlacing != PNG_INTERLACE_ADAM7)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Invalid parameter for interlacing type.");

	if (header_layout == PNG_COLOR_TYPE_PALETTE)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Currently paletted PNG is not supported.");

	if (header_layout == PNG_COLOR_TYPE_PALETTE && header.GetBitDepth() == 16)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Paletted PNG does not support 16 bit.");

	if (header_bit_depth < 8 && header_layout != PNG_COLOR_TYPE_GRAY && header_layout != PNG_COLOR_TYPE_PALETTE)
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Bit depth of less than 8 bit is only acceptable for grayscale and palletted images.");

	// 0.c - Checking that the buffer matches the header

	if (header_layout != PNG_COLOR_TYPE_PALETTE) {
		if (buffer_layout != header_layout)
			throw codec_fatal_exception(CodecExceptions::Png_InitError, "Layout mismatch bitween provided header and provided image.");
	}
	else {
		if (buffer_layout != PNG_COLOR_TYPE_GRAY
			&& buffer_layout != PNG_COLOR_TYPE_GA)
			throw codec_fatal_exception(CodecExceptions::Png_InitError, "Paletted PNG mode only accepts grayscale and grayscale-alpha image buffers as input.");
	}

	if (image->GetHeight() != header.GetHeight() || image->GetWidth() != header.GetWidth())
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Dimensions mismatch between provided header and provided image.");

	if (header_bit_depth < 8) {
		if (buffer_bit_depth != 8)
			throw codec_fatal_exception(CodecExceptions::Png_InitError, "To write png with bit depth less than 8 provide 8 bit image buffer.");
	}
	else {
		if (buffer_bit_depth != header_bit_depth)
			throw codec_fatal_exception(CodecExceptions::Png_InitError, "Bit depth set in the header do not match provided image.");
	}

	//----------------------------------------------------------------------
	// 1 - Opening the file pointed by file_path for writing

	//Opening file in C mode for binary writing
	FILE* file_handle;
	const char* c_file_path = reinterpret_cast<const char*>(file_path.generic_u8string().c_str());
	//Trying to open the file in reading binary mode
	errno_t fopen_error = fopen_s(&file_handle, c_file_path, "wb");
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

	//Allocating and initializing png writer structure
	//Note: If error_ptr is not set (NULL) setjump should be set up (see libpng manual).
	//	    setjump is omitted since we use ErrorExitHandler to catch fatal errors.
	//Note: If warn_fn is not set (NULL) warnings will be printed to standard output. 
	//      If callback provided by user in warning_callback_data is NULL 
	//      WarningHandler() will be called and will simply do nothing.
	png_structp png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,								// Library version
		static_cast<png_voidp>(&warning_callback_data),	    // error_ptr - Referencing warning callback and its arguments
															// in writer object (accessible by calling png_get_error_ptr(png_ptr))
		reinterpret_cast<png_error_ptr>(&ErrorExitHandler),	// error_fn - Fatal error handler function
		reinterpret_cast<png_error_ptr>(&WarningHandler)	// warn_fn - Warning handler function
	);
	if (png_ptr == NULL) { //Failed to allocate
		//Closing the file, throwing exception
		fclose(file_handle);
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Failed to initialize PNG encoder.");
	}

	//Allocating info structure for writing
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) { //Failed to allocate
		//Closing the file, destroying png object, throwing exception
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(file_handle);
		throw codec_fatal_exception(CodecExceptions::Png_InitError, "Failed to initialize PNG encoder.");
	}

	//Initializing writing mechanism
	try {
		png_init_io(png_ptr, file_handle);
	}
	catch (codec_fatal_exception e) {
		//Closing the file, destroying png objects, rethrowing exception
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(file_handle);
		throw;
	}

	//----------------------------------------------------------------------
	// 3 - Configuring compressor for given image

	//Providing encoder with basic info about the image and basic comression settings
	png_set_IHDR(
		png_ptr,	//PNG structure
		info_ptr,	//Info structure
		header.GetWidth(),		//Image width in pixels
		header.GetHeight(),		//Image height in pixels
		header_bit_depth,		//Per color bit depth
		header_layout,			//Data layout
		header_interlacing,		//Write file as interlaced or not
		PNG_COMPRESSION_TYPE_DEFAULT,	//Compression type - must be default
		PNG_FILTER_TYPE_DEFAULT			//Filter method - for non-embedded png stream must be default
	);

	//Writing the header
	try {
		png_write_info(png_ptr, info_ptr);
	}
	catch (codec_fatal_exception e) {
		//Closing the file, destroying png objects, rethrowing exception
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(file_handle);
		throw;
	}

	//----------------------------------------------------------------------
	// 4 - Setting the compressor to the ready state

	//If the image is 16 bit per channel each channel bytes stored in little-endian format (least signigicant byte first).
	//PNG format stores bytes in big-endian format (most significant byte first).
	//So, we have to swap byte order.
	if (header.GetBitDepth() == 16)
		png_set_swap(png_ptr);

	//If header is set to less than 8 bit and the layout is grayscale
	//we have to perform bit crush
	ImageBuffer_Byte* bitcrushed_image = nullptr;
	if (bitcrush) {
		bitcrushed_image = CrushBitDepth(image, header_bit_depth);
		//This setting tells the compressor that each byte holds bitcrushed value for one grayscale pixel
		png_set_packing(png_ptr);
	}

	//----------------------------------------------------------------------
	// 5 - Compressing and writing the image

	//Data alias for writing
	png_bytepp image_data;
	if (bitcrush)
		image_data = static_cast<png_bytepp>(bitcrushed_image->GetData());
	else
		image_data = static_cast<png_bytepp>(image->GetData());

	//Finally, we compress the image in one call
	try {
		png_write_image(png_ptr, image_data);	//Writing
	}
	catch (codec_fatal_exception e) {
		//Closing the file, destroying png object, rethrowing exception
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(file_handle);
		throw;
	}

	//----------------------------------------------------------------------
	// 6 - Cleaning

	//Finishing the writing
	png_write_end(png_ptr, info_ptr);

	//Deallocating the structures
	png_destroy_write_struct(&png_ptr, &info_ptr);

	//Closing the file
	fclose(file_handle);

	//Deallocating bitcrushed image
	if (bitcrush)
		delete bitcrushed_image;

	return;
}