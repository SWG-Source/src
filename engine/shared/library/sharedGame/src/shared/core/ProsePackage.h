//======================================================================
//
// ProsePackage.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ProsePackage_H
#define INCLUDED_ProsePackage_H

#include "StringId.h"
#include "sharedGame/ProsePackageParticipant.h"

class ProsePackageParticipant;

//======================================================================

// See CuiStringVariables.h for %flags

class ProsePackage
{
public:

	typedef ProsePackageParticipant Participant;

	StringId        stringId;

	Participant     actor;
	Participant     target;
	Participant     other;

	int             digitInteger;
	float           digitFloat;

	bool            complexGrammar;

	explicit ProsePackage (bool _complexGrammar = false);

	bool            isValid () const;

	bool            operator == (const ProsePackage & rhs) const;
};

//----------------------------------------------------------------------

inline bool ProsePackage::isValid () const
{
	return !stringId.isInvalid ();
}

//----------------------------------------------------------------------

inline bool  ProsePackage::operator == (const ProsePackage & rhs) const
{
	return 
		stringId       == rhs.stringId && 
		actor          == rhs.actor && 
		target         == rhs.target && 
		other          == rhs.other && 
		digitInteger   == rhs.digitInteger && 
		digitFloat     == rhs.digitFloat && 
		complexGrammar == rhs.complexGrammar;
}

//======================================================================

#endif
