// ======================================================================
//
// LogMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_LogMessage_H
#define	_INCLUDED_LogMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"

// ======================================================================

class LogMessage : public GameNetworkMessage
{
public:
	LogMessage();
	LogMessage(uint64 timestamp, std::string const &procId, std::string const &channel, std::string const &text, Unicode::String const &unicodeAttach);
	LogMessage(Archive::ReadIterator &source);
	LogMessage(LogMessage const &source);
	~LogMessage();

	uint64                  getTimestamp() const;
	std::string const &     getProcId() const;
	std::string const &     getChannel() const;
	std::string const &     getText() const;
	Unicode::String const & getUnicodeAttach() const;

	LogMessage &            operator=(LogMessage const &rhs);

private:
	Archive::AutoVariable<uint64>          m_timestamp;
	Archive::AutoVariable<std::string>     m_procId;
	Archive::AutoVariable<std::string>     m_channel;
	Archive::AutoVariable<std::string>     m_text;
	Archive::AutoVariable<Unicode::String> m_unicodeAttach;
};

// ======================================================================

inline uint64 LogMessage::getTimestamp() const
{
	return m_timestamp.get();
}

// ----------------------------------------------------------------------

inline std::string const &LogMessage::getProcId() const
{
	return m_procId.get();
}

// ----------------------------------------------------------------------

inline std::string const &LogMessage::getChannel() const
{
	return m_channel.get();
}

// ----------------------------------------------------------------------

inline std::string const &LogMessage::getText() const
{
	return m_text.get();
}

// ----------------------------------------------------------------------

inline Unicode::String const &LogMessage::getUnicodeAttach() const
{
	return m_unicodeAttach.get();
}

// ======================================================================

#endif	// _INCLUDED_LogMessage_H
