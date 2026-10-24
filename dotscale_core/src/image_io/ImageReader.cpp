#include "../image_io/ImageReader.h"

#include <ios>
#include <system_error>

namespace dotscale {

/// Opens file pointed by file_path for binary reading in C mode.
void ImageReader::open_c_file_for_reading(std::filesystem::path& file_path) {

	// Trying to open the file in reading binary mode
	// This is WIN32 ONLY! On windows file_path stores wide char - utf16
	// _wfopen_s expects to work with wide char and is MSVC specific.
	errno_t fopen_error = _wfopen_s(&_c_file_handle, file_path.c_str(), L"rb");

	if (fopen_error != 0)
		throw std::ios_base::failure(
			"Failed to open image file for reading: " + file_path.string(),
			std::error_code(fopen_error, std::generic_category())
		);
}


} // namespace dotscale