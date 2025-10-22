#include "GaussCurve.h"


//--------------------------------
//  POINT VALUE
//--------------------------------

/// <summary>
/// Takes in x as integer with fractional precision of 60 bits and returns 32 bit fractional precision function value.
/// If x is 0, then 0 is returned (instead of number that represents 1.0) 
/// because 1 cannot be represented with 32 bit precision by 32 bit number.
/// </summary>
ufxd32_32_t GaussCurve::Value_x60_v32(ufxd64_60_t x) {
	// IMPORTANT NOTE ----
	// Calculating top of res_top MUST be done in two steps, because applying bit mask produces
	// unsigned intermediate value and shifting it immedeately without storing in a signed variable
	// will lose the sign and will introduce hard to track bug.

	// Edges
	if (x == 0)
		return 0;

	if (x > X_CUTOFF_VAL_60)
		return 0;

	// Getting relevant spline coefficients
	// There are 64 splines for unit.
	// Spline index in a sense is a fixed precision number
	// with 64 steps of precision. 
	// 64 steps correspond to 6 bits of precision.
	// Therefore to get spline index we round down x to 6 bit precision.
	ValueSplineCoefs cf = _vspline_coefs[x >> 54];

	// Result buffers
	fxd64_60_t res = 0;
	fxd64_28_t res_top = 0;
	fxd64_60_t res_bot = 0;

	// Splitting x
	fxd64_28_t x_top = (x & FxdMath::MASK_32_TOP) >> 32;
	fxd64_60_t x_bot = x & FxdMath::MASK_32_BOT;

	// STEP 1 -- ax
	fxd64_28_t a_top = cf.a & FxdMath::MASK_32_TOP;
	a_top = a_top >> 32;
	fxd64_60_t a_bot = cf.a & FxdMath::MASK_32_BOT;
	res = ((a_top * x_top) << 4) + ((a_top * x_bot) >> 28) + ((a_bot * x_top) >> 28) + ((a_bot * x_bot) >> 60);

	// STEP 2 -- ax + b
	res = res + cf.b;

	// STEP 3 -- (ax + b)x
	res_top = res & FxdMath::MASK_32_TOP;
	res_top = res_top >> 32;
	res_bot = res & FxdMath::MASK_32_BOT;
	res = ((res_top * x_top) << 4) + ((res_top * x_bot) >> 28) + ((res_bot * x_top) >> 28) + ((res_bot * x_bot) >> 60);

	// STEP 4 -- (ax + b)x + c
	res = res + cf.c;

	// STEP 5 -- ((ax + b)x + c)x
	res_top = res & FxdMath::MASK_32_TOP;
	res_top = res_top >> 32;
	res_bot = res & FxdMath::MASK_32_BOT;
	res = ((res_top * x_top) << 4) + ((res_top * x_bot) >> 28) + ((res_bot * x_top) >> 28) + ((res_bot * x_bot) >> 60);

	// STEP 6 -- ((ax + b)x + c)x + d
	res = res + cf.d;

	// Making result 32 bit
	ufxd32_32_t res_32 = static_cast<ufxd32_32_t>(res >> 28);

	return res_32;
}





//--------------------------------
//  AVERAGE VALUE
//--------------------------------

