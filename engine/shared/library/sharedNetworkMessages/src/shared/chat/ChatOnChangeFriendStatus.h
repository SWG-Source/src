// ChatOnChangeFriendStatus.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

#ifndef	_INCLUDED_ChatOnChangeFriendStatus_H
#define	_INCLUDED_ChatOnChangeFriendStatus_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatOnChangeFriendStatus : public GameNetworkMessage
{
public:
	ChatOnChangeFriendStatus(unsigned int sequence, const NetworkId & character, const ChatAvatarId & friendName, bool add, unsigned int result);
	ChatOnChangeFriendStatus(Archive::ReadIterator & source);
	~ChatOnChangeFriendStatus();

	const NetworkId &     getCharacter      () const;
	const ChatAvatarId &  getFriendName     () const;
	unsigned int          getSequence       () const;
	bool                  getAdd            () const;
	unsigned int          getResultCode     () const;

private:
	ChatOnChangeFriendStatus & operator = (const ChatOnChangeFriendStatus & rhs);
	ChatOnChangeFriendStatus(const ChatOnChangeFriendStatus & source);

	Archive::AutoVariable<NetworkId>      character;
	Archive::AutoVariable<ChatAvatarId>   friendName;
	Archive::AutoVariable<unsigned int>   sequence;  
	Archive::AutoVariable<bool>           add;  
	Archive::AutoVariable<unsigned int>   resultCode;  
};

//-----------------------------------------------------------------------

inline const NetworkId & ChatOnChangeFriendStatus::getCharacter () const
{
	return character.get();
}
//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnChangeFriendStatus::getFriendName () const
{
	return friendName.get();
}
//-----------------------------------------------------------------------

inline unsigned int ChatOnChangeFriendStatus::getSequence () const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

inline bool ChatOnChangeFriendStatus::getAdd () const
{
	return add.get();
}

//-----------------------------------------------------------------------

inline unsigned int ChatOnChangeFriendStatus::getResultCode () const
{
	return resultCode.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnChangeFriendStatus_H
