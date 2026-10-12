#pragma once
#include <filesystem>
#include "Tester_Base.h"
#include "Blurer.h"
#include "ImageBuffer.h"

class Tester_Blur : Tester_Base {

	void TestBlur(std::filesystem::path file) {
		
		Blurer blur(ImagePixelLayout::RGB, 1000, 500, 1533542);
		
	
	
	
	}


};