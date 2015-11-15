// ======================================================================
//
// PlanetRemoveObject.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetRemoveObject_H
#define INCLUDED_PlanetRemoveObject_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Tells Planet Server that we have removed an object from the world.
 *
 * Sent from:  Gameserver 
 * Sent to:  Planet Server
 * Action:  Remove the object from Planet Server's data structures, if
 * it hasn't been removed already.
 */
class PlanetRemoveObject : public GameNetworkMessage
{
  public:
	PlanetRemoveObject  (NetworkId objectId);
	PlanetRemoveObject  (Archive::ReadIterator & source);
	~PlanetRemoveObject ();

	NetworkId getObjectId() const;

  private:
	Archive::AutoVariable<NetworkId> m_objectId;

	PlanetRemoveObject();
	PlanetRemoveObject(const PlanetRemoveObject&);
	PlanetRemoveObject& operator= (const PlanetRemoveObject&);
};

// ----------------------------------------------------------------------

inline NetworkId PlanetRemoveObject::getObjectId() const
{
	return m_objectId.get();
}

// ======================================================================

#endif
