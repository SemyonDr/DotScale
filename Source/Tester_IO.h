#pragma once

//STL
#include <cmath>
#include <cstdint>
//Internal - debug
#include "Tester_Base.h"
#include "ImageGenerator.h"
//Internal - other
#include "ImageBufferPrinter.h"
#include "ImageFileInfo.h"
#include "GammaDispatcher.h"

class Tester_IO : Tester_Base {
public:


	//--------------------------------
	//	JPEG TESTERS
	//--------------------------------


	/// <summary>
	/// Legacy? I honestly don't remember if this should have been deleted.
	/// </summary>
	static void TestJpegReader_Simple(std::string file_path);

	/// <summary>
	/// Tests JPEG reader by reading a file.
	/// Image is assumed to be in TestImages folder.
	/// </summary>
	static void TestJpegReader(std::string image_name);

	/// <summary>
	/// Tests JPEG writer by generating and writing an image.
	/// </summary>
	static void TestJpegWriter();

	/// <summary>
	/// Opens and decompresses JPEG file and writes it back.
	/// </summary>
	/// <param name="file_path"></param>
	static void TestJpegIOAtOnce(std::string file_path);

	/// <summary>
	/// Tests reading and writing jpeg image by chuncks.
	/// </summary>
	static void TestJpegReaderByChunks(std::string file_path, int chunk_size);

	//--------------------------------
	//	PNG TESTERS
	//--------------------------------

	/// <summary>
	/// Tests PNG reader by reading a file.
	/// Image is assumed to be in TestImages folder.
	/// </summary>
	static void TestPngReader(std::string image_name);

	/// <summary>
	/// Tests Png writer by generating and writing an image.
	/// </summary>
	static void TestPngWriter();

	/// <summary>
	/// Opens and decompresses PNG file and writes it back.
	/// </summary>
	/// <param name="file_path"></param>
	static void TestPngIOAtOnce(std::string file_path);

	/// <summary>
	/// Tests reading and writing PNG files in chunks.
	/// </summary>
	static void TestPngReaderByChunks(std::string file_path, int chunk_size);



private:



};