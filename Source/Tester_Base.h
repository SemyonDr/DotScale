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
	static ImageBuffer_Byte OpenImage(int num_tabs, std::filesystem::path file_path, ImageFileInfo* info_ptr);

	/// <summary>
	/// Opens an image and removes gamma.
	/// </summary>
	static ImageBuffer_uint16 OpenImageAndRemoveGamma(int num_tabs, std::filesystem::path file_path, ImageFileInfo* info_ptr);

	/// <summary>
	/// Writes image at given path.
	/// </summary>
	static void WriteImage(int num_tabs, const ImageBuffer_Byte& image, const ImageFileInfo& info);

	/// <summary>
	/// Applies gamma and writes back image.
	/// </summary>
	static void ApplyGammaAndWriteImage(int num_tabs, const ImageBuffer_uint16& image, const ImageFileInfo& info);


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
	static bool IsJpeg_ByExtension(std::filesystem::path file_path);

	/// <summary>
	/// Returns true if file path points to PNG a file. Only checks extension, not actual file contents.
	/// </summary>
	static bool IsPng_ByExtension(std::filesystem::path file_path);


	/// <summary>
	/// Checks file extension and returns image file type this extension indicates.
	/// </summary>
	static FileFormat GetImageTypeByExpension(std::filesystem::path file_path);


};

