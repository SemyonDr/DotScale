#pragma once

/*
	Collection of helpful functions for dealing with JPEG through libjpeg.

*/

#include <exception>
#include <any>
#include <functional>
#include "jpeglib.h"

#include "../image/ImageEnums.h"


namespace dotscale {

namespace io_util {

	/// Passes custom callback to image io classes
	struct CodecWarningCallback {
		std::function<void(const std::string&, std::any&)> func = nullptr;
		std::any arg = {};
	};
	
} // namespace io_util

namespace jpeg_util {
	

	/// Returns name of a JPEG colorspace.
	constexpr const char* jpeg_colorspace_name(J_COLOR_SPACE layout) {
		switch (layout) {
			// Error
			case J_COLOR_SPACE::JCS_UNKNOWN: return "Unknown";
			// Actual JPEG file layouts
			case J_COLOR_SPACE::JCS_GRAYSCALE: return "Grayscale";
			case J_COLOR_SPACE::JCS_RGB: return "RGB";                
			case J_COLOR_SPACE::JCS_YCbCr: return "YCbCr";             
			case J_COLOR_SPACE::JCS_CMYK: return "CMYK";               
			case J_COLOR_SPACE::JCS_YCCK: return "YCCK";               
			// Additional application-side layouts that the codec supports
			case J_COLOR_SPACE::JCS_EXT_RGB: return "Extended RGB";    /* Same as regular RGB */
			case J_COLOR_SPACE::JCS_EXT_RGBX: return "Extended RGBX";           
			case J_COLOR_SPACE::JCS_EXT_BGR: return "Extended BGR";            
			case J_COLOR_SPACE::JCS_EXT_BGRX: return "Extended BGRX";           
			case J_COLOR_SPACE::JCS_EXT_XBGR: return "Extended XBGR";           
			case J_COLOR_SPACE::JCS_EXT_XRGB: return "Extended XRGB";           
			// X component set to 0xFF by the codec when decompressing
			case J_COLOR_SPACE::JCS_EXT_RGBA: return "Extended RGBA";           
			case J_COLOR_SPACE::JCS_EXT_BGRA: return "Extended BGRA";           
			case J_COLOR_SPACE::JCS_EXT_ABGR: return "Extended ABGR";           
			case J_COLOR_SPACE::JCS_EXT_ARGB: return "Extended ARGB";           
			case J_COLOR_SPACE::JCS_RGB565: return "RGB565";
			default: throw std::exception("Value is not recognized as a JPEG colorspace.");
		}
	}


	// Finds suitable image layout to store given JPEG colorspace.
	constexpr PixelLayout jpeg_colorspace_to_storage(J_COLOR_SPACE layout) {
		switch (layout) {
			case J_COLOR_SPACE::JCS_UNKNOWN: throw std::exception("Cannot determine storage image layout for Unknown JPEG colorspace.");
			// Actual JPEG file layouts
			case J_COLOR_SPACE::JCS_GRAYSCALE: return PixelLayout::G;
			case J_COLOR_SPACE::JCS_RGB: return PixelLayout::RGB;
			case J_COLOR_SPACE::JCS_YCbCr: return PixelLayout::RGB;
			case J_COLOR_SPACE::JCS_CMYK: return PixelLayout::RGB;
			case J_COLOR_SPACE::JCS_YCCK: return PixelLayout::RGB;
			// Additional application-side layouts that the codec supports
			case J_COLOR_SPACE::JCS_EXT_RGB: return PixelLayout::RGB;    /* Same as regular RGB */
			case J_COLOR_SPACE::JCS_EXT_RGBX: return PixelLayout::RGBA;
			case J_COLOR_SPACE::JCS_EXT_BGR: return PixelLayout::RGBA;
			case J_COLOR_SPACE::JCS_EXT_BGRX: return PixelLayout::RGBA;
			case J_COLOR_SPACE::JCS_EXT_XBGR: return PixelLayout::RGBA;
			case J_COLOR_SPACE::JCS_EXT_XRGB: return PixelLayout::RGBA;
			// X component set to 0xFF by the codec when decompressing
			case J_COLOR_SPACE::JCS_EXT_RGBA: return PixelLayout::RGBA;
			case J_COLOR_SPACE::JCS_EXT_BGRA: return PixelLayout::RGBA;
			case J_COLOR_SPACE::JCS_EXT_ABGR: return PixelLayout::RGBA;
			case J_COLOR_SPACE::JCS_EXT_ARGB: return PixelLayout::RGBA;
			case J_COLOR_SPACE::JCS_RGB565: return PixelLayout::RGB;
			default: throw std::exception("Value is not recognized as a JPEG colorspace.");
		}
	}


