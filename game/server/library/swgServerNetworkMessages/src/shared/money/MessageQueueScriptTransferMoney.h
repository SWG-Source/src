//========================================================================
//
// MessageQueueResourcePoolCollectResource.h.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueScriptTransferMoney_H
#define INCLUDED_MessageQueueScriptTransferMoney_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

// ======================================================================

/**
 * Sent by:  Any script
 * Sent to:  Any ServerObject
 * Action:   Do the described money transfer, then send the results to a
 *           script callback.
 */

class MessageQueueScriptTransferMoney : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

  public:
		enum TransactionType {TT_bankTransfer, TT_cashTransfer, TT_bankWithdrawal, TT_bankDeposit, TT_bankTransferToNamedAccount, TT_bankTransferFromNamedAccount};

  public:
	MessageQueueScriptTransferMoney(TransactionType typeId, const NetworkId &target, const std::string &namedAccount, int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary);
	virtual ~MessageQueueScriptTransferMoney();

	TransactionType              getTypeId() const;
	const NetworkId &            getTarget() const;
	const std::string &          getNamedAccount() const;
	int                          getAmount() const;
	const NetworkId &            getReplyTo() const;
	const std::string &          getSuccessCallback() const;
	const std::string &          getFailCallback() const;
	const std::vector<int8> & getPackedDictionary() const;
		
  private:
	TransactionType      m_typeId;
	NetworkId            m_target;
	std::string          m_namedAccount;
	int                  m_amount;
	NetworkId            m_replyTo;
	std::string          m_successCallback;
	std::string          m_failCallback;
	std::vector<int8> m_packedDictionary;
		 
  private:
	MessageQueueScriptTransferMoney&	operator=	(const MessageQueueScriptTransferMoney & source);
	MessageQueueScriptTransferMoney(const MessageQueueScriptTransferMoney & source);
};

// ======================================================================

inline MessageQueueScriptTransferMoney::TransactionType MessageQueueScriptTransferMoney::getTypeId() const
{
	return m_typeId;
}

// ----------------------------------------------------------------------

inline const NetworkId & MessageQueueScriptTransferMoney::getTarget() const
{
	return m_target;
}

// ----------------------------------------------------------------------

inline const std::string & MessageQueueScriptTransferMoney::getNamedAccount() const
{
	return m_namedAccount;
}

// ----------------------------------------------------------------------

inline int MessageQueueScriptTransferMoney::getAmount() const
{
	return m_amount;
}

// ----------------------------------------------------------------------

inline const NetworkId & MessageQueueScriptTransferMoney::getReplyTo() const
{
	return m_replyTo;
}

// ----------------------------------------------------------------------

inline const std::string & MessageQueueScriptTransferMoney::getSuccessCallback() const
{
	return m_successCallback;
}

// ----------------------------------------------------------------------

inline const std::string & MessageQueueScriptTransferMoney::getFailCallback() const
{
	return m_failCallback;
}

// ----------------------------------------------------------------------

inline const std::vector<int8> & MessageQueueScriptTransferMoney::getPackedDictionary() const
{
	return m_packedDictionary;
}

// ======================================================================

#endif
