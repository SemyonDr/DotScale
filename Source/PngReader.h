#pragma once
//STL
#include <string>
#include <iostream>
#include <fstream>
#include <exception>
//Third party
#include "png.h"
//Internal
#include "ImageReader.h"
#include "Exceptions.h"
#include "PngHeaderInfo.h"
#include "LibPngCallbacks.h"

///<summary>
///Class for reading PNG files from disk.
///Uses libpng for reading.
///Opens the file when constructing the reader and keeps the file opened until reading is finished, or reader is destroyed.
///</summary>
class PngReader : public ImageReader, LibPngCallbacks
{
public:
	//--------------------------------
	//	ACCESSORS
	//--------------------------------

	///<summary>
	///Returns PNG header object.
	///</summary>
	PngHeaderInfo GetPngHeader() {
		return _png_header;
	}

	//--------------------------------
	//	CHUNK READER
	//--------------------------------

	///<summary>
	///Reads next block of rows starting at NextRow.
	///Returns resulting bitmap.
	///Advances NextRow by num_lines.
	///If num_lines is bigger than number of rows left reads all available rows.
	///When all rows are already read returns NULL.
	///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
	///</summary>
	ImageBuffer_Byte* ReadNextRows(int num_rows) override;


	//--------------------------------
	//	WHOLE FILE READING
	//--------------------------------

	///<summary>
	///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
	///</summary>
	///<param name="file_path">Path to file to read.</param>
	///<param name="headerPtr">Writes info to PNG header located at this pointer. If NULL it is ignored.</param>
	/// <param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	static ImageBuffer_Byte* ReadPngFile(std::filesystem::path file_path, PngHeaderInfo* headerPtr, WarningCallbackData warning_callback_data);

	///<summary>
	///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
	///<para>PNG header is not saved.</para>
	///</summary>
	///<param name="file_path">Path to file to read.</param>
	/// <param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	static ImageBuffer_Byte* ReadPngFile(std::filesystem::path file_path, WarningCallbackData warning_callback_data) {
		return ReadPngFile(file_path, NULL, warning_callback_data);
	}

	///<summary>
	///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
	///<para>Decoder warnings will be ignored.</para>
	///</summary>
	///<param name="file_path">Path to file to read.</param>
	///<param name="headerPtr">Writes info to PNG header located at this pointer. If NULL it is ignored.</param>
	static ImageBuffer_Byte* ReadPngFile(std::filesystem::path file_path, PngHeaderInfo* headerPtr) {
		WarningCallbackData warning_callback_data(NULL, NULL);
		return ReadPngFile(file_path, headerPtr, warning_callback_data);
	}

	///<summary>
	///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
	///<para>PNG header is not saved.</para>
	///<para>Decoder warnings will be ignored.</para>
	///</summary>
	///<param name="file_path">Path to file to read.</param>
	///<param name="headerPtr">Writes info to PNG header located at this pointer. If NULL it is ignored.</param>
	static ImageBuffer_Byte* ReadPngFile(std::filesystem::path file_path) {
		WarningCallbackData warning_callback_data(NULL, NULL);
		return ReadPngFile(file_path, NULL, warning_callback_data);
	}


	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------

	///<summary>
	///<para>Opens file pointed by file_path and reads the file header.</para>
	///<para>Header is accessible by calling GetCommonHeader() and GetPngHeader().</para>
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the header.</para>
	///</summary>
	PngReader(std::filesystem::path file_path, WarningCallbackData warning_callback_data);

	///<summary>
	///<para>Opens file pointed by file_path and reads the file header.</para>
	///<para>Header is accessible by calling GetCommonHeader() and GetPngHeader().</para>
	///<para>Decoder warnings will be ignored.</para>
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the header.</para>
	///</summary>
	PngReader(std::filesystem::path file_path) 
		: PngReader(file_path, WarningCallbackData(NULL, NULL)) {
	
	}

	//--------------------------------
	//	DEFAULT DESTRUCTOR
	//--------------------------------

	~PngReader() {
		//If reader is ready we finish its operations and close the file
		if (_state == ReaderStates::Ready_Start || _state == ReaderStates::Ready_Continue) {
			//Finalizing reading
			png_read_end(_png_read_struct_ptr, NULL);
			//Releasing the decompressor object memory
			png_destroy_read_struct(&_png_read_struct_ptr, &_png_info_ptr, (png_infopp)NULL);
			_is_decompressor_initialized = false;
			//Closing the file
			fclose(_file_handle);
			_is_file_opened = false;
		}
		else
			//In all other cases we just use flags to determine what should be cleaned
			CleanUp();
	}


private:
	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	///<summary>
	///Png header object.
	///</summary>
	PngHeaderInfo _png_header;

	//--------------------------------
	//	LIBPNG DATA STRUCTURES
	//--------------------------------

	png_structp _png_read_struct_ptr = NULL;
	png_infop _png_info_ptr = NULL;

	//--------------------------------
	//	UTILITY METHODS
	//--------------------------------

	/// <summary>
	/// Translates PNG file color type (pixel layout) to corresponding pixel layot used in ImageBuffer objects.
	/// </summary>
	static ImagePixelLayout PngLayoutToImageLayout(unsigned char png_file_color_type);

	/// <summary>
	/// Translates PNG file bit depth to corresponding bit depth parameter used in ImageBuffer objects.
	/// </summary>
	/// <param name="png_bit_depth"></param>
	/// <returns></returns>
	static BitDepth PngBitDepthToImageBitDepth(unsigned int png_bit_depth);

	//--------------------------------
	//	PRIVATE METHODS
	//--------------------------------

	///<summary>
	///Closes file and destroys decompressor
	///if file is opened and decompressor exists.
	///</summary>
	void CleanUp();

	//--------------------------------
	//	PRIVATE CONSTRUCTOR
	//--------------------------------

	///<summary>
	///Default constructor (hidden).
	///</summary>
	PngReader() {
	}

	//--------------------------------
	//	ARCHIVED METHODS
	//--------------------------------

	///<summary>
	///Archived method. Whole reading procedure is written for one pass.
	///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
	///</summary>
	///<param name="file_path">Path to file to read.</param>
	///<param name="headerPtr">Writes info to PNG header located at this pointer. If NULL it is ignored.</param>
	///<param name="warningCallback">Pointer to function to handle warnings produced by libPng. Can be NULL.</param>
	///<param name="warningCallbackArgsPtr">Arguments to be given to warning handler function. Can be NULL.</param>
	static ImageBuffer_Byte* ReadPngFile_Archive(std::filesystem::path file_path, PngHeaderInfo* headerPtr, WarningCallbackData warning_callback_data);
};