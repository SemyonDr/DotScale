#pragma once
#include "jpeglib.h"

///<summary>
///Contains data that was read from JPEG file header.
///Data container.
///</summary>
class JpegHeaderInfo {

	///<summary>
	///Data fields can be set only by JpegReader/Writer object.
	///</summary>
	friend class JpegReader;
	friend class JpegWriter;

public:
	//--------------------------------
	//	PUBLIC GETTERS
	//--------------------------------

	///<summary>
	///Image height in pixels.
	///</summary>
	unsigned int GetHeight() { return _height; }

	///<summary>
	///Image width in pixels.
	///</summary>
	unsigned int GetWidth() { return _width; }

	///<summary>
	///Number of color components in the pixel.
	///</summary>
	int GetNumComponents() { return _num_components; }

	///<summary>
	///Image colorspace as defined by libjpeg.
	///</summary>
	J_COLOR_SPACE GetColorSpace() { return _color_space; }

	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	///<summary>
	///Default constructor.
	///</summary>
	JpegHeaderInfo() { }

	///<summary>
	///Initializing constructor.
	///</summary>
	JpegHeaderInfo(unsigned int height, unsigned int width, int num_components, J_COLOR_SPACE color_space) {
		_height = height;
		_width = width;
		_num_components = num_components;
		_color_space = color_space;
	}

	///<summary>
	///Copy constructor.
	///</summary>
	JpegHeaderInfo(const JpegHeaderInfo& copy_source) noexcept {
		_height = copy_source._height;
		_width = copy_source._width;
		_num_components = copy_source._num_components;
		_color_space = copy_source._color_space;
	}

	///<summary>
	///Copy assigment.
	///</summary>
	JpegHeaderInfo& operator=(const JpegHeaderInfo& copy_source) {
		_height = copy_source._height;
		_width = copy_source._width;
		_num_components = copy_source._num_components;
		_color_space = copy_source._color_space;
		return *this;
	}

	///<summary>
	///Move constructor.
	///</summary>
	JpegHeaderInfo(const JpegHeaderInfo&& move_source) noexcept {
		_height = move_source._height;
		_width = move_source._width;
		_num_components = move_source._num_components;
		_color_space = move_source._color_space;
	}

	///<summary>
	///Move assigment.
	///</summary>
	JpegHeaderInfo&& operator=(const JpegHeaderInfo&& move_source) {
		_height = move_source._height;
		_width = move_source._width;
		_num_components = move_source._num_components;
		_color_space = move_source._color_space;
	}



private:
	//--------------------------------
	//	HEADER DATA
	//--------------------------------

	///<summary>
	///Image height in pixels.
	///</summary>
	unsigned int _height = 0;

	///<summary>
	///Image width in pixels.
	///</summary>
	unsigned int _width = 0;

	///<summary>
	///Number of color components in the pixel.
	///</summary>
	int _num_components = 0;

	///<summary>
	///Image colorspace as defined by libjpeg.
	///</summary>
	J_COLOR_SPACE _color_space = J_COLOR_SPACE::JCS_UNKNOWN;
};