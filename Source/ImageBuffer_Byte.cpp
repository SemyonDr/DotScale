#include "ImageBuffer_Byte.h"

//--------------------------------
//	GET/SET
//--------------------------------

///<summary>
	///Image data.
	///Array of rows of pixel samples that ordered according to image layout.
	///Do not deallocate.
	///</summary>
uint8_t** ImageBuffer_Byte::GetData() {
	switch (_bitDepth)
	{
		case BD_8_BIT:
			return _image_8bit->_data;
		case BD_16_BIT:
			return reinterpret_cast<uint8_t**>(_image_16bit->_data);
		case BD_32_BIT:
			return reinterpret_cast<uint8_t**>(_image_32bit->_data);
		default:
			return NULL;
	}
}

/// <summary>
/// Tells if data for this buffer is allocated.
/// </summary>
/// <returns></returns>
bool ImageBuffer_Byte::IsAllocated() {
	switch (_bitDepth)
	{
		case BD_8_BIT:
			return _image_8bit->_allocated;
		case BD_16_BIT:
			return _image_16bit->_allocated;
		case BD_32_BIT:
			return _image_32bit->_allocated;
		default:
			return false;
	}
}

/// <summary>
/// If data array was allocated outside of control of this object
/// appropriate flag should be set.
/// </summary>
void ImageBuffer_Byte::SetAllocated() {
	switch (_bitDepth)
	{
		case BD_8_BIT:
			_image_8bit->SetAllocated();
			return;
		case BD_16_BIT:
			_image_16bit->SetAllocated();
			return;
		case BD_32_BIT:
			_image_32bit->SetAllocated();
			return;
	}
}

/// <summary>
/// If data array was deallocated outside of control of this object
/// appropriate flag should be set.
/// </summary>
void ImageBuffer_Byte::SetDeallocated() {
	switch (_bitDepth)
	{
		case BD_8_BIT:
			_image_8bit->SetDeallocated();
			return;
		case BD_16_BIT:
			_image_16bit->SetDeallocated();
			return;
		case BD_32_BIT:
			_image_32bit->SetDeallocated();
			return;
	}
}



//--------------------------------
//	METHODS
//--------------------------------

/// <summary>
/// If data is deallocated creates new data array.
/// </summary>
void ImageBuffer_Byte::AllocateData() {
	switch (_bitDepth)
	{
		case BD_8_BIT:
			_image_8bit->AllocateData();
			return;
		case BD_16_BIT:
			_image_16bit->AllocateData();
			return;
		case BD_32_BIT:
			_image_32bit->AllocateData();
			return;
	}
}

/// <summary>
/// Returns a copy of this image with alpha channel removed.
/// </summary>
ImageBuffer_Byte* ImageBuffer_Byte::RemoveAlphaChannel() {
	switch (_bitDepth)
	{
	case BitDepth::BD_8_BIT:
		return new ImageBuffer_Byte(_image_8bit->RemoveAlphaChannel());
	case BitDepth::BD_16_BIT:
		return new ImageBuffer_Byte(_image_16bit->RemoveAlphaChannel());
	case BitDepth::BD_32_BIT:
		return new ImageBuffer_Byte(_image_32bit->RemoveAlphaChannel());
	default:
		return NULL;
	}
}



