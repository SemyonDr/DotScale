#pragma once

//STL
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <chrono>
//Internal
#include "JpegReader.h"
#include "JpegWriter.h"
#include "PngReader.h"
#include "PngWriter.h"
//Debug
#include "Printer.h"
#include "Stopwatch.h"
#include "ImageFileInfo.h"



/// <summary>
/// Collection of basic methods used by tester classes.
/// </summary>
class Tester_Base
{
public:
	//--------------------------------
	//	IO METHODS
	//--------------------------------

	/// <summary>
	/// Opens an image.
	/// </summary>
	static ImageFileInfo OpenImage(int num_tabs, std::filesystem::path file_path);

	/// <summary>
	/// Method to call when jpeg codec produces warnings.
	/// Prints warning to console.
	/// Args are considered integer number of tabs before the warning message
	/// </summary>
	static void JPEGWarningHandler(std::string message, void* args) {
		int num_tabs = *reinterpret_cast<int*>(args);
		std::cout << tabs(num_tabs) << "JPEG codec Warning: " << message << std::endl;
	}

	/// <summary>
	/// Method to call when png codec produces warnings.
	/// Prints warning to console.
	/// Args are considered integer number of tabs before the warning message.
	/// </summary>
	static void PNGWarningHandler(std::string message, void* args) {
		int num_tabs = *reinterpret_cast<int*>(args);
		std::cout << tabs(num_tabs) << "PNG codec Warning: " << message << std::endl;
	}

	//-----------------------------------
	//	PATH AND FILESYSTEM MANIPULATION
	//-----------------------------------
	
	/// <summary>
	/// Adds appendix to the filename in given path
	/// </summary>
	static std::filesystem::path AddAppendixToFilename(std::filesystem::path source_path, std::string appendix);

	/// <summary>
	/// Creates output directory in TestImages.
	/// Returns path to the output folder.
	/// </summary>
	static std::filesystem::path CreateOutputFolder(std::string out_folder_name);


	//--------------------------------
	//	UTILITY
	//--------------------------------

	/// <summary>
	/// Returns true if file path points to JPEG a file. Only checks extension, not actual file contents.
	/// </summary>
	/// <returns></returns>
	static bool IsJpeg_ByExtension(std::filesystem::path file_path);

	/// <summary>
	/// Returns true if file path points to PNG a file. Only checks extension, not actual file contents.
	/// </summary>
	/// <returns></returns>
	static bool IsPng_ByExtension(std::filesystem::path file_path);


};

