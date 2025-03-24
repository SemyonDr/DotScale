#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "ImageFileInfo.h"
#include "ImageBuffer_Byte.h"

#include "EnumToString.h"


//--------------------------------
//	OUTSIDE OF CLASS PRINTING METHODS
//--------------------------------

/// <summary>
/// Prints given number of tabs.
/// </summary>
/// <param name="num"></param>
void PrintTabs(int num);

/// <summary>
/// Array of strings of tabs.
/// </summary>
const std::string tabs_str_array[] = {
	"",

	"\t",
	"\t\t",
	"\t\t\t",
	"\t\t\t\t",

	"\t\t\t\t\t",
	"\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t",

	"\t\t\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t\t\t\t\t",

	"\t\t\t\t\t\t\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t",
};

/// <summary>
/// Returns given number of tabs as a string.
/// Max 16 tabs.
/// </summary>
std::string tabs(int num);



//--------------------------------
//	PRINTER CLASS
//--------------------------------

/// <summary>
/// Collection of methods for debug and test printing.
/// </summary>
class Printer
{
public:
	static void EmptyLine() {
		std::cout << std::endl;
	}

	/// <summary>
	/// Prints info about image file.
	/// </summary>
	static void PrintImageFileInfo(int num_tabs, const ImageFileInfo& image_info);

	/// <summary>
	/// Prints info about image buffer object.
	/// </summary>
	/// <param name="num_tabs"></param>
	/// <param name="image"></param>
	static void PrintImageBufferInfo(int num_tabs, const ImageBuffer_Byte& image);

	/// <summary>
	/// Print contents of a jpeg header.
	/// </summary>
	static void PrintJpegHeader(int num_tabs, const JpegHeaderInfo& header);

	/// <summary>
	/// Prins contents of a png header.
	/// </summary>
	static void PrintPngHeader(int num_tabs, const PngHeaderInfo& header);

	/// <summary>
	/// Prints file path.
	/// </summary>
	static void PrintFilePath(int num_tabs, const std::filesystem::path& file_path);
};