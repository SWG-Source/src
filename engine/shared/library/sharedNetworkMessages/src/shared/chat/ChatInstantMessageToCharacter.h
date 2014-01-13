// ChatInstantMessageToCharacter.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatInstantMessageToCharacter_H
#define	_INCLUDED_ChatInstantMessageToCharacter_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatInstantMessageToCharacter : public GameNetworkMessage
{
public:
	ChatInstantMessageToCharacter(unsigned int sequence, const ChatAvatarId & characterName, const Unicode::String & message, const Unicode::String & outOfBand);
	ChatInstantMessageToCharacter(Archive::ReadIterator & source);
	~ChatInstantMessageToCharacter();

	const ChatAvatarId &     getCharacterName  () const;
	const Unicode::String &  getMessage        () const;
	const Unicode::String &  getOutOfBand      () const;
	const unsigned int       getSequence       () const;

private:
	ChatInstantMessageToCharacter & operator = (const ChatInstantMessageToCharacter & rhs);
	ChatInstantMessageToCharacter(const ChatInstantMessageToCharacter & source);
	
	Archive::AutoVariable<ChatAvatarId>     characterName;
	Archive::AutoVariable<Unicode::String>  message;
	Archive::AutoVariable<Unicode::String>  outOfBand;
	Archive::AutoVariable<unsigned int>     sequence;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatInstantMessageToCharacter::getCharacterName() const
{
	return characterName.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatInstantMessageToCharacter::getMessage() const
{
	return message.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatInstantMessageToCharacter::getOutOfBand() const
{
	return outOfBand.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatInstantMessageToCharacter::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatInstantMessageToCharacter_H
