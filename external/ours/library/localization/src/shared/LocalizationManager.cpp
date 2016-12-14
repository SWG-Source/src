// ======================================================================
//
// LocalizationManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLocalization.h"
#include "LocalizationManager.h"

#include "LocalizedStringTable.h"
#include "StringId.h"
#include "UnicodeUtils.h"
#include "fileInterface/AbstractFile.h"
#include <cassert>
#include <cctype>
#include <cstdio>
#include <vector>
#include <ctime>

// ======================================================================

namespace LocalizationManagerNamespace
{
	const std::string s_pathString = "string/";
	const std::string s_suffix     = ".stf";

	const std::string s_englishLocale = "en";

	std::string s_pathStringWithLocale;
	std::string s_pathStringWithEnglishLocale;

	typedef std::vector<LocalizedStringTable const *>  PreloadedStringTables;

	PreloadedStringTables ms_preloadedStringTables;

	bool s_debugStrings = false;
	bool s_displayStringIdInfo = false;


	Unicode::String s_debugDisplayColor(Unicode::narrowToWide(" \\#00ff00"));

	//----------------------------------------------------------------------

	void createBadStringRepresentation (const StringId & id, Unicode::String & value)
	{
		if (s_displayStringIdInfo) 
		{
			value += s_debugDisplayColor;
		}

		value += Unicode::narrowToWide(id.getText());

		if (!value.empty ())
		{
			size_t pos = 0;

			while (pos != std::string::npos)
			{
				const size_t underscore = value.find (Unicode::unicode_char_t ('_'), pos);
				if (underscore != std::string::npos)
				{
					value [underscore] = ' ';
					pos = underscore + 1;
				}
				else
					break;
			}

			value [0] = static_cast<Unicode::unicode_char_t>(toupper (value [0]));
		}
	}
}

using namespace LocalizationManagerNamespace;

//----------------------------------------------------------------------

bool                    LocalizationManager::ms_installed = 0;
LocalizationManager::LocalizationManagerHashMap * LocalizationManager::ms_singletonHashMap = nullptr;
Unicode::NarrowString * LocalizationManager::ms_firstLocaleLoaded = nullptr;

//-----------------------------------------------------------------

LocalizationManager::LocalizationManager (AbstractFileFactory * fileFactory, const Unicode::NarrowString & localeName, DebugBadStringsFunc debugBadStringsFunc, bool displayBadStringIds) :
m_fileFactory         (fileFactory),
m_stringTableMap      (),
m_englishStringTableMap (),
m_localeName          (localeName),
m_debugBadStringsFunc (debugBadStringsFunc),
m_displayBadStringIds (displayBadStringIds)
{
	s_pathStringWithLocale = s_pathString + m_localeName;
	s_pathStringWithLocale.push_back ('/');
	s_pathStringWithEnglishLocale = s_pathString + s_englishLocale;
	s_pathStringWithEnglishLocale.push_back ('/');
	if(s_englishLocale.compare(m_localeName) == 0  || m_localeName.compare("none") == 0)
	{
		m_usingEnglishLocale = true;
	}
	else
	{
		m_usingEnglishLocale = false;
	}

	assert (m_fileFactory != nullptr);//lint !e1924 // c-style cast.  MSVC bug
}

//-----------------------------------------------------------------

LocalizationManager::~LocalizationManager ()
{
	m_debugBadStringsFunc = 0;

	for (StringTableMap_t::iterator iter = m_stringTableMap.begin (); iter != m_stringTableMap.end (); ++iter)
	{
		LocalizedStringTable * const table = (*iter).second.second;

		if (table)
		{
			//-- the LocalizationManager holds a reference to any table requested
			assert (table->m_referenceCount == 1);//lint !e1924 // c-style cast.  MSVC bug

			delete table;
			(*iter).second.second = 0;
		}
	}

	m_stringTableMap.clear ();

	if(!m_usingEnglishLocale)
	{
		for (StringTableMap_t::iterator iter = m_englishStringTableMap.begin (); iter != m_englishStringTableMap.end (); ++iter)
		{
			LocalizedStringTable * const table = (*iter).second.second;

			if (table)
			{
				//-- the LocalizationManager holds a reference to any table requested
				assert (table->m_referenceCount == 1);//lint !e1924 // c-style cast.  MSVC bug

				delete table;
				(*iter).second.second = 0;
			}
		}

		m_englishStringTableMap.clear ();

	}
	delete m_fileFactory;
	m_fileFactory = 0;
}

