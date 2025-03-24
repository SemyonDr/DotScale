#pragma once
//STL
#include <string>
#include <filesystem>
#include <fstream>
#include <exception>
//Internal
#include "ImageBuffer_Byte.h"
#include "ImageBufferInfo.h"

///<summary>
///Base class for image writers. Provides common interface for writing an image file.
///This class is abstract and cannot be instantiated.
///</summary>
class ImageWriter {

public:
	//--------------------------------
	//	PUBLIC GETTERS
	//--------------------------------

	///<summary>
	///First row to be written by WriteNextRows.
	///</summary>
	int GetNextRowIndex() { return _next_row_index; }

	///<summary>
	///Path to the file associated with this writer.
	///</summary>
	std::filesystem::path GetFilePath() { return _file_path; }

	///<summary>
	///Basic header data of the image to be written.
	///</summary>
	ImageBufferInfo GetCommonHeader() { return _image_info; }

	///<summary>
	///Returns how many rows are left to write for this image.
	///</summary>
	int RowsLeftToWrite() {
		if (_next_row_index < _image_info._height)
			return _image_info._height - _next_row_index;
		else
			return 0;
	}

	///<summary>
	///Reports true if this writer has finished the writing.
	///</summary>
	bool IsFinished() {
		if (_state != WriterStates::Finished)
			return false;
		else
			return true;
	}

	//--------------------------------
	//	PUBLIC METHODS
	//--------------------------------

	///<summary>
	///Writes next block of rows starting at NextRow.
	///Advances NextRow by the height of given image.
	///If number of lines in the provided image is bigger than number of rows left writes what is possible.
	///</summary>
	virtual void WriteNextRows(const ImageBuffer_Byte& image) = 0;

	//--------------------------------
	//	PUBLIC CONSTRUCTORS
	//--------------------------------	

	///<summary>
	///Sets file path for this writer object.
	///</summary>
	ImageWriter(std::filesystem::path file_path) : ImageWriter() {
		_file_path = file_path;
	}

protected:
	//--------------------------------
	//	PRIVATE METHODS
	//--------------------------------


	/// <summary>
	/// <para>Opens file pointed by file_path for binary writing in C mode.</para>
	/// <para>Can throw std::ifstream::failure if opening file failed.</para>
	/// </summary>
	/// <param name="file_path">Path to file to open.</param>
	void OpenCFileForBinaryWriting(std::filesystem::path file_path) {

		//Opening file in C mode for binary writing
		int c_file_path_size = file_path.string().length() + 1;
		char* c_file_path = new char[c_file_path_size];
		strcpy_s(c_file_path, c_file_path_size, file_path.string().c_str());

		//Trying to open the file in reading binary mode
		errno_t fopen_error = fopen_s(&_file_handle, c_file_path, "wb");
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
	/// Advances rows counter and returns how many of num_rows will actually be written.
	/// </summary>
	int AdvanceRows(int num_rows) {
		//Checking how many rows are actually available to write until the file ends
		int actual_num_rows; //Actual number of rows to write

		//If by writing num_rows we exceed remaining rows in the file we just write what is left
		if (_next_row_index + num_rows >= _image_info._height) {
			actual_num_rows = _image_info._height - _next_row_index;
			//We set the flag that indicates that writing is finished
			_state = WriterStates::Finished;
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
	///Describes possible states for the writer object.
	///States are introduced to account for possible future logic change of the class
	///where tracking the state will be required.
	///</summary>
	enum WriterStates {
		Uninitialized, //Initial state - file closed, compressor not initialized.
		Ready_Start, //File is open, compressor initialized, set and ready to write the file, but no actual lines were written yet
		Ready_Continue, //File is open, compressor initialized, set and ready to write the file. Some lines were written and compressor is ready to continue writing
		Finished, //File is closed, compressor not initialized, no lines left to write
		Failed	//File and compressor states should be consulted specifically
	};

	WriterStates _state = WriterStates::Uninitialized;
	bool _is_file_opened = false;
	bool _is_compressor_initialized = false;

	//--------------------------------
	//	DEFAULT CONSTRUCTOR
	//--------------------------------	

	ImageWriter() {
		_next_row_index = 0;
	}

	//--------------------------------
	//	PRIVATE DATA
	//--------------------------------

	/// <summary>
	/// C file handle of the file opened by this writer.
	/// </summary>
	FILE* _file_handle = NULL;

	///<summary>
	///Path to the file associated with this writer.
	///</summary>
	std::filesystem::path _file_path;

	///<summary>
	///First row to be written by ReadNextRows.
	///</summary>
	int _next_row_index = 0;

	///<summary>
	///Basic header data of the image to be written.
	///</summary>
	ImageBufferInfo _image_info;

};