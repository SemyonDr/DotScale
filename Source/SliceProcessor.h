#pragma once
#include <string>
#include <stdexcept>

/// <summary>
/// States possible for a Slice Processor.
/// </summary>
enum SliceProcessorState {
	Uninitialized, // Before initialization.
	Ready_Start, // Ready to process first slice.
	Ready_Continue, // Ready to process next slice.
	Finished, // Finished processing slices and will process no more.
	Failed // Internal state is invalid and cannot be recovered.
};


/// <summary>
/// Simple base class for all classes that process images in sequence of slices.
/// </summary>
class SliceProcessor {
public:
	//--------------------------------
	//	ACCESSOR
	//--------------------------------

	SliceProcessorState GetState() { return _state; }

protected:

	//--------------------------------
	//	PRIVATE SETTER
	//--------------------------------

	inline void SetState(SliceProcessorState new_state) { _state = new_state; }
	inline void SetState_Start() { _state = SliceProcessorState::Ready_Start; }
	inline void SetState_Continue() { _state = SliceProcessorState::Ready_Continue; }
	inline void SetState_Finished() { _state = SliceProcessorState::Finished; }
	inline void SetState_Failed() { _state = SliceProcessorState::Failed; }

	void SetSliceProcessorName(std::string name) { _processor_name = name; }

	//--------------------------------
	// PRIVATE DATA
	//--------------------------------

	SliceProcessorState _state = Uninitialized;
	std::string _processor_name = "Slice Processor";

	//--------------------------------
	//	PRIVATE METHODS
	//--------------------------------

	/// <summary>
	/// Checks if processor state is suitable for processing next chunk.
	/// If state is somehow invalid throws exception.
	/// If state is "Finished" returns false.
	/// If state is valid for processing next slice returns true.
	/// </summary>
	bool CheckStateForNext() {
		switch (_state)
		{
			case SliceProcessorState::Uninitialized:
				throw new std::runtime_error(_processor_name.append(" error: Not initialized."));
				break;

			case SliceProcessorState::Failed:
				throw new std::runtime_error(_processor_name.append(" error: Failed state."));
				break;

			case SliceProcessorState::Finished:
				return false;

			default:
				return true;
		}
	}

};