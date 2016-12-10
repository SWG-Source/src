// ======================================================================
//
// PlayerObject.h
//
// Copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef	_PLAYER_OBJECT_H
#define	_PLAYER_OBJECT_H

// ======================================================================

#include "Archive/AutoDeltaMap.h"
#include "Archive/AutoDeltaPackedMap.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/IntangibleObject.h"
#include "serverGame/Region.h"
#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/StationId.h"
#include "sharedFoundation/Watcher.h"
#include "sharedGame/CollectionsDataTable.h"
#include "sharedGame/CraftingData.h"
#include "sharedGame/MatchMakingId.h"
#include "sharedGame/PlayerData.h"
#include "sharedGame/PlayerQuestData.h"
#include "sharedGame/Waypoint.h"
#include "sharedMath/Quaternion.h"
#include "sharedNetworkMessages/MessageQueueCraftExperiment.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedSkillSystem/SkillObject.h"

#include <set>

// ======================================================================

struct ChatAvatarId;
class MessageQueueDraftSlots;
class MessageQueueDraftSlotsQueryResponse;
class ServerPlayerObjectTemplate;
class TangibleObject;

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

/**
 * Class to hold all the player and account-level data.  A character should
 * keep this object, even if the creature/vehicle/etc. they control changes.
 */
class PlayerObject : public IntangibleObject
{
public:

	struct ChatLogEntry
	{
		ChatLogEntry();

		int m_index;
		time_t m_time;
	};

	typedef std::list<ChatLogEntry>    ChatLog;
	typedef std::vector<std::string>   StringVector;
	typedef std::map<std::string, int> ExperiencePointMap;
	typedef std::map<NetworkId, Waypoint> WaypointMap;
	typedef Archive::AutoDeltaPackedMap<uint32, PlayerQuestData> PlayerQuestDataMap;

	explicit    PlayerObject(const ServerPlayerObjectTemplate* newTemplate);
	virtual     ~PlayerObject();

	virtual PlayerObject *asPlayerObject();
	virtual PlayerObject const *asPlayerObject() const;

	static void removeDefaultTemplate(void);
	static std::set<PlayerObject const *> const &getAllPlayerObjects();

	void               addMembersToPackages  ();
	Waypoint const     createWaypoint        (const Location & location, bool ignoreLimits);
	Waypoint const     createOrUpdateReusableWaypoint(const Location & location, const std::string & objvarTrackingName, const Unicode::String & waypointNameIfCreated, int waypointColorIfCreated = -1);
	Waypoint &         getWaypoint           (const NetworkId & waypointId);
	void               getWaypointsInDatapad (std::vector<NetworkId> & target);
	void               getWaypointAttributes (const NetworkId & waypointId, ServerObject::AttributeVector & data);
	void               destroyWaypoint       (const NetworkId & waypoint);
	WaypointMap const &getWaypoints          () const;
	void               setGroupWaypoints     (WaypointMap const &groupWaypointMap);

	virtual void  getAttributes        (std::vector<std::pair<std::string, Unicode::String> > &data) const;
	virtual void  endBaselines();
	virtual void  onLoadedFromDatabase();
	static std::string  getAccountDescription(const ServerObject *);
	static std::string  getAccountDescription(const NetworkId &);
	       std::string  getAccountDescription() const;

public:
	StationId            getStationId() const;
	float                getCheaterLevel() const;
	int                  getBornDate() const;
	int                  getAgeInDays() const;
	static int           getCurrentBornDate();
	uint32               getPlayedTime() const;
	float                getPlayedTimeAccumOnly() const;
	void                 setPlayedTimeAccumOnly(float playedTimeAccum);

	unsigned long        getSessionPlayTimeDuration() const;
	unsigned long        getSessionActivePlayTimeDuration() const;
	int32                getSessionStartPlayTime() const;
	int32                getSessionLastActiveTime() const;
	void                 setSessionPlayTimeInfo(int32 sessionStartPlayTime, int32 sessionLastActiveTime, unsigned long sessionActivePlayTimeDuration);

	void                 setStationId(StationId account);
	void                 setCheaterLevel(float level);
	void                 setBornDate();
	void                 adjustBornDate(int adjustment);
	void                 alterPlayedTime(float time);

	void                 setMatchMakingPersonalProfileId(MatchMakingId const &id);
	MatchMakingId const &getMatchMakingPersonalProfileId() const;

	void                 setMatchMakingCharacterProfileId(MatchMakingId const &id);
	MatchMakingId const &getMatchMakingCharacterProfileId() const;

	void                 toggleSearchableByCtsSourceGalaxy();
	void                 toggleDisplayLocationInSearchResults();
	void                 toggleAnonymous();
	void                 toggleHelper();
	void                 toggleRolePlay();
	void                 toggleOutOfCharacter();
	void				 toggleLookingForWork();


	bool                 isAwayFromKeyBoard() const;
	void                 toggleAwayFromKeyBoard();

	void                 toggleDisplayingFactionRank();

	bool                 isLookingForGroup() const;
	void                 setLookingForGroup(bool const lookingForGroup);
	void                 toggleLookingForGroup();

	StringVector const & getSortedLowercaseFriendList() const;
	void                 setFriendList(const StringVector &friendList);
	bool                 isFriend(const std::string &name) const;
	void                 requestFriendList() const;
	void                 findFriend(const std::string & name);
	void                 replyLocationToFriend(const std::string & friendName, const NetworkId & friendId);
	void                 receiveLocationToFriend(const NetworkId & friendId, const Location & friendLocation);
	void                 failLocationToFriend(const NetworkId & friendId);
	void                 receiveFailLocationToFriend(const NetworkId & friendId);
	
	StringVector const & getSortedLowercaseIgnoreList() const;
	void                 setIgnoreList(const StringVector &ignoreList);
	bool                 isIgnoring(const std::string &name) const;
	void                 requestIgnoreList() const;

	void                 setTitle(std::string const &title);
	std::string const &  getTitle() const;

	bool                 adjustLotCount(int adjustment);

	void                 setSpokenLanguage(int const languageId);
	int                  getSpokenLanguage() const;

	void                 setLinkDead(bool const linkDead);
	bool                 getLinkDead() const;

	static bool          isIgnoring(NetworkId const &target, NetworkId const &source);

