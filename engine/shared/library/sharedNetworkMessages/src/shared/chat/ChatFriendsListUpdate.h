// ChatFriendsListUpdate.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatFriendsListUpdate_H
#define	_INCLUDED_ChatFriendsListUpdate_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatFriendsListUpdate : public GameNetworkMessage
{
public:
	ChatFriendsListUpdate(const ChatAvatarId & characterName, const bool isOnline);
	ChatFriendsListUpdate(Archive::ReadIterator & source);
	~ChatFriendsListUpdate();

	const ChatAvatarId &  getCharacterName  () const;
	const bool            getIsOnline       () const;

private:
	ChatFriendsListUpdate & operator = (const ChatFriendsListUpdate & rhs);
	ChatFriendsListUpdate(const ChatFriendsListUpdate & source);

	Archive::AutoVariable<ChatAvatarId>  characterName;
	Archive::AutoVariable<bool>          isOnline;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatFriendsListUpdate::getCharacterName() const
{
	return characterName.get();
}

//-----------------------------------------------------------------------

inline const bool ChatFriendsListUpdate::getIsOnline() const
{
	return isOnline.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatFriendsListUpdate_H
