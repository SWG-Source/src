// ======================================================================
//
// UpdatePlayerCountMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/UpdatePlayerCountMessage.h"

// ======================================================================

UpdatePlayerCountMessage::UpdatePlayerCountMessage(bool loadedRecently, int playerCount, int emptySceneCount, int freeTrialCount, int tutorialSceneCount, int falconSceneCount) :
		GameNetworkMessage("UpdatePlayerCountMessage"),
		m_loadedRecently(loadedRecently),
		m_playerCount(playerCount),
		m_freeTrialCount(freeTrialCount),
		m_emptySceneCount(emptySceneCount),
		m_tutorialSceneCount(tutorialSceneCount),
		m_falconSceneCount(falconSceneCount)
{
	addVariable(m_loadedRecently);
	addVariable(m_playerCount);
	addVariable(m_freeTrialCount);
	addVariable(m_emptySceneCount);
	addVariable(m_tutorialSceneCount);
	addVariable(m_falconSceneCount);
}

// ----------------------------------------------------------------------

UpdatePlayerCountMessage::UpdatePlayerCountMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("UpdatePlayerCountMessage"),
		m_loadedRecently(false),
		m_playerCount(0),
		m_freeTrialCount(0),
		m_emptySceneCount(0),
		m_tutorialSceneCount(0),
		m_falconSceneCount(0)
{
	addVariable(m_loadedRecently);
	addVariable(m_playerCount);
	addVariable(m_freeTrialCount);
	addVariable(m_emptySceneCount);
	addVariable(m_tutorialSceneCount);
	addVariable(m_falconSceneCount);
	
	unpack(source);
}

// ======================================================================
