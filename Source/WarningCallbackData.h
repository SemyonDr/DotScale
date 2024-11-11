#pragma once
#include <string>

///<summary>
///Data structure for storing callback pointer that handle warnings produced by image codecs.
///</summary>
class WarningCallbackData {
public:
	///<summary>
	///Callback provided by a codec consumer to handle warnings issued by the codec.
	///</summary>
	void (*warningCallback)(std::string, void*) = NULL;

	///<summary>
	///Pointer to the arguments for consumer provided warnings callback.
	///</summary>
	void* warningCallbackArgs_ptr = NULL;

	//--------------------------------
	//	CONSTRUCTORS
	//--------------------------------

	///<summary>
	///Default constructor.
	///</summary>
	WarningCallbackData() {
		warningCallback = NULL;
		warningCallbackArgs_ptr = NULL;
	}

	///<summary>
	///Initializing constructor.
	///</summary>
	WarningCallbackData(void (*callback)(std::string, void*), void* args_ptr) {
		warningCallback = callback;
		warningCallbackArgs_ptr = args_ptr;
	}

	///<summary>
	///Copy constructor.
	///</summary>
	WarningCallbackData(WarningCallbackData& copy_source) {
		warningCallback = copy_source.warningCallback;
		warningCallbackArgs_ptr = copy_source.warningCallbackArgs_ptr;
	}

	///<summary>
	///Move constructor.
	///</summary>
	WarningCallbackData(WarningCallbackData&& move_source) noexcept {
		warningCallback = move_source.warningCallback;
		warningCallbackArgs_ptr = move_source.warningCallbackArgs_ptr;
	}
};