/// <summary>
/// Finds average function value on given interval.
/// Interval is given with 60-bit precision.
/// Average value is returned with 32-bit precision.
/// </summary>
ufxd32_32_t GaussCurve::Average_x60_v32(ufxd64_60_t left, ufxd64_60_t right) {
	// Finding area
	ufxd64_63_t area = Area_x60_v63(left, right);

	// Now we divide the area by the interval width
	ufxd64_60_t width = right - left;
	const ufxd64_28_t w_top = width >> 32;
	const ufxd64_60_t w_bot = width & FxdMath::MASK_32_BOT;

	// We divide in a manner similar to finding the square root.
	// We use following relation
	//		area / width = res  -->  res * width = area
	// res is split into sum of 1 digit values (r1 + r2 + r3 + r4 + ... + r32)
	// And therefore:
	//		area = r1*width + r2*width + r3*width + ... + r32*width
	// On each iteration we find if ri=0 or ri=2^32-i by checking if
	// (2^32-i)*width will exceed area remainder.
	// Note on possible values:
	// This method assumes that area and width are valid ones.
	// This implies that average value is always < 1.0 and
	// can fit into 32 bits with 32 bits precision.
	// Also assumed that max integer part of the area is 1
	uint64_t res = 0;
	uint64_t cand = 0x80000000; // 1000 0000 0000 0000 0000 0000 0000 0000
	int cand_index = 31;
	ufxd64_63_t term = 0;

	// For cand that are larger than 29 bits term value (w_top*cand << 3) might overflow past 32 bits depending on w_top (if width is more than 1.0 it will overflow).
	// For these 3 cases we add additional overflow check. 
	while (cand > 0x10000000) {		// 0001 0000 0000 0000 0000 0000 0000 0000
		term = w_top << cand_index;

		if (term < 0x2000000000000000) { // 0010 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
			term <<= 3; // This might overflow without the check
			term += (w_bot << (cand_index - 29));
			if (term <= area) {
				res = res | cand;
				area -= term;
			}
		}
		cand_index--;
		cand >>= 1;
	}

	while (cand != 0) {
		term = (w_top << (cand_index + 3)) +	// 28 + 32 = 60; 60 + 3 = 63
			((w_bot << cand_index) >> 29);		// 60 + 32 = 92; 92 - 29 = 63
		if (term <= area) {
			res = res | cand;
			area -= term;
		}
		cand_index--;
		cand >>= 1;
	}

	return static_cast<ufxd32_32_t>(res);
}

//--------------------------------
//  AREA
//--------------------------------


/// <summary>
/// Calculates area under the function, or function integral on an interval.
/// Expects interval boundaries with 60-bit precision.
/// Returns area with 63-bit precision.
/// </summary>
ufxd64_63_t GaussCurve::Area_x60_v63(ufxd64_60_t left, ufxd64_60_t right) {
	// Edge cases
	if (right <= left)
		return 0;

	if (left > X_CUTOFF_AREA_60)
		return 0;

	if (right > X_CUTOFF_AREA_60)
		if (left == 0)
			return AREAS[10][0];
		else
			right = X_CUTOFF_AREA_60;

	// Result
	ufxd64_63_t area = 0;

	// State
	int scale = 10;
	ufxd64_60_t left_edge = left;
	ufxd64_60_t right_edge = right;
	uint64_t next_segment_index = 0;
	uint64_t segment_index = 0;
	ufxd64_60_t segment_width = 0;
	ufxd64_60_t segment_left = 0;
	ufxd64_60_t segment_right = 0;

	int shamt = 0;

	// =================================================================
	// Stage 1 -- Finding first scale segment of which fits in the range
	while (scale > 0) {
		// Getting first segment on the left side of 'right'
		scale--;

		shamt = 53 + scale;
		segment_width = AREA_SEG_WIDTH[scale];
		next_segment_index = right >> shamt;
		if (next_segment_index == 0)
			// 'right' sits withing segment 0
			continue;
		segment_index = next_segment_index - 1;
		segment_right = next_segment_index << shamt;
		segment_left = segment_right - segment_width;
		if (segment_right <= left)
			// Segment is outside of interval
			continue;
		if (segment_left >= left) {
			// Segment fits inside the interval
			area += AREAS[scale][segment_index];
			right_edge = segment_right;
			// Checking if second segment also fits
			if (segment_index > 0 && left <= segment_left - segment_width) {
				area += AREAS[scale][segment_index - 1];
				left_edge = segment_left - segment_width;
			}
			else
				left_edge = segment_left;

			// If segment fits loop is broken and we go to the stage 2
			break;
		}
	}

	// =================================================================
	// Stage 2 -- Now we iterate scales to find additional segments for left and right parts
	// right part is range [right_edge, right]
	// left part is range [left, left_edge]
	while (scale > 0) {
		scale--;

		shamt = 53 + scale;
		segment_width = AREA_SEG_WIDTH[scale];

		// Right side
		segment_right = right_edge + segment_width;
		segment_index = right_edge >> shamt;
		if (segment_right <= right) {
			area += AREAS[scale][segment_index];
			right_edge = segment_right;
		}
		// Left side
		next_segment_index = left_edge >> shamt;
		if (next_segment_index == 0)
			continue;
		segment_index = next_segment_index - 1;
		segment_left = left_edge - segment_width;
		if (left <= segment_left) {
			area += AREAS[scale][segment_index];
			left_edge = segment_left;
		}
	}

	// =================================================================
	// Stage 3 -- Taking integrals of sub-scale segments.
	shamt = 53;
	area += SubscaleIntegral_x60_v63(_aspline_coefs[right >> shamt], right_edge, right);
	area += SubscaleIntegral_x60_v63(_aspline_coefs[left >> shamt], left, left_edge);

	return area;
}



