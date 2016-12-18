// ======================================================================
//
// ShipObject.cpp
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipObject.h"

#include "UnicodeUtils.h"
#include "serverGame/AiShipController.h"
#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/MiningAsteroidController.h"
#include "serverGame/MissileManager.h"
#include "serverGame/NebulaManagerServer.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlayerShipController.h"
#include "serverGame/ProjectileManager.h"
#include "serverGame/PvpUpdateObserver.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerShipObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipComponentDataBooster.h"
#include "serverGame/ShipComponentDataCapacitor.h"
#include "serverGame/ShipComponentDataEngine.h"
#include "serverGame/ShipComponentDataManager.h"
#include "serverGame/ShipComponentDataReactor.h"
#include "serverGame/ShipComponentDataShield.h"
#include "serverGame/ShipComponentDataWeapon.h"
#include "serverGame/ShipController.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/Nebula.h"
#include "sharedGame/NebulaManager.h"
#include "sharedGame/SharedShipObjectTemplate.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentFlags.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedGame/ShipSlotIdManager.h"
#include "sharedGame/ShipTurretManager.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <algorithm>

//lint -esym(749,ColumnData::CD_weaponCapacitorIndex)

// ======================================================================

namespace ShipObjectNamespace
{
	//-- Column data for cs_shipTypeFileName
	enum ColumnData
	{
		CD_name,
		CD_slideDampener,
		CD_maximumChassisHitPoints,
		CD_maximumChassisComponentMass
	};

	//-- ShipData holds data per ship type for cs_shipTypeFileName
	struct ShipData
	{
	public:

		ShipData();
		~ShipData();

	public:

		std::string m_name;
		float m_slideDampener;
		float m_maximumChassisHitPoints;
		float m_chassisComponentMassMaximum;
		ShipComponentData * m_components [static_cast<int>(ShipChassisSlotType::SCST_num_types)];
		int m_lotsConsumed;

	private:
		ShipData(ShipData const &); //lint -esym(754, ShipData::ShipData)
		ShipData & operator=(ShipData const &);
	};

	char const * const cs_shipTypeFileName = "datatables/ship/shiptype.iff";
	std::string const cs_defaultShipType = "default";

	typedef std::map<std::string, ShipData *> ShipTypeShipDataMap;
	ShipTypeShipDataMap ms_shipTypeShipDataMap;
	ShipData const * ms_defaultShipData = nullptr;

	ShipComponentDataEngine * ms_defaultEngine = nullptr;

	unsigned int s_lastShipId = 0;
	unsigned int const s_maxShipId = 4096;
	uint32 s_shipIdUsed[(s_maxShipId+31)/32];

	// ----------------------------------------------------------------------

	void remove();
	CreatureObject const *findPilotingCreature(ShipObject const &ship);
	void loadShipTypeDataTable(DataTable const &dataTable);
	uint16 allocateShipId();
	void freeShipId(uint16 shipId);

	// ----------------------------------------------------------------------

	float const s_boosterAvailableTimeMinimum = 2.0f;
	float const s_boosterAvailableTimeMaximum = 10.0f;

	float ms_shotValidationTolerance = 0.75f;
}

using namespace ShipObjectNamespace;

// ======================================================================
// PUBLIC ShipObjectNamespace::ShipData
// ======================================================================

ShipData::ShipData() :
m_name(),
m_slideDampener(0.f),
m_maximumChassisHitPoints(0.f),
m_chassisComponentMassMaximum(0.0f),
m_lotsConsumed(0)
{
	Zero(m_components); //lint !e1514 // (Creating temporary to copy 'ShipComponentData *[20]' to 'ShipComponentData **&' (context: arg. no. 1))
}

// ----------------------------------------------------------------------

ShipData::~ShipData()
{
	std::for_each (m_components, m_components + static_cast<int>(ShipChassisSlotType::SCST_num_types), PointerDeleter ());
}

// ======================================================================
// STATIC PUBLIC ShipObject
// ======================================================================

void ShipObject::install()
{
	DEBUG_FATAL(!ms_shipTypeShipDataMap.empty(), ("ShipObject::install - already installed"));
	ExitChain::add(ShipObjectNamespace::remove, "ShipObjectNamespace::remove");

	//-- Load the ship type data table
	Iff iff;
	if (!iff.open(cs_shipTypeFileName, true))
	{
		DEBUG_WARNING(true, ("ShipObject::install: could not open file %s", cs_shipTypeFileName));
		return;
	}

	DataTable dataTable;
	dataTable.load(iff);
	loadShipTypeDataTable(dataTable);
	DataTableManager::addReloadCallback(cs_shipTypeFileName, loadShipTypeDataTable);

	ShipComponentDescriptor const * const genericEngineDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByName(ConstCharCrcString("eng_generic"));
	FATAL(genericEngineDescriptor == nullptr, ("ShipObject genericEngineDescriptor [eng_generic] not found"));
	ms_defaultEngine = new ShipComponentDataEngine(*genericEngineDescriptor);

	// mark shipId 0 as used
	memset(s_shipIdUsed, 0, sizeof(s_shipIdUsed));
	s_shipIdUsed[0] = 1;

	ms_shotValidationTolerance = ConfigFile::getKeyBool("GameServer", "shotValidationTolerance", ms_shotValidationTolerance);
}

// ----------------------------------------------------------------------

class ShipObject::FireShotQueueData
{
public:
	ConstWatcher<Client> gunnerClient; //lint !e1925 //public
	int weaponIndex; //lint !e1925 //public
	Transform transform_p; //lint !e1925 //public
	int targetComponent; //lint !e1925 //public
	uint32 syncStampLong; //lint !e1925 //public

	FireShotQueueData(Client const &_gunnerClient, int _weaponIndex, Transform const & _transform_p, int _targetComponent, uint32 _syncStampLong) :
		gunnerClient(&_gunnerClient),
		weaponIndex(_weaponIndex),
		transform_p(_transform_p),
		targetComponent(_targetComponent),
		syncStampLong(_syncStampLong)
	{
	}

	FireShotQueueData() :
		gunnerClient(0),
		weaponIndex(0),
		transform_p(),
		targetComponent(0),
		syncStampLong(0)
	{
	}
};

// ----------------------------------------------------------------------

ShipObject const * ShipObject::getContainingShipObject(ServerObject const * serverObject)
{
	return getContainingShipObject(const_cast<ServerObject *>(serverObject));
}

// ----------------------------------------------------------------------

ShipObject * ShipObject::getContainingShipObject(ServerObject * serverObject)
{
	while (serverObject && !serverObject->asShipObject())
		serverObject = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*serverObject));

	return serverObject ? serverObject->asShipObject() : 0;
}

// ======================================================================
// STATIC PRIVATE ShipObject
// ======================================================================

void ShipObjectNamespace::remove()
{
	ms_defaultShipData = nullptr;

	delete ms_defaultEngine;
	ms_defaultEngine = nullptr;

	//-- Delete the ship type to ship data map
	std::for_each(ms_shipTypeShipDataMap.begin(), ms_shipTypeShipDataMap.end(), PointerDeleterPairSecond());
	ms_shipTypeShipDataMap.clear();
}

// ======================================================================
// PUBLIC ShipObject
// ======================================================================

float const ShipObject::ms_powerPulsePeriodSecs = 1.5f;

// ----------------------------------------------------------------------

ShipObject::ShipObject(ServerShipObjectTemplate const *newTemplate) :
	TangibleObject(newTemplate),
	m_shipId(0),
	m_slideDampener(0.f),
	m_currentYaw(0.f),
	m_currentPitch(0.f),
	m_currentRoll(0.f),
	m_currentChassisHitPoints(0.f),
	m_maximumChassisHitPoints(0.f),
	m_weaponRefireTimers(nullptr),
	m_numberOfHits(0),
	m_chassisType                      (0),
	m_chassisComponentMassMaximum(0.0f),
	m_chassisComponentMassCurrent(0.0f),
	m_chassisSpeedMaximumModifier(1.0f),
	m_shipActualAccelerationRate       (0.0f),
	m_shipActualDecelerationRate       (0.0f),
	m_shipActualPitchAccelerationRate  (0.0f),
	m_shipActualYawAccelerationRate    (0.0f),
	m_shipActualRollAccelerationRate   (0.0f),
	m_shipActualPitchRateMaximum       (0.0f),
	m_shipActualYawRateMaximum         (0.0f),
	m_shipActualRollRateMaximum        (0.0f),
	m_shipActualSpeedMaximum           (0.0f),
	m_componentArmorHitpointsMaximum   (),
	m_componentArmorHitpointsCurrent   (),
	m_componentEfficiencyGeneral       (),
	m_componentEfficiencyEnergy        (),
	m_componentEnergyMaintenanceRequirement   (),
	m_componentMass                    (),
	m_componentCrc                     (),
	m_componentCrcForClient            (),
	m_componentHitpointsCurrent        (),
	m_componentHitpointsMaximum        (),
	m_componentFlags                   (),
	m_componentNames(),
	m_weaponDamageMaximum              (),
	m_weaponDamageMinimum              (),
	m_weaponEffectivenessShields       (),
	m_weaponEffectivenessArmor         (),
	m_weaponEnergyPerShot              (),
	m_weaponRefireRate                 (),
	m_weaponEfficiencyRefireRate(),
	m_weaponAmmoCurrent                (),
	m_weaponAmmoMaximum                (),
	m_weaponAmmoType                   (),
	m_shieldHitpointsFrontCurrent      (0.0f),
	m_shieldHitpointsFrontMaximum      (0.0f),
	m_shieldHitpointsBackCurrent       (0.0f),
	m_shieldHitpointsBackMaximum       (0.0f),
	m_shieldRechargeRate               (0.0f),
	m_capacitorEnergyCurrent           (0.0f),
	m_capacitorEnergyMaximum           (0.0f),
	m_capacitorEnergyRechargeRate      (0.0f),
	m_engineAccelerationRate           (0.0f),
	m_engineDecelerationRate           (0.0f),
	m_enginePitchAccelerationRate      (0.0f),
	m_engineYawAccelerationRate        (0.0f),
	m_engineRollAccelerationRate       (0.0f),
	m_enginePitchRateMaximum           (0.0f),
	m_engineYawRateMaximum             (0.0f),
	m_engineRollRateMaximum            (0.0f),
	m_engineSpeedMaximum               (0.0f),
	m_engineSpeedRotationFactorMaximum(1.0f),
	m_engineSpeedRotationFactorMinimum(1.0f),
	m_engineSpeedRotationFactorOptimal(0.5f),
	m_reactorEnergyGenerationRate      (0.0f),
	m_boosterEnergyCurrent             (0.0f),
	m_boosterEnergyMaximum             (0.0f),
	m_boosterEnergyRechargeRate        (0.0f),
	m_boosterEnergyConsumptionRate     (0.0f),
	m_boosterAcceleration              (0.0f),
	m_boosterSpeedMaximum              (0.0f),
	m_droidInterfaceCommandSpeed       (0.0f),
	m_installedDroidControlDevice      (),
	m_cargoHoldContentsMaximum(0),
	m_cargoHoldContentsCurrent(0),
	m_cargoHoldContents(),
	m_cargoHoldContentsResourceTypeInfo(),
	m_timeSinceLastPowerPulse          (0.0f),
	m_fireShotQueue(new FireShotQueue),
	m_nebulas(new IntVector),
	m_nebulaEffectReactor(0.0f),
	m_nebulaEffectEngine(0.0f),
	m_nebulaEffectShields(0.0f),
	m_pilotLookAtTarget(),
	m_pilotLookAtTargetSlot(ShipChassisSlotType::SCST_invalid),
	m_targetableSlotBitfield      (),
	m_turretTargets(),
	m_wingName(),
	m_typeName(),
	m_difficulty(),
	m_faction(),
	m_guildId(),
	m_spaceFaction(0),
	m_spaceFactionAllies(),
	m_spaceFactionEnemies(),
	m_spaceFactionIsAggro(false),
	m_lotsConsumed(0),
	m_autoAggroImmuneTimer(new Timer),
	m_damageAggroImmune(false),
	m_turretWeaponIndices(new BitArray),
	m_movementPercent(1.0f),
	m_invulnerabilityTimer(0.f),
	m_boosterAvailableTimer(new Timer(0.0f)),
	m_fastestWeaponProjectileSpeed(0.0f)
{
	m_weaponRefireTimers = new Timer[ShipChassisSlotType::cms_numWeaponIndices];

	calcPvpableState();

	//--
	addMembersToPackages();

	//-- Extract data from the ship data table
	constructFromTemplate();

	SharedObjectTemplate const * const sharedObjectTemplate = getSharedTemplate();
	if (!sharedObjectTemplate)
	{
		WARNING(true, ("ShipObject [%s] unable to load shared template [%s]", newTemplate->getName(), newTemplate->getSharedTemplate().c_str()));
	}
	else
	{
		SharedShipObjectTemplate const * const sharedShipObjectTemplate = safe_cast<SharedShipObjectTemplate const *>(sharedObjectTemplate);
		if (!sharedShipObjectTemplate)
		{
			WARNING(true, ("ShipObject [%s] shared template [%s] is not a SharedShipObjectTemplate", newTemplate->getName(), newTemplate->getSharedTemplate().c_str()));
		}
		else
		{
			setLocalFlag(LocalObjectFlags::ShipObject_PlayerControlled, sharedShipObjectTemplate->getPlayerControlled());
			setLocalFlag(LocalObjectFlags::ShipObject_HasWings, sharedShipObjectTemplate->getHasWings());
		}
	}
}

