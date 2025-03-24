#include "Tester_Base.h"
#include "GammaConverter.h"
#include "GammaDispatcher.h"

//--------------------------------
//	IO METHODS
//--------------------------------

//Opens an image
//Returns image as a reference and writes image info object to given pointer
ImageBuffer_Byte Tester_Base::OpenImage(int num_tabs, std::filesystem::path file_path, ImageFileInfo* info_ptr) {
	Stopwatch watch;

	std::cout << tabs(num_tabs) << "Opening \"" << file_path.filename().string() << "\"" << std::endl;
	
	FileFormat format = GetImageTypeByExpension(file_path);

	if (format == FileFormat::FF_UNSUPPORTED)
		throw std::runtime_error("Trying to open file that is not an image.");

	ImageFileInfo image_info(file_path, format);

	try {
		//Reading jpeg
		if (format == FileFormat::FF_JPEG) {
			std::cout << tabs(num_tabs + 1) << "Reading and decompressing JPEG." << std::endl;
			int warning_args = num_tabs + 2;

			JpegHeaderInfo header;

			watch.Start();
			ImageBuffer_Byte decompressed_image = 
				JpegReader::ReadJpegFile(
					file_path, 
					&header,
					WarningCallbackData(&JPEGWarningHandler, &warning_args));
			watch.Stop();
			std::cout << tabs(num_tabs + 1) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;
			Printer::EmptyLine();

			//Filling image info
			ImageBufferInfo buf_info(decompressed_image.GetHeight(), decompressed_image.GetWidth(), decompressed_image.GetLayout(), decompressed_image.GetBitPerComponent());
			image_info.SetImgBufferInfo(buf_info);
			image_info.SetJpegHeader(header);
			*info_ptr = std::move(image_info);

			Printer::PrintImageFileInfo(num_tabs + 1, image_info);
			Printer::EmptyLine();

			return decompressed_image;
		}
		//Reading png
		if (format == FileFormat::FF_PNG) {
			std::cout << tabs(num_tabs + 1) << "Reading and decompressing PNG." << std::endl;
			int warning_args = num_tabs + 2;

			PngHeaderInfo header;

			watch.Start();
			ImageBuffer_Byte decompressed_image =
				PngReader::ReadPngFile(
					file_path, 
					&header, 
					WarningCallbackData(&PNGWarningHandler, &warning_args));
			watch.Stop();
			std::cout << tabs(num_tabs + 1) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;
			Printer::EmptyLine();

			//Filling image info
			ImageBufferInfo buf_info(decompressed_image.GetHeight(), decompressed_image.GetWidth(), decompressed_image.GetLayout(), decompressed_image.GetBitPerComponent());
			image_info.SetImgBufferInfo(buf_info);
			image_info.SetPngHeader(header);
			*info_ptr = std::move(image_info);

			Printer::PrintImageFileInfo(num_tabs + 1, image_info);
			Printer::EmptyLine();

			return decompressed_image;
		}
	}
	catch (std::ifstream::failure e) {
		std::cout << tabs(num_tabs + 2) << e.what() << std::endl;
		throw e;
	}
	catch (codec_fatal_exception e) {
		std::cout << tabs(num_tabs + 2) << e.GetFullMessage() << std::endl;
		throw e;
	}
}



/// <summary>
/// Opens an image and removes gamma.
/// </summary>
ImageBuffer_uint16 Tester_Base::OpenImageAndRemoveGamma(int num_tabs, std::filesystem::path file_path, ImageFileInfo* info_ptr) {
	Stopwatch watch;

	//Reading the image
	ImageBuffer_Byte corrected_image = OpenImage(num_tabs, file_path, info_ptr);

	//Removing gamma correction (sRGB assumed)
	std::cout << tabs(num_tabs + 1) << "Removing gamma correction (sRGB assummed)." << std::endl;
	watch.Start();
	GammaConverter* gc = GammaDispatcher::GetConverter(RawImageGammaProfile::sRGB, nullptr);
	ImageBuffer_uint16 linear_image = gc->RemoveGammaCorrection(corrected_image);
	watch.Stop();
	std::cout << tabs(num_tabs + 1) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;
	Printer::EmptyLine();

	return linear_image;
}




/// <summary>
/// Writes image at given path.
/// </summary>
void Tester_Base::WriteImage(int num_tabs, const ImageBuffer_Byte& image, const ImageFileInfo& info) {
	Stopwatch watch;

	std::cout << tabs(num_tabs) << "Writing \"" << info.GetFilePath().filename().string() << "\"" << std::endl;

	Printer::PrintImageFileInfo(num_tabs + 1, info);
	Printer::EmptyLine();

	try {
		// Not writing
		if (info.GetFileFormat() == FileFormat::FF_UNSUPPORTED)
			throw std::runtime_error("Trying to write image file of unknown type.");

		// Writing jpeg
		if (info.GetFileFormat() == FileFormat::FF_JPEG) {
			std::cout << tabs(num_tabs + 1) << "Compressing and writing JPEG." << std::endl;
			int warning_args = num_tabs + 2;

			watch.Start();
			JpegWriter::WriteJPEG(
				info.GetFilePath(),
				image,
				info.GetJpegHeader(),
				100,
				WarningCallbackData(&JPEGWarningHandler, &warning_args));
			watch.Stop();
			std::cout << tabs(num_tabs + 1) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;
			Printer::EmptyLine();

			return;
		}
		//Reading png
		if (info.GetFileFormat() == FileFormat::FF_PNG) {
			std::cout << tabs(num_tabs + 1) << "Compressing and writing PNG." << std::endl;
			int warning_args = num_tabs + 2;

			PngHeaderInfo header;

			watch.Start();
			PngWriter::WritePng(
				info.GetFilePath(),
				image,
				info.GetPngHeader(),
				WarningCallbackData(&JPEGWarningHandler, &warning_args));
			watch.Stop();
			std::cout << tabs(num_tabs + 1) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;
			Printer::EmptyLine();

			return;
		}
	}
	catch (std::ifstream::failure e) {
		std::cout << tabs(num_tabs + 2) << e.what() << std::endl;
		throw e;
	}
	catch (codec_fatal_exception e) {
		std::cout << tabs(num_tabs + 2) << e.GetFullMessage() << std::endl;
		throw e;
	}
}


