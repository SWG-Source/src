// ChatRemoveFriend.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatRemoveFriend_H
#define	_INCLUDED_ChatRemoveFriend_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatRemoveFriend : public GameNetworkMessage
{
public:
	ChatRemoveFriend(const ChatAvatarId & characterName);
	ChatRemoveFriend(Archive::ReadIterator & source);
	~ChatRemoveFriend();

	const ChatAvatarId &  getCharacterName  () const;

private:
	ChatRemoveFriend & operator = (const ChatRemoveFriend & rhs);
	ChatRemoveFriend(const ChatRemoveFriend & source);

	Archive::AutoVariable<ChatAvatarId>   characterName;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatRemoveFriend::getCharacterName () const
{
	return characterName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatRemoveFriend_H
