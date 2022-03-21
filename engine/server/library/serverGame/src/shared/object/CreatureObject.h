//========================================================================
//
// CreatureObject.h
// Copyright 2001-2003, Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#ifndef INCLUDED_CreatureObject_H
#define INCLUDED_CreatureObject_H

#include "Archive/AutoDeltaMap.h"
#include "Archive/AutoDeltaPackedMap.h"
#include "Archive/AutoDeltaSet.h"
#include "serverGame/GroupIdObserver.h"
#include "serverGame/TangibleObject.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/AttribMod.h"
#include "sharedGame/Buff.h"
#include "sharedGame/Command.h"
#include "serverGame/PvpUpdateAndCellPermissionsObserver.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/WearableEntry.h"
#include "sharedObject/Container.h"
#include "sharedSkillSystem/SkillObjectArchive.h"
#include "swgSharedUtility/Attributes.def"
#include "swgSharedUtility/Behaviors.def"
#include "swgSharedUtility/Locomotions.def"
#include "swgSharedUtility/MentalStates.def"
#include "swgSharedUtility/Postures.def"
#include "swgSharedUtility/States.def"

#pragma warning(disable : 4100 4514) //@todo investigate why we need this?
#include <string>
#include <vector>

class ClientMfdStatusUpdateMessage;
class ConstCharCrcLowerString;
class CreatureController;
class DraftSchematicObject;
class GroupObject;
class LfgCharacterData;
class ManufactureSchematicObject;
struct MatchMakingCharacterResult;
class MessageQueueMissionListResponseData;
class MissionObject;
class ProsePackage;
class ServerCreatureObjectTemplate;
class ServerWeaponObjectTemplate;
class ShipObject;
class SkillObject;
class TriggerVolume;
class ValueDictionary;
class WeaponObject;

namespace Crafting
{
	struct CustomValue;
}

namespace Archive
{
	class ByteStream;
	class ReadIterator;
}

// ----------------------------------------------------------------------

namespace LocalObjectFlags
{
	enum
	{
		CreatureObject_RequiresMovementInfoUpdate = TangibleObject_Max,

		// This must come last.
		CreatureObject_Max
	};
}

// ======================================================================

class CreatureAttitude
{
public:
	CreatureAttitude();

	enum eAngerState
	{
		kAngerCalm,
		kAngerAttack,
		kAngerFrenzy,
		kAngerStateCount
	};

	enum eFearState
	{
		kFearCalm,
		kFearAlert,
		kFearThreaten,
		kFearFlee,
		kFearPanic,
		kFearStateCount
	};

	int m_behavior;
	MentalStates::Value m_currentValues[MentalStates::NumberOfMentalStates];
	eAngerState m_angerState;
	eFearState m_fearState;
	bool operator !=(const CreatureAttitude &other) const;
	float operator -(const CreatureAttitude &other) const;
};

// ======================================================================

	struct CreatureMod
	{
		AttribMod::AttribMod mod;        // the modifier description
		int                  maxVal;     // the max value of the mod, at the current time
		int                  currentVal; // the current value of the mod, at the current time
		uint32               startTime;  // time when the attrib mod was added to the creature
	};

// ======================================================================

class MonitoredCreatureMovement
{
public:
	MonitoredCreatureMovement();

	float m_lastDistance;
	float m_skittishness; // meters/meter
	float m_curve; // power curve used to advance

	bool operator!=(MonitoredCreatureMovement const &other) const;
	float operator-(MonitoredCreatureMovement const &other) const;
};

// ======================================================================

/** 
* CreatureObject refers to a tangible object that generally moves on its own accord.  This would include
* player avatars, creatures, droids, and other self-controlled entities.
*/

class CreatureObject : public TangibleObject
{
	friend class CreatureController;
	friend class AICreatureController;
public:

	enum Difficulty
	{
		  D_normal
		, D_elite
		, D_boss
	};

	enum AttributeRegeneration
	{
		AR_normal,
		AR_combat,
		AR_count
	};

	static char const * getMentalStateString(MentalStates::Enumerator const mentalState);
	static char const * getBehaviorString(Behaviors::Enumerator const behavior);

	typedef std::map<std::string, int>            ExperiencePointMap;
	typedef std::set<const SkillObject *>         SkillList;
	typedef std::vector<std::string>              StringVector;
	typedef std::pair<std::pair<NetworkId /*player*/, std::string /*name*/>, NetworkId /*ship*/> PlayerAndShipPair;
	
	CreatureObject(const ServerCreatureObjectTemplate* newTemplate);
	virtual            ~CreatureObject();

	virtual CreatureObject *       asCreatureObject();
	virtual CreatureObject const * asCreatureObject() const;

	static CreatureObject * getCreatureObject(NetworkId const & networkId);
	
	static CreatureObject const * asCreatureObject(Object const * object);
	static CreatureObject * asCreatureObject(Object * object);

	static void         install();
	static void         removeDefaultTemplate();

	virtual bool        alwaysSendReliableTransform() const;
	bool                isSpawningAllowed() const;
	void                addToSpawnQueue ();
	void                removeFromSpawnQueue  ();
	static void         runSpawnQueue();
	static void         showSpawnQueue(Unicode::String & result);
	virtual void        onClientReady (Client *c);
	virtual void        onClientAboutToLoad();
	virtual void        onLoadingScreenComplete();
	virtual bool        canManipulateObject(ServerObject const &target, bool movingObject, bool doPermissionCheckOnItem, bool doPermissionCheckOnParent, float maxDistance, Container::ContainerErrorCode&, bool skipNoTradeCheck = false, bool * allowedByGodMode = nullptr) const;

	virtual void        onRemovingFromWorld();
	void                addMembersToPackages();
	virtual void        addToWorld();

	virtual Controller*       createDefaultController  ();
	CreatureController*       getCreatureController() ;
	const CreatureController* getCreatureController() const;

	virtual void        setAuthority();
	virtual void        setAuthServerProcessId(uint32 processId);
	virtual float       alter(float time);
	
	virtual bool        canDestroy() const;
	
	float               getScaleFactor () const;
	void                setScaleFactor(float scale);
	virtual float       getHeight     () const;
	virtual float       getRadius     () const;
	virtual Object const * getStandingOn () const; // returns the object this creature is standing on
	virtual float       getFarNetworkUpdateRadius() const;

	Attributes::Value   getAttribute             (Attributes::Enumerator attribute) const;
	Attributes::Value   getUnmodifiedAttribute   (Attributes::Enumerator attribute) const;
	Attributes::Value   getMaxAttribute          (Attributes::Enumerator attribute, bool capStat = true) const;
	Attributes::Value   getUnmodifiedMaxAttribute(Attributes::Enumerator attribute) const;
	Attributes::Value   getAttributeBonus        (Attributes::Enumerator attribute) const;
	void                resetAttribute           (Attributes::Enumerator attrib, Attributes::Value);
	void                initializeAttribute      (Attributes::Enumerator attrib, Attributes::Value);
	bool                drainAttributes          (Attributes::Value action, Attributes::Value mind);
	int                 testDrainAttribute       (Attributes::Enumerator attribute, Attributes::Value value) const;
	int                 getShockWounds           () const;
	void                setShockWounds           (int wound);
	void                setAttribute             (Attributes::Enumerator attribute, Attributes::Value value);
	virtual void        getAttributes            (std::vector<std::pair<std::string, Unicode::String> > &data) const;
	void                sendTimedModData         (uint32 id, float time, bool updateCache = true);
	void                sendCancelTimedMod       (uint32 id);
	
