// GrantSkill.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "SecureTradeMessages.h"

#include "sharedFoundation/NetworkId.h"


//========================================================================

BeginTradeMessage::BeginTradeMessage(const NetworkId & player) :
GameNetworkMessage("BeginTradeMessage"),
m_player(player)
{
	addVariable(m_player);
}

BeginTradeMessage::BeginTradeMessage(Archive::ReadIterator & source) :
GameNetworkMessage("BeginTradeMessage"),
m_player(NetworkId::cms_invalid)
{
	addVariable(m_player);
	unpack(source);
}

BeginTradeMessage::~BeginTradeMessage()
{
}

//========================================================================


AbortTradeMessage::AbortTradeMessage() : 
GameNetworkMessage("AbortTradeMessage")
{
}

AbortTradeMessage::AbortTradeMessage(Archive::ReadIterator & source) :
GameNetworkMessage("AbortTradeMessage")
{
	UNREF(source);
}

AbortTradeMessage::~AbortTradeMessage()
{
}

//========================================================================

AddItemMessage::AddItemMessage(const NetworkId & object) :
GameNetworkMessage("AddItemMessage"),
m_object(object)
{
	addVariable(m_object);
}

AddItemMessage::AddItemMessage(Archive::ReadIterator & source) :
GameNetworkMessage("AddItemMessage"),
m_object(NetworkId::cms_invalid)
{
	addVariable(m_object);
	unpack(source);
}

AddItemMessage::~AddItemMessage()
{
}

//========================================================================

const char * const AddItemFailedMessage::MessageType = "AddItemFailedMessage";

//----------------------------------------------------------------------

AddItemFailedMessage::AddItemFailedMessage(const NetworkId & object) :
GameNetworkMessage(MessageType),
m_object(object)
{
	addVariable(m_object);
}

//----------------------------------------------------------------------

AddItemFailedMessage::AddItemFailedMessage(Archive::ReadIterator & source) :
GameNetworkMessage(MessageType),
m_object(NetworkId::cms_invalid)
{
	addVariable(m_object);
	unpack(source);
}

AddItemFailedMessage::~AddItemFailedMessage()
{
}

//========================================================================


RemoveItemMessage::RemoveItemMessage(const NetworkId & object) :
GameNetworkMessage("RemoveItemMessage"),
m_object(object)
{
	addVariable(m_object);
}

RemoveItemMessage::RemoveItemMessage(Archive::ReadIterator & source) :
GameNetworkMessage("RemoveItemMessage"),
m_object(NetworkId::cms_invalid)
{
	addVariable(m_object);
	unpack(source);
}

RemoveItemMessage::~RemoveItemMessage()
{
}

//========================================================================

GiveMoneyMessage::GiveMoneyMessage(int amount) :
GameNetworkMessage("GiveMoneyMessage"),
m_amount(amount)
{
	addVariable(m_amount);
}

GiveMoneyMessage::GiveMoneyMessage(Archive::ReadIterator & source) :
GameNetworkMessage("GiveMoneyMessage"),
m_amount(0)
{
	addVariable(m_amount);
	unpack(source);
}

GiveMoneyMessage::~GiveMoneyMessage()
{
}
//========================================================================

TradeCompleteMessage::TradeCompleteMessage() : 
GameNetworkMessage("TradeCompleteMessage")
{
}

TradeCompleteMessage::TradeCompleteMessage(Archive::ReadIterator & source) :
GameNetworkMessage("TradeCompleteMessage")
{
			UNREF(source);
}

TradeCompleteMessage::~TradeCompleteMessage()
{

}


//========================================================================
AcceptTransactionMessage::AcceptTransactionMessage() : 
GameNetworkMessage("AcceptTransactionMessage")
{
}

AcceptTransactionMessage::AcceptTransactionMessage(Archive::ReadIterator & source) :
GameNetworkMessage("AcceptTransactionMessage")
{
			UNREF(source);
}

AcceptTransactionMessage::~AcceptTransactionMessage()
{

}


//========================================================================

UnAcceptTransactionMessage::UnAcceptTransactionMessage() : 
GameNetworkMessage("UnAcceptTransactionMessage")
{
}

UnAcceptTransactionMessage::UnAcceptTransactionMessage(Archive::ReadIterator & source) :
GameNetworkMessage("UnAcceptTransactionMessage")
{
			UNREF(source);
}

UnAcceptTransactionMessage::~UnAcceptTransactionMessage()
{

}

//========================================================================

VerifyTradeMessage::VerifyTradeMessage() : 
GameNetworkMessage("VerifyTradeMessage")
{
}

VerifyTradeMessage::VerifyTradeMessage(Archive::ReadIterator & source) :
GameNetworkMessage("VerifyTradeMessage")
{
			UNREF(source);
}

VerifyTradeMessage::~VerifyTradeMessage()
{

}

//========================================================================

DenyTradeMessage::DenyTradeMessage() : 
GameNetworkMessage("DenyTradeMessage")
{
}

DenyTradeMessage::DenyTradeMessage(Archive::ReadIterator & source) :
GameNetworkMessage("DenyTradeMessage")
{
		UNREF(source);
}

DenyTradeMessage::~DenyTradeMessage()
{
}

//========================================================================
BeginVerificationMessage::BeginVerificationMessage() : 
GameNetworkMessage("BeginVerificationMessage")
{
}

BeginVerificationMessage::BeginVerificationMessage(Archive::ReadIterator & source) :
GameNetworkMessage("BeginVerificationMessage")
{
		UNREF(source);
}

BeginVerificationMessage::~BeginVerificationMessage()
{
}

//========================================================================

