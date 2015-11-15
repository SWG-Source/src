#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "CreateVendorMarketMessage.h"

CreateVendorMarketMessage::CreateVendorMarketMessage(Archive::ReadIterator & source) :
GameNetworkMessage("CreateVendorMarketMessage"),
m_responseId(),
m_trackId(),
m_ownerId(),
m_location(),
m_playerVendorLimit(),
m_entranceCharge()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_playerVendorLimit);
	AutoByteStream::addVariable(m_entranceCharge);
	unpack(source);
}

CreateVendorMarketMessage::CreateVendorMarketMessage(
	int                 responseId,
	const NetworkId &   ownerId,
	const std::string & location,
	int                 playerVendorLimit,
	int                 entranceCharge
) :
GameNetworkMessage("CreateVendorMarketMessage"),
m_responseId(responseId),
m_trackId(0),
m_ownerId(ownerId),
m_location(location),
m_playerVendorLimit(playerVendorLimit),
m_entranceCharge(entranceCharge)
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_playerVendorLimit);
	AutoByteStream::addVariable(m_entranceCharge);
}

CreateVendorMarketMessage::~CreateVendorMarketMessage()
{
}

