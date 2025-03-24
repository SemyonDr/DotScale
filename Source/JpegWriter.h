#pragma once
//STL
#include <string>
#include <iostream>
#include <fstream>
#include <exception>
//Third party
#include "jpeglib.h"
#include "jerror.h"
//Internal
#include "ImageWriter.h"
#include "Exceptions.h"
#include "JpegHeaderInfo.h"
#include "LibJpegCallbacks.h"



///<summary>
///Class for writing JPEG images to disk.
///Uses libjpeg-turbo library for encoding.
///Writes whole image at once.
///</summary>
///<remarks>
///Error handling for libjpeg is done by supplying callback methods to the library.
///Those methods will be called when errors are encountered in file by libjpeg.
///For fatal errors ErrorExitHandler is called and throws exception for JpegReader consumer to catch.
///Exception must be thrown and catched and procedures terminated and execution should jump.
///
///For warnings WarningHandler is used. To avoid forcing the consumer to deal with libjpeg objects directly
///this handler calls callback provided by JpegReader consumer and passes warning message as a parameter.
///</remarks>
class JpegWriter : public ImageWriter, public LibJpegCallbacks
{
public:

	//--------------------------------
	//	CHUNK WRITER
	//--------------------------------

	///<summary>
	///Writes next block of rows starting at NextRow.
	///Advances NextRow by the height of given image.
	///If number of lines in the provided image is bigger than number of rows left writes what is possible.
	///<para>Can throw codec_fatal_exception if failed to write.</para>
	///</summary>
	void WriteNextRows(const ImageBuffer_Byte& image) override;


	//--------------------------------
	//	WHOLE FILE WRITING
	//--------------------------------

	/*
		Those 4 static methods are for compressing and writing a whole image object to jpeg file in one call.
		There is 1 main method and 3 variations that do not require providing jpeg header, warning callbacks, or both.
	*/


	///<summary>
	///Static method. Compresses and writes image file to the given path.
	///Image is expected to be 8 bit per channel and not containing alpha channels.
	///Does not register warnings. 
	///Output file colorspace is deduced from image object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to compress the image.</para>
	///</summary>
	///<param name="file_path">Path to the output file.</param>
	///<param name="image">Image to write.</param>
	///<param name="quality">Jpeg codec compression quality setting. 1-100</param>
	static void WriteJPEG(std::filesystem::path file_path, const ImageBuffer_Byte& image, int quality) {
		JpegHeaderInfo header_info;
		header_info._height = image.GetHeight();
		header_info._width = image.GetWidth();
		switch (image.GetLayout())
		{
		case UNDEF:
			break;
		case G:
			header_info._color_space = J_COLOR_SPACE::JCS_GRAYSCALE;
			header_info._num_components = 1;
			break;
		case GA:
			header_info._color_space = J_COLOR_SPACE::JCS_GRAYSCALE;
			header_info._num_components = 2;
			break;
		case RGB:
			header_info._color_space = J_COLOR_SPACE::JCS_RGB;
			header_info._num_components = 3;
			break;
		case RGBA:
			header_info._color_space = J_COLOR_SPACE::JCS_EXT_RGBA;
			header_info._num_components = 4;
			break;
		default:
			break;
		}
		WriteJPEG(file_path, image, header_info, quality, WarningCallbackData(NULL, NULL));
	}


