//========================================================================
//
// ChangeUniverseProcessMessage.h - tells CentralServer to unload all instances of
// an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ChangeUniverseProcessMessage_H
#define	_INCLUDED_ChangeUniverseProcessMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChangeUniverseProcessMessage : public GameNetworkMessage
{
public:

	explicit ChangeUniverseProcessMessage  (int id);
	ChangeUniverseProcessMessage           (Archive::ReadIterator & source);
	~ChangeUniverseProcessMessage ();

	int          getId() const;

private:
	Archive::AutoVariable<int>                 m_id;

	ChangeUniverseProcessMessage();
	ChangeUniverseProcessMessage(const ChangeUniverseProcessMessage&);
	ChangeUniverseProcessMessage& operator= (const ChangeUniverseProcessMessage&);
};


//-----------------------------------------------------------------------

inline int ChangeUniverseProcessMessage::getId(void) const
{
	return m_id.get();
}	

// ----------------------------------------------------------------------

#endif
