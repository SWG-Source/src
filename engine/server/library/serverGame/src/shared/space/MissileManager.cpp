// ======================================================================
//
// MissileManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/MissileManager.h"

#include "serverGame/Missile.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/ServerObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

// ======================================================================

MissileManager * MissileManager::ms_instance = nullptr;

// ======================================================================

void MissileManager::install()
{
	DEBUG_FATAL(ms_instance,("Called MissileManager::install when it was already installed."));
	ExitChain::add(&remove, "MissileManager::remove");
	ms_instance = new MissileManager;
}

// ----------------------------------------------------------------------

void MissileManager::remove()
{
	delete ms_instance;
	ms_instance = nullptr;
}

// ----------------------------------------------------------------------

MissileManager::MissileManager() :
		m_missiles(),
		m_missilesForTarget(),
		m_missileQueue(),
		m_missileTypeData()
{
	DataTable * missileDataTable = DataTableManager::getTable("datatables/space/missiles.iff", true);
	FATAL(!missileDataTable,("Missile data table could not be opened."));
	int numRows = missileDataTable->getNumRows();
	for (int row=0; row<numRows; ++row)
	{
		MissileTypeDataRecord data;
		data.m_countermeasureTimeFactor = missileDataTable->getFloatValue("countermeasure_time_factor",row);
		data.m_maxTime = missileDataTable->getFloatValue("max_time",row);
		data.m_minTime = missileDataTable->getFloatValue("min_time",row);
		data.m_speed = missileDataTable->getFloatValue("server_speed",row);
		data.m_targetAcquisitionAngle = missileDataTable->getFloatValue("target_acquisition_angle",row);

		m_missileTypeData[missileDataTable->getIntValue("missile_type_id",row)] = data;
	}
	DataTableManager::close("datatables/space/missiles.iff");
}

// ----------------------------------------------------------------------

MissileManager::~MissileManager()
{
}

// ----------------------------------------------------------------------

bool MissileManager::requestFireMissile(const NetworkId &sourcePlayer, const NetworkId &sourceShip, const NetworkId &targetShip, int weaponIndex, int missileType, ShipChassisSlotType::Type targetedComponent)
{
	Missile * const newMissile = addMissile(sourcePlayer, sourceShip, weaponIndex, missileType, targetShip, targetedComponent);
	
	if(newMissile->fire())
	{
		return true;
	}
	else
	{
		IGNORE_RETURN(removeMissile(newMissile->getMissileId()));
		return false;
	}
}

// ----------------------------------------------------------------------

/**
 * Called when a client or AI requests launching a countermeasure
 */
void MissileManager::requestFireCountermeasure(const NetworkId &player, const NetworkId &ship, const int weaponIndex) const
{
	// Countermeasure code is entirely driven by script
	Object *source = NetworkIdManager::getObjectById(ship);

	if (source)
	{
		ServerObject *sourceServerObject = NON_NULL(source->asServerObject());

		ScriptParams params;
		params.addParam(weaponIndex);
		params.addParam(player);
		IGNORE_RETURN (sourceServerObject->getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_FIRED_COUNTERMEASURE, params));
	}
}

// ----------------------------------------------------------------------

/**
 * Called by script to actually launch a countermeasure
 */
bool MissileManager::fireCountermeasure(NetworkId const & sourceShip, int const targetMissileId, bool const successful, int const countermeasureType)
{
	if (successful)
	{
		Missile * const missile = getMissile(targetMissileId);
		if (missile)
		{
			missile->countermeasure(sourceShip, countermeasureType);
			return true;
		}
		else
			return false;
	}
	else
	{
		ServerObject const * const sourceShipObject = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(sourceShip));
		if (sourceShipObject)
		{
			Missile::launchFailedCountermeasure(*sourceShipObject, targetMissileId, countermeasureType);
			return true;
		}
		else
			return false;
	}
}

// ----------------------------------------------------------------------

