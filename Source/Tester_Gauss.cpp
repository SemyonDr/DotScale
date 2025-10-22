#include "Tester_Gauss.h"

void Tester_Gauss::TestValue32(uint32_t num_points, boolean print) {
	Stopwatch watch;

	// Generating 60 bit presicision x points
	std::random_device rd; // This will provide the random seed
	std::mt19937_64 gen(rd()); // Random number generator for 64 bit numbers takes the seed
	std::uniform_int_distribution<uint64_t> dist(0, 7798021677424194372); // Uniform distribution description

	// Generating numbers
	std::cout << "Generating " << num_points << " points for x value." << std::endl;
	watch.Start();
	ufxd64_60_t* x_point = new ufxd64_60_t[num_points];
	for (int i = 0; i < num_points; i++)
		x_point[i] = dist(gen);
	watch.Stop();
	std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;

	// Calculating function values
	ufxd32_32_t* g_values = new ufxd32_32_t[num_points];
	std::cout << "Calculating " << num_points << " values of gauss function." << std::endl;
	watch.Start();
	for (int i = 0; i < num_points; i++)
		g_values[i] = GaussCurve::Value_x60_v32(x_point[i]);
	watch.Stop();
	std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;

	// Printing values
	if (print) {
		std::cout << "Calculated values are" << std::endl;
		for (int i = 0; i < num_points; i++)
			std::cout << "\t" << x_point[i] << "\t\t" << g_values[i] << std::endl;
	}
}





void Tester_Gauss::TestArea63(uint32_t num_intervals, boolean print) {
	Stopwatch watch;

	// Generating 60 bit presicision intervals
	std::random_device rd; // This will provide the random seed
	std::mt19937_64 gen(rd()); // Random number generator for 64 bit numbers takes the seed
	std::uniform_int_distribution<uint64_t> dist(0, 7475975381435023360); // Uniform distribution description

	// Generating numbers
	struct Interval {
		ufxd64_60_t left;
		ufxd64_60_t right;
	};
	std::cout << "Generating " << num_intervals << " intervals." << std::endl;
	watch.Start();
	Interval* intervals = new Interval[num_intervals];
	ufxd64_60_t first = 0;
	ufxd64_60_t second = 0;
	for (int i = 0; i < num_intervals; i++) {
		first = dist(gen);
		second = dist(gen);
		if (first < second) {
			intervals[i].left = first;
			intervals[i].right = second;
		}
		else {
			intervals[i].left = second;
			intervals[i].right = first;
		}
	}
	watch.Stop();
	std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;

	// Calculating areas
	ufxd64_63_t* g_areas = new ufxd64_63_t[num_intervals];
	std::cout << "Calculating " << num_intervals << " areas of gauss function." << std::endl;
	watch.Start();
	for (int i = 0; i < num_intervals; i++)
		g_areas[i] = GaussCurve::Area_x60_v63(intervals[i].left, intervals[i].right);
	watch.Stop();
	std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;

	// Printing values
	if (print) {
		std::cout << "Calculated areas are" << std::endl;
		for (int i = 0; i < num_intervals; i++)
			std::cout << "\t[ " << intervals[i].left << " .. " << intervals[i].right << " ]\t-->\t" << g_areas[i] << std::endl;

		std::cout << "Intervals for copy-paste:" << std::endl;
		for (int i = 0; i < num_intervals - 1; i++)
			std::cout << "(" << intervals[i].left << ", " << intervals[i].right << ")," << std::endl;
		std::cout << "(" << intervals[num_intervals - 1].left << ", " << intervals[num_intervals - 1].right << ")" << std::endl;
	}
}





void Tester_Gauss::TestSqrt(int num_values) {
	Stopwatch watch;

	// Generating x values
	std::random_device rd; // This will provide the random seed
	std::mt19937_64 gen(rd()); // Random number generator for 64 bit numbers takes the seed
	std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFFFFFF); // Uniform distribution description

	// Generating values
	std::cout << "Generating " << num_values << " x value s." << std::endl;
	watch.Start();
	uint64_t* x = new uint64_t[num_values];
	for (int i = 0; i < num_values; i++)
		x[i] = dist(gen);
	watch.Stop();
	std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;

	// Taking roots
	std::cout << "Taking " << num_values << " roots." << std::endl;
	uint64_t* root = new uint64_t[num_values];
	watch.Start();
	for (int i = 0; i < num_values; i++)
		root[i] = FxdMath::sqrt_64_rst(x[i]);
	watch.Stop();
	std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;

	// Printing the report
	std::cout << "Printing roots:" << std::endl;
	for (int i = 0; i < num_values; i++)
		std::cout << x[i] << "\t-->\t" << root[i] << std::endl;

	Printer::EmptyLine();
	std::cout << "Specific root test:" << std::endl;
	uint64_t x_single = 175986756;
	std::cout << x_single << "\t-->\t" << FxdMath::sqrt_64_rst(x_single) << std::endl;

}





void Tester_Gauss::TestAverages(uint32_t num_intervals, boolean print) {
	Stopwatch watch;

	// Generating 60 bit presicision intervals
	std::random_device rd; // This will provide the random seed
	std::mt19937_64 gen(rd()); // Random number generator for 64 bit numbers takes the seed
	std::uniform_int_distribution<uint64_t> dist(0, 7475975381435023360); // Uniform distribution description

	// Generating numbers
	struct Interval {
		ufxd64_60_t left;
		ufxd64_60_t right;
	};
	std::cout << "Generating " << num_intervals << " intervals." << std::endl;
	watch.Start();
	Interval* intervals = new Interval[num_intervals];
	ufxd64_60_t first = 0;
	ufxd64_60_t second = 0;
	for (int i = 0; i < num_intervals; i++) {
		first = dist(gen);
		second = dist(gen);
		if (first < second) {
			intervals[i].left = first;
			intervals[i].right = second;
		}
		else {
			intervals[i].left = second;
			intervals[i].right = first;
		}
	}
	watch.Stop();
	std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;

	// Calculating average values and areas
	ufxd32_32_t* g_avg = new ufxd32_32_t[num_intervals];
	ufxd64_63_t* g_area = new ufxd64_63_t[num_intervals];
	std::cout << "Calculating " << num_intervals << " averages of gauss function." << std::endl;
	watch.Start();
	for (int i = 0; i < num_intervals; i++) {
		g_avg[i] = GaussCurve::Average_x60_v32(intervals[i].left, intervals[i].right);
		g_area[i] = GaussCurve::Area_x60_v63(intervals[i].left, intervals[i].right);
	}
	watch.Stop();
	std::cout << "\tDone! Elapsed time: " << watch.elapsed_string() << std::endl;

	// Printing values
	if (print) {
		std::cout << "Calculated averages are" << std::endl;
		for (int i = 0; i < num_intervals; i++) {
			std::cout << "\t[ " << intervals[i].left << " .. " << intervals[i].right << " ]" << std::endl;
			std::cout << "\t\tWidth:\t" << (intervals[i].right - intervals[i].left) << std::endl;
			std::cout << "\t\tArea:\t" << g_area[i] << std::endl;
			std::cout << "\t\tAverage:\t" << g_avg[i] << std::endl;
			Printer::EmptyLine();
		}

		std::cout << "Intervals for copy-paste:" << std::endl;
		for (int i = 0; i < num_intervals - 1; i++)
			std::cout << "(" << intervals[i].left << ", " << intervals[i].right << ")," << std::endl;
		std::cout << "(" << intervals[num_intervals - 1].left << ", " << intervals[num_intervals - 1].right << ")" << std::endl;
	}
}