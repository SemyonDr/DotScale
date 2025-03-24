#pragma once
#include "jpeglib.h"

///<summary>
///Contains data that was read from JPEG file header.
///Data container.
///</summary>
class JpegHeaderInfo {

	///<summary>
	///Data fields can be set directly by JpegReader/Writer object.
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
	inline unsigned int GetHeight() const { return _height; }

	///<summary>
	///Image width in pixels.
	///</summary>
	inline unsigned int GetWidth() const { return _width; }

	///<summary>
	///Number of color components in the pixel.
	///</summary>
	inline int GetNumComponents() const { return _num_components; }

	///<summary>
	///Image colorspace as defined by libjpeg.
	///</summary>
	inline J_COLOR_SPACE GetColorSpace() const { return _color_space; }

	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	/// <summary>
	/// Default constructor.
	/// </summary>
	JpegHeaderInfo() { }


	///<summary>
	///Initializing constructor.
	///</summary>
	JpegHeaderInfo(unsigned int height, unsigned int width, int num_components, J_COLOR_SPACE color_space) {
		if (num_components != 1 &&
			num_components != 3 &&
			num_components != 4)
			throw new std::invalid_argument("Setting unsupported number of Jpeg Components in JpegHeader object.");

		if (color_space != J_COLOR_SPACE::JCS_GRAYSCALE &&
			color_space != J_COLOR_SPACE::JCS_RGB &&
			color_space != J_COLOR_SPACE::JCS_YCCK &&
			color_space != J_COLOR_SPACE::JCS_CMYK &&
			color_space != J_COLOR_SPACE::JCS_YCbCr)
			throw new std::invalid_argument("Setting unsupported Color Space in JpegHeader object.");

		_height = height;
		_width = width;
		_num_components = num_components;
		_color_space = color_space;
	}

	//--------------------------------
	//	COPY/MOVE
	//--------------------------------

	/// <summary>
	/// Copy constructor.
	/// </summary>
	/// <param name="other"></param>
	JpegHeaderInfo(const JpegHeaderInfo& other) :
		_height(other._height), 
		_width(other._width), 
		_num_components(other._num_components), 
		_color_space(other._color_space) {
	}

	/// <summary>
	/// Copy assigment.
	/// </summary>
	JpegHeaderInfo& operator=(const JpegHeaderInfo& other) {
		if (&other == this)
			return *this;

		this->_height = other._height;
		this->_width = other._width;
		this->_num_components = other._num_components;
		this->_color_space = other._color_space;

		return *this;
	}

	/// <summary>
	/// Move constructor.
	/// </summary>
	JpegHeaderInfo(JpegHeaderInfo&& other) :
		_height(other._height),
		_width(other._width),
		_num_components(other._num_components),
		_color_space(other._color_space) {
	}


	/// <summary>
	/// Move assigment.
	/// </summary>
	JpegHeaderInfo& operator=(JpegHeaderInfo&& other) {
		if (&other == this)
			return *this;
		this->_height = other._height;
		this->_width = other._width;
		this->_num_components = other._num_components;
		this->_color_space = other._color_space;
		return *this;
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