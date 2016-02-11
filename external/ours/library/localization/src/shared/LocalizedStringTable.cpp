// ======================================================================
//
// LocalizedStringTable.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLocalization.h"
#include "LocalizedStringTable.h"

#include "fileInterface/AbstractFile.h"
#include "LocalizedString.h"
#include "UnicodeUtils.h"

#include <algorithm>

#include <cstdio>
#include <cassert>
#include <cctype>

// ======================================================================

namespace
{
	struct AbbrevFinder
	{
		const std::string & m_str;
		
		AbbrevFinder (const std::string & str) : m_str (str) {}
		
		bool operator () (const LocalizedStringTable::NameMap_t::value_type & t) const
		{
#if defined(WIN32)
			return _strnicmp (t.first.c_str (), m_str.c_str (), m_str.length ()) == 0;
#elif defined(linux)
			return strncasecmp (t.first.c_str (), m_str.c_str (), m_str.length ()) == 0;
#endif
		}
		
		AbbrevFinder (const AbbrevFinder & rhs) : m_str (rhs.m_str) {}
		
	private:
		AbbrevFinder & operator=(const AbbrevFinder &);
	};

	//----------------------------------------------------------------------

	const std::string & getValidString ()
	{		
		static std::string valid;
		static bool init = false;
		
		if (!init)
		{
			valid.reserve (38); // 26 alphabet + 10 digits + '_' + '+'
			int c = 0;
			
			for (c = 'a'; c <= 'z'; ++c)
				valid.push_back (static_cast<char>(c));
			
			for (c = '0'; c <= '9'; ++c)
				valid.push_back (static_cast<char>(c));
			
			valid.push_back ('_');
			valid.push_back ('+');
			init = true;
		}
		
		return valid;
	}

	//----------------------------------------------------------------------
	
	// Set the current version of the LocalizedString
	int const FILE_VERSION = 1;
}

//----------------------------------------------------------------------

const LocalizedStringTable::magic_type LocalizedStringTable::ms_MAGIC = 0xabcd;

//-----------------------------------------------------------------

LocalizedStringTable::LocalizedStringTable (const std::string & filename) :
m_name (),
m_filename (filename),
m_map (),
m_nameMap (),
m_nextUniqueId (0),
m_version(FILE_VERSION),
m_referenceCount (0)
{
}

//-----------------------------------------------------------------

LocalizedStringTable::LocalizedStringTable () :
m_name (),
m_filename (0),
m_map (),
m_nameMap (),
m_nextUniqueId (0),
m_version(FILE_VERSION),
m_referenceCount (0)
{
}

//-----------------------------------------------------------------

LocalizedStringTable::LocalizedStringTable (const LocalizedStringTable & rhs) :
m_name (rhs.m_name),
m_filename (rhs.m_filename),
m_map (rhs.m_map),
m_nameMap (),
m_nextUniqueId (rhs.m_nextUniqueId),
m_version(FILE_VERSION),
m_referenceCount (0)
{
	// TODO: make reference count copy? - jww
}

//-----------------------------------------------------------------

LocalizedStringTable::~LocalizedStringTable ()
{
	for (Map_t::iterator iter = m_map.begin (); iter != m_map.end (); ++iter)
	{
		delete (*iter).second;
		(*iter).second = 0;
	}

	m_map.clear ();
}
//-----------------------------------------------------------------


/**
* Load a version 0000 stringtable from a file stream
*/

