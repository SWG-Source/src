// ======================================================================
//
// Random.h
// By Eric Sebesta
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#ifndef _RANDOM_H_
#define _RANDOM_H_

#include "sharedRandom/RandomGenerator.h"

// ======================================================================
// Provides an interface to a global random number generator
//
// Remarks:
//
//    It must have install() called before use.  It creates a global random number 
//    generator.  To create local random number generators, use RandomGenerator.  
//    The RandomGenerator class depends on this class to generate the seeds for it.

class Random
{
private:
	//private static data members
	static RandomGenerator* rand;
	static bool             installed;

public:
	//public static member functions
	static void  install     (uint32 newSeed);
	static void  remove      (void);

	static real  randomReal  (void);
	static real  randomReal  (real range);
	static real  randomReal  (real low, real high);
	static int32 random      (void);
	static int32 random      (int32 range);
	static int32 random      (int32 low, int32 high);

	static int32 getSeed     (void);
	static void  setSeed     (uint32 newSeed);

	static bool  isInstalled (void);

private:
	// disable: default constructor, copy constructor, assignment operator
  Random (void);
	Random (const Random&);
	Random &operator =(const Random&);
};

// ----------------------------------------------------------------------
/**
 * Static function to get a real random number between 0 and 1.
 * 
 * None.
 * 
 * @return A real random number between (and including) 0 and max(real)
 */

inline real Random::randomReal (void)
{
	DEBUG_FATAL(!installed, ("not installed"));
	return rand->randomReal();
}

// ----------------------------------------------------------------------
/**
 * Static function to get a int32 random number between 0 and max(int32).
 * 
 * None.
 * 
 * @return A int32 random number between (and including) 0 and max(int32)
 */

inline int32 Random::random (void)
{
	DEBUG_FATAL(!installed, ("not installed"));
	return rand->random();
}

// ----------------------------------------------------------------------
/**
 * Static function to get a real random number between low and high.
 * 
 * None.
 * 
 * @return A real random number between (and including) low and high.
 */

inline real Random::randomReal (real low, real high)
{
	DEBUG_FATAL(!installed, ("not installed"));
	return rand->randomReal(low, high);
}

// ----------------------------------------------------------------------
/**
 * Static function to get a int32 random number between low and high.
 * 
 * None.
 * 
 * @return A int32 random number between (and including) low and high.
 */

inline int32 Random::random (int32 low, int32 high)
{
	DEBUG_FATAL(!installed, ("not installed"));
	return rand->random(low, high);
}

// ----------------------------------------------------------------------
/**
 * Static function to get a int32 random number between 0 and range.
 * 
 * None.
 * 
 * @return A int32 random number between (and including) 0 and range.
 */

inline int32 Random::random (int32 range)
{
	DEBUG_FATAL(!installed, ("not installed"));
	DEBUG_FATAL(range < 0, ("range < 0, use random(-range, 0)"));
	return rand->random(0, range);
}

// ----------------------------------------------------------------------
/**
 * Static function to get a real random number between 0 and range.
 * 
 * None.
 * 
 * @return A real random number between (and including) 0 and range.
 */

inline real Random::randomReal (real range)
{
	DEBUG_FATAL(!installed, ("not installed"));
	DEBUG_FATAL(range < 0, ("range < 0, use randomReal(-range, 0)"));
	return rand->randomReal (CONST_REAL(0), range);
}

// ----------------------------------------------------------------------
/**
 * Static function to get the current global seed.
 * 
 * None.
 * 
 * @return The current seed.
 */

inline int32 Random::getSeed (void)
{
	DEBUG_FATAL(!installed, ("not installed"));
	return rand->getSeed();
}

// ----------------------------------------------------------------------
/**
 * Static function to set the global seed.
 * 
 * Can be used midgame to change the seed.  Note that if seeded to a previously used value, repetition may result.
 */

inline void Random::setSeed (uint32 newSeed)
{
	DEBUG_FATAL(!installed, ("not installed"));
	rand->setSeed(newSeed);
}

// ----------------------------------------------------------------------
/**
 * Static function determine if this module is installed.
 * 
 * None
 */

inline bool Random::isInstalled ()
{
	DEBUG_FATAL(!installed, ("not installed"));
	return installed;
}

// ======================================================================

#endif
