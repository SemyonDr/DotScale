#pragma once

///<summary>
///Layout of data in each pixel of the image.
///</summary>
enum ImagePixelLayout {
	UNDEF = 0,	//Undefined
	G = 1,		//Grayscale
	GA = 2,		//Grayscale + Alpha
	RGB = 3,	//Red-Green-Blue
	RGBA = 4,	//Red-Green-Blue-Alpha
};

/// <summary>
/// Contains corresponding number of components when ImagePixelLayout used as index.
/// </summary>
const int numCmpIndex[] = {0,1,2,3,4};

/// <summary>
/// Returns number of components in particular layout.
/// </summary>
inline int NumComponentsOfLayout(ImagePixelLayout layout) {
	return numCmpIndex[static_cast<int>(layout)];
}

///<summary>
/// Bit depth of color component of an image.
///</summary>
enum BitDepth {
	BD_8_BIT = 8,
	BD_16_BIT = 16,
	BD_32_BIT = 32
};


/// <summary>
/// Gamma correction type (color profile) used when the image was created.
/// </summary>
enum RawImageGammaProfile {
	sRGB,			//Gamma correction according to sRGB standart
	PlainGamma		//Simple x^gamma correction
};


/// <summary>
/// Image file formats known to this app.
/// </summary>
enum FileFormat {
	FF_UNSUPPORTED,
	FF_JPEG,
	FF_PNG
};

