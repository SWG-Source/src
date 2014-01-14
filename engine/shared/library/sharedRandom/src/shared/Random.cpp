// ======================================================================
//
// Random.cpp
// By Eric Sebesta
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#include "sharedRandom/FirstSharedRandom.h"
#include "sharedRandom/Random.h"

#include "sharedRandom/RandomGenerator.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

RandomGenerator* Random::rand;
bool             Random::installed;

// ======================================================================
// Static function install this module
//
// Remarks:
//
//   None.

void Random::install(uint32 newSeed)
{
	DEBUG_FATAL(installed, ("already installed"));
	installed = true;
	ExitChain::add(Random::remove, "Random::remove", 0, false);
	rand = new RandomGenerator(newSeed);
}

// ----------------------------------------------------------------------
/**
 * Static function to remove this module.
 * 
 * None.
 */

void Random::remove(void)
{
	DEBUG_FATAL(!installed, ("not installed"));
	installed = false;
	delete rand;
}

// ======================================================================
