#include "Stopwatch.h"
#include <iostream>
#include <iomanip>  //Formatting methods


void Stopwatch::Start() {
	if (running == false) {
		start_time = std::chrono::high_resolution_clock::now();
		running = true;
	}
	else {
		throw std::exception("Stopwatch is already running.");
	}
}


void Stopwatch::Stop() {
	if (running == true) {
		end_time = std::chrono::high_resolution_clock::now();
		running = false;
	}
	else {
		throw std::exception("Stopwatch is not running");
	}
}

long Stopwatch::elapsed_milliseconds() {
	if (running == false) {
		std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
		return static_cast<long>(elapsed.count());
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}

long long Stopwatch::elapsed_microseconds() {
	if (running == false) {
		std::chrono::duration<double, std::micro> elapsed = end_time - start_time;
		return static_cast<long long>(elapsed.count());
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}

std::string Stopwatch::elapsed_string() {
	if (running == false) {
		std::chrono::duration<double, std::micro> elapsed = end_time - start_time;
		long long elapsed_mcs = static_cast<long long>(elapsed.count());

		//Constructing the string
		std::stringstream ss;
		if (elapsed_mcs < 1000) { //Smaller that millisecond
			ss << elapsed_mcs << " micro seconds";
			return ss.str();
		}
		if (elapsed_mcs >= 1000 && elapsed_mcs < 1000000) { //From millisecond to second
			ss << elapsed_mcs / 1000 << "." << std::setw(3) << std::setfill('0') << elapsed_mcs % 1000 << " milliseconds (" << elapsed_mcs << ")";
			return ss.str();
		}
		if (elapsed_mcs >= 1000000) { //Seconds
			ss << elapsed_mcs / 1000000 << "." << std::setw(3) << std::setfill('0') << (elapsed_mcs % 1000000)/1000 << " seconds (" << elapsed_mcs << ")";
			return ss.str();
		}

		return "[error getting string from elapsed time]";
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}

long long Stopwatch::elapsed_nanoseconds() {
	if (running == false) {
		std::chrono::duration<double, std::nano> elapsed = end_time - start_time;
		return static_cast<long long>(elapsed.count());
	}
	else {
		throw std::exception("Stopwatch isn't stopped.");
	}
}



Stopwatch::Stopwatch() {


}