#pragma once

#include <filesystem>
#include "ImageEnums.h"
#include "ImageBuffer_Byte.h"
#include "ImageBufferInfo.h"
#include "JpegHeaderInfo.h"
#include "PngHeaderInfo.h"

/// <summary>
/// This class holds all the information on particular image file.
/// </summary>
class ImageFileInfo {

public:

	//--------------------------------
	//	ACCESSORS
	//--------------------------------	

	/// <summary>
	/// Image file format.
	/// </summary>
	FileFormat GetFileFormat() const { return _file_format; }

	/// <summary>
	/// Path to the file.
	/// </summary>
	std::filesystem::path GetFilePath() const { return _file_path; }

	/// <summary>
	/// Header for JPEG image.
	/// </summary>
	JpegHeaderInfo GetJpegHeader() const { return _jpeg_header; }

	/// <summary>
	/// Header for PNG image.
	/// </summary>
	PngHeaderInfo GetPngHeader() const { return _png_header; }

	/// <summary>
	/// Decompressed image buffer description.
	/// </summary>
	ImageBufferInfo GetImgBufferInfo() const { return _img_buf_info; }

	/// <summary>
	/// Set new file path.
	/// </summary>
	void SetFilePath(const std::filesystem::path file_path) { this->_file_path = file_path; }

	/// <summary>
	/// Set new JPEG header data (copying).
	/// </summary>
	void SetJpegHeader(JpegHeaderInfo jpeg_header) { this->_jpeg_header = jpeg_header; }

	/// <summary>
	/// Set new PNG header data (copying).
	/// </summary>
	void SetPngHeader(PngHeaderInfo png_header) { this->_png_header = png_header; }

	/// <summary>
	/// Set new buffer description (copying).
	/// </summary>
	void SetImgBufferInfo(ImageBufferInfo buffer_info) { this->_img_buf_info = buffer_info; }

	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------	

	/// <summary>
	/// Default constructor is disabled.
	/// </summary>
	ImageFileInfo() = delete;

	/// <summary>
	/// Initializing constructor by file format.
	/// </summary>
	ImageFileInfo(FileFormat format) : 
		_file_format(format) { 
	}

	/// <summary>
	/// Builging JPEG file info
	/// </summary>
	ImageFileInfo(JpegHeaderInfo jpeg_header) :
		_file_format(FileFormat::FF_JPEG),
		_jpeg_header(jpeg_header.GetHeight(), jpeg_header.GetWidth(), jpeg_header.GetNumComponents(), jpeg_header.GetColorSpace()) {
	}

	/// <summary>
	/// Building PNG file info.
	/// </summary>
	ImageFileInfo(PngHeaderInfo png_header) :
		_file_format(FileFormat::FF_PNG),
		_png_header(png_header.GetHeight(), png_header.GetWidth(), png_header.GetBitDepth(), png_header.GetPngColorType(), png_header.GetPngInterlaceType()) {
	}

	/// <summary>
	/// Initializing constructor by file format.
	/// </summary>
	ImageFileInfo(std::filesystem::path file_path, FileFormat format) :
		_file_path(file_path),
		_file_format(format) {
	}

	/// <summary>
	/// Builging JPEG file info
	/// </summary>
	ImageFileInfo(std::filesystem::path file_path, JpegHeaderInfo jpeg_header) :
		_file_path(file_path),
		_file_format(FileFormat::FF_JPEG),
		_jpeg_header(jpeg_header.GetHeight(), jpeg_header.GetWidth(), jpeg_header.GetNumComponents(), jpeg_header.GetColorSpace()),
		_img_buf_info(jpeg_header) {
		
	}

	/// <summary>
	/// Building PNG file info.
	/// </summary>
	ImageFileInfo(std::filesystem::path file_path, PngHeaderInfo png_header) :
		_file_path(file_path),
		_file_format(FileFormat::FF_PNG),
		_png_header(png_header.GetHeight(), png_header.GetWidth(), png_header.GetBitDepth(), png_header.GetPngColorType(), png_header.GetPngInterlaceType()) {
	}

	//--------------------------------
	//	COPY/MOVE
	//--------------------------------	

	/// <summary>
	/// Copy constructor.
	/// </summary>
	ImageFileInfo(const ImageFileInfo& other) :
		_file_format(other._file_format),
		_file_path(other._file_path),
		_jpeg_header(other._jpeg_header),
		_png_header(other._png_header),
		_img_buf_info(other._img_buf_info) {
		
	}

	/// <summary>
	/// Copy assigment.
	/// </summary>
	ImageFileInfo& operator=(const ImageFileInfo& other) {
		if (&other == this)
			return *this;

		this->_file_path = other._file_path;
		this->_file_format = other._file_format;
		this->_jpeg_header = other._jpeg_header;
		this->_png_header = other._png_header;
		this->_img_buf_info = other._img_buf_info;

		return *this;
	}

	/// <summary>
	/// Move constructor.
	/// </summary>
	ImageFileInfo(ImageFileInfo&& other) :
		_file_format(std::move(other._file_format)),
		_file_path(std::move(other._file_path)),
		_jpeg_header(std::move(other._jpeg_header)),
		_png_header(std::move(other._png_header)),
		_img_buf_info(other._img_buf_info) {
	}

	/// <summary>
	/// Move assigment.
	/// </summary>
	ImageFileInfo& operator=(ImageFileInfo&& other) {
		if (&other == this)
			return *this;

		this->_file_path = std::move(other._file_path);
		this->_file_format = std::move(other._file_format);
		this->_jpeg_header = std::move(other._jpeg_header);
		this->_png_header = std::move(other._png_header);
		this->_img_buf_info = other._img_buf_info;

		return *this;
	}

private:

//--------------------------------
//	PRIVATE DATA FIELDS
//--------------------------------	

	/// <summary>
	/// Path to the file.
	/// </summary>
	std::filesystem::path _file_path;

	/// <summary>
	/// Image file format.
	/// </summary>
	FileFormat _file_format;

	/// <summary>
	/// Header for JPEG image.
	/// </summary>
	JpegHeaderInfo _jpeg_header;

	/// <summary>
	/// Header for PNG image.
	/// </summary>
	PngHeaderInfo _png_header;

	/// <summary>
	/// Decompressed image buffer description.
	/// </summary>
	ImageBufferInfo _img_buf_info;

};