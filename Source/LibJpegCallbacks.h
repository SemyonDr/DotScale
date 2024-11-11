#pragma once
//STL
#include <string>
//Third party
#include "jpeglib.h"
#include "jerror.h"
//Internal
#include "Exceptions.h"
#include "WarningCallbackData.h"

///<summary>
///This class stores pointers to error and warning callbacks used by LibJpeg.
///Intended to be used as an inhereted part of JpegReader and JpegWriter.
///</summary>
class LibJpegCallbacks {
	//--------------------------------
	//	SETTERS
	//--------------------------------

	///<summary>
	///Sets callback function to be called when warning message is emmited by libjpeg.
	///</summary>
	///<param name="warningCallback">
	///<para>Function to be called when a warning is issued by libjpeg. Signature is:</para>
	///<para>void warningCallback(std::string warning_message, void* callback_args)</para>
	///</param>
	///<param name="args">Pointer to the arguments to be used by warnings callback. Can be NULL.</param>
	void SetWarningsCallback(void (*warningCallback)(std::string, void*), void* argsPtr) {
		_warning_callback_data.warningCallback = warningCallback;
		_warning_callback_data.warningCallbackArgs_ptr = argsPtr;
	}

protected:
	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	///<summary>
	///Callback and (optionally) its arguments provided by a consumer of JpegReader to handle warnings issued by libjpeg.
	///</summary>
	WarningCallbackData _warning_callback_data;


	//--------------------------------
	//	CALLBACK METHODS
	//--------------------------------

	///<summary>
	///Callback used when libjpeg decoding routines encounter fatal error and are terminated.
	///Throws an exception containing libjpeg error message.
	///</summary>
	static void ErrorExitHandler(j_common_ptr jpeg_object) {
		//Using last error message number to extract message text from messages table.
		const char* error_message = jpeg_object->err->jpeg_message_table[jpeg_object->err->last_jpeg_message];
		//Throwing
		throw codec_fatal_exception(CodecExceptions::Jpeg_DecodingError, error_message);
	}


	///<summary>
	///<para>Callback used when libjpeg routines encounter non-fatal error, or corrupt data and are able to continue.</para>
	///<para>Extracts warning message and calls for WarningCallback which is provided by the consumer.</para>
	///<para>Expects pointer to WarningCallbackData to be stored in client_data field of jpeg_object.</para>
	///</summary>
	static void WarningHandler(j_common_ptr jpeg_object, int msg_level) {
		//Extracting associated warning callback data from reference in jpeg_object
		void (*warningCallback)(std::string, void*) = reinterpret_cast<WarningCallbackData*>(jpeg_object->client_data)->warningCallback;
		void* warningCallbackArgs_Ptr = reinterpret_cast<WarningCallbackData*>(jpeg_object->client_data)->warningCallbackArgs_ptr;

		//Triggering warning callback if one is registered with the reader object
		if (warningCallback != NULL && msg_level == -1) {
			//Extracting the message produced by libjpeg
			std::string message = std::string(jpeg_object->err->jpeg_message_table[jpeg_object->err->last_jpeg_message]);
			//Calling the user provided handler
			warningCallback(message, warningCallbackArgs_Ptr);
		}
	}

};