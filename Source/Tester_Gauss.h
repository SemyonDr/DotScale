#pragma once

//STL
#include <cmath>
#include <cstdint>
#include <random>
// Internal
#include "GaussCurve.h"
//Internal - debug
#include "Tester_Base.h"


class Tester_Gauss : Tester_Base {


public:

	static void TestValue32(uint32_t num_points, boolean print);

	static void TestArea63(uint32_t num_intervals, boolean print);

	static void TestSqrt(int num_values);

	static void TestAverages(uint32_t num_intervals, boolean print);
	

};

