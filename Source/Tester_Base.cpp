#include "Tester_Base.h"

//--------------------------------
//	IO METHODS
//--------------------------------

//Opens an image
ImageFileInfo Tester_Base::OpenImage(int num_tabs, std::filesystem::path file_path) {
	Stopwatch watch;

	//Result
	ImageFileInfo image_info;

	std::cout << tabs(num_tabs) << "Opening \"" << file_path.filename().string() << "\"" << std::endl;

	image_info.file_path = file_path;

	
	try {
		//Reading jpeg
		if (IsJpeg_ByExtension(file_path)) {
			image_info.file_format = FileFormat::FF_JPEG;
			std::cout << tabs(num_tabs + 1) << "Reading and decompressing JPEG." << std::endl;
			int warning_args = num_tabs + 2;

			watch.Start();
			image_info.decompressed_image = 
				JpegReader::ReadJpegFile(
					file_path, 
					&image_info.jpeg_header, 
					WarningCallbackData(&JPEGWarningHandler, &warning_args));
			watch.Stop();
			std::cout << tabs(num_tabs + 1) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;
		
		}
		else {
		//Reading png
			if (IsPng_ByExtension(file_path)) {
				image_info.file_format = FileFormat::FF_PNG;
				std::cout << tabs(num_tabs + 1) << "Reading and decompressing PNG." << std::endl;
				int warning_args = num_tabs + 2;

				watch.Start();
				image_info.decompressed_image =
					PngReader::ReadPngFile(
						file_path, 
						&image_info.png_header, 
						WarningCallbackData(&PNGWarningHandler, &warning_args));
				watch.Stop();
				std::cout << tabs(num_tabs + 1) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;
			}
			else {
				image_info.file_format = FileFormat::FF_UNSUPPORTED;
				return image_info;
			}
		}
	}
	catch (std::ifstream::failure e) {
		std::cout << tabs(num_tabs + 2) << e.what() << std::endl;
		image_info.decompressed_image = NULL;
		return image_info;
	}
	catch (codec_fatal_exception e) {
		std::cout << tabs(num_tabs + 2) << e.GetFullMessage() << std::endl;
		image_info.decompressed_image = NULL;
		return image_info;
	}


	Printer::PrintImageFileInfo(num_tabs + 1, image_info);

	return image_info;
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