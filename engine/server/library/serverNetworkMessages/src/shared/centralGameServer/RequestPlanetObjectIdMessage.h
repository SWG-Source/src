//========================================================================
//
// RequestPlanetObjectIdMessage.h - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_RequestPlanetObjectIdMessage_H
#define	_INCLUDED_RequestPlanetObjectIdMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class RequestPlanetObjectIdMessage : public GameNetworkMessage
{
public:
	RequestPlanetObjectIdMessage  (const std::string &sceneId);
	RequestPlanetObjectIdMessage  (Archive::ReadIterator & source);
	~RequestPlanetObjectIdMessage ();

	std::string getSceneId (void) const;

private:
	Archive::AutoVariable<std::string> m_sceneId;

	RequestPlanetObjectIdMessage();
	RequestPlanetObjectIdMessage(const RequestPlanetObjectIdMessage&);
	RequestPlanetObjectIdMessage& operator= (const RequestPlanetObjectIdMessage&);
};

// ----------------------------------------------------------------------

inline std::string RequestPlanetObjectIdMessage::getSceneId (void) const
{
	return m_sceneId.get();
}

// ======================================================================

#endif	// _INCLUDED_RequestPlanetObjectIdMessage_H

