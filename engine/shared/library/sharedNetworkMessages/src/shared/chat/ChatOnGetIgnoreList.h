// ChatOnGetIgnoreList.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

#ifndef	_INCLUDED_ChatOnGetIgnoreList_H
#define	_INCLUDED_ChatOnGetIgnoreList_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include <vector>

//-----------------------------------------------------------------------

class ChatOnGetIgnoreList : public GameNetworkMessage
{
public:
	ChatOnGetIgnoreList(const NetworkId & character, const std::vector<ChatAvatarId> & ignoreList);
	ChatOnGetIgnoreList(Archive::ReadIterator & source);
	~ChatOnGetIgnoreList();

	const NetworkId &                  getCharacter      () const;
	const std::vector<ChatAvatarId> &  getIgnoreList     () const;

private:
	ChatOnGetIgnoreList & operator = (const ChatOnGetIgnoreList & rhs);
	ChatOnGetIgnoreList(const ChatOnGetIgnoreList & source);

	Archive::AutoVariable<NetworkId>   character;
	Archive::AutoArray<ChatAvatarId>      ignores;  
};

//-----------------------------------------------------------------------

inline const NetworkId & ChatOnGetIgnoreList::getCharacter () const
{
	return character.get();
}
//-----------------------------------------------------------------------

inline const std::vector<ChatAvatarId> & ChatOnGetIgnoreList::getIgnoreList () const
{
	return ignores.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnGetIgnoreList_H