/// <summary>
/// Returns value of indefinite integral over the polynomial that is used in
/// function approximation for area calculation.
/// </summary>
ufxd64_63_t GaussCurve::AreaPolynomialValue_x60_v63(const AreaSplineCoefs& cf, ufxd64_60_t x) {
	// IMPORTANT NOTE ----
	// Calculating top of res_top MUST be done in two steps, because applying bit mask produces
	// unsigned intermediate value and shifting it immedeately without storing in signed variable
	// will lose the sign and will introduce hard to track bug.

	// Result buffers
	fxd64_60_t res = 0;
	fxd64_28_t res_top = 0;
	fxd64_60_t res_bot = 0;

	// Splitting x
	const fxd64_28_t x_top = (x & FxdMath::MASK_32_TOP) >> 32;
	const fxd64_60_t x_bot = x & FxdMath::MASK_32_BOT;

	// STEP 1 -- c1*x
	fxd64_28_t c1_top = cf.c1 & FxdMath::MASK_32_TOP;
	c1_top = c1_top >> 32;
	fxd64_60_t c1_bot = cf.c1 & FxdMath::MASK_32_BOT;
	res = ((c1_top * x_top) << 4) + ((c1_top * x_bot) >> 28) + ((c1_bot * x_top) >> 28) + ((c1_bot * x_bot) >> 60);

	// STEP 2 -- c1*x + c2
	res = res + cf.c2;

	// STEP 3 -- (c1*x + c2)*x
	res_top = res & FxdMath::MASK_32_TOP;
	res_top = res_top >> 32;
	res_bot = res & FxdMath::MASK_32_BOT;
	res = ((res_top * x_top) << 4) + ((res_top * x_bot) >> 28) + ((res_bot * x_top) >> 28) + ((res_bot * x_bot) >> 60);

	// STEP 4 -- (c1*x + c2)*x+c3
	res = res + cf.c3;

	// STEP 5 -- ((c1*x + c2)*x+c3)*x
	res_top = res & FxdMath::MASK_32_TOP;
	res_top = res_top >> 32;
	res_bot = res & FxdMath::MASK_32_BOT;
	res = ((res_top * x_top) << 4) + ((res_top * x_bot) >> 28) + ((res_bot * x_top) >> 28) + ((res_bot * x_bot) >> 60);

	// STEP 6 -- ((c1*x + c2)*x+c3)*x+c4
	res = res + cf.c4;

	// STEP 7 -- (((c1*x + c2)*x+c3)*x+c4)*x
	res_top = res & FxdMath::MASK_32_TOP;
	res_top = res_top >> 32;
	res_bot = res & FxdMath::MASK_32_BOT;
	res = ((res_top * x_top) << 4) + ((res_top * x_bot) >> 28) + ((res_bot * x_top) >> 28) + ((res_bot * x_bot) >> 60);

	// STEP 8 -- (((c1*x + c2)*x+c3)*x+c4)*x+c5
	res = res + cf.c5;

	// STEP 9 -- ((((c1*x + c2)*x+c3)*x+c4)*x+c5)*x
	res_top = res & FxdMath::MASK_32_TOP;
	res_top = res_top >> 32;
	res_bot = res & FxdMath::MASK_32_BOT;
	res = ((res_top * x_top) << 4) + ((res_top * x_bot) >> 28) + ((res_bot * x_top) >> 28) + ((res_bot * x_bot) >> 60);

	// STEP 10 -- ((((c1*x + c2)*x+c3)*x+c4)*x+c5)*x+c6
	res = res + cf.c6;

	// STEP 11 -- (((((c1*x + c2)*x+c3)*x+c4)*x+c5)*x+c6)*x
	// Last step returns 63 bit precision value
	// Since x is non-negative and any polynomial value is also non-negative and pol_val = res*x
	// res at this point is non-negative. All terms in the sum are non-negative.
	// We use new unsigned variable to store 63 bit precision.
	// Note that shifts are modified to produce 63 bit precision result instead of 60 bit.
	ufxd64_63_t res_top_unsigned = (res & FxdMath::MASK_32_TOP) >> 32;
	ufxd64_63_t res_bot_unsigned = res & FxdMath::MASK_32_BOT;
	ufxd64_63_t x_top_unsigned = (x & FxdMath::MASK_32_TOP) >> 32;
	ufxd64_63_t x_bot_unsigned = x & FxdMath::MASK_32_BOT;
	ufxd64_63_t res_unsigned =
		((res_top_unsigned * x_top_unsigned) << 7) +
		((res_top_unsigned * x_bot_unsigned) >> 25) +
		((res_bot_unsigned * x_top_unsigned) >> 25) +
		((res_bot_unsigned * x_bot_unsigned) >> 57);

	return res_unsigned;
}