/// <summary>
/// Applies gamma and writes back image.
/// </summary>
void Tester_Base::ApplyGammaAndWriteImage(int num_tabs, const ImageBuffer_uint16& image, const ImageFileInfo& info) {
	Stopwatch watch;

	// Applying gamma correction (sRGB assumed)
	std::cout << tabs(num_tabs) << "Applying gamma correction (sRGB assummed)." << std::endl;
	watch.Start();
	GammaConverter* gc = GammaDispatcher::GetConverter(RawImageGammaProfile::sRGB, nullptr);
	ImageBuffer_Byte corrected_image = gc->ApplyGammaCorrection(image, info.GetImgBufferInfo().GetBitDepth());
	watch.Stop();
	std::cout << tabs(num_tabs) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;
	Printer::EmptyLine();

	//Reading the image
	WriteImage(num_tabs, corrected_image, info);

	return;
}


//--------------------------------
//	PATH MANIPULATION
//--------------------------------

/// <summary>
/// Adds appendix to the filename in given path
/// </summary>
std::filesystem::path Tester_Base::AddAppendixToFilename(std::filesystem::path source_path, std::string appendix) {
	std::filesystem::path out_path(source_path);
	std::string out_filename = source_path.stem().string() + appendix + source_path.extension().string();
	out_path.replace_filename(out_filename);
	return out_path;
}



/// <summary>
/// Creates output directory in TestImages.
/// Returns path to the output folder.
/// </summary>
std::filesystem::path Tester_Base::CreateOutputFolder(std::string out_folder_name) {

	//Getting string of current date and time
	//This is kind of ridicuosly complicated
	std::chrono::time_point current_time_chrono = std::chrono::system_clock::now(); //Taking current time
	std::time_t current_time_t = std::chrono::system_clock::to_time_t(current_time_chrono); //Converting to old C format
	std::tm current_time_tm; //This structure breaks down time value to components
	localtime_s(&current_time_tm, &current_time_t); //Filling the structure
	std::stringstream cur_time_ss; //This string stream can receive formatted output
	cur_time_ss << std::put_time(&current_time_tm, "%Y%m%d%H%M%S"); //Formatting tm object to string
	std::string current_time_string = cur_time_ss.str(); //Extracting string from the stream

	//Constructing output file path
	std::filesystem::path out_folder_path(std::string(TEST_IMAGES_PATH_STR) + std::string("\\") + "OUT_" + out_folder_name + std::string("_") + current_time_string + std::string("\\"));

	//Creating ouput directory on the disc
	std::filesystem::create_directory(out_folder_path);

	return out_folder_path;
}



//--------------------------------
//	UTILITY
//--------------------------------

/// <summary>
/// Returns true if file path points to JPEG a file. Only checks extension, not actual file contents.
/// </summary>
/// <returns></returns>
bool Tester_Base::IsJpeg_ByExtension(std::filesystem::path file_path) {
	//Extracting extension as a string (including the dot)
	std::string extension = file_path.extension().string();

	//Converting extension to lowercase for uniformity
	for (int i = 0; i < extension.length(); i++)
		extension[i] = tolower(extension[i]);

	//Checking 
	if (extension == ".jpeg" || extension == ".jpg")
		return true;
	else
		return false;
}



/// <summary>
/// Returns true if file path points to PNG a file. Only checks extension, not actual file contents.
/// </summary>
/// <returns></returns>
bool Tester_Base::IsPng_ByExtension(std::filesystem::path file_path) {
	//Extracting extension as a string (including the dot)
	std::string extension = file_path.extension().string();

	//Converting extension to lowercase for uniformity
	for (int i = 0; i<extension.length(); i++)
		extension[i] = tolower(extension[i]);

	//Checking 
	if (extension == ".png")
		return true;
	else
		return false;
}


/// <summary>
/// Checks file extension and returns image file type this extension indicates.
/// </summary>
FileFormat Tester_Base::GetImageTypeByExpension(std::filesystem::path file_path) {
	//Extracting extension as a string (including the dot)
	std::string extension = file_path.extension().string();

	//Converting extension to lowercase for uniformity
	for (int i = 0; i < extension.length(); i++)
		extension[i] = tolower(extension[i]);

	//Checking 
	if (extension == ".png")
		return FileFormat::FF_PNG;
	if (extension == ".jpeg" || extension == ".jpg")
		return FileFormat::FF_JPEG;

	return FileFormat::FF_UNSUPPORTED;
}

