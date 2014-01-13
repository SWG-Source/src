///////////////////////////////////////////////////////////////////////////////
//
// ChatDeleteAllPersistentMessages.cpp
// Copyright 2003, Sony Online Entertainment Inc., all rights reserved. 
//
///////////////////////////////////////////////////////////////////////////////

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatDeleteAllPersistentMessages.h"

//-----------------------------------------------------------------------------

ChatDeleteAllPersistentMessages::ChatDeleteAllPersistentMessages(NetworkId const &sourceNetworkId, NetworkId const &targetNetworkId)
 : GameNetworkMessage("ChatDeleteAllPersistentMessages")
 , m_sourceNetworkId(sourceNetworkId)
 , m_targetNetworkId(targetNetworkId)
{
	addVariable(m_sourceNetworkId);
	addVariable(m_targetNetworkId);
}

//-----------------------------------------------------------------------------
ChatDeleteAllPersistentMessages::ChatDeleteAllPersistentMessages(Archive::ReadIterator & source)
 : GameNetworkMessage("ChatDeleteAllPersistentMessages")
 , m_sourceNetworkId()
 , m_targetNetworkId()
{
	addVariable(m_sourceNetworkId);
	addVariable(m_targetNetworkId);
	unpack(source);
}

//-----------------------------------------------------------------------------

ChatDeleteAllPersistentMessages::~ChatDeleteAllPersistentMessages()
{
}

//-----------------------------------------------------------------------------

NetworkId const &ChatDeleteAllPersistentMessages::getSourceNetworkId() const
{
	return m_sourceNetworkId.get();
}

//-----------------------------------------------------------------------------

NetworkId const &ChatDeleteAllPersistentMessages::getTargetNetworkId() const
{
	return m_targetNetworkId.get();
}

///////////////////////////////////////////////////////////////////////////////