	float               getInvulnerabilityTimer  () const;
	void                setInvulnerabilityTimer  (float duration);
	virtual bool        isInvulnerable           () const;
	virtual void        handleCMessageTo         (const MessageToPayload &message);

	virtual void        addAim ();
	virtual void        clearAims ();
	void                pushedMe(const NetworkId & attackerId, const Vector & attackerPos, const Vector & defenderPos, float distance);

	// slot manipulation functions

	ServerObject *        getInventory  ();
	const ServerObject *  getInventory  () const;
	ServerObject *        getDatapad    ();
	const ServerObject *  getDatapad    () const;
	ServerObject *        getBank       ();
	const ServerObject *  getBank       () const;
	ServerObject*         getHair       ();
	const ServerObject *  getHair       () const;
	ServerObject *        getAppearanceInventory ();
	const ServerObject *  getAppearanceInventory () const;
	ServerObject *		  getHangar		();
	const ServerObject *  getHangar     () const;
	ServerObject *        getObjectInSlot (const ConstCharCrcLowerString & slotName);
	const ServerObject *  getObjectInSlot (const ConstCharCrcLowerString & slotName) const;

	//Container triggers
	virtual bool        onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer);
	virtual int         onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer);
	virtual void        onContainerLostItem(ServerObject * destination, ServerObject& item, ServerObject* transferer);
	virtual void        onContainerGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer);

    virtual void        onContainerChildLostItem(ServerObject * destination, ServerObject& item, ServerObject* source, ServerObject* transferer);
	virtual void        onContainerChildGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer);
	
	virtual void        onContainerTransferComplete(ServerObject *oldContainer, ServerObject *newContainer);

	// trigger volume functions
	virtual void                              onAddedToTriggerVolume       (TriggerVolume & triggerVolume);
	virtual void                              onRemovedFromTriggerVolume   (TriggerVolume & triggerVolume);

	virtual const std::set<TriggerVolume *> * getTriggerVolumeEntered      () const;

	// crafting functions
	const std::map<std::pair<uint32, uint32>,int> &  getDraftSchematics () const;
	bool                                isIngredientInInventory(const Object & ingredient) const;
	void                                disableSchematicFiltering(void);
	void                                enableSchematicFiltering(void);
	bool                                isSchematicFilteringEnabled(void);
	void                                getManufactureSchematics(std::vector<const ManufactureSchematicObject *> & schematics);
	void                                getManufactureSchematics(std::vector<const ManufactureSchematicObject *> & schematics, uint32 craftingTypes);

	// inventory functions
	bool isItemEquipped (const Object & item) const;

	// skill system methods
	int                 getExperiencePoints(const std::string & experienceType) const;
	const std::map<std::string, int> &  getExperiencePoints() const;
	const SkillList &   getSkillList() const;
	const std::map<std::string, std::pair<int, int> > & getModMap() const;
	const int           getModValue(const std::string & modName) const;
	const int           getEnhancedModValue(const std::string & modName) const;	
	const int           getEnhancedModValueUncapped(const std::string & modName) const;  // This version does not respect getMaxCreatureSkillModBonus
	void                setModValue(const std::string & modName, const int value);
	void                addModValue(const std::string & modName, const int value, bool fromSkill);
	const int           grantExperiencePoints(const std::string & experienceType, int amount);
	virtual const bool  grantSkill(const SkillObject & newSkill);
	const bool          grantSchematicGroup(const std::string & groupName, bool fromSkill);
	const bool          grantSchematic(const std::string & schematicName, bool fromSkill);
	const bool          grantSchematic(uint32 schematicCrc, bool fromSkill);
	const bool          revokeSchematic(const std::string & schematicName, bool fromSkill);
	const bool          revokeSchematic(uint32 schematicCrc, bool fromSkill);
	const bool          hasSchematic(const std::string & schematicName);
	const bool          hasSchematic(uint32 schematicCrc);
	const bool          hasSkill(const SkillObject & skill) const;
	bool                hasCertificationsForItem(const TangibleObject & item) const;
	virtual void        revokeSkill(const SkillObject & oldSkill, bool silent = false);
	//---

	bool                showDanceVisuals(bool enabled);
	bool                showMusicianVisuals(bool enabled);
	void                sendMusicFlourish(int flourishIndex);

	virtual bool        isDisabled() const;
	bool                isIncapacitated () const;
	void                setIncapacitated(bool flag, const NetworkId & attackerId);
	bool                isDead () const;
	bool                makeDead(const NetworkId & killer, const NetworkId & corpse);
	bool                makeNotDead();
	const NetworkId&    getLookAtTarget() const;
	void                setLookAtTarget(const NetworkId&);	
	const NetworkId&    getIntendedTarget() const;
	void                setIntendedTarget(const NetworkId&);
	virtual bool        wantSawAttackTriggers() const;

	void                                addAttributeModifier(const std::string & name, Attributes::Enumerator attrib, int value, float duration, float attackRate, float decayRate, int flags);
	void                                addSkillmodModifier(const std::string & name, const std::string & skill, int value, float duration, int flags);
	int                                 addAttributeModifier(const AttribMod::AttribMod & mod, ServerObject * attacker = nullptr);
	bool                                hasAttribModifier(const std::string & modName) const;
	void                                removeAttributeModifier(const std::string & modName);
	void                                removeAttributeModifiers(Attributes::Enumerator attribute);
	void                                removeAllAttributeModifiers ();
	void								removeAllAttributeAndSkillmodMods ();
	const AttribMod::AttribMod *        getAttributeModifier(const std::string & modName) const;
	const std::map<uint32, CreatureMod> & getAttributeModifiers () const;

	// Mental state functions	
	MentalStates::Value  getMentalState(MentalStates::Enumerator attribute) const;
	MentalStates::Value  getMentalStateToward(const NetworkId &id, MentalStates::Enumerator attribute) const;
	MentalStates::Value  getUnmodifiedMentalState(MentalStates::Enumerator attribute) const;
	MentalStates::Value  getMaxMentalState(MentalStates::Enumerator attribute) const;
	float                getMentalStateDecay(MentalStates::Enumerator state) const;
	void                 getBehaviorTargets(Behaviors::Enumerator behavior, std::vector<NetworkId> &targets);
	Behaviors::Enumerator getBehavior() const;
	Behaviors::Enumerator getBehaviorToward(const NetworkId &target) const;

	//AI Behavior State modifiers
	float               getAcceleration(SharedCreatureObjectTemplate::MovementTypes  movementType) const;
	float               getTurnRate(SharedCreatureObjectTemplate::MovementTypes  movementType) const;
	float               getApproachTriggerRange() const;
	float               getSlopeModAngleMin() const;
	float               getSlopeModAngleMax() const;
	float               getSlopeModPercentMin() const;
	float               getSlopeModPercentMax() const;
	float               getWaterModPercentMin() const;
	float               getWaterModPercentMax() const;

	WeaponObject*                   getDefaultWeapon () const;
	WeaponObject*                   getReadiedWeapon () const;
	WeaponObject*                   getCurrentWeapon ();
	bool                            swapDefaultWeapons(WeaponObject &newDefaultWeapon, ServerObject &weaponContainer);
	void                            setCurrentWeapon(WeaponObject & weapon);
	int                             getInstrumentVisualId () const;
	int                             getInstrumentAudioId () const;
	GroupObject*                    getGroup() const;
	void                            setGroup(GroupObject *group, bool disbandingCurrentGroup);
	NetworkId const &               getGroupInviterId() const;
	std::string const &             getGroupInviterName() const;
	CreatureObject*                 getGroupInviter() const;
	void                            setGroupInviter(NetworkId const &inviterId, std::string const& inviterName, NetworkId const &inviterShipId);
	NetworkId const &               getInviterForPendingGroup() const;
	void                            setInviterForPendingGroup(NetworkId const &inviterId);
	int                             getPerformanceType() const;
	void                            setPerformanceType(int performanceType);
	int                             getPerformanceStartTime() const;
	void                            setPerformanceStartTime(int performanceStartTime);
	NetworkId const &               getPerformanceListenTarget() const;
	void                            setPerformanceListenTarget(NetworkId const &who);
	NetworkId const &               getPerformanceWatchTarget() const;
	void                            setPerformanceWatchTarget(NetworkId const &who);
	int                             getGuildId() const;
	void                            setGuildId(int guildId);
	unsigned long                   getTimeToUpdateGuildWarPvpStatus() const;
	void                            setTimeToUpdateGuildWarPvpStatus(unsigned long timeToUpdateGuildWarPvpStatus);
	bool                            getGuildWarEnabled() const;
	void                            setGuildWarEnabled(bool guildWarEnabled);
	int                             getMilitiaOfCityId() const;
	void                            setMilitiaOfCityId(int cityId);
	int                             getLocatedInCityId() const;
	void                            setLocatedInCityId(int cityId);
	NetworkId const &               getMasterId() const;
	void                            setMasterId(NetworkId const &masterId);

	SharedCreatureObjectTemplate::Gender  getGender () const;
	uint32                                getMood () const;
	void                                  setMood(uint32 mood);
	Postures::Enumerator                  getPosture () const;
	Locomotions::Enumerator               getLocomotion () const;
	bool                                  getState(States::Enumerator whichState) const;
	void                                  setState(States::Enumerator whichState, bool value);
	float                                 getWalkSpeed () const; // adjusted for posture and movement scale
	float                                 getRunSpeed () const; // adjusted for posture and movement scale
	float                                 getAccelerationForSpeed (float currentSpeed) const;
	float                                 getTurnRateForSpeed     (float currentSpeed) const;
	float                                 getSlopeMod             () const;
	uint32                                getSayMode () const;
	void                                  setSayMode(uint32 sayMode);
	const std::string &                   getAnimationMood () const;
	void                                  setAnimationMood(const std::string &);
	int                                   getNiche () const;
	SharedCreatureObjectTemplate::Species getSpecies () const;
	SharedCreatureObjectTemplate::Race    getRace () const;
	float                                 getStepHeight () const;
	bool                                  canMove () const;

	void                                  onBiographyRetrieved(const NetworkId &owner, const Unicode::String &bio);
	void                                  onCharacterMatchRetrieved(MatchMakingCharacterResult const &results);
	
	void                                  setMovementStationary();
	void                                  setMovementWalk();
	void                                  setMovementRun();

	bool                                  setSlopeModAngle(float angle);
	float                                 getSlopeModAngle() const;
	bool                                  setSlopeModPercent(float percent);
	float                                 getBaseSlopeModPercent() const;
	float                                 getSlopeModPercent() const;
	void                                  recomputeSlopeModPercent();
	bool                                  setWaterModPercent(float percent);
	float                                 getWaterModPercent() const;
	bool                                  setMovementScale(float percent);
	float                                 getMovementScale() const;
	bool                                  setMovementPercent(float percent);
	float                                 getMovementPercent() const;
	bool                                  setTurnPercent(float percent);
	float                                 getTurnPercent() const;
	bool                                  setAccelScale(float percent);
	float                                 getAccelScale() const;
	bool                                  setAccelPercent(float percent);
	float                                 getAccelPercent() const;
	float                                 getSwimHeight() const;
	void                                  setBaseWalkSpeed(float speed);
	float                                 getBaseWalkSpeed() const;
	void                                  setBaseRunSpeed(float speed);
	float                                 getBaseRunSpeed() const;

	int                                   getCover () const;
	bool                                  getCoverVisibility() const;

	void                                  sitOnObject(NetworkId const &chairCellId, Vector const &chairPosition_w);
	virtual void                          speakText(MessageQueueSpatialChat const &spatialChat);
	virtual bool                          immediateLogoutAllowed();
	
	//Banks
	ServerObject *                         getBankContainer() const;
	Unicode::String                        getBankName() const;
	bool                                   joinBank(Unicode::String bankName);
	void                                   quitBank();
	bool                                   isBankMember(Unicode::String bankName) const;

	// Missions
	ServerObject *                         getMissionBag() const;
	bool                                   assignMission(MissionObject * mission);
	void                                   addMissionToPlayerListRequest(MissionObject * mission);
	std::vector<CachedNetworkId>           getMissions();
	void                                   missionListRequestComplete(ServerObject *terminal);
	void                                   setMissionBoardUISequenceId(const uint8 sequence);
	void                                   addToMissionRequestQueue(const NetworkId & terminalId);
	void                                   removeFromMissionRequestQueue(const NetworkId & terminalId);
	static void                            updateMissionRequestQueue();
	void                                   setNumberOfMissionsWantedInMissionBag(int numberOfMissions);

	// ======
	// Mounts: PUBLIC: implemented in CreatureObject_Mounts.cpp
	// ======

	// Called on mount creature.
	int                                    getMountabilityStatus() const;
	void                                   makePetMountable();
	bool                                   isMountable() const;
	bool                                   isMountableAndHasRoomForAnother() const;
	NetworkId                              getPetControlDeviceId() const;
	bool                                   detachRider(NetworkId const rider);
	bool                                   detachAllRiders();
	CreatureObject                  const *getPrimaryMountingRider() const;
	CreatureObject                        *getPrimaryMountingRider();
	void                                   getMountingRiders(std::vector<const CreatureObject *> & riders) const;
	void                                   getMountingRiders(std::vector<CreatureObject *> & riders);

	// Called on rider.
	bool                                   mountCreature(CreatureObject &mountObject);
	CreatureObject                 const  *getMountedCreature() const;
	CreatureObject                        *getMountedCreature();
	void                                   emergencyDismountForRider();

	// ======
	// Ships: PUBLIC: implemented in CreatureObject_Ships.cpp
	// ======

	ShipObject                            *getPilotedShip();
	ShipObject const                      *getPilotedShip() const;
	bool                                   pilotShip(ServerObject &pilotSlotObject);
	bool                                   unpilotShip();

	void                                   getAllShipsInDatapad(std::vector<NetworkId> & ships) const;

	// ======

	struct CreatureObjectPointerHash
	{
		size_t operator()(const CreatureObject * const ptr) const
		{
			return (reinterpret_cast<const size_t>(ptr) >> 4);
		};
	};

	typedef std::unordered_set<const CreatureObject *, CreatureObjectPointerHash> AllCreaturesSet;
	static const AllCreaturesSet & getAllCreatures();

	bool monitorCreatureMovement(const CachedNetworkId &ofTarget, float i_skittishness, float i_curve);
	bool ignoreCreatureMovement(const CachedNetworkId &ofTarget);
	
	virtual CommandQueue * getCommandQueue() const;

	bool isInTutorial() const;

	// Jedi functions
	void addJediToAccountAck(void);
	bool isJedi() const;

	// Housing functions
	int                     getMaxNumberOfLots() const;
	NetworkId               getHouse(void) const;
	void                    setHouse(const CachedNetworkId & houseId);

	// Character transfer
	bool receiveCharacterTransferMessage(const std::vector<unsigned char> & packedCharacterData);
	void  receiveCharacterTransferStatusMessage(const std::string & statusMessage);
	std::vector<unsigned char> uploadCharacterData(const bool withItems, const bool allowOverride);

	int loadPackedHouses();

	void setClientUsesAnimationLocomotion(bool const enabled);
	void addSlowDownEffect(const TangibleObject & defender, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime);
	bool addSlowDownEffectProxy(const TangibleObject & defender, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime);
	void removeSlowDownEffect();
	void removeSlowDownEffectProxy();
	void addTerrainSlopeEffect(const Vector & normal);

	void recomputeCommandSeries();
	void incrementKillMeter(int amount);

