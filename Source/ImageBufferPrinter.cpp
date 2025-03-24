#include "ImageBufferPrinter.h"

	//--------------------------------
	//	PUBLIC PRINTER
	//--------------------------------


/// <summary>
/// Prints given number of pixels from an 8bit image at given position.
/// </summary>
/// <param name="px_inLine">How many pixels in printed line.</param>
/// <param name="image">Image buffer object pointer.</param>
/// <param name="start_row">From which row to print.</param>
/// <param name="start_col">Starting column in the row.</param>
/// <param name="num_px">How many pixels to print.</param>
void ImageBufferPrinter::PrintPixelsAt(int num_tabs, int px_inLine, const ImageBuffer_uint8& image, int start_row, int start_col, int num_px) {
	//Aliases
	uint8_t** data = image.GetDataPtr();
	int height = image.GetHeight();
	int width = image.GetWidth();
	ImagePixelLayout layout = image.GetLayout();

	PrintPixelsAt_Prompt(num_tabs, height, width, start_row, start_col, num_px);
	std::cout << tabs(num_tabs + 1) << "Bit depth is 8 bit per pixel color component. " << std::endl;
	std::cout << tabs(num_tabs + 1) << "Layout is " << PrintPixelsAt_ImageLayoutToString(layout) << std::endl;

	PrintPixelsAt_FromData<uint8_t, &FormatIntForPrinting_8>(num_tabs + 1, px_inLine, data, layout, start_row, start_col, num_px);
}

/// <summary>
/// Prints given number of pixels from an 16 bit image at given position.
/// </summary>
/// <param name="px_inLine">How many pixels in printed line.</param>
/// <param name="image">Image buffer object pointer.</param>
/// <param name="start_row">From which row to print.</param>
/// <param name="start_col">Starting column in the row.</param>
/// <param name="num_px">How many pixels to print.</param>
void ImageBufferPrinter::PrintPixelsAt(int num_tabs, int px_inLine, const ImageBuffer_uint16& image, int start_row, int start_col, int num_px) {
//Aliases
	uint16_t** data = image.GetDataPtr();
	int height = image.GetHeight();
	int width = image.GetWidth();
	ImagePixelLayout layout = image.GetLayout();

	PrintPixelsAt_Prompt(num_tabs, height, width, start_row, start_col, num_px);
	std::cout << tabs(num_tabs + 1) << "Bit depth is 16 bit per pixel color component. " << std::endl;
	std::cout << tabs(num_tabs + 1) << "Layout is " << PrintPixelsAt_ImageLayoutToString(layout) << std::endl;

	PrintPixelsAt_FromData<uint16_t, &FormatIntForPrinting_16>(num_tabs + 1, px_inLine, data, layout, start_row, start_col, num_px);
}

/// <summary>
/// Prints given number of pixels from an 32bit image at given position.
/// </summary>
/// <param name="px_inLine">How many pixels are in printed line.</param>
/// <param name="image">Image buffer object pointer.</param>
/// <param name="start_row">From which row to print.</param>
/// <param name="start_col">Starting column in the row.</param>
/// <param name="num_px">How many pixels to print.</param>
void ImageBufferPrinter::PrintPixelsAt(int num_tabs, int px_inLine, const ImageBuffer_uint32& image, int start_row, int start_col, int num_px) {
//Aliases
	uint32_t** data = image.GetDataPtr();
	int height = image.GetHeight();
	int width = image.GetWidth();
	ImagePixelLayout layout = image.GetLayout();

	PrintPixelsAt_Prompt(num_tabs, height, width, start_row, start_col, num_px);
	std::cout << tabs(num_tabs + 1) << "Bit depth is 32 bit per pixel color component. " << std::endl;
	std::cout << tabs(num_tabs + 1) << "Layout is " << PrintPixelsAt_ImageLayoutToString(layout) << std::endl;

	PrintPixelsAt_FromData<uint32_t, &FormatIntForPrinting_32>(num_tabs + 1, px_inLine, data, layout, start_row, start_col, num_px);
}

/// <summary>
/// Prints given number of pixels from an image at given position.
/// </summary>
/// <param name="px_inLine">How many pixels in printed line.</param>
/// <param name="image">Image buffer object pointer.</param>
/// <param name="start_row">From which row to print.</param>
/// <param name="start_col">Starting column in the row.</param>
/// <param name="num_px">How many pixels to print.</param>
void ImageBufferPrinter::PrintPixelsAt(int num_tabs, int px_inLine, const ImageBuffer_Byte& image, int start_row, int start_col, int num_px) {
//Aliases
	uint8_t** data = image.GetDataPtr();
	int height = image.GetHeight();
	int width = image.GetWidth();
	ImagePixelLayout layout = image.GetLayout();
	BitDepth bit_depth = image.GetBitPerComponent();

	PrintPixelsAt_Prompt(num_tabs, height, width, start_row, start_col, num_px);
	std::cout << tabs(num_tabs + 1) << "Bit depth is " << bit_depth << "bit per pixel color component. " << std::endl;
	std::cout << tabs(num_tabs + 1) << "Layout is " << PrintPixelsAt_ImageLayoutToString(layout) << std::endl;

	switch (bit_depth) {
		case BD_8_BIT:
			PrintPixelsAt_FromData<uint8_t, &FormatIntForPrinting_8>(num_tabs + 1, px_inLine, data, layout, start_row, start_col, num_px);
			break;
		case BD_16_BIT:
			PrintPixelsAt_FromData<uint16_t, &FormatIntForPrinting_16>(num_tabs + 1, px_inLine, reinterpret_cast<uint16_t**>(data), layout, start_row, start_col, num_px);
			break;
		case BD_32_BIT:
			PrintPixelsAt_FromData<uint32_t, &FormatIntForPrinting_32>(num_tabs + 1, px_inLine, reinterpret_cast<uint32_t**>(data), layout, start_row, start_col, num_px);
			break;
	}
}

