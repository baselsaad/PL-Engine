#pragma once

namespace PL_Engine
{
	class Random {
	public:
		// Generate a random integer within the specified range
		static int Range(int min, int max);

		// Generate a random float within the specified range
		static float Range(float min, float max);

		// Generate a random double within the specified range
		static double Range(double min, double max);

	private:
		static std::default_random_engine s_Generator;
	};

}