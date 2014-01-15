#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "CommoditiesLoadDoneMessage.h"

CommoditiesLoadDoneMessage::CommoditiesLoadDoneMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CommoditiesLoadDone"),
	m_auctionLocationsCount(),
	m_marketAuctionsCount(),
	m_marketAuctionAttributesCount(),
	m_marketAuctionBidsCount()
{
	AutoByteStream::addVariable(m_auctionLocationsCount);
	AutoByteStream::addVariable(m_marketAuctionsCount);
	AutoByteStream::addVariable(m_marketAuctionAttributesCount);
	AutoByteStream::addVariable(m_marketAuctionBidsCount);
	unpack(source);
}

CommoditiesLoadDoneMessage::CommoditiesLoadDoneMessage(
	const int            auctionLocationsCount,
	const int            marketAuctionsCount,
	const int            marketAuctionAttributesCount,
	const int            marketAuctionBidsCount
):
	GameNetworkMessage("CommoditiesLoadDone"),
	m_auctionLocationsCount(auctionLocationsCount),
	m_marketAuctionsCount(marketAuctionsCount),
	m_marketAuctionAttributesCount(marketAuctionAttributesCount),
	m_marketAuctionBidsCount(marketAuctionBidsCount)
{
	AutoByteStream::addVariable(m_auctionLocationsCount);
	AutoByteStream::addVariable(m_marketAuctionsCount);
	AutoByteStream::addVariable(m_marketAuctionAttributesCount);
	AutoByteStream::addVariable(m_marketAuctionBidsCount);
}

CommoditiesLoadDoneMessage::~CommoditiesLoadDoneMessage()
{
}