bool LocalizedStringTable::load_0000 (AbstractFile & fl)
{	
	if (!fl.read (&m_nextUniqueId, sizeof (LocalizedString::id_type)))
		return false;

	LocalizedString::id_type num_entries;
	
	if (!fl.read (&num_entries, sizeof (LocalizedString::id_type)))
		return false;
	
	// load the string table

	{
		for (size_t i = 0; i < num_entries; ++i)
		{
			LocalizedString * loc_str = LocalizedString::load_0000 (fl);
			
			if (loc_str == 0)
				return false;
			
			assert (m_map.find (loc_str->getId ()) == m_map.end ()); //lint !e1924 // c-style cast.  MSVC bug
			
			std::pair<Map_t::const_iterator, bool> retval = m_map.insert (Map_t::value_type (loc_str->getId (), loc_str));
			
			// failed to insert string into map?
			assert (retval.second == true); //lint !e1924 // c-style cast.  MSVC bug
			
			if (retval.second == false)
				return false;

			m_nextUniqueId = std::max (m_nextUniqueId, loc_str->getId () + 1);
		}
	}
	
	// load the name lookup table	for (size_t i = 0; i < num_entries; ++i)

	{
		for (size_t i = 0; i < num_entries; ++i)
		{
			LocalizedString::id_type id;
			
			if (!fl.read (&id, sizeof (LocalizedString::id_type)))
				return false;
			
			LocalizedString::id_type buflen;

			if (!fl.read (&buflen, sizeof (LocalizedString::id_type)))
				return false;

			// buflen does not include nullptr terminator
			char * buf = new char [buflen+1];

			assert (buf != nullptr); //lint !e1924 // c-style cast.  MSVC bug

			buf [buflen] = 0;

			if (buflen && !fl.read (buf, buflen))
			{
				delete[] buf;
				buf = 0;
				return false;
			}

			std::pair<NameMap_t::const_iterator, bool> retval = m_nameMap.insert (NameMap_t::value_type (buf, id));

			delete[] buf;
			buf = 0;

			// failed to insert pair into map?
			assert (retval.second == true); //lint !e1924 // c-style cast.  MSVC bug

			if (retval.second == false)
			{
				return false;
			}
		}
	}

	setVersion(0);

	return true;
}

// ----------------------------------------------------------------------

/**
* Load a version 0001 stringtable from a file stream
*/

bool LocalizedStringTable::load_0001(AbstractFile & fl)
{	
	if (!fl.read (&m_nextUniqueId, sizeof (LocalizedString::id_type)))
		return false;

	LocalizedString::id_type num_entries;
	
	if (!fl.read (&num_entries, sizeof (LocalizedString::id_type)))
		return false;
	
	// load the string table

	{
		for (size_t i = 0; i < num_entries; ++i)
		{
			LocalizedString * loc_str = LocalizedString::load_0001(fl);
			
			if (loc_str == 0)
				return false;
			
			assert (m_map.find (loc_str->getId ()) == m_map.end ()); //lint !e1924 // c-style cast.  MSVC bug
			
			std::pair<Map_t::const_iterator, bool> retval = m_map.insert (Map_t::value_type (loc_str->getId (), loc_str));
			
			// failed to insert string into map?
			assert (retval.second == true); //lint !e1924 // c-style cast.  MSVC bug
			
			if (retval.second == false)
				return false;

			m_nextUniqueId = std::max (m_nextUniqueId, loc_str->getId () + 1);
		}
	}
	
	// load the name lookup table	for (size_t i = 0; i < num_entries; ++i)

	{
		for (size_t i = 0; i < num_entries; ++i)
		{
			LocalizedString::id_type id;
			
			if (!fl.read (&id, sizeof (LocalizedString::id_type)))
				return false;
			
			LocalizedString::id_type buflen;

			if (!fl.read (&buflen, sizeof (LocalizedString::id_type)))
				return false;

			// buflen does not include nullptr terminator
			char * buf = new char [buflen+1];

			assert (buf != nullptr); //lint !e1924 // c-style cast.  MSVC bug

			buf [buflen] = 0;

			if (buflen && !fl.read (buf, buflen))
			{
				delete[] buf;
				buf = 0;
				return false;
			}

			std::pair<NameMap_t::const_iterator, bool> retval = m_nameMap.insert (NameMap_t::value_type (buf, id));

			delete[] buf;
			buf = 0;

			// failed to insert pair into map?
			assert (retval.second == true); //lint !e1924 // c-style cast.  MSVC bug

			if (retval.second == false)
			{
				return false;
			}
		}
	}

	setVersion(1);

	return true;
}


// ----------------------------------------------------------------------

/**
* Load a table from a file specified by filename.
*/

