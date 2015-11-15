// ============================================================================
//
// TextManager.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/TextManager.h"

#include "LocalizationManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/TextIterator.h"
#include "sharedMath/PackedRgb.h"
#include "sharedUtility/DataTable.h"
#include "UnicodeUtils.h"
#include "utf8.h"

#include <map>
#include <vector>

// ============================================================================
//
// TextManagerNamespace
//
// ============================================================================

// ----------------------------------------------------------------------------
namespace TextManagerNamespace
{
	typedef std::map<Unicode::String, bool> CussWords;         // string/allow sub-string match
	typedef std::vector<Unicode::String>    UnicodeStringVector;

	bool      s_installed = false;
	CussWords s_cussWords;

	void getAppropriateWord(Unicode::String &text);
	void tokenize(const Unicode::String &source, UnicodeStringVector &target);
	void getFilterLetter(Unicode::String &filterLetter);
	bool isExactMatch(Unicode::String const &text);
	bool isAlpha(Unicode::unicode_char_t const character);
	void checkText(Unicode::String &text, int const startIndex, int const endIndex);
}

using namespace TextManagerNamespace;

// ----------------------------------------------------------------------------
bool TextManagerNamespace::isExactMatch(Unicode::String const &text)
{
	bool result = false;
	Unicode::String lowerText(Unicode::toLower(text));
	CussWords::iterator iterCussWords = s_cussWords.find(lowerText);

	if (iterCussWords != s_cussWords.end())
	{
		result = true;
	}

	return result;
}