	int                  getMaxForcePower         (void) const;
	virtual void         setMaxForcePower         (int value);
	int                  getForcePower            (void) const;
	void                 setForcePower            (int value);
	float                getForcePowerRegenRate   (void) const;
	void                 setForcePowerRegenRate   (float value);
	void                 regenerateForcePower     (float time);

	//@todo It is unfortunate that these jedi-related things are in the supposedly generic PlayerObject
    virtual bool         isJedi() const;			
	void                 addJediToAccount() const;
	void                 addJediToAccountAck() const;
	void                 makeSpectralOnAccount() const;
	void                 makeSpectralOnAccountAck() const;

	void                 checkStomach();

	virtual void         handleCMessageTo(const MessageToPayload &message);

	void                  logChat(int const logIndex);

	ChatLog const &getChatLog() const;
	void cleanChatLog();

	void checkTheater(const Vector & pos, const std::string & scene, float speed, float updateRadius);
	bool setTheater(const std::string & datatable, const Vector & position, const std::string & scene, const std::string & script, const NetworkId & creator, const std::string & name, IntangibleObject::TheaterLocationType locationType);
	void clearTheater();
	bool hasTheater() const;
	const NetworkId & getTheater() const;

public:

	int                                 getExperiencePoints(const std::string & experienceType) const;
	const std::map<std::string, int> &  getExperiencePoints() const;
	int                                 getExperienceLimit(const std::string & experienceType) const;
	int                                 grantExperiencePoints(const std::string & experienceType, int amount);
	void                                clearSchematics();
	bool                                grantSchematicGroup(const std::string & groupNameWithModifier, bool fromSkill);
	bool                                grantSchematic(uint32 schematicCrc, bool fromSkill);
	bool                                revokeSchematic(uint32 schematicCrc, bool fromSkill);
	bool                                hasSchematic(uint32 schematicCrc);

	// crafting functions
	bool                                isCrafting(void) const;
	const std::map<std::pair<uint32, uint32>,int> & getDraftSchematics () const;
	const CachedNetworkId &             getCraftingTool () const;
	void                                setCraftingTool(const TangibleObject & tool);
	const CachedNetworkId &             getCraftingStation () const;
	void                                setCraftingStation(const TangibleObject * station);
	const DraftSchematicObject *        getCurrentDraftSchematic () const;
	bool                                requestCraftingSession(const NetworkId & toolId);
	void                                sendUseableDraftSchematics(std::vector<uint32> & schematicNames);
	bool                                requestDraftSlots(uint32 draftSchematicCrc, MessageQueueDraftSlots * message, MessageQueueDraftSlotsQueryResponse * queryMessage);
	void                                selectDraftSchematic(int index);
	Crafting::CraftingError             fillSlot(int slotIndex, int slotOptionIndex, const NetworkId & ingredientIndex);
	Crafting::CraftingError             emptySlot(int slotIndex, const NetworkId & targetContainer);
	int                                 goToNextCraftingStage ();
	int                                 startCraftingExperiment ();
	Crafting::CraftingResult            experiment(const std::vector<MessageQueueCraftExperiment::ExperimentInfo> & experiments, int totalPoints, int corelevel);
	bool                                customize(int property, int value) const;
	int                                 setCustomizationData(const Unicode::String & name, int appearanceIndex, const std::vector<Crafting::CustomValue> & customizations, int itemCount) const;
	bool                                createPrototype(bool keepPrototype);
	bool                                createManufacturingSchematic(void);
	bool                                restartCrafting ();
	void                                stopCrafting (bool normalExit);
	enum Crafting::CraftingStage        getCraftingStage () const;
	void                                setCraftingLevel(int level);
	int                                 getCraftingLevel() const;
	void                                setAllowEmptySlot(bool allow);

	int                                 getAccountNumLots() const;
	void                                setAccountNumLotsOverLimitSpam();

	CreatureObject *                    getCreatureObject();
	const CreatureObject *              getCreatureObject() const;

	void                                setPriviledgedTitle(int8 priviledgedTitle);

	bool                                getIsUnsticking() const;
	void                                setIsUnsticking(bool value);

	//-- old quest system
	bool                                isQuestComplete(int questId) const;
	bool                                isQuestActive(int questId) const;
	void                                activateQuest(int questId);
	void                                deactivateQuest(int questId);
	void                                completeQuest(int questId);
	void                                clearCompletedQuest(int questId);

	//--new quest system
	enum QuestResult {QR_success, QR_alreadyActive, QR_noSuchQuest, QR_noSuchTask, QR_alreadyComplete, QR_failedPrerequisites, QR_failedExclusion, QR_questNotActive, QR_taskNotActive, QR_noSuchPlayer, QR_questNotCompleted, QR_alreadyReceivedReward};

	QuestResult                         questCanActivateQuest(uint32 questCrc) const;
	QuestResult                         questActivateQuest(uint32 questCrc, NetworkId const & questGiver);
	QuestResult                         questCompleteQuest(uint32 questCrc);
	QuestResult                         questGrantQuestReward(uint32 questCrc, std::string const & selectedReward);
	void                                questClearQuest(uint32 questCrc);
	QuestResult                         questCanActivateQuestTask(uint32 questCrc, int taskId) const;
	QuestResult                         questActivateQuestTask(uint32 questCrc, int taskId);
	QuestResult                         questCompleteQuestTask(uint32 questCrc, int taskId);
	QuestResult                         questFailQuestTask(uint32 questCrc, int taskId);
	QuestResult                         questClearQuestTask(uint32 questCrc, int taskId);
	bool                                questHasActiveQuest(uint32 questCrc) const;
	bool                                questHasCompletedQuest(uint32 questCrc) const;
	bool                                questHasActiveQuestTask(uint32 questCrc, int taskId) const;
	bool                                questHasCompletedQuestTask(uint32 questCrc, int taskId) const;
	uint32                              questGetQuestStatus(uint32 const questCrc) const;
	std::vector<int>                 questGetAllActiveQuestIds() const;
	std::vector<int>                 questGetAllCompletedQuestIds() const;
	bool                                questPlayerCanClaimRewardFor(uint32 questCrc);

	uint32                              getCurrentQuest() const;
	void                                setCurrentQuest(uint32 questCrc);
	
