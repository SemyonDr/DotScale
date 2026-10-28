#pragma once

#include <filesystem>
#include <optional>

#include "jpeglib.h"
#include "jerror.h"

#include "../image_io/ImageIO.h"
#include "../image_io/headers/ImageHeader.h"
#include "../image_io/headers/JpegHeader.h"
#include "../image/ImageBuffer.h"
#include "../image_io/ImageReader.h"

namespace dotscale {


/// Class for reading JPEG images from disk.
/// Uses libjpeg-turbo library for decoding.
/// Opens the file when constructing the reader and keeps the file opened 
/// until reading is finished, or reader is destroyed.
/// If you want to receive warnings supply own callback function.
class JpegReader : public ImageReader
{

public:
	//--------------------------------
	//	ACCESSORS
	//--------------------------------

	const JpegHeader& get_jpeg_header() const noexcept { return _jpeg_header;  }

	const ImageHeader& get_image_header() const noexcept override { return _image_header; }


	//--------------------------------
	//	READER
	//--------------------------------

	/// Reads next block of rows.
	ImageBuffer_uintv read_next(uint32_t lines) override;

	//--------------------------------
	//	CONSTRUCTOR
	//--------------------------------

	JpegReader(
		std::filesystem::path file_path, 
		std::optional<io_image::CodecWarningCallback> warning_callback = std::nullopt);


	//--------------------------------
	//	DESTRUCTOR
	//--------------------------------

	~JpegReader();

private:

	//--------------------------------
	//	INIT
	//--------------------------------

	/// Opens file and passes callbacks to the decompressor.
	void _init_decompressor();

	/// Reads jpeg header.
	JpegHeader _read_header();


	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	/* Do not change the order of header fields
	   or this will break the initialization.
	   This order determines order of execution of init list
	   of the constructor.
	   common header depends on jpeg header
	   jpeg depends on init()
	   init() depends on warning callback
	*/
	
	std::optional<io_image::CodecWarningCallback> _warning_callback;
	JpegHeader _jpeg_header;
	ImageHeader _image_header;

	//--------------------------------
	//	LIBJPEG DATA STRUCTURES
	//--------------------------------
	
	///LibJpeg decompressor object.
	struct jpeg_decompress_struct _jpeg_decomp;

	///Error manager object for decompressor.
	struct jpeg_error_mgr _jerr_decomp;

};

} // namespace dotscale