// ======================================================================
//
// TangibleObject.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TangibleObject_H
#define INCLUDED_TangibleObject_H

#include "Archive/AutoDeltaByteStream.h"
#include "Archive/AutoDeltaSet.h"
#include "Archive/AutoDeltaSetObserver.h"
#include "Archive/AutoDeltaVariableCallback.h"
#include "Archive/AutoDeltaVariableObserver.h"
#include "Archive/AutoDeltaVectorObserver.h"
#include "serverGame/CellPermissions.h"
#include "serverGame/HateList.h"
#include "serverGame/Pvp.h"
#include "serverGame/PvpEnemiesObserver.h"
#include "serverGame/PvpUpdateObserver.h"
#include "serverGame/ServerArmorTemplate.h"
#include "serverGame/ServerObject.h"
#include "serverGame/TangibleConditionObserver.h"
#include "serverUtility/LocationData.h"
#include "serverUtility/PvpEnemy.h"
#include "sharedGame/Command.h"
#include "sharedGame/NpcConversationData.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgSharedUtility/CombatEngineData.h"

#include <list> // why is this needed?... stl forward decl isnt working here

class Client;
class CommandQueue;
class CreatureObject;
class CustomizationData;
class Footprint;
class IncubatorCommitMessage;
class ManufactureSchematicObject;
class NpcConversation;
class ProsePackage;
class RegionPvp;
class ServerSynchronizedUi;
class ServerTangibleObjectTemplate;
class StringId;

namespace CombatEngineData
{
	struct CombatData;
	struct DamageData;

	typedef std::vector<CachedNetworkId> TargetIdList;
};

namespace LocalObjectFlags
{
	enum
	{
		TangibleObject_Pvpable                   = ServerObject_Max,
		TangibleObject_CustomizationDataModified = ServerObject_Max+1,

		// This must come last.
		TangibleObject_Max
	};
}

/**
  *  A TangibleObject is an object that has a physical representation in the world.  It has physical properties such
  *  as weight and volume.  Most everything that a character interacts with is a tangible item.
  */
class TangibleObject : public ServerObject
{
	friend class TangibleController;

public:

	// these MUST be reflected in: 
	// //depot/swg/current/dsrc/sku.0/sys.server/compiled/game/object/tangible_object_template.tdf
	// //depot/swg/current/dsrc/sku.0/sys.server/compiled/game/script/base_class.java
	// //depot/swg/current/src/engine/client/library/clientGame/src/shared/object/TangibleObject.h
	// //depot/swg/current/src/engine/server/library/serverGame/src/shared/object/TangibleObject.h

	enum Conditions
	{
		C_onOff                = 0x00000001,
		C_vendor               = 0x00000002,
		C_insured              = 0x00000004,
		C_conversable          = 0x00000008,
		C_hibernating          = 0x00000010,
		C_magicItem            = 0x00000020,
		C_aggressive           = 0x00000040,
		C_wantSawAttackTrigger = 0x00000080,
		C_invulnerable         = 0x00000100,
		C_disabled             = 0x00000200,
		C_uninsurable          = 0x00000400,
		C_interesting          = 0x00000800,
		C_mount                = 0x00001000,    //   Set programmatically by mount system.  Do not set this in the template.
		C_crafted              = 0x00002000,    //   Set programmatically by crafting system.  Do not set this in the template.
		C_wingsOpened          = 0x00004000,    //   Set programmatically by wing system.  Do not set this in the template.
		C_spaceInteresting     = 0x00008000,
		C_docking              = 0x00010000,    //   Set programmatically by docking system.  Do not set this in the template.
		C_destroying           = 0x00020000,    //   Set programmatically by destruction system.  Do not set this in the template.
		C_commable             = 0x00040000,
		C_dockable             = 0x00080000,
		C_eject                = 0x00100000,
		C_inspectable          = 0x00200000,
		C_transferable         = 0x00400000,
		C_inflightTutorial     = 0x00800000,
		C_spaceCombatMusic     = 0x01000000,    //   Set programmatically by the AI system.  Do not set this in the template.
		C_encounterLocked      = 0x02000000,
		C_spawnedCreature      = 0x04000000,
		C_holidayInteresting   = 0x08000000,
		C_locked			   = 0x10000000,
	};

