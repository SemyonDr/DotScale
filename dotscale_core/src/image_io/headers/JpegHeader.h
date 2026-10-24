#pragma once

#include <optional>
#include <array>
#include <cstdint>
#include "jpeglib.h"


namespace dotscale {


/// JPEG image file header in libjpeg terms.
class JpegHeader {

public:

	//--------------------------------
	//	DEFINITIONS
	//--------------------------------

	// JPEG quantization tables
	using QuantTable = std::array<uint16_t, 64>;
	using QuantTables = std::array<std::optional<QuantTable>, 4>;
	
	// Chroma sampling factors
	struct SamplingFactors {
		int horizontal;
		int vertical;
	};
	using ComponentSampling =
		std::array<SamplingFactors, 4>;

	// Pixel per length density, defining physical size.
	// uint 0 - not set, 1 - DPI, 2 - dots per cm
	struct JfifDensity {
		uint8_t unit;
		uint16_t x_density;
		uint16_t y_density;
	};

	//--------------------------------
	//	PUBLIC ACCESSORS
	//--------------------------------

	uint32_t get_height() const noexcept { return _height; }

	uint32_t get_width() const noexcept { return _width; }

	J_COLOR_SPACE get_colorspace() const noexcept { return _color_space; }

	int get_num_cmp() const { return jpeg_util::jpeg_colorspace_num_cmp(_color_space); }

	QuantTables get_quant_tables() const noexcept { return _quant_tables; }

	ComponentSampling get_cmp_sampling() const noexcept { return _cmp_sampling; }

	std::optional<JfifDensity> get_jfif_density() const noexcept { return _jfif_density; }

	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	JpegHeader(
		uint32_t height, 
		uint32_t width, 
		J_COLOR_SPACE color_space, 
		bool progressive, 
		QuantTables quant_tables, 
		ComponentSampling component_sampling,
		std::optional<JfifDensity> jfif_density = std::nullopt)
			: _height(height), 
			_width(width), 
			_color_space(color_space), 
			_progressive(progressive), 
			_quant_tables(std::move(quant_tables)), 
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
	QuantTables _quant_tables;
	ComponentSampling _cmp_sampling;
	std::optional<JfifDensity> _jfif_density;
};

} // namespace dotscale