//--------------------------------
//  VERSIONS WITH DEBUG PRINT
//--------------------------------


ufxd32_32_t GaussCurve::Average_x60_v32_debug_short(ufxd64_60_t left, ufxd64_60_t right) {
	// Finding area
	ufxd64_63_t area = Area_x60_v63(left, right);

	// Now we divide the area by the interval width
	ufxd64_60_t width = right - left;
	const ufxd64_28_t w_top = width >> 32;
	const ufxd64_60_t w_bot = width & FxdMath::MASK_32_BOT;

	// We divide in a manner similar to finding the square root.
	// We use following relation
	//		area / width = res  -->  res * width = area
	// res is split into sum of 1 digit values (r1 + r2 + r3 + r4 + ... + r32)
	// And therefore:
	//		area = r1*width + r2*width + r3*width + ... + r32*width
	// On each iteration we find if ri=0 or ri=2^32-i by checking if
	// (2^32-i)*width will exceed area remainder.
	// Note on possible values:
	// This method assumes that area and width are valid ones.
	// This implies that average value is always < 1.0 and
	// can fit into 32 bits with 32 bits precision.
	// Also assumed that max integer part of the area is 1
	uint64_t res = 0;
	uint64_t cand = 0x80000000; // 1000 0000 0000 0000 0000 0000 0000 0000
	int cand_index = 31;
	ufxd64_63_t term = 0;

	// For cand that are larger than 29 bits term value (w_top*cand << 3) might overflow past 32 bits depending on w_top (if width is more than 1.0 it will overflow).
	// For these 3 cases we add additional overflow check. 
	while (cand > 0x10000000) {		// 0001 0000 0000 0000 0000 0000 0000 0000
		Printer::EmptyLine();
		std::cout << "\t[" << cand_index << "]:" << std::endl;
		Printer::PrintValueReportIntBinary64("area", area);
		Printer::PrintValueReportIntBinary64("cand", cand);
		term = w_top << cand_index;
		Printer::PrintValueReportIntBinary64("term top", term);
		if (term < 0x2000000000000000) { // 0010 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
			term <<= 3; // This might overflow without the check
			term += (w_bot << (cand_index - 29));
			Printer::PrintValueReportIntBinary64("term", term);
			if (term <= area) {
				res = res | cand;
				area -= term;
			}
		}
		Printer::PrintValueReportIntBinary64("res", res);
		cand_index--;
		cand >>= 1;
	}

	while (cand != 0) {
		Printer::EmptyLine();
		Printer::PrintValueReportIntBinary64("area", area);
		Printer::PrintValueReportIntBinary64("cand", cand);
		term = (w_top << (cand_index + 3)) +	// 28 + 32 = 60; 60 + 3 = 63
			((w_bot << cand_index) >> 29);		// 60 + 32 = 92; 92 - 29 = 63
		Printer::PrintValueReportIntBinary64("term", term);
		if (term <= area) {
			res = res | cand;
			area -= term;
		}
		Printer::PrintValueReportIntBinary64("res", res);
		cand_index--;
		cand >>= 1;
	}

	return static_cast<ufxd32_32_t>(res);
}




