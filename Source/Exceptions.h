#pragma once
#include <exception>

/*
class codec_fatal_exception : public std::exception {
public:
	

	explicit codec_fatal_exception(const char* message) : msg(message) {
	
	}

	codec_fatal_exception(codec_fatal_exception const&) noexcept = default;

	codec_fatal_exception& operator=(codec_fatal_exception const&) noexcept = default;
	~codec_fatal_exception() override = default;

	const char* what() const noexcept override { return msg; }
private:
	const char* msg;
};

*/

///<summary>
///Enumeration of possible codec exceptions.
///</summary>
enum CodecExceptions {
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
	//--------------------------------
	//	GETTERS
	//--------------------------------

	///<summary>
	///Type of the exception.
	///</summary>
	CodecExceptions GetType() { return _exception_type; }

	///<summary>
	///Error message produced by the codec.
	///</summary>
	std::string GetCodecMessage() { return _codec_message; }

	///<summary>
	///Full exception message.
	///</summary>
	std::string GetFullMessage() {
		std::string full_message = ExceptionTypeToString(_exception_type);
		if (_codec_message != "")
			full_message.append(" with message: \"").append(_codec_message).append("\".");
		else
			full_message.append(".");

		return full_message;
	}



	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------

	///<summary>
	///Exception constructor.
	///</summary>
	///<param name="type">Type of the encountered exception.</param>
	///<param name="codec_message">Associated message produced by the codec.</param>
	codec_fatal_exception(CodecExceptions exception_type, std::string codec_message) : std::runtime_error(codec_message) {
		_codec_message = codec_message;
		_exception_type = exception_type;
	}



private:
	std::string _codec_message;
	CodecExceptions _exception_type;

	std::string ExceptionTypeToString(CodecExceptions exception_type) {
		switch (exception_type)
		{
		case Jpeg_InitError:
			return "JPEG codec initialization failed";
		case Jpeg_DecodingError:
			return "JPEG decoding failed";
		case Jpeg_EncodingError:
			return "JPEG encoding failed";
		case Png_InitError:
			return "PNG codec initialization failed";
		case Png_DecodingError:
			return "PNG decoding failed";
		case Png_EncodingError:
			return "PNG encoding failed";
		default:
			return "";
		}
	}
};