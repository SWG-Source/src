// ChatOnUnbanAvatarFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnUnbanAvatarFromRoom_H
#define	_INCLUDED_ChatOnUnbanAvatarFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

//-----------------------------------------------------------------------

class ChatOnUnbanAvatarFromRoom : public GameNetworkMessage
{
public:
	ChatOnUnbanAvatarFromRoom(const unsigned int sequence, const unsigned int result, const std::string &roomName, const ChatAvatarId &banner, const ChatAvatarId &bannee );
	ChatOnUnbanAvatarFromRoom(Archive::ReadIterator & source);
	~ChatOnUnbanAvatarFromRoom();

	const std::string  & getRoomName  () const;
	const ChatAvatarId & getUnbanner    () const;
	const ChatAvatarId & getUnbannee    () const;
	const unsigned int   getResult    () const;
	const unsigned int   getSequence  () const;

private:
	ChatOnUnbanAvatarFromRoom & operator = (const ChatOnUnbanAvatarFromRoom & rhs);
	ChatOnUnbanAvatarFromRoom(const ChatOnUnbanAvatarFromRoom & source);

	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<ChatAvatarId>  banner;
	Archive::AutoVariable<ChatAvatarId>  bannee;
	Archive::AutoVariable<unsigned int>  result;
	Archive::AutoVariable<unsigned int>  sequence;

};

//-----------------------------------------------------------------------

inline const std::string & ChatOnUnbanAvatarFromRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnUnbanAvatarFromRoom::getUnbanner() const
{
	return banner.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnUnbanAvatarFromRoom::getUnbannee() const
{
	return bannee.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnUnbanAvatarFromRoom::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnUnbanAvatarFromRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnUnbanAvatarFromRoom_H

