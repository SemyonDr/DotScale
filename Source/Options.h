#pragma once

#include <filesystem>
#include <optional>

// ------------------------
// Options for the CLI app
// ------------------------


// ------------------------
// Image scaling options
// ------------------------

enum class ScaleMode {
	Relative,	// Scale by percentage
	Absolute,	// Scale to exact size
	Dpi			// Target resolution
};

struct RelativeScale {
	double vertical;
	double horizontal;
};

enum class AbsScaleTarget {
	Explicit,	// New size is given for vert and hor sides explicitly
	LongEdge,	// New size is given for the long edge, short edge proportional
	ShortEdge,  // New size is given for the short edge, long edge proportional
	Vertical,	// New size is given for the vertical edge, short edge proportional
	Horizontal,	// New size is given for the short edge, long edge proportional
	Equal		// One size applies to both sides (causes distortion if the image is not a square)
};

struct AbsoluteScale {
	int targeted; // Used if only one size provided for absolute size
	int vertical;
	int horizontal;
};

/// <summary>
/// Image scaling options.
/// </summary>
struct ScaleOptions {
	ScaleMode mode;
	std::optional<RelativeScale> scale_rel;
	std::optional<AbsScaleTarget> abs_target;
	std::optional<AbsoluteScale> scale_abs;
	bool enlarge = false;	// Enlarge image if it is smaller that new size
};

// ------------------------
// Image Blur options
// ------------------------

// How blur radius is interpreted
enum class BlurMode {
	Relavive,	// Always relative to the short side
	Absolute
};

struct BlurOptions {
	BlurMode mode;
	double radius;
};

// ------------------------
// Filesystem options
// ------------------------

struct FilesystemOptions {
	std::filesystem::path input_path;
	std::filesystem::path output_path;
};


// ------------------------
// All options
// ------------------------

struct DotscaleOptions {
	FilesystemOptions filesystem;
	ScaleOptions scale;
	BlurOptions blur;
};