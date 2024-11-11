#pragma once

#include <filesystem>
#include "ImageEnums.h"
#include "ImageBuffer_Byte.h"
#include "ImageBufferInfo.h"
#include "JpegHeaderInfo.h"
#include "PngHeaderInfo.h"

class ImageFileInfo {

public:
	FileFormat file_format;
	ImageBuffer_Byte* decompressed_image = nullptr;
	std::filesystem::path file_path;
	JpegHeaderInfo jpeg_header;
	PngHeaderInfo png_header;

	ImageFileInfo() {};


private:


};