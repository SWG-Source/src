// ChatRequestLog.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatRequestLog_H
#define	_INCLUDED_ChatRequestLog_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatRequestLog : public GameNetworkMessage
{
public:
	ChatRequestLog(const unsigned int sequence, Unicode::String const &player);
	ChatRequestLog(Archive::ReadIterator &);
	~ChatRequestLog();

	Unicode::String const &getPlayer() const;
	const unsigned int       getSequence     () const;

private:
	ChatRequestLog & operator = (const ChatRequestLog & rhs);
	ChatRequestLog(const ChatRequestLog & source);

	Archive::AutoVariable<Unicode::String> m_player;
	Archive::AutoVariable<unsigned int>     sequence;
};

//-----------------------------------------------------------------------

inline Unicode::String const &ChatRequestLog::getPlayer() const
{
	return m_player.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatRequestLog::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatRequestLog_H
