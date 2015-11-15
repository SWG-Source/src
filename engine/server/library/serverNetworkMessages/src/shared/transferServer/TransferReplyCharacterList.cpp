// TransferReplyCharacterList.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/TransferReplyCharacterList.h"

//-----------------------------------------------------------------------

TransferReplyCharacterList::TransferReplyCharacterList(unsigned int track, unsigned int stationId, const AvatarList & avatarList) :
GameNetworkMessage("TransferReplyCharacterList"),
m_avatarList(avatarList),
m_stationId(stationId),
m_track(track)
{
	addVariable(m_avatarList);
	addVariable(m_stationId);
	addVariable(m_track);
}

//-----------------------------------------------------------------------

TransferReplyCharacterList::TransferReplyCharacterList(Archive::ReadIterator & source) :
GameNetworkMessage("TransferReplyCharacterList"),
m_avatarList(),
m_stationId(),
m_track()
{
	addVariable(m_avatarList);
	addVariable(m_stationId);
	addVariable(m_track);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

TransferReplyCharacterList::~TransferReplyCharacterList()
{
}

//-----------------------------------------------------------------------

const AvatarList & TransferReplyCharacterList::getAvatarList() const
{
	return m_avatarList.get();
}

//-----------------------------------------------------------------------

unsigned int TransferReplyCharacterList::getStationId() const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

unsigned int TransferReplyCharacterList::getTrack() const
{
	return m_track.get();
}

//-----------------------------------------------------------------------
