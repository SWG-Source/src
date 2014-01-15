#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "DeleteAuctionLocationMessage.h"

DeleteAuctionLocationMessage::DeleteAuctionLocationMessage(Archive::ReadIterator & source) :
GameNetworkMessage("DeleteAuctionLocationMessage"),
m_responseId(),
m_trackId(),
m_locationId(),
m_whoRequested()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_locationId);
	AutoByteStream::addVariable(m_whoRequested);
	unpack(source);
}

DeleteAuctionLocationMessage::DeleteAuctionLocationMessage(
		int responseId,
		const NetworkId & locationId,
		const std::string & whoRequested
) :
GameNetworkMessage("DeleteAuctionLocationMessage"),
m_responseId(responseId),
m_trackId(0),
m_locationId(locationId),
m_whoRequested(whoRequested)
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_locationId);
	AutoByteStream::addVariable(m_whoRequested);
}

DeleteAuctionLocationMessage::~DeleteAuctionLocationMessage()
{
}

