#pragma once
#include <exception>

#include "ImageEnums.h"

/// <summary>
/// Provides basic parameters of a buffer for storing an image.
/// </summary>
class ImageBuffer_Base {
	friend class ImageBuffer_Byte;

public:
	//--------------------------------
	//	GET/SET
	//--------------------------------

	///<summary>
	///Layout of data in each pixel of the image.
	///</summary>
	ImagePixelLayout GetLayout() { return _layout; }

	///<summary>
	///Height of an image in pixels.
	///</summary>
	int GetHeight() { return _height; }

	///<summary>
	///Width of an image in pixels.
	///</summary>
	int GetWidth() { return _width; }

	///<summary>
	///Number of components in the pixel.
	///</summary>
	int GetNumCmp() { return _numCmp; }

	/// <summary>
	/// Number of individual pixel components in a full row.
	/// </summary>
	int GetCmpWidth() { return _numCmp * _width; }

	///<summary>
	///If this image layout has alpha channel.
	///</summary>
	bool GetHasAlpha() { return _hasAlpha; }

	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	/// <summary>
	///Default constructor is disabled.
	/// </summary>
	ImageBuffer_Base() = delete;

	///<summary>
	///Creates image description with given dimensions and layout.
	///Rows are not allocated.
	///</summary>
	ImageBuffer_Base(int height, int width, ImagePixelLayout layout) : _height(height), _width(width), _layout(layout) {

		switch (_layout) {
		case UNDEF:
			throw std::exception("Cannot create RawImage with undefined layout.");
			return;
		case G:
			_numCmp = 1;
			_hasAlpha = false;
			break;
		case GA:
			_numCmp = 2;
			_hasAlpha = true;
			break;
		case RGB:
			_numCmp = 3;
			_hasAlpha = false;
			break;
		case RGBA:
			_numCmp = 4;
			_hasAlpha = true;
			break;
		default:
			break;
		}
	}

protected:
	//--------------------------------
	//	FIELDS
	//--------------------------------

	///<summary>
	///Layout of data in each pixel of the image.
	///</summary>
	ImagePixelLayout _layout = UNDEF;

	///<summary>
	///Height of an image in pixels.
	///</summary>
	int _height = 0;

	///<summary>
	///Width of an image in pixels.
	///</summary>
	int _width = 0;

	///<summary>
	///Number of conponents in the pixel.
	///</summary>
	int _numCmp = 0;

	///<summary>
	///If this image layout has alpha channel.
	///</summary>
	bool _hasAlpha = false;

};