	virtual void                         virtualOnLogout();
	virtual void                         getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const;
	virtual void                         setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source);
	
	void                                setRoleIconChoice(int roleIconChoice);
	int                                 getRoleIconChoice() const;
	void setAggroImmuneDuration(int const duration);
	bool isAggroImmune() const;

	std::string const & getSkillTemplate() const;
	bool setSkillTemplate(std::string const & templateName, bool clientRequest);

	std::string const & getWorkingSkill() const;
	bool setWorkingSkill(std::string const & skillName, bool clientRequest);

	bool getIsFromLogin(void) const;
	void setIsFromLogin(bool val); 

	bool isNormalPlayer() const;
	bool isCustomerServiceRepresentative() const;
	bool isDeveloper() const;
	bool isWarden() const;

	void setPendingRequestQuestInformation(uint32 questCrc, NetworkId const & questGiver);
	uint32 getPendingRequestQuestCrc() const;
	NetworkId const & getPendingRequestQuestGiver() const;

	int getNumberOfActiveQuests() const;
	void setPlayerQuestDataComplete(uint32 questCrc, PlayerQuestData & playerQuestData);
	void pickNewCurrentQuest();

	int32 getCurrentGcwPoints() const;
	int32 getCurrentGcwRating() const;
	int32 getCurrentPvpKills() const;
	int64 getLifetimeGcwPoints() const;
	int32 getMaxGcwImperialRating() const;
	int32 getMaxGcwRebelRating() const;
	int32 getLifetimePvpKills() const;
	int32 getNextGcwRatingCalcTime() const;
	int getCurrentGcwRank() const;
	float getCurrentGcwRankProgress() const;
	int getMaxGcwImperialRank() const;
	int getMaxGcwRebelRank() const;
	int32 getGcwRatingActualCalcTime() const;
	bool needsGcwRatingRecalculated() const;

	void modifyCurrentGcwPoints(int count, bool modifyLifetimeValue);
	void modifyCurrentGcwRating(int count, bool modifyMaxValue);
	void modifyCurrentPvpKills(int count, bool modifyLifetimeValue);
	void modifyLifetimeGcwPoints(int count);
	void modifyMaxGcwImperialRating(int count);
	void modifyMaxGcwRebelRating(int count);
	void modifyLifetimePvpKills(int count);
	void modifyNextGcwRatingCalcTime(int weekCount);
	void setNextGcwRatingCalcTime(bool alwaysSendMessageToForRecalc);
	void ctsUseOnlySetGcwInfo(int32 currentGcwPoints, int32 currentGcwRating, int32 currentPvpKills, int64 lifetimeGcwPoints, int32 maxGcwImperialRating, int32 maxGcwRebelRating, int32 lifetimePvpKills, int32 nextGcwRatingCalcTime);

	void addToPlayerHateList(NetworkId const & id);
	void removeFromPlayerHateList(NetworkId const & id);
	void clearPlayerHateList();

	uint32 getSessionActivity() const;
	void clearSessionActivity();
	void addSessionActivity(uint32 activity);
	int32 getKillMeter() const;
	void incrementKillMeter(int amount);
	void updateKillMeter(float deltaTime);

	NetworkId const &getPetId() const;
	void setPetId(NetworkId const & newId);
	void setPetCommandList(std::vector<std::string> const &commands);
	void setPetToggledCommands(std::vector<std::string> const &commands);

	bool modifyCollectionSlotValue(std::string const & slotName, int64 delta);

	bool getCollectionSlotValue(std::string const & slotName, unsigned long & value) const;
	bool getCollectionSlotValue(CollectionsDataTable::CollectionInfoSlot const & slotInfo, unsigned long & value) const;

	bool hasCompletedCollectionSlotPrereq(std::string const & slotName, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo = nullptr) const;
	bool hasCompletedCollectionSlotPrereq(CollectionsDataTable::CollectionInfoSlot const & slotInfo, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo = nullptr) const;

	bool hasCompletedCollectionSlot(std::string const & slotName) const;
	bool hasCompletedCollectionSlot(CollectionsDataTable::CollectionInfoSlot const & slotInfo) const;
	static bool hasCompletedCollectionSlot(CollectionsDataTable::CollectionInfoSlot const & slotInfo, unsigned long slotValue);

	bool hasCompletedCollection(std::string const & collectionName) const;

	bool hasCompletedCollectionPage(std::string const & pageName) const;

	bool hasCompletedCollectionBook(std::string const & bookName) const;

	int getCompletedCollectionSlotCountInCollection(std::string const & collectionName, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo = nullptr) const;

	int getCompletedCollectionSlotCountInPage(std::string const & pageName, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo = nullptr) const;
	int getCompletedCollectionCountInPage(std::string const & pageName, std::vector<CollectionsDataTable::CollectionInfoCollection const *> * collectionInfo = nullptr) const;

	int getCompletedCollectionSlotCountInBook(std::string const & bookName, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo = nullptr) const;
	int getCompletedCollectionCountInBook(std::string const & bookName, std::vector<CollectionsDataTable::CollectionInfoCollection const *> * collectionInfo = nullptr) const;
	int getCompletedCollectionPageCountInBook(std::string const & bookName, std::vector<CollectionsDataTable::CollectionInfoPage const *> * collectionInfo = nullptr) const;
	int getCompletedCollectionBookCount(std::vector<CollectionsDataTable::CollectionInfoBook const *> * collectionInfo = nullptr) const;

	void migrateLegacyBadgesToCollection(std::vector<int> const & badges);

	// chat spam info
	int getChatSpamSpatialNumCharacters() const;
	void updateChatSpamSpatialNumCharacters(NetworkId const & character, int spatialNumCharacters);

	int getChatSpamNonSpatialNumCharacters() const;
	void handleChatStatisticsFromChatServer(NetworkId const & character, int chatSpamTimeEndInterval, int spatialNumCharacters, int nonSpatialNumCharacters);

	int getChatSpamTimeEndInterval() const;

	time_t getChatSpamNextTimeToNotifyPlayerWhenLimited() const;
	void setChatSpamNextTimeToNotifyPlayerWhenLimited(time_t chatSpamNextTimeToNotifyPlayerWhenLimited);

	// citizenship info
	void updateCitizenshipInfo();

	// GCW Region info
	void updateGcwRegionInfo();
	std::string const & getCurrentGcwRegion() const;

	void updateGcwDefenderRegionInfo();
	std::pair<std::string, std::pair<bool, bool> > const & getCityGcwDefenderRegionInfo() const;
	std::pair<std::string, std::pair<bool, bool> > const & getGuildGcwDefenderRegionInfo() const;

	// squelch info
	void squelch(NetworkId const & squelchedById, std::string const & squelchedByName, int squelchDurationSeconds);
	void unsquelch();
	NetworkId const & getSquelchedById() const;
	int getSecondsUntilUnsquelched(); // returns 0 if not currently squelched; returns < 0 if indefinitely squelched

	void modifyShowBackpack(bool b);
	void modifyShowHelmet(bool b);

	bool getShowBackpack() const;
	bool getShowHelmet() const;

	void setGuildRank(BitArray const & guildRank);
	BitArray const & getGuildRank() const;

	void setCitizenRank(BitArray const & citizenRank);
	BitArray const & getCitizenRank() const;

	EnvironmentInfo getEnvironmentFlags() const;
	void overrideDefaultAttack(std::string const & attack);
	std::string const & getDefaultAttackOverride() const;

	void setEnvironmentFlags(EnvironmentInfo const & flags);

	// getter/setter for variables to track the status of the
	// /pickupAllRoomItemsIntoInventory and
	// /dropAllInventoryItemsIntoRoom commands
	int getPickupDropAllItemsOperationId() const;
	void setPickupDropAllItemsOperationId(int pickupDropAllItemsOperationId);

	NetworkId const & getPickupDropAllItemsCellId() const;
	void setPickupDropAllItemsCellId(NetworkId const & pickupDropAllItemsCellId);

	bool getPickupAllItemsAllowInventoryOverload() const;
	void setPickupAllItemsAllowInventoryOverload(bool pickupAllItemsAllowInventoryOverload);

	int getPickupDropAllItemsNumItemsSoFar() const;
	void setPickupDropAllItemsNumItemsSoFar(int pickupDropAllItemsNumItemsSoFar);

	std::set<NetworkId> & getPickupDropAllItemsProcessedItemsSoFar();

	// getter/setter for variables to track the status of a restore decoration layout operation
	int getRestoreDecorationOperationId() const;
	void setRestoreDecorationOperationId(int restoreDecorationOperationId);

	NetworkId const & getRestoreDecorationPobId() const;
	void setRestoreDecorationPobId(NetworkId const & restoreDecorationPobId);

	NetworkId const & getRestoreDecorationObjectBeingRestored() const;
	void setRestoreDecorationObjectBeingRestored(NetworkId const & restoreDecorationObjectBeingRestored);

	int getRestoreDecorationTotalCount() const;
	void setRestoreDecorationTotalCount(int restoreDecorationTotalCount);

	int getRestoreDecorationAttemptedCount() const;
	void setRestoreDecorationAttemptedCount(int restoreDecorationAttemptedCount);

	int getRestoreDecorationSuccessCount() const;
	void setRestoreDecorationSuccessCount(int restoreDecorationSuccessCount);

	std::list<std::pair<std::pair<CachedNetworkId, CachedNetworkId>, std::pair<Vector, Quaternion> > > & getRestoreDecorationLayoutList();

	static bool canAccessGalacticReserve(real x, real z);
	void depositToGalacticReserve(bool ignoreCooldownAndLocationCheck);
	void withdrawFromGalacticReserve();
	void depersistGalacticReserveDeposit();
	int8 getGalacticReserveDeposit() const;

