// ======================================================================
//
// CommandParser.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCommandParser/FirstSharedCommandParser.h"
#include "sharedCommandParser/CommandParser.h"
#include "sharedCommandParser/CommandPermissionManager.h"

#include "sharedFoundation/NetworkId.h"
#include "UnicodeUtils.h" 

#include <algorithm>
#include <cstdio>
#include <vector>
#include <set>
#include <map>

// ======================================================================

//- TODO: load these messages from the StringTable to localize the commandparser - jww

static const CommandParser::ErrorInfoData errors[static_cast<size_t>(CommandParser::ERR_NUM_ERRS)+1] =
{
	{CommandParser::ERR_NONE,                    "None - error!"},
	{CommandParser::ERR_SUCCESS,                 "Command completed succesfully."},
	{CommandParser::ERR_FAIL,                    "Command failed!"},
	{CommandParser::ERR_NO_ERROR_MSG,            "No error message available!"},
	{CommandParser::ERR_CMD_NOT_FOUND,           "Command not found."},
	{CommandParser::ERR_SUBCMD_NOT_FOUND,        "Subcommand not found."},
	{CommandParser::ERR_NO_HANDLER,              "No handler for this command."},
	{CommandParser::ERR_NO_SCENE,                "No scene available."},
	{CommandParser::ERR_SCENE_INVALID_OPERATION, "Scene does not support this operation."},
	{CommandParser::ERR_INVALID_ARGUMENTS,       "Invalid arguments."},
	{CommandParser::ERR_NOT_ENOUGH_ARGUMENTS,    "Not enough arguments."},
	{CommandParser::ERR_NO_AVATAR,               "No avatar found!"},
	{CommandParser::ERR_SENDING_COMMAND_TO_SERVER, "Sending command to server."},
	{CommandParser::ERR_NO_CONTROLLER,           "No controller found!"},
	{CommandParser::ERR_NO_NODES,                "No CommandParser nodes attached."},
	{CommandParser::ERR_NO_SUBCOMMAND_SPECIFIED, "No subcommand specified.  Subcommands:\n"},
	{CommandParser::ERR_NO_PARSER_IMPLEMENTED,   "No parser implemented!"},
	{CommandParser::ERR_INVALID_OBJECT,          "Invalid object."},
	{CommandParser::ERR_INVALID_SCRIPT,          "Invalid script name or other error."},
	{CommandParser::ERR_INVALID_TRIGGER,         "Invalid trigger id."},
	{CommandParser::ERR_NOT_SKUFREE_TEMPLATE,    "Can't create a non skufree asset in a free trial area."},
	{CommandParser::ERR_SCRIPT_CONTINUE,         "Trigger returns SCRIPT_CONTINUE"},
	{CommandParser::ERR_SCRIPT_OVERRIDE,         "Trigger returns SCRIPT_OVERRIDE"},
	{CommandParser::ERR_UNSUPPORTED_TRIGGER,     "Cannot invoke trigger from the command line."},
	{CommandParser::ERR_INVALID_PARAMETER_COUNT, "Wrong number of parameters for trigger, needed "},
	{CommandParser::ERR_INVALID_OBJVAR,          "Cannot find objvar."},
	{CommandParser::ERR_LIST_OBJVAR_NOT_SUPPORTED, "Objvar type = list, function not supported."},
	{CommandParser::ERR_INVALID_INT_OBJVAR,      "Invalid data for int objvar."},
	{CommandParser::ERR_INVALID_REAL_OBJVAR,     "Invalid data for real objvar."},
	{CommandParser::ERR_PERMISSION_DENIED,       "Permission denied."},
	{CommandParser::ERR_NO_TERRAIN,              "Scene does not have a terrain."},
	{CommandParser::ERR_TERRAIN_GENERATION,      "Could not generate terrain at these coords."},
	{CommandParser::ERR_INVALID_USER,            "The user issuing the command was not found."},
	{CommandParser::ERR_NO_TARGET_OBJECT,        "The target given was not found."},
	{CommandParser::ERR_BAD_ATTACKER,            "The attacker is not a tangible object."},
	{CommandParser::ERR_BAD_DEFENDER,            "The target is not a tangible object."},
	{CommandParser::ERR_QUEUE_COMMAND_FAIL,      "Failed to queue command."},
	{CommandParser::ERR_CANNOT_XFER_AUTH,        "Request to transfer authority denied."},
	{CommandParser::ERR_INVALID_TEMPLATE,        "Invalid Object template specified."},
	{CommandParser::ERR_INVALID_CONTAINER_TRANSFER, "Invalid Container Transfer."},
	{CommandParser::ERR_FILE_NOT_FOUND,          "File not found."},
	{CommandParser::ERR_INVALID_INGREDIENT,      "Invalid ingredient."},
	{CommandParser::ERR_INVALID_STATION,         "Object not a crafting station."},
	{CommandParser::ERR_STATION_IN_USE,          "Crafting station in use."},
	{CommandParser::ERR_FILLSLOT_FAIL,           "Error filling the schematic slot."},
	{CommandParser::ERR_CUSTOMIZE_FAIL,          "Error setting customization property."},
	{CommandParser::ERR_TEMPLATE_NOT_LOADED,     "Template Not Loaded"},
	{CommandParser::ERR_ACTIVE_MANF_STATION,     "Manufacturing station active, please turn off."},
	{CommandParser::ERR_SCRIPTING_ENABLED,       "Scripting is enabled."},
	{CommandParser::ERR_SCRIPTING_DISABLED,      "Scripting is disabled."},
	{CommandParser::ERR_NUM_ERRS,                ""}

};

