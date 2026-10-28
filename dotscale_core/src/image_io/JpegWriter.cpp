#include "JpegWriter.h"

namespace dotscale {




//--------------------------------
//	WRITER
//--------------------------------

/// Writes next block of image lines.
void JpegWriter::write_next(const ImageBuffer_uintv& block) {


	//----------------------------------------------------------------------
	// 1 - Arguments and state check

	if (_state == WriterState::Failed)
		throw codec_fatal_exception(CodecExceptionType::Jpeg_DecodingError, "Trying to write after the writer has failed.");

	if (_state == WriterState::Finished)
		throw codec_fatal_exception(CodecExceptionType::Jpeg_DecodingError, "Trying to write after the writer has finished.");

	if (!std::holds_alternative<ImageBuffer_uint8>(block))
		throw codec_fatal_exception(CodecExceptionType::Jpeg_DecodingError, "Provided bit depth is not supported by the jpeg writer.");

	auto& image = std::get<ImageBuffer_uint8>(block);

	if (image.get_layout() != _image_header.get_layout())
		throw codec_fatal_exception(
			CodecExceptionType::Jpeg_EncodingError, 
			"Expected layout mismatch. Expected " + io_image::layout_name(_image_header.get_layout()) + 
			" but received " + io_image::layout_name(image.get_layout()) + ".");

	if (image.get_width() != _image_header.get_width())
		throw codec_fatal_exception(
			CodecExceptionType::Jpeg_EncodingError, 
			"Expected width mismatch. Expected " + std::to_string(_image_header.get_width()) +
			" but received " + std::to_string(image.get_width()) + ".");

	if (rows_left() < image.get_height())
		throw codec_fatal_exception(
			CodecExceptionType::Jpeg_EncodingError,
			"Attempting to write lines after the declared image height.");


	//----------------------------------------------------------------------
	// 2 - Compressing and writing the image rows
	
	try {
		// Image data alias
		auto image_data = image.get_data_ptr();		
		// Compressor excects array of rows, but we give it single-row arrays
		for (int row = 0; row < image.get_height(); row++) {
			JSAMPROW row_ptr = image_data[row];
			jpeg_write_scanlines(&_jpeg_comp, &row_ptr, 1);
		}
	}
	catch (codec_fatal_exception e) {
		jpeg_destroy_compress(&_jpeg_comp);
		std::fclose(_c_file_handle);
		_state = WriterState::Failed;
		throw;
	}

	//----------------------------------------------------------------------
	// 3 - Finishing compressing

	_next_row += image.get_height();

	//If we have finished writing the file we close it and deallocate the compressor
	if (_next_row == _height) {
		jpeg_finish_compress(&_jpeg_comp);
		jpeg_destroy_compress(&_jpeg_comp);;
		fclose(_c_file_handle);
		_state = WriterState::Finished;
	}

}


//--------------------------------
//	INIT
//--------------------------------

/// Opens file and passes callbacks to the decompressor
void JpegWriter::_init_compressor() {

	open_c_file_for_writing(_file_path);

	//Assigning error manager to compressor
	_jpeg_comp.err = jpeg_std_error(&_jerr_comp);
	//Registering jpeg error handler
	_jerr_comp.error_exit = &io_jpeg::libjpeg_error_exit_handler;
	//Registering warning handler function
	_jerr_comp.emit_message = &io_jpeg::libjpeg_warning_handler;
	//Initializing compressor object
	jpeg_create_compress(&_jpeg_comp);

	//Assigning opened file as the compression destination
	jpeg_stdio_dest(&_jpeg_comp, _c_file_handle);

	//Referencing warning callback in the compressor object
	if (_warning_callback.has_value())
		_jpeg_comp.client_data = &_warning_callback;
}



/// Applies jpeg header parameters to the compressor.
void JpegWriter::_configure_compressor() {
	// Dimensions and input colorspace needs to be set before applying defaults

	// Passing image dimensions
	_jpeg_comp.image_height = _jpeg_header.get_height();
	_jpeg_comp.image_width = _jpeg_header.get_width();

	// Passing image buffer colorspace and number of color components
	J_COLOR_SPACE input_colorspace = io_jpeg::jpeg_colorspace_storage_alias(_image_header.get_layout());
	_jpeg_comp.in_color_space = input_colorspace;
	_jpeg_comp.input_components = io_jpeg::jpeg_colorspace_num_cmp(input_colorspace);

	// Applying default settings
	jpeg_set_defaults(&_jpeg_comp);

	// Setting the file colorspace according to the JpegHeader object
	jpeg_set_colorspace(&_jpeg_comp, _jpeg_header.get_colorspace());

	// Explicitly saying to use slow, but accurate compression method (default)
	_jpeg_comp.dct_method = JDCT_ISLOW;

	// Setting compression quality by value or provided quantization tables
	if (_quality.has_value())
		jpeg_set_quality(&_jpeg_comp, _quality.value(), TRUE);
	else
		_send_quantization();

	// Setting sampling factors
	if (_jpeg_header.has_sampling()) {
		auto sampling = _jpeg_header.get_cmp_sampling().value();
		for (int cmp = 0; cmp < _jpeg_header.get_num_cmp(); cmp++) {
			_jpeg_comp.comp_info[cmp].h_samp_factor = sampling[cmp].horizontal;
			_jpeg_comp.comp_info[cmp].v_samp_factor = sampling[cmp].vertical;
		}
	}

	// Setting JFIF scaling
	if (_jpeg_header.has_jfif_density()) {
		auto density = _jpeg_header.get_jfif_density().value();
		_jpeg_comp.density_unit = density.unit;
		_jpeg_comp.X_density = density.x_density;
		_jpeg_comp.Y_density = density.y_density;
	}


}


/// Sends quantization tables from the header to the compressor
void JpegWriter::_send_quantization() {
	auto quantization = _jpeg_header.get_quantization().value();

	for (int cmp = 0; cmp < _jpeg_header.get_num_cmp(); cmp++) {
		int table_index = quantization.map[cmp];
		_jpeg_comp.comp_info[cmp].quant_tbl_no = table_index;
		JQUANT_TBL* trg_table = _jpeg_comp.quant_tbl_ptrs[table_index];

		// We create table if it was not created by default compressor init
		if (trg_table == NULL) {
			trg_table = jpeg_alloc_quant_table(
				reinterpret_cast<j_common_ptr>(&_jpeg_comp));
			_jpeg_comp.quant_tbl_ptrs[table_index] = trg_table;
		}

		for (int i = 0; i < DCTSIZE2; i++)
			trg_table->quantval[i] = quantization.tables[table_index][i];
		// Makes compressor use table
		trg_table->sent_table = false;
	}
}



//--------------------------------
//	CONSTRUCTORS
//--------------------------------

/// Uses provided quality setting.
JpegWriter::JpegWriter(
	std::filesystem::path file_path,
	const JpegHeader& header,
	int quality,
	std::optional<io_image::CodecWarningCallback> warning_callback)
	: JpegWriter(file_path, header, std::optional<int>{quality}, warning_callback) {}


/// Expects quantization tables in the header.
JpegWriter::JpegWriter(
	std::filesystem::path file_path,
	const JpegHeader& header,
	std::optional<io_image::CodecWarningCallback> warning_callback)
		: JpegWriter(file_path, header, std::nullopt, warning_callback) { }


/// Opens file pointed by file_path and writes the header.
JpegWriter::JpegWriter(
	std::filesystem::path file_path, 
	const JpegHeader& header,
	std::optional<int> quality,
	std::optional<io_image::CodecWarningCallback> warning_callback) 
		: ImageWriter(file_path),
		_warning_callback(warning_callback),
		_jpeg_header(header),
		_image_header(ImageHeader::from_jpeg(header)),
		_quality(quality)
{
	if (!io_jpeg::jpeg_colorspace_is_out(header.get_colorspace()))
		throw codec_fatal_exception(
			CodecExceptionType::Jpeg_InitError, 
			"Writing JPEG with output colorspace \"" + io_jpeg::jpeg_colorspace_name(header.get_colorspace()) + "\" is not supported.");

	if (!_quality.has_value()  && !header.has_quantization())
		throw codec_fatal_exception(
			CodecExceptionType::Jpeg_InitError,
			"Expected quantization tables are missing.");

	// Creating compressor object
	_init_compressor();

	// Configuring compressor for given image
	_configure_compressor();

	try {
		//Initializing compression procedure and writing the header
		jpeg_start_compress(&_jpeg_comp, TRUE);
	} catch (codec_fatal_exception e) {
		fclose(_c_file_handle);
		jpeg_destroy_compress(&_jpeg_comp);
		_state = WriterState::Failed;
		throw;
	}
	
	//File is opened, compressor is ready and the header was written
	_state = WriterState::Ready;
}







} // namespace dotscale