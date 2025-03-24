#include "Tester_IO.h"

//--------------------------------
//	JPEG TESTERS
//--------------------------------

/// <summary>
/// Legacy? I honestly don't remember if this should have been deleted.
/// </summary>
void Tester_IO::TestJpegReader_Simple(std::string file_path) {
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
		ImageBuffer_Byte image = reader.ReadNextRows(reader.GetJpegHeader().GetHeight());
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
		ImageBuffer_Byte image = reader.ReadNextRows(reader.GetJpegHeader().GetHeight());
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
	ImageBuffer_Byte image = ImageGenerator::CircularGradient(1600, 1000, BitDepth::BD_8_BIT);
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
		JpegHeaderInfo jpeg_header(image.GetHeight(), image.GetWidth(), 3, J_COLOR_SPACE::JCS_RGB);

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



/// <summary>
/// Opens and decompresses JPEG file and writes it back.
/// </summary>
void Tester_IO::TestJpegIOAtOnce(std::string file_path) {

	//Creating file path object
	std::filesystem::path in_file_path(std::string(TEST_IMAGES_PATH_STR) + "\\" + file_path);
	//Creating file folder for output
	std::filesystem::path out_dir_path = CreateOutputFolder("TestJpegIOAtOnce");

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
		std::cout << "\tReading the file." << std::endl;
		ImageBuffer_Byte image = JpegReader::ReadJpegFile(in_file_path, &in_header, warning_callback_data);

		//File info
		std::cout << "\tReading is finished." << std::endl;
		std::cout << "\t\tImage object description is:" << std::endl;
		Printer::PrintImageBufferInfo(3, image);
		std::cout << "\t\tJpeg header description is:" << std::endl;
		Printer::PrintJpegHeader(3, in_header);
		Printer::EmptyLine();

		//Writing file

		//Path for output file
		std::filesystem::path out_file_path(out_dir_path);
		out_file_path.replace_filename(in_file_path.filename());
		out_file_path = AddAppendixToFilename(out_file_path, "_copy");
		std::cout << "\tWriting JPEG image back at once." << std::endl;
		Printer::PrintFilePath(1, out_file_path);
		Printer::EmptyLine();

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
/// Tests reading and writing jpeg image by chuncks.
/// </summary>
void Tester_IO::TestJpegReaderByChunks(std::string file_path, int chunk_size) {
	//Creating file path object
	std::filesystem::path in_file_path(std::string(TEST_IMAGES_PATH_STR) + "\\" + file_path);
	//Creating file folder for output
	std::filesystem::path out_dir_path = CreateOutputFolder("TestJpegReaderByChunks");

	//Intro
	std::cout << "Reading and writing back JPEG image file by chunks with chunk size of " << chunk_size << " rows." << std::endl;
	std::cout << "\tFile path is:" << std::endl;
	std::cout << "\t\t" << in_file_path << std::endl;
	std::cout << std::endl;

	if (IsJpeg_ByExtension(std::filesystem::path(file_path)) == false) {
		std::cout << "\tFile is not a JPEG by extension." << std::endl;
		return;
	}

	// 1 ---------------------
	//Reading the file at once
	std::cout << "\tReading the file at once." << std::endl;
	ImageBuffer_Byte image_at_once(1, 1, ImagePixelLayout::G, BitDepth::BD_8_BIT, false); //Empty
	try {
		image_at_once = std::move(JpegReader::ReadJpegFile(in_file_path));
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
		std::cout << "\tInitializing Reader.";
		JpegReader jpeg_reader(in_file_path);
		std::cout << "\tDONE." << std::endl;

		JpegHeaderInfo jpeg_header = jpeg_reader.GetJpegHeader();

		//Writer init
		std::cout << "\tInitializing Writer.";
		//Path for output file
		std::filesystem::path out_file_path(out_dir_path);
		out_file_path.replace_filename(in_file_path.filename());
		out_file_path = AddAppendixToFilename(out_file_path, "_chunks_copy");
		int num_tabs_in_warning = 3;
		JpegWriter jpeg_writer(out_file_path, jpeg_header, 100, WarningCallbackData(&JPEGWarningHandler, &num_tabs_in_warning));
		std::cout << "\tDONE. Writer path:" << std::endl;
		std::cout << "\t\t" << out_file_path.string() << std::endl;

		//Number of chunks
		int num_chunks = jpeg_header.GetHeight() / chunk_size;
		if (static_cast<unsigned int>(num_chunks * chunk_size) < jpeg_header.GetHeight())
			num_chunks++;
		std::cout << "\tNumber of chunks to read is " << num_chunks << "." << std::endl;

		//Image that will be combined from the chunks
		//We read first chunk outside the loop to initialize the image
		std::cout << "\t\tReading chunk 0.\tNext line is " << jpeg_reader.GetNextRowIndex() << "." << std::endl;
		ImageBuffer_Byte combined_image = jpeg_reader.ReadNextRows(chunk_size);
		std::cout << "\t\tWriting chunk 0.\tNext line is " << jpeg_writer.GetNextRowIndex() << "." << std::endl;
		jpeg_writer.WriteNextRows(combined_image);

		//All remaining chunks we do in a loop
		for (int chunk = 1; chunk < num_chunks; chunk++) {

			//Reading chunk
			std::cout << "\t\tReading chunk " << chunk << ":";
			std::cout << "\tNext line is " << jpeg_reader.GetNextRowIndex() << ".";
			ImageBuffer_Byte image_chunk = jpeg_reader.ReadNextRows(chunk_size);
			std::cout << "\t" << image_chunk.GetHeight() << " lines were read." << std::endl;

			//Appending the chunk to combined image
			combined_image.Append(image_chunk);

			//Writing chunk
			std::cout << "\t\tWriting chunk " << chunk << ":";
			int rows_before = jpeg_writer.GetNextRowIndex();
			std::cout << "\tNext line is " << rows_before << ".";
			jpeg_writer.WriteNextRows(image_chunk);
			std::cout << "\t" << jpeg_writer.GetNextRowIndex() - rows_before << " lines were written." << std::endl;
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
		if (image_at_once.GetHeight() == combined_image.GetHeight() && image_at_once.GetWidth() == combined_image.GetWidth()) {
			std::cout << "\tImage dimensions match." << std::endl;
			int cmp_width = combined_image.GetWidth() * combined_image.GetNumCmp();
			long miss_counter = 0;

			uint8_t** cmb_data = combined_image.GetDataPtr();
			uint8_t** onc_data = image_at_once.GetDataPtr();

			for (int row = 0; row < combined_image.GetHeight(); row++)
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
		//Path for output file
		std::filesystem::path out_file_path_combined(out_file_path);
		out_file_path_combined.replace_filename(in_file_path.filename());
		out_file_path_combined = AddAppendixToFilename(out_file_path_combined, "_chunks_combined_copy");
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
		ImageBuffer_Byte image = reader.ReadNextRows(reader.GetPngHeader().GetHeight());
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
	ImageBuffer_Byte image = ImageGenerator::CircularGradient(img_height, img_width, BitDepth::BD_8_BIT);
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
		PngHeaderInfo png_header(image.GetHeight(), image.GetWidth(), 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE);

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



/// <summary>
/// Opens and decompresses PNG file and writes it back.
/// </summary>
/// <param name="file_path"></param>
void Tester_IO::TestPngIOAtOnce(std::string file_path) {
	//Creating file path object
	std::filesystem::path in_file_path(std::string(TEST_IMAGES_PATH_STR) + "\\" + file_path);
	//Creating file folder for output
	std::filesystem::path out_dir_path = CreateOutputFolder("TestPngIOAtOnce");

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
		ImageBuffer_Byte image = PngReader::ReadPngFile(in_file_path, &png_header, warning_callback_data);

		//File info
		std::cout << "\tReading is finished." << std::endl;
		std::cout << "\t\tImage object description is:" << std::endl;
		Printer::PrintImageBufferInfo(3, image);
		std::cout << std::endl;
		//std::cout << "\t\tpng header description is:" << std::endl;
		//Printer::PrintPngHeader(3, png_header);

		//Writing file
		//Path for output file
		std::filesystem::path out_file_path(out_dir_path);
		out_file_path.replace_filename(in_file_path.filename());
		out_file_path = AddAppendixToFilename(out_file_path, "_copy");
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
/// Tests reading and writing PNG files in chunks.
/// </summary>
void Tester_IO::TestPngReaderByChunks(std::string file_path, int chunk_size) {
	//Creating file path object
	std::filesystem::path in_file_path(std::string(TEST_IMAGES_PATH_STR) + "\\" + file_path);
	//Creating file folder for output
	std::filesystem::path out_dir_path = CreateOutputFolder("TestPngByChunks");

	//Path for output file
	std::filesystem::path out_file_path(out_dir_path);
	out_file_path.replace_filename(in_file_path.filename());
	out_file_path = AddAppendixToFilename(out_file_path, "_copy");

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
	ImageBuffer_Byte image_at_once(1, 1, ImagePixelLayout::G, BitDepth::BD_8_BIT, false); //Empty
	try {
		image_at_once = std::move(PngReader::ReadPngFile(in_file_path));
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
	std::cout << "\tReading is finished. Image object description is:" << std::endl;
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
		//Path for output file
		std::filesystem::path out_file_path(out_dir_path);
		out_file_path.replace_filename(in_file_path.filename());
		out_file_path = AddAppendixToFilename(out_file_path, "_chunks_copy");
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
		ImageBuffer_Byte combined_image = png_reader.ReadNextRows(chunk_size);
		std::cout << "\t\tWriting chunk 0.\tNext line is " << png_writer.GetNextRowIndex() << "." << std::endl;
		png_writer.WriteNextRows(combined_image);

		//All remaining chunks we do in a loop
		for (int chunk = 1; chunk < num_chunks; chunk++) {

			//Reading chunk
			std::cout << "\t\tReading chunk " << chunk << ":";
			std::cout << "\tNext line is " << png_reader.GetNextRowIndex() << ".";
			ImageBuffer_Byte image_chunk = png_reader.ReadNextRows(chunk_size);
			std::cout << "\t" << image_chunk.GetHeight() << " lines were read." << std::endl;

			//Appending the chunk to combined image
			combined_image.Append(image_chunk);

			//Writing chunk
			std::cout << "\t\tWriting chunk " << chunk << ":";
			int rows_before = png_writer.GetNextRowIndex();
			std::cout << "\tNext line is " << rows_before << ".";
			png_writer.WriteNextRows(image_chunk);
			std::cout << "\t" << png_writer.GetNextRowIndex() - rows_before << " lines were written." << std::endl;
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
		if (image_at_once.GetHeight() == combined_image.GetHeight() && image_at_once.GetWidth() == combined_image.GetWidth()) {
			std::cout << "\tImage dimensions match." << std::endl;
			int cmp_width = combined_image.GetWidth() * combined_image.GetNumCmp();
			long miss_counter = 0;

			uint8_t** cmb_data = combined_image.GetDataPtr();
			uint8_t** onc_data = image_at_once.GetDataPtr();

			for (int row = 0; row < combined_image.GetHeight(); row++)
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
		std::filesystem::path out_file_path_combined(out_dir_path);
		out_file_path_combined.replace_filename(in_file_path.filename());
		out_file_path_combined = AddAppendixToFilename(out_file_path_combined, "_chunks_combined_copy");
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