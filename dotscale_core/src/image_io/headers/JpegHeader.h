#pragma once

#include <optional>
#include <cstdint>
#include "jpeglib.h"

#include "../../image_io/JpegIO.h"

namespace dotscale {

/// JPEG image file header in libjpeg terms.
class JpegHeader {

public:

	//--------------------------------
	//	PUBLIC ACCESSORS
	//--------------------------------

	uint32_t get_height() const noexcept { return _height; }

	uint32_t get_width() const noexcept { return _width; }

	J_COLOR_SPACE get_colorspace() const noexcept { return _color_space; }

	int get_num_cmp() const { return io_jpeg::jpeg_colorspace_num_cmp(_color_space); }

	bool has_quantization() const noexcept { return _quantization.has_value(); }

	std::optional<io_jpeg::Quantization> get_quantization() const noexcept { return _quantization; }

	bool has_sampling() const noexcept { return _cmp_sampling.has_value(); }

	std::optional<io_jpeg::ComponentSampling> get_cmp_sampling() const noexcept { return _cmp_sampling; }

	bool has_jfif_density() const noexcept { return _jfif_density.has_value(); }

	std::optional<io_jpeg::JfifDensity> get_jfif_density() const noexcept { return _jfif_density; }

	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	JpegHeader(
		uint32_t height, 
		uint32_t width, 
		J_COLOR_SPACE color_space, 
		bool progressive, 
		std::optional<io_jpeg::ComponentSampling> component_sampling = std::nullopt,
		std::optional<io_jpeg::Quantization> quantization = std::nullopt,
		std::optional<io_jpeg::JfifDensity> jfif_density = std::nullopt)
			: _height(height), 
			_width(width), 
			_color_space(color_space), 
			_progressive(progressive), 
			_quantization(std::move(quantization)),
			_cmp_sampling(std::move(component_sampling)),
			_jfif_density(jfif_density)
	{ }

	
private:
	//--------------------------------
	//	HEADER DATA
	//--------------------------------

	uint32_t _height;
	uint32_t _width;
	J_COLOR_SPACE _color_space;

	bool _progressive;
	std::optional<io_jpeg::Quantization> _quantization;
	std::optional<io_jpeg::ComponentSampling> _cmp_sampling;
	std::optional<io_jpeg::JfifDensity> _jfif_density;
};

} // namespace dotscale