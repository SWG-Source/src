#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "CMCreateLocationMessage.h"


CMCreateLocationMessage::CMCreateLocationMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CMCreateLocationMessage"),
	m_locationId(),
	m_locationString(),
	m_ownerId(),
	m_salesTax(),
	m_bankId(),
	m_emptyDate(),
	m_lastAccessDate(),
	m_inactiveDate(),
	m_status(),
	m_searchEnabled(),
	m_entranceCharge()
{
	AutoByteStream::addVariable(m_locationId);
	AutoByteStream::addVariable(m_locationString);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_salesTax);
	AutoByteStream::addVariable(m_bankId);
	AutoByteStream::addVariable(m_emptyDate);
	AutoByteStream::addVariable(m_lastAccessDate);
	AutoByteStream::addVariable(m_inactiveDate);
	AutoByteStream::addVariable(m_status);
	AutoByteStream::addVariable(m_searchEnabled);
	AutoByteStream::addVariable(m_entranceCharge);
	unpack(source);
}

CMCreateLocationMessage::CMCreateLocationMessage(
	const NetworkId &    locationId,
	const std::string &  locationString,
	const NetworkId &    ownerId,
	int                  salesTax,
	const NetworkId &    bankId,
	int                  emptyDate,
	int                  lastAccessDate,
	int                  inactiveDate,
	int                  status,
	bool                 searchEnabled,
	int                  entranceCharge
):
	GameNetworkMessage("CMCreateLocationMessage"),
	m_locationId(locationId),
	m_locationString(locationString),
	m_ownerId(ownerId),
	m_salesTax(salesTax),
	m_bankId(bankId),
	m_emptyDate(emptyDate),
	m_lastAccessDate(lastAccessDate),
	m_inactiveDate(inactiveDate),
	m_status(status),
	m_searchEnabled(searchEnabled),
	m_entranceCharge(entranceCharge)
{
	AutoByteStream::addVariable(m_locationId);
	AutoByteStream::addVariable(m_locationString);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_salesTax);
	AutoByteStream::addVariable(m_bankId);
	AutoByteStream::addVariable(m_emptyDate);
	AutoByteStream::addVariable(m_lastAccessDate);
	AutoByteStream::addVariable(m_inactiveDate);
	AutoByteStream::addVariable(m_status);
	AutoByteStream::addVariable(m_searchEnabled);
	AutoByteStream::addVariable(m_entranceCharge);
}

CMCreateLocationMessage::~CMCreateLocationMessage()
{
}

