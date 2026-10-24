#pragma once

#include <stdexcept>
#include <string>

namespace dotscale {


enum class CodecExceptionType {
	Jpeg_InitError,
	Jpeg_DecodingError,
	Jpeg_EncodingError,

	Png_InitError,
	Png_DecodingError,
	Png_EncodingError
};


///<summary>
///Exception object for image codec errors.
///</summary>
class codec_fatal_exception : public std::runtime_error {
public:

	CodecExceptionType type() { return _type; }

	std::string codec_message() { return _codec_message; }

	std::string full_message() {
		if (_codec_message != "")
			return codec_fatal_exception::_type_to_prefix(_type) + std::string(" with message: \"") + _codec_message + std::string("\".");
		else
			return codec_fatal_exception::_type_to_prefix(_type) + std::string("\".");
	}

	codec_fatal_exception(CodecExceptionType exception_type, std::string codec_message) : std::runtime_error(codec_message) {
		_codec_message = codec_message;
		_type = exception_type;
	}



private:
	std::string _codec_message;
	CodecExceptionType _type;

	static std::string _type_to_prefix(CodecExceptionType type) {
		switch (type) {
		case CodecExceptionType::Jpeg_InitError:
			return "JPEG codec initialization failed";
		case CodecExceptionType::Jpeg_DecodingError:
			return "JPEG decoding failed";
		case CodecExceptionType::Jpeg_EncodingError:
			return "JPEG encoding failed";
		case CodecExceptionType::Png_InitError:
			return "PNG codec initialization failed";
		case CodecExceptionType::Png_DecodingError:
			return "PNG decoding failed";
		case CodecExceptionType::Png_EncodingError:
			return "PNG encoding failed";
		default:
			return "";
		}
	}
};



} // namespace dotscale 