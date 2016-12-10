// ======================================================================
//
// MissileManager.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MissileManager_H
#define INCLUDED_MissileManager_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedGame/ShipChassisSlotType.h"
#include <queue>
#include <functional>

// ======================================================================

class Missile;

// ======================================================================

/**
 * A singleton to manage missiles in flight.  Keeps track of missiles,
 * and calls the appropriate script functions as needed.
 */
class MissileManager
{
  public:
	int          getNearestUnlockedMissileForTarget  (const NetworkId &target) const;
	bool         isTargetedByMissile                 (const NetworkId &target) const;
	void         getAllUnlockedMissilesForTarget     (const NetworkId &target, std::vector<int> &results) const;

	bool         destroyMissile               (int missileId);
	bool         requestFireMissile           (const NetworkId &sourcePlayer, const NetworkId &sourceShip, const NetworkId &targetShip, int weaponIndex, int missileType, ShipChassisSlotType::Type targetedComponent);
	void         requestFireCountermeasure    (const NetworkId &player, const NetworkId &ship, const int weaponIndex) const;
	bool         fireCountermeasure           (NetworkId const & sourceShip, int const targetMissileId, bool const successful, int const countermeasureType);
	void         scheduleMissile              (int missileId, uint32 nextUpdateTime);
	void         update                       ();
	const Missile * getConstMissile           (int missileId) const;

	float getCountermeasureTimeFactorByMissileType (int missileTypeId) const;
	float getMaxTimeByMissileType                  (int missileTypeId) const;
	float getMinTimeByMissileType                  (int missileTypeId) const;
	float getSpeedByMissileType                    (int missileTypeId) const;
	float getRangeByMissileType(int missileTypeId) const;
	float getTargetAcquisitionAngle(int missileType);
	
  public:
	static void  install                      ();
	static void  remove                       ();

	static MissileManager & getInstance       ();

  private:
	class QueueSorter : public std::binary_function<std::pair<int, int>, std::pair<int, int>, bool>
	{
	  public:
		bool operator() (const std::pair<int, int> &lhs, const std::pair<int, int> &rhs);
	};

	struct MissileTypeDataRecord
	{
		float m_countermeasureTimeFactor;
		float m_maxTime;
		float m_minTime;
		float m_speed;
		float m_targetAcquisitionAngle;
	};
	
	typedef std::map<int, Missile*> MissilesType;
	typedef std::multimap<NetworkId, int> MissilesForTargetType;
	typedef std::priority_queue<std::pair<uint32, int>, std::vector<std::pair<uint32, int> >, QueueSorter> MissileQueueType;
	typedef std::map<int, MissileTypeDataRecord> MissileTypeDataType;

  private:
	Missile *    addMissile                   (const NetworkId &sourceShip, const NetworkId &sourcePlayer, int weaponIndex, int missileType, const NetworkId & targetShip, ShipChassisSlotType::Type targetedComponent);
	bool         removeMissile                (int missileId);
	Missile *    getMissile                   (int missileId);
	const MissileTypeDataRecord * getMissileTypeDataRecord (int missileTypeId) const;

  private:
	MissilesType m_missiles;
	MissilesForTargetType m_missilesForTarget;
	MissileQueueType m_missileQueue;
	MissileTypeDataType m_missileTypeData;
	
  private:
	MissileManager();
	~MissileManager();
	
	static MissileManager *ms_instance;
};

// ======================================================================

inline MissileManager & MissileManager::getInstance()
{
	DEBUG_FATAL(!ms_instance,("MissileManager not installed."));
	return *ms_instance;
}

// ======================================================================

inline bool MissileManager::QueueSorter::operator() (const std::pair<int, int> &lhs, const std::pair<int, int> &rhs)
{
	return (lhs.first < rhs.first);
}

// ======================================================================

#endif
