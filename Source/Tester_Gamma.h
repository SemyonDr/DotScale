#pragma once

#include "Tester_Base.h"
#include "GammaConverter.h"
#include "GammaDispatcher.h"
#include "ImageBufferPrinter.h"

class Tester_Gamma : Tester_Base{
public:
	//--------------------------------
	//	CONVERTER TESTER
	//--------------------------------

	/// <summary>
	/// Run gamma conversion test for sRGB image.
	/// </summary>
	static void TestSRGBConversion(std::string file_path) {
		Stopwatch watch;

		//Creating file path object
		std::filesystem::path in_file_path(std::string(TEST_IMAGES_PATH_STR) + "\\" + file_path);
		//Creating file folder for output
		std::filesystem::path out_dir_path = CreateOutputFolder("TestSRGBConversion");

		//Path for output file
		std::filesystem::path out_file_path(out_dir_path);
		out_file_path.replace_filename(in_file_path.filename());
		out_file_path = AddAppendixToFilename(out_file_path, "_srgb_test");

		std::cout << "Testing SRG conversion by converting image to linear colorspace and back." << std::endl;

		//Opening the image
		ImageFileInfo src_image_info(FileFormat::FF_UNSUPPORTED); //Will be rewritten
		ImageBuffer_Byte src_image = OpenImage(1, in_file_path, &src_image_info);
		std::cout << tabs(1) << "First 16 pixels of original image:" << std::endl;
		ImageBufferPrinter::PrintPixelsAt(1, 8, src_image, 0, 0, 16);

		//Producing srgb gamma converter
		GammaConverter* gconv = GammaDispatcher::GetConverter(RawImageGammaProfile::sRGB, NULL);

		//Converting to linear
		std::cout << tabs(1) << "Converting to Linear" << std::endl;
		watch.Start();
		ImageBuffer_uint16 lin_image = gconv->RemoveGammaCorrection(src_image);
		watch.Stop();
		std::cout << tabs(2) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;

		std::cout << tabs(1) << "First 16 pixels of linear image:" << std::endl;
		ImageBufferPrinter::PrintPixelsAt(1, 8, lin_image, 0, 0, 16);

		//Converting back and writing
		if (src_image_info.GetFileFormat() == FileFormat::FF_JPEG) {
			std::cout << tabs(1) << "Converting back to sRGB:" << std::endl;
			watch.Start();
			ImageBuffer_Byte trg_image = gconv->ApplyGammaCorrection(lin_image, BitDepth::BD_8_BIT);
			watch.Stop();
			std::cout << tabs(2) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;

			std::cout << tabs(1) << "First 16 pixels of converted back image:" << std::endl;
			ImageBufferPrinter::PrintPixelsAt(1, 8, trg_image, 0, 0, 16);

			JpegWriter::WriteJPEG(out_file_path, trg_image, 100);
		}
		else {
			std::cout << "Only JPEG is currently supported, sorry" << std::endl;
		}
	}



};