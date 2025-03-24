#include "Stopwatch.h"
#include <iostream>
#include <iomanip>  //Formatting methods


//--------------------------------
//	MANIPULATION
//--------------------------------

/// <summary>
/// Starts the clock if it is not already running.
/// </summary>
void Stopwatch::Start() {
	if (_is_running == false) {
		_is_running = true;
		_elapsed = std::chrono::high_resolution_clock::duration::zero();
		_segment = std::chrono::high_resolution_clock::duration::zero();\
		_start_time = std::chrono::high_resolution_clock::now();
	}
}

/// <summary>
/// Stops the clock. Elapsed time can be extracted at this point.
/// </summary>
void Stopwatch::Stop() {
	if (_is_running) {
		if (_is_paused == false) {
			_end_time = std::chrono::high_resolution_clock::now();
			_elapsed += _end_time - _start_time;
			_segment = _end_time - _start_time;
		}

		_is_running = false;
		_is_paused = false;
	}
}

/// <summary>
/// Pauses current run. Time elapsed so far can be extracted. Continue() will continue the same run.
/// </summary>
void Stopwatch::Pause() {
	if (_is_running) {
		_is_paused = true;
		_end_time = std::chrono::high_resolution_clock::now();
		_elapsed += _end_time - _start_time;
		_segment = _end_time - _start_time;
	}
}

/// <summary>
/// Continues the run if the watch is running, but paused.
/// </summary>
void Stopwatch::Continue() {
	if (_is_running && _is_paused) {
		_is_paused = false;
		_start_time = std::chrono::high_resolution_clock::now();
	}
}


//--------------------------------
//	ACCESS
//--------------------------------

long long Stopwatch::elapsed_milliseconds() {
	if (_is_running == false || _is_paused) {
		std::chrono::duration<long long, std::milli> elapsed_milli = std::chrono::duration_cast<std::chrono::milliseconds>(_elapsed);
		return elapsed_milli.count();
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}

long long Stopwatch::elapsed_microseconds() {
	if (_is_running == false || _is_paused) {
		std::chrono::duration<long long, std::micro> elapsed_micro = std::chrono::duration_cast<std::chrono::microseconds>(_elapsed);
		return elapsed_micro.count();
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}

long long Stopwatch::elapsed_nanoseconds() {
	if (_is_running == false || _is_paused) {
		std::chrono::duration<long long, std::nano> elapsed_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(_elapsed);
		return elapsed_nano.count();
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}


std::string Stopwatch::elapsed_string() {
	if (_is_running == false || _is_paused) {
		std::chrono::duration<long long, std::nano> elapsed_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(_elapsed);
		long long elapsed_nano_count = elapsed_nano.count();

		return NanoDurationToString(elapsed_nano_count);
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}


//--------------------------------
//	ACCESS FOR SEGMENT
//--------------------------------


long long Stopwatch::segment_milliseconds() {
	if (_is_running == false || _is_paused) {
		std::chrono::duration<long long, std::milli> elapsed_milli = std::chrono::duration_cast<std::chrono::milliseconds>(_segment);
		return elapsed_milli.count();
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}

long long Stopwatch::segment_microseconds() {
	if (_is_running == false || _is_paused) {
		std::chrono::duration<long long, std::micro> elapsed_micro = std::chrono::duration_cast<std::chrono::microseconds>(_segment);
		return elapsed_micro.count();
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}

long long Stopwatch::segment_nanoseconds() {
	if (_is_running == false || _is_paused) {
		std::chrono::duration<long long, std::nano> elapsed_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(_segment);
		return elapsed_nano.count();
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}


std::string Stopwatch::segment_string() {
	if (_is_running == false || _is_paused) {
		std::chrono::duration<long long, std::nano> segment_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(_segment);
		long long segment_nano_count = segment_nano.count();

		return NanoDurationToString(segment_nano_count);
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}


std::string Stopwatch::NanoDurationToString(long long duration_in_nanosec) {
	//Constructing the string
	std::stringstream ss;

	//Seconds or more
	if (duration_in_nanosec >= 1'000'000'000) {
		ss << duration_in_nanosec / 1'000'000'000 << "." << std::setw(3) << std::setfill('0') << (duration_in_nanosec % 1'000'000'000) / 1'000'000 << " seconds (" << duration_in_nanosec << ")";
		return ss.str();
	}

	//Less than a second
	// One millisecond or more
	if (duration_in_nanosec >= 1'000'000) {
		ss << duration_in_nanosec / 1'000'000 << "." << std::setw(3) << std::setfill('0') << (duration_in_nanosec % 1'000'000) / 1'000 << " milliseconds (" << duration_in_nanosec << ")";
		return ss.str();
	}

	//Less than a millisecond
	// One microsecond or more
	if (duration_in_nanosec >= 1'000) {
		ss << duration_in_nanosec / 1'000 << "." << std::setw(3) << std::setfill('0') << duration_in_nanosec % 1'000 << " microseconds (" << duration_in_nanosec << ")";
		return ss.str();
	}

	// Nanoseconds
	ss << duration_in_nanosec << " nanoseconds (" << duration_in_nanosec << ")";

	return ss.str();

}



Stopwatch::Stopwatch() {


}