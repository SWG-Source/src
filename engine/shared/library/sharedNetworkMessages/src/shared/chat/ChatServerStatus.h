// ChatServerStatus.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatServerStatus_H
#define	_INCLUDED_ChatServerStatus_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatServerStatus : public GameNetworkMessage
{
public:
	explicit ChatServerStatus(const bool status);
	explicit ChatServerStatus(Archive::ReadIterator & source);
	~ChatServerStatus();

	const bool  getStatus  () const;

private:
	ChatServerStatus & operator = (const ChatServerStatus & rhs);
	ChatServerStatus(const ChatServerStatus & source);
	
	Archive::AutoVariable<bool>  status;
};

//-----------------------------------------------------------------------

inline const bool ChatServerStatus::getStatus() const
{
	return status.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatServerStatus_H
