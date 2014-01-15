//========================================================================
//
// PlanetObjectIdMessage.h - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_PlanetObjectIdMessage_H
#define	_INCLUDED_PlanetObjectIdMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class PlanetObjectIdMessage : public GameNetworkMessage
{
public:
	PlanetObjectIdMessage  (const std::string &sceneId, NetworkId planetObject);
	PlanetObjectIdMessage  (Archive::ReadIterator & source);
	~PlanetObjectIdMessage ();

	std::string getSceneId    (void) const;
	NetworkId getPlanetObject (void) const;

private:
	Archive::AutoVariable<std::string> m_sceneId;
	Archive::AutoVariable<NetworkId> m_planetObject;

	PlanetObjectIdMessage();
	PlanetObjectIdMessage(const PlanetObjectIdMessage&);
	PlanetObjectIdMessage& operator= (const PlanetObjectIdMessage&);
};

// ----------------------------------------------------------------------

inline std::string PlanetObjectIdMessage::getSceneId (void) const
{
	return m_sceneId.get();
}

// ----------------------------------------------------------------------

inline NetworkId PlanetObjectIdMessage::getPlanetObject (void) const
{
	return m_planetObject.get();
}
// ======================================================================

#endif	// _INCLUDED_PlanetObjectIdMessage_H

