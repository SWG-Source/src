//
// FileName.h
// asommers 5-26-99
//
// copyright 1999, bootprint entertainment
//

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/NameGenerator.h"

#include "sharedRandom/Random.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "UnicodeUtils.h"

#include <algorithm>
#include <set>
#include <stdio.h>

//====================================================================

NameGenerator::NameGenerator(const std::string &i_language, const std::string &directory)
{
	std::string datatablePath("datatables/" + directory + "/");
	m_initialized = false;

	if (i_language.empty())
		return;

	DataTable * vowelTable = DataTableManager::getTable(datatablePath+"vowels.iff", true);
	DataTable * consonantTable = DataTableManager::getTable(datatablePath+"consonants.iff", true);
	DataTable * specialTable = DataTableManager::getTable(datatablePath+"special.iff", true);
	DataTable * professionPatternTable = DataTableManager::getTable(datatablePath+"profession_patterns.iff", true);
	DataTable * professionTable = DataTableManager::getTable(datatablePath+"profession.iff", true);
	DataTable * ruleTable = DataTableManager::getTable(datatablePath+"rules.iff", true);

	if (!ruleTable || 
		!professionTable || 
		!professionPatternTable || 
		!specialTable ||
		!consonantTable ||
		!vowelTable)
	{
		WARNING_STRICT_FATAL(true, ("Couldn't open a naming data table with path %s language %s\n", datatablePath.c_str(), i_language.c_str()));
		return;
	}

	int ruleRow = findRow(ruleTable, "id", i_language);
	if (ruleRow < 0)
	{
		WARNING(true, ("Couldn't find language [%s] in rule table path %s\n", i_language.c_str(), datatablePath.c_str()));
		return;
	}

	pushNameTemplate(ruleTable, ruleRow, "fname_");
	pushNameTemplate(ruleTable, ruleRow, "lname_");

	loadColumn(m_initialConsonants, consonantTable, i_language, "placement", 'b');
	loadColumn(m_medialConsonants, consonantTable, i_language, "placement", 'm');
	loadColumn(m_finalConsonants, consonantTable, i_language, "placement", 'e');

	loadColumn(m_vowels, vowelTable, i_language);
	loadColumn(m_specialSyllables, specialTable, i_language);
	loadColumn(m_professionPatterns, professionPatternTable, i_language);

	// now extract all of the word types from profession patterns
	std::vector<OddsValue>::iterator ppi;
	bool inLiteral = false;
	for (ppi = m_professionPatterns.begin(); ppi != m_professionPatterns.end(); ++ppi)
	{
		const std::string &value = ppi->value;
		std::string::const_iterator c;
		for (c=value.begin(); c!=value.end(); ++c)
		{
			if (*c == '\\') // skip literals
			{
				inLiteral = true;
			}
			else if (*c == '*') // skip
			{
			}
			else
			{
				if (!inLiteral)
				{
					std::map<char, std::vector<OddsValue> *>::iterator wordIter = m_professions.find(*c);
					if (wordIter == m_professions.end())
					{
						m_professions[*c] = new std::vector<OddsValue>();
						loadColumn(*m_professions[*c], professionTable, i_language, "type", *c);
					}
				}
				inLiteral = false;
			}
		}
	}

	if (!m_nameTemplates.empty())
	{
		m_initialized = true;
	}
}

//--------------------------------------------------------------------

NameGenerator::~NameGenerator()
{
	std::map<char, std::vector<OddsValue> * >::iterator i;
	for (i=m_professions.begin(); i!=m_professions.end(); ++i)
	{
		delete i->second;
	}
	m_professions.clear();
}

//--------------------------------------------------------------------

Unicode::String NameGenerator::generateRandomName() const
{
	if (!m_initialized)
		return Unicode::String();

	std::vector<NameTemplate>::const_iterator i;
	bool first = true;
	std::string result;
	for (i=m_nameTemplates.begin(); i!=m_nameTemplates.end(); ++i)
	{
		if (Random::randomReal() < i->m_odds)
		{
			if (!first)
			{
				result += " ";
			}
			result += generateSingleName(*i);
			first = false;
		}
	}
	if (first)
	{
		result = generateSingleName(m_nameTemplates.front());
	}
	return Unicode::narrowToWide(capitalizeName(result));
}

//--------------------------------------------------------------------

int NameGenerator::findRow(DataTable * table, const std::string &column, const std::string &value)
{
	int i;
	for (i=0; i<table->getNumRows(); ++i)
	{
		if (value == table->getStringValue(column, i))
		{
			return i;
		}
	}
	return -1;
}

//--------------------------------------------------------------------

