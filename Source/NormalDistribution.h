#pragma once

#include "GaussCurve.h"


/// <summary>
/// Represents Normal Distribution function.
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
	//  POINT VALUE
	//--------------------------------

	/// <summary>
	/// Returns function value at point x.
	/// Sigma assumed to be 1.
	/// </summary>
	/// <param name="x"> -- Signed 60 bit precision.</param>
	/// <returns>Function value at x, 32 bit precision.</returns>
	static ufxd32_32_t Value_x60(fxd64_60_t x) {
		if (x < 0)
			x *= -1;
		ufxd64_32_t val_32_un = static_cast<ufxd64_32_t>(GaussCurve::Value_x60_v32(x));
		return Normalize_x32_v32(val_32_un);
	}

	/// <summary>
	/// Returns function value at point x.
	/// Sigma assumed to be 1.
	/// </summary>
	/// <param name="x"> -- Signed 60 bit precision.</param>
	/// <returns>Function value at x, 64 bit precision.</returns>
	static ufxd32_32_t Value_x60(fxd64_60_t x) {
		if (x < 0)
			x *= -1;
		ufxd64_32_t val_32_un = static_cast<ufxd64_32_t>(GaussCurve::Value_x60_v32(x));
		return Normalize_x32_v32(val_32_un);
	}



};


