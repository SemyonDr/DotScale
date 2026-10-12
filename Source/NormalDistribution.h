#pragma once

#include "GaussCurve.h"


/// <summary>
/// Represents Normal Distribution function.
/// 1/(sigma*sqrt(2pi)) * e^(-x^2/2sigma^2)
/// </summary>
class NormalDistribution {
private:
	//--------------------------------
	//  NORMALIZATION CONSTANT
	//--------------------------------

	/*
		Normalization constant is
		1/sqrt(2pi) = 0.39894228040143267793994605993438
	*/

	/// <summary>
	/// Normalization constant.
	/// 64-bit precision.
	/// </summary>
	static constexpr ufxd64_64_t NC_64 = 7359186146747302452;

	/// <summary>
	/// Normalization constant.
	/// 32-bit precision.
	/// </summary>
	static constexpr ufxd32_32_t NC_32 = 1713444047;

	/// <summary>
	/// Top part of the normalization constant.
	/// 32-bit precision.
	/// </summary>
	static constexpr ufxd64_32_t NC_64_TOP = 1713444047;

	/// <summary>
	/// Bottom part of the normalization constant.
	/// 64-bit precision.
	/// </summary>
	static constexpr ufxd64_64_t NC_64_BOT = 1356415540;

	//--------------------------------
	//  NORMALIZATION METHODS
	//--------------------------------

	/// <summary>
	/// Applies normalization constant.
	/// Sigma is assumed to be 1.
	/// </summary>
	/// <param name="val"> -- 32-bit precision 32-bit value.</param>
	/// <returns>32-bit precision 32-bit value (less than 1).</returns>
	static ufxd32_32_t Normalize_x32_v32(ufxd32_32_t val) {
		return static_cast<ufxd32_32_t>((
			(val * NC_64_TOP) +			// 32 + 32 = 64
			((val * NC_64_BOT) << 32)	// 32 + 64 = 96; 96 - 32 = 64
			) >> 32);					// 64 - 32 = 32
	}

	/// <summary>
	/// Applies normalization constant.
	/// Sigma is assumed to be 1.
	/// </summary>
	/// <param name="val">32-bit precision 32-bit value (less than 1).</param>
	/// <returns>64-bit precision 64-bit value.</returns>
	static ufxd64_64_t Normalize_x32_v64(ufxd32_32_t val) {
		return (
			(val * NC_64_TOP) +			// 32 + 32 = 64
			((val * NC_64_BOT) << 32)	// 32 + 64 = 96; 96 - 32 = 64
			);
	}

public:

	//--------------------------------
	//  POINT VALUE -- SIGMA = 1
	//--------------------------------

	/// <summary>
	/// Returns function value at point x.
	/// Sigma is assumed to be 1.
	/// </summary>
	/// <param name="x"> -- Signed 16 bit precision.</param>
	/// <returns>Function value at x, 32 bit precision.</returns>
	static ufxd32_32_t Value_x16(fxd32_16_t x) {
		if (x == 0)
			return NC_32;
		if (x < 0)
			x *= -1;
		ufxd32_32_t gval = GaussCurve::Value_x16_v32(static_cast<ufxd32_16_t>(x));
		// Dividing by sqrt(2pi)
		return static_cast<ufxd32_32_t>(((gval * NC_64_TOP) + ((gval * NC_64_BOT) << 32)) >> 32);					
	}

	/// <summary>
	/// Returns function value at point x.
	/// Sigma is assumed to be 1.
	/// </summary>
	/// <param name="x"> -- Signed 32 bit precision.</param>
	/// <returns>Function value at x, 32 bit precision.</returns>
	static ufxd32_32_t Value_x32(fxd64_32_t x) {
		if (x == 0)
			return NC_32;
		if (x < 0)
			x *= -1;
		ufxd32_32_t gval = GaussCurve::Value_x32_v32(static_cast<ufxd64_32_t>(x));
		// Dividing by sqrt(2pi)
		return static_cast<ufxd32_32_t>(((gval * NC_64_TOP) + ((gval * NC_64_BOT) << 32)) >> 32);
	}

