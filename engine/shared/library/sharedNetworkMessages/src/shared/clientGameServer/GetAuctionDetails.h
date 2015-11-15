// ======================================================================
//
// GetAuctionDetails.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetAuctionDetails_H
#define	_GetAuctionDetails_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------


class GetAuctionDetails : public GameNetworkMessage
{
public:

	GetAuctionDetails(const NetworkId & item);
	explicit GetAuctionDetails(Archive::ReadIterator &source);

	~GetAuctionDetails();

public: // methods

	NetworkId                          getItem() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId>         m_itemId;           //item
};

// ----------------------------------------------------------------------

inline NetworkId GetAuctionDetails::getItem() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

#endif // _GetAuctionDetails_H

