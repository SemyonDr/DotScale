#pragma once
#include <cmath>
#include "GammaConverter.h"

class GammaConverter_sRGB : public GammaConverter
{
private:
	//--------------------------------------------
	//	PRE-CALCULATED TABLES
	//--------------------------------------------

	static constexpr uint16_t _table_toLinear_8bit_data[] = { 
		#include ".\data\table_SRGBtoLinear_8.data" 
	};
	static constexpr uint16_t _table_toLinear_16bit_data[] = {
		#include ".\data\table_SRGBtoLinear_16.data"
	};

	//Table index is a value on linear brightness scale and value is corresponding gamma corrected value - 8 or 16 bit
	static constexpr uint8_t _table_toSRGB_8bit_data[] = {
		#include ".\data\table_LinearToSRGB_8.data"
	};
	static constexpr uint16_t _table_toSRGB_16bit_data[] = {
		#include ".\data\table_LinearToSRGB_16.data"
	};

	//--------------------------------------------
	//	TO LINEAR TABLE BUILDING METHODS
	//--------------------------------------------

	/// <summary>
	/// Builds conversion table for 8 bit sRGB values.
	/// Table is used to convert sRGB brightness value (non-linear) to linear scale normalized to [0..1].
	/// </summary>
	void FillTableToLinear_8bit() override;

	/// <summary>
	/// Builds conversion table for 16 bit sRGB values.
	/// Table is used to convert sRGB brightness value (non-linear) to linear scale normalized to [0..1].
	/// </summary>
	void FillTableToLinear_16bit() override;


	//--------------------------------------------
	//	FROM LINEAR TABLE BUILDING METHODS
	//--------------------------------------------

	/// <summary>
	/// Builds conversion table for 8 bit sRGB values.
	/// Table is used to convert linear scale brightness values (normalized [0..1] to sRGB brightness value.
	/// </summary>
	void FillTableToGamma_8bit() override;

	/// <summary>
	/// Builds conversion table for 16 bit sRGB values.
	/// Table is used to convert linear scale brightness values (normalized [0..1] to sRGB brightness value.
	/// </summary>
	void FillTableToGamma_16bit() override;

};

