// ======================================================================
//
// CommandParser.h
// copyright (c) 2001 Sony Online Entertainment
//
// jwatson
//
// ======================================================================

#ifndef INCLUDED_CommandParser_H
#define INCLUDED_CommandParser_H

#include "Unicode.h"

class CommandPermissionManager;
class NetworkId;

// ======================================================================

/**
* CommandParser is a generic class for parsing command lines.
* A CommandParser instance is simply a node in a hierarchical tree of
* commands and subcommands.
*
* A command node may be constructed in 2 ways:
*
* 1. Subclass the CommandParser to handle the command.  The command is handled
*    by the performParsing () method, which subclasses should ovverride.  Attach
*    an instance of your subclass to the appropriate node of your command tree.
*
* 2. Simply create an instance of CommandParser, with its delegate set to be 
*    any other CommandParser instance.  The delegate's performParsing () will
*    get called when your new command gets invoked.  argv[0] in performParsing
*    will be the command that corresponds to your new node.  This technique is
*    useful for adding small, lightweight subcommands to a command node, where
*    creating subclasses would be overkill and annoying.
*
*  performParsing () is the only method that must be overridden by subclasses.
*
*  One of the parse () methods and one of the tabCompleTokens () are the entry
*  points into the tree to parse a command or tab complete a command, respectively.
*
*  Examples of how to use this system may be found in the SWUI project.
*
*/

class CommandParser
{
public:
	
	typedef Unicode::String          String_t;
	typedef std::vector<String_t> StringVector_t;

	enum ErrorType
	{
		ERR_NONE,
		ERR_SUCCESS,
		ERR_FAIL,
		ERR_NO_ERROR_MSG,
		ERR_CMD_NOT_FOUND,
		ERR_SUBCMD_NOT_FOUND,
		ERR_NO_HANDLER,
		ERR_NO_SCENE,
		ERR_SCENE_INVALID_OPERATION,
		ERR_NO_AVATAR,
		ERR_NO_CONTROLLER,
		ERR_INVALID_ARGUMENTS,
		ERR_NOT_ENOUGH_ARGUMENTS,
		ERR_SENDING_COMMAND_TO_SERVER,
		ERR_NO_NODES,
		ERR_NO_SUBCOMMAND_SPECIFIED,
		ERR_NO_PARSER_IMPLEMENTED,
		ERR_INVALID_OBJECT,
		ERR_INVALID_SCRIPT,
		ERR_INVALID_TRIGGER,
		ERR_NOT_SKUFREE_TEMPLATE,
		ERR_SCRIPT_CONTINUE,
		ERR_SCRIPT_OVERRIDE,
		ERR_UNSUPPORTED_TRIGGER,
		ERR_INVALID_PARAMETER_COUNT,
		ERR_INVALID_OBJVAR,
		ERR_LIST_OBJVAR_NOT_SUPPORTED,
		ERR_INVALID_INT_OBJVAR,
		ERR_INVALID_REAL_OBJVAR,
		ERR_PERMISSION_DENIED,
		ERR_NO_TERRAIN,
		ERR_TERRAIN_GENERATION,
		ERR_INVALID_USER,
		ERR_NO_USER_OBJECT,
		ERR_NO_TARGET_OBJECT,
		ERR_BAD_ATTACKER,
		ERR_BAD_DEFENDER,
		ERR_QUEUE_COMMAND_FAIL,
		ERR_CANNOT_XFER_AUTH,
		ERR_INVALID_TEMPLATE,
		ERR_INVALID_CONTAINER_TRANSFER,
		ERR_FILE_NOT_FOUND,
		ERR_INVALID_INGREDIENT,
		ERR_INVALID_STATION,
		ERR_STATION_IN_USE,
		ERR_FILLSLOT_FAIL,
		ERR_CUSTOMIZE_FAIL,
		ERR_TEMPLATE_NOT_LOADED,
		ERR_ACTIVE_MANF_STATION,
		ERR_SCRIPTING_ENABLED,
		ERR_SCRIPTING_DISABLED,


		ERR_NUM_ERRS
	};

	struct ErrorInfoData
	{
		ErrorType    m_type;
		const char * m_msg;
	};

	struct CmdInfo
	{
		const char * m_cmd;
		size_t       m_minArgs;
		const char * m_args;
		const char * m_help;
	};		
	
public:

	                        CommandParser (const CmdInfo & info, CommandParser * delegate);
	                        CommandParser (const String_t & cmd, size_t minArgs, const String_t & args, const String_t & help, CommandParser * delegate);
	                        CommandParser (const char * cmd, size_t minArgs, const char * args, const char * help, CommandParser * delegate);
	virtual                ~CommandParser ();

	const String_t &        getCmd                       () const;
	const String_t &        getArgs                      () const;
	const String_t &        getHelp                      () const;
	const size_t            getMinArgs                   () const;

	ErrorType               parse                        (const NetworkId & userId, const String_t & str, String_t & result);

	ErrorType               parse                        (const NetworkId & userId, const StringVector_t & argv, size_t tokenNum, const String_t & originalCommand, String_t & result);

	bool                    tabCompleteToken             (const String_t & str, size_t tokenPos, StringVector_t & results, size_t & token_start, size_t & token_end);
	bool                    tabCompleteToken             (const StringVector_t & argv, size_t tokenNum, size_t whichToken, StringVector_t & results);

