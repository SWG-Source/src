// ======================================================================
//
// UpdatePlayerCountMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UpdatePlayerCountMessage_H
#define INCLUDED_UpdatePlayerCountMessage_H

// ======================================================================

#include "Unicode.h"
#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

/**
 * Sent from:  Connection Server
 * Sent to:  Central
 * Action:  Reports the number of players connected to that ConnectionServer
 */
class UpdatePlayerCountMessage : public GameNetworkMessage
{
  public:
	UpdatePlayerCountMessage(bool loadedRecently, int playerCount, int freeTrialCount, int emptySceneCount, int tutorialSceneCount, int falconSceneCount);
	UpdatePlayerCountMessage(Archive::ReadIterator & source);

  public:
	bool getLoadedRecently() const;
	int getCount() const;
	int getFreeTrialCount() const;
	int getEmptySceneCount() const;
	int getTutorialSceneCount() const;
	int getFalconSceneCount() const;
	
  private:
	Archive::AutoVariable<bool> m_loadedRecently;
	Archive::AutoVariable<int> m_playerCount;
	Archive::AutoVariable<int> m_freeTrialCount;
	Archive::AutoVariable<int> m_emptySceneCount;
	Archive::AutoVariable<int> m_tutorialSceneCount;
	Archive::AutoVariable<int> m_falconSceneCount;
};

// ======================================================================

inline bool UpdatePlayerCountMessage::getLoadedRecently() const
{
	return m_loadedRecently.get();
}

// ----------------------------------------------------------------------

inline int UpdatePlayerCountMessage::getCount() const
{
	return m_playerCount.get();
}

// ----------------------------------------------------------------------

inline int UpdatePlayerCountMessage::getFreeTrialCount() const
{
	return m_freeTrialCount.get();
}

// ----------------------------------------------------------------------

inline int UpdatePlayerCountMessage::getEmptySceneCount() const
{
	return m_emptySceneCount.get();
}

// ----------------------------------------------------------------------

inline int UpdatePlayerCountMessage::getTutorialSceneCount() const
{
	return m_tutorialSceneCount.get();
}

// ----------------------------------------------------------------------

inline int UpdatePlayerCountMessage::getFalconSceneCount() const
{
	return m_falconSceneCount.get();
}

// ======================================================================

#endif
 
