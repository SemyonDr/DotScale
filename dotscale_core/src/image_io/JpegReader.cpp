#include "JpegReader.h"

#include <cstdio>
#include <variant>
#include "../Exceptions.h"

namespace dotscale {

//--------------------------------
//	CHUNK READER
//--------------------------------

/// Reads next block of rows.
ImageBuffer_uintv JpegReader::read_next(uint32_t lines) {
	
	//----------------------------------------------------------------------
	// 1 - Arguments check

	// If reading has finished
	if (_state == ReaderState::Finished)
		throw codec_fatal_exception(CodecExceptionType::Jpeg_DecodingError, "Trying to read after the reader has finished.");

	// If we have enough rows
	if (_next_row + lines > _height)
		throw codec_fatal_exception(CodecExceptionType::Jpeg_DecodingError, "Trying to read past the last line.");

	//----------------------------------------------------------------------
	// 2 - Decompressing the image

	//This object will represent the result of file reading.
	ImageBuffer_uintv decompressed_image(
			std::in_place_type<ImageBuffer_uint8>,
			lines,
			_image_header.get_width(),
			_image_header.get_layout()
	);

	auto data_ptr = std::get<ImageBuffer_uint8>(decompressed_image).get_data_ptr();

	try {
		//Aqcuiring decompressed rows.
		//jpeg_read_scanlines requires array of rows as a buffer since it can read multiple rows at once.
		//We read one row at a time because ImageBufferByte data rows are not allocated continuously.
		for (int row = 0; row < lines; row++) {
			JSAMPROW row_ptr = data_ptr[row];
			jpeg_read_scanlines(
				&_jpeg_decomp,		// Jpeg image managing object.
				&row_ptr,			// 1-row buffer for reading current row (defined as 2D)
				1					// Number of lines to be read at once.
			);
		}
	}
	catch (codec_fatal_exception e) { // Will be thrown by error_exit callback if something goes wrong
		jpeg_destroy_decompress(&_jpeg_decomp);
		fclose(_c_file_handle);
		throw; //rethrowing
	}

	//----------------------------------------------------------------------
	// 3 - Finishing decompressing

	// Advancing the file position
	_next_row += lines;

	//If we have finished reading the file we close it, finalize the reading and deallocate the decompressor
	if (_next_row == _height) {
		jpeg_finish_decompress(&_jpeg_decomp);
		jpeg_destroy_decompress(&_jpeg_decomp);
		fclose(_c_file_handle);
		_state = ReaderState::Finished;
	}

	//Returning decompressed image object
	return decompressed_image;
}



//--------------------------------
//	INIT
//--------------------------------

/// Opens file and passes callbacks to the decompressor
void JpegReader::_init_decompressor() {
	
	// Opening the file pointed by file_path
	open_c_file_for_reading(_file_path);

	// Initializing decompressor data structures

	//Assigning error manager to decompressor
	_jpeg_decomp.err = jpeg_std_error(&_jerr_decomp);
	//Registering jpeg error handler
	_jerr_decomp.error_exit = &_error_exit_handler;
	//Registering warning handler function
	_jerr_decomp.emit_message = &_warning_handler;
	//Initializing decompressor object
	jpeg_create_decompress(&_jpeg_decomp);

	//Setting opened file as a source for decompressor
	jpeg_stdio_src(&_jpeg_decomp, _c_file_handle);

	//Referencing the warning callback in the decompressor object
	if (_warning_callback.has_value())
		_jpeg_decomp.client_data = &(_warning_callback.value());
}


/// Reads jpeg header.
JpegHeader JpegReader::_read_header() {
	//Reading JPEG header to get the image info.
	//If error is encountered ErrorExitHandler will be called.
	try {
		jpeg_read_header(&_jpeg_decomp, TRUE);
	}
	catch (codec_fatal_exception e) {
		fclose(_c_file_handle);
		jpeg_destroy_decompress(&_jpeg_decomp);
		throw;
	}

	//Jpeg header object

	// Extracting quantization tables
	JpegHeader::QuantTables quant_tables{};
	for (int i = 0; i < NUM_QUANT_TBLS; i++) {
		const auto* src_tbl = _jpeg_decomp.quant_tbl_ptrs[i];

		if (src_tbl != NULL) {
			JpegHeader::QuantTable table{};

			for (int j = 0; j < DCTSIZE2; j++)
				table[j] = src_tbl->quantval[j];
			quant_tables[i] = table;
		}
	}

	// Extracting subchroma sampling
	JpegHeader::ComponentSampling chroma_sampling{};
	for (int i = 0; i < _jpeg_decomp.num_components; i++) {
		chroma_sampling[i] = {
			_jpeg_decomp.comp_info[i].h_samp_factor,
			_jpeg_decomp.comp_info[i].v_samp_factor
		};
	}

	// Extracting JFIF Density
	std::optional<JpegHeader::JfifDensity> jfif_density;
	if (_jpeg_decomp.saw_JFIF_marker) {
		jfif_density = JpegHeader::JfifDensity{
			.unit = _jpeg_decomp.density_unit,
			.x_density = _jpeg_decomp.X_density,
			.y_density = _jpeg_decomp.Y_density
		};
	}

	return JpegHeader(
		_jpeg_decomp.image_height,
		_jpeg_decomp.image_width,
		_jpeg_decomp.jpeg_color_space,
		_jpeg_decomp.progressive_mode,
		quant_tables,
		chroma_sampling,
		jfif_density
	);
}



//--------------------------------
//	CONSTRUCTOR
//--------------------------------




/// Opens JPEG file, reads header and prepares reader for block reading.
JpegReader::JpegReader(std::filesystem::path file_path, std::optional<io_util::CodecWarningCallback> warning_callback)
	: ImageReader(file_path), 
	_warning_callback(warning_callback),
	_jpeg_header((
			_init_decompressor(),
			_read_header()
	)),
	_image_header(ImageHeader::from_jpeg(_jpeg_header)) {

	// Setting the decompressor to the ready state

	//Setting decompressor output colorspace
	//We decompress to grayscale if the file is explicitly grayscale.
	//We convert all other file color spaces to RGB when decompressing.
	_jpeg_decomp.out_color_space = 
		_jpeg_decomp.jpeg_color_space == J_COLOR_SPACE::JCS_GRAYSCALE ?
		J_COLOR_SPACE::JCS_GRAYSCALE : J_COLOR_SPACE::JCS_RGB;

	//Setting decompression method choosing slow and accurate
	_jpeg_decomp.dct_method = J_DCT_METHOD::JDCT_ISLOW;

	//Setting decompressor to ready state
	//Image info will be available after this call
	jpeg_start_decompress(&_jpeg_decomp);

	//Now file is opened and decompressor is ready to decompress the image lines
	_state = ReaderState::Ready;
}



//--------------------------------
//	DESTRUCTOR
//--------------------------------

JpegReader::~JpegReader() {
	// If we have not finishied yet
	if (_state == ReaderState::Ready) {
		jpeg_destroy_decompress(&_jpeg_decomp);
		fclose(_c_file_handle);
		_state = ReaderState::Finished;
	}
}



//--------------------------------
//	ERROR/WARNING HANDLERS
//--------------------------------

/// Called when libjpeg routines encounter non-fatal error, or corrupt data and are able to continue.
/// Redirects to user-supplied callback.
void JpegReader::_warning_handler(j_common_ptr jpeg_object, int msg_level) {
	// No-warning setting
	if (msg_level != -1)
		return;

	// Extracting callback data from jpeg object
	auto* warning_callback = static_cast<io_util::CodecWarningCallback*>(jpeg_object->client_data);

	// Checking if the callback was set
	if (warning_callback == NULL || warning_callback->func == nullptr)
		return;

	// Formatting libjpeg warning message
	char msg[JMSG_LENGTH_MAX];
	jpeg_object->err->format_message(jpeg_object, msg);

	// Passing to user-defined function
	warning_callback->func(std::string(msg), warning_callback->arg);
}


///Callback used when libjpeg decoding routines encounter fatal error and are terminated.
///Throws an exception containing libjpeg error message.
void JpegReader::_error_exit_handler(j_common_ptr jpeg_object) {
	// Formatting libjpeg warning message
	char msg[JMSG_LENGTH_MAX];
	jpeg_object->err->format_message(jpeg_object, msg);
	//Throwing
	throw codec_fatal_exception(CodecExceptionType::Jpeg_DecodingError, msg);
}



} // namespace dotscale