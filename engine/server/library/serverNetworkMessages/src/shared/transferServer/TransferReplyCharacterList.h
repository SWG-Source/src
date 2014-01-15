// TransferReplyCharacterList.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TransferReplyCharacterList_H
#define	_INCLUDED_TransferReplyCharacterList_H

//-----------------------------------------------------------------------
#include "serverNetworkMessages/AvatarList.h"
#include "serverNetworkMessages/AvatarListArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <string>

//-----------------------------------------------------------------------

class TransferReplyCharacterList : public GameNetworkMessage
{
public:
	explicit TransferReplyCharacterList(unsigned int track, unsigned int fromStationId, const AvatarList & avatarList);
	explicit TransferReplyCharacterList(Archive::ReadIterator & source);
	~TransferReplyCharacterList();

	const AvatarList &   getAvatarList     () const;
	unsigned int         getStationId      () const;
	unsigned int         getTrack          () const;

private:
	TransferReplyCharacterList & operator = (const TransferReplyCharacterList & rhs);
	TransferReplyCharacterList(const TransferReplyCharacterList & source);

	Archive::AutoVariable<AvatarList>    m_avatarList;
	Archive::AutoVariable<unsigned int>  m_stationId;
	Archive::AutoVariable<unsigned int>  m_track;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TransferReplyCharacterList_H