// ----------------------------------------------------------------------

ShipObject::~ShipObject()
{
	//-- This must be the first line in the destructor to invalidate any watchers watching this object
	nullWatchers();

	delete m_boosterAvailableTimer;
	m_boosterAvailableTimer = nullptr;
	
	if (getLocalFlag(LocalObjectFlags::ShipObject_ShipIdAssigned))
		freeShipId(m_shipId.get());

	delete m_fireShotQueue;
	m_fireShotQueue = nullptr;

	delete[] m_weaponRefireTimers;
	m_weaponRefireTimers = nullptr;

	delete m_nebulas;
	m_nebulas = nullptr;

	delete m_autoAggroImmuneTimer;
	delete m_turretWeaponIndices;
}

// ----------------------------------------------------------------------

void ShipObject::endBaselines()
{
	// Assure we don't have a stale shipId (breaks AutoDelta rules because there is not an appropriate package for m_shipId)
	if (!getLocalFlag(LocalObjectFlags::ShipObject_ShipIdAssigned))
		m_shipId = 0;

	TangibleObject::endBaselines();

	if (isAuthoritative())
	{
		m_componentCrcForClient.clear();
		for (Archive::AutoDeltaPackedMap<int, uint32>::const_iterator it = m_componentCrc.begin(); it != m_componentCrc.end(); ++it)
		{
			int const key = (*it).first;
			uint32 const value = (*it).second;
			m_componentCrcForClient.set(key, value);
		}
		internalSetChassisComponentMassCurrent(computeChassisComponentMassCurrent());

		if (isPlayerShip())
			m_guildId = GuildInterface::getGuildId(getOwnerId());
	}

	if (!getAppearance())
	{
		PortalProperty * const portalProperty = getPortalProperty();
		if (portalProperty)
			portalProperty->createAppearance();
	}
	
	//-- obtain the resource type info
	{
		m_cargoHoldContentsResourceTypeInfo.clear();
		NetworkIdIntMap::const_iterator const end = m_cargoHoldContents.end();
		for (NetworkIdIntMap::const_iterator it = m_cargoHoldContents.begin(); it != end; ++it)
		{
			NetworkId const & resourceTypeId = (*it).first;
			
			ResourceTypeObject const * const resourceTypeObject = ServerUniverse::getInstance().getResourceTypeById(resourceTypeId);
			if (nullptr == ServerUniverse::getInstance().getResourceTypeById(resourceTypeId))
			{
				WARNING(true, ("ShipObject::endBaselines invalid resource type [%s]", resourceTypeId.getValueString().c_str()));
				continue;
			}
			m_cargoHoldContentsResourceTypeInfo.set(
				resourceTypeId, 
				ResourceTypeInfoPair(Unicode::narrowToWide(resourceTypeObject->getResourceName()), resourceTypeObject->getParentClass().getResourceClassName()));
		}
	}
}

// ----------------------------------------------------------------------

void ShipObject::virtualOnSetAuthority()
{
	if (isPlayerShip())
		m_guildId = GuildInterface::getGuildId(getOwnerId());
	TangibleObject::virtualOnSetAuthority();
}

// ----------------------------------------------------------------------

ShipObject * ShipObject::asShipObject()
{
	return this;
}

// ----------------------------------------------------------------------

ShipObject const * ShipObject::asShipObject() const
{
	return this;
}

// ----------------------------------------------------------------------

void ShipObject::onAddedToWorld()
{
	if (isPlayerShip() && !getClient())
		setInvulnerabilityTimer(ConfigServerGame::getShipLoadInvulnerableTimeWithoutClient());

	NON_NULL(safe_cast<ShipController *>(getController()))->onAddedToWorld();
	TangibleObject::onAddedToWorld();
}

// ----------------------------------------------------------------------

void ShipObject::onContainerGainItem(ServerObject &item, ServerObject *source, ServerObject *transferer)
{
	// notify of gunner add if needed
	if (item.isPlayerControlled())
		handleGunnerChange(item);

	TangibleObject::onContainerGainItem(item, source, transferer);
}

// ----------------------------------------------------------------------

void ShipObject::onContainerChildGainItem(ServerObject &item, ServerObject *source, ServerObject *transferer)
{
	// If an object with a client has been transferred into a subcontainer of a
	// ship, and is not in the world, they need auth client baselines.
	Client const * const client = item.getClient();
	if (client && !item.isInWorld())
		sendAuthClientBaselines(*client);

	// notify of gunner add if needed
	if (item.isPlayerControlled())
		handleGunnerChange(item);

	// chain to base class
	TangibleObject::onContainerChildGainItem(item, source, transferer);
}

// ----------------------------------------------------------------------

void ShipObject::onContainerLostItem(ServerObject *destination, ServerObject &item, ServerObject *transferer)
{
	// notify of gunner remove if needed
	if (item.isPlayerControlled())
		handleGunnerChange(item);

	TangibleObject::onContainerLostItem(destination, item, transferer);
}

// ----------------------------------------------------------------------

void ShipObject::onContainerChildLostItem(ServerObject *destination, ServerObject &item, ServerObject *source, ServerObject *transferer)
{
	// notify of gunner remove if needed
	if (item.isPlayerControlled())
		handleGunnerChange(item);

	TangibleObject::onContainerChildLostItem(destination, item, source, transferer);
}

// ----------------------------------------------------------------------

bool ShipObject::isVisibleOnClient(Client const &client) const
{
	if (!TangibleObject::isVisibleOnClient(client))
		return false;

	// In ground scenes, ships in the world are only visible to clients they contain, and gods
	if (client.isGod() || !isInWorld() || ServerWorld::isSpaceScene())
		return true;

	for (ServerObject const *o = client.getCharacterObject(); o; o = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*o)))
		if (o == this)
			return true;

	return false;
}

// ----------------------------------------------------------------------

void ShipObject::onLoadingScreenComplete()
{
	TangibleObject::onLoadingScreenComplete();

	if (isInvulnerable())
		setInvulnerabilityTimer(ConfigServerGame::getShipLoadInvulnerableTimeWithClient());
}

// ----------------------------------------------------------------------

float ShipObject::getCurrentSpeed() const
{
	return NON_NULL(safe_cast<const ShipController *>(getController()))->getSpeed();
}

// ----------------------------------------------------------------------

bool ShipObject::setSlideDampener(float slideDampener)
{
	m_slideDampener = slideDampener;
	return true;
}

// ----------------------------------------------------------------------

float ShipObject::getSlideDampener() const
{
	return m_slideDampener.get();
}

// ----------------------------------------------------------------------

