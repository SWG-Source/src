// ======================================================================
//
// MessageQueueScriptTransferMoney.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "swgServerNetworkMessages/MessageQueueScriptTransferMoney.h"

#include <string>
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueScriptTransferMoney, CM_scriptTransferMoney);

// ======================================================================

MessageQueueScriptTransferMoney::MessageQueueScriptTransferMoney(TransactionType typeId, const NetworkId &target, const std::string &namedAccount, int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary) :
		m_typeId           (typeId),
		m_target           (target),
		m_namedAccount     (namedAccount),
		m_amount           (amount),
		m_replyTo          (replyTo),
		m_successCallback  (successCallback),
		m_failCallback     (failCallback),
		m_packedDictionary (packedDictionary)
{
	DEBUG_FATAL(target != NetworkId::cms_invalid && namedAccount.size() > 0,("Target cannot be specified if a named account is specified.\n"));
}

// ----------------------------------------------------------------------

MessageQueueScriptTransferMoney::~MessageQueueScriptTransferMoney()
{
}
// ----------------------------------------------------------------------

void MessageQueueScriptTransferMoney::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueScriptTransferMoney* const msg = safe_cast<const MessageQueueScriptTransferMoney*> (data);
	if (msg)
	{
		Archive::put (target, static_cast<int>(msg->getTypeId()));
		Archive::put (target, msg->getTarget());
		Archive::put (target, msg->getNamedAccount());
		Archive::put (target, msg->getAmount());
		Archive::put (target, msg->getReplyTo());
		Archive::put (target, msg->getSuccessCallback());
		Archive::put (target, msg->getFailCallback());
		Archive::put (target, msg->getPackedDictionary());
	}
}

//----------------------------------------------------------------------

MessageQueue::Data* MessageQueueScriptTransferMoney::unpack(Archive::ReadIterator & source)
{
	int               typeId;
	NetworkId         target;
	std::string       namedAccount;
	int               amount;
	NetworkId         replyTo;
	std::string       successCallback;
	std::string       failCallback;
	std::vector<int8> packedDictionary;

	Archive::get (source, typeId);
	Archive::get (source, target);
	Archive::get (source, namedAccount);
	Archive::get (source, amount);
	Archive::get (source, replyTo);
	Archive::get (source, successCallback);
	Archive::get (source, failCallback);
	Archive::get (source, packedDictionary);
	
	return new MessageQueueScriptTransferMoney(static_cast<MessageQueueScriptTransferMoney::TransactionType>(typeId),target,namedAccount,amount,replyTo,successCallback,failCallback,packedDictionary);
}


// ======================================================================
