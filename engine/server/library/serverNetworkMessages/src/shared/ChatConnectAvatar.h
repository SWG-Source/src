// ChatConnectAvatar.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatConnectAvatar_H
#define	_INCLUDED_ChatConnectAvatar_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include <string>

//-----------------------------------------------------------------------

class ChatConnectAvatar : public GameNetworkMessage
{
public:
	ChatConnectAvatar(const std::string & characterName, const NetworkId & characterId, const unsigned int stationId, bool isSecure, bool isSubscribed);
	explicit ChatConnectAvatar(Archive::ReadIterator & source);
	~ChatConnectAvatar();

	const NetworkId &    getCharacterId    () const;
	const std::string &  getCharacterName  () const;
	const bool           getIsSecure       () const;
	const bool           getIsSubscribed   () const;
	const unsigned int   getStationId      () const;

private:
	ChatConnectAvatar & operator = (const ChatConnectAvatar & rhs);
	ChatConnectAvatar(const ChatConnectAvatar & source);

	Archive::AutoVariable<NetworkId>    characterId;
	Archive::AutoVariable<std::string>  characterName;
	Archive::AutoVariable<unsigned int> stationId;
	Archive::AutoVariable<bool>         isSecure;
	Archive::AutoVariable<bool>         isSubscribed;
};

//-----------------------------------------------------------------------

inline const NetworkId & ChatConnectAvatar::getCharacterId() const
{
	return characterId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatConnectAvatar::getCharacterName() const
{
	return characterName.get();
}

//-----------------------------------------------------------------------

inline const bool ChatConnectAvatar::getIsSecure() const
{
	return isSecure.get();
}

//-----------------------------------------------------------------------

inline const bool ChatConnectAvatar::getIsSubscribed() const
{
	return isSubscribed.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatConnectAvatar::getStationId() const
{
	return stationId.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatConnectAvatar_H
