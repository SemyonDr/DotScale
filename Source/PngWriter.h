#pragma once

//STL
#include <string>
#include <iostream>
#include <fstream>
#include <exception>
//Third party
#include "png.h"
//Internal
#include "ImageBuffer_Byte.h"
#include "Exceptions.h"
#include "ImageWriter.h"
#include "PngHeaderInfo.h"
#include "LibPngCallbacks.h"

///<summary>
///Class for writing PNG files to disk.
///Writes whole file at once.
///Uses libpng for writing.
///</summary> 
class PngWriter :public ImageWriter, LibPngCallbacks
{
public:

	//--------------------------------
	//	CHUNK WRITER
	//--------------------------------

	///<summary>
	///Writes next block of rows starting at NextRow.
	///Advances NextRow by the height of given image.
	///If number of lines in the provided image is bigger than number of rows left writes what is possible.
	///</summary>
	void WriteNextRows(const ImageBuffer_Byte& image);

	//--------------------------------
	//	WHOLE FILE WRITING
	//--------------------------------

	/*
		Those 4 static methods are for compressing and writing a whole image object to a png file in one call.
		There is 1 main method and 3 variations that do not require providing png header, warning callbacks, or both.
	*/

	/// <summary>
	/// <para>Static method. Compresses and writes image file to the given path.</para>
	/// <para>ImageBuffer parameters should match the header.</para>
	/// <para>Can throw std::ifstream::failure if failed to open file.</para>
	/// <para>Can throw codec_fatal_exception if failed to compress the image.</para>
	/// </summary>
	/// <param name="file_path">Path to the output file.</param>
	/// <param name="image">Image to write.</param>
	/// <param name="header">PNG header that describes the file.</param>
	/// <param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	static void WritePng(std::filesystem::path file_path, const ImageBuffer_Byte& image, PngHeaderInfo header, WarningCallbackData warning_callback_data);

	/// <summary>
	/// <para>Static method. Compresses and writes image file to the given path.</para>
	/// <para>Image parameters are deducted from the buffer.</para>
	/// <para>Can throw std::ifstream::failure if failed to open file.</para>
	/// <para>Can throw codec_fatal_exception if failed to compress the image.</para>
	/// </summary>
	/// <param name="file_path">Path to the output file.</param>
	/// <param name="image">Image to write.</param>
	/// <param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	static void WritePng(std::filesystem::path file_path, const ImageBuffer_Byte& image, WarningCallbackData warning_callback_data) {
			//Deducting header
		PngHeaderInfo header_info(
			image.GetHeight(),
			image.GetWidth(),
			ImageBitDepthToPngBitDepth(image.GetBitPerComponent()),
			ImageLayoutToPngLayout(image.GetLayout()),
			PNG_INTERLACE_NONE
		);
		WritePng(file_path, image, header_info, warning_callback_data);
	}

	/// <summary>
	/// <para>Static method. Compresses and writes image file to the given path.</para>
	/// <para>ImageBuffer parameters should match the header.</para>
	/// <para>Does not register warnings.</para>
	/// <para>Can throw std::ifstream::failure if failed to open file.</para>
	/// <para>Can throw codec_fatal_exception if failed to compress the image.</para>
	/// </summary>
	/// <param name="file_path">Path to the output file.</param>
	/// <param name="image">Image to write.</param>
	/// <param name="header">PNG header that describes the file.</param>
	static void WritePng(std::filesystem::path file_path, const ImageBuffer_Byte& image, PngHeaderInfo header) {
		WarningCallbackData warning_callback(NULL, NULL);
		WritePng(file_path, image, header, warning_callback);
	}

	/// <summary>
	/// <para>Static method. Compresses and writes image file to the given path.</para>
	/// <para>Does not register warnings.</para>
	/// <para>Image parameters are deducted from the buffer.</para>
	/// <para>Can throw std::ifstream::failure if failed to open file.</para>
	/// <para>Can throw codec_fatal_exception if failed to compress the image.</para>
	/// </summary>
	/// <param name="file_path">Path to the output file.</param>
	/// <param name="image">Image to write.</param>
	static void WritePng(std::filesystem::path file_path, const ImageBuffer_Byte& image) {
		//Deducting header
		PngHeaderInfo header_info(
			image.GetHeight(),
			image.GetWidth(),
			ImageBitDepthToPngBitDepth(image.GetBitPerComponent()),
			ImageLayoutToPngLayout(image.GetLayout()),
			PNG_INTERLACE_NONE
		);
		//Creating empty callbacks object
		WarningCallbackData warning_callback(NULL, NULL);
		
		WritePng(file_path, image, header_info, warning_callback);
	}




	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------

	///<summary>
	///<para>Opens file pointed by file_path and writes the header.</para>
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to write the header.</para>
	///</summary>
	///<param name="file_path">Path to the file to be written.</param>
	///<param name="header">PNG header describing the file.</param>
	///<param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	PngWriter(std::filesystem::path file_path, PngHeaderInfo header, WarningCallbackData warning_callback_data);

	///<summary>
	///<para>Opens file pointed by file_path and writes the header.</para>
	///<para>Encoder warnings will be ignored.</para>
	///<para>Can throw std::ifstream::failure if failed to open file.</para>
	///<para>Can throw codec_fatal_exception if failed to write the header.</para>
	///</summary>
	///<param name="file_path">Path to the file to be written.</param>
	///<param name="header">PNG header describing the file.</param>
	///<param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	PngWriter(std::filesystem::path file_path, PngHeaderInfo header) 
		: PngWriter(file_path, header, WarningCallbackData(NULL, NULL)) {
	
	}