	/// Can JPEG file use this libjpeg colorspace.
	constexpr bool jpeg_colorspace_is_out(J_COLOR_SPACE layout) {
		switch (layout) {
		// Error
			case J_COLOR_SPACE::JCS_UNKNOWN: return false;
			// Actual JPEG file layouts
			case J_COLOR_SPACE::JCS_GRAYSCALE: return true;
			case J_COLOR_SPACE::JCS_RGB: return true;
			case J_COLOR_SPACE::JCS_YCbCr: return true;
			case J_COLOR_SPACE::JCS_CMYK: return true;
			case J_COLOR_SPACE::JCS_YCCK: return true;
			// Additional application-side layouts that the codec supports
			case J_COLOR_SPACE::JCS_EXT_RGB: return true;    /* Same as regular RGB */
			case J_COLOR_SPACE::JCS_EXT_RGBX: return false;
			case J_COLOR_SPACE::JCS_EXT_BGR: return false;
			case J_COLOR_SPACE::JCS_EXT_BGRX: return false;
			case J_COLOR_SPACE::JCS_EXT_XBGR: return false;
			case J_COLOR_SPACE::JCS_EXT_XRGB: return false;
			// X component set to 0xFF by the codec when decompressing
			case J_COLOR_SPACE::JCS_EXT_RGBA: return false;
			case J_COLOR_SPACE::JCS_EXT_BGRA: return false;
			case J_COLOR_SPACE::JCS_EXT_ABGR: return false;
			case J_COLOR_SPACE::JCS_EXT_ARGB: return false;
			case J_COLOR_SPACE::JCS_RGB565: return false;
			default: throw std::exception("Value is not recognized as a JPEG colorspace.");
		}
	}


	/// Number of components in a color space.
	constexpr int jpeg_colorspace_num_cmp(J_COLOR_SPACE layout) {
		switch (layout) {
			// Error
			case J_COLOR_SPACE::JCS_UNKNOWN: throw std::exception("Cannot determine number of components for Unknown JPEG colorspace.");
			// Actual JPEG file layouts
			case J_COLOR_SPACE::JCS_GRAYSCALE: return 1;
			case J_COLOR_SPACE::JCS_RGB: return 3;
			case J_COLOR_SPACE::JCS_YCbCr: return 3;
			case J_COLOR_SPACE::JCS_CMYK: return 4;
			case J_COLOR_SPACE::JCS_YCCK: return 4;
			// Additional application-side layouts that the codec supports
			case J_COLOR_SPACE::JCS_EXT_RGB: return 3;    /* Same as regular RGB */
			case J_COLOR_SPACE::JCS_EXT_RGBX: return 4;
			case J_COLOR_SPACE::JCS_EXT_BGR: return 3;
			case J_COLOR_SPACE::JCS_EXT_BGRX: return 4;
			case J_COLOR_SPACE::JCS_EXT_XBGR: return 4;
			case J_COLOR_SPACE::JCS_EXT_XRGB: return 4;
			// X component set to 0xFF by the codec when decompressing
			case J_COLOR_SPACE::JCS_EXT_RGBA: return 4;
			case J_COLOR_SPACE::JCS_EXT_BGRA: return 4;
			case J_COLOR_SPACE::JCS_EXT_ABGR: return 4;
			case J_COLOR_SPACE::JCS_EXT_ARGB: return 4;
			case J_COLOR_SPACE::JCS_RGB565: return 3;
			default: throw std::exception("Value is not recognized as a JPEG colorspace.");
		}
	}
	
} // namespace jpeg_util




} // namespace dotscale 