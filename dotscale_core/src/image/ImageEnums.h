#pragma once

/*
	Definitions used for describing image objects.
*/

namespace dotscale {


/// Layout of the data in an image pixel.
enum PixelLayout {
	G,		//Grayscale
	GA,		//Grayscale + Alpha
	RGB,	//Red-Green-Blue
	RGBA,	//Red-Green-Blue-Alpha
};


/// Returns the number of components in a particular layout.
constexpr int pixel_cmp_count(PixelLayout layout) {
	switch (layout) {
		case PixelLayout::G: return 1;
		case PixelLayout::GA: return 2;
		case PixelLayout::RGB: return 3;
		case PixelLayout::RGBA: return 4;
	}

	return 0;
}


/// Bit depth of a color component of a pixel.
enum class BitDepth {
	BD_8_BIT = 8,
	BD_16_BIT = 16,
	BD_32_BIT = 32
};


/// Gamma correction type (color profile) used when the image was created.
enum class GammaProfile {
	sRGB,			//Gamma correction according to sRGB standart
	PlainGamma		//Simple x^gamma correction
};


/// Image file formats known to this app.
enum FileFormat {
	Unsupported,
	JPEG,
	PNG
};


} // namespace dotscale