#pragma once

#include "ImageBuffer.h"

/// <summary>
/// Composite image buffer object for storing images as uint arrays.
/// Image may be 8, 16 or 32 bits per component.
/// </summary>
class ImageBuffer_Byte {
public:
	//--------------------------------
	//	GET/SET
	//--------------------------------

	///<summary>
	///Image data.
	///Array of rows of pixel samples that ordered according to image layout.
	///Do not deallocate.
	///</summary>
	uint8_t** GetData();

	///<summary>
	///Layout of data in each pixel of the image.
	///</summary>
	ImagePixelLayout GetLayout() { return _image_base->_layout; }

	/// <summary>
	///Bit depth of the image.
	/// </summary>
	BitDepth GetBitPerComponent() { return _bitDepth; }

	///<summary>
	///Height of an image in pixels.
	///</summary>
	int GetHeight() { return _image_base->_height; }

	///<summary>
	///Width of an image in pixels.
	///</summary>
	int GetWidth() { return _image_base->_width; }

	///<summary>
	///Number of components in the pixel.
	///</summary>
	int GetNumCmp() { return _image_base->_numCmp; }

	/// <summary>
	/// Number of individual pixel components in a full row.
	/// </summary>
	int GetCmpWidth() { return _image_base->_numCmp*_image_base->_width; }

	///<summary>
	///If this image layout has alpha channel.
	///</summary>
	bool GetHasAlpha() { return _image_base->_hasAlpha; }

	/// <summary>
	/// Tells if data for this buffer is allocated.
	/// </summary>
	/// <returns></returns>
	bool IsAllocated();

	/// <summary>
	/// If data array was allocated outside of control of this object
	/// appropriate flag should be set.
	/// </summary>
	void SetAllocated();

	/// <summary>
	/// If data array was deallocated outside of control of this object
	/// appropriate flag should be set.
	/// </summary>
	void SetDeallocated();


	//--------------------------------
	//	METHODS
	//--------------------------------

	/// <summary>
	/// If data is deallocated creates new data array.
	/// </summary>
	void AllocateData();

	/// <summary>
	/// Returns a copy of this image with alpha channel removed.
	/// </summary>
	ImageBuffer_Byte* RemoveAlphaChannel();

	/// <summary>
	/// Converts 8 bit image to 16 bit image with the same layout.
	/// </summary>
	ImageBuffer_Byte* ChangeBitDepth(BitDepth bitDepth);

	/// <summary>
	/// Transforms the layout and size of the image
	/// to match provided size and layout.
	/// Size is transformed by trimming/expanding with black. No pixel manipulation occurs.
	/// </summary>
	/// <param name="height">New height.</param>
	/// <param name="width">New width.</param>
	/// <param name="layout">New layout.</param>
	/// <returns>Transformed image buffer.</returns>
	ImageBuffer_Byte* TransformBuffer(int height, int width, ImagePixelLayout layout);

	
	/// <summary>
	/// Inserts given image into this image starting at a specified line.
	/// If widths mismatch columns will be trimmed/expanded with black fill.
	/// </summary>
	/// <param name="image">Image to insert</param>
	/// <param name="line">Line in this image where first line of inserted image should be placed.</param>
	void InsertAtLine(ImageBuffer_Byte* image, int line);

	/// <summary>
	/// Inserts given image into this image starting at the first line.
	/// If widths mismatch columns will be trimmed/expanded with black fill.
	/// </summary>
	/// <param name="image">Image to prepend.</param>
	void Prepend(ImageBuffer_Byte* image) {
		InsertAtLine(image, 0);
	}

	/// <summary>
	/// Inserts given image into this image starting right after the last line.
	/// If widths mismatch columns will be trimmed/expanded with black fill.
	/// </summary>
	/// <param name="image">Image to append.</param>
	void Append(ImageBuffer_Byte* image) {
		InsertAtLine(image, GetHeight());
	}
	


	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	/// <summary>
	///Default constructor is disabled.
	/// </summary>
	ImageBuffer_Byte() = delete;

	///<summary>
	///Creates empty image with given dimensions and layout.
	///Each row is allocated individually.
	///</summary>
	ImageBuffer_Byte(int height, int width, ImagePixelLayout layout, BitDepth bit_depth);

	/// <summary>
	/// Creates ImageBuffer_Byte by wrapping given image buffer.
	/// </summary>
	ImageBuffer_Byte(ImageBuffer_uint8* image) {
		_bitDepth = BitDepth::BD_8_BIT;
		_image_8bit = image;
		_image_base = static_cast<ImageBuffer_Base*>(image);
	}

	/// <summary>
	/// Creates ImageBuffer_Byte by wrapping given image buffer.
	/// </summary>
	ImageBuffer_Byte(ImageBuffer_uint16* image) {
		_bitDepth = BitDepth::BD_16_BIT;
		_image_16bit = image;
		_image_base = static_cast<ImageBuffer_Base*>(image);
	}

	/// <summary>
	/// Creates ImageBuffer_Byte by wrapping given image buffer.
	/// </summary>
	ImageBuffer_Byte(ImageBuffer_uint32* image) {
		_bitDepth = BitDepth::BD_32_BIT;
		_image_32bit = image;
		_image_base = static_cast<ImageBuffer_Base*>(image);
	}

	/// <summary>
	/// Copy constructor.
	/// </summary>
	ImageBuffer_Byte(const ImageBuffer_Byte& source);

	/// <summary>
	/// Move constructor.
	/// </summary>
	ImageBuffer_Byte(ImageBuffer_Byte&& source);

	/// <summary>
	/// Copy assigment.
	/// </summary>
	ImageBuffer_Byte& operator=(ImageBuffer_Byte& source);


	//--------------------------------
	//	DESTRUCTOR
	//--------------------------------

	/// <summary>
	/// Destroys underlying image object.
	/// </summary>
	~ImageBuffer_Byte();

protected:
	//--------------------------------
	//	FIELDS
	//--------------------------------

	BitDepth _bitDepth = BD_8_BIT;

	ImageBuffer_uint8* _image_8bit = NULL;
	ImageBuffer_uint16* _image_16bit = NULL;
	ImageBuffer_uint32* _image_32bit = NULL;
	ImageBuffer_Base* _image_base = NULL;
};