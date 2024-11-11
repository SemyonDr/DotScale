#include "GammaConverter_PlainGamma.h"


//--------------------------------------------
//	TO LINEAR TABLE BUILDING METHODS
//--------------------------------------------

/// <summary>
/// Builds conversion table for 8 bit plain gamma corrected values.
/// Table is used to convert plain gamma corrected brightness value (non-linear) to linear scale normalized to [0..1].
/// </summary>
void GammaConverter_PlainGamma::FillTableToLinear_8bit() {
	//Formula for conversion is (n/255)^gamma*65535.
	for (int gamma_value = 0; gamma_value < WIDTH_8BIT; gamma_value++) {
		double linear_value_float = std::pow(static_cast<double>(gamma_value) / MAX_8BIT_DOUBLE, gamma) * MAX_16BIT_DOUBLE;

		long linear_value_int = std::lround(linear_value_float);

		if (linear_value_int > MAX_16BIT)
			linear_value_int = MAX_16BIT;
		if (linear_value_int < 0)
			linear_value_int = 0;

		_table_toLinear_8bit[gamma_value] = static_cast<uint16_t>(linear_value_int);
	}
}


/// <summary>
/// Builds conversion table for 16 bit plain gamma corrected values.
/// Table is used to convert plain gamma corrected brightness value (non-linear) to linear scale normalized to [0..1].
/// </summary>
void GammaConverter_PlainGamma::FillTableToLinear_16bit() {
	//Formula for conversion is (n/65535)^gamma*65535.
	for (int gamma_value = 0; gamma_value < WIDTH_16BIT; gamma_value++) {
		double linear_value_float = std::pow(static_cast<double>(gamma_value) / MAX_16BIT_DOUBLE, gamma) * MAX_16BIT_DOUBLE;

		long linear_value_int = std::lround(linear_value_float);

		if (linear_value_int > MAX_16BIT)
			linear_value_int = MAX_16BIT;
		if (linear_value_int < 0)
			linear_value_int = 0;

		_table_toLinear_16bit[gamma_value] = static_cast<uint16_t>(linear_value_int);
	}
}



//--------------------------------------------
//	FROM LINEAR TABLE BUILDING METHODS
//--------------------------------------------

/// <summary>
/// Builds conversion table for 8 bit plain gamma corrected values.
/// Table is used to convert linear scale brightness values (normalized [0..1] to plain gamma corrected brightness value.
/// </summary>
void GammaConverter_PlainGamma::FillTableToGamma_8bit() {
	//Formula for converting back is n = ((Linear/65535)^1/gamma)*255
	double power = 1.0 / gamma;

	for (int linear_value = 0; linear_value < WIDTH_16BIT; linear_value++) {
		//Calculating
		double gamma_value_float = std::pow(static_cast<double>(linear_value) / MAX_16BIT_DOUBLE, power) * MAX_8BIT_DOUBLE;

		long gamma_value_int = std::lround(gamma_value_float);

		//Boundary checks
		if (gamma_value_int > MAX_8BIT)
			gamma_value_int = MAX_8BIT;
		if (gamma_value_int < 0)
			gamma_value_int = 0;
		//Writing the result
		_table_toGamma_8bit[linear_value] = static_cast<uint8_t>(gamma_value_int);
	}
}


/// <summary>
/// Builds conversion table for 16 bit plain gamma corrected values.
/// Table is used to convert linear scale brightness values (normalized [0..1] to plain gamma corrected brightness value.
/// </summary>
void GammaConverter_PlainGamma::FillTableToGamma_16bit() {
	//Formula for converting back is n = ((linear/65535)^1/gamma)*65535
	double power = 1.0 / gamma;

	for (int linear_value = 0; linear_value < WIDTH_16BIT; linear_value++) {
		//Calculating
		double gamma_value_float = std::pow(linear_value/MAX_16BIT_DOUBLE, power) * MAX_16BIT_DOUBLE;
		long gamma_value_int = std::lround(gamma_value_float);

		//Boundary checks
		if (gamma_value_int > MAX_16BIT)
			gamma_value_int = MAX_16BIT;
		if (gamma_value_int < 0)
			gamma_value_int = 0;

		//Writing the result
		_table_toGamma_16bit[linear_value] = static_cast<uint16_t>(gamma_value_int);
	}
}