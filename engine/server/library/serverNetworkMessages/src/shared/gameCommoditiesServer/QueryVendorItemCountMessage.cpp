#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "QueryVendorItemCountMessage.h"

QueryVendorItemCountMessage::QueryVendorItemCountMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("QueryVendorItemCountMessage"),
	m_responseId(),
	m_trackId(),
	m_vendorId()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_vendorId);
	unpack(source);
}

QueryVendorItemCountMessage::QueryVendorItemCountMessage(
	const int responseId,
	const int trackId,
	const NetworkId &vendorId
):
	GameNetworkMessage("QueryVendorItemCountMessage"),
	m_responseId(responseId),
	m_trackId(trackId),
	m_vendorId(vendorId)
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_vendorId);
}

QueryVendorItemCountMessage::~QueryVendorItemCountMessage()
{
}
