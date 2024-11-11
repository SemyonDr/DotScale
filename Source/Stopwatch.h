#pragma once
#include <chrono>
#include <exception>
#include <sstream>  //Sting stream for formatting strings


class Stopwatch {

public:
	void Start();
	void Stop();
	long elapsed_milliseconds();
	long long elapsed_microseconds();
	long long elapsed_nanoseconds();
	std::string elapsed_string();

	Stopwatch();

private:
	std::chrono::high_resolution_clock::time_point start_time;
	std::chrono::high_resolution_clock::time_point end_time;
	bool running = false;
};



