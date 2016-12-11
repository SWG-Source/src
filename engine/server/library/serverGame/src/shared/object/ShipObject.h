// ======================================================================
//
// ShipObject.h
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _INCLUDED_ShipObject_H
#define _INCLUDED_ShipObject_H

// ======================================================================

#include "Archive/AutoDeltaPackedMap.h"
#include "serverGame/TangibleObject.h"
#include "sharedGame/ShipChassisSlotType.h"

// ======================================================================

class AppearanceTemplate;
class BitArray;
class MessageQueueSpatialChat;
class ServerShipObjectTemplate;
class ShipComponentData;
class ShipComponentDataEngine;
class Timer;

// ======================================================================

namespace LocalObjectFlags
{
	enum
	{
		ShipObject_PlayerControlled = TangibleObject_Max,
		ShipObject_HasWings         = TangibleObject_Max+1,
		ShipObject_ShipIdAssigned   = TangibleObject_Max+2,

		// This must come last.
		ShipObject_Max
	};
}

// ======================================================================

class ShipObject: public TangibleObject
{
	friend class ShipController;
	class FireShotQueueData;

public:

	static void install();

	static ShipObject const * getContainingShipObject(ServerObject const * serverObject);
	static ShipObject * getContainingShipObject(ServerObject * serverObject);

	static ShipObject const * asShipObject(Object const * object);
	static ShipObject * asShipObject(Object * object);

	typedef std::vector<int> IntVector;
	typedef std::map<NetworkId, int> NetworkIdIntMap;
	typedef std::pair<Unicode::String, std::string> ResourceTypeInfoPair;
	typedef std::map<NetworkId, ResourceTypeInfoPair> ResourceTypeInfoMap;

public:

	ShipObject(ServerShipObjectTemplate const *newTemplate);
	virtual ~ShipObject();

	virtual float alter(float time);

	virtual ShipObject * asShipObject();
	virtual ShipObject const * asShipObject() const;

	uint16 getShipId();

	virtual void hearText(ServerObject const &source, MessageQueueSpatialChat const &spatialChat, int chatMessageIndex);

	virtual Controller *createDefaultController();
	virtual void onAddedToWorld();
	virtual void onContainerGainItem(ServerObject &item, ServerObject *source, ServerObject *transferer);
	virtual void onContainerChildGainItem(ServerObject &item, ServerObject *source, ServerObject *transferer);
	virtual void onContainerLostItem(ServerObject *destination, ServerObject &item, ServerObject *transferer);
	virtual void onContainerChildLostItem(ServerObject *destination, ServerObject &item, ServerObject *source, ServerObject *transferer);
	virtual bool isVisibleOnClient(const Client & client) const;
	virtual void onLoadingScreenComplete();

	CreatureObject const *getPilot() const;
	CreatureObject *getPilot();
	void findAllPassengers(std::vector<CreatureObject *> & passengers, bool onlyPlayerControlled);

	Vector getCurrentVelocity_p() const;
	Vector getTargetLead_p(const Vector & startPosition_p, float projectileSpeed) const;
	float getApproximateAttackRange(bool includeMissiles = false) const;

	bool isCapitalShip() const;
	
	//-- Engine system
	float getCurrentSpeed() const;
	bool setSlideDampener(float slideDampener);
	float getSlideDampener() const;
	bool setCurrentYaw(float currentYaw);
	float getCurrentYaw() const;
	bool setCurrentPitch(float currentPitch);
	float getCurrentPitch() const;
	bool setCurrentRoll(float currentRoll);
	float getCurrentRoll() const;

	//-- Chassis system
	bool setCurrentChassisHitPoints(float currentChassisHitPoints);
	float getCurrentChassisHitPoints() const;
	bool setMaximumChassisHitPoints(float maximumChassisHitPoints);
	float getMaximumChassisHitPoints() const;

	bool hasWings() const;
	bool wingsOpened() const;
	void openWings();
	void closeWings();

