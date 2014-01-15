// ======================================================================
//
// CommandCppFuncs.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CommandCppFuncs_H
#define INCLUDED_CommandCppFuncs_H

class Command;
class NetworkId;

class CommandCppFuncs // static class
{
public:
	static void install();
	static void remove();

	static void commandFuncTransferMisc(Command const & c, NetworkId const &actor, NetworkId const &target, Unicode::String const & params);
};

#endif // INCLUDED_CommandCppFuncs_H
