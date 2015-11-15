// ======================================================================
//
// LogMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/LogMessage.h"

// ======================================================================

LogMessage::LogMessage() :
	GameNetworkMessage("LogMessage"),
	m_timestamp(0),
	m_procId(),
	m_channel(),
	m_text(),
	m_unicodeAttach()
{
	addVariable(m_timestamp);
	addVariable(m_procId);
	addVariable(m_channel);
	addVariable(m_text);
	addVariable(m_unicodeAttach);
}

// ----------------------------------------------------------------------

LogMessage::LogMessage(uint64 timestamp, std::string const &procId, std::string const &channel, std::string const &text, Unicode::String const &unicodeAttach) :
	GameNetworkMessage("LogMessage"),
	m_timestamp(timestamp),
	m_procId(procId),
	m_channel(channel),
	m_text(text),
	m_unicodeAttach(unicodeAttach)
{
	addVariable(m_timestamp);
	addVariable(m_procId);
	addVariable(m_channel);
	addVariable(m_text);
	addVariable(m_unicodeAttach);
}

// ----------------------------------------------------------------------

LogMessage::LogMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("LogMessage"),
	m_timestamp(0),
	m_procId(),
	m_channel(),
	m_text(),
	m_unicodeAttach()
{
	addVariable(m_timestamp);
	addVariable(m_procId);
	addVariable(m_channel);
	addVariable(m_text);
	addVariable(m_unicodeAttach);
	unpack(source);
}

// ----------------------------------------------------------------------

LogMessage::LogMessage(LogMessage const &rhs) :
	GameNetworkMessage("LogMessage"),
	m_timestamp(rhs.m_timestamp),
	m_procId(rhs.m_procId),
	m_channel(rhs.m_channel),
	m_text(rhs.m_text),
	m_unicodeAttach(rhs.m_unicodeAttach)
{
	addVariable(m_timestamp);
	addVariable(m_procId);
	addVariable(m_channel);
	addVariable(m_text);
	addVariable(m_unicodeAttach);
}

// ----------------------------------------------------------------------

LogMessage::~LogMessage()
{
}

// ----------------------------------------------------------------------

LogMessage & LogMessage::operator=(LogMessage const &rhs)
{
	if (this != &rhs)
	{
		m_timestamp = rhs.m_timestamp;
		m_procId = rhs.m_procId;
		m_channel = rhs.m_channel;
		m_text = rhs.m_text;
		m_unicodeAttach = rhs.m_unicodeAttach;
	}

	return *this;
}

// ======================================================================
