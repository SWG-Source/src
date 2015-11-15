// ChatRequestLog.cpp
// Copyright 2000-02, Sony Online Createtainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatRequestLog.h"

//-----------------------------------------------------------------------

ChatRequestLog::ChatRequestLog(const unsigned int s, Unicode::String const &player) :
GameNetworkMessage("ChatRequestLog"),
m_player(player),
sequence(s)
{
	addVariable(m_player);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatRequestLog::ChatRequestLog(Archive::ReadIterator & source) :
GameNetworkMessage("ChatRequestLog"),
m_player(),
sequence()
{
	addVariable(m_player);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatRequestLog::~ChatRequestLog()
{
}

//-----------------------------------------------------------------------

