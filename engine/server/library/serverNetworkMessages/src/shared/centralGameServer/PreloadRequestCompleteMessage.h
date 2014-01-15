//========================================================================
//
// PreloadRequestCompleteMessage.h
//
// copyright 2002 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_PreloadRequestCompleteMessage_H
#define	_INCLUDED_PreloadRequestCompleteMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent From:  PlanetServer
 * Sent To:    DatabaseProcess (via Central)
 * Action:     All the objects/chunks needed for preload have now been requested.
 *             When pending loads are completed, notify the game server
 *             that it has all of the preload stuff.
 *
 * Sent From:  DatabaseProcess
 * Sent To:    GameServer
 * Action:     The DatabaseProcess has sent the GameServer all the objects
 *             it needs for preload.  When they are all instantiated, send this
 *             message to the PlanetServer.
 *
 * Sent From:  GameServer
 * Sent To:    PlanetServer
 * Action:     The GameServer has instantiated all the objects from the
 *             preload list.
 */

class PreloadRequestCompleteMessage : public GameNetworkMessage
{
  public:
	PreloadRequestCompleteMessage  (uint32 gameServerId, uint32 preloadAreaId);
	PreloadRequestCompleteMessage  (Archive::ReadIterator & source);
	~PreloadRequestCompleteMessage ();

	const uint32 getGameServerId() const;
	const uint32 getPreloadAreaId() const;
        
  private:
	Archive::AutoVariable<uint32> m_gameServerId;
	Archive::AutoVariable<uint32> m_preloadAreaId;
       
  private:        
	PreloadRequestCompleteMessage();
	PreloadRequestCompleteMessage(const PreloadRequestCompleteMessage&);
	PreloadRequestCompleteMessage& operator= (const PreloadRequestCompleteMessage&);
};

// ======================================================================

inline const uint32 PreloadRequestCompleteMessage::getGameServerId() const
{
	return m_gameServerId.get();
}

// ----------------------------------------------------------------------

inline const uint32 PreloadRequestCompleteMessage::getPreloadAreaId() const
{
	return m_preloadAreaId.get();
}

// ======================================================================

#endif	// _INCLUDED_PreloadRequestCompleteMessage_H

