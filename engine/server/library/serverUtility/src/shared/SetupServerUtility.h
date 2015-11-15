// ======================================================================
//
// SetupServerUtility.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupServerUtility_H
#define INCLUDED_SetupServerUtility_H

// ======================================================================

class SetupServerUtility
{
public:

	static void install();

private:
	SetupServerUtility();
	SetupServerUtility(const SetupServerUtility &);
	SetupServerUtility &operator =(const SetupServerUtility &);
};

// ======================================================================

#endif
