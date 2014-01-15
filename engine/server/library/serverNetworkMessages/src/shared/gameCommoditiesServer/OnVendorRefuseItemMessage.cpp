#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnVendorRefuseItemMessage.h"

OnVendorRefuseItemMessage::OnVendorRefuseItemMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnVendorRefuseItemMessage"),
m_responseId(),
m_trackId(),
m_itemId(),
m_vendorId(),
m_itemOwnerId(),
m_resultCode()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_itemOwnerId);
	AutoByteStream::addVariable(m_resultCode);
	unpack(source);
}

OnVendorRefuseItemMessage::OnVendorRefuseItemMessage(
		int responseId,
		int resultCode,
		const NetworkId & itemId,
		const NetworkId & vendorId,
		const NetworkId & itemOwnerId
) :
GameNetworkMessage("OnVendorRefuseItemMessage"),
m_responseId(responseId),
m_trackId(0),
m_itemId(itemId),
m_vendorId(vendorId),
m_itemOwnerId(itemOwnerId),
m_resultCode(resultCode)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_itemOwnerId);
	AutoByteStream::addVariable(m_resultCode);
}

OnVendorRefuseItemMessage::~OnVendorRefuseItemMessage()
{
}

