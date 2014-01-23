// ======================================================================
//
// LocalizationManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocalizationManager_H
#define INCLUDED_LocalizationManager_H


#if WIN32
// stl warning func not inlined
#pragma warning (disable:4710)
// unref inline func removed
#pragma warning (disable:4514)
// symbol name too long
#pragma warning (disable:4786)
#endif

#include <unordered_map>
#include "Unicode.h"
#include "UnicodeUtils.h"

class LocalizedStringTable;
class LocalizedString;
class AbstractFileFactory;
class StringId;

// ======================================================================

/**
* A LocalizationManager represents a collection of LocalizedStringTables for a single locale.
* This can be used as a singleton, or with multiple instances in a larger collection.
*/

class LocalizationManager
{
public:

	// return values from getLocalizedStringValue
	enum StringValueCode
	{
		SVC_ok,
		SVC_bad_table,
		SVC_bad_name
	};

	typedef void (*DebugBadStringsFunc)    (const StringId &, bool);
	
public:

	                              LocalizationManager     (AbstractFileFactory * fileFactory, const Unicode::NarrowString & localeName, DebugBadStringsFunc debugBadStringsFunc = 0, bool displayBadStringIds = true);
	static LocalizationManager &  getManager              ();
	static LocalizationManager &  getManager              (Unicode::NarrowString locale);

	                             ~LocalizationManager     ();

	typedef std::pair<time_t, LocalizedStringTable *> TimedStringTable;
	typedef std::unordered_map<Unicode::NarrowString, TimedStringTable> StringTableMap_t;
	typedef std::unordered_map<Unicode::NarrowString, LocalizationManager *> LocalizationManagerHashMap;

	static void                   install                 (AbstractFileFactory * fileFactory, Unicode::UnicodeNarrowStringVector & localeNames, bool debugStrings, DebugBadStringsFunc debugBadStringsFunc = 0, bool displayBadStringIds = true);
	static void                   remove                  ();

	LocalizedStringTable *        fetchStringTable        (const Unicode::NarrowString & name);
	void                          fetchStringTable        (const LocalizedStringTable * table);

	void                          releaseStringTable      (const LocalizedStringTable * table);

	const Unicode::NarrowString & getLocaleName           ();

	void                          setLocaleName           (const Unicode::NarrowString & name);

	const StringTableMap_t &      getTableMap             () const;

	StringValueCode               getLocalizedStringValue (const StringId & id, Unicode::String & value, bool useEnglish = false);

	void                          purgeUnusedStringTables ();

	void                          garbageCollectUnused    (int timeoutThresholdSecs = 20);

	static void debugDisplayStrings(bool const debugEnabled);
	static void debugDisplayStringColor(Unicode::String const & colorString);

private:
	                              LocalizationManager ();
	                              LocalizationManager (const LocalizationManager & rhs);
	LocalizationManager &         operator=    (const LocalizationManager & rhs);

	void                          preload                 (const Unicode::NarrowString & name);
	void                          releasePreloadedAssets  ();

	LocalizedStringTable *        fetchStringTable        (const Unicode::NarrowString & name, bool forceUseEnglish);
	
	static bool                   ms_installed;
	static LocalizationManagerHashMap * ms_singletonHashMap;
	static Unicode::NarrowString * ms_firstLocaleLoaded;
	

	AbstractFileFactory *         m_fileFactory;

	StringTableMap_t              m_stringTableMap;
	StringTableMap_t              m_englishStringTableMap;
	Unicode::NarrowString         m_localeName;
	DebugBadStringsFunc           m_debugBadStringsFunc;
	bool                          m_displayBadStringIds;

	bool                          m_usingEnglishLocale;
};

//-----------------------------------------------------------------

/**
* Get the short name of the locale.  E.g. "english", "japanese", etc.
* The locale name affects the path the LocalizationManager searches in,
* and should represent the paths present in the AbstractFileFactory
*/

inline const Unicode::NarrowString & LocalizationManager::getLocaleName ()
{
	return m_localeName;
}

//-----------------------------------------------------------------

/**
* Set the short name of the locale.  E.g. "english", "japanese", etc.
* The locale name affects the path the LocalizationManager searches in,
* and should represent the paths present in the AbstractFileFactory
*/

inline void LocalizationManager::setLocaleName (const Unicode::NarrowString & name)
{
	m_localeName = name;
}

//-----------------------------------------------------------------

/**
* Get a const ref to the table name -> table mapping
*/

inline const LocalizationManager::StringTableMap_t &  LocalizationManager::getTableMap () const
{
	return m_stringTableMap;
}

// ======================================================================

#endif
