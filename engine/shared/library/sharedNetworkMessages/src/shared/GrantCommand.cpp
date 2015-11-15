// GrantCommand.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "GrantCommand.h"

//-----------------------------------------------------------------------

GrantCommand::GrantCommand(const std::string & c) :
GameNetworkMessage("GrantCommand"),
commandName(c)
{
	addVariable(commandName);
}

//-----------------------------------------------------------------------

GrantCommand::GrantCommand(Archive::ReadIterator & source) :
GameNetworkMessage("GrantCommand"),
commandName()
{
	addVariable(commandName);
	unpack(source);
}

//-----------------------------------------------------------------------

GrantCommand::~GrantCommand()
{
}

//-----------------------------------------------------------------------