	/// <summary>
	/// Returns function value at point x.
	/// Sigma is assumed to be 1.
	/// </summary>
	/// <param name="x"> -- Signed 60 bit precision.</param>
	/// <returns>Function value at x, 32 bit precision.</returns>
	static ufxd32_32_t Value_x60(fxd64_60_t x) {
		if (x == 0)
			return NC_32;
		if (x < 0)
			x *= -1;
		ufxd32_32_t gval = GaussCurve::Value_x60_v32(static_cast<ufxd64_60_t>(x));
		// Dividing by sqrt(2pi)
		return static_cast<ufxd32_32_t>(((gval * NC_64_TOP) + ((gval * NC_64_BOT) << 32)) >> 32);
	}


	//--------------------------------
	//  AREA
	//--------------------------------

	/// <summary>
	/// Calculates area under the function, or function integral on an interval.
	/// Expects interval boundaries with 60-bit precision.
	/// Returns area with 63-bit precision.
	/// Assumes that left <= right
	/// </summary>
	static ufxd64_63_t Area_x60_v63(fxd64_60_t left, fxd64_60_t right) {
		// We have 4 cases:
		// - 0 width
		// - Whole interval is on the positive side
		// - Whole interval is on the negative side
		// - Interval spans negative to positive
	
		if (left == right)
			return 0;

		// Positive side
		if (left >= 0 && right >= 0) {
			ufxd64_63_t garea = GaussCurve::Area_x60_v63(static_cast<ufxd64_60_t>(left), static_cast<ufxd64_60_t>(right));
			ufxd64_31_t garea_top = garea >> 32;
			ufxd64_63_t garea_bot = garea & FxdMath::MASK_64_32_BOT;
			return (garea_top * NC_64_TOP) +	// 31 + 32 = 63
				((garea_top * NC_64_BOT +	garea_bot * NC_64_TOP) >> 32); 
				// 31 + 64 = 95 and 63 + 32 = 95
				// 95 - 32 = 63				
				// ((garea_bot * NC_64_BOT) >> 64) -- the last term is dropped		
				// 63 + 64 - 64 = 63
				// No reason to calculate this because
				// the value will be too small to represent in 64 bit variable
		}

		// Negative side
		if (left <= 0 && right <= 0) {
			ufxd64_63_t garea = GaussCurve::Area_x60_v63(static_cast<ufxd64_60_t>(-1*right), static_cast<ufxd64_60_t>(-1*left));
			ufxd64_31_t garea_top = garea >> 32;
			ufxd64_63_t garea_bot = garea & FxdMath::MASK_64_32_BOT;
			return (garea_top * NC_64_TOP) +
				((garea_top * NC_64_BOT + garea_bot * NC_64_TOP) >> 32);
		}

		// Spans negative and positive
		ufxd64_63_t garea_neg = GaussCurve::Area_x60_v63(0, static_cast<ufxd64_60_t>(-1 * left));
		ufxd64_63_t garea_pos = GaussCurve::Area_x60_v63(0, static_cast<ufxd64_60_t>(right));
		// Normalizing negative
		ufxd64_31_t garea_neg_top = garea_neg >> 32;
		ufxd64_63_t garea_neg_bot = garea_neg & FxdMath::MASK_64_32_BOT;
		ufxd64_63_t area_neg = (garea_neg_top * NC_64_TOP) + ((garea_neg_top * NC_64_BOT + garea_neg_bot * NC_64_TOP) >> 32);
		// Normalizing positive
		ufxd64_31_t garea_pos_top = garea_pos >> 32;
		ufxd64_63_t garea_pos_bot = garea_pos & FxdMath::MASK_64_32_BOT;
		ufxd64_63_t area_pos = (garea_pos_top * NC_64_TOP) + ((garea_pos_top * NC_64_BOT + garea_pos_bot * NC_64_TOP) >> 32);
		// Returning total sum
		return area_neg + area_pos;

	}



	//--------------------------------
	//  AVERAGE VALUE
	//--------------------------------