	explicit TangibleObject(const ServerTangibleObjectTemplate* newTemplate);
	virtual ~TangibleObject();

	static void install();

	static TangibleObject * getTangibleObject(NetworkId const & networkId);
	static TangibleObject * asTangibleObject(Object * object);
	static TangibleObject const * asTangibleObject(Object const * object);

	virtual TangibleObject *       asTangibleObject();
	virtual TangibleObject const * asTangibleObject() const;

	static void         removeDefaultTemplate   (void);

	virtual Controller* createDefaultController (void);
	void                addMembersToPackages    ();

	virtual float       alter    (float time);
	virtual void        conclude ();

	virtual void getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const;
	virtual void setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source);

	// combat functions
	virtual void                           addAim(void);
	virtual void                           clearAims(void);
	        bool                           isInCombat(void) const;

	void createCombatData();
	CombatEngineData::CombatData * getCombatData();
	CombatEngineData::CombatData const * getCombatData() const;

	// Hate List
	bool addHate(NetworkId const & target, float const hate);
	void addHateOverTime(NetworkId const & target, float const hate, int const seconds);
	bool setHate(NetworkId const & target, float const hate);
	float getHate(NetworkId const & target) const;
	std::map<NetworkId, std::pair<float, std::pair<unsigned long, unsigned long> > > const & getHateOverTime() const;
	float getMaxHate() const;
	CachedNetworkId const & getHateTarget() const;
	void removeHateTarget(NetworkId const & target);
	void clearHateList();
	HateList::UnSortedList const & getUnSortedHateList() const;
	void getSortedHateList(HateList::SortedList & sortedHateList) const; // Slower, don't call every frame
	void verifyHateList();
	bool isHatedBy(Object * const object);
	bool isOnHateList(NetworkId const & target) const;
	int getTimeSinceLastHateListUpdate() const;
	bool isHateListEmpty() const;
	HateList::RecentList const & getRecentHateList() const;
	void clearRecentHateList();
	void resetHateTimer();
	void setHateListAutoExpireTargetEnabled(bool enabled);
	bool isHateListAutoExpireTargetEnabled() const;
	int getHateListAutoExpireTargetDuration() const;
	bool isHateListOwnerPlayer() const;

	void forceHateTarget(NetworkId const & target);

	float getAILeashTime() const;
	void  setAILeashTime( float time );

	void                                   clearDamageList(void);

	virtual bool                 canDropInWorld () const;

	// access functions
	const char *         getCustomAppearance() const;
	const std::string &  getAppearanceData() const;
	int                  getDamageTaken() const;
	int                  getMaxHitPoints() const;
	bool                 isVisible() const;
	void                 initializeVisibility();
	void                 setVisible(bool isVisible);
	bool                 isHidden() const;
	void                 setHidden(bool const hide);

	int                  getCombatSkeleton() const;
	virtual bool         isDisabled() const;

	virtual const NetworkId &    getOwnerId() const;
	virtual bool         isOwner(NetworkId const &id, Client const *client) const;

	virtual bool         isVisibleOnClient(const Client & client) const;
	virtual bool         isInvulnerable() const;
	bool                 isInsured(void) const;
	void                 setInsured(bool insured);
	bool                 isUninsurable(void) const;
	void                 setUninsurable(bool uninsurable);
	virtual const char * getClientSharedTemplateName() const;
	int                  getCount(void) const;
	void                 setCount(int value);
	void                 incrementCount(int delta);
	int                  getCondition(void) const;
	bool                 hasCondition(int condition) const;
	void                 setCondition(int condition);
	void                 clearCondition(int condition);
	void                 rollupStructure(NetworkId const & owner, bool warnOnly);

	bool                hasEncumbrances() const;
	bool                getEncumbrances(std::vector<int> & encumbrances) const;

	virtual void        forwardServerObjectSpecificBaselines() const;
	virtual void        sendObjectSpecificBaselinesToClient(Client const &client) const;
	virtual void        onRemovingFromWorld();
	virtual void        unload();
	virtual void        onPermanentlyDestroyed();

	void getEquippedItems(uint32 combatBone, std::vector<TangibleObject *> & items) const;
	TangibleObject * getRandomEquippedItem(uint32 combatBone) const;


	void        setCustomAppearance(const std::string & newCustomAppearance);
	void        setAppearanceData(const std::string & newAppearanceData);

	int         getInterestRadius() const;

	std::vector<LocationData> const & getLocationTargets() const;
	void        addLocationTarget(const LocationData & location);
	void        removeLocationTarget(const Unicode::String & locationName);

	virtual int getNumberOfLots() const;

	// crafting functions
	bool                               isCrafted(void) const;
	const NetworkId                    getCraftedId(void) const;
	void                               setCraftedId(const NetworkId & id);
	int                                getCraftingType(void) const;
	int                                getCraftedType(void) const;
	void                               setCraftedType(int type);
	const NetworkId                    getCreatorId(void) const;
	void                               setCreatorId(const NetworkId & creatorId);
	const Unicode::String              getCreatorName(void) const;
	int                                getCreatorXp(void) const;
	void                               setCreatorXp(int xp);
	int                                getCreatorXpType(void) const;
	void                               setCreatorXpType(int type);
	uint32                             getSourceDraftSchematic() const;
	bool                               isCraftingTool(void) const;
	bool                               isCraftingStation(void) const;
	bool                               isRepairTool(void) const;
	ServerObject *                     getIngredientHopper(void) const;
	bool                               startCraftingSession(CreatureObject & crafter);
	bool                               stopCraftingSession(void);
	bool                               isIngredientInHopper(const NetworkId & ingredientId) const;
	bool                               addObjectToOutputSlot(ServerObject & object, ServerObject * transferer);
	ManufactureSchematicObject *       getCraftingManufactureSchematic(void) const;
	ManufactureSchematicObject *       removeCraftingManufactureSchematic(void);
	void                               setCraftingManufactureSchematic(ManufactureSchematicObject & schematic);
	void                               clearCraftingManufactureSchematic(void);
	ServerObject *                     getCraftingPrototype(void) const;
	void                               setCraftingPrototype(ServerObject & prototype);
	void                               clearCraftingPrototype(void);
	void                               setVisibleComponents(const std::vector<int> & components);
	static const SlotId &              getCraftingPrototypeSlotId();
	static const SlotId &              getCraftingManufactureSchematicSlotId();
	static const char *                generateSerialNumber(const NetworkId & id);

	void                               getAttribBonuses(std::vector<std::pair<int, int> > & attribBonuses) const;
	bool                               getAttribBonuses(std::vector<int> & attribBonuses) const;
	int                                getAttribBonus(int attribute) const;
	void                               setAttribBonus(int attribute, int bonus);
	void                               getSkillModBonuses(std::vector<std::pair<std::string, int> > & skillModBonuses, bool includeCategorizedSkillmod = true) const;
	int                                getSkillModBonus(const std::string & skillMod, bool includeCategorizedSkillmod = true) const;
	void                               setSkillModBonus(const std::string & skillMod, int bonus);
	void                               setSkillModBonus(const std::string & category, const std::string & skillMod, int bonus);
	void                               removeCategorizedSkillModBonuses(const std::string & category);
	bool                               addSkillModSocketBonus(const std::string & skillMod, int bonus);
	bool                               addSkillModSocketBonuses(const std::vector<std::pair<std::string, int> > & skillModBonuses);
	int                                getSkillModSockets(void) const;
	void                               setSkillModSockets(int sockets);
	void                               getRequiredCertifications(std::vector<std::string> & results) const;

	void                               forceExecuteCommand(Command const &command, NetworkId const &targetId, Unicode::String const &params, Command::ErrorCode &status, bool commandIsFromCommandQueue);
	virtual void                       setOwnerId(const NetworkId &id);
	void                               setMaxHitPoints(int maxHitPoints);
	void                               setDisabled(bool disabled);
	void                               setInvulnerable(bool invulnerable);
	virtual void                       applyDamage(const CombatEngineData::DamageData &damageData);
	virtual void                       getAttributes(const NetworkId & playerId, AttributeVector &data) const;
	virtual void                       getAttributes(AttributeVector &data) const;
	virtual void                       getAttributesForAuction(AttributeVector &data) const;

	// text output functions
	void                               showFlyText(const StringId &outputText, float scale, int r, int g, int b);
	void                               showFlyText(const Unicode::String &outputTextOOB, float scale, int r, int g, int b);
	void                               showFlyText(const Unicode::String &outputTextOOB, float scale, int r, int g, int b, int flags);
	void                               showCombatText(const TangibleObject & attackerObject, const StringId &outputText, float scale, int r, int g, int b);
	void                               showCombatText(const TangibleObject & attackerObject, const Unicode::String &outputTextOOB, float scale, int r, int g, int b);
	virtual void                       handleCMessageTo(const MessageToPayload &message);

	CustomizationData *                fetchCustomizationData   ();
	const CustomizationData *          fetchCustomizationData   () const;

	virtual int                        onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer);
	virtual void                       onContainerGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer);

	virtual bool                       onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer);
	virtual bool                       onContainerChildAboutToLoseItem(ServerObject *destination, ServerObject &item, ServerObject *transferer);
	virtual bool                       onContainerChildAboutToGainItem(ServerObject &item, ServerObject *destination, ServerObject *transferer);
	virtual void                       onContainerChildLostItem(ServerObject *destination, ServerObject &item, ServerObject *source, ServerObject *transferrer);
	virtual void                       onContainerChildGainItem(ServerObject &item, ServerObject *source, ServerObject *transferer);

	// In TangibleObject_PobItemLimits.cpp:
	virtual bool                       onPobChildAboutToLoseItem(ServerObject *destination, ServerObject &item, ServerObject *transferer);
	virtual bool                       onPobChildAboutToGainItem(ServerObject &item, ServerObject *destination, ServerObject *transferer);
	virtual void                       onPobChildLostItem(ServerObject *destination, ServerObject &item, ServerObject *source, ServerObject *transferer);
	virtual void                       onPobChildGainItem(ServerObject &item, ServerObject *source, ServerObject *transferer);
	int                                getPobItemCount() const; // returns the number of items (that count against the item limit) currently in this building
	int								   getPobBaseItemLimit() const; // returns the base number of items this building can hold
	int                                getPobTotalItemLimit(int & base, int & additional) const; // returns the total number of items this building can hold (base + storage increase)
	void                               fixUpPobItemLimit();
	void                               deletePobPersistedContents(CreatureObject const *player, int deleteReason);
	NetworkId const &                  movePobItemToPlayer(CreatureObject const &player, int index, bool overrideOwnership);
	NetworkId const &                  movePobItemToPlayer(CreatureObject const &player, NetworkId const &item, bool overrideOwnership);

	virtual void                       onContainedItemAppearanceDataModified(TangibleObject const & target, std::string const & oldValue, std::string const & value);

	Footprint                         *getFootprint();
	Footprint const                   *getFootprint() const;

	//permissions functions
	bool         isOnAdminList(const CreatureObject& player) const;
	bool         isOnHopperList(const CreatureObject& player) const;

	// NPC conversation functions -----------------------------------------------
	bool                              startNpcConversation          (TangibleObject & npc, const std::string & convoName, NpcConversationData::ConversationStarter starter, uint32 appearanceOverrideSharedTemplateCrc);
	void                              endNpcConversation            ();
	void                              endNpcConversation            (StringId const & stringId, Unicode::String const & oob);
	void                              clearNpcConversation          ();
	void                              sendNpcConversationMessage    (const StringId & stringId, const ProsePackage & pp, const Unicode::String & oob);
	bool                              addNpcConversationResponse    (const StringId & stringId, const ProsePackage & pp);
	bool                              removeNpcConversationResponse (const StringId & stringId, const ProsePackage & pp);
	void                              sendNpcConversationResponses  ();
	void                              respondToNpc                  (int responseIndex);
	bool                              isInNpcConversation           () const;
	void                              addConversation               (const NetworkId & conversant);
	void                              removeConversation            (const NetworkId & conversant);
	const std::vector<NetworkId> & getConversations              () const;
	void                              handlePlayerResponseToNpcConversation (const std::string & conversationName, const NetworkId & player, const StringId & response, const ProsePackage & pp);
	virtual void                      setCacheVersion (const int cacheVersion);

	// bio-link functions
	void                               setBioLink(const NetworkId & playerId);
	void                               setPendingBioLink();
	void                               clearBioLink();
	NetworkId                          getBioLink() const;
	virtual bool                       isBioLinked() const;

	virtual CommandQueue * getCommandQueue() const;

	void commandQueueEnqueue(Command const &command, NetworkId const &targetId, Unicode::String const &params, uint32 sequenceId = 0, bool clearable = true, Command::Priority priority = Command::CP_Default, bool fromServer = false);
	void commandQueueRemove(uint32 sequenceId);
	void commandQueueClear();
	bool commandQueueHasCommandFromGroup(uint32 groupHash) const;
	void commandQueueClearCommandsFromGroup(uint32 groupHash, bool force = false);

	bool hasAutoDeclineDuel() const;

	void handleIncubatorCommit(CreatureObject const & owner, IncubatorCommitMessage const & msg);
	void handleIncubatorCancel(CreatureObject const & owner);

