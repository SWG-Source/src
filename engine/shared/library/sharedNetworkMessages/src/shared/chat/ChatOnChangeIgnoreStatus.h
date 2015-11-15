// ChatOnChangeIgnoreStatus.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

#ifndef	_INCLUDED_ChatOnChangeIgnoreStatus_H
#define	_INCLUDED_ChatOnChangeIgnoreStatus_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatOnChangeIgnoreStatus : public GameNetworkMessage
{
public:
	ChatOnChangeIgnoreStatus(unsigned int sequence, const NetworkId & character, const ChatAvatarId & ignoreName, bool ignore, unsigned int result);
	ChatOnChangeIgnoreStatus(Archive::ReadIterator & source);
	~ChatOnChangeIgnoreStatus();

	const NetworkId &     getCharacter      () const;
	const ChatAvatarId &  getIgnoreName     () const;
	unsigned int          getSequence       () const;
	bool                  getIgnore         () const;
	unsigned int          getResultCode     () const;

private:
	ChatOnChangeIgnoreStatus & operator = (const ChatOnChangeIgnoreStatus & rhs);
	ChatOnChangeIgnoreStatus(const ChatOnChangeIgnoreStatus & source);

	Archive::AutoVariable<NetworkId>      character;
	Archive::AutoVariable<ChatAvatarId>   ignoreName;
	Archive::AutoVariable<unsigned int>   sequence;  
	Archive::AutoVariable<bool>           ignore;  
	Archive::AutoVariable<unsigned int>   resultCode;  
};

//-----------------------------------------------------------------------

inline const NetworkId & ChatOnChangeIgnoreStatus::getCharacter () const
{
	return character.get();
}
//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnChangeIgnoreStatus::getIgnoreName () const
{
	return ignoreName.get();
}
//-----------------------------------------------------------------------

inline unsigned int ChatOnChangeIgnoreStatus::getSequence () const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

inline bool ChatOnChangeIgnoreStatus::getIgnore () const
{
	return ignore.get();
}

//-----------------------------------------------------------------------

inline unsigned int ChatOnChangeIgnoreStatus::getResultCode () const
{
	return resultCode.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnChangeIgnoreStatus_H
