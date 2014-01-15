// ======================================================================
//
// UpdateObjectOnPlanetMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UpdateObjectOnPlanetMessage_H
#define INCLUDED_UpdateObjectOnPlanetMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Updates Planet Server with the status of an object.
 *
 * Sent from:  Gameserver 
 * Sent to:  Planet Server
 * Action:  This message is sent periodically to update Planet's
 * information about an object.  This may cause Planet Server to create new
 * proxies, load more of the map, etc.
 *
 * Planet Server should check whether it knows about the object, and add it to
 * its data if this is the first time it has heard of the object.
 *
 * The Gameserver should always send this message when it becomes
 * authoritative.  After that, it only needs to send it when the object's
 * position changes by more than a certain threshold.
 *
 * The coordinates are reduced to int's because the Planet Server does not
 * need more than 1-meter accuracy.
 *
 * @todo:  Should this be replaced with something that uses the auto-
 * synchronization system?
 */
class UpdateObjectOnPlanetMessage : public GameNetworkMessage
{
  public:
	UpdateObjectOnPlanetMessage  (const NetworkId &objectId, const NetworkId &topmostContainer, int x, int y, int z, int interestRadius, int locationReservationRadius, bool watched, bool requiresSimulation, const int objectTag, int level, bool hibernating, uint32 templateCrc, int aiActivity, int creationType);
	UpdateObjectOnPlanetMessage  (Archive::ReadIterator & source);
	~UpdateObjectOnPlanetMessage ();

	const NetworkId &getObjectId() const;
	const NetworkId &getTopmostContainer() const;
	int getX() const;
	int getY() const;
	int getZ() const;
	int getInterestRadius() const;
	int getLocationReservationRadius() const;
	int getObjectTypeTag() const;
	const bool getWatched() const;
	const bool getRequiresSimulation() const;
	int getLevel() const;
	bool getHibernating() const;
	uint32 getTemplateCrc() const;
	int getAiActivity() const;
	int getCreationType() const;

  private:
	Archive::AutoVariable<NetworkId> m_objectId;
	Archive::AutoVariable<NetworkId> m_topmostContainer;
	Archive::AutoVariable<int> m_x;
	Archive::AutoVariable<int> m_y;
	Archive::AutoVariable<int> m_z;
	Archive::AutoVariable<int> m_interestRadius;
	Archive::AutoVariable<int> m_locationReservationRadius;
	Archive::AutoVariable<int> m_objectTypeTag;
	Archive::AutoVariable<bool> m_watched;
	Archive::AutoVariable<bool> m_requiresSimulation;
	Archive::AutoVariable<int> m_level;
	Archive::AutoVariable<bool> m_hibernating;
	Archive::AutoVariable<uint32> m_templateCrc;
	Archive::AutoVariable<int> m_aiActivity;
	Archive::AutoVariable<int> m_creationType;

  private:
	UpdateObjectOnPlanetMessage();
	UpdateObjectOnPlanetMessage(const UpdateObjectOnPlanetMessage&);
	UpdateObjectOnPlanetMessage& operator= (const UpdateObjectOnPlanetMessage&);
};

// ----------------------------------------------------------------------

inline const NetworkId &UpdateObjectOnPlanetMessage::getObjectId() const
{
	return m_objectId.get();
}

// ----------------------------------------------------------------------

inline int UpdateObjectOnPlanetMessage::getX() const
{
	return m_x.get();
}

//-----------------------------------------------------------------------

inline int UpdateObjectOnPlanetMessage::getObjectTypeTag() const
{
	return m_objectTypeTag.get();
}

// ----------------------------------------------------------------------

inline int UpdateObjectOnPlanetMessage::getY() const
{
	return m_y.get();
}

// ----------------------------------------------------------------------

inline int UpdateObjectOnPlanetMessage::getZ() const
{
	return m_z.get();
}

// ----------------------------------------------------------------------

inline int UpdateObjectOnPlanetMessage::getLevel() const
{
	return m_level.get();
}

// ----------------------------------------------------------------------

inline bool UpdateObjectOnPlanetMessage::getHibernating() const
{
	return m_hibernating.get();
}

// ----------------------------------------------------------------------

inline uint32 UpdateObjectOnPlanetMessage::getTemplateCrc() const
{
	return m_templateCrc.get();
}

// ----------------------------------------------------------------------

inline int UpdateObjectOnPlanetMessage::getAiActivity() const
{
	return m_aiActivity.get();
}

// ----------------------------------------------------------------------

inline int UpdateObjectOnPlanetMessage::getCreationType() const
{
	return m_creationType.get();
}

// ----------------------------------------------------------------------

inline int UpdateObjectOnPlanetMessage::getInterestRadius() const
{
	return m_interestRadius.get();
}

//-----------------------------------------------------------------------

inline int UpdateObjectOnPlanetMessage::getLocationReservationRadius() const
{
	return m_locationReservationRadius.get();
}

//-----------------------------------------------------------------------

inline const bool UpdateObjectOnPlanetMessage::getWatched() const
{
	return m_watched.get();
}

//-----------------------------------------------------------------------

inline const bool UpdateObjectOnPlanetMessage::getRequiresSimulation() const
{
	return m_requiresSimulation.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &UpdateObjectOnPlanetMessage::getTopmostContainer() const
{
	return m_topmostContainer.get();
}
// ======================================================================

#endif
