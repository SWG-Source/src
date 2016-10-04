// ======================================================================
//
// LocalizedStringTableRW.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLocalization.h"
#include "LocalizedStringTableReaderWriter.h"

#include "LocalizedString.h"
#include "fileInterface/AbstractFile.h"
#include "UnicodeUtils.h"

#include <algorithm>
#include <cstdio>

#include <cassert>
#include <cctype>

namespace LocalizedStringTableReaderWriterNamespace
{
	void unfubarMicrosoftInvalidTextCharacters(Unicode::String & str)
	{
		typedef std::pair <Unicode::unicode_char_t, Unicode::unicode_char_t> FubarCharacterInfo;
		typedef std::pair <Unicode::unicode_char_t, Unicode::String>         FubarStringInfo;

		static const FubarCharacterInfo FubarCharactersCourtesyMicrosoft[] =
		{
			FubarCharacterInfo(139,   '<'),
			FubarCharacterInfo(155,   '>'),

			FubarCharacterInfo(8216,   '\''),
			FubarCharacterInfo(8217,   '\''),
			FubarCharacterInfo(8221,   '"'),
			FubarCharacterInfo(8220,   '"'),
			FubarCharacterInfo(149,   '-'),
			FubarCharacterInfo(150,   '-'),
			FubarCharacterInfo(151,   '-'),
			FubarCharacterInfo(152,   '~')
		};

		static const int FubarCharactersCourtesyMicrosoftLength = sizeof(FubarCharactersCourtesyMicrosoft) / sizeof(FubarCharactersCourtesyMicrosoft[0]);

		static const FubarStringInfo FubarStringsCourtesyMicrosoft[] =
		{
			FubarStringInfo(133,   Unicode::narrowToWide("...")),
			//-- carriage return
		FubarStringInfo(13,    Unicode::narrowToWide(""))
		};

		static const int FubarStringsCourtesyMicrosoftLength = sizeof(FubarStringsCourtesyMicrosoft) / sizeof(FubarStringsCourtesyMicrosoft[0]);

		{
			for (int i = 0; i < FubarCharactersCourtesyMicrosoftLength; ++i)
			{
				const Unicode::unicode_char_t correctCharacter = FubarCharactersCourtesyMicrosoft[i].second;
				const Unicode::unicode_char_t fubarCharacter = FubarCharactersCourtesyMicrosoft[i].first;

				size_t pos = 0;
				while ((pos = str.find(fubarCharacter, pos)) != Unicode::String::npos)
				{
					str[pos] = correctCharacter;
					++pos;
				}
			}
		}

		{
			for (int i = 0; i < FubarStringsCourtesyMicrosoftLength; ++i)
			{
				const Unicode::unicode_char_t fubarCharacter = FubarStringsCourtesyMicrosoft[i].first;
				const Unicode::String & correctString = FubarStringsCourtesyMicrosoft[i].second;

				size_t pos = 0;

				while ((pos = str.find(fubarCharacter, pos)) != Unicode::String::npos)
				{
					str.replace(pos, 1, correctString);
					pos += correctString.size();
				}
			}
		}
	}
}

using namespace LocalizedStringTableReaderWriterNamespace;

//-----------------------------------------------------------------

LocalizedStringTableRW::LocalizedStringTableRW(const Unicode::NarrowString & filename) :
	LocalizedStringTable(filename),
	m_idNameMap()
{
}

//-----------------------------------------------------------------

LocalizedStringTableRW::LocalizedStringTableRW(const LocalizedStringTable & rhs) :
	LocalizedStringTable(rhs),
	m_idNameMap()
{
}

//-----------------------------------------------------------------

