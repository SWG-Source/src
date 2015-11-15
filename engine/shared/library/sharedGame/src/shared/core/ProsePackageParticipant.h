//======================================================================
//
// ProsePackageParticipant.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ProsePackageParticipant_H
#define INCLUDED_ProsePackageParticipant_H

#include "sharedFoundation/NetworkId.h"

//======================================================================

class ProsePackageParticipant
{
public:
	NetworkId          id;
	StringId           stringId;
	Unicode::String    str;

	bool               isValid () const;

	ProsePackageParticipant ();

	bool               operator== (const ProsePackageParticipant & rhs) const;
};

//----------------------------------------------------------------------

inline bool ProsePackageParticipant::isValid () const
{
	return id.isValid () || !stringId.isInvalid () || !str.empty ();
}

//----------------------------------------------------------------------

inline bool ProsePackageParticipant::operator== (const ProsePackageParticipant & rhs) const
{
	return id == rhs.id && stringId == rhs.stringId && str == rhs.str;
}


//======================================================================

#endif
