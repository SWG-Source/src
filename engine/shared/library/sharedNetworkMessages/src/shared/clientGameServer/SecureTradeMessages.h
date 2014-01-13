//========================================================================
//
// SecureTradeMessages.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_SecureTradeMessages_H
#define INCLUDED_SecureTradeMessages_H

#include "sharedNetworkMessages/GameNetworkMessage.h"


//========================================================================

class BeginTradeMessage : public GameNetworkMessage
{
public:
	explicit BeginTradeMessage(const NetworkId & player);
	BeginTradeMessage(Archive::ReadIterator & source);
	~BeginTradeMessage();

	const NetworkId &    getPlayerId() const;

private:
	Archive::AutoVariable<NetworkId> m_player;
};

inline const NetworkId & BeginTradeMessage::getPlayerId() const
{
	return m_player.get();
}


//========================================================================

class AbortTradeMessage : public GameNetworkMessage
{
public:
	AbortTradeMessage();
	AbortTradeMessage(Archive::ReadIterator & source);
	~AbortTradeMessage();

};

//========================================================================

class AddItemMessage : public GameNetworkMessage
{
public:
	explicit AddItemMessage(const NetworkId & object);
	AddItemMessage(Archive::ReadIterator & source);
	~AddItemMessage();

	const NetworkId &    getNetworkId() const;

private:
	Archive::AutoVariable<NetworkId> m_object;

};

inline const NetworkId & AddItemMessage::getNetworkId() const
{
	return m_object.get();
}


//========================================================================

class AddItemFailedMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	explicit AddItemFailedMessage(const NetworkId & object);
	AddItemFailedMessage(Archive::ReadIterator & source);
	~AddItemFailedMessage();

	const NetworkId &    getNetworkId() const;

private:
	Archive::AutoVariable<NetworkId> m_object;

};

inline const NetworkId & AddItemFailedMessage::getNetworkId() const
{
	return m_object.get();
}


//========================================================================

class RemoveItemMessage : public GameNetworkMessage
{
public:
	explicit RemoveItemMessage(const NetworkId & objectId);
	RemoveItemMessage(Archive::ReadIterator & source);
	~RemoveItemMessage();

	const NetworkId &    getNetworkId() const;

private:
	Archive::AutoVariable<NetworkId> m_object;
};

inline const NetworkId & RemoveItemMessage::getNetworkId() const
{
	return m_object.get();
}


//========================================================================

class GiveMoneyMessage : public GameNetworkMessage
{
public:
	explicit GiveMoneyMessage(int amount);
	GiveMoneyMessage(Archive::ReadIterator & source);
	~GiveMoneyMessage();

	int    getAmount() const;

private:
	Archive::AutoVariable<int> m_amount;

};

inline int GiveMoneyMessage::getAmount() const
{
	return m_amount.get();
}


//========================================================================

class TradeCompleteMessage : public GameNetworkMessage
{
public:
	TradeCompleteMessage();
	TradeCompleteMessage(Archive::ReadIterator & source);
	~TradeCompleteMessage();

};

//========================================================================
class AcceptTransactionMessage : public GameNetworkMessage
{
public:
	AcceptTransactionMessage();
	AcceptTransactionMessage(Archive::ReadIterator & source);
	~AcceptTransactionMessage();

};

//========================================================================

class UnAcceptTransactionMessage : public GameNetworkMessage
{
public:
	UnAcceptTransactionMessage();
	UnAcceptTransactionMessage(Archive::ReadIterator & source);
	~UnAcceptTransactionMessage();

};

//========================================================================

class VerifyTradeMessage : public GameNetworkMessage
{
public:
	VerifyTradeMessage();
	VerifyTradeMessage(Archive::ReadIterator & source);
	~VerifyTradeMessage();

};

//========================================================================

class DenyTradeMessage : public GameNetworkMessage
{
public:
	DenyTradeMessage();
	DenyTradeMessage(Archive::ReadIterator & source);
	~DenyTradeMessage();

};

//========================================================================


class BeginVerificationMessage : public GameNetworkMessage
{
public:
	BeginVerificationMessage();
	BeginVerificationMessage(Archive::ReadIterator & source);
	~BeginVerificationMessage();

};

//========================================================================


#endif
