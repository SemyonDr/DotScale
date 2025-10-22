#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <cstdint>
#include "FixedMath.h"
#include "Printer.h"

#define VALUE_SPLINE_COEFS_FNAME "gauss_data/value_approx_coef_64spiu_60bit.data"
#define AREA_SPLINE_COEFS_FNAME "gauss_data/area_approx_coef_128spiu_60bit.data"
#define STR(x) #x
#define AREA_PRECALC_FNAME(scale) "gauss_data/areas_63bit_scale_" STR(scale) ".data"

#define AREA_PRECALC_FNAME_0 "gauss_data/areas_63bit_scale_0.data"
#define AREA_PRECALC_FNAME_1 "gauss_data/areas_63bit_scale_1.data"
#define AREA_PRECALC_FNAME_2 "gauss_data/areas_63bit_scale_2.data"
#define AREA_PRECALC_FNAME_3 "gauss_data/areas_63bit_scale_3.data"
#define AREA_PRECALC_FNAME_4 "gauss_data/areas_63bit_scale_4.data"
#define AREA_PRECALC_FNAME_5 "gauss_data/areas_63bit_scale_5.data"
#define AREA_PRECALC_FNAME_6 "gauss_data/areas_63bit_scale_6.data"
#define AREA_PRECALC_FNAME_7 "gauss_data/areas_63bit_scale_7.data"
#define AREA_PRECALC_FNAME_8 "gauss_data/areas_63bit_scale_8.data"
#define AREA_PRECALC_FNAME_9 "gauss_data/areas_63bit_scale_9.data"
#define AREA_PRECALC_FNAME_10 "gauss_data/areas_63bit_scale_10.data"


/// <summary>
/// Approximates Gaussian Curve function g(x) = e^(-(x^2/2)) with high precision.
/// </summary>
class GaussCurve {
private:

	struct ValueSplineCoefs {
		fxd64_60_t a;
		fxd64_60_t b;
		fxd64_60_t c;
		fxd64_60_t d;
	};

	struct AreaSplineCoefs {
		fxd64_60_t c1;
		fxd64_60_t c2;
		fxd64_60_t c3;
		fxd64_60_t c4;
		fxd64_60_t c5;
		fxd64_60_t c6;
	};


	//--------------------------------
	//  PRECALCULATED DATA
	//--------------------------------

	// 64 splines for every unit up to x~6.76.
	// Interval of the last spline includes point x=6.76.
	// 433 splines total.
	static constexpr uint16_t NUM_VAL_SPLINES = 64 * 6 + 49;

	// 128 splines for every unit up to x~6.478.
	// Interval of the last spline includes point x=6.478.
	// 830 splines total.
	static constexpr uint16_t NUM_AREA_SPLINES = 128 * 6 + 62;

	/// <summary>
	/// Approximation spline coefficients for function value.
	/// </summary>
	static constexpr ValueSplineCoefs _vspline_coefs[NUM_VAL_SPLINES] = {
		#include VALUE_SPLINE_COEFS_FNAME
	};

	/// <summary>
	/// Approximation spline coefficients for function value.
	/// </summary>
	static constexpr AreaSplineCoefs _aspline_coefs[NUM_AREA_SPLINES] = {
		#include AREA_SPLINE_COEFS_FNAME
	};

	static constexpr ufxd64_63_t Areas_0[830] = {
		#include AREA_PRECALC_FNAME_0
	};
	static constexpr ufxd64_63_t Areas_1[415] = {
		#include AREA_PRECALC_FNAME_1
	};
	static constexpr ufxd64_63_t Areas_2[208] = {
		#include AREA_PRECALC_FNAME_2
	};
	static constexpr ufxd64_63_t Areas_3[104] = {
		#include AREA_PRECALC_FNAME_3
	};
	static constexpr ufxd64_63_t Areas_4[52] = {
		#include AREA_PRECALC_FNAME_4
	};
	static constexpr ufxd64_63_t Areas_5[26] = {
		#include AREA_PRECALC_FNAME_5
	};
	static constexpr ufxd64_63_t Areas_6[13] = {
		#include AREA_PRECALC_FNAME_6
	};
	static constexpr ufxd64_63_t Areas_7[7] = {
		#include AREA_PRECALC_FNAME_7
	};
	static constexpr ufxd64_63_t Areas_8[4] = {
		#include AREA_PRECALC_FNAME_8
	};
	static constexpr ufxd64_63_t Areas_9[2] = {
		#include AREA_PRECALC_FNAME_9
	};
	static constexpr ufxd64_63_t Areas_10[1] = {
		#include AREA_PRECALC_FNAME_10
	};

	static constexpr const ufxd64_63_t* AREAS[] = {
		Areas_0, Areas_1, Areas_2, Areas_3,
		Areas_4, Areas_5, Areas_6, Areas_7,
		Areas_8, Areas_9, Areas_10
	};

	//--------------------------------
	//  CONSTANTS
	//--------------------------------

	/// <summary>
	/// This is x cutoff value for 16 bit fractional precision x values.
	/// 443266 = 0110.[1100 0011 1000 0010] = 6.[50050] ~ 6.7637
	/// This is last x value that would not return 0.
	/// </summary>
	static constexpr ufxd32_16_t X_CUTOFF_VAL_16 = 443266;

	/// <summary>
	/// This is x cutoff value for 60 bit fractional precision x values.
	/// This is last x value that would not return 0.
	/// </summary>
	static constexpr ufxd64_60_t X_CUTOFF_VAL_60 = 7798021677424194372;