void NameGenerator::pushNameTemplate(DataTable * table, int row, const std::string &prefix)
{
	float odds = table->getIntValue(prefix+"chance", row) / 100.f; // don't convert this to *0.01f, we need to insure that int(100)/100.0f equals exactly 1.
	if (odds == 0)
		return;

	NameTemplate temp;
	temp.m_odds = odds;
	temp.m_chanceProfession = table->getIntValue(prefix+"chance_profession", row) * 0.01f;
	temp.m_syllablesMin = table->getIntValue(prefix+"syllables_min", row);
	temp.m_syllablesMax = table->getIntValue(prefix+"syllables_max", row);
	temp.m_charsMin = table->getIntValue(prefix+"chars_min", row);
	temp.m_charsMax = table->getIntValue(prefix+"chars_max", row);
	temp.m_specialCharChance = table->getIntValue(prefix+"special_char_chance", row) * 0.01f;
	temp.m_specialChars = table->getStringValue(prefix+"special_char", row);
	temp.m_specialCharMax = table->getIntValue(prefix+"special_char_max", row);
	temp.m_chanceBeginsSpecial = table->getIntValue(prefix+"chance_begins_special", row) * 0.01f;
	temp.m_chanceMedialSpecial = table->getIntValue(prefix+"chance_medial_special", row) * 0.01f;
	temp.m_chanceEndsSpecial = table->getIntValue(prefix+"chance_ends_special", row) * 0.01f;
	temp.m_maxSpecialSyllables = table->getIntValue(prefix+"max_special_syllables", row);

	m_nameTemplates.push_back(temp);
}

//--------------------------------------------------------------------

std::string NameGenerator::generateSingleName(const NameTemplate &nt) const
{
	if (Random::randomReal() < nt.m_chanceProfession)
		return generateProfessionName(nt);
	else
		return generateRandomName(nt);
}

//--------------------------------------------------------------------

std::string NameGenerator::generateRandomName(const NameTemplate &nt) const
{
	for (;;)
	{
		int syllables = nt.m_syllablesMin;
		while (syllables < nt.m_syllablesMax)
		{
			float syllableChance = 0.6f - syllables / (nt.m_syllablesMax*2.0f);
			if (Random::randomReal() > syllableChance)
				break;
			++syllables;
		}

		bool nextSyllableIsFirst = true;
		std::string result;
		int specialCharCount = 0;
		std::set<std::string> selectedSpecialSyllables;
		int i;
		for (i=0; i<syllables; ++i)
		{
			std::string oldResult(result);
			bool firstSyllable = nextSyllableIsFirst;
			nextSyllableIsFirst = false;
			bool finalSyllable = (i == syllables - 1);
			bool appendSpecial = false;
			if (!finalSyllable)
			{
				if (specialCharCount < nt.m_specialCharMax && Random::randomReal() < nt.m_specialCharChance)
				{
					appendSpecial = true;
					finalSyllable = true;
					nextSyllableIsFirst = true;
				}
			}

			float specialSyllableChance = nt.m_chanceMedialSpecial;
			if (firstSyllable)
				specialSyllableChance = nt.m_chanceBeginsSpecial;
			else if (finalSyllable)
				specialSyllableChance = nt.m_chanceEndsSpecial;
			if (Random::randomReal() < specialSyllableChance && (int)selectedSpecialSyllables.size() < nt.m_maxSpecialSyllables)
			{
				for (;;)
				{
					std::string specialSyllable = selectSpecialSyllable();
					if (selectedSpecialSyllables.find(specialSyllable) != selectedSpecialSyllables.end())
						continue;
					result = appendSyllable(result, specialSyllable);
					selectedSpecialSyllables.insert(specialSyllable);
					break;
				}
			}
			else
			{
				std::string syllablePattern;
				float syllableSelect = Random::randomReal();
				if (syllableSelect < 0.40f)
				{
					syllablePattern = "cv";
				}
				else if (syllableSelect < 0.80f)
				{
					syllablePattern = "vc";
				}
				else
				{
					syllablePattern = "v";
				}
				std::string::const_iterator syllIter;
				std::string syllable;
				for (syllIter = syllablePattern.begin(); syllIter != syllablePattern.end(); ++syllIter)
				{
					if (*syllIter == 'c')
					{
						if (firstSyllable && syllIter == syllablePattern.begin())
						{
							syllable += selectInitialConsonant();
						}
						else if (finalSyllable && (syllIter+1) == syllablePattern.end())
						{
							syllable += selectFinalConsonant();
						}
						else
						{
							syllable += selectMedialConsonant();
						}
					}
					else
					{
						syllable += selectVowel();
					}
				}
				result = appendSyllable(result, syllable);
			}
			if (result == oldResult)
				--i; // it's still alright to append because if the endings matched, the old ending must have been a valid final consonant.
			// append a special character
			if (appendSpecial && !result.empty())
			{
				int s = nt.m_specialChars.size();
				char toInsert = nt.m_specialChars[Random::random(s-1)];
				result = appendSyllable(result, std::string(1, toInsert));
				++specialCharCount;
			}
		}
		// now do sanity checks
		// retry if the length is bad
		int resultSize = static_cast<int>(result.size());
		if (resultSize < nt.m_charsMin)
			continue;
		else if (resultSize > nt.m_charsMax)
			continue;

		// continue if we have a lonely special syllable
		if (getFragmentType(result) == kSpecial)
			continue;

		// now capitalize
		return result;
	}
}