bool ShipObject::setCurrentYaw(float const currentYaw)
{
	FATAL(!isAuthoritative(), ("ShipObject::setCurrentYaw: called on non-authoritative object [%s]", getNetworkId().getValueString().c_str()));

	if (currentYaw >= 0.f && currentYaw <= getShipActualYawRateMaximum())
	{
		m_currentYaw = currentYaw;
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

float ShipObject::getCurrentYaw() const
{
	return m_currentYaw;
}

// ----------------------------------------------------------------------

bool ShipObject::setCurrentPitch(float const currentPitch)
{
	FATAL(!isAuthoritative(), ("ShipObject::setCurrentPitch: called on non-authoritative object [%s]", getNetworkId().getValueString().c_str()));

	if (currentPitch >= 0.f && currentPitch <= getShipActualPitchRateMaximum())
	{
		m_currentPitch = currentPitch;
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

float ShipObject::getCurrentPitch() const
{
	return m_currentPitch;
}

// ----------------------------------------------------------------------

bool ShipObject::setCurrentRoll(float const currentRoll)
{
	FATAL(!isAuthoritative(), ("ShipObject::setCurrentRoll: called on non-authoritative object [%s]", getNetworkId().getValueString().c_str()));

	if (currentRoll >= 0.f && currentRoll <= getShipActualRollRateMaximum())
	{
		m_currentRoll = currentRoll;
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

float ShipObject::getCurrentRoll() const
{
	return m_currentRoll;
}

// ----------------------------------------------------------------------

bool ShipObject::setCurrentChassisHitPoints(float const currentChassisHitPoints)
{
	FATAL(!isAuthoritative(), ("ShipObject::setCurrentChassisHitPoints: called on non-authoritative object [%s]", getNetworkId().getValueString().c_str()));

	if (currentChassisHitPoints >= 0.f && currentChassisHitPoints <= m_maximumChassisHitPoints.get())
	{
		m_currentChassisHitPoints = currentChassisHitPoints;
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

float ShipObject::getCurrentChassisHitPoints() const
{
	return m_currentChassisHitPoints.get();
}

// ----------------------------------------------------------------------

bool ShipObject::setMaximumChassisHitPoints(float const maximumChassisHitPoints)
{
	FATAL(!isAuthoritative(), ("ShipObject::setMaximumChassisHitPoints: called on non-authoritative object [%s]", getNetworkId().getValueString().c_str()));

	if (maximumChassisHitPoints >= 0.f)
	{
		m_maximumChassisHitPoints = maximumChassisHitPoints;
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

float ShipObject::getMaximumChassisHitPoints() const
{
	return m_maximumChassisHitPoints.get();
}

// ----------------------------------------------------------------------

/**
 * Updates the ship.
 *
 * @param time		current game time
 *
 * @return alter result
 */
float ShipObject::alter(float const elapsedTime)
{
	PROFILER_AUTO_BLOCK_DEFINE("ShipObject::alter");

	if (!isInWorld())
		return TangibleObject::alter(elapsedTime);

	if (!m_boosterAvailableTimer->isExpired())
		m_boosterAvailableTimer->updateNoReset(elapsedTime);

	if (m_invulnerabilityTimer)
	{
		PvpUpdateObserver o(this, Archive::ADOO_generic);
		m_invulnerabilityTimer = std::max(0.f, m_invulnerabilityTimer-elapsedTime);
	}

	//----------------------------------------------------------------------
	//-- update the list of nebulas containing the ship

	{
		PROFILER_AUTO_BLOCK_DEFINE("nebulas");

		m_nebulaEffectReactor = 0.0f;
		m_nebulaEffectEngine = 0.0f;
		m_nebulaEffectShields = 0.0f;

		static IntVector oldNebulas;
		oldNebulas = *m_nebulas;

		m_nebulas->clear();

		static NebulaManager::NebulaVector nebulaVector;
		nebulaVector.clear();
		NebulaManager::getNebulasAtPosition(getPosition_w(), nebulaVector);
		if (!nebulaVector.empty())
		{
			m_nebulas->reserve(nebulaVector.size());
			for (NebulaManager::NebulaVector::const_iterator it = nebulaVector.begin(); it != nebulaVector.end(); ++it)
			{
				Nebula const * const nebula = NON_NULL(*it);
				int const id = nebula->getId();
				m_nebulas->push_back(id);

				if (!std::binary_search(oldNebulas.begin(), oldNebulas.end(), id))
				{
					//-- todo: trigger script
				}

				m_nebulaEffectReactor += nebula->getEffectReactor();
				m_nebulaEffectEngine += nebula->getEffectEngine();
				m_nebulaEffectShields += nebula->getEffectShields();

			}

			std::sort(m_nebulas->begin(), m_nebulas->end());
		}

		for (IntVector::const_iterator it = oldNebulas.begin(); it != oldNebulas.end(); ++it)
		{
			int const id = *it;
			if (!std::binary_search(m_nebulas->begin(), m_nebulas->end(), id))
			{
				//-- todo: trigger script
			}
			else
			{
				NebulaManagerServer::handleEnvironmentalDamage(*this, id);
			}
		}

		m_nebulaEffectReactor = clamp(-0.5f, m_nebulaEffectReactor, 1.0f);
		m_nebulaEffectEngine = clamp(-0.5f, m_nebulaEffectEngine, 1.0f);
		m_nebulaEffectShields = clamp(-2.0f, m_nebulaEffectShields, 1.0f);

	}

	//----------------------------------------------------------------------

	if (isAuthoritative())
	{
		//-- Weapon system updates
		for (int i = 0; i < ShipChassisSlotType::cms_numWeaponIndices; ++i)
		{
			ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(i + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));
			m_weaponRefireTimers [i].setExpireTime(getWeaponActualRefireRate (weaponChassisSlotType));
			IGNORE_RETURN(m_weaponRefireTimers [i].updateNoReset(elapsedTime));
		}

		m_timeSinceLastPowerPulse += elapsedTime;

		if (m_timeSinceLastPowerPulse > ms_powerPulsePeriodSecs)
		{
			handlePowerPulse (ms_powerPulsePeriodSecs);
			m_timeSinceLastPowerPulse -= ms_powerPulsePeriodSecs;
		}

		//-- fire all queued shots
		if (!m_fireShotQueue->empty())
		{
			for (FireShotQueue::iterator it = m_fireShotQueue->begin(); it != m_fireShotQueue->end();)
			{
				FireShotQueueData const & fireShotQueueData = *it;
				//-- Make sure our refire timer has expired.  This is necessary because long server frames can
				//-- cause multiple shots on the same weapon to be enqueued in the same frame
				if (m_weaponRefireTimers[fireShotQueueData.weaponIndex].isExpired())
					internalHandleFireShot(fireShotQueueData.gunnerClient, fireShotQueueData.weaponIndex, fireShotQueueData.transform_p, fireShotQueueData.syncStampLong, true, NetworkId::cms_invalid, static_cast<ShipChassisSlotType::Type>(fireShotQueueData.targetComponent), false);

				it = m_fireShotQueue->erase(it);
			}
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("compute");

			PlayerShipController * const controller = isPlayerShip() ? safe_cast<PlayerShipController *>(getController()) : 0;

			{
				float const accelerationRate = computeShipActualAccelerationRate();
				if (accelerationRate != m_shipActualAccelerationRate.get())
				{
					m_shipActualAccelerationRate = accelerationRate;
					if (controller)
						controller->onShipAccelerationRateChanged(accelerationRate);
				}
			}

			{
				float const decelerationRate = computeShipActualDecelerationRate();
				if (decelerationRate != m_shipActualDecelerationRate.get())
				{
					m_shipActualDecelerationRate = decelerationRate;
					if (controller)
						controller->onShipDecelerationRateChanged(decelerationRate);
				}
			}

			m_shipActualPitchAccelerationRate   = computeShipActualPitchAccelerationRate();
			m_shipActualYawAccelerationRate     = computeShipActualYawAccelerationRate();
			m_shipActualRollAccelerationRate    = computeShipActualRollAccelerationRate();

			{
				float const pitchRateMaximum = computeShipActualPitchRateMaximum();
				float const yawRateMaximum = computeShipActualYawRateMaximum();
				float const rollRateMaximum = computeShipActualRollRateMaximum();
				if (   pitchRateMaximum != m_shipActualPitchRateMaximum.get()
				    || yawRateMaximum != m_shipActualYawRateMaximum.get()
				    || rollRateMaximum != m_shipActualRollRateMaximum.get())
				{
					m_shipActualPitchRateMaximum = pitchRateMaximum;
					m_shipActualYawRateMaximum = yawRateMaximum;
					m_shipActualRollRateMaximum = rollRateMaximum;
					if (controller)
						controller->onShipYprMaximumChanged(std::max(std::max(pitchRateMaximum, yawRateMaximum), rollRateMaximum));
				}
			}

			{
				float const speedMaximum = computeShipActualSpeedMaximum();
				if (speedMaximum != m_shipActualSpeedMaximum.get())
				{
					m_shipActualSpeedMaximum = speedMaximum;
					if (controller)
						controller->onShipSpeedMaximumChanged(speedMaximum);
				}
			}
		}
		
		IGNORE_RETURN(m_autoAggroImmuneTimer->updateNoReset(elapsedTime));
		
		if (getShieldHitpointsFrontCurrent() > 0.0f)
			IGNORE_RETURN(setComponentFlags(ShipChassisSlotType::SCST_shield_0, getComponentFlags(ShipChassisSlotType::SCST_shield_0) | ShipComponentFlags::F_shieldsFront));
		else if (isSlotInstalled(ShipChassisSlotType::SCST_shield_0))
			IGNORE_RETURN(setComponentFlags(ShipChassisSlotType::SCST_shield_0, getComponentFlags(ShipChassisSlotType::SCST_shield_0) & ~ShipComponentFlags::F_shieldsFront));
		
		if (getShieldHitpointsBackCurrent() > 0.0f)
			IGNORE_RETURN(setComponentFlags(ShipChassisSlotType::SCST_shield_0, getComponentFlags(ShipChassisSlotType::SCST_shield_0) | ShipComponentFlags::F_shieldsBack));
		else if (isSlotInstalled(ShipChassisSlotType::SCST_shield_0))
			IGNORE_RETURN(setComponentFlags(ShipChassisSlotType::SCST_shield_0, getComponentFlags(ShipChassisSlotType::SCST_shield_0) & ~ShipComponentFlags::F_shieldsBack));
	}
	else
		m_timeSinceLastPowerPulse = 0.0f;

	float alterResult = TangibleObject::alter(elapsedTime);
	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

// ----------------------------------------------------------------------

/**
 * Creates a controller for the ship.
 *
 * @return the new controller
 */
Controller *ShipObject::createDefaultController()
{
	Controller * controller = 0;
	if (isPlayerShip())
		controller = new PlayerShipController(this);
	else if (getGameObjectType() == SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic ||
			getGameObjectType() == SharedObjectTemplate::GOT_ship_mining_asteroid_static)
		controller = new MiningAsteroidController(this);
	else
		controller = new AiShipController(this);

	setController(controller);

	return controller;
}

// ----------------------------------------------------------------------

/**
 * Get the ship's pilot, if any.
 *
 * @return the pilot if there is one, otherwise nullptr
 */
CreatureObject *ShipObject::getPilot()
{
	return const_cast<CreatureObject *>(findPilotingCreature(*this));
} //lint !e1762 logically nonconst

// ----------------------------------------------------------------------

CreatureObject const *ShipObject::getPilot() const
{
	return findPilotingCreature(*this);
}

// ----------------------------------------------------------------------

void ShipObject::findAllPassengers(std::vector<CreatureObject *> & passengers, bool onlyPlayerControlled)
{
	if (!getPortalProperty())
	{
		SlottedContainer const * const slottedContainer = getSlottedContainerProperty();
		if (slottedContainer != 0)
		{
			ContainerConstIterator ii = slottedContainer->begin();
			ContainerConstIterator iiEnd = slottedContainer->end();

			for (; ii != iiEnd; ++ii)
			{
				Object * const object = (*ii).getObject();
				ServerObject * const serverObject = (object != 0) ? object->asServerObject() : 0;
				CreatureObject * const creatureObject = (serverObject != 0) ? serverObject->asCreatureObject() : 0;
				if (creatureObject != 0)
				{
					if ((onlyPlayerControlled) && (!creatureObject->isPlayerControlled()))
					{
						continue;
					}
					passengers.push_back(creatureObject);
				}
			}
		}
	}

	else
	{
		// ship is a POB ship
		Container const * const pobContainer = ContainerInterface::getContainer(*this);
		if (pobContainer != 0)
		{
			ContainerConstIterator ii = pobContainer->begin();
			ContainerConstIterator iiEnd = pobContainer->end();
			for (; ii != iiEnd; ++ii)
			{
				Object * const containerObject = (*ii).getObject();
				ServerObject * const cell = (containerObject != 0) ? containerObject->asServerObject() : 0;

				if (cell != 0)
				{
					Container const * const cellContainer = ContainerInterface::getContainer(*cell);

					if (cellContainer != 0)
					{
						ContainerConstIterator jj = cellContainer->begin();
						ContainerConstIterator jjEnd = cellContainer->end();
						for (; jj != jjEnd; ++jj)
						{
							Object * const cellObject = (*jj).getObject();
							ServerObject * const cellContent = (cellObject != 0) ? cellObject->asServerObject() : 0;
							CreatureObject * const cellCreatureObject = (cellContent != 0) ? cellContent->asCreatureObject() : 0;

							if (cellCreatureObject != 0)
							{
								if ((onlyPlayerControlled) && (!cellCreatureObject->isPlayerControlled()))
								{
									continue;
								}
								passengers.push_back(cellCreatureObject);
							}
							else
							{
								SlottedContainer const * const slottedContainer = (cellContent != 0) ? cellContent->getSlottedContainerProperty() : 0;

								if (slottedContainer != 0)
								{
									ContainerConstIterator kk = slottedContainer->begin();
									ContainerConstIterator kkEnd = slottedContainer->end();

									for (; kk != kkEnd; ++kk)
									{
										Object * const object = (*kk).getObject();
										ServerObject * const serverObject = (object != 0) ? object->asServerObject() : 0;
										CreatureObject * const slotCreatureObject = (serverObject != 0) ? serverObject->asCreatureObject() : 0;

										if (slotCreatureObject != 0)
										{
											if ((onlyPlayerControlled) && (!slotCreatureObject->isPlayerControlled()))
											{
												continue;
											}
					
											passengers.push_back(slotCreatureObject);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

bool ShipObject::hasWings() const
{
	return getLocalFlag(LocalObjectFlags::ShipObject_HasWings);
}

// ----------------------------------------------------------------------

bool ShipObject::wingsOpened() const
{
	return hasCondition(static_cast<int>(C_wingsOpened));
}

// ----------------------------------------------------------------------

void ShipObject::openWings()
{
	setCondition(static_cast<int>(C_wingsOpened));
}

// ----------------------------------------------------------------------

void ShipObject::closeWings()
{
	clearCondition(static_cast<int>(C_wingsOpened));
}

// ----------------------------------------------------------------------

bool ShipObject::hasEnergyForAShot(int const weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return false;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));

	float const energyRequired = getWeaponActualEnergyPerShot(weaponChassisSlotType);

	if (energyRequired <= 0.0f)
		return true;

	if (isSlotInstalled(ShipChassisSlotType::SCST_capacitor) && isComponentFunctional(ShipChassisSlotType::SCST_capacitor))
		return energyRequired <= getCapacitorEnergyCurrent ();

	return false;
}

// ----------------------------------------------------------------------

bool ShipObject::hasAmmoForAShot(int const weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return false;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));

	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);

	if (componentCrc)
	{
		if (ShipComponentWeaponManager::isAmmoConsuming(componentCrc))
		{
			int const ammoAvailable = getWeaponAmmoCurrent (weaponChassisSlotType);
			return ammoAvailable > 0;
		}
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void ShipObject::internalHandleFireShot(Client const *gunnerClient, int const weaponIndex, Transform const & transform_p, uint32 const syncStampLong, bool clientShot, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, bool const fromAutoTurret)
{
	if (!canFireShot(weaponIndex))
		return;

	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));
	Transform serverTransform_p(transform_p);
	float deltaTime = 0.f;

	bool const isBeam = isBeamWeapon(weaponIndex);

	//-- client shots must be synchronized
	if (clientShot && !isBeam)
	{
		//-- Adjust to where the shot would be now even though it was fired on the client
		if (gunnerClient)
		{
			deltaTime = gunnerClient->computeDeltaTimeInSeconds(syncStampLong);
			DEBUG_WARNING(deltaTime < 0.f, ("ShipObject::internalHandleFireShot: deltaTime %1.2f computed for object [id=%s, template=%s] is < 0", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		}

		serverTransform_p.move_l(Vector::unitZ * deltaTime * getWeaponProjectileSpeed(weaponIndex));
	}

	//-- non-client shots don't need validation
	if (clientShot && !validateShot(weaponIndex, transform_p))
		return;

	CreatureObject const *gunnerCreature = gunnerClient ? safe_cast<CreatureObject const *>(gunnerClient->getCharacterObject()) : 0;

	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);

	bool const isMissileWeapon = ShipComponentWeaponManager::isMissile(componentCrc);
	bool const isCountermeasureWeapon = ShipComponentWeaponManager::isCountermeasure(componentCrc);

	NetworkId missileTargetId = targetId;

	if (!missileTargetId.isValid())
	{
		//-- find the target for the missile if applicable
		if (isMissileWeapon)
		{
			if (!gunnerCreature)
			{
				WARNING(true, ("ShipObject::internalHandleFireShot cannot fire missile with a nullptr gunner"));
				return;
			}

			missileTargetId = gunnerCreature->getLookAtTarget();

			if (!missileTargetId.isValid())
			{
				StringId cantFireStringId("space_pilot_command", "cannot_fire_without_lookat");
				Chat::sendSystemMessage(*gunnerCreature, cantFireStringId, Unicode::emptyString);
				return;
			}
		}
	}

	//-- Reset the refire timer
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, weaponIndex, ShipChassisSlotType::cms_numWeaponIndices);
	m_weaponRefireTimers[weaponIndex].reset();

	if (isMissileWeapon)
	{
		uint32 const ammoType = getWeaponAmmoType(weaponChassisSlotType);
		if (!MissileManager::getInstance().requestFireMissile(gunnerCreature ? gunnerCreature->getNetworkId() : NetworkId::cms_invalid, getNetworkId(), missileTargetId, weaponIndex, static_cast<int>(ammoType), targetedComponent))
		{
			if (gunnerCreature)
			{
				StringId cantFireStringId("space_pilot_command", "gunner_fire_missile_failed");
				Chat::sendSystemMessage(*gunnerCreature, cantFireStringId, Unicode::emptyString);
			}
			return;
		}
	}
	else if (isCountermeasureWeapon)
	{
		MissileManager::getInstance().requestFireCountermeasure(gunnerCreature ? gunnerCreature->getNetworkId() : NetworkId::cms_invalid, getNetworkId(), weaponIndex);
	}
	else
	{
		int const projectileIndex = getProjectileIndexForWeapon(weaponIndex);
		float const projectileTimeToLive = computeWeaponProjectileTimeToLive(weaponIndex);
		float const projectileSpeed = getWeaponProjectileSpeed(weaponIndex);

		float const timeToLive = projectileTimeToLive - deltaTime;
		if (timeToLive > 0.0f)
			ProjectileManager::create(gunnerClient, *this, weaponIndex, projectileIndex, targetedComponent, deltaTime, serverTransform_p, 10.f, projectileSpeed, timeToLive, fromAutoTurret);
	}

	//-- deduct the energy and ammo used
	float const energyRequired = getWeaponActualEnergyPerShot(weaponChassisSlotType);
	float const newEnergy = getCapacitorEnergyCurrent () - energyRequired;
	IGNORE_RETURN(setCapacitorEnergyCurrent (newEnergy));

	if (ShipComponentWeaponManager::isAmmoConsuming(componentCrc))
		IGNORE_RETURN(setWeaponAmmoCurrent(weaponChassisSlotType, getWeaponAmmoCurrent(weaponChassisSlotType) - 1));
}

// ----------------------------------------------------------------------

void ShipObject::enqueueFireShotClient(Client const &gunnerClient, int const weaponIndex, Transform const & transform_p, int targetComponent, uint32 const syncStampLong)
{
	m_fireShotQueue->push_back(FireShotQueueData(gunnerClient, weaponIndex, transform_p, targetComponent, syncStampLong));
}

// ----------------------------------------------------------------------

/**
 * Shoot at the specified target, if possible.
 */
void ShipObject::fireShotNonTurretServer(int const weaponIndex, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, float const yawErrorRadians, float const pitchErrorRadians)
{
	DEBUG_FATAL(isTurret(weaponIndex),("Programmer bug:  called fireShotNonTurretServer on object %s, weapon %i, but the weapon is a turret.",getNetworkId().getValueString().c_str(), weaponIndex));

	Transform fireTransform = getTransform_o2p(); 

	if (pitchErrorRadians > FLT_MIN)
	{
		float const pitch = pitchErrorRadians * 0.5f;
		fireTransform.pitch_l(Random::randomReal(-pitch, pitch));
	}
	
	if (yawErrorRadians > FLT_MIN)
	{
		float const yaw = yawErrorRadians * 0.5f;
		fireTransform.yaw_l(Random::randomReal(-yaw, yaw));
	}

	internalHandleFireShot(0, weaponIndex, fireTransform, 0, false, targetId, targetedComponent, false);
}

// ----------------------------------------------------------------------

/**
 * Shoot a turret at the specified target, if possible.
 *
 * @param turretMissYaw the amount of yaw to miss the target by
 * @param turretMissYaw the amount of pitch to miss the target by
 */
void ShipObject::fireShotTurretServer(int const weaponIndex, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, bool const goodShot, bool const fromAutoTurret)
{
	PROFILER_AUTO_BLOCK_DEFINE("ShipObject::FireShotTurretServer");
	DEBUG_FATAL(!isTurret(weaponIndex),("Programmer bug:  called fireShotTurretServer on object %s, weapon %i, but the weapon is not a turret.",getNetworkId().getValueString().c_str(), weaponIndex));

	Object const * const targetObject = NetworkIdManager::getObjectById(targetId);

	if (targetObject == nullptr)
	{
		WARNING(true, ("ERROR: The targetId(%s) could not be resolved to an Object. A turret shot will NOT be fired.", targetId.getValueString().c_str()));
		return;
	}

	ServerObject const * const targetServerObject = targetObject->asServerObject();
	ShipObject const * const targetShipObject = (targetServerObject != nullptr) ? targetServerObject->asShipObject() : nullptr;

	if (   !targetShipObject
	    && goodShot)
	{
		WARNING(true, ("ERROR: The targetId(%s) could not be resolved to a ShipObject. Perfect shot requested, but there is no way to lead with a perfect shot.", targetId.getValueString().c_str()));
	}

	if (   (targetShipObject != nullptr)
	    && goodShot)
	{
		// If its a good shot, lead perfectly

		Transform const & turretTransform = getTurretTransform(weaponIndex);
		Vector const & turretPosition_p = turretTransform.getPosition_p();
		float const weaponProjectileSpeed = getWeaponProjectileSpeed(weaponIndex);
		Vector const & perfectShotPosition_p = targetShipObject->getTargetLead_p(turretPosition_p, weaponProjectileSpeed);
		Vector const k(perfectShotPosition_p - turretPosition_p);
		Vector const j(Vector::perpendicular(k));

		Transform perfectShotTransform;
		perfectShotTransform.setPosition_p(turretPosition_p);
		perfectShotTransform.setLocalFrameKJ_p(k, j);
		perfectShotTransform.reorthonormalize();
		
		internalHandleFireShot(0, weaponIndex, perfectShotTransform, 0, false, targetId, targetedComponent, fromAutoTurret);
	}
	else
	{
		// If its a bad shot, miss with greater error the further away the target

		Transform const & turretTransform = getTurretTransform(weaponIndex);
		Vector const & turretPosition_p = turretTransform.getPosition_p();
		Vector const & targetPosition_w = targetObject->getPosition_w();
		Vector const directionToTargetPosition_w(targetPosition_w - turretPosition_p);
		float const approximateDistanceToTarget = directionToTargetPosition_w.approximateMagnitude();

		// Calculate the error for the shot in local space

		Vector badShotPosition_l;
		{
			ShipController const * const ownerShipController = getController()->asShipController();
			DEBUG_WARNING(!ownerShipController, ("ERROR: Controller could not be resolved to a ShipController(%s)", getDebugInformation().c_str()));
			
			float const missHalfAngle = (ownerShipController != nullptr) ? (ownerShipController->getTurretMissAngle() / 2.0f) : 0.0f;
			float const targetRadius = (targetServerObject != nullptr) ? targetServerObject->getRadius() : 0.0f;

			Transform transform;
			transform.roll_l(Random::randomReal() * PI_TIMES_2);

			badShotPosition_l = transform.rotate_l2p(Vector(0.0f, targetRadius + missHalfAngle * approximateDistanceToTarget, 0.0f));
		}

		// Calculate the final shot position in world space

		Vector badShotPosition_w;
		{
			Transform transform;
			transform.setPosition_p(turretPosition_p);
			transform.setLocalFrameKJ_p(directionToTargetPosition_w, Vector::perpendicular(directionToTargetPosition_w));
			transform.reorthonormalize();

			float const x = badShotPosition_l.x;
			float const y = badShotPosition_l.y;
			float const z = approximateDistanceToTarget;

			badShotPosition_w = transform.rotateTranslate_l2p(Vector(x, y, z));
		}

		Vector const directionToBadShotPosition_w(badShotPosition_w - turretPosition_p);

		Transform badShotTransform;
		badShotTransform.setPosition_p(turretPosition_p);
		badShotTransform.setLocalFrameKJ_p(directionToBadShotPosition_w, Vector::perpendicular(directionToBadShotPosition_w));
		badShotTransform.reorthonormalize();

		internalHandleFireShot(0, weaponIndex, badShotTransform, 0, false, targetId, targetedComponent, fromAutoTurret);
	}
}

// ----------------------------------------------------------------------

int ShipObject::getNumberOfHits() const
{
	return m_numberOfHits;
}

// ----------------------------------------------------------------------

void ShipObject::setNumberOfHits(int const numberOfHits)
{
	m_numberOfHits = numberOfHits;
}

// ----------------------------------------------------------------------

TangibleObject const &ShipObject::getPvpViewer(Client const &client) const
{
	TangibleObject const * const clientObj = safe_cast<TangibleObject const *>(client.getCharacterObject());
	FATAL(!clientObj, ("Client trying to get pvp viewer with no character object?"));
	ShipObject const * const clientShip = getContainingShipObject(clientObj);
	if (clientShip)
		return *clientShip;
	return *clientObj;
}

// ----------------------------------------------------------------------

int ShipObject::getNumberOfLots() const
{
	return m_lotsConsumed;
}

// ----------------------------------------------------------------------

bool ShipObject::isAlwaysStationary() const
{
	return getGameObjectType() == static_cast<int>(SharedObjectTemplate::GOT_ship_station);
}

// ======================================================================
// PRIVATE ShipObject
// ======================================================================

void ShipObject::constructFromTemplate()
{
	//-- Get the ship object template
	ServerShipObjectTemplate const * const serverShipObjectTemplate = safe_cast<ServerShipObjectTemplate const *>(getObjectTemplate());

	//-- Find the ship data corresponding to the ship type
	ShipData const * shipData = ms_defaultShipData;
	std::string const & shipType = serverShipObjectTemplate->getShipType();
	
	if (!shipType.empty())
	{
		ShipTypeShipDataMap::iterator iter = ms_shipTypeShipDataMap.find(shipType);
		if (iter != ms_shipTypeShipDataMap.end())
			shipData = iter->second;
		else
			DEBUG_WARNING(true, ("Could not find shipType %s in %s, using default", shipType.c_str(), cs_shipTypeFileName));
	}
	
	m_slideDampener = shipData->m_slideDampener;
	m_maximumChassisHitPoints = shipData->m_maximumChassisHitPoints;
	m_chassisComponentMassMaximum = shipData->m_chassisComponentMassMaximum;
	m_currentChassisHitPoints = m_maximumChassisHitPoints;
	m_lotsConsumed = shipData->m_lotsConsumed;

	//-- setup ship chassis

	ShipChassis const * shipChassis = ShipChassis::findShipChassisByName (TemporaryCrcString (shipType.c_str (), true));
	if (shipChassis == nullptr)
	{
		DEBUG_WARNING (true, ("ShipObject::constructFromTemplate failed to find a valid ship chassis for [%s], trying generic", shipType.c_str ()));
		shipChassis = ShipChassis::findShipChassisByName (TemporaryCrcString ("generic", true));
	}

	if (shipChassis != nullptr)
		m_chassisType = shipChassis->getCrc ();
	else
	{
		DEBUG_WARNING (true, ("ShipObject::constructFromTemplate failed to find a valid ship chassis for [%s] or generic", shipType.c_str ()));
	}

	//set initial slot targetability from the chassis setttings.  Code or script can change these at runtime
	if(shipChassis != nullptr)
	{
		for (int slot = 0; slot < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++slot)
		{
			if(ShipChassis::isSlotTargetable(shipChassis->getCrc(), static_cast<ShipChassisSlotType::Type>(slot)))
				setSlotTargetable(static_cast<ShipChassisSlotType::Type>(slot));
			else
				clearSlotTargetable(static_cast<ShipChassisSlotType::Type>(slot));
		}
	}

	//-- setup pseudo components
	{
		for (int i = 0; i < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++i)
		{
			ShipComponentData * const shipComponentData = shipData->m_components[i];

			if (shipComponentData != nullptr)
			{
				if (!installComponentFromData(i, *shipComponentData)) {
					std::string chassis = shipChassis->getName().getString();
						
					DEBUG_WARNING((chassis != "player_yt1300"), ("ShipObject::constructFromTemplate() failed to install component at slot [%s]", 
						ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(i)).c_str()));
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

bool ShipObject::canFireShot(int const weaponIndex) const
{
	//-- Make sure if the ship has wings, that the wings are opened
	if (!isCountermeasure(weaponIndex) && hasWings() && !wingsOpened())
		return false;

	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return false;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));

	if (!isSlotInstalled(weaponChassisSlotType))
		return false;

	if (!isComponentFunctional(weaponChassisSlotType))
		return false;

	//-- Make sure we have enough energy
	if (!hasEnergyForAShot(weaponIndex))
		return false;

	if (!hasAmmoForAShot(weaponIndex))
		return false;

	//-- Make sure we have a refire rate
	if (getWeaponActualRefireRate(weaponChassisSlotType) <= 0.f)
		return false;

	//-- Make sure our refire timer has expired
	if (!m_weaponRefireTimers[weaponIndex].isExpired())
		return false;

	return true;
}

// ----------------------------------------------------------------------

bool ShipObject::validateShot(int const weaponIndex, Transform const & transform_p) const
{
	if (weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return false;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));

	//@todo: get weapon projectile index from weapon component
	if (!isSlotInstalled(weaponChassisSlotType))
		return false;

	if (!ConfigServerGame::getShipShotValidationEnabled())
		return true;

	Vector testDirection_p;
	Vector testPosition_p;
	float const maximumYawPitchRoll = std::max(getShipActualYawRateMaximum(), std::max(getShipActualPitchRateMaximum(), getShipActualRollRateMaximum()));
	float const testOrientationTolerance = maximumYawPitchRoll * ms_shotValidationTolerance;

	if (isTurret(weaponIndex))
	{
		// For turrets, we determine the direction within the rotation constraints
		// closest to the shot direction to use for the orientation test.

		Transform turretTransform_p = getTurretTransform(weaponIndex);
		Vector const direction_o = turretTransform_p.rotate_p2l(transform_p.getLocalFrameK_p());

		float yaw = direction_o.theta();
		float pitch = direction_o.phi();

		uint32 const chassisCrc = getChassisType();
		float const minYaw = ShipTurretManager::getTurretMinYaw(chassisCrc, weaponIndex);
		float const maxYaw = ShipTurretManager::getTurretMaxYaw(chassisCrc, weaponIndex);
		float const minPitch = ShipTurretManager::getTurretMinPitch(chassisCrc, weaponIndex);
		float const maxPitch = ShipTurretManager::getTurretMaxPitch(chassisCrc, weaponIndex);

    // clamp yaw and/or pitch if they are not allowed complete rotation
		if ((maxYaw-(minYaw+0.001)) < PI_TIMES_2)
			yaw = clamp(minYaw, yaw, maxYaw);
		if ((maxPitch-(minPitch+0.001)) < PI_TIMES_2)
			pitch = clamp(minPitch, pitch, maxPitch);

		turretTransform_p.yaw_l(yaw);
		turretTransform_p.pitch_l(pitch);

		testDirection_p = turretTransform_p.getLocalFrameK_p();
		testPosition_p = turretTransform_p.getPosition_p();
	}
	else
	{
		testDirection_p = getObjectFrameK_p();
		testPosition_p = getPosition_p();
	}

	//-- Is our angle too far out of sync?
	if (acos(transform_p.getLocalFrameK_p().dot(testDirection_p)) > testOrientationTolerance)
	{
		LOG("cheater", ("firing guns in wrong direction"));
		return false;
	}

	//-- Is our gun position too far from where we are? (max ship speed * ms_shotValidationTolerance)
	if (transform_p.getPosition_p().magnitudeBetweenSquared(testPosition_p) > sqr(getShipActualSpeedMaximum() * ms_shotValidationTolerance))
	{
		LOG("cheater", ("firing guns too far from current position"));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

ServerObject const *ShipObject::getControlDevice() const
{
	// If the ship is in a control device, return it
	ServerObject const * const containedBy = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));
	if (containedBy && containedBy->getGameObjectType() == static_cast<int>(SharedObjectTemplate::GOT_data_ship_control_device))
		return containedBy;

	// Find the owner of the ship, and search their datapad for an empty ship control device
	ServerObject const * const owner = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(getOwnerId()));
	if (owner)
	{
		CreatureObject const * const ownerCreature = owner->asCreatureObject();
		if (ownerCreature)
		{
			ServerObject const * const datapad = ownerCreature->getDatapad();
			if (datapad)
			{
				Container const * const container = ContainerInterface::getContainer(*datapad);
				if (container)
				{
					for (ContainerConstIterator i = container->begin(); i != container->end(); ++i)
					{
						ServerObject const * const content = safe_cast<ServerObject const *>((*i).getObject());
						if (   content
						    && content->getGameObjectType() == static_cast<int>(SharedObjectTemplate::GOT_data_ship_control_device)
						    && !content->isBeingDestroyed())
						{
							// content is a ship control device, but we only want to return it if it is empty
							bool empty = true;
							Container const * const scdContainer = ContainerInterface::getContainer(*content);
							if (scdContainer)
							{
								for (ContainerConstIterator j = scdContainer->begin(); j != scdContainer->end(); ++j)
								{
									ServerObject const * const scdContent = safe_cast<ServerObject const *>((*j).getObject());
									if (scdContent && !scdContent->isBeingDestroyed())
									{
										empty = false;
										break;
									}
								}
							}

							if (empty)
								return content;
						}
					}
				}
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

void ShipObject::calcPvpableState()
{
	setPvpable(true);
}

// ----------------------------------------------------------------------

NetworkId ShipObject::getInstalledDroidControlDevice() const
{
	return m_installedDroidControlDevice.get();
}

// ----------------------------------------------------------------------

void ShipObject::setInstalledDroidControlDevice(NetworkId const & droidControlDevice)
{
	m_installedDroidControlDevice.set(droidControlDevice);
}

// ----------------------------------------------------------------------

void ShipObject::setPilotLookAtTarget(const NetworkId& id)
{
	if(isAuthoritative())
	{
		m_pilotLookAtTarget.set(id);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setLookAtTarget, new MessageQueueGenericValueType<NetworkId>(id));
	}

	CreatureObject * const pilot = getPilot();
	if(pilot)
	{
		pilot->setLookAtTarget(id);
	}

	setPilotLookAtTargetSlot (ShipChassisSlotType::SCST_num_types);
}

// ----------------------------------------------------------------------

void ShipObject::setLookAtTargetFromPilot(const NetworkId& id)
{
	if(isAuthoritative())
	{
		m_pilotLookAtTarget.set(id);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setLookAtTarget, new MessageQueueGenericValueType<NetworkId>(id));
	}

	setPilotLookAtTargetSlot (ShipChassisSlotType::SCST_num_types);
}

// ----------------------------------------------------------------------

ShipComponentDataEngine const & ShipObject::getShipDataEngine() const
{
	ServerShipObjectTemplate const * const serverShipObjectTemplate = safe_cast<ServerShipObjectTemplate const *>(getObjectTemplate());

	//-- Find the ship data corresponding to the ship type
	ShipData const * shipData = ms_defaultShipData;
	std::string const & shipType = serverShipObjectTemplate->getShipType();
	
	if (!shipType.empty())
	{
		ShipTypeShipDataMap::iterator iter = ms_shipTypeShipDataMap.find(shipType);
		if (iter != ms_shipTypeShipDataMap.end())
			shipData = iter->second;
		else
			DEBUG_WARNING(true, ("Could not find shipType %s in %s, using default", shipType.c_str(), cs_shipTypeFileName));
	}

	ShipComponentDataEngine const * const engine = safe_cast<ShipComponentDataEngine const *>(shipData->m_components[static_cast<size_t>(ShipChassisSlotType::SCST_engine)]);

	if (engine == nullptr)
		return *ms_defaultEngine;

	return *engine;
}

// ----------------------------------------------------------------------

void ShipObject::handleGunnerChange(ServerObject const &player)
{
	int gunnerWeaponIndex = -1;

	SlottedContainmentProperty const * const slottedContainmentProperty = ContainerInterface::getSlottedContainmentProperty(player);
	if (slottedContainmentProperty)
	{
		int const currentArrangement = slottedContainmentProperty->getCurrentArrangement();
		if (currentArrangement != -1)
		{
			SlottedContainmentProperty::SlotArrangement const &slotArrangement = slottedContainmentProperty->getSlotArrangement(currentArrangement);
			for (SlottedContainmentProperty::SlotArrangement::const_iterator i = slotArrangement.begin(); i != slotArrangement.end(); ++i)
			{
				int const weaponIndex = ShipSlotIdManager::getGunnerSlotWeaponIndex(*i);
				if (weaponIndex != -1)
					gunnerWeaponIndex = weaponIndex;
			}
		}
	}

	ShipController * const shipController = getController()->asShipController();
	PlayerShipController * const playerShipController = (shipController != nullptr) ? shipController->asPlayerShipController() : nullptr;

	if (playerShipController != nullptr)
	{
		playerShipController->updateGunnerWeaponIndex(player.getNetworkId(), gunnerWeaponIndex);
	}
}

// ----------------------------------------------------------------------

void ShipObject::getAuthClients(std::set<Client const *> &authClients) const
{
	if (!isPlayerShip())
		return;

	// Build the set of clients which should receive authClient packages.
	// This should mean only clients with this object in their containment
	// chain which are in slots of containers, but to make the check
	// cheaper we actually check for not in world rather than in slot.
	std::set<Client *> const &observers = getObservers();
	for (std::set<Client *>::const_iterator i = observers.begin(); i != observers.end(); ++i)
	{
		ServerObject * const characterObject = (*i)->getCharacterObject();
		if (characterObject && !characterObject->isInWorld())
		{
			for (ServerObject const *so = characterObject; so; so = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*so)))
				if (so == this)
					IGNORE_RETURN(authClients.insert(*i));
		}
	}
	// Also, we want any client we are contained by to receive authClient packages.
	{
		for (ServerObject const *so = this; so; so = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*so)))
		{
			Client const * const client = so->getClient();
			if (client)
				IGNORE_RETURN(authClients.insert(client));
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Returns a vector representing the ship's current velocity (speed * direction of travel)
 */
Vector ShipObject::getCurrentVelocity_p() const
{
	return NON_NULL(safe_cast<const ShipController *>(getController()))->getVelocity();
}

// ----------------------------------------------------------------------

/**
 * Given the starting position and the speed of a projectile, compute a
 * point to aim at that would hit this ship.
 */
Vector ShipObject::getTargetLead_p(const Vector & startPosition_p, float projectileSpeed) const
{
	//TODO:  There is also a formula for this on the client.  It is a little bit different, but it is derived from the same trig principles.  We should consolidate the two formulas (or pick the fastest one) and put them in a shared library

	DEBUG_FATAL((projectileSpeed <= 0.0f), ("ShipObject::getTargetLead_p() Invalid projectile speed specified(%.f), must be > 0.0", projectileSpeed));

	if (projectileSpeed <= 0.0f)
	{
		return startPosition_p;
	}

	// Step 1:  figure out the angles involved
	Vector normalizedVelocity = getCurrentVelocity_p();
	if (!normalizedVelocity.normalize())
		return getPosition_p();

	Vector normalizedToTarget = getPosition_p() - startPosition_p;
	if (!normalizedToTarget.normalize())
		return getPosition_p();

	float cosineAlpha = normalizedVelocity.dot(normalizedToTarget);
	float alpha = acos (cosineAlpha);
	float sineAlpha = sin(alpha);
	float sineBeta = sineAlpha * getCurrentSpeed() / projectileSpeed;
	float beta = asin (sineBeta);
	float gamma = (PI-alpha)-beta; //lint !e578 //hides global ::gamma
	float sineGamma = sin(gamma);

	// Step 2:  figure out the time to target
	float time = 0.0f;
	
	if ((getCurrentSpeed() * sineGamma) > 0.0f)
	{
		time = (getPosition_p().magnitudeBetween(startPosition_p) * sineBeta) / (getCurrentSpeed() * sineGamma);
	}

	// Step 3:  figure out where we'd be by then
	Vector const endLocation_p(getPosition_p() + getCurrentVelocity_p() * time);

	return endLocation_p;
}

// ----------------------------------------------------------------------

bool ShipObject::isCapitalShip() const
{
	SharedObjectTemplate::GameObjectType got = static_cast<SharedObjectTemplate::GameObjectType>(getGameObjectType());
	return got == SharedObjectTemplate::GOT_ship_capital || got == SharedObjectTemplate::GOT_ship_station;
}

// ----------------------------------------------------------------------

uint32 ShipObject::getSpaceFaction() const
{
	return m_spaceFaction;
}

// ----------------------------------------------------------------------

std::vector<uint32> const &ShipObject::getSpaceFactionAllies() const
{
	return m_spaceFactionAllies;
}

// ----------------------------------------------------------------------

std::vector<uint32> const &ShipObject::getSpaceFactionEnemies() const
{
	return m_spaceFactionEnemies;
}

// ----------------------------------------------------------------------

bool ShipObject::getSpaceFactionIsAggro() const
{
	return m_spaceFactionIsAggro;
}

// ----------------------------------------------------------------------

void ShipObject::setSpaceFaction(uint32 spaceFaction)
{
	DEBUG_FATAL(!isAuthoritative(), ("ShipObject::setSpaceFaction called for %s while nonauthoritative", getNetworkId().getValueString().c_str()));
	PvpUpdateObserver o(this, Archive::ADOO_generic);
	m_spaceFaction = spaceFaction;
}

// ----------------------------------------------------------------------

void ShipObject::setSpaceFactionAllies(std::vector<uint32> const &spaceFactionAllies)
{
	DEBUG_FATAL(!isAuthoritative(), ("ShipObject::setSpaceFactionAllies called for %s while nonauthoritative", getNetworkId().getValueString().c_str()));
	PvpUpdateObserver o(this, Archive::ADOO_generic);
	m_spaceFactionAllies = spaceFactionAllies;
}

// ----------------------------------------------------------------------

void ShipObject::setSpaceFactionEnemies(std::vector<uint32> const &spaceFactionEnemies)
{
	DEBUG_FATAL(!isAuthoritative(), ("ShipObject::setSpaceFactionEnemies called for %s while nonauthoritative", getNetworkId().getValueString().c_str()));
	PvpUpdateObserver o(this, Archive::ADOO_generic);
	m_spaceFactionEnemies = spaceFactionEnemies;
}

// ----------------------------------------------------------------------

void ShipObject::setSpaceFactionIsAggro(bool isAggro)
{
	DEBUG_FATAL(!isAuthoritative(), ("ShipObject::setSpaceFactionIsAggro called for %s while nonauthoritative", getNetworkId().getValueString().c_str()));
	PvpUpdateObserver o(this, Archive::ADOO_generic);
	m_spaceFactionIsAggro = isAggro;
}

// ----------------------------------------------------------------------

ShipObject::IntVector const & ShipObject::getNebulas() const
{
	return *NON_NULL(m_nebulas);
}

// ----------------------------------------------------------------------

void ShipObject::setWingName(std::string const & wingName)
{
	m_wingName.set(wingName);
}

// ----------------------------------------------------------------------

std::string const & ShipObject::getWingName() const
{
	return m_wingName.get();
}

// ----------------------------------------------------------------------

void ShipObject::setTypeName(std::string const & typeName)
{
	m_typeName.set(typeName);
}

// ----------------------------------------------------------------------

std::string const & ShipObject::getTypeName() const
{
	return m_typeName.get();
}

// ----------------------------------------------------------------------

void ShipObject::setDifficulty(std::string const & difficulty)
{
	m_difficulty.set(difficulty);
}

// ----------------------------------------------------------------------

std::string const & ShipObject::getDifficulty() const
{
	return m_difficulty.get();
}

// ----------------------------------------------------------------------

void ShipObject::setFaction(std::string const & faction)
{
	m_faction.set(faction);
}

// ----------------------------------------------------------------------

std::string const & ShipObject::getFaction() const
{
	return m_faction.get();
}

// ----------------------------------------------------------------------

bool ShipObject::hasTurrets() const
{
	return !m_turretWeaponIndices->empty();
}

// ----------------------------------------------------------------------

/**
 * Returns the approximate distance at which the ship can shoot a target, i.e.
 * the longest range of any installed weapons (excluding missiles).
 * Does not take into account the position of the weapons on the ship, only
 * their range.
 */

float ShipObject::getApproximateAttackRange(bool const includeMissiles) const
{
	float maxSoFar = 0.0f;
	for(int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		ShipChassisSlotType::Type const chassisSlot = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex);
		uint32 const componentCrc = getComponentCrc(chassisSlot);
		
		if (   isSlotInstalled(chassisSlot)
			&& !ShipComponentWeaponManager::isCountermeasure(componentCrc)
		    && (includeMissiles || !ShipComponentWeaponManager::isMissile(componentCrc)))
		{
			float const range = getWeaponRange(weaponIndex);
			if (range > maxSoFar)
				maxSoFar = range;
		}
	}
	return maxSoFar;
}

// ----------------------------------------------------------------------

void ShipObject::onTargetedByMissile(int const missileId)
{
	NON_NULL(safe_cast<ShipController *>(getController()))->onTargetedByMissile(missileId);	
}

// ----------------------------------------------------------------------

void ShipObject::setAutoAggroImmuneTime(float const time) const
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipObject::setAutoAggroImmuneTime() owner(%s) time(%.2f)", getNetworkId().getValueString().c_str(), time));

	m_autoAggroImmuneTimer->setExpireTime(time);
	m_autoAggroImmuneTimer->reset();
}

// ----------------------------------------------------------------------

bool ShipObject::isAutoAggroImmune() const
{
	return WithinEpsilonInclusive(m_autoAggroImmuneTimer->getExpireTime(), -1.0f, 0.0001f) || !m_autoAggroImmuneTimer->isExpired();
}

// ----------------------------------------------------------------------

void ShipObject::setDamageAggroImmune(bool const enabled)
{
	m_damageAggroImmune = enabled;
}

// ----------------------------------------------------------------------

bool ShipObject::isDamageAggroImmune() const
{
	return m_damageAggroImmune;
}

// ----------------------------------------------------------------------

Vector ShipObject::getInterceptPosition(float timeDelta) const
{
	return getPosition_w() + (getCurrentVelocity_p() * timeDelta);
}

// ----------------------------------------------------------------------

CreatureObject const *ShipObjectNamespace::findPilotingCreature(ShipObject const &ship)
{
	if (!ship.getPortalProperty())
	{
		// non-portallized ship, so check ship_pilot slot of ship
		SlottedContainer const * const slottedContainer = ship.getSlottedContainerProperty();
		if (slottedContainer)
		{
			Container::ContainerErrorCode err = Container::CEC_Success;
			Container::ContainedItem const item = slottedContainer->getObjectInSlot(ShipSlotIdManager::getShipPilotSlotId(), err);
			ServerObject const * const so = safe_cast<ServerObject const *>(item.getObject());
			if (so)
				return so->asCreatureObject();
		}
	}
	else
	{
		// portallized ship, so check ship_pilot_pob slot of immediate contents of cells
		Container const * const pobContainer = ContainerInterface::getContainer(ship);
		if (pobContainer)
		{
			for (ContainerConstIterator i = pobContainer->begin(); i != pobContainer->end(); ++i)
			{
				ServerObject const * const cell = safe_cast<ServerObject const *>((*i).getObject());
				if (cell)
				{
					Container const * const cellContainer = ContainerInterface::getContainer(*cell);
					for (ContainerConstIterator j = cellContainer->begin(); j != cellContainer->end(); ++j)
					{
						ServerObject const * const cellContent = safe_cast<ServerObject const *>((*j).getObject());
						SlottedContainer const * const slottedContainer = cellContent->getSlottedContainerProperty();
						if (slottedContainer)
						{
							Container::ContainerErrorCode err = Container::CEC_Success;
							Container::ContainedItem const item = slottedContainer->getObjectInSlot(ShipSlotIdManager::getPobShipPilotSlotId(), err);
							ServerObject const * const so = safe_cast<ServerObject const *>(item.getObject());
							if (so)
								return so->asCreatureObject();
						}
					}
				}
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

void ShipObjectNamespace::loadShipTypeDataTable(DataTable const &dataTable)
{
	std::for_each(ms_shipTypeShipDataMap.begin(), ms_shipTypeShipDataMap.end(), PointerDeleterPairSecond());
	ms_shipTypeShipDataMap.clear();

	int const numberOfRows = dataTable.getNumRows();
	for (int row = 0; row < numberOfRows; ++row)
	{
		std::string const name = dataTable.getStringValue(static_cast<int>(CD_name), row);
		if (ms_shipTypeShipDataMap.find(name) != ms_shipTypeShipDataMap.end())
		{
			DEBUG_WARNING(true, ("ShipObjectNamespace::loadShipTypeDataTable: file %s specifies duplicate shipType %s", cs_shipTypeFileName, name.c_str()));
			continue;
		}

		ShipData * const shipData = new ShipData;
		shipData->m_name = name;
		shipData->m_slideDampener = dataTable.getFloatValue(static_cast<int>(CD_slideDampener), row);
		shipData->m_maximumChassisHitPoints = dataTable.getFloatValue(static_cast<int>(CD_maximumChassisHitPoints), row);
		shipData->m_chassisComponentMassMaximum = dataTable.getFloatValue(static_cast<int>(CD_maximumChassisComponentMass), row);
		shipData->m_lotsConsumed = dataTable.getIntValue("lots_consumed", row);

		//-- find components
		for (int i = 0; i < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++i)
		{
			shipData->m_components [i] = nullptr;
			std::string const & slotName = ShipChassisSlotType::getNameFromType (static_cast<ShipChassisSlotType::Type>(i));
			if (!dataTable.doesColumnExist (slotName))
				continue;

			std::string const & componentName = dataTable.getStringValue (slotName, row);
			if (componentName.empty ())
				continue;

			uint32 const componentCrc = Crc::normalizeAndCalculate (componentName.c_str ());
			ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc (componentCrc);

			if (shipComponentDescriptor == nullptr)
				WARNING (true, ("ShipObject ship type [%s] specified invalid [%s] component [%s]", name.c_str (), slotName.c_str (), componentName.c_str ()));
			else
				shipData->m_components[i] = ShipComponentDataManager::create(*shipComponentDescriptor);

			ShipComponentData * const shipComponentData = shipData->m_components[i];

			if (shipComponentData == nullptr)
				continue;

			//-- get common data
			const std::string colname_armor = slotName + "_arm";
			const std::string colname_hitpoints = slotName + "_hp";

			shipComponentData->m_armorHitpointsCurrent = dataTable.getFloatValue(colname_armor, row);
			shipComponentData->m_armorHitpointsMaximum = shipComponentData->m_armorHitpointsCurrent;
			shipComponentData->m_hitpointsCurrent = dataTable.getFloatValue(colname_hitpoints, row);
			shipComponentData->m_hitpointsMaximum = shipComponentData->m_hitpointsCurrent;

			//-- get component-specific data
			switch (shipComponentData->getDescriptor().getComponentType())
			{
			case ShipComponentType::SCT_reactor:
				{
					ShipComponentDataReactor * const reactor = safe_cast<ShipComponentDataReactor *>(shipComponentData);

					//-- npc ships reactors' generate unlimited energy
					reactor->m_energyGenerationRate = 999999999.0f;
				}
				break;
			case ShipComponentType::SCT_engine:
				{
					ShipComponentDataEngine * const engine = safe_cast<ShipComponentDataEngine *>(shipComponentData);

					static std::string const colname_accelerationRate = slotName + "_accel";
					static std::string const colname_decelerationRate = slotName + "_decel";
					static std::string const colname_pitchAccelerationRate = slotName + "_pitch_accel";
					static std::string const colname_yawAccelerationRate = slotName + "_yaw_accel";
					static std::string const colname_rollAccelerationRate = slotName + "_roll_accel";
					static std::string const colname_pitchRateMaximum = slotName + "_pitch";
					static std::string const colname_yawRateMaximum = slotName + "_yaw";
					static std::string const colname_rollRateMaximum = slotName + "_roll";
					static std::string const colname_speedMaximum = slotName + "_speed";

					engine->m_engineAccelerationRate = dataTable.getFloatValue(colname_accelerationRate, row);
					engine->m_engineDecelerationRate = dataTable.getFloatValue(colname_decelerationRate, row);
					engine->m_enginePitchAccelerationRate = convertDegreesToRadians(dataTable.getFloatValue(colname_pitchAccelerationRate, row));
					engine->m_engineYawAccelerationRate = convertDegreesToRadians(dataTable.getFloatValue(colname_yawAccelerationRate, row));
					engine->m_engineRollAccelerationRate = convertDegreesToRadians(dataTable.getFloatValue(colname_rollAccelerationRate, row));
					engine->m_enginePitchRateMaximum = convertDegreesToRadians(dataTable.getFloatValue(colname_pitchRateMaximum, row));
					engine->m_engineYawRateMaximum = convertDegreesToRadians(dataTable.getFloatValue(colname_yawRateMaximum, row));
					engine->m_engineRollRateMaximum = convertDegreesToRadians(dataTable.getFloatValue(colname_rollRateMaximum, row));
					engine->m_engineSpeedMaximum = dataTable.getFloatValue(colname_speedMaximum, row);
				}
				break;
			case ShipComponentType::SCT_shield:
				{
					ShipComponentDataShield * const shield = safe_cast<ShipComponentDataShield *>(shipComponentData);

					static std::string const colname_hitpointsFront = slotName + "_front";
					static std::string const colname_hitpointsBack = slotName + "_back";
					static std::string const colname_rechargeRate = slotName + "_regen";

					shield->m_shieldHitpointsFrontCurrent = dataTable.getFloatValue(colname_hitpointsFront, row);
					shield->m_shieldHitpointsFrontMaximum = shield->m_shieldHitpointsFrontCurrent;
					shield->m_shieldHitpointsBackCurrent = dataTable.getFloatValue(colname_hitpointsBack, row);
					shield->m_shieldHitpointsBackMaximum = shield->m_shieldHitpointsBackCurrent;
					shield->m_shieldRechargeRate = dataTable.getFloatValue(colname_rechargeRate, row);
				}
				break;
			case ShipComponentType::SCT_armor:
				//-- nothing to do for armor
				break;
			case ShipComponentType::SCT_weapon:
				{
					ShipComponentDataWeapon * const weapon = safe_cast<ShipComponentDataWeapon *>(shipComponentData);

					std::string const colname_weaponDamageMaximum = slotName + "_dam_max";
					std::string const colname_weaponDamageMinimum = slotName + "_dam_min";
					std::string const colname_weaponEffectivenessShields = slotName + "_eff_sh";
					std::string const colname_weaponEffectivenessArmor = slotName + "_eff_arm";
					std::string const colname_weaponEnergyPerShot = slotName + "_drain";
					std::string const colname_weaponRefireRate = slotName + "_rate";
					std::string const colname_weaponAmmoCurrent = slotName + "_ammo";
					std::string const colname_weaponAmmoType = slotName + "_ammo_type";

					weapon->m_weaponDamageMaximum = dataTable.getFloatValue(colname_weaponDamageMaximum, row);
					weapon->m_weaponDamageMinimum = dataTable.getFloatValue(colname_weaponDamageMinimum, row);
					weapon->m_weaponEffectivenessShields = dataTable.getFloatValue(colname_weaponEffectivenessShields, row);
					weapon->m_weaponEffectivenessArmor = dataTable.getFloatValue(colname_weaponEffectivenessArmor, row);
					weapon->m_weaponEnergyPerShot = dataTable.getFloatValue(colname_weaponEnergyPerShot, row);
					weapon->m_weaponRefireRate = dataTable.getFloatValue(colname_weaponRefireRate, row);
					weapon->m_weaponAmmoCurrent = dataTable.getIntValue(colname_weaponAmmoCurrent, row);
					weapon->m_weaponAmmoMaximum = weapon->m_weaponAmmoCurrent;
					weapon->m_weaponAmmoType = static_cast<uint32>(dataTable.getIntValue(colname_weaponAmmoType, row));
				}
				break;
			case ShipComponentType::SCT_capacitor:
				{
					ShipComponentDataCapacitor * const capacitor = safe_cast<ShipComponentDataCapacitor *>(shipComponentData);

					static std::string const colname_energy = slotName + "_energy";
					static std::string const colname_recharge = slotName + "_recharge";

					capacitor->m_capacitorEnergyCurrent = dataTable.getFloatValue(colname_energy, row);
					capacitor->m_capacitorEnergyMaximum = capacitor->m_capacitorEnergyCurrent;
					capacitor->m_capacitorEnergyRechargeRate = dataTable.getFloatValue(colname_recharge, row);
				}
				break;
			case ShipComponentType::SCT_booster:
				{
					ShipComponentDataBooster * const booster = safe_cast<ShipComponentDataBooster *>(shipComponentData);

					static std::string const colname_energy = slotName + "_energy";
					static std::string const colname_recharge = slotName + "_recharge";
					static std::string const colname_consumption = slotName + "_consumption";
					static std::string const colname_accel = slotName + "_accel";
					static std::string const colname_speed = slotName + "_speed";

					booster->m_boosterEnergyCurrent = dataTable.getFloatValue(colname_energy, row);
					booster->m_boosterEnergyMaximum = booster->m_boosterEnergyCurrent;
					booster->m_boosterEnergyRechargeRate = dataTable.getFloatValue(colname_recharge, row);
					booster->m_boosterEnergyConsumptionRate = dataTable.getFloatValue(colname_consumption, row);
					booster->m_boosterAcceleration = dataTable.getFloatValue(colname_accel, row);
					booster->m_boosterSpeedMaximum = dataTable.getFloatValue(colname_speed, row);
				}
				break;
			case ShipComponentType::SCT_droid_interface:
				break;
			case ShipComponentType::SCT_bridge:
				break;
			case ShipComponentType::SCT_hangar:
				break;
			case ShipComponentType::SCT_targeting_station:
				break;
			case ShipComponentType::SCT_cargo_hold:
				break;
			case ShipComponentType::SCT_modification:
				break;
			case ShipComponentType::SCT_num_types:
				break;
			}
		}

		std::pair<ShipTypeShipDataMap::iterator, bool> result = ms_shipTypeShipDataMap.insert(std::make_pair(name, shipData));
		DEBUG_FATAL(!result.second, ("ShipObjectNamespace::loadShipTypeDataTable: Failed to insert %s", name.c_str()));
		UNREF(result);
	}

	//-- Extract the default ship data
	ShipTypeShipDataMap::iterator iter = ms_shipTypeShipDataMap.find(cs_defaultShipType);
	if (iter != ms_shipTypeShipDataMap.end())
		ms_defaultShipData = iter->second;
	else
		DEBUG_WARNING(true, ("ShipObjectNamespace::loadShipTypeDataTable: file %s does not specify shipType %s", cs_shipTypeFileName, cs_defaultShipType.c_str()));
}

// ----------------------------------------------------------------------

void ShipObject::hearText(ServerObject const &source, MessageQueueSpatialChat const &spatialChat, int chatMessageIndex)
{
	CreatureObject const * const sourceCreature = source.asCreatureObject();
	if (sourceCreature)
	{
		// let player creatures in ship stations hear it, but use the ship as the source
		static std::vector<CreatureObject *> passengers;
		findAllPassengers(passengers, true);
		if (!passengers.empty())
		{
			uint32 flags = spatialChat.getFlags();
			if (   sourceCreature->getState(States::PilotingShip)
			    || sourceCreature->getState(States::PilotingPobShip))
				flags |= MessageQueueSpatialChat::F_shipPilot;
			else if (sourceCreature->getState(States::ShipOperations))
				flags |= MessageQueueSpatialChat::F_shipOperations;
			else if (sourceCreature->getState(States::ShipGunner))
				flags |= MessageQueueSpatialChat::F_shipGunner;
			ServerObject const * const sourceShip = safe_cast<ServerObject const *>(ContainerInterface::getTopmostContainer(source));
			MessageQueueSpatialChat const shipSpatialChat(
				sourceShip->getNetworkId(),
				spatialChat.getTargetId(),
				spatialChat.getText(),
				spatialChat.getVolume(),
				spatialChat.getChatType(),
				spatialChat.getMoodType(),
				flags,	
				spatialChat.getLanguage(),
				spatialChat.getOutOfBand(),
				sourceCreature->getAssignedObjectName());
			for (std::vector<CreatureObject *>::const_iterator i = passengers.begin(); i != passengers.end(); ++i)
				if (!(*i)->isInWorld())
					(*i)->hearText(*sourceShip, shipSpatialChat, chatMessageIndex);
			passengers.clear();
		}
	}
}

// ----------------------------------------------------------------------

uint16 ShipObject::getShipId()
{
	FATAL(!isAuthoritative() || !isInWorld(), ("ShipObject::getShipId - tried to get a shipId for %s while either nonauth or not in world", getDebugInformation().c_str()));

	if (!getLocalFlag(LocalObjectFlags::ShipObject_ShipIdAssigned))
	{
		m_shipId = allocateShipId();
		setLocalFlag(LocalObjectFlags::ShipObject_ShipIdAssigned, true);
	}

	return m_shipId.get();
}

// ----------------------------------------------------------------------

bool ShipObject::setMovementPercent (float newMultiplier)
{
	if (isAuthoritative())
	{
		m_movementPercent = newMultiplier;
		return true;
	}
	else
		return false;
}

// ----------------------------------------------------------------------

float ShipObject::getMovementPercent() const
{
	return m_movementPercent;
}

// ----------------------------------------------------------------------

void ShipObject::setInvulnerabilityTimer(float duration)
{
	//note: this isn't really a warning but an FYI - in the case of the falcon for the npe instance, it is always invulnerable
	DEBUG_REPORT_LOG(true, ("Setting ship invulnerability timer for %s to %g\n", getDebugInformation().c_str(), duration));
	PvpUpdateObserver o(this, Archive::ADOO_generic);
	m_invulnerabilityTimer = duration;
}

// ----------------------------------------------------------------------

bool ShipObject::isInvulnerable() const
{
	if (m_invulnerabilityTimer)
		return true;
	return TangibleObject::isInvulnerable();
}

// ----------------------------------------------------------------------

void ShipObject::handleCMessageTo(MessageToPayload const &message)
{
	if (message.getMethod() == "C++SceneWarpDelayed")
	{
		if (isInWorld())
		{
			std::vector<int8> const &packedData = message.getPackedDataVector();
			std::string const packedDataString(packedData.begin(), packedData.end());
			Unicode::String const packedDataStringWide(Unicode::narrowToWide(packedDataString));
			Unicode::UnicodeStringVector result;
			Unicode::tokenize(packedDataStringWide, result);
			if (result.size() == 10) //number of params pushed in by the C++SceneWarpDelayed Message in GameServer::requestSceneWarpDelayed
			{
				std::string const &sceneName = Unicode::wideToNarrow(result[0]);
				Vector newPosition_w;
				newPosition_w.x = static_cast<float>(atof(Unicode::wideToNarrow(result[1]).c_str()));
				newPosition_w.y = static_cast<float>(atof(Unicode::wideToNarrow(result[2]).c_str()));
				newPosition_w.z = static_cast<float>(atof(Unicode::wideToNarrow(result[3]).c_str()));
				NetworkId const newContainer(Unicode::wideToNarrow(result[4]));
				Vector newPosition_p;
				newPosition_p.x = static_cast<float>(atof(Unicode::wideToNarrow(result[5]).c_str()));
				newPosition_p.y = static_cast<float>(atof(Unicode::wideToNarrow(result[6]).c_str()));
				newPosition_p.z = static_cast<float>(atof(Unicode::wideToNarrow(result[7]).c_str()));
				std::string callback = Unicode::wideToNarrow(result[8]);
				if (callback == "noCallback")
					callback.clear();
				std::string const &forceLoadScreenStr = Unicode::wideToNarrow(result[9]);
				bool const forceLoadScreen = (forceLoadScreenStr == "1") ? true : false;

				GameServer::getInstance().requestSceneWarp(
					CachedNetworkId(*this),
					sceneName,
					newPosition_w,
					newContainer,
					newPosition_p,
					callback.c_str(),
					forceLoadScreen);
			}
		}
	}
	else if (message.getMethod() == "C++SceneWarpDelayedContainer")
	{
		if (isInWorld())
		{
			std::vector<int8> const &packedData = message.getPackedDataVector();
			std::string const packedDataString(packedData.begin(), packedData.end());
			Unicode::String const packedDataStringWide(Unicode::narrowToWide(packedDataString));
			Unicode::UnicodeStringVector result;
			Unicode::tokenize(packedDataStringWide, result);
			if (result.size() == 11) //number of params pushed in by the C++SceneWarpDelayedContainer Message in GameServer::requestSceneWarpDelayed
			{
				std::string const &sceneName = Unicode::wideToNarrow(result[0]);
				Vector newPosition_w;
				newPosition_w.x = static_cast<float>(atof(Unicode::wideToNarrow(result[1]).c_str()));
				newPosition_w.y = static_cast<float>(atof(Unicode::wideToNarrow(result[2]).c_str()));
				newPosition_w.z = static_cast<float>(atof(Unicode::wideToNarrow(result[3]).c_str()));
				NetworkId const newBuilding(Unicode::wideToNarrow(result[4]));
				std::string const & newCellName = Unicode::wideToNarrow(result[5]);
				Vector newPosition_p;
				newPosition_p.x = static_cast<float>(atof(Unicode::wideToNarrow(result[6]).c_str()));
				newPosition_p.y = static_cast<float>(atof(Unicode::wideToNarrow(result[7]).c_str()));
				newPosition_p.z = static_cast<float>(atof(Unicode::wideToNarrow(result[8]).c_str()));
				std::string callback = Unicode::wideToNarrow(result[9]);
				if (callback == "noCallback")
					callback.clear();
				std::string const &forceLoadScreenStr = Unicode::wideToNarrow(result[10]);
				bool const forceLoadScreen = (forceLoadScreenStr == "1") ? true : false;

				GameServer::getInstance().requestSceneWarp(
					CachedNetworkId(*this),
					sceneName,
					newPosition_w,
					newBuilding,
					newCellName,
					newPosition_p,
					callback.c_str(),
					forceLoadScreen);
			}
		}
	}
	else
	{
		TangibleObject::handleCMessageTo(message);
	}
}

// ----------------------------------------------------------------------

uint16 ShipObjectNamespace::allocateShipId()
{
	unsigned int shipId = s_lastShipId;
	unsigned int sentinel = 0;
	do
	{
		// infinite loop sentinel
		FATAL(++sentinel > s_maxShipId, ("No more available ship id"));

		shipId = (shipId+1u)%s_maxShipId;
	} while (s_shipIdUsed[shipId/32]&(1u<<(shipId%32)));
	s_shipIdUsed[shipId/32] |= (1u<<(shipId%32));
	s_lastShipId = shipId;
	return static_cast<uint16>(shipId);
}

// ----------------------------------------------------------------------

void ShipObjectNamespace::freeShipId(uint16 shipId)
{
	s_shipIdUsed[shipId/32] &= ~(1u<<(shipId%32));
}

// ----------------------------------------------------------------------

ShipObject const * ShipObject::asShipObject(Object const * object)
{
	ServerObject const * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;

	return (serverObject != nullptr) ? serverObject->asShipObject() : nullptr;
}

// ----------------------------------------------------------------------

ShipObject * ShipObject::asShipObject(Object * object)
{
	ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;

	return (serverObject != nullptr) ? serverObject->asShipObject() : nullptr;
}

// ----------------------------------------------------------------------

bool ShipObject::isBoosterReady() const
{
	return m_boosterAvailableTimer->isExpired();
}

// ----------------------------------------------------------------------

void ShipObject::restartBoosterTimer()
{
	float const boosterSpeedBonus = getBoosterSpeedMaximum();
	float const decelRate = getShipActualDecelerationRate();

	float expireTime = 0.0f;

	//-- compute how long it will take to decelerate to our normal speed
	if (decelRate > 0.0f)
		expireTime = boosterSpeedBonus / decelRate;

	expireTime = clamp(s_boosterAvailableTimeMinimum, expireTime, s_boosterAvailableTimeMaximum);

	m_boosterAvailableTimer->setExpireTime(expireTime);
	m_boosterAvailableTimer->reset();
}

// ----------------------------------------------------------------------

float ShipObject::getChassisSpeedMaximumModifier() const
{
	return m_chassisSpeedMaximumModifier.get();
}

// ----------------------------------------------------------------------

void ShipObject::setChassisSpeedMaximumModifier(float f)
{
	m_chassisSpeedMaximumModifier = f;
}

//----------------------------------------------------------------------

bool ShipObject::isBeamWeapon(int weaponIndex)
{
	if(weaponIndex >= 0 && weaponIndex < ShipChassisSlotType::cms_numWeaponIndices)
	{
		int const chassisSlot = static_cast<int>(ShipChassisSlotType::SCST_weapon_first) + weaponIndex;
		uint32 const componentCrc = getComponentCrc(chassisSlot);
		if (componentCrc != 0)
			return ShipComponentWeaponManager::hasFlags(componentCrc, ShipComponentWeaponManager::F_beam);
	}

	return false;
}

//----------------------------------------------------------------------

void ShipObject::stopFiringWeapon(int weaponIndex)
{
	ProjectileManager::stopBeam(*this, weaponIndex);

	typedef MessageQueueGenericValueType<int> MessageType;
	MessageType * const newMsg = new MessageType(weaponIndex);
						
	appendMessage(CM_shipStopFiring, 0.0f, newMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::DEST_ALL_CLIENT | GameControllerMessageFlags::RELIABLE);
}

// ======================================================================

