// ChatServerRoomOwner.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstChatServer.h"
#include "ChatServerRoomOwner.h"
#include "ChatInterface.h"
#include "ChatServer.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

ChatServerRoomOwner::ChatServerRoomOwner(const ChatRoom * r) :
roomID(r->getRoomID()),
roomData(),
lastUsed(time(0)),
lastPopulated(time(0)),
wideRoomAddress(r->getAddress().string_data, r->getAddress().string_length),
roomAddress(wideRoomAddress.data(), wideRoomAddress.size()),
flaggedForDelete(false),
groupChatRoom(false)
{
	if(r)
	{
		makeRoomData(*r, roomData);
		if (!_stricmp(ChatServer::getChatRoomNameNarrow(r).c_str(), ChatRoomTypes::ROOM_GROUP.c_str()) && !_stricmp(Unicode::wideToNarrow(ChatServer::toUnicodeString(r->getCreatorName())).c_str(), "SYSTEM"))
			groupChatRoom = true;
		else
			groupChatRoom = false;
	}
	else
	{
		groupChatRoom = false;
	}
}

//-----------------------------------------------------------------------

ChatServerRoomOwner::ChatServerRoomOwner(const ChatServerRoomOwner & source) :
roomID(source.roomID),
roomData(source.roomData),
lastUsed(time(0)),
lastPopulated(time(0)),
wideRoomAddress(source.wideRoomAddress),
roomAddress(wideRoomAddress.data(), wideRoomAddress.size()),
flaggedForDelete(false),
groupChatRoom(source.groupChatRoom)
{
}

//-----------------------------------------------------------------------

ChatServerRoomOwner::~ChatServerRoomOwner()
{
	roomID = 0;
	lastUsed = 0;
	lastPopulated = 0;
}

//-----------------------------------------------------------------------

ChatServerRoomOwner & ChatServerRoomOwner::operator = (const ChatServerRoomOwner & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
		roomID = rhs.roomID; 
		roomData = rhs.roomData;
		lastUsed = rhs.lastUsed;
		lastPopulated = rhs.lastPopulated;
		wideRoomAddress = rhs.wideRoomAddress;
		roomAddress = wideRoomAddress;
		flaggedForDelete = rhs.flaggedForDelete;
		groupChatRoom = rhs.groupChatRoom;
	}
	return *this;
}

//-----------------------------------------------------------------------

const ChatRoom * ChatServerRoomOwner::getRoom() const
{
	ChatInterface * const chatInterface = ChatServer::getChatInterface();
	if (chatInterface)
		return chatInterface->getRoom(roomID);

	return nullptr;
}

//-----------------------------------------------------------------------

const ChatRoomData & ChatServerRoomOwner::getRoomData() const
{
	return roomData;
}

//-----------------------------------------------------------------------

void ChatServerRoomOwner::updateRoomData()
{
	ChatRoom const * const room = getRoom();
	if(room)
	{
		makeRoomData(*room, roomData);
		if (!_stricmp(ChatServer::getChatRoomNameNarrow(room).c_str(), ChatRoomTypes::ROOM_GROUP.c_str()) && !_stricmp(Unicode::wideToNarrow(ChatServer::toUnicodeString(room->getCreatorName())).c_str(), "SYSTEM"))
			groupChatRoom = true;
		else
			groupChatRoom = false;
	}
	else
	{
		groupChatRoom = false;
	}
}

//-----------------------------------------------------------------------

void ChatServerRoomOwner::updateRoomData(const ChatRoom *newRoom)
{
	if(newRoom)
	{
		roomID = newRoom->getRoomID(),
		makeRoomData(*newRoom, roomData);
		if (!_stricmp(ChatServer::getChatRoomNameNarrow(newRoom).c_str(), ChatRoomTypes::ROOM_GROUP.c_str()) && !_stricmp(Unicode::wideToNarrow(ChatServer::toUnicodeString(newRoom->getCreatorName())).c_str(), "SYSTEM"))
			groupChatRoom = true;
		else
			groupChatRoom = false;
	}
}

//-----------------------------------------------------------------------

const ChatUnicodeString & ChatServerRoomOwner::getAddress() const
{
	return roomAddress;
}

//-----------------------------------------------------------------------

void ChatServerRoomOwner::touch()
{
	lastUsed = time(0);
}

//-----------------------------------------------------------------------

time_t ChatServerRoomOwner::getLastUsed() const
{
	return lastUsed;	
}

//-----------------------------------------------------------------------

bool ChatServerRoomOwner::isFlaggedForDelete() const
{
	return flaggedForDelete;	
}

//-----------------------------------------------------------------------

void ChatServerRoomOwner::flagForDelete() 
{
	flaggedForDelete = true;
}

//-----------------------------------------------------------------------

time_t ChatServerRoomOwner::getLastPopulated()
{
	return lastPopulated;	
}

//-----------------------------------------------------------------------
