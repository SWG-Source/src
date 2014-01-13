// ChatAddFriend.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatAddFriend_H
#define	_INCLUDED_ChatAddFriend_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatAddFriend : public GameNetworkMessage
{
public:
	ChatAddFriend(const unsigned int sequence, const ChatAvatarId & characterName);
	ChatAddFriend(Archive::ReadIterator & source);
	~ChatAddFriend();

	const ChatAvatarId &  getCharacterName  () const;
	const unsigned int    getSequence       () const;

private:
	ChatAddFriend & operator = (const ChatAddFriend & rhs);
	ChatAddFriend(const ChatAddFriend & source);

	Archive::AutoVariable<ChatAvatarId>  characterName;
	Archive::AutoVariable<unsigned int>  sequence;  
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatAddFriend::getCharacterName () const
{
	return characterName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatAddFriend::getSequence () const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatAddFriend_H
