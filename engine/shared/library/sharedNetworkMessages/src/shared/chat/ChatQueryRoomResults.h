// ChatQueryRoomResults.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatQueryRoomResults_H
#define	_INCLUDED_ChatQueryRoomResults_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatRoomData.h"
#include "sharedNetworkMessages/ChatRoomDataArchive.h"

//-----------------------------------------------------------------------

class ChatQueryRoomResults : public GameNetworkMessage
{
public:
	ChatQueryRoomResults(unsigned int sequence, const ChatRoomData & roomData, const std::vector<ChatAvatarId> & avatars, const std::vector<ChatAvatarId> & moderators, const std::vector<ChatAvatarId> & invitees, const std::vector<ChatAvatarId> & banned);
	ChatQueryRoomResults(Archive::ReadIterator &);
	~ChatQueryRoomResults();

	const std::vector<ChatAvatarId> &  getAvatars     () const;
	const std::vector<ChatAvatarId> &  getInvitees    () const;
	const std::vector<ChatAvatarId> &  getModerators  () const;
	const std::vector<ChatAvatarId> &  getBanned      () const;
	const ChatRoomData &               getRoomData    () const;
	const unsigned int                 getSequence    () const;

private:
	ChatQueryRoomResults & operator = (const ChatQueryRoomResults & rhs);
	ChatQueryRoomResults(const ChatQueryRoomResults & source);

	Archive::AutoArray<ChatAvatarId>     avatars;
	Archive::AutoArray<ChatAvatarId>     invitees;
	Archive::AutoArray<ChatAvatarId>     moderators;
	Archive::AutoArray<ChatAvatarId>     banned;
	Archive::AutoVariable<ChatRoomData>  roomData;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const std::vector<ChatAvatarId> & ChatQueryRoomResults::getAvatars() const
{
	return avatars.get();
}

//-----------------------------------------------------------------------

inline const std::vector<ChatAvatarId> & ChatQueryRoomResults::getInvitees() const
{
	return invitees.get();
}

//-----------------------------------------------------------------------

inline const std::vector<ChatAvatarId> & ChatQueryRoomResults::getModerators() const
{
	return moderators.get();
}

//-----------------------------------------------------------------------

inline const ChatRoomData & ChatQueryRoomResults::getRoomData() const
{
	return roomData.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatQueryRoomResults::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatQueryRoomResults_H
