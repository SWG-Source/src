// ChatDisconnectAvatar.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatDisconnectAvatar_H
#define	_INCLUDED_ChatDisconnectAvatar_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatDisconnectAvatar : public GameNetworkMessage
{
public:
	explicit ChatDisconnectAvatar(const NetworkId & characterId);
	explicit ChatDisconnectAvatar(Archive::ReadIterator & source);
	~ChatDisconnectAvatar();

	const NetworkId &  getCharacterId  () const;

private:
	ChatDisconnectAvatar & operator = (const ChatDisconnectAvatar & rhs);
	ChatDisconnectAvatar(const ChatDisconnectAvatar & source);

	Archive::AutoVariable<NetworkId>  characterId;
};

//-----------------------------------------------------------------------

inline const NetworkId & ChatDisconnectAvatar::getCharacterId() const
{
	return characterId.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatDisconnectAvatar_H