//-----------------------------------------------------------------

void LocalizationManager::install (AbstractFileFactory * fileFactory, Unicode::UnicodeNarrowStringVector & localeNameMap, bool debugStrings, DebugBadStringsFunc debugBadStringsFunc, bool displayBadStringIds)
{
	assert (!ms_installed);//lint !e1924 // c-style cast.  MSVC bug
	assert (!ms_singletonHashMap);//lint !e1924 // c-style cast.  MSVC bug
	assert (!ms_firstLocaleLoaded);

	ms_singletonHashMap = new LocalizationManagerHashMap();

	for (auto iter = localeNameMap.begin(); iter != localeNameMap.end(); ++iter )
	{
		Unicode::NarrowString localeName = (*iter);
		LocalizationManager * localizationManager = new LocalizationManager (fileFactory, localeName, debugBadStringsFunc, displayBadStringIds);
		(*ms_singletonHashMap)[localeName] = localizationManager;
	}

	ms_firstLocaleLoaded = new Unicode::NarrowString((*(localeNameMap.begin())));

	ms_installed = true;

	s_debugStrings = debugStrings;
}

//-----------------------------------------------------------------

void LocalizationManager::remove  ()
{
	assert (ms_installed);//lint !e1924 // c-style cast.  MSVC bug
	assert (ms_singletonHashMap != nullptr);//lint !e1924 // c-style cast.  MSVC bug
	assert (ms_firstLocaleLoaded != nullptr);

	LocalizationManagerHashMap::iterator end = ms_singletonHashMap->end();
	for (LocalizationManagerHashMap::iterator it = ms_singletonHashMap->begin(); it != end; ++it)
	{
		LocalizationManager * current = (*it).second;
		(*it).second = nullptr;
		delete current;
	}

	ms_singletonHashMap->clear();
	delete ms_singletonHashMap;
	ms_singletonHashMap = nullptr;

	delete ms_firstLocaleLoaded;
	ms_firstLocaleLoaded = nullptr;

	ms_installed = false;
}

//-----------------------------------------------------------------

/**
* Get the default locale (English).  Undefined results if called without first calling install ()
*/ 
LocalizationManager & LocalizationManager::getManager ()
{
	return *(*LocalizationManager::ms_singletonHashMap)[*ms_firstLocaleLoaded];
}

//-----------------------------------------------------------------

/**
* Get the locale specified by locale ("en" == English, "ja" == Japanese).  Undefined results if called without first calling install ()
*/ 
LocalizationManager & LocalizationManager::getManager (const Unicode::NarrowString &locale)
{
	LocalizationManagerHashMap::iterator f = LocalizationManager::ms_singletonHashMap->find(locale);
	if(f != LocalizationManager::ms_singletonHashMap->end())
		return *((*f).second);
	return getManager();
}

//-----------------------------------------------------------------

void LocalizationManager::preload (const Unicode::NarrowString & name)
{
	LocalizedStringTable *table = fetchStringTable(name);
	if (table)
		ms_preloadedStringTables.push_back (table);
}

//-----------------------------------------------------------------

void LocalizationManager::releasePreloadedAssets ()
{
	while (!ms_preloadedStringTables.empty ())
	{
		releaseStringTable (ms_preloadedStringTables.back ());
		ms_preloadedStringTables.pop_back ();
	}
}

//-----------------------------------------------------------------

LocalizedStringTable * LocalizationManager::fetchStringTable(const Unicode::NarrowString & name)
{
	return fetchStringTable(name, false);
}

//-----------------------------------------------------------------

