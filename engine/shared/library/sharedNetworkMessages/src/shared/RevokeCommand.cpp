// RevokeCommand.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "RevokeCommand.h"

//-----------------------------------------------------------------------

RevokeCommand::RevokeCommand(const std::string & c) :
GameNetworkMessage("RevokeCommand"),
commandName(c)
{
	addVariable(commandName);
}

//-----------------------------------------------------------------------

RevokeCommand::RevokeCommand(Archive::ReadIterator & source) :
GameNetworkMessage("RevokeCommand"),
commandName()
{
	addVariable(commandName);
	unpack(source);
}

//-----------------------------------------------------------------------

RevokeCommand::~RevokeCommand()
{
}

//-----------------------------------------------------------------------

