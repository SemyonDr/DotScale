#pragma once

#include "png.h"


///<summary>
///Data object for storing PNG image file header copy_source.
///</summary>
class PngHeaderInfo {
public:
	//--------------------------------
	//	GETTERS
	//--------------------------------

	unsigned int GetHeight() { return _height; }
	unsigned int GetWidth() { return _width; }
	unsigned char GetBitDepth() { return _bit_depth; }
	unsigned char GetPngColorType() { return _png_color_type; }
	unsigned char GetPngInterlaceType() { return _png_interlace_type; }

	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	///<summary>
	///Default constructor.
	///</summary>
	PngHeaderInfo() { }

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

	///<summary>
	///Copy constructor.
	///</summary>
	PngHeaderInfo(PngHeaderInfo& copy_source) {
		_height = copy_source._height;
		_width = copy_source._width;
		_bit_depth = copy_source._bit_depth;
		_png_color_type = copy_source._png_color_type;
		_png_interlace_type = copy_source._png_interlace_type;
	}

	/// <summary>
	/// Copy assigment
	/// </summary>
	PngHeaderInfo& operator=(const PngHeaderInfo& copy_source) {
		if (this != &copy_source) {
			_height = copy_source._height;
			_width = copy_source._width;
			_bit_depth = copy_source._bit_depth;
			_png_color_type = copy_source._png_color_type;
			_png_interlace_type = copy_source._png_interlace_type;
		}
		return *this;
	}

	///<summary>
	///Move constructor.
	///</summary>
	PngHeaderInfo(PngHeaderInfo&& move_source) noexcept {
		_height = move_source._height;
		_width = move_source._width;
		_bit_depth = move_source._bit_depth;
		_png_color_type = move_source._png_color_type;
		_png_interlace_type = move_source._png_interlace_type;
	}

	/// <summary>
	/// Move assigment.
	/// </summary>
	/// <param name="move_source"></param>
	/// <returns></returns>
	PngHeaderInfo&& operator=(const PngHeaderInfo&& move_source) {
		_height = move_source._height;
		_width = move_source._width;
		_bit_depth = move_source._bit_depth;
		_png_color_type = move_source._png_color_type;
		_png_interlace_type = move_source._png_interlace_type;
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