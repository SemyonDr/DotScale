#pragma once

#include <iomanip>
#include <sstream>
#include <cstdint>
#include <string>
#include "FixedMathTypes.h"


/// <summary>
/// I do not recommend calling functions directly 
/// if they are called many times in a loop, since inlining is hard to force.
/// Use it as a code snippet reference.
/// </summary>
class FxdMath {

public:

	//--------------------------------
	//  CONSTANTS
	//--------------------------------

	/// <summary>
	/// This masks keeps top 32 bits of 64 bit number.
	/// </summary>
	static constexpr uint64_t MASK_32_TOP = 0xFFFFFFFF00000000;
	/// <summary>
	/// This masks keeps bottom 32 bits of 64 bit number.
	/// </summary>
	static constexpr uint64_t MASK_32_BOT = 0x00000000FFFFFFFF;

	//--------------------------------
	//  MULTIPLICATION
	//--------------------------------

	/// <summary>
	/// Multiplies two fixed precision numbers. <br/>
	///		x   -- unsigned.   64 bits width.   60 bits precision. <br/>
	///		y   -- signed.     64 bits width.   60 bits precision. <br/>
	///		------------------------------------------------------ <br/>
	///		res -- signed.     64 bits width.   60 bits precision. <br/>
	/// Keeps all original precision.
	/// </summary>
	/// <param name="x">-- Unsigned. 64 bits width. 60 bits precision.</param>
	/// <param name="y">-- Signed. 64 bits width. 60 bits precision.</param>
	/// <returns>Signed. 64 bits width. 60 bits precision.</returns>
	static inline fxd64_60_t mult_s64_60(ufxd64_60_t x, fxd64_60_t y) {
		int64_t x_top = (x & MASK_32_TOP) >> 32; // 60 - 32 = 28 bits precision
		int64_t x_bot = x & MASK_32_BOT; // 60 bits precision

		int64_t y_top = (y & MASK_32_TOP) >> 32; // 60 - 32 = 28 bits precision
		int64_t y_bot = y & MASK_32_BOT; // 60 bits precision

		return ((y_top * x_top) << 4) +		// 28 + 28 = 56 bits precision. 56 + 4 = 60
			((y_top * x_bot) >> 28) +		// 28 + 60 = 88 bits precision. 88 - 28 = 60
			((y_bot * x_top) >> 28) +		// 60 + 28 = 88 bits precision. 88 - 28 = 60
			((y_bot * x_bot) >> 60);		// 60 + 60 = 120 bits precision. 120 - 60 = 60
	}


	//--------------------------------
	//  SPLIT
	//--------------------------------

	/// <summary> Separates top 32 bit part of a number and shifts if right. </summary>
	static constexpr inline fxd64_28_t top_32(fxd64_60_t val) noexcept {
		// It is important to make separation in two steps because applying the mask
		// makes intermediate result unsigned and the sign would be lost
		// if following is performed
		// return (val & MASK_32_TOP) >> 32
		fxd64_60_t res = val & MASK_32_TOP;
		return res >> 32;
	}

	/// <summary> Separates bottom 32 bit part of a number. </summary>
	static constexpr inline fxd64_60_t bot_32(fxd64_60_t val) noexcept {
		return val & MASK_32_BOT;
	}


	//--------------------------------
	//  DIVISION
	//--------------------------------

	




	//--------------------------------
	//  SQUARE ROOT
	//--------------------------------


	/// <summary>
	/// Finds square root of an integer number.
	/// Uses restorative algorithm and finds the value in 32 iterations.
	/// Returned value can fit into uint32_t.
	/// </summary>
	static inline uint64_t sqrt_64_rst(uint64_t x) {
		// Here we use x as the remainder variable
		uint64_t res = 0;
		uint64_t candidate = 0x4000000000000000; // 0100 0000 0000 0000 ...
		uint64_t term = 0;

		// Skipping leading zeroes in x
		while (candidate > x)
			candidate >>= 2;

		// Iterating
		while (candidate != 0) {
			// Next term to check
			term = res | candidate;
			if (term <= x) {
				// Reducing the remainder
				x -= term;
				// Counting the result
				res = (res >> 1)| candidate;
			}
			else
				res >>= 1;
			// Advancing to the next iteration
			candidate >>= 2;
		}

		return res;
	}



	//--------------------------------
	//  CONVERSION TO DOUBLE
	//--------------------------------


	static inline double to_double(uint64_t val, int precision) {
		uint64_t mask_bot = 0xFFFFFFFFFFFFFFFF >> (64 - precision);
		double res = static_cast<double>((val & (~mask_bot)) >> precision);
		res += static_cast<double>(val & mask_bot) / static_cast<double>(1 << precision);
		return res;
	}

	static inline double to_double_32(ufxd64_32_t val) {
		double res = static_cast<double>(val >> 16);
		res += static_cast<double>(val & 0xFFFF) / 65536.0;
		return res;
	}

	static inline double to_double_60(ufxd64_60_t val) {
		double res = static_cast<double>(val >> 60);
		res += static_cast<double>(val & 0xFFFFFFFFFFFFFFF) / 1152921504606846976.0;
		return res;
	}

