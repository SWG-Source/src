// ======================================================================
//
// CreateAuctionResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CreateAuctionResponseMessage.h"

// ======================================================================

const char * const CreateAuctionResponseMessage::MessageType = "CreateAuctionResponseMessage";

//----------------------------------------------------------------------

CreateAuctionResponseMessage::CreateAuctionResponseMessage(NetworkId itemId, AuctionResult result, std::string const &itemRestrictedRejectionMessage) :
	GameNetworkMessage(MessageType),
	m_itemId(itemId),
	m_result(result),
	m_itemRestrictedRejectionMessage(itemRestrictedRejectionMessage)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_itemRestrictedRejectionMessage);
}

// ======================================================================

CreateAuctionResponseMessage::CreateAuctionResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage(MessageType),
	m_itemId(),
	m_result(),
	m_itemRestrictedRejectionMessage()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_itemRestrictedRejectionMessage);
	unpack(source);
}

// ======================================================================

CreateAuctionResponseMessage::~CreateAuctionResponseMessage()
{
}
