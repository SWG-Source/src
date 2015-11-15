// ChatOnAddFriend.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnAddFriend_H
#define	_INCLUDED_ChatOnAddFriend_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatOnAddFriend : public GameNetworkMessage
{
public:
	ChatOnAddFriend(const unsigned int sequence, const unsigned int result);
	ChatOnAddFriend(Archive::ReadIterator & source);
	~ChatOnAddFriend();

	const unsigned int  getResult    () const;
	const unsigned int  getSequence  () const;

private:
	ChatOnAddFriend & operator = (const ChatOnAddFriend & rhs);
	ChatOnAddFriend(const ChatOnAddFriend & source);

	Archive::AutoVariable<unsigned int>  result;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const unsigned int ChatOnAddFriend::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnAddFriend::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnAddFriend_H

