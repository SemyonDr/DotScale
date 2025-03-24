#include "ImageGenerator.h"

/// <summary>
/// Creates an RGB image containing circular gradient with color channel shift.
/// </summary>
ImageBuffer_Byte ImageGenerator::CircularGradient(int height, int width, BitDepth bit_depth) {

	//Resulting image buffer
	ImageBuffer_Byte image(height, width, ImagePixelLayout::RGB, bit_depth);

	double center_x_red = static_cast<double>(width) / 2.0 + static_cast<double>(width) * 0.05;
	double center_y_red = static_cast<double>(height) / 2.0 - static_cast<double>(height) * 0.03;

	double center_x_green = static_cast<double>(width) / 2.0 - static_cast<double>(width) * 0.05;
	double center_y_green = static_cast<double>(height) / 2.0 - static_cast<double>(height) * 0.03;

	double center_x_blue = static_cast<double>(width) / 2.0;
	double center_y_blue = static_cast<double>(height) / 2.0 + static_cast<double>(height) * 0.05;

	double gradient_radius = 1.0;
	if (height >= width)
		gradient_radius = static_cast<double>(height) / 2.0;
	else
		gradient_radius = static_cast<double>(width) / 2.0;

	switch (bit_depth)
	{
		case BD_8_BIT: {
			uint8_t** data = image.GetDataPtr();
			double px_x = 0.0;
			double px_y = 0.0;
			double distance = 0.0;
			double val_double = 0.0;
			long val_long = 0;
			for (int row = 0; row < height; row++) {
				for (int col = 0; col < width; col++) {
					px_x = static_cast<double>(col) + 0.5;
					px_y = static_cast<double>(row) + 0.5;

					//Red
					distance = std::sqrt(std::pow(center_x_red - px_x, 2) + std::pow(center_y_red - px_y, 2));
					val_double = (distance * static_cast<double>(UINT8_MAX)) / (gradient_radius * 1.2);
					val_long = std::clamp(std::lround(val_double), static_cast<long>(0), static_cast<long>(UINT8_MAX));
					data[row][col * 3 + 0] = UINT8_MAX - static_cast<uint8_t>(val_long);

					//Green
					distance = std::sqrt(std::pow(center_x_green - px_x, 2) + std::pow(center_y_green - px_y, 2));
					val_double = (distance * static_cast<double>(UINT8_MAX)) / (gradient_radius * 1.2);
					val_long = std::clamp(std::lround(val_double), static_cast<long>(0), static_cast<long>(UINT8_MAX));
					data[row][col * 3 + 1] = UINT8_MAX - static_cast<uint8_t>(val_long);

					//Blue
					distance = std::sqrt(std::pow(center_x_blue - px_x, 2) + std::pow(center_y_blue - px_y, 2));
					val_double = (distance * static_cast<double>(UINT8_MAX)) / (gradient_radius * 1.2);
					val_long = std::clamp(std::lround(val_double), static_cast<long>(0), static_cast<long>(UINT8_MAX));
					data[row][col * 3 + 2] = UINT8_MAX - static_cast<uint8_t>(val_long);
				}
			}
			return image;
		}

		case BD_16_BIT: {
			uint16_t** data = reinterpret_cast<uint16_t**>(image.GetDataPtr());
			double px_x = 0.0;
			double px_y = 0.0;
			double distance = 0.0;
			double val_double = 0.0;
			long val_long = 0;
			for (int row = 0; row < height; row++) {
				for (int col = 0; col < width; col++) {
					px_x = static_cast<double>(col) + 0.5;
					px_y = static_cast<double>(row) + 0.5;

					//Red
					distance = std::sqrt(std::pow(center_x_red - px_x, 2) + std::pow(center_y_red - px_y, 2));
					val_double = (distance * static_cast<double>(UINT16_MAX)) / (gradient_radius * 1.2);
					val_long = std::clamp(std::lround(val_double), static_cast<long>(0), static_cast<long>(UINT16_MAX));
					data[row][col * 3 + 0] = UINT16_MAX - static_cast<uint16_t>(val_long);

					//Green
					distance = std::sqrt(std::pow(center_x_green - px_x, 2) + std::pow(center_y_green - px_y, 2));
					val_double = (distance * static_cast<double>(UINT16_MAX)) / (gradient_radius * 1.2);
					val_long = std::clamp(std::lround(val_double), static_cast<long>(0), static_cast<long>(UINT16_MAX));
					data[row][col * 3 + 1] = UINT16_MAX - static_cast<uint16_t>(val_long);

					//Blue
					distance = std::sqrt(std::pow(center_x_blue - px_x, 2) + std::pow(center_y_blue - px_y, 2));
					val_double = (distance * static_cast<double>(UINT16_MAX)) / (gradient_radius * 1.2);
					val_long = std::clamp(std::lround(val_double), static_cast<long>(0), static_cast<long>(UINT16_MAX));
					data[row][col * 3 + 2] = UINT16_MAX - static_cast<uint16_t>(val_long);
				}
			}
			return image;
		}

		case BD_32_BIT: {
			uint32_t** data = reinterpret_cast<uint32_t**>(image.GetDataPtr());
			double px_x = 0.0;
			double px_y = 0.0;
			double distance = 0.0;
			double val_double = 0.0;
			long val_long = 0;
			for (int row = 0; row < height; row++) {
				for (int col = 0; col < width; col++) {
					px_x = static_cast<double>(col) + 0.5;
					px_y = static_cast<double>(row) + 0.5;

					//Red
					distance = std::sqrt(std::pow(center_x_red - px_x, 2) + std::pow(center_y_red - px_y, 2));
					val_double = (distance * static_cast<double>(UINT32_MAX)) / (gradient_radius*1.2);
					val_long = std::clamp(std::lround(val_double), static_cast<long>(0), static_cast<long>(UINT32_MAX));
					data[row][col * 3 + 0] = UINT32_MAX - static_cast<uint32_t>(val_long);

					//Green
					distance = std::sqrt(std::pow(center_x_green - px_x, 2) + std::pow(center_y_green - px_y, 2));
					val_double = (distance * static_cast<double>(UINT32_MAX)) / (gradient_radius * 1.2);
					val_long = std::clamp(std::lround(val_double), static_cast<long>(0), static_cast<long>(UINT32_MAX));
					data[row][col * 3 + 1] = UINT32_MAX - static_cast<uint32_t>(val_long);

					//Blue
					distance = std::sqrt(std::pow(center_x_blue - px_x, 2) + std::pow(center_y_blue - px_y, 2));
					val_double = (distance * static_cast<double>(UINT32_MAX)) / (gradient_radius * 1.2);
					val_long = std::clamp(std::lround(val_double), static_cast<long>(0), static_cast<long>(UINT32_MAX));
					data[row][col * 3 + 2] = UINT32_MAX - static_cast<uint32_t>(val_long);
				}
			}
			return image;
		}
	}

	return image;
}