protected:
	virtual void                       endBaselines();
	virtual void                       initializeFirstTimeObject();
	virtual void                       onLoadedFromDatabase();
	virtual void                       virtualOnSetAuthority();
	int                                getCraftingToolPrototypeTime() const;
	void                               getAttributesForCraftingTool (AttributeVector & data) const;
	void                               getAttributesForShipComponent (AttributeVector & data) const;

	virtual void                       updatePlanetServerInternal(bool forceUpdate) const;
	virtual float                      getPvpRegionCheckTime();

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;
	virtual void setInCombat(bool inCombat);

private:

	struct MaxHitPointsCallback
	{
		void modified(TangibleObject &target, int oldValue, int value, bool isLocal) const;
	};
	friend struct MaxHitPointsCallback;
	struct AttackableOverrideCallback
	{
		void modified(TangibleObject &target, bool oldValue, bool value, bool isLocal) const;
	};
	friend struct AttackableOverrideCallback;
	// used by m_appearanceData string
	struct AppearanceDataCallback
	{
		void modified(TangibleObject &target, const std::string &oldValue, const std::string &value, bool isLocal) const;
	};
	friend struct AppearanceDataCallback;
	// used by m_customAppearance string
	struct CustomAppearanceCallback
	{
		void modified(TangibleObject &target, const std::string &oldValue, const std::string &value, bool isLocal) const;
	};
	friend struct CustomAppearanceCallback;

	struct VisibilityDataCallback
	{
		void modified(TangibleObject &target, bool oldValue, bool value, bool isLocal) const;
	};
	friend struct VisibilityDataCallback;

	struct CombatStateChangedCallback
	{
		void modified(TangibleObject & target, bool oldValue, bool newValue, bool local) const;
	};
	friend struct CombatStateChangedCallback;

	struct PassiveRevealPlayerCharacterObserver
	{
		PassiveRevealPlayerCharacterObserver(TangibleObject *tangible, Archive::AutoDeltaObserverOp operation);
		~PassiveRevealPlayerCharacterObserver();

		TangibleObject * m_target;
	};
	friend struct PassiveRevealPlayerCharacterObserver;

