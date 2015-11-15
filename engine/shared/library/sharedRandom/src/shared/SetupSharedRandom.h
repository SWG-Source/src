// ======================================================================
//
// SetupSharedRandom.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupSharedRandom_H
#define INCLUDED_SetupSharedRandom_H

// ======================================================================

class SetupSharedRandom
{
public:

	static void install(uint32 seed);

private:
	SetupSharedRandom();
	SetupSharedRandom(const SetupSharedRandom &);
	SetupSharedRandom &operator =(const SetupSharedRandom &);
};

// ======================================================================

#endif