LocalizedStringTable * LocalizationManager::fetchStringTable(const Unicode::NarrowString & name, bool forceUseEnglish)
{
	// DBE - this is to prevent using the m_englishStringTableMap English exception if we're already localizing to English.
	// The rest of the code assumes that if we're localizing to English, m_englishStringTableMap will never be used.
	if (m_usingEnglishLocale)
	{
		forceUseEnglish=false;
	}

	// This bool is used to determine if we should try again with English.  We shouldn't if we are running in English (m_usingEnglishLocale)
	// or if we are already trying again with English (forceUseEnglish).
	bool const useEnglish = forceUseEnglish || m_usingEnglishLocale;

	// Use the locale specified unless we are forcing the use of English
	StringTableMap_t & stmap = (forceUseEnglish) ? m_englishStringTableMap : m_stringTableMap;
	std::string const & pathPrefix = (forceUseEnglish) ? s_pathStringWithEnglishLocale : s_pathStringWithLocale;

	LocalizedStringTable * table = 0;

	StringTableMap_t::iterator const find_iter = stmap.find (name);
	if(find_iter != stmap.end ())
	{
		TimedStringTable & tst = (*find_iter).second;
		//-- this can be nullptr
		table     = tst.second;
		tst.first = time(0);
	}
	else
	{
		static std::string filename;
		filename.clear ();
		filename += pathPrefix + name + s_suffix;

		table = LocalizedStringTable::load (*m_fileFactory, filename);

		if (table)
		{
			table->m_name = name;
			//-- the LocalizationManager holds a reference to any table requested, until purged
			fetchStringTable(table);
		}

		stmap.insert (std::make_pair (name, TimedStringTable (time(0), table)));
	}

	if (table)
		fetchStringTable(table);
	else if (!useEnglish)
		table = fetchStringTable(name, true);

	return table;
}

//----------------------------------------------------------------------

void LocalizationManager::garbageCollectUnused    (int timeoutThresholdSecs)
{
	const time_t currentTime = time (0);  //update last-used time
	for (StringTableMap_t::iterator iter = m_stringTableMap.begin (); iter != m_stringTableMap.end ();)
	{
		TimedStringTable & tst = (*iter).second;

		LocalizedStringTable * const table = tst.second;

		if (table && table->m_referenceCount == 1)
		{
			const time_t lastTouched = tst.first;
			const time_t diff        = currentTime - lastTouched;

			//-- if the string table is older than the threshold, dump it
			if (diff > timeoutThresholdSecs)
			{
				releaseStringTable(table);
				m_stringTableMap.erase (iter++);
				continue;
			}
		}

		++iter;
	}
	if(!m_usingEnglishLocale)
	{
		for (StringTableMap_t::iterator iter = m_englishStringTableMap.begin (); iter != m_englishStringTableMap.end ();)
		{
			TimedStringTable & tst = (*iter).second;

			LocalizedStringTable * const table = tst.second;

			if (table && table->m_referenceCount == 1)
			{
				const time_t lastTouched = tst.first;
				const time_t diff        = currentTime - lastTouched;

				//-- if the string table is older than the threshold, dump it
				if (diff > timeoutThresholdSecs)
				{
					releaseStringTable(table);
					m_englishStringTableMap.erase (iter++);
					continue;
				}
			}

			++iter;
		}
	}
}

//----------------------------------------------------------------------

void LocalizationManager::purgeUnusedStringTables ()
{
	for (StringTableMap_t::iterator iter = m_stringTableMap.begin (); iter != m_stringTableMap.end ();)
	{
		LocalizedStringTable * const table = (*iter).second.second;

		if (table)
		{
			//-- the LocalizationManager holds a reference to any table requested
			if (table->m_referenceCount == 1)
			{
				releaseStringTable(table);
				iter = m_stringTableMap.begin();
				continue;
			}
		}

		++iter;
	}
	if(!m_usingEnglishLocale)
	{
		for (StringTableMap_t::iterator iter = m_englishStringTableMap.begin (); iter != m_englishStringTableMap.end ();)
		{
			LocalizedStringTable * const table = (*iter).second.second;

			if (table)
			{
				//-- the LocalizationManager holds a reference to any table requested
				if (table->m_referenceCount == 1)
				{
					releaseStringTable(table);
					iter = m_englishStringTableMap.begin();
					continue;
				}
			}

			++iter;
		}
	}
}

//-----------------------------------------------------------------

void LocalizationManager::fetchStringTable    (const LocalizedStringTable * table)
{
	if (table == 0)
		return;

	++(table->m_referenceCount);
}

//-----------------------------------------------------------------