	//--------------------------------
	//  CONVERSION FROM DOUBLE
	//--------------------------------

	/// <summary>
	/// Assumes that double is non-negative.
	/// </summary>
	static inline uint64_t from_double(double val, int precision) {
		double val_int;
		double val_fract = std::modf(val, &val_int);
		uint64_t res = static_cast<uint64_t>(val_int) << precision;
		res += static_cast<uint64_t>(val_fract * static_cast<double>(1 << precision));
		return res;
	}

	/// <summary>
	/// Assumes that double is non-negative.
	/// </summary>
	static inline ufxd64_60_t from_double_u60(double val) {
		double val_int;
		double val_fract = std::modf(val, &val_int);
		ufxd64_60_t res = static_cast<uint64_t>(val_int) << 60;
		res += static_cast<uint64_t>(val_fract * 1152921504606846976.0);
		return res;
	}

	/// <summary>
	/// Assumes that double is non-negative.
	/// </summary>
	static inline ufxd64_32_t from_double_u32(double val) {
		double val_int;
		double val_fract = std::modf(val, &val_int);
		ufxd64_32_t res = static_cast<uint64_t>(val_int) << 32;
		res += static_cast<uint64_t>(val_fract * 4294967296.0);
		return res;
	}

	//--------------------------------
	//  STRING REPRESENTATION
	//--------------------------------

	inline static const std::string legend_bin_64 = "_       8         7         6         5         4         3         2         1";

	static inline std::string to_str_fxd_u64(uint64_t val, int precision) {
		std::ostringstream oss;
		uint64_t top = val >> precision;
		uint64_t bot = val - (top << precision);
		oss << "fxd_" << (top) << "." << std::setw(DECIMAL_WIDTH[precision]) << std::setfill('0') << bot;
		return oss.str();
	}

	static inline std::string to_str_binary_u32(uint32_t val) {
		std::ostringstream oss;
		uint32_t mask = 0x80000000;
		for (int i = 0; i < 32; i++) {
			if ((val & mask) == 0)
				oss << 0;
			else
				oss << 1;
			if ((i + 1) % 4 == 0)
				oss << " ";
			mask >>= 1;
		}
		return oss.str();
	}

	static inline std::string to_str_binary_u64(uint64_t val) {
		std::ostringstream oss;
		uint64_t mask = 0x8000000000000000;
		for (int i = 0; i < 64; i++) {
			if ((val & mask) == 0)
				oss << 0;
			else
				oss << 1;
			if ((i + 1) % 4 == 0)
				oss << " ";
			mask >>= 1;
		}
		return oss.str();
	}

private:

	//--------------------------------
	//  CONSTANTS
	//--------------------------------

	constexpr static uint16_t DECIMAL_WIDTH[65] = {
		0,				//  0                         ..
		1, 1, 1,		//  1                          1		 2                         3		 3                         7
		2, 2, 2,		//  4                         15		 5                        31		 6                        63
		3, 3, 3,		//  7                        127		 8                       255		 9                       511
		4, 4, 4, 4,		// 10                      1,023		11                     2,047		12                     4,095	13                     8,191
		5, 5, 5,		// 14					  16,383		15                    32,767		16                    65,535
		6, 6, 6,		// 17                    131,071		18                   262,143		19                   524,287
		7, 7, 7, 7,		// 20				   1,048,575		21                 2,097,151		22                 4,194,303	23                 8,388,607
		8, 8, 8,		// 24                 16,777,215		25                33,554,431		26                67,108,863
		9, 9, 9,		// 27                134,217,727		28               268,435,455		29               536,870,911
		10, 10, 10, 10, // 30              1,073,741,823		31             2,147,483,647		32             4,294,967,295	33             8,589,934,591
		11, 11, 11,		// 34             17,179,869,183		35            34,359,738,367		36            68,719,476,735
		12, 12, 12,		// 37            137,438,953,471		38           274,877,906,943		39           549,755,813,887
		13, 13, 13, 13, // 40          1,099,511,627,775		41         2,199,023,255,551        42         4,398,046,511,103	43         8,796,093,022,207
		14, 14, 14,		// 44         17,592,186,044,415		45        35,184,372,088,831		46        70,368,744,177,663
		15, 15, 15,		// 47        140,737,488,355,327		48       281,474,976,710,655		49       562,949,953,421,311
		16,	16,	16, 16,	// 50      1,125,899,906,842,623		51     2,251,799,813,685,247		52     4,503,599,627,370,495	53     9,007,199,254,740,991
		17,	17, 17,		// 54     18,014,398,509,481,983		55    36,028,797,018,963,968		56    72,057,594,037,927,935
		18,	18, 18,		// 57    144,115,188,075,855,871		58   288,230,376,151,711,743		59   576,460,752,303,423,487
		19,	19, 19, 19, // 60  1,152,921,504,606,846,975		61 2,305,843,009,213,693,951		62 4,611,686,018,427,387,903	63 9,223,372,036,854,775,807
		20				// 64 18,446,744,073,709,551,615
	};
};