public:
	void      setCover(int cover);
	void      setCoverVisibility(bool isVisible);
	void      setLocomotion(Locomotions::Enumerator posture);
	void      setMaxAttribute(Attributes::Enumerator attribute, Attributes::Value value, bool verifyPlayerMaxStats = true);
	void      setMentalState(MentalStates::Enumerator state, MentalStates::Value value);
	void      setMentalStateToward(const NetworkId &target, MentalStates::Enumerator state, MentalStates::Value value);
	void      setMentalStateTowardClampBehavior(const NetworkId &target, MentalStates::Enumerator state, MentalStates::Value value, Behaviors::Enumerator behavior);
	void      setMaxMentalState(MentalStates::Enumerator state, MentalStates::Value value);
	void      setMentalStateDecay(MentalStates::Enumerator state, float value);
	void      setPosture(Postures::Enumerator posture, bool isClientImmediate = false);
	
	void      requestMovementInfoUpdate();

	bool      getIsStatic() const;
	void      setIsStatic(bool);

	bool      isBeast() const;
	void      setIsBeast(bool);

	bool      setForceShowHam(bool show);

	static void  runMissionCreationQueue();

	virtual   bool hasBounty(const CreatureObject & target) const;
	virtual   bool hasBounty() const;
	virtual std::vector<NetworkId> const & getJediBountiesOnMe() const;
	bool      hasBountyMissionForTarget(const NetworkId & targetId) const;

	void      setAlternateAppearance(std::string const &sharedObjectTemplateName);

	std::vector<WearableEntry> getWearableData();
	void     setWearableData(std::vector<WearableEntry> const & packedWearables);

	virtual void onContainedItemAppearanceDataModified(TangibleObject const & target, std::string const & oldValue, std::string const & value);

	void forceNotifyRegionsCheck();
	void checkNotifyRegions();

	void addMissionCriticalObject(NetworkId const & networkId);
	void removeMissionCriticalObject(NetworkId const & networkId);
	void clearMissionCriticalObjects();
	bool isMissionCriticalObject(NetworkId const & networkId) const;
	typedef std::set<NetworkId> MissionCriticalObjectSet;
	MissionCriticalObjectSet const & getMissionCriticalObjects() const;
	typedef std::set<std::pair<NetworkId, NetworkId> > GroupMissionCriticalObjectSet;
	void setGroupMissionCriticalObjects(GroupMissionCriticalObjectSet const & groupMissionCriticalObjectSet);
	
	bool processExpertiseRequest(std::vector<std::string> const &addExpertisesNamesList, bool clearAllExpertisesFirst);
	bool clearAllExpertises();

	float getPseudoPlayedTime() const;
	void setPseudoPlayedTime(float alterTime);
	
	bool hasBuff(uint32 buffNameCrc) const;
	void addBuff(uint32 buffNameCrc, float duration = 0.0f, float dynamicValue = 0.0f, NetworkId caster = NetworkId::cms_invalid, uint32 stackCount = 1 );
	void addBuff(uint32 buffNameCrc, float timeTillRemoval, float dynamicValue, float totalDuration, NetworkId caster, uint32 stackCount = 1);
	void removeBuff(uint32 buffNameCrc);
	void getAllBuffs(std::vector<uint32>  & buffCrcs) const;
	bool getBuff(uint32 buffNameCrc, Buff & buff) const;
	void decrementBuff(uint32 const buffNameCrc, uint32 const stacksToRemove = 1);
	void decayBuff(uint32 buffNameCrc, float decayPercentage);

	int16 getLevel() const;
	int  getLevelXp() const;
	void setLevel(int level);  // this is called to force this object to the specified level irregardless of the current skills - this can change level xp and health
	void recalculateLevel();
	void fixupPersistentBuffsAfterLoading();
	void fixupLevelXpAfterLoading();

	void doWarmupChecks(
		const Command &command,
		const NetworkId &target,
		const Unicode::String &params,
		Command::ErrorCode &status,
		int &detail );

	Difficulty getDifficulty() const;
	void setDifficulty(Difficulty const difficulty);

	int32 getHologramType() const;
	void setHologramType(int32 newHologramType);

	bool getVisibleOnMapAndRadar() const;
	void setVisibleOnMapAndRadar(bool visible);

	float getRegenRate(Attributes::Enumerator poolAttrib) const;
	void  setRegenRate(Attributes::Enumerator poolAttrib, float value);

	float getLavaResistance() const;
	unsigned long getLastWaterDamageTime() const;
	void setLastWaterDamageTime(unsigned long newTime);

	std::map<std::string, int> const & getCommandList() const;
	void clearCommands();
	bool grantCommand(std::string const & commandName, bool fromSkill);
	void revokeCommand(std::string const & command, bool fromSkill, bool ignoreCount = false);
	bool hasCommand(std::string const & commandName) const;

	void setLookAtYaw(const float lookAtYaw, bool useLookAtYaw);
	float getLookAtYaw() const;
	bool getUseLookAtYaw() const;
	
	bool     isAppearanceEquippable(const char *appearanceTemplateName);

	void getLfgCharacterData(LfgCharacterData & lfgCharacterData) const;
	
	void clientMinigameOpen(ValueDictionary const & messageData);
	void clientMinigameClose(ValueDictionary const & messageData);

	void			  setDecoyOrigin(NetworkId const & originalCreature);
	NetworkId const & getDecoyOrigin() const;

	void saveDecorationLayout(ServerObject const & pobSourceObject, int saveSlotNumber, std::string const & description);
	void restoreDecorationLayout(ServerObject const & pobTargetObject, int saveSlotNumber);

