// ============================================================================
// 
// GameGameLanguageManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_GameLanguageManager_H
#define INCLUDED_GameLanguageManager_H

#include "sharedGame/SharedCreatureObjectTemplate.h"

//-----------------------------------------------------------------------------
class GameLanguageManager
{
public:

	typedef std::vector<std::string> StringVector;

	static void install();

public:

	static bool isLanguageValid(std::string const &languageName);
	static bool isLanguageValid(int const languageId);
	static bool isLanguageAbbreviationValid(Unicode::String const &abbreviation, uint32 &language);
	static bool isLanguageAudible(int const languageId);

	static void getLanguages(StringVector &languages);
	static void getLanguageName(int const languageId, std::string &name);
	static void getLocalizedLanguageName(int const languageId, Unicode::String &localizedLanguage);
	static void getLocalizedLanguageName(std::string const &language, Unicode::String &localizedLanguage);
	static void getLanguageSpeakSkillModName(int const languageId, std::string &skillModName);
	static void getLanguageComprehendSkillModName(int const languageId, std::string &skillModName);
	static int  getBasicLanguageId();
	static int  getStartingLanguage(SharedCreatureObjectTemplate::Species const species);
	static int  getLanguageId(std::string const &language);

	static void convertToLanguage(Unicode::String const &source, int const languageId, Unicode::String &destination);

private:

	static void remove();

	// Disable

	GameLanguageManager();
	~GameLanguageManager();
	GameLanguageManager(GameLanguageManager const &);
	GameLanguageManager &operator =(GameLanguageManager const &);
};

// ============================================================================

#endif // INCLUDED_GameLanguageManager_H
