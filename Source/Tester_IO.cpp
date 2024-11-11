#include "Tester_IO.h"

//--------------------------------
//	JPEG TESTERS
//--------------------------------

/// <summary>
/// Tests JPEG reader by reading a file.
/// Image is assumed to be in TestImages folder.
/// </summary>
void Tester_IO::TestJpegReader(std::string image_name) {
	Stopwatch watch;

	//Creating file path object
	std::filesystem::path in_file_path(std::string(TEST_IMAGES_PATH_STR) + "\\" + image_name);

	//Intro
	std::cout << "TEST: Reading JPEG file and printing results." << std::endl;
	std::cout << "\tFile path is:" << std::endl;
	Printer::PrintFilePath(2, in_file_path);
	Printer::EmptyLine();

	if (IsJpeg_ByExtension(in_file_path) == false) {
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
		watch.Start();
		std::cout << "\tInitializing JPEG reader (opening the file and reading the header):" << std::endl;
		JpegReader reader(in_file_path, warning_callback_data);
		watch.Stop();
		std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
		Printer::EmptyLine();

		std::cout << "\tPrinting JPEG file header: " << std::endl;
		Printer::PrintJpegHeader(2, reader.GetJpegHeader());
		Printer::EmptyLine();

		//Reading file lines
		std::cout << "\tReading all the lines of the image at once and decompressing JPEG:" << std::endl;
		watch.Start();
		ImageBuffer_Byte* image = reader.ReadNextRows(reader.GetJpegHeader().GetHeight());
		watch.Stop();
		std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
		Printer::EmptyLine();

		//Image info
		std::cout << "\tImage object description is:" << std::endl;
		Printer::PrintImageBufferInfo(2, image);
		Printer::EmptyLine();

		//Image contents
		ImageBufferPrinter::PrintPixelsAt(1, 8, image, 0, 0, 16);
	}
	catch (std::ifstream::failure e) {
		std::cout << "\tifstream::failure exception encountered with the message:" << std::endl;
		std::cout << "\t\t" << e.what() << std::endl;
	}
	catch (codec_fatal_exception e) {
		std::cout << "\tcodec fatal exception encountered with the message:" << std::endl;
		std::cout <<"\t\t" << e.GetFullMessage() << std::endl;
	}

	//Outro
	std::cout << "JPEG reading test is finished." << std::endl;
	std::cout << "--------------------------------" << std::endl;
	Printer::EmptyLine();
}


/// <summary>
/// Tests JPEG writer by generating and writing an image.
/// </summary>
void Tester_IO::TestJpegWriter() {
	Stopwatch watch;

	std::string file_name = "circular_gradient.jpg";

	//Creating file folder for output
	std::filesystem::path out_dir_path = CreateOutputFolder("TestJpegWriter");
	//Path for output file
	std::filesystem::path out_file_path(out_dir_path);
	out_file_path.replace_filename(std::filesystem::path(file_name));

	//Intro
	std::cout << "TEST: Writing generated image to JPEG file." << std::endl;
	std::cout << "\tFile path is:" << std::endl;
	Printer::PrintFilePath(2, out_file_path);
	Printer::EmptyLine();

	//Generating the image to write
	std::cout << "\tGenerating the image - circular gradient" << std::endl;
	watch.Start();
	ImageBuffer_Byte* image = ImageGenerator::CircularGradient(1600, 1000, BitDepth::BD_8_BIT);
	watch.Stop();
	std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
	Printer::EmptyLine();

	//Image info
	std::cout << "\tImage object information:" << std::endl;
	Printer::PrintImageBufferInfo(2, image);
	Printer::EmptyLine();

	try {
		//Warning handler
		int warning_tabs = 2;
		WarningCallbackData warning_callback_data(&JPEGWarningHandler, &warning_tabs);

		//Header container
		JpegHeaderInfo jpeg_header(image->GetHeight(), image->GetWidth(), 3, J_COLOR_SPACE::JCS_RGB);

		//Printing the header
		std::cout << "\tWe will try to write JPEG with following header: " << std::endl;
		Printer::PrintJpegHeader(2, jpeg_header);
		Printer::EmptyLine();

		//Reader init
		watch.Start();
		std::cout << "\tInitializing JPEG writer (opening the file and writing the header):" << std::endl;
		JpegWriter writer(out_file_path, jpeg_header, 100, warning_callback_data);
		watch.Stop();
		std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
		Printer::EmptyLine();

		//Writing file lines
		std::cout << "\tCompressing all the lines at once and writing the JPEG:" << std::endl;
		watch.Start();
		writer.WriteNextRows(image);
		watch.Stop();
		std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
		Printer::EmptyLine();
	}
	catch (std::ifstream::failure e) {
		std::cout << "\tifstream::failure exception encountered with the message:" << std::endl;
		std::cout << "\t\t" << e.what() << std::endl;
	}
	catch (codec_fatal_exception e) {
		std::cout << "\tcodec fatal exception encountered with the message:" << std::endl;
		std::cout << "\t\t" << e.GetFullMessage() << std::endl;
	}

	//Outro
	std::cout << "JPEG writing test is finished." << std::endl;
	std::cout << "--------------------------------" << std::endl;
	Printer::EmptyLine();
}




//--------------------------------
//	PNG TESTERS
//--------------------------------


/// <summary>
/// Tests PNG reader by reading a file.
/// Image is assumed to be in TestImages folder.
/// </summary>
void Tester_IO::TestPngReader(std::string image_name) {
	Stopwatch watch;

	//Creating file path object
	std::filesystem::path in_file_path(std::string(TEST_IMAGES_PATH_STR) + "\\" + image_name);

	//Intro
	std::cout << "TEST: Reading PNG file and printing results." << std::endl;
	std::cout << "\tFile path is:" << std::endl;
	Printer::PrintFilePath(2, in_file_path);
	Printer::EmptyLine();

	if (IsPng_ByExtension(in_file_path) == false) {
		std::cout << "\tAbort: File is not a PNG by extension." << std::endl;
		return;
	}

	try {
		//Warning handler
		int warning_tabs = 2;
		WarningCallbackData warning_callback_data(&PNGWarningHandler, &warning_tabs);

		//Reader init
		watch.Start();
		std::cout << "\tInitializing PNG reader (opening the file and reading the header):" << std::endl;
		PngReader reader(in_file_path, warning_callback_data);
		watch.Stop();
		std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
		Printer::EmptyLine();

		std::cout << "\tPrinting PNG file header: " << std::endl;
		Printer::PrintPngHeader(2, reader.GetPngHeader());
		Printer::EmptyLine();

		//Reading file lines
		std::cout << "\tReading all the lines of the image at once and decompressing PNG:" << std::endl;
		watch.Start();
		ImageBuffer_Byte* image = reader.ReadNextRows(reader.GetPngHeader().GetHeight());
		watch.Stop();
		std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
		Printer::EmptyLine();

		//Image info
		std::cout << "\tImage object description is:" << std::endl;
		Printer::PrintImageBufferInfo(2, image);
		Printer::EmptyLine();

		//Image contents
		ImageBufferPrinter::PrintPixelsAt(1, 4, image, 0, 0, 16);
	}
	catch (std::ifstream::failure e) {
		std::cout << "\tifstream::failure exception encountered with the message:" << std::endl;
		std::cout << "\t\t" << e.what() << std::endl;
	}
	catch (codec_fatal_exception e) {
		std::cout << "\tcodec fatal exception encountered with the message:" << std::endl;
		std::cout << "\t\t" << e.GetFullMessage() << std::endl;
	}

	//Outro
	std::cout << "PNG reading test is finished." << std::endl;
	std::cout << "--------------------------------" << std::endl;
	Printer::EmptyLine();
}



/// <summary>
/// Tests Png writer by generating and writing an image.
/// </summary>
void Tester_IO::TestPngWriter() {
	Stopwatch watch;

	std::string file_name = "circular_gradient.png";

	//Creating file folder for output
	std::filesystem::path out_dir_path = CreateOutputFolder("TestPngWriter");
	//Path for output file
	std::filesystem::path out_file_path(out_dir_path);
	out_file_path.replace_filename(std::filesystem::path(file_name));

	//Intro
	std::cout << "TEST: Writing generated image to PNG file." << std::endl;
	std::cout << "\tFile path is:" << std::endl;
	Printer::PrintFilePath(2, out_file_path);
	Printer::EmptyLine();

	//Image parameters
	int img_height = 1600;
	int img_width = 1000;

	//Generating the image to write
	std::cout << "\tGenerating the image - circular gradient" << std::endl;
	watch.Start();
	ImageBuffer_Byte* image = ImageGenerator::CircularGradient(img_height, img_width, BitDepth::BD_8_BIT);
	watch.Stop();
	std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
	Printer::EmptyLine();

	//Image info
	std::cout << "\tImage object information:" << std::endl;
	Printer::PrintImageBufferInfo(2, image);
	Printer::EmptyLine();

	try {
		//Warning handler
		int warning_tabs = 2;
		WarningCallbackData warning_callback_data(&PNGWarningHandler, &warning_tabs);

		//Header container
		PngHeaderInfo png_header(image->GetHeight(), image->GetWidth(), 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE);

		//Printing the header
		std::cout << "\tWe will try to write a PNG with the following header: " << std::endl;
		Printer::PrintPngHeader(2, png_header);
		Printer::EmptyLine();

		//Reader init
		watch.Start();
		std::cout << "\tInitializing PNG writer (opening the file and writing the header):" << std::endl;
		PngWriter writer(out_file_path, png_header, warning_callback_data);
		watch.Stop();
		std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
		Printer::EmptyLine();

		//Writing file lines
		std::cout << "\tCompressing all the lines at once and writing the PNG:" << std::endl;
		watch.Start();
		writer.WriteNextRows(image);
		watch.Stop();
		std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
		Printer::EmptyLine();
	}
	catch (std::ifstream::failure e) {
		std::cout << "\tifstream::failure exception encountered with the message:" << std::endl;
		std::cout << "\t\t" << e.what() << std::endl;
	}
	catch (codec_fatal_exception e) {
		std::cout << "\tcodec fatal exception encountered with the message:" << std::endl;
		std::cout << "\t\t" << e.GetFullMessage() << std::endl;
	}

	//Outro
	std::cout << "PNG writing test is finished." << std::endl;
	std::cout << "--------------------------------" << std::endl;
	Printer::EmptyLine();
}