	///<summary>
	///Static method. Compresses and writes image file to the given path.
	///Image is expected to be 8 bit per channel and not containing alpha channels. 
	///Output file colorspace is deduced from image object.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to compress the image.</para>
	///</summary>
	///<param name="file_path">Path to the output file.</param>
	///<param name="image">Image to write.</param>
	///<param name="quality">Jpeg codec compression quality setting. 1-100</param>
	/// <param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	static void WriteJPEG(std::filesystem::path file_path, const ImageBuffer_Byte& image, int quality, WarningCallbackData warning_callback_data) {
		JpegHeaderInfo header_info;
		header_info._height = image.GetHeight();
		header_info._width = image.GetWidth();
		switch (image.GetLayout())
		{
		case UNDEF:
			break;
		case G:
			header_info._color_space = J_COLOR_SPACE::JCS_GRAYSCALE;
			header_info._num_components = 1;
			break;
		case GA:
			header_info._color_space = J_COLOR_SPACE::JCS_GRAYSCALE;
			header_info._num_components = 2;
			break;
		case RGB:
			header_info._color_space = J_COLOR_SPACE::JCS_RGB;
			header_info._num_components = 3;
			break;
		case RGBA:
			header_info._color_space = J_COLOR_SPACE::JCS_EXT_RGBA;
			header_info._num_components = 4;
			break;
		default:
			break;
		}
		WriteJPEG(file_path, image, header_info, quality, warning_callback_data);
	}


	///<summary>
	///Static method. Compresses and writes image file to the given path. 
	///Image is expected to be 8 bit per channel and not containing alpha channels. 
	///Does not register warnings.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to compress the image.</para>
	///</summary>
	///<param name="file_path">Path to the output file.</param>
	///<param name="image">Image to write.</param>
	///<param name="header">Jpeg header data. Should contain output colorspace.</param>
	///<param name="quality">Jpeg codec compression quality setting. 1-100</param>
	static void WriteJPEG(std::filesystem::path file_path, const ImageBuffer_Byte& image, JpegHeaderInfo header, int quality) {
		WriteJPEG(file_path, image, header, quality, WarningCallbackData(NULL,NULL));
	}

	///<summary>
	/// Static method. Compresses and writes image file to the given path.
	/// Image is expected to be 8 bit per channel and not containing alpha channels.
	/// <para>Can throw std::ifstream::failure if failed to open file.</para>
	/// <para>Can throw codec_fatal_exception if failed to compress the image.</para>
	/// </summary>
	/// <param name="file_path">Path to the output file.</param>
	/// <param name="image">Image to write.</param>
	/// <param name="header">Jpeg header data. Should contain output colorspace.</param>
	/// <param name="quality">Jpeg codec compression quality setting. 1-100</param>
	/// <param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	static void WriteJPEG(std::filesystem::path file_path, const ImageBuffer_Byte& image, JpegHeaderInfo header, int quality, WarningCallbackData warning_callback_data);






	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------

	///<summary>
	///<para>Opens file pointed by file_path and writes the header.</para>
	///<para>JPEG format does not support alpha channels. Alpha channels should be mixed in, or removed by the consumer before passing an image object to the writer, or exception will be produced.</para>
	///<para>Provided image dimensions take precedence over header ones. Layout actually written to the file should be provided in JpegHeaderInfo.</para>
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to write the header.</para>
	///</summary>
	///<param name="file_path">Path to the file to be written.</param>
	///<param name="header">JPEG header describing the file.</param>
	///<param name="quality">JPEG compression quality setting.</param>
	JpegWriter(std::filesystem::path file_path, JpegHeaderInfo header, int quality, WarningCallbackData warning_callback_data);


	//--------------------------------
	//	DEFAULT DESTRUCTOR
	//--------------------------------

	~JpegWriter() {
		jpeg_destroy_compress(&jpeg_comp);
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
	///LibJpeg compressor object.
	///</summary>
	struct jpeg_compress_struct jpeg_comp;

	///<summary>
	///Error manager object for compressor.
	///</summary>
	struct jpeg_error_mgr jerr_comp;

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
	//	DEFAULT CONSTRUCTOR
	//--------------------------------

	JpegWriter() { }


	//--------------------------------
	//	ARCHIVED METHODS
	//--------------------------------


	///<summary>
	/// Archived method. Whole reading procedure is written for one pass.
	///Static method. Compresses and writes image file to the given path.
	///Image is expected to be 8 bit per channel and not containing alpha channels.
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to compress the image.</para>
	///</summary>
	///<param name="file_path">Path to the output file.</param>
	///<param name="image">Image to write.</param>
	///<param name="header">Jpeg header data. Should contain output colorspace.</param>
	///<param name="quality">Jpeg codec compression quality setting. 1-100</param>
	///<param name="warning_callback_ptr">Pointer to the warning callback function. Can be NULL.</param>
	///<param name="warning_callback_args_ptr">Pointer to the warning callback function arguments. Can be NULL.</param>
	static void WriteJPEG_Archive(std::filesystem::path file_path, ImageBuffer_Byte* image, JpegHeaderInfo header, int quality, void(*warning_callback_ptr)(std::string, void*), void* warning_callback_args_ptr);
};