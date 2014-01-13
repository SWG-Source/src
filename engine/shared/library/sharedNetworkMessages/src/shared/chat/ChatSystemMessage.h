// ChatSystemMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatSystemMessage_H
#define	_INCLUDED_ChatSystemMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class ChatSystemMessage : public GameNetworkMessage
{
public:
	enum Flags
	{
		PERSONAL      = 0x00,
		BROADCAST     = 0x01,
		F_chatBoxOnly = 0x02,
		F_quest = 0x04
	};

	ChatSystemMessage(const unsigned char flags, const Unicode::String & message, const Unicode::String & outOfBand);
	ChatSystemMessage(Archive::ReadIterator & source);
	~ChatSystemMessage();

	const unsigned char      getFlags      () const;
	const Unicode::String &  getMessage    () const;
	const Unicode::String &  getOutOfBand  () const;
private:
	ChatSystemMessage & operator = (const ChatSystemMessage & rhs);
	ChatSystemMessage(const ChatSystemMessage & source);

	Archive::AutoVariable<unsigned char>    flags;
	Archive::AutoVariable<Unicode::String>  message;
	Archive::AutoVariable<Unicode::String>  outOfBand;

};

//-----------------------------------------------------------------------

inline const unsigned char ChatSystemMessage::getFlags() const
{
	return flags.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatSystemMessage::getMessage() const
{
	return message.get();
}


//-----------------------------------------------------------------------

inline const Unicode::String & ChatSystemMessage::getOutOfBand() const
{
	return outOfBand.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatSystemMessage_H