protected:

	virtual void        endBaselines();
	virtual void        onLoadedFromDatabase();
	virtual void        initializeFirstTimeObject();
	virtual void setInCombat(bool inCombat);

	//void      setAnimationState(AnimationState state);
	void      clipMentalStatesToward();
	void      deleteUnusedMentalStatesToward(bool * i_stateChanged);
	void      initializeDefaultWeapon();

	virtual void applyDamage(const CombatEngineData::DamageData &damageData);
	        
	virtual void decayAttributes(float time); // advance buffs, etc.
	virtual void decayMentalStates(float time); // do decay towards base value
	virtual void reportMonitoredCreatures(float time);

	Locomotions::Enumerator calcLocomotion() const;
	const MovementTable * getMovementTable() const;
	
	virtual bool handleContentsSetup();
	virtual void forwardServerObjectSpecificBaselines() const;
	virtual void sendObjectSpecificBaselinesToClient(Client const &client) const;
	virtual float getPvpRegionCheckTime();
	virtual void updatePlanetServerInternal(bool forceUpdate) const;

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;

	virtual void calcPvpableState();

	uint32 internalGetPlayedTime() const;
	void alterPseudoPlayedTime(float alterTime);

	void updateBuffs();
	bool stillHasBuffState(uint32 buffNameCrc, int state) const;
	
	void setLevelData(int16 level, int levelXp, int health);
	virtual void levelChanged() const;

	virtual void observersCountChanged();

	bool isStopped() const;
	bool isWalking() const;
	bool isRunning() const;
	