	bool hasEnergyForAShot(int weaponIndex) const;
	bool hasAmmoForAShot(int weaponIndex) const;
	bool hasTurrets() const;
	void setTurretTarget(int weaponIndex, CachedNetworkId const & target);
	CachedNetworkId const & getTurretTarget(int weaponIndex) const;
	Transform const getTurretTransform(int weaponIndex) const;
	bool isMissile(int weaponIndex) const;
	bool isTurret(int weaponIndex) const;
	bool isCountermeasure(int weaponIndex) const;
	bool isProjectile(int weaponIndex) const;
	bool canTurretFireTowardsLocation_p(int weaponIndex, Vector const & targetLocation_p) const;
	void setWeaponRefireTimerPercent(int weaponId, float refireTimerPercent);
	bool isBeamWeapon(int weaponIndex);
	
	//-- Create a projectile from an authoritative client
	void enqueueFireShotClient(Client const &gunnerClient, int weaponIndex, Transform const & transform_p, int targetComponent, uint32 syncStampLong);

	//-- Create a projectile from our current position (used for ai ships)
	void fireShotNonTurretServer(int const weaponIndex, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, float yawErrorRadians = 0.0f, float pitchErrorRadians = 0.0f);
	void fireShotTurretServer(int const weaponIndex, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, bool const goodShot, bool const fromAutoTurret);

	void onTargetedByMissile(int const missileId);
	
	const NetworkId&    getPilotLookAtTarget() const;
	void                setPilotLookAtTarget(const NetworkId&);
	void                setLookAtTargetFromPilot(const NetworkId&);
	ShipChassisSlotType::Type getPilotLookAtTargetSlot() const;
	void                setPilotLookAtTargetSlot(ShipChassisSlotType::Type slot);
	void                setSlotTargetable(ShipChassisSlotType::Type slot);
	void                clearSlotTargetable(ShipChassisSlotType::Type slot);
	bool                getSlotTargetable(ShipChassisSlotType::Type slot) const;
	bool                isValidTargetableSlot (ShipChassisSlotType::Type slot) const;
	int findTargetChassisSlotByPosition(Vector const & position_o) const;
	void findTransformsForComponent(int const chassisSlot, std::vector<Transform> & transforms) const;
	AppearanceTemplate const * getExteriorAppearance() const;

	int getNumberOfHits() const;
	void setNumberOfHits(int numberOfHits);

	//--
	//-- Componentized ship physics system.
	//-- these accessors return the correctly modified physics parameters
	//-- these computed parameters include:
	//--    engine component parameters
	//--    engine efficiency
	//--    booster component parameters, if active
	//--    booster component efficiency, if active
	//--    mass of the ship
	//--    rotational inertia of the ship
	//--

	float     getShipActualAccelerationRate       () const;
	float     getShipActualDecelerationRate       () const;
	float     getShipActualPitchAccelerationRate  () const;
	float     getShipActualYawAccelerationRate    () const;
	float     getShipActualRollAccelerationRate   () const;
	float     getShipActualPitchRateMaximum       () const;
	float     getShipActualYawRateMaximum         () const;
	float     getShipActualRollRateMaximum        () const;
	float     getShipActualYprRateMaximum         () const;
	float     getShipActualSpeedMaximum           () const;

	//-- 
	//-- Ship Component System
	//--

	float getOverallHealth() const;
	float getOverallHealthWithShieldsAndArmor() const;

	uint32    getChassisType                           () const;
	float getChassisComponentMassMaximum() const;
	float getChassisComponentMassCurrent() const;

	bool      isSlotInstalled                          (int chassisSlot) const;
	bool      isComponentDisabled                      (int chassisSlot) const;
	bool      isComponentLowPower                      (int chassisSlot) const;
	bool      isComponentActive                        (int chassisSlot) const;
	bool      isComponentDemolished(int chassisSlot) const;
	bool isComponentDisabledNeedsPower(int chassisSlot) const;
	bool isComponentFunctional(int chassisSlot) const;

