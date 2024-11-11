#pragma once
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

	int GetHeight() { return _height; }
	int GetWidth() { return _width; }
	ImagePixelLayout GetLayout() { return _layout; }
	BitDepth GetBitDepth() { return _bit_depth; }


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
		_height = height;
		_width = width;
		_layout = layout;
		_bit_depth = bitDepth;
	}

	///<summary>
	///Copy constructor.
	///</summary>
	ImageBufferInfo(ImageBufferInfo& source) {
		_height = source._height;
		_width = source._width;
		_layout = source._layout;
		_bit_depth = source._bit_depth;
	}

	///<summary>
	///Move constructor.
	///</summary>
	ImageBufferInfo(ImageBufferInfo&& source) noexcept {
		_height = source._height;
		_width = source._width;
		_layout = source._layout;
		_bit_depth = source._bit_depth;
	}

private:

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	int _height = 0;
	int _width = 0;
	BitDepth _bit_depth = BitDepth::BD_8_BIT;
	ImagePixelLayout _layout = ImagePixelLayout::UNDEF;

};