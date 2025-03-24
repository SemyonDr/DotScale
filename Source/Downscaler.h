#pragma once

//STL
#include <iostream>
#include <cmath>
#include <cstdint>
//Third Party
#include "oneapi/tbb.h"
//Internal
#include "FixedFraction.h"
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
	Downscaler(ImagePixelLayout layout, uint32_t src_height, uint32_t src_width, uint32_t new_height, uint32_t new_width) :
		_layout(layout),
		_src_height(src_height),
		_src_width(src_width),
		_trg_height(new_height),
		_trg_width(new_width) 
	{
		if (layout == ImagePixelLayout::UNDEF)
			throw new std::runtime_error("Downscaler init: Cannot downscale image with undefined layout.");

		_destinations_for_rows = new uint32_t[_src_height * 2];
		_destinations_for_cols = new uint32_t[_src_width * 2];
			
		_weigths_for_rows = new fxdfrc_t[_src_height * 2];
		_weigths_for_cols = new fxdfrc_t[_src_width * 2];

		_partial_row = new uint32_t[new_width * NumComponentsOfLayout(_layout)];
		ResetPartialRow();

		_frame_height = (static_cast<uint32_t>(_src_height) << 16) / new_height;
		_frame_width = (static_cast<uint32_t>(_src_width) << 16) / new_width;
		uint64_t temp = static_cast<uint64_t>(_frame_height) * static_cast<uint64_t>(_frame_width);
		if ((temp >> 32) > 65536)
			throw std::runtime_error("Downscaler: Scaling factor is too small - uint overflow is possible. Do not downscale to less than 1/256 in one step.");
		_frame_area = static_cast<uint32_t>(temp >> 16);

		FindDestinations(_destinations_for_rows, _weigths_for_rows, _src_height, new_height, _frame_height);
		FindDestinations(_destinations_for_cols, _weigths_for_cols, _src_width, new_width, _frame_width);

		// Setting state
		SetState_Start();
	}

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

	/// <summary>
	/// 
	/// </summary>
	ImageBuffer_uint16 DownscaleNext(const ImageBuffer_uint16& slice) {
		// 0) --------------------------------------------------------------------------------
		// Checking state and argument

		if (CheckStateForNext() == false)
			return ImageBuffer_uint16(0, _trg_width, _layout, false);

		if (slice.GetLayout() != _layout)
			throw new std::invalid_argument("Downscaler: Chunk layout mismatch.");

		if (slice.GetWidth() != _src_width)
			throw new std::invalid_argument("Downscaler: Chunk width mismatch.");

		if (slice.GetHeight() + _next_row_index > _src_height)
			throw new std::runtime_error("Downscaler: Chunk height exceeds expected source image size.");

		// 1) --------------------------------------------------------------------------------
		// Compressing horizontally

		ImageBuffer_uint32 hcompressed;

		// If scaling is only vertical we skip horizontal compression
		if (_src_width == _trg_width) {
			hcompressed = ImageBuffer_uint32(slice.GetHeight(), slice.GetWidth(), _layout, true);
			for (uint32_t row = 0; row < slice.GetHeight(); row++)
				for (uint32_t cmp = 0; cmp < slice.GetCmpWidth(); cmp++)
					hcompressed[row][cmp] = static_cast<uint32_t>(slice[row][cmp]);
		}
		else
			hcompressed = CompressHorizontally(slice);

		// 2) --------------------------------------------------------------------------------
		// Compressing vertically

		ImageBuffer_uint32 compressed;

		if (_src_height == _trg_height) {
			compressed = ImageBuffer_uint32(hcompressed.GetHeight(), hcompressed.GetWidth(), hcompressed.GetLayout(), true);
			for (uint32_t row = 0; row < hcompressed.GetHeight(); row++)
				for (uint32_t cmp = 0; cmp < hcompressed.GetCmpWidth(); cmp++)
					compressed[row][cmp] = hcompressed[row][cmp];
		}
		else
			compressed = CompressNextVertically(hcompressed);

		// This version is no longer needed
		hcompressed.~ImageBuffer(); 

		// 3) --------------------------------------------------------------------------------
		// Averaging

		ImageBuffer_uint16 downscaled = AverageDown(compressed);

		// Area of the frame
		uint64_t temp = static_cast<uint64_t>(_frame_height) * static_cast<uint64_t>(_frame_width);
		fxdfrc_t area = static_cast<uint32_t>(temp >> 16);

		// 4) --------------------------------------------------------------------------------
		// Advancing the state

		_next_row_index += slice.GetHeight();

		if (_state == SliceProcessorState::Ready_Start)
			SetState_Continue();

		if (_next_row_index >= _src_height)
			SetState_Finished();

		return downscaled;
	}