	//-- all components
	float     getComponentArmorHitpointsMaximum        (int chassisSlot) const;
	float     getComponentArmorHitpointsCurrent        (int chassisSlot) const;
	float     getComponentEfficiencyGeneral            (int chassisSlot) const;
	float     getComponentEfficiencyEnergy             (int chassisSlot) const;
	float     getComponentEnergyMaintenanceRequirement (int chassisSlot) const;
	float     getComponentMass                         (int chassisSlot) const;
	uint32    getComponentCrc                          (int chassisSlot) const;
	float     getComponentHitpointsCurrent             (int chassisSlot) const;
	float     getComponentHitpointsMaximum             (int chassisSlot) const;
	int       getComponentFlags                        (int chassisSlot) const;
	Unicode::String const &getComponentName(int chassisSlot) const;
	NetworkId const & getComponentCreator(int chassisSlot) const;

	//-- weapons
	//-- The following methods are per weapon

	float     getWeaponDamageMaximum            (int chassisSlot) const;
	float     getWeaponDamageMinimum            (int chassisSlot) const;
	float     getWeaponEffectivenessShields     (int chassisSlot) const;
	float     getWeaponEffectivenessArmor       (int chassisSlot) const;
	float     getWeaponEnergyPerShot            (int chassisSlot) const;
	float     getWeaponActualEnergyPerShot(int chassisSlot) const;
	float     getWeaponRefireRate               (int chassisSlot) const;
	float     getWeaponActualRefireRate         (int chassisSlot) const;

	float getWeaponEfficiencyRefireRate(int chassisSlot) const;
	int getWeaponAmmoCurrent(int chassisSlot) const;
	int getWeaponAmmoMaximum(int chassisSlot) const;
	uint32 getWeaponAmmoType(int chassisSlot) const;

	float getFastestWeaponProjectileSpeed() const;
	float getWeaponProjectileSpeed(int weaponIndex) const;
	int getProjectileIndexForWeapon(int weaponIndex) const;
	float getWeaponRange(int weaponIndex) const;
	float computeWeaponProjectileTimeToLive(int weaponIndex) const;

	//-- shields
	//-- The following methods are per shield

	float     getShieldHitpointsFrontCurrent    () const;
	float     getShieldHitpointsFrontMaximum    () const;
	float     getShieldHitpointsBackCurrent     () const;
	float     getShieldHitpointsBackMaximum     () const;
	float     getShieldRechargeRate             () const;

	//-- capacitor
	//-- The following methods are per capacitor

	float     getCapacitorEnergyCurrent         () const;
	float     getCapacitorEnergyMaximum         () const; // raw maximum of fully functioning capacitor
	float     getCapacitorEnergyEffectiveMaximum() const; // zero if capacitor disabled
	float     getCapacitorEnergyRechargeRate    () const;

	//-- engine
	float     getEngineAccelerationRate         () const;
	float     getEngineDecelerationRate         () const;
	float     getEnginePitchAccelerationRate    () const;
	float     getEngineYawAccelerationRate      () const;
	float     getEngineRollAccelerationRate     () const;
	float     getEnginePitchRateMaximum         () const;
	float     getEngineYawRateMaximum           () const;
	float     getEngineRollRateMaximum          () const;
	float     getEngineSpeedMaximum             () const;
	float getEngineSpeedRotationFactorMaximum() const;
	float getEngineSpeedRotationFactorMinimum() const;
	float getEngineSpeedRotationFactorOptimal() const;

	//-- reactor
	float     getReactorEnergyGenerationRate    () const;

	//-- booster
	float     getBoosterEnergyCurrent           () const;
	float     getBoosterEnergyMaximum           () const;
	float     getBoosterEnergyRechargeRate      () const;
	float     getBoosterEnergyConsumptionRate   () const;
	float     getBoosterAcceleration            () const;
	float     getBoosterSpeedMaximum            () const;
	bool      isBoosterActive                   () const;

