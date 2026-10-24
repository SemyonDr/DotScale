#pragma once

/*
	Definitions used for describing image objects.
*/

#include <utility>

namespace dotscale {


/// Layout of the data in an image pixel.
enum class PixelLayout {
	G,		//Grayscale
	GA,		//Grayscale + Alpha
	RGB,	//Red-Green-Blue
	RGBA,	//Red-Green-Blue-Alpha
};

/// Bit depth of a color component of a pixel.
enum class BitDepth {
	BD_8_BIT,
	BD_16_BIT,
	BD_32_BIT
};


/// Gamma correction type (color profile) used when the image was created.
enum class GammaProfile {
	sRGB,			//Gamma correction according to sRGB standart
	PlainGamma		//Simple x^gamma correction
};


/// Image file formats known to this app.
enum class FileFormat {
	Unsupported,
	JPEG,
	PNG
};


namespace util {

/// Returns the number of components in a particular layout.
	constexpr int pixel_cmp_count(PixelLayout layout) {
		switch (layout) {
			case PixelLayout::G: return 1;
			case PixelLayout::GA: return 2;
			case PixelLayout::RGB: return 3;
			case PixelLayout::RGBA: return 4;
		}

		std::unreachable(); // c++23
	}

	/// Checks if given pixel layout has alpha channel
	constexpr bool layout_has_alpha(PixelLayout layout) {
		switch (layout) {
			case PixelLayout::G: return false;
			case PixelLayout::GA: return true;
			case PixelLayout::RGB: return false;
			case PixelLayout::RGBA: return true;
		}

		std::unreachable(); // c++23
	}

	/// Says if a layout is grayscale.
	constexpr bool layout_is_gray(PixelLayout layout) {
		switch (layout) {
			case PixelLayout::G: return true;
			case PixelLayout::GA: return true;
			case PixelLayout::RGB: return false;
			case PixelLayout::RGBA: return false;
		}

		std::unreachable(); // c++23
	}
} // namespace util


} // namespace dotscale