protected:

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	const fxdfrc_t WEIGHT_MIN = 0;
	const fxdfrc_t WEIGHT_MAX = 65536;

	ImagePixelLayout _layout;
	uint32_t _src_height = 0;
	uint32_t _src_width = 0;

	uint32_t _trg_height = 0;
	uint32_t _trg_width = 0;

	uint32_t* _destinations_for_rows = nullptr; //Destination row indices for rows
	uint32_t* _destinations_for_cols = nullptr; //Destination column indices for columns
	fxdfrc_t* _weigths_for_rows = nullptr;
	fxdfrc_t* _weigths_for_cols = nullptr;
	fxdfrc_t _frame_width = 0;
	fxdfrc_t _frame_height = 0;
	fxdfrc_t _frame_area = 0;

	uint32_t* _partial_row = nullptr;
	bool _partial_set = false;

	uint32_t _next_row_index = 0;

	//--------------------------------
	//	PRIVATE METHODS
	//--------------------------------

	ImageBuffer_uint16 AverageDown(const ImageBuffer_uint32& src_image) {
		//Alias for lambda capture
		fxdfrc_t area = _frame_area;

		// Result
		ImageBuffer_uint16 trg_image(src_image.GetHeight(), src_image.GetWidth(), src_image.GetLayout(), true);

		//Creating task for each row
		tbb::task_group tg;
		for (int src_row = 0; src_row < src_image.GetHeight(); src_row++) {
			tg.run(
				[src_row, &src_image, &trg_image, area] {
					uint32_t src_cmp_width = src_image.GetCmpWidth();
					uint64_t temp = 0;

					for (int cmp = 0; cmp < src_cmp_width; cmp++) {
						temp = (static_cast<uint64_t>(src_image[src_row][cmp]) << 32) / static_cast<uint64_t>(area);
						trg_image[src_row][cmp] = static_cast<uint16_t>(temp >> 16);
					}
				}
			);
		}
		//Waiting for tasks to finish
		tg.wait();
		
		return trg_image;
	}


	/// <summary>
	/// Compresses image brightness into a single row.
	/// For each column accumulates summary brightness of this column and ADDS the result to the corresponding pixel of given target row.
	/// </summary>
	void CompressToLine(const ImageBuffer_uint32& src_image, uint32_t* target) {
		switch (src_image.GetLayout())
		{
			case ImagePixelLayout::G: {
				//Creating task for each source column
				tbb::task_group tg;
				for (int src_px = 0; src_px < src_image.GetWidth(); src_px++) {
					tg.run(
						[src_px, &src_image, &target] {
							for (uint32_t src_row = 0; src_row < src_image.GetHeight(); src_row++) {
								// Gray
								target[src_px] += src_image[src_row][src_px];
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::GA: {
				//Creating task for each source column
				tbb::task_group tg;
				for (int src_px = 0; src_px < src_image.GetWidth(); src_px++) {
					tg.run(
						[src_px, &src_image, &target] {
							for (uint32_t src_row = 0; src_row < src_image.GetHeight(); src_row++) {
								target[src_px * 2 + 0] += src_image[src_row][src_px * 2 + 0]; // Gray
								target[src_px * 2 + 1] += src_image[src_row][src_px * 2 + 1]; // Alpha
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::RGB: {
				//Creating task for each source column
				tbb::task_group tg;
				for (int src_px = 0; src_px < src_image.GetWidth(); src_px++) {
					tg.run(
						[src_px, &src_image, &target] {
							for (uint32_t src_row = 0; src_row < src_image.GetHeight(); src_row++) {
								target[src_px * 3 + 0] += src_image[src_row][src_px * 3 + 0]; // Red
								target[src_px * 3 + 1] += src_image[src_row][src_px * 3 + 1]; // Green
								target[src_px * 3 + 2] += src_image[src_row][src_px * 3 + 2]; // Blue
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::RGBA: {
				//Creating task for each source column
				tbb::task_group tg;
				for (int src_px = 0; src_px < src_image.GetWidth(); src_px++) {
					tg.run(
						[src_px, &src_image, &target] {
							for (uint32_t src_row = 0; src_row < src_image.GetHeight(); src_row++) {
								target[src_px * 4 + 0] += src_image[src_row][src_px * 4 + 0]; // Red
								target[src_px * 4 + 1] += src_image[src_row][src_px * 4 + 1]; // Green
								target[src_px * 4 + 2] += src_image[src_row][src_px * 4 + 2]; // Blue
								target[src_px * 4 + 3] += src_image[src_row][src_px * 4 + 3]; // Alpha
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			default:
				break;
		}
	} // End: CompressToLine()




	ImageBuffer_uint32 CompressNextVertically(const ImageBuffer_uint32& src_slice) {
		// Aliases for lambda captures
		uint32_t* partial_row = _partial_row;
		uint32_t* destinations_rows = _destinations_for_rows;
		uint32_t* weights_rows = _weigths_for_rows;

		// Indices
		uint32_t src_slice_start = _next_row_index; // Row index on which this slice starts in the complete source image
		uint32_t src_slice_end = _next_row_index + src_slice.GetHeight();
		uint32_t slc_end_of_complete = 0; // End of source lines that form complete line in src_slice
		uint32_t trg_start_row = _destinations_for_rows[src_slice_start * 2 + 0];

		bool next_partial_set = false;

		// 1) --------------------------------------------------------------------------------

		// First we have to find resulting image slice height
		uint32_t trg_height = 0;
		// We count one completed row for every change in destination
		for (uint32_t dest_index = (src_slice_start * 2) + 1; dest_index < (src_slice_end * 2); dest_index++)
			if (_destinations_for_rows[dest_index - 1] != _destinations_for_rows[dest_index]) {
				trg_height++;
				slc_end_of_complete = (dest_index / 2) - src_slice_start;
			}
			
		// Checking if this slice will produce new partial row or last set of rows can be counted towards output slice
		if (src_slice_end < _src_height) {
			if (_destinations_for_rows[(src_slice_end - 1) * 2 + 1] == _destinations_for_rows[(src_slice_end) * 2 + 0])
				next_partial_set = true;
			else {
				trg_height++;
				slc_end_of_complete = src_slice.GetHeight();
			}
		}
		else { // If this slice is the last in the source image we also let last block of lines to be counted towards output
			trg_height++;
			slc_end_of_complete = src_slice.GetHeight();
		}

		// 2) --------------------------------------------------------------------------------

		// If there is not enough lines in source image to produce at least one complete line
		// we compress all lines into partial and return nothing
		if (trg_height == 0) {
			//If partial was not set we reset its content
			if (_partial_set == false) {
				ResetPartialRow();
				_partial_set = true;
			}

			CompressToLine(src_slice, _partial_row);

			// Returning empty image buffer
			return ImageBuffer_uint32(0, src_slice.GetWidth(), src_slice.GetLayout(), false);
		}

		// 3) --------------------------------------------------------------------------------

		// Now we know that there is at least one line to output
		
		// Allocating the result
		ImageBuffer_uint32 trg_image(trg_height, src_slice.GetWidth(), src_slice.GetLayout(), true);
		trg_image.SetToZero();

		// Copying contents of previously accumulated partial row
		if (_partial_set)
			for (uint32_t cmp = 0; cmp < src_slice.GetCmpWidth(); cmp++)
				trg_image[0][cmp] = _partial_row[cmp];

		//Resetting partial row
		_partial_set = next_partial_set;
		if (_partial_set)
			ResetPartialRow();


		// 4) --------------------------------------------------------------------------------

		// Compressing rows

		switch (src_slice.GetLayout())
		{
			case ImagePixelLayout::G: {
				//Creating task for each source column
				tbb::task_group tg;
				for (int col = 0; col < src_slice.GetWidth(); col++) {
					tg.run(
						[col, &src_slice, &trg_image, destinations_rows, weights_rows, src_slice_start, slc_end_of_complete, trg_start_row, partial_row, next_partial_set] {
							// Variables for loop
							uint32_t left_dest = 0;
							uint32_t right_dest = 0;
							uint32_t left_weight = 0;
							uint32_t right_weight = 0;
							uint64_t temp_left = 0;
							uint64_t temp_right = 0;

							// Compressing complete rows
							for (uint32_t slc_row = 0; slc_row < slc_end_of_complete; slc_row++) {
								left_dest = destinations_rows[(src_slice_start + slc_row) * 2 + 0] - trg_start_row;
								right_dest = destinations_rows[(src_slice_start + slc_row) * 2 + 1] - trg_start_row;
								left_weight = weights_rows[(src_slice_start + slc_row) * 2 + 0];
								right_weight = weights_rows[(src_slice_start + slc_row) * 2 + 1];

								//Gray
								temp_left = static_cast<uint64_t>(src_slice[slc_row][col]) * static_cast<uint64_t>(left_weight);
								temp_right = static_cast<uint64_t>(src_slice[slc_row][col]) * static_cast<uint64_t>(right_weight);
								trg_image[left_dest][col] += static_cast<uint32_t>(temp_left >> 16); //Left part
								trg_image[right_dest][col] += static_cast<uint32_t>(temp_right >> 16); //Right part
							}

							if (next_partial_set) {
								// Accumulating halfs of the first row after complete rows
								left_dest = destinations_rows[(src_slice_start + slc_end_of_complete) * 2 + 0] - trg_start_row;
								right_dest = destinations_rows[(src_slice_start + slc_end_of_complete) * 2 + 1] - trg_start_row;
								left_weight = weights_rows[(src_slice_start + slc_end_of_complete) * 2 + 0];
								right_weight = weights_rows[(src_slice_start + slc_end_of_complete) * 2 + 1];

								if (left_dest == right_dest) {
									partial_row[col] += src_slice[slc_end_of_complete][col]; //Gray
								}
								else {
									//Gray
									temp_left = static_cast<uint64_t>(src_slice[slc_end_of_complete][col]) * static_cast<uint64_t>(left_weight);
									temp_right = static_cast<uint64_t>(src_slice[slc_end_of_complete][col]) * static_cast<uint64_t>(right_weight);
									trg_image[left_dest][col] += static_cast<uint32_t>(temp_left >> 16); //Left part
									partial_row[col] += static_cast<uint32_t>(temp_right >> 16); //Right part
								}

								// Compressing remaining rows into partial row
								for (uint32_t slc_row = slc_end_of_complete + 1; slc_row < src_slice.GetHeight(); slc_row++) {
									partial_row[col] += src_slice[slc_row][col]; //Gray
								}
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::GA: {
				//Creating task for each source column
				tbb::task_group tg;
				for (int col = 0; col < src_slice.GetWidth(); col++) {
					tg.run(
						[col, &src_slice, &trg_image, destinations_rows, weights_rows, src_slice_start, slc_end_of_complete, trg_start_row, partial_row, next_partial_set] {
							// Variables for loop
							uint32_t left_dest = 0;
							uint32_t right_dest = 0;
							uint32_t left_weight = 0;
							uint32_t right_weight = 0;
							uint64_t temp_left = 0;
							uint64_t temp_right = 0;

							// Compressing complete rows
							for (uint32_t slc_row = 0; slc_row < slc_end_of_complete; slc_row++) {
								left_dest = destinations_rows[(src_slice_start + slc_row) * 2 + 0] - trg_start_row;
								right_dest = destinations_rows[(src_slice_start + slc_row) * 2 + 1] - trg_start_row;
								left_weight = weights_rows[(src_slice_start + slc_row) * 2 + 0];
								right_weight = weights_rows[(src_slice_start + slc_row) * 2 + 1];

								// Gray
								temp_left = static_cast<uint64_t>(src_slice[slc_row][col * 2 + 0]) * static_cast<uint64_t>(left_weight);
								temp_right = static_cast<uint64_t>(src_slice[slc_row][col * 2 + 0]) * static_cast<uint64_t>(right_weight);
								trg_image[left_dest][col * 2 + 0] += static_cast<uint32_t>(temp_left >> 16); //Left part
								trg_image[right_dest][col * 2 + 0] += static_cast<uint32_t>(temp_right >> 16); //Right part
								// Alpha
								temp_left = static_cast<uint64_t>(src_slice[slc_row][col * 2 + 1]) * static_cast<uint64_t>(left_weight);
								temp_right = static_cast<uint64_t>(src_slice[slc_row][col * 2 + 1]) * static_cast<uint64_t>(right_weight);
								trg_image[left_dest][col * 2 + 1] += static_cast<uint32_t>(temp_left >> 16); //Left part
								trg_image[right_dest][col * 2 + 1] += static_cast<uint32_t>(temp_right >> 16); //Right part
							}

							if (next_partial_set) {
								// Accumulating halfs of the first row after complete rows
								left_dest = destinations_rows[(src_slice_start + slc_end_of_complete) * 2 + 0] - trg_start_row;
								right_dest = destinations_rows[(src_slice_start + slc_end_of_complete) * 2 + 1] - trg_start_row;
								left_weight = weights_rows[(src_slice_start + slc_end_of_complete) * 2 + 0];
								right_weight = weights_rows[(src_slice_start + slc_end_of_complete) * 2 + 1];

								if (left_dest == right_dest) {
									partial_row[col * 2 + 0] += src_slice[slc_end_of_complete][col * 2 + 0]; // Gray
									partial_row[col * 2 + 1] += src_slice[slc_end_of_complete][col * 2 + 1]; // Alpha
								}
								else {
									// Gray
									temp_left = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 2 + 0]) * static_cast<uint64_t>(left_weight);
									temp_right = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 2 + 0]) * static_cast<uint64_t>(right_weight);
									trg_image[left_dest][col * 2 + 0] += static_cast<uint32_t>(temp_left >> 16); //Left part
									partial_row[col * 2 + 0] += static_cast<uint32_t>(temp_right >> 16); //Right part
									// Alpha
									temp_left = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 2 + 1]) * static_cast<uint64_t>(left_weight);
									temp_right = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 2 + 1]) * static_cast<uint64_t>(right_weight);
									trg_image[left_dest][col * 2 + 1] += static_cast<uint32_t>(temp_left >> 16); //Left part
									partial_row[col * 2 + 1] += static_cast<uint32_t>(temp_right >> 16); //Right part
								}

								// Compressing remaining rows into partial row
								for (uint32_t slc_row = slc_end_of_complete + 1; slc_row < src_slice.GetHeight(); slc_row++) {
									partial_row[col * 3 + 0] += src_slice[slc_row][col * 3 + 0]; // Gray
									partial_row[col * 3 + 1] += src_slice[slc_row][col * 3 + 1]; // Alpha
								}
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::RGB: {
				//Creating task for each source column
				tbb::task_group tg;
				for (int col = 0; col < src_slice.GetWidth(); col++) {
					tg.run(
						[col, &src_slice, &trg_image, destinations_rows, weights_rows, src_slice_start, slc_end_of_complete, trg_start_row, partial_row, next_partial_set] {
							// Variables for loop
							uint32_t left_dest = 0;
							uint32_t right_dest = 0;
							uint32_t left_weight = 0;
							uint32_t right_weight = 0;
							uint64_t temp_left = 0;
							uint64_t temp_right = 0;

							// Compressing complete rows
							for (uint32_t slc_row = 0; slc_row < slc_end_of_complete; slc_row++) {
								left_dest = destinations_rows[(src_slice_start + slc_row) * 2 + 0] - trg_start_row;
								right_dest = destinations_rows[(src_slice_start + slc_row) * 2 + 1] - trg_start_row;
								left_weight = weights_rows[(src_slice_start + slc_row) * 2 + 0];
								right_weight = weights_rows[(src_slice_start + slc_row) * 2 + 1];

								//Red
								temp_left = static_cast<uint64_t>(src_slice[slc_row][col * 3 + 0]) * static_cast<uint64_t>(left_weight);
								temp_right = static_cast<uint64_t>(src_slice[slc_row][col * 3 + 0]) * static_cast<uint64_t>(right_weight);
								trg_image[left_dest][col * 3 + 0] += static_cast<uint32_t>(temp_left >> 16); //Left part
								trg_image[right_dest][col * 3 + 0] += static_cast<uint32_t>(temp_right >> 16); //Right part
								//Green
								temp_left = static_cast<uint64_t>(src_slice[slc_row][col * 3 + 1]) * static_cast<uint64_t>(left_weight);
								temp_right = static_cast<uint64_t>(src_slice[slc_row][col * 3 + 1]) * static_cast<uint64_t>(right_weight);
								trg_image[left_dest][col * 3 + 1] += static_cast<uint32_t>(temp_left >> 16); //Left part
								trg_image[right_dest][col * 3 + 1] += static_cast<uint32_t>(temp_right >> 16); //Right part
								//Blue
								temp_left = static_cast<uint64_t>(src_slice[slc_row][col * 3 + 2]) * static_cast<uint64_t>(left_weight);
								temp_right = static_cast<uint64_t>(src_slice[slc_row][col * 3 + 2]) * static_cast<uint64_t>(right_weight);
								trg_image[left_dest][col * 3 + 2] += static_cast<uint32_t>(temp_left >> 16); //Left part
								trg_image[right_dest][col * 3 + 2] += static_cast<uint32_t>(temp_right >> 16); //Right part
							}

							if (next_partial_set) {
								// Accumulating halfs of the first row after complete rows
								left_dest = destinations_rows[(src_slice_start + slc_end_of_complete) * 2 + 0] - trg_start_row;
								right_dest = destinations_rows[(src_slice_start + slc_end_of_complete) * 2 + 1] - trg_start_row;
								left_weight = weights_rows[(src_slice_start + slc_end_of_complete) * 2 + 0];
								right_weight = weights_rows[(src_slice_start + slc_end_of_complete) * 2 + 1];

								if (left_dest == right_dest) {
									partial_row[col * 3 + 0] += src_slice[slc_end_of_complete][col * 3 + 0]; //Red
									partial_row[col * 3 + 1] += src_slice[slc_end_of_complete][col * 3 + 1]; //Green
									partial_row[col * 3 + 2] += src_slice[slc_end_of_complete][col * 3 + 2]; //Blue
								}
								else {
									//Red
									temp_left = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 3 + 0]) * static_cast<uint64_t>(left_weight);
									temp_right = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 3 + 0]) * static_cast<uint64_t>(right_weight);
									trg_image[left_dest][col * 3 + 0] += static_cast<uint32_t>(temp_left >> 16); //Left part
									partial_row[col * 3 + 0] += static_cast<uint32_t>(temp_right >> 16); //Right part
									//Green
									temp_left = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 3 + 1]) * static_cast<uint64_t>(left_weight);
									temp_right = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 3 + 1]) * static_cast<uint64_t>(right_weight);
									trg_image[left_dest][col * 3 + 1] += static_cast<uint32_t>(temp_left >> 16); //Left part
									partial_row[col * 3 + 1] += static_cast<uint32_t>(temp_right >> 16); //Right part
									//Blue
									temp_left = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 3 + 2]) * static_cast<uint64_t>(left_weight);
									temp_right = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 3 + 2]) * static_cast<uint64_t>(right_weight);
									trg_image[left_dest][col * 3 + 2] += static_cast<uint32_t>(temp_left >> 16); //Left part
									partial_row[col * 3 + 2] += static_cast<uint32_t>(temp_right >> 16); //Right part
								}

								// Compressing remaining rows into partial row
								for (uint32_t slc_row = slc_end_of_complete + 1; slc_row < src_slice.GetHeight(); slc_row++) {
									partial_row[col * 3 + 0] += src_slice[slc_row][col * 3 + 0]; //Red
									partial_row[col * 3 + 1] += src_slice[slc_row][col * 3 + 1]; //Green
									partial_row[col * 3 + 2] += src_slice[slc_row][col * 3 + 2]; //Blue
								}
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::RGBA: {
				//Creating task for each source column
				tbb::task_group tg;
				for (int col = 0; col < src_slice.GetWidth(); col++) {
					tg.run(
						[col, &src_slice, &trg_image, destinations_rows, weights_rows, src_slice_start, slc_end_of_complete, trg_start_row, partial_row, next_partial_set] {
							// Variables for loop
							uint32_t left_dest = 0;
							uint32_t right_dest = 0;
							uint32_t left_weight = 0;
							uint32_t right_weight = 0;
							uint64_t temp_left = 0;
							uint64_t temp_right = 0;

							// Compressing complete rows
							for (uint32_t slc_row = 0; slc_row < slc_end_of_complete; slc_row++) {
								left_dest = destinations_rows[(src_slice_start + slc_row) * 2 + 0] - trg_start_row;
								right_dest = destinations_rows[(src_slice_start + slc_row) * 2 + 1] - trg_start_row;
								left_weight = weights_rows[(src_slice_start + slc_row) * 2 + 0];
								right_weight = weights_rows[(src_slice_start + slc_row) * 2 + 1];

								// Red
								temp_left = static_cast<uint64_t>(src_slice[slc_row][col * 4 + 0]) * static_cast<uint64_t>(left_weight);
								temp_right = static_cast<uint64_t>(src_slice[slc_row][col * 4 + 0]) * static_cast<uint64_t>(right_weight);
								trg_image[left_dest][col * 4 + 0] += static_cast<uint32_t>(temp_left >> 16); //Left part
								trg_image[right_dest][col * 4 + 0] += static_cast<uint32_t>(temp_right >> 16); //Right part
								// Green
								temp_left = static_cast<uint64_t>(src_slice[slc_row][col * 4 + 1]) * static_cast<uint64_t>(left_weight);
								temp_right = static_cast<uint64_t>(src_slice[slc_row][col * 4 + 1]) * static_cast<uint64_t>(right_weight);
								trg_image[left_dest][col * 4 + 1] += static_cast<uint32_t>(temp_left >> 16); //Left part
								trg_image[right_dest][col * 4 + 1] += static_cast<uint32_t>(temp_right >> 16); //Right part
								// Blue
								temp_left = static_cast<uint64_t>(src_slice[slc_row][col * 4 + 2]) * static_cast<uint64_t>(left_weight);
								temp_right = static_cast<uint64_t>(src_slice[slc_row][col * 4 + 2]) * static_cast<uint64_t>(right_weight);
								trg_image[left_dest][col * 4 + 2] += static_cast<uint32_t>(temp_left >> 16); //Left part
								trg_image[right_dest][col * 4 + 2] += static_cast<uint32_t>(temp_right >> 16); //Right part
								// Alpha
								temp_left = static_cast<uint64_t>(src_slice[slc_row][col * 4 + 3]) * static_cast<uint64_t>(left_weight);
								temp_right = static_cast<uint64_t>(src_slice[slc_row][col * 4 + 3]) * static_cast<uint64_t>(right_weight);
								trg_image[left_dest][col * 4 + 3] += static_cast<uint32_t>(temp_left >> 16); //Left part
								trg_image[right_dest][col * 4 + 3] += static_cast<uint32_t>(temp_right >> 16); //Right part
							}

							if (next_partial_set) {
								// Accumulating halfs of the first row after complete rows
								left_dest = destinations_rows[(src_slice_start + slc_end_of_complete) * 2 + 0] - trg_start_row;
								right_dest = destinations_rows[(src_slice_start + slc_end_of_complete) * 2 + 1] - trg_start_row;
								left_weight = weights_rows[(src_slice_start + slc_end_of_complete) * 2 + 0];
								right_weight = weights_rows[(src_slice_start + slc_end_of_complete) * 2 + 1];

								if (left_dest == right_dest) {
									partial_row[col * 4 + 0] += src_slice[slc_end_of_complete][col * 4 + 0]; // Red
									partial_row[col * 4 + 1] += src_slice[slc_end_of_complete][col * 4 + 1]; // Green
									partial_row[col * 4 + 2] += src_slice[slc_end_of_complete][col * 4 + 2]; // Blue
									partial_row[col * 4 + 3] += src_slice[slc_end_of_complete][col * 4 + 3]; // Alpha
								}
								else {
									// Red
									temp_left = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 4 + 0]) * static_cast<uint64_t>(left_weight);
									temp_right = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 4 + 0]) * static_cast<uint64_t>(right_weight);
									trg_image[left_dest][col * 4 + 0] += static_cast<uint32_t>(temp_left >> 16); //Left part
									partial_row[col * 4 + 0] += static_cast<uint32_t>(temp_right >> 16); //Right part
									// Green
									temp_left = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 4 + 1]) * static_cast<uint64_t>(left_weight);
									temp_right = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 4 + 1]) * static_cast<uint64_t>(right_weight);
									trg_image[left_dest][col * 4 + 1] += static_cast<uint32_t>(temp_left >> 16); //Left part
									partial_row[col * 4 + 1] += static_cast<uint32_t>(temp_right >> 16); //Right part
									// Blue
									temp_left = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 4 + 2]) * static_cast<uint64_t>(left_weight);
									temp_right = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 4 + 2]) * static_cast<uint64_t>(right_weight);
									trg_image[left_dest][col * 4 + 2] += static_cast<uint32_t>(temp_left >> 16); //Left part
									partial_row[col * 4 + 2] += static_cast<uint32_t>(temp_right >> 16); //Right part
									// Alpha
									temp_left = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 4 + 3]) * static_cast<uint64_t>(left_weight);
									temp_right = static_cast<uint64_t>(src_slice[slc_end_of_complete][col * 4 + 3]) * static_cast<uint64_t>(right_weight);
									trg_image[left_dest][col * 4 + 3] += static_cast<uint32_t>(temp_left >> 16); //Left part
									partial_row[col * 4 + 3] += static_cast<uint32_t>(temp_right >> 16); //Right part
								}

								// Compressing remaining rows into partial row
								for (uint32_t slc_row = slc_end_of_complete + 1; slc_row < src_slice.GetHeight(); slc_row++) {
									partial_row[col * 4 + 0] += src_slice[slc_row][col * 4 + 0]; // Red
									partial_row[col * 4 + 1] += src_slice[slc_row][col * 4 + 1]; // Green
									partial_row[col * 4 + 2] += src_slice[slc_row][col * 4 + 2]; // Blue
									partial_row[col * 4 + 3] += src_slice[slc_row][col * 4 + 3]; // Alpha
								}
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			default:
				break;
		}

		return trg_image;
	}



	/// <summary>
	/// Compresses the image horizontally to given width.
	/// In the returned image value of each pixel is the accumulated sum of values of corresponding pixels in the same row of original image.
	/// </summary>
	/// <param name="src_image"></param>
	/// <returns></returns>
	ImageBuffer_uint32 CompressHorizontally(const ImageBuffer_uint16& src_image) {

		//Aliases
		int src_height = src_image.GetHeight();
		int src_width = src_image.GetWidth();
		ImagePixelLayout layout = src_image.GetLayout();
		uint32_t* destinations_cols = _destinations_for_cols;
		uint32_t* weights_cols = _weigths_for_cols;

		//Result
		ImageBuffer_uint32 trg_image(src_height, _trg_width, layout, true);

		//First we initialize all image values to 0
		trg_image.SetToZero();

		switch (layout)
		{
			case ImagePixelLayout::G: {
				//Creating task for each row
				tbb::task_group tg;
				for (int src_row = 0; src_row < src_height; src_row++) {
					tg.run(
						[src_row, &src_image, &trg_image, src_width, destinations_cols, weights_cols] {
							//Variables for loop
							uint32_t left_dest = 0;
							uint32_t right_dest = 0;
							uint32_t left_weight = 0;
							uint32_t right_weight = 0;

							for (int src_px = 0; src_px < src_width; src_px++) {
								left_dest = destinations_cols[src_px * 2 + 0];
								right_dest = destinations_cols[src_px * 2 + 1];
								left_weight = weights_cols[src_px * 2 + 0];
								right_weight = weights_cols[src_px * 2 + 1];
								trg_image[src_row][left_dest] += (src_image[src_row][src_px] * left_weight) >> 16; //Left part
								trg_image[src_row][right_dest] += (src_image[src_row][src_px] * right_weight) >> 16; //Right part
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::GA: {
				//Creating task for each row
				tbb::task_group tg;
				for (int src_row = 0; src_row < src_height; src_row++) {
					tg.run(
						[src_row, &src_image, &trg_image, src_width, destinations_cols, weights_cols] {
							//Variables for loop
							uint32_t left_dest = 0;
							uint32_t right_dest = 0;
							uint32_t left_weight = 0;
							uint32_t right_weight = 0;

							for (int src_px = 0; src_px < src_width; src_px++) {
								left_dest = destinations_cols[src_px * 2 + 0];
								right_dest = destinations_cols[src_px * 2 + 1];
								left_weight = weights_cols[src_px * 2 + 0];
								right_weight = weights_cols[src_px * 2 + 1];
								//Gray
								trg_image[src_row][left_dest * 2 + 0] += (src_image[src_row][src_px * 2 + 0] * left_weight) >> 16; //Left part
								trg_image[src_row][right_dest * 2 + 0] += (src_image[src_row][src_px * 2 + 0] * right_weight) >> 16; //Right part
								//Alpha
								trg_image[src_row][left_dest * 2 + 1] += (src_image[src_row][src_px * 2 + 1] * left_weight) >> 16; //Left part
								trg_image[src_row][right_dest * 2 + 1] += (src_image[src_row][src_px * 2 + 1] * right_weight) >> 16; //Right part
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::RGB: {
				//Creating task for each row
				tbb::task_group tg;
				for (int src_row = 0; src_row < src_height; src_row++) {
					tg.run(
						[src_row, &src_image, &trg_image, src_width, destinations_cols, weights_cols] {
							//Variables for loop
							uint32_t left_dest = 0;
							uint32_t right_dest = 0;
							uint32_t left_weight = 0;
							uint32_t right_weight = 0;

							for (int src_px = 0; src_px < src_width; src_px++) {
								left_dest = destinations_cols[src_px * 2 + 0];
								right_dest = destinations_cols[src_px * 2 + 1];
								left_weight = weights_cols[src_px * 2 + 0];
								right_weight = weights_cols[src_px * 2 + 1];
								//Red
								trg_image[src_row][left_dest * 3 + 0] += (src_image[src_row][src_px * 3 + 0] * left_weight) >> 16; //Left part
								trg_image[src_row][right_dest * 3 + 0] += (src_image[src_row][src_px * 3 + 0] * right_weight) >> 16; //Right part
								//Green
								trg_image[src_row][left_dest * 3 + 1] += (src_image[src_row][src_px * 3 + 1] * left_weight) >> 16; //Left part
								trg_image[src_row][right_dest * 3 + 1] += (src_image[src_row][src_px * 3 + 1] * right_weight) >> 16; //Right part
								//Blue
								trg_image[src_row][left_dest * 3 + 2] += (src_image[src_row][src_px * 3 + 2] * left_weight) >> 16; //Left part
								trg_image[src_row][right_dest * 3 + 2] += (src_image[src_row][src_px * 3 + 2] * right_weight) >> 16; //Right part
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::RGBA: {
				//Creating task for each row
				tbb::task_group tg;
				for (int src_row = 0; src_row < src_height; src_row++) {
					tg.run(
						[src_row, &src_image, &trg_image, src_width, destinations_cols, weights_cols] {
							//Variables for loop
							uint32_t left_dest = 0;
							uint32_t right_dest = 0;
							uint32_t left_weight = 0;
							uint32_t right_weight = 0;

							for (int src_px = 0; src_px < src_width; src_px++) {
								left_dest = destinations_cols[src_px * 2 + 0];
								right_dest = destinations_cols[src_px * 2 + 1];
								left_weight = weights_cols[src_px * 2 + 0];
								right_weight = weights_cols[src_px * 2 + 1];
								//Red
								trg_image[src_row][left_dest * 4 + 0] += (src_image[src_row][src_px * 4 + 0] * left_weight) >> 16; //Left part
								trg_image[src_row][right_dest * 4 + 0] += (src_image[src_row][src_px * 4 + 0] * right_weight) >> 16; //Right part
								//Green
								trg_image[src_row][left_dest * 4 + 1] += (src_image[src_row][src_px * 4 + 1] * left_weight) >> 16; //Left part
								trg_image[src_row][right_dest * 4 + 1] += (src_image[src_row][src_px * 4 + 1] * right_weight) >> 16; //Right part
								//Blue
								trg_image[src_row][left_dest * 4 + 2] += (src_image[src_row][src_px * 4 + 2] * left_weight) >> 16; //Left part
								trg_image[src_row][right_dest * 4 + 2] += (src_image[src_row][src_px * 4 + 2] * right_weight) >> 16; //Right part
								//Alpha
								trg_image[src_row][left_dest * 4 + 3] += (src_image[src_row][src_px * 4 + 3] * left_weight) >> 16; //Left part
								trg_image[src_row][right_dest * 4 + 3] += (src_image[src_row][src_px * 4 + 3] * right_weight) >> 16; //Right part
							}
						}
					);
				}
				//Waiting for tasks to finish
				tg.wait();
			}
			break;

			default:
				break;
		}

		//Result
		return trg_image;
	}



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
	void FindDestinations(uint32_t* destinations, fxdfrc_t* weights, uint32_t src_size, uint32_t new_size, fxdfrc_t frame_size) {
		/// The source row (or column) is conceptually divided into contiguous frames of fixed (possibly fractional) size.
		/// Each frame corresponds to one pixel in the downscaled image.
		/// A source pixel may be split between two frames if a frame boundary cuts through it.
		/// If a pixel is not split by a frame boundary, its left part gets a zero weight (WEIGHT_MIN) and the right part
		/// gets full weight (WEIGHT_MAX), both mapping to the same target pixel.

		uint32_t src_px = 0; // source pixels iterator
		fxdfrc_t next_frame_pos = frame_size; // Position (possibly fractional) in the source row where the next frame begins
		fxdfrc_t weight_last = WEIGHT_MIN; // Weight of the left half of the last pixel of the frame

		// Setting left half of the first source pixel
		destinations[src_px * 2 + 0] = 0;
		weights[src_px * 2 + 0] = weight_last;

		// Process each frame (each target pixel) except the last one.
		for (int trg_px = 0; trg_px < new_size; trg_px++) {
			// Right part of the first pixel of the frame
			destinations[src_px * 2 + 1] = trg_px;
			weights[src_px * 2 + 1] = WEIGHT_MAX - weight_last;
			src_px++;

			// Pixels that entirely belong to the frame
			while (src_px < (next_frame_pos >> 16)) {
				//Left part
				destinations[src_px * 2 + 0] = trg_px;
				weights[src_px * 2 + 0] = WEIGHT_MIN;
				//Right part
				destinations[src_px * 2 + 1] = trg_px;
				weights[src_px * 2 + 1] = WEIGHT_MAX;

				src_px++;
			}

			// Left part of the last pixel of the frame
			weight_last = next_frame_pos & 0x0000ffff; // Extracting fractional part
			destinations[src_px * 2 + 0] = trg_px;
			weights[src_px * 2 + 0] = weight_last;

			// Advancing the frame position to the next frame
			next_frame_pos += frame_size;
		}

		// The remaining source pixels all contribute to the last target pixel.
		int trg_px = new_size - 1;

		// Right part of the first pixel of the last frame
		destinations[src_px * 2 + 1] = trg_px;
		weights[src_px * 2 + 1] = WEIGHT_MAX - weight_last;
		src_px++;

		// Remaining source pixels
		while (src_px < src_size) {
			//Left part
			destinations[src_px * 2 + 0] = trg_px;
			weights[src_px * 2 + 0] = WEIGHT_MIN;
			//Right part
			destinations[src_px * 2 + 1] = trg_px;
			weights[src_px * 2 + 1] = WEIGHT_MAX;

			src_px++;
		}
	}




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