/// <summary>
/// Converts 8 bit image to 16 bit image with the same layout.
/// </summary>
ImageBuffer_Byte* ImageBuffer_Byte::ChangeBitDepth(BitDepth bitDepth) {
	//Aliases
	int height = _image_base->_height;
	int width = _image_base->_width;
	int cmp_width = _image_base->GetCmpWidth();
	ImagePixelLayout layout = _image_base->_layout;

	switch (_bitDepth)
	{
		case BitDepth::BD_8_BIT: { //Source: 8 bit
			//Aliases
			uint8_t** src_data = _image_8bit->GetData();

			switch (bitDepth)
			{
				case BD_8_BIT: { //8 to 8 - return a copy
					ImageBuffer_uint8* trg_image = new ImageBuffer_uint8(*_image_8bit); //Making a copy
					return new ImageBuffer_Byte(trg_image);
				}

				case BD_16_BIT: { //8 to 16 - multiply by 257
					ImageBuffer_uint16* trg_image = NULL;

					if (_image_8bit->_allocated == false)
						trg_image = new ImageBuffer_uint16(height, width, layout, false);
					else {
						//Init target 16 bit image buffer
						trg_image = new ImageBuffer_uint16(height, width, layout);
						uint16_t** trg_data = trg_image->GetData();
						uint16_t expanded = 0;

						//Converting
						for (int row = 0; row < height; row++) {
							for (int cmp = 0; cmp < cmp_width; cmp++) {
								expanded = static_cast<uint16_t>(src_data[row][cmp]);
								trg_data[row][cmp] = expanded * 257;
							}
						}
					}
				
					return new ImageBuffer_Byte(trg_image);
				}

				case BD_32_BIT: { //8 to 32 - multiply by 16,843,009
					ImageBuffer_uint32* trg_image = NULL;

					if (_image_8bit->_allocated == false)
						trg_image = new ImageBuffer_uint32(height, width, layout, false);
					else {
						//Init target 32 bit image buffer
						trg_image = new ImageBuffer_uint32(height, width, layout);
						uint32_t** trg_data = trg_image->GetData();
						uint32_t expanded = 0;

						//Converting
						for (int row = 0; row < height; row++) {
							for (int cmp = 0; cmp < cmp_width; cmp++) {
								expanded = static_cast<uint32_t>(src_data[row][cmp]);
								trg_data[row][cmp] = expanded * 16843009;
							}
						}
					}

					return new ImageBuffer_Byte(trg_image);
				}
			}
		}//END: Case 8 bit source


		case BitDepth::BD_16_BIT: { //Source: 16 bit
			//Aliases
			uint16_t**  src_data = _image_16bit->GetData();

			switch (bitDepth)
			{
				case BD_8_BIT: { //16 to 8 - shift right
					ImageBuffer_uint8* trg_image = NULL;
					if (_image_16bit->_allocated == false)
						trg_image = new ImageBuffer_uint8(height, width, layout, false);
					else {
						//Init target 8 bit image buffer
						trg_image = new ImageBuffer_uint8(height, width, layout);
						uint8_t** trg_data = trg_image->GetData();

						//Converting
						for (int row = 0; row < height; row++)
							for (int cmp = 0; cmp < cmp_width; cmp++)
								trg_data[row][cmp] = static_cast<uint8_t>(src_data[row][cmp] >> 8);
					}

					return new ImageBuffer_Byte(trg_image);
				}

				case BD_16_BIT: { //16 to 16 - return a copy
					ImageBuffer_uint16* trg_image = new ImageBuffer_uint16(*_image_16bit); //Making a copy
					return new ImageBuffer_Byte(trg_image);
				}

				case BD_32_BIT: { //16 to 32 - multiply by 65,537
					ImageBuffer_uint32* trg_image = NULL;

					if (_image_16bit->_allocated == false)
						trg_image = new ImageBuffer_uint32(height, width, layout, false);
					else {
						//Init target 32 bit image buffer
						trg_image = new ImageBuffer_uint32(height, width, layout);
						uint32_t** trg_data = trg_image->GetData();
						uint32_t expanded = 0;

						//Converting
						for (int row = 0; row < height; row++) {
							for (int cmp = 0; cmp < cmp_width; cmp++) {
								expanded = static_cast<uint32_t>(src_data[row][cmp]);
								trg_data[row][cmp] = expanded * 65537;
							}
						}
					}

					return new ImageBuffer_Byte(trg_image);
				}
			}
		}//END: Case 16 bit source

		case BitDepth::BD_32_BIT: { //Source: 32 bit
			//Aliases
			uint32_t** src_data = _image_32bit->GetData();

			switch (bitDepth)
			{
				case BD_8_BIT: { //32 to 8 - shift right 24
					ImageBuffer_uint8* trg_image = NULL;

					if (_image_32bit->_allocated == false)
						trg_image = new ImageBuffer_uint8(height, width, layout, false);
					else {
						//Init target 8 bit image buffer
						trg_image = new ImageBuffer_uint8(height, width, layout);
						uint8_t** trg_data = trg_image->GetData();

						//Converting
						for (int row = 0; row < height; row++)
							for (int cmp = 0; cmp < cmp_width; cmp++)
								trg_data[row][cmp] = static_cast<uint8_t>(src_data[row][cmp] >> 24);

					}

					return new ImageBuffer_Byte(trg_image);
				}

				case BD_16_BIT: { //32 to 16 - shift right 16
					ImageBuffer_uint16* trg_image = NULL;

					if (_image_32bit->_allocated == false)
						trg_image = new ImageBuffer_uint16(height, width, layout, false);
					else {
						//Init target 8 bit image buffer
						trg_image = new ImageBuffer_uint16(height, width, layout);
						uint16_t** trg_data = trg_image->GetData();

						//Converting
						for (int row = 0; row < height; row++)
							for (int cmp = 0; cmp < cmp_width; cmp++)
								trg_data[row][cmp] = static_cast<uint16_t>(src_data[row][cmp] >> 16);
					}

					return new ImageBuffer_Byte(trg_image);
				}

				case BD_32_BIT: { //32 to 32 - return copy
					ImageBuffer_uint32* trg_image = new ImageBuffer_uint32(*_image_32bit);
					return new ImageBuffer_Byte(trg_image);
				}
			}
		}//END: Case 32 bit source
	}//END: Switch - source bit depth

	return NULL;
}



