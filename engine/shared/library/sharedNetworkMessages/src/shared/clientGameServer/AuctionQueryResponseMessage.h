// ======================================================================
//
// AuctionQueryResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionQueryResponseMessage_H
#define	_AuctionQueryResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

struct AuctionQueryResponse_AuctionData
{
	int64 auctionId;
	std::string location;
	int64 ownerId;
	int32 minBid;
	int32 timer;
	int64 itemId;
	int32 soldFlag;
	int64 highBidderId;
	int32 itemType;
	int32 resourceContainerClassCrc;
	int16 itemQuantity;
	int32 itemTimer;
	int32 highBid;
	int32 highBidMaxProxy;
};

class AuctionQueryResponseMessage : public GameNetworkMessage
{
public:
	typedef AuctionQueryResponse_AuctionData AuctionData;
	typedef std::vector<AuctionData> AuctionDataVector;

	AuctionQueryResponseMessage(int requestId, int typeFlag, AuctionDataVector const &auctionData);
	explicit AuctionQueryResponseMessage(Archive::ReadIterator &source);

	~AuctionQueryResponseMessage();

public: // methods

	int                       getRequestId() const;
	int                       getTypeFlag() const;
	AuctionDataVector const & getAuctionData() const;

public: // types

private: 
	Archive::AutoVariable<int>       m_requestId;
	Archive::AutoVariable<int>       m_typeFlag;
	Archive::AutoArray<AuctionData>  m_auctionData;
};

// ----------------------------------------------------------------------

inline int AuctionQueryResponseMessage::getRequestId() const
{
	return m_requestId.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryResponseMessage::getTypeFlag() const
{
	return m_typeFlag.get();
}

// ----------------------------------------------------------------------

inline AuctionQueryResponseMessage::AuctionDataVector const & AuctionQueryResponseMessage::getAuctionData() const
{
	return m_auctionData.get();
}

// ----------------------------------------------------------------------

namespace Archive
{

	inline void get(ReadIterator &source, AuctionQueryResponse_AuctionData &target)
	{
		get(source, target.auctionId);
		get(source, target.location);
		get(source, target.ownerId);
		get(source, target.minBid);
		get(source, target.timer);
		get(source, target.itemId);
		get(source, target.soldFlag);
		get(source, target.highBidderId);
		get(source, target.itemType);
		get(source, target.resourceContainerClassCrc);
		get(source, target.itemQuantity);
		get(source, target.itemTimer);
		get(source, target.highBid);
		get(source, target.highBidMaxProxy);
	}

	inline void put(ByteStream &target, AuctionQueryResponse_AuctionData const &source)
	{
		put(target, source.auctionId);
		put(target, source.location);
		put(target, source.ownerId);
		put(target, source.minBid);
		put(target, source.timer);
		put(target, source.itemId);
		put(target, source.soldFlag);
		put(target, source.highBidderId);
		put(target, source.itemType);
		put(target, source.resourceContainerClassCrc);
		put(target, source.itemQuantity);
		put(target, source.itemTimer);
		put(target, source.highBid);
		put(target, source.highBidMaxProxy);
	}

}

// ----------------------------------------------------------------------

#endif // _AuctionQueryResponseMessage_H

