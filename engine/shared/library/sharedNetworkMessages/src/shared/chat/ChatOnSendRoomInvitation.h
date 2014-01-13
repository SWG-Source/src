// ChatOnSendRoomInvitation.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnSendRoomInvitation_H
#define	_INCLUDED_ChatOnSendRoomInvitation_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatOnSendRoomInvitation : public GameNetworkMessage
{
public:
	ChatOnSendRoomInvitation(const unsigned int sequence, const unsigned int result);
	explicit ChatOnSendRoomInvitation(Archive::ReadIterator & source);
	~ChatOnSendRoomInvitation();

	const unsigned int getResult    () const;
	const unsigned int getSequence  () const;

private:
	ChatOnSendRoomInvitation & operator = (const ChatOnSendRoomInvitation & rhs);
	ChatOnSendRoomInvitation(const ChatOnSendRoomInvitation & source);

	Archive::AutoVariable<unsigned int>  result;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const unsigned int ChatOnSendRoomInvitation::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnSendRoomInvitation::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnSendRoomInvitation_H
