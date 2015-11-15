// ChatChangeFriendStatus.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatChangeFriendStatus_H
#define	_INCLUDED_ChatChangeFriendStatus_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatChangeFriendStatus : public GameNetworkMessage
{
public:
	ChatChangeFriendStatus(unsigned int sequence, const ChatAvatarId & characterName, const ChatAvatarId & friendName, bool add);
	ChatChangeFriendStatus(Archive::ReadIterator & source);
	~ChatChangeFriendStatus();

	const ChatAvatarId &  getCharacterName  () const;
	const ChatAvatarId &  getFriendName     () const;
	unsigned int          getSequence       () const;
	bool                  getAdd            () const;

private:
	ChatChangeFriendStatus & operator = (const ChatChangeFriendStatus & rhs);
	ChatChangeFriendStatus(const ChatChangeFriendStatus & source);

	Archive::AutoVariable<ChatAvatarId>   characterName;
	Archive::AutoVariable<ChatAvatarId>   friendName;
	Archive::AutoVariable<unsigned int>   sequence;  
	Archive::AutoVariable<bool>           add;  
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatChangeFriendStatus::getCharacterName () const
{
	return characterName.get();
}
//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatChangeFriendStatus::getFriendName () const
{
	return friendName.get();
}
//-----------------------------------------------------------------------

inline unsigned int ChatChangeFriendStatus::getSequence () const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

inline bool ChatChangeFriendStatus::getAdd () const
{
	return add.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatChangeFriendStatus_H
