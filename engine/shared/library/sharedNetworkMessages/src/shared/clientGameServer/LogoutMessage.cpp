// ======================================================================
//
// LogoutMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "LogoutMessage.h"

// ======================================================================

LogoutMessage::LogoutMessage() :
	GameNetworkMessage("LogoutMessage")
{
}

// ----------------------------------------------------------------------

LogoutMessage::LogoutMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("LogoutMessage")
{
	unpack(source);
}

// ----------------------------------------------------------------------

LogoutMessage::~LogoutMessage()
{
}

// ======================================================================