ufxd32_32_t GaussCurve::Average_x60_v32_debug(ufxd64_60_t left, ufxd64_60_t right) {
// Finding area
	ufxd64_63_t area = Area_x60_v63(left, right);

	// Now we divide the area by the interval width
	ufxd64_60_t width = right - left;
	const ufxd64_28_t w_top = width >> 32;
	const ufxd64_60_t w_bot = width & FxdMath::MASK_32_BOT;

	// We divide in a manner similar to finding the square root.
	// We use following relation
	//		area / width = res  -->  res * width = area
	// res is split into sum of 1 digit values (r1 + r2 + r3 + r4 + ... + r32)
	// And therefore:
	//		area = r1*width + r2*width + r3*width + ... + r32*width
	// On each iteration we find if ri=0 or ri=2^32-i by checking if
	// (2^32-i)*width will exceed area remainder.
	uint64_t res = 0;
	uint64_t cand = 0x0000000080000000;
	ufxd64_63_t term = 0;

	uint32_t res_binary[32]; // -------------------------------------------------- DEBUG
	for (int i = 0; i < 32; i++) // -------------------------------------------------- DEBUG
		res_binary[i] = 0; // -------------------------------------------------- DEBUG

	std::cout << "Area:\t" << area << std::endl;		// -------------------------------------------------- DEBUG
	std::cout << "Width:\t" << width << std::endl;		// -------------------------------------------------- DEBUG
	Printer::PrintValueReportIntBinary64("\tw_top:", w_top);	// -------------------------------------------------- DEBUG
	Printer::PrintValueReportIntBinary64("\tw_bot:", w_bot);	 // -------------------------------------------------- DEBUG
	// Finding every binary digit of the average value
	int cand_index = 31;	// -------------------------------------------------- DEBUG
	while (cand != 0) {
		term = (w_top << (cand_index + 3)) +	// 28 + 32 = 60; 60 + 3 = 63
			(w_bot << (cand_index - 29));		// 60 + 32 = 92; 92 - 29 = 63

		uint64_t top_term = w_top << cand_index;
		if ((top_term & 0xE000000000000000) != 0) {
			Printer::PrintValueReportIntBinary64("w_top * cand:", top_term);	// -------------------------------------------------- DEBUG
			std::cout << "\t--- Top term overflows ---" << std::endl;		// -------------------------------------------------- DEBUG
			std::cout << "\t--- XXX NOT PASSED XXX ---" << std::endl;		// -------------------------------------------------- DEBUG
			cand_index--;	// -------------------------------------------------- DEBUG
			cand >>= 1;
			continue;
		}


		uint64_t top_term_63 = top_term << 3;
		uint64_t bot_term = w_bot << cand_index;
		uint64_t bot_term_63 = bot_term >> 29;

		term = top_term_63 + bot_term_63;

		std::cout << std::endl; // -------------------------------------------------- DEBUG
		std::cout << "\t\t\t\t\t\t" << FxdMath::legend_bin_64 << std::endl; // -------------------------------------------------- DEBUG

		Printer::PrintValueReportIntBinary64((std::ostringstream() << "Candidate [" << cand_index << "]:").str(), cand);	// -------------------------------------------------- DEBUG
		Printer::PrintValueReportIntBinary64("w_top * cand:", top_term);	// -------------------------------------------------- DEBUG
		Printer::PrintValueReportIntBinary64("w_top * cand << 3:", top_term_63);	// -------------------------------------------------- DEBUG
		Printer::PrintValueReportIntBinary64("w_bot * cand:", bot_term);	// -------------------------------------------------- DEBUG
		Printer::PrintValueReportIntBinary64("w_bot * cand >> 29:", bot_term_63);	// -------------------------------------------------- DEBUG
		Printer::PrintValueReportIntBinary64("Term:", term);	// -------------------------------------------------- DEBUG

		if (term <= area) {
			res = res | cand;
			area -= term;
			std::cout << "\t--- VVV PASSED VVV ---" << std::endl;		// -------------------------------------------------- DEBUG
			res_binary[31 - cand_index] = 1;		// -------------------------------------------------- DEBUG
		}
		else {
			std::cout << "\t--- XXX NOT PASSED XXX ---" << std::endl;		// -------------------------------------------------- DEBUG
		}
		Printer::PrintValueReportIntBinary64("Remainder:", area);	// -------------------------------------------------- DEBUG
		cand_index--;	// -------------------------------------------------- DEBUG
		cand >>= 1;

		Printer::PrintValueReportIntBinary64("Result:", res);	// -------------------------------------------------- DEBUG
		std::cout << std::endl; // -------------------------------------------------- DEBUG
	}

	std::cout << std::endl;

	return static_cast<ufxd32_32_t>(res);
}




