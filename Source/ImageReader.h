#pragma once
//STL
#include <string>
#include <filesystem>
//Internal
#include "ImageBuffer_Byte.h"
#include "ImageBufferInfo.h"

///<summary>
///Base class for image readers. Provides common interface for reading an image file.
///This class is abstract and cannot be instantiated.
///</summary>
class ImageReader {



public:
	//--------------------------------
	//	PUBLIC GETTERS
	//--------------------------------

	///<summary>
	///First row to be read by ReadNextRows.
	///</summary>
	int GetNextRowIndex() { return _next_row_index; }

	///<summary>
	///Path to the file associated with this reader.
	///</summary>
	std::filesystem::path GetFilePath() { return _file_path; }

	///<summary>
	///Basic header data extracted from the image.
	///</summary>
	ImageBufferInfo GetCommonHeader() { return _image_info; }

	///<summary>
	///Returns how many rows are left to read in the image.
	///</summary>
	int RowsLeftToRead() {
		if (_next_row_index < _image_info._height)
			return _image_info._height - _next_row_index;
		else
			return 0;
	}

	///<summary>
	///Reports true if this reader has finished the reading.
	///</summary>
	bool IsFinished() {
		if (_state != ReaderStates::Finished)
			return false;
		else
			return true;
	}


	//--------------------------------
	//	PUBLIC METHODS
	//--------------------------------

	///<summary>
	///Reads next block of rows starting at NextRow.
	///Returns resulting bitmap.
	///Advances NextRow by num_lines.
	///If num_lines is bigger than number of rows left reads all available rows.
	///When all rows are already read returns NULL.
	///</summary>
	virtual ImageBuffer_Byte* ReadNextRows(int num_lines) = 0;


	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------	

	///<summary>
	///Sets file path for this reader object.
	///</summary>
	ImageReader(std::filesystem::path file_path) : ImageReader() {
		_file_path = file_path;
	}

protected:

	//--------------------------------
	//	PRIVATE METHODS
	//--------------------------------

	/// <summary>
	/// <para>Opens file pointed by file_path for binary reading in C mode.</para>
	/// <para>Can throw std::ifstream::failure if opening file failed.</para>
	/// </summary>
	/// <param name="file_path">Path to file to open.</param>
	void OpenCFileForBinaryReading(std::filesystem::path file_path) {
	
		//Extracting file path as UTF-8 c-style string to use with c file opener.
		const char* c_file_path = reinterpret_cast<const char*>(file_path.generic_u8string().c_str());
		//Trying to open the file in reading binary mode
		errno_t fopen_error = fopen_s(&_file_handle, c_file_path, "rb");

		if (fopen_error != 0) {
			//Constructing error message string
			char err_msg_buffer[256];
			errno_t strerror_error = strerror_s(err_msg_buffer, 256, fopen_error);
			//Throwing exception	
			if (strerror_error == 0) //Message string constructed
				throw std::ifstream::failure(std::string(err_msg_buffer));
			else
				throw std::ifstream::failure("");
		}

		//Setting state flag
		_is_file_opened = true;
	}

	/// <summary>
	/// Advances rows counter and returns how many of num_rows will actually be read.
	/// </summary>
	int AdvanceRows(int num_rows) {
		//Checking how many rows are actually available to read until the file ends
		int actual_num_rows; //Actual number of rows to read

		//If by reading num_rows we exceed remaining rows in the file we just read what is left
		if (_next_row_index + num_rows >= _image_info._height) {
			actual_num_rows = _image_info._height - _next_row_index;
			//We set the flag that indicates that reading is finished
			_state = ReaderStates::Finished;
		}
		else
			actual_num_rows = num_rows;

		//Advancing row indexer
		_next_row_index += actual_num_rows;

		return actual_num_rows;
	}

	//--------------------------------
	//	INTERNAL STATES
	//--------------------------------

	///<summary>
	///Describes possible states for the reader object.
	///States are introduced to account for possible future logic change of the class
	///where tracking the state will be required.
	///</summary>
	enum ReaderStates {
		Uninitialized, //Initial state - file closed, decompressor not initialized, header data is not available
		Ready_Start, //File is open, decompressor initialized, set and ready to read the file, but no actual lines were read
		Ready_Continue, //File is open, decompressor initialized, set and ready to read the file. Some lines were read and decompressor ready to continue reading
		Finished, //File is closed, decompressor not initialized, header data is accessible, no lines left to read
		Failed	//File and decompressor states should be consulted specifically
	};

	ReaderStates _state = ReaderStates::Uninitialized;
	bool _is_file_opened = false;
	bool _is_decompressor_initialized = false;

	//--------------------------------
	//	DEFAULT CONSTRUCTOR
	//--------------------------------	

	ImageReader() {
		_next_row_index = 0;
	}

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	///<summary>
	///Pointer to the c file handle of the file opened by this reader.
	///</summary>
	FILE* _file_handle = NULL;

	///<summary>
	///Path to the file associated with this reader.
	///</summary>
	std::filesystem::path _file_path;

	///<summary>
	///First row to be read by ReadNextRows.
	///</summary>
	int _next_row_index = 0;

	///<summary>
	///Header data extracted from the image.
	///</summary>
	ImageBufferInfo _image_info;

};