// ChatGetIgnoreList.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

#ifndef	_INCLUDED_ChatGetIgnoreList_H
#define	_INCLUDED_ChatGetIgnoreList_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include <vector>

//-----------------------------------------------------------------------

class ChatGetIgnoreList : public GameNetworkMessage
{
public:
	ChatGetIgnoreList(const ChatAvatarId & characterName);
	ChatGetIgnoreList(Archive::ReadIterator & source);
	~ChatGetIgnoreList();

	const ChatAvatarId &               getCharacterName  () const;

private:
	ChatGetIgnoreList & operator = (const ChatGetIgnoreList & rhs);
	ChatGetIgnoreList(const ChatGetIgnoreList & source);

	Archive::AutoVariable<ChatAvatarId>   characterName;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatGetIgnoreList::getCharacterName () const
{
	return characterName.get();
}
//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatGetIgnoreList_H
