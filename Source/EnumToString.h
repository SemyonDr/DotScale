#pragma once

//STL
#include <string>
//Internal
#include "ImageEnums.h"

class EnumToString {
public:

	//--------------------------------
	//	ENUM TO STRING METHODS
	//--------------------------------

	/// <summary>
	/// Converts Image Layout to string.
	/// </summary>
	static std::string LayoutToString(ImagePixelLayout layout) {
		switch (layout) {
			case ImagePixelLayout::G:
				return "Gray";
			case ImagePixelLayout::GA:
				return "GrayAlpha";
			case ImagePixelLayout::RGB:
				return "RGB";
			case ImagePixelLayout::RGBA:
				return "RGBA";
			default:
				return "";
		}
	}

	/// <summary>
	/// Converts BitDepth to string.
	/// </summary>
	static std::string BitDepthToString(BitDepth bit_depth) {
		switch (bit_depth)
		{
			case BD_8_BIT:
				return "8 bit";
			case BD_16_BIT:
				return "16 bit";
			case BD_32_BIT:
				return "32 bit";
			default:
				return "";
		}
	}

	/// <summary>
	/// Converts file format enum items to string.
	/// </summary>
	static std::string FileFormaToString(FileFormat file_format) {
		switch (file_format)
		{
			case FF_UNSUPPORTED:
				return "Unsupported.";
			case FF_JPEG:
				return "JPEG";
			case FF_PNG:
				return "PNG";
			default:
				return "";
		}
	}

	/// <summary>
	/// Covnerts libjpeg JPEG colorspace enum items to string.
	/// </summary>
	/// <param name="jpeg_layout"></param>
	/// <returns></returns>
	static std::string JpegLayoutToString(J_COLOR_SPACE jpeg_layout) {
		switch (jpeg_layout)
		{
			// Internal to JPEG files
			case JCS_UNKNOWN:
				return "Unknown";
			case JCS_GRAYSCALE:
				return "Grayscale (Internal)";
			case JCS_RGB:      
				return "RGB (Internal)";
			case JCS_YCbCr:            
				return "YCbCr (Internal)";
			case JCS_CMYK:
				return "CMYK (Internal)";
			case JCS_YCCK:
				return "YCCK (Internal)";
			// Can be used as input/output setting to codec
			case JCS_EXT_RGB:
				return "RGB (External)";
			case JCS_EXT_RGBX:
				return "RGBX (External)";
			case JCS_EXT_BGR:
				return "BGR (External)";
			case JCS_EXT_BGRX:
				return "BGRX (External)";
			case JCS_EXT_XBGR:
				return "XBGR (External)";
			case JCS_EXT_XRGB:
				return "XRGB (External)";
			case JCS_EXT_RGBA:
				return "RGBA (External)";
			case JCS_EXT_BGRA:
				return "BGRA (External)";
			case JCS_EXT_ABGR:
				return "ABGR (External)";
			case JCS_EXT_ARGB:
				return "ARGB (External)";
			case JCS_RGB565:
				return "RGB 5-6-5";
			default:
				return "";
		}
	}


	/// <summary>
	/// Converts PNG file colorspace to string.
	/// </summary>
	static std::string PngLayoutToString(unsigned char png_layout) {
		switch (png_layout)
		{
			case PNG_COLOR_TYPE_GRAY:
				return "Grayscale";
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				return "Grayscale-Alpha";
			case PNG_COLOR_TYPE_RGB:
				return "RGB";
			case PNG_COLOR_TYPE_RGBA:
				return "RGBA";
			case PNG_COLOR_TYPE_PALETTE:
				return "Palette";
			default:
				return "Invalid";
		}
	}


	static std::string PngInterlaceTypeToString(unsigned char interlace_type) {
		switch (interlace_type) {
			case PNG_INTERLACE_NONE:
				return "No Interlacing";
			case PNG_INTERLACE_ADAM7:
				return "Adam 7";
			default:
				return "Invalid";
		}
	}
};