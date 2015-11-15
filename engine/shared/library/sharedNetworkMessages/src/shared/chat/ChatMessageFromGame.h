// ChatMessageFromGame.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatMessageFromGame_H
#define	_INCLUDED_ChatMessageFromGame_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatMessageFromGame : public GameNetworkMessage
{
public:
	enum MessageTypes
	{
		INSTANT,
		PERSISTENT,
		ROOM
	};

	ChatMessageFromGame(enum MessageTypes messageType, const std::string & from, const std::string & to, const Unicode::String & message, const Unicode::String & oob);
	ChatMessageFromGame(enum MessageTypes messageType, const std::string & from, const std::string & to, const Unicode::String & message, const Unicode::String & oob, const Unicode::String & subject);
	ChatMessageFromGame(enum MessageTypes messageType, const std::string & from, const Unicode::String & message, const Unicode::String & oob, const std::string & room);
	explicit ChatMessageFromGame(Archive::ReadIterator & source);
	~ChatMessageFromGame();


	const std::string &      getFrom         () const;
	const Unicode::String &  getMessage      () const;
	const MessageTypes       getMessageType  () const;
	const Unicode::String &  getOutOfBand    () const;
	const std::string &      getRoom         () const;
	const Unicode::String &  getSubject      () const;
	const std::string &      getTo           () const;

private:
	ChatMessageFromGame & operator = (const ChatMessageFromGame & rhs);
	ChatMessageFromGame(const ChatMessageFromGame & source);

	Archive::AutoVariable<std::string>      from;
	Archive::AutoVariable<Unicode::String>  message;
	Archive::AutoVariable<unsigned char>    messageType;
	Archive::AutoVariable<Unicode::String>  oob;
	Archive::AutoVariable<std::string>      room;
	Archive::AutoVariable<Unicode::String>  subject;
	Archive::AutoVariable<std::string>      to;
};

//-----------------------------------------------------------------------

inline const std::string &  ChatMessageFromGame::getFrom() const
{
	return from.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatMessageFromGame::getMessage() const
{
	return message.get();
}

//-----------------------------------------------------------------------

inline const ChatMessageFromGame::MessageTypes ChatMessageFromGame::getMessageType() const
{
	return static_cast<MessageTypes>(messageType.get());
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatMessageFromGame::getOutOfBand() const
{
	return oob.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatMessageFromGame::getRoom() const
{
	return room.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatMessageFromGame::getSubject() const
{
	return subject.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatMessageFromGame::getTo() const
{
	return to.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatMessageFromGame_H
