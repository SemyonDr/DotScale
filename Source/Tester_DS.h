#pragma once

#include "Tester_Base.h"
#include "GammaDispatcher.h"
#include "Downscaler.h"

class Tester_DS : Tester_Base {
public:

	static void Test_DownscalerSliced(uint32_t slice_height, double factor, std::string file_path) {
		Stopwatch watch;

		//Creating file path object
		std::filesystem::path in_file_path(std::string(TEST_IMAGES_PATH_STR) + "\\" + file_path);
		//Creating file folder for output
		std::filesystem::path out_dir_path = CreateOutputFolder("Test_DownscalingByChunks");

		//Path for output file
		std::filesystem::path out_file_path(out_dir_path);
		out_file_path.replace_filename(in_file_path.filename());
		out_file_path = AddAppendixToFilename(out_file_path, "_downscaled_chunks");

		//Intro
		std::cout << "Downscaling and writing back JPEG image file slice by slice with slice height of " << slice_height << " rows." << std::endl;
		std::cout << "\tFile path is:" << std::endl;
		std::cout << "\t\t" << in_file_path << std::endl;
		std::cout << std::endl;

		// Reading original image
		ImageFileInfo in_finfo(FileFormat::FF_UNSUPPORTED);
		ImageBuffer_uint16 src_image = OpenImageAndRemoveGamma(1, std::filesystem::path(in_file_path), &in_finfo);

		// Setting up downscaling
		int num_slices = src_image.GetHeight() / slice_height;
		int remainder_height = src_image.GetHeight() - slice_height * num_slices;
		if (factor > 1.0 || factor <= 0.0)
			factor = 1.0;
		uint32_t new_height = static_cast<uint32_t>(factor * static_cast<double>(src_image.GetHeight()));
		uint32_t new_width = static_cast<uint32_t>(factor * static_cast<double>(src_image.GetWidth()));

		std::cout << tabs(1) << "Downscaling in " << num_slices << "x" << slice_height << "+" << remainder_height << " slices to size [" << new_height << "x" << new_width << "]" << std::endl;
		std::cout << tabs(2) << "Initializing downscaler:" << std::endl;
		watch.Start();
		Downscaler scaler(src_image.GetLayout(), src_image.GetHeight(), src_image.GetWidth(), new_height, new_width);
		watch.Stop();
		std::cout << tabs(2) << "Done! Elapsed time: " << watch.elapsed_string() << std::endl;
		Printer::EmptyLine();

		// Allocating result
		ImageBuffer_uint16 trg_image(0, new_width, src_image.GetLayout(), true);
		
		// Downscaling slices
		watch.Start();
		watch.Pause();
		for (int slc = 0; slc < num_slices; slc++) {
			std::cout << tabs(2) << "Slice [" << slc << "]: ";
			ImageBuffer_uint16 src_slice = src_image.GetSlice(slc * slice_height, slice_height);
			std::cout << src_slice.GetHeight() << "px --> ";
			watch.Continue();
			ImageBuffer_uint16 res_slice = scaler.DownscaleNext(src_slice);
			watch.Pause();
			trg_image.Append(res_slice);
			std::cout << res_slice.GetHeight() << "px | Done! Elapsed time: " << watch.segment_string() << std::endl;
		}
		// Last slice
		if (remainder_height > 0) {
			std::cout << tabs(2) << "Remainder slice: ";
			ImageBuffer_uint16 src_slice = src_image.GetSlice(num_slices * slice_height, remainder_height);
			std::cout << src_slice.GetHeight() << "px --> ";
			watch.Continue();
			ImageBuffer_uint16 res_slice = scaler.DownscaleNext(src_slice);
			watch.Pause();
			trg_image.Append(res_slice);
			std::cout << src_slice.GetHeight() << " --> " << res_slice.GetHeight() << "px | Done! Elapsed time: " << watch.segment_string() << std::endl;
		}
		watch.Stop();
		Printer::EmptyLine();
		std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;
		int num_slices_total = num_slices;
		if (remainder_height > 0)
			num_slices_total++;
		std::cout << "\tAverage slice time: " << Stopwatch::NanoDurationToString(watch.elapsed_nanoseconds() / num_slices_total) << std::endl;
		Printer::EmptyLine();

		std::cout << "\tResulting image size [HxW]: " << trg_image.GetHeight() << "x" << trg_image.GetWidth() << std::endl;
		Printer::EmptyLine();

		// Writing the result
		if (in_finfo.GetFileFormat() == FileFormat::FF_JPEG) {
			JpegHeaderInfo in_header = in_finfo.GetJpegHeader();
			JpegHeaderInfo out_header(new_height, new_width, in_header.GetNumComponents(), in_header.GetColorSpace());
			ImageFileInfo out_finfo(out_file_path, out_header);
			
			ApplyGammaAndWriteImage(1, trg_image, out_finfo);

			return;
		}
		if (in_finfo.GetFileFormat() == FileFormat::FF_PNG) {
			PngHeaderInfo in_header = in_finfo.GetPngHeader();
			PngHeaderInfo out_header(new_height, new_width, in_header.GetBitDepth(), in_header.GetPngColorType(), in_header.GetPngInterlaceType());

			ImageFileInfo out_finfo(out_file_path, out_header);

			ApplyGammaAndWriteImage(1, trg_image, out_finfo);

			return;
		}
	}




};