	//-- droid interface
	float     getDroidInterfaceCommandSpeed     () const;

	void setChassisComponentMassMaximum(float massMaximum);

	//-- setters for all components

	bool       setComponentArmorHitpointsMaximum        (int chassisSlot, float componentArmorHitpointsMaximum);
	bool       setComponentArmorHitpointsCurrent        (int chassisSlot, float componentArmorHitpointsCurrent);
	bool       setComponentEfficiencyGeneral            (int chassisSlot, float componentEfficiencyGeneral);
	bool       setComponentEfficiencyEnergy             (int chassisSlot, float componentEfficiencyEnergy);
	bool       setComponentEnergyMaintenanceRequirement (int chassisSlot, float componentEnergyMaintenanceRequirement);
	bool       setComponentMass                         (int chassisSlot, float componentMass);
	void       setComponentCrc                          (int chassisSlot, uint32 componentCrc);
	bool       setComponentHitpointsCurrent             (int chassisSlot, float componentHitpointsCurrent);
	bool       setComponentHitpointsMaximum             (int chassisSlot, float componentHitpointsMaximum);
	bool       setComponentFlags                        (int chassisSlot, int componentFlags);
	bool       setComponentName(int chassisSlot, Unicode::String const & name);
	bool       setComponentCreator(int chassisSlot, NetworkId const & creator);
	bool       setComponentDisabled                     (int chassisSlot, bool componentDisabled);
	bool setComponentDisabledNeedsPower(int chassisSlot, bool componentDisabledNeedsPower);

	bool       setComponentLowPower                     (int chassisSlot, bool componentLowPower);
	bool       setComponentActive                       (int chassisSlot, bool componentActive);
	bool       setComponentDemolished(int chassisSlot, bool componentDemolished);

	//-- weapons
	//-- The following methods are per weapon

	bool       setWeaponDamageMaximum            (int chassisSlot, float weaponDamageMaximum);
	bool       setWeaponDamageMinimum            (int chassisSlot, float weaponDamageMinimum);
	bool       setWeaponEffectivenessShields     (int chassisSlot, float weaponEffectivenessShields);
	bool       setWeaponEffectivenessArmor       (int chassisSlot, float weaponEffectivenessArmor);
	bool       setWeaponEnergyPerShot            (int chassisSlot, float weaponEnergyPerShot);
	bool       setWeaponRefireRate               (int chassisSlot, float weaponRefireRate);
	bool setWeaponEfficiencyRefireRate(int chassisSlot, float weaponEfficiencyRefire);
	bool setWeaponAmmoCurrent(int chassisSlot, int weaponAmmoCurrent);
	bool setWeaponAmmoMaximum(int chassisSlot, int weaponAmmoMaximum);
	bool setWeaponAmmoType(int chassisSlot, uint32 weaponAmmoType);

	//-- shields

	bool       setShieldHitpointsFrontCurrent    (float shieldHitpointsFrontCurrent);
	bool       setShieldHitpointsFrontMaximum    (float shieldHitpointsFrontMaximum);
	bool       setShieldHitpointsBackCurrent     (float shieldHitpointsBackCurrent);
	bool       setShieldHitpointsBackMaximum     (float shieldHitpointsBackMaximum);
	bool       setShieldRechargeRate             (float shieldRechargeRate);

	//-- capacitor

	bool       setCapacitorEnergyCurrent         (float capacitorEnergyCurrent);
	bool       setCapacitorEnergyMaximum         (float capacitorEnergyMaximum);
	bool       setCapacitorEnergyRechargeRate    (float capacitorEnergyRechargeRate);

