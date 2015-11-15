// ChatOnBanAvatarFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnBanAvatarFromRoom_H
#define	_INCLUDED_ChatOnBanAvatarFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

//-----------------------------------------------------------------------

class ChatOnBanAvatarFromRoom : public GameNetworkMessage
{
public:
	ChatOnBanAvatarFromRoom(const unsigned int sequence, const unsigned int result, const std::string &roomName, const ChatAvatarId &banner, const ChatAvatarId &bannee );
	ChatOnBanAvatarFromRoom(Archive::ReadIterator & source);
	~ChatOnBanAvatarFromRoom();

	const std::string  & getRoomName  () const;
	const ChatAvatarId & getBanner    () const;
	const ChatAvatarId & getBannee    () const;
	const unsigned int   getResult    () const;
	const unsigned int   getSequence  () const;

private:
	ChatOnBanAvatarFromRoom & operator = (const ChatOnBanAvatarFromRoom & rhs);
	ChatOnBanAvatarFromRoom(const ChatOnBanAvatarFromRoom & source);

	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<ChatAvatarId>  banner;
	Archive::AutoVariable<ChatAvatarId>  bannee;
	Archive::AutoVariable<unsigned int>  result;
	Archive::AutoVariable<unsigned int>  sequence;

};

//-----------------------------------------------------------------------

inline const std::string & ChatOnBanAvatarFromRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnBanAvatarFromRoom::getBanner() const
{
	return banner.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnBanAvatarFromRoom::getBannee() const
{
	return bannee.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnBanAvatarFromRoom::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnBanAvatarFromRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnBanAvatarFromRoom_H

