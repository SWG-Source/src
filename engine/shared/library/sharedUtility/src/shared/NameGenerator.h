//
// FileName.h
// asommers 5-26-99
//
// copyright 1999, bootprint entertainment
//

#ifndef INCLUDED_NameGenerator_H
#define INCLUDED_NameGenerator_H

#include <vector>
#include <map>
#include <string>

class DataTable;

//====================================================================

class NameGenerator
{
public:
	NameGenerator(const std::string &language, const std::string &directory);
	virtual ~NameGenerator();

	virtual bool verifyName(const Unicode::String &name) const;
	virtual Unicode::String generateRandomName() const;

	bool getInitialized() const;
private:
	bool m_initialized;

	struct NameTemplate
	{
		float m_odds;
		float m_chanceProfession;
		int m_syllablesMin;
		int m_syllablesMax;
		int m_charsMin;
		int m_charsMax;
		float m_specialCharChance;
		std::string m_specialChars;
		int m_specialCharMax;
		float m_chanceBeginsSpecial;
		float m_chanceMedialSpecial;
		float m_chanceEndsSpecial;
		int m_maxSpecialSyllables;
	};
	std::vector<NameTemplate> m_nameTemplates;

	struct OddsValue
	{
		std::string value;
		float odds;
		std::string extra;
		bool operator <(const OddsValue &o) const { return value < o.value; }
		struct findFloat { bool operator ()(const OddsValue &o, float f) const { return o.odds < f; } };
		struct findString { bool operator ()(const OddsValue &o, const std::string &f) const { return o.value < f; } };
	};

	std::vector<OddsValue> m_initialConsonants;
	std::vector<OddsValue> m_medialConsonants;
	std::vector<OddsValue> m_finalConsonants;
	std::vector<OddsValue> m_vowels;
	std::vector<OddsValue> m_specialSyllables;
	std::vector<OddsValue> m_professionPatterns;
	std::map<char, std::vector<OddsValue> * > m_professions;

	std::string selectInitialConsonant(std::string * extra = 0) const;
	std::string selectMedialConsonant(std::string * extra = 0) const;
	std::string selectFinalConsonant(std::string * extra = 0) const;
	std::string selectVowel(std::string * extra = 0) const;
	std::string selectSpecialSyllable(std::string * extra = 0) const;
	std::string selectProfessionPattern(std::string * extra = 0) const;
	std::string selectProfession(char type, std::string * extra = 0) const;

	std::string generateSingleName(const NameTemplate &nt) const;
	std::string generateProfessionName(const NameTemplate &nt) const;
	std::string generateRandomName(const NameTemplate &nt) const;
	std::string capitalizeName(const std::string &name) const;

	int findRow(DataTable * table, const std::string &column, const std::string &value);
	void pushNameTemplate(DataTable * table, int row, const std::string &prefix);
	void balanceOdds(std::vector<OddsValue>::iterator begin, std::vector<OddsValue>::iterator end);
	std::string selectRandomFrom(std::vector<OddsValue>::const_iterator begin, std::vector<OddsValue>::const_iterator end, std::string * extra = 0) const;
	void loadColumn(std::vector<OddsValue> &result, DataTable * source, const std::string &i_language);
	void loadColumn(std::vector<OddsValue> &result, DataTable * source, const std::string &i_language, const std::string &filterColumn, const char maskChar);
	std::string appendSyllable(const std::string &left, const std::string &right) const;
	enum eFragmentType
	{
		kVowel,
		kConsonant,
		kSpecial,
		kMixed
	};
	eFragmentType getFragmentType(const std::string &test) const;
	bool hasString(const std::vector<OddsValue> &array, const std::string &test) const;

	// disable copying
	NameGenerator(const NameGenerator &o);
	NameGenerator &operator = (const NameGenerator &o);
}; 

//--------------------------------------------------------------------

inline std::string NameGenerator::selectInitialConsonant(std::string * extra) const
{
	return selectRandomFrom(m_initialConsonants.begin(), m_initialConsonants.end(), extra);
}

//--------------------------------------------------------------------

inline std::string NameGenerator::selectMedialConsonant(std::string * extra) const
{
	return selectRandomFrom(m_medialConsonants.begin(), m_medialConsonants.end(), extra);
}

//--------------------------------------------------------------------

inline std::string NameGenerator::selectFinalConsonant(std::string * extra) const
{
	return selectRandomFrom(m_finalConsonants.begin(), m_finalConsonants.end(), extra);
}

//--------------------------------------------------------------------

inline std::string NameGenerator::selectVowel(std::string * extra) const
{
	return selectRandomFrom(m_vowels.begin(), m_vowels.end(), extra);
}

//--------------------------------------------------------------------

inline std::string NameGenerator::selectSpecialSyllable(std::string * extra) const
{
	return selectRandomFrom(m_specialSyllables.begin(), m_specialSyllables.end(), extra);
}

//--------------------------------------------------------------------

inline std::string NameGenerator::selectProfessionPattern(std::string * extra) const
{
	return selectRandomFrom(m_professionPatterns.begin(), m_professionPatterns.end(), extra);
}

//--------------------------------------------------------------------

inline std::string NameGenerator::selectProfession(char type, std::string * extra) const
{
	std::map<char, std::vector<OddsValue> *>::const_iterator i = m_professions.find(type);
	if (i==m_professions.end())
		return std::string();

	return selectRandomFrom(i->second->begin(), i->second->end(), extra);
}

//--------------------------------------------------------------------

inline bool NameGenerator::getInitialized() const
{
	return m_initialized;
}

//====================================================================

#endif
