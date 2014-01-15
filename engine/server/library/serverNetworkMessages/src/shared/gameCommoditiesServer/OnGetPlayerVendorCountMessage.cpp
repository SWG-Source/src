#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnGetPlayerVendorCountMessage.h"

OnGetPlayerVendorCountMessage::OnGetPlayerVendorCountMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("OnGetPlayerVendorCountMessage"),
	m_responseId(),
	m_trackId(),
	m_playerId(),
	m_vendorCount(),
	m_vendorList()
{
 	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_vendorCount);
	AutoByteStream::addVariable(m_vendorList);
	unpack(source);
}

OnGetPlayerVendorCountMessage::OnGetPlayerVendorCountMessage(
	int responseId,
	const NetworkId & playerId,
	int vendorCount,
	const std::vector<NetworkId> & vendorList
) :
	GameNetworkMessage("OnGetPlayerVendorCountMessage"),
	m_responseId(responseId),
	m_trackId(0),
	m_playerId(playerId),
	m_vendorCount(vendorCount),
	m_vendorList(vendorList)
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_vendorCount);
	AutoByteStream::addVariable(m_vendorList);
}

OnGetPlayerVendorCountMessage::~OnGetPlayerVendorCountMessage()
{
}