protected:

	virtual const SharedObjectTemplate * getDefaultSharedTemplate(void) const;
	virtual void                         virtualOnSetAuthority();

private:
	int   getForcePowerRegen(float time);
	void  onWaypointChanged(const Waypoint & newValue);
	void  onWaypointCreated(const NetworkId &, const Waypoint &);
	void  onWaypointSet(NetworkId const &, Waypoint const &, Waypoint const &);
	void  onWaypointRemoved(NetworkId const &, const Waypoint &);

	void  questCheckQuestLimits();

	void  handleRecalculateGcwRating();
	void  sendRecalculateGcwRatingMessageTo(int delay);

private:
	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

	MessageDispatch::Callback *  m_callback;

	Archive::AutoDeltaVariable<StationId> m_stationId;

	// ***DEPRECATED DO NOT USE*** 
	// see CreatureObject::getHouse() and CreatureObject::setHouse() for replacement
	Archive::AutoDeltaVariable<CachedNetworkId> m_houseId; ///< The player's domicile

	Archive::AutoDeltaVariable<int> m_accountNumLots;   ///< Number of lots used by all the characters on this account (on this server cluster only)
	Archive::AutoDeltaVariable<int> m_accountNumLotsOverLimitSpam; /// controls whether the player should be spammed for exceeding the lot limit
	Archive::AutoDeltaVariable<int> m_accountMaxLotsAdjustment;   ///< Extra lots available to this account (or possibly penalty lots taken away)
	Archive::AutoDeltaVariable<bool>  m_accountIsOutcast;  ///< Is this account outcast?
	Archive::AutoDeltaVariable<float> m_accountCheaterLevel; ///< how suspected of cheating this person is, for use with movement validation

	PlayerObject(const PlayerObject &);
	PlayerObject & operator = ( const PlayerObject & );

	// player abilities
	Archive::AutoDeltaMap<std::pair<uint32, uint32>, int> m_draftSchematics;    // draft schematics the player may use
	Archive::AutoDeltaMap<std::string, int>              m_experiencePoints;   // xp name->amount map
	Archive::AutoDeltaVariable<int>                      m_expModified;        // just a flag that we increment any time the creature is granted new xp
	Archive::AutoDeltaVariable<int>                      m_maxForcePower;      ///< Maximum force power the player can have
	Archive::AutoDeltaVariable<int>                      m_forcePower;         ///< Current force power the player has
	Archive::AutoDeltaVariable<float>                    m_forceRegenRate;     ///< Rate at which the force power regenerates (units/sec)
	Archive::AutoDeltaVariable<float>                    m_forceRegenValue;    ///< Amount of force power that's regenerated

	// crafting system data
	Archive::AutoDeltaVariable<int>                  m_craftingLevel;           // crafting level of the current crafting session
	Archive::AutoDeltaVariable<int>                  m_experimentPoints;        // experiment points available to the player
	Archive::AutoDeltaVariable<int>                  m_craftingStage;           // what stage in the crafting process a player is in
	Archive::AutoDeltaVariable<CachedNetworkId>      m_craftingTool;         // tool a player is crafting with
	Archive::AutoDeltaVariable<CachedNetworkId>      m_craftingStation;         // station a player is crafting with
	Archive::AutoDeltaVariable<NetworkId>            m_craftingComponentBioLink;// bio-link id of a component
	std::vector<uint32>                           m_useableDraftSchematics;  // draft schematics a player may currently select from
	ConstWatcher<DraftSchematicObject>               m_draftSchematic;          // draft schematic the player is crafting with
	static bool                                      m_allowEmptySlot;          // flag to allow emptySlot() to be called when not in the crafting assembly stage.

	Archive::AutoDeltaVariable<MatchMakingId> m_matchMakingPersonalProfileId;    // What the player defines their personality as 
	Archive::AutoDeltaVariable<MatchMakingId> m_matchMakingCharacterProfileId;   // What the player defines their character as
	
	// these 2 lists ***MUST BE LOWERCASE AND MUST BE KEPT SORTED FOR QUICK LOOKUP***
	Archive::AutoDeltaVector<std::string>                     m_friendList;
	Archive::AutoDeltaVector<std::string>                     m_ignoreList;
	
	Archive::AutoDeltaVariable<std::string>                   m_skillTitle;
	Archive::AutoDeltaVariable<int>                           m_spokenLanguage;
	Archive::AutoDeltaMap<NetworkId, Waypoint, PlayerObject>  m_waypoints;
	Archive::AutoDeltaMap<NetworkId, Waypoint, PlayerObject>  m_groupWaypoints;
	Archive::AutoDeltaVariable<int>                           m_bornDate;
	Archive::AutoDeltaVariable<uint32>                        m_playedTime;
	float                                                     m_playedTimeAccum;

	// information to track player's play time during this particular session
	// this information is kept and tracked in the connection server, but is
	// cached here for use by the game server
	Archive::AutoDeltaVariable<int32>                         m_sessionStartPlayTime;          // time when the player started playing the character
	Archive::AutoDeltaVariable<int32>                         m_sessionLastActiveTime;         // the client will detect when the player is "active" or "inactive"; this keeps track of the last time that the client said the player was "active"; if 0, it means the client is currently "inactive"
	Archive::AutoDeltaVariable<unsigned long>                 m_sessionActivePlayTimeDuration; // total amount of play time player was active (i.e. at the mouse/keyboard/joystick)

	Archive::AutoDeltaVariable<int>                           m_food;
	Archive::AutoDeltaVariable<int>                           m_maxFood;
	Archive::AutoDeltaVariable<int>                           m_drink;
	Archive::AutoDeltaVariable<int>                           m_maxDrink;
	Archive::AutoDeltaVariable<int>                           m_meds;
	Archive::AutoDeltaVariable<int>                           m_maxMeds;

	Archive::AutoDeltaVariable<int8>                          m_privledgedTitle;
	Archive::AutoDeltaVariable<BitArray>                      m_completedQuests;
	Archive::AutoDeltaVariable<BitArray>                      m_activeQuests;
	Archive::AutoDeltaVariable<uint32>                        m_currentQuest;

	Archive::AutoDeltaPackedMap<uint32, PlayerQuestData>      m_quests;
	
	Archive::AutoDeltaVariable<std::string>                   m_theaterDatatable;
	Archive::AutoDeltaVariable<Vector>                        m_theaterPosition;
	Archive::AutoDeltaVariable<std::string>                   m_theaterScene;
	Archive::AutoDeltaVariable<std::string>                   m_theaterScript;
	Archive::AutoDeltaVariable<int>                           m_theaterNumObjects;
	Archive::AutoDeltaVariable<float>                         m_theaterRadius;
	Archive::AutoDeltaVariable<NetworkId>                     m_theaterCreator;
	Archive::AutoDeltaVariable<std::string>                   m_theaterName;
	Archive::AutoDeltaVariable<NetworkId>                     m_theaterId;
	Archive::AutoDeltaVariable<int>                           m_theaterLocationType;
	Archive::AutoDeltaVariable<int>                           m_roleIconChoice;
	Archive::AutoDeltaVariable<time_t> m_aggroImmuneDuration;
	Archive::AutoDeltaVariable<time_t> m_aggroImmuneStartTime;

	ChatLog * const m_chatLog;
	time_t m_chatLogPurgeTime;
	bool m_isUnsticking;

	//when asking a player to accept or complete a quest, remember the quest that we asked them about

	//TODO sync to other servers as necessary?
	uint32 m_pendingRequestQuestCrc;
	NetworkId m_pendingRequestQuestGiver;

	Archive::AutoDeltaVariable<std::string> m_skillTemplate;
	Archive::AutoDeltaVariable<std::string> m_workingSkill;
	Archive::AutoDeltaVariable<bool>                          m_isFromLogin;   ///< This defaults to true, and is set to false in CreatureObject::virtualOnSetClient, (used to distinguish when coming from login as opposed to from a scene warp)

	Archive::AutoDeltaVariable<int32>                         m_currentGcwPoints;
	Archive::AutoDeltaVariable<int32>                         m_currentGcwRating;
	Archive::AutoDeltaVariable<int32>                         m_currentPvpKills;
	Archive::AutoDeltaVariable<int64>                         m_lifetimeGcwPoints;
	Archive::AutoDeltaVariable<int32>                         m_maxGcwImperialRating;
	Archive::AutoDeltaVariable<int32>                         m_maxGcwRebelRating;
	Archive::AutoDeltaVariable<int32>                         m_lifetimePvpKills;
	Archive::AutoDeltaVariable<int32>                         m_nextGcwRatingCalcTime;
	Archive::AutoDeltaVariable<int>                           m_currentGcwRank;
	Archive::AutoDeltaVariable<float>                         m_currentGcwRankProgress;
	Archive::AutoDeltaVariable<int>                           m_maxGcwImperialRank;
	Archive::AutoDeltaVariable<int>                           m_maxGcwRebelRank;

	// the Epoch when the next messageTo to cause GCW rating recalculation will occur
	Archive::AutoDeltaVariable<int32>                         m_gcwRatingActualCalcTime;

	// copy of the player's hate list sent to the authoritative client
	Archive::AutoDeltaSet<NetworkId>                          m_playerHateList;

	// activity that the player has done during this session
	Archive::AutoDeltaVariable<uint32>                        m_sessionActivity;
	// these two variables represent the commando kill meter
	Archive::AutoDeltaVariable<int32>                         m_killMeter;
	// this variable stores the fractional state of the meter but is not persisted nor synchronized across servers.
	float                                                     m_killMeterRemainder;

	Archive::AutoDeltaVariable<NetworkId>					  m_petId;
	Archive::AutoDeltaVector<std::string>                     m_petCommandList;
	Archive::AutoDeltaVector<std::string>                     m_petToggledCommands;  // These are the commands on the
																					 // pet toolbar that should display
																					 // visually as being toggled on

	// both m_collections and m_collections2 form a "contiguous" collections bit array
	// we only break them up into multiple members because of DB storage restrictions
	Archive::AutoDeltaVariable<BitArray>                      m_collections;
	Archive::AutoDeltaVariable<BitArray>                      m_collections2;
	Archive::AutoDeltaVariable<BitArray> *                    m_allCollections[2];

	// chat spam info
	Archive::AutoDeltaVariable<int>                           m_chatSpamSpatialNumCharacters;
	Archive::AutoDeltaVariable<int>                           m_chatSpamNonSpatialNumCharacters;
	Archive::AutoDeltaVariable<int>                           m_chatSpamTimeEndInterval;
	Archive::AutoDeltaVariable<int>                           m_chatSpamNextTimeToSyncWithChatServer;
	time_t                                                    m_chatSpamNextTimeToNotifyPlayerWhenLimited;

	// citizenship info
	Archive::AutoDeltaVariable<std::string>                   m_citizenshipCity;
	Archive::AutoDeltaVariable<int8>                          m_citizenshipType; // CityDataCitizenType

	// GCW Region info
	Archive::AutoDeltaVariable<std::string>                   m_currentGcwRegion;
	// std::pair<region, std::pair<qualify for region bonus, qualify to use region defender title> >
	Archive::AutoDeltaVariable<std::pair<std::string, std::pair<bool, bool> > > m_cityGcwDefenderRegion;
	Archive::AutoDeltaVariable<std::pair<std::string, std::pair<bool, bool> > > m_guildGcwDefenderRegion;

	// squelch info
	Archive::AutoDeltaVariable<NetworkId>                     m_squelchedById;   // id of the toon who squelched this toon; is NetworkId::cms_invalid if this toon is not squelched
	Archive::AutoDeltaVariable<std::string>                   m_squelchedByName; // name of the toon who squelched this toon
	Archive::AutoDeltaVariable<int32>                         m_squelchExpireTime; // the Epoch time when the toon will be unsquelched; is < 0 for indefinite squelch

	Archive::AutoDeltaVariable<bool>						  m_showBackpack;
	Archive::AutoDeltaVariable<bool>						  m_showHelmet;

	Archive::AutoDeltaVariable<EnvironmentInfo>               m_environmentFlags; // Force Day, Night, whatever else.
	// Default attack override
	Archive::AutoDeltaVariable<std::string>                   m_defaultAttackOverride; // This string will override the user's default attack

	Archive::AutoDeltaVariable<BitArray>                      m_guildRank;
	Archive::AutoDeltaVariable<BitArray>                      m_citizenRank;

	// for use to track the status of the /pickupAllRoomItemsIntoInventory
	// and /dropAllInventoryItemsIntoRoom commands, which occurs over a
	// period of time; these are not proxied, as the only way the cell/structure
	// in which the player is standing when the command is executed can
	// change authority is during a game server crash recovery scenario
	// (or a warp in the case of a POB ship), in which case we'll just cancel
	// the currently executing /pickupAllRoomItemsIntoInventory
	// or /dropAllInventoryItemsIntoRoom command, and the player do it again
	int m_pickupDropAllItemsOperationId;
	NetworkId m_pickupDropAllItemsCellId;
	bool m_pickupAllItemsAllowInventoryOverload;
	int m_pickupDropAllItemsNumItemsSoFar;
	std::set<NetworkId> m_pickupDropAllItemsProcessedItemsSoFar;

	// for use to track the status of a restore decoration layout operation,
	// which occurs over a period of time; these are not proxied, as the only
	// way the structure in which the player is standing when the command
	// is executed can change authority is during a game server crash recovery
	// scenario (or a warp in the case of a POB ship), in which case we'll just
	// cancel the currently executing operation, and the player can do it again
	int m_restoreDecorationOperationId;
	NetworkId m_restoreDecorationPobId;
	NetworkId m_restoreDecorationObjectBeingRestored;
	int m_restoreDecorationTotalCount;
	int m_restoreDecorationAttemptedCount;
	int m_restoreDecorationSuccessCount;
	std::list<std::pair<std::pair<CachedNetworkId, CachedNetworkId>, std::pair<Vector, Quaternion> > > m_restoreDecorationLayoutList;

	Archive::AutoDeltaVariable<int8> m_galacticReserveDeposit;

	Archive::AutoDeltaVariable<int64> m_pgcRatingCount;
	Archive::AutoDeltaVariable<int64> m_pgcRatingTotal;
	Archive::AutoDeltaVariable<int> m_pgcLastRatingTime;
};

