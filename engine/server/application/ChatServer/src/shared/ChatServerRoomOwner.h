// ChatServerRoomOwner.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatServerRoomOwner_H
#define	_INCLUDED_ChatServerRoomOwner_H

//-----------------------------------------------------------------------

#include "ChatAPI/ChatRoom.h"
#include "sharedNetworkMessages/ChatRoomData.h"

using namespace ChatSystem;

//-----------------------------------------------------------------------

class ChatServerRoomOwner
{
public:
	explicit ChatServerRoomOwner(const ChatRoom * room);
	virtual ~ChatServerRoomOwner();
	ChatServerRoomOwner & operator = (const ChatServerRoomOwner & rhs);
	ChatServerRoomOwner(const ChatServerRoomOwner & source);

	const ChatRoom *      getRoom         () const;
	const ChatRoomData &  getRoomData     () const;
	void                  updateRoomData  ();
	void                  updateRoomData  (const ChatRoom *room);
	void                  touch           ();
	time_t                getLastUsed     () const;
	time_t                getLastPopulated();
	const ChatUnicodeString &   getAddress      () const;
	void                  flagForDelete();
	bool                  isFlaggedForDelete() const;
	bool                  isGroupChatRoom() const;

private:

	unsigned           roomID;
	ChatRoomData       roomData; 
	time_t             lastUsed;
	time_t             lastPopulated;
	Unicode::String    wideRoomAddress;
	ChatUnicodeString  roomAddress;
	bool               flaggedForDelete;
	bool               groupChatRoom;
};//lint !e1712 default constructor not defined for class

//-----------------------------------------------------------------------

inline bool ChatServerRoomOwner::isGroupChatRoom() const
{
	return groupChatRoom;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatServerRoomOwner_H
