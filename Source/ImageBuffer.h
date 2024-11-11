#pragma once
//See ImageBuffer aliases at the end of this file

//STL
#include <exception>
//Internal
#include "ImageBuffer_Base.h"


/// <summary>
/// Object that stores an image as two dimensional array of pixels.
/// Data is not incapsulated and maintaining consistency is a responsibiliy of the user.
/// </summary>
template <typename T, T TMin, T TMax, T (*RGBtoG)(const T, const T, const T)> 
class ImageBuffer : public ImageBuffer_Base {
	friend class ImageBuffer_Byte;

public:
	//--------------------------------
	//	GET/SET
	//--------------------------------

	///<summary>
	///Image data.
	///Array of rows of pixel samples that ordered according to image layout.
	///Do not deallocate.
	///</summary>
	T** GetData() { return _data; }

	/// <summary>
	/// Sets data array for this buffer and sets "allocated" flag to true.
	/// </summary>
	/// <param name="data"></param>
	void SetData(uint8_t** data) {
		_data = data;
		_allocated = true;
	}

	/// <summary>
	/// Tells if data for this buffer is allocated.
	/// </summary>
	/// <returns></returns>
	bool IsAllocated() {
		return _allocated;
	}

	/// <summary>
	/// If data array was allocated outside of control of this object
	/// appropriate flag should be set.
	/// </summary>
	void SetAllocated() {
		_allocated = true;
	}

	/// <summary>
	/// If data array was deallocated outside of control of this object
	/// appropriate flag should be set.
	/// </summary>
	void SetDeallocated() {
		_allocated = false;
		_data = NULL;
	}

	//--------------------------------
	//	METHODS
	//--------------------------------

	/// <summary>
	/// If data is deallocated creates new data array.
	/// </summary>
	void AllocateData() {
		if (_allocated == false) {
			//Allocating array for row pointers
			_data = new T * [_height];

			//Allocating each row
			int cmpWidth = _width * _numCmp;
			for (int row = 0; row < _height; row++)
				_data[row] = new T[cmpWidth];

			//Setting the flag
			_allocated = true;
		}
	}

