#include "GammaConverter.h"


//--------------------------------
//	PUBLIC CONVERSION METHODS
//--------------------------------



/// <summary>
/// Converts image with linear scale brightness values [0..65535] to gamma-corrected color space.
/// Resulting image bit depth is specified in bitDepth argument.
/// </summary>
ImageBuffer_Byte GammaConverter::ApplyGammaCorrection(const ImageBuffer_uint16& linear_image, BitDepth bitDepth) {
	//Aliases
	int image_height = linear_image.GetHeight();
	int image_width = linear_image.GetWidth();
	uint16_t** linear_data = linear_image.GetDataPtr();

	
	//Allocating resulting image
	ImageBuffer_Byte image(image_height, image_width, linear_image.GetLayout(), bitDepth);
	uint8_t** data = image.GetDataPtr(); //Alias

	//Depending on bit depth
	if (bitDepth == BitDepth::BD_8_BIT) { //8 bit
		//Checking if conversion table is initialized
		if (_is_table_toGamma_8bit_initialized == false)
			InitializeTable_ToGamma_8bit();

		switch (linear_image.GetLayout()) {
			case ImagePixelLayout::G: {
				//Running threads processing rows
				tbb::task_group tg;
				for (int row = 0; row < image_height; row++) {
					tg.run(
						[&, row] {
							for (int px = 0; px < image_width; px++) { //Iterating row pixels
								//Retrieving converted value
								data[row][px] = _table_toGamma_8bit[linear_data[row][px]];
							}
						}
					); //tg.run
				}
				//Waiting for threads to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::GA: {
				//Running threads processing rows
				tbb::task_group tg;
				for (int row = 0; row < image_height; row++) {
					tg.run(
						[&, row] {
							for (int px = 0; px < image_width; px++) { //Iterating row pixels
								data[row][px * 2 + 0] = _table_toGamma_8bit[linear_data[row][px * 2 + 0]];
								data[row][px * 2 + 1] = static_cast<uint8_t>(linear_data[row][px * 2 + 1] / 257); //Alpha is scaled down
							}
						}
					); //tg.run
				}
				//Waiting for threads to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::RGB: {
				//Running threads processing rows
				tbb::task_group tg;
				for (int row = 0; row < image_height; row++) {
					tg.run<>(
						[&, row] {
							//Iterating row pixels				
							for (int px = 0; px < image_width; px++) {
								data[row][px * 3 + 0] = _table_toGamma_8bit[linear_data[row][px * 3 + 0]];
								data[row][px * 3 + 1] = _table_toGamma_8bit[linear_data[row][px * 3 + 1]];
								data[row][px * 3 + 2] = _table_toGamma_8bit[linear_data[row][px * 3 + 2]];
							}
						}
					);
				}
				//Waiting for threads to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::RGBA: {
				//Running threads processing rows
				tbb::task_group tg;
				for (int row = 0; row < image_height; row++) {
					tg.run<>(
						[&, row] {
							//Iterating row pixels
							for (int px = 0; px < image_width; px++) {
								data[row][px * 4 + 0] = _table_toGamma_8bit[linear_data[row][px * 4 + 0]];
								data[row][px * 4 + 1] = _table_toGamma_8bit[linear_data[row][px * 4 + 1]];
								data[row][px * 4 + 2] = _table_toGamma_8bit[linear_data[row][px * 4 + 2]];
								data[row][px * 4 + 3] = static_cast<uint8_t>(linear_data[row][px * 4 + 3] / 257); //Alpha is scaled down
							}
						}
					);
				}
				//Waiting for threads to finish
				tg.wait();
			}
			break;

			default:
				break;
		}
	}
	else { //16 bit
		//Checking if conversion table is initialized
		if (_is_table_toGamma_16bit_initialized == false)
			InitializeTable_ToGamma_16bit();

		uint16_t** data_16 = reinterpret_cast<uint16_t**>(data); //Alias for output data array allows to write 16-bit values

		switch (linear_image.GetLayout())
		{

			case ImagePixelLayout::G: {
				//Running threads processing rows
				tbb::task_group tg;
				for (int row = 0; row < image_height; row++) {
					tg.run<>(
						[&, row] {
							for (int px = 0; px < image_width; px++) { //Iterating row pixels
								data_16[row][px] = _table_toGamma_16bit[linear_data[row][px]];
							}
						}
					);
				}
				//Waiting for threads to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::GA: {
				//Running threads processing rows
				tbb::task_group tg;
				for (int row = 0; row < image_height; row++) {
					tg.run<>(
						[&, row] {
							for (int px = 0; px < image_width; px++) { //Iterating row pixels
								data_16[row][px * 2 + 0] = _table_toGamma_16bit[linear_data[row][px * 2 + 0]];
								data_16[row][px * 2 + 1] = linear_data[row][px * 2 + 1]; //Alpha is copied
							}
						}
					);
				}
				//Waiting for threads to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::RGB: {
				//Running threads processing rows
				tbb::task_group tg;
				for (int row = 0; row < image_height; row++) {
					tg.run<>(
						[&, row] {
							for (int px = 0; px < image_width; px++) { //Iterating row pixels
								data_16[row][px * 3 + 0] = _table_toGamma_16bit[linear_data[row][px * 3 + 0]];
								data_16[row][px * 3 + 1] = _table_toGamma_16bit[linear_data[row][px * 3 + 1]];
								data_16[row][px * 3 + 2] = _table_toGamma_16bit[linear_data[row][px * 3 + 2]];
							}
						}
					);
				}
				//Waiting for threads to finish
				tg.wait();
			}
			break;

			case ImagePixelLayout::RGBA: {
				//Running threads processing rows
				tbb::task_group tg;
				for (int row = 0; row < image_height; row++) {
					tg.run<>(
						[&, row] {
							for (int px = 0; px < image_width; px++) { //Iterating row pixels
								data_16[row][px * 4 + 0] = _table_toGamma_16bit[linear_data[row][px * 4 + 0]];
								data_16[row][px * 4 + 1] = _table_toGamma_16bit[linear_data[row][px * 4 + 1]];
								data_16[row][px * 4 + 2] = _table_toGamma_16bit[linear_data[row][px * 4 + 2]];
								data_16[row][px * 4 + 3] = _table_toGamma_16bit[linear_data[row][px * 4 + 3]];
							}
						}
					);
				}
				//Waiting for threads to finish
				tg.wait();
			}
			break;

			default:
				break;
		}
	}

	return image;
}



