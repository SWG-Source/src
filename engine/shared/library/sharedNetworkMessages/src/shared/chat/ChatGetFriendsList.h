// ChatGetFriendsList.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

#ifndef	_INCLUDED_ChatGetFriendsList_H
#define	_INCLUDED_ChatGetFriendsList_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include <vector>

//-----------------------------------------------------------------------

class ChatGetFriendsList : public GameNetworkMessage
{
public:
	ChatGetFriendsList(const ChatAvatarId & characterName);
	ChatGetFriendsList(Archive::ReadIterator & source);
	~ChatGetFriendsList();

	const ChatAvatarId &               getCharacterName  () const;

private:
	ChatGetFriendsList & operator = (const ChatGetFriendsList & rhs);
	ChatGetFriendsList(const ChatGetFriendsList & source);

	Archive::AutoVariable<ChatAvatarId>   characterName;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatGetFriendsList::getCharacterName () const
{
	return characterName.get();
}
//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatGetFriendsList_H