//----------------------------------------------------------------------

namespace
{
	const Unicode::String s_alias_cmd       = Unicode::narrowToWide ("alias");
	const Unicode::String s_alias_cmd_slash = Unicode::narrowToWide ("/") + s_alias_cmd;
};

//-----------------------------------------------------------------

CommandParser::String_t *              CommandParser::ms_ERR_NO_ERROR_MSG_MSG;
CommandParser::ErrorMsgMap_t *         CommandParser::ms_errorMsgMap;
size_t                                 CommandParser::ms_instanceCount;
const CommandPermissionManager *       CommandParser::ms_permissionManager;

//-----------------------------------------------------------------

CommandParser::CommandParser (const String_t & cmd,
							  size_t minArgs,
							  const String_t & args,
							  const String_t & help,
							  CommandParser * delegate) :
m_cmd (cmd),
m_minArgs (minArgs),
m_args (args),
m_help (help),
m_default (false),
m_parent (0),
m_delegate (delegate),
m_subCommands (NON_NULL (new CommandParserSet_t))
{
	++ms_instanceCount;
	initStatics ();
}

//-----------------------------------------------------------------

CommandParser::CommandParser (const char * const cmd,
							  size_t minArgs,
							  const char * const args,
							  const char * const help,
							  CommandParser * delegate) :
m_cmd (),
m_minArgs (minArgs),
m_args (),
m_help (),
m_default (false),
m_parent (0),
m_delegate (delegate),
m_subCommands (NON_NULL (new CommandParserSet_t))
{
	++ms_instanceCount;
	initStatics ();

	std::string cmd_narrow (cmd);
	m_cmd.append (cmd_narrow.begin (), cmd_narrow.end ());

	std::string args_narrow (args);
	m_args.append (args_narrow.begin (), args_narrow.end ());

	std::string help_narrow (help);
	m_help.append (help_narrow.begin (), help_narrow.end ());
}

//-----------------------------------------------------------------

CommandParser::CommandParser (const CommandParser::CmdInfo & info, CommandParser * delegate) :
m_cmd (Unicode::narrowToWide (info.m_cmd)),
m_minArgs (info.m_minArgs),
m_args (Unicode::narrowToWide (info.m_args)),
m_help (Unicode::narrowToWide (info.m_help)),
m_default (false),
m_parent (0),
m_delegate (delegate),
m_subCommands (NON_NULL (new CommandParserSet_t))
{
	++ms_instanceCount;
	initStatics ();
}

//-----------------------------------------------------------------