// ======================================================================

inline StationId PlayerObject::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline float PlayerObject::getCheaterLevel() const
{
	return m_accountCheaterLevel.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getBornDate() const
{
	return m_bornDate.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getSessionStartPlayTime() const
{
	return m_sessionStartPlayTime.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getSessionLastActiveTime() const
{
	return m_sessionLastActiveTime.get();
}

// ----------------------------------------------------------------------

inline const std::map<std::pair<uint32, uint32>,int> & PlayerObject::getDraftSchematics () const
{
	return m_draftSchematics.getMap();
}

// ----------------------------------------------------------------------

inline void PlayerObject::clearSchematics()
{
	m_draftSchematics.clear();
}

// ----------------------------------------------------------------------

inline const CachedNetworkId & PlayerObject::getCraftingTool() const
{
	return m_craftingTool.get();
}

// ----------------------------------------------------------------------

inline const CachedNetworkId & PlayerObject::getCraftingStation() const
{
	return m_craftingStation.get();
}

// ----------------------------------------------------------------------

inline const DraftSchematicObject * PlayerObject::getCurrentDraftSchematic () const
{
	return m_draftSchematic.getPointer();
}

// ----------------------------------------------------------------------

inline enum Crafting::CraftingStage PlayerObject::getCraftingStage () const
{
	return static_cast<enum Crafting::CraftingStage>(m_craftingStage.get());
}

// ----------------------------------------------------------------------

inline void PlayerObject::setCraftingLevel(int level)
{
	m_craftingLevel = level;
}

// ----------------------------------------------------------------------

inline int PlayerObject::getCraftingLevel() const
{
	return m_craftingLevel.get();
}

// ----------------------------------------------------------------------

inline bool PlayerObject::isCrafting(void) const
{
	return getCraftingStage() != Crafting::CS_none;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setAllowEmptySlot(bool allow)
{
	m_allowEmptySlot = allow;	
}

// ----------------------------------------------------------------------

inline int PlayerObject::getAccountNumLots() const
{
	return m_accountNumLots.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getMaxForcePower(void) const
{
	return m_maxForcePower.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getForcePower(void) const
{
	return m_forcePower.get();
}

// ----------------------------------------------------------------------

inline float PlayerObject::getForcePowerRegenRate() const
{
	return m_forceRegenRate.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & PlayerObject::getTheater() const
{
	return m_theaterId.get();
}

// ----------------------------------------------------------------------

inline bool PlayerObject::getIsFromLogin(void) const
{
	return m_isFromLogin.get();
}

// ----------------------------------------------------------------------

inline void PlayerObject::setIsFromLogin(bool val) 
{
	m_isFromLogin = val;
}

//----------------------------------------------------------------------

inline bool PlayerObject::isNormalPlayer() const
{
	return m_privledgedTitle.get() == static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer);
}

//----------------------------------------------------------------------

inline bool PlayerObject::isCustomerServiceRepresentative() const
{
	return m_privledgedTitle.get() == static_cast<int8>(PlayerDataPriviledgedTitle::CustomerServiceRepresentative);
}

//----------------------------------------------------------------------

inline bool PlayerObject::isDeveloper() const
{
	return m_privledgedTitle.get() == static_cast<int8>(PlayerDataPriviledgedTitle::Developer);
}

//----------------------------------------------------------------------

inline bool PlayerObject::isWarden() const
{
	return m_privledgedTitle.get() == static_cast<int8>(PlayerDataPriviledgedTitle::Warden);
}

// ----------------------------------------------------------------------

inline uint32 PlayerObject::getCurrentQuest() const
{
	return m_currentQuest.get();
}

// ----------------------------------------------------------------------

inline void PlayerObject::setCurrentQuest(uint32 questCrc)
{
	m_currentQuest = questCrc;
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getCurrentGcwPoints() const
{
	return m_currentGcwPoints.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getKillMeter() const
{
	return m_killMeter.get();
}

inline NetworkId const & PlayerObject::getPetId() const
{
	return m_petId.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getCurrentGcwRating() const
{
	return m_currentGcwRating.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getCurrentPvpKills() const
{
	return m_currentPvpKills.get();
}

// ----------------------------------------------------------------------

inline int64 PlayerObject::getLifetimeGcwPoints() const
{
	return m_lifetimeGcwPoints.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getMaxGcwImperialRating() const
{
	return m_maxGcwImperialRating.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getMaxGcwRebelRating() const
{
	return m_maxGcwRebelRating.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getLifetimePvpKills() const
{
	return m_lifetimePvpKills.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getNextGcwRatingCalcTime() const
{
	return m_nextGcwRatingCalcTime.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getCurrentGcwRank() const
{
	return m_currentGcwRank.get();
}

// ----------------------------------------------------------------------

inline float PlayerObject::getCurrentGcwRankProgress() const
{
	return m_currentGcwRankProgress.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getMaxGcwImperialRank() const
{
	return m_maxGcwImperialRank.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getMaxGcwRebelRank() const
{
	return m_maxGcwRebelRank.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getGcwRatingActualCalcTime() const
{
	return m_gcwRatingActualCalcTime.get();
}

// ----------------------------------------------------------------------

inline void PlayerObject::addToPlayerHateList(NetworkId const & id)
{
	m_playerHateList.insert(id);
}

// ----------------------------------------------------------------------

inline void PlayerObject::removeFromPlayerHateList(NetworkId const & id)
{
	m_playerHateList.erase(id);
}

// ----------------------------------------------------------------------

inline void PlayerObject::clearPlayerHateList()
{
	m_playerHateList.clear();
}

// ----------------------------------------------------------------------

inline uint32 PlayerObject::getSessionActivity() const
{
	return m_sessionActivity.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getChatSpamSpatialNumCharacters() const
{
	return m_chatSpamSpatialNumCharacters.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getChatSpamNonSpatialNumCharacters() const
{
	return m_chatSpamNonSpatialNumCharacters.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getChatSpamTimeEndInterval() const
{
	return m_chatSpamTimeEndInterval.get();
}

// ----------------------------------------------------------------------

inline time_t PlayerObject::getChatSpamNextTimeToNotifyPlayerWhenLimited() const
{
	return m_chatSpamNextTimeToNotifyPlayerWhenLimited;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setChatSpamNextTimeToNotifyPlayerWhenLimited(time_t chatSpamNextTimeToNotifyPlayerWhenLimited)
{
	m_chatSpamNextTimeToNotifyPlayerWhenLimited = chatSpamNextTimeToNotifyPlayerWhenLimited;
}

// ----------------------------------------------------------------------

inline NetworkId const & PlayerObject::getSquelchedById() const
{
	return m_squelchedById.get();
}

// ----------------------------------------------------------------------

inline bool PlayerObject::getShowHelmet() const
{
	return m_showHelmet.get();
}

// ----------------------------------------------------------------------

inline bool PlayerObject::getShowBackpack() const
{
	return m_showBackpack.get();
}

// ----------------------------------------------------------------------

inline PlayerObject::StringVector const & PlayerObject::getSortedLowercaseFriendList() const
{
	return m_friendList.get();
}

// ----------------------------------------------------------------------

inline PlayerObject::StringVector const & PlayerObject::getSortedLowercaseIgnoreList() const
{
	return m_ignoreList.get();
}

// ----------------------------------------------------------------------

inline EnvironmentInfo PlayerObject::getEnvironmentFlags() const
{
	return m_environmentFlags.get();
}

// ----------------------------------------------------------------------

inline void PlayerObject::overrideDefaultAttack(std::string const & attack)
{
	m_defaultAttackOverride = attack;
}

// ----------------------------------------------------------------------

inline std::string const & PlayerObject::getDefaultAttackOverride() const
{
	return m_defaultAttackOverride.get();
}

// ----------------------------------------------------------------------

inline void PlayerObject::setGuildRank(BitArray const & guildRank)
{
	m_guildRank.set(guildRank);
}

// ----------------------------------------------------------------------

inline BitArray const & PlayerObject::getGuildRank() const
{
	return m_guildRank.get();
}

// ----------------------------------------------------------------------

inline void PlayerObject::setCitizenRank(BitArray const & citizenRank)
{
	m_citizenRank.set(citizenRank);
}

// ----------------------------------------------------------------------

inline BitArray const & PlayerObject::getCitizenRank() const
{
	return m_citizenRank.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getPickupDropAllItemsOperationId() const
{
	return m_pickupDropAllItemsOperationId;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setPickupDropAllItemsOperationId(int pickupDropAllItemsOperationId)
{
	m_pickupDropAllItemsOperationId = pickupDropAllItemsOperationId;
}

// ----------------------------------------------------------------------

inline NetworkId const & PlayerObject::getPickupDropAllItemsCellId() const
{
	return m_pickupDropAllItemsCellId;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setPickupDropAllItemsCellId(NetworkId const & pickupDropAllItemsCellId)
{
	m_pickupDropAllItemsCellId = pickupDropAllItemsCellId;
}

// ----------------------------------------------------------------------

inline bool PlayerObject::getPickupAllItemsAllowInventoryOverload() const
{
	return m_pickupAllItemsAllowInventoryOverload;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setPickupAllItemsAllowInventoryOverload(bool pickupAllItemsAllowInventoryOverload)
{
	m_pickupAllItemsAllowInventoryOverload = pickupAllItemsAllowInventoryOverload;
}

// ----------------------------------------------------------------------

inline int PlayerObject::getPickupDropAllItemsNumItemsSoFar() const
{
	return m_pickupDropAllItemsNumItemsSoFar;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setPickupDropAllItemsNumItemsSoFar(int pickupDropAllItemsNumItemsSoFar)
{
	m_pickupDropAllItemsNumItemsSoFar = pickupDropAllItemsNumItemsSoFar;
}

// ----------------------------------------------------------------------

inline std::set<NetworkId> & PlayerObject::getPickupDropAllItemsProcessedItemsSoFar()
{
	return m_pickupDropAllItemsProcessedItemsSoFar;
}

// ----------------------------------------------------------------------

inline int PlayerObject::getRestoreDecorationOperationId() const
{
	return m_restoreDecorationOperationId;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setRestoreDecorationOperationId(int restoreDecorationOperationId)
{
	m_restoreDecorationOperationId = restoreDecorationOperationId;
}

// ----------------------------------------------------------------------

inline NetworkId const & PlayerObject::getRestoreDecorationPobId() const
{
	return m_restoreDecorationPobId;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setRestoreDecorationPobId(NetworkId const & restoreDecorationPobId)
{
	m_restoreDecorationPobId = restoreDecorationPobId;
}

// ----------------------------------------------------------------------

inline NetworkId const & PlayerObject::getRestoreDecorationObjectBeingRestored() const
{
	return m_restoreDecorationObjectBeingRestored;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setRestoreDecorationObjectBeingRestored(NetworkId const & restoreDecorationObjectBeingRestored)
{
	m_restoreDecorationObjectBeingRestored = restoreDecorationObjectBeingRestored;
}

// ----------------------------------------------------------------------

inline int PlayerObject::getRestoreDecorationTotalCount() const
{
	return m_restoreDecorationTotalCount;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setRestoreDecorationTotalCount(int restoreDecorationTotalCount)
{
	m_restoreDecorationTotalCount = restoreDecorationTotalCount;
}

// ----------------------------------------------------------------------

inline int PlayerObject::getRestoreDecorationAttemptedCount() const
{
	return m_restoreDecorationAttemptedCount;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setRestoreDecorationAttemptedCount(int restoreDecorationAttemptedCount)
{
	m_restoreDecorationAttemptedCount = restoreDecorationAttemptedCount;
}

// ----------------------------------------------------------------------

inline int PlayerObject::getRestoreDecorationSuccessCount() const
{
	return m_restoreDecorationSuccessCount;
}

// ----------------------------------------------------------------------

inline void PlayerObject::setRestoreDecorationSuccessCount(int restoreDecorationSuccessCount)
{
	m_restoreDecorationSuccessCount = restoreDecorationSuccessCount;
}

// ----------------------------------------------------------------------

inline std::list<std::pair<std::pair<CachedNetworkId, CachedNetworkId>, std::pair<Vector, Quaternion> > > & PlayerObject::getRestoreDecorationLayoutList()
{
	return m_restoreDecorationLayoutList;
}

// ----------------------------------------------------------------------

inline int8 PlayerObject::getGalacticReserveDeposit() const
{
	return m_galacticReserveDeposit.get();
}

// ----------------------------------------------------------------------

inline std::string const & PlayerObject::getCurrentGcwRegion() const
{
	return m_currentGcwRegion.get();
}

// ----------------------------------------------------------------------

inline std::pair<std::string, std::pair<bool, bool> > const & PlayerObject::getCityGcwDefenderRegionInfo() const
{
	return m_cityGcwDefenderRegion.get();
}

// ----------------------------------------------------------------------

inline std::pair<std::string, std::pair<bool, bool> > const & PlayerObject::getGuildGcwDefenderRegionInfo() const
{
	return m_guildGcwDefenderRegion.get();
}

// ======================================================================

#endif	// _PLAYER_OBJECT_H
