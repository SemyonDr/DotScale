#include "../image_io/JpegIO.h"


namespace dotscale {
namespace io_jpeg {

//--------------------------------
//	ERROR/WARNING HANDLERS
//--------------------------------

/// Called when libjpeg routines encounter non-fatal error, or corrupt data and are able to continue.
/// Redirects to user-supplied callback.
void libjpeg_warning_handler(j_common_ptr jpeg_object, int msg_level) {
	// No-warning setting
	if (msg_level != -1)
		return;

	// Extracting callback data from jpeg object
	auto* warning_callback = static_cast<io_image::CodecWarningCallback*>(jpeg_object->client_data);

	// Checking if the callback was set
	if (warning_callback == NULL || warning_callback->func == nullptr)
		return;

	// Formatting libjpeg warning message
	char msg[JMSG_LENGTH_MAX];
	jpeg_object->err->format_message(jpeg_object, msg);

	// Passing to user-defined function
	warning_callback->func(std::string(msg), warning_callback->arg);
}


///Callback used when libjpeg decoding routines encounter fatal error and are terminated.
///Throws an exception containing libjpeg error message.
void libjpeg_error_exit_handler(j_common_ptr jpeg_object) {
	// Formatting libjpeg warning message
	char msg[JMSG_LENGTH_MAX];
	jpeg_object->err->format_message(jpeg_object, msg);
	//Throwing
	throw codec_fatal_exception(CodecExceptionType::Jpeg_DecodingError, msg);
}



} // namespace io_jpeg
} // namespace dotscale 