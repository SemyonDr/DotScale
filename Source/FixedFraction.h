#pragma once
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <string>

//Fixed resolution fractional, 16 bit for integer part, 16 bit for fractional part, see notes.
using fxdfrc_t = uint32_t;

inline double fxdfrc_to_double(fxdfrc_t val) {
	double res = static_cast<double>((val >> 16));
	res += static_cast<double>(val & 0x0000FFFF) / 65536.0;
	return res;
}

inline std::string fxdfrc_to_string(fxdfrc_t val) {
	std::ostringstream oss;
	oss << "fxd_" << (val >> 16) << "." << std::setw(5) << std::setfill('0') << (val & 0x0000FFFF);
	return oss.str();
}