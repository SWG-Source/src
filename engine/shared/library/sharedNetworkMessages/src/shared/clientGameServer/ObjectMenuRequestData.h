

// ======================================================================
//
// ObjectMenuRequestData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectMenuRequestData_H
#define INCLUDED_ObjectMenuRequestData_H

//-----------------------------------------------------------------------

#include "Unicode.h"
#include "StringId.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"

//----------------------------------------------------------------------

struct ObjectMenuRequestData
{
	static const uint8 F_enabled;
	static const uint8 F_serverNotify;
	static const uint8 F_outOfRange;

	//----------------------------------------------------------------------

	uint8           m_id;
	uint8           m_parent;
	uint16          m_menuItemType; // menu item action id (i.e. ITEM_ROTATE_RIGHT)
	uint8           m_flags;
	Unicode::String m_label;
	StringId        m_labelId;

	//----------------------------------------------------------------------

	ObjectMenuRequestData (uint8 id, uint8 parent, uint16 menuItemType, const Unicode::String & label, bool enabled, bool serverNotify);
	ObjectMenuRequestData ();

	bool hasFlag        (uint8 f) const;
	bool isEnabled      () const;
	bool isServerNotify () const;
	bool isOutOfRange   () const;
	void setOutOfRange  (bool b);
	void setServerNotify(bool b);

	bool operator==(const ObjectMenuRequestData & rhs) const;
};

//----------------------------------------------------------------------

inline ObjectMenuRequestData::ObjectMenuRequestData (uint8 id, uint8 parent, uint16 menuItemType, const Unicode::String & label, bool enabled, bool serverNotify) :
m_id (id),
m_parent (parent),
m_menuItemType (menuItemType),
m_flags (0),
m_label (label),
m_labelId ()
{
	if (enabled)
		m_flags |= F_enabled;
	if (serverNotify)
		m_flags |= F_serverNotify;
}

//----------------------------------------------------------------------

inline ObjectMenuRequestData::ObjectMenuRequestData () :
m_id (0),
m_parent (0),
m_menuItemType (0),
m_flags (F_enabled),
m_label (),
m_labelId ()
{
}

//----------------------------------------------------------------------

inline bool ObjectMenuRequestData::hasFlag (uint8 f) const
{
	return (m_flags & f) != 0;
}

//----------------------------------------------------------------------

inline bool ObjectMenuRequestData::isEnabled () const
{
	return hasFlag (F_enabled);
}

//----------------------------------------------------------------------

inline bool ObjectMenuRequestData::isServerNotify () const
{
	return hasFlag (F_serverNotify);
}

//----------------------------------------------------------------------

inline bool ObjectMenuRequestData::isOutOfRange   () const
{
	return hasFlag (F_outOfRange);
}

//----------------------------------------------------------------------

/**
* Equality deliberately ignores flags & strings since they are usually transient.
*/

inline bool ObjectMenuRequestData::operator==(const ObjectMenuRequestData & rhs) const
{
	return (m_id == rhs.m_id) && (m_parent == rhs.m_parent) && (m_menuItemType == rhs.m_menuItemType);
}

//----------------------------------------------------------------------

#endif
