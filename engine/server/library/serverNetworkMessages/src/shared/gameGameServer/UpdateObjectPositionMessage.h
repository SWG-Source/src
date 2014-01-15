// ======================================================================
//
// UpdateObjectPositionMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_UpdateObjectPositionMessage_H
#define	_INCLUDED_UpdateObjectPositionMessage_H

// ======================================================================

#include "sharedMath/Transform.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

class UpdateObjectPositionMessage: public GameNetworkMessage
{
public:

	UpdateObjectPositionMessage(NetworkId const &networkId, Transform const &transform, Transform const &worldspaceTransform, NetworkId const &containerId, int slotArrangement, NetworkId const &loadWith, bool playerControlled, bool bCreatureObject);
	UpdateObjectPositionMessage(Archive::ReadIterator &source);

	NetworkId const &getNetworkId() const;
	Transform const &getTransform() const;
	Transform const &getWorldspaceTransform() const;
	NetworkId const &getContainerId() const;
	int getSlotArrangement() const;
	NetworkId const &getLoadWith() const;
	bool isCreatureObject() const; 
	bool getPlayerControlled() const;
	
private:
	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<Transform> m_transform;
	Archive::AutoVariable<Transform> m_worldspaceTransform;
	Archive::AutoVariable<NetworkId> m_containerId;
	Archive::AutoVariable<int> m_slotArrangement;
	Archive::AutoVariable<NetworkId> m_loadWith;
	Archive::AutoVariable<bool> m_playerControlled;
	Archive::AutoVariable<bool> m_creatureObject;


private:
	UpdateObjectPositionMessage(UpdateObjectPositionMessage const &);
	UpdateObjectPositionMessage &operator=(UpdateObjectPositionMessage const &);
};

// ======================================================================

inline NetworkId const &UpdateObjectPositionMessage::getNetworkId() const
{
	return m_networkId.get();
}

// ----------------------------------------------------------------------

inline Transform const &UpdateObjectPositionMessage::getTransform() const
{
	return m_transform.get();
}

// ----------------------------------------------------------------------

inline Transform const &UpdateObjectPositionMessage::getWorldspaceTransform() const
{
	return m_worldspaceTransform.get();
}

// ----------------------------------------------------------------------

inline NetworkId const &UpdateObjectPositionMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline int UpdateObjectPositionMessage::getSlotArrangement() const
{
	return m_slotArrangement.get();
}

// ----------------------------------------------------------------------

inline NetworkId const &UpdateObjectPositionMessage::getLoadWith() const
{
	return m_loadWith.get();
}


inline bool UpdateObjectPositionMessage::isCreatureObject() const
{
	return m_creatureObject.get();
}


// ----------------------------------------------------------------------

inline bool UpdateObjectPositionMessage::getPlayerControlled() const
{
	return m_playerControlled.get();
}


// ======================================================================

#endif	// _INCLUDED_UpdateObjectPositionMessage_H

