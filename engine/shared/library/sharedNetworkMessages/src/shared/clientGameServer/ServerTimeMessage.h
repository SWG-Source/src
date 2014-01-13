// ServerTimeMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ServerTimeMessage_H
#define	_INCLUDED_ServerTimeMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ServerTimeMessage : public GameNetworkMessage
{
public:
	explicit ServerTimeMessage(const int64 timeSeconds);
	explicit ServerTimeMessage(Archive::ReadIterator & source);
	~ServerTimeMessage();

	const int64  getTimeSeconds  () const;

private:
	ServerTimeMessage & operator = (const ServerTimeMessage & rhs);
	ServerTimeMessage(const ServerTimeMessage & source);

	Archive::AutoVariable<int64>  timeSeconds;
};

//-----------------------------------------------------------------------

inline const int64 ServerTimeMessage::getTimeSeconds()  const
{
	return timeSeconds.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ServerTimeMessage_H