void LocalizationManager::releaseStringTable (const LocalizedStringTable * table)
{
	if (table == 0)
		return;

	if (--(table->m_referenceCount) == 0)
	{
		const StringTableMap_t::iterator find_iter = m_stringTableMap.find (table->getName ());

		//If we're using english it should be in m_stringTableMap
		assert ((find_iter != m_stringTableMap.end () && table == (*find_iter).second.second) || !m_usingEnglishLocale);//lint !e1924 // c-style cast.  MSVC bug

		// The re-check of m_referenceCount is because there can potentially be two tables
		// of the same name when loading with a locale other than English.  This can happen if an ID
		// doesn't exist in the locale specific string file, whether it does exist in the English string
		// file or not.
		if(find_iter != m_stringTableMap.end() &&
			table == (*find_iter).second.second)
		{
			delete (*find_iter).second.second;
			(*find_iter).second.second = 0;

			m_stringTableMap.erase (find_iter);
		}
		else if(!m_usingEnglishLocale)
		{
			const StringTableMap_t::iterator find_iter = m_englishStringTableMap.find (table->getName ());

			assert (find_iter != m_englishStringTableMap.end ());//lint !e1924 // c-style cast.  MSVC bug

			delete (*find_iter).second.second;
			(*find_iter).second.second = 0;

			m_englishStringTableMap.erase (find_iter);
		}
	}

}

//----------------------------------------------------------------------

LocalizationManager::StringValueCode LocalizationManager::getLocalizedStringValue (const StringId & id, Unicode::String & value, bool useEnglish)
{
	value.clear ();


	// This finds the English string table if it exists, so if the table isn't found then it won't be found.
	LocalizedStringTable * const table = fetchStringTable (id.getTable (), useEnglish);
	if (!table)
	{
		if (m_displayBadStringIds || s_displayStringIdInfo)
		{
			if (s_displayStringIdInfo) 
			{
				value += s_debugDisplayColor;
			}
			value.push_back ('[');
			value.append (Unicode::narrowToWide (id.getTable ()));
			value.push_back (']');
			value.push_back (':');
			value.append (Unicode::narrowToWide (id.getText ()));
		}
		else
		{
			createBadStringRepresentation (id, value);
		}

		if (s_debugStrings && m_debugBadStringsFunc)
		{
			m_debugBadStringsFunc (id, true);
		}

		return SVC_bad_table;
	}

	const LocalizedString * locstr = 0;

	if (id.getTextIndex () > 0)
		locstr = table->getLocalizedString (id.getTextIndex ());
	else
	{
		locstr = table->getLocalizedString (id.getText ());
		if (locstr)
			id.setTextIndex (locstr->getId ());
	}

	if (locstr)
	{
		// @todo: add any string substitution
		value = locstr->getString ();

		if (s_displayStringIdInfo)
		{
			value += s_debugDisplayColor;
			value.push_back ('[');
			value.append (Unicode::narrowToWide (id.getTable ()));
			value.push_back (']');
			value.push_back (':');
			value.append (Unicode::narrowToWide (id.getText ()));
		}

		releaseStringTable (table);
		return SVC_ok;
	}
	else
	{
		// The localized string table exists, but the id is not in it.  See if the id exists in the English version of the file.
		if(!useEnglish && !m_usingEnglishLocale)
		{
			LocalizationManager::StringValueCode stringValueCode = getLocalizedStringValue(id, value, true);
			if(stringValueCode == LocalizationManager::SVC_ok)
			{
				if (m_displayBadStringIds || s_displayStringIdInfo)
				{
					if (s_displayStringIdInfo) 
					{
						value += s_debugDisplayColor;
					}
					value.append (Unicode::narrowToWide (id.getTable ()));
					value.push_back (':');
					value.push_back ('[');
					value.append (Unicode::narrowToWide (id.getText ()));
					value.push_back (']');
				}

				releaseStringTable (table);
				return SVC_ok;
			}
		}

		if (m_displayBadStringIds || s_displayStringIdInfo)
		{
			if (s_displayStringIdInfo) 
			{
				value += s_debugDisplayColor;
			}
			value.append (Unicode::narrowToWide (id.getTable ()));
			value.push_back (':');
			value.push_back ('[');
			value.append (Unicode::narrowToWide (id.getText ()));
			value.push_back (']');
		}
		else
		{
			createBadStringRepresentation (id, value);
		}

		releaseStringTable (table);

		if (s_debugStrings && m_debugBadStringsFunc)
		{
			m_debugBadStringsFunc (id, false);
		}

		return SVC_bad_name;
	}
}

//----------------------------------------------------------------------

void LocalizationManager::debugDisplayStrings(bool const debugEnabled)
{
	s_displayStringIdInfo = debugEnabled;
}

//----------------------------------------------------------------------

void LocalizationManager::debugDisplayStringColor(Unicode::String const & colorString)
{
	s_debugDisplayColor = Unicode::narrowToWide(" ") + colorString;
}


// ======================================================================
