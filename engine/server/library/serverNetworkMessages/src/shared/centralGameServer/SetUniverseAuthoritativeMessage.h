//========================================================================
//
// SetUniverseAuthoritativeMessage.h - tells Gameserver another Gameserver wants to load a
// proxy from an authoritative object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_SetUniverseAuthoritativeMessage_H
#define	_INCLUDED_SetUniverseAuthoritativeMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Central
 * Sent to:  all GameServers
 * Action:  Tells the GameServers which one should be authoritative for
 *          universe objects.
 */

class SetUniverseAuthoritativeMessage : public GameNetworkMessage
{
  public:
	SetUniverseAuthoritativeMessage  (uint32 process);
	SetUniverseAuthoritativeMessage  (Archive::ReadIterator & source);
	~SetUniverseAuthoritativeMessage ();

	uint32              getProcess(void) const;
        
  private:
	Archive::AutoVariable<uint32>      m_process;		

  private:
	SetUniverseAuthoritativeMessage();
	SetUniverseAuthoritativeMessage(const SetUniverseAuthoritativeMessage&);
	SetUniverseAuthoritativeMessage& operator= (const SetUniverseAuthoritativeMessage&);
};


//-----------------------------------------------------------------------

inline uint32 SetUniverseAuthoritativeMessage::getProcess(void) const
{
	return m_process.get();
}

// ----------------------------------------------------------------------

#endif	// _INCLUDED_SetUniverseAuthoritativeMessage_H