	//-- engine
	bool       setEngineAccelerationRate         (float engineAccelerationRate);
	bool       setEngineDecelerationRate         (float engineDecelerationRate);
	bool       setEnginePitchAccelerationRate    (float enginePitchAccelerationRate);
	bool       setEngineYawAccelerationRate      (float engineYawAccelerationRate);
	bool       setEngineRollAccelerationRate     (float engineRollAccelerationRate);
	bool       setEnginePitchRateMaximum         (float enginePitchRateMaximum);
	bool       setEngineYawRateMaximum           (float engineYawRateMaximum);
	bool       setEngineRollRateMaximum          (float engineRollRateMaximum);
	bool       setEngineSpeedMaximum             (float engineSpeedMaximum);
	bool setEngineSpeedRotationFactorMaximum(float engineSpeedRotationFactor);
	bool setEngineSpeedRotationFactorMinimum(float engineSpeedRotationFactorMin);
	bool setEngineSpeedRotationFactorOptimal(float engineSpeedRotationFactorSweetSpot);

	//-- reactor
	bool       setReactorEnergyGenerationRate    (float reactorEnergyGenerationRate);

	//-- booster
	bool       setBoosterEnergyCurrent           (float boosterEnergyCurrent);
	bool       setBoosterEnergyMaximum           (float boosterEnergyMaximum);
	bool       setBoosterEnergyRechargeRate      (float boosterEnergyRechargeRate);
	bool       setBoosterEnergyConsumptionRate   (float boosterEnergyConsumptionRate);
	bool       setBoosterAcceleration            (float boosterAcceleration);
	bool       setBoosterSpeedMaximum            (float boosterSpeedMaximum);
	void setBoosterActive(bool acvtive);
	bool isBoosterInstalled() const;

	//-- speed multiplier (for god mode)
	bool setMovementPercent (float newMultiplier);
	float getMovementPercent() const;

	//-- droid interface
	bool       setDroidInterfaceCommandSpeed     (float droidInterfaceCommandSpeed);

	//-- component support

	bool       canInstallComponent               (int chassisSlot, TangibleObject const & component) const;
	bool       hasSlot                           (int chassisSlot) const;

	/**
	* installComponent destroys the component if the installation succeeds
	*/

	bool                installComponent        (NetworkId const & installerId, int chassisSlot, TangibleObject & component);

	bool installComponentFromData(int chassisSlot, ShipComponentData const & shipComponentData);
	bool                pseudoInstallComponent  (int chassisSlot, uint32 componentCrc);
	TangibleObject *    uninstallComponent      (NetworkId const & uninstallerId, int chassisSlot, ServerObject & containerTarget, bool allowOverload = false);
	void                purgeComponent          (int chassisSlot);
	ShipComponentData * createShipComponentData (int chassisSlot) const;

	NetworkId getInstalledDroidControlDevice    () const;
	void setInstalledDroidControlDevice         (NetworkId const & droidControlDevice);

	ServerObject const *getControlDevice        () const;

	int getCargoHoldContentsMaximum() const;
	bool setCargoHoldContentsMaximum(int contents);
	int getCargoHoldContentsCurrent() const;

	void setCargoHoldContents(NetworkIdIntMap const & contents);
	NetworkIdIntMap const & getCargoHoldContents() const;
	int getCargoHoldContent(NetworkId const & resourceTypeId) const;
	void setCargoHoldContent(NetworkId const & resourceTypeId, int amount);

	float computeActualComponentEfficiencyGeneral(int chassisType) const;

	// space faction related functions
	uint32 getSpaceFaction() const;
	std::vector<uint32> const &getSpaceFactionAllies() const;
	std::vector<uint32> const &getSpaceFactionEnemies() const;
	bool getSpaceFactionIsAggro() const;

	void setSpaceFaction(uint32 spaceFaction);
	void setSpaceFactionAllies(std::vector<uint32> const &spaceFactionAllies);
	void setSpaceFactionEnemies(std::vector<uint32> const &spaceFactionEnemies);
	void setSpaceFactionIsAggro(bool isAggro);

	bool isPlayerShip() const;

	IntVector const & getNebulas() const;

