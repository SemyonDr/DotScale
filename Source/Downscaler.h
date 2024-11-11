#pragma once

//STL
#include <cstdint>
//Internal
#include "ImageBufferInfo.h"

/// <summary>
/// Object that performs downscaling.
/// </summary>
class Downscaler
{


protected:
	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	int* _destinations_rows = nullptr;
	int* _destinations_cols = nullptr;
	uint32_t* _weigths_rows = nullptr;
	uint32_t* _weigths_cols = nullptr;
	double _frame_size_hor = 0.0;
	double _frame_size_vert = 0.0;

	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------

	/// <summary>
	/// Builds new Downscaler object for specified image and scaling.
	/// </summary>
	Downscaler(ImageBufferInfo image_info, int new_height, int new_width) {
		
		
	
	}

};