//--------------------------------------------------------------------

std::string NameGenerator::generateProfessionName(const NameTemplate &nt) const
{
	std::string professionPattern = selectProfessionPattern();
	if (professionPattern.empty())
		return std::string();

	std::set<std::string> usedRoots;
	std::string result;

	std::string::const_iterator i;
	bool append = false;
	for (i=professionPattern.begin(); i!=professionPattern.end(); ++i)
	{
		if (*i == '\\')
		{
			++i;
			if (i != professionPattern.end())
			{
				result += *i;
			}
		}
		else if (*i == '*')
		{
			result = appendSyllable(result, generateRandomName(nt));
		}
		else
		{
			for (;;)
			{
				std::string profession, root;
				profession = selectProfession(*i, &root);
				if (!root.empty() && usedRoots.find(root) != usedRoots.end())
					continue;
				if (append)
				{
					result = appendSyllable(result, profession);
					append = false;
				}
				else
				{
					result += profession;
				}
				usedRoots.insert(root);
				break;
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------

void NameGenerator::balanceOdds(std::vector<OddsValue>::iterator begin, std::vector<OddsValue>::iterator end)
{
	std::sort(begin, end);

	std::vector<OddsValue>::iterator i;
	float total = 0;
	for (i=begin; i!=end; ++i)
		total += i->odds;

	float accum = 0;
	for (i=begin; i!=end; ++i)
	{
		float odds = i->odds / total;
		accum += odds;
		i->odds = accum;
	}
	// correct rounding errors
	if (end > begin)
		(end-1)->odds = 1.0f;
}

//--------------------------------------------------------------------

std::string NameGenerator::selectRandomFrom(std::vector<OddsValue>::const_iterator begin, std::vector<OddsValue>::const_iterator end, std::string * extra) const
{
	if (begin == end)
	{
		if (extra)
			*extra = std::string();
		return std::string();
	}

	float rr = Random::randomReal();
	FATAL(rr >= 1.0f, ("randomReal returned an invalid random number %g\n", rr));

	std::vector<OddsValue>::const_iterator i = std::lower_bound(begin, end, rr, OddsValue::findFloat());
	/*
	std::vector<OddsValue>::const_iterator i;
	for (i=begin; i!=end; ++i)
	{
		if (rr < i->odds)
		{
			if (extra)
				*extra = i->extra;
			return i->value;
		}
		rr -= i->odds;
	}
	// just in case of rounding errors
	return (end-1)->value;
	*/
	if (extra)
		*extra = i->extra;
	return i->value;
}

//--------------------------------------------------------------------

std::string NameGenerator::capitalizeName(const std::string &result) const
{
	std::string capitalizedResult;
	std::string::const_iterator capIter;
	bool capNext = true;
	for (capIter = result.begin(); capIter != result.end(); ++capIter)
	{
		if (capNext)
		{
			capitalizedResult += (char)toupper(*capIter);
		}
		else
		{
			capitalizedResult += *capIter;
		}
		if (*capIter == ' ')
		{
			capNext = true;
		}
		else if (*capIter == '\'' || *capIter == '-')
		{
			capNext = Random::randomReal() > 0.5f;
		}
		else
		{
			capNext = false;
		}
	}
	return capitalizedResult;
}

//--------------------------------------------------------------------

void NameGenerator::loadColumn(std::vector<OddsValue> &result, DataTable * source, const std::string &i_language)
{
	int rows = source->getNumRows();
	int languageColumnIndex = source->findColumnNumber(i_language);

	int i;
	for (i=0; i<rows; ++i)
	{
		std::string value = source->getStringValue(0, i);

		int odds = source->getIntValue(languageColumnIndex, i);
		if (odds == 0)
			continue;

		OddsValue temp;
		temp.value = value;
		temp.odds = (float)odds;
		result.push_back(temp);
	}

	balanceOdds(result.begin(), result.end());
}

//--------------------------------------------------------------------

void NameGenerator::loadColumn(std::vector<OddsValue> &result, DataTable * source, const std::string &i_language, const std::string &maskColumn, const char maskChar)
{
	int rows = source->getNumRows();
	int maskColumnIndex = source->findColumnNumber(maskColumn);
	int languageColumnIndex = source->findColumnNumber(i_language);

	int i;
	for (i=0; i<rows; ++i)
	{
		std::string value = source->getStringValue(0, i);

		int odds = source->getIntValue(languageColumnIndex, i);
		if (odds == 0)
			continue;

		std::string mask = source->getStringValue(maskColumnIndex, i);
		if (mask.find(maskChar) == std::string::npos)
			continue;

		OddsValue temp;
		temp.value = value;
		temp.odds = (float)odds;
		if (maskColumnIndex >= 2)
		{
			temp.extra = source->getStringValue(1, i);
		}
		result.push_back(temp);
	}

	balanceOdds(result.begin(), result.end());
}

//--------------------------------------------------------------------

std::string NameGenerator::appendSyllable(const std::string &left, const std::string &right) const
{
	if (left.empty())
		return right;

	// now we try to merge/simplify repeated letters.  If there are too many repeats, we insert
	// a remedial vowel or consonant to try and make the word more pronounceable

	// grab the longest ending vowel or consonant string from left
	std::string::const_iterator l = left.end();
	eFragmentType leftFragmentType = kMixed;
	std::string leftFragment;
	while (l != left.begin())
	{
		--l;
		std::string leftEnd(l, left.end());
		eFragmentType fragment = getFragmentType(leftEnd);
		if (fragment == kMixed || fragment == kSpecial)
			break;
		leftFragment = leftEnd;
		leftFragmentType = fragment;
	}

	eFragmentType rightFragmentType = kMixed;
	std::string rightFragment;
	std::string::const_iterator r = right.begin();
	while (r != right.end())
	{
		++r;
		std::string rightBegin(right.begin(), r);
		eFragmentType fragment = getFragmentType(rightBegin);
		if (fragment == kMixed || fragment == kSpecial)
			break;
		rightFragment = rightBegin;
		rightFragmentType = fragment;
	}

	if (leftFragmentType != rightFragmentType)
		return left + right;

	if (leftFragment == rightFragment)
		return left;

	if (leftFragmentType == kConsonant)
		return left + selectVowel() + right;
	else
		return left + selectMedialConsonant() + right;
}

//--------------------------------------------------------------------

NameGenerator::eFragmentType NameGenerator::getFragmentType(const std::string &test) const
{
	if (hasString(m_vowels, test))
		return kVowel;
	if (hasString(m_initialConsonants, test))
		return kConsonant;
	if (hasString(m_medialConsonants, test))
		return kConsonant;
	if (hasString(m_finalConsonants, test))
		return kConsonant;
	if (hasString(m_specialSyllables, test))
		return kSpecial;
	return kMixed;
}

//--------------------------------------------------------------------

bool NameGenerator::hasString(const std::vector<OddsValue> &array, const std::string &test) const
{
	std::vector<OddsValue>::const_iterator i;
	i = std::lower_bound(array.begin(), array.end(), test, OddsValue::findString());
	if (i == array.end())
		return false;
	if (i->value != test)
		return false;
	return true;
}

//--------------------------------------------------------------------

bool NameGenerator::verifyName(const Unicode::String &name) const
{
	size_t minNameCount = 0;
	std::vector<NameTemplate>::const_iterator i;
	for (i = m_nameTemplates.begin(); i!=m_nameTemplates.end(); ++i)
	{
		if (i->m_odds >= 1.0f)
		{
			++minNameCount;
		}
	}

	int pos=0;
	size_t nameindex = 0;
	for (;;)
	{
		if (nameindex >= m_nameTemplates.size())
			return false;

		int newpos = name.find(' ', pos);
		int namelength = newpos - pos;
		if (newpos == int(Unicode::String::npos))
		{
			namelength = name.size() - pos;
		}
		if (namelength < m_nameTemplates[nameindex].m_charsMin)
			return false;
		if (namelength > m_nameTemplates[nameindex].m_charsMax)
			return false;
		pos = newpos + 1;
		++nameindex;
		if (newpos == int(Unicode::String::npos))
		{
			if (nameindex < minNameCount)
				return false;
			else
				return true;
		}
	}
	return true; // just in case, should always return from the newpos == npos test
}

//====================================================================