	void setWingName(std::string const & wingName);
	std::string const & getWingName() const;
	void setTypeName(std::string const & typeName);
	std::string const & getTypeName() const;
	void setDifficulty(std::string const & difficultyName);
	std::string const & getDifficulty() const;
	void setFaction(std::string const & factionName);
	std::string const & getFaction() const;
	virtual TangibleObject const &getPvpViewer(Client const &client) const;
	virtual int getNumberOfLots() const;

	bool canFireShot(int weaponIndex) const;
	bool isAlwaysStationary() const;

	void setAutoAggroImmuneTime(float const time) const;
	bool isAutoAggroImmune() const;

	void setDamageAggroImmune(bool const enabled);
	bool isDamageAggroImmune() const;

	Vector getInterceptPosition(float timeDelta) const;

	void setInvulnerabilityTimer(float duration);
	virtual bool isInvulnerable() const;
	virtual void handleCMessageTo(MessageToPayload const &message);

	bool isBoosterReady() const;
	void restartBoosterTimer();

	float getChassisSpeedMaximumModifier() const;
	void setChassisSpeedMaximumModifier(float f);
	virtual void getAuthClients(std::set<Client const *> &authClients) const;

	void stopFiringWeapon(int weaponIndex);

protected:
	virtual void calcPvpableState();
	virtual void endBaselines();
	virtual void virtualOnSetAuthority();
	virtual void handleGunnerChange(ServerObject const &player);

private:
	ShipObject();
	ShipObject(ShipObject const &);
	ShipObject &operator=(ShipObject const &);

	void constructFromTemplate();

	//-- Implementation of this function is located in generated/Packager.cpp
	void addMembersToPackages();

	bool validateShot(int weaponIndex, Transform const & transform_p) const;

	void handlePowerPulse (float elapsedTimeSecs);
	TangibleObject *    internalUninstallComponent      (NetworkId const & uninstallerId, int chassisSlot, ServerObject * containerTarget, bool allowOverload = false);

	float     computeShipActualAccelerationRate       () const;
	float     computeShipActualDecelerationRate       () const;
	float     computeShipActualPitchAccelerationRate  () const;
	float     computeShipActualYawAccelerationRate    () const;
	float     computeShipActualRollAccelerationRate   () const;
	float     computeShipActualPitchRateMaximum       () const;
	float     computeShipActualYawRateMaximum         () const;
	float     computeShipActualRollRateMaximum        () const;
	float     computeShipActualSpeedMaximum           () const;

	float computeChassisComponentMassCurrent() const;

	void internalHandleFireShot(Client const *gunnerClient, int const weaponIndex, Transform const & transform_p, uint32 const syncStampLong, bool clientShot, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, bool fromAutoTurret);

	ShipComponentDataEngine const & getShipDataEngine() const;

	void internalSetChassisComponentMassCurrent(float massCurrent);

	float computeSpeedRotationFactor() const;

	int computeCargoHoldContentsCurrent() const;
	bool setCargoHoldContentsCurrent(int contents);

private:
	Archive::AutoDeltaVariable<uint16> m_shipId;

	//-- Engine system
	Archive::AutoDeltaVariable<float> m_slideDampener;
	float m_currentYaw;  
	float m_currentPitch;  
	float m_currentRoll;  

	//-- Chassis system
	Archive::AutoDeltaVariable<float> m_currentChassisHitPoints;
	Archive::AutoDeltaVariable<float> m_maximumChassisHitPoints;

	Timer * m_weaponRefireTimers;

	int m_numberOfHits;

	//-- 
	//-- Ship Component System
	//--

	//-- all components

	Archive::AutoDeltaVariable<uint32>    m_chassisType;
	Archive::AutoDeltaVariable<float>     m_chassisComponentMassMaximum;
	Archive::AutoDeltaVariable<float>     m_chassisComponentMassCurrent;
	Archive::AutoDeltaVariable<float>     m_chassisSpeedMaximumModifier;

