#pragma once

#include "../../image_io/ImageIO.h"
#include "../../image/ImageEnums.h"
#include "../../image_io/headers/JpegHeader.h"
#include "../../image_io/headers/PngHeader.h"


namespace dotscale {

/// Basic metadata for a decompressed image.
class ImageHeader {

public:

	//--------------------------------
	//	PUBLIC ACCESSORS
	//--------------------------------

	uint32_t get_height() const noexcept { return _height; }

	uint32_t get_width() const noexcept { return _width; }

	PixelLayout get_layout() const noexcept { return _layout; }

	BitDepth get_bit_depth() const noexcept { return _bit_depth; }

	int get_num_cmp() const {
		return util::pixel_cmp_count(_layout);
	}

	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------

	ImageHeader(uint32_t height, uint32_t width, PixelLayout layout, BitDepth bitDepth) 
		: _width(width), _height(height), _layout(layout), _bit_depth(bitDepth) {	}


	/// Converts from jpeg header.
	ImageHeader static from_jpeg(const JpegHeader& jpeg_header)  {
		return ImageHeader(
			jpeg_header.get_height(),
			jpeg_header.get_width(),
			io_jpeg::jpeg_colorspace_to_storage(jpeg_header.get_colorspace()),
			BitDepth::BD_8_BIT
		);
	}


private:

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	uint32_t _height;
	uint32_t _width;
	PixelLayout _layout;
	BitDepth _bit_depth;
};

} // namespace dotscale