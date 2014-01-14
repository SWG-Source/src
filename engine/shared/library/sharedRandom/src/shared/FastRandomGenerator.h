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
   FastRandomGenerator(long s) { setSeed(s); }

	void setSeed(long s) 
	{
		const long ps = s&0x7fffffff;
		m_I = (ps==0) ? ZERO_ALT : ps;
	}

   // return a random number r where 0<=r<1
	float randomFloat()
	{
      long f = (_rand()>>8)|IEEE_FLOAT_BITS; 
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

	long _rand()
	{
		long k;
		k=m_I/IQ;
		m_I=IA*(m_I-k*IQ)-IR*k;
		if (m_I<0) m_I += IM;
		k = m_I;
		return k;
	}

	long _randMax() { return IM-1; }

	long m_I;
};

// ======================================================================

#endif