public:

	typedef Archive::AutoDeltaVariableObserver<int, TangibleConditionObserver, TangibleObject> ConditionObserverType;
	typedef Archive::AutoDeltaVariableObserver<int, PvpUpdateObserver, TangibleObject> PvpType;
	typedef Archive::AutoDeltaVariableObserver<uint32, PvpUpdateObserver, TangibleObject> PvpFaction;
	typedef Archive::AutoDeltaVariableObserver<uint32, PvpUpdateObserver, TangibleObject> PvpRegionCrc;
	typedef Archive::AutoDeltaVectorObserver<PvpEnemy, PvpEnemiesObserver, TangibleObject> PvpEnemies;

	Pvp::PvpType       getPvpType() const;
	Pvp::PvpType       getPvpMercenaryType() const;
	void               setPvpType(Pvp::PvpType pvpType);
	Pvp::PvpType       getPvpFutureType() const;
	void               setPvpFutureType(Pvp::PvpType pvpType);
	Pvp::FactionId     getPvpFaction() const;
	Pvp::FactionId     getPvpMercenaryFaction() const;
	virtual void       setPvpFaction(Pvp::FactionId factionId);
	void               setPvpMercenaryFaction(Pvp::FactionId factionId, Pvp::PvpType pvpType);
	Region const *     getPvpRegion() const;
	void               setPvpRegion(Region const *pvpRegion);
	PvpEnemies const & getPvpEnemies() const;
	PvpEnemies &       getPvpEnemies();
	virtual TangibleObject const &getPvpViewer(Client const &client) const;

	bool isNonPvpObject() const;

	virtual bool wantSawAttackTriggers() const;
	void setWantSawAttackTriggers(bool enable);

	int getCombatDuration() const;

	void setAttackableOverride(bool attackable);

	int getPassiveRevealRange(NetworkId const & target) const;
	void addPassiveReveal(TangibleObject const & target, int range);
	void addPassiveReveal(NetworkId const & target, int range, bool isTargetPlayerCharacter);
	void removePassiveReveal(NetworkId const & target);
	void removeAllPassiveReveal();

	std::map<NetworkId, int> const & getPassiveReveal() const;
	std::set<NetworkId> const & getPassiveRevealPlayerCharacter() const;

	bool setOverrideMapColor(unsigned char r, unsigned char g, unsigned char b);
	bool clearOverrideMapColor();
	bool getOverrideMapColor(unsigned char & r, unsigned char & g, unsigned char & b) const;

	bool isLocked() const;
	void addUserToAccessList(NetworkId const user);
	void removeUserFromAccessList(NetworkId const user);
	bool isUserOnAccessList(NetworkId const user) const;
	void clearUserAccessList();

	void addGuildToAccessList(int guildId);
	void removeGuildFromAccessList(int guildId);
	bool isGuildOnAccessList(int guildId) const;
	void clearGuildAccessList();

	void getUserAccessList(std::vector<NetworkId> & ids);
	void getGuildAccessList(std::vector<int> & ids);

	void addObjectEffect(std::string const & filename, std::string const & hardpoint, Vector const & offset, float scale, std::string const & label);
	void removeObjectEffect(std::string const & label);
	void removeAllObjectEffects();
	bool hasObjectEffect(std::string const & label);

