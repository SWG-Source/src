// ======================================================================
//
// LocalizedStringTableRW.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocalizedStringTableRW_H
#define INCLUDED_LocalizedStringTableRW_H

#if WIN32
// stl warning func not inlined
#pragma warning (disable:4710)
// unref inline func removed
#pragma warning (disable:4514)
// symbol name too long
#pragma warning (disable:4786)
#endif

#include "LocalizedStringTable.h"
#include "LocalizedString.h"

#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <map>

class AbstractFile;

//-----------------------------------------------------------------

class LocalizedStringTableRW : public LocalizedStringTable
{
public:

	class StringRW;

	typedef std::map <LocalizedString::id_type, Unicode::NarrowString> IdNameMap_t;

	explicit                              LocalizedStringTableRW   (const Unicode::NarrowString & name);
	explicit                              LocalizedStringTableRW   (const LocalizedStringTable & rhs);

	LocalizedString::id_type              getNextUniqueId          () const;
	void                                  setNextUniqueId          (LocalizedString::id_type id);

	void                                  setName                  (const Unicode::NarrowString & name);
	Map_t &                               getMap                   (); //lint !e1511 // member hides non-virtual member

	const Unicode::NarrowString *         getNameById              (LocalizedString::id_type id) const;

	static LocalizedStringTableRW *       loadRW                   (AbstractFileFactory & fileFactory, const Unicode::NarrowString & filename);

	bool                                  write              (AbstractFile & fl) const;
	bool                                  writeRW                  (AbstractFileFactory & fileFactory, const Unicode::NarrowString & filename) const;

	LocalizedString *                     addString                (const Unicode::String & str, Unicode::NarrowString & theNameResult);
	LocalizedString *                     addString                (LocalizedString * locstr, const Unicode::NarrowString & name, std::string & resultStr);
	LocalizedString *                     addString                (LocalizedString * locstr, const Unicode::String & name, std::string & resultStr);

	bool                                  removeStringByName       (const Unicode::NarrowString & name);
	LocalizedString *                     getLocalizedStringByName (const Unicode::NarrowString & name);
	LocalizedString *                     getLocalizedStringByName (const Unicode::String & name);
	LocalizedString *                     getLocalizedString       (LocalizedString::id_type id); //lint !e1511 // member hides non-virtual member

	bool                                  rename                   (const Unicode::NarrowString & name, const Unicode::NarrowString & newName);

	static bool                           str_write           (AbstractFile & fl, LocalizedString & locstr);
	
	static const Unicode::String &        str_getString            (LocalizedString & locstr);
	static void                           str_setString            (LocalizedString & locstr, const Unicode::String & str);
	static LocalizedString::id_type &     str_getId                (LocalizedString & locstr);
	static LocalizedString::crc_type & str_getCrc(LocalizedString & locstr);
	static LocalizedString::crc_type & str_getSourceCrc(LocalizedString & locstr);

	void                                  prepareTable             (const LocalizedStringTableRW & rhs);

	IdNameMap_t &                   getIdNameMap ();
	NameMap_t &                     getNameMap (); //lint !e1511 // member hides non-virtual member

private:
	                                LocalizedStringTableRW ();
	                                LocalizedStringTableRW (const LocalizedStringTableRW & rhs);
	LocalizedStringTableRW &        operator=    (const LocalizedStringTableRW & rhs);

	IdNameMap_t                     m_idNameMap;
};

//-----------------------------------------------------------------

inline LocalizedString::id_type LocalizedStringTableRW::getNextUniqueId () const
{
	return m_nextUniqueId;
}

//-----------------------------------------------------------------

inline void LocalizedStringTableRW::setNextUniqueId (const LocalizedString::id_type id)
{
	m_nextUniqueId = id;
}

//-----------------------------------------------------------------

inline LocalizedStringTableRW::Map_t & LocalizedStringTableRW::getMap ()
{
	return m_map; //lint !e1536 // exposing low access member
}

//-----------------------------------------------------------------

inline const Unicode::String & LocalizedStringTableRW::str_getString (LocalizedString & locstr)
{
	return locstr.m_str;
}

//-----------------------------------------------------------------

inline void LocalizedStringTableRW::str_setString (LocalizedString & locstr, const Unicode::String & str)
{
	locstr.m_str = str;
	locstr.buildCrc();
	locstr.resetLineCounts ();
}

//-----------------------------------------------------------------

inline LocalizedString::id_type & LocalizedStringTableRW::str_getId (LocalizedString & locstr)
{
	return locstr.m_id;
}

//-----------------------------------------------------------------

inline LocalizedString::crc_type & LocalizedStringTableRW::str_getCrc (LocalizedString & locstr)
{
	return locstr.m_crc;
}

//-----------------------------------------------------------------

inline LocalizedString::crc_type & LocalizedStringTableRW::str_getSourceCrc(LocalizedString & locstr)
{
	return locstr.m_sourceCrc;
}

//-----------------------------------------------------------------

inline LocalizedString * LocalizedStringTableRW::getLocalizedStringByName (const Unicode::NarrowString & name)
{
	return getLocalizedString (getIdByName (name));
}

//-----------------------------------------------------------------

inline LocalizedString *       LocalizedStringTableRW::getLocalizedString (LocalizedString::id_type id)
{
	Map_t::const_iterator iter = m_map.find (id);

	return (iter != m_map.end ()) ? (*iter).second : 0;
}

//-----------------------------------------------------------------

inline const Unicode::NarrowString * LocalizedStringTableRW::getNameById (LocalizedString::id_type id) const
{
	IdNameMap_t::const_iterator iter = m_idNameMap.find (id);

	return (iter != m_idNameMap.end ()) ? &(*iter).second : 0;
}

//-----------------------------------------------------------------

inline LocalizedStringTableRW::IdNameMap_t &    LocalizedStringTableRW::getIdNameMap ()
{
	return m_idNameMap; //lint !e1536 // exposing low access member
}

//-----------------------------------------------------------------

inline LocalizedStringTable::NameMap_t &          LocalizedStringTableRW::getNameMap ()
{
	return m_nameMap; //lint !e1536 // exposing low access member
}

// ======================================================================

#endif
