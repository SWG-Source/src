// ChatPutAvatarInRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatPutAvatarInRoom_H
#define	_INCLUDED_ChatPutAvatarInRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatPutAvatarInRoom : public GameNetworkMessage
{
public:
	ChatPutAvatarInRoom(const std::string & avatarName, const std::string & roomName, bool forceCreate, bool createPrivate);
	explicit ChatPutAvatarInRoom(Archive::ReadIterator & source);
	~ChatPutAvatarInRoom();

	const std::string &  getAvatarName  () const;
	const std::string &  getRoomName    () const;
	bool                 getForceCreate () const;
	bool                 getCreatePrivate() const;

private:
	ChatPutAvatarInRoom & operator = (const ChatPutAvatarInRoom & rhs);
	ChatPutAvatarInRoom(const ChatPutAvatarInRoom & source);

	Archive::AutoVariable<std::string>  m_avatarName;
	Archive::AutoVariable<std::string>  m_roomName;
	Archive::AutoVariable<bool>         m_forceCreate;
	Archive::AutoVariable<bool>         m_createPrivate;
};

//-----------------------------------------------------------------------

inline const std::string & ChatPutAvatarInRoom::getAvatarName() const
{
	return m_avatarName.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatPutAvatarInRoom::getRoomName() const
{
	return m_roomName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatPutAvatarInRoom_H