protected:
	ServerSynchronizedUi * createSynchronizedUi ();
	virtual void calcPvpableState();
	bool isCustomizationDataModified() const;
	void setCustomizationDataModified(bool modified);
	void setPvpable(bool pvpable);

private:

	static void customizationDataModificationCallback(const CustomizationData &customizationData, const void *context);

private:

	TangibleObject();
	TangibleObject(const TangibleObject& rhs);
	TangibleObject&	operator=(const TangibleObject& rhs);

	void handleContainerSetup();
	void onArrivedAtLocation(const LocationData & location);
	void customAppearanceModified(const std::string & value);
	void appearanceDataModified(const std::string & value);
	void visibilityDataModified();
	int  alterHitPoints(int delta, bool ignoreInvulnerable, const NetworkId & source = NetworkId::cms_invalid);
	float getNextAlterTime(float baseAlterTime) const;
	const char * const getDamageTypeString(ServerArmorTemplate::DamageType damage) const;

	void copyUserAccessListToObjVars();
	void readInUserAccessListObjVars();

	void copyGuildAccessListToObjVars();
	void readInGuildAccessListObjVars();

private:

	class CraftingToolSyncUi;
	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

	// @todo: what happens to m_combatData if this object is migrated?
	mutable CombatEngineData::CombatData * m_combatData;	///< info needed by the combat engine if this object is involved in combat