/// <summary>
/// Prints given number of pixels from an image at given position.
/// </summary>
/// <param name="px_inLine">How many pixels in printed line.</param>
/// <param name="image">Image buffer object pointer.</param>
/// <param name="start_row">From which row to print.</param>
/// <param name="start_col">Starting column in the row.</param>
/// <param name="num_px">How many pixels to print.</param>
void ImageBufferPrinter::PrintPixelsAt(int num_tabs, int px_inLine, const ImageBuffer_float& image, int start_row, int start_col, int num_px) {
//Aliases
	float** data = image.GetDataPtr();
	int height = image.GetHeight();
	int width = image.GetWidth();
	ImagePixelLayout layout = image.GetLayout();

	PrintPixelsAt_Prompt(num_tabs, height, width, start_row, start_col, num_px);
	std::cout << tabs(num_tabs + 1) << "Bit depth is 32 bit per pixel color component (floating point). " << std::endl;
	std::cout << tabs(num_tabs + 1) << "Layout is " << PrintPixelsAt_ImageLayoutToString(layout) << std::endl;

	PrintPixelsAt_FromData<float, &FormatFloatForPrinting>(num_tabs + 1, px_inLine, data, layout, start_row, start_col, num_px);
}


//--------------------------------
//	PRIVATE PRINTER
//--------------------------------

/// <summary>
/// Prints prompt before printing pixels. Returns actual number of pixels to be printed
/// </summary>
int ImageBufferPrinter::PrintPixelsAt_Prompt(int num_tabs, int img_height, int img_width, int start_row, int start_col, int num_px) {
	//How many pixels we would actually print
	int actual_num = num_px;
	if (start_row + num_px > img_width) {
		actual_num = img_width - start_row;
		if (actual_num < 0)
			std::cout << tabs(num_tabs) << "Printing " << 0 << " pixels on row " << start_row << " starting from pixel number " << start_col << ". " << std::endl;
		else
			std::cout << tabs(num_tabs) << "Printing " << actual_num << " pixels on row " << start_row << " starting from pixel number " << start_col << " till the end of the row. " << std::endl;
	}
	else
		std::cout << tabs(num_tabs) << "Printing " << actual_num << " pixels on row " << start_row << " starting from pixel number " << start_col << ". " << std::endl;

	if (start_row >= img_height) {
		std::cout << tabs(num_tabs + 1) << "Printing aborted: Starting row is outside of the image." << std::endl;
		return 0;
	}

	if (start_col >= img_width) {
		std::cout << tabs(num_tabs + 1) << "Printing aborted: Starting column is outside of the image." << std::endl;
		return 0;
	}

	return actual_num;
}

//--------------------------------
//	UTILITY
//--------------------------------

inline std::string ImageBufferPrinter::PrintPixelsAt_ImageLayoutToString(ImagePixelLayout layout) {
	switch (layout)
	{
		case UNDEF:
			return "undefined";
		case G:
			return "Grayscale";
		case GA:
			return "(Grayscale Alpha)";
		case RGB:
			return "(Red Green Blue)";
		case RGBA:
			return "(Red Green Bblue Alpha)";
		default:
			return "";
	}
}



//--------------------------------
//	FORMATTERS
//--------------------------------

/// <summary>
/// Formats float values between 0 and 1 as ".123". If value is bigger than 1 it is indicated by "^" in place of ".". 
/// </summary>
std::string ImageBufferPrinter::FormatFloatForPrinting(float value) {
	//String stream for building resulting string
	std::stringstream ss;

	//Scaling value to be between [0..1000]
	value *= 1000;

	//Writing dot
	if (value >= 1.0f)
		ss << ".";
	else
		ss << "^";

	//Writing number
	ss.precision(3);
	ss << value;

	//Returning resulting string
	return ss.str();
}

std::string ImageBufferPrinter::FormatIntForPrinting_8(uint8_t value) {
	return std::to_string(static_cast<unsigned int>(value));
}

std::string ImageBufferPrinter::FormatIntForPrinting_16(uint16_t value) {
	return std::to_string(static_cast<unsigned int>(value));
}

std::string ImageBufferPrinter::FormatIntForPrinting_32(uint32_t value) {
	return std::to_string(static_cast<unsigned int>(value));
}