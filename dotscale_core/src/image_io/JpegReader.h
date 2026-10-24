#pragma once
//STL
#include <iostream>
#include <fstream>
//Third party
#include "jpeglib.h"
#include "jerror.h"
//Internal

#include "../image_io/IOUtil.h"
#include "../image_io/ImageReader.h"
#include "../image_io/headers/JpegHeader.h"
#include "../image/ImageBuffer.h"

namespace dotscale {


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
class JpegReader : public ImageReader
{

public:
	//--------------------------------
	//	ACCESSORS
	//--------------------------------

	const JpegHeader& get_jpeg_header() const noexcept { return _jpeg_header;  }

	const ImageHeader& get_image_header() const noexcept override { return _image_header; }


	//--------------------------------
	//	CHUNK READER
	//--------------------------------

	/// Reads next block of rows.
	ImageBuffer_uintv read_next(uint32_t num_rows) override;

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


	JpegReader(std::filesystem::path file_path) : JpegReader(file_path, WarningCallbackData(NULL, NULL)) {
	
	}

	JpegReader(std::filesystem::path file_path, io_util::CodecWarningCallback warning_callback);


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
	//	ERROR/WARNING HANDLERS
	//--------------------------------

	/// Called when libjpeg routines encounter non-fatal error, or corrupt data and are able to continue.
	/// Redirects to user-supplied callback.
	static void _warning_handler(j_common_ptr jpeg_object, int msg_level);


	///Callback used when libjpeg decoding routines encounter fatal error and are terminated.
	///Throws an exception containing libjpeg error message.
	static void _error_exit_handler(j_common_ptr jpeg_object);


	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	JpegHeader _jpeg_header;
	ImageHeader _image_header;

	io_util::CodecWarningCallback _warning_callback;

	//--------------------------------
	//	LIBJPEG DATA STRUCTURES
	//--------------------------------
	
	///LibJpeg decompressor object.
	struct jpeg_decompress_struct jpeg_decomp;

	///Error manager object for decompressor.
	struct jpeg_error_mgr jerr_decomp;

};

} // namespace dotscale