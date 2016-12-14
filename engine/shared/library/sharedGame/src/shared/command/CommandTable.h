// ======================================================================
//
// CommandTable.h
//
// Copyright 2002-2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CommandTable_H
#define INCLUDED_CommandTable_H

// ======================================================================

#include "sharedGame/CommandCppFunc.def"

// ======================================================================

class Command;

// ======================================================================

class CommandTable // static class
{
public:
	typedef std::map<uint32, Command> CommandMap;

	static void                loadCommandTables  (std::vector<std::string> const &commandTableFilenames);
	static void                addCppFunction     (std::string const &funcName, CommandCppFunc::Type func);
	static Command const &     getCommand         (uint32 commandHash);
	static CommandMap const &  getCommandMap      ();
	static Command const &     getNullCommand     ();
	static bool isCommandQueueCommand(uint32 hash);
};

// ======================================================================

#endif // INCLUDED_CommandTable_H

