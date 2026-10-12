#include "ImageBuffer_Byte.h"

//--------------------------------
//	GET/SET
//--------------------------------

///<summary>
///Image data.
///Array of rows of pixel samples that ordered according to image layout.
///Do not deallocate.
///</summary>
uint8_t** ImageBuffer_Byte::GetDataPtr() const {
	switch (_bitDepth)
	{
		case BD_8_BIT:
			return _image_8bit->_data;
		case BD_16_BIT:
			return reinterpret_cast<uint8_t**>(_image_16bit->_data);
		case BD_32_BIT:
			return reinterpret_cast<uint8_t**>(_image_32bit->_data);
		default:
			return nullptr;
	}
}

/// <summary>
/// Tells if data for this buffer is allocated.
/// </summary>
/// <returns></returns>
bool ImageBuffer_Byte::IsAllocated() const {
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
ImageBuffer_Byte ImageBuffer_Byte::RemoveAlphaChannel() const {
	switch (_bitDepth)
	{
	case BitDepth::BD_8_BIT: {
		ImageBuffer_uint8 alpha_stripped_image = this->_image_8bit->RemoveAlphaChannel(); //Removing alpha channel
		ImageBuffer_uint8* alpha_stripped_image_on_heap = new ImageBuffer_uint8(std::move(alpha_stripped_image)); //Moving image object to heap
		return ImageBuffer_Byte::BuildByWrapping(alpha_stripped_image_on_heap); //Wrapping IB_Byte object
	}

	case BitDepth::BD_16_BIT: {
		ImageBuffer_uint16 alpha_stripped_image = this->_image_16bit->RemoveAlphaChannel(); //Removing alpha channel
		ImageBuffer_uint16* alpha_stripped_image_on_heap = new ImageBuffer_uint16(std::move(alpha_stripped_image)); //Moving image object to heap
		return ImageBuffer_Byte::BuildByWrapping(alpha_stripped_image_on_heap); //Wrapping IB_Byte object
	}

	case BitDepth::BD_32_BIT: {
		ImageBuffer_uint8 alpha_stripped_image = this->_image_8bit->RemoveAlphaChannel(); //Removing alpha channel
		ImageBuffer_uint8* alpha_stripped_image_on_heap = new ImageBuffer_uint8(std::move(alpha_stripped_image)); //Moving image object to heap
		return ImageBuffer_Byte::BuildByWrapping(alpha_stripped_image_on_heap); //Wrapping IB_Byte object
	}

	default:
		//No suitable switch option
		throw new std::exception("ImageBuffer_Byte -- Invalid internal State: BitDepth is not properly set");
	}
}



/// <summary>
/// Changes bit depth of this image and returns the result.
/// </summary>
ImageBuffer_Byte ImageBuffer_Byte::ChangeBitDepth(BitDepth bitDepth) const {
	//Aliases
	int height = _image_base->_height;
	int width = _image_base->_width;
	int cmp_width = _image_base->GetCmpWidth();
	ImagePixelLayout layout = _image_base->_layout;

	switch (_bitDepth)
	{
		case BitDepth::BD_8_BIT: { //Source: 8 bit
			//Aliases
			uint8_t** src_data = _image_8bit->GetDataPtr();

			switch (bitDepth)
			{
				case BD_8_BIT: { //8 to 8 - return a copy
					ImageBuffer_uint8* trg_image = _image_8bit->Clone();
					return ImageBuffer_Byte::BuildByWrapping(trg_image);
				}

				case BD_16_BIT: { //8 to 16 - multiply by 257
					ImageBuffer_uint16* trg_image = NULL;

					if (_image_8bit->_allocated == false)
						trg_image = new ImageBuffer_uint16(height, width, layout, false);
					else {
						//Init target 16 bit image buffer
						trg_image = new ImageBuffer_uint16(height, width, layout);
						uint16_t** trg_data = trg_image->GetDataPtr();
						uint16_t expanded = 0;

						//Converting
						for (int row = 0; row < height; row++) {
							for (int cmp = 0; cmp < cmp_width; cmp++) {
								expanded = static_cast<uint16_t>(src_data[row][cmp]);
								trg_data[row][cmp] = expanded * 257;
							}
						}
					}
				
					return ImageBuffer_Byte::BuildByWrapping(trg_image);
				}

				case BD_32_BIT: { //8 to 32 - multiply by 16,843,009
					ImageBuffer_uint32* trg_image = NULL;

					if (_image_8bit->_allocated == false)
						trg_image = new ImageBuffer_uint32(height, width, layout, false);
					else {
						//Init target 32 bit image buffer
						trg_image = new ImageBuffer_uint32(height, width, layout);
						uint32_t** trg_data = trg_image->GetDataPtr();
						uint32_t expanded = 0;

						//Converting
						for (int row = 0; row < height; row++) {
							for (int cmp = 0; cmp < cmp_width; cmp++) {
								expanded = static_cast<uint32_t>(src_data[row][cmp]);
								trg_data[row][cmp] = expanded * 16843009;
							}
						}
					}

					return ImageBuffer_Byte::BuildByWrapping(trg_image);
				}
			}
		}//END: Case 8 bit source


		case BitDepth::BD_16_BIT: { //Source: 16 bit
			//Aliases
			uint16_t**  src_data = _image_16bit->GetDataPtr();

			switch (bitDepth)
			{
				case BD_8_BIT: { //16 to 8 - shift right
					ImageBuffer_uint8* trg_image = NULL;
					if (_image_16bit->_allocated == false)
						trg_image = new ImageBuffer_uint8(height, width, layout, false);
					else {
						//Init target 8 bit image buffer
						trg_image = new ImageBuffer_uint8(height, width, layout);
						uint8_t** trg_data = trg_image->GetDataPtr();

						//Converting
						for (int row = 0; row < height; row++)
							for (int cmp = 0; cmp < cmp_width; cmp++)
								trg_data[row][cmp] = static_cast<uint8_t>(src_data[row][cmp] >> 8);
					}

					return ImageBuffer_Byte::BuildByWrapping(trg_image);
				}

				case BD_16_BIT: { //16 to 16 - return a copy
					ImageBuffer_uint16* trg_image = _image_16bit->Clone();
					return ImageBuffer_Byte::BuildByWrapping(trg_image);
				}

				case BD_32_BIT: { //16 to 32 - multiply by 65,537
					ImageBuffer_uint32* trg_image = NULL;

					if (_image_16bit->_allocated == false)
						trg_image = new ImageBuffer_uint32(height, width, layout, false);
					else {
						//Init target 32 bit image buffer
						trg_image = new ImageBuffer_uint32(height, width, layout);
						uint32_t** trg_data = trg_image->GetDataPtr();
						uint32_t expanded = 0;

						//Converting
						for (int row = 0; row < height; row++) {
							for (int cmp = 0; cmp < cmp_width; cmp++) {
								expanded = static_cast<uint32_t>(src_data[row][cmp]);
								trg_data[row][cmp] = expanded * 65537;
							}
						}
					}

					return ImageBuffer_Byte::BuildByWrapping(trg_image);
				}
			}
		}//END: Case 16 bit source

		case BitDepth::BD_32_BIT: { //Source: 32 bit
			//Aliases
			uint32_t** src_data = _image_32bit->GetDataPtr();

			switch (bitDepth)
			{
				case BD_8_BIT: { //32 to 8 - shift right 24
					ImageBuffer_uint8* trg_image = NULL;

					if (_image_32bit->_allocated == false)
						trg_image = new ImageBuffer_uint8(height, width, layout, false);
					else {
						//Init target 8 bit image buffer
						trg_image = new ImageBuffer_uint8(height, width, layout);
						uint8_t** trg_data = trg_image->GetDataPtr();

						//Converting
						for (int row = 0; row < height; row++)
							for (int cmp = 0; cmp < cmp_width; cmp++)
								trg_data[row][cmp] = static_cast<uint8_t>(src_data[row][cmp] >> 24);

					}

					return ImageBuffer_Byte::BuildByWrapping(trg_image);
				}

				case BD_16_BIT: { //32 to 16 - shift right 16
					ImageBuffer_uint16* trg_image = NULL;

					if (_image_32bit->_allocated == false)
						trg_image = new ImageBuffer_uint16(height, width, layout, false);
					else {
						//Init target 8 bit image buffer
						trg_image = new ImageBuffer_uint16(height, width, layout);
						uint16_t** trg_data = trg_image->GetDataPtr();

						//Converting
						for (int row = 0; row < height; row++)
							for (int cmp = 0; cmp < cmp_width; cmp++)
								trg_data[row][cmp] = static_cast<uint16_t>(src_data[row][cmp] >> 16);
					}

					return ImageBuffer_Byte::BuildByWrapping(trg_image);
				}

				case BD_32_BIT: { //32 to 32 - return copy
					ImageBuffer_uint32* trg_image = _image_32bit->Clone();
					return ImageBuffer_Byte::BuildByWrapping(trg_image);
				}
			}
		}//END: Case 32 bit source
	}//END: Switch - source bit depth

	//No suitable switch option
	throw new std::exception("ImageBuffer_Byte -- Invalid internal State: BitDepth is not properly set");
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
ImageBuffer_Byte ImageBuffer_Byte::TransformBuffer(int height, int width, ImagePixelLayout layout) const {
	switch (_bitDepth)
	{
		case BD_8_BIT: {
			ImageBuffer_uint8 trg_image = _image_8bit->TransformBuffer(height, width, layout);
			ImageBuffer_uint8* trg_image_heap_ptr = new ImageBuffer_uint8(std::move(trg_image));
			return ImageBuffer_Byte::BuildByWrapping(trg_image_heap_ptr);
		}

		case BD_16_BIT: {
			ImageBuffer_uint16 trg_image = _image_16bit->TransformBuffer(height, width, layout);
			ImageBuffer_uint16* trg_image_heap_ptr = new ImageBuffer_uint16(std::move(trg_image));
			return ImageBuffer_Byte::BuildByWrapping(trg_image_heap_ptr);
		}

		case BD_32_BIT: {
			ImageBuffer_uint32 trg_image = _image_32bit->TransformBuffer(height, width, layout);
			ImageBuffer_uint32* trg_image_heap_ptr = new ImageBuffer_uint32(std::move(trg_image));
			return ImageBuffer_Byte::BuildByWrapping(trg_image_heap_ptr);
		}
	}

	//No suitable switch option
	throw new std::exception("ImageBuffer_Byte -- Invalid internal State: BitDepth is not properly set");
}


/// <summary>
/// Inserts given image into this image starting at a specified line.
/// If widths mismatch columns will be trimmed/expanded with black fill.
/// </summary>
/// <param name="image">Image to insert</param>
/// <param name="line">Line in this image where first line of inserted image should be placed.</param>
void ImageBuffer_Byte::InsertAtLine(const ImageBuffer_Byte& image, const int line){
	//Image buffer to be inserted
	//ImageBuffer_Byte* src_image_ptr = &image;

	switch (_bitDepth)
	{
		case BD_8_BIT: {
			if (image._bitDepth == BitDepth::BD_8_BIT)
				this->_image_8bit->InsertAtLine(*(image._image_8bit), line);
			else {
				ImageBuffer_Byte image_bit_matched = image.ChangeBitDepth(BitDepth::BD_8_BIT);
				this->_image_8bit->InsertAtLine(*(image_bit_matched._image_8bit), line);
			}									
		}
		break;

		case BD_16_BIT: {
			if (image._bitDepth == BitDepth::BD_16_BIT)
				this->_image_16bit->InsertAtLine(*(image._image_16bit), line);
			else {
				ImageBuffer_Byte image_bit_matched = image.ChangeBitDepth(BitDepth::BD_16_BIT);
				this->_image_16bit->InsertAtLine(*(image_bit_matched._image_16bit), line);
			}
		}
		break;
		
		case BD_32_BIT: {
			if (image._bitDepth == BitDepth::BD_32_BIT)
				this->_image_32bit->InsertAtLine(*(image._image_32bit), line);
			else {
				ImageBuffer_Byte image_bit_matched = image.ChangeBitDepth(BitDepth::BD_32_BIT);
				this->_image_32bit->InsertAtLine(*(image_bit_matched._image_32bit), line);
			}
		}
		break;
	}
}


//--------------------------------
//	CONSTRUCTORS
//--------------------------------


///<summary>
///Creates empty image with given dimensions and layout.
///Data can be allocated or not.
///</summary>
ImageBuffer_Byte::ImageBuffer_Byte(int height, int width, ImagePixelLayout layout, BitDepth bit_depth, bool isAllocated) {
	_bitDepth = bit_depth;

	switch (bit_depth)
	{
		case BD_8_BIT:
			_image_8bit = new ImageBuffer_uint8(height, width, layout, isAllocated);
			_image_base = static_cast<ImageBuffer_Base*>(_image_8bit);
			break;

		case BD_16_BIT:
			_image_16bit = new ImageBuffer_uint16(height, width, layout, isAllocated);
			_image_base = static_cast<ImageBuffer_Base*>(_image_16bit);
			break;

		case BD_32_BIT:
			_image_32bit = new ImageBuffer_uint32(height, width, layout, isAllocated);
			_image_base = static_cast<ImageBuffer_Base*>(_image_32bit);
			break;

		default:
			break;
	}
}


///<summary>
///Creates empty image with given dimensions and layout.
///Allocates data.
///Each row is allocated individually.
///Image content is not set and cannot be assumed.
///</summary>
ImageBuffer_Byte::ImageBuffer_Byte(int height, int width, ImagePixelLayout layout, BitDepth bit_depth)
	: ImageBuffer_Byte(height, width, layout, bit_depth, true) {

}


//--------------------------------
//	COPY/MOVE
//--------------------------------


/// <summary>
/// Move constructor.
/// </summary>
ImageBuffer_Byte::ImageBuffer_Byte(ImageBuffer_Byte&& other) {
	//Copying fields
	_bitDepth = other._bitDepth;
	
	//Copying underlying image pointer
	//and setting it to null in original 
	//to indicate that data was moved out
	switch (_bitDepth)
	{
		case BitDepth::BD_8_BIT:
			_image_8bit = other._image_8bit;
			other._image_8bit = nullptr;
		break;

		case BitDepth::BD_16_BIT:
			_image_16bit = other._image_16bit;
			other._image_16bit = nullptr;
		break;

		case BitDepth::BD_32_BIT:
			_image_32bit = other._image_32bit;
			other._image_32bit = nullptr;
		break;

		default:
			break;
	}

	//Image base pointer
	_image_base = other._image_base;
	other._image_base = nullptr;
}


/// <summary>
/// Move assigment.
/// </summary>
ImageBuffer_Byte& ImageBuffer_Byte::operator=(ImageBuffer_Byte&& other) {
	//Self assigment check
	if (&other == this)
		return *this;

	//If some image is already referenced we dispose of it
	switch (this->_bitDepth)
	{
		case BD_8_BIT:
			if (_image_8bit != nullptr) {
				delete _image_8bit;
				_image_8bit = nullptr;
			}
			break;

		case BD_16_BIT:
			if (_image_16bit != nullptr) {
				delete _image_16bit;
				_image_16bit = nullptr;
			}
			break;

		case BD_32_BIT:
			if (_image_32bit != nullptr) {
				delete _image_32bit;
				_image_32bit = nullptr;
			}
			break;

		default:
			break;
	}

	//Setting bit depth
	this->_bitDepth = other._bitDepth;

	//Copying underlying image pointer
	//and setting it to null in original 
	//to indicate that data was moved out
	switch (_bitDepth)
	{
		case BD_8_BIT:
			_image_8bit = other._image_8bit;
			other._image_8bit = nullptr;
			break;
		case BD_16_BIT:
			_image_16bit = other._image_16bit;
			other._image_16bit = nullptr;
			break;
		case BD_32_BIT:
			_image_32bit = other._image_32bit;
			other._image_32bit = nullptr;
			break;
		default:
			break;
	}

	//Image base pointer
	_image_base = other._image_base;
	other._image_base = nullptr;

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
		if (_image_8bit != nullptr)
			delete _image_8bit;
		break;

	case BD_16_BIT:
		if (_image_8bit != nullptr)
			delete _image_16bit;
		break;

	case BD_32_BIT:
		if (_image_8bit != nullptr)
			delete _image_32bit;
		break;

	default:
		break;
	}
}


//--------------------------------
//	ARCHIVE
//--------------------------------

/*

/// <summary>
/// Copy constructor.
/// </summary>
ImageBuffer_Byte::ImageBuffer_Byte(const ImageBuffer_Byte& other) {
	_bitDepth = other._bitDepth;

	//Aliases
	int height = other._image_base->GetHeight();
	int width = other._image_base->GetWidth();
	ImagePixelLayout layout = other._image_base->GetLayout();
	int cmp_width = other._image_base->GetCmpWidth();

	switch (_bitDepth)
	{
		case BitDepth::BD_8_BIT: {
			if (other._image_8bit->_allocated) {
				_image_8bit = new ImageBuffer_uint8(height, width, layout);

				uint8_t** src_data = other._image_8bit->GetDataPtr();
				uint8_t** copy_data = _image_8bit->GetDataPtr();
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
			if (other._image_16bit->_allocated) {
				_image_16bit = new ImageBuffer_uint16(height, width, layout);

				uint16_t** src_data = reinterpret_cast<uint16_t**>(other._image_16bit->GetDataPtr());
				uint16_t** copy_data = reinterpret_cast<uint16_t**>(_image_16bit->GetDataPtr());
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
			if (other._image_32bit->_allocated) {
				_image_32bit = new ImageBuffer_uint32(height, width, layout);

				uint32_t** src_data = reinterpret_cast<uint32_t**>(other._image_32bit->GetDataPtr());
				uint32_t** copy_data = reinterpret_cast<uint32_t**>(_image_32bit->GetDataPtr());
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
				uint8_t** src_data = source._image_8bit->GetDataPtr();
				uint8_t** copy_data = _image_8bit->GetDataPtr();

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

				uint16_t** src_data = reinterpret_cast<uint16_t**>(source._image_16bit->GetDataPtr());
				uint16_t** copy_data = reinterpret_cast<uint16_t**>(_image_16bit->GetDataPtr());
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

				uint32_t** src_data = reinterpret_cast<uint32_t**>(source._image_32bit->GetDataPtr());
				uint32_t** copy_data = reinterpret_cast<uint32_t**>(_image_32bit->GetDataPtr());
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

*/