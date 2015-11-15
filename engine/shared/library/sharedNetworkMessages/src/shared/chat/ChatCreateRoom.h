// ChatCreateRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatCreateRoom_H
#define	_INCLUDED_ChatCreateRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatCreateRoom : public GameNetworkMessage
{
public:
	ChatCreateRoom(const unsigned int sequence, const std::string & ownerName, const std::string & roomName, const bool isModerated, const bool isPublic, const std::string & roomTitle);
	ChatCreateRoom(Archive::ReadIterator &);
	~ChatCreateRoom();

	const bool               getIsModerated  () const;
	const bool               getIsPublic     () const;
	const std::string &      getOwnerName    () const;
	const std::string &      getRoomName     () const;
	const unsigned int       getSequence     () const;
	const std::string &  getRoomTitle    () const;

private:
	ChatCreateRoom & operator = (const ChatCreateRoom & rhs);
	ChatCreateRoom(const ChatCreateRoom & source);

	Archive::AutoVariable<bool>             isPublic;
	Archive::AutoVariable<bool>             isModerated;
	Archive::AutoVariable<std::string>      ownerName;
	Archive::AutoVariable<std::string>      roomName;
	Archive::AutoVariable<std::string>      roomTitle;
	Archive::AutoVariable<unsigned int>     sequence;
};

//-----------------------------------------------------------------------

inline const bool ChatCreateRoom::getIsPublic() const
{
	return isPublic.get();
}

//-----------------------------------------------------------------------

inline const bool ChatCreateRoom::getIsModerated() const
{
	return isModerated.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatCreateRoom::getOwnerName() const
{
	return ownerName.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatCreateRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatCreateRoom::getRoomTitle() const
{
	return roomTitle.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatCreateRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatCreateRoom_H
