// ChatOnRequestLog.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnRequestLog_H
#define	_INCLUDED_ChatOnRequestLog_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------


struct ChatLogEntry
{
	ChatLogEntry()
     : m_from()
	 , m_to()
	 , m_channel()
	 , m_message()
	 , m_time(0)
	{
	}

    ChatLogEntry(Unicode::String const &from, Unicode::String const &to, Unicode::String const &channel, Unicode::String const &message, time_t time)
	 : m_from(from)
	 , m_to(to)
	 , m_channel(channel)
	 , m_message(message)
	 , m_time(time)
	 {
	 }

    Unicode::String m_from;
    Unicode::String m_to;
    Unicode::String m_channel;
    Unicode::String m_message;
    time_t m_time;
};

namespace Archive
{
	void get (ReadIterator & source, ChatLogEntry & target);
	void put (ByteStream & target, const ChatLogEntry & source);
}

//-----------------------------------------------------------------------

class ChatOnRequestLog : public GameNetworkMessage
{
public:
	ChatOnRequestLog(unsigned int sequence, const std::vector<ChatLogEntry> &entries);
	ChatOnRequestLog(Archive::ReadIterator &);
	~ChatOnRequestLog();

	const std::vector<ChatLogEntry> &  getLogEntries  () const;
	const unsigned int                 getSequence    () const;

private:
	ChatOnRequestLog & operator = (const ChatOnRequestLog & rhs);
	ChatOnRequestLog(const ChatOnRequestLog & source);

	Archive::AutoArray<ChatLogEntry>     logEntries;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const std::vector<ChatLogEntry> & ChatOnRequestLog::getLogEntries() const
{
	return logEntries.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnRequestLog::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnRequestLog_H
