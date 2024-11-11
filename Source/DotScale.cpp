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

int main()
{
	if (false) {
		Tester_IO::TestJpegReader("parrot.jpg");
	}

	if (false) {
		Tester_IO::TestJpegWriter();
	}

	if (false) {
		Tester_IO::TestPngReader("parrot_RGB_16bit_sRGB.png");
	}
	
	if (true) {
		Tester_IO::TestPngWriter();
	}

	if (false) {
		Tester_IO::TestJpegReader_Simple(".\\TestImages\\parrot.jpg");
	}

	if (false) {
		Tester_IO::TestJpegIOAtOnce(".\\TestImages\\parrot.jpg");
	}

	if (false) {
		Tester_IO::TestPngIOAtOnce(".\\TestImages\\parrot_RGB_8bit_sRGB.png");
	}
	
	if (false) {
		Tester_IO::TestJpegReaderByChunks(".\\TestImages\\parrot.jpg", 89);
	}

	if (false) {
		Tester_IO::TestPngReaderByChunks(".\\TestImages\\parrot_RGB_16bit_sRGB.png",117);
	}
	
	
}
