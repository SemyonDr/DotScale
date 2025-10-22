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
#include "Tester_IO.h"
#include "Tester_Gamma.h"
#include "Tester_DS.h"
#include "Tester_Gauss.h"

int main()
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

		if (true) {
			Tester_Gauss::TestAverages(30, true);
			Tester_Gauss::TestAverages(1000000, false);
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