	static constexpr double X_CUTOFF_VAL_DOUBLE = 6.76370563500189520345213222932073451471722531778671339602157;


	/// <summary>
	/// After this x value area is considered to be 0.
	/// This value is the width of the area approximation segment 1/128
	/// multiplied by 830 (number of segments). 
	/// In other words is the right edge of the last segment. 
	/// </summary>
	static constexpr ufxd64_60_t X_CUTOFF_AREA_60 = 7475975381435023360;

	/// <summary>
	/// Width of segments for various scales.
	/// Largest scale 10 with base 10 width 8.0 is not listed
	/// because it is not used and will not fit into the variable.
	/// </summary>
	static constexpr ufxd64_60_t AREA_SEG_WIDTH[10] = {
		9007199254740992,		// 0	1/128
		18014398509481984,		// 1	1/64
		36028797018963968,		// 2	1/32
		72057594037927936,		// 3	1/16
		144115188075855872,		// 4	1/8
		288230376151711744,		// 5	1/4
		576460752303423488,		// 6	1/2
		1152921504606846976,	// 7	1
		2305843009213693952,	// 8	2
		4611686018427387904		// 9	4
	};

public:

	//--------------------------------
	//  POINT VALUE
	//--------------------------------

	/// <summary>
	/// Value of the curve at point x.
	/// If x is 0, then 0 is returned (instead of number that represents 1.0) 
	/// because 1 cannot be represented with 32 bit precision by 32 bit number.
	/// </summary>
	/// <param name="x"> -- 16 bit fixed precision.</param>
	/// <returns>Function value 32 bit fixed precision.</returns>
	static ufxd32_32_t Value_x16_v32(ufxd32_16_t x) {
		return Value_x60_v32(static_cast<ufxd64_60_t>(x) << 44);
	}

	/// <summary>
	/// Value of the curve at point x.
	/// If x is 0, then 0 is returned (instead of number that represents 1.0) 
	/// because 1 cannot be represented with 32 bit precision by 32 bit number.
	/// </summary>
	/// <param name="x"> -- 32 bit fixed precision.</param>
	/// <returns>Function value 32 bit fixed precision.</returns>
	static ufxd32_32_t Value_x32_v32(ufxd32_32_t x) {
		return Value_x60_v32(static_cast<ufxd64_60_t>(x) << 28);
	}

	/// <summary>
	/// Value of the curve at point x.
	/// If x is 0, then 0 is returned (instead of number that represents 1.0) 
	/// because 1 cannot be represented with 32 bit precision by 32 bit number.
	/// This is a highest resolution version of x and g(x).
	/// </summary>
	/// <param name="x"> -- 60 bit fixed precision.</param>
	/// <returns>Function value 32 bit fixed precision.</returns>
	static ufxd32_32_t Value_x60_v32(ufxd64_60_t x);


	/// <summary>
	/// Value of the curve at point x.
	/// Precision is reduced.
	/// </summary>
	/// <param name="x">Point to evaluate.</param>
	/// <returns>Function value.</returns>
	static double Value_Double(double x) {
		if (x == 0)
			return 1.0;

		if (x < 0)
			x *= -1.0;

		if (x > X_CUTOFF_VAL_DOUBLE)
			return 0.0;

		ufxd64_60_t x_60 = FxdMath::from_double_u60(x); // x*2^60
		ufxd32_32_t value_32 = Value_x60_v32(x_60);

		return FxdMath::to_double_32(value_32); // v / 2^32
	}

	//--------------------------------
	//  AVERAGE VALUE
	//--------------------------------

	/// <summary>
	/// Finds average function value on given interval.
	/// Interval is given with 60-bit precision.
	/// Average value is returned with 32-bit precision.
	/// </summary>
	static ufxd32_32_t Average_x60_v32(ufxd64_60_t left, ufxd64_60_t right);



	//--------------------------------
	//  AREA
	//--------------------------------

	/// <summary>
	/// Calculates area under the function, or function integral on an interval.
	/// Expects interval boundaries with 60-bit precision.
	/// Returns area with 63-bit precision.
	/// </summary>
	static ufxd64_63_t Area_x60_v63(ufxd64_60_t left, ufxd64_60_t right);





private:

	//--------------------------------
	//  AREA SUPPLIMENTARY FUNCTIONS
	//--------------------------------

	/// <summary>
	/// Calculates integral approximation over given
	/// approximation spline. Assumes that left and right are inside the segment.
	/// </summary>
	static ufxd64_63_t SubscaleIntegral_x60_v63(const AreaSplineCoefs& cf, ufxd64_60_t left, ufxd64_60_t right) {
		return AreaPolynomialValue_x60_v63(cf, right) - AreaPolynomialValue_x60_v63(cf, left);
	}

	/// <summary>
	/// Returns value of indefinite integral over the polynomial that is used in
	/// function approximation for area calculation.
	/// </summary>
	static ufxd64_63_t AreaPolynomialValue_x60_v63(const AreaSplineCoefs& cf, ufxd64_60_t x);



	//--------------------------------
	//  DEBUG PRINTOUT VERSIONS
	//--------------------------------
	
	static ufxd32_32_t Average_x60_v32_debug(ufxd64_60_t left, ufxd64_60_t right);

	static ufxd32_32_t Average_x60_v32_debug_short(ufxd64_60_t left, ufxd64_60_t right);

	static ufxd64_63_t Area_x60_v63_debug(ufxd64_60_t left, ufxd64_60_t right);
};


