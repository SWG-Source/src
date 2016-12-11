// ======================================================================
//
// RegexList.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_RegexList_H
#define INCLUDED_RegexList_H

// ======================================================================

class DataTable;
class StringId;

#include <vector>

// ======================================================================

/**
 * A class to encapsulate a list of regular expressions.  A string can
 * be compared against the list to see if it matches any of the expressions.
 *
 * Each expression can have a description associated with it.
 * This is used, for example, in the naming filter system to give a reason
 * the name is on the list, e.g. reserved, obscene, etc.  (This description
 * is indended to be a string name for a localization string table.)
 */
class RegexList
{
public:
	RegexList(const DataTable & sourceTable);
	~RegexList();
	
	bool doesStringMatch(const Unicode::String &name) const;
	bool doesStringMatch(const Unicode::String &name, std::string &ruleDescription) const;
	bool doesStringMatch(const Unicode::String &name, std::string &ruleDescription, std::vector<std::string> &ignoreRules) const;

private:

	class Entry;
	typedef std::vector<Entry*>  EntryVector;
		
private:

	EntryVector  m_regexList;

};

// ----------------------------------------------------------------------

inline bool RegexList::doesStringMatch(const Unicode::String &name) const
{
	std::string ignoreRuleDescription;
	return doesStringMatch(name,ignoreRuleDescription);	
}

// ======================================================================

#endif
