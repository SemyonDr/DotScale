#pragma once
//STL
#include <iostream>
#include <fstream>
#include <exception>
//Third party
#include "jpeglib.h"
#include "jerror.h"
//Internal
#include "ImageReader.h"
#include "Exceptions.h"
#include "JpegHeaderInfo.h"
#include "LibJpegCallbacks.h"



///<summary>
///Class for reading JPEG images from disk.
///Uses libjpeg-turbo library for decoding.
///Opens the file when constructing the reader and keeps the file opened until reading is finished, or reader is destroyed.
///</summary>
///<remarks>
///Error handling for libjpeg is done by supplying callback methods to the library.
///Those methods will be called when errors are encountered in file by libjpeg.
///For fatal errors ErrorExitHandler is called and throws codec_fatal_exception for JpegReader consumer to catch.
///Exception must be thrown and catched and procedures terminated and execution should jump.
///
///For warnings WarningHandler is used. To avoid forcing the consumer to deal with libjpeg objects directly
///this handler calls callback provided by JpegReader consumer and passes warning message as a parameter.
///</remarks>
class JpegReader : public ImageReader, public LibJpegCallbacks
{


public:
	//--------------------------------
	//	ACCESSORS
	//--------------------------------

	///<summary>
	///Returns JPEG header object.
	///</summary>
	JpegHeaderInfo GetJpegHeader() { 
		return _jpeg_header;
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
	ImageBuffer_Byte ReadNextRows(int num_rows) override;

	//--------------------------------
	//	WHOLE FILE READING
	//--------------------------------

	///<summary>
	///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
	///</summary>
	///<param name="file_path">Path to file to read.</param>
	///<param name="headerPtr">Writes info to JPEG header located at this pointer. If NULL it is ignored.</param>
	///<param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	static ImageBuffer_Byte ReadJpegFile(std::filesystem::path file_path, JpegHeaderInfo* headerPtr, WarningCallbackData warning_callback_data);

	///<summary>
	///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
	///<para>JPEG header is not saved.</para>
	///</summary>
	///<param name="file_path">Path to file to read.</param>
	///<param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	static ImageBuffer_Byte ReadJpegFile(std::filesystem::path file_path, WarningCallbackData warning_callback_data) {
		return ReadJpegFile(file_path, NULL, warning_callback_data);
	}

	///<summary>
	///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
	///<para>Decoder warnings will be ignored.</para>
	///</summary>
	///<param name="file_path">Path to file to read.</param>
	///<param name="headerPtr">Writes info to JPEG header located at this pointer. If NULL it is ignored.</param>
	static ImageBuffer_Byte ReadJpegFile(std::filesystem::path file_path, JpegHeaderInfo* headerPtr) {
		return ReadJpegFile(file_path, headerPtr, WarningCallbackData(NULL,NULL));
	}

	///<summary>
	///Static method. Reads and decompresses file pointed to by file_path and returns an image buffer object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the image.</para>
	///<para>JPEG header is not saved.</para>
	///<para>Decoder warnings will be ignored.</para>
	///</summary>
	///<param name="file_path">Path to file to read.</param>
	static ImageBuffer_Byte ReadJpegFile(std::filesystem::path file_path) {
		return ReadJpegFile(file_path, NULL, WarningCallbackData(NULL,NULL));
	};


	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------

	///<summary>
	///<para>Opens file pointed by file_path and reads the file header.</para>
	///<para>Header is accessible by calling GetCommonHeader() and GetJpegHeader().</para>
	///<para>Decoder warnings will be ignored.</para>
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the header.</para>
	///</summary>
	JpegReader(std::filesystem::path file_path) : JpegReader(file_path, WarningCallbackData(NULL, NULL)) {
	
	}

	///<summary>
	///<para>Opens file pointed by file_path and reads the file header.</para>
	///<para>Header is accessible by calling GetCommonHeader() and GetJpegHeader().</para>
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to decompress the header.</para>
	///</summary>
	JpegReader(std::filesystem::path file_path, WarningCallbackData warning_callback_data);


	//--------------------------------
	//	DEFAULT DESTRUCTOR
	//--------------------------------

	~JpegReader() {
		//If reader is ready we finish its operations and close the file
		if (_state == ReaderStates::Ready_Start || _state == ReaderStates::Ready_Continue) {
			//Finalizing reading (not that well described step, may be unnecessary)
			jpeg_finish_decompress(&jpeg_decomp);
			//Releasing the decompressor object memory
			jpeg_destroy_decompress(&jpeg_decomp);
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
	///Jpeg header object.
	///</summary>
	JpegHeaderInfo _jpeg_header;

	//--------------------------------
	//	LIBJPEG DATA STRUCTURES
	//--------------------------------
	
	///<summary>
	///LibJpeg decompressor object.
	///</summary>
	struct jpeg_decompress_struct jpeg_decomp;

	///<summary>
	///Error manager object for decompressor.
	///</summary>
	struct jpeg_error_mgr jerr_decomp;

	//--------------------------------
	//	UTILITY METHODS
	//--------------------------------

	/// <summary>
	/// Translates jpeg file colorspace (pixel layout) to appropriate pixel layout used in ImageBuffer objects.
	/// </summary>
	static ImagePixelLayout JpegLayoutToImageLayout(J_COLOR_SPACE jpeg_colorspace);

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
	JpegReader() { 
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
	///<param name="headerPtr">Writes info to JPEG header located at this pointer. If NULL it is ignored.</param>
	///<param name="warningCallback">Pointer to function to handle warnings produced by libJpeg. Can be NULL.</param>
	///<param name="warningCallbackArgsPtr">Arguments to be given to warning handler function. Can be NULL.</param>
	static ImageBuffer_Byte* ReadJpegFile_Archive(std::filesystem::path file_path, JpegHeaderInfo* headerPtr, void (*warningCallback)(std::string, void*), void* warningCallbackArgsPtr);
};