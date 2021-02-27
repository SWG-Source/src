// ======================================================================
//
// ConsoleCommandParserDefault.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserDefault.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"

#include "serverGame/Chat.h"
#include "sharedFoundation/NetworkId.h"

#ifdef __linux__
#define _wcsicmp wcscasecmp
#endif

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"serverhelp", 0, "",                  "List serverside commands."},
	{"",           0, "",                  ""} // this must be last
};

//-----------------------------------------------------------------

/** 
* Predicate for searching for a command by abbreviation.
*/

struct StrEqualsAbbrevNoCase
{
	const CommandParser::String_t & str;
	
	explicit                StrEqualsAbbrevNoCase (const CommandParser::String_t & theStr) : str (theStr) {}
	
	bool                    operator() (const ConsoleCommandParserDefault::AliasMap_t::value_type & t) const
	{
		return CommandParser::isAbbrev (str, t.first);
	}
	
	                        StrEqualsAbbrevNoCase (const StrEqualsAbbrevNoCase & rhs) : str (rhs.str) {}
	
private:
	StrEqualsAbbrevNoCase & operator= (const StrEqualsAbbrevNoCase & rhs); //lint !e754
	                        StrEqualsAbbrevNoCase ();
};


//-----------------------------------------------------------------
//--
//--
//-----------------------------------------------------------------


ConsoleCommandParserDefault::ConsoleCommandParserDefault (void) :
CommandParser ("", 0, "...", "all server commands", 0),
m_aliases (),
m_aliasGuard (false),
m_aliasGuardSet ()
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------

void ConsoleCommandParserDefault::setAlias (const String_t & key, const String_t & value)
{
	UNREF (key);
	UNREF (value);
	
	const String_t &   aliasString = Unicode::narrowToWide ("\\#aaffaaalias\\#ffffff: ");
	m_aliases [key] = value;
	

	StringVector_t vec;
	vec.push_back (key);


	addSubCommand ( new CommandParser (key, 0, String_t (), aliasString + value, this));
}


//-----------------------------------------------------------------

bool ConsoleCommandParserDefault::removeAlias (const String_t & key)
{

	//AliasMap_t::iterator iter = std::find_if (m_aliases.begin (), m_aliases.end (), StrEqualsNoCase (key));
	AliasMap_t::iterator iter = m_aliases.find (key); 

	if (iter != m_aliases.end ())
	{
		m_aliases.erase (iter);

		StringVector_t vec;
		vec.push_back (key);

		CommandParser * parser = findParser (vec, 0);
		
		if (parser != 0)
		{
			if (removeParser (parser))
				delete parser;
		}

		return true;
	}

	return false;
}


//-----------------------------------------------------------------

bool ConsoleCommandParserDefault::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL (node);
	UNREF (originalCommand);
	UNREF (userId);

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	//-----------------------------------------------------------------

	if (isAbbrev( argv [0], "serverhelp"))
	{
		CommandParser * parser = findParser (argv, 1);

		if (parser == 0)
		{
			String_t str;

			for (size_t i = 1; i < argv.size (); ++i)
			{
				str.append (argv [i]).append (1, ' ');
			}

			result += getErrorMessage ( str, ERR_CMD_NOT_FOUND);
		}
		else
			parser->showHelp (result);
	}

	return true;
}


// ======================================================================
