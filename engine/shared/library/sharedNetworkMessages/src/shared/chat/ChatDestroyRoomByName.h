// ChatDestroyRoomByName.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatDestroyRoomByName_H
#define	_INCLUDED_ChatDestroyRoomByName_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatDestroyRoomByName : public GameNetworkMessage
{
public:
	explicit ChatDestroyRoomByName(const std::string & roomPath);
	explicit ChatDestroyRoomByName(Archive::ReadIterator & ri);
	~ChatDestroyRoomByName();

	const std::string &  getRoomPath  () const;

private:
	ChatDestroyRoomByName & operator = (const ChatDestroyRoomByName & rhs);
	ChatDestroyRoomByName(const ChatDestroyRoomByName & source);

	Archive::AutoVariable<std::string>  roomPath;
};

//-----------------------------------------------------------------------

inline const std::string & ChatDestroyRoomByName::getRoomPath() const
{
	return roomPath.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatDestroyRoomByName_H
