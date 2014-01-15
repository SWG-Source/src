// ======================================================================
//
// LoginUpgradeAccountMessage.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoginUpgradeAccountMessage.h"

// ======================================================================

LoginUpgradeAccountMessage::LoginUpgradeAccountMessage (UpgradeType upgradeType, const StationId &stationId, const NetworkId &character, const NetworkId &replyToObject, const std::string &replyMessage, const std::string &packedMessageData, bool isAck) :
		GameNetworkMessage("LoginUpgradeAccountMessage"),
		m_upgradeType(static_cast<int>(upgradeType)),
		m_stationId(stationId),
		m_character(character),
		m_replyToObject(replyToObject),
		m_replyMessage(replyMessage),
		m_packedMessageData(packedMessageData),
		m_isAck(isAck)
 
{
	addVariable(m_upgradeType);
	addVariable(m_stationId);
	addVariable(m_character);
	addVariable(m_replyToObject);
	addVariable(m_replyMessage);
	addVariable(m_packedMessageData);
	addVariable(m_isAck);	
 }

// ----------------------------------------------------------------------

LoginUpgradeAccountMessage::LoginUpgradeAccountMessage (Archive::ReadIterator & source) :
		GameNetworkMessage("LoginUpgradeAccountMessage"),
		m_upgradeType(),
		m_stationId(),
		m_character()
{
	addVariable(m_upgradeType);
	addVariable(m_stationId);
	addVariable(m_character);
	addVariable(m_replyToObject);
	addVariable(m_replyMessage);
	addVariable(m_packedMessageData);
	addVariable(m_isAck);	

	unpack(source);
}

// ----------------------------------------------------------------------

LoginUpgradeAccountMessage::~LoginUpgradeAccountMessage()
{
}

// ----------------------------------------------------------------------

void LoginUpgradeAccountMessage::setAck()
{
	m_isAck.set(true);
}

// ======================================================================
