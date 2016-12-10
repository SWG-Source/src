// ChatInviteGroupMembersToRoom.h
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved. 
// Author: Roy Fleck

#ifndef	_INCLUDED_ChatInviteGroupMembersToRoom_H
#define	_INCLUDED_ChatInviteGroupMembersToRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedFoundation/NetworkId.h"
#include <string>

//-----------------------------------------------------------------------

class ChatInviteGroupMembersToRoom : public GameNetworkMessage
{
public:
	ChatInviteGroupMembersToRoom(const NetworkId & invitor, const ChatAvatarId & groupLeaderId, const std::string & roomName, const std::vector<NetworkId> & members);
	explicit ChatInviteGroupMembersToRoom(Archive::ReadIterator & source);
	~ChatInviteGroupMembersToRoom();

	const NetworkId &                 getInvitorNetworkId() const;
	const ChatAvatarId &              getGroupLeaderId   () const;
	const std::string &               getRoomName        () const;
	const std::vector<NetworkId> & getInvitedMembers  () const;

private:
	ChatInviteGroupMembersToRoom & operator = (const ChatInviteGroupMembersToRoom & rhs);
	ChatInviteGroupMembersToRoom(const ChatInviteGroupMembersToRoom & source);

	Archive::AutoVariable<NetworkId>    invitorNetworkId;
	Archive::AutoVariable<ChatAvatarId> groupLeaderId;
	Archive::AutoVariable<std::string>  roomName;
	Archive::AutoArray<NetworkId>       invitedMembers;
};

//-----------------------------------------------------------------------

inline const NetworkId & ChatInviteGroupMembersToRoom::getInvitorNetworkId() const
{
	return invitorNetworkId.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatInviteGroupMembersToRoom::getGroupLeaderId() const
{
	return groupLeaderId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatInviteGroupMembersToRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatInviteGroupMembersToRoom_H
