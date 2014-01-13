// ChatDestroyRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatDestroyRoom_H
#define	_INCLUDED_ChatDestroyRoom_H

//-----------------------------------------------------------------------

class ChatDestroyRoom : public GameNetworkMessage
{
public:
	ChatDestroyRoom(const unsigned int sequence, const unsigned int roomId);
	ChatDestroyRoom(Archive::ReadIterator &);
	~ChatDestroyRoom();

	const unsigned int   getRoomId     () const;
	const unsigned int   getSequence     () const;

private:
	ChatDestroyRoom & operator = (const ChatDestroyRoom & rhs);
	ChatDestroyRoom(const ChatDestroyRoom & source);

	Archive::AutoVariable<unsigned int>  roomId;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const unsigned int ChatDestroyRoom::getRoomId() const
{
	return roomId.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatDestroyRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatDestroyRoom_H