// BPM TangibleObject // Begin persisted members.
	PvpType                        m_pvpType;
	PvpType                        m_pvpMercenaryType;
	PvpType                        m_pvpFutureType;
	PvpFaction                     m_pvpFaction;
	PvpFaction                     m_pvpMercenaryFaction;
	PvpRegionCrc                   m_pvpRegionCrc;
	PvpEnemies                     m_pvpEnemies;
	Archive::AutoDeltaVariable<int>                m_damageTaken;			///< Amount of damage this object has taken
	Archive::AutoDeltaVariableCallback<int, MaxHitPointsCallback, TangibleObject> m_maxHitPoints;			///< The number of hitpoints this object has.
	Archive::AutoDeltaVariableObserver<NetworkId, CellPermissions::UpdateObserver, TangibleObject> m_ownerId;

	Archive::AutoDeltaVariableCallback<std::string, CustomAppearanceCallback, TangibleObject> m_customAppearance;
	Archive::AutoDeltaVariableCallback<std::string, AppearanceDataCallback, TangibleObject> m_appearanceData;

	Archive::AutoDeltaVector<LocationData>  m_locationTargets;

	Archive::AutoDeltaSet<int>              m_components;      // component table ids of visible components attached to this object

// EPM

	Archive::AutoDeltaVariableCallback<bool, VisibilityDataCallback, TangibleObject> m_visible;       // flag that the object is visible
	Archive::AutoDeltaVariableCallback<bool, VisibilityDataCallback, TangibleObject> m_hideFromClient;// flag that the object is visible only to the owner of the object
	Archive::AutoDeltaVariable<int>         m_count;         // generic counter
	ConditionObserverType                   m_condition;     // object condition bit flags
	Archive::AutoDeltaVariable<NetworkId>   m_creatorId;
	Archive::AutoDeltaVariable<uint32>      m_sourceDraftSchematic;

	typedef Archive::AutoDeltaVector<CachedNetworkId, TangibleObject> TargetIdList;

	HateList m_hateList;

	// for applying "hate over time"
	Archive::AutoDeltaMap<NetworkId, std::pair<float, std::pair<unsigned long, unsigned long> > > m_hateOverTime;

	Archive::AutoDeltaVariableCallback<bool, CombatStateChangedCallback, TangibleObject> m_inCombat;
	Archive::AutoDeltaVariable<time_t> m_combatStartTime;

	// allows script to make an object (i.e. like a turret) attackable
	Archive::AutoDeltaVariableCallback<bool, AttackableOverrideCallback, TangibleObject> m_attackableOverride;

	// tracks objects that have passively revealed this object
	Archive::AutoDeltaMap<NetworkId, int> m_passiveReveal;

	// tracks player character objects that have passively revealed this object
	// m_passiveRevealPlayerCharacter is a subset of m_passiveReveal
	Archive::AutoDeltaSetObserver<NetworkId, PassiveRevealPlayerCharacterObserver, TangibleObject> m_passiveRevealPlayerCharacter;

	// allows script to override the color of an object as it appears on maps, etc.
	Archive::AutoDeltaVariable<uint32> m_mapColorOverride;

	// Access list of users if the item is condition locked.
	Archive::AutoDeltaSet<NetworkId> m_accessList;

	// Access list of guilds that can access the item if it's locked.
	Archive::AutoDeltaSet<int>       m_guildAccessList;

	// List of object effects.
	Archive::AutoDeltaMap<std::string, std::pair<std::string, std::pair<std::string, std::pair<Vector, float> > > > m_effectsMap;