	virtual bool            performParsing               (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);

	void                    showHelp                     (String_t & result) const;

	CommandParser *         addSubCommand                (CommandParser * node);

	bool                    isAbbrev                     (const String_t & str) const;
	
	const String_t &        getErrorMessage              (ErrorType code) const;
	String_t                getErrorMessage              (const String_t & prefix, ErrorType code) const;
	String_t                getFullErrorMessage          (ErrorType code) const;
	String_t                getNotEnoughArgumentsMessage () const;
	bool                    isDefault                    () const;
	void                    setDefault                   (bool b);

	static bool             isAbbrev                     (const String_t & str, const String_t & wholeWord);
	static bool             isAbbrev                     (const String_t & str, const char * wholeWord);

	static bool             isCommand                    (const String_t & str, const char * cmd);

	static void             setPermissionManager         (const CommandPermissionManager * permissionManager);

	static void             reconstructString            (const StringVector_t & args, size_t startToken, size_t endToken, bool quote, Unicode::String & result);

	static void             chopInputStrings             (const String_t & str, StringVector_t & sv);

	String_t                constructFullPath            () const;
	String_t                constructFullPath            (const String_t & subCmd) const;

	CommandParser * getDelegate() const;

	CommandParser *         findParser                   (const StringVector_t & argv, size_t tokenNum);

protected:

	void                    createDelegateCommands (const CmdInfo cmds[]);
	bool                    removeParser           (CommandParser * parser);
	void                    deleteSubCommands      ();

private:
	                        CommandParser ();
	                        CommandParser (const CommandParser & rhs);
	CommandParser &         operator=     (const CommandParser & rhs);

	static void             initStatics ();

private:

	String_t                m_cmd;
	size_t                  m_minArgs;
	String_t                m_args;
	String_t                m_help;
	bool                    m_default;

	const CommandParser *   m_parent;
	CommandParser       *   m_delegate;

	typedef std::map<int, String_t> ErrorMsgMap_t;

	static size_t           ms_instanceCount;
	static String_t *       ms_ERR_NO_ERROR_MSG_MSG;
	static ErrorMsgMap_t *  ms_errorMsgMap;

	const static CommandPermissionManager * ms_permissionManager;

	/**
	* Comparator for sorting the set of commands
	*/

	struct Comparator
	{
		bool        operator () (const CommandParser * a, const CommandParser * b) const;
	};

	typedef std::set<CommandParser*, Comparator> CommandParserSet_t;

	CommandParserSet_t *      m_subCommands;
		
	/** 
	* Predicate for searching for a command by abbreviation.
	*/
	
	struct EqualsAbbrevNoCase
	{
		const String_t &     m_str;

		explicit             EqualsAbbrevNoCase (const String_t & theStr) : m_str (theStr) {}
		
		inline bool          operator() (const CommandParser * t) const
		{
			return t->isAbbrev (m_str);
		}
		
		                     EqualsAbbrevNoCase (const EqualsAbbrevNoCase & rhs) : m_str (rhs.m_str) {}
		
	private:
		EqualsAbbrevNoCase & operator= (const EqualsAbbrevNoCase & rhs); //lint !e754
		                     EqualsAbbrevNoCase ();
	};

	/** 
	* Predicate for searching for a command by full name.
	*/
	
	struct EqualsCmdString
	{
		const String_t &     m_str;

		explicit             EqualsCmdString (const String_t & theStr) : m_str (theStr) {}
		
		inline bool   operator() (const CommandParser * t) const
		{
			return t->getCmd () == m_str;
		}

		                     EqualsCmdString (const EqualsCmdString & rhs) : m_str (rhs.m_str) {}
		
	private:
		EqualsCmdString &    operator= (const EqualsCmdString & rhs); //lint !e754
		                     EqualsCmdString ();
	};

	/** 
	* Predicate for searching for a default command.
	*/
	
	struct IsDefault
	{
		inline bool operator() (const CommandParser * t) const
		{
			return t->isDefault ();
		}
	};
};


// ======================================================================


inline const CommandParser::String_t & CommandParser::getCmd () const
{
	return m_cmd;
}

//-----------------------------------------------------------------

inline const CommandParser::String_t & CommandParser::getArgs () const
{
	return m_args;
}

//-----------------------------------------------------------------

inline const CommandParser::String_t & CommandParser::getHelp () const
{
	return m_help;
}

//-----------------------------------------------------------------

inline const size_t CommandParser::getMinArgs () const
{
	return m_minArgs;
}

//-----------------------------------------------------------------

inline bool CommandParser::isDefault () const
{
	return m_default;
}

//-----------------------------------------------------------------

inline void CommandParser::setDefault (const bool b)
{
	m_default = b;
}

//-----------------------------------------------------------------

inline bool CommandParser::Comparator::operator () (const CommandParser * a, const CommandParser * b) const
{
	return a->getCmd () < b->getCmd ();
};

//-----------------------------------------------------------------

inline bool CommandParser::isAbbrev (const String_t & abbrev) const
{
	return isAbbrev (abbrev, m_cmd);
}

//-----------------------------------------------------------------

inline void CommandParser::setPermissionManager (const CommandPermissionManager * permissionManager)
{
	ms_permissionManager = permissionManager;
}

//-----------------------------------------------------------------

#endif