LocalizedStringTable *LocalizedStringTable::load (AbstractFileFactory & fileFactory, const std::string & filename)
{
	
	char version = -1;
	
	AbstractFile * fl = openLoadFile (fileFactory, filename, version);
	
	if (fl == 0)
		return 0;
	
	LocalizedStringTable * table = 0;
	
	switch (version)
	{
	case 0:
		table = new LocalizedStringTable (filename);
		assert (table != nullptr); //lint !e1924 // c-style cast.  MSVC bug

		if (table->load_0000 (*fl) == false)
		{
			delete table;
			table = 0;
		}
		break;

	case 1:
		table = new LocalizedStringTable (filename);
		assert (table != nullptr); //lint !e1924 // c-style cast.  MSVC bug
		
		if (table->load_0001 (*fl) == false)
		{
			delete table;
			table = 0;
		}
		break;

	default:
		assert (true); //lint !e1924 // c-style cast.  MSVC bug
		break;
	}
	
	delete fl;
	return table;
}

//-----------------------------------------------------------------

/**
* 
*/

AbstractFile * LocalizedStringTable::openLoadFile (AbstractFileFactory & fileFactory, const std::string & filename, char & version)
{
	static_cast<void>(filename);

	AbstractFile * fl = fileFactory.createFile (filename.c_str(), "rb");
	
	if (fl == 0)
		return 0;

	magic_type magic;

	if (!fl->read (&magic, sizeof(magic_type)))
	{
		delete fl;
		return 0;
	}

	// TODO: swab magic if big endian

	// wrong magic file type found in file header
	if (magic != LocalizedStringTable::ms_MAGIC)
	{
		delete fl;
		return 0;
	}

	char local_version;

	if (!fl->read (&local_version, sizeof(char)))
	{
		delete fl;
		return 0;
	}

	version = local_version;

	return fl;
}

//-----------------------------------------------------------------

/**
* The name must be cased the same as the key
*/

LocalizedString::id_type     LocalizedStringTable::getIdByAbbrev (const std::string & name, std::string & fullName) const
{

	NameMap_t::const_iterator it = std::find_if (m_nameMap.begin (), m_nameMap.end (), AbbrevFinder (name));
	if (it != m_nameMap.end ())
	{
		fullName = (*it).first;
		return (*it).second;
	}
	
	return 0;
}

//-----------------------------------------------------------------

/**
* Find an id by case-insensitive abbreviation.  Slow linear search w/ string manipulation overhead.
*/

LocalizedString::id_type      LocalizedStringTable::getIdByAbbrevNocase (const std::string & name, std::string & fullName) const
{
	return getIdByAbbrev (Unicode::toLower (name), fullName);
}

//-----------------------------------------------------------------

/**
* Find an id by name.  Slow linear search.
*/

const std::string * LocalizedStringTable::getStringNameByIdSlowly (size_t id) const
{
	for ( LocalizedStringTable::NameMap_t::const_iterator it = m_nameMap.begin (); it != m_nameMap.end (); ++it)
	{
		if ((*it).second == id)
			return &(*it).first;
	}

	return 0;
}

//----------------------------------------------------------------------

bool LocalizedStringTable::validateStringName (const std::string & name)
{
	if (name.empty ())
		return false;

	const std::string & valid = getValidString ();

	//-- name must begin with a lowercase alpha
	return isalpha (name [0]) && name.find_first_not_of (valid) == name.npos;
}

//----------------------------------------------------------------------

void LocalizedStringTable::fixupStringName (std::string & name)
{
	if (name.empty ())
		return;
	
	for (size_t i = 0; i < name.size ();)
	{
		const char c = name [i];
		if (isalpha (c))
			name [i] = static_cast<char>(tolower (c));
		else if (c == ' ')
			name [i] = '_';
		else if (!isdigit (c) && c != '_')
		{
			name.erase (i, 1);
			continue;
		}
		++i;
	}

	while (!name.empty () && !isalpha (name [0]))
		name.erase (static_cast<size_t>(0), 1);
}


//----------------------------------------------------------------------

char LocalizedStringTable::getCurrentVersion()
{
	return FILE_VERSION;	
}

// ======================================================================
