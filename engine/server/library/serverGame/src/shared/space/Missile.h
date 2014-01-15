// ======================================================================
//
// Missile.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Missile_H
#define INCLUDED_Missile_H

// ======================================================================

#include "sharedObject/CachedNetworkId.h"
#include "sharedGame/ShipChassisSlotType.h"

class GameNetworkMessage;

// ======================================================================

class Missile
{
  public:
	enum MissileState
		{
			MS_None,       // missile has been created, but not launched yet
			MS_Launched,   // missile is in space with a designated target
			MS_Miss,       // target was too far away, missile will be destroyed
			MS_Kill        // missile will be deleted at the next update() call
		};
		
  public:
		Missile(const NetworkId &sourcePlayer, const NetworkId &sourceShip, int weaponIndex, int missileType, const NetworkId & targetShip, ShipChassisSlotType::Type targetedComponent);
	~Missile();

  public:
	int               getMissileId       () const;
	const NetworkId & getSource          () const;
	MissileState      getState           () const;
	const NetworkId & getTargetShip      () const;
	int               getTimeSinceFired  () const;
	int               getTotalTime       () const;
	int               getImpactTime      () const;
	int               getType            () const;
	int               getWeaponIndex      () const;
	ShipChassisSlotType::Type getTargetedComponent() const;

	void              destroy            ();
	void              countermeasure     (NetworkId const & ship, int const countermeasureType);
	static void       launchFailedCountermeasure(ServerObject const & ship, int const missileId, int const countermeasureType);
	bool              fire               ();
	bool              update             (uint32 effectiveTime);
	
  private:
	void              impactTarget          ();
	ServerObject *    getSourceServerObject () const;
	ServerObject *    getTargetServerObject () const;
	void              sendToAllObservers    (const GameNetworkMessage &message) const;
	static void       sendToAllObservers    (ServerObject const & object, GameNetworkMessage const & message);

  private:
	const int               m_missileId;
	const CachedNetworkId   m_sourceShip;
	const CachedNetworkId   m_sourcePlayer;
	CachedNetworkId         m_targetShip;
	uint32                  m_impactTime;
	MissileState            m_state;
	int                     m_weaponIndex;
	int                     m_missileType;
	ShipChassisSlotType::Type m_targetedComponent;
	uint32                  m_fireTime;

  private:
	static int        ms_nextMissileId;

  private:
	Missile & operator=(const Missile &); //disable
	Missile(const Missile &); //disable
};

// ======================================================================

inline int Missile::getMissileId() const
{
	return m_missileId;
}

// ----------------------------------------------------------------------

inline Missile::MissileState Missile::getState() const
{
	return m_state;
}

// ----------------------------------------------------------------------

inline const NetworkId &Missile::getTargetShip() const
{
	return m_targetShip;
}

// ----------------------------------------------------------------------

inline int Missile::getWeaponIndex() const
{
	return m_weaponIndex;
}

// ----------------------------------------------------------------------

inline ShipChassisSlotType::Type Missile::getTargetedComponent() const
{
	return m_targetedComponent;
}

// ----------------------------------------------------------------------

inline int Missile::getType() const
{
	return m_missileType;
}

// ----------------------------------------------------------------------

inline const NetworkId & Missile::getSource() const
{
	return m_sourceShip;
}

// ----------------------------------------------------------------------

inline int Missile::getTotalTime() const
{
	return m_impactTime - m_fireTime;
}

// ----------------------------------------------------------------------

inline int Missile::getImpactTime() const
{
	return m_impactTime;
}

// ======================================================================

#endif