private:

	// Disabled.
	CreatureObject();
	CreatureObject(const CreatureObject& rhs);
	CreatureObject&	operator=(const CreatureObject& rhs);

	static void remove();

	// ======
	// Mounts: PRIVATE: implemented in CreatureObject_Mounts.cpp
	// ======

	static void  installForMounts();

	void         transferRiderPositionToMount();
	void         alterAuthoritativeForMounts();
	void         alterAnyForMounts();
	bool         onContainerAboutToTransferForMounts(ServerObject const * destination, ServerObject* transferer);
	
	// ======

	Attributes::Value getAdjustedAttribute(Attributes::Enumerator attribute, Attributes::Value value) const;

	void     setupInventory();
	void     setupSkillData();
	int      alterAttribute(Attributes::Enumerator attrib, int delta, bool checkIncapacitation, const NetworkId & source = NetworkId::cms_invalid, bool force = false);
	void     testIncapacitation(const NetworkId & attackerId);
	void     initializeNewPlayer  ();
	
	void     addPackedWearable(std::string const &appearanceData, int arrangementIndex, NetworkId const &networkId, uint32 sharedTemplateCrcValue, const BaselinesMessage * weaponSharedBaselines = nullptr, const BaselinesMessage * weaponSharedNpBaselines = nullptr);
	void     addPackedAppearanceWearable(std::string const &appearanceData, int arrangementIndex, NetworkId const &networkId, uint32 sharedTemplateCrcValue);	
	void     packWearables();
	void     computeTotalAttributes ();

	bool     potentiallyAddToTerrain();

	void     addModBonus(const std::string & modName, int bonus);
	void     addAttribBonus(int attrib, int bonus);
	void     updateSlopeMovement(const std::string & modName);

	int      getCurrentTargetsTotal(const std::map<Attributes::Enumerator, Attributes::Value> & targets);
	
	int      changeAttribModCurrentValue(Attributes::Enumerator attrib, int delta, bool checkPoolOverflow);
	void     recomputeAttribModTotals(bool checkPoolOverflow);

	void     updateMovementInfo();
	void     updateVehiclePhysicsData ();

	void     unequipAllItems();
	
	void     updateGroup();
	
	void     handleTutorialTransition();

protected:
	virtual void       virtualOnAddedToWorldCell();
	virtual void       virtualOnRemovedFromWorldCell();
	virtual void       virtualOnSetAuthority();
	virtual void       virtualOnReleaseAuthority();
	virtual void       virtualOnSetClient();
	virtual void       virtualOnLogout();