protected:
	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	/// <summary>
	/// Holds png header for the image that is written by this writer.
	/// </summary>
	PngHeaderInfo _png_header;

	/// <summary>
	/// Flag that shows if file is grayscale and less than 8 bit per pixel
	/// Because of image buffers have minimum bit depth 8 bit per pixel 
	/// additional manipulations will be required
	/// and the flag will be used to indicate the need to perform them.
	/// </summary>
	bool _is_low_depth_grayscale = false;

	//--------------------------------
	//	LIBPNG DATA STRUCTURES
	//--------------------------------

	/// <summary>
	/// PNG encoder object.
	/// </summary>
	png_structp png_ptr = NULL;

	/// <summary>
	/// PNG info object.
	/// </summary>
	png_infop info_ptr = NULL;


	//--------------------------------
	//	UTILITY METHODS
	//--------------------------------

	/// <summary>
	/// Takes in an image with grayscale layout and outputs
	/// grayscale image with given bit depth - 1, 2 or 4 bit per pixel.
	/// One byte per value is used. To write PNG correctly set png_set_packing(png_ptr).
	/// </summary>
	/// <param name="image">Input image buffer assumed to be grayscale 8 bit per pixel.</param>
	/// <param name="bit_depth">Desired bit depth. Possible values are 1, 2, 4.</param>
	static ImageBuffer_Byte CrushBitDepth(const ImageBuffer_Byte& src_image, int bit_depth);
	
	/// <summary>
	/// Translates Pixel Layout of ImageBuffer to png file layout.
	/// </summary>
	static unsigned int ImageLayoutToPngLayout(ImagePixelLayout layout);

	/// <summary>
	/// Translates PNG file color type (pixel layout) to corresponding pixel layot used in ImageBuffer objects.
	/// </summary>
	static ImagePixelLayout PngLayoutToImageLayout(unsigned char png_file_color_type);

	/// <summary>
	/// Translates Bit Depth of ImageBuffer to png bit depth.
	/// </summary>
	static unsigned int ImageBitDepthToPngBitDepth(BitDepth bit_depth);

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
	///Closes file and destroys compressor object
	///if file is opened and compressor exists.
	///</summary>
	void CleanUp();

	//--------------------------------
	//	ARCHIVED METHODS
	//--------------------------------


	/// <summary>
	/// Archived method. Whole reading procedure is written for one pass.
	/// <para>Static method. Compresses and writes image file to the given path.</para>
	/// <para>Buffer parameters should match the header.</para>
	/// <para>Can throw std::ifstream::failure if failed to open file.</para>
	/// <para>Can throw codec_fatal_exception if failed to compress the image.</para>
	/// </summary>
	/// <param name="file_path">Path to the output file.</param>
	/// <param name="image">Image to write.</param>
	/// <param name="header">PNG header data. Should contain valid output colorspace.</param>
	/// <param name="warning_callback_data">Warning callback and its arguments. Both can be set to NULL inside the structure.</param>
	static void WritePng_Archive(std::filesystem::path file_path, ImageBuffer_Byte* image, PngHeaderInfo header, WarningCallbackData warning_callback_data);
};