ufxd64_63_t GaussCurve::Area_x60_v63_debug(ufxd64_60_t left, ufxd64_60_t right) {
	// Edge cases
	if (right <= left)
		return 0;

	if (left > X_CUTOFF_AREA_60)
		return 0;

	if (right > X_CUTOFF_AREA_60)
		if (left == 0)
			return AREAS[10][0];
		else
			right = X_CUTOFF_AREA_60;


	// ------------------------------------- DEBUG
	struct ScaleInfo {
		ufxd64_60_t left_edge = 0;
		ufxd64_60_t right_edge = 0;
		ufxd64_63_t left_block = 0;
		ufxd64_63_t right_block = 0;
		ufxd64_63_t total = 0;
		uint32_t left_index = 0;
		uint32_t right_index = 0;
	};
	struct AreaCalcInfo {
		ScaleInfo scale[11];
		ScaleInfo left_subscale;
		ScaleInfo right_subscale;
		ufxd64_63_t result;
	};
	AreaCalcInfo info;

	// ------------------------------------- DEBUG

	// Result
	ufxd64_63_t area = 0;

	// State
	int scale = 10;
	ufxd64_60_t left_edge = left;
	ufxd64_60_t right_edge = right;
	uint64_t next_segment_index = 0;
	uint64_t segment_index = 0;
	ufxd64_60_t segment_width = 0;
	ufxd64_60_t segment_left = 0;
	ufxd64_60_t segment_right = 0;

	info.scale[scale].left_edge = left_edge; // -------------------------------------------------------------------------------- DEBUG
	info.scale[scale].right_edge = right_edge; // -------------------------------------------------------------------------------- DEBUG

	int shamt = 0;

	// =================================================================
	// Stage 1 -- Finding first scale segment of which fits in the range
	while (scale > 0) {
		// Getting first segment on the left side of 'right'
		scale--;
		info.scale[scale].total = area; // -------------------------------------------------------------------------------- DEBUG
		info.scale[scale].left_edge = left_edge; // -------------------------------------------------------------------------------- DEBUG
		info.scale[scale].right_edge = right_edge; // -------------------------------------------------------------------------------- DEBUG

		shamt = 53 + scale;
		segment_width = AREA_SEG_WIDTH[scale];
		next_segment_index = right >> shamt;
		if (next_segment_index == 0)
			// 'right' sits withing segment 0
			continue;
		segment_index = next_segment_index - 1;
		segment_right = next_segment_index << shamt;
		segment_left = segment_right - segment_width;
		if (segment_right <= left)
			// Segment is outside of interval
			continue;
		if (segment_left >= left) {
			// Segment fits inside the interval
			area += AREAS[scale][segment_index];
			info.scale[scale].total = area; // -------------------------------------------------------------------------------- DEBUG
			info.scale[scale].right_block = AREAS[scale][segment_index]; //---------------------------------------------------- DEBUG
			info.scale[scale].right_index = segment_index; // ----------------------------------------------------------------- DEBUG
			right_edge = segment_right;
			// Checking if second segment also fits
			if (segment_index > 0 && left <= segment_left - segment_width) {
				area += AREAS[scale][segment_index - 1];
				left_edge = segment_left - segment_width;
				info.scale[scale].total = area; // -------------------------------------------------------------------------------- DEBUG
				info.scale[scale].left_block = AREAS[scale][segment_index]; //---------------------------------------------------- DEBUG
				info.scale[scale].left_index = segment_index; // ----------------------------------------------------------------- DEBUG
			}
			else
				left_edge = segment_left;
			info.scale[scale].right_edge = right_edge; // ---------------------------------------------------------------------- DEBUG
			info.scale[scale].left_edge = left_edge; // ------------------------------------------------------------------------ DEBUG

			// If segment fits loop is broken and we go to stage 2
			break;
		}
	}

	// =================================================================
	// Stage 2 -- Now we iterate scales to find additional segments for left and right parts
	// right part is range [right_edge, right]
	// left part is range [left, left_edge]
	while (scale > 0) {
		scale--;
		info.scale[scale].total = area; // -------------------------------------------------------------------------------- DEBUG
		info.scale[scale].left_edge = left_edge; // -------------------------------------------------------------------------------- DEBUG
		info.scale[scale].right_edge = right_edge; // -------------------------------------------------------------------------------- DEBUG

		shamt = 53 + scale;
		segment_width = AREA_SEG_WIDTH[scale];

		// Right side
		segment_right = right_edge + segment_width;
		segment_index = right_edge >> shamt;
		if (segment_right <= right) {
			area += AREAS[scale][segment_index];
			right_edge = segment_right;
			info.scale[scale].total = area; // -------------------------------------------------------------------------------- DEBUG
			info.scale[scale].right_block = AREAS[scale][segment_index]; // --------------------------------------------------- DEBUG
			info.scale[scale].right_index = segment_index; // ----------------------------------------------------------------- DEBUG
			info.scale[scale].right_edge = right_edge; // ----------------------------------------------------------------- DEBUG
		}
		// Left side
		next_segment_index = left_edge >> shamt;
		if (next_segment_index == 0)
			continue;
		segment_index = next_segment_index - 1;
		segment_left = left_edge - segment_width;
		if (left <= segment_left) {
			area += AREAS[scale][segment_index];
			left_edge = segment_left;
			info.scale[scale].total = area; // -------------------------------------------------------------------------------- DEBUG
			info.scale[scale].left_block = AREAS[scale][segment_index]; // --------------------------------------------------- DEBUG
			info.scale[scale].left_index = segment_index; // ----------------------------------------------------------------- DEBUG
			info.scale[scale].left_edge = left_edge; // ----------------------------------------------------------------- DEBUG
		}
	}

	// =================================================================
	// Stage 3 -- Taking integrals of sub-scale segments.
	shamt = 53;

	area += SubscaleIntegral_x60_v63(_aspline_coefs[right >> shamt], right_edge, right);
	info.right_subscale.left_edge = left_edge;
	info.right_subscale.right_edge = right_edge;
	info.right_subscale.total = area;
	info.right_subscale.right_index = right >> shamt;
	info.right_subscale.right_block = SubscaleIntegral_x60_v63(_aspline_coefs[right >> shamt], right_edge, right);


	area += SubscaleIntegral_x60_v63(_aspline_coefs[left >> shamt], left, left_edge);
	info.left_subscale.left_edge = left_edge;
	info.left_subscale.right_edge = right_edge;
	info.left_subscale.total = area;
	info.left_subscale.left_index = left >> shamt;
	info.left_subscale.left_block = SubscaleIntegral_x60_v63(_aspline_coefs[left >> shamt], left, left_edge);


	info.result = area; // ------------------------------------- DEBUG

	return area;
}