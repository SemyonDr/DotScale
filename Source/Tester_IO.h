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
	/// Tests JPEG reader by reading a file.
	/// Image is assumed to be in TestImages folder.
	/// </summary>
	static void TestJpegReader(std::string image_name);

	/// <summary>
	/// Tests JPEG writer by generating and writing an image.
	/// </summary>
	static void TestJpegWriter();


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

	//--------------------------------
	//	BASIC READING AND WRITING TESTS
	//--------------------------------

	/// <summary>
	/// Opens and decompresses JPEG file and writes it back.
	/// </summary>
	/// <param name="file_path"></param>
	static void TestJpegIOAtOnce(std::string file_path) {

		std::filesystem::path in_file_path(file_path);

		//Intro
		std::cout << "TEST: Reading and writing back JPEG image file at once." << std::endl;
		Printer::PrintFilePath(1, in_file_path);
		Printer::EmptyLine();

		if (IsJpeg_ByExtension(std::filesystem::path(file_path)) == false) {
			std::cout << "\tAbort: File is not a JPEG by extension." << std::endl;
			return;
		}	

		try {
			//Warning handler
			int warning_tabs = 2;
			WarningCallbackData warning_callback_data(&JPEGWarningHandler, &warning_tabs);

			//Header container
			JpegHeaderInfo in_header;
			
			//Reading file
			std::cout << "\t1) Reading the file." << std::endl;
			ImageBuffer_Byte* image = JpegReader::ReadJpegFile(file_path, &in_header, warning_callback_data);

			//File info
			std::cout << "\tReading is finished." << std::endl;
			std::cout << "\t\tImage object description is:" << std::endl;
			Printer::PrintImageBufferInfo(3, image);
			std::cout << "\t\tJpeg header description is:" << std::endl;
			Printer::PrintJpegHeader(3, in_header);
			Printer::EmptyLine();

			//Writing file
			std::filesystem::path out_file_path = AddAppendixToFilename(in_file_path, "_copy");
			std::cout << "\tWriting JPEG image back at once." << std::endl;
			Printer::PrintFilePath(1, out_file_path);
			Printer::EmptyLine();
			
			//JpegWriter::WriteJPEG(out_file_path, image, in_header, 100);

			JpegHeaderInfo out_header(in_header.GetHeight(), in_header.GetWidth(), 3, in_header.GetColorSpace());

			std::cout << "\t\tInitializing the writer.";
			JpegWriter writer(out_file_path, out_header, 100, warning_callback_data);
			std::cout << " -- Done!." << std::endl;
			std::cout << "\t\tWriting.";
			writer.WriteNextRows(image);
			std::cout << " -- Done!." << std::endl;
			
			std::cout << "Writing is finished." << std::endl;
		}
		catch (std::ifstream::failure e) {
			std::cout << e.what() << std::endl;
			return;
		}
		catch (codec_fatal_exception e) {
			std::cout << e.GetFullMessage() << std::endl;
			return;
		}
	}



	/// <summary>
	/// Opens and decompresses PNG file and writes it back.
	/// </summary>
	/// <param name="file_path"></param>
	static void TestPngIOAtOnce(std::string file_path) {
		//Intro
		std::cout << "Reading and writing PNG image file at once." << std::endl;
		std::cout << "\tFile path is:" << std::endl;
		std::cout << "\t\t" << file_path << std::endl;
		std::cout << std::endl;

		if (IsPng_ByExtension(std::filesystem::path(file_path)) == false) {
			std::cout << "\tAbort: File is not a PNG by extension." << std::endl;
			return;
		}

		try {
			//Warning handler
			int warning_tabs = 2;
			WarningCallbackData warning_callback_data(&PNGWarningHandler, &warning_tabs);

			//Header container
			PngHeaderInfo png_header;

			//Reading file
			std::filesystem::path in_file_path(file_path);
			ImageBuffer_Byte* image = PngReader::ReadPngFile(file_path, &png_header, warning_callback_data);

			//File info
			std::cout << "\tReading is finished." << std::endl;
			std::cout << "\t\tImage object description is:" << std::endl;
			Printer::PrintImageBufferInfo(3, image);
			std::cout << std::endl;
			//std::cout << "\t\tpng header description is:" << std::endl;
			//Printer::PrintPngHeader(3, png_header);

			//Writing file
			std::filesystem::path out_file_path = AddAppendixToFilename(in_file_path, "_copy");
			std::cout << "\tWriting PNG image back at once. File path is:" << std::endl;
			std::cout << "\t\t" << out_file_path.string() << std::endl;
			PngWriter::WritePng(out_file_path, image);
			std::cout << "Writing is finished." << std::endl;
		}
		catch (std::ifstream::failure e) {
			std::cout << e.what() << std::endl;
			return;
		}
		catch (codec_fatal_exception e) {
			std::cout << e.GetFullMessage() << std::endl;
			return;
		}
	}


	/// <summary>
	/// 
	/// </summary>
	/// <param name="file_path"></param>
	/// <param name="chunk_size"></param>
	static void TestJpegReaderByChunks(std::string file_path, int chunk_size) {
		//Intro
		std::cout << "Reading and writing back JPEG image file by chunks with chunk size of " << chunk_size << " rows." << std::endl;
		std::cout << "\tFile path is:" << std::endl;
		std::cout << "\t\t" << file_path << std::endl;
		std::cout << std::endl;

		if (IsJpeg_ByExtension(std::filesystem::path(file_path)) == false) {
			std::cout << "\tFile is not a JPEG by extension." << std::endl;
			return;
		}

		// 1 ---------------------
		//Reading the file at once
		std::cout << "\tReading the file at once." << std::endl;
		std::filesystem::path in_file_path(file_path);
		ImageBuffer_Byte* image_at_once = NULL;
		try {
			image_at_once = JpegReader::ReadJpegFile(in_file_path);
		}
		catch (std::ifstream::failure e) {
			std::cout << e.what() << std::endl;
			return;
		}
		catch (codec_fatal_exception e) {
			std::cout << e.GetFullMessage() << std::endl;
			return;
		}

		//File info
		std::cout << tabs(1) << "Reading is finished. Image object description is:" << std::endl;
		Printer::PrintImageBufferInfo(2, image_at_once);
		std::cout << std::endl;

		try {
			// 2 -------------------------------------
			//Reading the file by chunks
			std::cout << "\tReading and writing jpeg file by chunks." << std::endl;

			//Reader init
			std::cout  << "\tInitializing Reader.";
			JpegReader jpeg_reader(in_file_path);
			std::cout << "\tDONE." << std::endl;

			JpegHeaderInfo jpeg_header = jpeg_reader.GetJpegHeader();

			//Writer init
			std::cout << "\tInitializing Writer.";
			std::filesystem::path out_file_path = AddAppendixToFilename(in_file_path, "_chunks_copy");
			int num_tabs_in_warning = 3;
			JpegWriter jpeg_writer(out_file_path, jpeg_header, 100, WarningCallbackData(&JPEGWarningHandler,&num_tabs_in_warning));
			std::cout << "\tDONE. Writer path:" << std::endl;
			std::cout << "\t\t" << out_file_path.string() << std::endl;

			//Number of chunks
			int num_chunks = jpeg_header.GetHeight() / chunk_size;
			if (static_cast<unsigned int>(num_chunks * chunk_size) < jpeg_header.GetHeight())
				num_chunks++;
			std::cout << "\tNumber of chunks to read is " << num_chunks << "." << std::endl;

			//Image that will be combined from the chunks
			//We read first chunk outside the loop to initialize the image
			std::cout << "\t\tReading chunk 0.\tNext line is " << jpeg_reader.GetNextRowIndex() << "."  << std::endl;
			ImageBuffer_Byte* combined_image = jpeg_reader.ReadNextRows(chunk_size);
			std::cout << "\t\tWriting chunk 0.\tNext line is " << jpeg_writer.GetNextRowIndex() << "." << std::endl;
			jpeg_writer.WriteNextRows(combined_image);

			//All remaining chunks we do in a loop
			for (int chunk = 1; chunk < num_chunks; chunk++) {

				//Reading chunk
				std::cout << "\t\tReading chunk " << chunk << ":";
				std::cout << "\tNext line is " << jpeg_reader.GetNextRowIndex() << ".";
				ImageBuffer_Byte* image_chunk = jpeg_reader.ReadNextRows(chunk_size);
				std::cout << "\t" << image_chunk->GetHeight() << " lines were read." << std::endl;

				//Appending the chunk to combined image
				combined_image->Append(image_chunk);

				//Writing chunk
				std::cout << "\t\tWriting chunk " << chunk << ":";
				int rows_before = jpeg_writer.GetNextRowIndex();
				std::cout << "\tNext line is " << rows_before << ".";
				jpeg_writer.WriteNextRows(image_chunk);
				std::cout << "\t" << jpeg_writer.GetNextRowIndex() - rows_before << " lines were written." << std::endl;

				//Disposing
				delete image_chunk;
			}

			// 3 -------------------------------------
			//Checking results

			//Checking states
			if (jpeg_reader.IsFinished())
				std::cout << "\tJpeg reader state is finished." << std::endl;
			else
				std::cout << "\tJpeg reader state is NOT finished." << std::endl;
			std::cout << std::endl;

			std::cout << "\tCombined image info:" << std::endl;
			Printer::PrintImageBufferInfo(2, combined_image);

			//Checking images obtained by 2 different readings
			if (image_at_once->GetHeight() == combined_image->GetHeight() && image_at_once->GetWidth() == combined_image->GetWidth()) {
				std::cout << "\tImage dimensions match." << std::endl;
				int cmp_width = combined_image->GetWidth() * combined_image->GetNumCmp();
				long miss_counter = 0;

				uint8_t** cmb_data = combined_image->GetData();
				uint8_t** onc_data = image_at_once->GetData();

				for (int row = 0; row < combined_image->GetHeight(); row++)
					for (int cmp = 0; cmp < cmp_width; cmp++)
						if (cmb_data[row][cmp] != onc_data[row][cmp])
							miss_counter++;

				if (miss_counter == 0)
					std::cout << "\tImage data match." << std::endl;
				else

					std::cout << "\tImage data do not match " << miss_counter << " times." << std::endl;

			}
			else
				std::cout << "\tImage dimensions do not match. Images are not comparable." << std::endl;
			std::cout << std::endl;

			//Writing combined image
			std::filesystem::path out_file_path_combined = AddAppendixToFilename(in_file_path, "_chunks_combined_copy");
			std::cout << "\tWriting combined image. File path:" << std::endl;
			std::cout << "\t\t" << out_file_path_combined.string() << std::endl;
			JpegWriter::WriteJPEG(out_file_path_combined, combined_image, 100);
			std::cout << "\tDONE writing!" << std::endl;
		}
		catch (std::ifstream::failure e) {
			std::cout << e.what() << std::endl;
			return;
		}
		catch (codec_fatal_exception e) {
			std::cout << e.GetFullMessage() << std::endl;
			return;
		}
	}



	static void TestJpegReader_Simple(std::string file_path) {
		std::filesystem::path in_file_path(file_path);

		//Intro
		std::cout << "Reading and writing JPEG image file in simple steps." << std::endl;
		std::cout << "\tFile path is:" << std::endl;
		std::cout << "\t\t" << file_path << std::endl;
		std::cout << "\t\t" << std::filesystem::absolute(in_file_path).string() << std::endl;
		std::cout << std::endl;

		if (IsJpeg_ByExtension(std::filesystem::path(file_path)) == false) {
			std::cout << "\tAbort: File is not a JPEG by extension." << std::endl;
			return;
		}

		try {
			//Warning handler
			int warning_tabs = 2;
			WarningCallbackData warning_callback_data(&JPEGWarningHandler, &warning_tabs);

			//Header container
			JpegHeaderInfo jpeg_header;

			//Reader init
			std::cout << "\tInitializing JPEG reader (opening the file and reading the header):" << std::endl;
			JpegReader reader(in_file_path, warning_callback_data);
			std::cout << "\t\tInit done! Printing JPEG file header:" << std::endl;
			Printer::PrintJpegHeader(2, reader.GetJpegHeader());
			std::cout << "\tNext row to read is: " << reader.GetNextRowIndex() << std::endl;

			//Reading file
			std::cout << "\tReading all the lines of the image:" << std::endl;
			ImageBuffer_Byte* image = reader.ReadNextRows(reader.GetJpegHeader().GetHeight());
			std::cout << "\t\tDone! Rows left to read: " << reader.GetJpegHeader().GetHeight() - reader.GetNextRowIndex() << std::endl;

			//File info
			std::cout << "\tReading is finished." << std::endl;
			std::cout << "\t\tImage object description is:" << std::endl;
			Printer::PrintImageBufferInfo(3, image);
			std::cout << std::endl;

			/*
			//Writing file
			std::filesystem::path out_file_path = AddAppendixToFilename(in_file_path, "_copy");
			std::cout << "\tWriting JPEG image back at once. File path is:" << std::endl;
			std::cout << "\t\t" << out_file_path.string() << std::endl;
			JpegWriter::WriteJPEG(out_file_path, image, 100);
			std::cout << "Writing is finished." << std::endl;
			*/
		}
		catch (std::ifstream::failure e) {
			std::cout << e.what() << std::endl;
			return;
		}
		catch (codec_fatal_exception e) {
			std::cout << e.GetFullMessage() << std::endl;
			return;
		}
	}



	/// <summary>
	/// 
	/// </summary>
	/// <param name="file_path"></param>
	/// <param name="chunk_size"></param>
	static void TestPngReaderByChunks(std::string file_path, int chunk_size) {
		//Intro
		std::cout << "Reading and writing back PNG image file by chunks with chunk size of " << chunk_size << " rows." << std::endl;
		std::cout << "\tFile path is:" << std::endl;
		std::cout << "\t\t" << file_path << std::endl;
		std::cout << std::endl;

		if (IsPng_ByExtension(std::filesystem::path(file_path)) == false) {
			std::cout << "\tFile is not a PNG by extension." << std::endl;
			return;
		}

		// 1 ---------------------
		//Reading the file at once
		std::cout << "\tReading the file at once." << std::endl;
		std::filesystem::path in_file_path(file_path);
		ImageBuffer_Byte* image_at_once = NULL;
		try {
			image_at_once = PngReader::ReadPngFile(in_file_path);
		}
		catch (std::ifstream::failure e) {
			std::cout << e.what() << std::endl;
			return;
		}
		catch (codec_fatal_exception e) {
			std::cout << e.GetFullMessage() << std::endl;
			return;
		}

		//File info
		std::cout <<  "\tReading is finished. Image object description is:" << std::endl;
		Printer::PrintImageBufferInfo(2, image_at_once);
		std::cout << std::endl;

		try {
			// 2 -------------------------------------
			//Reading the file by chunks
			std::cout << "\tReading and writing png file by chunks." << std::endl;

			//Reader init
			std::cout << "\tInitializing Reader.";
			PngReader png_reader(in_file_path);
			std::cout << "\tDONE." << std::endl;

			PngHeaderInfo png_header = png_reader.GetPngHeader();

			//Writer init
			std::cout << "\tInitializing Writer.";
			std::filesystem::path out_file_path = AddAppendixToFilename(in_file_path, "_chunks_copy");
			int num_tabs_in_warning = 3;
			PngWriter png_writer(out_file_path, png_header, WarningCallbackData(&PNGWarningHandler, &num_tabs_in_warning));
			std::cout << "\tDONE. Writer path:" << std::endl;
			std::cout << "\t\t" << out_file_path.string() << std::endl;

			//Number of chunks
			int num_chunks = png_header.GetHeight() / chunk_size;
			if (static_cast<unsigned int>(num_chunks * chunk_size) < png_header.GetHeight())
				num_chunks++;
			std::cout << "\tNumber of chunks to read is " << num_chunks << "." << std::endl;

			//Image that will be combined from the chunks
			//We read first chunk outside the loop to initialize the image
			std::cout << "\t\tReading chunk 0.\tNext line is " << png_reader.GetNextRowIndex() << "." << std::endl;
			ImageBuffer_Byte* combined_image = png_reader.ReadNextRows(chunk_size);
			std::cout << "\t\tWriting chunk 0.\tNext line is " << png_writer.GetNextRowIndex() << "." << std::endl;
			png_writer.WriteNextRows(combined_image);

			//All remaining chunks we do in a loop
			for (int chunk = 1; chunk < num_chunks; chunk++) {

				//Reading chunk
				std::cout << "\t\tReading chunk " << chunk << ":";
				std::cout << "\tNext line is " << png_reader.GetNextRowIndex() << ".";
				ImageBuffer_Byte* image_chunk = png_reader.ReadNextRows(chunk_size);
				std::cout << "\t" << image_chunk->GetHeight() << " lines were read." << std::endl;

				//Appending the chunk to combined image
				combined_image->Append(image_chunk);

				//Writing chunk
				std::cout << "\t\tWriting chunk " << chunk << ":";
				int rows_before = png_writer.GetNextRowIndex();
				std::cout << "\tNext line is " << rows_before << ".";
				png_writer.WriteNextRows(image_chunk);
				std::cout << "\t" << png_writer.GetNextRowIndex() - rows_before << " lines were written." << std::endl;

				//Disposing
				delete image_chunk;
			}

			// 3 -------------------------------------
			//Checking results

			//Checking states
			if (png_reader.IsFinished())
				std::cout << "\tPNG reader state is finished." << std::endl;
			else
				std::cout << "\tPNG reader state is NOT finished." << std::endl;
			std::cout << std::endl;

			std::cout << "\tCombined image info:" << std::endl;
			Printer::PrintImageBufferInfo(2, combined_image);

			//Checking images obtained by 2 different readings
			if (image_at_once->GetHeight() == combined_image->GetHeight() && image_at_once->GetWidth() == combined_image->GetWidth()) {
				std::cout << "\tImage dimensions match." << std::endl;
				int cmp_width = combined_image->GetWidth() * combined_image->GetNumCmp();
				long miss_counter = 0;

				uint8_t** cmb_data = combined_image->GetData();
				uint8_t** onc_data = image_at_once->GetData();

				for (int row = 0; row < combined_image->GetHeight(); row++)
					for (int cmp = 0; cmp < cmp_width; cmp++)
						if (cmb_data[row][cmp] != onc_data[row][cmp])
							miss_counter++;

				if (miss_counter == 0)
					std::cout << "\tImage data match." << std::endl;
				else

					std::cout << "\tImage data do not match " << miss_counter << " times." << std::endl;

			}
			else
				std::cout << "\tImage dimensions do not match. Images are not comparable." << std::endl;
			std::cout << std::endl;

			//Writing combined image
			std::filesystem::path out_file_path_combined = AddAppendixToFilename(in_file_path, "_chunks_combined_copy");
			std::cout << "\tWriting combined image. File path:" << std::endl;
			std::cout << "\t\t" << out_file_path_combined.string() << std::endl;
			PngWriter::WritePng(out_file_path_combined, combined_image, WarningCallbackData(&PNGWarningHandler, &num_tabs_in_warning));
			std::cout << "\tDONE writing!" << std::endl;
		}
		catch (std::ifstream::failure e) {
			std::cout << e.what() << std::endl;
			return;
		}
		catch (codec_fatal_exception e) {
			std::cout << e.GetFullMessage() << std::endl;
			return;
		}
	}


	/// <summary>
	/// 
	/// </summary>
	/// <param name="filename"></param>
	static void TestSRGBConversion(std::string filename) {
		Stopwatch watch;

		std::cout << "Testing SRG conversion by converting image to linear colorspace and back." << std::endl;

		//Opening the image
		ImageFileInfo src_image_info = OpenImage(1, filename);
		ImageBuffer_Byte* src_image = src_image_info.decompressed_image;
		std::cout << tabs(1) << "First 16 pixels of original image:" << std::endl;
		ImageBufferPrinter::PrintPixelsAt(1, 8, src_image, 0, 0, 16);

		//Producing srgb gamma converter
		GammaConverter* gconv = GammaDispatcher::GetConverter(RawImageGammaProfile::sRGB, NULL);
		
		//Converting to linear
		std::cout << tabs(1) << "Converting to Linear" << std::endl;
		watch.Start();
		ImageBuffer_uint16* lin_image = gconv->RemoveGammaCorretion(src_image);
		watch.Stop();
		std::cout << tabs(2) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;

		std::cout << tabs(1) << "First 16 pixels of linear image:" << std::endl;
		ImageBufferPrinter::PrintPixelsAt(1, 8, lin_image, 0, 0, 16);

		//Converting back and writing
		if (src_image_info.file_format == FileFormat::FF_JPEG) {
			std::cout << tabs(1) << "Converting back to sRGB:" << std::endl;
			watch.Start();
			ImageBuffer_Byte* trg_image = gconv->ApplyGammaCorrection(lin_image, BitDepth::BD_8_BIT);
			watch.Stop();
			std::cout << tabs(2) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;

			std::cout << tabs(1) << "First 16 pixels of converted back image:" << std::endl;
			ImageBufferPrinter::PrintPixelsAt(1, 8, trg_image, 0, 0, 16);

			JpegWriter::WriteJPEG(AddAppendixToFilename(src_image_info.file_path, "srgb_test"), trg_image, 100);
		}
		else {
			std::cout << "Only JPEG is currently supported, sorry" << std::endl;
		}
	}

private:



};