protected:
	// NPC conversation data
	NpcConversation *                                m_npcConversation;
	Archive::AutoDeltaVector<NetworkId>              m_conversations; ///< Ids of everyone this creature is in conversation with (for NPC conversations)

///@ todo define destroy permissions and equipment slot type list and resource list.
//  destroyPermissions;					///< The permissions list of who is allowed to destroy an item.
//	ResourceList resourceList;			///< List of resources that make up this item.

};

// ======================================================================

inline void TangibleObject::CustomAppearanceCallback::modified(TangibleObject &target, const std::string &oldValue, const std::string &value, bool isLocal) const
{
	UNREF(oldValue);
	UNREF(isLocal);
	target.customAppearanceModified(value);
}

inline void TangibleObject::VisibilityDataCallback::modified(TangibleObject &target, bool oldValue, bool newValue, bool isLocal) const
{
	UNREF(isLocal);
	if (oldValue != newValue)
		target.visibilityDataModified();
}

// ======================================================================

inline TangibleObject::PassiveRevealPlayerCharacterObserver::PassiveRevealPlayerCharacterObserver(TangibleObject *tangible, Archive::AutoDeltaObserverOp)
: m_target(tangible)
{
}

inline TangibleObject::PassiveRevealPlayerCharacterObserver::~PassiveRevealPlayerCharacterObserver()
{
	if (m_target)
		m_target->visibilityDataModified();
}

// ======================================================================

inline const char * TangibleObject::getCustomAppearance() const
{
	if (!m_customAppearance.get().empty())
		return m_customAppearance.get().c_str();
	return getSharedTemplateName();
}

inline void TangibleObject::setCustomAppearance(const std::string & newCustomAppearance)
{
	m_customAppearance = newCustomAppearance;
}

inline const std::string & TangibleObject::getAppearanceData() const
{
	return m_appearanceData.get();
}