/// <summary>
/// Transforms the layout and size of the image
/// to match provided size and layout.
/// Size is transformed by trimming/expanding with black. No pixel manipulation occurs.
/// </summary>
/// <param name="height">New height.</param>
/// <param name="width">New width.</param>
/// <param name="layout">New layout.</param>
/// <returns>Transformed image buffer.</returns>
ImageBuffer_Byte* ImageBuffer_Byte::TransformBuffer(int height, int width, ImagePixelLayout layout) {
	switch (_bitDepth)
	{
		case BD_8_BIT: {
			ImageBuffer_uint8* trg_image = _image_8bit->TransformBuffer(height, width, layout);
			return new ImageBuffer_Byte(_image_8bit);
		}

		case BD_16_BIT: {
			ImageBuffer_uint16* trg_image = _image_16bit->TransformBuffer(height, width, layout);
			return new ImageBuffer_Byte(_image_16bit);
		}

		case BD_32_BIT: {
			ImageBuffer_uint32* trg_image = _image_32bit->TransformBuffer(height, width, layout);
			return new ImageBuffer_Byte(_image_32bit);
		}
	}

	return NULL;
}


/// <summary>
/// Inserts given image into this image starting at a specified line.
/// If widths mismatch columns will be trimmed/expanded with black fill.
/// </summary>
/// <param name="image">Image to insert</param>
/// <param name="line">Line in this image where first line of inserted image should be placed.</param>
void ImageBuffer_Byte::InsertAtLine(ImageBuffer_Byte* image, int line) {
	//Image buffer to be inserted
	ImageBuffer_Byte* src_image = image;

	//If bit depths do not match we change bit depth of inserted image
	if (image->_bitDepth != this->_bitDepth)
		src_image = image->ChangeBitDepth(this->_bitDepth);

	switch (_bitDepth)
	{
		case BD_8_BIT:
			_image_8bit->InsertAtLine(src_image->_image_8bit, line);
		break;

		case BD_16_BIT:
			_image_16bit->InsertAtLine(src_image->_image_16bit, line);
			break;

		case BD_32_BIT:
			_image_32bit->InsertAtLine(src_image->_image_32bit, line);
			break;
	}
}


//--------------------------------
//	CONSTRUCTORS
//--------------------------------