void CommandParser::initStatics (void)
{
	if (ms_errorMsgMap == 0)
	{
		ms_errorMsgMap          = new ErrorMsgMap_t;

		for (int i = 0; ; ++i)
		{
			const CommandParser::ErrorInfoData & infoData = errors [i];

			if (infoData.m_type == ERR_NUM_ERRS)
				break;

			const Unicode::String str (Unicode::narrowToWide (infoData.m_msg));
			const std::pair<ErrorMsgMap_t::iterator, bool> retval =
				ms_errorMsgMap->insert (std::make_pair (static_cast<int>(infoData.m_type), str));

			DEBUG_FATAL (!retval.second, ("error adding error message.\n"));
			UNREF (retval);
		}

		ms_ERR_NO_ERROR_MSG_MSG = new String_t ((*ms_errorMsgMap)[static_cast<int>(ERR_NO_ERROR_MSG)]);
	}
}

//----------------------------------------------------------------------

void CommandParser::deleteSubCommands      ()
{
	for (CommandParserSet_t::iterator iter = m_subCommands->begin (); iter != m_subCommands->end (); ++iter)
	{
		 delete *iter;
	}
	
	m_subCommands->clear ();
}

//-----------------------------------------------------------------

CommandParser::~CommandParser ()
{
	m_delegate = 0;
	m_parent = 0;

	for (CommandParserSet_t::iterator iter = m_subCommands->begin (); iter != m_subCommands->end (); ++iter)
	{
		 delete *iter;
	}

	m_subCommands->clear ();
	delete m_subCommands;
	m_subCommands = 0;

	DEBUG_FATAL (ms_instanceCount == 0, ("CommandParser instanceCount went negative... bogus constructors?\n"));

	--ms_instanceCount;

	if (ms_instanceCount == 0)
	{
		delete ms_errorMsgMap;
		ms_errorMsgMap = 0;
		delete ms_ERR_NO_ERROR_MSG_MSG;
		ms_ERR_NO_ERROR_MSG_MSG = 0;
	}
}

//----------------------------------------------------------------------

void CommandParser::chopInputStrings (const String_t & str, StringVector_t & sv)
{
	size_t pos    = 0;
	size_t endpos = 0;
	Unicode::String commandPart;
	
	const static Unicode::unicode_char_t sep [2] = { ';', 0 };

	while (Unicode::getFirstToken (str, pos, endpos, commandPart, sep))
	{
		Unicode::trim (commandPart);

		if (!commandPart.empty ())
		{
			//-- if this is the magic alias command, stop choppin
			if (isAbbrev (s_alias_cmd, commandPart) || isAbbrev (s_alias_cmd_slash, commandPart))
			{
				commandPart = str.substr (pos);
				Unicode::trim (commandPart);
				endpos = static_cast<size_t>(str.npos);
			}

			sv.push_back (commandPart);
		}

		if (endpos == Unicode::String::npos)
			break;

		pos = endpos + 1;
	}
}

//-----------------------------------------------------------------

CommandParser::ErrorType CommandParser::parse (const NetworkId & userId, const String_t & str, String_t & result)
{
	//----------------------------------------------------------------------
	//- first break the string into parts via semicolons, then
	//- tokenize each portion of the string.

	ErrorType last_retval = ERR_NONE;

	StringVector_t sv;

	chopInputStrings (str, sv);

	for (StringVector_t::const_iterator it = sv.begin (); it != sv.end (); ++it)
	{
		const Unicode::String & commandPart = *it;
		
		if (!result.empty () && result [result.size () - 1] != '\n')
			IGNORE_RETURN (result.append (1, '\n'));		

		size_t dummy;
		StringVector_t v;
		Unicode::tokenize (commandPart, v, 0, dummy, dummy, dummy);

		last_retval = parse (userId, v, 0, str, result);
	}

	return last_retval;
}

	
//-----------------------------------------------------------------

CommandParser * CommandParser::findParser (const StringVector_t & argv, const size_t tokenNum)
{
	if (tokenNum >= argv.size ())
		return this;

	// look for the appropriate subcommand
	const String_t & cur_cmd = argv [tokenNum];

	CommandParserSet_t::iterator find_iter = std::find_if (m_subCommands->begin (), m_subCommands->end (), EqualsAbbrevNoCase (cur_cmd));
	
	if (find_iter == m_subCommands->end ())
	{
		return 0;
	}

	// more arguments are available, so descend the command parser tree

	CommandParser & cmd = **find_iter;
	return cmd.findParser (argv, tokenNum + 1);
}

