// ======================================================================
//
// LocalizedStringTable.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocalizedStringTable_H
#define INCLUDED_LocalizedStringTable_H


#if WIN32
// stl warning func not inlined
#pragma warning (disable:4710)
// unref inline func removed
#pragma warning (disable:4514)
// symbol name too long
#pragma warning (disable:4786)
#endif

#include "Unicode.h"
#include "LocalizedString.h"

#include <map>

class LocalizedString;
class AbstractFile;
class AbstractFileFactory;

// ======================================================================

/**
* A LocalizedStringTable maps ids and names to LocalizedString.
* The fastest lookup is done via find-by-id.
*/

class LocalizedStringTable
{
public:

	class        StringSearchPredicate;
	friend class LocalizationManager;


	typedef std::map<LocalizedString::id_type, LocalizedString *>     Map_t;
	typedef std::map<std::string, LocalizedString::id_type> NameMap_t;

	// TODO: make this typedef platform dependent
	typedef long                  magic_type;
	static const magic_type       ms_MAGIC;

	explicit                      LocalizedStringTable (const std::string & filename);
	virtual                      ~LocalizedStringTable ();

	const std::string &           getName     () const;
	const std::string &           getFileName () const;
	const Map_t &                 getMap      () const;
	const NameMap_t &             getNameMap          () const;

	static LocalizedStringTable * load                (AbstractFileFactory & fileFactory, const std::string & filename);

	const LocalizedString *       getLocalizedString  (LocalizedString::id_type id) const;
	const LocalizedString *       getLocalizedString  (const std::string & name) const;
	LocalizedString::id_type      getIdByName         (const std::string & name) const;
	LocalizedString::id_type      getIdByAbbrev       (const std::string & name, std::string & fullName) const;
	LocalizedString::id_type      getIdByAbbrevNocase (const std::string & name, std::string & fullName) const;

	const std::string *           getStringNameByIdSlowly (size_t id) const;

	static bool                   validateStringName (const std::string & name);
	static void                   fixupStringName    (std::string & name);

	char getVersion() const;
	static char getCurrentVersion();

protected:

	                              LocalizedStringTable (const LocalizedStringTable & rhs);

	bool                          load_0000(AbstractFile & fl);
	bool                          load_0001(AbstractFile & fl);

	static AbstractFile *         openLoadFile (AbstractFileFactory & fileFactory, const std::string & filename, char & version);

	void setVersion(char version);

	std::string                   m_name;
	std::string                   m_filename;
	Map_t                         m_map;
	NameMap_t                     m_nameMap;
	LocalizedString::id_type      m_nextUniqueId;
	char m_version;

	/**
	* used by LocalizationManager
	*/
	mutable size_t                m_referenceCount;


private:
	                              LocalizedStringTable ();
	LocalizedStringTable &        operator=            (const LocalizedStringTable & rhs);
};

//-----------------------------------------------------------------

/**
* Get the short name of this table.
*/

inline const std::string & LocalizedStringTable::getName () const
{
	return m_name;
}

//-----------------------------------------------------------------

/**
* Get the absolute filename of this table.
*/

inline const std::string & LocalizedStringTable::getFileName () const
{
	return m_filename;
}

//-----------------------------------------------------------------

/**
* Get a const reference to the id -> LocalizedString map
*/

inline const LocalizedStringTable::Map_t & LocalizedStringTable::getMap () const
{
	return m_map;
}

//-----------------------------------------------------------------

/**
* Get a const reference to the name -> id map
*/

inline const LocalizedStringTable::NameMap_t & LocalizedStringTable::getNameMap () const
{
	return m_nameMap;
}

//-----------------------------------------------------------------

/**
* Find a LocalizedString by id.  Fast map lookup
*/

inline const LocalizedString *       LocalizedStringTable::getLocalizedString (LocalizedString::id_type id) const
{
	Map_t::const_iterator iter = m_map.find (id);

	return (iter != m_map.end ()) ? (*iter).second : 0;
}

//-----------------------------------------------------------------

/**
* Find an id by name.  Fast map lookup.
* index zero is special and indicates a failed lookup.
*/

inline LocalizedString::id_type LocalizedStringTable::getIdByName (const std::string & name) const
{
	NameMap_t::const_iterator iter = m_nameMap.find (name);

	return (iter != m_nameMap.end ()) ? (*iter).second : 0;
}

//-----------------------------------------------------------------

/**
* Find a LocalizedString by id.  Done via 2 fast map lookups.
*/

inline const LocalizedString *       LocalizedStringTable::getLocalizedString (const std::string & name) const
{
	return getLocalizedString (getIdByName (name));
}

//-----------------------------------------------------------------

inline char LocalizedStringTable::getVersion() const
{
	return m_version;
}

//-----------------------------------------------------------------

inline void LocalizedStringTable::setVersion(char const version)
{
	m_version = version;
}

// ======================================================================

#endif
