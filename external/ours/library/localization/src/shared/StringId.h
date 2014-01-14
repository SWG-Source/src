//========================================================================
//
// StringId.h - Used to access an entry in a string table.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_StringId_H
#define _INCLUDED_StringId_H

#include <string>
#include "Unicode.h"

// ======================================================================

namespace DB
{
	class BufferString;
}

//========================================================================
// class StringId

class StringId
{
	friend int          operator -  (const StringId & lhs, const StringId & rhs);
	friend bool         operator != (const StringId & lhs, const StringId & rhs);
	friend bool         operator == (const StringId & lhs, const StringId & rhs);
	friend bool         operator <  (const StringId & lhs, const StringId & rhs);

public:

	static const StringId   cms_invalid;

	//-- LocalizationUnicodeString is simply a typedef that is equivalent to Unicode::String
	//-- it is defined here to prevent #including Unicode.h
	typedef std::basic_string<unsigned short> LocUnicodeString;

	                        StringId       ();
	                        StringId       (const std::string & table, const std::string & text);
	                        StringId       (const std::string & table, unsigned long textIndex);
	explicit                StringId       (const std::string & canonicalRepresentation);

	const std::string &     getTable       () const;
	const std::string &     getText        () const;
	unsigned long           getTextIndex   () const;

	bool                    isInvalid      () const;
	bool                    isValid        () const;
	void                    clear          ();

	std::string             getDebugString () const;
	
	std::string             getCanonicalRepresentation () const;
	
	void                    setTable       (const std::string & table);
	void                    setText        (const std::string & text);

	//-- text index is a mutable property of the StringId
	//-- it is set the first time a StringId is localized, to speed up future lookups

	void                    setTextIndex   (unsigned long textIndex) const;

	bool                    localize       (LocUnicodeString & result, bool forceEnglish = false) const;
	LocUnicodeString        localize       (bool forceEnglish = false) const;
	bool                    localize       (LocUnicodeString & result, const Unicode::NarrowString & locale, bool forceEnglish = false) const;
	LocUnicodeString        localize       (const Unicode::NarrowString & locale, bool forceEnglish = false) const;

	static LocUnicodeString decodeString   (const LocUnicodeString & str);
	static StringId         decodeStringId (const LocUnicodeString & str);

public:
	// These are defined in MiscPack.cpp, to avoid making this library require linking with the database library
	void                unpackFromDatabase (const DB::BufferString &table, const DB::BufferString &text);
	void                packToDatabase     (DB::BufferString &table, DB::BufferString &text) const;
	
private:

	std::string                 m_table;
	std::string                 m_text;
	mutable unsigned long       m_textIndex; //if this number is nonzero, assume it is valid
};

//----------------------------------------------------------------------

inline const std::string & StringId::getTable() const
{
	return m_table;
}

//----------------------------------------------------------------------

inline const std::string & StringId::getText() const
{
	return m_text;
}

//----------------------------------------------------------------------

inline unsigned long StringId::getTextIndex() const
{
	return m_textIndex;
}

//========================================================================
// friend functions

inline int operator -(const StringId & lhs, const StringId & rhs)
{
	return (lhs != rhs) ? 1 : 0;
}

//----------------------------------------------------------------------

inline bool operator !=(const StringId & lhs, const StringId & rhs)
{
	return !(lhs == rhs);
}

//----------------------------------------------------------------------

inline bool operator ==(const StringId & lhs, const StringId & rhs)
{
	if (&lhs == &rhs)
		return true;
	if (lhs.m_table	!= rhs.m_table)
		return false;
	if (lhs.m_text != rhs.m_text)
		return false;
	if (lhs.m_text.empty() && lhs.m_textIndex != rhs.m_textIndex)
		return false;
	return true;
}

//----------------------------------------------------------------------

inline bool operator <(const StringId & lhs, const StringId & rhs)
{
	if (&lhs == &rhs)
		return false;
	if (lhs.m_table	!= rhs.m_table)
		return lhs.m_table < rhs.m_table;
	if (lhs.m_text != rhs.m_text)
		return lhs.m_text < rhs.m_text;
	if (lhs.m_text.empty() && lhs.m_textIndex != rhs.m_textIndex)
		return lhs.m_textIndex < rhs.m_textIndex;
	return false;
}

//----------------------------------------------------------------------

#endif	// _INCLUDED_StringId_H