//-----------------------------------------------------------------

/**
* Note: tokenNum can exceed the capacity of argv
* @param originalCommand 
*/

CommandParser::ErrorType CommandParser::parse (const NetworkId & userId, const StringVector_t & argv, const size_t tokenNum, const String_t & originalCommand, String_t & result) 
{
	// this command is a leaf node, thus it is capable of perfoming the interpret
	if (m_subCommands->size () == 0)
	{		
		// performInterpret () does not get called on the root node
		if (m_parent == 0)
		{
			result += getErrorMessage (ERR_NO_NODES);
			return ERR_NO_NODES;
		}

		// not enough arguments
		if (m_minArgs > (argv.size () - tokenNum))
		{
			result += getNotEnoughArgumentsMessage ();
			return ERR_NOT_ENOUGH_ARGUMENTS;
		}
		
		//-- check for adequate permissions
		if (ms_permissionManager)
		{
			Unicode::String path;
			static const Unicode::unicode_char_t pathSep = '.';

			const CommandParser * cmd = this;

			while (cmd && cmd->m_parent)
			{
				if (path.empty ())
					path = cmd->getCmd ();
				else
				{
					path = cmd->getCmd () + pathSep + path;
				}
				cmd = cmd->m_parent;
			}

			if (ms_permissionManager->isCommandAllowed (userId, path) == false)
			{
				result += getErrorMessage (getCmd (), ERR_PERMISSION_DENIED);
				return ERR_PERMISSION_DENIED;
			}
		}

		// construct the truncated string vector the the performInterpret () method
		StringVector_t passArgv;
		passArgv.reserve (argv.size () - tokenNum + 1);
		
		DEBUG_FATAL (tokenNum == 0, ("tokenNum == 0 and no subCommands -- error.\n"));

		{
			passArgv.push_back (m_cmd);

			for (size_t i = tokenNum; i < argv.size (); ++i)
				passArgv.push_back (argv [i]);
		}
		
		CommandParser * handler = m_delegate ? m_delegate : this;

		if (handler->performParsing(userId, passArgv, originalCommand, result, this))
			return ERR_SUCCESS;

		return ERR_NO_HANDLER;
	}
	
	// no more arguments, this is not a fully qualified command line
	if (tokenNum >= argv.size ())
	{

		// use default command if applicable

		CommandParserSet_t::iterator default_iter = std::find_if (m_subCommands->begin (), m_subCommands->end (), IsDefault ());
	
		if (default_iter != m_subCommands->end ())
		{
			CommandParser & cmd = **default_iter;
			
			// we need a modifed argument vector with the default argument on it
			StringVector_t vect = argv;
			vect.push_back (cmd.m_cmd);
		
			return cmd.parse (userId, vect, tokenNum + 1, originalCommand, result);
		}

		// list possible subcommands
		
		result += getErrorMessage (constructFullPath (), ERR_NO_SUBCOMMAND_SPECIFIED);
		
		showHelp (result);
		
		return ERR_SUCCESS;
	}

	// look for the appropriate subcommand
	const String_t & cur_cmd = argv [tokenNum];

	CommandParserSet_t::iterator find_iter = std::find_if (m_subCommands->begin (), m_subCommands->end (), EqualsAbbrevNoCase (cur_cmd));
	
	if (find_iter == m_subCommands->end ())
	{
		const ErrorType etype = m_parent ? ERR_SUBCMD_NOT_FOUND : ERR_CMD_NOT_FOUND;
		result += getErrorMessage (constructFullPath (argv [tokenNum]), etype);
		return etype;
	}

	// more arguments are available, so descend the command parser tree

	CommandParser & cmd = **find_iter;
	return cmd.parse (userId, argv, tokenNum + 1, originalCommand, result);
}

//-----------------------------------------------------------------

bool CommandParser::tabCompleteToken (const String_t & str, const size_t tokenPos, StringVector_t & results, size_t & token_start, size_t & token_end)
{
	StringVector_t v;
	size_t whichToken;
	Unicode::tokenize (str, v, tokenPos, whichToken, token_start, token_end);
	
	// ends with space, add an empty token
	if (whichToken == (v.size () - 1) && tokenPos > 0 && str [tokenPos-1] == ' ')
	{
		v.push_back (String_t ());
		++whichToken;
		token_start = tokenPos;
		token_end = tokenPos;
	}

	return tabCompleteToken (v, 0, whichToken, results);
}