/// <summary>
/// Converts image brightness values from gamma-corrected color space to brightness linear scale 16 bit [0..65535].
/// </summary>
ImageBuffer_uint16 GammaConverter::RemoveGammaCorrection(const ImageBuffer_Byte& image) {
	//Aliases
	int image_height = image.GetHeight();
	int image_width = image.GetWidth();
	uint8_t** data = image.GetDataPtr();

	//Allocating resulting image
	ImageBuffer_uint16 linear_image(image_height, image_width, image.GetLayout());
	uint16_t** linear_data = linear_image.GetDataPtr(); //Alias

	//Depending on bit depth
	if (image.GetBitPerComponent() == BitDepth::BD_8_BIT) { //8 bit
		//Checking if conversion table is initialized
		if (_is_table_toLinear_8bit_initialized == false)
			InitializeTable_ToLinear_8bit();

		switch (image.GetLayout())
		{
		case ImagePixelLayout::G: {
			//Running thread processing rows
			//Task for each row
			tbb::task_group tg;
			for (int row = 0; row < image_height; row++) {
				tg.run(
					[&, row] {
						for (int px = 0; px < image_width; px++) { //Iterating row pixels
							linear_data[row][px] = _table_toLinear_8bit[data[row][px]];
						}
					}
				);
			}
			//Waiting for tasks to finish
			tg.wait();
		}
		break;

		case ImagePixelLayout::GA: {
			//Running threads processing rows
			tbb::task_group tg;
			for (int row = 0; row < image_height; row++) {
				tg.run(
					[&, row] {
						for (int px = 0; px < image_width; px++) { //Iterating row pixels
							linear_data[row][px * 2 + 0] = _table_toLinear_8bit[data[row][px * 2 + 0]]; //Gray
							linear_data[row][px * 2 + 1] = data[row][px * 2 + 1] * 257; //Alpha channel is scaled
						}
					}
				);
			}
			//Waiting for tasks to finish
			tg.wait();
		}
		break;

		case ImagePixelLayout::RGB: {
			//Running threads processing rows
			tbb::task_group tg;
			for (int row = 0; row < image_height; row++) {
				tg.run(
					[&, row] {
						for (int px = 0; px < image_width; px++) { //Iterating row pixels
							linear_data[row][px * 3 + 0] = _table_toLinear_8bit[data[row][px * 3 + 0]]; //Red
							linear_data[row][px * 3 + 1] = _table_toLinear_8bit[data[row][px * 3 + 1]]; //Green
							linear_data[row][px * 3 + 2] = _table_toLinear_8bit[data[row][px * 3 + 2]]; //Blue
						}
					}
				);
			}
			//Waiting for tasks to finish
			tg.wait();
		}
		break;

		case ImagePixelLayout::RGBA: {
			//Running threads processing rows
			tbb::task_group tg;
			for (int row = 0; row < image_height; row++) {
				tg.run(
					[&, row] {
						for (int px = 0; px < image_width; px++) { //Iterating row pixels
							linear_data[row][px * 4 + 0] = _table_toLinear_8bit[data[row][px * 4 + 0]]; //Red
							linear_data[row][px * 4 + 1] = _table_toLinear_8bit[data[row][px * 4 + 1]]; //Green
							linear_data[row][px * 4 + 2] = _table_toLinear_8bit[data[row][px * 4 + 2]]; //Blue
							linear_data[row][px * 4 + 3] = data[row][px * 4 + 3] * 257; //Alpha is scaled
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
	}
	else { //16 bit
		//Checking if conversion table is initialized
		if (_is_table_toLinear_16bit_initialized == false)
			InitializeTable_ToLinear_16bit();

		uint16_t** data_16 = reinterpret_cast<uint16_t**>(data);

		switch (image.GetLayout())
		{
		case ImagePixelLayout::G: {
			//Running threads processing rows
			tbb::task_group tg;
			for (int row = 0; row < image_height; row++) {
				tg.run(
					[&, row] {
						for (int px = 0; px < image_width; px++) { //Iterating row pixels
							linear_data[row][px] = _table_toLinear_16bit[data_16[row][px]];	//Gray
						}
					}
				);
			}
			//Waiting for tasks to finish
			tg.wait();
		}
		break;

		case ImagePixelLayout::GA: {
			//Running threads processing rows
			tbb::task_group tg;
			for (int row = 0; row < image_height; row++) {
				tg.run(
					[&, row] {
						for (int px = 0; px < image_width; px++) { //Iterating row pixels
							linear_data[row][px * 2 + 0] = _table_toLinear_16bit[data_16[row][px * 2 + 0]]; //Gray
							linear_data[row][px * 2 + 1] = data_16[row][px * 2 + 1]; //Alpha is copied
						}
					}
				);
			}
			//Waiting for tasks to finish
			tg.wait();
		}
		break;

		case ImagePixelLayout::RGB: {
			//Running threads processing rows
			tbb::task_group tg;
			for (int row = 0; row < image_height; row++) {
				tg.run(
					[&, row] {
						for (int px = 0; px < image_width; px++) { //Iterating row pixels
							linear_data[row][px * 3 + 0] = _table_toLinear_16bit[data_16[row][px * 3 + 0]]; //Red
							linear_data[row][px * 3 + 1] = _table_toLinear_16bit[data_16[row][px * 3 + 1]]; //Green
							linear_data[row][px * 3 + 2] = _table_toLinear_16bit[data_16[row][px * 3 + 2]]; //Blue
						}
					}
				);
			}
			//Waiting for tasks to finish
			tg.wait();
		}
		break;

		case ImagePixelLayout::RGBA: {
			//Running threads processing rows
			tbb::task_group tg;
			for (int row = 0; row < image_height; row++) {
				tg.run(
					[&, row] {
						for (int px = 0; px < image_width; px++) { //Iterating row pixels
							linear_data[row][px * 4 + 0] = _table_toLinear_16bit[data_16[row][px * 4 + 0]]; //Red
							linear_data[row][px * 4 + 1] = _table_toLinear_16bit[data_16[row][px * 4 + 1]]; //Green
							linear_data[row][px * 4 + 2] = _table_toLinear_16bit[data_16[row][px * 4 + 2]]; //Blue
							linear_data[row][px * 4 + 3] = data_16[row][px * 4 + 2]; //Alpha is copied
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
	}

	return linear_image;
}







//--------------------------------
//	TABLE INITIALIZATION METHODS
//--------------------------------

void GammaConverter::InitializeTable_ToLinear_8bit() {
	//Allocating the table
	_table_toLinear_8bit = new uint16_t[WIDTH_8BIT];

	//Filling the table (defined in the derived class)
	FillTableToLinear_8bit();

	//Setting flag telling that table was initialized
	_is_table_toLinear_8bit_initialized = true;
}


void GammaConverter::InitializeTable_ToLinear_16bit() {
	//Allocating the table
	_table_toLinear_16bit = new uint16_t[WIDTH_16BIT];

	//Filling the table (defined in the derived class)
	FillTableToLinear_16bit();

	//Setting flag telling that table was initialized
	_is_table_toLinear_16bit_initialized = true;
}


void GammaConverter::InitializeTable_ToGamma_8bit() {
	//Allocating the table
	_table_toGamma_8bit = new uint8_t[WIDTH_16BIT];

	//Filling the table (defined in the derived class)
	FillTableToGamma_8bit();

	//Setting flag telling that table was initialized
	_is_table_toGamma_8bit_initialized = true;
}


void GammaConverter::InitializeTable_ToGamma_16bit() {
	//Allocating the table
	_table_toGamma_16bit = new uint16_t[WIDTH_16BIT];

	//Filling the table (defined in the derived class)
	FillTableToGamma_16bit();

	//Setting flag telling that table was initialized
	_is_table_toGamma_16bit_initialized = true;
}