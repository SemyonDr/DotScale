#pragma once
#include <cmath>
#include "GammaConverter.h"


class GammaConverter_PlainGamma : public GammaConverter
{
public:
	//--------------------------------------------
	//	GET/SET
	//--------------------------------------------

	double GetGamma() { return gamma; }

	//--------------------------------------------
	//	CONSTRUCTORS
	//--------------------------------------------

	GammaConverter_PlainGamma(double gamma) {
		this->gamma = gamma;
	}

private:
	//--------------------------------------------
	//	PRIVATE DATA
	//--------------------------------------------
	double gamma = 0.0;



	//--------------------------------------------
	//	TO LINEAR TABLE BUILDING METHODS
	//--------------------------------------------

	/// <summary>
	/// Builds conversion table for 8 bit plain gamma corrected values.
	/// Table is used to convert plain gamma corrected brightness value (non-linear) to linear scale normalized to [0..1].
	/// </summary>
	void FillTableToLinear_8bit() override;

	/// <summary>
	/// Builds conversion table for 16 bit plain gamma corrected values.
	/// Table is used to convert plain gamma corrected brightness value (non-linear) to linear scale normalized to [0..1].
	/// </summary>
	void FillTableToLinear_16bit() override;



	//--------------------------------------------
	//	FROM LINEAR TABLE BUILDING METHODS
	//--------------------------------------------

	/// <summary>
	/// Builds conversion table for 8 bit plain gamma corrected values.
	/// Table is used to convert linear scale brightness values (normalized [0..1] to plain gamma corrected brightness value.
	/// </summary>
	void FillTableToGamma_8bit() override;

	/// <summary>
	/// Builds conversion table for 16 bit plain gamma corrected values.
	/// Table is used to convert linear scale brightness values (normalized [0..1] to plain gamma corrected brightness value.
	/// </summary>
	void FillTableToGamma_16bit() override;

};

