#pragma once

//STL
#include <string>
//Internal
#include "WarningCallbackData.h"

///<summary>
///This class stores pointers to error and warning callbacks used by LibPng.
///Intended to be used as an inhereted part of PngReader and PngWriter.
///</summary>
class LibPngCallbacks {
	//--------------------------------
	//	SETTERS
	//--------------------------------

	///<summary>
	///Sets callback function to be called when warning message is emmited by libpng.
	///</summary>
	///<param name="warningCallback">
	///<para>Function to be called when a warning is issued by libpng. Signature is:</para>
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
	///Callback used when libpng decoding routines encounter fatal error and are terminated.
	///Throws an exception that contains libpng error message.
	///</summary>
	static void ErrorExitHandler(png_const_structrp png_ptr, png_const_charp error_message) {
		//Throwing
		throw codec_fatal_exception(CodecExceptions::Png_DecodingError, error_message);
	}

	///<summary>
	///Callback used when libpng routines encounter non-fatal error, or corrupt data and are able to continue.
	///Extracts warning message and calls for WarningCallback which is provided by the consumer.
	///</summary>
	static void WarningHandler(png_const_structrp png_ptr, png_const_charp error_message) {
		//Extracting pointer to warning callback object from png_ptr
		WarningCallbackData* warning_callback_data_ptr = reinterpret_cast<WarningCallbackData*>(png_get_error_ptr(png_ptr));

		//Extracting pointers to warning callback and its arguments (just for ease of reading this function code)
		void (*warning_callback)(std::string, void*) = warning_callback_data_ptr->warningCallback;
		void* callback_args = warning_callback_data_ptr->warningCallbackArgs_ptr;

		//Triggering warning callback if one is registered with the reader object
		if (warning_callback != NULL)
			warning_callback(std::string(error_message), callback_args);
	}
};