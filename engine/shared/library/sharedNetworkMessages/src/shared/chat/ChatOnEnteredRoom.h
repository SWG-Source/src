// ChatOnEnteredRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnEnteredRoom_H
#define	_INCLUDED_ChatOnEnteredRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatOnEnteredRoom : public GameNetworkMessage
{
public:
	ChatOnEnteredRoom(unsigned int sequence, const unsigned int result, const unsigned int roomId, const ChatAvatarId & characterName);
	ChatOnEnteredRoom(Archive::ReadIterator &);
	~ChatOnEnteredRoom();

	const ChatAvatarId &  getCharacterName  () const;
	const unsigned int    getResult         () const;
	const unsigned int    getRoomId         () const;
	const unsigned int    getSequence       () const;

private:
	ChatOnEnteredRoom & operator = (const ChatOnEnteredRoom & rhs);
	ChatOnEnteredRoom(const ChatOnEnteredRoom & source);

	Archive::AutoVariable<ChatAvatarId>  characterName;
	Archive::AutoVariable<unsigned int>  result;
	Archive::AutoVariable<unsigned int>  roomId;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnEnteredRoom::getCharacterName() const
{
	return characterName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnEnteredRoom::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnEnteredRoom::getRoomId() const
{
	return roomId.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnEnteredRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnEnteredRoom_H
