#pragma once
#include "JpegHeaderInfo.h"
#include "ImageEnums.h"


///<summary>
///This class represents basic image metadata common
///between different image file headers.
///</summary>
class ImageBufferInfo {
	//Data can be set by ImageReader classes.
	friend class ImageReader;
	friend class JpegReader;
	friend class PngReader;
	//Data can be set by ImageWriter classes.
	friend class ImageWriter;
	friend class JpegWriter;
	friend class PngWriter;

public:

	//--------------------------------
	//	PUBLIC ACCESSORS
	//--------------------------------

	int GetHeight() const { return _height; }
	int GetWidth() const { return _width; }
	ImagePixelLayout GetLayout() const { return _layout; }
	BitDepth GetBitDepth() const { return _bit_depth; }
	int GetNumComponents() const {
		switch (_layout)
		{
			case UNDEF:
				return 0;
			case G:
				return 1;
			case GA:
				return 2;
			case RGB:
				return 3;
			case RGBA:
				return 4;
			default:
				return 0;
		}
	}

	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------

	///<summary>
	///Default constructor;
	///</summary>
	ImageBufferInfo() {
	}

	///<summary>
	///Constructs the header object.
	///</summary>
	ImageBufferInfo(int height, int width, ImagePixelLayout layout, BitDepth bitDepth) {
		this->_height = height;
		this->_width = width;
		this->_layout = layout;
		this->_bit_depth = bitDepth;

		switch (layout)
		{
			case G:
				_num_components = 1;
				break;
			case GA:
				_num_components = 2;
				break;
			case RGB:
				_num_components = 3;
				break;
			case RGBA:
				_num_components = 4;
				break;
		}
	}

	/// <summary>
	/// Constructs the header object according to given jpeg header.
	/// </summary>
	ImageBufferInfo(const JpegHeaderInfo& jpeg_header) {
		_bit_depth = BitDepth::BD_8_BIT;
		_height = static_cast<int>(jpeg_header.GetHeight());
		_width = static_cast<int>(jpeg_header.GetWidth());
	
		switch (jpeg_header.GetColorSpace())
		{
			case J_COLOR_SPACE::JCS_GRAYSCALE:
				_layout = ImagePixelLayout::G;
				break;

			case J_COLOR_SPACE::JCS_RGB:
				_layout = ImagePixelLayout::RGB;
				break;

			default:
				_layout = ImagePixelLayout::RGB;
				break;
		}

		switch (_layout)
		{
			case G:
				_num_components = 1;
				break;
			case GA:
				_num_components = 2;
				break;
			case RGB:
				_num_components = 3;
				break;
			case RGBA:
				_num_components = 4;
				break;
		}
	}


	//--------------------------------
	//	COPY/MOVE
	//--------------------------------


	///<summary>
	///Copy constructor.
	///</summary>
	ImageBufferInfo(const ImageBufferInfo& other) {
		this->_height = other._height;
		this->_width = other._width;
		this->_layout = other._layout;
		this->_bit_depth = other._bit_depth;
	}

	/// <summary>
	/// Copy assigment
	/// </summary>
	ImageBufferInfo& operator=(const ImageBufferInfo& other) {
		if (&other == this)
			return *this;

		this->_height = other._height;
		this->_width = other._width;
		this->_layout = other._layout;
		this->_bit_depth = other._bit_depth;

		return *this;
	}


	///<summary>
	///Move constructor.
	///</summary>
	ImageBufferInfo(ImageBufferInfo&& other) noexcept {
		this->_height = other._height;
		this->_width = other._width;
		this->_layout = other._layout;
		this->_bit_depth = other._bit_depth;
	}

	/// <summary>
	/// Move assigment.
	/// </summary>
	ImageBufferInfo& operator=(ImageBufferInfo&& other) noexcept {
		if (&other == this)
			return *this;

		this->_height = other._height;
		this->_width = other._width;
		this->_layout = other._layout;
		this->_bit_depth = other._bit_depth;

		return *this;
	}


private:

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	int _height = 0;
	int _width = 0;
	int _num_components = 0;
	BitDepth _bit_depth = BitDepth::BD_8_BIT;
	ImagePixelLayout _layout = ImagePixelLayout::UNDEF;

};