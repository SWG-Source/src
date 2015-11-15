// ChatRoomList.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatRoomList_H
#define	_INCLUDED_ChatRoomList_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatRoomData.h"
#include "sharedNetworkMessages/ChatRoomDataArchive.h"
#include <vector>

//-----------------------------------------------------------------------

class ChatRoomList : public GameNetworkMessage
{
public:
	explicit ChatRoomList(const std::vector<ChatRoomData> & roomData);
	explicit ChatRoomList(Archive::ReadIterator &);
	~ChatRoomList();

	const std::vector<ChatRoomData> &  getRoomData      () const;

private:
	ChatRoomList & operator = (const ChatRoomList & rhs);
	ChatRoomList(const ChatRoomList & source);

	Archive::AutoArray<ChatRoomData>  roomData;
};

//-----------------------------------------------------------------------

inline const std::vector<ChatRoomData> & ChatRoomList::getRoomData() const
{
	return roomData.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatRoomList_H
