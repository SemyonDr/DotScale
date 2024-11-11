#pragma once

//STL
#include <algorithm>
#include <cmath>
#include <cstdint>
//Internal - other
#include "ImageBuffer_Byte.h"

class ImageGenerator
{
public:

	/// <summary>
	/// Creates an RGB image containing circular gradient with color channel shift.
	/// </summary>
	static ImageBuffer_Byte* CircularGradient(int height, int width, BitDepth bit_depth);
};

