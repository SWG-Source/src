#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "engine/shared/library/sharedFoundationTypes/src/linux/FoundationTypesLinux.h"
#include "engine/shared/library/sharedRandom/src/shared/FastRandomGenerator.h"

static_assert(sizeof(void *) == 8 && sizeof(long) == 8, "Run these checks for Linux x64 (LP64)");

namespace
{
void check(bool condition, char const *message)
{
	if (!condition)
	{
		std::fprintf(stderr, "%s\n", message);
		std::exit(1);
	}
}

// A direct modular reference avoids duplicating the optimized Schrage recurrence.
class ReferenceRandom
{
public:
	explicit ReferenceRandom(std::uint32_t seed) : state(seed & 0x7fffffffU)
	{
		if (state == 0)
			state = 123459876;
	}

	int next()
	{
		state = (state * 16807) % 2147483647;
		return static_cast<int>(state);
	}

	float nextFloat()
	{
		return static_cast<float>(next() >> 8) / 8388608.0f;
	}

private:
	std::uint64_t state;
};
}

int main()
{
	std::uint32_t const seeds[] = {0, 1, 127773, 123459876, 2147483646U, 2147483647U, 0x80000000U, 0xffffffffU};
	for (std::uint32_t seed : seeds)
	{
		FastRandomGenerator integers(seed), floats(seed), bounded(seed), scaled(seed);
		ReferenceRandom integerReference(seed), floatReference(seed), boundedReference(seed), scaledReference(seed);
		for (int sample = 0; sample < 10000; ++sample)
		{
			check(integers.random() == integerReference.next(), "Integer sequence changed");
			float const value = floats.randomFloat();
			check(value >= 0.0f && value < 1.0f, "randomFloat() is outside [0,1)");
			check(value == floatReference.nextFloat(), "Float sequence changed");
			int const index = bounded.random(10);
			check(index >= 0 && index < 10, "random(10) is outside [0,10)");
			check(index == boundedReference.next() % 10, "Bounded integer sequence changed");
			check(bounded.random(-7, 13) == boundedReference.next() % 20 - 7, "Integer interval changed");
			check(scaled.randomFloat(8.0f) == scaledReference.nextFloat() * 8.0f, "Float scaling changed");
			check(scaled.randomFloat(-4.0f, 4.0f) == scaledReference.nextFloat() * 8.0f - 4.0f, "Float interval changed");
		}
	}

	FastRandomGenerator reset(1), defaultGenerator;
	reset.setSeed(127773);
	check(reset.random() == 2147480811, "Negative recurrence intermediate was not corrected");
	ReferenceRandom defaultReference(0);
	check(defaultGenerator.random() == defaultReference.next(), "Default seed changed");
	std::puts("RNG: 80,000 samples match the reference, including float and bounded outputs");
}
