// ======================================================================
//
// DeleteCharacterMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/DeleteCharacterMessage.h"

// ======================================================================

DeleteCharacterMessage::DeleteCharacterMessage(uint32 clusterId, const NetworkId &characterId) :
		GameNetworkMessage("DeleteCharacterMessage"),
		m_clusterId(clusterId),
		m_characterId(characterId)
{
	addVariable(m_clusterId);
	addVariable(m_characterId);
}

//-----------------------------------------------------------------------

DeleteCharacterMessage::DeleteCharacterMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("DeleteCharacterMessage"),
		m_clusterId(),
		m_characterId()
{
	addVariable(m_clusterId);
	addVariable(m_characterId);
	unpack(source);
}

// ----------------------------------------------------------------------

DeleteCharacterMessage::~DeleteCharacterMessage()
{
}

// ======================================================================