	/// <summary>
	/// Finds average function value on given interval.
	/// Interval is given with 60-bit precision.
	/// Average value is returned with 32-bit precision.
	/// Assumed that left <= right.
	/// </summary>
	static ufxd32_32_t Average_x60_v32(fxd64_60_t left, fxd64_60_t right) {
		// We have 4 cases:
		// - 0 width
		// - Whole interval is on the positive side
		// - Whole interval is on the negative side
		// - Interval spans negative to positive
		// First three cases are trivial since they boil down to calling GaussCurve:Average(),
		// but the third is more involved.

		if (left == right) {
			return Value_x60(left);
		}

		// On positive half
		if (left >= 0 && right >= 0) {
			ufxd32_32_t gval = GaussCurve::Average_x60_v32(static_cast<ufxd64_60_t>(left), static_cast<ufxd64_60_t>(right));
			return static_cast<ufxd32_32_t>((
				(gval * NC_64_TOP) +		// 32 + 32 = 64
				((gval * NC_64_BOT) << 32)	// 32 + 64 = 96; 96 - 32 = 64
				) >> 32);					// 64 - 32 = 32
		}

		// On negative half
		if (left <= 0 && right <= 0) {
			ufxd32_32_t gval = GaussCurve::Average_x60_v32(static_cast<ufxd64_60_t>((-1)*right), static_cast<ufxd64_60_t>((-1)*left));
			return static_cast<ufxd32_32_t>((
				(gval * NC_64_TOP) +		// 32 + 32 = 64
				((gval * NC_64_BOT) << 32)	// 32 + 64 = 96; 96 - 32 = 64
				) >> 32);					// 64 - 32 = 32
		}

		// Spans negative and positive
		// We find two areas and then average them together
		ufxd64_63_t garea_neg = GaussCurve::Area_x60_v63(0, static_cast<ufxd64_60_t>((-1) * left));
		ufxd64_63_t garea_pos = GaussCurve::Area_x60_v63(0, static_cast<ufxd64_60_t>(right));
		// Before adding up areas to avoid overflow we shift them right one bit:
		// Max value for the area of one half is around 1.25, but two areas can add up to 2.5
		// and this requires 2 integer bits to store.
		ufxd64_62_t garea = (garea_neg >> 1) + (garea_pos >> 1);
		// Interval integer part can fit into 4 bits of unsigned number
		ufxd64_60_t width = (static_cast<ufxd64_60_t>(right) + static_cast<ufxd64_60_t>((-1)*left));
		const ufxd64_28_t w_top = width >> 32;
		const ufxd64_60_t w_bot = width & FxdMath::MASK_64_32_BOT;

		// We divide total area by interval width to find the average value
		// Look up GaussCurve::Average_x60_v32() implementation for explanations of the algorithm.
		// Note, that here area is 62 bit precision, while there it is 63 bit precision.
		uint64_t res = 0;
		uint64_t cand = 0x80000000; // 1000 0000 0000 0000 0000 0000 0000 0000
		int cand_index = 31;
		ufxd64_63_t term = 0;

		while (cand > 0x20000000) {		// cand > 0010 0000 0000 0000 0000 0000 0000 0000
			term = w_top << cand_index;

			if (term < 0x4000000000000000) { // 0100 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
				term <<= 2; // This might overflow without the check
				term += (w_bot << (cand_index - 30));
				if (term <= garea) {
					res = res | cand;
					garea -= term;
				}
			}
			cand_index--;
			cand >>= 1;
		}

		while (cand != 0) {
			term = (w_top << (cand_index + 2)) +	// 28 + 32 = 60; 60 + 2 = 62
				((w_bot << cand_index) >> 30);		// 60 + 32 = 92; 92 - 30 = 62
			if (term <= garea) {
				res = res | cand;
				garea -= term;
			}
			cand_index--;
			cand >>= 1;
		}

		// Averaging result
		ufxd32_32_t gavg = static_cast<ufxd32_32_t>(res);
		// And we still need to divide by sqrt(2pi) to transition to the normal distribution
		return static_cast<ufxd32_32_t>((
			(gavg * NC_64_TOP) +		// 32 + 32 = 64
			((gavg * NC_64_BOT) << 32)	// 32 + 64 = 96; 96 - 32 = 64
			) >> 32);					// 64 - 32 = 32
	}








};


