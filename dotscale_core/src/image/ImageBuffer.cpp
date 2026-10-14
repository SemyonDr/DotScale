#include "ImageBuffer.h"

#include <exception>

/*
	Look to the bottom of the file to find template instantiations.
*/

namespace dotscale {

//--------------------------------
//	PROPERTIES
//--------------------------------

/// Layout of the data in an image pixel.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
PixelLayout ImageBuffer<T, TMin, TMax, rgb_to_g>
::get_layout() const noexcept { return _layout; }


/// Height of the image in pixels.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
std::size_t ImageBuffer<T, TMin, TMax, rgb_to_g>
::get_height() const noexcept { return _height; }


/// Width of the image in pixels.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
std::size_t ImageBuffer<T, TMin, TMax, rgb_to_g>
::get_width() const noexcept { return _width; }

	/// Total number of components in one image row.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
std::size_t ImageBuffer<T, TMin, TMax, rgb_to_g>
::get_cmp_width() const noexcept {
	return _width * pixel_cmp_count(_layout);
}

/// Number of components in a pixel.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
int ImageBuffer<T, TMin, TMax, rgb_to_g>
::get_num_cmp() const noexcept { return pixel_cmp_count(_layout); }


/// If this image layout has alpha channel.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
bool ImageBuffer<T, TMin, TMax, rgb_to_g>
::has_alpha() const noexcept { return layout_has_alpha(_layout); }


/// Rows per continuous memory block.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
std::size_t ImageBuffer<T, TMin, TMax, rgb_to_g>
::get_rows_per_block() const noexcept { return _rows_per_block; }



//--------------------------------
//	ACCESSORS
//--------------------------------

///Image data.
///Array of rows of pixel samples that ordered according to image layout.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
T** ImageBuffer<T, TMin, TMax, rgb_to_g>
::get_data_ptr() const noexcept { return _row_ptrs.data(); }



//--------------------------------
//	METHODS
//--------------------------------

/// Sets all pixels to minimum value.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
void ImageBuffer<T, TMin, TMax, rgb_to_g>
::set_zero() {
	for (auto row : _row_ptrs)
		for (int cmp = 0; cmp < get_cmp_width(); cmp++)
			row[cmp] = TMin;
}


/// Makes a copy of this image buffer.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
ImageBuffer<T, TMin, TMax, rgb_to_g> ImageBuffer<T, TMin, TMax, rgb_to_g>
::clone() const {
	ImageBuffer<T, TMin, TMax, rgb_to_g> copy(_height, _width, _layout);

	T** cpy_data = copy.get_data_ptr();
	T** this_data = this->get_data_ptr();

	for (int row = 0; row < _height; row++)
		for (int cmp = 0; cmp < get_cmp_width(); cmp++)
			cpy_data[row][cmp] = this_data[row][cmp];

	return copy;
}


/// Returns a copy of this image with alpha channel removed.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
ImageBuffer<T, TMin, TMax, rgb_to_g> ImageBuffer<T, TMin, TMax, rgb_to_g>
::remove_alpha() const {

	// For layouts that do not contain alpha channel we return a copy.
	if (!has_alpha())
		return clone();

	// Making a copy without alpha
	switch (_layout)
	{
		case PixelLayout::GA: {
			ImageBuffer<T, TMin, TMax, rgb_to_g> trg_image(_height, _width, PixelLayout::G);

			for (int row = 0; row < _height; row++) {
				T* trg_row = trg_image[row];
				T* src_row = _row_ptrs[row];
				for (int px = 0; px < _width; px++)
					trg_row[px] = src_row[px * 2];
			}

			return trg_image;
		}

		case PixelLayout::RGBA: {
			ImageBuffer<T, TMin, TMax, rgb_to_g> trg_image(_height, _width, PixelLayout::RGB);

			for (int row = 0; row < _height; row++) {
				T* trg_row = trg_image[row];
				T* src_row = _row_ptrs[row];
				for (int px = 0; px < _width; px++) {
					trg_row[px * 3 + 0] = src_row[px * 4 + 0];
					trg_row[px * 3 + 1] = src_row[px * 4 + 1];
					trg_row[px * 3 + 2] = src_row[px * 4 + 2];
				}
			}

			return trg_image;
		}
	}
}


/// Templated implementation of the reshape() method.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
template<PixelLayout src_layout, PixelLayout trg_layout>
ImageBuffer<T, TMin, TMax, rgb_to_g> ImageBuffer<T, TMin, TMax, rgb_to_g>
::reshape_templ(int height, int width) const {

	ImageBuffer<T, TMin, TMax, rgb_to_g> trg_image(height, width, layout);

	// Layout details
	constexpr int src_cmp_count = pixel_cmp_count(src_layout);
	constexpr int trg_cmp_count = pixel_cmp_count(trg_layout);
	constexpr bool src_alpha = layout_has_alpha(src_layout);
	constexpr bool trg_alpha = layout_has_alpha(trg_layout);
	constexpr bool src_gray = layout_is_gray(src_layout);
	constexpr bool trg_gray = layout_is_gray(trg_layout);

	//Aliases
	int src_height = this->_height;
	int src_width = this->_width;
	int trg_height = height;
	int trg_width = width;

	for (int row = 0; row < std::min(src_height, trg_height); row++) {
		T* src_row = this->_row_ptrs[row];
		T* trg_row = trg_image.get_data_ptr[row];
		// Step one - copy existing columns of each row
		for (int px = 0; px < std::min(src_width, trg_width); px++) {
			// Color
			if constexpr (trg_gray) {
				if constexpr (src_gray)
					trg_row[px * trg_cmp_count] = src_row[px * src_cmp_count];
				else
					trg_row[px * trg_cmp_count] = rgb_to_g(src_row[px * src_cmp_count + 0], src_row[px * src_cmp_count + 1], src_row[px * src_cmp_count + 2]);
			}
			else {
				if constexpr (src_gray) {
					trg_row[px * trg_cmp_count + 0] = src_row[px * src_cmp_count];
					trg_row[px * trg_cmp_count + 1] = src_row[px * src_cmp_count];
					trg_row[px * trg_cmp_count + 2] = src_row[px * src_cmp_count];
				}
				else {
					trg_row[px * trg_cmp_count + 0] = src_row[px * src_cmp_count + 0];
					trg_row[px * trg_cmp_count + 1] = src_row[px * src_cmp_count + 1];
					trg_row[px * trg_cmp_count + 2] = src_row[px * src_cmp_count + 2];
				}
			}
			// Alpha channel
			if constexpr (trg_alpha) {
				if constexpr (src_alpha)
					trg_row[(px + 1) * trg_cmp_count - 1] = src_row[(px + 1) * src_cmp_count - 1];
				else
					trg_row[(px + 1) * trg_cmp_count - 1] = TMax;
			}
			// else -- We maybe can blend alpha instead of dropping it, but this needs to be decided...
		}
		// Step two - fill added columns with zeroes
		for (int px = copy_width; px < trg_width; px++) {
			if constexpr (trg_gray)
				trg_data[row][px * trg_cmp_count + 0] = TMin;
			else {
				trg_row[px * trg_cmp_count + 0] = TMin;
				trg_row[px * trg_cmp_count + 1] = TMin;
				trg_row[px * trg_cmp_count + 2] = TMin;
			}
			if constexpr (trg_alpha)
				trg_row[(px + 1) * trg_cmp_count - 1] = TMax;
		}
	}
	// Step three - fill added rows with zeroes
	for (int row = copy_height; row < trg_height; row++) {
		for (int px = 0; px < trg_width; px++) {
			if constexpr (trg_gray)
				trg_row[row][px * trg_cmp_count + 0] = TMin;
			else {
				trg_row[px * trg_cmp_count + 0] = TMin;
				trg_row[px * trg_cmp_count + 1] = TMin;
				trg_row[px * trg_cmp_count + 2] = TMin;
			}
			if constexpr (trg_alpha)
				trg_row[(px + 1) * trg_cmp_count - 1] = TMax;
		}
	}

	return trg_image;
}


/// Transforms the layout and size of the image to match provided size and layout.
/// Size is transformed by trimming/expanding with black. No pixel manipulation occurs.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
ImageBuffer<T, TMin, TMax, rgb_to_g> ImageBuffer<T, TMin, TMax, rgb_to_g>
::reshape(int height, int width, PixelLayout layout) const {
	PixelLayout src_layout = _layout;
	PixelLayout trg_layout = layout;

	// Template instantiations
	switch (src_layout) {
		case (PixelLayout::G):
			switch (trg_layout) {
				case (PixelLayout::G):
					return reshape_templ< PixelLayout::G, PixelLayout::G>(height, width);
				case (PixelLayout::GA):
					return reshape_templ< PixelLayout::G, PixelLayout::GA>(height, width);
				case (PixelLayout::RGB):
					return reshape_templ< PixelLayout::G, PixelLayout::RGB>(height, width);
				case (PixelLayout::RGBA):
					return reshape_templ< PixelLayout::G, PixelLayout::RGBA>(height, width);
			}
		case (PixelLayout::GA):
			switch (trg_layout) {
				case (PixelLayout::G):
					return reshape_templ< PixelLayout::GA, PixelLayout::G>(height, width);
				case (PixelLayout::GA):
					return reshape_templ< PixelLayout::GA, PixelLayout::GA>(height, width);
				case (PixelLayout::RGB):
					return reshape_templ< PixelLayout::GA, PixelLayout::RGB>(height, width);
				case (PixelLayout::RGBA):
					return reshape_templ< PixelLayout::GA, PixelLayout::RGBA>(height, width);
			}
		case (PixelLayout::RGB):
			switch (trg_layout) {
				case (PixelLayout::G):
					return reshape_templ< PixelLayout::RGB, PixelLayout::G>(height, width);
				case (PixelLayout::GA):
					return reshape_templ< PixelLayout::RGB, PixelLayout::GA>(height, width);
				case (PixelLayout::RGB):
					return reshape_templ< PixelLayout::RGB, PixelLayout::RGB>(height, width);
				case (PixelLayout::RGBA):
					return reshape_templ< PixelLayout::RGB, PixelLayout::RGBA>(height, width);
			}
		case (PixelLayout::RGBA):
			switch (trg_layout) {
				case (PixelLayout::G):
					return reshape_templ< PixelLayout::RGBA, PixelLayout::G>(height, width);
				case (PixelLayout::GA):
					return reshape_templ< PixelLayout::RGBA, PixelLayout::GA>(height, width);
				case (PixelLayout::RGB):
					return reshape_templ< PixelLayout::RGBA, PixelLayout::RGB>(height, width);
				case (PixelLayout::RGBA):
					return reshape_templ< PixelLayout::RGBA, PixelLayout::RGBA>(height, width);
			}
	}
	std::unreachable();

}


/// Inserts lines of an image into this buffer starting at a specified line.
/// Widths and layouts must match.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
void ImageBuffer<T, TMin, TMax, rgb_to_g>
::insert_at(ImageBuffer<T, TMin, TMax, rgb_to_g>& image, int line) {

	// Checks
	if (this->_width != image._width || this->_layout != image._layout)
		throw new std::exception("Trying to combine images with mismatching shapes.");
	if (line > _heigth || line < 0)
		throw new std::exception("Insertion line index out of range.");

	// Allocating new blocks to accomodate inserted image and copying data
	int num_blocks = (image._height + _rows_per_block - 1) / _rows_per_block;
	std::vector<unique_ptr<T[]>> new_blocks(num_blocks);
	std::vector<T*> new_line_ptrs(image._height);
	std::size_t cmp_width = get_cmp_width();
	for (int i = 0, row = 0; i < image._height; i++) {
		std::size_t block_height = std::min(_rows_per_block, image._height - row);
		new_blocks[i] = std::make_unique_for_overwrite<T[]>( cmp_width * block_height);

		T* block_ptr = new_blocks[i].get();
		for (int blrow = 0; blrow < block_height; blrow++, row++) {
			T* row_ptr = block_ptr + blrow * cmp_width;
			new_line_ptrs[row] = row_ptr;
			T* src_row_ptr = image[row];
			for (int cmp = 0; cmp < cmp_width; cmp++)
				row_ptr[cmp] = src_row_ptr[cmp];
		}
	}

	// Combining data arrays
	if (line == 0)
		_blocks.insert(
			_blocks.begin(),
			std::make_move_iterator(new_blocks.begin()),
			std::make_move_iterator(new_blocks.end()));
	else
		_blocks.insert(
			_blocks.end(),
			std::make_move_iterator(new_blocks.begin()),
			std::make_move_iterator(new_blocks.end()));
	new_blocks.clear();

	// Combining line pointers
	_row_ptrs.reserve(_row_ptrs.size() + new_line_ptrs.size());
	_row_ptrs.insert(
		_row_ptrs.begin() + line,
		new_line_ptrs.begin(),
		new_line_ptrs.end());
}


/// Inserts given image into this image starting at the first line.
/// Widths and layouts must match.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
void ImageBuffer<T, TMin, TMax, rgb_to_g>
::prepend(const ImageBuffer<T, TMin, TMax, rgb_to_g>& image) {
	InsertAtLine(image, 0);
}


/// Inserts given image into this image starting right after the last line.
/// Widths and layouts must match.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
void ImageBuffer<T, TMin, TMax, rgb_to_g>
::append(const ImageBuffer<T, TMin, TMax, rgb_to_g>& image) {
	InsertAtLine(image, _height);
}


/// Returns Image Buffer that contains `height` rows of this buffer, starting from given line.
template <typename T, T TMin, T TMax, T(*rgb_to_g)(const T, const T, const T)>
ImageBuffer<T, TMin, TMax, rgb_to_g> ImageBuffer<T, TMin, TMax, rgb_to_g>
::get_slice(int start, int height) const {
	if (start >= _height)
		throw std::exception("Slice starting line is out of range.");
	if (start + height > _height)
		throw std::exception("Slice height is too large.");

	ImageBuffer<T, TMin, TMax, rgb_to_g> trg_image(height, _width, _layout);

	for (uint32_t src_row = start, trg_row = 0; src_row < start + height; src_row++, trg_row++) {
		T* src_row_ptr = _row_ptrs[src_row];
		T* trg_row_ptr = trg_image[trg_row];
		for (uint32_t cmp = 0; cmp < get_cmp_width(); cmp++)
			trg_row_ptr[cmp] = src_row_ptr[cmp];
	}

	return trg_image;
}



//--------------------------------
//	CONSTRUCTORS
//--------------------------------

/// Creates an empty image buffer with given dimensions and layout.
/// Image content is not initialized.
template <typename T, T TMin, T TMax, T (*rgb_to_g)(const T, const T, const T)> 
ImageBuffer<T, TMin, TMax, rgb_to_g>
::ImageBuffer(int height, int width, PixelLayout layout)
		: _height(height), _width(width), _layout(layout) {

	// We allocate continuos blocks of rows of pre-set size.
	const int num_blocks = (_height + _rows_per_block - 1) / _rows_per_block;

	_blocks = std::vector<std::unique_ptr<T[]>>(num_blocks);
	_row_ptrs = std::vector<T*>(_heigth);

	const int cmp_width = get_cmp_width();
	for (int i = 0, row = 0; i < height; i++) {
		std::size_t block_height = std::min(_rows_per_block, _height - row);
		_blocks[i] = std::make_unique_for_overwrite<T[]>(cmp_width * block_height);
		
		T* block_ptr = _blocks[i].get();
		for (int blrow = 0; blrow < block_height; blrow++, row++)
			_row_ptrs[row] = block_ptr + blrow * cmp_width;
	}		
}







//--------------------------------
//	TEMPLATE FUNCTIONS
//--------------------------------

/// Averages 3 color components of a pixel.
uint8_t rgb_to_g_uint8(const uint8_t red, const uint8_t green, const uint8_t blue) {
	uint16_t avg = static_cast<uint16_t>(red);
	avg += static_cast<uint16_t>(green);
	avg += static_cast<uint16_t>(blue);
	return static_cast<uint8_t>(avg / 3);
}


/// Averages 3 color components of a pixel.
uint16_t rgb_to_g_uint16(const uint16_t red, const uint16_t green, const uint16_t blue) {
	uint32_t avg = static_cast<uint32_t>(red);
	avg += static_cast<uint32_t>(green);
	avg += static_cast<uint32_t>(blue);
	return static_cast<uint16_t>(avg / 3);
}


/// Averages 3 color components of a pixel.
uint32_t rgb_to_g_uint32(const uint32_t red, const uint32_t green, const uint32_t blue) {
	uint64_t avg = static_cast<uint64_t>(red);
	avg += static_cast<uint64_t>(green);
	avg += static_cast<uint64_t>(blue);
	return static_cast<uint32_t>(avg / 3);
}


/// Averages 3 color components of a pixel.
float rgb_to_g_float(const float red, const float green, const float blue) {
	float avg = red;
	avg += green;
	avg += blue;
	return avg / 3.0f;
}


//--------------------------------
//	TEMPLATE INSTANTIATION
//--------------------------------

template class ImageBuffer < uint8_t, 0, 255, &rgb_to_g_uint8 >;
template class ImageBuffer<uint16_t, 0, 65535, &rgb_to_g_uint16>;
template class ImageBuffer<uint32_t, 0, 4294967295, &rgb_to_g_uint32>;
template class ImageBuffer<float, 0.0f, 1.0f, &rgb_to_g_float>;


} // namespace dotscale


