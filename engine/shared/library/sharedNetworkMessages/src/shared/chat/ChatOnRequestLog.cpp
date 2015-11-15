// ChatOnRequestLog.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnRequestLog.h"

//-----------------------------------------------------------------------

ChatOnRequestLog::ChatOnRequestLog(const unsigned int s, const std::vector<ChatLogEntry> & entries) :
GameNetworkMessage("ChatOnRequestLog"),
logEntries(),
sequence(s)
{
	logEntries.set(entries);
	addVariable(logEntries);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnRequestLog::ChatOnRequestLog(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnRequestLog"),
logEntries(),
sequence()
{
	addVariable(logEntries);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnRequestLog::~ChatOnRequestLog()
{
}

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ChatLogEntry & target)
	{
		get (source, target.m_from);
		get (source, target.m_to);
		get (source, target.m_channel);
		get (source, target.m_message);
		get (source, target.m_time);	
	}

	void put (ByteStream & target, const ChatLogEntry & source)
	{
		put (target, source.m_from);
		put (target, source.m_to);
		put (target, source.m_channel);
		put (target, source.m_message);
		put (target, source.m_time);
	}
}

//======================================================================