void MissileManager::update()
{
	if (!m_missileQueue.empty())
	{
		DEBUG_FATAL(!ServerClock::getInstance().isSet(),("A missile was launched before the clock was set.  This is illegal because missiles depend on the clock for their timing."));
		
		uint32 effectiveTime = ServerClock::getInstance().getGameTimeSeconds();

		while (!m_missileQueue.empty() && m_missileQueue.top().first <= effectiveTime)
		{
			Missile *nextMissile = getMissile(m_missileQueue.top().second);
			m_missileQueue.pop();
			if (nextMissile)
			{
				bool result = nextMissile->update(effectiveTime);
				if (!result)
					IGNORE_RETURN(removeMissile(nextMissile->getMissileId()));
			}
		}
	}
}

// ----------------------------------------------------------------------

void MissileManager::scheduleMissile(int missileId, uint32 nextUpdateTime)
{
	m_missileQueue.push(std::make_pair(nextUpdateTime, missileId));
}

// ----------------------------------------------------------------------

Missile * MissileManager::addMissile(const NetworkId &sourceShip, const NetworkId &sourcePlayer, int weaponIndex, int missileType, const NetworkId &targetShip, ShipChassisSlotType::Type targetedComponent)
{
	Missile * const newMissile = new Missile(sourceShip, sourcePlayer, weaponIndex, missileType, targetShip, targetedComponent);
	m_missiles[newMissile->getMissileId()]=newMissile;
	IGNORE_RETURN(m_missilesForTarget.insert(std::make_pair(targetShip, newMissile->getMissileId())));
	return newMissile;
}

// ----------------------------------------------------------------------

bool MissileManager::removeMissile(int missileId)
{
	MissilesType::iterator i=m_missiles.find(missileId);
	if (i==m_missiles.end())
	{
		WARNING_STRICT_FATAL(true,("called removeMissile with missile %i that doesn't exist.",missileId));
		return false;
	}
	NetworkId targetShip(i->second->getTargetShip());

	std::pair<MissilesForTargetType::iterator, MissilesForTargetType::iterator> range=m_missilesForTarget.equal_range(targetShip);
	for (MissilesForTargetType::iterator j=range.first; j!=range.second; ++j)
	{
		if (j->second == missileId)
		{
			m_missilesForTarget.erase(j);
			break;
		}
	}
	
	delete i->second;
	m_missiles.erase(i);

	return true;
}

// ----------------------------------------------------------------------

Missile * MissileManager::getMissile(int missileId)
{
	MissilesType::const_iterator i=m_missiles.find(missileId);
	if (i!=m_missiles.end())
		return i->second;
	else
		return nullptr;
}	

// ----------------------------------------------------------------------

const Missile * MissileManager::getConstMissile(int missileId) const
{
	MissilesType::const_iterator i=m_missiles.find(missileId);
	if (i!=m_missiles.end())
		return i->second;
	else
		return nullptr;
}

// ----------------------------------------------------------------------

/**
 * Get an unlocked missile aimed at the specified target.  If there is more
 * than one, chose the one with the least time left.
 */
int MissileManager::getNearestUnlockedMissileForTarget(const NetworkId &target) const
{
	const std::pair<MissilesForTargetType::const_iterator, MissilesForTargetType::const_iterator> range=m_missilesForTarget.equal_range(target);

	const Missile *targetedMissile=nullptr;
	for (MissilesForTargetType::const_iterator i=range.first; i!=range.second; ++i)
	{
		const Missile * const missile=getConstMissile(i->second);
		DEBUG_FATAL(!missile,("Programmer bug:  Missile %i was in m_missilesForTarget, but was not in m_missiles",i->second));
		if (missile && missile->getState() == Missile::MS_Launched &&
			(!targetedMissile || (missile->getImpactTime() < targetedMissile->getImpactTime()))) //lint !e774:  missile is non-nullptr in debug mode
				targetedMissile = missile;
	}

	if (targetedMissile)
		return targetedMissile->getMissileId();
	else
		return 0;
}

// ----------------------------------------------------------------------

/**
 * Returns true if the specified object is targeted by at least one missile.
 */
bool MissileManager::isTargetedByMissile(const NetworkId &target) const
{
	return (m_missilesForTarget.find(target) != m_missilesForTarget.end());
}

// ----------------------------------------------------------------------

