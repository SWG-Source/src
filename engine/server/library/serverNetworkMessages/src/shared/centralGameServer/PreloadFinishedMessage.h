//========================================================================
//
// PreloadFinishedMessage.h
//
// copyright 2002 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_PreloadFinishedMessage_H
#define	_INCLUDED_PreloadFinishedMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent From:  PlanetServer
 * Sent To:    Central
 * Action:     All the GameServers on this planet have/haven't finished preloading.
 *
 * Sent From:  LoginServer
 * Sent To:    Central
 * Action:     All the planets in the cluster have/haven't finished preloading.
 *             If preloading is already finished when the login server connects,
 *             this message is sent immediately.
 */

class PreloadFinishedMessage : public GameNetworkMessage
{
  public:
	PreloadFinishedMessage  (bool finished);
	PreloadFinishedMessage  (Archive::ReadIterator & source);
	~PreloadFinishedMessage ();

	const uint32 getGameServerId() const;

	bool getFinished() const;
        
  private:
	Archive::AutoVariable<bool>   m_finished;
       
  private:        
	PreloadFinishedMessage(const PreloadFinishedMessage&);
	PreloadFinishedMessage& operator= (const PreloadFinishedMessage&);
};

// ----------------------------------------------------------------------

inline bool PreloadFinishedMessage::getFinished() const
{
	return m_finished.get();
}

// ======================================================================

#endif	// _INCLUDED_PreloadFinishedMessage_H

