#pragma once
#include <chrono>
#include <exception>
#include <sstream>  //Sting stream for formatting strings


class Stopwatch {

public:
	/// <summary>
	/// Starts the clock if it is not already running.
	/// </summary>
	void Start();

	/// <summary>
	/// Stops the clock. Elapsed time can be extracted at this point.
	/// </summary>
	void Stop();

	/// <summary>
	/// Pauses current run. Time elapsed so far can be extracted. Continue() will continue the same run.
	/// </summary>
	void Pause();

	/// <summary>
	/// Continues the run if the watch is running, but paused.
	/// </summary>
	void Continue();

	long long elapsed_milliseconds();
	long long elapsed_microseconds();
	long long elapsed_nanoseconds();
	std::string elapsed_string();

	long long segment_milliseconds();
	long long segment_microseconds();
	long long segment_nanoseconds();
	std::string segment_string();

	static std::string NanoDurationToString(long long duration_in_nano);

	Stopwatch();

private:
	std::chrono::high_resolution_clock::time_point _start_time;
	std::chrono::high_resolution_clock::time_point _end_time;
	std::chrono::high_resolution_clock::duration _elapsed = std::chrono::high_resolution_clock::duration::zero();
	std::chrono::high_resolution_clock::duration _segment = std::chrono::high_resolution_clock::duration::zero();
	bool _is_running = false;
	bool _is_paused = false;

	
};



