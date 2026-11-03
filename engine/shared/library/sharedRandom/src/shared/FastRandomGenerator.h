// ======================================================================
//
// RandomGenerator.h
// By Ben Earhart
//
// copyright 2005 Bootprint Entertainment
//
// ======================================================================

#ifndef _FAST_RANDOM_GENERATOR_H_
#define _FAST_RANDOM_GENERATOR_H_

#include <cstdint>
#include <cstring>

// ======================================================================

class FastRandomGenerator
{
public:
   FastRandomGenerator()       { m_I=ZERO_ALT; }
   FastRandomGenerator(uint32 s) { setSeed(s); }

	void setSeed(uint32 s)
	{
		const std::int32_t ps = static_cast<std::int32_t>(s&0x7fffffffU);
		m_I = (ps==0) ? ZERO_ALT : ps;
	}

   // return a random number r where 0<=r<1
	float randomFloat()
	{
      const std::uint32_t bits = (static_cast<std::uint32_t>(_rand())>>8)|IEEE_FLOAT_BITS;
      float f;
      static_assert(sizeof(f) == sizeof(bits), "FastRandomGenerator requires 32-bit floats");
      std::memcpy(&f, &bits, sizeof(f));
      return f-1.0f;
	}
	float randomFloat(float range)           { return randomFloat()*range; }
	float randomFloat(float low, float high) { return randomFloat()*(high-low) + low; }

	int random()                       { return _rand(); }
	int random(int range)              { return random()%range; }
	int random(int low, int high)		  { return (random()%(high-low)) + low; }

protected:

	enum { IEEE_FLOAT_BITS=0x3f800000 };

	enum { 
		IA = 16807, 
		IM = 2147483647, 
		IQ = 127773, 
		IR = 2836, 
		ZERO_ALT = 123459876
	};

	std::int32_t _rand()
	{
		std::int32_t k;
		k=m_I/IQ;
		m_I=IA*(m_I-k*IQ)-IR*k;
		if (m_I<0) m_I += IM;
		k = m_I;
		return k;
	}

	std::int32_t _randMax() { return IM-1; }

	std::int32_t m_I;
};

// ======================================================================

#endif


