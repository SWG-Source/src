// ======================================================================
//
// RegexList.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/RegexList.h"

#include "StringId.h"
#include "Unicode.h"
#include "UnicodeUtils.h"
#include "pcre.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedRegex/RegexServices.h"
#include "sharedUtility/DataTable.h"

#include <algorithm>

// ======================================================================

class RegexList::Entry
{
public:

	Entry(char const *pattern, bool matchWords, char const *reason);
	~Entry();

	pcre const        *getCompiledRegex() const;
	bool               getMatchWords() const;
	std::string const &getReason() const;

private:

	pcre                         *m_pcre;
	bool                          m_matchWords;
	std::string                   m_reason;

};

// ======================================================================
// class RegexList::Entry
// ======================================================================

RegexList::Entry::Entry(char const *pattern, bool matchWords, char const *reason):
	m_pcre(0),
	m_matchWords(matchWords),
	m_reason(reason)
{
	NOT_NULL(pattern);
	NOT_NULL(reason);

	//-- Compile the regular expression pattern.
	char const *errorString = 0;
	int         errorOffset = 0;

	m_pcre = pcre_compile(pattern, 0, &errorString, &errorOffset, nullptr);
	WARNING(!m_pcre, ("failed to compile regex pattern [%s] into a pcre regex.", pattern));
}

// ----------------------------------------------------------------------

RegexList::Entry::~Entry()
{
	if (m_pcre)
	{
		RegexServices::freeMemory(m_pcre);
		m_pcre = 0;
	}
}

// ----------------------------------------------------------------------

inline pcre const *RegexList::Entry::getCompiledRegex() const
{
	return m_pcre;
}

// ----------------------------------------------------------------------

inline bool RegexList::Entry::getMatchWords() const
{
	return m_matchWords;
}

// ----------------------------------------------------------------------

std::string const &RegexList::Entry::getReason() const
{
	return m_reason;
}

// ======================================================================

RegexList::RegexList(const DataTable & sourceTable):
	m_regexList()
{
	int rowCount = sourceTable.getNumRows();
	m_regexList.reserve(rowCount);

	int i;
	int reasonColumn = sourceTable.findColumnNumber("reason");
	int reservedNameColumn = sourceTable.findColumnNumber("reservedName");
	int matchWholeNameColumn = sourceTable.findColumnNumber("matchWholeName");

	for(i = 0; i < rowCount; ++i)
	{
		std::string const  regexPattern = Unicode::toLower(sourceTable.getStringValue(reservedNameColumn, i));
		bool const         matchWords   = sourceTable.getIntValue(matchWholeNameColumn, i) == 0;
		std::string const  reason       = sourceTable.getStringValue(reasonColumn, i);

		m_regexList.push_back(new Entry(regexPattern.c_str(), matchWords, reason.c_str()));
	}
}

// ----------------------------------------------------------------------

RegexList::~RegexList()
{
	std::for_each(m_regexList.begin(), m_regexList.end(), PointerDeleter());
	RegexList::EntryVector().swap(m_regexList);
}

// ----------------------------------------------------------------------

bool RegexList::doesStringMatch(const Unicode::String &name, std::string &ruleDescription) const
{
	std::vector<std::string> dummy;
	return doesStringMatch(name, ruleDescription, dummy);
}

// ----------------------------------------------------------------------

bool RegexList::doesStringMatch(const Unicode::String &name, std::string &ruleDescription, std::vector<std::string> &ignoreRules) const
{
	// parse the name into words
	std::string testName = Unicode::toLower(Unicode::wideToNarrow(name));
	std::vector<std::string> names;

	int pos=0;
	for (;;)
	{
		int newpos = testName.find(' ', pos);
		if (newpos != int(Unicode::String::npos))
		{
			names.push_back(std::string(testName.c_str()+pos, newpos - pos));
			pos = newpos + 1;
		}
		else
		{
			names.push_back(std::string(testName.c_str()+pos));
			break;
		}
	}

	int const maxCaptureCount  = 10;
	int const dataElementCount = maxCaptureCount * 3;
	int       captureData[dataElementCount];

	//-- Test each rule against the input.
	EntryVector::const_iterator const endIt = m_regexList.end();
	for (EntryVector::const_iterator it = m_regexList.begin(); it != endIt; ++it)
	{
		Entry const *const entry = *it;
		if (!entry)
			continue;

		pcre const *const compiledRegex = entry->getCompiledRegex();
		if (!compiledRegex)
			continue;

		if (entry->getMatchWords())
		{
			std::vector<std::string>::iterator nameIter;
			for (nameIter = names.begin(); nameIter != names.end(); ++nameIter)
			{
				int const returnCode = pcre_exec(compiledRegex, nullptr, nameIter->c_str(), nameIter->length(), 0, 0, captureData, dataElementCount);
				if (returnCode >= 0)
				{
					ruleDescription = entry->getReason();
					if(std::find(ignoreRules.begin(), ignoreRules.end(), ruleDescription) == ignoreRules.end())
						return true;
				}
				WARNING(returnCode < -1, ("RegexList::doesStringMatch(): pcre_exec() failed, return code [%d].", returnCode));
			}
		}
		else
		{
			int const returnCode = pcre_exec(compiledRegex, nullptr, testName.c_str(), testName.length(), 0, 0, captureData, dataElementCount);
			if (returnCode >= 0)
			{
				ruleDescription = entry->getReason();
				if(std::find(ignoreRules.begin(), ignoreRules.end(), ruleDescription) == ignoreRules.end())
						return true;
			}
			WARNING(returnCode < -1, ("RegexList::doesStringMatch(): pcre_exec() failed, return code [%d].", returnCode));			
		}
	}

	//-- If we get this far, no patterns matched the subject string.
	return false;
}

// ======================================================================
