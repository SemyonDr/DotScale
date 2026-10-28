#pragma once

#include <filesystem>
#include <optional>
#include <variant>

#include "jpeglib.h"
#include "jerror.h"

#include "../image/ImageBuffer.h"
#include "../image_io/ImageIO.h"
#include "../image_io/headers/JpegHeader.h"
#include "../image_io/headers/ImageHeader.h"
#include "../image_io/ImageWriter.h"

#include "../Exceptions.h"

namespace dotscale {



/// Writes JPEG images to disk.
/// Uses libjpeg-turbo library for encoding.
class JpegWriter : public ImageWriter
{
public:

	//--------------------------------
	//	WRITER
	//--------------------------------

	/// Writes next block of image lines.
	void write_next(const ImageBuffer_uintv& block) override;


	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------

	/// Uses provided quality setting.
	JpegWriter(
		std::filesystem::path file_path, 
		const JpegHeader& header, 
		int quality, 
		std::optional<io_image::CodecWarningCallback> warning_callback = std::nullopt);


	/// Expects quantization tables in the header.
	JpegWriter(
		std::filesystem::path file_path,
		const JpegHeader& header,
		std::optional<io_image::CodecWarningCallback> warning_callback = std::nullopt);


	//--------------------------------
	//	DEFAULT DESTRUCTOR
	//--------------------------------

	~JpegWriter() {
		jpeg_destroy_compress(&_jpeg_comp);
	}


private:

	//--------------------------------
	//	PRIVATE CONSTRUCTOR
	//--------------------------------

	JpegWriter(
		std::filesystem::path file_path,
		const JpegHeader& header,
		std::optional<int> quality,
		std::optional<io_image::CodecWarningCallback> warning_callback = std::nullopt);


	//--------------------------------
	//	INIT
	//--------------------------------

	/// Opens file and passes callbacks to the decompressor
	void _init_compressor();

	/// Applies jpeg header parameters to the compressor.
	void _configure_compressor();

	/// Sends quantization tables from the header to the compressor
	void _send_quantization();

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	JpegHeader _jpeg_header;
	ImageHeader _image_header;
	std::optional<io_image::CodecWarningCallback> _warning_callback;
	std::optional<int> _quality;

	//--------------------------------
	//	LIBJPEG DATA STRUCTURES
	//--------------------------------

	///LibJpeg compressor object.
	struct jpeg_compress_struct _jpeg_comp;

	///Error manager object for compressor.
	struct jpeg_error_mgr _jerr_comp;

};

} // namespace dotscale