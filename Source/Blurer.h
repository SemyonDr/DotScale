#pragma once

#include "FixedMath.h"
#include "SliceProcessor.h"
#include "ImageBuffer.h"
#include "NormalDistribution.h"

/// <summary>
/// This object applies Gaussian Blur effect.
/// </summary>
class Blurer : SliceProcessor {

public:
	//--------------------------------
	//	PUBLIC CONSTRUCTOR
	//--------------------------------


	/// <summary>
	/// Sets up the blurer for processing an image with given parameters.
	/// Min radius is 0.5. Smaller values will be rounded up to 0.5.
	/// </summary>
	Blurer(ImagePixelLayout layout, uint32_t height, uint32_t width, ufxd32_16_t radius)
		: _layout(layout), _height(height), _width(width) {
		
		// Clamping radius value
		if (radius < HALF_32_16)
			_radius = HALF_32_16;
		else
			_radius = radius;

		// Calculating distance weights
		InitializeDistanceWeights(radius);

		//Allocating extension buffers
		_ext_top = ImageBuffer_uint16(height, width, layout);
		
	
	}

	/// <summary>
	/// Calculates pixel distance weights matrix for given radius.
	/// </summary>
	void InitializeDistanceWeights(ufxd32_16_t radius) {
		/*	Integer radius value does not include the central pixel
			[6][5][4][3][2][1][0][1][2][3][4][5][6]
			^---radius_int---^   ^---radius_int---^ */

		// Extracting integer radius.
		// We give whole pixel for the fractional part if it is present.
		_radius_int = static_cast<uint16_t>(((radius - HALF_32_16)  & FxdMath::MASK_32_16_TOP) >> 16);
		bool have_fraction = false;
		if ((radius & FxdMath::MASK_32_16_BOT) != 0) {
			_radius_int++;
			have_fraction = true;
		}

		// Initializing weigths array
		// +1 is for the central pixel weight
		_dweights = new ufxd32_32_t[_radius_int + 1];

		// We map given blur radius to Gauss Curve function x axis.
		// radius value is mapped to x=GF_CUTOFF on the x axis of Gauss Curve.
		
		
		// WORK IN PROGRESS
		// To find width of one pixel interval on gauss function scale we divide pwidth=GF_CUTOFF/radius.
		// For division we use relation pwidth*radius = GF_CUTOFF.
		// radius is 16 bit precision.
		// GF_CUTOFF is 62 bit precision.
		// Result is 60 bit precision.
		// pwidth is cut into 1 bit cand digits each of which is checked separately and has the same precision as the result.
		// radius*cand produces values with precision 16+60=76. 
		// We need to shift them right by 14 before comparison with GF_CUTOFF 62 bit remainder.
		// Or we can start with cand that is already shifted right by 14. 
		// We would need to remove this pre shift and add res shift when only 14 cand values would be left.
		ufxd64_16_t radius64 = static_cast<ufxd64_16_t>(radius); // Casting radius to wider type
		ufxd64_62_t cutoff = GF_CUTOFF; // We will modify this variable during division, so value is copied
		ufxd64_62_t term = 0;   // cand*radius in the same precision as the cutoff.
		ufxd64_60_t pwidth = 0; // Pixel width -- the result. 60 bit precision
		ufxd64_60_t cand = 0x2000000000000; // 2^63 >> 14 = 2^49 Same precision as the result
		uint64_t cand_top = 0x2000; // cand >> 32
		int cand_index = 49; // 63-14
		int cand_index_top = 17; 

		// Part 1: 
		// cand is shifted right by 14.
		// cand is still 32 bit or more wide after this 14-bit shift
		// cand can cause overflow over 64bit when multiplied with radius.
		// First we check for overflow and then for the actual term if it is not overflowing.
		// To check for overflow we shift cand by 32 right and check if the resulting term is larger than 32 bit width. 
		// Shift 14+32=46 cand index range [17..00]
		// Shift 14 cand index range [49..32]
		// Actual cand index range [63..46]
		while (cand_index >= 32) {
			// Checking for overflow
			// Precision: 16 + (60-14-32) = 16+14 = 30
			// Max width: 32 + (64-14-32) = 32 + 18 = 50
			term = radius64 << (cand_index - 32);
		
			if ((term & FxdMath::MASK_64_32_TOP) == 0) { 
				// shifted term bits over 31st are 0 -- no overflow
				// Checking actual term
				// Precision: 16 + (60-14) = 62
				// Width max is 64 bit after the overflow check
				term = radius64 << cand_index;

				if (term <= cutoff) {
					// Writing down result bit and updating remainder
					pwidth = pwidth | (cand << 14); // (remember that cand was shifted by 14)
					cutoff -= term;
				}
			}
			cand_index--;
			cand >>= 1;
		}

		// Part 2:
		// cand is shifted right by 14
		// cand is still 14 bit or more wide after the shift
		// There will be no overflow at this point
		// Shift 14 cand index range [31..00]
		// Actual cand index range [45..14]
		while (cand_index >= 0) {
			// Precision: 16 + (60-14) = 62
			// Max width: 32 + (46-14) = 32 + 32 = 64
			term = radius64 << cand_index;
			
			if (term <= cutoff) {
				pwidth = pwidth | (cand << 14);
				cutoff -= term;
			}

			cand_index--;
			cand >>= 1;
		}

		// Part 3:
		// cand is not shifted anymore
		// Actual cand index range [13..00]
		cand = 0x2000;
		cand_index = 13;
		while (cand_index >= 0) {
			// Precision: (16 + 62) - 14 = 76 - 14 = 62
			// Max width: (32 + 14) - 14 = 16
			term = (radius64 << cand_index) >> 14;

			if (term <= cutoff) {
				pwidth = pwidth | cand;
				cutoff -= term;
			}
			cand_index--;
			cand >>= 1;
		}

		// Now we find the actual pixel weights. 
		// Each weight proportionally is the area of the gauss function slice for this pixel.
		// Areas for the whole span from -3.0 to 3.0 must add up to 1.0.
		// So, we divide the area of each slice by the total area, which we know in advance.
		// Decimal value for the area of gaussian function e^(-x^2/2) from -3.0 to 0.3 is (source - wolframalpha)
		// 2.4998608894830950472438318293828940282117103010647686313772283534
		// We can fit it with 62 bit fractional precision.
		ufxd64_62_t nf = 11528573512042642983; // Total area - normalizing factor
		// 
		// We kind of don't really need these variables,  because we can have a shortcut...
		// Division for normalization here is Weight = SliceArea/TotalArea, or W=A/N to short.
		// To actually divide we split W to candidate bits and check them according to equation A = W*N.
		// But because on each step W is reduced to one candidate bit and N is const we can pre-calculate
		// values of cand*N and then simply run them against the given slice area.


		// Slice span. First pixel is split in halves.
		ufxd64_60_t left = 0;
		ufxd64_60_t right = pwidth >> 1;
		ufxd64_32_t pweight64_raw = 2 * GaussCurve::Area_x60_v63(left, right); // not normalized
		// Advancing the span
		left = right;
		right += pwidth;
		_dweights[0] = 0;

		// Normalizing the first pixel
		// cand is (Total area * 2^31) >> 32 or just TotalArea/2 where TotalArea is 62bit fractional precision
		// Total Area is normalizing factor nf
		// cand is also 62 bit precision
		ufxd64_62_t cand = 5764286756021321491;
		uint32_t bit = 0x80000000; // 1000 0000.0000 0000.0000 0000.0000 0000
		while (bit != 0) {
			if (cand < pweight64_raw) {
				_dweights[0] &= bit;
				pweight64_raw -= cand;
			}
			cand >>= 1;
			bit >>= 1;
		}

		// Finding the rest of the weights and normalizing as we go
		for (int i = 1; i < _radius_int; i++) {
			pweight64_raw = GaussCurve::Area_x60_v63(left, right);
			// Advancing the span
			left = right;
			right += pwidth;
			// Normalizing
			cand = 5764286756021321491;
			bit = 0x80000000; // 1000 0000.0000 0000.0000 0000.0000 0000
			while (bit != 0) {
				if (cand < pweight64_raw) {
					_dweights[i] &= bit;
					pweight64_raw -= cand;
				}
				cand >>= 1;
				bit >>= 1;
			}
		}

		// DEBUG

		// TODO REMINDER
		/*
			You map radius value to x=3 and slice ND function up to x=3 into r+0.5 intervals.
			For each interval you find AREA (not average!).
			Then you need to normalize weights you have found, so they would add up to 1!
			Also you need to look into issues of this normalization, since adding weigths to 1 might cause overflow, so be careful.

			You can precalculate normalization constant which is integral of GF from -GF_CUTOFF to +GF_CUTOFF.
			For GF_CUTOFF = 3.0 it is around 2.5.
		*/

		
	}