///<summary>
///Creates empty image with given dimensions and layout.
///Each row is allocated individually.
///</summary>
ImageBuffer_Byte::ImageBuffer_Byte(int height, int width, ImagePixelLayout layout, BitDepth bit_depth) {
	_bitDepth = bit_depth;

	switch (bit_depth)
	{
	case BD_8_BIT:
		_image_8bit = new ImageBuffer_uint8(height, width, layout);
		_image_base = static_cast<ImageBuffer_Base*>(_image_8bit);
		break;

	case BD_16_BIT:
		_image_16bit = new ImageBuffer_uint16(height, width, layout);
		_image_base = static_cast<ImageBuffer_Base*>(_image_16bit);
		break;

	case BD_32_BIT:
		_image_32bit = new ImageBuffer_uint32(height, width, layout);
		_image_base = static_cast<ImageBuffer_Base*>(_image_32bit);
		break;

	default:
		break;
	}
}



/// <summary>
/// Copy constructor.
/// </summary>
ImageBuffer_Byte::ImageBuffer_Byte(const ImageBuffer_Byte& source) {
	_bitDepth = source._bitDepth;

	//Aliases
	int height = source._image_base->GetHeight();
	int width = source._image_base->GetWidth();
	ImagePixelLayout layout = source._image_base->GetLayout();
	int cmp_width = source._image_base->GetCmpWidth();

	switch (_bitDepth)
	{
		case BitDepth::BD_8_BIT: {
			if (source._image_8bit->_allocated) {
				_image_8bit = new ImageBuffer_uint8(height, width, layout);
				
				uint8_t** src_data = source._image_8bit->GetData();
				uint8_t** copy_data = _image_8bit->GetData();
				for (int row = 0; row < height; row++) {
					for (int cmp = 0; cmp < cmp_width; cmp++) {
						copy_data[row][cmp] = src_data[row][cmp];
					}
				}
			}
			else
				_image_8bit = new ImageBuffer_uint8(height, width, layout, false);

			_image_base = static_cast<ImageBuffer_Base*>(_image_8bit);
		}
		break;

		case BitDepth::BD_16_BIT: {
			if (source._image_16bit->_allocated) {
				_image_16bit = new ImageBuffer_uint16(height, width, layout);

				uint16_t** src_data = reinterpret_cast<uint16_t**>(source._image_16bit->GetData());
				uint16_t** copy_data = reinterpret_cast<uint16_t**>(_image_16bit->GetData());
				for (int row = 0; row < height; row++) {
					for (int cmp = 0; cmp < cmp_width; cmp++) {
						copy_data[row][cmp] = src_data[row][cmp];
					}
				}
			}
			else
				_image_16bit = new ImageBuffer_uint16(height, width, layout, false);

			_image_base = static_cast<ImageBuffer_Base*>(_image_16bit);
		}
		break;

		case BitDepth::BD_32_BIT: {
			if (source._image_32bit->_allocated) {
				_image_32bit = new ImageBuffer_uint32(height, width, layout);

				uint32_t** src_data = reinterpret_cast<uint32_t**>(source._image_32bit->GetData());
				uint32_t** copy_data = reinterpret_cast<uint32_t**>(_image_32bit->GetData());
				for (int row = 0; row < height; row++) {
					for (int cmp = 0; cmp < cmp_width; cmp++) {
						copy_data[row][cmp] = src_data[row][cmp];
					}
				}
			}
			else
				_image_32bit = new ImageBuffer_uint32(height, width, layout, false);
	
			_image_base = static_cast<ImageBuffer_Base*>(_image_32bit);
		}
		break;

	default:
		break;
	}
}



