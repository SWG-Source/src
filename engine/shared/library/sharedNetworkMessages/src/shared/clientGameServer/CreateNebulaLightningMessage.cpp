//======================================================================
//
// CreateNebulaLightningMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CreateNebulaLightningMessage.h"

//======================================================================

char const * const CreateNebulaLightningMessage::MessageType = "CreateNebulaLightningMessage";

//----------------------------------------------------------------------

CreateNebulaLightningMessage::CreateNebulaLightningMessage(NebulaLightningData const & nebulaLightningData) :
GameNetworkMessage(MessageType),
m_nebulaLightningData(nebulaLightningData)
{
	addVariable(m_nebulaLightningData);
}

//----------------------------------------------------------------------

CreateNebulaLightningMessage::CreateNebulaLightningMessage(Archive::ReadIterator &source) :
GameNetworkMessage(MessageType),
m_nebulaLightningData()
{
	addVariable(m_nebulaLightningData);
	unpack(source);
}

//----------------------------------------------------------------------

NebulaLightningData const & CreateNebulaLightningMessage::getNebulaLightningData() const
{
	return m_nebulaLightningData.get();
}

//======================================================================