bool LocalizedStringTableRW::str_write(AbstractFile & fl, LocalizedString & locstr)
{
	// TODO: swab all values on big endian systems

	LocalizedString::id_type id = locstr.m_id;
	LocalizedString::crc_type crcSource = locstr.m_sourceCrc;
	LocalizedString::id_type buflen = locstr.m_str.length();

	if (!fl.write(sizeof(LocalizedString::id_type), &id))
		return false;

	if (!fl.write(sizeof(LocalizedString::crc_type), &crcSource))
		return false;

	if (!fl.write(sizeof(LocalizedString::id_type), &buflen))
		return false;

	// TODO: swab this buffer
	Unicode::unicode_char_t * buf = new Unicode::unicode_char_t[buflen + 1];

	assert(buf != nullptr); //lint !e1924 // c-style cast.  MSVC bug

	buf[buflen] = 0;
	memcpy(buf, locstr.m_str.c_str(), sizeof(Unicode::unicode_char_t) * buflen);

	if (buflen && !fl.write(buflen * sizeof(Unicode::unicode_char_t), buf))
	{
		delete[] buf;
		return false;
	}

	delete[] buf;

	return true;
}

//-----------------------------------------------------------------

bool LocalizedStringTableRW::write(AbstractFile & fl) const
{
	// TODO: swab these values on big endian systems
	LocalizedString::id_type next_unique = m_nextUniqueId;
	LocalizedString::id_type num_entries = m_map.size();

	if (!fl.write(sizeof(LocalizedString::id_type), &next_unique))
		return false;

	if (!fl.write(sizeof(LocalizedString::id_type), &num_entries))
		return false;

	// write the string table

	{
		for (Map_t::const_iterator iter = m_map.begin(); iter != m_map.end(); ++iter)
		{
			if (str_write(fl, *((*iter).second)) == false)
				return false;
		}
	}

	// write the string/id map
	{
		for (NameMap_t::const_iterator iter = m_nameMap.begin(); iter != m_nameMap.end(); ++iter)
		{
			// TODO: swab all these values on big endian systems
			LocalizedString::id_type id = (*iter).second;
			LocalizedString::id_type buflen = (*iter).first.length();

			if (!fl.write(sizeof(LocalizedString::id_type), &id))
				return false;

			if (!fl.write(sizeof(LocalizedString::id_type), &buflen))
				return false;

			char * buf = new char[buflen + 1];

			assert(buf != nullptr); //lint !e1924 // c-style cast.  MSVC bug

			buf[buflen] = 0;

			memcpy(buf, (*iter).first.c_str(), buflen);

			if (buflen && !fl.write(sizeof(char) * buflen, buf))
			{
				delete[] buf;
				buf = 0;
				return false;
			}

			delete[] buf;
			buf = 0;
		}
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 */

LocalizedStringTableRW *LocalizedStringTableRW::loadRW(AbstractFileFactory & fileFactory, const Unicode::NarrowString & filename)
{
	char version = -1;

	AbstractFile * const fl = openLoadFile(fileFactory, filename, version);

	if (fl == 0)
		return 0;

	LocalizedStringTableRW * table = nullptr;

	if (!fl->isOpen())
	{
	}
	else
	{
		switch (version)
		{
		case 0:
			table = new LocalizedStringTableRW(filename);
			assert(table != nullptr);  //lint !e1924 // c-style cast.  MSVC bug

			if (table->load_0000(*fl) == false)
			{
				delete table;
				table = nullptr;
			}
			break;

		case 1:
			table = new LocalizedStringTableRW(filename);
			assert(table != nullptr);  //lint !e1924 // c-style cast.  MSVC bug

			if (table->load_0001(*fl) == false)
			{
				delete table;
				table = nullptr;
			}
			break;
		default:
			assert(true); //lint !e1924 // c-style cast.  MSVC bug
			break;
		}
	}

	delete fl;

	if (table)
	{
		//Go through all the strings and un-microsoft-fubar them
		for (Map_t::iterator mapIter = table->m_map.begin(); mapIter != table->m_map.end(); ++mapIter)
		{
			LocalizedString *str = (*mapIter).second;
			Unicode::String & strRef = str->m_str;
			LocalizedStringTableReaderWriterNamespace::unfubarMicrosoftInvalidTextCharacters(strRef);
		}

		// populate idNameMap
		table->m_idNameMap.clear();

		for (NameMap_t::const_iterator iter = table->m_nameMap.begin(); iter != table->m_nameMap.end(); ++iter)
		{
			table->m_idNameMap.insert(IdNameMap_t::value_type((*iter).second, (*iter).first));
		}
	}

	return table;
}

// ----------------------------------------------------------------------
/**
 */

bool LocalizedStringTableRW::writeRW(AbstractFileFactory & fileFactory, const Unicode::NarrowString & filename) const
{
	//Go through all the strings and un-microsoft-fubar them
	for (Map_t::const_iterator mapIter = m_map.begin(); mapIter != m_map.end(); ++mapIter)
	{
		LocalizedString *str = (*mapIter).second;
		Unicode::String & strRef = str->m_str;
		LocalizedStringTableReaderWriterNamespace::unfubarMicrosoftInvalidTextCharacters(strRef);
	}

	AbstractFile * const fl = fileFactory.createFile(filename.c_str(), "wb");

	if (fl == 0)
		return false;

	bool retval = false;

	if (!fl->isOpen())
	{
		retval = false;
	}
	else
	{
		// TODO: swab magic if big endian

		magic_type local_magic = LocalizedStringTable::ms_MAGIC;

		if (!fl->write(sizeof(magic_type), &local_magic))
		{
			delete fl;
			return false;
		}

		char currentVersion = LocalizedStringTable::getCurrentVersion();
		if (!fl->write(sizeof(char), &currentVersion))
		{
			delete fl;
			return false;
		}

		retval = write(*fl);
	}

	delete fl;
	return retval;
}

//-----------------------------------------------------------------

LocalizedString *  LocalizedStringTableRW::addString(const Unicode::String & str, Unicode::NarrowString & theNameResult)
{
	LocalizedString * const locstr = (m_map[m_nextUniqueId] = new LocalizedString(m_nextUniqueId, int(time(0)), str));

	assert(locstr != nullptr);   //lint !e1924 // c-style cast.  MSVC bug

	char buf[64];
	sprintf(buf, "%03ld_default", m_nextUniqueId);

	Unicode::NarrowString name(buf);

	const std::pair<NameMap_t::const_iterator, bool> retval = m_nameMap.insert(NameMap_t::value_type(name, m_nextUniqueId));

	assert(retval.second == true);  //lint !e1924 // c-style cast.  MSVC bug

	m_idNameMap[locstr->getId()] = name;

	theNameResult = name;

	++m_nextUniqueId;

	return retval.second == true ? locstr : 0;
}

//-----------------------------------------------------------------

LocalizedString *             LocalizedStringTableRW::addString(LocalizedString * locstr, const Unicode::NarrowString & name, std::string & resultStr)
{
	char buf[1024];
	const size_t id = locstr->getId();

	{
		Map_t::const_iterator it = m_map.find(id);
		if (it != m_map.end())
		{
			const std::string *     old_name = getNameById(id);
			const LocalizedString * old_locstr = (*it).second;
			const std::string       old_str = old_locstr ? Unicode::wideToNarrow(old_locstr->getString()) : std::string();

			sprintf(buf,
				"LocalizedStringTableRW::addString failed inserting duplicate id [%d]\n"
				"Existing string name=[%s], str=[%s]\n",
				id,
				old_name ? old_name->c_str() : "",
				old_str.c_str());
			resultStr += buf;
			return 0;
		}
	}

	{
		NameMap_t::const_iterator it = m_nameMap.find(name);
		if (it != m_nameMap.end())
		{
			const size_t            old_id = (*it).second;
			const std::string *     old_name = getNameById(old_id);
			const LocalizedString * old_locstr = getLocalizedString(old_id);
			const std::string       old_str = old_locstr ? Unicode::wideToNarrow(old_locstr->getString()) : std::string();

			sprintf(buf,
				"LocalizedStringTableRW::addString failed inserting duplicate name [%s]\n"
				"Existing string id=[%d] name=[%s], str=[%s]\n",
				name.c_str(),
				id,
				old_name ? old_name->c_str() : "",
				old_str.c_str());
			resultStr += buf;
			return 0;
		}
	}

	{
		IdNameMap_t::const_iterator it = m_idNameMap.find(id);
		if (it != m_idNameMap.end())
		{
			const std::string &     old_name = (*it).second;
			const LocalizedString * old_locstr = getLocalizedString(id);
			const std::string       old_str = old_locstr ? Unicode::wideToNarrow(old_locstr->getString()) : std::string();

			sprintf(buf,
				"LocalizedStringTableRW::addString failed inserting duplicate name [%s]\n"
				"Existing string name=[%s], str=[%s]\n",
				name.c_str(),
				old_name.c_str(),
				old_str.c_str());
			resultStr += buf;
			return 0;
		}
	}

	m_map[id] = locstr;
	m_nameMap[name] = locstr->getId();
	m_idNameMap[id] = name;

	m_nextUniqueId = std::max(m_nextUniqueId, locstr->getId() + 1);

	return locstr;
}

//----------------------------------------------------------------------

LocalizedString *             LocalizedStringTableRW::addString(LocalizedString * locstr, const Unicode::String & name, std::string & resultStr)
{
	return addString(locstr, Unicode::wideToNarrow(name), resultStr);
}

//-----------------------------------------------------------------

bool LocalizedStringTableRW::removeStringByName(const Unicode::NarrowString & name)
{
	NameMap_t::iterator find_iter_id = m_nameMap.find(name);

	if (find_iter_id == m_nameMap.end())
		return false;

	const size_t id = (*find_iter_id).second;

	Map_t::iterator find_iter_string = m_map.find(id);

	assert(find_iter_string != m_map.end());  //lint !e1924 // c-style cast.  MSVC bug

	IdNameMap_t::iterator find_iter_id_map = m_idNameMap.find(id);

	assert(find_iter_id_map != m_idNameMap.end());  //lint !e1924 // c-style cast.  MSVC bug

	m_nameMap.erase(find_iter_id);
	m_map.erase(find_iter_string);
	m_idNameMap.erase(find_iter_id_map);

	return true;
}

//-----------------------------------------------------------------

bool LocalizedStringTableRW::rename(const Unicode::NarrowString & name, const Unicode::NarrowString & newName)
{
	//
	if (name == newName)
		return false;

	// names that start with numerals are special
	if (isdigit(newName[0]))
		return false;

	NameMap_t::iterator find_iter_id = m_nameMap.find(newName);

	// newName already exists
	if (find_iter_id != m_nameMap.end())
	{
		LocalizedString::id_type id = (*find_iter_id).second;

		LocalizedString * locstr = getLocalizedString(id);

		static_cast<void>(locstr);

		return false;
	}

	find_iter_id = m_nameMap.find(name);

	// old name does not exist
	if (find_iter_id == m_nameMap.end())
		return false;

	size_t id = (*find_iter_id).second;

	m_nameMap.erase(find_iter_id);

	const std::pair<NameMap_t::const_iterator, bool> retval = m_nameMap.insert(NameMap_t::value_type(newName, id));

	assert(retval.second == true);  //lint !e1924 // c-style cast.  MSVC bug

	m_idNameMap[id] = newName;

	return retval.second == true;
}

//-----------------------------------------------------------------

void LocalizedStringTableRW::setName(const Unicode::NarrowString & name)
{
	m_name = name;
}

//-----------------------------------------------------------------

void LocalizedStringTableRW::prepareTable(const LocalizedStringTableRW & rhs)
{
	m_idNameMap.clear();
	m_nameMap.clear();
	m_map.clear();

	m_idNameMap = rhs.m_idNameMap;
	m_nameMap = rhs.m_nameMap;
	m_nextUniqueId = rhs.m_nextUniqueId;
	m_name = rhs.m_name;

	for (Map_t::const_iterator iter = rhs.m_map.begin(); iter != rhs.m_map.end(); ++iter)
	{
		const LocalizedString * rhs_locstr = (*iter).second;

		LocalizedString * locstr = new LocalizedString(rhs_locstr->m_id, 0, rhs_locstr->m_str);

		assert(locstr != nullptr); //lint !e1924 // c-style cast.  MSVC bug
		m_map.insert(Map_t::value_type(locstr->getId(), locstr));
	}
}

//-----------------------------------------------------------------

LocalizedString * LocalizedStringTableRW::getLocalizedStringByName(const Unicode::String & name)
{
	return getLocalizedStringByName(Unicode::wideToNarrow(name));
}

// ======================================================================