/// <summary>
/// Move constructor.
/// </summary>
ImageBuffer_Byte::ImageBuffer_Byte(ImageBuffer_Byte&& source) {
	_bitDepth = source._bitDepth;
	int height = source._image_base->GetHeight();
	int width = source._image_base->GetWidth();
	int layout = source._image_base->GetLayout();
	int cmp_width = source._image_base->GetNumCmp() * width;

	switch (_bitDepth)
	{
		case BitDepth::BD_8_BIT: {
			_image_8bit = source._image_8bit;
			source._image_8bit = NULL;
		}
		break;

		case BitDepth::BD_16_BIT: {
			_image_16bit = source._image_16bit;
			source._image_16bit = NULL;
		}
		break;

		case BitDepth::BD_32_BIT: {
			_image_32bit = source._image_32bit;
			source._image_32bit = NULL;
		}
		break;

		default:
			break;
	}

	_image_base = source._image_base;
}



/// <summary>
/// Copy assigment.
/// </summary>
ImageBuffer_Byte& ImageBuffer_Byte::operator=(ImageBuffer_Byte& source) {
	if (this == &source)
		return *this;

	_bitDepth = source._bitDepth;

	int height = source._image_base->GetHeight();
	int width = source._image_base->GetWidth();
	ImagePixelLayout layout = source._image_base->GetLayout();
	int cmp_width = source._image_base->GetCmpWidth();

	switch (_bitDepth)
	{
		case BitDepth::BD_8_BIT: {
			//Deleting old data
			_image_8bit->~ImageBuffer();
				
			//Copying data if it is allocated with the source
			if (source._image_8bit->_allocated) {
				//Allocating new internal buffer
				_image_8bit = new ImageBuffer_uint8(height, width, layout);

				//Aliases
				uint8_t** src_data = source._image_8bit->GetData();
				uint8_t** copy_data = _image_8bit->GetData();

				//Copying
				for (int row = 0; row < height; row++)
					for (int cmp = 0; cmp < cmp_width; cmp++)
						copy_data[row][cmp] = src_data[row][cmp];
			}
			else //If data is not allocated we create unallocated buffer
				_image_8bit = new ImageBuffer_uint8(height, width, layout, false);
		}
		break;

		case BitDepth::BD_16_BIT: {
			_image_16bit->~ImageBuffer();

			if (source._image_16bit->_allocated) {
				_image_16bit = new ImageBuffer_uint16(height, width, layout);

				uint16_t** src_data = reinterpret_cast<uint16_t**>(source._image_16bit->GetData());
				uint16_t** copy_data = reinterpret_cast<uint16_t**>(_image_16bit->GetData());
				for (int row = 0; row < height; row++)
					for (int cmp = 0; cmp < cmp_width; cmp++)
						copy_data[row][cmp] = src_data[row][cmp];
			}
			else
				_image_16bit = new ImageBuffer_uint16(height, width, layout, false);
		}
		break;

		case BitDepth::BD_32_BIT: {
			_image_32bit->~ImageBuffer();
			if (source._image_32bit->_allocated) {
				_image_32bit = new ImageBuffer_uint32(height, width, layout);

				uint32_t** src_data = reinterpret_cast<uint32_t**>(source._image_32bit->GetData());
				uint32_t** copy_data = reinterpret_cast<uint32_t**>(_image_32bit->GetData());
				for (int row = 0; row < height; row++)
					for (int cmp = 0; cmp < cmp_width; cmp++)
						copy_data[row][cmp] = src_data[row][cmp];
			}
			else
				_image_32bit = new ImageBuffer_uint32(height, width, layout, false);
		}
		break;
	}

	return *this;
}


//--------------------------------
//	DESTRUCTOR
//--------------------------------

/// <summary>
/// Destroys underlying image.
/// </summary>
ImageBuffer_Byte::~ImageBuffer_Byte() {
	//Image pointer can be NULL if data was moved with move constructor.
	switch (_bitDepth)
	{
	case BD_8_BIT:
		if (_image_8bit != NULL)
			_image_8bit->~ImageBuffer_uint8();
		break;

	case BD_16_BIT:
		if (_image_8bit != NULL)
			_image_16bit->~ImageBuffer_uint16();
		break;

	case BD_32_BIT:
		if (_image_8bit != NULL)
			_image_32bit->~ImageBuffer_uint32();
		break;

	default:
		break;
	}
}


