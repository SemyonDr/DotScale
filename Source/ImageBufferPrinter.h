#pragma once
//STL
#include <iostream> //Stream for printing
#include <sstream>  //Sting stream for formatting strings
#include <iomanip>  //Formatting methods
//Internal
#include "Printer.h"
#include "ImageBuffer.h"
#include "ImageBuffer_Byte.h"

class ImageBufferPrinter {
public:
	//--------------------------------
	//	PUBLIC PRINTER
	//--------------------------------


	/// <summary>
	/// Prints given number of pixels from an image at given position.
	/// </summary>
	/// <param name="px_inLine">How many pixels in printed line.</param>
	/// <param name="image">Image buffer object pointer.</param>
	/// <param name="start_row">From which row to print.</param>
	/// <param name="start_col">Starting column in the row.</param>
	/// <param name="num_px">How many pixels to print.</param>
	static void PrintPixelsAt(int num_tabs, int px_inLine, ImageBuffer_uint8* image, int start_row, int start_col, int num_px);

	/// <summary>
	/// Prints given number of pixels from an image at given position.
	/// </summary>
	/// <param name="px_inLine">How many pixels in printed line.</param>
	/// <param name="image">Image buffer object pointer.</param>
	/// <param name="start_row">From which row to print.</param>
	/// <param name="start_col">Starting column in the row.</param>
	/// <param name="num_px">How many pixels to print.</param>
	static void PrintPixelsAt(int num_tabs, int px_inLine, ImageBuffer_uint16* image, int start_row, int start_col, int num_px);

	/// <summary>
	/// Prints given number of pixels from an image at given position.
	/// </summary>
	/// <param name="px_inLine">How many pixels in printed line.</param>
	/// <param name="image">Image buffer object pointer.</param>
	/// <param name="start_row">From which row to print.</param>
	/// <param name="start_col">Starting column in the row.</param>
	/// <param name="num_px">How many pixels to print.</param>
	static void PrintPixelsAt(int num_tabs, int px_inLine, ImageBuffer_uint32* image, int start_row, int start_col, int num_px);

	/// <summary>
	/// Prints given number of pixels from an image at given position.
	/// </summary>
	/// <param name="px_inLine">How many pixels in printed line.</param>
	/// <param name="image">Image buffer object pointer.</param>
	/// <param name="start_row">From which row to print.</param>
	/// <param name="start_col">Starting column in the row.</param>
	/// <param name="num_px">How many pixels to print.</param>
	static void PrintPixelsAt(int num_tabs, int px_inLine, ImageBuffer_Byte* image, int start_row, int start_col, int num_px);

	/// <summary>
	/// Prints given number of pixels from an image at given position.
	/// </summary>
	/// <param name="px_inLine">How many pixels in printed line.</param>
	/// <param name="image">Image buffer object pointer.</param>
	/// <param name="start_row">From which row to print.</param>
	/// <param name="start_col">Starting column in the row.</param>
	/// <param name="num_px">How many pixels to print.</param>
	static void PrintPixelsAt(int num_tabs, int px_inLine, ImageBuffer_float* image, int start_row, int start_col, int num_px);

private:
	//--------------------------------
	//	PRIVATE PRINTER
	//--------------------------------

	/// <summary>
	/// Prints prompt before printing pixels. Returns actual number of pixels to be printed
	/// </summary>
	static int PrintPixelsAt_Prompt(int num_tabs, int img_height, int img_width, int start_row, int start_col, int num_px);


