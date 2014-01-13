// ChatRequestRoomList.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatRequestRoomList_H
#define	_INCLUDED_ChatRequestRoomList_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatRequestRoomList : public GameNetworkMessage
{
public:
	ChatRequestRoomList();
	ChatRequestRoomList(Archive::ReadIterator &);
	~ChatRequestRoomList();

private:
	ChatRequestRoomList & operator = (const ChatRequestRoomList & rhs);
	ChatRequestRoomList(const ChatRequestRoomList & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatRequestRoomList_H
