//======================================================================
//
// ServerObjectAttributeIds.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ServerObjectAttributeIds_H
#define INCLUDED_ServerObjectAttributeIds_H

//======================================================================

#define MAKE_ATTRIB_ID(a) const std::string a = #a;

//----------------------------------------------------------------------

namespace
{
	MAKE_ATTRIB_ID (wpn_damage_radius);
}

//======================================================================

#endif
