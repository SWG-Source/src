//========================================================================
//
// ObjectSetMessage.def - sub-message ids for controller message CM_setMemberData
//
// copyright 2001 Sony Online Entertainment
//
// data files should be including ObjectSetMessage.def
// code files should be including ObjectSetMessage.h
//
//========================================================================

#ifndef _INCLUDED_ObjectSetMessage_H
#define _INCLUDED_ObjectSetMessage_H

#include "sharedObject/ObjectSetMessage.def"

// data types for ObjectSetMessage
enum OsmDataType
{
	ODT_bool,			// maps to bool
	ODT_char,			// maps to char
	ODT_int16,			// maps to int16
	ODT_int32,			// maps to int32
	ODT_uint16,			// maps to uint16
	ODT_uint32,			// maps to uint32
	ODT_real,			// maps to real
	ODT_stdstring,		// maps to an stl string
	ODT_unicodestring,	// maps to our unicode wide character strings
	ODT_attrib			// maps to struct OdtAttribute
};



// types for CreatureObject
struct OdtAttribute
{
	signed char  type;			// attribute type (CreatureObject::AttributeEnumerator)
	int16 value;		// attribute value
};


#endif	// _INCLUDED_ObjectSetMessage_H
