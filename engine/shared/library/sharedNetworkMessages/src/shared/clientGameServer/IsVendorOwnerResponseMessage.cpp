// ======================================================================
//
// IsVendorOwnerResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/IsVendorOwnerResponseMessage.h"

// ======================================================================

const char * const IsVendorOwnerResponseMessage::MessageType = "IsVendorOwnerResponseMessage";

//----------------------------------------------------------------------

IsVendorOwnerResponseMessage::IsVendorOwnerResponseMessage(
	const NetworkId &containerId,
	const std::string &marketName,
	IsVendorOwnerResponseMessage::VendorOwnerResult ownerResult, 
	AuctionResult result,
	uint16 maxPageSize
) :
	GameNetworkMessage(MessageType),
	m_ownerResult(static_cast<int>(ownerResult)),
	m_result(result),
	m_containerId(containerId),
	m_marketName(marketName),
	m_maxPageSize(maxPageSize)
{
	AutoByteStream::addVariable(m_ownerResult);
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_containerId);
	AutoByteStream::addVariable(m_marketName);
	AutoByteStream::addVariable(m_maxPageSize);
}

// ======================================================================

IsVendorOwnerResponseMessage::IsVendorOwnerResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage(MessageType),
	m_ownerResult(0),
	m_result(0),
	m_containerId(),
	m_marketName(),
	m_maxPageSize(0)
{
	AutoByteStream::addVariable(m_ownerResult);
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_containerId);
	AutoByteStream::addVariable(m_marketName);
	AutoByteStream::addVariable(m_maxPageSize);
	unpack(source);
}

// ======================================================================

IsVendorOwnerResponseMessage::~IsVendorOwnerResponseMessage()
{
}