private:

	struct PostureChangeCallback
	{
		void modified(CreatureObject &target, Postures::Enumerator oldValue, Postures::Enumerator value, bool isLocal) const;
	};
	friend struct PostureChangeCallback;

	struct LocomotionChangeCallback
	{
		void modified(CreatureObject &target, Locomotions::Enumerator oldValue, Locomotions::Enumerator value, bool isLocal) const;
	};
	friend struct LocomotionChangeCallback;

	struct StateChangeCallback
	{
		void modified(CreatureObject &target, uint64 oldValue, uint64 newValue, bool isLocal) const;
	};
	friend struct StateChangeCallback;

	typedef std::vector<MessageQueueMissionListResponseData>         MessageQueueMissionListResponseDataVector;
	MessageQueueMissionListResponseDataVector      m_missionResponseEntries;

	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given
	
	CreatureAttitude const *getAttitudeToward(NetworkId const &m_id) const;
	bool                 updateBehavior(NetworkId const *id, CreatureAttitude &state) const;
	void                 notifyTargetChange(int state, int newState, bool * i_stateChanged);
	void                 checkAndRestoreRequiredSlots();

	void                 getAvailableLocomotions();
	
	void                 setStatesDueToContainment();
	
	bool	             doesLocomotionInvalidateCommand(Command const &cmd) const;

	int                  internalGetInstrumentAudioId(NetworkId const &id) const;
	int                  internalGetInstrumentVisualId(NetworkId const &id) const;

	int getExpertisePointsSpentForPlayerInTree(int tree);
	void getExpertisesForPlayer(SkillList & expertiseList) const;
	int getExpertiseRankForPlayer(std::string const & expertiseName);
	int getRemainingExpertisePoints() const;

	mutable CommandQueue * m_commandQueue;
	Archive::AutoDeltaVariable<bool>                 m_isStatic;

	// same goes for the shield
	Watcher<TangibleObject>                          m_shield;		// personal shield that absorbs damage before any armor


	float                                            m_regeneration[Attributes::NumberOfAttributes];   ///< Accumulated regeneration points
	float                                            m_regenerationTime;                               ///< Accumulated regeneration time
	
	// BPM CreatureObject : TangibleObject // Begin persisted members.
	Archive::AutoDeltaVector<Attributes::Value>      m_attributes;           ///< The current attributes (health, action, mind) of the mobile.
	Archive::AutoDeltaVector<Attributes::Value>      m_maxAttributes;        ///< The max unaffected attributes of this mobile.
	Archive::AutoDeltaVector<Attributes::Value>      m_totalAttributes;      ///< The current attributes, with all mods applied
	Archive::AutoDeltaVector<Attributes::Value>      m_totalMaxAttributes;   ///< The max attributes, with all mods applied
	Archive::AutoDeltaVector<Attributes::Value>      m_attribBonus;          ///< Bonus from items that are added to the max attrib values
	Archive::AutoDeltaVariable<int>                  m_shockWounds;          ///< Shock wounds taken
	Archive::AutoDeltaVariable<bool>                 m_allowSARegen;         ///< If this is set to false, special ability pools will not regenerate
	Archive::AutoDeltaMap<uint32, CreatureMod>       m_attributeModList;     ///< List of modifies to the mobile's attributes (such as wounds or buffs).
	Archive::AutoDeltaVector<Attributes::Value>      m_cachedCurrentAttributeModValues; ///< The current value of all the attribute mods being added to the attribute values.
	Archive::AutoDeltaVector<Attributes::Value>      m_cachedMaxAttributeModValues;     ///< The current value of all the attribute mods being added to the max attribute values.
	Archive::AutoDeltaVariable<CreatureAttitude>     m_currentAttitude;      ///< The current general attitude
	Archive::AutoDeltaVariable<Behaviors::Enumerator> m_lastBehavior;        ///< The last overall selected behavior
	Archive::AutoDeltaMap<NetworkId, CreatureAttitude> m_mentalStatesToward;   ///< The current mental states toward another mobile
	Archive::AutoDeltaVector<MentalStates::Value>    m_maxMentalStates;      ///< The max unaffected mental states of this mobile.
	Archive::AutoDeltaVector<float>                  m_mentalStateDecays;    ///< The time it takes for the mental state to decay halfway to it's original value
	MentalStates::Value    m_fearCutoffs[CreatureAttitude::kFearStateCount];          ///< Cutoff values for fear state transitions
	MentalStates::Value    m_angerCutoffs[CreatureAttitude::kAngerStateCount];         ///< Cutoff values for anger state transitions
	std::set<NetworkId>                              m_targetsByBehavior[Behaviors::NumberOfBehaviors];

	Archive::AutoDeltaVariable<float>                m_scaleFactor;          ///< Value to resize the creature on the client by

	Archive::AutoDeltaMap<NetworkId, MonitoredCreatureMovement> m_monitoredCreatureMovements; ///< The creatures we are monitoring
	Archive::AutoDeltaVariable<Vector> m_lastMonitorReportPosition;
	Archive::AutoDeltaVariableObserver<int, PvpUpdateAndCellPermissionsObserver, CreatureObject> m_guildId;

	// when switching guild war pvp status using the guild war exemption/exclusive list,
	// add a delay to when the actually switch takes place, to prevent exploit of quickly
	// switching in and out guiild war pvp using the guild war exemption/exclusive list
	Archive::AutoDeltaVariable<unsigned long> m_timeToUpdateGuildWarPvpStatus;

	Archive::AutoDeltaVariableObserver<bool, PvpUpdateObserver, CreatureObject> m_guildWarEnabled;
	Archive::AutoDeltaVariableObserver<int, PvpUpdateObserver, CreatureObject> m_militiaOfCityId;
	Archive::AutoDeltaVariableObserver<int, PvpUpdateObserver, CreatureObject> m_locatedInCityId;
	Archive::AutoDeltaVariableObserver<NetworkId, PvpUpdateObserver, CreatureObject> m_masterId;

	// ***DEPRECATED*** use PlayerObject::m_currentGcwRank instead
	Archive::AutoDeltaVariable<uint8>                m_rank;          ///< The rank designation value for this player. 0 = no rank designated.

	// Worldspace XYZ used to figure out where to put plaers if they logout from a buildout structure (which the DB isn't aware of)
	Archive::AutoDeltaVariable<float>		m_wsX;
	Archive::AutoDeltaVariable<float>		m_wsY;
	Archive::AutoDeltaVariable<float>		m_wsZ;

	//EPM
	
	Archive::AutoDeltaVariable<unsigned char>        m_animState;     ///< The current animation state.
	Archive::AutoDeltaVariable<unsigned char>        m_mood;          ///< The mood the mobile is in.
	Archive::AutoDeltaVariable<unsigned char>        m_sayMode;       ///< The current default say text that a character uses when speaking.
	Archive::AutoDeltaVariable<std::string>          m_animationMood;
	Archive::AutoDeltaVariableCallback<Postures::Enumerator, PostureChangeCallback, CreatureObject> m_posture;       ///< The posture of the mobile
	Archive::AutoDeltaVariableCallback<Locomotions::Enumerator, LocomotionChangeCallback, CreatureObject> m_locomotion;       ///< The locomotion of the mobile
	Archive::AutoDeltaVariable<int>                  m_stopWalkRun;   ///< does the creature want to stop, walk, or run?
	Archive::AutoDeltaVariableCallback<uint64, StateChangeCallback, CreatureObject> m_states;       ///< The states set on the creature
	Archive::AutoDeltaVariable<float>                m_movementScale;     // scale to the creature's base movement rate
	Archive::AutoDeltaVariable<float>                m_movementPercent;   // script-defined mod to the creature's base movement rate
	Archive::AutoDeltaVariable<float>                m_baseWalkSpeed;
	Archive::AutoDeltaVariable<float>                m_walkSpeed;         // scale to the creature's base movement rate
	Archive::AutoDeltaVariable<float>                m_baseRunSpeed;
	Archive::AutoDeltaVariable<float>                m_runSpeed;          // scale to the creature's base movement rate
	Archive::AutoDeltaVariable<float>                m_accelScale;        // scale to the creature's base acceleration rate
	Archive::AutoDeltaVariable<float>                m_accelPercent;      // script-defined mod to the creature's base acceleration rate
	Archive::AutoDeltaVariable<float>                m_turnScale;         // scale to the creature's base turn rate
	Archive::AutoDeltaVariable<float>                m_slopeModAngle;     // angle at which the creature starts to slow down
	Archive::AutoDeltaVariable<float>                m_baseSlopeModPercent;   // unmodified scale to the creature's base movement rate for a 90 degree slope
	Archive::AutoDeltaVariable<float>                m_slopeModPercent;   // scale to the creature's base movement rate for a 90 degree slope, modified by skill
	Archive::AutoDeltaVariable<float>                m_waterModPercent;   // scale to the creature's base movement rate for movement in water

	Archive::AutoDeltaVariable<NetworkId>  m_lookAtTarget;  ///< Creature's current target
	Archive::AutoDeltaVariable<NetworkId>  m_intendedTarget;  ///< Creature's intended target
	
	Archive::AutoDeltaVariable<int>                  m_cover;         ///< current cover value for combat
	Archive::AutoDeltaVariable<bool>                 m_coverVisibility; // whether the creature is visibile
	Archive::AutoDeltaVariable<CachedNetworkId> m_currentWeapon; ///< current weapon creature is using in combat

	// pair: unmodified skill mod, bonus from equipped objects
	Archive::AutoDeltaMap<std::string, std::pair<int, int> >   m_modMap;        // sync calculated mod information with client and server (pistol_accuracy, for example)

	Archive::AutoDeltaSet<const SkillObject *>    m_skills;

	// Mission system related data
	ServerObject *                                   m_missionTerminal;
	
	int                                              m_deferComputeTotalAttributes;
	Archive::AutoDeltaVariableObserver<CachedNetworkId, GroupIdObserver, CreatureObject> m_group;
	Archive::AutoDeltaVariable<PlayerAndShipPair>    m_groupInviter;
	Archive::AutoDeltaVariable<NetworkId>            m_inviterForPendingGroup;  // "pending group" means you accepted an invitation but have not received a group object
	Archive::AutoDeltaVariable<int>                  m_performanceType;
	Archive::AutoDeltaVariable<int>                  m_performanceStartTime;
	Archive::AutoDeltaVariable<NetworkId>            m_performanceListenTarget;
	Archive::AutoDeltaVariable<NetworkId>            m_performanceWatchTarget;

	/// Specifies the string used by the client to determine what to do when in the Postures::SkillAnimating posture.
	Archive::AutoDeltaVariable<std::string>          m_animatingSkillData;

	Archive::AutoDeltaVariable<int16>                m_level;
	int16                                            m_previousLevel;
	Archive::AutoDeltaVariable<int>                  m_totalLevelXp;
	Archive::AutoDeltaVariable<int>                  m_levelHealthGranted;

	Archive::AutoDeltaVector<WearableEntry>          m_wearableData;
	Archive::AutoDeltaVariableObserver<float, PvpUpdateObserver, CreatureObject> m_invulnerabilityTimer;

	Archive::AutoDeltaVariable<std::string>          m_alternateAppearanceSharedObjectTemplateName;
	ClientMfdStatusUpdateMessage *                   m_clientMfdStatusUpdateMessage;

	// the list of timed mods that has been sent to the client;
	// we store this in a vector instead of a set/map because
	// we want to preserve the order that the mods were added,
	// so that when we need to update the client with this information,
	// the client will display the information in the same order
	// as it previously dispayed it (in the modifiers window)
	Archive::AutoDeltaVector<uint32>                 m_timedMod;
	Archive::AutoDeltaVector<float>                  m_timedModDuration;
	Archive::AutoDeltaVector<uint32>                 m_timedModUpdateTime;
	
	uint8    m_missionListRequestSequenceId;
	bool     m_addedToSpawnQueue;
	int      m_numberOfMissionsWantedInMissionBag;

	//-- Cached by updateMovementInfo() so we don't have to make expensive MovementTable calls every alter call in calcLocomotion().
	Locomotions::Enumerator  m_fastLocomotion;
	Locomotions::Enumerator  m_slowLocomotion;
	Locomotions::Enumerator  m_stationaryLocomotion;

	struct VehiclePhysicsData
	{
		float m_minMoveSpeed;
		float m_walkSpeed;
		float m_runSpeed;
		float m_accelerationMin;
		float m_accelerationMax;
		float m_turnRateMin;
		float m_turnRateMax;
	};

	VehiclePhysicsData * m_vehiclePhysicsData;
	
	Vector m_lastRegionNotifyPosition;
	Archive::AutoDeltaSet<std::pair<std::string, std::string> > m_notifyRegions;

	Archive::AutoDeltaSet<NetworkId> m_missionCriticalObjectSet;
	Archive::AutoDeltaSet<std::pair<NetworkId, NetworkId> > m_groupMissionCriticalObjectSet;

	/** The list of TriggerVolume this object is currently in
	 */
	std::set<TriggerVolume *> m_triggerVolumeEntered;

	float m_pseudoPlayedTime;

	//Buffs
	Archive::AutoDeltaMap<uint32, Buff::PackedBuff>       m_buffs;
	Archive::AutoDeltaPackedMap<uint32, Buff::PackedBuff> m_persistedBuffs;

	Archive::AutoDeltaVariable<bool> m_clientUsesAnimationLocomotion;
	Archive::AutoDeltaVariable<unsigned char> m_difficulty;
	Archive::AutoDeltaVariable<int32> m_hologramType;
	Archive::AutoDeltaVariable<bool> m_visibleOnMapAndRadar;

	//Look at position
	float m_lookAtYaw;
	bool m_useLookAtYaw;
	int m_lookAtPositionSequenceId;

	bool m_fixedupPersistentBuffsAfterLoading;
	bool m_fixedupLevelXpAfterLoading;
	float m_lavaResistance;
	unsigned long m_lastWaterDamageTime; // used for timing last damage taken by lava (and other future harmful water types)

	float m_attribRegenMultipliers[AR_count];
	Archive::AutoDeltaMap<std::string, int> m_commands; // game commands a creature may execute

	Archive::AutoDeltaVariable<bool> m_isBeast;
	Archive::AutoDeltaVariable<bool> m_forceShowHam;

	Timer m_regionFlagTimer; // Occasionally verify we have the correct region flags.

	Archive::AutoDeltaVector<WearableEntry> m_wearableAppearanceData; // Vector for our appearance items.

	Archive::AutoDeltaVariable<NetworkId>   m_decoyOrigin; // The OID of the player whom we copied for this decoy creature.

    // The max number of lots available to a player.  This value is also defined in base_class.java
	int m_maxHousingLots;
};

