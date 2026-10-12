#pragma once

//STL
#include <iostream>
#include <cmath>
#include <cstdint>
//Third Party
#include "oneapi/tbb.h"
//Internal
#include "FixedMath.h"
#include "SliceProcessor.h"
#include "ImageBuffer.h"
#include "ImageBufferInfo.h"
//Debug (to delete)





/// <summary>
/// Object that performs downscaling.
/// </summary>
class Downscaler : SliceProcessor {
public:
	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------

	/// <summary>
	/// Builds new Downscaler object for specified image and scaling.
	/// Source width and height should be less than 2^16 = 65536 (not inclusive).
	/// New height and width should be less or equal to the old ones.
	/// </summary>
	Downscaler(ImagePixelLayout layout, uint32_t src_height, uint32_t src_width, uint32_t new_height, uint32_t new_width);

	/// <summary>
	/// Destructor.
	/// </summary>
	~Downscaler() {
		delete[] _destinations_for_cols;
		delete[] _destinations_for_rows;
		delete[] _weigths_for_cols;
		delete[] _weigths_for_rows;
		delete[] _partial_row;
	}


	//--------------------------------
	//	PUBLIC METHODS
	//--------------------------------

	/// <summary>
	/// Processes next slice of the image.
	/// </summary>
	ImageBuffer_uint16 DownscaleNext(const ImageBuffer_uint16& slice);


	/// <summary>
	/// Downscales the image.
	/// </summary>
	static ImageBuffer_uint16 Downscale(const ImageBuffer_uint16& image, uint32_t new_height, uint32_t new_width) {
		ImagePixelLayout layout = image.GetLayout();
		Downscaler scaler(image.GetLayout(), image.GetHeight(), image.GetWidth(), new_height, new_width);
		return scaler.DownscaleNext(image);
	}

protected:

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	const ufxd32_16_t WEIGHT_MIN = 0;
	const ufxd32_16_t WEIGHT_MAX = 65536;

	ImagePixelLayout _layout;
	uint32_t _src_height = 0;
	uint32_t _src_width = 0;

	uint32_t _trg_height = 0;
	uint32_t _trg_width = 0;

	uint32_t* _destinations_for_rows = nullptr; //Destination row indices for rows
	uint32_t* _destinations_for_cols = nullptr; //Destination column indices for columns
	ufxd32_16_t* _weigths_for_rows = nullptr;
	ufxd32_16_t* _weigths_for_cols = nullptr;
	ufxd32_16_t _frame_width = 0;
	ufxd32_16_t _frame_height = 0;
	ufxd32_16_t _frame_area = 0;

	uint32_t* _partial_row = nullptr;
	bool _partial_set = false;

	uint32_t _next_row_index = 0;

	//--------------------------------
	//	PRIVATE METHODS
	//--------------------------------

	/// <summary>
	/// Take in accumulator buffer and uses frame area to produces averaged values for pixels.
	/// </summary>
	ImageBuffer_uint16 AverageDown(const ImageBuffer_uint32& src_image);


	/// <summary>
	/// Compresses image brightness into a single row.
	/// For each column accumulates summary brightness of this column and ADDS the result to the corresponding pixel of given target row.
	/// </summary>
	void CompressToLine(const ImageBuffer_uint32& src_image, uint32_t* target);



	/// <summary>
	/// Compresses next sequential slice vertically. Produces buffer of accumulated values.
	/// </summary>
	ImageBuffer_uint32 CompressNextVertically(const ImageBuffer_uint32& src_slice);



	/// <summary>
	/// Compresses the image horizontally to given width.
	/// In the returned image value of each pixel is the accumulated sum of values of corresponding pixels in the same row of original image.
	/// </summary>
	/// <param name="src_image"></param>
	/// <returns></returns>
	ImageBuffer_uint32 CompressHorizontally(const ImageBuffer_uint16& src_image);



	//--------------------------------
	//	INIT
	//--------------------------------

	/// <summary>
	/// Precompute the mapping of source pixels to target pixels for downscaling.
	/// 
	/// For every source pixel, this function computes two entries (for its left and right parts):
	/// - The target pixel index that the part contributes to.
	/// - The fractional weight (in fixed-point format) of that contribution.
	/// </summary>
	/// <param name="destinations">Array (length = src_size * 2) to store the target pixel index for each part.</param>
	/// <param name="weights">Array (length = src_size * 2) to store the fixed-point weight for each part.</param>
	/// <param name="src_size">Number of pixels in the source row.</param>
	/// <param name="new_size">Number of pixels in the downscaled (target) row.</param>
	/// <param name="frame_size">Size of each frame (in fixed-point format), representing one target pixel.</param>
	void FindDestinations(uint32_t* destinations, ufxd32_16_t* weights, uint32_t src_size, uint32_t new_size, ufxd32_16_t frame_size);




	//--------------------------------
	//	UTILITY
	//--------------------------------

	/// <summary>
	/// Sets values stored in partial row to 0.
	/// </summary>
	void ResetPartialRow() {
		uint32_t cmp_width = _trg_width * NumComponentsOfLayout(_layout);
		for (uint32_t cmp = 0; cmp < cmp_width; cmp++)
			_partial_row[cmp] = 0;
	}

};

