#pragma once
//STL
#include <exception>
#include "ImageBuffer.h"

/// <summary>
/// Composite image buffer object for storing images as uint arrays.
/// Image may be 8, 16 or 32 bits per component.
/// </summary>
/// <remarks>Not a descendand of ImageBuffer.</remarks>
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
	uint8_t** GetDataPtr() const;

	///<summary>
	///Layout of data in each pixel of the image.
	///</summary>
	ImagePixelLayout GetLayout() const { return _image_base->_layout; }

	/// <summary>
	///Bit depth of the image.
	/// </summary>
	BitDepth GetBitPerComponent() const { return _bitDepth; }

	///<summary>
	///Height of an image in pixels.
	///</summary>
	int GetHeight() const { return _image_base->_height; }

	///<summary>
	///Width of an image in pixels.
	///</summary>
	int GetWidth() const { return _image_base->_width; }

	///<summary>
	///Number of components in the pixel.
	///</summary>
	int GetNumCmp() const { return _image_base->_numCmp; }

	/// <summary>
	/// Number of individual pixel components in a full row.
	/// </summary>
	int GetCmpWidth() const { return _image_base->_numCmp*_image_base->_width; }

	///<summary>
	///If this image layout has alpha channel.
	///</summary>
	bool GetHasAlpha() const { return _image_base->_hasAlpha; }

	/// <summary>
	/// Tells if data for this buffer is allocated.
	/// </summary>
	/// <returns></returns>
	bool IsAllocated() const;

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
	/// Creates a copy of this image on the heap and returns a pointer.
	/// </summary>
	/// <returns>Pointer to the clone image.</returns>
	ImageBuffer_Byte* Clone() const {
		//Making a copy without data.
		//Underlying image object will be created as part of IB_Byte object, but with empty data pointer.
		ImageBuffer_Byte* copy = new ImageBuffer_Byte(this->GetHeight(), this->GetWidth(), this->GetLayout(), _bitDepth, false);

		//We delete underlying image object of the copy before replacing
		switch (_bitDepth)
		{
			case BD_8_BIT:
				delete copy->_image_8bit;
				break;
			case BD_16_BIT:
				delete copy->_image_16bit;
				break;
			case BD_32_BIT:
				delete copy->_image_32bit;
				break;
		}
		copy->_image_base = nullptr;

		//Copying underlying image
		switch (_bitDepth)
		{
			case BD_8_BIT:
				copy->_image_8bit = this->_image_8bit->Clone();
				copy->_image_base = static_cast<ImageBuffer_Base*>(copy->_image_8bit);
				break;
			case BD_16_BIT:
				copy->_image_16bit = this->_image_16bit->Clone();
				copy->_image_base = static_cast<ImageBuffer_Base*>(copy->_image_16bit);
				break;
			case BD_32_BIT:
				copy->_image_32bit = this->_image_32bit->Clone();
				copy->_image_base = static_cast<ImageBuffer_Base*>(copy->_image_32bit);
				break;
		}

		return copy;
	}

	/// <summary>
	/// If data is deallocated creates new data array.
	/// </summary>
	void AllocateData();

	
	/// <summary>
	/// Returns a copy of this image with alpha channel removed.
	/// </summary>
	ImageBuffer_Byte RemoveAlphaChannel() const;

	

	/// <summary>
	/// Changes bit depth of this image and returns the result.
	/// </summary>
	ImageBuffer_Byte ChangeBitDepth(BitDepth bitDepth) const;
	
	/// <summary>
	/// Transforms the layout and size of the image
	/// to match provided size and layout.
	/// Size is transformed by trimming/expanding with black. No pixel manipulation occurs.
	/// </summary>
	/// <param name="height">New height.</param>
	/// <param name="width">New width.</param>
	/// <param name="layout">New layout.</param>
	/// <returns>Transformed image buffer.</returns>
	ImageBuffer_Byte TransformBuffer(int height, int width, ImagePixelLayout layout) const;

	
	/// <summary>
	/// Inserts given image into this image starting at a specified line.
	/// If widths mismatch columns will be trimmed/expanded with black fill.
	/// </summary>
	/// <param name="image">Image to insert</param>
	/// <param name="line">Line in this image where first line of inserted image should be placed.</param>
	void InsertAtLine(const ImageBuffer_Byte& image, int line);

	/// <summary>
	/// Inserts given image into this image starting at the first line.
	/// If widths mismatch columns will be trimmed/expanded with black fill.
	/// </summary>
	/// <param name="image">Image to prepend.</param>
	void Prepend(const ImageBuffer_Byte& image) {
		InsertAtLine(image, 0);
	}

	/// <summary>
	/// Inserts given image into this image starting right after the last line.
	/// If widths mismatch columns will be trimmed/expanded with black fill.
	/// </summary>
	/// <param name="image">Image to append.</param>
	void Append(const ImageBuffer_Byte& image) {
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
	///Data can be allocated or not.
	///</summary>
	ImageBuffer_Byte(int height, int width, ImagePixelLayout layout, BitDepth bit_depth, bool isAllocated);


	///<summary>
	///Creates empty image with given dimensions and layout.
	///Allocates data.
	///Each row is allocated individually.
	///Image content is not set and cannot be assumed.
	///</summary>
	ImageBuffer_Byte(int height, int width, ImagePixelLayout layout, BitDepth bit_depth);

	//--------------------------------
	//	WRAP CONSTRUCTORS
	//--------------------------------

	/// <summary>
	/// Creates ImageBuffer_Byte by wrapping given image buffer pointer.
	/// </summary>
	static ImageBuffer_Byte BuildByWrapping(ImageBuffer_uint8* image_ptr) {
		ImageBuffer_Byte result(image_ptr->GetHeight(), image_ptr->GetWidth(), image_ptr->GetLayout(), BitDepth::BD_8_BIT, false);
		result._image_8bit = image_ptr;
		result._image_base = static_cast<ImageBuffer_Base*>(image_ptr);
		return result;
	}

	/// <summary>
	/// Creates ImageBuffer_Byte by wrapping given image buffer pointer.
	/// </summary>
	static ImageBuffer_Byte BuildByWrapping(ImageBuffer_uint16* image_ptr) {
		ImageBuffer_Byte result(image_ptr->GetHeight(), image_ptr->GetWidth(), image_ptr->GetLayout(), BitDepth::BD_16_BIT, false);
		result._image_16bit = image_ptr;
		result._image_base = static_cast<ImageBuffer_Base*>(image_ptr);
		return result;
	}

	/// <summary>
	/// Creates ImageBuffer_Byte by wrapping given image buffer pointer.
	/// </summary>
	static ImageBuffer_Byte BuildByWrapping(ImageBuffer_uint32* image_ptr) {
		ImageBuffer_Byte result(image_ptr->GetHeight(), image_ptr->GetWidth(), image_ptr->GetLayout(), BitDepth::BD_32_BIT, false);
		result._image_32bit = image_ptr;
		result._image_base = static_cast<ImageBuffer_Base*>(image_ptr);
		return result;
	}

	//--------------------------------
	//	COPY/MOVE
	//--------------------------------

	//Copy methods can be found in ARCHIVE section in .cpp file

	/// <summary>
	/// Copy constructor is disabled to prevent accidental copying.
	/// Use .Clone() method.
	/// </summary>
	ImageBuffer_Byte(const ImageBuffer_Byte& source) = delete;

	/// <summary>
	/// Copy assigment is disable to prevent accidental copying.
	/// Use .Clone() method.
	/// </summary>
	ImageBuffer_Byte& operator=(ImageBuffer_Byte& source) = delete;

	/// <summary>
	/// Move constructor.
	/// </summary>
	ImageBuffer_Byte(ImageBuffer_Byte&& other);

	/// <summary>
	/// Move assigment.
	/// </summary>
	ImageBuffer_Byte& operator=(ImageBuffer_Byte&& other);

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

	ImageBuffer_uint8* _image_8bit = nullptr;
	ImageBuffer_uint16* _image_16bit = nullptr;
	ImageBuffer_uint32* _image_32bit = nullptr;
	ImageBuffer_Base* _image_base = nullptr;
};