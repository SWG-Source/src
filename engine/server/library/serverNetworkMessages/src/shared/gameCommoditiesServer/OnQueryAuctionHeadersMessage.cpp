#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnQueryAuctionHeadersMessage.h"

// ----------------------------------------------------------------------

OnQueryAuctionHeadersMessage::OnQueryAuctionHeadersMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnQueryAuctionHeadersMessage"),
m_responseId(),
m_trackId(),
m_playerId(),
m_queryType(),
m_numAuctions(),
m_auctionData(),
m_resultCode(),
m_queryOffset(),
m_hasMorePages()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_queryType);
	AutoByteStream::addVariable(m_numAuctions);
	AutoByteStream::addVariable(m_auctionData);
	AutoByteStream::addVariable(m_resultCode);
	AutoByteStream::addVariable(m_queryOffset);
	AutoByteStream::addVariable(m_hasMorePages);
	unpack(source);
}

// ----------------------------------------------------------------------

OnQueryAuctionHeadersMessage::OnQueryAuctionHeadersMessage(
		int responseId,
		int resultCode,
		const NetworkId & playerId,
		int queryType,
		const std::vector<AuctionDataHeader> & auctions,
		unsigned int queryOffset,
		bool hasMorePages
) :
GameNetworkMessage("OnQueryAuctionHeadersMessage"),
m_responseId(responseId),
m_trackId(0),
m_playerId(playerId),
m_queryType(queryType),
m_numAuctions(0),
m_auctionData(),
m_resultCode(resultCode),
m_queryOffset(queryOffset),
m_hasMorePages(hasMorePages)
{
  m_auctionData.set(auctions);
  m_numAuctions.set(auctions.size());
  AutoByteStream::addVariable(m_responseId);
  AutoByteStream::addVariable(m_trackId);
  AutoByteStream::addVariable(m_playerId);
  AutoByteStream::addVariable(m_queryType);
  AutoByteStream::addVariable(m_numAuctions);
  AutoByteStream::addVariable(m_auctionData);
  AutoByteStream::addVariable(m_resultCode);
  AutoByteStream::addVariable(m_queryOffset);
  AutoByteStream::addVariable(m_hasMorePages);
}

// ----------------------------------------------------------------------

OnQueryAuctionHeadersMessage::~OnQueryAuctionHeadersMessage()
{

}

