//========================================================================
//
// PreloadRequestCompleteMessage.cpp
//
// copyright 2002 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/PreloadRequestCompleteMessage.h"

#include <vector>

#include "serverNetworkMessages/CentralGameServerMessages.h"

//-----------------------------------------------------------------------

PreloadRequestCompleteMessage::PreloadRequestCompleteMessage(uint32 gameServerId, uint32 preloadAreaId) :
		GameNetworkMessage("PreloadRequestCompleteMessage"),
		m_gameServerId(gameServerId),
		m_preloadAreaId(preloadAreaId)
{
	addVariable(m_gameServerId);
	addVariable(m_preloadAreaId);
}

//-----------------------------------------------------------------------

PreloadRequestCompleteMessage::PreloadRequestCompleteMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("PreloadRequestCompleteMessage"),
		m_gameServerId(),
		m_preloadAreaId()
{
	addVariable(m_gameServerId);
	addVariable(m_preloadAreaId);
	unpack(source);
}

//-----------------------------------------------------------------------

PreloadRequestCompleteMessage::~PreloadRequestCompleteMessage()
{
}

// ======================================================================
