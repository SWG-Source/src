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

// ======================================================================

class FastRandomGenerator
{
public:
   FastRandomGenerator()       { m_I=ZERO_ALT; }
   FastRandomGenerator(uint32 s) { setSeed(s); }

	void setSeed(uint32 s)
	{
		const uint32 ps = s&0x7fffffff;
		m_I = (ps==0) ? ZERO_ALT : ps;
	}

   // return a random number r where 0<=r<1
	float randomFloat()
	{
      float f = static_cast<float>((_rand()>>8)|IEEE_FLOAT_BITS);
      return (*(float *)(void *)&f)-1.0f; 
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

	uint32 _rand()
	{
		uint32 k;
		k=m_I/IQ;
		m_I=IA*(m_I-k*IQ)-IR*k;
		if (m_I<0) m_I += IM;
		k = m_I;
		return k;
	}

	uint32 _randMax() { return IM-1; }

	uint32 m_I;
};

// ======================================================================

#endif