//----------------------------------------------------------------------
/*
inline CreatureObject::AnimationState CreatureObject::getAnimationState() const
{
    return static_cast<AnimationState>(m_animState.get());
}
*/
//----------------------------------------------------------------------

inline const std::set<TriggerVolume *> * CreatureObject::getTriggerVolumeEntered() const
{
	return &m_triggerVolumeEntered;
}

//----------------------------------------------------------------------

inline bool CreatureObject::getIsStatic() const
{
	return m_isStatic.get();
}

//----------------------------------------------------------------------

inline bool CreatureObject::isBeast() const
{
	return m_isBeast.get();
}

//----------------------------------------------------------------------

inline float CreatureObject::getScaleFactor () const
{
	return m_scaleFactor.get();
}

//----------------------------------------------------------------------

inline Attributes::Value CreatureObject::getUnmodifiedAttribute(Attributes::Enumerator attribute) const
{
	DEBUG_FATAL(attribute < 0 || attribute >= Attributes::NumberOfAttributes, ("attribute out of range\n"));
	return m_attributes[attribute];
}

//----------------------------------------------------------------------

inline Attributes::Value CreatureObject::getUnmodifiedMaxAttribute(Attributes::Enumerator attribute) const
{
	DEBUG_FATAL(attribute < 0 || attribute >= Attributes::NumberOfAttributes, ("attribute out of range\n"));

	// @NOTE: special case for health attribute adds level health
	return (Attributes::Health == attribute) ? m_maxAttributes[attribute] + m_levelHealthGranted.get() : m_maxAttributes[attribute];
}

//----------------------------------------------------------------------

inline int CreatureObject::getShockWounds () const
{
	return m_shockWounds.get();
}

//----------------------------------------------------------------------

inline Attributes::Value CreatureObject::getAttributeBonus(Attributes::Enumerator attribute) const
{
	DEBUG_FATAL(attribute < 0 || attribute >= Attributes::NumberOfAttributes, ("attribute out of range\n"));
	return m_attribBonus[attribute];
}

//----------------------------------------------------------------------

inline MentalStates::Value CreatureObject::getUnmodifiedMentalState(MentalStates::Enumerator attribute) const
{
	DEBUG_FATAL(attribute < 0 || attribute >= MentalStates::NumberOfMentalStates, ("mental state out of range\n"));
	//@ todo - decay here, or elsewhere?
	return m_currentAttitude.get().m_currentValues[attribute];
}

//----------------------------------------------------------------------

inline MentalStates::Value CreatureObject::getMaxMentalState(MentalStates::Enumerator attribute) const
{
	DEBUG_FATAL(attribute < 0 || attribute >= MentalStates::NumberOfMentalStates, ("mental state out of range\n"));
	return m_maxMentalStates[attribute];
}

//----------------------------------------------------------------------

