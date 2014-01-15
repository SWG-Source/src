#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnQueryVendorItemCountReplyMessage.h"

OnQueryVendorItemCountReplyMessage::OnQueryVendorItemCountReplyMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("OnQueryVendorItemCountReplyMessage"),
	m_responseId(),
	m_vendorId(),
	m_vendorItemCount(),
	m_searchEnabled()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_vendorItemCount);
	AutoByteStream::addVariable(m_searchEnabled);
	unpack(source);
}

OnQueryVendorItemCountReplyMessage::OnQueryVendorItemCountReplyMessage(
	const int responseId,
	const NetworkId &vendorId,
	const int vendorItemCount,
	const bool searchEnabled
):
	GameNetworkMessage("OnQueryVendorItemCountReplyMessage"),
	m_responseId(responseId),
	m_vendorId(vendorId),
	m_vendorItemCount(vendorItemCount),
	m_searchEnabled(searchEnabled)
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_vendorItemCount);
	AutoByteStream::addVariable(m_searchEnabled);
}

OnQueryVendorItemCountReplyMessage::~OnQueryVendorItemCountReplyMessage()
{
}
