// ======================================================================
//
// SpeciesRestrictions.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SpeciesRestrictions_H
#define INCLUDED_SpeciesRestrictions_H

// ======================================================================

/**
 * A simple class that manages the rules about what game feature bits
 * are required to create characters of particular species.
 */
class SpeciesRestrictions
{
  public:
	static bool canCreateCharacter(uint32 gameFeatures, uint32 object);
};

// ======================================================================

#endif