//-----------------------------------------------------------------

bool CommandParser::tabCompleteToken (const StringVector_t & argv, const size_t tokenNum, const size_t whichToken, StringVector_t & results)
{
	if (tokenNum > whichToken)
		return true;

//	DEBUG_FATAL (argv.empty (), ("argv is empty in tab complete\n"));

	// look for the appropriate subcommand
	const String_t empty;
	const String_t & cur_cmd = argv.empty () ? empty : argv [tokenNum];

	// this command(s) should be under this node
	if (whichToken == tokenNum)
	{		
		for (CommandParserSet_t::const_iterator iter = m_subCommands->begin (); iter != m_subCommands->end (); ++iter)
		{
			const CommandParser & sub_cmd = **iter;

			if (sub_cmd.isAbbrev (cur_cmd))
				results.push_back (sub_cmd.m_cmd);
		}

		return true;
	}

	CommandParserSet_t::iterator find_iter = std::find_if (m_subCommands->begin (), m_subCommands->end (), EqualsAbbrevNoCase (cur_cmd));
	
	if (find_iter == m_subCommands->end ())
	{
		return true;
	}

	// more arguments are available, so descend the command parser tree

	CommandParser & cmd = **find_iter;
	return cmd.tabCompleteToken (argv, tokenNum + 1, whichToken, results);
}

//-----------------------------------------------------------------

void  CommandParser::showHelp (String_t & result) const
{
	if (m_subCommands->size () > 0)
	{
		result += 
			Unicode::narrowToWide ("\\#ffffffCommands:\n"
						 "-------------------------------------------------------\n\n");

		if (m_parent)
		{
			result.append (m_cmd);
			result.append (Unicode::narrowToWide ("\\%030\\#00ff00"));
			result.append (m_args);
			result.append (Unicode::narrowToWide ("\\#ffffff\n\\>000"));
			result.append (m_help);
			result.append (Unicode::narrowToWide ("\\>000"));
			result.append (1, '\n');
		}

		for (CommandParserSet_t::const_iterator iter = m_subCommands->begin (); iter != m_subCommands->end (); ++iter)
		{
			const CommandParser & sub_cmd = **iter;
			result.append (2, ' ');
			result.append (sub_cmd.m_cmd);
			result.append (Unicode::narrowToWide ("\\%030\\#00ff00"));
			result.append (sub_cmd.m_args);
			result.append (Unicode::narrowToWide ("\\#ffffff\n\\>000"));
			result.append (sub_cmd.m_help);			
			result.append (Unicode::narrowToWide ("\\>000"));
			result.append (1, '\n');
		}
	}
	else
	{
		result += Unicode::narrowToWide ("Usage: ");
		result.append (m_cmd);
		result.append (1, ' ');
		result.append (m_args);
		result.append (Unicode::narrowToWide ("   : "));
		result.append (m_help);
		result.append (1, '\n');
	}
}

//-----------------------------------------------------------------

bool  CommandParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (userId);
	UNREF (argv);
	UNREF (originalCommand);
	UNREF (node);

	if (!result.empty () && result [result.size () - 1] != '\n')
		IGNORE_RETURN (result.append (1, '\n'));

	result += getErrorMessage (constructFullPath (), ERR_NO_PARSER_IMPLEMENTED);
	return true;
} 

//-----------------------------------------------------------------
/**
* overwrite any existing subnode
*/
CommandParser * CommandParser::addSubCommand (CommandParser * node)
{
	CommandParserSet_t::iterator find_iter = std::find_if (m_subCommands->begin (), m_subCommands->end (), EqualsCmdString (node->m_cmd));

	// remove existing
	if (find_iter != m_subCommands->end ())
	{
		delete *find_iter;
		m_subCommands->erase (find_iter);
	}

	std::pair<CommandParserSet_t::iterator, bool> retval = m_subCommands->insert (node);
	
	DEBUG_FATAL (retval.second == false, ("Error in command parser.\n"));
	UNREF (retval);

	node->m_parent = this;
	return node;
}


