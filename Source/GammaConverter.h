#pragma once
//STL
#include <cmath>
//Third party
#include "oneapi\tbb.h"
//Internal
#include "ImageBuffer_Byte.h"

//Constants definitions to improve code readibility
#define WIDTH_8BIT 256
#define WIDTH_16BIT 65536
#define MAX_8BIT 255
#define MAX_8BIT_DOUBLE 255.0
#define MAX_16BIT 65535
#define MAX_16BIT_DOUBLE 65535.0


/// <summary>
/// Base abstract class for gamma correction converters.
/// Derived classes define conversion tables initialization methods.
/// </summary>
///<remarks>
/// Values stored in image files are gamma-corrected and percieved brightnesses are not distibuted uniformly trough the values. (for example 50% brightness is 187 instead of 127 for 8 bit and 2.2 gamma).
/// To process image (i.e. downscale) we should correct for this non-uniformity and recalculate values so brightness is uniformly distributed between them. (so 127 will represent 50% brightness).
/// Linear brightness will be stored in unsigned integer 16bit [0..65535] to preserve precision for calculations.
/// This class uses lookup tables for the conversion to and from linear brightness scale.
/// Tables are created on-demand when first requested.
///</remarks>
class GammaConverter
{
public:
	//--------------------------------
	//	PUBLIC CONVERSION METHODS
	//--------------------------------

	/// <summary>
	/// Converts image with linear scale brightness values [0..65535] to gamma-corrected color space.
	/// Resulting image bit depth is specified in bitDepth argument.
	/// </summary>
	ImageBuffer_Byte* ApplyGammaCorrection(ImageBuffer_uint16* linear_image, BitDepth bitDepth);

	/// <summary>
	/// Converts image brightness values from gamma-corrected color space to brightness on the linear scale [0..65535].
	/// </summary>
	ImageBuffer_uint16* RemoveGammaCorretion(ImageBuffer_Byte* corrected_image);

protected:
	//--------------------------------
	//  CONVERSION TABLES
	//--------------------------------

	//Table index is gamma corrected brightness and the value is corresponding value on linear brightness scale, 16bit
	uint16_t* _table_toLinear_8bit = NULL;
	uint16_t* _table_toLinear_16bit = NULL;

	//Table index is a value on linear brightness scale and value is corresponding gamma corrected value - 8 or 16 bit
	uint8_t* _table_toGamma_8bit = NULL;
	uint16_t* _table_toGamma_16bit = NULL;



	//--------------------------------
	//  INITIALIZATION FLAGS
	//--------------------------------

	bool _is_table_toLinear_8bit_initialized = false;
	bool _is_table_toLinear_16bit_initialized = false;
	bool _is_table_toGamma_8bit_initialized = false;
	bool _is_table_toGamma_16bit_initialized = false;



	//--------------------------------
	//	TABLE INITIALIZATION METHODS
	//--------------------------------

	void InitializeTable_ToLinear_8bit();
	void InitializeTable_ToLinear_16bit();

	void InitializeTable_ToGamma_8bit();
	void InitializeTable_ToGamma_16bit();



	//--------------------------------
	//	TABLE BUILDING METHODS - 
	//  TO BE DEFINED BY CONCRETE CLASSES
	//--------------------------------

	virtual void FillTableToLinear_8bit() = 0;
	virtual void FillTableToLinear_16bit() = 0;

	virtual void FillTableToGamma_8bit() = 0;
	virtual void FillTableToGamma_16bit() = 0;



	//--------------------------------
	//  CONSTRUCTOR (HIDDEN)
	//--------------------------------

	GammaConverter() {};
};