/**
 * Get a list of all the unlocked missiles aimed at the specified target.
 */
void MissileManager::getAllUnlockedMissilesForTarget(const NetworkId &target, std::vector<int> &results) const
{
	const std::pair<MissilesForTargetType::const_iterator, MissilesForTargetType::const_iterator> range=m_missilesForTarget.equal_range(target);

	for (MissilesForTargetType::const_iterator i=range.first; i!=range.second; ++i)
	{
		const Missile * const missile=getConstMissile(i->second);
		DEBUG_FATAL(!missile,("Programmer bug:  Missile %i was in m_missilesForTarget, but was not in m_missiles",i->second));
		if (missile && missile->getState() == Missile::MS_Launched) //lint !e774 // not always true (in release mode)
			results.push_back(missile->getMissileId());
	}
}

// ----------------------------------------------------------------------

/**
 * Flag a missile for destruction.
 *
 * @param missileId The id of the missile to destroy.
 * @param countermeasured True if the missile was destroyed by a countermeasure.
 * This influences how the destruction is shown on the client, but has no
 * other function on the server.
 */
bool MissileManager::destroyMissile(int const missileId) 
{
	Missile *missile=getMissile(missileId);
	if (missile)
	{
		missile->destroy();
		return true;
	}
	else
		return false;
}

// ----------------------------------------------------------------------

const MissileManager::MissileTypeDataRecord * MissileManager::getMissileTypeDataRecord(int missileTypeId) const
{
	MissileTypeDataType::const_iterator i=m_missileTypeData.find(missileTypeId);
	if (i!=m_missileTypeData.end())
		return &(i->second);
	else
		return nullptr;
}

// ----------------------------------------------------------------------

float MissileManager::getCountermeasureTimeFactorByMissileType(int missileTypeId) const
{
	const MissileTypeDataRecord * data = getMissileTypeDataRecord(missileTypeId);
	DEBUG_WARNING(!data,("Requested missile data for missile type %i, which is not defined in the data table.",missileTypeId));
	if (!data)
		return 0;
	return data->m_countermeasureTimeFactor;
}

// ----------------------------------------------------------------------

float MissileManager::getMaxTimeByMissileType(int missileTypeId) const
{
	const MissileTypeDataRecord * data = getMissileTypeDataRecord(missileTypeId);
	DEBUG_WARNING(!data,("Requested missile data for missile type %i, which is not defined in the data table.",missileTypeId));
	if (!data)
		return 0;
	return data->m_maxTime;
}

// ----------------------------------------------------------------------

float MissileManager::getMinTimeByMissileType(int missileTypeId) const
{
	const MissileTypeDataRecord * data = getMissileTypeDataRecord(missileTypeId);
	DEBUG_WARNING(!data,("Requested missile data for missile type %i, which is not defined in the data table.",missileTypeId));
	if (!data)
		return 0;
	return data->m_minTime;
}

// ----------------------------------------------------------------------

float MissileManager::getSpeedByMissileType(int missileTypeId) const
{
	const MissileTypeDataRecord * data = getMissileTypeDataRecord(missileTypeId);
	DEBUG_WARNING(!data,("Requested missile data for missile type %i, which is not defined in the data table.",missileTypeId));
	if (!data)
		return 0;
	return data->m_speed;
}

// ----------------------------------------------------------------------

float MissileManager::getRangeByMissileType(int const missileTypeId) const
{
	MissileTypeDataRecord const * const data = getMissileTypeDataRecord(missileTypeId);
	DEBUG_WARNING(!data,("Requested missile data for missile type %i, which is not defined in the data table.",missileTypeId));
	if (!data)
		return 0;
	return data->m_speed * data->m_maxTime;
}

// ----------------------------------------------------------------------

float MissileManager::getTargetAcquisitionAngle(int const missileTypeId)
{
	MissileTypeDataRecord const * const data = getMissileTypeDataRecord(missileTypeId);
	DEBUG_WARNING(!data,("Requested missile data for missile type %i, which is not defined in the data table.",missileTypeId));
	if (!data)
		return 0.0f;

	return data->m_targetAcquisitionAngle;
}


// ======================================================================