//-----------------------------------------------------------------

bool CommandParser::removeParser (CommandParser * const parser)
{
	CommandParserSet_t::iterator iter = m_subCommands->find (parser);

	if (iter == m_subCommands->end ())
	{
		return false;
	}
	else
	{
		m_subCommands->erase (iter);
		return true;
	}
}

//-----------------------------------------------------------------

const CommandParser::String_t & CommandParser::getErrorMessage (CommandParser::ErrorType code) const
{
	ErrorMsgMap_t::const_iterator iter = ms_errorMsgMap->find (static_cast<int>(code));

	if (iter == ms_errorMsgMap->end ())	
	{
		return *ms_ERR_NO_ERROR_MSG_MSG;
	}
	else
		return iter->second;
}

//-----------------------------------------------------------------

CommandParser::String_t CommandParser::getErrorMessage (const CommandParser::String_t & prefix, CommandParser::ErrorType code) const
{
	return prefix + Unicode::narrowToWide (": ") + getErrorMessage (code);
}

//-----------------------------------------------------------------

CommandParser::String_t CommandParser::getFullErrorMessage (CommandParser::ErrorType code) const
{
	return getErrorMessage (constructFullPath (), code);
}

//-----------------------------------------------------------------

CommandParser::String_t CommandParser::getNotEnoughArgumentsMessage (void) const
{
	return (constructFullPath () + Unicode::narrowToWide(": ") + getErrorMessage (ERR_NOT_ENOUGH_ARGUMENTS) + Unicode::narrowToWide ("\nUsage:  ") + m_cmd).append (1, ' ') + m_args;
}

//-----------------------------------------------------------------
/**
* createDelegateCommands takes an array of CmdInfo objects, terminated with a CmdInfo with an empty m_cmd
*/

void CommandParser::createDelegateCommands (const CmdInfo cmds[])
{
	for (int i = 0; ; ++i)
	{
		const CommandParser::CmdInfo & info = cmds [i];

		if (*info.m_cmd == 0)
			break;

		addSubCommand (new CommandParser (info, this));
	}
}

//----------------------------------------------------------------------

bool   CommandParser::isAbbrev (const String_t & str, const String_t & wholeStr)
{
	if (wholeStr.length () < str.length ())
		return false;

	if (str.empty ())
		return true;

	return Unicode::caseInsensitiveCompare (str, wholeStr, 0, str.length ());
}

//----------------------------------------------------------------------

void CommandParser::reconstructString (const StringVector_t & args, size_t startToken, size_t endToken, bool quote, Unicode::String & result)
{
	endToken = std::min (endToken, args.size ());

	result.clear ();

	for (size_t i = startToken; i < endToken; ++i)
	{
		if (i != startToken)
			result.append (1, ' ');

		const Unicode::String & str = args [i];

		if (quote && str.find ('\"') != str.npos)
		{
			result.append (1, '\"');
			result += str;
			result.append (1, '\"');
		}
		else
			result += str;
	}
}

//----------------------------------------------------------------------

CommandParser::String_t  CommandParser::constructFullPath  (const String_t & subCmd) const
{
	String_t p (constructFullPath());
	
	if (!subCmd.empty ())
	{
		if (!p.empty ())
			p.append (1, ' ');
		
		p.append (subCmd);
	}
	
	return p;
}

//----------------------------------------------------------------------

CommandParser::String_t CommandParser::constructFullPath   () const
{
	String_t p;
	
	if (m_parent)
		p = m_parent->constructFullPath();
	
	if (!p.empty ())
		p.append (1, ' ');
	
	p.append (m_cmd);
	
	return p;
}

//-----------------------------------------------------------------
//-- this method is very inefficient
//

bool   CommandParser::isAbbrev (const String_t & str, const char * const wholeWord)
{
	return isAbbrev (str, Unicode::narrowToWide (wholeWord));
}

//----------------------------------------------------------------------

bool CommandParser::isCommand (const String_t & str, const char * cmd)
{
	return str == Unicode::narrowToWide (cmd);
}

//----------------------------------------------------------------------

CommandParser * CommandParser::getDelegate() const
{
	return m_delegate;
}

// ======================================================================
