#include "GammaConverter_sRGB.h"

//--------------------------------------------
//	TO LINEAR TABLE INITIALIZATION METHODS
//--------------------------------------------

/// <summary>
/// Builds conversion table for converting 8 bit sRGB values.
/// Table is used to convert sRGB brightness value (non-linear) to 16bit linear scale [0..65535].
/// </summary>
void GammaConverter_sRGB::FillTableToLinear_8bit() {
	for (int i = 0; i < WIDTH_8BIT; i++)
		_table_toLinear_8bit[i] = _table_toLinear_8bit_data[i];
}



/// <summary>
/// Builds conversion table for 16 bit sRGB values.
/// Table is used to convert 16 bit sRGB brightness value (non-linear) to 16bit linear scale [0..65535].
/// </summary>
void GammaConverter_sRGB::FillTableToLinear_16bit() {
	for (int i = 0; i < WIDTH_16BIT; i++)
		_table_toLinear_16bit[i] = _table_toLinear_16bit_data[i];
}



//--------------------------------------------
//	FROM LINEAR TABLE INITIALIZATION METHODS
//--------------------------------------------

/// <summary>
/// Builds conversion table for 8 bit sRGB values.
/// Table is used to convert linear scale 16bit brightness values [0..65535] to sRGB 8bit brightness value.
/// </summary>
void GammaConverter_sRGB::FillTableToGamma_8bit() {
	for (int i = 0; i < WIDTH_16BIT; i++)
		_table_toGamma_8bit[i] = _table_toSRGB_8bit_data[i];
}



/// <summary>
/// Builds conversion table for 16 bit sRGB values.
/// Table is used to convert linear scale brightness values (normalized [0..1] to sRGB brightness value.
/// </summary>
void GammaConverter_sRGB::FillTableToGamma_16bit() {
	for (int i = 0; i < WIDTH_16BIT; i++)
		_table_toGamma_16bit[i] = _table_toSRGB_16bit_data[i];
}
