// ChatChangeIgnoreStatus.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

#ifndef	_INCLUDED_ChatChangeIgnoreStatus_H
#define	_INCLUDED_ChatChangeIgnoreStatus_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatChangeIgnoreStatus : public GameNetworkMessage
{
public:
	ChatChangeIgnoreStatus(unsigned int sequence, const ChatAvatarId & characterName, const ChatAvatarId & friendName, bool ignore);
	ChatChangeIgnoreStatus(Archive::ReadIterator & source);
	~ChatChangeIgnoreStatus();

	const ChatAvatarId &  getCharacterName  () const;
	const ChatAvatarId &  getIgnoreName     () const;
	unsigned int          getSequence       () const;
	bool                  getIgnore         () const;

private:
	ChatChangeIgnoreStatus & operator = (const ChatChangeIgnoreStatus & rhs);
	ChatChangeIgnoreStatus(const ChatChangeIgnoreStatus & source);

	Archive::AutoVariable<ChatAvatarId>   characterName;
	Archive::AutoVariable<ChatAvatarId>   friendName;
	Archive::AutoVariable<unsigned int>   sequence;  
	Archive::AutoVariable<bool>           ignore;  
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatChangeIgnoreStatus::getCharacterName () const
{
	return characterName.get();
}
//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatChangeIgnoreStatus::getIgnoreName () const
{
	return friendName.get();
}
//-----------------------------------------------------------------------

inline unsigned int ChatChangeIgnoreStatus::getSequence () const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

inline bool ChatChangeIgnoreStatus::getIgnore () const
{
	return ignore.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatChangeIgnoreStatus_H