	/// <summary>
	/// Prints number pixels of an image from given place as numbers.
	/// Number formatting function can be nullptr = default stdout formatting applies.
	/// </summary>
	template<typename T, std::string (*formatToPrint)(T)>
	static void PrintPixelsAt_FromData(int num_tabs, int px_inLine, T** data, ImagePixelLayout layout, int start_row, int start_col, int num_px) {

		int last_px = start_col + num_px - 1;
		int line_px = 0;
		std::cout << tabs(num_tabs);

		switch (layout) {

			case ImagePixelLayout::G:
				
				for (int px = start_col; px <= last_px; px++, line_px++) {
					if (formatToPrint != nullptr)
						std::cout << formatToPrint(data[start_row][px]); //Gray
					else
						std::cout << data[start_row][px]; //Gray

					if (px != last_px) //Space between values
						std::cout << " ";

					if (line_px == px_inLine-1) { //New line after px_inLine
						std::cout << std::endl << tabs(num_tabs);
						line_px = -1;
					}
				}
				
				std::cout << std::endl;
				break;

			case ImagePixelLayout::GA:
				for (int px = start_col; px <= last_px; px++, line_px++) {
					std::cout << "(";
					if (formatToPrint != nullptr) {
						std::cout << formatToPrint(data[start_row][px * 2 + 0]) << " "; //Gray
						std::cout << formatToPrint(data[start_row][px * 2 + 1]);		//Alpha
					}
					else {
						std::cout << data[start_row][px * 2 + 0] << " ";	//Gray
						std::cout << data[start_row][px * 2 + 1];			//Alpha
					}
					std::cout << ")";

					if (px != last_px) //Space between values
						std::cout << " ";

					if (line_px == px_inLine-1) { //New line after px_inLine
						std::cout << std::endl << tabs(num_tabs);
						line_px = -1;
					}
				}
				std::cout << std::endl;
				break;

			case ImagePixelLayout::RGB:
				for (int px = start_col; px <= last_px; px++, line_px++) {
					std::cout << "(";
					if (formatToPrint != nullptr) {
						std::cout << formatToPrint(data[start_row][px * 3 + 0]) << " "; //Red
						std::cout << formatToPrint(data[start_row][px * 3 + 1]) << " "; //Green
						std::cout << formatToPrint(data[start_row][px * 3 + 2]);		//Blue
					}
					else {
						std::cout << data[start_row][px * 3 + 0] << " ";	//Red
						std::cout << data[start_row][px * 3 + 1] << " ";	//Green
						std::cout << data[start_row][px * 3 + 2];			//Blue
					}
					std::cout << ")";

					if (px != last_px) //Space between values
						std::cout << " ";

					if (line_px == px_inLine-1) { //New line after px_inLine
						std::cout << std::endl << tabs(num_tabs);
						line_px = -1;
					}
				}
				std::cout << std::endl;
				break;

			case ImagePixelLayout::RGBA:
				for (int px = start_col; px <= last_px; px++, line_px++) {
					std::cout << "(";
					if (formatToPrint != nullptr) {
						std::cout << formatToPrint(data[start_row][px * 4 + 0]) << " "; //Red
						std::cout << formatToPrint(data[start_row][px * 4 + 1]) << " "; //Green
						std::cout << formatToPrint(data[start_row][px * 4 + 2]) << " "; //Blue
						std::cout << formatToPrint(data[start_row][px * 4 + 3]);		//Alpha
					}
					else {
						std::cout << data[start_row][px * 4 + 0] << " ";				//Red
						std::cout << data[start_row][px * 4 + 1] << " ";				//Green
						std::cout << data[start_row][px * 4 + 2] << " ";				//Blue
						std::cout << data[start_row][px * 4 + 3];						//Alpha
					}
					std::cout << ")";

					if (px != last_px) //Space between values
						std::cout << " ";

					if (line_px == px_inLine-1) { //New line after px_inLine
						std::cout << std::endl << tabs(num_tabs);
						line_px = -1;
					}
				}
				std::cout << std::endl;
				break;

			default:
				break;
		}
	}

	//--------------------------------
	//	UTILITY
	//--------------------------------

	static inline std::string PrintPixelsAt_ImageLayoutToString(ImagePixelLayout layout);


	//--------------------------------
	//	FORMATTERS
	//--------------------------------

	/// <summary>
	/// Formats float values between 0 and 1 as ".123". If value is bigger than 1 it is indicated by "^" in place of ".". 
	/// </summary>
	static std::string FormatFloatForPrinting(float value);

	static std::string FormatIntForPrinting_8(uint8_t value);

	static std::string FormatIntForPrinting_16(uint16_t value);

	static std::string FormatIntForPrinting_32(uint32_t value);
};