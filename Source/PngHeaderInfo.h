#pragma once

#include "png.h"


///<summary>
///Data object for storing PNG image file header.
///</summary>
class PngHeaderInfo {
public:
	//--------------------------------
	//	GETTERS
	//--------------------------------

	unsigned int GetHeight() const { return _height; }
	unsigned int GetWidth() const { return _width; }
	unsigned char GetBitDepth() const { return _bit_depth; }
	unsigned char GetPngColorType() const { return _png_color_type; }
	unsigned char GetPngInterlaceType() const { return _png_interlace_type; }

	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	/// <summary>
	/// Default constructor.
	/// </summary>
	PngHeaderInfo() {
	
	
	}

	///<summary>
	///Initializing constructor.
	///</summary>
	PngHeaderInfo(unsigned int height, unsigned int width, unsigned char bit_depth, int png_color_type, unsigned char png_interlace_type) {
		_height = height;
		_width = width;
		_bit_depth = bit_depth;
		_png_color_type = png_color_type;
		_png_interlace_type = png_interlace_type;
	}

	//--------------------------------
	//	COPY/MOVE
	//--------------------------------

	///<summary>
	///Copy constructor.
	///</summary>
	PngHeaderInfo(const PngHeaderInfo& copy_source) {
		_height = copy_source._height;
		_width = copy_source._width;
		_bit_depth = copy_source._bit_depth;
		_png_color_type = copy_source._png_color_type;
		_png_interlace_type = copy_source._png_interlace_type;
	}

	/// <summary>
	/// Copy assigment
	/// </summary>
	PngHeaderInfo& operator=(const PngHeaderInfo& other) {
		if (this == &other)
			return *this;

		_height = other._height;
		_width = other._width;
		_bit_depth = other._bit_depth;
		_png_color_type = other._png_color_type;
		_png_interlace_type = other._png_interlace_type;

		return *this;
	}

	///<summary>
	///Move constructor.
	///</summary>
	PngHeaderInfo(PngHeaderInfo&& other) noexcept {
		_height = other._height;
		_width = other._width;
		_bit_depth = other._bit_depth;
		_png_color_type = other._png_color_type;
		_png_interlace_type = other._png_interlace_type;
	}

	/// <summary>
	/// Move assigment.
	/// </summary>
	PngHeaderInfo& operator=(PngHeaderInfo&& other) noexcept{
		if (&other == this)
			return *this;

		_height = other._height;
		_width = other._width;
		_bit_depth = other._bit_depth;
		_png_color_type = other._png_color_type;
		_png_interlace_type = other._png_interlace_type;

		return *this;
	}

private:
	friend class PngReader;

	//--------------------------------
	//	HEADER DATA
	//--------------------------------

	unsigned int _height = 0;
	unsigned int _width = 0;
	unsigned char _bit_depth = 8;
	unsigned char _png_color_type = PNG_COLOR_TYPE_RGB;
	unsigned char _png_interlace_type = PNG_INTERLACE_NONE;
};