inline float CreatureObject::getMentalStateDecay(MentalStates::Enumerator attribute) const
{
	DEBUG_FATAL(attribute < 0 || attribute >= MentalStates::NumberOfMentalStates, ("mental state out of range\n"));
	return m_mentalStateDecays[attribute];
}

//----------------------------------------------------------------------

inline SharedCreatureObjectTemplate::Gender  CreatureObject::getGender() const
{
	return safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getGender();
}

//----------------------------------------------------------------------

inline float CreatureObject::getSlopeModAngleMin() const
{
	return safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSlopeModAngleMin() * PI / 180;
}

//----------------------------------------------------------------------

inline float CreatureObject::getSlopeModAngleMax() const
{
	return safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSlopeModAngleMax() * PI / 180;
}

//----------------------------------------------------------------------

inline float CreatureObject::getSlopeModPercentMin() const
{
	return safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSlopeModPercentMin();
}

//----------------------------------------------------------------------

inline void CreatureObject::recomputeSlopeModPercent()
{
	setSlopeModPercent(m_baseSlopeModPercent.get());
}

//----------------------------------------------------------------------

inline float CreatureObject::getSlopeModPercentMax() const
{
	return safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSlopeModPercentMax();
}

//----------------------------------------------------------------------

inline float CreatureObject::getWaterModPercentMin() const
{
	return safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getWaterModPercentMin();
}

//----------------------------------------------------------------------

inline float CreatureObject::getWaterModPercentMax() const
{
	return safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getWaterModPercentMax();
}

//----------------------------------------------------------------------

inline uint32 CreatureObject::getMood() const
{
	return static_cast<uint32>(m_mood.get());
}

//----------------------------------------------------------------------

inline Postures::Enumerator CreatureObject::getPosture () const
{
	return m_posture.get();
}

//----------------------------------------------------------------------

inline bool CreatureObject::getState(States::Enumerator whichState) const
{
	if(whichState == States::Combat)
	{
		return isInCombat();
	}

	return (m_states.get() & States::getStateMask(whichState)) != 0;
}

//----------------------------------------------------------------------

inline uint32 CreatureObject::getSayMode() const
{
	return static_cast<uint32>(m_sayMode.get());
}

//----------------------------------------------------------------------

inline const std::string & CreatureObject::getAnimationMood() const
{
	return m_animationMood.get();
}

//----------------------------------------------------------------------

inline bool CreatureObject::isIncapacitated () const
{
	Postures::Enumerator posture = getPosture();
	return ( (posture == Postures::Incapacitated && !getState(States::FeignDeath)) || posture == Postures::Dead);
}

//----------------------------------------------------------------------

inline bool CreatureObject::isDead () const
{
	//@todo: hook into death implementation;
	return getPosture() == Postures::Dead;
}

//----------------------------------------------------------------------

inline const NetworkId& CreatureObject::getLookAtTarget() const
{
	return m_lookAtTarget.get();
}

//----------------------------------------------------------------------

inline const NetworkId& CreatureObject::getIntendedTarget() const
{
	return m_intendedTarget.get();
}

//-----------------------------------------------------------------------

inline int CreatureObject::getCover () const
{
	return m_cover.get();
}

//-----------------------------------------------------------------------

inline Locomotions::Enumerator CreatureObject::getLocomotion () const
{
	return m_locomotion.get();
}

//--------------------------------------------------------------------

inline const MovementTable * CreatureObject::getMovementTable() const
{
	const SharedCreatureObjectTemplate * sharedTemplate = safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate());
	const MovementTable * table = sharedTemplate->getMovementTable();
	
	return table;
}

//--------------------------------------------------------------------

inline float CreatureObject::getSlopeModAngle() const
{
	return m_slopeModAngle.get();
}

//--------------------------------------------------------------------

inline float CreatureObject::getBaseSlopeModPercent() const
{
	return m_baseSlopeModPercent.get();
}

//--------------------------------------------------------------------

inline float CreatureObject::getSlopeModPercent() const
{
	return m_slopeModPercent.get();
}

//--------------------------------------------------------------------

inline float CreatureObject::getWaterModPercent() const
{
	return m_waterModPercent.get();
}

//--------------------------------------------------------------------

inline float CreatureObject::getMovementScale() const
{
	return m_movementScale.get();
}

// ----------------------------------------------------------------------

inline float CreatureObject::getMovementPercent() const
{
	return m_movementPercent.get();
}

//--------------------------------------------------------------------

inline float CreatureObject::getTurnPercent() const
{
	return m_turnScale.get();
}

//--------------------------------------------------------------------

inline float CreatureObject::getAccelScale() const
{
	return m_accelScale.get();
}

// ----------------------------------------------------------------------

inline float CreatureObject::getAccelPercent() const
{
	return m_accelPercent.get();
}

// ======================================================================

inline int16  CreatureObject::getLevel() const
{
	return m_level.get();
}

inline int  CreatureObject::getLevelXp() const
{
	return m_totalLevelXp.get();
}

inline float CreatureObject::getPseudoPlayedTime() const
{
	return m_pseudoPlayedTime;
}

//-----------------------------------------------------------------------

inline float CreatureObject::getLavaResistance() const
{
	return m_lavaResistance;
}

//-----------------------------------------------------------------------

inline unsigned long CreatureObject::getLastWaterDamageTime() const
{
	return m_lastWaterDamageTime;
}

// ======================================================================

namespace Archive
{
	void put(Archive::ByteStream &target,   CreatureAttitude const &state);
	void get(Archive::ReadIterator &source, CreatureAttitude &state);
	void put(Archive::ByteStream &target,   MonitoredCreatureMovement const &state);
	void get(Archive::ReadIterator &source, MonitoredCreatureMovement &state);
	void put(Archive::ByteStream &target,   CreatureMod const &mod);
	void get(Archive::ReadIterator &source, CreatureMod &mod);
}

// ======================================================================

bool operator!=(CreatureMod const &a, CreatureMod const &b);
inline bool operator==(CreatureMod &a, CreatureMod const &b)
{
	return !(a != b);
}

// ----------------------------------------------------------------------

inline int32 CreatureObject::getHologramType() const
{
	return m_hologramType.get();
}

// ----------------------------------------------------------------------

inline bool CreatureObject::getVisibleOnMapAndRadar() const
{
	return m_visibleOnMapAndRadar.get();
}

// ----------------------------------------------------------------------

inline void CreatureObject::setDecoyOrigin(const NetworkId &originalCreature)
{
	m_decoyOrigin = originalCreature;
}

// ----------------------------------------------------------------------

inline NetworkId const & CreatureObject::getDecoyOrigin() const
{
	return m_decoyOrigin.get();
}

// ----------------------------------------------------------------------

inline int CreatureObject::getGuildId() const
{
	return m_guildId.get();
}

//-----------------------------------------------------------------------

inline unsigned long CreatureObject::getTimeToUpdateGuildWarPvpStatus() const
{
	return m_timeToUpdateGuildWarPvpStatus.get();
}

//-----------------------------------------------------------------------

inline bool CreatureObject::getGuildWarEnabled() const
{
	return m_guildWarEnabled.get();
}

// ======================================================================

#endif	// INCLUDED_CreatureObject_H
