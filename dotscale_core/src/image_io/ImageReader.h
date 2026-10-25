#pragma once
//STL
#include <filesystem>
#include <cstdio>
//Internal
#include "../image_io/headers/ImageHeader.h"
#include "../image/ImageBuffer.h"

namespace dotscale {

/// Abstract base for image readers. 
/// Provides common interface for reading an image file.
class ImageReader {

public:
	//--------------------------------
	//	PUBLIC ACCESSORS
	//--------------------------------

	std::filesystem::path get_file_path() const noexcept { return _file_path; }

	virtual const ImageHeader& get_image_header() const noexcept = 0;

	/// First row to be read by read_next_rows().
	uint32_t next_row() const noexcept { return _next_row; }

	uint32_t rows_left() const noexcept { return _height - _next_row; }

	bool is_finished()  const noexcept { return _state == ReaderState::Finished; }


	//--------------------------------
	//	PUBLIC METHODS
	//--------------------------------

	///Reads next block of rows starting at NextRow.
	virtual ImageBuffer_uintv read_next(uint32_t lines) = 0;

	/// Reads all the lines that left.
	ImageBuffer_uintv read_all() { return read_next(rows_left()); }

	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------	

	ImageReader(std::filesystem::path file_path)
		: _file_path(file_path),
		_state(ReaderState::Uninitialized),
		_c_file_handle(NULL),
		_height(0),
		_next_row(0) { }

protected:

	//--------------------------------
	//	PRIVATE METHODS
	//--------------------------------

	/// Opens file pointed by file_path for binary reading in C mode.
	void open_c_file_for_reading(std::filesystem::path& file_path);


	//--------------------------------
	//	INTERNAL STATES
	//--------------------------------

	enum class ReaderState {
		Uninitialized,		// Initial state - file closed, decompressor not initialized, header data is not available
		Ready,				// File is open, decompressor initialized, set and ready to read the file.
		Finished,			// File is closed, decompressor not initialized, header data is accessible, no lines left to read
	};

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	ReaderState _state;
	std::FILE* _c_file_handle;	// C file
	std::filesystem::path _file_path;
	uint32_t _height;
	uint32_t _next_row;

};

} // namespace dotscale