// ----------------------------------------------------------------------------
void TextManagerNamespace::getAppropriateWord(Unicode::String &text)
{
	if (isExactMatch(text))
	{
		// An exact word match was found, plug the bad word with symbols

		unsigned int const letterCount = text.size();
		Unicode::String filterLetter;

		// Found a bad word, replace the word

		for (unsigned int i = 0; i < letterCount; ++i)
		{
			getFilterLetter(filterLetter);

			text[i] = filterLetter[0];
		}
	}
	else
	{
		// See if we can find a non-exact match, but a match by removing
		// all non-alphabet letters and condensing the result into a word

		Unicode::String alphabetString;
		alphabetString.reserve(text.size());
		int index = 0;

		for (unsigned int i = 0; i < text.size(); ++i)
		{
			if (isAlpha(text[i]))
			{
				alphabetString.push_back(text[i]);
				++index;
			}
		}

		if (isExactMatch(alphabetString))
		{
			// Now we found an exact match so plug all the alpha letters

			Unicode::String filterLetter;

			for (unsigned int i = 0; i < text.size(); ++i)
			{
				if (isAlpha(text[i]))
				{
					getFilterLetter(filterLetter);

					text[i] = filterLetter[0];
				}
			}
		}
		else
		{
			// Sub-string search time for words that are listed for sub-string searches

			Unicode::String lowerText(Unicode::toLower(text));
			unsigned int findStartPosition = 0;

			for (;;)
			{
				bool done = true;
				CussWords::iterator iterCussWords = s_cussWords.begin();

				for (; iterCussWords != s_cussWords.end(); ++iterCussWords)
				{
					Unicode::String const &cussWord = iterCussWords->first;
					bool const allowSubStringSearch = iterCussWords->second;

					if (!allowSubStringSearch)
					{
						continue;
					}

					unsigned int index = lowerText.find(cussWord, findStartPosition);

					if (index != Unicode::String::npos)
					{
						findStartPosition = index;

						// Found a bad word, replace the word

						unsigned int const letterCount = cussWord.size();
						Unicode::String filterLetter;

						for (unsigned int i = 0; i < letterCount; ++i)
						{
							getFilterLetter(filterLetter);

							text[index + i] = filterLetter[0];
							++findStartPosition;
						}

						// We are not done with a clean run so start over in case
						// there are multiple cuss words in the same string

						done = false;
						break;
					}
				}

				if (done)
				{
					break;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
void TextManagerNamespace::getFilterLetter(Unicode::String &filterLetter)
{
	int number = rand() % 7;

	switch (number)
	{
		default:
		case 1: { filterLetter = Unicode::narrowToWide("@"); } break;
		case 2: { filterLetter = Unicode::narrowToWide("#"); } break;
		case 3: { filterLetter = Unicode::narrowToWide("$"); } break;
		case 4: { filterLetter = Unicode::narrowToWide("%"); } break;
		case 5: { filterLetter = Unicode::narrowToWide("&"); } break;
		case 6: { filterLetter = Unicode::narrowToWide("*"); } break;
	}
}

// ----------------------------------------------------------------------------
void TextManagerNamespace::tokenize(const Unicode::String &source, UnicodeStringVector &target)
{
	UnicodeStringVector unicodeStringVector;

	IGNORE_RETURN(Unicode::tokenize(source, unicodeStringVector));

	target = unicodeStringVector;
}

// ----------------------------------------------------------------------------
bool TextManagerNamespace::isAlpha(Unicode::unicode_char_t const character)
{
	bool result = false;

	if (   (character <= 255)
		&& isalpha(character))
	{
		result = true;
	}

	return result;
}

// ----------------------------------------------------------------------------
void TextManagerNamespace::checkText(Unicode::String &text, int const startIndex, int const endIndex)
{
	int const length = endIndex - startIndex;
	Unicode::String subString(text.substr(startIndex, length));

	getAppropriateWord(subString);

	// Put the fixed text back into the string

	for (int index = 0; index < (endIndex - startIndex); ++index)
	{
		text[startIndex + index] = subString[index];
	}
}

// ============================================================================
//
// TextManager
//
// ============================================================================

// ----------------------------------------------------------------------------
void TextManager::install()
{
	InstallTimer const installTimer("TextManager::install");

	DEBUG_FATAL(s_installed,("TextManager already installed.\n"));

	// Cuss words

	char const profanityFile[] = "datatables/chat/profanity_filter.iff";
	Iff profanityFilterIff;

	if (profanityFilterIff.open(profanityFile, true))
	{
		DataTable dataTable;

		dataTable.load(profanityFilterIff);

		int const rowCount = dataTable.getNumRows();

		s_cussWords.clear();

		// Add all the words to the list in lowercase

		for (int index = 0; index < rowCount; ++index)
		{
			std::string const &word = dataTable.getStringValue(0, index);
			bool const allowSubStringMatch = (dataTable.getIntValue(1, index) != 0);

			Unicode::UTF16 wordBuf[50];

			Unicode::UTF8_convertToUTF16(const_cast<char *>(word.c_str()), wordBuf, sizeof(wordBuf)/sizeof(wordBuf[0]));

			Unicode::String const &unicodeWord = Unicode::String(wordBuf);

			s_cussWords.insert(std::make_pair(Unicode::toLower(unicodeWord), allowSubStringMatch));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load the profanity filter data table: %s", profanityFile));
	}

	ExitChain::add(remove,"TextManager::remove");
	s_installed = true;
}

// ----------------------------------------------------------------------------
void TextManager::remove()
{
	s_cussWords.clear();

	s_installed = false;
}

// Filter the text for any profanity.
// ----------------------------------------------------------------------------
Unicode::String TextManager::filterText(Unicode::String const &text)
{
	Unicode::String result(text);
	Unicode::String::const_iterator iterText = text.begin();
	int startIndex = Unicode::String::npos;
	int currentIndex = 0;

	{
		// Walk through the text finding sections of alpha characters.
		// Each set of alpha characters is checked for cuss words, all
		// non-alphabet is skipped and preserved.

		for (; iterText != text.end(); ++iterText)
		{
			Unicode::unicode_char_t const character = (*iterText);

			if (   (static_cast<unsigned>(startIndex) == Unicode::String::npos)
			    && isAlpha(character))
			{
				startIndex = currentIndex;
			}
			else if (   (static_cast<unsigned>(startIndex) != Unicode::String::npos)
				     && !isAlpha(character))
			{
				checkText(result, startIndex, currentIndex);

				startIndex = Unicode::String::npos;
			}

			++currentIndex;
		}

		// Possibly check the last word

		if (static_cast<unsigned>(startIndex) != Unicode::String::npos)
		{
			checkText(result, startIndex, currentIndex);
		}
	}

	{
		// Walk through the text finding sections between spaces.
		// Each set of text between spaces is checked for cuss words,
		// all non-alphabet is skipped and preserved.

		iterText = text.begin();
		startIndex = Unicode::String::npos;
		currentIndex = 0;

		for (; iterText != text.end(); ++iterText)
		{
			Unicode::unicode_char_t const character = (*iterText);

			if (   (static_cast<unsigned>(startIndex) == Unicode::String::npos)
			    && character != static_cast<Unicode::unicode_char_t>(' '))
			{
				startIndex = currentIndex;
			}
			else if (   (static_cast<unsigned>(startIndex) != Unicode::String::npos)
			         && character == static_cast<Unicode::unicode_char_t>(' '))
			{
				checkText(result, startIndex, currentIndex);

				startIndex = Unicode::String::npos;
			}

			++currentIndex;
		}

		// Possibly check the last word

		if (static_cast<unsigned>(startIndex) != Unicode::String::npos)
		{
			checkText(result, startIndex, currentIndex);
		}
	}

	return result;
}

// ----------------------------------------------------------------------------
bool TextManager::isAppropriateText(Unicode::String const &text)
{
	bool result = false;
	Unicode::String filteredText(filterText(text));

	// If the text looks clean so far, check the text again with color
	// codes removed.

	if (filteredText == text)
	{
		result = true;

		TextIterator textIterator(filteredText);
		Unicode::String colorCodeRemovedText(textIterator.getPrintableText());

		// Make sure there was some color codes removed before we waste time
		// trying to filter it

		if (filteredText.size() != colorCodeRemovedText.size())
		{
			Unicode::String colorCodeRemovedFilteredText(filterText(colorCodeRemovedText));

			if (colorCodeRemovedText != colorCodeRemovedFilteredText)
			{
				result = false;
			}
		}
	}

	return result;
}

// ============================================================================
