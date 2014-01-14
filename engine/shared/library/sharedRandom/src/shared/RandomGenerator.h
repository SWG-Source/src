// ======================================================================
//
// RandomGenerator.h
// By Eric Sebesta
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#ifndef _RANDOM_GENERATOR_H_
#define _RANDOM_GENERATOR_H_

// ======================================================================
// A class to generate random numbers
//
// Remarks:
//
//   This class generates 31-bit random numbers returned as either 32-bit
//   integers or as reals.   Note that Random::install() MUST be called prior
//   to creation of RandomGenerator classes.  Random represents a 
//   global random number generator, and is used to generate seeds for all
//   RandomGenerator classes.  Failing to call Random::install() will Fatal in 
//   debug builds  and have undefined behavior in release builds.  This class 
//   uses a random number generation algorithm found in Numerical Recipes in C,
//   on p. 280.

class RandomGenerator
{

private:
	//private member functions
	int32 randomNumber (void);

private:

	enum
	{
		IM     = 2147483647,
		NTAB   = 322,
		IA     = 16807,
		IQ     = 127773,
		IR     = 2836
	};

	static const real AM;
	static const real NDIV;
	static const real EPS;
	static const real RNMX;

private:

	//private member variables
	int32 idnum;
	int32 iy;
	int32 iv[NTAB];

public:
	//constructors and destructors
	          RandomGenerator  (void);
	explicit  RandomGenerator  (uint32 newSeed);
	         ~RandomGenerator  (void);

	//public member functions
	real  randomReal (void);
	real  randomReal (real range);
	real  randomReal (real low, real high);
	int32 random     (void);
	int32 random     (int32 range);
	int32 random     (int32 low, int32 high);

	int32 getSeed    (void);
	void  setSeed    (uint32 newSeed);
};

// ======================================================================
// Construct a RandomGenerator
//
// Remarks:
//
//   Uses the user-supplied seed.

inline RandomGenerator::RandomGenerator (uint32 newSeed)
: idnum (-static_cast<int32>(newSeed)),
	iy    (0)
{
	memset(iv, 0, sizeof(iv));
}

// ======================================================================
// Destruct a RandomGenerator
//
// Remarks:
//
//   None.

inline RandomGenerator::~RandomGenerator (void)
{
}

// ----------------------------------------------------------------------
/**
 * Get a real random number between 0 and max(real).
 * 
 * None.
 * 
 * @return A real random number between (and including) 0 and 1
 */

inline real RandomGenerator::randomReal (void)
{
	return static_cast<real>( randomNumber () ) / static_cast<real>( IM );
}

// ----------------------------------------------------------------------
/**
 * Get a int32 random number between 0 and max(int32).
 * 
 * None.
 * 
 * @return A int32 random number between (and including) 0 and max(int32)
 */

inline int32 RandomGenerator::random (void)
{
	return randomNumber ();
}

// ----------------------------------------------------------------------
/**
 * Get a real random number between low and high.
 * 
 * None.
 * 
 * @return A real random number between (and including) low and high.
 */

inline real RandomGenerator::randomReal (real low, real high)
{
	DEBUG_FATAL(low > high, ("low > high"));
	return static_cast<real>(randomReal() * (high - low) + low);
}

// ----------------------------------------------------------------------
/**
 * Get a int32 random number between low and high.
 * 
 * None.
 * 
 * @return A int32 random number between (and including) low and high.
 */

inline int32 RandomGenerator::random (int32 low, int32 high)
{
	DEBUG_FATAL(low > high, ("low > high"));
	return ( random () % ( high - low + 1 ) + low );
}

// ----------------------------------------------------------------------
/**
 * Get a int32 random number between 0 and range.
 * 
 * None.
 * 
 * @return A int32 random number between (and including) 0 and range.
 */

inline int32 RandomGenerator::random (int32 range)
{
	DEBUG_FATAL(range < 0, ("range < 0, use random(-range, 0)"));
	return random ( 0, range );
}

// ----------------------------------------------------------------------
/**
 * Get a real random number between 0 and range.
 * 
 * None.
 * 
 * @return A real random number between (and including) 0 and range.
 */

inline real RandomGenerator::randomReal (real range)
{
	DEBUG_FATAL(range < 0, ("range < 0, use randomReal(-range, 0)"));
	return randomReal ( CONST_REAL(0), range );
}

// ----------------------------------------------------------------------
/**
 * Get the current seed.
 * 
 * None.
 * 
 * @return The current seed.
 */

inline int32 RandomGenerator::getSeed (void)
{
	return iy;
}

// ----------------------------------------------------------------------
/**
 * Set the seed.
 * 
 * Can be used midgame to change the seed.  Note that if seeded to a previously used value, repetition may result.
 */

inline void RandomGenerator::setSeed (uint32 newSeed)
{
	iy = 0;
	idnum = -(static_cast<int32>(newSeed)); //since a negative value will cause the ran1() as used in randomNumber to "reseed"
}

// ======================================================================

#endif
