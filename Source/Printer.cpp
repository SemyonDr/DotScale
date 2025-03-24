#include "Printer.h"

//--------------------------------
//	OUTSIDE OF CLASS PRINTING METHODS
//--------------------------------

/// <summary>
/// Prints given number of tabs.
/// </summary>
/// <param name="num"></param>
void PrintTabs(int num) {
	const int max_tabs = 32;
	if (num > max_tabs)
		num = max_tabs;
	for (int i = 0; i < num; i++)
		std::cout << "\t";
};

/// <summary>
/// Returns given number of tabs as a string.
/// Max 16 tabs.
/// </summary>
std::string tabs(int num) {
	if (num <= 0) return tabs_str_array[0];

	if (num >= 16) return tabs_str_array[16];

	return tabs_str_array[num];
};


/// <summary>
/// Prints info about image file.
/// </summary>
void Printer::PrintImageFileInfo(int num_tabs, const ImageFileInfo& image_info) {
	
	FileFormat format = image_info.GetFileFormat();

	if (format == FileFormat::FF_UNSUPPORTED) {
		std::cout << tabs(num_tabs) << "File format is Unsupported." << std::endl;
		return;
	}
	if (format == FileFormat::FF_JPEG) {
		ImageBufferInfo buffer_info = image_info.GetImgBufferInfo();
		std::cout << tabs(num_tabs) << "File format is JPEG." << std::endl;
		std::cout << tabs(num_tabs) << "Dimensions [HxW]:\t\t" << buffer_info.GetHeight() << "x" << buffer_info.GetWidth() << std::endl;
		std::cout << tabs(num_tabs) << "Decompressed data layout:\t" << EnumToString::LayoutToString(buffer_info.GetLayout()) << std::endl;
		std::cout << tabs(num_tabs) << "JPEG data layout:\t\t" << EnumToString::JpegLayoutToString(image_info.GetJpegHeader().GetColorSpace()) << std::endl;
		std::cout << tabs(num_tabs) << "Bit Depth:\t\t\t" << "8 bit" << std::endl;
		return;
	}
	if (format == FileFormat::FF_PNG) {
		ImageBufferInfo buffer_info = image_info.GetImgBufferInfo();
		std::cout << tabs(num_tabs) << "File format is PNG." << std::endl;
		std::cout << tabs(num_tabs) << "Dimensions [HxW]:\t\t" << buffer_info.GetHeight() << "x" << buffer_info.GetWidth() << std::endl;
		std::cout << tabs(num_tabs) << "Decompressed data layout:\t" << EnumToString::LayoutToString(buffer_info.GetLayout()) << std::endl;
		std::cout << tabs(num_tabs) << "PNG data layout:\t\t" << EnumToString::PngLayoutToString(image_info.GetPngHeader().GetPngColorType()) << std::endl;
		std::cout << tabs(num_tabs) << "Bit Depth:\t\t\t" << EnumToString::BitDepthToString(buffer_info.GetBitDepth()) << std::endl;
		return;
	}
}

/// <summary>
/// Prints info about image buffer object.
/// </summary>
/// <param name="num_tabs"></param>
/// <param name="image"></param>
void Printer::PrintImageBufferInfo(int num_tabs, const ImageBuffer_Byte& image) {
	std::cout << tabs(num_tabs) << "Dimensions [HxW]:\t" << image.GetHeight() << "x" << image.GetWidth() << std::endl;
	std::cout << tabs(num_tabs) << "Data layout:\t\t" << EnumToString::LayoutToString(image.GetLayout()) << std::endl;
	std::cout << tabs(num_tabs) << "Bit Depth:\t\t" << EnumToString::BitDepthToString(image.GetBitPerComponent()) << std::endl;
}



/// <summary>
/// Print contents of a jpeg header.
/// </summary>
void Printer::PrintJpegHeader(int num_tabs, const JpegHeaderInfo& header) {
	std::cout << tabs(num_tabs) << "Dimensions [HxW]:\t" << header.GetHeight() << "x" << header.GetWidth() << std::endl;
	std::cout << tabs(num_tabs) << "JPEG Color Space:\t" << EnumToString::JpegLayoutToString(header.GetColorSpace()) << std::endl;
	std::cout << tabs(num_tabs) << "Number of components:\t" << header.GetNumComponents() << std::endl;
}




/// <summary>
/// Prins contents of a png header.
/// </summary>
void Printer::PrintPngHeader(int num_tabs, const PngHeaderInfo& header) {
	std::cout << tabs(num_tabs) << "Dimensions [HxW]:\t" << header.GetHeight() << "x" << header.GetWidth() << std::endl;
	std::cout << tabs(num_tabs) << "PNG File Layout:\t" << EnumToString::PngLayoutToString(header.GetPngColorType()) << std::endl;
	std::cout << tabs(num_tabs) << "Bit depth:\t\t" << static_cast<int>(header.GetBitDepth()) << " bit per component" << std::endl;
	std::cout << tabs(num_tabs) << "Interlace type:\t\t" << EnumToString::PngInterlaceTypeToString(header.GetPngInterlaceType()) << std::endl;
}




/// <summary>
/// Prints file path.
/// </summary>
void Printer::PrintFilePath(int num_tabs, const std::filesystem::path& file_path) {
	std::cout << tabs(num_tabs) << file_path.string() << std::endl;
	if (file_path.is_absolute() == false)
		std::cout << tabs(num_tabs) << std::filesystem::absolute(file_path).string() << std::endl;
}