	//--------------------------------
	//	DESTRUCTOR
	//--------------------------------

	~Blurer() {
		delete[] _dweights;
	
	}

protected:

	//--------------------------------
	//	CONSTANTS
	//--------------------------------

	/// <summary>
	/// Given radius is mapped onto gauss function up to this point.
	/// x = 3.0 with 62 bit precision.
	/// </summary>
	static constexpr ufxd64_62_t GF_CUTOFF = 0xC000000000000000;

	

	/// <summary>
	/// 0.5 in ufxd32_16_t.
	/// </summary>
	static constexpr ufxd32_16_t HALF_32_16 = 0x8000;

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	ufxd32_32_t* _dweights = nullptr;
	// Used for pixel weight normalization division process.
	// See InitializeDistanceWeights() for details.
	inline static const ufxd64_62_t ncand[64] = {
		0x4FFEDC439C23BF13,
		0x27FF6E21CE11DF89,
		0x13FFB710E708EFC4,
		0x09FFDB88738477E2,
		0x04FFEDC439C23BF1,
		0x027FF6E21CE11DF8,
		0x013FFB710E708EFC,
		0x009FFDB88738477E,
		0x004FFEDC439C23BF,
		0x0027FF6E21CE11DF,
		0x0013FFB710E708EF,
		0x0009FFDB88738477,
		0x0004FFEDC439C23B,
		0x00027FF6E21CE11D,
		0x00013FFB710E708E,
		0x00009FFDB8873847,
		0x00004FFEDC439C23,
		0x000027FF6E21CE11,
		0x000013FFB710E708,
		0x000009FFDB887384,
		0x000004FFEDC439C2,
		0x0000027FF6E21CE1,
		0x0000013FFB710E70,
		0x0000009FFDB88738,
		0x0000004FFEDC439C,
		0x00000027FF6E21CE,
		0x00000013FFB710E7,
		0x00000009FFDB8873,
		0x00000004FFEDC439,
		0x000000027FF6E21C,
		0x000000013FFB710E,
		0x000000009FFDB887,
	};


	//--------------------------------
	//	IMAGE PARAMETERS
	//--------------------------------

	ImagePixelLayout _layout;
	uint32_t _height;
	uint32_t _width;
	ufxd32_16_t _radius;
	uint16_t _radius_int;

	//--------------------------------
	//	IMAGE EXTENSION BUFFERS
	//--------------------------------

	ImageBuffer_uint16 _ext_top;
	ImageBuffer_uint16 _ext_bot;
	ImageBuffer_uint16 _ext_left;
	ImageBuffer_uint16 _ext_right;
	ImageBuffer_uint16 _ext_top_left;
	ImageBuffer_uint16 _ext_top_right;
	ImageBuffer_uint16 _ext_bot_left;
	ImageBuffer_uint16 _ext_bot_right;


};

