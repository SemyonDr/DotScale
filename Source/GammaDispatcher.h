#pragma once
#include <map>
#include "GammaConverter.h"
#include "GammaConverter_sRGB.h"
#include "GammaConverter_PlainGamma.h"

class GammaDispatcher
{
public:
	static GammaConverter* GetConverter(RawImageGammaProfile colorspace, void* param);

private:
	//--------------------------------
	//	DATA FIELDS
	//--------------------------------

	static std::map<double, GammaConverter_PlainGamma*> PG_converters;
	static GammaConverter_sRGB* sRGB_conv;

};

