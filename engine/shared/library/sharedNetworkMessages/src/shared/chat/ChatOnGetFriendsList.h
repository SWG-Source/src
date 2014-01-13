// ChatOnGetFriendsList.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

#ifndef	_INCLUDED_ChatOnGetFriendsList_H
#define	_INCLUDED_ChatOnGetFriendsList_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include <vector>

//-----------------------------------------------------------------------

class ChatOnGetFriendsList : public GameNetworkMessage
{
public:
	ChatOnGetFriendsList(const NetworkId & character, const std::vector<ChatAvatarId> & friends);
	ChatOnGetFriendsList(Archive::ReadIterator & source);
	~ChatOnGetFriendsList();

	const NetworkId &                  getCharacter      () const;
	const std::vector<ChatAvatarId> &  getFriends        () const;

private:
	ChatOnGetFriendsList & operator = (const ChatOnGetFriendsList & rhs);
	ChatOnGetFriendsList(const ChatOnGetFriendsList & source);

	Archive::AutoVariable<NetworkId>      character;
	Archive::AutoArray<ChatAvatarId>      friends;  
};

//-----------------------------------------------------------------------

inline const NetworkId & ChatOnGetFriendsList::getCharacter () const
{
	return character.get();
}
//-----------------------------------------------------------------------

inline const std::vector<ChatAvatarId> & ChatOnGetFriendsList::getFriends () const
{
	return friends.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnGetFriendsList_H