inline void TangibleObject::setAppearanceData(const std::string & newAppearanceData)
{
	m_appearanceData = newAppearanceData;
}

inline bool TangibleObject::isInCombat(void) const
{
	return m_inCombat.get();
}

inline Pvp::PvpType TangibleObject::getPvpType() const
{
	return static_cast<Pvp::PvpType>(m_pvpType.get());
}

inline Pvp::PvpType TangibleObject::getPvpMercenaryType() const
{
	return static_cast<Pvp::PvpType>(m_pvpMercenaryType.get());
}

inline Pvp::PvpType TangibleObject::getPvpFutureType() const
{
	return static_cast<Pvp::PvpType>(m_pvpFutureType.get());
}

inline Pvp::FactionId TangibleObject::getPvpFaction() const
{
	return static_cast<Pvp::FactionId>(m_pvpFaction.get());
}

inline Pvp::FactionId TangibleObject::getPvpMercenaryFaction() const
{
	return static_cast<Pvp::FactionId>(m_pvpMercenaryFaction.get());
}

inline TangibleObject::PvpEnemies const &TangibleObject::getPvpEnemies() const
{
	return m_pvpEnemies;
}

inline TangibleObject::PvpEnemies &TangibleObject::getPvpEnemies()
{
	return m_pvpEnemies;
}

inline int TangibleObject::getDamageTaken() const
{
	return m_damageTaken.get();
}

inline int TangibleObject::getMaxHitPoints() const
{
	return m_maxHitPoints.get();
}

inline void TangibleObject::setMaxHitPoints(int maxHitPoints)
{
	m_maxHitPoints = maxHitPoints;
}

inline int TangibleObject::getCount(void) const
{
	return m_count.get();
}

inline void TangibleObject::setCount(int value)
{
	m_count = value;
}

inline void TangibleObject::incrementCount(int delta)
{
	m_count = m_count.get() + delta;
}

inline int TangibleObject::getCondition(void) const
{
	return m_condition.get();
}

inline bool TangibleObject::hasCondition(int condition) const
{
	return (m_condition.get() & condition) != 0;
}

inline bool TangibleObject::isNonPvpObject() const
{
	return !getLocalFlag(LocalObjectFlags::TangibleObject_Pvpable);
}

//-----------------------------------------------------------------------

inline bool TangibleObject::isInNpcConversation () const
{
	return !m_conversations.empty();
}

//-----------------------------------------------------------------------

inline const std::vector<NetworkId> & TangibleObject::getConversations () const
{
	return m_conversations.get ();
}

//----------------------------------------------------------------------

inline uint32 TangibleObject::getSourceDraftSchematic() const
{
	return m_sourceDraftSchematic.get();
}

//----------------------------------------------------------------------

inline void TangibleObject::createCombatData()
{
	if (m_combatData == nullptr)
	{
		m_combatData = new CombatEngineData::CombatData;
	}
}

//----------------------------------------------------------------------

inline bool TangibleObject::isVisible() const
{
	return m_visible.get();
}

//----------------------------------------------------------------------

inline bool TangibleObject::isHidden() const
{
	return m_hideFromClient.get();
}

//----------------------------------------------------------------------

inline std::map<NetworkId, int> const & TangibleObject::getPassiveReveal() const
{
	return m_passiveReveal.getMap();
}

//----------------------------------------------------------------------

inline std::set<NetworkId> const & TangibleObject::getPassiveRevealPlayerCharacter() const
{
	return m_passiveRevealPlayerCharacter.get();
}

//----------------------------------------------------------------------

inline std::map<NetworkId, std::pair<float, std::pair<unsigned long, unsigned long> > > const & TangibleObject::getHateOverTime() const
{
	return m_hateOverTime.getMap();
}

//----------------------------------------------------------------------

inline bool TangibleObject::isHateListOwnerPlayer() const
{
	return m_hateList.isOwnerPlayer();
}
//----------------------------------------------------------------------

inline std::vector<LocationData> const & TangibleObject::getLocationTargets() const
{
	return m_locationTargets.get();
}

//----------------------------------------------------------------------

inline bool TangibleObject::isLocked() const
{
	return hasCondition(C_locked);
}

//----------------------------------------------------------------------

#endif	// INCLUDED_TangibleObject_H

