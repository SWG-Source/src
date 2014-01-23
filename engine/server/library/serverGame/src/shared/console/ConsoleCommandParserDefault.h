// ======================================================================
//
// ConsoleCommandParserDefault.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserDefault_H
#define INCLUDED_ConsoleCommandParserDefault_H

#include "sharedCommandParser/CommandParser.h"
#include <map>
#include <unordered_set>
#include "UnicodeUtils.h"

// ======================================================================
class ConsoleCommandParserDefault : public CommandParser
{
public:
	typedef std::map<String_t, String_t> AliasMap_t;
	typedef std::unordered_set<String_t>      AliasGuardSet_t;


	explicit               ConsoleCommandParserDefault (void);
	virtual bool           performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);

private:

	                       ConsoleCommandParserDefault (const ConsoleCommandParserDefault & rhs);
	                       ConsoleCommandParserDefault & operator= (const ConsoleCommandParserDefault & rhs);

	void                   setAlias (const String_t & key, const String_t & value);
	/**
	* This method should be removed one day when localization rules the earth -- jww
	*/
	void                   setAlias (const char * key, const char * value);
	bool                   removeAlias (const String_t & key);

private:

	AliasMap_t             m_aliases;
	bool                   m_aliasGuard;
	AliasGuardSet_t        m_aliasGuardSet;
};

// ======================================================================

inline void  ConsoleCommandParserDefault::setAlias (const char * const key, const char * const value)
{
	NOT_NULL (key);
	NOT_NULL (value);

	setAlias (Unicode::narrowToWide (key), Unicode::narrowToWide (value));
}
//-----------------------------------------------------------------


#endif	// INCLUDED_ConsoleCommandParserDefault_H
