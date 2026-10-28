#pragma once

#include <string>
#include <any>
#include <functional>

#include "jpeglib.h"

#include "../image/ImageEnums.h"


namespace dotscale {
namespace io_image {

/// Passes custom callback to image io classes
struct CodecWarningCallback {
	std::function<void(const std::string&, std::any&)> func = nullptr;
	std::any arg = {};
};
	
constexpr const std::string layout_name(PixelLayout layout) {
	switch (layout) {
		case PixelLayout::G: return "Grayscale";
		case PixelLayout::GA: return "Grayscale+Alpha";
		case PixelLayout::RGB: return "RGB";
		case PixelLayout::RGBA: return "RGBA";
	}

	std::unreachable();
}

} // namespace io_image
} // namespace dotscale 