	Archive::AutoDeltaVariable<float>     m_shipActualAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_shipActualDecelerationRate;
	Archive::AutoDeltaVariable<float>     m_shipActualPitchAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_shipActualYawAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_shipActualRollAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_shipActualPitchRateMaximum;
	Archive::AutoDeltaVariable<float>     m_shipActualYawRateMaximum;
	Archive::AutoDeltaVariable<float>     m_shipActualRollRateMaximum;
	Archive::AutoDeltaVariable<float>     m_shipActualSpeedMaximum;

	Archive::AutoDeltaPackedMap<int, float>     m_componentArmorHitpointsMaximum;
	Archive::AutoDeltaPackedMap<int, float>     m_componentArmorHitpointsCurrent;
	Archive::AutoDeltaPackedMap<int, float>     m_componentEfficiencyGeneral;
	Archive::AutoDeltaPackedMap<int, float>     m_componentEfficiencyEnergy;
	Archive::AutoDeltaPackedMap<int, float>     m_componentEnergyMaintenanceRequirement;
	Archive::AutoDeltaPackedMap<int, float>     m_componentMass;
	Archive::AutoDeltaPackedMap<int, uint32>    m_componentCrc;
	Archive::AutoDeltaMap<int, uint32>          m_componentCrcForClient;
	Archive::AutoDeltaPackedMap<int, float>     m_componentHitpointsCurrent;
	Archive::AutoDeltaPackedMap<int, float>     m_componentHitpointsMaximum;
	Archive::AutoDeltaPackedMap<int, int>       m_componentFlags;
	Archive::AutoDeltaPackedMap<int, Unicode::String> m_componentNames;
	Archive::AutoDeltaPackedMap<int, NetworkId> m_componentCreators;

	//-- weapons
	//-- The following maps have one entry per installed weapon .

	Archive::AutoDeltaPackedMap<int, float>     m_weaponDamageMaximum;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponDamageMinimum;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponEffectivenessShields;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponEffectivenessArmor;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponEnergyPerShot;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponRefireRate;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponEfficiencyRefireRate;
	Archive::AutoDeltaPackedMap<int, int>       m_weaponAmmoCurrent;
	Archive::AutoDeltaPackedMap<int, int>       m_weaponAmmoMaximum;
	Archive::AutoDeltaPackedMap<int, uint32>    m_weaponAmmoType;

	//-- shields
	//-- The following maps have one entry per installed shield.

	Archive::AutoDeltaVariable<float>     m_shieldHitpointsFrontCurrent;
	Archive::AutoDeltaVariable<float>     m_shieldHitpointsFrontMaximum;
	Archive::AutoDeltaVariable<float>     m_shieldHitpointsBackCurrent;
	Archive::AutoDeltaVariable<float>     m_shieldHitpointsBackMaximum;
	Archive::AutoDeltaVariable<float>     m_shieldRechargeRate;

	//-- capacitor
	//-- The following data is for component types which may not be installed in multiples.

	Archive::AutoDeltaVariable<float>     m_capacitorEnergyCurrent;
	Archive::AutoDeltaVariable<float>     m_capacitorEnergyMaximum;
	Archive::AutoDeltaVariable<float>     m_capacitorEnergyRechargeRate;

	//-- engine
	Archive::AutoDeltaVariable<float>     m_engineAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_engineDecelerationRate;
	Archive::AutoDeltaVariable<float>     m_enginePitchAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_engineYawAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_engineRollAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_enginePitchRateMaximum;
	Archive::AutoDeltaVariable<float>     m_engineYawRateMaximum;
	Archive::AutoDeltaVariable<float>     m_engineRollRateMaximum;
	Archive::AutoDeltaVariable<float>     m_engineSpeedMaximum;
	Archive::AutoDeltaVariable<float> m_engineSpeedRotationFactorMaximum;
	Archive::AutoDeltaVariable<float> m_engineSpeedRotationFactorMinimum;
	Archive::AutoDeltaVariable<float> m_engineSpeedRotationFactorOptimal;

