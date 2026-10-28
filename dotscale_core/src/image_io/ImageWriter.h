#pragma once

#include <string>
#include <filesystem>

#include "../image_io/ImageIO.h"
#include "../image/ImageBuffer.h"
#include "../image_io/headers/ImageHeader.h"

namespace dotscale {
	 


///Base class for image writers. Provides common interface for writing an image file.
///This class is abstract and cannot be instantiated.
///</summary>
class ImageWriter {

public:
	//--------------------------------
	//	PUBLIC ACCESSORS
	//--------------------------------

	/// Index of the next row to be written in the file.
	uint32_t next_row() const noexcept { return _next_row; }

	std::filesystem::path get_gile_path() const noexcept { return _file_path; }

	virtual const ImageHeader& get_image_header() const noexcept = 0;

	/// How many rows are left to write for this image.
	uint32_t rows_left() { return _height - _next_row; }

	bool is_finished() { return _state == WriterState::Finished; }

	//--------------------------------
	//	PUBLIC METHODS
	//--------------------------------

	/// Writes next block of rows starting at NextRow.
	virtual void write_next(const ImageBuffer_uintv& block) = 0;

	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------	

	///<summary>
	///Sets file path for this writer object.
	///</summary>
	ImageWriter(std::filesystem::path file_path)
		: _file_path(file_path),
		_c_file_handle(NULL),
		_next_row(0),
		_height(0),
		_state(WriterState::Uninitialized)
	{ }

protected:
	//--------------------------------
	//	PRIVATE METHODS
	//--------------------------------

	/// Opens file pointed by file_path for binary writing in C mode.
	void open_c_file_for_writing(std::filesystem::path file_path);


	//--------------------------------
	//	INTERNAL STATES
	//--------------------------------

	enum class WriterState {
		Uninitialized,
		Ready,
		Finished,
		Failed
	};

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	FILE* _c_file_handle;
	uint32_t _next_row;
	uint32_t _height;
	std::filesystem::path _file_path;
	WriterState _state;
};


} // namespace dotscale 