#pragma once

/*
	Main data container for dotscale.
	Look for template aliases at the bottom of the file.
*/


#include <vector>
#include <memory>

#include "ImageEnums.h"

namespace dotscale {

//--------------------------------
//	FUNCTIONS REQUIRED 
//  BY THE TEMPLATE
//--------------------------------

namespace helper {
	uint8_t rgb_to_g_uint8(const uint8_t red, const uint8_t green, const uint8_t blue);
	uint16_t rgb_to_g_uint16(const uint16_t red, const uint16_t green, const uint16_t blue);
	uint32_t rgb_to_g_uint32(const uint32_t red, const uint32_t green, const uint32_t blue);
	float rgb_to_g_float(const float red, const float green, const float blue);
}


/// Object that stores an image as two dimensional array of pixels.
/// Pixel data is not incapsulated and maintaining consistency is a responsibiliy of the user.
template <
	typename T,
	T TMin,
	T TMax,
	T (*rgb_to_g)(const T, const T, const T)
> 
class ImageBuffer {
public:
	//--------------------------------
	//	PROPERTIES
	//--------------------------------

	/// Layout of the data in an image pixel.
	PixelLayout get_layout() const noexcept;

	/// Height of the image in pixels.
	std::size_t get_height() const noexcept;

	/// Width of the image in pixels.
	std::size_t get_width() const noexcept;

	/// Total number of components in one image row.
	std::size_t get_cmp_width() const noexcept;

	/// Number of components in a pixel.
	int get_num_cmp() const noexcept;

	/// If this image layout has alpha channel.
	bool has_alpha() const noexcept;
	
	/// Rows per continuous memory block.
	std::size_t get_rows_per_block() const noexcept;


	//--------------------------------
	//	ACCESSORS
	//--------------------------------

	///Image data.
	///Array of rows of pixel samples that ordered according to image layout.
	T* const * get_data_ptr() const noexcept;


	//--------------------------------
	//	METHODS
	//--------------------------------

	/// Sets all pixels to minimum value.
	void set_zero();
	

	/// Makes a copy of this image buffer.
	ImageBuffer<T, TMin, TMax, rgb_to_g> clone() const;

	
	/// Returns a copy of this image with alpha channel removed.
	ImageBuffer<T, TMin, TMax, rgb_to_g> remove_alpha() const;


	/// Transforms the layout and size of the image to match provided size and layout.
	/// Size is transformed by trimming/expanding with black. No pixel manipulation occurs.
	/// Returns a copy.
	ImageBuffer<T, TMin, TMax, rgb_to_g> reshape(int height, int width, PixelLayout layout) const;

	
	/// Inserts lines of an image into this buffer starting at a specified line.
	/// Widths and layouts must match.
	void insert_at(const ImageBuffer<T, TMin, TMax, rgb_to_g>& image, int line);

	
	/// Inserts given image into this image starting at the first line.
	/// Widths and layouts must match.
	void prepend(const ImageBuffer<T, TMin, TMax, rgb_to_g>& image);

	
	/// Inserts given image into this image starting right after the last line.
	/// Widths and layouts must match.
	void append(const ImageBuffer<T, TMin, TMax, rgb_to_g>& image);

	
	/// Returns Image Buffer that contains `height` rows of this buffer, starting from given line.
	ImageBuffer<T, TMin, TMax, rgb_to_g> get_slice(int start, int height) const;



	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	/// Creates an empty image buffer with given dimensions and layout.
	/// Image content is not initialized.
	ImageBuffer(int height, int width, PixelLayout layout);



	//--------------------------------
	//	COPY/MOVE
	//--------------------------------
	
	///Copy constructor is disabled to prevent accidential copying.
	///.Clone() method should be used.
	ImageBuffer(const ImageBuffer<T, TMin, TMax, rgb_to_g>& other) = delete;

	
	/// Copy assigment is disabled to prevent accidential copying.
	/// .Clone() method should be used.
	ImageBuffer<T, TMin, TMax, rgb_to_g>& operator=(ImageBuffer<T, TMin, TMax, rgb_to_g>& other) = delete;

	/// Move is default.
	ImageBuffer(ImageBuffer<T, TMin, TMax, rgb_to_g>&& other) noexcept = default;
	ImageBuffer<T, TMin, TMax, rgb_to_g>& operator=(ImageBuffer<T, TMin, TMax, rgb_to_g>&& other) noexcept = default;

	//--------------------------------
	//	OPERATORS
	//--------------------------------
	
	/// Row pointers are accessible by index.
	inline T* operator[](std::size_t row) const noexcept {
		return _row_ptrs[row];
	}



private:

	/// Templated implementation of the reshape() function.
	template<PixelLayout src_layout, PixelLayout trg_layout>
	ImageBuffer<T, TMin, TMax, rgb_to_g> reshape_templ(int height, int width) const;



protected:

	//--------------------------------
	//	ALLOCATION SETTINGS
	//--------------------------------

	static constexpr std::size_t _rows_per_block = 64;

	//--------------------------------
	//	IMAGE DEFINITION
	//--------------------------------
	
	PixelLayout _layout;
	std::size_t _height;
	std::size_t _width;

	//--------------------------------
	//	ACTUAL DATA
	//--------------------------------

	std::vector<std::unique_ptr<T[]>> _blocks;
	std::vector<T*> _row_ptrs;

}; // class ImageBuffer





//--------------------------------
//	TEMPLATE ALIASES AND EXTERN
//--------------------------------

extern template class ImageBuffer < uint8_t, 0, 255, &helper::rgb_to_g_uint8 >;
extern template class ImageBuffer<uint16_t, 0, 65535, &helper::rgb_to_g_uint16>;
extern template class ImageBuffer<uint32_t, 0, 4294967295, &helper::rgb_to_g_uint32>;
extern template class ImageBuffer<float, 0.0f, 1.0f, &helper::rgb_to_g_float>;

using ImageBuffer_uint8 = ImageBuffer < uint8_t, 0, 255, &helper::rgb_to_g_uint8 >;
using ImageBuffer_uint16 = ImageBuffer<uint16_t, 0, 65535, &helper::rgb_to_g_uint16>;
using ImageBuffer_uint32 = ImageBuffer<uint32_t, 0, 4294967295, &helper::rgb_to_g_uint32>;
using ImageBuffer_float = ImageBuffer<float, 0.0f, 1.0f, &helper::rgb_to_g_float>;




} // namespace dotscale