	/// <summary>
	/// Returns a copy of this image with alpha channel removed.
	/// </summary>
	ImageBuffer<T, TMin, TMax, RGBtoG>* RemoveAlphaChannel() {

		//For layouts that do not contain alpha channel we return a copy.
		if (_layout == ImagePixelLayout::G || _layout == ImagePixelLayout::RGB)
			return new ImageBuffer<T, TMin, TMax, RGBtoG>(*this);

		//If there is no data we return empty image buffer
		if (_allocated == false) {
			switch (_layout)
			{
				case GA:
					return new ImageBuffer<T, TMin, TMax, RGBtoG>(_height, _width, ImagePixelLayout::G, false);

				case RGBA:
					return new ImageBuffer<T, TMin, TMax, RGBtoG>(_height, _width, ImagePixelLayout::RGB, false);
			}
		}

		//Removing alpha
		switch (_layout)
		{
			case GA: {
				//Allocating the result
				ImageBuffer<T, TMin, TMax, RGBtoG>* trg_image = new ImageBuffer<T, TMin, TMax, RGBtoG>(_height, _width, ImagePixelLayout::G);
				T** trg_data = trg_image->GetData();

				//Copying data without alpha channel
				for (int row = 0; row < _height; row++)
					for (int px = 0; px < _width; px++)
						trg_data[row][px] = _data[row][px * 2];

				return trg_image;
			}

			case RGBA: {
				//Allocating the result
				ImageBuffer<T, TMin, TMax, RGBtoG>* trg_image = new ImageBuffer<T, TMin, TMax, RGBtoG>(_height, _width, ImagePixelLayout::RGB);
				T** trg_data = trg_image->GetData();

				//Copying data without alpha channel
				for (int row = 0; row < _height; row++) {
					for (int px = 0; px < _width; px++) {
						trg_data[row][px * 3 + 0] = _data[row][px * 4 + 0];
						trg_data[row][px * 3 + 1] = _data[row][px * 4 + 1];
						trg_data[row][px * 3 + 2] = _data[row][px * 4 + 2];
					}
				}

				return trg_image;
			}
		}
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
	ImageBuffer<T, TMin, TMax, RGBtoG>* TransformBuffer(int height, int width, ImagePixelLayout layout) {

		//If data is not allocated we return new empty container
		if (_allocated == false)
			return new ImageBuffer<T, TMin, TMax, RGBtoG>(height, width, layout, false);

		//Allocating new buffer
		ImageBuffer<T, TMin, TMax, RGBtoG>* result = new ImageBuffer<T, TMin, TMax, RGBtoG>(height, width, layout);

		//Aliases
		T** src_data = this->_data;
		T** trg_data = result->GetData();
		int src_height = this->_height;
		int src_width = this->_width;
		int trg_height = height;
		int trg_width = width;
		ImagePixelLayout src_layout = this->_layout;
		ImagePixelLayout trg_layout = layout;
		T black = TMin;
		T opaque = TMax;

		//How many rows of source image will be copied
		int copy_height = 0;
		if (src_height <= trg_height)
			copy_height = src_height;
		else
			copy_height = trg_height;

		//How many columns of source image will be copied
		int copy_width = 0;
		if (src_width <= trg_width)
			copy_width = src_width;
		else
			copy_width = trg_width;

		//Processing layout to layout transformations
		switch (src_layout) {
			case ImagePixelLayout::G: {
				switch (trg_layout) {
					case ImagePixelLayout::G: {
						//G -> G
						//Same layout, just copying the data	
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++)
								trg_data[row][px] = src_data[row][px];
							//Empty columns
							for (int px = copy_width; px < trg_width; px++)
								trg_data[row][px] = black;
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++)
								trg_data[row][px] = black;
						}

						return result;
					}

					case ImagePixelLayout::GA: {
						//G -> GA
						//Adding alpha channel set to 255
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 2 + 0] = src_data[row][px];
								trg_data[row][px * 2 + 1] = opaque;
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 2 + 0] = black;
								trg_data[row][px * 2 + 1] = opaque;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 2 + 0] = black;
								trg_data[row][px * 2 + 1] = opaque;
							}
						}

						return result;
					}

					case ImagePixelLayout::RGB: {
						//G -> RGB
						//Copying grayscale to 3 channels
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 3 + 0] = src_data[row][px];
								trg_data[row][px * 3 + 1] = src_data[row][px];
								trg_data[row][px * 3 + 2] = src_data[row][px];
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 3 + 0] = black;
								trg_data[row][px * 3 + 1] = black;
								trg_data[row][px * 3 + 2] = black;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 3 + 0] = black;
								trg_data[row][px * 3 + 1] = black;
								trg_data[row][px * 3 + 2] = black;
							}
						}

						return result;
					}

					case ImagePixelLayout::RGBA: {
						//G -> RGBA
						//Copying grayscale to 3 channels and adding alpha channel set to opaque
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 4 + 0] = src_data[row][px];
								trg_data[row][px * 4 + 1] = src_data[row][px];
								trg_data[row][px * 4 + 2] = src_data[row][px];
								trg_data[row][px * 4 + 3] = opaque;
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 4 + 0] = black;
								trg_data[row][px * 4 + 1] = black;
								trg_data[row][px * 4 + 2] = black;
								trg_data[row][px * 4 + 3] = opaque;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 4 + 0] = black;
								trg_data[row][px * 4 + 1] = black;
								trg_data[row][px * 4 + 2] = black;
								trg_data[row][px * 4 + 3] = opaque;
							}
						}

						return result;
					}

				} //Target switch
			}//Case source grayscale

			case ImagePixelLayout::GA: {
				switch (trg_layout) {
					case ImagePixelLayout::G: {
						//GA -> G
						//Dropping alpha
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++)
								trg_data[row][px] = src_data[row][px * 2];
							//Empty columns
							for (int px = copy_width; px < trg_width; px++)
								trg_data[row][px] = black;
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++)
							for (int px = 0; px < trg_width; px++)
								trg_data[row][px] = black;

						return result;
					}

					case ImagePixelLayout::GA: {
						//GA -> GA
						//Same layout, simply copying
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 2 + 0] = src_data[row][px * 2 + 0];
								trg_data[row][px * 2 + 1] = src_data[row][px * 2 + 1];
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 2 + 0] = black;
								trg_data[row][px * 2 + 1] = opaque;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 2 + 0] = black;
								trg_data[row][px * 2 + 1] = opaque;
							}
						}

						return result;
					}

					case ImagePixelLayout::RGB: {
						//GA -> RGB
						//Copying grayscale to 3 channels, dropping alpha
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 3 + 0] = src_data[row][px * 2];
								trg_data[row][px * 3 + 1] = src_data[row][px * 2];
								trg_data[row][px * 3 + 2] = src_data[row][px * 2];
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 3 + 0] = black;
								trg_data[row][px * 3 + 1] = black;
								trg_data[row][px * 3 + 2] = black;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 3 + 0] = black;
								trg_data[row][px * 3 + 1] = black;
								trg_data[row][px * 3 + 2] = black;
							}
						}

						return result;
					}

					case ImagePixelLayout::RGBA: {
						//GA -> RGBA
						//Copying grayscale to 3 channels and alpha channel
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 4 + 0] = src_data[row][px * 2 + 0];
								trg_data[row][px * 4 + 1] = src_data[row][px * 2 + 0];
								trg_data[row][px * 4 + 2] = src_data[row][px * 2 + 0];
								trg_data[row][px * 4 + 3] = src_data[row][px * 2 + 1];
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 4 + 0] = black;
								trg_data[row][px * 4 + 1] = black;
								trg_data[row][px * 4 + 2] = black;
								trg_data[row][px * 4 + 3] = opaque;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 4 + 0] = black;
								trg_data[row][px * 4 + 1] = black;
								trg_data[row][px * 4 + 2] = black;
								trg_data[row][px * 4 + 3] = opaque;
							}
						}

						return result;
					}

				}//Target switch
			}//Case source gray alpha

			case ImagePixelLayout::RGB: {
				switch (trg_layout) {
					case ImagePixelLayout::G: {
						//RGB -> G
						//Averaging 3 components
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px] = RGBtoG(src_data[row][px * 3 + 0], src_data[row][px * 3 + 1], src_data[row][px * 3 + 2]);
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++)
								trg_data[row][px] = black;
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++)
							for (int px = 0; px < trg_width; px++)
								trg_data[row][px] = black;

						return result;
					}

					case ImagePixelLayout::GA: {
						uint16_t avg = 0;
						//RGB -> GA
						//Averaging 3 components and adding alpha
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 2 + 0] = RGBtoG(src_data[row][px * 3 + 0], src_data[row][px * 3 + 1], src_data[row][px * 3 + 2]);
								trg_data[row][px * 2 + 1] = opaque;
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 2 + 0] = black;
								trg_data[row][px * 2 + 1] = opaque;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 2 + 0] = black;
								trg_data[row][px * 2 + 1] = opaque;
							}
						}

						return result;
					}

					case ImagePixelLayout::RGB: {
						//RGB -> RGB
						//Same layout, simply copying
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 3 + 0] = src_data[row][px * 3 + 0];
								trg_data[row][px * 3 + 1] = src_data[row][px * 3 + 1];
								trg_data[row][px * 3 + 2] = src_data[row][px * 3 + 2];
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 3 + 0] = black;
								trg_data[row][px * 3 + 1] = black;
								trg_data[row][px * 3 + 2] = black;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 3 + 0] = black;
								trg_data[row][px * 3 + 1] = black;
								trg_data[row][px * 3 + 2] = black;
							}
						}

						return result;
					}

					case ImagePixelLayout::RGBA: {
						//RGB -> RGBA
						//Adding alpha
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 4 + 0] = src_data[row][px * 3 + 0];
								trg_data[row][px * 4 + 1] = src_data[row][px * 3 + 1];
								trg_data[row][px * 4 + 2] = src_data[row][px * 3 + 2];
								trg_data[row][px * 4 + 3] = opaque;
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 4 + 0] = black;
								trg_data[row][px * 4 + 1] = black;
								trg_data[row][px * 4 + 2] = black;
								trg_data[row][px * 4 + 3] = opaque;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 4 + 0] = black;
								trg_data[row][px * 4 + 1] = black;
								trg_data[row][px * 4 + 2] = black;
								trg_data[row][px * 4 + 3] = opaque;
							}
						}

						return result;
					}

				}//Target switch
			}//Case source RGB

			case ImagePixelLayout::RGBA: {
				switch (trg_layout) {
					case ImagePixelLayout::G: {
						uint16_t avg = 0;
						//RGBA -> G
						//Averaging 3 components, dropping alpha
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px] = RGBtoG(src_data[row][px * 4 + 0], src_data[row][px * 4 + 1], src_data[row][px * 4 + 2]);
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++)
								trg_data[row][px] = black;
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++)
							for (int px = 0; px < trg_width; px++)
								trg_data[row][px] = black;

						return result;
					}

					case ImagePixelLayout::GA: {
						uint16_t avg = 0;
						//RGBA -> GA
						//Averaging 3 components, copying alpha
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 2 + 0] = RGBtoG(src_data[row][px * 4 + 0], src_data[row][px * 4 + 1], src_data[row][px * 4 + 2]);
								trg_data[row][px * 2 + 1] = src_data[row][px * 4 + 3];
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 2 + 0] = black;
								trg_data[row][px * 2 + 1] = opaque;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 2 + 0] = black;
								trg_data[row][px * 2 + 1] = opaque;
							}
						}

						return result;
					}

					case ImagePixelLayout::RGB: {
						//RGBA -> RGB
						//Dropping alpha
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 3 + 0] = src_data[row][px * 4 + 0];
								trg_data[row][px * 3 + 1] = src_data[row][px * 4 + 1];
								trg_data[row][px * 3 + 2] = src_data[row][px * 4 + 2];
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 3 + 0] = black;
								trg_data[row][px * 3 + 1] = black;
								trg_data[row][px * 3 + 2] = black;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 3 + 0] = black;
								trg_data[row][px * 3 + 1] = black;
								trg_data[row][px * 3 + 2] = black;
							}
						}

						return result;
					}

					case ImagePixelLayout::RGBA: {
						//RGBA -> RGBA
						//Same layout, simply copying
						for (int row = 0; row < copy_height; row++) {
							for (int px = 0; px < copy_width; px++) {
								trg_data[row][px * 4 + 0] = src_data[row][px * 4 + 0];
								trg_data[row][px * 4 + 1] = src_data[row][px * 4 + 1];
								trg_data[row][px * 4 + 2] = src_data[row][px * 4 + 2];
								trg_data[row][px * 4 + 3] = src_data[row][px * 4 + 3];
							}
							//Empty columns
							for (int px = copy_width; px < trg_width; px++) {
								trg_data[row][px * 4 + 0] = black;
								trg_data[row][px * 4 + 1] = black;
								trg_data[row][px * 4 + 2] = black;
								trg_data[row][px * 4 + 3] = opaque;
							}
						}
						//Empty rows
						for (int row = copy_height; row < trg_height; row++) {
							for (int px = 0; px < trg_width; px++) {
								trg_data[row][px * 4 + 0] = black;
								trg_data[row][px * 4 + 1] = black;
								trg_data[row][px * 4 + 2] = black;
								trg_data[row][px * 4 + 3] = opaque;
							}
						}

						return result;
					}

				}//Switch target
			}//Case source RGBA

		}

		return result;
	}

	/// <summary>
	/// Inserts given image into this image starting at a specified line.
	/// If widths mismatch columns will be trimmed/expanded with black fill.
	/// </summary>
	/// <param name="image"></param>
	/// <param name="line"></param>
	void InsertAtLine(ImageBuffer<T, TMin, TMax, RGBtoG>* image, int line) {
		//Just a sanity check
		if (line < 0)
			return;

		//If data of this image is not allocated yet we allocate it
		if (_allocated == false)
			AllocateData();

		//We create a new image that conforms to width and layout of this image
		ImageBuffer<T, TMin, TMax, RGBtoG>* trans_img = image->TransformBuffer(image->GetHeight(), this->_width, this->_layout);

		//Aliases
		T** img_data = trans_img->GetData();
		int img_height = trans_img->GetHeight();
		int new_height = _height + img_height;

		//We check if there are new empty lines between this image and inserted one
		if (line > _height)
			new_height += line - _height;

		//New data array
		T** new_data = new T * [new_height];

		if (line < _height) {
			//Inserting given image between rows of this image (or prepending)
			//We are only moving row pointers from copy of inserted image to resulting image

			//Original rows before insert
			for (int src_row = 0; src_row < line; src_row++)
				new_data[src_row] = _data[src_row];
			//Inserted rows
			for (int img_row = 0; img_row < img_height; img_row++) {
				int trg_row = line + img_row;
				new_data[trg_row] = img_data[img_row];
			}
			//Original rows after the insert
			for (int src_row = line; src_row < _height; src_row++) {
				new_data[img_height + src_row] = _data[src_row];
			}
		}
		else {
			//Here given image will be put after this image possible with a set of empty lines in between
			//We are only moving row pointers from copy of inserted image to resulting image

			int cmp_width = this->_width;

			//Original rows
			for (int src_row = 0; src_row < _height; src_row++)
				new_data[src_row] = _data[src_row];
			//Blank rows
			for (int trg_row = _height; trg_row < line; trg_row++) {
				new_data[trg_row] = new T[cmp_width];
				for (int cmp = 0; cmp < cmp_width; cmp++)
					new_data[trg_row][cmp] = 0;
			}
			//Inserted image
			for (int img_row = 0; img_row < img_height; img_row++) {
				int trg_row = line + img_row;
				new_data[trg_row] = img_data[img_row];
			}
		}

		//Disposing of img data array without deallocating individual rows (they were moved) and deleting temporary object
		delete[] img_data;
		trans_img->SetDeallocated();
		trans_img->~ImageBuffer();

		//Setting new height
		_height = new_height;

		//Removing old rows array
		delete[] _data;

		//Setting new data
		_data = new_data;
	}

	/// <summary>
	/// Inserts given image into this image starting at the first line.
	/// If widths mismatch columns will be trimmed/expanded with black fill.
	/// </summary>
	/// <param name="image">Image to prepend.</param>
	void Prepend(ImageBuffer<T, TMin, TMax, RGBtoG>* image) {
		InsertAtLine(image, 0);
	}

	/// <summary>
	/// Inserts given image into this image starting right after the last line.
	/// If widths mismatch columns will be trimmed/expanded with black fill.
	/// </summary>
	/// <param name="image">Image to append.</param>
	void Append(ImageBuffer<T, TMin, TMax, RGBtoG>* image) {
		InsertAtLine(image, _height);
	}


	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	/// <summary>
	///Default constructor is disabled.
	/// </summary>
	ImageBuffer() = delete;

	///<summary>
	///Creates empty image with given dimensions and layout.
	///Data can be allocated or not.
	///</summary>
	ImageBuffer(int height, int width, ImagePixelLayout layout, bool isAllocated)
		: ImageBuffer_Base(height, width, layout) {
		if (isAllocated == true)
			AllocateData();
	}

	///<summary>
	///Creates empty image with given dimensions and layout.
	///Each row is allocated individually.
	///Image content is not set and cannot be assumed.
	///</summary>
	ImageBuffer(int height, int width, ImagePixelLayout layout)
		: ImageBuffer(height, width, layout, true) {

	}

	///<summary>
	///Copy constructor.
	///</summary>
	ImageBuffer(const ImageBuffer<T, TMin, TMax, RGBtoG>& source)
		: ImageBuffer_Base(source._height, source._width, source._layout) {

		//Copying the data if the source has it allocated
		_allocated = source._allocated;

		if (_allocated) {
			//How many pixel components there are in one image row
			int _cmpWidth = _width * _numCmp;

			//Producing data array for the copy
			T** _data = new T * [_height];
			//Allocating and copying rows
			for (int row = 0; row < _height; row++) {
				//Allocating row
				_data[row] = new T[_cmpWidth];
				//Copying data
				for (int cmp = 0; cmp < _cmpWidth; cmp++)
					_data[row][cmp] = source._data[row][cmp];
			}
		}
	}

	///<summary>
	///Move constructor.
	///</summary>
	ImageBuffer(ImageBuffer<T, TMin, TMax, RGBtoG>&& source)
		: ImageBuffer_Base(source._height, source._width, source._layout) {

		//Reassigning data
		_allocated = source._allocated;

		if (_allocated)
			_data = source._data;
		else
			_data = NULL;

		//Setting allocation flags, so source object can be safely disposed.
		source._allocated = false;
		source._data = NULL;
	}

	/// <summary>
	/// Copy assigment.
	/// </summary>
	ImageBuffer<T, TMin, TMax, RGBtoG>& operator=(ImageBuffer<T, TMin, TMax, RGBtoG>& other) {
		//Self assigment
		if (&other != this) {
			//Deleting existing data if present
			if (_allocated) {
				//Deallocating each row data
				for (int row = 0; row < _height; row++)
					delete[] _data[row];

				//Deallocating data array itself
				delete[] _data;

				_allocated = false;
			}
			
			//Base info
			_height = other._height;
			_width = other._width;
			_layout = other._layout;
			_numCmp = other._numCmp;
			_hasAlpha = other._hasAlpha;

			//Allocate and copy data if present
			if (other._allocated) {
				int cmp_width = _numCmp * _width;

				//Allocating rows array
				_data = new T * [_height];

				for (int row = 0; row < _height; row++) {
					//Allocating row and copying
					_data[row] = new T[cmp_width];
					for (int cmp = 0; cmp < cmp_width; cmp++)
						_data[row][cmp] = other._data[row][cmp];
				}

				_allocated = true;
			}
		}
		return *this;
	}

	/// <summary>
	/// Move assigment.
	/// </summary>
	ImageBuffer<T, TMin, TMax, RGBtoG>& operator=(ImageBuffer<T, TMin, TMax, RGBtoG>&& other) {
		if (&other != this) {
			//Deleting existing data if present
			if (_allocated) {
				//Deallocating each row data
				for (int row = 0; row < _height; row++)
					delete[] _data[row];

				//Deallocating data array itself
				delete[] _data;

				_allocated = false;
			}

			//ImageBuffer_Base fields
			_layout = other._layout;
			_height = other._height;
			_width = other._width;
			_numCmp = other._numCmp;
			_hasAlpha = other._hasAlpha;

			//Reassigning data
			_allocated = other._allocated;

			if (_allocated)
				_data = other._data;
			else
				_data = NULL;

			//Setting allocation flags for source object, so it can be safely disposed.
			other._allocated = false;
			other._data = NULL;
		}
		return *this;
	}


	//--------------------------------
	//	DESTRUCTOR
	//--------------------------------

	/// <summary>
	/// Deallocates data array.
	/// </summary>
	~ImageBuffer() {
		if (_allocated) {
			//Deallocating each row data
			for (int row = 0; row < _height; row++)
				delete[] _data[row];

			//Deallocating data array itself
			delete[] _data;
		}
	}

