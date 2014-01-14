// ============================================================================
// 
// GameLanguageManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/GameLanguageManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedUtility/DataTable.h"
#include "UnicodeUtils.h"

#include <map>
#include <vector>

// ============================================================================
//
// GameLanguageManagerNamespace
//
// ============================================================================

namespace GameLanguageManagerNamespace
{
	typedef std::vector<std::string>            StringVector;

	struct LanguageInfo
	{
		LanguageInfo();

		Unicode::String m_localizedName;
		StringVector    m_alphabet;
		int             m_id;
		bool            m_audible;
		std::string     m_speakSkillModName;
		std::string     m_comprehendSkillModName;
		std::string     m_stringIdName;
	};

	typedef std::map<std::string, LanguageInfo> GameLanguages;

	bool          s_installed = false;
	unsigned int  s_minimumAbbreviationLength = 5;
	GameLanguages s_gameLanguages;
}

using namespace GameLanguageManagerNamespace;

//-----------------------------------------------------------------------------
GameLanguageManagerNamespace::LanguageInfo::LanguageInfo()
 : m_localizedName()
 , m_alphabet()
 , m_id(0) // basic language
{
}

// ============================================================================
//
// GameLanguageManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void GameLanguageManager::install()
{
	InstallTimer const installTimer("GameLanguageManager::install");

	DEBUG_FATAL(s_installed,("GameLanguageManager already installed.\n"));
	s_installed = true;
	ExitChain::add(remove,"GameLanguageManager::remove");

	// Languages

	char const gameLanguageFile[] = "datatables/game_language/game_language.iff";
	Iff gameLanguageIff;

	if (gameLanguageIff.open(gameLanguageFile, true))
	{
		DataTable dataTable;

		dataTable.load(gameLanguageIff);

		int const rowCount = dataTable.getNumRows();

		s_gameLanguages.clear();

		// Add all the words to the list in lowercase

		int languageId = 1;

		for (int index = 0; index < rowCount; ++index)
		{
			LanguageInfo languageInfo;

			languageInfo.m_alphabet.reserve(26);
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("a", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("b", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("c", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("d", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("e", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("f", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("g", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("h", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("h", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("j", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("k", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("l", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("m", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("n", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("o", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("p", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("q", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("r", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("s", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("t", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("u", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("v", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("w", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("x", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("y", index));
			languageInfo.m_alphabet.push_back(dataTable.getStringValue("z", index));
			languageInfo.m_stringIdName = Unicode::toLower(dataTable.getStringValue("stringId", index));
			languageInfo.m_localizedName = StringId("game_language", languageInfo.m_stringIdName).localize();
			languageInfo.m_id = languageId;
			languageInfo.m_audible = (dataTable.getIntValue("audible", index) != 0);
			languageInfo.m_speakSkillModName = Unicode::toLower(dataTable.getStringValue("speakSkillModName", index));
			languageInfo.m_comprehendSkillModName = Unicode::toLower(dataTable.getStringValue("comprehendSkillModName", index));

			s_gameLanguages.insert(std::make_pair(languageInfo.m_stringIdName, languageInfo));
			++languageId;
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load the species language data table: %s", gameLanguageFile));
	}
}

// ----------------------------------------------------------------------------
void GameLanguageManager::remove()
{
	s_installed = false;
}

//-----------------------------------------------------------------------------
void GameLanguageManager::getLanguages(StringVector &languages)
{
	languages.clear();
	languages.reserve(s_gameLanguages.size());

	GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.begin();

	for (; iterGameLanguages != s_gameLanguages.end(); ++iterGameLanguages)
	{
		std::string const &gameLanguage = iterGameLanguages->first;

		languages.push_back(gameLanguage);
	}
}

//-----------------------------------------------------------------------------
bool GameLanguageManager::isLanguageValid(std::string const &languageName)
{
	bool result = false;
	GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.find(languageName);

	if (iterGameLanguages != s_gameLanguages.end())
	{
		result = true;
	}

	return result;
}

//-----------------------------------------------------------------------------
bool GameLanguageManager::isLanguageAbbreviationValid(Unicode::String const &abbreviation, uint32 &language)
{
	bool result = false;
	GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.begin();

	for (; iterGameLanguages != s_gameLanguages.end(); ++iterGameLanguages)
	{
		std::string lowerAbbreviation(Unicode::wideToNarrow(Unicode::toLower(abbreviation)));

		// See if the first three characters match up

		if (((lowerAbbreviation.size() >= s_minimumAbbreviationLength) ||
		     (lowerAbbreviation.size() == iterGameLanguages->second.m_stringIdName.size())) &&
		    (lowerAbbreviation.size() <= iterGameLanguages->second.m_stringIdName.size()))
		{
			result = true;

			// Check the characters of the abbreviation vs the actual name

			for (unsigned int i = 0; i < lowerAbbreviation.size(); ++i)
			{
				if (lowerAbbreviation[i] != iterGameLanguages->second.m_stringIdName[i])
				{
					result = false;
					break;
				}
			}

			if (result)
			{
				language = static_cast<uint32>(iterGameLanguages->second.m_id);
				break;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
bool GameLanguageManager::isLanguageValid(int const languageId)
{
	bool result = false;
	GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.begin();

	for (; iterGameLanguages != s_gameLanguages.end(); ++iterGameLanguages)
	{
		if (iterGameLanguages->second.m_id == languageId)
		{
			result = true;
			break;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void GameLanguageManager::getLanguageSpeakSkillModName(int const languageId, std::string &skillModName)
{
	skillModName.clear();

	std::string languageName;
	getLanguageName(languageId, languageName);

	GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.find(languageName);

	if (iterGameLanguages != s_gameLanguages.end())
	{
		skillModName = iterGameLanguages->second.m_speakSkillModName;
	}
}

//-----------------------------------------------------------------------------
void GameLanguageManager::getLanguageComprehendSkillModName(int const languageId, std::string &skillModName)
{
	skillModName.clear();

	std::string languageName;
	getLanguageName(languageId, languageName);

	GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.find(languageName);

	if (iterGameLanguages != s_gameLanguages.end())
	{
		skillModName = iterGameLanguages->second.m_comprehendSkillModName;
	}
}

//-----------------------------------------------------------------------------
int GameLanguageManager::getLanguageId(std::string const &language)
{
	int result = 0;

	GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.find(language);

	if (iterGameLanguages != s_gameLanguages.end())
	{
		result = iterGameLanguages->second.m_id;
	}

	return result;
}

//-----------------------------------------------------------------------------
bool GameLanguageManager::isLanguageAudible(int const languageId)
{
	bool result = false;

	if (languageId > 0)
	{
		GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.begin();

		for (; iterGameLanguages != s_gameLanguages.end(); ++iterGameLanguages)
		{
			if (iterGameLanguages->second.m_id == languageId)
			{
				result = iterGameLanguages->second.m_audible;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void GameLanguageManager::convertToLanguage(Unicode::String const &source, int const languageId, Unicode::String &destination)
{
	if (languageId > 0)
	{
		GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.begin();

		for (; iterGameLanguages != s_gameLanguages.end(); ++iterGameLanguages)
		{
			if (iterGameLanguages->second.m_id == languageId)
			{
				destination.clear();

				// Found the destination language, convert the string

				std::string result;
				std::string narrowString(Unicode::wideToNarrow(source));
				int const narrowStringSize = static_cast<int>(narrowString.size());
				
				for (int i = 0; i < narrowStringSize; ++i)
				{
					char const character = narrowString[i];
					int const adjustedCharacter = static_cast<int>((character < 97) ? (character + 32) : character) - 97;
					int const alphabetSize = static_cast<int>(iterGameLanguages->second.m_alphabet.size());

					if ((alphabetSize == 26) &&
						(adjustedCharacter >= 0) &&
						(adjustedCharacter < alphabetSize) &&
						!iterGameLanguages->second.m_alphabet[adjustedCharacter].empty())
					{
						if (character >= 97)
						{
							// lowercase

							result += iterGameLanguages->second.m_alphabet[adjustedCharacter];
						}
						else
						{
							// uppercase

							result += Unicode::toUpper(iterGameLanguages->second.m_alphabet[adjustedCharacter]);
						}
					}
					else
					{
						result += narrowString[i];
					}
				}

				destination = Unicode::narrowToWide(result);

				break;
			}
		}
	}
	else
	{
		destination = source;
	}
}

//-----------------------------------------------------------------------------
void GameLanguageManager::getLanguageName(int const languageId, std::string &name)
{
	name.clear();
	GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.begin();

	for (; iterGameLanguages != s_gameLanguages.end(); ++iterGameLanguages)
	{
		if (iterGameLanguages->second.m_id == languageId)
		{
			name = iterGameLanguages->first;
			break;
		}
	}
}

//-----------------------------------------------------------------------------
int GameLanguageManager::getBasicLanguageId()
{
	return getLanguageId("basic");
}

//-----------------------------------------------------------------------------
void GameLanguageManager::getLocalizedLanguageName(int const languageId, Unicode::String &localizedLanguage)
{
	std::string name;

	getLanguageName(languageId, name);
	getLocalizedLanguageName(name, localizedLanguage);
}

//-----------------------------------------------------------------------------
void GameLanguageManager::getLocalizedLanguageName(std::string const &language, Unicode::String &localizedLanguage)
{
	localizedLanguage.clear();

	if (language.empty())
	{
		localizedLanguage = SharedStringIds::basic.localize();
	}
	else
	{
		GameLanguages::const_iterator iterGameLanguages = s_gameLanguages.find(language);

		if (iterGameLanguages != s_gameLanguages.end())
		{
			localizedLanguage = iterGameLanguages->second.m_localizedName;
		}
	}
}

//-----------------------------------------------------------------------------
int GameLanguageManager::getStartingLanguage(SharedCreatureObjectTemplate::Species const species)
{
	int result = getBasicLanguageId();

	// Wookies start with Shyriiwook

	if (species == SharedCreatureObjectTemplate::SP_wookiee)
	{
		result = getLanguageId("shyriiwook");
	}

	return result;
}

// ============================================================================
