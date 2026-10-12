// DotScale.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//Windows
#include <direct.h>
//STL
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <list>
//Internal
#include "Options.h"
#include "Tester_IO.h"
#include "Tester_Gamma.h"
#include "Tester_DS.h"
#include "Tester_Gauss.h"
// 3D party
#include "CLI11.hpp"





/// <summary>
/// Sets up CLI options using CLI11 library.
/// </summary>
DotscaleOptions parse_cli_args(int argc, char** argv) {
	std::string app_description =
		"Application for high precision moire-free image downscaling.\n"
		"JPG and PNG files are supported.\n"
		"Image metadata is not preserved.";

	DotscaleOptions options;
	CLI::App dotscale_cli(app_description);
		
	// -----------------------
	// Options definitions

	std::vector<int> scaleArgs;
	dotscale_cli.add_option(
		"--scale_abs", 
		scaleArgs, 
		"New absolute size. Height and width, or single number and --scale_target specifier.")
		->expected(1, 2);

	dotscale_cli.add_option(
		"--scale_target",
		options.scale.abs_target,
		"Target for absolute scale value.")
		->transform(CLI::CheckedTransformer(
			std::map<std::string, AbsScaleTarget>{
				{ "long", AbsScaleTarget::LongEdge},
				{ "short", AbsScaleTarget::ShortEdge },
				{ "vert", AbsScaleTarget::Vertical },
				{ "hor", AbsScaleTarget::Horizontal },
				{ "equal", AbsScaleTarget::Equal }
			},
			CLI::ignore_case
		));

	dotscale_cli.add_option(
		"--scale_abs",
		scaleArgs,
		"New relative size in %. Single number for both dimensions, or height and width separately.")
		->expected(1, 2);

	dotscale_cli.add_option(
		"--blur",
		options.blur.radius,
		"Apply blur before resizing. Expects blur radius in px, may be fractional. Min radius is 0.5px.")
		->expected(1, 2);

	// -----------------------
	// Options parsing

	dotscale_cli.parse(argc, argv);

	

	return options;
}

int main(int argc, char** argv)
{
	try {
		if (false) {
			Tester_IO::TestJpegReader("parrot.jpg");
		}

		if (false) {
			Tester_IO::TestJpegWriter();
		}

		if (false) {
			Tester_IO::TestPngReader("parrot_RGB_16bit_sRGB.png");
		}

		if (false) {
			Tester_IO::TestPngWriter();
		}

		if (false) {
			Tester_IO::TestJpegReader_Simple("parrot.jpg");
		}

		if (false) {
			Tester_IO::TestJpegIOAtOnce("parrot.jpg");
		}

		if (false) {
			Tester_IO::TestPngIOAtOnce("parrot_RGB_8bit_sRGB.png");
		}

		if (false) {
			Tester_IO::TestJpegReaderByChunks("parrot.jpg", 89);
		}

		if (false) {
			Tester_IO::TestPngReaderByChunks("parrot_RGB_16bit_sRGB.png", 117);
		}

		if (false) {
			Tester_Gamma::TestSRGBConversion("parrot.jpg");
		}

		if (false) {
			Tester_DS::Test_DownscalerSliced(2842, 0.33, "parrot.jpg");
		}

		if (false) {
			Tester_Gauss::TestValue32(20, true);
			Tester_Gauss::TestValue32(10000000, false);
		}

		if (false) {
			Tester_Gauss::TestArea63(20, true);
			Tester_Gauss::TestArea63(10000000, false);
		}

		if (false) {
			Tester_Gauss::TestSqrt(10);
		}

		if (false) {
			Tester_Gauss::TestAverages(30, true);
			Tester_Gauss::TestAverages(1000000, false);
		}

		if (true) {
			
		}

	}
	catch (const std::exception& e) {
		std::cerr << "Unhandled Exception: " << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unknown exception occurred!" << std::endl;
		return 1;
	}
}
