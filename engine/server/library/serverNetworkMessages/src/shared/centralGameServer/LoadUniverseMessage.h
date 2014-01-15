//========================================================================
//
// LoadUniverseMessage.h - tells Gameserver another Gameserver wants to load a
// proxy from an authoritative object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_LoadUniverseMessage_H
#define	_INCLUDED_LoadUniverseMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class LoadUniverseMessage : public GameNetworkMessage
{
  public:
	LoadUniverseMessage  (uint32 process);
	LoadUniverseMessage  (Archive::ReadIterator & source);
	~LoadUniverseMessage ();

	uint32              getProcess(void) const;
        
  private:
	Archive::AutoVariable<uint32>      m_process;		// process id of Gameserver that wants to load the object

  private:
	LoadUniverseMessage();
	LoadUniverseMessage(const LoadUniverseMessage&);
	LoadUniverseMessage& operator= (const LoadUniverseMessage&);
};


//-----------------------------------------------------------------------

inline uint32 LoadUniverseMessage::getProcess(void) const
{
	return m_process.get();
}	// LoadUniverseMessage::getProcess

// ----------------------------------------------------------------------

#endif	// _INCLUDED_LoadUniverseMessage_H
