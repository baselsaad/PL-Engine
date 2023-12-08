#include "pch.h"
#include "Random.h"
#include <random>

namespace PL_Engine
{
	std::default_random_engine Random::s_Generator(std::random_device{}());

	int Random::Range(int min, int max)
	{
		std::uniform_int_distribution<int> distribution(min, max);
		return distribution(s_Generator);
	}

	float Random::Range(float min, float max)
	{
		std::uniform_real_distribution<float> distribution(min, max);
		return distribution(s_Generator);
	}

	double Random::Range(double min, double max)
	{
		std::uniform_real_distribution<double> distribution(min, max);
		return distribution(s_Generator);
	}
}
