#include "GammaDispatcher.h"

//--------------------------------
//	DATA FIELDS INITIALIZATION
//--------------------------------

std::map<double, GammaConverter_PlainGamma*> GammaDispatcher::PG_converters;
GammaConverter_sRGB* GammaDispatcher::sRGB_conv = NULL;



GammaConverter* GammaDispatcher::GetConverter(RawImageGammaProfile colorspace, void* param) {
	double gamma = 0.0;

	switch (colorspace)
	{
	case RawImageGammaProfile::sRGB:
		if (sRGB_conv == NULL)
			sRGB_conv = new GammaConverter_sRGB();
		return static_cast<GammaConverter*>(sRGB_conv);
		break;

	case RawImageGammaProfile::PlainGamma:
		gamma = *reinterpret_cast<double*>(param);
		if (PG_converters.count(gamma) == 0) {
			GammaConverter_PlainGamma* pg_conv = new GammaConverter_PlainGamma(gamma);
			PG_converters[gamma] = pg_conv;
			return pg_conv;
		}
		else
			return PG_converters[gamma];
		break;

	default:
		return NULL;
		break;
	}
}