	//-- reactor
	Archive::AutoDeltaVariable<float>     m_reactorEnergyGenerationRate;

	//-- booster
	Archive::AutoDeltaVariable<float>     m_boosterEnergyCurrent;
	Archive::AutoDeltaVariable<float>     m_boosterEnergyMaximum;
	Archive::AutoDeltaVariable<float>     m_boosterEnergyRechargeRate;
	Archive::AutoDeltaVariable<float>     m_boosterEnergyConsumptionRate;
	Archive::AutoDeltaVariable<float>     m_boosterAcceleration;
	Archive::AutoDeltaVariable<float>     m_boosterSpeedMaximum;

	//-- droid interface
	Archive::AutoDeltaVariable<float>     m_droidInterfaceCommandSpeed;

	Archive::AutoDeltaVariable<NetworkId> m_installedDroidControlDevice;

	//-- cargo hold
	Archive::AutoDeltaVariable<int>       m_cargoHoldContentsMaximum;
	Archive::AutoDeltaVariable<int>       m_cargoHoldContentsCurrent;

	Archive::AutoDeltaPackedMap<NetworkId, int> m_cargoHoldContents;
	Archive::AutoDeltaMap<NetworkId, std::pair<Unicode::String, std::string> > m_cargoHoldContentsResourceTypeInfo;

	float                                 m_timeSinceLastPowerPulse;

	typedef std::vector<FireShotQueueData> FireShotQueue;
	FireShotQueue *                       m_fireShotQueue;

	static float const                    ms_powerPulsePeriodSecs;

	IntVector *                           m_nebulas;

	float                                 m_nebulaEffectReactor;
	float                                 m_nebulaEffectEngine;
	float                                 m_nebulaEffectShields;

	Archive::AutoDeltaVariable<NetworkId> m_pilotLookAtTarget;
	Archive::AutoDeltaVariable<int>       m_pilotLookAtTargetSlot;
	Archive::AutoDeltaVariable<BitArray>  m_targetableSlotBitfield;

	std::map<int, CachedNetworkId> m_turretTargets;

	Archive::AutoDeltaVariable<std::string>  m_wingName;
	Archive::AutoDeltaVariable<std::string>  m_typeName;
	Archive::AutoDeltaVariable<std::string>  m_difficulty;
	Archive::AutoDeltaVariable<std::string>  m_faction;

	Archive::AutoDeltaVariable<int> m_guildId;

	uint32 m_spaceFaction;
	std::vector<uint32> m_spaceFactionAllies;
	std::vector<uint32> m_spaceFactionEnemies;
	bool m_spaceFactionIsAggro;
	int m_lotsConsumed;
	Timer * const m_autoAggroImmuneTimer;
	bool m_damageAggroImmune;
	BitArray * const m_turretWeaponIndices;
	float m_movementPercent;
	float m_invulnerabilityTimer;
	Timer * m_boosterAvailableTimer;
	float m_fastestWeaponProjectileSpeed;

private:
//BPM ShipObject : TangibleObject // Begin persisted members.
//EPM
};

// ======================================================================

inline const NetworkId& ShipObject::getPilotLookAtTarget() const
{
	return m_pilotLookAtTarget.get();
}

//----------------------------------------------------------------------

inline ShipChassisSlotType::Type ShipObject::getPilotLookAtTargetSlot() const
{
	return static_cast<ShipChassisSlotType::Type>(m_pilotLookAtTargetSlot.get());
}

// ----------------------------------------------------------------------

inline bool ShipObject::isPlayerShip() const
{
	return getLocalFlag(LocalObjectFlags::ShipObject_PlayerControlled);
}

//----------------------------------------------------------------------

inline bool ShipObject::isComponentFunctional(int chassisSlot) const
{
	return isSlotInstalled(chassisSlot) && !(isComponentDisabled(chassisSlot) || isComponentDemolished(chassisSlot));
}

// ======================================================================

#endif