protected:
	//--------------------------------
	//	FIELDS
	//--------------------------------
	///<summary>
	///Image data. 
	///Array of rows of pixel samples that are ordered according to image layout.
	///</summary>
	T** _data = NULL;

	/// <summary>
	/// Tells if data is allocated. Used by destructor.
	/// </summary>
	bool _allocated = false;

};



//--------------------------------
//	TEMPLATE ALIASES
//--------------------------------

/// <summary>
/// Averages 3 color components of a pixel.
/// </summary>
inline uint8_t RGBtoG_uint8(const uint8_t red, const uint8_t green, const uint8_t blue) {
	uint16_t avg = static_cast<uint16_t>(red);
	avg += static_cast<uint16_t>(green);
	avg += static_cast<uint16_t>(blue);
	return static_cast<uint8_t>(avg / 3);
}

/// <summary>
/// Averages 3 color components of a pixel.
/// </summary>
inline uint16_t RGBtoG_uint16(const uint16_t red, const uint16_t green, const uint16_t blue) {
	uint32_t avg = static_cast<uint32_t>(red);
	avg += static_cast<uint32_t>(green);
	avg += static_cast<uint32_t>(blue);
	return static_cast<uint16_t>(avg / 3);
}

/// <summary>
/// Averages 3 color components of a pixel.
/// </summary>
inline uint32_t RGBtoG_uint32(const uint32_t red, const uint32_t green, const uint32_t blue) {
	uint64_t avg = static_cast<uint64_t>(red);
	avg += static_cast<uint64_t>(green);
	avg += static_cast<uint64_t>(blue);
	return static_cast<uint32_t>(avg / 3);
}

/// <summary>
/// Averages 3 color components of a pixel.
/// </summary>
inline float RGBtoG_float(const float red, const float green, const float blue) {
	float avg = red;
	avg += green;
	avg += blue;
	return avg / 3.0f;
}

//Aliases for templates
using ImageBuffer_uint8 = ImageBuffer < uint8_t, 0, 255, &RGBtoG_uint8 >;
using ImageBuffer_uint16 = ImageBuffer<uint16_t, 0, 65535, &RGBtoG_uint16>;
using ImageBuffer_uint32 = ImageBuffer<uint32_t, 0, 4294967295, &RGBtoG_uint32>;
using ImageBuffer_float = ImageBuffer<float, 0.0f, 1.0f, &RGBtoG_float>;