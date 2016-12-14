// ======================================================================
//
// GetMoneyFromOfflineObjectMessage.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GetMoneyFromOfflineObjectMessage.h"

// ======================================================================

GetMoneyFromOfflineObjectMessage::GetMoneyFromOfflineObjectMessage(NetworkId const & sourceObject, int amount, NetworkId const & replyTo, std::string const & successCallback, std::string const & failCallback, std::vector<int8> const & packedDictionary, bool success) :
		GameNetworkMessage("GetMoneyFromOfflineObjectMessage"),
		m_sourceObject(sourceObject),
		m_amount(amount),
		m_replyTo(replyTo),
		m_successCallback(successCallback),
		m_failCallback(failCallback),
		m_packedDictionary(packedDictionary),
		m_success(success)
{
	addVariable(m_sourceObject);
	addVariable(m_amount);
	addVariable(m_replyTo);
	addVariable(m_successCallback);
	addVariable(m_failCallback);
	addVariable(m_packedDictionary);
	addVariable(m_success);
}

// ----------------------------------------------------------------------

GetMoneyFromOfflineObjectMessage::GetMoneyFromOfflineObjectMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("GetMoneyFromOfflineObjectMessage"),
		m_sourceObject(),
		m_amount(),
		m_replyTo(),
		m_successCallback(),
		m_failCallback(),
		m_packedDictionary(),
		m_success()
{
	addVariable(m_sourceObject);
	addVariable(m_amount);
	addVariable(m_replyTo);
	addVariable(m_successCallback);
	addVariable(m_failCallback);
	addVariable(m_packedDictionary);
	addVariable(m_success);

	unpack(source);
}

// ----------------------------------------------------------------------

GetMoneyFromOfflineObjectMessage::~GetMoneyFromOfflineObjectMessage ()
{
}

// ----------------------------------------------------------------------

NetworkId const & GetMoneyFromOfflineObjectMessage::getSourceObject() const
{
	return m_sourceObject.get();
}

// ----------------------------------------------------------------------

int GetMoneyFromOfflineObjectMessage::getAmount() const
{
	return m_amount.get();
}

// ----------------------------------------------------------------------

NetworkId const & GetMoneyFromOfflineObjectMessage::getReplyTo() const
{
	return m_replyTo.get();
}

// ----------------------------------------------------------------------

std::string const & GetMoneyFromOfflineObjectMessage::getSuccessCallback() const
{
	return m_successCallback.get();
}

// ----------------------------------------------------------------------

std::string const & GetMoneyFromOfflineObjectMessage::getFailCallback() const
{
	return m_failCallback.get();
}

// ----------------------------------------------------------------------

std::vector<int8> const & GetMoneyFromOfflineObjectMessage::getPackedDictionary() const
{
	return m_packedDictionary.get();
}

// ----------------------------------------------------------------------

bool GetMoneyFromOfflineObjectMessage::getSuccess() const
{
	return m_success.get();
}

// ======================================================================
