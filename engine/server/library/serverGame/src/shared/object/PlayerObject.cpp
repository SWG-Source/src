// ======================================================================
//
// PlayerObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlayerObject.h"

#include "LocalizationManager.h"
#include "serverGame/BiographyManager.h"
#include "serverGame/Chat.h"
#include "serverGame/CitizenInfo.h"
#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DestroyMessageManager.h"
#include "serverGame/FactoryObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GroupWaypointBuilder.h"
#include "serverGame/GuildInfo.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/GuildMemberInfo.h"
#include "sharedGame/GuildRankDataTable.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PgcRatingInfo.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/ProsePackageManagerServer.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerController.h"
#include "serverGame/ServerPlayerObjectTemplate.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TangibleObject.h"
#include "serverNetworkMessages/LoginUpgradeAccountMessage.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "serverScript/ConfigServerScript.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ConfigServerUtility.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/CitizenRankDataTable.h"
#include "sharedGame/CityData.h"
#include "sharedGame/DraftSchematicGroupManager.h"
#include "sharedGame/GameLanguageManager.h"
#include "sharedGame/LfgDataTable.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/Quest.h"
#include "sharedGame/QuestManager.h"
#include "sharedGame/QuestTask.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/TextIterator.h"
#include "sharedGame/TextManager.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueDraftSchematics.h"
#include "sharedNetworkMessages/MessageQueueDraftSlots.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsQueryResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/PlayMusicMessage.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedUtility/Location.h"
#include "sharedUtility/LocationArchive.h"
#include "UnicodeUtils.h"

#include <limits>
#include <list>

const SharedObjectTemplate * PlayerObject::m_defaultSharedTemplate = nullptr;
bool PlayerObject::m_allowEmptySlot = false;


//-----------------------------------------------------------------------
PlayerObject::ChatLogEntry::ChatLogEntry()
 : m_index(0)
 , m_time(0)
// , m_serverProcessId(0)
{
}

// ======================================================================

namespace PlayerObjectNamespace
{
	// slot that holds the datapad
	const ConstCharCrcLowerString DATAPAD_SLOT_NAME("datapad");

	// objvars for crafting
	const std::string OBJVAR_CRAFTING_STATION("crafting.station");
	const std::string OBJVAR_PRIVATE_STATION("crafting.private");
	const std::string OBJVAR_CRAFTING_PROTOTYPE_TIME("crafting.prototypeTime");
	const std::string OBJVAR_CRAFTING_PROTOTYPE_START_TIME("crafting.prototypeStartTime");
	const std::string OBJVAR_CRAFTING_PROTOTYPE_OBJECT("crafting.prototypeObject");
	const std::string OBJVAR_CRAFTING_PROTOTYPE_CRAFTER("crafting.prototypeCrafter");
	const std::string OBJVAR_CRAFTING_FAKE_PROTOTYPE("crafting.isFakePrototype");
	const std::string OBJVAR_CRAFTING_COMPONENT_CAN_BE_DAMAGED("crafting.damaged_ok");
	const std::string OBJVAR_FORCE_CRITICAL_ASSEMBLY("crafting.force_critical_assembly");
	const std::string OBJVAR_FORCE_CRITICAL_EXPERIMENT("crafting.force_critical_experiment");
	const std::string OBJVAR_FORCE_CRITICAL_SUCCESS("crafting.force_critical_success");
	const std::string OBJVAR_IN_CRAFTING_SESSION("crafting.inCraftingSession");

	// objvars for chat spam info
	const std::string OBJVAR_CHAT_SPAM_TIME_END_INTERVAL("chatSpam.timeEndInterval");
	const std::string OBJVAR_CHAT_SPAM_SPATIAL("chatSpam.spatial");
	const std::string OBJVAR_CHAT_SPAM_NON_SPATIAL("chatSpam.nonSpatial");

	// objvars for privileged title
	const std::string OBJVAR_PRIVILEGED_TITLE("privilegedTitle");

	// objvars for squelch info
	const std::string OBJVAR_SQUELCH_ID("squelch.id");
	const std::string OBJVAR_SQUELCH_NAME("squelch.name");
	const std::string OBJVAR_SQUELCH_EXPIRE("squelch.expire");

	// skill data granted independently from skills
	const std::string OBJVAR_NOT_SKILL_SCHEMATICS("_notskill.schematics");

	const std::string OBJVAR_EXPERIENCE_MULTIPLIER("xp_multiplier");
	const std::string OBJVAR_GAME_FACTION_RANKINGS("gcw");

	const std::string OBJVAR_DELETED_QUEST_COUNT("quests.numQuestsDeleted");

	const StringId COMPONENT_LOST_MSG("system_msg", "lost_component");

	const int MAX_KILL_METER = 50;

	//----------------------------------------------------------------------

	std::set<PlayerObject const *> s_allPlayerObjects;

	//----------------------------------------------------------------------

	bool isNestedInContainer (const NetworkId & itemId, const NetworkId & containerId)
	{
		NetworkId theItemId = itemId;
		for (;;)
		{
			if (theItemId == containerId)
				return true;

			const Object * const item = NetworkIdManager::getObjectById (theItemId);
			if (!item)
				break;

			const Object * const parent = ContainerInterface::getContainedByObject (*item);
			if (!parent)
				break;

			theItemId = parent->getNetworkId ();
		}

		return false;
	}

	void refScript(CreatureObject & owner, const std::string & scriptName)
	{
		GameScriptObject * script = owner.getScriptObject();
		if(script)
		{
			if(! script->hasScript(scriptName))
			{
				script->attachScript(scriptName, false);
			}

			int scriptRefCount = 0;
			const DynamicVariableList & objvars = owner.getObjVars();

			const std::string itemName = "quest.script." + scriptName;
			if (objvars.getItem(itemName, scriptRefCount))
			{
				scriptRefCount++;
			}
			else
			{
				scriptRefCount = 1;
			}

			owner.setObjVarItem(itemName, scriptRefCount);
		}
	}

	void derefScript(CreatureObject & owner, const std::string & scriptName)
	{
		int scriptRefCount = 0;
		const DynamicVariableList & objvars = owner.getObjVars();

		const std::string itemName = "quest.script." + scriptName;
		if (objvars.getItem(itemName, scriptRefCount))
		{
			scriptRefCount--;
			if(scriptRefCount < 0)
			{
				scriptRefCount = 0;
			}
		}
		else
		{
			scriptRefCount = 0;
		}

		if(scriptRefCount > 0)
		{
			owner.setObjVarItem(itemName, scriptRefCount);
		}
		else
		{
			owner.removeObjVarItem(itemName);
			GameScriptObject * script = owner.getScriptObject();
			if(script)
			{
				if(script->hasScript(scriptName))
				{
					script->detachScript(scriptName);
				}
			}
		}
	}

	//----------------------------------------------------------------------

	void checkAndSetCharacterAgeTitle(PlayerObject & player, bool thoroughCheck);
	void checkAndSetAccountAgeTitle(PlayerObject & player);

	//----------------------------------------------------------------------

//	size_t s_waypointLimit = 10;
	size_t s_questLimit = 1000;

	//----------------------------------------------------------------------

	// Class to help us sort quest data by age
	class QuestDataToSort
	{
	public:
		QuestDataToSort() :
			m_key(0)
		{}

		QuestDataToSort( uint32 key, PlayerQuestData const& questData ) :
			m_key(key),
			m_questData(questData)
		{}

		bool operator<(QuestDataToSort const & rhs) const
		{
			return (m_questData.getRelativeAgeIndex() < rhs.m_questData.getRelativeAgeIndex());
		}

		uint32                 GetPlayerQuestDataKey() const { return m_key; }
		PlayerQuestData const& GetPlayerQuestData() const    { return m_questData; }

	private:
		uint32          m_key;
		PlayerQuestData m_questData;
	};

	typedef std::list<QuestDataToSort> SortableQuestList;
	typedef std::vector< std::pair<int, int> > TaskChanceVector;

	void grantGcwFactionalPresenceScore(std::string const & gcwCategory, PlayerObject const & po, CreatureObject const & co);
};

using namespace PlayerObjectNamespace;

// ======================================================================

PlayerObject::PlayerObject(const ServerPlayerObjectTemplate* newTemplate) :
		IntangibleObject(newTemplate),
		m_callback(new MessageDispatch::Callback),
		m_stationId(0),
		m_houseId(CachedNetworkId(NetworkId::cms_invalid)),
		m_accountNumLots(0),
		m_accountNumLotsOverLimitSpam(0),
		m_accountMaxLotsAdjustment(0),
		m_accountIsOutcast(false),
		m_accountCheaterLevel(0.0f),
		m_draftSchematics(),
		m_experiencePoints(),
		m_expModified(0),
		m_maxForcePower(0),
		m_forcePower(0),
		m_forceRegenRate(0),
		m_forceRegenValue(0),
		m_craftingLevel(0),
		m_experimentPoints(8),
		m_craftingStage(static_cast<int>(Crafting::CS_none)),
		m_craftingTool(),
		m_craftingStation(),
		m_craftingComponentBioLink(),
		m_useableDraftSchematics(),
		m_draftSchematic(nullptr),
		m_matchMakingPersonalProfileId(),
		m_matchMakingCharacterProfileId(),
		m_friendList(),
		m_ignoreList(),
		m_skillTitle(),
		m_spokenLanguage(GameLanguageManager::getBasicLanguageId()),
		m_waypoints(),
		m_groupWaypoints(),
		m_bornDate(0),
		m_playedTime(0),
		m_playedTimeAccum(0),
		m_sessionStartPlayTime(0),
		m_sessionLastActiveTime(0),
		m_sessionActivePlayTimeDuration(0),
		m_food(0),
		m_maxFood(100),
		m_drink(0),
		m_maxDrink(100),
		m_meds(0),
		m_maxMeds(100),
		m_privledgedTitle(static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer)),
		m_completedQuests(),
		m_activeQuests(),
		m_currentQuest(0),
		m_theaterDatatable(),
		m_theaterPosition(),
		m_theaterScene(),
		m_theaterScript(),
		m_theaterNumObjects(),
		m_theaterRadius(),
		m_theaterCreator(),
		m_theaterName(),
		m_theaterId(),
		m_theaterLocationType(IntangibleObject::TLT_getGoodLocation),
		m_roleIconChoice(0),
		m_aggroImmuneDuration(0),
		m_aggroImmuneStartTime(0),
		m_chatLog(new ChatLog),
		m_chatLogPurgeTime(0),
		m_isUnsticking(false),
		m_pendingRequestQuestCrc(0),
		m_pendingRequestQuestGiver(NetworkId::cms_invalid),
		m_skillTemplate(),
		m_workingSkill(),
		m_isFromLogin(true),
		m_currentGcwPoints(0),
		m_currentGcwRating(-1),
		m_currentPvpKills(0),
		m_lifetimeGcwPoints(0),
		m_maxGcwImperialRating(-1),
		m_maxGcwRebelRating(-1),
		m_lifetimePvpKills(0),
		m_nextGcwRatingCalcTime(0),
		m_currentGcwRank(0),
		m_currentGcwRankProgress(0.0f),
		m_maxGcwImperialRank(0),
		m_maxGcwRebelRank(0),
		m_gcwRatingActualCalcTime(0),
		m_playerHateList(),
		m_sessionActivity(0),
		m_killMeter(0),
		m_killMeterRemainder(0.0f),
		m_petId(NetworkId::cms_invalid),
		m_petCommandList(0),
		m_petToggledCommands(0),
		m_collections(),
		m_collections2(),
		m_chatSpamSpatialNumCharacters(0),
		m_chatSpamNonSpatialNumCharacters(0),
		m_chatSpamTimeEndInterval(0),
		m_chatSpamNextTimeToSyncWithChatServer(0),
		m_chatSpamNextTimeToNotifyPlayerWhenLimited(0),
		m_citizenshipCity(),
		m_citizenshipType(static_cast<int8>(CityDataCitizenType::NotACitizen)),
		m_currentGcwRegion(),
		m_cityGcwDefenderRegion(std::make_pair(std::string(), std::make_pair(false, false))),
		m_guildGcwDefenderRegion(std::make_pair(std::string(), std::make_pair(false, false))),
		m_squelchedById(NetworkId::cms_invalid),
		m_squelchedByName(),
		m_squelchExpireTime(0),
		m_showBackpack(true),
		m_showHelmet(true),
		m_environmentFlags(0),
		m_defaultAttackOverride(),
		m_guildRank(),
		m_citizenRank(),
		m_pickupDropAllItemsOperationId(0),
		m_pickupDropAllItemsCellId(NetworkId::cms_invalid),
		m_pickupAllItemsAllowInventoryOverload(false),
		m_pickupDropAllItemsNumItemsSoFar(0),
		m_pickupDropAllItemsProcessedItemsSoFar(),
		m_restoreDecorationOperationId(0),
		m_restoreDecorationPobId(NetworkId::cms_invalid),
		m_restoreDecorationObjectBeingRestored(NetworkId::cms_invalid),
		m_restoreDecorationTotalCount(0),
		m_restoreDecorationAttemptedCount(0),
		m_restoreDecorationSuccessCount(0),
		m_restoreDecorationLayoutList(),
		m_galacticReserveDeposit(0),
		m_pgcRatingCount(0),
		m_pgcRatingTotal(0),
		m_pgcLastRatingTime(0)
{
	m_allCollections[0] = &m_collections;
	m_allCollections[1] = &m_collections2;

	addMembersToPackages();
	m_waypoints.setOnInsert(this, &PlayerObject::onWaypointCreated);
	m_waypoints.setOnSet(this, &PlayerObject::onWaypointSet);
	m_waypoints.setOnErase(this, &PlayerObject::onWaypointRemoved);
	IGNORE_RETURN(s_allPlayerObjects.insert(this));
}

//-----------------------------------------------------------------------

PlayerObject::~PlayerObject()
{
	delete m_callback;
	delete m_chatLog;
	IGNORE_RETURN(s_allPlayerObjects.erase(this));
}

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * PlayerObject::getDefaultSharedTemplate() const
{
static const ConstCharCrcLowerString templateName("object/player/base/shared_player_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "PlayerObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// PlayerObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void PlayerObject::removeDefaultTemplate()
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// PlayerObject::removeDefaultTemplate

// ----------------------------------------------------------------------

void PlayerObject::setStationId(StationId account)
{
	DEBUG_FATAL(!isAuthoritative(),("PlayerObject %s wasn't authoritative in setStationId.\n",getNetworkId().getValueString().c_str()));
	m_stationId=account;
}

// ----------------------------------------------------------------------

void PlayerObject::setCheaterLevel(float level)
{
	m_accountCheaterLevel = level;
}

//-----------------------------------------------------------------------

/**
 * Returns the current "birth" date. The birth date is defined as the number
 * of days since Jan 1st, 2001 (Jan 1st, 2001 = 0)
 */
int PlayerObject::getCurrentBornDate()
{
	// get the base date of Jan 1st, 2001
	struct tm baseTimeData;
	baseTimeData.tm_hour = 0;
	baseTimeData.tm_isdst = 0;
	baseTimeData.tm_mday = 1;
	baseTimeData.tm_min = 0;
	baseTimeData.tm_mon = 0;
	baseTimeData.tm_sec = 0;
	baseTimeData.tm_wday = 0;
	baseTimeData.tm_yday = 0;
	baseTimeData.tm_year = 101;
	time_t baseTime = mktime(&baseTimeData);

	// get the current time and compute the birth date
	time_t currentTime = time(nullptr);
	time_t delta = (currentTime - baseTime) / (60 * 60 * 24);
	delta += ((currentTime - baseTime) % (60 * 60 * 24) != 0 ? 1 : 0);
	return int(delta);
}	// PlayerObject::getCurrentBornDate

//-----------------------------------------------------------------------

/**
 * Sets the "birth" date of the player to the current date. The birth date is
 * defined as the number of days since Jan 1st, 2001 (Jan 1st, 2001 = 0)
 */
void PlayerObject::setBornDate()
{
	if (m_bornDate.get() != 0)
		return;

	if (isAuthoritative())
	{
		m_bornDate = (int)getCurrentBornDate();
	}
	else
	{
		WARNING(true, ("PlayerObject::setBornDate called on non-authoritative player"));
	}
}	// PlayerObject::setBornDate

//-----------------------------------------------------------------------

/**
* Adjust the "birth" date of the player by the specified number of days
*
* @param adjustment		the number of days (can be negative) to adjust the "birth" date by
*/
void PlayerObject::adjustBornDate(int adjustment)
{
	if (m_bornDate.get() == 0)
		return;

	if (isAuthoritative())
	{
		const int newBornDate = m_bornDate.get() + adjustment;
		if ((newBornDate <= 0) || (newBornDate > getCurrentBornDate()))
			return;

		m_bornDate = newBornDate;
	}
	else
	{
		WARNING(true, ("PlayerObject::adjustBornDate called on non-authoritative player"));
	}
}	// PlayerObject::adjustBornDate

//-----------------------------------------------------------------------

/**
 * Changes the count of the amount of time this player has been online. Note we only
 * update the time every 30 secs.
 *
 * @param time since last call to this function
 */
void PlayerObject::alterPlayedTime(float time)
{
static const float TIME_LIMIT = 30.0f;

	if (isAuthoritative())
	{
		m_playedTimeAccum += time;
		if (m_playedTimeAccum >= TIME_LIMIT)
		{
			m_playedTime = m_playedTime.get() + static_cast<uint32>(TIME_LIMIT);
			m_playedTimeAccum -= TIME_LIMIT;
		}
	}
	else
	{
		WARNING(true, ("PlayerObject::alterPlayedTime called on non-authoritative player"));
	}
}	// updatePlayedTime

//-----------------------------------------------------------------------

CreatureObject *PlayerObject::getCreatureObject()
{
	Object * const owner = ContainerInterface::getContainedByObject(*this);
	if (owner)
	{
		ServerObject * const serverOwner = owner->asServerObject();
		if (serverOwner)
			return serverOwner->asCreatureObject();
	}
	return 0;
}

//-----------------------------------------------------------------------

CreatureObject const * PlayerObject::getCreatureObject() const
{
	Object const * const owner = ContainerInterface::getContainedByObject(*this);
	if (owner)
	{
		ServerObject const * const serverOwner = owner->asServerObject();
		if (serverOwner)
			return serverOwner->asCreatureObject();
	}
	return 0;
}

// ----------------------------------------------------------------------

void PlayerObject::virtualOnSetAuthority()
{
	IntangibleObject::virtualOnSetAuthority();

	if (isCrafting())
	{
		const TangibleObject * tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
		if (tool != nullptr)
		{
			const ManufactureSchematicObject * manfSchematic = tool->getCraftingManufactureSchematic();
			if (manfSchematic != nullptr)
			{
				m_draftSchematic = DraftSchematicObject::getSchematic(
					manfSchematic->getDraftSchematic());
				if (m_draftSchematic.getPointer() == nullptr)
				{
					WARNING(true, ("PlayerObject::virtualOnSetAuthority object "
						"%s is flagged as crafting, but has bad manf schematic %lu",
						getNetworkId().getValueString().c_str(),
						manfSchematic->getDraftSchematic()));
				}
			}
			else
			{
				WARNING(true, ("PlayerObject::virtualOnSetAuthority object "
					"%s is flagged as crafting, but has no manf schematic",
					getNetworkId().getValueString().c_str()));
			}
		}
		else
		{
			WARNING(true, ("PlayerObject::virtualOnSetAuthority object "
				"%s is flagged as crafting, but has no crafting tool",
				getNetworkId().getValueString().c_str()));
		}
	}

	GroupWaypointBuilder::updateGroupWaypoints(*this, true);
}

// ----------------------------------------------------------------------

void PlayerObject::virtualOnLogout()
{
	IntangibleObject::virtualOnLogout();

	// stop crafting
	if (isCrafting())
		stopCrafting(false);

	// destroy any theater associated with us
	if (m_theaterId.get() != NetworkId::cms_invalid)
	{
		MessageToQueue::getInstance().sendMessageToC(m_theaterId.get(),
			"C++DestroySelf", "", 0, false);
		m_theaterId = NetworkId::cms_invalid;
	}

	// persist chat statistics
	setObjVarItem(OBJVAR_CHAT_SPAM_TIME_END_INTERVAL, m_chatSpamTimeEndInterval.get());
	setObjVarItem(OBJVAR_CHAT_SPAM_SPATIAL, m_chatSpamSpatialNumCharacters.get());
	setObjVarItem(OBJVAR_CHAT_SPAM_NON_SPATIAL, m_chatSpamNonSpatialNumCharacters.get());

	// persist privileged title
	if (m_privledgedTitle.get() != static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer))
		setObjVarItem(OBJVAR_PRIVILEGED_TITLE, static_cast<int>(m_privledgedTitle.get()));
	else
		removeObjVarItem(OBJVAR_PRIVILEGED_TITLE);

	// persist squelch info
	int const secondsUntilUnsquelched = getSecondsUntilUnsquelched();
	if (secondsUntilUnsquelched != 0)
	{
		setObjVarItem(OBJVAR_SQUELCH_ID, m_squelchedById.get());
		setObjVarItem(OBJVAR_SQUELCH_NAME, m_squelchedByName.get());
		if (secondsUntilUnsquelched < 0)
			setObjVarItem(OBJVAR_SQUELCH_EXPIRE, -1);
		else
			setObjVarItem(OBJVAR_SQUELCH_EXPIRE, static_cast<int>(ServerClock::getInstance().getGameTimeSeconds()) + secondsUntilUnsquelched);
	}
	else
	{
		removeObjVarItem(OBJVAR_SQUELCH_ID);
		removeObjVarItem(OBJVAR_SQUELCH_NAME);
		removeObjVarItem(OBJVAR_SQUELCH_EXPIRE);
	}
}

// ----------------------------------------------------------------------

int PlayerObject::getExperiencePoints(const std::string & experienceType) const
{
	int result = 0;
	std::map<std::string, int>::const_iterator f = m_experiencePoints.find(experienceType);
	if(f != m_experiencePoints.end())
	{
		result = (*f).second;
	}
	return result;
}

// ----------------------------------------------------------------------

const std::map<std::string, int> & PlayerObject::getExperiencePoints() const
{
	return m_experiencePoints.getMap();
}

// ----------------------------------------------------------------------

int PlayerObject::getExperienceLimit(const std::string & experienceType) const
{
	uint32 limit = static_cast<uint32>(-1);
	CreatureObject const * const owner = getCreatureObject();

	if(owner)
	{
		// find out the xp limit based on the player's skills
		const CreatureObject::SkillList & skills = owner->getSkillList();
		for (CreatureObject::SkillList::const_iterator iter = skills.begin();
			iter != skills.end(); ++iter)
		{
			if((*iter))
			{
				const SkillObject::ExperiencePair * xpInfo = (*iter)->getPrerequisiteExperience();
				if (xpInfo != nullptr && experienceType == xpInfo->first && xpInfo->second.second > 0)
				{
					if (limit == static_cast<uint32>(-1) ||
						limit < static_cast<uint32>(xpInfo->second.second))
					{
						limit = xpInfo->second.second;
					}
				}
			}
		}
		if (limit == static_cast<uint32>(-1))
		{
			// use the default limit
			limit = SkillManager::getInstance().getDefaultXpLimit(experienceType);
			DEBUG_WARNING(limit == static_cast<uint32>(-1), ("No default xp limit defined "
				"for xp type %s for player object %s", experienceType.c_str(), getNetworkId().getValueString().c_str()));
		}
	}

	DEBUG_WARNING(limit == static_cast<uint32>(-1), ("No xp limit defined "
			"for xp type %s for player object %s", experienceType.c_str(), getNetworkId().getValueString().c_str()));

	return limit;
}

// ----------------------------------------------------------------------

int PlayerObject::grantExperiencePoints(const std::string & experienceType, int amount)
{
	CreatureObject * const owner = getCreatureObject();
	NOT_NULL(owner);

	int amountGranted = amount;

	if(isAuthoritative())
	{
		if (amount > 0)
		{
			int multiplier = 1;

			if (!getObjVars().getItem(OBJVAR_EXPERIENCE_MULTIPLIER, multiplier))
				multiplier = ConfigServerGame::getXpMultiplier();

			amount *= multiplier;

			// adjust the xp based on what faction is doing best
			// NOTE: HACK HACK HACK!
			const PlanetObject * tatooine = ServerUniverse::getInstance().getTatooinePlanet();
			if (tatooine == nullptr)
				tatooine = ServerUniverse::getInstance().getPlanetByName("Tatooine");
			if (tatooine == nullptr)
			{
				WARNING(true, ("Can't find planet tatooine from ServerUniverse"));
			}
			else
			{
				std::string factionName;
				int factionPoints = 0;
				bool tie = false;
				const DynamicVariableList::NestedList factions(tatooine->getObjVars(),
					OBJVAR_GAME_FACTION_RANKINGS);
				for (DynamicVariableList::NestedList::const_iterator i(factions.begin());
					i != factions.end(); ++i)
				{
					int points = 0;
					if (i.getValue(points))
					{
						if (points > factionPoints)
						{
							tie = false;
							factionPoints = points;
							factionName = i.getName();
						}
						else if (points == factionPoints)
							tie = true;
					}
					else
					{
						WARNING(true, ("GCW faction %s value not stored as int",
							i.getName().c_str()));
					}
				}
				if (!tie && factionPoints > 0)
				{
					uint32 factionCrc = Crc::calculate(factionName.c_str());
					if (factionCrc == owner->getPvpFaction())
					{
						amount += static_cast<int>(amount * ConfigServerGame::getGcwXpBonus());
					}
				}
			}

		}

		// find out the xp limit based on the player's skills
		int const limit = getExperienceLimit(experienceType);

		// get the current xp
		int const current = getExperiencePoints(experienceType);

		// calculate the new xp
		int total = current + amount;

		// cap the total xp, if necessary
		if ((total > limit) && (limit >= 0))
			total = limit;

		if (ConfigServerScript::getLogBalance() && (total != current))
		{
			// log the grant
			unsigned long time = ServerClock::getInstance().getGameTimeSeconds();
			LOG("GameBalance", ("balancelog:%d:XP;%lu;%s;%s;%d;%d;%d;%d",
				static_cast<int>(GameServer::getInstance().getProcessId()), time,
				owner->getNetworkId().getValueString().c_str(), experienceType.c_str(),
				amount, current, total, (m_expModified.get()+1)));
		}

		// calculate the actual xp granted
		amountGranted = total - current;

		// only update the XP if it actually changed
		if (total != current)
		{
			m_experiencePoints.set(experienceType, total);
			m_expModified.set(m_expModified.get() + 1);
		}
	}
	else
	{
		owner->sendControllerMessageToAuthServer(CM_grantExperiencePoints, new MessageQueueGenericValueType<std::pair<std::string, int> >(std::make_pair(experienceType, amount)));
	}

	return amountGranted;
}

//-----------------------------------------------------------------------

bool PlayerObject::grantSchematicGroup(const std::string & groupNameWithModifier, bool fromSkill)
{
	if (groupNameWithModifier.size() <= 1 || (groupNameWithModifier[0] != '+' && groupNameWithModifier[0] != '-'))
		return true;

	if (isAuthoritative())
	{
		const std::string & groupName = groupNameWithModifier.substr (1);
		static DraftSchematicGroupManager::SchematicVector drafts;
		drafts.clear ();
		if (!DraftSchematicGroupManager::getSchematicsForGroup (groupName, drafts))
		{
			WARNING (true, ("Invalid schematic group [%s]", groupName.c_str ()));
			return false;
		}

		const bool addGroup = (groupNameWithModifier[0] == '+');

		for (DraftSchematicGroupManager::SchematicVector::const_iterator it = drafts.begin (); it != drafts.end (); ++it)
		{
			const std::pair<uint32, uint32> & schematic = *it;
			if (addGroup)
				grantSchematic(schematic.first, fromSkill);
			else
				revokeSchematic(schematic.first, fromSkill);
		}
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("Did not assign schematic group to proxy object %s", getNetworkId().getValueString().c_str()));
	}
	return true;
}

//-----------------------------------------------------------------------

bool PlayerObject::grantSchematic(uint32 schematicCrc, bool fromSkill)
{
	if (isAuthoritative())
	{
		// if we are crafting, end the crafting session
		if (isCrafting())
			stopCrafting(false);

		const DraftSchematicObject * schematic = DraftSchematicObject::getSchematic(
			schematicCrc);
		if (schematic != nullptr)
		{
			// add the schematic to the player's draft schematic list
			Archive::AutoDeltaMap<std::pair<uint32, uint32>, int>::const_iterator found = m_draftSchematics.find(schematic->getCombinedCrc());
			if (found == m_draftSchematics.end())
			{
				m_draftSchematics.set(schematic->getCombinedCrc(), 1);
				CreatureObject * owner = dynamic_cast<CreatureObject *>(ContainerInterface::getContainedByObject(*this));
				NOT_NULL(owner);
				if (!fromSkill)
				{
					// save schematic as objvar
					char buffer[256];
					sprintf(buffer, "%s.%lu", OBJVAR_NOT_SKILL_SCHEMATICS.c_str(),
						schematicCrc);
					owner->setObjVarItem(buffer, 1);
				}

				// tell scripts the schematic was given
				NOT_NULL(owner->getScriptObject());
				ScriptParams params;
				params.addParam(static_cast<int>(schematic->getCombinedCrc().first));
				params.addParam(fromSkill);
				IGNORE_RETURN(owner->getScriptObject()->trigAllScripts(
					Scripting::TRIG_GRANT_SCHEMATIC, params));
			}
			else if (fromSkill)
			{
				m_draftSchematics.set(schematic->getCombinedCrc(), (*found).second + 1);
			}
		}
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("Did not assign schematic to proxy object %s", getNetworkId().getValueString().c_str()));
	}
	return true;
}

//-----------------------------------------------------------------------

bool PlayerObject::revokeSchematic(uint32 schematicCrc, bool fromSkill)
{
	if (isAuthoritative())
	{
		// if we are crafting, end the crafting session
		if (isCrafting())
			stopCrafting(false);

		const DraftSchematicObject * schematic = DraftSchematicObject::getSchematic(schematicCrc);
		if (schematic != nullptr)
		{
			// remove the schematic from the player's draft schematic list
			Archive::AutoDeltaMap<std::pair<uint32, uint32>, int>::const_iterator found = m_draftSchematics.find(schematic->getCombinedCrc());
			if ((*found).second > 1 && fromSkill)
			{
				m_draftSchematics.set(schematic->getCombinedCrc(), (*found).second - 1);
			}
			else
			{
				WARNING((*found).second > 1, ("PlayerObject::revokeSchematic revoking schematic %s that has a ref count of %d from player %s, due to it being revoked outside the skill system",
					schematic->getObjectTemplateName(), (*found).second, getAccountDescription().c_str()));
				m_draftSchematics.erase(found);
				CreatureObject * owner = dynamic_cast<CreatureObject *>(ContainerInterface::getContainedByObject(*this));
				NOT_NULL(owner);
				if (!fromSkill)
				{
					// remove schematic from objvars
					char buffer[256];
					sprintf(buffer, "%s.%lu", OBJVAR_NOT_SKILL_SCHEMATICS.c_str(),
						schematicCrc);
					owner->removeObjVarItem(buffer);
				}

				// tell scripts the schematic was removed
				NOT_NULL(owner->getScriptObject());
				ScriptParams params;
				params.addParam(static_cast<int>(schematic->getCombinedCrc().first));
				params.addParam(fromSkill);
				IGNORE_RETURN(owner->getScriptObject()->trigAllScripts(
					Scripting::TRIG_REVOKE_SCHEMATIC, params));
			}
		}
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("Did not remove schematic from proxy object %s", getNetworkId().getValueString().c_str()));
	}
	return true;
}

//-----------------------------------------------------------------------

bool PlayerObject::hasSchematic(uint32 schematicCrc)
{
	const DraftSchematicObject * schematic = DraftSchematicObject::getSchematic(schematicCrc);
	if (schematic != nullptr)
	{
		std::map<std::pair<uint32, uint32>,int>::const_iterator found = m_draftSchematics.find(schematic->getCombinedCrc());
		if (found != m_draftSchematics.end())
			return true;
	}
	return false;
}


// ======================================================================
// ======================================================================
//
// Crafting functions
//
// ======================================================================
// ======================================================================

/**
 * Sets the player's crafting tool.
 *
 * @param tool		the crafting tool
 */
void PlayerObject::setCraftingTool(const TangibleObject & tool)
{
	m_craftingTool = CachedNetworkId(tool);
}	// PlayerObject::setCraftingTool

//-----------------------------------------------------------------------

/**
 * Sets the player's crafting station.
 *
 * @param station		the crafting station
 */
void PlayerObject::setCraftingStation(const TangibleObject * station)
{
	if (station != nullptr)
	{
		// verify the crafting station is valid
		if (station->getObjVars().hasItem(OBJVAR_CRAFTING_STATION))
		{
			m_craftingStation = CachedNetworkId(*station);

			// if we are private, make sure we have an ingredient hopper
			int privateStation = 0;
			station->getObjVars().getItem(OBJVAR_PRIVATE_STATION, privateStation);
			if (privateStation == 1 && getCraftingLevel() == 3 && station->getIngredientHopper() == nullptr)
			{
				CreatureObject * const owner = getCreatureObject();
				NOT_NULL(owner);

				WARNING(true, ("Player %s crafting near private station %s that "
					"has no ingredient hopper! The crafting session level will "
					"be reduced to 2",
					owner->getNetworkId().getValueString().c_str(),
					station->getNetworkId().getValueString().c_str()));
				setCraftingLevel(2);
			}
			return;
		}
	}
	m_craftingStation = CachedNetworkId::cms_cachedInvalid;
}	// PlayerObject::setCraftingStation

//-----------------------------------------------------------------------

/**
 * Called when the player wants to start crafting.
 *
 * @param toolId	the crafting tool the player wants to use
 *
 * @return true if the session was started, false if not
 */
bool PlayerObject::requestCraftingSession(const NetworkId & toolId)
{
	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return false;

	const std::string myId        = getNetworkId().getValueString();
	const char * const myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	const std::string toolIdStdString = toolId.getValueString();
	const char * const toolIdString   = toolIdStdString.c_str();
	UNREF(toolIdString);		// needed for release mode

	if (getCraftingTool() != NetworkId::cms_invalid)
	{
		if (m_craftingStage.get() == Crafting::CS_none ||
			m_craftingStage.get() == Crafting::CS_selectDraftSchematic)
		{
			stopCrafting(false);
		}
		else
		{
			DEBUG_WARNING(true, ("Player %s requesting crafting session with tool "
				"%s while crafting with tool %s", myIdString, toolIdString,
				getCraftingTool().getValueString().c_str()));
			return false;
		}
	}

	TangibleObject * const tool = dynamic_cast<TangibleObject *>(CachedNetworkId(toolId).getObject());
	if (tool == nullptr)
	{
		DEBUG_WARNING(true, ("Player %s requesting crafting session with invalid "
			"object %s", myIdString, toolIdString));
		return false;
	}

	// If the tool is a crafting station, try to find a compatible crafting tool to launch.
	if (tool->isCraftingStation())
	{
		int const toolType = tool->getCraftingType();

		ServerObject const * const inventory = owner->getInventory();
		if (inventory)
		{
			VolumeContainer const * const inventoryContainer = ContainerInterface::getVolumeContainer(*inventory);
			if (inventoryContainer)
			{
				for (ContainerConstIterator iter(inventoryContainer->begin()); iter != inventoryContainer->end(); ++iter)
				{
					CachedNetworkId const & objId = (*iter);
					TangibleObject * const obj = safe_cast<TangibleObject *>(objId.getObject());
					if ((obj != nullptr) && obj->isCraftingTool() && (toolType & obj->getCraftingType()))
					{
						return requestCraftingSession(objId) ;
					}
				}
			}
		}
	}
	else
	{
	return tool->startCraftingSession(*owner);
	}

	return false;
}	// PlayerObject::requestCraftingSession

//-----------------------------------------------------------------------

/**
 * Sends a list of draft schematic names that a player may craft with to the
 * player.
 *
 * @param schematicNames		draft schematic template names the player can use
 */
void PlayerObject::sendUseableDraftSchematics(std::vector<uint32> & schematicNames)
{
	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return;

	// keep around the names since we are just going to get an index back from
	// the client; the calling function will free up the old names
	m_useableDraftSchematics.swap(schematicNames);

	MessageQueueDraftSchematics * message = new MessageQueueDraftSchematics(getCraftingTool(), getCraftingStation());

	// get the name of each schematic and put it in the network message
	std::vector<uint32>::iterator iter;
	for (iter = m_useableDraftSchematics.begin(); iter != m_useableDraftSchematics.end();)
	{
		if (*iter != 0)
		{
			const DraftSchematicObject * const schematic = DraftSchematicObject::getSchematic(*iter);
			if (schematic != nullptr)
			{
				message->addSchematic(schematic->getCombinedCrc(),
					static_cast<int>(schematic->getCategory()));
				++iter;
			}
			else
			{
				const ConstCharCrcString & name = ObjectTemplateList::lookUp(*iter);
				if (!name.isEmpty())
				{
					WARNING(true, ("PlayerObject::sendUseableDraftSchematics bad draft "
						"schematic name %s", name.getString()));
				}
				else
				{
					WARNING(true, ("PlayerObject::sendUseableDraftSchematics bad draft "
						"schematic crc %u", *iter));
				}
				iter = m_useableDraftSchematics.erase(iter);
			}
		}
		else
			iter = m_useableDraftSchematics.erase(iter);
	}

	// send the names to the player
	(safe_cast<ServerController *>(owner->getController()))->appendMessage(
		static_cast<int>(CM_draftSchematicsMessage), 0.0f, message,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_CLIENT);

	m_craftingStage = static_cast<int>(Crafting::CS_selectDraftSchematic);
}	// PlayerObject::sendUseableDraftSchematics

//----------------------------------------------------------------------

/**
 * Sends the player info about a draft schematic's slots, filtered by the
 * player's skills.
 *
 * @param draftSchematicCrc  	the crc of the schematic to send
 * @param message				message to be sent to the player if we are getting slots for an object we are crafting
 * @param queryMessage			message to be sent to the player if we are getting slots for an arbitrary object
 *
 * @return true on success, false if draftSchematicName is not a valid schematic
 */
bool PlayerObject::requestDraftSlots(uint32 draftSchematicCrc, MessageQueueDraftSlots * message, MessageQueueDraftSlotsQueryResponse * queryMessage)
{
	ManufactureSchematicObject * manfSchematic = nullptr;
	MessageQueueDraftSlots::Slot slotInfo;
	MessageQueueDraftSlots::Option optionInfo;

	if ((message == nullptr && queryMessage == nullptr) ||
		(message != nullptr && queryMessage != nullptr))
	{
		return false;
	}

	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return false;

	const ConstCharCrcString & schematicName = ObjectTemplateList::lookUp(draftSchematicCrc);
	UNREF(schematicName);

	const DraftSchematicObject * const draftSchematic = DraftSchematicObject::getSchematic(draftSchematicCrc);
	if (draftSchematic == nullptr)
	{
		DEBUG_WARNING(true, ("Player %s requested invalid draft schematic %s", owner->getNetworkId().getValueString().c_str(), schematicName.getString()));
		return false;
	}

	if (message != nullptr)
	{
		m_draftSchematic = draftSchematic;

		TangibleObject * const tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
		NOT_NULL(tool);
		if (tool == nullptr)
			return false;

		tool->clearCraftingManufactureSchematic();
		tool->clearCraftingPrototype();

		// make a temporary manufacturing schematic based on the draft scematic
		manfSchematic = ServerWorld::createNewManufacturingSchematic(CachedNetworkId(
			*owner), *tool, tool->getCraftingManufactureSchematicSlotId(), false);
		if (manfSchematic == nullptr)
		{
			DEBUG_WARNING(true, ("Error creating manufacturing schematic!"));
			return false;
		}
		message->setManfSchemId(manfSchematic->getNetworkId ());
		tool->setCraftingManufactureSchematic(*manfSchematic);

		// flag that the schematic is being used in a crafting session
		manfSchematic->setObjVarItem(OBJVAR_IN_CRAFTING_SESSION, 1);

		// reset the experiment points
		m_experimentPoints = -1;

		// create a test prototype so the client will know what object is
		// being made
		ServerObject * prototype = manfSchematic->manufactureObject(owner->getNetworkId(),
			*tool, tool->getCraftingPrototypeSlotId(), true);
		if (prototype == nullptr)
		{
			DEBUG_WARNING(true, ("Error creating temp prototype!"));
			return false;
		}
		tool->setCraftingPrototype(*prototype);
		message->setPrototypeId(prototype->getNetworkId());
		message->setVolume(draftSchematic->getVolume());
		message->setCanManufacture(draftSchematic->getItemsPerContainer() > 0);

		DEBUG_LOG("crafting", ("PlayerObject::requestDraftSlots, created schematic "
			"%s, prototype %s for player %s",
			manfSchematic->getNetworkId().getValueString().c_str(),
			prototype->getNetworkId().getValueString().c_str(),
			getAccountDescription().c_str()));
	}

	if (queryMessage != nullptr)
	{
		queryMessage->setComplexity(static_cast<int>(draftSchematic->getComplexity()));
		queryMessage->setVolume(draftSchematic->getVolume());
		queryMessage->setCanManufacture(draftSchematic->getItemsPerContainer() > 0);
	}

	// get info about each slot in the schematic
	Crafting::IngredientSlot manfSlot;
	ServerDraftSchematicObjectTemplate::IngredientSlot slot;
	const int slotCount = draftSchematic->getSlotsCount();
	for (int i = 0; i < slotCount; ++i)
	{
		if (!draftSchematic->getSlot(slot, i))
		{
			DEBUG_WARNING(true, ("draft schematic slot %d for schematic %s not found!",i, schematicName.getString()));
			continue;
		}
		if (!slot.optional || slot.optionalSkillCommand.empty() ||
			owner->hasCommand(slot.optionalSkillCommand))
		{
			if (manfSchematic != nullptr)
			{
				// create the slot
				IGNORE_RETURN(manfSchematic->getSlot(slot.name, manfSlot));
				// set the manufacturing slot->draft slot map
				manfSchematic->setSlotIndex(slot.name, i);
			}

			slotInfo.name = slot.name;
			slotInfo.optional = slot.optional;
			bool componentSlot = false;
			const int optionsCount = static_cast<int>(slot.options.size());
			for (int j = 0; j < optionsCount; ++j)
			{
				const ServerIntangibleObjectTemplate::Ingredient & option = slot.options[static_cast<unsigned int>(j)];

				if (option.skillCommand.empty() || owner->hasCommand(option.skillCommand))
				{
					const ServerIntangibleObjectTemplate::SimpleIngredient & simple = option.ingredients[0];

					switch (option.ingredientType)
					{
					case Crafting::IT_template:
					case Crafting::IT_templateGeneric:
						{
							// get the default object name from the shared template
							const ObjectTemplate *const ingredientTemplate = ObjectTemplateList::fetch(simple.ingredient);
							if (ingredientTemplate == nullptr)
							{
								DEBUG_WARNING(true, ("draft schematic component ingredient %s not found!", simple.ingredient.c_str()));
								continue;
							}
							const SharedObjectTemplate * const sharedTemplate = safe_cast<const SharedObjectTemplate *>(ObjectTemplateList::fetch((safe_cast<const ServerObjectTemplate *>(ingredientTemplate))->getSharedTemplate()));
							if (sharedTemplate == nullptr)
							{
								DEBUG_WARNING(true, ("draft schematic component ingredient %s shared template not found!", simple.ingredient.c_str()));
								continue;
							}
							optionInfo.ingredient = Unicode::narrowToWide(sharedTemplate->DataResource::getName());
							sharedTemplate->releaseReference();
							ingredientTemplate->releaseReference();
						}
						break;
					case Crafting::IT_schematic:
					case Crafting::IT_schematicGeneric:
						{
							// get the default object name from the shared template of the crafted object
							const ObjectTemplate * const ingredientTemplate = ObjectTemplateList::fetch(simple.ingredient);
							if (ingredientTemplate == nullptr)
							{
								DEBUG_WARNING(true, ("draft schematic schematic ingredient %s not found!", simple.ingredient.c_str()));
								continue;
							}
							const ServerObjectTemplate * const craftedTemplate = safe_cast<const ServerDraftSchematicObjectTemplate *>(ingredientTemplate)->getCraftedObjectTemplate();
							if (craftedTemplate == nullptr)
							{
								DEBUG_WARNING(true, ("draft schematic schematic ingredient %s not found!", simple.ingredient.c_str()));
								continue;
							}
							const SharedObjectTemplate * const sharedTemplate = safe_cast<const SharedObjectTemplate *>(ObjectTemplateList::fetch((safe_cast<const ServerObjectTemplate *>(craftedTemplate))->getSharedTemplate()));
							if (sharedTemplate == nullptr)
							{
								DEBUG_WARNING(true, ("draft schematic component ingredient %s shared template not found!", simple.ingredient.c_str()));
								continue;
							}
							optionInfo.ingredient = Unicode::narrowToWide(sharedTemplate->DataResource::getName());
							sharedTemplate->releaseReference();
							craftedTemplate->releaseReference();
							ingredientTemplate->releaseReference();
						}
						break;
					default:
						optionInfo.ingredient = Unicode::narrowToWide(simple.ingredient);
						break;
					}
					optionInfo.name = simple.name;
					optionInfo.type = static_cast<Crafting::IngredientType>(static_cast<int>(option.ingredientType));
					optionInfo.amountNeeded = simple.count;
					slotInfo.options.push_back(optionInfo);
					if (static_cast<int>(option.ingredientType) == static_cast<int>(Crafting::IT_template) ||
						static_cast<int>(option.ingredientType) == static_cast<int>(Crafting::IT_templateGeneric) ||
						static_cast<int>(option.ingredientType) == static_cast<int>(Crafting::IT_item) ||
						static_cast<int>(option.ingredientType) == static_cast<int>(Crafting::IT_schematic) ||
						static_cast<int>(option.ingredientType) == static_cast<int>(Crafting::IT_schematicGeneric))
					{
						componentSlot = true;
					}
				}	// if (hasCommand(option.skillCommand))
			}	// for (int j = 0; j < optionsCount; ++j)
			if (componentSlot)
				slotInfo.hardpoint = slot.appearance;
			if (message != nullptr)
				message->addSlot(slotInfo);
			else
				queryMessage->addSlot(slotInfo);
			slotInfo.options.clear();
		}	// if (hasCommand(slot.skillCommand))
	}	// for (int i = 0; i < slotCount; ++i)

	// send the slot info to the player
	if (message != nullptr)
	{
		(safe_cast<ServerController *>(owner->getController()))->appendMessage(
			static_cast<int>(CM_draftSlotsMessage), 0.0f, message,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
	else
	{
		(safe_cast<ServerController *>(owner->getController()))->appendMessage(
			static_cast<int>(CM_draftSlotsQueryResponse), 0.0f, queryMessage,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
	return true;
}	// PlayerObject::requestDraftSlots

//----------------------------------------------------------------------

/**
 * Called when a player has selected a draft schematic to use in a crafting
 * tool. We send the player info about the slots that need to be filled
 * in the schematic, filtered by his skills.
 *
 * @param index		index into the available schematics the player can use. The
 *					index will be clipped if it is out of range.
 */
void PlayerObject::selectDraftSchematic(int index)
{
	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return;

	const std::string myId        = getNetworkId().getValueString();
	const char * const myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	// check the crafting tool
	if (getCraftingTool() == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Player %s trying to select draft schematic with "
			"no crafting tool!", myIdString));
	}
	else if (m_craftingStage.get() != static_cast<int>(Crafting::CS_selectDraftSchematic))
	{
		DEBUG_WARNING(true, ("Player %s trying to select draft schematic in wrong "
			"crafting stage %d!", myIdString, static_cast<int>(m_craftingStage.get())));
	}
	else
	{
		// check the available draft schematics
		if (m_useableDraftSchematics.empty ())
		{
			DEBUG_WARNING(true, ("Player %s selected a draft schematic to craft "
				"when none are available!", myIdString));
		}
		else
		{
			if (index < 0)
				index = 0;
			else if (index >= static_cast<int>(m_useableDraftSchematics.size()))
				index = static_cast<int>(m_useableDraftSchematics.size()) - 1;

			MessageQueueDraftSlots * message = new MessageQueueDraftSlots(
				getCraftingTool(), NetworkId::cms_invalid);
			if (requestDraftSlots(m_useableDraftSchematics[static_cast<unsigned int>(index)], message, nullptr))
			{
				m_craftingStage = static_cast<int>(Crafting::CS_assembly);
				m_craftingComponentBioLink = NetworkId::cms_invalid;
			}
			else
				delete message;
		}
	}
}	// PlayerObject::selectDraftSchematic

//----------------------------------------------------------------------

/**
 * Tries to fill the current manufacturing schematic with an ingredient in the player's
 * inventory or the crafting station's input hopper.
 *
 * @param slotIndex			index of the slot to fill
 * @param slotOptionIndex	index of the slot option to use
 * @param ingredientId		id of the ingredient to use
 *
 * @return CE_success on success, error code if the ingredient wasn't added
 */
Crafting::CraftingError PlayerObject::fillSlot(int slotIndex, int slotOptionIndex, const NetworkId & ingredientId)
{
	std::string myId = getNetworkId().getValueString();
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return Crafting::CE_noOwner;

	// check the draft schematic
	if (m_draftSchematic.getPointer() == nullptr)
	{
		WARNING(true, ("Player %s trying to fill slot with no draft schematic!", myIdString));
		return Crafting::CE_noDraftSchematic;
	}

	if (m_draftSchematic->getGameObjectType() == SharedObjectTemplate::GOT_data_draft_schematic_read_only)
	{
		return Crafting::CE_readOnlyDraftSchematic;
	}

	if (getCraftingStage() != Crafting::CS_assembly)
	{
		WARNING(true, ("Player %s trying to fill slot when not in assembly stage! "
			"Stage = %d, schematic = %s, ingredient = %s, slot = %d", myIdString,
			getCraftingStage(), m_draftSchematic->getTemplateName(),
			ingredientId.getValueString().c_str(), slotIndex));
		return Crafting::CE_notAssemblyStage;
	}

	// check the crafting tool
	if (getCraftingTool() == NetworkId::cms_invalid)
	{
		WARNING(true, ("Player %s trying to fill slot with no crafting tool!", myIdString));
		return Crafting::CE_noCraftingTool;
	}

	TangibleObject * const tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
	if (tool == nullptr)
	{
		WARNING(true, ("Player %s trying to fill slot with crafting tool %s, but tool is nullptr!", myIdString, getCraftingTool().getValueString().c_str()));
		return Crafting::CE_noCraftingTool;
	}

	// get the crafting station (if any)
	TangibleObject * const station = dynamic_cast<TangibleObject *>(getCraftingStation().getObject());

	// check the manufacturing schematic
	ManufactureSchematicObject * const manfSchematic = tool->getCraftingManufactureSchematic();
	if (manfSchematic == nullptr)
	{
		WARNING(true, ("Player %s trying to fill slot with no manufacturing schematic!", myIdString));
		return Crafting::CE_noManfSchematic;
	}

	// verify the slot index
	if (slotIndex < 0 || slotIndex >= manfSchematic->getSlotsCount())
	{
		WARNING(true, ("Player %s trying to fill invalid slot %d",myIdString, slotIndex));
		return Crafting::CE_invalidSlot;
	}

	// get the manufacturing schematic object
	Crafting::IngredientSlot manfSlot;
	if (!manfSchematic->getSlot(slotIndex, manfSlot, false))
	{
		WARNING(true, ("Got invalid slot %d result for manf schematic %s, but we were told it was ok! What's going on?", slotIndex, manfSchematic->getNetworkId().getValueString().c_str()));
		return Crafting::CE_invalidSlot;
	}

	// get the draft schematic slot
	ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
	if (!m_draftSchematic->getSlot(draftSlot, manfSlot.draftSlotIndex))
	{
		return Crafting::CE_invalidSlot;
	}

	// get the draft schematic option, convert the passed-in option index to the
	// unfiltered index
	int i, j;
	const ServerIntangibleObjectTemplate::Ingredient * slotOption = nullptr;
	const int optionsCount = static_cast<int>(draftSlot.options.size());
	for (i = 0, j = 0; i < optionsCount; ++i)
	{
		const ServerIntangibleObjectTemplate::Ingredient & tempOption = draftSlot.options[static_cast<unsigned int>(i)];
		if (tempOption.skillCommand.empty() || owner->hasCommand(tempOption.skillCommand))
		{
			if (j == slotOptionIndex)
			{
				slotOption = &tempOption;
				break;
			}
			else
				++j;
		}
	}
	if (i == optionsCount)
	{
		WARNING(true, ("Player %s selected invalid slot option %d for slot %d", myIdString, slotOptionIndex, slotIndex));
		return Crafting::CE_invalidSlotOption;
	}
	slotOptionIndex = i;

	if (slotOption == nullptr || slotOption->ingredients.size() != 1)
	{
		WARNING(true, ("slotOption is nullptr or ingredient %d for draft schematic %s, slot %s, has ingredient count != 1",
			slotOptionIndex, m_draftSchematic->getTemplateName(), draftSlot.name.getText().c_str()));
		return Crafting::CE_invalidIngredientSize;
	}
	const ServerIntangibleObjectTemplate::SimpleIngredient & slotIngredient = slotOption->ingredients[0];

	// see if the slot is already filled - also get how many ingredients are
	// needed and how many we already have
	int neededIngredientCount = slotIngredient.count;
	int currentIngredientCount = 0;
	if (manfSlot.ingredientType != Crafting::IT_none)
	{
		// the slot has something in it, see if it is full or not
		for (Crafting::Ingredients::const_iterator iter =
			manfSlot.ingredients.begin(); iter != manfSlot.ingredients.end(); ++iter)
		{
			currentIngredientCount += (*iter)->count;
		}
		if (currentIngredientCount >= neededIngredientCount)
		{
			DEBUG_WARNING(currentIngredientCount == neededIngredientCount, (
				"Player %s trying to fill filled slot %d", myIdString, slotIndex));
			WARNING_STRICT_FATAL(currentIngredientCount > neededIngredientCount,
				("HEY! Player %s trying to fill OVER filled slot %d (required = "
				"%d, current = %d)", myIdString, slotIndex, neededIngredientCount,
				currentIngredientCount));
			return Crafting::CE_slotFull;
		}
	}
	int numIngredientsToAdd = neededIngredientCount - currentIngredientCount;

	TangibleObject * const ingredient = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(ingredientId));
	if (ingredient == nullptr)
	{
		WARNING(true, ("No object for ingredient id %s", ingredientId.getValueString().c_str()));
		return Crafting::CE_invalidIngredient;
	}

	// check if the ingredient is bio-linked
	NetworkId bioLinkId(ingredient->getBioLink());
	if (bioLinkId != NetworkId::cms_invalid)
	{
		if (m_craftingComponentBioLink.get() == NetworkId::cms_invalid)
		{
			// this component defines the bio-link for the object
			m_craftingComponentBioLink = bioLinkId;
		}
		else
		{
			return Crafting::CE_invalidBioLink;
		}
	}
	else
		m_craftingComponentBioLink = NetworkId::cms_invalid;

	// see if the ingredient is in the player's inventory
	if (!owner->isIngredientInInventory(*ingredient))
	{
		// if we are crafting level 3, also check the crafting station's hopper
		if (getCraftingLevel() != 3 || (station != nullptr &&
			!station->isIngredientInHopper(ingredientId)))
		{
			DEBUG_WARNING(true, ("Player %s chose invalid ingredient %s", myIdString, ingredientId.getValueString().c_str()));
			return Crafting::CE_ingredientNotInInventory;
		}

	}

	// see if the ingredient is a resource or component
	ResourceContainerObject * const crate = dynamic_cast<ResourceContainerObject *>(ingredient);
	if (crate != nullptr)
	{
		// get the resource type name and class name
		ResourceTypeObject const * const resource = crate->getResourceType();
		NetworkId const resourceId = resource ? resource->getNetworkId() : NetworkId::cms_invalid;

		bool slotOk = false;

		if (resource)
		{
			// make sure the selected slot is the right type
			if (currentIngredientCount == 0)
			{
				if (slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_resourceType)
				{
					const std::string typeName(resource->getResourceName());
					if (typeName == slotIngredient.ingredient)
						slotOk = true;
				}
				else if (slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_resourceClass)
				{
					ResourceClassObject const * resourceClass = &(resource->getParentClass());
					while (resourceClass != nullptr)
					{
						const std::string className (resourceClass->getResourceClassName());
						if (className == slotIngredient.ingredient)
						{
							slotOk = true;
							break;
						}
						resourceClass = resourceClass->getParent();
					}
				}
			}
			else
			{
				// the resource type must be the exact one currently in the slot
				if (resource->getNetworkId() == manfSlot.ingredients.front()->ingredient)
					slotOk = true;
			}
		}

		if (slotOk)
		{
			// if we need more resources than we have, just empty the crate
			if (crate->getQuantity() < numIngredientsToAdd)
				numIngredientsToAdd = crate->getQuantity();

			// we can put the resource in the manufacturing schematic
			std::vector<std::pair<NetworkId, int> > sources;
			if (crate->removeResource(resourceId, numIngredientsToAdd, &sources))
			{
				if (currentIngredientCount == 0)
				{
					manfSchematic->setSlotOption(manfSlot.name, slotOptionIndex);
					manfSchematic->modifySlotComplexity(manfSlot.name, slotOption->complexity);
					manfSchematic->setSlotType(manfSlot.name, static_cast<int>(slotOption->ingredientType));
				}
				int count = sources.size();
				for (int i = 0; i < count; ++i)
				{
					manfSchematic->addSlotResource(manfSlot.name, resourceId, sources[i].second, sources[i].first);
					manfSchematic->addIngredient(resourceId, sources[i].second, sources[i].first);
				}
			}
			else
			{
				return Crafting::CE_cantRemoveResource;
			}
		}
		else
		{
			DEBUG_WARNING(true, ("Invalid resource ingredient for draft schematic %s, slot %s, option %d",
				m_draftSchematic->getTemplateName(), draftSlot.name.getText().c_str(), slotOptionIndex));
			return Crafting::CE_wrongResource;
		}
	}
	else
	{
		if (manfSchematic->hasIngredient (ingredient->getNetworkId ()))
		{
			DEBUG_WARNING(true, ("Attempt to add ingredient multiple times."));
			return Crafting::CE_invalidIngredient;
		}

		// see if the ingredient is a stacked loot item
		// @todo: stacked loot should work the same as FactoryObject objects
		if (ingredient->getScriptObject()->hasScript("object.onewayunstack"))
		{
			return Crafting::CE_stackedLoot;
		}

		// test if the component is a factory
		FactoryObject * factory = dynamic_cast<FactoryObject *>(ingredient);

		// if this is an old-style factory, we can't use it directly in crafting
		if (factory != nullptr && !factory->getLoadContents())
			factory = nullptr;

		// make sure the component isn't damaged
		if (factory == nullptr && ingredient->getDamageTaken() != 0 &&
			!ingredient->getObjVars().hasItem(OBJVAR_CRAFTING_COMPONENT_CAN_BE_DAMAGED))
		{
			DEBUG_WARNING(true, ("Tried to use damaged component %s for draft schematic %s, slot %s, option %d",
				ingredientId.getValueString().c_str(),
				m_draftSchematic->getTemplateName(), draftSlot.name.getText().c_str(),
				slotOptionIndex));
			return Crafting::CE_damagedComponent;
		}

		// make sure the ingredient is crafted, if required
		if ((slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_schematic ||
			slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_schematicGeneric) &&
		    !ingredient->isCrafted())
		{
			DEBUG_WARNING(true, ("Invalid component ingredient for draft schematic %s, slot %s, option %d",
				m_draftSchematic->getTemplateName(), draftSlot.name.getText().c_str(), slotOptionIndex));
			return Crafting::CE_wrongComponent;
		}

		// find the matching ingredient for the draft slot
		bool slotOk = false;
		if (currentIngredientCount == 0)
		{
			if (slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_item)
			{
				std::string tempName(Unicode::wideToNarrow(ingredient->getObjectName()));
				if (tempName == slotIngredient.ingredient)
				{
					slotOk = true;
				}
			}
			// for slots that require multiple non-generic components, make
			// sure the component being added is crafted
			else if (slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_templateGeneric ||
	            ((slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_template ||
				slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_schematic ||
				slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_schematicGeneric) &&
		        ingredient->isCrafted()))
			{
				std::string requiredIngredientName(slotIngredient.ingredient);

				// check the ingredient template name for every template in it's
				// heirarchy
				const ObjectTemplate * testTemplate = nullptr;
				if (slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_template ||
					slotOption->ingredientType == ServerIntangibleObjectTemplate::IT_templateGeneric)
				{
					if (factory == nullptr)
						testTemplate = ingredient->getObjectTemplate();
					else
						testTemplate = factory->getContainedObjectTemplate();
				}
				else
				{
					const DraftSchematicObject * itemSchematic = nullptr;
					if (factory == nullptr)
						itemSchematic = DraftSchematicObject::getSchematic(ingredient->getSourceDraftSchematic());
					else
						itemSchematic = DraftSchematicObject::getSchematic(factory->getDraftSchematic());
					if (itemSchematic != nullptr)
						testTemplate = itemSchematic->getObjectTemplate();
				}

				while (testTemplate != nullptr)
				{
					const std::string ingredientName(testTemplate->getName());
					if (ingredientName == requiredIngredientName)
					{
						slotOk = true;
						break;
					}
					else
					{
						// travel down the ingredient template heirarchy
						testTemplate = testTemplate->getBaseTemplate();
					}
				}
			}
		}
		else
		{
			// the ingredient must either be from the same template, or from the
			// same manufacturing schematic as the ingredient currently in the slot;
			// this is a configureable setting
			const Crafting::ComponentIngredient * testComponent = dynamic_cast<
				const Crafting::ComponentIngredient *>(manfSlot.ingredients.front().get());
			if (testComponent != nullptr)
			{
				if (ConfigServerGame::getCraftingComponentStrict() &&
					(slotOption->ingredientType != ServerIntangibleObjectTemplate::IT_templateGeneric &&
					slotOption->ingredientType != ServerIntangibleObjectTemplate::IT_schematicGeneric))
				{
					// component from same manf schematic
					if (ingredient->getCraftedId() == testComponent->ingredient)
						slotOk = true;
				}
				else
				{
					// component from same template
					if (factory == nullptr)
					{
						if (ingredient->getObjectTemplateName() != nullptr &&
							ingredient->getObjectTemplateName() == testComponent->templateName)
						{
							slotOk = true;
						}
					}
					else if (factory->getContainedTemplateName() != nullptr &&
						factory->getContainedTemplateName() == testComponent->templateName)
					{
						slotOk = true;
					}
				}
			}
		}
		if (slotOk)
		{
			if (factory == nullptr || factory->getCount() <= numIngredientsToAdd)
			{
				// adding normal component or a complete factory object
				if (manfSchematic->addIngredient(*ingredient))
				{
					if (manfSlot.ingredientType == Crafting::IT_none)
					{
						manfSchematic->setSlotType(manfSlot.name, static_cast<int>(slotOption->ingredientType));
						manfSchematic->setSlotOption(manfSlot.name, slotOptionIndex);
						manfSchematic->modifySlotComplexity(manfSlot.name, slotOption->complexity);
					}
					if (factory == nullptr)
					{
						manfSchematic->addSlotComponent(manfSlot.name, *ingredient,
							slotOption->ingredientType);
					}
					else
					{
						int count = factory->getCount();
						for (i = 0; i < count; ++i)
						{
							manfSchematic->addSlotComponent(manfSlot.name,
								*factory, slotOption->ingredientType);
						}
					}
				}
				else
					return Crafting::CE_cantTransferComponent;
			}
			else
			{
				// adding component(s) from a factory
				if (!factory->inCraftingSession())
				{
					if (!factory->startCraftingSession(*manfSchematic))
						return Crafting::CE_cantTransferComponent;
				}
				if (!factory->removeCraftingReferences(numIngredientsToAdd))
				{
					return Crafting::CE_cantTransferComponent;
				}
				if (manfSlot.ingredientType == Crafting::IT_none)
				{
					manfSchematic->setSlotType(manfSlot.name, static_cast<int>(slotOption->ingredientType));
					manfSchematic->setSlotOption(manfSlot.name, slotOptionIndex);
					manfSchematic->modifySlotComplexity(manfSlot.name, slotOption->complexity);
				}
				for (i = 0; i < numIngredientsToAdd; ++i)
				{
					manfSchematic->addSlotComponent(manfSlot.name, *factory,
						slotOption->ingredientType);
				}
			}
		}
		else
		{
			DEBUG_WARNING(true, ("Invalid component ingredient for draft schematic %s, slot %s, option %d",
				m_draftSchematic->getTemplateName(), draftSlot.name.getText().c_str(), slotOptionIndex));
			return Crafting::CE_wrongComponent;
		}
	}

	manfSchematic->computeComplexity();
	manfSchematic->signalClientChange();
	return Crafting::CE_success;
}	// PlayerObject::fillSlot

//----------------------------------------------------------------------

/**
 * Removes the ingredients in a schematic slot and returns them to the crafting
 * tool's ingredient hopper.
 *
 * @param slotIndex		    schematic slot to empty
 * @param targetContainerId the container the player wants to unload the slot into.
 *                          If NetworkId::cms_invalid, then unload the slot into
 *                          the same container it was loaded from.
 *
 * @return CE_success on success, error code if the ingredient wasn't removed
 */
Crafting::CraftingError PlayerObject::emptySlot(int slotIndex, const NetworkId & targetContainerId)
{
	const std::string & myId = getNetworkId().getValueString();
	const char * const myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return Crafting::CE_noOwner;

	if (getCraftingStage() != Crafting::CS_assembly && !m_allowEmptySlot)
	{
		WARNING(true, ("Player %s trying to empty slot when not in assembly stage!", myIdString));
		return Crafting::CE_notAssemblyStage;
	}

	// check the draft schematic
	if (m_draftSchematic.getPointer() == nullptr)
	{
		WARNING(true, ("Player %s trying to empty slot with no draft schematic!", myIdString));
		return Crafting::CE_noDraftSchematic;
	}

	// check the crafting tool
	if (getCraftingTool() == NetworkId::cms_invalid)
	{
		WARNING(true, ("Player %s trying to empty slot with no crafting tool!", myIdString));
		return Crafting::CE_noCraftingTool;
	}
	TangibleObject * const tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
	if (tool == nullptr)
	{
		WARNING(true, ("Player %s trying to empty slot with crafting tool %s, but tool is nullptr!", myIdString, getCraftingTool().getValueString().c_str()));
		return Crafting::CE_noCraftingTool;
	}

	// get the crafting station (if any)
	TangibleObject * station = dynamic_cast<TangibleObject *>(getCraftingStation().getObject());
	if (station != nullptr && getCraftingLevel() == 3 && station->getIngredientHopper() == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("Player %s trying to empty crafting slot near "
			"a crafting station %s that has no ingredient hopper!",
			owner->getNetworkId().getValueString().c_str(),
			station->getNetworkId().getValueString().c_str()));
		return Crafting::CE_badTargetHopper;
	}

	ServerObject * const inventory = owner->getInventory();
	if (inventory == nullptr)
	{
		WARNING (true, ("Player %s has no inventory.", myIdString));
		return Crafting::CE_noInventory;
	}

	//-- if the player specified a target container, try to use that
	ServerObject * targetContainer = 0;
	if (targetContainerId.isValid ())
	{
		if (!isNestedInContainer (targetContainerId, inventory->getNetworkId()))
		{
			// check worn containers, such as backpacks, but only check one level down so we don't catch the datapad
			bool containerIsWorn = false;
			Object const * const possibleContainer = NetworkIdManager::getObjectById (targetContainerId);
			if (possibleContainer)
			{
				containerIsWorn = owner->isItemEquipped (*possibleContainer);
			}

			// if we are crafting level 3, also check the crafting station's hopper
			if (!containerIsWorn &&
				(getCraftingLevel() != 3 || (station != nullptr &&
				!isNestedInContainer (targetContainerId, station->getIngredientHopper()->getNetworkId()))))
			{
				WARNING (true, ("Player %s attempted to empty slot into invalid container [%s].", myIdString, targetContainerId.getValueString().c_str()));
				return Crafting::CE_badTargetHopper;
			}
		}

		targetContainer = safe_cast<ServerObject *>(NetworkIdManager::getObjectById (targetContainerId));

		if (targetContainer == nullptr)
		{
			WARNING (true, ("Player %s attempted to empty slot into non-existant container [%s].", myIdString, targetContainerId.getValueString ().c_str ()));
			return Crafting::CE_badTargetContainer;
		}
	}
	//-- otherwise use the container that the ingredient came from in the first place
	else
	{
		//@todo: this needs to find the target container from which the slot was originally populated
		targetContainer = inventory;
	}

	// check the manufacturing schematic
	ManufactureSchematicObject * const manfSchematic = tool->getCraftingManufactureSchematic();
	if (manfSchematic == nullptr)
	{
		WARNING(true, ("Player %s trying to empty slot with no manufacturing schematic!", myIdString));
		return Crafting::CE_noManfSchematic;
	}

	if (manfSchematic->mustDestroyIngredients() &&
		getCraftingStage() != Crafting::CS_assembly)
	{
		WARNING(true, ("Player %s trying to empty slot when not in assembly stage!", myIdString));
		return Crafting::CE_notAssemblyStage;
	}

	// verify the slot index
	if (slotIndex < 0 || slotIndex >= manfSchematic->getSlotsCount())
	{
		WARNING(true, ("Player %s trying to empty invalid slot %d", myIdString, slotIndex));
		return Crafting::CE_invalidSlot;
	}

	// get the manufacturing schematic object
	Crafting::IngredientSlot manfSlot;
	if (!manfSchematic->getSlot(slotIndex, manfSlot, true))
	{
		return Crafting::CE_invalidSlot;
	}
	Crafting::IngredientType ingredientType = manfSlot.ingredientType;
	if (ingredientType == Crafting::IT_none || manfSlot.ingredients.empty())
	{
		DEBUG_WARNING(true, ("Player %s trying to empty already empty slot %d", myIdString, slotIndex));
		return Crafting::CE_emptySlot;
	}

	// get the draft schematic slot
	ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
	if (!m_draftSchematic->getSlot(draftSlot, manfSlot.draftSlotIndex))
	{
		return Crafting::CE_invalidSlot;
	}

	if (ingredientType == Crafting::IT_item || ingredientType == Crafting::IT_template ||
		ingredientType == Crafting::IT_templateGeneric || ingredientType == Crafting::IT_schematic ||
		ingredientType == Crafting::IT_schematicGeneric)
	{
		// ingredient is a component
		Crafting::Ingredients::const_iterator iter;
		for (iter = manfSlot.ingredients.begin(); iter != manfSlot.ingredients.end();
			++iter)
		{
			const Crafting::ComponentIngredient * const componentInfo = dynamic_cast<const Crafting::ComponentIngredient *>((*iter).get());
			NOT_NULL(componentInfo);
			TangibleObject * const component = manfSchematic->getComponent(*componentInfo);
			if (component != nullptr)
			{
				// move the component from the schematic to the player
				if (!manfSchematic->removeIngredient(*component, *targetContainer))
				{
					Unicode::String oob;
					{
						ProsePackage pp;
						ProsePackageManagerServer::createSimpleProsePackage (*component, COMPONENT_LOST_MSG, pp);
						OutOfBandPackager::pack(pp, -1, oob);
					}
					Chat::sendSystemMessage(*owner, Unicode::emptyString, oob);
				}
				// if this component is bio-linked, clear our link data
				NetworkId bioLinkId(component->getBioLink());
				if (bioLinkId != NetworkId::cms_invalid && bioLinkId == m_craftingComponentBioLink.get())
				{
					m_craftingComponentBioLink = NetworkId::cms_invalid;
				}
			}
		}
	}
	else
	{
		// ingredient is a resource
		VolumeContainer * const targetVolumeContainer = ContainerInterface::getVolumeContainer(*targetContainer);
		if (targetVolumeContainer == nullptr)
		{
			DEBUG_WARNING(true, ("PlayerObject::emptySlot: targetContainer %s does not have a volume container",
								 targetContainer->getNetworkId().getValueString().c_str()));
			return Crafting::CE_badTargetContainer;
		}
		Crafting::Ingredients::const_iterator iter;
		for (iter = manfSlot.ingredients.begin(); iter != manfSlot.ingredients.end();
			++iter)
		{
			Crafting::SimpleIngredient * resource = const_cast<Crafting::SimpleIngredient *>((*iter).get());
			const NetworkId resourceId(resource->ingredient);
			if (resourceId != NetworkId::cms_invalid)
			{
				// find a resource crate in the targetContainer containing the resourceId
				ContainerIterator iter2;
				for (iter2 = targetVolumeContainer->begin(); iter2 != targetVolumeContainer->end();
					++iter2)
				{
					if (*iter2 != NetworkId::cms_invalid)
					{
						ResourceContainerObject * crate = dynamic_cast<
							ResourceContainerObject *>(NetworkIdManager::getObjectById((*iter2)));
						if (crate != nullptr && crate->getResourceTypeId() == resourceId)
						{
							int emptySpace = crate->getMaxQuantity() - crate->getQuantity();
							if (emptySpace > 0)
							{
								if (emptySpace >= resource->count)
								{
									// we can put all of the resource in the crate
									IGNORE_RETURN(manfSchematic->removeIngredient(
										resourceId, resource->count, *crate));
									break;
								}
								else
								{
									// fill up the crate an keep looking
									IGNORE_RETURN(manfSchematic->removeIngredient(
										resourceId, emptySpace, *crate));
									resource->count -= emptySpace;
								}
							}
						}
					}
				}
				if (iter2 == targetVolumeContainer->end())
				{
					// create a new crate(s) in the target container
					std::string crateTemplateName;
					ResourceTypeObject const * const resourceType = ServerUniverse::getInstance().getResourceTypeById(resourceId);
					resourceType->getCrateTemplate(crateTemplateName);
					while (resource->count > 0)
					{
						ServerObject * crateObject = ServerWorld::createNewObject(
							crateTemplateName, *targetContainer, true);
						if (crateObject == nullptr)
						{
							// @todo: inform the player
							DEBUG_WARNING(true, ("PlayerObject::emptySlot tried to "
								"create a new resource crate %s in container %s but "
								"failed!", crateTemplateName.c_str(),
								targetContainerId.getValueString().c_str()));
							return Crafting::CE_cantCreateResourceCrate;
						}
						ResourceContainerObject * crate = safe_cast<
							ResourceContainerObject *>(crateObject);
						if (crate->getMaxQuantity() >= resource->count)
						{
							IGNORE_RETURN(manfSchematic->removeIngredient(resourceId,
								resource->count, *crate));
							resource->count = 0;
						}
						else
						{
							IGNORE_RETURN(manfSchematic->removeIngredient(resourceId,
								crate->getMaxQuantity(), *crate));
							resource->count -= crate->getMaxQuantity();
						}
					}
				}
			}
		}
	}

	// empty the slot
	manfSchematic->clearSlot(draftSlot.name);

	manfSchematic->computeComplexity();
	manfSchematic->signalClientChange();

	return Crafting::CE_success;
}	// PlayerObject::emptySlot

//----------------------------------------------------------------------

/**
 * Goes from the current stage in a crafting session to the next one.
 *
 * @return error code
 */
int PlayerObject::goToNextCraftingStage ()
{
	int result = Crafting::CR_failure;

	switch (m_craftingStage.get())
	{
		case Crafting::CS_assembly:
			result = startCraftingExperiment();
			break;
		case Crafting::CS_experiment:
			m_craftingStage = static_cast<int>(Crafting::CS_customize);
			result = Crafting::CR_success;
			break;
		case Crafting::CS_customize:
			m_craftingStage = static_cast<int>(Crafting::CS_finish);
			result = Crafting::CR_success;
			break;
		case Crafting::CS_none:
		case Crafting::CS_selectDraftSchematic:
		case Crafting::CS_finish:
		default:
			break;
	}
	return result;
}	// PlayerObject::goToNextCraftingStage

//----------------------------------------------------------------------

/**
 * Moves a crafting session to the experimentation phase. This only occurs if
 * all non-optional slots of the draft schematic have been filled. The ingredients
 * used in the schematic will be destroyed unless the draft schematic is flagged
 * as non-destroying.
 *
 * @return returns the success state of the assembly on success, -error code on fail
 */
int PlayerObject::startCraftingExperiment ()
{
	int i;

	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return -Crafting::CE_noOwner;

	std::string myId = getNetworkId().getValueString();
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	if (m_draftSchematic.getPointer() == nullptr)
	{
		DEBUG_WARNING(true, ("Player %s tried to start crafting experimentation with invalid schematic.", myIdString));
		return -Crafting::CE_noDraftSchematic;
	}
	if (getCraftingTool() == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Player %s tried to start crafting experimentation with invalid crafting tool.", myIdString));
		return -Crafting::CE_noCraftingTool;
	}

	if (m_draftSchematic->getGameObjectType() == SharedObjectTemplate::GOT_data_draft_schematic_read_only)
	{
		return -Crafting::CE_readOnlyDraftSchematic;
	}

	TangibleObject * tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
	NOT_NULL(tool);

	if (!tool)
	{
		DEBUG_WARNING(true, ("Player %s tried to start crafting experimentation with nullptr crafting tool [%s].", myIdString, getCraftingTool().getValueString().c_str()));
		return -Crafting::CE_noCraftingTool;
	}

	// check the manufacturing schematic
	ManufactureSchematicObject * const manfSchematic = tool->getCraftingManufactureSchematic();
	if (manfSchematic == nullptr)
	{
		DEBUG_WARNING(true, ("Player %s tried to start crafting experimentation with invalid manufacture schematic.", myIdString));
		return -Crafting::CE_noManfSchematic;
	}

	// make sure all the manufacturing schematic slots are filled
	Crafting::IngredientSlot slot;
	ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
	int count = manfSchematic->getSlotsCount();
	for (i = 0; i < count; ++i)
	{
		if (!manfSchematic->getSlot(i, slot, false))
		{
			return -Crafting::CE_invalidSlot;
		}

		// get the draft schematic slot
		ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
		if (!m_draftSchematic->getSlot(draftSlot, slot.draftSlotIndex))
			return -Crafting::CE_invalidSlot;

		Crafting::IngredientType type = slot.ingredientType;
		if (type == Crafting::IT_none)
		{
			if (!draftSlot.optional)
			{
				DEBUG_WARNING(true, ("Player %s tried to start crafting experimentation with empty slot #%d.", myIdString, i));
				return -Crafting::CE_emptySlotAssembly;
			}
		}
		else
		{
			int currentIngredientCount = 0;
			for (Crafting::Ingredients::const_iterator iter =
				slot.ingredients.begin(); iter != slot.ingredients.end(); ++iter)
			{
				currentIngredientCount += (*iter)->count;
			}
			if (currentIngredientCount != draftSlot.options[static_cast<unsigned int>(slot.draftSlotOption)].ingredients[0].count)
			{
				DEBUG_WARNING(true, ("Player %s tried to start crafting experimentation with partially full slot #%d.", myIdString, i));
				return -Crafting::CE_partialSlotAssembly;
			}
		}
	}

	// remove unused optional slots from the schematic
	for (i = 0; i < count;)
	{
		if (!manfSchematic->getSlot(i, slot, false))
		{
			continue;
		}

		// get the draft schematic slot
		ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
		IGNORE_RETURN(m_draftSchematic->getSlot(draftSlot, slot.draftSlotIndex));

		if (slot.ingredientType == Crafting::IT_none && draftSlot.optional)
		{
			manfSchematic->removeSlot(slot.name);
			--count;
		}
		else
			++i;
	}

	ServerObject * prototype = tool->getCraftingPrototype();
	if (prototype == nullptr)
	{
		DEBUG_WARNING(true, ("PlayerObject::startCraftingExperiment crafting "
			"tool %s has no prototype object!",
			tool->getNetworkId().getValueString().c_str()));
		return -Crafting::CE_noPrototype;
	}

	// bio-link the prototype
	if (m_craftingComponentBioLink.get() != NetworkId::cms_invalid)
		safe_cast<TangibleObject *>(prototype)->setBioLink(m_craftingComponentBioLink.get());
	else
		safe_cast<TangibleObject *>(prototype)->clearBioLink();

	// if the tool we are using has the special crit success objvar, store that
	// info on the schematic
	int forceCritSuccess = 0;
	if (tool->getObjVars().getItem(OBJVAR_FORCE_CRITICAL_ASSEMBLY, forceCritSuccess))
	{
		if (forceCritSuccess > 0)
			manfSchematic->setObjVarItem(OBJVAR_FORCE_CRITICAL_SUCCESS, forceCritSuccess);
	}

	// call the trigger OnManufacturingSchematicCreation
	int successState = static_cast<int>(Crafting::CR_internalFailure);
	m_experimentPoints = 0;
	ScriptParams params;
	params.addParam(owner->getNetworkId());
	params.addParam(prototype->getNetworkId());
	params.addParam(*manfSchematic);
	params.addParam(successState);
	params.addParam(m_experimentPoints.get());
	IGNORE_RETURN(manfSchematic->getScriptObject()->trigAllScripts(
		Scripting::TRIG_MANUFACTURING_SCHEMATIC_CREATION, params));
	successState = params.getIntParam(3);
	m_experimentPoints = params.getIntParam(4);

	if (successState == static_cast<int>(Crafting::CR_internalFailure))
	{
		// either the script crashed or we didn't even make it to the script
		WARNING(true, ("PlayerObject::startCraftingExperiment failed calling OnManufacturingSchematicCreation on schematic %s (%s). Prepare for no owner for player!",
			manfSchematic->getNetworkId().getValueString().c_str(),
			m_draftSchematic->getObjectTemplateName()));
		return successState;
	}

	if (forceCritSuccess > 0)
	{
		// if the forced crit success count has changed, update the tool
		int newForceCritSuccess = 0;
		if (manfSchematic->getObjVars().getItem(OBJVAR_FORCE_CRITICAL_SUCCESS, newForceCritSuccess))
		{
			if (newForceCritSuccess != forceCritSuccess)
			{
				if (newForceCritSuccess > 0)
					tool->setObjVarItem(OBJVAR_FORCE_CRITICAL_ASSEMBLY, newForceCritSuccess);
				else
					tool->removeObjVarItem(OBJVAR_FORCE_CRITICAL_ASSEMBLY);
			}
		}
		manfSchematic->removeObjVarItem(OBJVAR_FORCE_CRITICAL_SUCCESS);
	}

	// important: we need to set the crafting stage here so restartCrafting
	// will destroy the ingredients if we crit failed
	m_craftingStage = static_cast<int>(Crafting::CS_experiment);

	if (successState == static_cast<int>(Crafting::CR_criticalFailure))
	{
		// destroy the ingredients, restart the session at the assembly stage
		IGNORE_RETURN(restartCrafting());
		m_craftingStage = static_cast<int>(Crafting::CS_assembly);
	}
	else if (successState != static_cast<int>(Crafting::CR_internalFailure) &&
		(getCraftingLevel() <= 1 || ((m_draftSchematic->getItemsPerContainer() == 0 ||
		m_craftingComponentBioLink.get() != NetworkId::cms_invalid) &&
		m_draftSchematic->getExperimentalAttributesCount() == 0)))
	{
		// if we are not near a station, we cannot experiment
		m_craftingStage = static_cast<int>(Crafting::CS_customize);
	}
	// if we crit failed but don't want to destroy the object, change the success
	// back to crit fail to the ui will show the right thing
	if (successState == static_cast<int>(Crafting::CR_criticalFailureNoDestroy))
	{
		successState = static_cast<int>(Crafting::CR_criticalFailure);
		m_craftingStage = static_cast<int>(Crafting::CS_assembly);
	}

	return successState;
}	// PlayerObject::startCraftingExperiment

//----------------------------------------------------------------------

/**
 * Tries to experiment on a schematic attribute.
 *
 * @param experiments		list of attributes and points assigned to them
 * @param totalPoints		total experimentation points used
 *
 * @return error code
 */
Crafting::CraftingResult PlayerObject::experiment(const std::vector<MessageQueueCraftExperiment::ExperimentInfo> & experiments, int totalPoints, int corelevel)
{
	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return Crafting::CR_internalFailure;

	std::string myId = getNetworkId().getValueString();
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	// verify we are experimenting
	if (m_craftingStage.get() != static_cast<int>(Crafting::CS_experiment))
	{
		DEBUG_WARNING(true, ("Player %s tried to experiment when not in "
			"experimentation phase", myIdString));
		return Crafting::CR_internalFailure;
	}

	// verify the points allocated
	if (totalPoints < 1 || totalPoints > m_experimentPoints.get())
	{
		DEBUG_WARNING(true, ("Player %s sent invalid number of experiment points "
			"= %d (%d)", myIdString, totalPoints, m_experimentPoints.get() ));
		return Crafting::CR_internalFailure;
	}

	// verify the tool
	if (m_draftSchematic.getPointer() == nullptr || getCraftingTool() == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Player %s tried to start crafting experimentation "
			"with invalid schematic or crafting tool.", myIdString));
		return Crafting::CR_internalFailure;
	}
	TangibleObject * tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
	NOT_NULL(tool);
	if (!tool)
		return Crafting::CR_internalFailure;

	// check the manufacturing schematic
	ManufactureSchematicObject * const manfSchematic = tool->getCraftingManufactureSchematic();
	if (manfSchematic == nullptr)
	{
		DEBUG_WARNING(true, ("Player %s tried to experiment with invalid "
			"manufacture schematic.", myIdString));
		return Crafting::CR_internalFailure;
	}
	const std::vector<StringId> & attribNames = manfSchematic->getExperimentAttributeNames();

	// check the test prototype
	ServerObject * prototype = tool->getCraftingPrototype();
	if (prototype == nullptr)
	{
		DEBUG_WARNING(true, ("Player %s tried to experiment with invalid test "
			"prototype.", myIdString));
		return Crafting::CR_internalFailure;
	}

	std::vector<const StringId *> scriptAttributes;
	std::vector<int>              scriptPoints;

	std::vector<MessageQueueCraftExperiment::ExperimentInfo>::const_iterator iter;
	for (iter = experiments.begin(); iter != experiments.end(); ++iter)
	{
		int attributeIndex = (*iter).attributeIndex;
		int points = (*iter).experimentPoints;

		// verify points
		if (points <= 0)
		{
			DEBUG_WARNING(true, ("Player %s sent invalid experiment point count %d",
				myIdString, points));
			return Crafting::CR_internalFailure;
		}

		// verify attribute index
		if (attributeIndex < 0 || attributeIndex >= static_cast<int>(attribNames.size()))
		{
			DEBUG_WARNING(true, ("Player %s sent invalid attribute index %d",
				myIdString, attributeIndex));
			return Crafting::CR_internalFailure;
		}

		// set the experiment data for the index
		scriptAttributes.push_back(&attribNames[static_cast<unsigned int>(attributeIndex)]);
		scriptPoints.push_back(points);
	}

	m_experimentPoints = m_experimentPoints.get() - totalPoints;

	// if the tool we are using has the special crit success objvar, store that
	// info on the schematic
	int forceCritSuccess = 0;
	if (tool->getObjVars().getItem(OBJVAR_FORCE_CRITICAL_EXPERIMENT, forceCritSuccess))
	{
		if (forceCritSuccess > 0)
			manfSchematic->setObjVarItem(OBJVAR_FORCE_CRITICAL_SUCCESS, forceCritSuccess);
	}

	Crafting::CraftingResult experimentResult = Crafting::CR_failure;
	ScriptParams params;
	params.addParam(owner->getNetworkId());
	params.addParam(prototype->getNetworkId());
	params.addParam(scriptAttributes);
	params.addParam(scriptPoints);
	params.addParam(totalPoints);
	params.addParam(static_cast<int>(experimentResult));
	params.addParam(corelevel);
	IGNORE_RETURN(manfSchematic->getScriptObject()->trigAllScripts(Scripting::TRIG_CRAFTING_EXPERIMENT, params));
	experimentResult = static_cast<Crafting::CraftingResult>(params.getIntParam(5));

	if (forceCritSuccess > 0)
	{
		// if the forced crit success count has changed, update the tool
		int newForceCritSuccess = 0;
		if (manfSchematic->getObjVars().getItem(OBJVAR_FORCE_CRITICAL_SUCCESS, newForceCritSuccess))
		{
			if (newForceCritSuccess != forceCritSuccess)
			{
				if (newForceCritSuccess > 0)
					tool->setObjVarItem(OBJVAR_FORCE_CRITICAL_EXPERIMENT, newForceCritSuccess);
				else
					tool->removeObjVarItem(OBJVAR_FORCE_CRITICAL_EXPERIMENT);
			}
		}
		manfSchematic->removeObjVarItem(OBJVAR_FORCE_CRITICAL_SUCCESS);
	}

	return experimentResult;
}	// PlayerObject::experiment

//----------------------------------------------------------------------

/**
 * Sets a customization property value for the current manufacturing schematic.
 *
 * @param property		index of the customization property
 * @param value			value the property
 *
 * @return true if the property was set, false if not
 */
bool PlayerObject::customize(int property, int value) const
{
	std::string myId = getNetworkId().getValueString();
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	if (m_craftingStage.get() != static_cast<int>(Crafting::CS_customize))
	{
		DEBUG_WARNING(true, ("Player %s tried to customize when not in "
			"customization phase", myIdString));
		return false;
	}

	if (m_draftSchematic.getPointer() == nullptr || getCraftingTool() == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Player %s tried to customize with invalid "
			"schematic or crafting tool.", myIdString));
		return false;
	}

	TangibleObject * tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
	NOT_NULL(tool);
	if (!tool)
		return false;
	ManufactureSchematicObject * const manfSchematic = tool->getCraftingManufactureSchematic();
	NOT_NULL(manfSchematic);
	ServerObject * prototype = tool->getCraftingPrototype();
	NOT_NULL(prototype);

	return manfSchematic->setCustomization(property, value, *prototype);
}	// PlayerObject::customize

//----------------------------------------------------------------------

/**
 * Sets the customization data for a crafter's current manufacturing schematic.
 *
 * @param name              the name of the object being crafted
 * @param appearanceIndex   index of the appearance of the object
 * @param customizations    customization information
 * @param itemCount         if making a manf schematic, the number of items the
 *                          schematic can make
 *
 * @return error code
 */
int PlayerObject::setCustomizationData(const Unicode::String & name,
	int appearanceIndex, const std::vector<Crafting::CustomValue> & customizations,
	int itemCount) const
{
	// Strip any color codes from the text

	TextIterator textIterator(name);
	Unicode::String strippedText(textIterator.getPrintableText());

	std::string myId = getNetworkId().getValueString();
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	if (m_craftingStage.get() != static_cast<int>(Crafting::CS_customize))
	{
		DEBUG_WARNING(true, ("Player %s tried to customize when not in "
			"customization phase", myIdString));
		return Crafting::CE_notCustomizeStage;
	}

	if (m_draftSchematic.getPointer() == nullptr)
	{
		DEBUG_WARNING(true, ("Player %s tried to customize with invalid schematic.", myIdString));
		return Crafting::CE_noDraftSchematic;
	}
	if (getCraftingTool() == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Player %s tried to customize with invalid crafting tool.", myIdString));
		return Crafting::CE_noCraftingTool;
	}

	// make sure the object name is ok
	if (!strippedText.empty() && (strippedText.size() >= 127 || !TextManager::isAppropriateText(strippedText)))
	{
		return Crafting::CE_invalidCraftedName;
	}

	TangibleObject * tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
	NOT_NULL(tool);
	if (!tool)
	{
		LOG("Crafting", ("We should have FATALd in debug, but we didn't...no tool for player %s tool %s", myIdString, getCraftingTool().getValueString().c_str()));
		return Crafting::CE_noCraftingTool;
	}

	ManufactureSchematicObject * const manfSchematic = tool->getCraftingManufactureSchematic();
	NOT_NULL(manfSchematic);
	if (!manfSchematic)
	{
		LOG("Crafting", ("We should have FATALd in debug, but we didn't...no manfschem for player %s", myIdString));
		return Crafting::CE_noDraftSchematic;
	}
	TangibleObject * prototype = safe_cast<TangibleObject *>(tool->getCraftingPrototype());
	NOT_NULL(prototype);

	if (!prototype)
	{
		LOG("Crafting", ("We should have FATALd in debug, but we didn't...no prototype for player %s", myIdString));
		return Crafting::CE_noCraftingTool;
	}

	if (!strippedText.empty())
	{
		prototype->setObjectName(strippedText);
		manfSchematic->setObjectName(strippedText);
	}

	manfSchematic->setCustomAppearance(appearanceIndex);
	manfSchematic->setCount(std::min (1000, itemCount));
	prototype->setCustomAppearance(manfSchematic->getCustomAppearance());

	std::vector<Crafting::CustomValue>::const_iterator iter;
	for (iter = customizations.begin(); iter != customizations.end(); ++iter)
	{
		IGNORE_RETURN(manfSchematic->setCustomization((*iter).property, (*iter).value, *prototype));
	}
	return Crafting::CE_success;
}	// PlayerObject::setCustomizationData

//----------------------------------------------------------------------

/**
 * Creates a prototype object from the current manufacturing schematic.
 *
 * @param keepPrototype		flag that this is a real prototype; if this is false
 *							we don't persist the prototype or give it to the player.
 *
 * @return true on success, false on fail
 */
bool PlayerObject::createPrototype(bool keepPrototype)
{
	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return false;

	std::string myId = getNetworkId().getValueString();
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	// @todo: need to filter the name

	// verify we are in the final crafting phase
	if (m_craftingStage.get() != static_cast<int>(Crafting::CS_finish))
	{
		DEBUG_WARNING(true, ("Player %s tried to create prototype when not in "
			"final phase", myIdString));
		return false;
	}

	if (m_draftSchematic.getPointer() == nullptr || getCraftingTool() == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Player %s tried to create prototype with invalid "
			"schematic or crafting tool.", myIdString));
		return false;
	}

	TangibleObject * tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
	if (!tool)
	{
		WARNING_STRICT_FATAL(true, ("Could not find tool during create prototype phase."));
		return false;
	}
	TangibleObject * prototype = safe_cast<TangibleObject *>(tool->getCraftingPrototype());
	if (!prototype)
	{
		WARNING_STRICT_FATAL(true, ("Could not find prototype during create prototype phase."));
		return false;
	}

	ManufactureSchematicObject * const manfSchematic = tool->getCraftingManufactureSchematic();
	if (manfSchematic == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("No manf schematic in crafting tool %s",
			tool->getNetworkId().getValueString().c_str()));
		return false;
	}

	// double-check the schematic data
	Crafting::IngredientSlot sourceSlot;
	ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
	int numSlots = manfSchematic->getSlotsCount();
	for (int i = 0; i < numSlots; ++i)
	{
		if (!manfSchematic->getSlot(i, sourceSlot, true))
		{
			WARNING_STRICT_FATAL(true, ("PlayerObject::createPrototype player "
				"%s has invalid manf schematic: cannot get slot info for slot %d",
				owner->getNetworkId().getValueString().c_str(), i));
			return false;
		}
		if (!m_draftSchematic->getSlot(draftSlot, sourceSlot.name))
		{
			WARNING_STRICT_FATAL(true, ("PlayerObject::createPrototype player "
				"%s has invalid manf schematic: cannot get draft info for slot %s",
				owner->getNetworkId().getValueString().c_str(),
				sourceSlot.name.getCanonicalRepresentation().c_str()));
			return false;
		}
		if (sourceSlot.ingredientType == Crafting::IT_invalid ||
			sourceSlot.ingredientType == Crafting::IT_none)
		{
			WARNING_STRICT_FATAL(true, ("PlayerObject::createPrototype player "
				"%s has invalid manf schematic: slot %d has invalid ingredient "
				"type %d", owner->getNetworkId().getValueString().c_str(), i,
				static_cast<int>(sourceSlot.ingredientType)));
			return false;
		}
		if (sourceSlot.draftSlotOption < 0 ||
			sourceSlot.draftSlotOption >= static_cast<int>(draftSlot.options.size()))
		{
			WARNING_STRICT_FATAL(true, ("PlayerObject::createPrototype player "
				"%s has invalid manf schematic: slot %d has invalid slot option %d",
				owner->getNetworkId().getValueString().c_str(), i,
				sourceSlot.draftSlotOption));
			return false;
		}
		if (sourceSlot.draftSlotIndex < 0)
		{
			WARNING_STRICT_FATAL(true, ("PlayerObject::createPrototype player "
				"%s has invalid manf schematic: slot %d has invalid slot index %d",
				owner->getNetworkId().getValueString().c_str(), i,
				sourceSlot.draftSlotIndex));
			return false;
		}
	}

	// call the trigger OnManufactureObject on the manf schematic to allow the
	// script to initialize the prototype
	ScriptParams params;
	params.addParam(owner->getNetworkId());
	params.addParam(prototype->getNetworkId());
	params.addParam(*manfSchematic);
	params.addParam(true);
	params.addParam(keepPrototype);
	if (manfSchematic->getScriptObject()->trigAllScripts(
		Scripting::TRIG_MANUFACTURE_OBJECT, params) == SCRIPT_OVERRIDE)
	{
		return false;
	}

	if (!tool->addObjectToOutputSlot(*prototype, owner))
		return false;

	if (!manfSchematic->setObjectComponents(prototype, false))
	{
		return false;
	}

	// tell the client to delete the prototype - we will recreate it when it is
	// finished being "built"; this is so that the client can be updated with
	// the prototype's appearance, which may change during the crafting process
	ObserveTracker::onCraftingPrototypeCreated(*owner, *prototype);

	if (keepPrototype)
	{
		// save the prototype to the database
		prototype->persist();

		ScriptParams craftparams;
		craftparams.addParam(prototype->getNetworkId());
		craftparams.addParam(*manfSchematic);

		GameScriptObject * scriptObject = owner->getScriptObject();
		if(scriptObject)
		{
			scriptObject->trigAllScripts(Scripting::TRIG_CRAFTED_PROTOTYPE, craftparams);
		}
	}
	else
	{
		prototype->setObjVarItem(OBJVAR_CRAFTING_FAKE_PROTOTYPE, 1);
	}

	// remove the manufacturing schematic
	tool->clearCraftingManufactureSchematic();
	m_experimentPoints = -1;
	m_craftingStage = static_cast<int>(Crafting::CS_none);

	// lock the tool for the time needed to create the prototype
	float prototypeTime = m_draftSchematic->getPrototypeTime() * prototype->getComplexity();
	if (prototypeTime < 1.0f)
		prototypeTime = 1.0f;

	// send a message to ourself that will unlock the prototype
	params.clear();
	params.addParam(owner->getNetworkId(), "crafter");
	params.addParam(prototype->getNetworkId(), "prototype");
	ScriptDictionaryPtr dictionary;
	getScriptObject()->makeScriptDictionary(params, dictionary);
	if (dictionary.get() != nullptr)
	{
		dictionary->serialize();

		if (owner->getObjVars().hasItem("crafting_qa"))
		{
			Client * client = owner->getClient();

			if (client != nullptr && client->isGod())
			{
				Chat::sendSystemMessage(*owner, Unicode::narrowToWide("Crafting time changed due to god mode and crafting_qa objvar."), Unicode::emptyString);
				prototypeTime = 1;
			}
		}

//		MessageToQueue::getInstance().sendMessageToJava(tool->getNetworkId(), "prototypeDone", dictionary->getSerializedData(), prototypeTime, true);
		MessageToQueue::getInstance().sendMessageToJava(tool->getNetworkId(), "prototypeDone", dictionary->getSerializedData(), static_cast<int>(prototypeTime), false);

		tool->setObjVarItem(OBJVAR_CRAFTING_PROTOTYPE_TIME, prototypeTime);
		tool->setObjVarItem(OBJVAR_CRAFTING_PROTOTYPE_START_TIME, static_cast<int>(ServerClock::getInstance().getGameTimeSeconds()));
		tool->setObjVarItem(OBJVAR_CRAFTING_PROTOTYPE_OBJECT, prototype->getNetworkId());
		tool->setObjVarItem(OBJVAR_CRAFTING_PROTOTYPE_CRAFTER, owner->getNetworkId());

		// message the object to update it's counter for the amount of time left
		// before the prototype is done
//		MessageToQueue::getInstance().sendMessageToC(tool->getNetworkId(), "C++updateCounter", "", 1.0f, true);
		MessageToQueue::getInstance().sendMessageToC(tool->getNetworkId(), "C++updateCounter", "", 1, false);
	}
	else
	{
//		DEBUG_WARNING(true, ("Could not make script dictionary for prototype delay"));
		WARNING(true, ("PlayerObject::createPrototype for player %s could not "
			"make script dictionary for prototype delay",
			owner->getNetworkId().getValueString().c_str()));
	}

	LOG("CustomerService", ("Crafting:%s has crafted a prototype item %s",
//		PlayerObject::getAccountDescription(owner->getNetworkId()).c_str(), ServerObject::getLogDescription(prototype).c_str()));
		PlayerObject::getAccountDescription(owner->getNetworkId()).c_str(),
		ServerObject::getLogDescription(prototype).c_str()));

	return true;
}	// PlayerObject::createPrototype

//----------------------------------------------------------------------

/**
 * Moves the manufacturing schematic from it's temporary container to the output
 * hopper of the crafting tool.
 *
 * @return true on success, false on fail
 */
bool PlayerObject::createManufacturingSchematic()
{
	static SlotId datapadSlotId = SlotIdManager::findSlotId(DATAPAD_SLOT_NAME);

	// if we are not near a station, we cannot make a manf schematic
	if (getCraftingLevel() <= 1)
		return false;

	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return false;

	// @todo: need to filter the name

	// verify we are in the final crafting phase
	if (m_craftingStage.get() != static_cast<int>(Crafting::CS_finish))
	{
		WARNING(true, ("Player %s tried to create manf schematic when not in final phase", getNetworkId().getValueString().c_str ()));
		return false;
	}

	if (m_draftSchematic.getPointer() == nullptr || getCraftingTool() == NetworkId::cms_invalid)
	{
		WARNING(true, ("Player %s tried to create manf schematic with invalid schematic or crafting tool.", getNetworkId().getValueString().c_str ()));
		return false;
	}
	if (m_draftSchematic->getItemsPerContainer() == 0)
	{
		WARNING(true, ("Player %s tried to create manf schematic with schematic "
			"%s that disallows manufacturing.", getAccountDescription().c_str(),
			m_draftSchematic->getObjectTemplateName()));
		return false;
	}

	TangibleObject * const tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
	if (tool == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("Could not find crafting tool during create manf schematic phase."));
		return false;
	}
	ManufactureSchematicObject * const manfSchematic = tool->removeCraftingManufactureSchematic();
	if (manfSchematic == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("Could not find manf schematic during create manf schematic phase."));
		return false;
	}
	TangibleObject * const prototype = safe_cast<TangibleObject *>(tool->getCraftingPrototype());
	if (prototype == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("Could not find prototype during create manf schematic phase."));
		return false;
	}

	// no manufacturing a bio-linked item
	if (m_craftingComponentBioLink.get() != NetworkId::cms_invalid)
	{
		WARNING(true, ("Player %s tried to create manf schematic while using a "
			"bio-linked component %s.", getAccountDescription().c_str(),
			m_craftingComponentBioLink.get().getValueString().c_str()));
		return false;
	}

	// double-check the schematic data
	Crafting::IngredientSlot sourceSlot;
	ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
	int numSlots = manfSchematic->getSlotsCount();
	for (int i = 0; i < numSlots; ++i)
	{
		if (!manfSchematic->getSlot(i, sourceSlot, true))
		{
			WARNING_STRICT_FATAL(true, ("PlayerObject::createManufacturingSchematic player "
				"%s has invalid manf schematic: cannot get slot info for slot %d",
				getAccountDescription().c_str(), i));
			return false;
		}
		if (!m_draftSchematic->getSlot(draftSlot, sourceSlot.name))
		{
			WARNING_STRICT_FATAL(true, ("PlayerObject::createManufacturingSchematic player "
				"%s has invalid manf schematic: cannot get draft info for slot %s",
				getAccountDescription().c_str(),
				sourceSlot.name.getCanonicalRepresentation().c_str()));
			return false;
		}
		if (sourceSlot.ingredientType == Crafting::IT_invalid ||
			sourceSlot.ingredientType == Crafting::IT_none)
		{
			WARNING_STRICT_FATAL(true, ("PlayerObject::createManufacturingSchematic player "
				"%s has invalid manf schematic: slot %d has invalid ingredient "
				"type %d", getAccountDescription().c_str(), i,
				static_cast<int>(sourceSlot.ingredientType)));
			return false;
		}
		if (sourceSlot.draftSlotOption < 0 ||
			sourceSlot.draftSlotOption >= static_cast<int>(draftSlot.options.size()))
		{
			WARNING_STRICT_FATAL(true, ("PlayerObject::createManufacturingSchematic player "
				"%s has invalid manf schematic: slot %d has invalid slot option %d",
				getAccountDescription().c_str(), i,
				sourceSlot.draftSlotOption));
			return false;
		}
		if (sourceSlot.draftSlotIndex < 0)
		{
			WARNING_STRICT_FATAL(true, ("PlayerObject::createManufacturingSchematic player "
				"%s has invalid manf schematic: slot %d has invalid slot index %d",
				getAccountDescription().c_str(), i,
				sourceSlot.draftSlotIndex));
			return false;
		}
	}

	// call the trigger OnFinalizeSchematic on the manf schematic to allow the
	// script to initialize it
	ScriptParams params;
	params.addParam(owner->getNetworkId());
	params.addParam(prototype->getNetworkId());
	params.addParam(*manfSchematic);
	if (manfSchematic->getScriptObject()->trigAllScripts(Scripting::TRIG_FINALIZE_SCHEMATIC, params) == SCRIPT_OVERRIDE)
	{
		return false;
	}

	manfSchematic->clearSlotSources();
	manfSchematic->storeObjectAttributes (*prototype);
	manfSchematic->removeObjVarItem(OBJVAR_IN_CRAFTING_SESSION);
	manfSchematic->persist();

	ServerObject * const datapad = owner->getDatapad ();
	if (datapad == nullptr)
	{
		DEBUG_WARNING(true, ("Can't find datapad for player %s", getAccountDescription().c_str()));
		return false;
	}

	Container::ContainerErrorCode tmp = Container::CEC_Success;
	if (!ContainerInterface::transferItemToVolumeContainer(*datapad, *manfSchematic, nullptr, tmp))
	{
		return false;
	}

	tool->clearCraftingPrototype();
	m_experimentPoints = -1;
	m_craftingStage = static_cast<int>(Crafting::CS_none);

	LOG("CustomerService", ("Crafting:%s has crafted a manufacturing schematic %s",
		getAccountDescription().c_str(), ServerObject::getLogDescription(manfSchematic).c_str()));

	return true;
}	// PlayerObject::createManufacturingSchematic

//----------------------------------------------------------------------

/**
 * Restarts the crafting session with the same draft schematic as this one.
 *
 * @return true on success, false on fail
 */
bool PlayerObject::restartCrafting ()
{
	CreatureObject * const owner = getCreatureObject();
	if (owner == nullptr)
		return false;

	std::string myId = getNetworkId().getValueString();
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	// make sure we are crafting
	if (m_craftingStage.get() == static_cast<int>(Crafting::CS_none))
	{
		DEBUG_WARNING(true, ("Player %s tried to restart crafting when not crafting",
			myIdString));
		return false;
	}
	if (m_craftingStage.get() == static_cast<int>(Crafting::CS_finish))
	{
		DEBUG_WARNING(true, ("Player %s tried to restart crafting when crafting over",
			myIdString));
		return false;
	}

	// verify the tool
	if (m_draftSchematic.getPointer() == nullptr || getCraftingTool() == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Player %s tried to start crafting experimentation "
			"with invalid schematic or crafting tool.", myIdString));
		return false;
	}
	TangibleObject * tool = safe_cast<TangibleObject *>(getCraftingTool().getObject());
	NOT_NULL(tool);
	if (!tool)
		return false;

	// reset the manf schematic
	ManufactureSchematicObject * manfSchematic = tool->getCraftingManufactureSchematic();
	if (manfSchematic == nullptr)
	{
		DEBUG_WARNING(true, ("Player %s has no manufacturing schematic!", myIdString));
		return false;
	}

	// if we are at the assembly stage of crafting, or the draft schematic says
	// not to destroy the ingredients, put the ingredients in the schematic back
	// in the player's inventory or station hopper
	if (!manfSchematic->mustDestroyIngredients() ||
		m_craftingStage.get() == static_cast<int>(Crafting::CS_assembly))
	{
		setAllowEmptySlot(true);
		int count = manfSchematic->getSlotsCount();
		// note this loop is odd because when we clear a slot it may
		// actually be removed if it was optional
		for (int i = 0; i < count;)
		{
			if (emptySlot(i, NetworkId::cms_invalid))
			{
				if (count == manfSchematic->getSlotsCount())
					++i;
				else
					count = manfSchematic->getSlotsCount();
			}
			else
				++i;
		}
		setAllowEmptySlot(false);
	}
	else
	{
		manfSchematic->destroyAllIngredients();
	}

	// reset the manf schematic data
	manfSchematic->reset();
	m_craftingComponentBioLink = NetworkId::cms_invalid;

	m_craftingStage = static_cast<int>(Crafting::CS_assembly);
	return true;
}	// PlayerObject::restartCrafting

//----------------------------------------------------------------------

/**
 * Ends a crafting session.
 *
 * @param normalExit		flag that we are exiting due to normal circumstances
 */
void PlayerObject::stopCrafting (bool normalExit)
{
	CreatureObject * const owner = getCreatureObject();

	if (getCraftingTool().getObject() != nullptr)
	{
		TangibleObject * tool = dynamic_cast<TangibleObject *>(getCraftingTool().getObject());
		if (tool != nullptr)
		{
			IGNORE_RETURN(tool->stopCraftingSession());
		}

		// tell scripts that the session has ended
		ScriptParams params;
		if (owner != nullptr)
			params.addParam(owner->getNetworkId());
		else
			params.addParam(getNetworkId());
		if (getCurrentDraftSchematic() != nullptr)
			params.addParam(getCurrentDraftSchematic()->getTemplateName());
		else
			params.addParam("");
		params.addParam(static_cast<int>(m_craftingStage.get()));
		params.addParam(normalExit);
		IGNORE_RETURN(tool->getScriptObject()->trigAllScripts(
			Scripting::TRIG_CRAFTING_DONE, params));
	}

	m_craftingStage = static_cast<int>(Crafting::CS_none);
	m_useableDraftSchematics.clear();
	m_craftingTool = CachedNetworkId::cms_cachedInvalid;
	m_craftingComponentBioLink = NetworkId::cms_invalid;

	// tell the player crafting has ended
	if (owner != nullptr && owner->getController() != nullptr)
	{
		(safe_cast<ServerController *>(owner->getController()))->appendMessage(
			static_cast<int>(CM_craftingSessionEnded), 0.0f,
				new MessageQueueGenericValueType<bool>(normalExit),
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}	// PlayerObject::stopCrafting

// ----------------------------------------------------------------------

void PlayerObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	IntangibleObject::getAttributes(data);
}

/**
 * Sets the player's friend list
 *
 * @param friendList - The new player's friend list
 */
void PlayerObject::setFriendList(const StringVector &friendList)
{
	//DEBUG_REPORT_LOG(true, ("PlayerObject::setFriendList() - Old Friend List\n"));
	//DEBUG_REPORT_LOG(true, ("PlayerObject::setFriendList() - <count> %d\n", m_friendList.size()));
	//DEBUG_REPORT_LOG(true, ("PlayerObject::setFriendList() - New Friend List\n"));
	//DEBUG_REPORT_LOG(true, ("PlayerObject::setFriendList() - <count> %d\n", friendList.size()));

	if (isAuthoritative())
	{
		// construct lowercase, sorted friend list
		size_t const size = friendList.size();
		StringVector friendListLowercaseSorted(size);
		for (size_t index = 0; index < size; ++index)
		{
			friendListLowercaseSorted[index] = Unicode::toLower(friendList[index]);
		}

		if (!friendListLowercaseSorted.empty())
		{
			std::sort(friendListLowercaseSorted.begin(), friendListLowercaseSorted.end());
		}

		LOG("ChatFriendIgnoreList", ("Friend=%d, %s", friendListLowercaseSorted.size(), getAccountDescription().c_str()));
		m_friendList.set(friendListLowercaseSorted);
	}
}

// ----------------------------------------------------------------------

void PlayerObject::requestFriendList() const
{
	CreatureObject const *owner = safe_cast<const CreatureObject *>(ContainerInterface::getContainedByObject(*this));

	if (owner != nullptr)
	{
		ChatAvatarId chatAvatarId(Chat::constructChatAvatarId(*owner));

		DEBUG_WARNING(chatAvatarId.name.empty(), ("Chat avatar id name is empty"));

		Chat::getFriendsList(chatAvatarId.getFullName());
	}
}

/**
 * Determines whether the specified name is a friend of this player
 *
 * @param name - The name of the friend
 */
bool PlayerObject::isFriend(const std::string &name) const
{
	std::string firstName;
	size_t pos = 0;
	IGNORE_RETURN(Unicode::getFirstToken(name, 0, pos, firstName));

	return std::binary_search(m_friendList.begin(), m_friendList.end(), Unicode::toLower(firstName));
}

/**
 * Sets the player's list of people to ignore
 *
 * @param ignoreList - The new player's list of people to ignore
 */
void PlayerObject::setIgnoreList(const StringVector &ignoreList)
{
	//DEBUG_REPORT_LOG(true, ("PlayerObject::setIgnoreList() - Old Ignore List\n"));
	//DEBUG_REPORT_LOG(true, ("PlayerObject::setIgnoreList() - <count> %d\n", m_ignoreList.size()));
	//DEBUG_REPORT_LOG(true, ("PlayerObject::setIgnoreList() - New Ignore List\n"));
	//DEBUG_REPORT_LOG(true, ("PlayerObject::setIgnoreList() - <count> %d\n", ignoreList.size()));

	if (isAuthoritative ())
	{
		// construct lowercase, sorted ignore list
		size_t const size = ignoreList.size();
		StringVector ignoreListLowercaseSorted(size);
		for (size_t index = 0; index < size; ++index)
		{
			ignoreListLowercaseSorted[index] = Unicode::toLower(ignoreList[index]);
		}

		if (!ignoreListLowercaseSorted.empty())
		{
			std::sort(ignoreListLowercaseSorted.begin(), ignoreListLowercaseSorted.end());
		}

		LOG("ChatFriendIgnoreList", ("Ignore=%d, %s", ignoreListLowercaseSorted.size(), getAccountDescription().c_str()));
		m_ignoreList.set(ignoreListLowercaseSorted);
	}
}

// ----------------------------------------------------------------------

void PlayerObject::requestIgnoreList() const
{
	CreatureObject const *owner = safe_cast<const CreatureObject *>(ContainerInterface::getContainedByObject(*this));

	if (owner != nullptr)
	{
		ChatAvatarId chatAvatarId(Chat::constructChatAvatarId(*owner));

		DEBUG_WARNING(chatAvatarId.name.empty(), ("Chat avatar id name is empty"));

		Chat::getIgnoreList(chatAvatarId.getFullName());
	}
}

// ----------------------------------------------------------------------

bool PlayerObject::isIgnoring(const std::string &name) const
{
	std::string firstName;
	size_t pos = 0;
	IGNORE_RETURN(Unicode::getFirstToken(name, 0, pos, firstName));

	return std::binary_search(m_ignoreList.begin(), m_ignoreList.end(), Unicode::toLower(firstName));
}

/**
 * Sets a player's personal matchmaking id
 *
 * @param matchMakingId - The new match making id which describes their personality
 */
void PlayerObject::setMatchMakingPersonalProfileId(MatchMakingId const &id)
{
	if (isAuthoritative ())
	{
		//DEBUG_REPORT_LOG(true, ("PlayerObject::setMatchMakingPersonalProfileId() <old> %s <new> %s\n", m_matchMakingPersonalProfileId.get().getDebugIntString().c_str(), id.getDebugIntString().c_str()));

		m_matchMakingPersonalProfileId = id;
	}
}

// ----------------------------------------------------------------------

MatchMakingId const &PlayerObject::getMatchMakingPersonalProfileId() const
{
	return m_matchMakingPersonalProfileId.get();
}

/**
 * Sets a player's character matchmaking id
 *
 * @param matchMakingId - The new match making id which describes their character
 */
void PlayerObject::setMatchMakingCharacterProfileId(MatchMakingId const &id)
{
	if (isAuthoritative())
	{
		m_matchMakingCharacterProfileId = id;

		//DEBUG_REPORT_LOG(true, ("MatchMakingCharacterProfileId: <anonymous>         %s\n", m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_anonymous) ? "Yes" : "No"));
		//DEBUG_REPORT_LOG(true, ("MatchMakingCharacterProfileId: <helper>            %s\n", m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_helper) ? "Yes" : "No"));
		//DEBUG_REPORT_LOG(true, ("MatchMakingCharacterProfileId: <roleplay>          %s\n", m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_rolePlay) ? "Yes" : "No"));
		//DEBUG_REPORT_LOG(true, ("MatchMakingCharacterProfileId: <looking for group> %s\n", m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_lookingForGroup) ? "Yes" : "No"));

		CreatureObject const * const owner = getCreatureObject();
		if (owner)
		{
			std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
			std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = connectedCharacterLfgData.find(owner->getNetworkId());
			if (iterFind != connectedCharacterLfgData.end())
			{	
				bool const anonymous = id.isBitSet(MatchMakingId::B_anonymous);
				if (iterFind->second.anonymous != anonymous)
					ServerUniverse::setConnectedCharacterAnonymousData(owner->getNetworkId(), anonymous);

				bool const searchableByCtsSourceGalaxy = id.isBitSet(MatchMakingId::B_searchableByCtsSourceGalaxy);
				if (iterFind->second.searchableByCtsSourceGalaxy != searchableByCtsSourceGalaxy)
					ServerUniverse::setConnectedCharacterSearchableByCtsSourceGalaxyData(owner->getNetworkId(), searchableByCtsSourceGalaxy);

				bool const displayLocationInSearchResults = id.isBitSet(MatchMakingId::B_displayLocationInSearchResults);
				if (iterFind->second.displayLocationInSearchResults != displayLocationInSearchResults)
					ServerUniverse::setConnectedCharacterDisplayLocationInSearchResultsData(owner->getNetworkId(), displayLocationInSearchResults);

				BitArray characterInterests = iterFind->second.characterInterests;

				if (id.isBitSet(MatchMakingId::B_lookingForGroup))
					LfgDataTable::setBit("lfg", characterInterests);
				else
					LfgDataTable::clearBit("lfg", characterInterests);

				if (id.isBitSet(MatchMakingId::B_helper))
					LfgDataTable::setBit("helper", characterInterests);
				else
					LfgDataTable::clearBit("helper", characterInterests);

				if (id.isBitSet(MatchMakingId::B_rolePlay))
					LfgDataTable::setBit("rp", characterInterests);
				else
					LfgDataTable::clearBit("rp", characterInterests);

				if (id.isBitSet(MatchMakingId::B_lookingForWork))
					LfgDataTable::setBit("lfw", characterInterests);
				else
					LfgDataTable::clearBit("lfw", characterInterests);

				if (iterFind->second.characterInterests != characterInterests)
					ServerUniverse::setConnectedCharacterInterestsData(owner->getNetworkId(), characterInterests);
			}
		}
	}
}

MatchMakingId const &PlayerObject::getMatchMakingCharacterProfileId() const
{
	return m_matchMakingCharacterProfileId.get();
}

/**
* Toggles the player's SearchableByCtsSourceGalaxy status.
*/
void PlayerObject::toggleSearchableByCtsSourceGalaxy()
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.toggleBit(MatchMakingId::B_searchableByCtsSourceGalaxy);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}

/**
* Toggles the player's DisplayLocationInSearchResults status.
*/
void PlayerObject::toggleDisplayLocationInSearchResults()
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.toggleBit(MatchMakingId::B_displayLocationInSearchResults);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}

/**
 * Toggles the player's anonymous status.
 */
void PlayerObject::toggleAnonymous()
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.toggleBit(MatchMakingId::B_anonymous);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}

/**
 * Toggles the player's helper status.
 */
void PlayerObject::toggleHelper()
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.toggleBit(MatchMakingId::B_helper);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}

/**
 * Toggles the player's roleplay status.
 */
void PlayerObject::toggleRolePlay()
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.toggleBit(MatchMakingId::B_rolePlay);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}

// ----------------------------------------------------------------------
bool PlayerObject::isLookingForGroup() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_lookingForGroup);
}

// ----------------------------------------------------------------------

void PlayerObject::setLookingForGroup(bool const lookingForGroup)
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.resetBit(MatchMakingId::B_lookingForGroup);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}

/**
 * Toggles the player's looking for group status.
 */
void PlayerObject::toggleLookingForGroup()
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.toggleBit(MatchMakingId::B_lookingForGroup);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}

/**
 * Returns the player's away from keyboard status.
 */
bool PlayerObject::isAwayFromKeyBoard() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_awayFromKeyBoard);
}

/**
 * Toggles the player's away from keyboard status.
 */
void PlayerObject::toggleAwayFromKeyBoard()
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.toggleBit(MatchMakingId::B_awayFromKeyBoard);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}

/**
 * Toggles the player's display of their faction rank.
 */
void PlayerObject::toggleDisplayingFactionRank()
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.toggleBit(MatchMakingId::B_displayingFactionRank);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}


// ----------------------------------------------------------------------

void PlayerObject::setLinkDead(bool const linkDead)
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		if (linkDead)
		{
			matchMakingId.setBit(MatchMakingId::B_linkDead);
		}
		else
		{
			matchMakingId.resetBit(MatchMakingId::B_linkDead);
		}

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}

// ----------------------------------------------------------------------

bool PlayerObject::getLinkDead() const
{
	MatchMakingId const matchMakingId(m_matchMakingCharacterProfileId.get());
	return matchMakingId.isBitSet(MatchMakingId::B_linkDead);
}

// ----------------------------------------------------------------------

bool PlayerObject::adjustLotCount(int adjustment)
{
	CreatureObject * const owner = getCreatureObject();
	if (!owner)
		return false;

	if (isAuthoritative())
	{
		if (adjustment > 0 &&
			(m_accountNumLots.get() + adjustment) > (ConfigServerGame::getMaxLotsPerAccount() + m_accountMaxLotsAdjustment.get()))
		{
			DEBUG_REPORT_LOG(true,("Account %lu is not allowed to use any more lots.  (Already has %i, limit is %i, tried to use %i more.)\n",
								   m_stationId.get(), m_accountNumLots.get(), ConfigServerGame::getMaxLotsPerAccount() + m_accountMaxLotsAdjustment.get(), adjustment));
			return false;
		}
		else
		{
			m_accountNumLots = m_accountNumLots.get() + adjustment;
			// make sure the number of used lot doesn't go below 0, because
			// it means we would be giving the player extra lots
			if (m_accountNumLots.get() < 0)
				m_accountNumLots = 0;

			// if no longer over the lot limit, remove lot over limit restriction
			if (m_accountNumLots.get() <= owner->getMaxNumberOfLots())
			{
				m_accountNumLotsOverLimitSpam = 0;
				owner->removeObjVarItem("lotOverlimit");
			}

			return true;
		}
	}
	else
	{
		owner->sendControllerMessageToAuthServer(CM_adjustLotCount, new MessageQueueGenericValueType<int>(adjustment));
		return true;
	}
}

// ----------------------------------------------------------------------

void PlayerObject::setTitle(std::string const &title)
{
	if (isAuthoritative())
	{
		// Make sure the skill title requested is valid
		if (title == "city_gcw_region_defender")
		{
			std::pair<std::string, std::pair<bool, bool> > const & cityGcwDefenderRegionInfo = getCityGcwDefenderRegionInfo();
			if (!cityGcwDefenderRegionInfo.first.empty() && cityGcwDefenderRegionInfo.second.second)
			{
				if (title != m_skillTitle.get())
					m_skillTitle = title;
			}
		}
		else if (title == "guild_gcw_region_defender")
		{
			std::pair<std::string, std::pair<bool, bool> > const & guildGcwDefenderRegionInfo = getGuildGcwDefenderRegionInfo();
			if (!guildGcwDefenderRegionInfo.first.empty() && guildGcwDefenderRegionInfo.second.second)
			{
				if (title != m_skillTitle.get())
					m_skillTitle = title;
			}
		}
		else if (title.empty() || (title == "citizenship") || (SkillManager::getInstance().getSkill(title) != nullptr) || (CollectionsDataTable::isASlotTitle(title) != nullptr) || (CollectionsDataTable::isACollectionTitle(title) != nullptr) || (CollectionsDataTable::isAPageTitle(title) != nullptr) || (GuildRankDataTable::isARankTitle(title) != nullptr) || (CitizenRankDataTable::isARankTitle(title) != nullptr))
		{
			if (title != m_skillTitle.get())
				m_skillTitle = title;
		}
		else
		{
			WARNING(true, ("The player is trying to set a title that does not exist in the SkillManager or the CollectionsDataTable or the GuildRankDataTable or the CitizenRankDataTable: [%s]", title.c_str()));
		}
	}
}

// ----------------------------------------------------------------------

std::string const &PlayerObject::getTitle() const
{
	return m_skillTitle.get();
}

// ----------------------------------------------------------------------

void PlayerObject::setSpokenLanguage(int const languageId)
{
	if (isAuthoritative() && GameLanguageManager::isLanguageValid(languageId))
	{
		m_spokenLanguage = languageId;
	}
}

// ----------------------------------------------------------------------

int PlayerObject::getSpokenLanguage() const
{
	return m_spokenLanguage.get();
}

// ----------------------------------------------------------------------

bool PlayerObject::isIgnoring(NetworkId const &target, NetworkId const &source)
{
	std::string const &sourceName = NameManager::getInstance().getPlayerName(source);
	if (!sourceName.empty())
	{
		Object const * const o = NetworkIdManager::getObjectById(target);

		if (o)
		{
			ServerObject const * const so = o->asServerObject();
			if (so)
			{
				CreatureObject const * const co = so->asCreatureObject();
				if (co)
				{
					PlayerObject const * const targetPlayerObject = PlayerCreatureController::getPlayerObject(co);
					if (targetPlayerObject)
						return targetPlayerObject->isIgnoring(sourceName);
				}
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

void PlayerObject::setMaxForcePower(int value)
{
	if (isAuthoritative())
	{
		if (value < 0)
			value = 0;
		m_maxForcePower = value;
		if (m_forcePower.get() > value)
			m_forcePower = value;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setMaxForcePower,
			new MessageQueueGenericValueType<int>(value));
	}
}

//----------------------------------------------------------------------

void PlayerObject::setForcePower(int value)
{
	if (isAuthoritative())
	{
		if (value < 0)
			value = 0;
		else if (value > m_maxForcePower.get())
			value = m_maxForcePower.get();
		m_forcePower = value;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setForcePower,
			new MessageQueueGenericValueType<int>(value));
	}
}

//----------------------------------------------------------------------

void PlayerObject::setForcePowerRegenRate(float value)
{
	if ((isJedi() == 0) && (getMaxForcePower() == 0))
		return;

	if (isAuthoritative())
	{
		if (value < ConfigServerGame::getMinForcePowerRegenRate())
			value = ConfigServerGame::getMinForcePowerRegenRate();

		m_forceRegenRate = value;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setForceRegenRate,
			new MessageQueueGenericValueType<float>(value));
	}
}

//----------------------------------------------------------------------

void PlayerObject::regenerateForcePower(float time)
{
	int forcePower = getForcePower();
	if (forcePower < getMaxForcePower())
	{
		setForcePower(forcePower + getForcePowerRegen(time));
	}
}

//----------------------------------------------------------------------

/**
 * Returns the amount of force power regenerated for a given timeslice.
 *
 * @param time		amount of time that has passed
 *
 * @return the amount of force power regenerated
 */
int PlayerObject::getForcePowerRegen(float time)
{
	// make sure we are force sensitive
	if ((isJedi() == 0) && (getMaxForcePower() == 0))
		return 0;

	if (time <= 0)
	{
		WARNING(true, ("PlayerObject::getForcePowerRegen called for player %s "
			"with time %f <= 0", getAccountDescription().c_str(), time));
		return 0;
	}

	// double-check the regen rate
	float minRate = ConfigServerGame::getMinForcePowerRegenRate();
	if (minRate <= 0)
	{
		WARNING_STRICT_FATAL(true, ("PlayerObject::getForcePowerRegen: min regen "
			"rate = %f", minRate));
		minRate = 0.1f;
	}
	float rate = getForcePowerRegenRate();
	if (rate < minRate)
	{
		WARNING(true, ("PlayerObject::getForcePowerRegen called for player %s "
			"with regen rate %f < min rate %f", getAccountDescription().c_str(),
			rate, minRate));
		setForcePowerRegenRate(minRate);
		rate = minRate;
	}

	float regenValue = 0;

	// compute the amount of regen power accumulated; if it is over the threshold,
	// we regenerate
	float newValue = m_forceRegenValue.get() + rate * time;
	if (newValue > ConfigServerGame::getRegenThreshold())
	{
		regenValue = static_cast<float>(floor(newValue));
		newValue -= regenValue;

	}
	m_forceRegenValue = newValue;

	return static_cast<int>(regenValue);
}	// PlayerObject::getForcePowerRegen

//----------------------------------------------------------------------

/**
 * Grant a Jedi slot to this account.  (Calling this function more than
 * once will not create multiple Jedi slots.)
 * NOTE:  Not guaranteed to succeed, because it depends on a connection to
 * a LoginServer, which is not guaranteed to be up.  Call this function
 * again if you don't get addJediToAccountAck() in a reasonable amount of time.
 */
void PlayerObject::addJediToAccount() const
{
	const CreatureObject * owner = safe_cast<const CreatureObject *>(ContainerInterface::getContainedByObject(*this));
	if (owner)
	{
		LOG("CustomerService", ("Jedi: Requesting extra character slot for player %s", getAccountDescription().c_str()));
		LoginUpgradeAccountMessage const luam(LoginUpgradeAccountMessage::UT_addJedi,m_stationId.get(),owner->getNetworkId(),getNetworkId(),"C++JediAdded","",false);
		GameServer::getInstance().sendToCentralServer(luam);
	}
}

// ----------------------------------------------------------------------

/**
 * Acknowledgement to addJediToAccount().  Until this function is called,
 * it is not guaranteed that the account is enabled for Jedi.
 */
void PlayerObject::addJediToAccountAck() const
{
	CreatureObject * owner = const_cast<CreatureObject *>(safe_cast<
		const CreatureObject *>(ContainerInterface::getContainedByObject(*this)));
	if (owner)
	{
		LOG("CustomerService", ("Jedi: Got extra character slot ack for player %s", getAccountDescription().c_str()));
		owner->addJediToAccountAck();
	}
}

// ----------------------------------------------------------------------

/**
 * Flags that this character is spectral in the account database.  Not
 * guaranteed -- repeat the call if makeSpectralOnAccountAck() is not called.
 */
void PlayerObject::makeSpectralOnAccount() const
{
	const CreatureObject * owner = safe_cast<const CreatureObject *>(ContainerInterface::getContainedByObject(*this));
	if (owner)
	{
		LoginUpgradeAccountMessage const luam(LoginUpgradeAccountMessage::UT_setSpectral,m_stationId.get(),owner->getNetworkId(), getNetworkId(),"C++SpectralSet","",false);
		GameServer::getInstance().sendToCentralServer(luam);
	}
}

// ----------------------------------------------------------------------

void PlayerObject::makeSpectralOnAccountAck() const
{
	const CreatureObject * owner = safe_cast<const CreatureObject *>(ContainerInterface::getContainedByObject(*this));
	if (owner)
	{
		Chat::sendSystemMessage(*owner,Unicode::narrowToWide("Character marked as spectral in the login database."),Unicode::String());
	}
}

// ----------------------------------------------------------------------

void PlayerObject::handleCMessageTo(const MessageToPayload &message)
{
	if (message.getMethod() == "C++JediAdded")
	{
		addJediToAccountAck();
	}
	else if (message.getMethod() == "C++SpectralSet")
	{
		makeSpectralOnAccountAck();
	}
	else if (message.getMethod() == "C++ClearTheater")
	{
		clearTheater();
	}
	else if (message.getMethod() == "C++TheaterDestroyed")
	{
		const std::vector<int8> & data = message.getPackedDataVector();
		std::string theaterIdString(data.begin(), data.end());
		if (theaterIdString == m_theaterId.get().getValueString())
			m_theaterId = NetworkId::cms_invalid;
	}
	else if (message.getMethod() == "C++RecalculateGcwRating")
	{
		m_gcwRatingActualCalcTime = 0;
		handleRecalculateGcwRating();
	}
	else if (message.getMethod() == "C++updateConnectedCharacterLfgData")
	{
		CreatureObject * const owner = getCreatureObject();
		if (m_sessionStartPlayTime.get() != 0)
		{
			if (owner)
			{
				// check for changes to the player's GCW Region information, and update if necessary; must be
				// called first as it updates information that is used later by other stuff in this function
				updateGcwRegionInfo();

				{
				LfgCharacterData lfgCharacterData;
				owner->getLfgCharacterData(lfgCharacterData);

				std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
				std::map<NetworkId, LfgCharacterData>::const_iterator iterFindLfg = connectedCharacterLfgData.find(owner->getNetworkId());
				if ((iterFindLfg == connectedCharacterLfgData.end()) || (iterFindLfg->second != lfgCharacterData))
					ServerUniverse::setConnectedCharacterLfgData(owner->getNetworkId(), lfgCharacterData);

				// grant GCW factional presence score
				if (!lfgCharacterData.locationFactionalPresenceGcwRegion.empty())
					grantGcwFactionalPresenceScore(lfgCharacterData.locationFactionalPresenceGcwRegion, *this, *owner);
				}

				{
				BiographyManager::requestBiography(owner->getNetworkId(), nullptr);
				}

				bool needsTitleCheck = false;

				// if the character is a guild member (or just sponsored), make sure the guild member
				// profession and level matches the actual profession and level of the character

				// also check to see if the guild member rank has changed, and if so, update the guild member rank information
				{
					int guildId = owner->getGuildId();
					bool const isGuildMember = (guildId != 0);

					// if character is not a guild member, see if character has been sponsored for guild membership
					if (guildId == 0)
					{
						guildId = GuildInterface::getGuildIdForSponsored(owner->getNetworkId());

						// no longer a guild member, so make sure guild rank is empty
						if (!m_guildRank.get().empty())
						{
							setGuildRank(BitArray());

							// force a title check to make sure the character isn't
							// using a guild rank title he is no longer entitled to
							needsTitleCheck = true;
						}
					}

					if (guildId > 0)
					{
						GuildMemberInfo const * const guildMemberInfo = GuildInterface::getGuildMemberInfo(guildId, owner->getNetworkId());
						if (guildMemberInfo)
						{
							std::string const & professionSkillTemplate = getSkillTemplate();
							bool const professionSkillTemplateDiffers = (!professionSkillTemplate.empty() && (guildMemberInfo->m_professionSkillTemplate != professionSkillTemplate));

							int const level = owner->getLevel();
							bool const levelDifers = ((level > 0) && (guildMemberInfo->m_level != level));

							if (professionSkillTemplateDiffers || levelDifers)
							{
								GuildInterface::setGuildMemberProfessionInfo(guildId, owner->getNetworkId(), (professionSkillTemplateDiffers ? professionSkillTemplate : guildMemberInfo->m_professionSkillTemplate), (levelDifers ? level : guildMemberInfo->m_level));
							}

							// check to see if guild rank has changed
							if (isGuildMember && (m_guildRank.get() != guildMemberInfo->m_rank))
							{
								setGuildRank(guildMemberInfo->m_rank);

								// force a title check to make sure the character isn't
								// using a guild rank title he is no longer entitled to
								needsTitleCheck = true;
							}
						}
					}
				}

				// if the character is a citizen, make sure the citizen profession and
				// level matches the actual profession and level of the character

				// also check to see if the citizen rank has changed, and if so, update the citizen rank information
				{
					std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(owner->getNetworkId());
					CitizenInfo const * const citizenInfo = (!cityIds.empty() ? CityInterface::getCitizenInfo(cityIds.front(), owner->getNetworkId()) : nullptr);
					if (!citizenInfo)
					{
						// not a citizen, so make sure the citizen rank is empty
						if (!m_citizenRank.get().empty())
						{
							setCitizenRank(BitArray());

							// force a title check to make sure the character isn't
							// using a citizen rank title he is no longer entitled to
							needsTitleCheck = true;
						}
					}
					else
					{
						std::string const & professionSkillTemplate = getSkillTemplate();
						bool const professionSkillTemplateDiffers = (!professionSkillTemplate.empty() && (citizenInfo->m_citizenProfessionSkillTemplate != professionSkillTemplate));

						int const level = owner->getLevel();
						bool const levelDifers = ((level > 0) && (citizenInfo->m_citizenLevel != level));

						if (professionSkillTemplateDiffers || levelDifers)
						{
							CityInterface::setCitizenProfessionInfo(cityIds.front(), owner->getNetworkId(), (professionSkillTemplateDiffers ? professionSkillTemplate : citizenInfo->m_citizenProfessionSkillTemplate), (levelDifers ? level : citizenInfo->m_citizenLevel));
						}

						// check to see if citizen rank has changed
						if (m_citizenRank.get() != citizenInfo->m_citizenRank)
						{
							setCitizenRank(citizenInfo->m_citizenRank);

							// force a title check to make sure the character isn't
							// using a citizen rank title he is no longer entitled to
							needsTitleCheck = true;
						}
					}
				}

				// if necessary, force a title check to make sure the character isn't
				// using a guild or citizen rank title he is no longer entitled to
				if (needsTitleCheck)
				{
					MessageToQueue::getInstance().sendMessageToC(owner->getNetworkId(),
						"C++CheckCharacterTitle", "", 5, false);
				}

				// check for changes to the player's citizenship information, and update if necessary
				updateCitizenshipInfo();

				// check for changes to the player's GCW Region Defender information, and update if necessary
				updateGcwDefenderRegionInfo();

				// check for update to the player's PGC rating information
				PgcRatingInfo const * const pgcRatingInfo = CityInterface::getPgcRating(owner->getNetworkId());
				if (pgcRatingInfo)
				{
					if (m_pgcRatingCount.get() != pgcRatingInfo->m_ratingCount)
					{
						m_pgcRatingCount.set(pgcRatingInfo->m_ratingCount);
					}

					if (m_pgcRatingTotal.get() != pgcRatingInfo->m_ratingTotal)
					{
						m_pgcRatingTotal.set(pgcRatingInfo->m_ratingTotal);
					}

					if (m_pgcLastRatingTime.get() != pgcRatingInfo->m_lastRatingTime)
					{
						m_pgcLastRatingTime.set(pgcRatingInfo->m_lastRatingTime);
					}
				}
			}

			// check/set the character's proper "character age" title
			checkAndSetCharacterAgeTitle(*this, false);
		}

		// check to see if the character needs to get (delay) flagged as enabled/disabled for guild war
		//
		// check to see if the guild member's guild war pvp status has changed, and if so, set the 
		// delay time to start counting down to when the guild war pvp status will get changed
		if (owner)
		{
			if (owner->getTimeToUpdateGuildWarPvpStatus())
			{
				if (ServerClock::getInstance().getGameTimeSeconds() > owner->getTimeToUpdateGuildWarPvpStatus())
					owner->setGuildWarEnabled(GuildInterface::getGuildMemberGuildWarEnabled(owner->getGuildId(), owner->getNetworkId()));
			}
			else if (owner->getGuildId())
			{
				// see if guild war pvp status has changed, and if yes, start countdown timer to change the status
				if (GuildInterface::getGuildMemberGuildWarEnabled(owner->getGuildId(), owner->getNetworkId()) != owner->getGuildWarEnabled())
					owner->setTimeToUpdateGuildWarPvpStatus(ServerClock::getInstance().getGameTimeSeconds() + static_cast<unsigned long>(ConfigServerGame::getPvpGuildWarExemptionExclusiveDelaySeconds()));
			}
			else if (owner->getGuildWarEnabled())
			{
				// not in a guild, so should be guild war pvp disabled
				owner->setGuildWarEnabled(false);
			}
		}

		// check for over lot limit condition
		setAccountNumLotsOverLimitSpam();
	}
	else if (message.getMethod() == "C++RequestChatFriendsList")
	{
		requestFriendList();
	}
	else if (message.getMethod() == "C++RequestChatIgnoreList")
	{
		requestIgnoreList();
	}
	else
	{
		IntangibleObject::handleCMessageTo(message);
	}
}

// ----------------------------------------------------------------------

void PlayerObject::endBaselines()
{
	IntangibleObject::endBaselines();

	const CreatureObject * owner = safe_cast<const CreatureObject *>(ContainerInterface::getContainedByObject(*this));

	if (owner != nullptr)
	{
		SharedCreatureObjectTemplate::Species const species = owner->getSpecies();
		setSpokenLanguage(GameLanguageManager::getStartingLanguage(species));

//		if (owner->isAuthoritative() && ServerUniverse::getInstance().getXpManager() != nullptr)
//		{
//			// get any xp we earned while offline
//			ServerUniverse::getInstance().getXpManager()->requestXp(*this);
//		}
	}

	// check for empty force values from the db
	if (m_maxForcePower.get() < 0)
		m_maxForcePower = 0;
	if (m_forcePower.get() < 0)
		m_forcePower = 0;
	if (m_forceRegenRate.get() < 0)
		m_forceRegenRate = 0;

	if (isAuthoritative())
	{
		GroupWaypointBuilder::updateGroupWaypoints(*this, true);

		// calculate rank values when loaded for the first time from DB
		Pvp::PvpRankInfo const & pvpRankInfo = Pvp::getRankInfo(m_currentGcwRating.get());
		m_currentGcwRank = pvpRankInfo.rank;

		if (pvpRankInfo.rank > 0)
		{
			m_currentGcwRankProgress = static_cast<float>(std::min(static_cast<int>(m_currentGcwRating.get()), pvpRankInfo.maxRating) - pvpRankInfo.minRating) * 100.0f / static_cast<float>(pvpRankInfo.maxRating - pvpRankInfo.minRating + 1);
		}
		else
		{
			m_currentGcwRankProgress = 0.0f;
		}

		m_maxGcwImperialRank = Pvp::getRankInfo(m_maxGcwImperialRating.get()).rank;
		m_maxGcwRebelRank = Pvp::getRankInfo(m_maxGcwRebelRating.get()).rank;

		// depersist chat statistics
		int temp;
		DynamicVariableList const & objVars = getObjVars();
		if (objVars.getItem(OBJVAR_CHAT_SPAM_TIME_END_INTERVAL, temp))
		{
			m_chatSpamTimeEndInterval = temp;

			if (objVars.getItem(OBJVAR_CHAT_SPAM_SPATIAL, temp))
				m_chatSpamSpatialNumCharacters = temp;
			else
				m_chatSpamSpatialNumCharacters = 0;

			if (objVars.getItem(OBJVAR_CHAT_SPAM_NON_SPATIAL, temp))
				m_chatSpamNonSpatialNumCharacters = temp;
			else
				m_chatSpamNonSpatialNumCharacters = 0;

			m_chatSpamNextTimeToSyncWithChatServer = 0;
		}
		else
		{
			m_chatSpamTimeEndInterval = static_cast<int>(::time(nullptr)) + (ConfigServerGame::getChatSpamLimiterIntervalMinutes() * 60);
			m_chatSpamSpatialNumCharacters = 0;
			m_chatSpamNonSpatialNumCharacters = 0;
			m_chatSpamNextTimeToSyncWithChatServer = 0;
		}

		// depersist privileged title
		if (objVars.getItem(OBJVAR_PRIVILEGED_TITLE, temp))
			m_privledgedTitle.set(static_cast<int8>(temp));
		else
			m_privledgedTitle.set(static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer));

		// depersist squelch info
		m_squelchedById = NetworkId::cms_invalid;
		m_squelchedByName = std::string();
		m_squelchExpireTime = 0;

		NetworkId tempNetworkId;
		std::string tempString;
		if ((objVars.getItem(OBJVAR_SQUELCH_ID, tempNetworkId)) && (objVars.getItem(OBJVAR_SQUELCH_NAME, tempString)) && (objVars.getItem(OBJVAR_SQUELCH_EXPIRE, temp)))
		{
			if (tempNetworkId.isValid())
			{						
				if (temp < 0)
				{
					m_squelchedById = tempNetworkId;
					m_squelchedByName = tempString;
					m_squelchExpireTime = -1;
				}
				else if (temp > 0)
				{
					int const gameTimeNow = static_cast<int>(ServerClock::getInstance().getGameTimeSeconds());
					if (gameTimeNow < temp)
					{
						m_squelchedById = tempNetworkId;
						m_squelchedByName = tempString;
						m_squelchExpireTime = static_cast<int32>(::time(nullptr)) + (temp - gameTimeNow);
					}
				}
			}
		}

		// remove expired GCW contribution
		{
			DynamicVariableList::NestedList const gcwContribution(getObjVars(), "gcwContributionTracking");
			if (!gcwContribution.empty())
			{
				int const timeExpired = static_cast<int>(::time(nullptr)) - (60 * 60 * 24 * 30); // 30 days
				std::list<std::string> gcwContributionToRemove;
				int timeLastContributed;
				for (DynamicVariableList::NestedList::const_iterator i = gcwContribution.begin(); i != gcwContribution.end(); ++i)
				{
					if (!Pvp::getGcwScoreCategory(i.getName()) || !i.getValue(timeLastContributed) || (timeLastContributed < timeExpired))
						gcwContributionToRemove.push_back(std::string("gcwContributionTracking.") + i.getName());
				}

				if (!gcwContributionToRemove.empty())
				{
					for (std::list<std::string>::const_iterator iterRemove = gcwContributionToRemove.begin(); iterRemove != gcwContributionToRemove.end(); ++iterRemove)
						removeObjVarItem(*iterRemove);
				}
			}
		}

		// check/set the character's proper "character age" title (age may/will changed while the character was logged out)
		checkAndSetCharacterAgeTitle(*this, true);

		// start timer to periodically update logged in character global data
		MessageToQueue::sendRecurringMessageToC(getNetworkId(), "C++updateConnectedCharacterLfgData", std::string(), 60);
	}
}

//-----------------------------------------------------------------------

void PlayerObject::onLoadedFromDatabase()
{
	IntangibleObject::onLoadedFromDatabase();

	// clear any deprecated quests
	{
		static std::vector<uint32> deprecatedQuests;
		for (PlayerQuestDataMap::const_iterator i=m_quests.begin(); i!=m_quests.end(); ++i)
		{
			if (!QuestManager::getQuest(i->first))
				deprecatedQuests.push_back(i->first);
		}

		for (std::vector<uint32>::const_iterator oldQuest=deprecatedQuests.begin(); oldQuest!=deprecatedQuests.end(); ++oldQuest)
			questClearQuest(*oldQuest);

		deprecatedQuests.clear();
	}
	CreatureObject * const owner = getCreatureObject();
	if(owner && owner->isInitialized())
	{
		owner->fixupPersistentBuffsAfterLoading();
		owner->fixupLevelXpAfterLoading();
	}
}

//-----------------------------------------------------------------------

Waypoint const PlayerObject::createWaypoint(const Location & location, bool ignoreLimits)
{
	float const maxSaneCoordinate = 100000;
	Vector const & where = location.getCoordinates();
	if ((where.x < -maxSaneCoordinate) ||
		(where.x > maxSaneCoordinate) ||
		(where.y < -maxSaneCoordinate) ||
		(where.y > maxSaneCoordinate) ||
		(where.z < -maxSaneCoordinate) ||
		(where.z > maxSaneCoordinate))
	{
		WARNING(true,("INSANE_WAYPOINT_COORDINATES:  PlayerObject %s attempted to create a waypoint with coordinates that were way out of range", getNetworkId().getValueString().c_str()));
		return Waypoint();
	}

	if (ignoreLimits || m_waypoints.size() < static_cast<size_t>(ConfigServerGame::getMaxWaypointsPerCharacter()))
	{
		Waypoint waypoint(ObjectIdManager::getNewObjectId());
		waypoint.setLocation(location);
		m_waypoints.set(waypoint.getNetworkId(), waypoint);
		Archive::AutoDeltaMap<NetworkId, Waypoint>::const_iterator f = m_waypoints.find(waypoint.getNetworkId());
		if(f != m_waypoints.end())
		{
			Waypoint & w = const_cast<Waypoint &>(f->second);
			m_callback->connect(w.getChangeNotification(), *this, &PlayerObject::onWaypointChanged);

			// call TRIG_WAYPOINT_CREATED
			CreatureObject * const owner = getCreatureObject();
			if(owner)
			{
				GameScriptObject * script = owner->getScriptObject();
				if(script)
				{
					ScriptParams params;
					params.addParam(w.getNetworkId());
					IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_WAYPOINT_CREATED, params));
				}
			}

			// set a reasonable default color (the caller can change it)
			if (strncmp("space_",location.getSceneId(),6)==0)
				w.setColor(Waypoint::Space);
			else
				w.setColor(Waypoint::Blue);

			return w;
		}
	}

	return Waypoint();
}

//-----------------------------------------------------------------------

Waypoint const PlayerObject::createOrUpdateReusableWaypoint(const Location & location, const std::string & objvarTrackingName, const Unicode::String & waypointNameIfCreated, int waypointColorIfCreated /*= -1*/)
{
	const std::string objvarName = std::string("reuseableWp.") + objvarTrackingName;
	const DynamicVariableList & objvars = getObjVars();
	NetworkId waypointId;
	bool newWpCreated = false;
	if (!objvars.hasItem(objvarName) || (objvars.getType(objvarName) != DynamicVariable::NETWORK_ID))
	{
		// create a new waypoint
		Waypoint waypoint(createWaypoint(location, true));
		newWpCreated = true;
		waypointId = waypoint.getNetworkId();
		setObjVarItem(objvarName, waypointId);
	}
	else
	{
		objvars.getItem(objvarName, waypointId);

		// find the existing waypoint in the datapad
		std::vector<NetworkId> waypointsInDatapad;
		getWaypointsInDatapad(waypointsInDatapad);
		if(std::find(waypointsInDatapad.begin(), waypointsInDatapad.end(), waypointId) == waypointsInDatapad.end())
		{
			// original waypoint has been destroyed, create a new one
			Waypoint waypoint(createWaypoint(location, true));
			newWpCreated = true;
			waypointId = waypoint.getNetworkId();
			setObjVarItem(objvarName, waypointId);
		}
	}

	if (waypointId != NetworkId::cms_invalid)
	{
		Waypoint waypoint = Waypoint::getWaypointById(waypointId);
		waypoint.setActive(true);
		waypoint.setLocation(location);

		if (newWpCreated)
		{
			if (!waypointNameIfCreated.empty())
				waypoint.setName(waypointNameIfCreated);

			if ((waypointColorIfCreated >= 0) && (waypointColorIfCreated < static_cast<int>(Waypoint::NumColors)))
				waypoint.setColor(static_cast<uint8>(waypointColorIfCreated));
		}

		return waypoint;
	}

	return Waypoint();
}

//-----------------------------------------------------------------------

void PlayerObject::destroyWaypoint(const NetworkId & id)
{
	// call TRIG_WAYPOINT_DESTROYED
	Archive::AutoDeltaMap<NetworkId, Waypoint>::const_iterator f = m_waypoints.find(id);
	if(f != m_waypoints.end())
	{
		Waypoint & w = const_cast<Waypoint &>(f->second);
		CreatureObject * const owner = getCreatureObject();
		if(owner)
		{
			GameScriptObject * script = owner->getScriptObject();
			if(script)
			{
				ScriptParams params;
				params.addParam(w.getNetworkId());
				IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_WAYPOINT_DESTROYED, params));
			}
		}
	}
	m_waypoints.erase(id);
}

//-----------------------------------------------------------------------

void PlayerObject::onWaypointChanged(const Waypoint & waypointData)
{
	Archive::AutoDeltaMap<NetworkId, Waypoint>::const_iterator f = m_waypoints.find(waypointData.getNetworkId());
	if(f != m_waypoints.end())
	{
		if(isAuthoritative())
			m_waypoints.set(waypointData.getNetworkId(), waypointData);
	}
}

//-----------------------------------------------------------------------

void PlayerObject::onWaypointCreated(const NetworkId &, const Waypoint & w)
{
	m_callback->connect(const_cast<Waypoint &>(w).getChangeNotification(), *this, &PlayerObject::onWaypointChanged);
	GroupWaypointBuilder::updateGroupWaypoints(*this, true);
}

// ----------------------------------------------------------------------

void PlayerObject::onWaypointSet(NetworkId const &, Waypoint const &, Waypoint const &)
{
	GroupWaypointBuilder::updateGroupWaypoints(*this, true);
}

// ----------------------------------------------------------------------

void PlayerObject::onWaypointRemoved(NetworkId const &, Waypoint const &)
{
	GroupWaypointBuilder::updateGroupWaypoints(*this, true);
}

//-----------------------------------------------------------------------

Waypoint & PlayerObject::getWaypoint(const NetworkId & id)
{
	Archive::AutoDeltaMap<NetworkId, Waypoint>::const_iterator f = m_waypoints.find(id);
	if(f != m_waypoints.end())
	{
		return const_cast<Waypoint &>(f->second);
	}

	static const Waypoint empty;
	static Waypoint notFound;
	notFound = empty;
	return notFound;
}

//-----------------------------------------------------------------------

void PlayerObject::getWaypointsInDatapad(std::vector<NetworkId> & target)
{
	Archive::AutoDeltaMap<NetworkId, Waypoint>::const_iterator i;
	for(i = m_waypoints.begin(); i != m_waypoints.end(); ++i)
	{
		target.push_back(i->second.getNetworkId());
	}
}

//-----------------------------------------------------------------------

void PlayerObject::getWaypointAttributes(const NetworkId & waypointId, ServerObject::AttributeVector & data)
{
	CreatureObject * const owner = getCreatureObject();
	if(owner)
	{
		GameScriptObject * script = owner->getScriptObject();
		if(script)
		{
			Archive::AutoDeltaMap<NetworkId, Waypoint>::const_iterator f = m_waypoints.find(waypointId);
			if(f != m_waypoints.end())
			{
				if (script->hasTrigger (Scripting::TRIG_WAYPOINT_ON_GET_ATTRIBUTES))
				{
					static const int MAX_ATTRIBS = 20;

					// allow scripts to add attribute data to the vector
					static std::vector<const char *>            names   (MAX_ATTRIBS, static_cast<const char *>(0));
					static std::vector<const Unicode::String *> attribs (MAX_ATTRIBS, static_cast<const Unicode::String *>(0));

					//-- clear the vectors
					names.clear    ();
					attribs.clear  ();
					names.resize   (MAX_ATTRIBS, static_cast<const char *>(0));
					attribs.resize (MAX_ATTRIBS, static_cast<const Unicode::String *>(0));

					ScriptParams params;
					params.addParam (waypointId);
					params.addParam (owner->getNetworkId());
					params.addParam (names);
					params.addParam (attribs);

					IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_WAYPOINT_ON_GET_ATTRIBUTES, params));

					const std::vector<const char *> & newNames              = params.getStringArrayParam  (2);
					const std::vector<const Unicode::String *> & newAttribs = params.getUnicodeArrayParam (3);

					// copy the returned strings to data
					for (int i = 0; i < MAX_ATTRIBS; ++i)
					{
						const char * const            newName   = newNames   [i];
						const Unicode::String * const newAttrib = newAttribs [i];

						if (newName && *newName && newAttrib && !newAttrib->empty ())
						{
							data.push_back (std::make_pair (std::string (newName), *newAttrib));
						}
						else
							break;
					}
				}

			}
		}
	}
}

//-----------------------------------------------------------------------

std::string PlayerObject::getAccountDescription(const NetworkId & creatureId)
{
	std::string result = getAccountDescription(safe_cast<ServerObject *>(NetworkIdManager::getObjectById(creatureId)));
	if (result.empty() || result == "UNKNOWN")
		result = creatureId.getValueString();
	return result;
}

//-----------------------------------------------------------------------

std::string PlayerObject::getAccountDescription(const ServerObject * creature)
{
	std::string desc = "";
	if (creature)
	{
		const PlayerObject * player = PlayerCreatureController::getPlayerObject(creature->asCreatureObject());
		if (creature->asCreatureObject() && player)
		{
			return player->getAccountDescription();
		}
		else if (creature->asCreatureObject())
		{
			desc += "Non-Player ";
			desc += Unicode::wideToNarrow(creature->getObjectName());
			desc += "(";
			desc += creature->getNetworkId().getValueString();
			desc += ") ";
			desc += creature->getObjectTemplateName();
		}
		else
			desc = creature->getNetworkId().getValueString();
	}
	else
	{
		desc = "UNKNOWN";
	}
	return desc;
}

//-----------------------------------------------------------------------

std::string PlayerObject::getAccountDescription() const
{
	const ServerObject * owner = safe_cast<const ServerObject *>(
		ContainerInterface::getContainedByObject(*this));
	if (owner == nullptr)
		return "UNKNOWN";

	bool isGod = false;
	bool isSecure = false;
	Client * client = nullptr;
	if (owner && owner->getClient())
	{
		client = owner->getClient();
		if (client->isGod())
			isGod = true;
		if (client->isGodValidated())
			isSecure = true;
	}

	std::string desc;
	desc += "Player ";
	desc += Unicode::wideToNarrow(owner->getObjectName());
	desc += "(";
	desc += owner->getNetworkId().getValueString();
	desc += ") StationId(";
	char stationIdBuf[32] = {"\0"};
	snprintf(stationIdBuf, sizeof(stationIdBuf), "%lu", m_stationId.get());
	stationIdBuf[sizeof(stationIdBuf) - 1] = 0;
	desc += stationIdBuf;
	if (isGod)
		desc += " isGod";
	else if (isSecure)
		desc += " isSecure";

	if (client && client->isFreeTrialAccount())
	{
		desc += " isFreeTrial";
	}

	desc += ")";

	return desc;
}

//-----------------------------------------------------------------------

void PlayerObject::logChat(int const logIndex)
{
	if (m_chatLog != nullptr)
	{
		time_t const logTime = Os::getRealSystemTime();

		ChatLogEntry chatLogEntry;

		chatLogEntry.m_index = logIndex;
		chatLogEntry.m_time = logTime;

		m_chatLog->push_back(chatLogEntry);

		cleanChatLog();
	}
}

//-----------------------------------------------------------------------

PlayerObject::ChatLog const &PlayerObject::getChatLog() const
{
	NOT_NULL(m_chatLog);

	return *m_chatLog;
}

//-----------------------------------------------------------------------

void PlayerObject::checkStomach()
{
	int temp = 0;

	const CreatureObject * const creature = getCreatureObject();

	if(creature)
	{
		const DynamicVariableList & objvars = creature->getObjVars();

		if (objvars.getItem("stomach.food", temp))
		{
			m_food = temp;
		}
		if (objvars.getItem("stomach.drink",temp))
		{
			m_drink = temp;
		}
		if (objvars.getItem("stomach.medicine",temp))
		{
			m_meds = temp;
		}
	}
}

//-----------------------------------------------------------------------

void PlayerObject::cleanChatLog()
{
	// See if we need to purge any old messages

	time_t currentTime;
	time(&currentTime);

	if (m_chatLogPurgeTime != currentTime)
	{
		m_chatLogPurgeTime = currentTime;

		// See if anything needs to be purged from the front of the logs

		time_t const chatLogTime = ConfigServerUtility::getChatLogMinutes() * 60;
		int chatLogCount = static_cast<int>(m_chatLog->size());

		ChatLog::iterator iterChatLog = m_chatLog->begin();
		while (!m_chatLog->empty())
		{
			const time_t messageTime = iterChatLog->m_time;

			if (messageTime < (currentTime - chatLogTime) || (chatLogCount > ConfigServerUtility::getPlayerMaxChatLogLines()))
			{
				iterChatLog = m_chatLog->erase(iterChatLog);
				--chatLogCount;
			}
			else
			{
				break;
			}
		}
	}
}

//----------------------------------------------------------------------

bool PlayerObject::isJedi() const
{
	return false;
}

//------------------------------------------------------------------------------------------

bool PlayerObject::getIsUnsticking() const
{
	return m_isUnsticking;
}

//------------------------------------------------------------------------------------------

void PlayerObject::setIsUnsticking(bool value)
{
	m_isUnsticking = value;
}

// ========================================================================================
//----------------------------------------------------------------------

void PlayerObject::findFriend(const std::string & friendName)
{
	// ask the name manager for the network id of the friend
	std::string normalizedName = NameManager::normalizeName(friendName);
	NetworkId target = NameManager::getInstance().getPlayerId(normalizedName);
	const CreatureObject * creatureObject = getCreatureObject();
	if(target != NetworkId::cms_invalid)
	{
		if(creatureObject)
		{
			std::string myName = Unicode::wideToNarrow(creatureObject->getAssignedObjectFirstName());

			// shortcut: if the player is on this server, don't send a
			// messageTo
			CreatureObject * friendObject = safe_cast<CreatureObject *>(NetworkIdManager::getObjectById(target));
			if(friendObject && friendObject->isAuthoritative() && PlayerCreatureController::getPlayerObject(friendObject))
			{
				// extract the PlayerObject from the CreatureObject
				PlayerObject * friendPlayer = PlayerCreatureController::getPlayerObject(friendObject);
				if(friendPlayer)
				{
					// go ahead and invoke replyLocationToFriend on the
					// local PlayerObject.
					friendPlayer->replyLocationToFriend(myName, creatureObject->getNetworkId());
				}
			}
			else
			{
				// the player is not on this server, send a message to C for the
				// friend's network ID.
				Archive::ByteStream payload;
				Archive::put(payload, creatureObject->getNetworkId());
				Archive::put(payload, myName);

				//@todo: This should really use sprintf to pack the data into a string, rather than reintrepreting a byte stream as a string

				MessageToQueue::getInstance().sendMessageToC(target, "FindFriend", std::string(reinterpret_cast<const char * const>(payload.getBuffer()), payload.getSize()), 0, false);
			}
		}
	}
	else
	{
		StringId noName("ui_cmnty", "friend_location_failed_noname");
		Chat::sendSystemMessage(*creatureObject, noName, Unicode::emptyString);
	}
}

//----------------------------------------------------------------------

void PlayerObject::replyLocationToFriend(const std::string & friendName, const NetworkId & friendNetworkId)
{
	// Do not send location information to players who are not
	// on this player's friend list (e.g. the Bounty hunter trying
	// to find a Jedi)
	if(isFriend(friendName))
	{
		// the requesting player is our friend, send the location
		CreatureObject * creatureObject = getCreatureObject();
		if(creatureObject)
		{
			Vector coordinates = creatureObject->findPosition_w();
			std::string scene = ServerWorld::getSceneId();
			Location location(coordinates, NetworkId::cms_invalid, Location::getCrcBySceneName(scene));
			// If the requestor is on this server, invoke the response
			// directly, otherwise send a message to the remote
			// creature object with the data.
			CreatureObject * friendCreatureObject = safe_cast<CreatureObject *>(NetworkIdManager::getObjectById(friendNetworkId));
			if(friendCreatureObject && friendCreatureObject->isAuthoritative())
			{
				// retrieve the PlayerObject associated with the
				// character and give it the coordinates
				PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(friendCreatureObject);
				if(playerObject)
				{
					playerObject->receiveLocationToFriend(creatureObject->getNetworkId(), location);
				}
				else
				{
					// send an error to the requesting player
					failLocationToFriend(friendNetworkId);
				}
			}
			else
			{
				// the character is not on this server, send the response
				// to the requestor's network Id
				Archive::ByteStream response;
				Archive::put(response, location);
				Archive::put(response, creatureObject->getNetworkId());
				MessageToQueue::getInstance().sendMessageToC(friendNetworkId, "LocationToFriend", std::string(reinterpret_cast<const char * const>(response.getBuffer()), response.getSize()), 0, false);

			}
		}
		else
		{
			// send an error to the requesting player
			failLocationToFriend(friendNetworkId);
		}
	}
	else
	{
		// send a failure to the requestor, let them know that
		// both players must be on the friends list
		failLocationToFriend(friendNetworkId);
	}
}

//----------------------------------------------------------------------

void PlayerObject::receiveLocationToFriend(const NetworkId & friendId, const Location & friendLocation)
{
	// receiving a response to the findFriend request
	CreatureObject * creatureObject = getCreatureObject();
	if(creatureObject)
	{
		const DynamicVariableList & objvars = creatureObject->getObjVars();
		NetworkId friendWaypointId;
		bool newWpCreated = false;
		if(! objvars.hasItem("friend_waypoint_id"))
		{
			// create a new "friend" waypoint
			Waypoint friendWaypoint(createWaypoint(friendLocation, true));
			newWpCreated = true;
			friendWaypointId = friendWaypoint.getNetworkId();
			creatureObject->setObjVarItem("friend_waypoint_id", friendWaypointId);
		}
		else
		{
			objvars.getItem("friend_waypoint_id", friendWaypointId);
			// find the existing waypoint in the datapad
			std::vector<NetworkId> waypointsInDatapad;
			getWaypointsInDatapad(waypointsInDatapad);
			if(std::find(waypointsInDatapad.begin(), waypointsInDatapad.end(), friendWaypointId) == waypointsInDatapad.end())
			{
				// original waypoint has been destroyed, create a new one
				Waypoint newWaypoint(createWaypoint(friendLocation, true));
				newWpCreated = true;
				friendWaypointId = newWaypoint.getNetworkId();
				creatureObject->setObjVarItem("friend_waypoint_id", friendWaypointId);
			}
		}

		std::string friendName = NameManager::getInstance().getPlayerName(friendId);
		if(friendWaypointId != NetworkId::cms_invalid)
		{
			Waypoint friendWaypoint = Waypoint::getWaypointById(friendWaypointId);
			friendWaypoint.setActive(true);
			friendWaypoint.setLocation(friendLocation);
			friendWaypoint.setName(Unicode::narrowToWide(NameManager::getInstance().getPlayerName(friendId)));

			// findFriend waypoint is purple by default
			if (newWpCreated)
				friendWaypoint.setColor(Waypoint::Purple);

			Unicode::String oob;
			ProsePackage pp;
			if(! friendName.empty())
			{
				if (newWpCreated)
				{
					pp.actor.str = Unicode::narrowToWide(friendName);
					pp.stringId = StringId("ui_cmnty", "friend_location_create_new_wp");
					OutOfBandPackager::pack(pp, -1, oob);
					Chat::sendSystemMessage(*creatureObject, Unicode::emptyString, oob);
				}
				else
				{
					pp.actor.str = Unicode::narrowToWide(friendName);
					pp.stringId = StringId("ui_cmnty", "friend_location");
					OutOfBandPackager::pack(pp, -1, oob);
					Chat::sendSystemMessage(*creatureObject, Unicode::emptyString, oob);
				}
			}
			else
			{
				StringId noName("ui_cmnty", "friend_location_failed_noname");
				Chat::sendSystemMessage(*creatureObject, noName, Unicode::emptyString);
			}
		}
		else
		{
			if( !friendName.empty())
			{
				ProsePackage pp;
				pp.actor.str = Unicode::narrowToWide(friendName);
				pp.stringId = StringId("ui_cmnty", "friend_location_failed");
				Unicode::String oob;
				OutOfBandPackager::pack(pp, -1, oob);
				Chat::sendSystemMessage(*creatureObject, Unicode::emptyString, oob);
			}
			else
			{
				StringId noName("ui_cmnty", "friend_location_failed_noname");
				Chat::sendSystemMessage(*creatureObject, noName, Unicode::emptyString);
			}
		}
	}
}

//----------------------------------------------------------------------

void PlayerObject::failLocationToFriend(const NetworkId & friendId)
{
	CreatureObject * requestor = safe_cast<CreatureObject *>(NetworkIdManager::getObjectById(friendId));
	if(requestor && requestor->isAuthoritative())
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(requestor);
		if(player)
		{
			player->receiveFailLocationToFriend(getCreatureObject()->getNetworkId());
		}
	}
	else
	{
		Archive::ByteStream response;
		Archive::put(response, getCreatureObject()->getNetworkId());
		MessageToQueue::getInstance().sendMessageToC(friendId, "FailLocationToFriend", std::string(reinterpret_cast<const char * const>(response.getBuffer()), response.getSize()), 0, false);
	}
}

//----------------------------------------------------------------------

void PlayerObject::receiveFailLocationToFriend(const NetworkId & friendId)
{
	CreatureObject * creatureObject = getCreatureObject();
	if(creatureObject)
	{
		std::string friendName = NameManager::getInstance().getPlayerName(friendId);
		if(! friendName.empty())
		{
			ProsePackage pp;
			pp.actor.str = Unicode::narrowToWide(friendName);
			pp.stringId = StringId("ui_cmnty", "friend_location_failed");
			Unicode::String oob;
			OutOfBandPackager::pack(pp, -1, oob);
			Chat::sendSystemMessage(*creatureObject, Unicode::emptyString, oob);
		}
		else
		{
			StringId noName("ui_cmnty", "friend_location_failed_noname");
			Chat::sendSystemMessage(*creatureObject, noName, Unicode::emptyString);
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObject::setPriviledgedTitle(int8 priviledgedTitle)
{
	if (isAuthoritative())
	{
		if (m_privledgedTitle.get() != priviledgedTitle)
		{
			// if becoming a warden, send system message and warden introduction mail
			if (priviledgedTitle == static_cast<int8>(PlayerDataPriviledgedTitle::Warden))
			{
				CreatureObject * const owner = getCreatureObject();
				if (owner)
				{
					static const Unicode::String mailSubject = Unicode::narrowToWide("@" + StringId("warden", "warden_introduction_subject").getCanonicalRepresentation());
					
					ProsePackage pp;
					pp.stringId = StringId("warden", "warden_introduction_body");

					Unicode::String oob;
					OutOfBandPackager::pack(pp, -1, oob);

					Chat::sendPersistentMessage("system", Unicode::wideToNarrow(owner->getAssignedObjectName()), mailSubject, Unicode::emptyString, oob);

					Chat::sendSystemMessage(*owner, StringId("warden", "grant_warden"), Unicode::emptyString);
				}
			}
			// if warden being revoked, send system message
			else if (m_privledgedTitle.get() == static_cast<int8>(PlayerDataPriviledgedTitle::Warden))
			{
				CreatureObject * const owner = getCreatureObject();
				if (owner)
					Chat::sendSystemMessage(*owner, StringId("warden", "revoke_warden"), Unicode::emptyString);
			}

			m_privledgedTitle.set(priviledgedTitle);

			// persist privileged title
			if (priviledgedTitle != static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer))
				setObjVarItem(OBJVAR_PRIVILEGED_TITLE, static_cast<int>(priviledgedTitle));
			else
				removeObjVarItem(OBJVAR_PRIVILEGED_TITLE);
		}
	}
	else
	{
		CreatureObject * const owner = getCreatureObject();
		if (owner)
			owner->sendControllerMessageToAuthServer(CM_setPriviledgedTitle, new MessageQueueGenericValueType<int>(static_cast<int>(priviledgedTitle)));
	}
}

//------------------------------------------------------------------------------------------

void PlayerObject::activateQuest(int questId)
{
	BitArray b = m_activeQuests.get();
	b.setBit(questId);
	m_activeQuests.set(b);
	ScriptParams params;
	params.addParam(questId);
	CreatureObject * creatureObject = getCreatureObject();
	if(creatureObject)
	{
		GameScriptObject * gameScriptObject = creatureObject->getScriptObject();
		if(gameScriptObject)
		{
			IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_QUEST_ACTIVATED, params));
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObject::deactivateQuest(int questId)
{
	BitArray b = m_activeQuests.get();
	b.clearBit(questId);
	m_activeQuests.set(b);
}

// ----------------------------------------------------------------------

void PlayerObject::completeQuest(int questId)
{
	BitArray b = m_activeQuests.get();
	b.clearBit(questId);
	m_activeQuests.set(b);
	b = m_completedQuests.get();
	b.setBit(questId);
	m_completedQuests.set(b);
}

// ----------------------------------------------------------------------

bool PlayerObject::isQuestActive(int questId) const
{
	return m_activeQuests.get().testBit(questId);
}

// ----------------------------------------------------------------------

bool PlayerObject::isQuestComplete(int questId) const
{
	return m_completedQuests.get().testBit(questId);
}

// ----------------------------------------------------------------------

void PlayerObject::clearCompletedQuest(int questId)
{
	BitArray b = m_completedQuests.get();
	b.clearBit(questId);
	m_completedQuests.set(b);
}

// ----------------------------------------------------------------------

/**
 * Checks if the theater assigned to this player should be spawned. NOTE: if we
 * spawn the theater, our theater data will be cleared.
 *
 * @param pos			our position
 * @param scene			our scene
 * @param speed			our speed
 * @param updateRadius	distance when we start receiving objects
 */
void PlayerObject::checkTheater(const Vector & pos, const std::string & scene,
	float speed, float updateRadius)
{
	if (!isAuthoritative())
		return;

	if (!hasTheater())
		return;

	if (m_theaterScene.get() != scene)
		return;

	// we want to spawn the theater so that all the objects will have been created
	// by the time the theater comes into view
	float timeToCreate = (m_theaterNumObjects.get() / IntangibleObject::getObjectsCreatedPerFrame()) * 0.25f;
	float creationDistance = m_theaterRadius.get() + updateRadius * 1.5f + speed * timeToCreate;
	float distance = pos.magnitudeBetweenSquared(m_theaterPosition.get());
	if (distance < creationDistance * creationDistance)
	{
		DEBUG_LOG("Theater", ("PlayerObject spawning theater at a distance of "
			"%f(%f, %f, %f, %f)\n", static_cast<float>(sqrt(distance)), creationDistance,
			speed, updateRadius, timeToCreate));
		IntangibleObject * theater = IntangibleObject::spawnTheater(m_theaterDatatable.get(),
			m_theaterPosition.get(), m_theaterScript.get(),
			static_cast<IntangibleObject::TheaterLocationType>(m_theaterLocationType.get()));
		if (theater != nullptr)
		{
			const CreatureObject * owner = getCreatureObject();
			if (owner != nullptr)
				theater->setPlayer(*owner);
			theater->setTheaterCreator(m_theaterCreator.get());
			if (!theater->setTheaterName(m_theaterName.get()))
			{
				WARNING(true, ("PlayerObject::checkTheater could not create "
					"theater with name %s", m_theaterName.get().c_str()));
				theater->permanentlyDestroy(DeleteReasons::SetupFailed);
				theater = nullptr;
			}
		}
		if (theater == nullptr)
		{
			CreatureObject * creatureObject = getCreatureObject();
			if (creatureObject)
			{
				GameScriptObject * gameScriptObject = creatureObject->getScriptObject();
				if (gameScriptObject)
				{
					ScriptParams params;
					params.addParam(m_theaterDatatable.get().c_str());
					params.addParam(m_theaterName.get().c_str());
					IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_PLAYER_THEATER_FAIL, params));
				}
			}
		}
		else
			m_theaterId = theater->getNetworkId();

		// there seems to be a problem if an objvar gets set and cleared in the
		// same frame, so we message ourselves to clean up next frame
		clearTheater();
		//m_theaterDatatable = "";
		//MessageToQueue::getInstance().sendMessageToC(getNetworkId(), "C++ClearTheater", std::string(), 0.1f, true);
	}
}	// PlayerObject::checkTheater

// ----------------------------------------------------------------------

/**
 * Sets the data for a theater that will be created when this player gets near
 * it. NOTE: we do not store the data if the player already has a theater.
 *
 * @param datatable		the datatable where the theater info is stored
 * @param position		the general location of the theater (the actual location
 *						is determined by getGoodLocation() when the theater is spawned)
 * @param scene			the planet/scene where the theater is
 * @param script		the script to be attached to the theater object
 * @param creator		who's creating the theater
 * @param name          the theater's name
 * @param locationType	how to create the theater
 *
 * @return true on success, false on fail
 */
bool PlayerObject::setTheater(const std::string & datatable, const Vector & position,
	const std::string & scene, const std::string & script, const NetworkId & creator,
	const std::string & name, IntangibleObject::TheaterLocationType locationType)
{
	if (!isAuthoritative())
	{
		WARNING(true, ("PlayerObject::setTheater called on unauthoritative player"));
		return false;
	}

	if (hasTheater())
	{
		WARNING(true, ("PlayerObject::setTheater called on player with theater"));
		return false;
	}

	m_theaterNumObjects = IntangibleObject::getNumObjects(datatable);
	if (m_theaterNumObjects.get() == 0)
	{
		WARNING(true, ("PlayerObject::setTheater called with invalid datatable %s",
			datatable.c_str()));
		return false;
	}

	if (ServerUniverse::getInstance().getPlanetByName(scene) == nullptr)
	{
		WARNING(true, ("PlayerObject::setTheater called with invalid scene %s",
			scene.c_str()));
		return false;
	}

	m_theaterDatatable = datatable;
	m_theaterPosition = position;
	m_theaterScene = scene;
	m_theaterScript = script;
	m_theaterCreator = creator;
	m_theaterName = name;
	m_theaterLocationType = static_cast<int>(locationType);

	// note that we set the radius to 3 times the radius returned since that is
	// the area that the theater could be created in
	m_theaterRadius = IntangibleObject::getRadius(datatable) * 3.0f;
	return true;
}

// ----------------------------------------------------------------------

/**
 * Clears the player's theater data.
 */
void PlayerObject::clearTheater()
{
	if (!isAuthoritative())
		return;

	m_theaterDatatable = "";
	m_theaterPosition = Vector(0,0,0);
	m_theaterScene = "";
	m_theaterScript = "";
	m_theaterRadius = 0;
	m_theaterCreator = NetworkId::cms_invalid;
	m_theaterName = "";
	m_theaterLocationType = IntangibleObject::TLT_getGoodLocation;
}

// ----------------------------------------------------------------------

/**
 * Checks if the player has theater data.
 */
bool PlayerObject::hasTheater() const
{
	return !m_theaterDatatable.get().empty();
}

// ----------------------------------------------------------------------

PlayerObject *PlayerObject::asPlayerObject()
{
	return this;
}

// ----------------------------------------------------------------------

PlayerObject const *PlayerObject::asPlayerObject() const
{
	return this;
}

// ======================================================================
// Quest System functions
// ======================================================================

PlayerObject::QuestResult PlayerObject::questCanActivateQuest(uint32 const questCrc) const
{
	Quest const * const quest=QuestManager::getQuest(questCrc);
	if (!quest)
		return QR_noSuchQuest;

	if (questHasActiveQuest(questCrc))
		return QR_alreadyActive;

	if (!quest->isRepeatable() && questHasCompletedQuest(questCrc))
		return QR_alreadyComplete;

	{
		std::vector<uint32> const & prerequisites = quest->getPrerequisiteQuests();
		for (std::vector<uint32>::const_iterator i=prerequisites.begin(); i!=prerequisites.end(); ++i)
		{
			if (!questHasCompletedQuest(*i))
				return QR_failedPrerequisites;
		}
	}

	{
		std::vector<uint32> const & exclusion = quest->getExclusionQuests();
		for (std::vector<uint32>::const_iterator i=exclusion.begin(); i!=exclusion.end(); ++i)
		{
			if (questHasCompletedQuest(*i) || questHasActiveQuest(*i))
				return QR_failedExclusion;
		}
	}

	return QR_success;
}

// ----------------------------------------------------------------------


PlayerObject::QuestResult PlayerObject::questActivateQuest(uint32 const questCrc, NetworkId const & questGiver)
{
	QuestResult canActivateResult = questCanActivateQuest(questCrc);
	if (canActivateResult != QR_success)
		return canActivateResult;

	questCheckQuestLimits();

	PlayerQuestData playerQuestData(questGiver);
	m_quests.set(questCrc, playerQuestData);
	setCurrentQuest(questCrc);

	CreatureObject * owner = getCreatureObject();
	if(owner)
	{
		GameScriptObject * script = owner->getScriptObject();
		if(script)
		{
			ScriptParams params;
			params.addParam(static_cast<int>(questCrc));
			IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_QUEST_ACTIVATED, params));
		}
	}

	questActivateQuestTask(questCrc, 0);

	return QR_success;
}

// ----------------------------------------------------------------------

void PlayerObject::questCheckQuestLimits()
{
	if (m_quests.size() > s_questLimit)
	{
		// Randomly remove some quests to get the number back down to the limit
		// Most of the quests in the list should be completed, so try
		// to prefer completed quests when picking ones to delete
		//
		// The hope is that game mechanisms will keep the number of quests
		// within the limits, and this code won't have to be used.

		CreatureObject * const owner = NON_NULL(getCreatureObject());

		size_t numberToDelete = m_quests.size() - s_questLimit;
		int previousNumberDeleted;
		if (!owner->getObjVars().getItem(OBJVAR_DELETED_QUEST_COUNT, previousNumberDeleted))
			previousNumberDeleted=0;
		IGNORE_RETURN(owner->setObjVarItem(OBJVAR_DELETED_QUEST_COUNT, previousNumberDeleted + static_cast<int>(numberToDelete)));

		// We need to sort the quests to delete by age
		SortableQuestList possibleQuestsToDelete;

		// First find all the completed quests than can be deleted
		for (PlayerQuestDataMap::const_iterator i=m_quests.begin(); i!=m_quests.end(); ++i)
		{
			PlayerQuestData const& questData = i->second;
			if (questData.isCompleted())
			{
				possibleQuestsToDelete.push_back(QuestDataToSort(i->first, questData));
			}
		}

		// Sort the possible quests by age
		possibleQuestsToDelete.sort();

		// Determine whether we found enough quests to delete
		if (possibleQuestsToDelete.size() < numberToDelete)
		{
			SortableQuestList additionalPossibleQuestsToDelete;

			// Our failure case has hit a failure case!  There aren't enough completed quests to stay within the limit.
			for (PlayerQuestDataMap::const_iterator i=m_quests.begin(); i!=m_quests.end() && possibleQuestsToDelete.size() < numberToDelete; ++i)
			{
				PlayerQuestData const& questData = i->second;
				if (!questData.isCompleted())
				{
					additionalPossibleQuestsToDelete.push_back(QuestDataToSort(i->first, questData));
				}
			}

			// Sort the additional quests by age
			additionalPossibleQuestsToDelete.sort();

			// Add the additional quests to the end of the possibilities list
			possibleQuestsToDelete.splice(possibleQuestsToDelete.end(),
			                              additionalPossibleQuestsToDelete,
			                              additionalPossibleQuestsToDelete.begin(),
			                              additionalPossibleQuestsToDelete.end());
		}

		while (numberToDelete > 0)
		{
			DEBUG_FATAL(possibleQuestsToDelete.empty(),("Programmer bug:  there is an error in the questCheckQuestLimits logic such that possibleQuestsToDelete was unexpectedly empty"));

			// Remove the "oldest" quest from the beginning of the list
			QuestDataToSort const& questDataToSort    = possibleQuestsToDelete.front();
			uint32 const           playerQuestDataKey = questDataToSort.GetPlayerQuestDataKey();
			questClearQuest(playerQuestDataKey);
			possibleQuestsToDelete.pop_front();
			--numberToDelete;

			LOG("CustomerService",("Removed quest %lu from player %s because there were too many quests in the journal", playerQuestDataKey, NON_NULL(getCreatureObject())->getNetworkId().getValueString().c_str()));
		}
	}
}

// ----------------------------------------------------------------------

PlayerObject::QuestResult PlayerObject::questCompleteQuest(uint32 const questCrc)
{
	Quest const * const quest = QuestManager::getQuest(questCrc);
	if (!quest)
		return QR_noSuchQuest;

	if(!questHasActiveQuest(questCrc))
		return QR_questNotActive;

	if(questHasCompletedQuest(questCrc))
		return QR_alreadyComplete;

	PlayerQuestDataMap::const_iterator questData=m_quests.find(questCrc);
	if (questData==m_quests.end())
		return QR_questNotActive;

	//clear any leftover tasks for this quest
	int taskId = 0;
	for(; taskId < quest->getNumberOfTasks(); ++taskId)
	{
		if(questHasActiveQuestTask(questCrc, taskId))
			questClearQuestTask(questCrc, taskId);
	}

	PlayerQuestData newQuestData(questData->second); // unfortunately have to copy the quest data, because AutoDeltaPackedMap does not have non-const iterators
	setPlayerQuestDataComplete(questCrc, newQuestData);

	CreatureObject * owner = getCreatureObject();
	if(owner)
	{
		ScriptParams params;
		params.addParam(static_cast<int>(questCrc));
		GameScriptObject * const scriptObject = owner->getScriptObject();
		if(scriptObject)
		{
			scriptObject->trigAllScripts(Scripting::TRIG_QUEST_COMPLETED, params);
		}
	}

	CreatureObject * const playerCreature = getCreatureObject();
	//see if this quest needs to go through our acceptance/completion UI flow
	if(QuestManager::isQuestAcceptable(quest->getName().getString()))
	{
		//-- Request the recipient's client to have the emitter object render the text.
		Controller * const controller = playerCreature->getController();
		if(controller)
		{
			setPendingRequestQuestInformation(questCrc, NetworkId::cms_invalid);
			MessageQueueGenericValueType<int> * const msg = new MessageQueueGenericValueType<int>(static_cast<int>(questCrc));
			controller->appendMessage(static_cast<int>(CM_requestCompleteQuest), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}
	else
	{
		questGrantQuestReward(static_cast<uint32>(questCrc), "");
	}

	return QR_success;
}

// ----------------------------------------------------------------------

PlayerObject::QuestResult PlayerObject::questGrantQuestReward(uint32 const questCrc, std::string const & selectedReward)
{
	Quest const * const quest=QuestManager::getQuest(questCrc);
	if (!quest)
		return QR_noSuchQuest;

	if(!questHasCompletedQuest(questCrc))
		return QR_questNotCompleted;

	PlayerQuestDataMap::const_iterator questData = m_quests.find(questCrc);

	PlayerQuestData newQuestData(questData->second); // unfortunately have to copy the quest data, because AutoDeltaPackedMap does not have non-const iterators

	if(newQuestData.hasReceivedReward())
	{
		return QR_alreadyReceivedReward;
	}

	CreatureObject * owner = getCreatureObject();
	if(owner)
	{
		ScriptParams params;
		params.addParam(static_cast<int>(questCrc));
		if(quest->isAnExclusiveStaticLootReward(selectedReward))
			params.addParam(selectedReward.c_str());
		else
			params.addParam("");
		GameScriptObject * const scriptObject = owner->getScriptObject();
		if(scriptObject)
		{
			if(scriptObject->trigAllScripts(Scripting::TRIG_QUEST_GRANT_REWARD, params) == SCRIPT_CONTINUE)
			{
				newQuestData.setHasReceivedReward(true);
				setPlayerQuestDataComplete(questCrc, newQuestData);
			}
		}
	}

	return QR_success;
}

// ----------------------------------------------------------------------

bool PlayerObject::questPlayerCanClaimRewardFor(uint32 const questCrc)
{
	PlayerQuestDataMap::const_iterator const questData = m_quests.find(questCrc);

	PlayerQuestData const newQuestData(questData->second); // unfortunately have to copy the quest data, because AutoDeltaPackedMap does not have non-const iterators

	return (questHasCompletedQuest(questCrc) && !newQuestData.hasReceivedReward());
}

// ----------------------------------------------------------------------

void PlayerObject::questClearQuest(uint32 const questCrc)
{
	Quest const * const quest=QuestManager::getQuest(questCrc);
	if(quest)
	{
		//clear any leftover tasks for this quest
		int taskId = 0;
		for(; taskId < quest->getNumberOfTasks(); ++taskId)
		{
			if(questHasActiveQuestTask(questCrc, taskId))
				questClearQuestTask(questCrc, taskId);
		}
	}

	m_quests.erase(questCrc);

	//-- if the cleared quest is the current quest, pick a new one
	if (questCrc == getCurrentQuest())
	{
		pickNewCurrentQuest();
	}

	CreatureObject * owner = getCreatureObject();
	if(owner)
	{
		ScriptParams params;
		params.addParam(static_cast<int>(questCrc));
		GameScriptObject * const scriptObject = owner->getScriptObject();
		if(scriptObject)
		{
			scriptObject->trigAllScripts(Scripting::TRIG_QUEST_CLEARED, params);
		}
	}
}

// ----------------------------------------------------------------------

PlayerObject::QuestResult PlayerObject::questCanActivateQuestTask(uint32 const questCrc, int const taskId) const
{
	Quest const * const quest = QuestManager::getQuest(questCrc);
	if (!quest)
		return QR_noSuchQuest;
	QuestTask const * const questTask = quest->getTask(taskId);
	if (!questTask)
		return QR_noSuchTask;
	PlayerQuestDataMap::const_iterator questData=m_quests.find(questCrc);
	if (questData==m_quests.end())
		return QR_questNotActive;
	if (questData->second.isTaskActive(taskId))
		return QR_alreadyActive;
	if (!questTask->isRepeatable() && questData->second.isTaskCompleted(taskId))
		return QR_alreadyComplete;

	// check for prerequisites/exclusions
	{
		std::vector<int> const & prerequisites = questTask->getPrerequisiteTasks();
		for(std::vector<int>::const_iterator i = prerequisites.begin(); i!= prerequisites.end(); ++i)
		{
			if (!questData->second.isTaskCompleted(*i))
				return QR_failedPrerequisites;
		}
	}
	{
		std::vector<int> const & exclustion = questTask->getExclusionTasks();
		for(std::vector<int>::const_iterator i = exclustion.begin(); i!= exclustion.end(); ++i)
		{
			if (questData->second.isTaskCompleted(*i) || questData->second.isTaskActive(*i))
				return QR_failedExclusion;
		}
	}

	return QR_success;
}

// ----------------------------------------------------------------------

PlayerObject::QuestResult PlayerObject::questActivateQuestTask(uint32 const questCrc, int const taskId)
{
	QuestResult canActivateResult = questCanActivateQuestTask(questCrc, taskId);
	if (canActivateResult != QR_success)
		return canActivateResult;

	PlayerQuestDataMap::const_iterator questData=m_quests.find(questCrc);
	DEBUG_FATAL(questData==m_quests.end(),("Programmer bug:  questCanActivateQuestTask(%lu, %i) returned success, but the quest was not in m_quests", questCrc, taskId));

	PlayerQuestData newQuestData(questData->second); // unfortunately have to copy the quest data, because AutoDeltaPackedMap does not have non-const iterators
	newQuestData.activateTask(taskId);
	m_quests.set(questCrc, newQuestData);

	const Quest * const quest = QuestManager::getQuest(questCrc);
	if(quest)
	{
		const QuestTask * const task = quest->getTask(taskId);
		if(task)
		{
			ScriptParams params;
			params.addParam(static_cast<int>(questCrc));
			params.addParam(static_cast<int>(taskId));

			CreatureObject * owner = getCreatureObject();
			if(owner)
			{
				GameScriptObject * script = owner->getScriptObject();
				if(script)
				{
					const std::string & attachScript = task->getAttachScript();
					if(!attachScript.empty())
					{
						refScript(*owner, attachScript);
						{
							IGNORE_RETURN(script->trigOneScript(attachScript, Scripting::TRIG_TASK_ACTIVATED, params));
						}
					}

					IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_SOME_TASK_ACTIVATED, params));
				}
			}

			CreatureObject * const co = getCreatureObject();
			if(co)
			{
				if(!task->getMusicOnActivate().empty())
				{
					Client * const c = co->getClient();
					if (c)
					{
						PlayMusicMessage const msg(task->getMusicOnActivate(), NetworkId::cms_invalid, 0, false);
						c->send(msg, true);
					}
				}
			}
		}
	}

	return QR_success;
}

// ----------------------------------------------------------------------

PlayerObject::QuestResult PlayerObject::questCompleteQuestTask(uint32 const questCrc, int const taskId)
{
	Quest const * const quest = QuestManager::getQuest(questCrc);
	if (!quest)
		return QR_noSuchQuest; // Quest has been removed from the data table, apparently
	QuestTask const * const questTask = quest->getTask(taskId);
	if (!questTask)
		return QR_noSuchTask;

	PlayerQuestDataMap::const_iterator questData=m_quests.find(questCrc);
	if (questData==m_quests.end())
		return QR_questNotActive;
	if (!questData->second.isTaskActive(taskId))
		return QR_taskNotActive;

	PlayerQuestData newQuestData(questData->second);
	newQuestData.completeTask(taskId);
	m_quests.set(questCrc, newQuestData);

	ScriptParams params;
	params.addParam(static_cast<int>(questCrc));
	params.addParam(static_cast<int>(taskId));

	CreatureObject * owner = getCreatureObject();
	if(owner)
	{
		GameScriptObject * script = owner->getScriptObject();
		if(script)
		{
			const std::string & attachScript = questTask->getAttachScript();
			if(!attachScript.empty())
			{
				IGNORE_RETURN(script->trigOneScript(attachScript, Scripting::TRIG_TASK_COMPLETED, params));
				derefScript(*owner, attachScript);
			}

			IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_SOME_TASK_COMPLETED, params));
		}
	}

	if (questTask->questCompletesOnTaskCompletion())
	{
		questCompleteQuest(questCrc);
		return QR_success;
	}
	else if (questTask->questClearsOnTaskCompletion())
	{
		questClearQuest(questCrc);
		return QR_success;
	}

	TaskChanceVector tasksWithChanceToActivate;
	int totalChance = 0;

	QuestTask::TaskIdList const & nextTasks = questTask->getTasksOnComplete();
	for (QuestTask::TaskIdList::const_iterator i=nextTasks.begin(); i != nextTasks.end(); ++i)
	{
		QuestTask const * const taskOnComplete = quest->getTask(*i);
		if (taskOnComplete)
		{
			int const chanceToActivate = taskOnComplete->getChanceToActivate();
			if (chanceToActivate <= 0)
				IGNORE_RETURN(questActivateQuestTask(questCrc, *i)); // activate all that are possible, ignore the ones that aren't
			else
			{
				tasksWithChanceToActivate.push_back(std::make_pair(*i, chanceToActivate));
				totalChance += chanceToActivate;
			}
		}
	}

	if (totalChance)
	{
		int const winningChance = Random::random(totalChance - 1);
		int previousChance = 0;

		TaskChanceVector::const_iterator currentTaskChance = tasksWithChanceToActivate.begin();

		for (;currentTaskChance != tasksWithChanceToActivate.end(); ++currentTaskChance)
		{
			int const currentChance = previousChance + currentTaskChance->second;

			if (winningChance >= previousChance && winningChance < currentChance)
			{
				IGNORE_RETURN(questActivateQuestTask(questCrc, currentTaskChance->first));
				break;
			}

			previousChance = currentChance;
		}
	}

	//closeout quest if they're done with all the tasks, if the quest is marked to allow this
	//The "space quests" written using breinhart's quest backend will NOT use this, the "ground quests" will
	bool questIsComplete = true;
	for(int it = 0; it < quest->getNumberOfTasks(); ++it)
	{
		bool const completedTask = questHasCompletedQuestTask(questCrc, it);
		if(!completedTask)
		{
			questIsComplete = false;
			break;
		}
	}
	if(questIsComplete && quest->getCompleteQuestWhenTasksComplete())
	{
		questCompleteQuest(questCrc);
	}
	return QR_success;
}

// ----------------------------------------------------------------------

PlayerObject::QuestResult PlayerObject::questFailQuestTask(uint32 const questCrc, int const taskId)
{
	Quest const * const quest = QuestManager::getQuest(questCrc);
	if (!quest)
		return QR_noSuchQuest; // Quest has been removed from the data table, apparently
	QuestTask const * const questTask = quest->getTask(taskId);
	if (!questTask)
		return QR_noSuchTask;

	PlayerQuestDataMap::const_iterator questData=m_quests.find(questCrc);
	if (questData==m_quests.end())
		return QR_questNotActive;
	if (!questData->second.isTaskActive(taskId))
		return QR_taskNotActive;

	PlayerQuestData newQuestData(questData->second);
	newQuestData.clearTask(taskId);
	m_quests.set(questCrc, newQuestData);

	ScriptParams params;
	params.addParam(static_cast<int>(questCrc));
	params.addParam(static_cast<int>(taskId));

	CreatureObject * owner = getCreatureObject();
	if(owner)
	{
		GameScriptObject * script = owner->getScriptObject();
		if(script)
		{
			const std::string & attachScript = questTask->getAttachScript();
			if(!attachScript.empty())
			{
				IGNORE_RETURN(script->trigOneScript(attachScript, Scripting::TRIG_TASK_FAILED, params));
				derefScript(*owner, attachScript);
			}

			IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_SOME_TASK_FAILED, params));
		}
	}

	if (questTask->questCompletesOnTaskFailure())
	{
		questCompleteQuest(questCrc);
		return QR_success;
	}
	else if (questTask->questClearsOnTaskFailure())
	{
		questClearQuest(questCrc);
		return QR_success;
	}

	QuestTask::TaskIdList const & nextTasks = questTask->getTasksOnFail();
	for (QuestTask::TaskIdList::const_iterator i=nextTasks.begin(); i != nextTasks.end(); ++i)
	{
		IGNORE_RETURN(questActivateQuestTask(questCrc, *i)); // activate all that are possible, ignore the ones that aren't
	}

	return QR_success;
}

// ----------------------------------------------------------------------

PlayerObject::QuestResult PlayerObject::questClearQuestTask(uint32 const questCrc, int const taskId)
{
	Quest const * const quest = QuestManager::getQuest(questCrc);
	if (!quest)
		return QR_noSuchQuest; // Quest has been removed from the data table, apparently
	QuestTask const * const questTask = quest->getTask(taskId);
	if (!questTask)
		return QR_noSuchTask;

	PlayerQuestDataMap::const_iterator questData=m_quests.find(questCrc);
	if (questData==m_quests.end())
		return QR_questNotActive;
	if (!questData->second.isTaskActive(taskId))
		return QR_taskNotActive;

	PlayerQuestData newQuestData(questData->second);
	newQuestData.clearTask(taskId);
	m_quests.set(questCrc, newQuestData);

	ScriptParams params;
	params.addParam(static_cast<int>(questCrc));
	params.addParam(static_cast<int>(taskId));

	CreatureObject * owner = getCreatureObject();
	if(owner)
	{
		GameScriptObject * script = owner->getScriptObject();
		if(script)
		{
			const std::string & attachScript = questTask->getAttachScript();
			if(!attachScript.empty())
			{
				IGNORE_RETURN(script->trigOneScript(attachScript, Scripting::TRIG_TASK_CLEARED, params));
				derefScript(*owner, attachScript);
			}

			IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_SOME_TASK_CLEARED, params));
		}
	}

	return QR_success;
}

// ----------------------------------------------------------------------

bool PlayerObject::questHasActiveQuest(uint32 const questCrc) const
{
	PlayerQuestDataMap::const_iterator questData=m_quests.find(questCrc);
	return (questData!=m_quests.end() && !questData->second.isCompleted());
}

// ----------------------------------------------------------------------

bool PlayerObject::questHasCompletedQuest(uint32 const questCrc) const
{
	PlayerQuestDataMap::const_iterator questData=m_quests.find(questCrc);
	return (questData!=m_quests.end() && questData->second.isCompleted());
}

// ----------------------------------------------------------------------

bool PlayerObject::questHasActiveQuestTask(uint32 const questCrc, int const taskId) const
{
	PlayerQuestDataMap::const_iterator questData=m_quests.find(questCrc);
	if (questData==m_quests.end())
		return false;

	return (questData->second.isTaskActive(taskId));

}

// ----------------------------------------------------------------------

bool PlayerObject::questHasCompletedQuestTask(uint32 const questCrc, int const taskId) const
{
	PlayerQuestDataMap::const_iterator questData=m_quests.find(questCrc);
	if (questData==m_quests.end())
		return false;

	return (questData->second.isTaskCompleted(taskId));
}

// ----------------------------------------------------------------------

uint32 PlayerObject::questGetQuestStatus(uint32 const questCrc) const
{
	PlayerQuestDataMap::const_iterator questData = m_quests.find(questCrc);

	if (questData == m_quests.end())
		return 0;

	return (questData->second.getActiveTasks() << 16) | (questData->second.getCompletedTasks() & 0xFFFF);
}

// ----------------------------------------------------------------------

std::set<PlayerObject const *> const &PlayerObject::getAllPlayerObjects() // static
{
	return s_allPlayerObjects;
}

// ----------------------------------------------------------------------

PlayerObject::WaypointMap const &PlayerObject::getWaypoints() const
{
	return m_waypoints.getMap();
}

// ----------------------------------------------------------------------

void PlayerObject::getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const
{
	if(name == "quests")
	{
		Archive::AutoDeltaMap<unsigned long, PlayerQuestData>(m_quests).pack(target);
	}
	else if(name == "completedQuests")
	{
		m_completedQuests.pack(target);
	}
	else if(name == "activeQuests")
	{
		m_activeQuests.pack(target);
	}
	else if(name == "currentQuest")
	{
		m_currentQuest.pack(target);
	}
	else if(name == "bornDate")
	{
		m_bornDate.pack(target);
	}
	else if(name == "playedTime")
	{
		m_playedTime.pack(target);
	}
	else if(name == "food")
	{
		m_food.pack(target);
	}
	else if(name == "maxFood")
	{
		m_maxFood.pack(target);
	}
	else if(name == "drink")
	{
		m_drink.pack(target);
	}
	else if(name == "maxDrink")
	{
		m_maxDrink.pack(target);
	}
	else if(name == "meds")
	{
		m_meds.pack(target);
	}
	else if(name == "maxMeds")
	{
		m_maxMeds.pack(target);
	}
	else if(name == "privledgedTitle")
	{
		m_privledgedTitle.pack(target);
	}
	else if(name == "matchMakingPersonalProfileId")
	{
		m_matchMakingPersonalProfileId.pack(target);
	}
	else if(name == "matchMakingCharacterProfileId")
	{
		m_matchMakingCharacterProfileId.pack(target);
	}
	else if(name == "workingSkill")
	{
		m_workingSkill.pack(target);
	}
	else if(name == "skillTemplate")
	{
		m_skillTemplate.pack(target);
	}
	else if(name == "collections")
	{
		m_collections.pack(target);
	}
	else if(name == "collections2")
	{
		m_collections2.pack(target);
	}
	else
	{
		IntangibleObject::getByteStreamFromAutoVariable(name, target);
	}
}

// ----------------------------------------------------------------------

void PlayerObject::setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source)
{
	Archive::ReadIterator ri(source);
	if(name == "quests")
	{
		typedef Archive::AutoDeltaMap<unsigned long, PlayerQuestData>::Command Commands;
		std::vector<Commands> quests;

		m_quests.clear();
		Archive::AutoDeltaMap<unsigned long, PlayerQuestData>(m_quests).unpack(ri, quests);

		for (std::vector<Commands>::const_iterator questIter = quests.begin(); questIter != quests.end(); ++questIter)
		{
			m_quests.set(questIter->key, questIter->value);
		}
	}
	else if(name == "completedQuests")
	{
		m_completedQuests.unpackDelta(ri);
	}
	else if(name == "activeQuests")
	{
		m_activeQuests.unpackDelta(ri);
	}
	else if(name == "currentQuest")
	{
		m_currentQuest.unpackDelta(ri);
	}
	else if(name == "bornDate")
	{
		m_bornDate.unpackDelta(ri);
	}
	else if(name == "playedTime")
	{
		m_playedTime.unpackDelta(ri);
	}
	else if(name == "food")
	{
		m_food.unpackDelta(ri);
	}
	else if(name == "maxFood")
	{
		m_maxFood.unpackDelta(ri);
	}
	else if(name == "drink")
	{
		m_drink.unpackDelta(ri);
	}
	else if(name == "maxDrink")
	{
		m_maxDrink.unpackDelta(ri);
	}
	else if(name == "meds")
	{
		m_meds.unpackDelta(ri);
	}
	else if(name == "maxMeds")
	{
		m_maxMeds.unpackDelta(ri);
	}
	else if(name == "privledgedTitle")
	{
		m_privledgedTitle.unpackDelta(ri);
	}
	else if(name == "matchMakingPersonalProfileId")
	{
		m_matchMakingPersonalProfileId.unpackDelta(ri);
	}
	else if(name == "matchMakingCharacterProfileId")
	{
		m_matchMakingCharacterProfileId.unpackDelta(ri);
	}
	else if(name == "workingSkill")
	{
		m_workingSkill.unpackDelta(ri);
	}
	else if(name == "skillTemplate")
	{
		m_skillTemplate.unpackDelta(ri);
	}
	else if(name == "collections")
	{
		m_collections.unpackDelta(ri);
	}
	else if(name == "collections2")
	{
		m_collections2.unpackDelta(ri);
	}
	else
	{
		IntangibleObject::setAutoVariableFromByteStream(name, source);
	}
	addObjectToConcludeList();
}

// ----------------------------------------------------------------------

void PlayerObject::setGroupWaypoints(WaypointMap const &groupWaypoints)
{
	DEBUG_FATAL(!isAuthoritative(), ("PlayerObject::setGroupWaypoints: called on non-authoritative object %s", getDebugInformation().c_str()));
	if (isAuthoritative())
	{
		//-- Anything in the new set, we need to set (the AutoDeltaMap will throw out anything unchanged)
		{
			for (WaypointMap::const_iterator i = groupWaypoints.begin(); i != groupWaypoints.end(); ++i)
				m_groupWaypoints.set((*i).first, (*i).second);
		}

		//-- Anything that is not in the original set, we need to remove
		{
			for (WaypointMap::const_iterator i = m_groupWaypoints.begin(); i != m_groupWaypoints.end(); ++i)
				if (groupWaypoints.find((*i).first) == groupWaypoints.end())
					m_groupWaypoints.erase((*i).first);
		}
	}
}

// ----------------------------------------------------------------------

uint32 PlayerObject::getPlayedTime() const
{
	return static_cast<uint32>(m_playedTime.get() + m_playedTimeAccum);
}

// ----------------------------------------------------------------------

float PlayerObject::getPlayedTimeAccumOnly() const
{
	return m_playedTimeAccum;
}

// ----------------------------------------------------------------------

void PlayerObject::setPlayedTimeAccumOnly(float playedTimeAccum)
{
	m_playedTimeAccum = playedTimeAccum;
}

// ----------------------------------------------------------------------

unsigned long PlayerObject::getSessionPlayTimeDuration() const
{
	time_t const sessionStartPlayTime = static_cast<time_t>(m_sessionStartPlayTime.get());
	if (sessionStartPlayTime > 0)
	{
		time_t const now = ::time(nullptr);
		if (now > sessionStartPlayTime)
			return static_cast<unsigned long>(now - sessionStartPlayTime);
	}

	return 0;
}

// ----------------------------------------------------------------------

unsigned long PlayerObject::getSessionActivePlayTimeDuration() const
{
	unsigned long activePlayTimeDuration = m_sessionActivePlayTimeDuration.get();

	time_t const sessionLastActiveTime = static_cast<time_t>(m_sessionLastActiveTime.get());
	if (sessionLastActiveTime > 0)
	{
		time_t const now = ::time(nullptr);
		if (now > sessionLastActiveTime)
			activePlayTimeDuration += static_cast<unsigned long>(now - sessionLastActiveTime);
	}

	return activePlayTimeDuration;
}

// ----------------------------------------------------------------------

void PlayerObject::setSessionPlayTimeInfo(int32 sessionStartPlayTime, int32 sessionLastActiveTime, unsigned long sessionActivePlayTimeDuration)
{
	// shouldn't be calling this on a non-auth object
	if (!isAuthoritative())
		return;

	int32 const previousSessionStartPlayTime = m_sessionStartPlayTime.get();
	int32 const previousSessionLastActiveTime = m_sessionLastActiveTime.get();

	m_sessionStartPlayTime = sessionStartPlayTime;
	m_sessionLastActiveTime = sessionLastActiveTime;
	m_sessionActivePlayTimeDuration = sessionActivePlayTimeDuration;

	// just connected
	if ((previousSessionStartPlayTime == 0) && (sessionStartPlayTime != 0))
	{
		CreatureObject const * const owner = getCreatureObject();
		if (owner)
		{
			LfgCharacterData lfgCharacterData;
			owner->getLfgCharacterData(lfgCharacterData);

			ServerUniverse::setConnectedCharacterLfgData(owner->getNetworkId(), lfgCharacterData);

			BiographyManager::requestBiography(owner->getNetworkId(), nullptr);

			// check/set "account age" title
			checkAndSetAccountAgeTitle(*this);
		}
	}
	// just disconnected
	else if ((sessionStartPlayTime == 0) && (previousSessionStartPlayTime != 0))
	{
		CreatureObject const * const owner = getCreatureObject();
		if (owner)
		{
			ServerUniverse::removeConnectedCharacterData(owner->getNetworkId());
		}
	}
	else if (sessionStartPlayTime != 0)
	{
		// just went active/inactive
		if (((previousSessionLastActiveTime == 0) && (sessionLastActiveTime != 0)) ||
		    ((sessionLastActiveTime == 0) && (previousSessionLastActiveTime != 0)))
		{
			CreatureObject const * const owner = getCreatureObject();
			if (owner)
			{
				LfgCharacterData lfgCharacterData;
				owner->getLfgCharacterData(lfgCharacterData);

				std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
				std::map<NetworkId, LfgCharacterData>::const_iterator iterFindLfg = connectedCharacterLfgData.find(owner->getNetworkId());
				if ((iterFindLfg == connectedCharacterLfgData.end()) || (iterFindLfg->second != lfgCharacterData))
					ServerUniverse::setConnectedCharacterLfgData(owner->getNetworkId(), lfgCharacterData);
			}
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObject::setRoleIconChoice(int roleIconChoice)
{
	m_roleIconChoice = roleIconChoice;
}

// ----------------------------------------------------------------------

int PlayerObject::getRoleIconChoice() const
{
	return m_roleIconChoice.get();
}

// ----------------------------------------------------------------------

void PlayerObject::setAggroImmuneDuration(int const time)
{
	m_aggroImmuneDuration = static_cast<time_t>(time);
	m_aggroImmuneStartTime = Os::getRealSystemTime();
}

// ----------------------------------------------------------------------

bool PlayerObject::isAggroImmune() const
{
	time_t const elapsedTime = Os::getRealSystemTime() - m_aggroImmuneStartTime.get();

	return (elapsedTime < m_aggroImmuneDuration.get());
}

// ----------------------------------------------------------------------

int PlayerObject::getAgeInDays() const
{
	return int(getCurrentBornDate() - getBornDate());
}

// ----------------------------------------------------------------------

std::string const & PlayerObject::getSkillTemplate() const
{
	return m_skillTemplate.get();
}

// ----------------------------------------------------------------------

bool PlayerObject::setSkillTemplate(std::string const & templateName, bool const clientRequest)
{
	std::string const previousTemplateName(m_skillTemplate.get());

	// we need to pass an empty string to script to do cleanup, but we don't want to actually change the player's skill
	if (!templateName.empty())
		m_skillTemplate.set(templateName);

	CreatureObject * const owner = getCreatureObject();
	if(owner)
	{
		GameScriptObject * const script = owner->getScriptObject();
		if(script)
		{
			ScriptParams params;

			params.addParam(templateName.c_str());
			params.addParam(clientRequest);

			if (script->trigAllScripts(Scripting::TRIG_SKILL_TEMPLATE_CHANGED, params) == SCRIPT_DEFAULT)
			{
				m_skillTemplate.set(previousTemplateName);
				LOG("ScriptInvestigation", ("Scripts blocked setSkillTemplate( %s ) on Player( %s ).  Reverting to %s.", templateName.c_str(), getAccountDescription().c_str(), previousTemplateName.c_str()));
			}
		}

		if (previousTemplateName != m_skillTemplate.get())
		{
			uint8 const newProfession = static_cast<uint8>(LfgCharacterData::convertSkillTemplateToProfession(m_skillTemplate.get()));

			GroupObject * const group = owner->getGroup();
			if (group)
			{
				uint8 const currentProfessionForGroup = group->getMemberProfession(owner->getNetworkId());

				if (newProfession != currentProfessionForGroup)
					group->setMemberProfession(owner->getNetworkId(), newProfession);
			}

			std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
			std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = connectedCharacterLfgData.find(owner->getNetworkId());
			if ((iterFind != connectedCharacterLfgData.end()) && (iterFind->second.profession != static_cast<LfgCharacterData::Profession>(newProfession)))
				ServerUniverse::setConnectedCharacterProfessionData(owner->getNetworkId(), static_cast<LfgCharacterData::Profession>(newProfession));
		}
	}

	return (templateName != previousTemplateName);
}

// ----------------------------------------------------------------------

std::string const & PlayerObject::getWorkingSkill() const
{
	return m_workingSkill.get();
}

// ----------------------------------------------------------------------

bool PlayerObject::setWorkingSkill(std::string const & skillName, bool const clientRequest)
{
	std::string const previousWorkingSkill(m_workingSkill.get());

	m_workingSkill.set(skillName);

	CreatureObject * const owner = getCreatureObject();
	if(owner)
	{
		GameScriptObject * const script = owner->getScriptObject();
		if(script)
		{
			ScriptParams params;

			params.addParam(skillName.c_str());
			params.addParam(clientRequest);

			if (script->trigAllScripts(Scripting::TRIG_WORKING_SKILL_CHANGED, params) == SCRIPT_DEFAULT)
			{
				m_workingSkill.set(previousWorkingSkill);
				LOG("ScriptInvestigation", ("Scripts blocked setWorkingSkill( %s ) on Player( %s ). Reverting to %s.", skillName.c_str(), getAccountDescription().c_str(), previousWorkingSkill.c_str()));
			}
		}
	}


	return (previousWorkingSkill != skillName);
}

// ----------------------------------------------------------------------

void PlayerObject::setPendingRequestQuestInformation(uint32 questCrc, NetworkId const & questGiver)
{
	m_pendingRequestQuestCrc = questCrc;
	m_pendingRequestQuestGiver = questGiver;
}

// ----------------------------------------------------------------------

uint32 PlayerObject::getPendingRequestQuestCrc() const
{
	return m_pendingRequestQuestCrc;
}

// ----------------------------------------------------------------------

NetworkId const & PlayerObject::getPendingRequestQuestGiver() const
{
	return m_pendingRequestQuestGiver;
}

// ----------------------------------------------------------------------

int PlayerObject::getNumberOfActiveQuests() const
{
	int counter = 0;
	for (PlayerQuestDataMap::const_iterator i = m_quests.begin(); i != m_quests.end(); ++i)
	{
		PlayerQuestData const questData(i->second); // unfortunately have to copy the quest data, because AutoDeltaPackedMap does not have non-const iterators
		if(!questData.isCompleted())
		{
			++counter;
		}
	}
	return counter;
}

// ----------------------------------------------------------------------

std::vector<int> PlayerObject::questGetAllActiveQuestIds() const
{
	std::vector<int> result;
	for (PlayerQuestDataMap::const_iterator i=m_quests.begin(); i!=m_quests.end(); ++i)
	{
		if(!i->second.isCompleted())
		{
			result.push_back(i->first);
		}
	}
	return result;
}

// ----------------------------------------------------------------------

std::vector<int> PlayerObject::questGetAllCompletedQuestIds() const
{
	std::vector<int> result;
	for (PlayerQuestDataMap::const_iterator i=m_quests.begin(); i!=m_quests.end(); ++i)
	{
		if(i->second.isCompleted())
		{
			result.push_back(i->first);
		}
	}
	return result;
}

// ----------------------------------------------------------------------

void PlayerObject::setPlayerQuestDataComplete(uint32 questCrc, PlayerQuestData & playerQuestData)
{
	playerQuestData.complete();
	m_quests.set(questCrc, playerQuestData);

	//-- if the completed quest is the current quest, pick a new one
	if (questCrc == getCurrentQuest())
	{
		pickNewCurrentQuest();
	}
}

// ----------------------------------------------------------------------

void PlayerObject::pickNewCurrentQuest()
{
	uint32 lastRelativeAgeIndex = 0;
	uint32 lastQuestCrc = 0;

	for (PlayerQuestDataMap::const_iterator i = m_quests.begin(); i != m_quests.end(); ++i)
	{
		PlayerQuestData const & questData = i->second;
		uint32 const questCrc = i->first;

		if (!questData.isCompleted() && questData.getRelativeAgeIndex() >= lastRelativeAgeIndex)
		{
			lastQuestCrc = questCrc;
			lastRelativeAgeIndex = questData.getRelativeAgeIndex();
		}
	}

	setCurrentQuest(lastQuestCrc);
}

// ----------------------------------------------------------------------

void PlayerObject::modifyCurrentGcwPoints(int const count, bool modifyLifetimeValue)
{
	CreatureObject * const owner = getCreatureObject();
	if (!owner)
		return;

	if (isAuthoritative())
	{
		if (PvpData::isImperialFactionId(owner->getPvpFaction()) || PvpData::isRebelFactionId(owner->getPvpFaction()))
		{
			static int32 const max = std::numeric_limits<int32>::max();

			// m_currentGcwPoints should always be >= 0
			int32 const currentValue = std::max(static_cast<int32>(0), m_currentGcwPoints.get());

			// limit count so it doesn't cause overflow
			int32 const adjustedCount = std::min(static_cast<int32>(count), max - currentValue);

			int32 temp = 0;
			if (adjustedCount != 0)
			{
				temp = m_currentGcwPoints.get();
				m_currentGcwPoints = std::max(static_cast<int32>(0), currentValue + adjustedCount);
			}

			if (modifyLifetimeValue && (count > 0))
				modifyLifetimeGcwPoints(count);

			if (adjustedCount != 0)
			{
				char buffer[256];
				sprintf(buffer, INT64_FORMAT_SPECIFIER, m_lifetimeGcwPoints.get());

				LOG("CustomerService", ("PvP_Ranking:%s|adjust point=%d|current=%ld|new=%ld|lifetime=%s",
					getAccountDescription().c_str(),
					count,
					temp,
					m_currentGcwPoints.get(),
					buffer));
			}
		}
		else
		{
			// non-factional characters is always at 0
			if (m_currentGcwPoints.get() != 0)
			{
				LOG("CustomerService", ("PvP_Ranking:%s|adjust point=%d|setting current points (%ld) to 0 because not factional",
					getAccountDescription().c_str(),
					count,
					m_currentGcwPoints.get()));

				m_currentGcwPoints = 0;
			}
		}

		// make sure that m_nextGcwRatingCalcTime is set so that the
		// rating will be recalculated at the next recalculation interval
		setNextGcwRatingCalcTime(false);
	}
	else
	{
		owner->sendControllerMessageToAuthServer(CM_modifyCurrentGcwPoints, new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(count, (modifyLifetimeValue ? 1 : 0))));
	}
}

// ----------------------------------------------------------------------

void PlayerObject::modifyCurrentGcwRating(int const count, bool modifyMaxValue)
{
	CreatureObject * const owner = getCreatureObject();
	if (!owner)
		return;

	if (isAuthoritative())
	{
		if (PvpData::isImperialFactionId(owner->getPvpFaction()) || PvpData::isRebelFactionId(owner->getPvpFaction()))
		{
			static int32 const max = std::numeric_limits<int32>::max();

			// m_currentGcwRating should always be >= 0
			int32 const currentValue = std::max(static_cast<int32>(Pvp::getMinRatingForRank()), m_currentGcwRating.get());

			// limit count so it doesn't cause overflow
			int32 const adjustedCount = std::min(static_cast<int32>(count), max - currentValue);

			if (adjustedCount != 0)
			{
				int32 const temp = m_currentGcwRating.get();

				m_currentGcwRating = std::min(std::max(static_cast<int32>(Pvp::getMinRatingForRank()), currentValue + adjustedCount), static_cast<int32>(Pvp::getMaxRatingForRank()));

				// calculate new rank
				Pvp::PvpRankInfo const & pvpRankInfo = Pvp::getRankInfo(m_currentGcwRating.get());
				int const oldRank = m_currentGcwRank.get();
				m_currentGcwRank = pvpRankInfo.rank;

				if (pvpRankInfo.rank > 0)
				{
					m_currentGcwRankProgress = static_cast<float>(std::min(static_cast<int>(m_currentGcwRating.get()), pvpRankInfo.maxRating) - pvpRankInfo.minRating) * 100.0f / static_cast<float>(pvpRankInfo.maxRating - pvpRankInfo.minRating + 1);
				}
				else
				{
					m_currentGcwRankProgress = 0.0f;
				}

				if (modifyMaxValue)
				{
					if (PvpData::isImperialFactionId(owner->getPvpFaction()) && (m_currentGcwRating.get() > m_maxGcwImperialRating.get()))
					{
						m_maxGcwImperialRating = m_currentGcwRating;
						m_maxGcwImperialRank = Pvp::getRankInfo(m_maxGcwImperialRating.get()).rank;
					}
					else if (PvpData::isRebelFactionId(owner->getPvpFaction()) && (m_currentGcwRating.get() > m_maxGcwRebelRating.get()))
					{
						m_maxGcwRebelRating = m_currentGcwRating;
						m_maxGcwRebelRank = Pvp::getRankInfo(m_maxGcwRebelRating.get()).rank;
					}
				}

				LOG("CustomerService", ("PvP_Ranking:%s|adjust rating=%d|current=%ld|new=%ld|max imp=%ld|max reb=%ld",
					getAccountDescription().c_str(),
					count,
					temp,
					m_currentGcwRating.get(),
					m_maxGcwImperialRating.get(),
					m_maxGcwRebelRating.get()));

				// notify script if rank changed
				if (oldRank != m_currentGcwRank.get())
				{
					GameScriptObject * const script = owner->getScriptObject();

					if (script != 0)
					{
						ScriptParams params;
						params.addParam(oldRank);
						params.addParam(m_currentGcwRank.get());
						script->trigAllScripts(Scripting::TRIG_PVP_RANKING_CHANGED, params);
					}
				}
			}
		}
		else
		{
			// non-factional characters is always at -1
			if (m_currentGcwRating.get() != -1)
			{
				LOG("CustomerService", ("PvP_Ranking:%s|adjust rating=%d|setting current rating (%ld) to -1 because not factional",
					getAccountDescription().c_str(),
					count,
					m_currentGcwRating.get()));

				m_currentGcwRating = -1;
			}

			int const oldRank = m_currentGcwRank.get();
			m_currentGcwRank = 0;
			m_currentGcwRankProgress = 0.0f;

			// notify script if rank changed
			if (oldRank != m_currentGcwRank.get())
			{
				GameScriptObject * const script = owner->getScriptObject();

				if (script != 0)
				{
					ScriptParams params;
					params.addParam(oldRank);
					params.addParam(m_currentGcwRank.get());
					script->trigAllScripts(Scripting::TRIG_PVP_RANKING_CHANGED, params);
				}
			}
		}

		// make sure that m_nextGcwRatingCalcTime is set so that the
		// rating will be recalculated at the next recalculation interval
		setNextGcwRatingCalcTime(false);
	}
	else
	{
		owner->sendControllerMessageToAuthServer(CM_modifyCurrentGcwRating, new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(count, (modifyMaxValue ? 1 : 0))));
	}
}

// ----------------------------------------------------------------------

void PlayerObject::modifyCurrentPvpKills(int const count, bool modifyLifetimeValue)
{
	CreatureObject * const owner = getCreatureObject();
	if (!owner)
		return;

	if (isAuthoritative())
	{
		if (PvpData::isImperialFactionId(owner->getPvpFaction()) || PvpData::isRebelFactionId(owner->getPvpFaction()))
		{
			static int32 const max = std::numeric_limits<int32>::max();

			// m_currentPvpKills should always be >= 0
			int32 const currentValue = std::max(static_cast<int32>(0), m_currentPvpKills.get());

			// limit count so it doesn't cause overflow
			int32 const adjustedCount = std::min(static_cast<int32>(count), max - currentValue);

			int32 temp = 0;
			if (adjustedCount != 0)
			{
				temp = m_currentPvpKills.get();
				m_currentPvpKills = std::max(static_cast<int32>(0), currentValue + adjustedCount);
			}

			if (modifyLifetimeValue && (count > 0))
				modifyLifetimePvpKills(count);

			if (adjustedCount != 0)
			{
				LOG("CustomerService", ("PvP_Ranking:%s|adjust pvp kill=%d|current=%ld|new=%ld|lifetime=%ld",
					getAccountDescription().c_str(),
					count,
					temp,
					m_currentPvpKills.get(),
					m_lifetimePvpKills.get()));
			}
		}
		else
		{
			// non-factional characters is always at 0
			if (m_currentPvpKills.get() != 0)
			{
				LOG("CustomerService", ("PvP_Ranking:%s|adjust pvp kill=%d|setting current pvp kill (%ld) to 0 because not factional",
					getAccountDescription().c_str(),
					count,
					m_currentPvpKills.get()));

				m_currentPvpKills = 0;
			}
		}

		// make sure that m_nextGcwRatingCalcTime is set so that the
		// rating will be recalculated at the next recalculation interval
		setNextGcwRatingCalcTime(false);
	}
	else
	{
		owner->sendControllerMessageToAuthServer(CM_modifyCurrentPvpKills, new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(count, (modifyLifetimeValue ? 1 : 0))));
	}
}

// ----------------------------------------------------------------------

void PlayerObject::modifyLifetimeGcwPoints(int const count)
{
	if (isAuthoritative())
	{
		static int64 const max = std::numeric_limits<int64>::max();

		// m_lifetimeGcwPoints should always be >= 0
		int64 const currentValue = std::max(static_cast<int64>(0), m_lifetimeGcwPoints.get());

		// limit count so it doesn't cause overflow
		int64 const adjustedCount = std::min(static_cast<int64>(count), max - currentValue);

		if (adjustedCount != 0)
			m_lifetimeGcwPoints = std::max(static_cast<int64>(0), currentValue + adjustedCount);
	}
	else
	{
		CreatureObject * const owner = getCreatureObject();
		if (owner)
			owner->sendControllerMessageToAuthServer(CM_modifyLifetimeGcwPoints, new MessageQueueGenericValueType<int>(count));
	}
}

// ----------------------------------------------------------------------

void PlayerObject::modifyMaxGcwImperialRating(int const count)
{
	if (isAuthoritative())
	{
		static int32 const min = std::numeric_limits<int32>::min() + 1;
		static int32 const max = std::numeric_limits<int32>::max() - 1;

		// m_maxGcwImperialRating should always be >= -1
		int32 currentValue = std::max(static_cast<int32>(-1), m_maxGcwImperialRating.get());
		currentValue = std::min(currentValue, max);

		// limit count so it doesn't cause underflow or overflow
		int32 adjustedCount = std::max(static_cast<int32>(count), min);
		adjustedCount = std::min(adjustedCount, max - currentValue);

		if (adjustedCount != 0)
		{
			m_maxGcwImperialRating = std::min(std::max(static_cast<int32>(-1), currentValue + adjustedCount), static_cast<int32>(Pvp::getMaxRatingForRank()));
			m_maxGcwImperialRank = Pvp::getRankInfo(m_maxGcwImperialRating.get()).rank;
		}
	}
	else
	{
		CreatureObject * const owner = getCreatureObject();
		if (owner)
			owner->sendControllerMessageToAuthServer(CM_modifyMaxGcwImperialRating, new MessageQueueGenericValueType<int>(count));
	}
}

// ----------------------------------------------------------------------

void PlayerObject::modifyMaxGcwRebelRating(int const count)
{
	if (isAuthoritative())
	{
		static int32 const min = std::numeric_limits<int32>::min() + 1;
		static int32 const max = std::numeric_limits<int32>::max() - 1;

		// m_maxGcwRebelRating should always be >= -1
		int32 currentValue = std::max(static_cast<int32>(-1), m_maxGcwRebelRating.get());
		currentValue = std::min(currentValue, max);

		// limit count so it doesn't cause underflow or overflow
		int32 adjustedCount = std::max(static_cast<int32>(count), min);
		adjustedCount = std::min(adjustedCount, max - currentValue);

		if (adjustedCount != 0)
		{
			m_maxGcwRebelRating = std::min(std::max(static_cast<int32>(-1), currentValue + adjustedCount), static_cast<int32>(Pvp::getMaxRatingForRank()));
			m_maxGcwRebelRank = Pvp::getRankInfo(m_maxGcwRebelRating.get()).rank;
		}
	}
	else
	{
		CreatureObject * const owner = getCreatureObject();
		if (owner)
			owner->sendControllerMessageToAuthServer(CM_modifyMaxGcwRebelRating, new MessageQueueGenericValueType<int>(count));
	}
}

// ----------------------------------------------------------------------

void PlayerObject::modifyLifetimePvpKills(int const count)
{
	if (isAuthoritative())
	{
		static int32 const max = std::numeric_limits<int32>::max();

		// m_lifetimePvpKills should always be >= 0
		int32 const currentValue = std::max(static_cast<int32>(0), m_lifetimePvpKills.get());

		// limit count so it doesn't cause overflow
		int32 const adjustedCount = std::min(static_cast<int32>(count), max - currentValue);

		if (adjustedCount != 0)
			m_lifetimePvpKills = std::max(static_cast<int32>(0), currentValue + adjustedCount);
	}
	else
	{
		CreatureObject * const owner = getCreatureObject();
		if (owner)
			owner->sendControllerMessageToAuthServer(CM_modifyLifetimePvpKills, new MessageQueueGenericValueType<int>(count));
	}
}

// ----------------------------------------------------------------------

void PlayerObject::modifyNextGcwRatingCalcTime(int const weekCount)
{
	if (isAuthoritative())
	{
		// don't need to do anything if the player doesn't need rating recalculated
		if (!needsGcwRatingRecalculated())
			return;

		static int32 const max = std::numeric_limits<int32>::max();
		int32 const now = static_cast<int32>(::time(nullptr));

		// m_nextGcwRatingCalcTime should always be >= 0
		int32 const currentValue = std::max(static_cast<int32>(0), m_nextGcwRatingCalcTime.get());

		// limit weekCount to -1000 to 1000 with 0 meaning "now"
		int32 adjustedWeekCount = std::max(static_cast<int32>(-1000), static_cast<int32>(weekCount));
		adjustedWeekCount = std::min(static_cast<int32>(1000), adjustedWeekCount);

		// if necessary, cancel the currently pending messageTo for rating recalculation
		if (m_nextGcwRatingCalcTime.get() > 0)
		{
			cancelMessageTo("C++RecalculateGcwRating");
			m_gcwRatingActualCalcTime = 0;
		}

		if (adjustedWeekCount != 0)
		{
			int32 adjustedSeconds = 60 * 60 * 24 * 7 * adjustedWeekCount;

			// limit adjustedSeconds so it doesn't cause overflow
			adjustedSeconds = std::min(adjustedSeconds, max - currentValue);

			// don't allow the recalc time to be too far in the past
			if (adjustedSeconds != 0)
				m_nextGcwRatingCalcTime = std::max(static_cast<int32>(CalendarTimeNamespace::g_CalendarTimeEarliestTime), currentValue + adjustedSeconds);
		}
		else // 0 means "now"
		{
			m_nextGcwRatingCalcTime = now;
		}

		// if necessary, send messageTo to trigger rating recalculation
		if (m_nextGcwRatingCalcTime.get() > 0)
		{
			// randomly spread out the recalculation so they don't all
			// happen at exactly the same time and lag out the server,
			// unless the recalculation time has passed in which case
			// we want to do the recalculation much more quickly
			if (m_nextGcwRatingCalcTime.get() <= now)
				sendRecalculateGcwRatingMessageTo(5);
			else
				sendRecalculateGcwRatingMessageTo(std::max(30, static_cast<int>(m_nextGcwRatingCalcTime.get() - now)) + 5 + (rand() % 60));
		}
	}
	else
	{
		CreatureObject * const owner = getCreatureObject();
		if (owner)
			owner->sendControllerMessageToAuthServer(CM_modifyNextGcwRatingCalcTime, new MessageQueueGenericValueType<int>(weekCount));
	}
}

// ----------------------------------------------------------------------
void PlayerObject::ctsUseOnlySetGcwInfo(int32 currentGcwPoints, int32 currentGcwRating, int32 currentPvpKills, int64 lifetimeGcwPoints, int32 maxGcwImperialRating, int32 maxGcwRebelRating, int32 lifetimePvpKills, int32 nextGcwRatingCalcTime)
{
	// should only be called on authoritative object
	if (!isAuthoritative())
		return;

	// can't do anything if we're not attached to the creature object
	if (!getCreatureObject())
		return;

	int32 const oldCurrentGcwPoints = m_currentGcwPoints.get();
	int32 const oldCurrentGcwRating = m_currentGcwRating.get();
	int32 const oldCurrentPvpKills = m_currentPvpKills.get();
	int64 const oldLifetimeGcwPoints = m_lifetimeGcwPoints.get();
	int32 const oldMaxGcwImperialRating = m_maxGcwImperialRating.get();
	int32 const oldMaxGcwRebelRating = m_maxGcwRebelRating.get();
	int32 const oldLifetimePvpKills = m_lifetimePvpKills.get();
	int32 const oldNextGcwRatingCalcTime = m_nextGcwRatingCalcTime.get();

	m_currentGcwPoints = std::max(static_cast<int32>(0), currentGcwPoints);
	m_currentGcwRating = std::min(std::max(static_cast<int32>(-1), currentGcwRating), static_cast<int32>(Pvp::getMaxRatingForRank()));
	m_currentPvpKills = std::max(static_cast<int32>(0), currentPvpKills);
	m_lifetimeGcwPoints = std::max(static_cast<int64>(0), lifetimeGcwPoints);
	m_maxGcwImperialRating = std::min(std::max(static_cast<int32>(-1), maxGcwImperialRating), static_cast<int32>(Pvp::getMaxRatingForRank()));
	m_maxGcwRebelRating = std::min(std::max(static_cast<int32>(-1), maxGcwRebelRating), static_cast<int32>(Pvp::getMaxRatingForRank()));
	m_lifetimePvpKills = std::max(static_cast<int32>(0), lifetimePvpKills);

	// don't allow the recalc time to be too far in the past
	m_nextGcwRatingCalcTime = ((nextGcwRatingCalcTime == 0) ? 0 : std::max(static_cast<int32>(CalendarTimeNamespace::g_CalendarTimeEarliestTime), nextGcwRatingCalcTime));

	Pvp::PvpRankInfo const & pvpRankInfo = Pvp::getRankInfo(m_currentGcwRating.get());
	m_currentGcwRank = pvpRankInfo.rank;

	if (pvpRankInfo.rank > 0)
	{
		m_currentGcwRankProgress = static_cast<float>(std::min(static_cast<int>(m_currentGcwRating.get()), pvpRankInfo.maxRating) - pvpRankInfo.minRating) * 100.0f / static_cast<float>(pvpRankInfo.maxRating - pvpRankInfo.minRating + 1);
	}
	else
	{
		m_currentGcwRankProgress = 0.0f;
	}

	m_maxGcwImperialRank = Pvp::getRankInfo(m_maxGcwImperialRating.get()).rank;
	m_maxGcwRebelRank = Pvp::getRankInfo(m_maxGcwRebelRating.get()).rank;

	char buffer1[128];
	char buffer2[128];
	sprintf(buffer1, INT64_FORMAT_SPECIFIER, oldLifetimeGcwPoints);
	sprintf(buffer2, INT64_FORMAT_SPECIFIER, m_lifetimeGcwPoints.get());

	LOG("CustomerService", ("PvP_Ranking:%s|applying info from CTS|point:%ld -> %ld|rating:%ld -> %ld|pvp kill:%ld -> %ld|lifetime point:%s -> %s|max imp rating:%ld -> %ld|max reb rating:%ld -> %ld|lifetime pvp kill:%ld -> %ld|calc time:%ld -> %ld",
		getAccountDescription().c_str(),
		oldCurrentGcwPoints, m_currentGcwPoints.get(),
		oldCurrentGcwRating, m_currentGcwRating.get(),
		oldCurrentPvpKills, m_currentPvpKills.get(),
		buffer1, buffer2,
		oldMaxGcwImperialRating, m_maxGcwImperialRating.get(),
		oldMaxGcwRebelRating, m_maxGcwRebelRating.get(),
		oldLifetimePvpKills, m_lifetimePvpKills.get(),
		oldNextGcwRatingCalcTime, m_nextGcwRatingCalcTime.get()));

	// if necessary, cancel the currently pending messageTo for rating recalculation
	if (oldNextGcwRatingCalcTime > 0)
	{
		cancelMessageTo("C++RecalculateGcwRating");
		m_gcwRatingActualCalcTime = 0;
	}

	// make sure that m_nextGcwRatingCalcTime is set so that the
	// rating will be recalculated at the next recalculation interval
	setNextGcwRatingCalcTime(true);
}

// ----------------------------------------------------------------------

void PlayerObject::setNextGcwRatingCalcTime(bool const alwaysSendMessageToForRecalc)
{
	// should only be called on authoritative object
	if (!isAuthoritative())
		return;

	bool needToSendMessageTo = alwaysSendMessageToForRecalc;
	int32 const now = static_cast<int32>(::time(nullptr));

	// if the player needs a rating recalculation,
	// make sure that m_nextGcwRatingCalcTime is set so that the
	// rating will be recalculated at the next recalculation interval
	if (needsGcwRatingRecalculated())
	{
		if (m_nextGcwRatingCalcTime.get() <= 0)
		{
			time_t const nextCalcTime = CalendarTime::getNextGMTTimeOcurrence(static_cast<time_t>(now), ConfigServerGame::getGcwRecalcTimeDayOfWeek(), ConfigServerGame::getGcwRecalcTimeHour(), ConfigServerGame::getGcwRecalcTimeMinute(), ConfigServerGame::getGcwRecalcTimeSecond());
			if (nextCalcTime > 0)
			{
				m_nextGcwRatingCalcTime = static_cast<int32>(nextCalcTime);
				needToSendMessageTo = true;
			}
		}
	}
	else
	{
		// if necessary, cancel the currently pending messageTo for rating recalculation
		if (m_nextGcwRatingCalcTime.get() > 0)
		{
			cancelMessageTo("C++RecalculateGcwRating");
			m_gcwRatingActualCalcTime = 0;
		}

		m_nextGcwRatingCalcTime = 0;
	}

	// send messageTo to trigger rating recalculation
	if (needToSendMessageTo && (m_nextGcwRatingCalcTime.get() > 0))
	{
		// randomly spread out the recalculation so they don't all
		// happen at exactly the same time and lag out the server,
		// unless the recalculation time has passed in which case
		// we want to do the recalculation much more quickly
		if (m_nextGcwRatingCalcTime.get() <= now)
			sendRecalculateGcwRatingMessageTo(5);
		else
			sendRecalculateGcwRatingMessageTo(std::max(30, static_cast<int>(m_nextGcwRatingCalcTime.get() - now)) + 5 + (rand() % 60));
	}
}

// ----------------------------------------------------------------------

void PlayerObject::handleRecalculateGcwRating()
{
	// should only be called on authoritative object
	if (!isAuthoritative())
		return;

	// can't do anything if we're not attached to the creature object
	if (!getCreatureObject())
		return;

	// is it time to recalculate?
	int32 const now = static_cast<int32>(::time(nullptr));
	if ((m_nextGcwRatingCalcTime.get() > 0) && (m_nextGcwRatingCalcTime.get() <= now))
	{
		// is recalculation required?
		if (needsGcwRatingRecalculated())
		{
			int32 nextCalcInterval = m_nextGcwRatingCalcTime.get();
			int32 points = std::max(static_cast<int32>(0), m_currentGcwPoints.get());
			int32 currentRating = m_currentGcwRating.get();
			int32 previousRating;
			int totalRatingAdjustment, totalEarnedRating, totalEarnedRatingAfterDecay, cappedRatingAdjustment;
			bool moreRecalculationNeeded = false;
			while (true)
			{
				// calculate rating adjustment for interval
				totalRatingAdjustment = Pvp::calculateRatingAdjustment(static_cast<int>(points), static_cast<int>(currentRating), totalEarnedRating, totalEarnedRatingAfterDecay, cappedRatingAdjustment);

				// don't apply rating loss if we are in a "rating loss exclusion interval"
				if ((totalRatingAdjustment < 0) && (Pvp::isInGcwRankDecayExclusionInterval(static_cast<time_t>(nextCalcInterval))))
				{
					// CS log
					LOG("CustomerService", ("PvP_Ranking:%s|NOT APPLYING RATING LOSS|interval %ld (%s) (%s)|current rating=%ld|points=%ld|total earned rating=%ld|total earned rating after decay=%ld|capped rating adjustment=%ld|final rating adjustment=%ld",
						getAccountDescription().c_str(),
						nextCalcInterval,
						CalendarTime::convertEpochToTimeStringGMT(static_cast<time_t>(nextCalcInterval)).c_str(),
						CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(nextCalcInterval)).c_str(),
						currentRating,
						points,
						totalEarnedRating,
						totalEarnedRatingAfterDecay,
						cappedRatingAdjustment,
						totalRatingAdjustment));
				}
				else
				{
					// update rating
					previousRating = currentRating;
					currentRating += static_cast<int32>(totalRatingAdjustment);

					// make sure updated rating is in range of valid rating
					currentRating = std::max(currentRating, static_cast<int32>(Pvp::getMinRatingForRank()));
					currentRating = std::min(currentRating, static_cast<int32>(Pvp::getMaxRatingForRank()));

					// CS log
					LOG("CustomerService", ("PvP_Ranking:%s|interval %ld (%s) (%s)|current rating=%ld|new rating=%ld|points=%ld|total earned rating=%ld|total earned rating after decay=%ld|capped rating adjustment=%ld|final rating adjustment=%ld",
						getAccountDescription().c_str(),
						nextCalcInterval,
						CalendarTime::convertEpochToTimeStringGMT(static_cast<time_t>(nextCalcInterval)).c_str(),
						CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(nextCalcInterval)).c_str(),
						previousRating,
						currentRating,
						points,
						totalEarnedRating,
						totalEarnedRatingAfterDecay,
						cappedRatingAdjustment,
						totalRatingAdjustment));
				}

				// point have all been converted
				points = 0;

				// check to see if we need to do another calculation for the next interval
				nextCalcInterval = static_cast<int32>(CalendarTime::getNextGMTTimeOcurrence(static_cast<time_t>(nextCalcInterval)+1, ConfigServerGame::getGcwRecalcTimeDayOfWeek(), ConfigServerGame::getGcwRecalcTimeHour(), ConfigServerGame::getGcwRecalcTimeMinute(), ConfigServerGame::getGcwRecalcTimeSecond()));
				if (nextCalcInterval <= 0)
					break;

				if (nextCalcInterval > now)
					break;

				Pvp::PvpRankInfo const & pvpNewRankInfo = Pvp::getRankInfo(currentRating);
				if (pvpNewRankInfo.rank < 1)
					break;

				if (pvpNewRankInfo.ratingDecayBalance <= 0)
					break;

				// if rating increased, then exit loop so that the new rating
				// can be updated so that the max rating can be saved, and rank
				// increase notification gets sent; otherwise, the rating
				// increase may get "lost" with subsequent decrease due to decay
				// as this loop iterates; if further calculation is required for more
				// intervals, then send another messageTo to trigger more calculations
				if (currentRating > m_currentGcwRating.get())
				{
					moreRecalculationNeeded = true;
					break;
				}
			}

			// decrementing the value by a really large
			// number will cause it to reset to 0
			static int const min = std::numeric_limits<int>::min();

			// apply new GCW values
			if (m_currentGcwPoints.get() != 0)
				modifyCurrentGcwPoints(min, false);

			if (m_currentPvpKills.get() != 0)
				modifyCurrentPvpKills(min, false);

			if (currentRating != m_currentGcwRating.get())
				modifyCurrentGcwRating((currentRating - m_currentGcwRating.get()), true);

			// if more recalculations are needed, send
			// messageTo to trigger more recalculations
			if (moreRecalculationNeeded)
			{
				m_nextGcwRatingCalcTime = nextCalcInterval;
				sendRecalculateGcwRatingMessageTo(5);
			}
			else
			{
				// make sure that m_nextGcwRatingCalcTime is set so that the
				// rating will be recalculated at the next recalculation interval
				m_nextGcwRatingCalcTime = 0;
				setNextGcwRatingCalcTime(false);
			}
		}
		else
		{
			m_nextGcwRatingCalcTime = 0;
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObject::sendRecalculateGcwRatingMessageTo(int delay)
{
	// cancel any pending messageTo
	cancelMessageTo("C++RecalculateGcwRating");

	// send new messageTo
	int const adjustedDelay = std::max(5, delay);
	MessageToQueue::getInstance().sendMessageToC(getNetworkId(), "C++RecalculateGcwRating", "", adjustedDelay, false);
	m_gcwRatingActualCalcTime = static_cast<int32>(::time(nullptr) + adjustedDelay);
}

// ----------------------------------------------------------------------

bool PlayerObject::needsGcwRatingRecalculated() const
{
	Pvp::PvpRankInfo const & pvpRankInfo = Pvp::getRankInfo(m_currentGcwRating.get());
	return ((m_currentGcwPoints.get() > 0) || (m_currentPvpKills.get() > 0) || ((pvpRankInfo.rank > 0) && (pvpRankInfo.ratingDecayBalance > 0)));
}

// ----------------------------------------------------------------------

void PlayerObject::clearSessionActivity()
{
	if (isAuthoritative())
	{
		m_sessionActivity = 0;
	}
	else
	{
		CreatureObject * const owner = getCreatureObject();
		if (owner)
			owner->sendControllerMessageToAuthServer(CM_clearSessionActivity, 0);
	}
}

// ----------------------------------------------------------------------

void PlayerObject::addSessionActivity(uint32 activity)
{
	if (isAuthoritative())
	{
		m_sessionActivity = m_sessionActivity.get() | activity;
	}
	else
	{
		CreatureObject * const owner = getCreatureObject();
		if (owner)
			owner->sendControllerMessageToAuthServer(CM_addSessionActivity, new MessageQueueGenericValueType<unsigned long>(static_cast<unsigned long>(activity)));
	}
}

// ----------------------------------------------------------------------

void PlayerObject::setPetId(NetworkId const & newId)
{
	m_petId.set(newId);
}

// ----------------------------------------------------------------------

void PlayerObject::setPetCommandList(std::vector<std::string> const &commands)
{
	m_petCommandList.set(commands);
}

// ----------------------------------------------------------------------

void PlayerObject::setPetToggledCommands(std::vector<std::string> const &commands)
{
	m_petToggledCommands.set(commands);
}

// ----------------------------------------------------------------------

void PlayerObject::updateKillMeter(float deltaTime)
{
	if(!isAuthoritative())
		return;

	if(m_killMeter.get() == 0)
		return;
	float currentValue = m_killMeter.get() + m_killMeterRemainder;
	float newValue = (currentValue) - (m_killMeter.get() / 150.0f) * deltaTime;
	int32 newValueInt = static_cast<int32>(newValue);
	m_killMeterRemainder = (newValue - newValueInt);
	if(m_killMeterRemainder > 0.01f)
	{
		newValueInt += 1;
		m_killMeterRemainder -= 1.0f;
	}
	if(newValueInt != m_killMeter.get())
		m_killMeter = newValueInt;

	return;
}

// ----------------------------------------------------------------------

void PlayerObject::incrementKillMeter(int amount)
{
	if(isAuthoritative())
	{
		if(m_killMeter.get() + amount > MAX_KILL_METER)
		{
			m_killMeter = MAX_KILL_METER;
			m_killMeterRemainder = 0.0f;
		}
		else
		{
			m_killMeter = m_killMeter.get() + amount;
		}
	}
	else
	{
		CreatureObject *owner = getCreatureObject();
		owner->sendControllerMessageToAuthServer(CM_incrementKillMeter, new MessageQueueGenericValueType<int>(amount));
	}
}

// ----------------------------------------------------------------------

bool PlayerObject::modifyCollectionSlotValue(std::string const & slotName, int64 delta)
{
	CreatureObject * const owner = getCreatureObject();
	if (!owner)
		return false;

	if (isAuthoritative())
	{
		CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
		if (!slotInfo)
		{
			DEBUG_WARNING(true, ("Trying to modify a collection slot for (%s) using an invalid collection slot name (%s)", getAccountDescription().c_str(), slotName.c_str()));
			return false;
		}

		Archive::AutoDeltaVariable<BitArray> * const collections = m_allCollections[slotInfo->slotIdIndex];

		// bit-type slot
		if (!slotInfo->counterTypeSlot)
		{
			// set bit-type slot
			if (delta > 0)
			{
				if (!collections->get().testBit(slotInfo->beginSlotId))
				{
					// check prereqs first
					if (!hasCompletedCollectionSlotPrereq(*slotInfo))
						return false;

					BitArray b = collections->get();
					b.setBit(slotInfo->beginSlotId);
					collections->set(b);

					LOG("CustomerService", ("Collection:%s added collection %d (%s/%s/%s/%s)",
						getAccountDescription().c_str(),
						slotInfo->absoluteBeginSlotId,
						slotInfo->collection.page.book.name.c_str(),
						slotInfo->collection.page.name.c_str(),
						slotInfo->collection.name.c_str(),
						slotInfo->name.c_str()));

					// for "server first" tracking, we need to check if the collection
					// is completed *BEFORE* triggering script, because script may clear
					// a completed collection (for repeatable collections), so that
					// when we come back from triggering script, the collection will not
					// appear to be completed, even though it really was completed and
					// "server first" should have been granted
					bool grantServerFirst = false;
					if (slotInfo->collection.trackServerFirst && (slotInfo->collection.serverFirstClaimTime <= 0))
					{
						bool const isSecuredLogin = (owner->getClient() && owner->getClient()->isGodValidated());
						if (!ConfigServerGame::getDisableCollectionServerFirstGrantForSecuredLogin() || !isSecuredLogin)
						{
							if (hasCompletedCollection(slotInfo->collection.name))
							{
								grantServerFirst = true;
							}
						}
					}

					GameScriptObject * gameScriptObject = owner->getScriptObject();
					if (gameScriptObject && slotInfo->notifyScriptOnModify)
					{
						ScriptParams params;
						params.addParam(slotInfo->collection.page.book.name.c_str()); // book
						params.addParam(slotInfo->collection.page.name.c_str()); // page
						params.addParam(slotInfo->collection.name.c_str());      // collection
						params.addParam(slotInfo->name.c_str());                 // slot
						params.addParam(false);                                  // is counter-type slot?
						params.addParam(static_cast<int>(0));                    // previous slot value
						params.addParam(static_cast<int>(1));                    // current slot value
						params.addParam(static_cast<int>(1));                    // max slot value
						params.addParam(true);                                   // slot completed?

						IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_COLLECTION_SLOT_MODIFIED, params));
					}

					// grant "server first", if necessary
					if (grantServerFirst)
					{
						PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
						if (planetObject)
						{
							planetObject->setCollectionServerFirst(slotInfo->collection, owner->getNetworkId(), owner->getAssignedObjectName(), getAccountDescription());
						}
					}
				}
			}
			// clear bit-type slot
			else if (delta < 0)
			{
				if (collections->get().testBit(slotInfo->beginSlotId))
				{
					BitArray b = collections->get();
					b.clearBit(slotInfo->beginSlotId);
					collections->set(b);

					LOG("CustomerService", ("Collection:%s removed collection %d (%s/%s/%s/%s)",
						getAccountDescription().c_str(),
						slotInfo->absoluteBeginSlotId,
						slotInfo->collection.page.book.name.c_str(),
						slotInfo->collection.page.name.c_str(),
						slotInfo->collection.name.c_str(),
						slotInfo->name.c_str()));

					GameScriptObject * gameScriptObject = owner->getScriptObject();
					if (gameScriptObject && slotInfo->notifyScriptOnModify)
					{
						ScriptParams params;
						params.addParam(slotInfo->collection.page.book.name.c_str()); // book
						params.addParam(slotInfo->collection.page.name.c_str()); // page
						params.addParam(slotInfo->collection.name.c_str());      // collection
						params.addParam(slotInfo->name.c_str());                 // slot
						params.addParam(false);                                  // is counter-type slot?
						params.addParam(static_cast<int>(1));                    // previous slot value
						params.addParam(static_cast<int>(0));                    // current slot value
						params.addParam(static_cast<int>(1));                    // max slot value
						params.addParam(false);                                  // slot completed?

						IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_COLLECTION_SLOT_MODIFIED, params));
					}
				}
			}
		}
		// counter-type slot
		else if (delta != 0)
		{
			// check prereqs first
			if ((delta > 0) && !hasCompletedCollectionSlotPrereq(*slotInfo))
				return false;

			int64 const currentSlotValue = static_cast<int64>(collections->get().getValue(slotInfo->beginSlotId, slotInfo->endSlotId));
			int64 const newSlotValue = clamp(static_cast<int64>(0), (currentSlotValue + delta), ((slotInfo->maxSlotValue > 0) ? static_cast<int64>(slotInfo->maxSlotValue) : static_cast<int64>(slotInfo->maxValueForNumBits)));

			// did slot value changed?
			if (currentSlotValue != newSlotValue)
			{
				BitArray b = collections->get();
				b.setValue(slotInfo->beginSlotId, slotInfo->endSlotId, static_cast<unsigned long>(newSlotValue));
				collections->set(b);

				LOG("CustomerService", ("Collection:%s modified collection %d-%d (%s/%s/%s/%s) from %lu to %lu",
					getAccountDescription().c_str(),
					slotInfo->absoluteBeginSlotId,
					slotInfo->absoluteEndSlotId,
					slotInfo->collection.page.book.name.c_str(),
					slotInfo->collection.page.name.c_str(),
					slotInfo->collection.name.c_str(),
					slotInfo->name.c_str(),
					static_cast<unsigned long>(currentSlotValue),
					static_cast<unsigned long>(newSlotValue)));

				bool const completedCollectionSlot = hasCompletedCollectionSlot(*slotInfo, static_cast<unsigned long>(newSlotValue));

				// for "server first" tracking, we need to check if the collection
				// is completed *BEFORE* triggering script, because script may clear
				// a completed collection (for repeatable collections), so that
				// when we come back from triggering script, the collection will not
				// appear to be completed, even though it really was completed and
				// "server first" should have been granted
				bool grantServerFirst = false;
				if (completedCollectionSlot && slotInfo->collection.trackServerFirst && (slotInfo->collection.serverFirstClaimTime <= 0))
				{
					bool const isSecuredLogin = (owner->getClient() && owner->getClient()->isGodValidated());
					if (!ConfigServerGame::getDisableCollectionServerFirstGrantForSecuredLogin() || !isSecuredLogin)
					{
						if (hasCompletedCollection(slotInfo->collection.name))
						{
							grantServerFirst = true;
						}
					}
				}

				GameScriptObject * gameScriptObject = owner->getScriptObject();
				if (gameScriptObject && slotInfo->notifyScriptOnModify)
				{
					ScriptParams params;
					params.addParam(slotInfo->collection.page.book.name.c_str()); // book
					params.addParam(slotInfo->collection.page.name.c_str()); // page
					params.addParam(slotInfo->collection.name.c_str());      // collection
					params.addParam(slotInfo->name.c_str());                 // slot
					params.addParam(true);                                   // is counter-type slot?
					params.addParam(static_cast<int>(currentSlotValue));     // previous slot value
					params.addParam(static_cast<int>(newSlotValue));         // current slot value
					params.addParam(static_cast<int>(slotInfo->maxSlotValue)); // max slot value
					params.addParam(completedCollectionSlot);                // slot completed?

					IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_COLLECTION_SLOT_MODIFIED, params));
				}

				// grant "server first", if necessary
				if (grantServerFirst)
				{
					PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
					if (planetObject)
					{
						planetObject->setCollectionServerFirst(slotInfo->collection, owner->getNetworkId(), owner->getAssignedObjectName(), getAccountDescription());
					}
				}
			}
		}
	}
	else
	{
		owner->sendControllerMessageToAuthServer(CM_modifyCollectionSlotValue, new MessageQueueGenericValueType<std::pair<std::string, int64> >(std::make_pair(slotName, delta)));
	}

	return true;
}

// ----------------------------------------------------------------------

bool PlayerObject::getCollectionSlotValue(std::string const & slotName, unsigned long & value) const
{
	value = 0;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if (!slotInfo)
	{
		DEBUG_WARNING(true, ("Trying to get a collection slot value for (%s) using an invalid collection slot name (%s)", getAccountDescription().c_str(), slotName.c_str()));
		return false;
	}

	return getCollectionSlotValue(*slotInfo, value);
}

// ----------------------------------------------------------------------

bool PlayerObject::getCollectionSlotValue(CollectionsDataTable::CollectionInfoSlot const & slotInfo, unsigned long & value) const
{
	// bit-type slot
	if (!slotInfo.counterTypeSlot)
	{
		if (m_allCollections[slotInfo.slotIdIndex]->get().testBit(slotInfo.beginSlotId))
			value = 1;
		else
			value = 0;
	}
	else
	{
		value = m_allCollections[slotInfo.slotIdIndex]->get().getValue(slotInfo.beginSlotId, slotInfo.endSlotId);
	}

	return true;
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionSlotPrereq(std::string const & slotName, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo /*= nullptr*/) const
{
	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if (!slotInfo)
	{
		DEBUG_WARNING(true, ("Checking for collection slot prereq completion for (%s) using an invalid collection slot name (%s)", getAccountDescription().c_str(), slotName.c_str()));
		return false;
	}

	return hasCompletedCollectionSlotPrereq(*slotInfo, collectionInfo);
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionSlotPrereq(CollectionsDataTable::CollectionInfoSlot const & slotInfo, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo /*= nullptr*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	if (slotInfo.prereqsPtr.empty())
		return true;

	bool completed = true;
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterPrereqs = slotInfo.prereqsPtr.begin(); iterPrereqs != slotInfo.prereqsPtr.end(); ++iterPrereqs)
	{
		if (!hasCompletedCollectionSlot(**iterPrereqs))
		{
			if (!collectionInfo)
				return false;

			collectionInfo->push_back(*iterPrereqs);
			completed = false;
		}
	}

	return completed;
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionSlot(std::string const & slotName) const
{
	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if (!slotInfo)
	{
		DEBUG_WARNING(true, ("Checking for collection slot completion for (%s) using an invalid collection slot name (%s)", getAccountDescription().c_str(), slotName.c_str()));
		return false;
	}

	return hasCompletedCollectionSlot(*slotInfo);
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionSlot(CollectionsDataTable::CollectionInfoSlot const & slotInfo) const
{
	// bit-type slot
	if (!slotInfo.counterTypeSlot)
		return m_allCollections[slotInfo.slotIdIndex]->get().testBit(slotInfo.beginSlotId);

	// counter-type slot is only completed if a max slot value is
	// specified and the current slot value equals the max slot value
	return ((slotInfo.maxSlotValue > 0) && (m_allCollections[slotInfo.slotIdIndex]->get().getValue(slotInfo.beginSlotId, slotInfo.endSlotId) >= slotInfo.maxSlotValue));
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionSlot(CollectionsDataTable::CollectionInfoSlot const & slotInfo, unsigned long slotValue)
{
	// bit-type slot
	if (!slotInfo.counterTypeSlot)
		return (slotValue > 0);

	// counter-type slot is only completed if a max slot value is
	// specified and the current slot value equals the max slot value
	return ((slotInfo.maxSlotValue > 0) && (slotValue >= slotInfo.maxSlotValue));
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollection(std::string const & collectionName) const
{
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection(collectionName);

	if (slots.empty())
		return false;

	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		if (!hasCompletedCollectionSlot(**iter))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionPage(std::string const & pageName) const
{
	std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage(pageName);

	if (collections.empty())
		return false;

	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = collections.begin(); iter != collections.end(); ++iter)
	{
		if (!hasCompletedCollection((*iter)->name))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionBook(std::string const & bookName) const
{
	std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook(bookName);

	if (pages.empty())
		return false;

	for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iter = pages.begin(); iter != pages.end(); ++iter)
	{
		if (!hasCompletedCollectionPage((*iter)->name))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionSlotCountInCollection(std::string const & collectionName, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo /*= nullptr*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection(collectionName);
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		if (hasCompletedCollectionSlot(**iter))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionSlotCountInPage(std::string const & pageName, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo /*= nullptr*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInPage(pageName);
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		if (hasCompletedCollectionSlot(**iter))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionCountInPage(std::string const & pageName, std::vector<CollectionsDataTable::CollectionInfoCollection const *> * collectionInfo /*= nullptr*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage(pageName);
	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = collections.begin(); iter != collections.end(); ++iter)
	{
		if (hasCompletedCollection((*iter)->name))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionSlotCountInBook(std::string const & bookName, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo /*= nullptr*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInBook(bookName);
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		if (hasCompletedCollectionSlot(**iter))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionCountInBook(std::string const & bookName, std::vector<CollectionsDataTable::CollectionInfoCollection const *> * collectionInfo /*= nullptr*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInBook(bookName);
	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = collections.begin(); iter != collections.end(); ++iter)
	{
		if (hasCompletedCollection((*iter)->name))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionPageCountInBook(std::string const & bookName, std::vector<CollectionsDataTable::CollectionInfoPage const *> * collectionInfo /*= nullptr*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook(bookName);
	for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iter = pages.begin(); iter != pages.end(); ++iter)
	{
		if (hasCompletedCollectionPage((*iter)->name))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionBookCount(std::vector<CollectionsDataTable::CollectionInfoBook const *> * collectionInfo /*= nullptr*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoBook const *> const & books = CollectionsDataTable::getAllBooks();
	for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator iter = books.begin(); iter != books.end(); ++iter)
	{
		if (hasCompletedCollectionBook((*iter)->name))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

void PlayerObject::migrateLegacyBadgesToCollection(std::vector<int> const & badges)
{
	if (badges.empty())
		return;

	// get the list of all possible collection id for the old badges
	static std::map<int, CollectionsDataTable::CollectionInfoSlot const *> badgeCollectionIds;
	if (badgeCollectionIds.empty())
	{
		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & badgeCollectionInfo = CollectionsDataTable::getSlotsInBook("badge_book");
		for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = badgeCollectionInfo.begin(); iter != badgeCollectionInfo.end(); ++iter)
		{
			if ((*iter)->slotIdIndex == 0)
			{
				badgeCollectionIds[(*iter)->absoluteBeginSlotId] = *iter;
			}
		}
	}

	BitArray ba = m_collections.get();
	bool anyBadgeMigrated = false;

	char buffer[32];
	std::string badgesString;
	CollectionsDataTable::CollectionInfoSlot const * slotInfo;
	int badgeIndex = 0;
	for (std::vector<int>::const_iterator iter = badges.begin(); iter != badges.end(); ++iter)
	{
		// for CS logging
		snprintf(buffer, sizeof(buffer)-1, "%d", *iter);
		buffer[sizeof(buffer)-1] = '\0';
		if (!badgesString.empty())
			badgesString += ", ";
		badgesString += buffer;

		for (int i = 0; i < 32; ++i)
		{
			if (((*iter & (1 << i)) != 0) && (badgeCollectionIds.count(badgeIndex) >= 1) && (!ba.testBit(badgeIndex)))
			{
				anyBadgeMigrated = true;
				ba.setBit(badgeIndex);

				slotInfo = badgeCollectionIds[badgeIndex];
				if (slotInfo)
				{
					LOG("CustomerService", ("Collection:%s migrated badge to collection %d (%s/%s/%s/%s)",
						getAccountDescription().c_str(),
						badgeIndex,
						slotInfo->collection.page.book.name.c_str(),
						slotInfo->collection.page.name.c_str(),
						slotInfo->collection.name.c_str(),
						slotInfo->name.c_str()));
				}
				else
				{
					LOG("CustomerService", ("Collection:%s migrated badge to collection %d",
						getAccountDescription().c_str(),
						badgeIndex));
				}
			}

			++badgeIndex;
		}
	}

	if (anyBadgeMigrated)
		m_collections.set(ba);

	// CS log the badge objvars so we have a record of badges that were migrated
	if (!badgesString.empty())
		LOG("CustomerService", ("Collection:%s migrated legacy badges (%s) over to collection system", getAccountDescription().c_str(), badgesString.c_str()));
}

// ----------------------------------------------------------------------

void PlayerObject::updateChatSpamSpatialNumCharacters(NetworkId const & character, int spatialNumCharacters)
{
	if (!isAuthoritative())
		return;

	int const timeNow = static_cast<int>(::time(nullptr));
	bool syncChatServer = false;

	// new interval, reset
	if (timeNow > m_chatSpamTimeEndInterval.get())
	{
		m_chatSpamTimeEndInterval = timeNow + (ConfigServerGame::getChatSpamLimiterIntervalMinutes() * 60);
		m_chatSpamSpatialNumCharacters = spatialNumCharacters;
		m_chatSpamNonSpatialNumCharacters = 0;
		syncChatServer = true;
	}
	// update spatial character count
	else
	{
		m_chatSpamSpatialNumCharacters = m_chatSpamSpatialNumCharacters.get() + spatialNumCharacters;
	}

	// sync chat character count with chat server
	if ((syncChatServer) || (timeNow > m_chatSpamNextTimeToSyncWithChatServer.get()))
	{
		time_t timeUnsquelch = static_cast<time_t>(getSecondsUntilUnsquelched());
		if (timeUnsquelch > 0)
			timeUnsquelch += ::time(nullptr);

		GenericValueTypeMessage<std::pair<std::pair<std::pair<NetworkId, int>, int>, std::pair<int, int> > > chatStatistics("ChatStatisticsGS", std::make_pair(std::make_pair(std::make_pair(character, static_cast<int>(timeUnsquelch)), m_chatSpamTimeEndInterval.get()), std::make_pair(m_chatSpamSpatialNumCharacters.get(), m_chatSpamNonSpatialNumCharacters.get())));
		Chat::sendToChatServer(chatStatistics);
		m_chatSpamNextTimeToSyncWithChatServer = timeNow + ConfigServerGame::getChatStatisticsReportIntervalSeconds();
	}
}

// ----------------------------------------------------------------------

void PlayerObject::handleChatStatisticsFromChatServer(NetworkId const & character, int chatSpamTimeEndInterval, int spatialNumCharacters, int nonSpatialNumCharacters)
{
	if (!isAuthoritative())
		return;

	int const timeNow = static_cast<int>(::time(nullptr));
	bool syncChatServer = false;

	// new interval, reset
	if (timeNow > m_chatSpamTimeEndInterval.get())
	{
		m_chatSpamTimeEndInterval = timeNow + (ConfigServerGame::getChatSpamLimiterIntervalMinutes() * 60);
		m_chatSpamSpatialNumCharacters = 0;
		m_chatSpamNonSpatialNumCharacters = 0;
		syncChatServer = true;
	}
	// chat server is in sync with us, update non-spatial
	// character count with value from chat server
	else if (chatSpamTimeEndInterval == m_chatSpamTimeEndInterval.get())
	{
		m_chatSpamNonSpatialNumCharacters = nonSpatialNumCharacters;
	}
	// chat server is out of sync, ignore data, and resync with chat server
	else
	{
		syncChatServer = true;
	}

	// sync chat character count with chat server
	if (syncChatServer || ((spatialNumCharacters != m_chatSpamSpatialNumCharacters.get()) && (timeNow > m_chatSpamNextTimeToSyncWithChatServer.get())))
	{
		time_t timeUnsquelch = static_cast<time_t>(getSecondsUntilUnsquelched());
		if (timeUnsquelch > 0)
			timeUnsquelch += ::time(nullptr);

		GenericValueTypeMessage<std::pair<std::pair<std::pair<NetworkId, int>, int>, std::pair<int, int> > > chatStatistics("ChatStatisticsGS", std::make_pair(std::make_pair(std::make_pair(character, static_cast<int>(timeUnsquelch)), m_chatSpamTimeEndInterval.get()), std::make_pair(m_chatSpamSpatialNumCharacters.get(), m_chatSpamNonSpatialNumCharacters.get())));
		Chat::sendToChatServer(chatStatistics);
		m_chatSpamNextTimeToSyncWithChatServer = timeNow + ConfigServerGame::getChatStatisticsReportIntervalSeconds();
	}
}

// ----------------------------------------------------------------------

void PlayerObject::updateCitizenshipInfo()
{
	if (!isAuthoritative())
		return;

	CreatureObject const * const owner = getCreatureObject();
	if (!owner)
		return;

	std::string cityName;
	int8 citizenType = static_cast<int8>(CityDataCitizenType::NotACitizen);

	std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(owner->getNetworkId());
	if (!cityIds.empty())
	{
		int const cityId = cityIds.front();
		CityInfo const & cityInfo = CityInterface::getCityInfo(cityId);
		cityName = cityInfo.getCityName();
		if (!cityName.empty())
		{
			if (owner->getNetworkId() == cityInfo.getLeaderId())
				citizenType = static_cast<int8>(CityDataCitizenType::Mayor);
			else if (CityInterface::isCityMilitia(cityId, owner->getNetworkId()))
				citizenType = static_cast<int8>(CityDataCitizenType::Militia);
			else
				citizenType = static_cast<int8>(CityDataCitizenType::Citizen);
		}
	}

	if (m_citizenshipCity.get() != cityName)
		m_citizenshipCity = cityName;

	if (m_citizenshipType.get() != citizenType)
		m_citizenshipType = citizenType;
}

// ----------------------------------------------------------------------

void PlayerObject::updateGcwRegionInfo()
{
	if (!isAuthoritative())
		return;

	CreatureObject const * const owner = getCreatureObject();
	if (!owner)
		return;

	std::string const & sceneId = ServerWorld::getSceneId();
	Vector const pos = owner->findPosition_w();
	std::string const * currentGcwRegion = &(Pvp::getGcwScoreCategoryRegion(sceneId, pos));
	if (currentGcwRegion->empty())
	{
		// if not in a GCW contested region, use the default region
		Pvp::GcwScoreCategory const * const defaultGcwRegion = Pvp::getGcwScoreDefaultCategoryForPlanet(sceneId);
		if (defaultGcwRegion)
			currentGcwRegion = &(defaultGcwRegion->categoryName);
	}

	// set the GCW region the character is currently in
	m_currentGcwRegion.set(*currentGcwRegion);
}

// ----------------------------------------------------------------------

void PlayerObject::updateGcwDefenderRegionInfo()
{
	if (!isAuthoritative())
		return;

	CreatureObject const * const owner = getCreatureObject();
	if (!owner)
		return;

	int timeQualifyForBonus = -1;
	bool qualifyForWarPlannerTitle = false;

	std::string cityGcwDefenderRegion;
	bool cityGcwDefenderRegionHasBonus = false;;
	bool cityGcwDefenderRegionHasTitle = false;;

	std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(owner->getNetworkId());
	if (!cityIds.empty())
	{
		CityInfo const & cityInfo = CityInterface::getCityInfo(cityIds.front());
		uint32 const cityFaction = cityInfo.getFaction();

		// mayor who is same aligned with his aligned city can use the "War Planner" title
		if (!PvpData::isNeutralFactionId(cityFaction) && (cityFaction == owner->getPvpFaction()) && (cityInfo.getLeaderId() == owner->getNetworkId()))
			qualifyForWarPlannerTitle = true;

		std::string const & gcwDefenderRegion = cityInfo.getGcwDefenderRegion();
		if (!gcwDefenderRegion.empty())
		{
			cityGcwDefenderRegion = gcwDefenderRegion;

			int const timeJoinedGcwDefenderRegion = cityInfo.getTimeJoinedGcwDefenderRegion();
			timeQualifyForBonus = static_cast<int>(::time(nullptr)) - (ConfigServerGame::getGcwDaysRequiredForGcwRegionDefenderBonus() * 86400);
			if (!PvpData::isNeutralFactionId(cityFaction) && (cityFaction == owner->getPvpFaction()) && (timeJoinedGcwDefenderRegion < timeQualifyForBonus))
			{				
				cityGcwDefenderRegionHasBonus = true;

				int regionPercentileScore = 0;
				if (PvpData::isImperialFactionId(cityFaction))
					regionPercentileScore = ServerUniverse::getInstance().getGcwImperialScorePercentile(gcwDefenderRegion);
				else if (PvpData::isRebelFactionId(cityFaction))
					regionPercentileScore = 100 - ServerUniverse::getInstance().getGcwImperialScorePercentile(gcwDefenderRegion);

				// title is available once the region is 70% controlled
				if (regionPercentileScore >= 70)
					cityGcwDefenderRegionHasTitle = true;
			}
		}
	}

	std::string guildGcwDefenderRegion;
	bool guildGcwDefenderRegionHasBonus = false;;
	bool guildGcwDefenderRegionHasTitle = false;;

	int const guildId = owner->getGuildId();
	if (guildId > 0)
	{
		GuildInfo const * const gi = GuildInterface::getGuildInfo(guildId);
		if (gi)
		{
			uint32 const guildFaction = GuildInterface::getGuildCurrentFaction(*gi);

			// guild leader who is same aligned with his aligned guild that meets the
			// minimum guild member count requirement can use the "War Planner" title
			if (!PvpData::isNeutralFactionId(guildFaction) && (guildFaction == owner->getPvpFaction()) && (gi->m_leaderId == owner->getNetworkId()) && (gi->getCountMembersOnly() >= ConfigServerGame::getGcwGuildMinMembersForGcwRegionDefender()))
				qualifyForWarPlannerTitle = true;

			std::string const & gcwDefenderRegion = GuildInterface::getGuildCurrentGcwDefenderRegion(*gi);
			if (!gcwDefenderRegion.empty())
			{
				guildGcwDefenderRegion = gcwDefenderRegion;

				int const timeJoinedGcwDefenderRegion = GuildInterface::getTimeJoinedGuildCurrentGcwDefenderRegion(*gi);
				if (timeQualifyForBonus < 0)
					timeQualifyForBonus = static_cast<int>(::time(nullptr)) - (ConfigServerGame::getGcwDaysRequiredForGcwRegionDefenderBonus() * 86400);

				if (!PvpData::isNeutralFactionId(guildFaction) && (guildFaction == owner->getPvpFaction()) && (timeJoinedGcwDefenderRegion < timeQualifyForBonus))
				{
					guildGcwDefenderRegionHasBonus = true;

					int regionPercentileScore = 0;
					if (PvpData::isImperialFactionId(guildFaction))
						regionPercentileScore = ServerUniverse::getInstance().getGcwImperialScorePercentile(gcwDefenderRegion);
					else if (PvpData::isRebelFactionId(guildFaction))
						regionPercentileScore = 100 - ServerUniverse::getInstance().getGcwImperialScorePercentile(gcwDefenderRegion);

					// title is available once the region is 70% controlled
					if (regionPercentileScore >= 70)
						guildGcwDefenderRegionHasTitle = true;
				}
			}
		}
	}

	m_cityGcwDefenderRegion.set(std::make_pair(cityGcwDefenderRegion, std::make_pair(cityGcwDefenderRegionHasBonus, cityGcwDefenderRegionHasTitle)));
	m_guildGcwDefenderRegion.set(std::make_pair(guildGcwDefenderRegion, std::make_pair(guildGcwDefenderRegionHasBonus, guildGcwDefenderRegionHasTitle)));

	// clear title if not qualified
	{
		std::string const & currentTitle = getTitle();
		if (currentTitle == "city_gcw_region_defender")
		{
			if (cityGcwDefenderRegion.empty() || !cityGcwDefenderRegionHasTitle)
				setTitle(std::string());
		}
		else if (currentTitle == "guild_gcw_region_defender")
		{
			if (guildGcwDefenderRegion.empty() || !guildGcwDefenderRegionHasTitle)
				setTitle(std::string());
		}
	}

	// grant/clear "War Planner" title
	{
		std::string const & currentTitle = getTitle();
		if (qualifyForWarPlannerTitle)
		{
			if (PvpData::isImperialFactionId(owner->getPvpFaction()))
			{
				modifyCollectionSlotValue("imperial_gcw_war_planner", 1ll);
				modifyCollectionSlotValue("rebel_gcw_war_planner", -1ll);

				if (currentTitle == "rebel_gcw_war_planner")
					setTitle("imperial_gcw_war_planner");
			}
			else if (PvpData::isRebelFactionId(owner->getPvpFaction()))
			{
				modifyCollectionSlotValue("imperial_gcw_war_planner", -1ll);
				modifyCollectionSlotValue("rebel_gcw_war_planner", 1ll);

				if (currentTitle == "imperial_gcw_war_planner")
					setTitle("rebel_gcw_war_planner");
			}
			else
			{
				modifyCollectionSlotValue("imperial_gcw_war_planner", -1ll);
				modifyCollectionSlotValue("rebel_gcw_war_planner", -1ll);

				if ((currentTitle == "imperial_gcw_war_planner") || (currentTitle == "rebel_gcw_war_planner"))
					setTitle(std::string());
			}
		}
		else
		{
			modifyCollectionSlotValue("imperial_gcw_war_planner", -1ll);
			modifyCollectionSlotValue("rebel_gcw_war_planner", -1ll);

			if ((currentTitle == "imperial_gcw_war_planner") || (currentTitle == "rebel_gcw_war_planner"))
				setTitle(std::string());
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObject::squelch(NetworkId const & squelchedById, std::string const & squelchedByName, int squelchDurationSeconds)
{
	if (isAuthoritative())
	{
		m_squelchedById = squelchedById;
		m_squelchedByName = squelchedByName;

		// persist squelch info
		setObjVarItem(OBJVAR_SQUELCH_ID, squelchedById);
		setObjVarItem(OBJVAR_SQUELCH_NAME, squelchedByName);

		if (squelchDurationSeconds < 0)
		{
			m_squelchExpireTime = -1;

			// persist squelch info
			setObjVarItem(OBJVAR_SQUELCH_EXPIRE, -1);
		}
		else
		{
			m_squelchExpireTime = static_cast<int32>(::time(nullptr)) + squelchDurationSeconds;

			// persist squelch info
			setObjVarItem(OBJVAR_SQUELCH_EXPIRE, static_cast<int>(ServerClock::getInstance().getGameTimeSeconds()) + squelchDurationSeconds);
		}

		// update chat server
		CreatureObject * const owner = getCreatureObject();
		if (owner)
		{
			GenericValueTypeMessage<std::pair<NetworkId, int> > setUnsquelchTime("SetUnsquelchTime", std::make_pair(owner->getNetworkId(), static_cast<int>(m_squelchExpireTime.get())));
			Chat::sendToChatServer(setUnsquelchTime);
		}
	}
	else
	{
		CreatureObject * const owner = getCreatureObject();
		if (owner)
			owner->sendControllerMessageToAuthServer(CM_squelch, new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > >(std::make_pair(squelchedByName, std::make_pair(squelchedById, squelchDurationSeconds))));
	}
}

// ----------------------------------------------------------------------

void PlayerObject::unsquelch()
{
	if (isAuthoritative())
	{
		m_squelchedById = NetworkId::cms_invalid;
		m_squelchedByName = std::string();
		m_squelchExpireTime = 0;

		// persist squelch info
		removeObjVarItem(OBJVAR_SQUELCH_ID);
		removeObjVarItem(OBJVAR_SQUELCH_NAME);
		removeObjVarItem(OBJVAR_SQUELCH_EXPIRE);

		// update chat server
		CreatureObject * const owner = getCreatureObject();
		if (owner)
		{
			GenericValueTypeMessage<std::pair<NetworkId, int> > setUnsquelchTime("SetUnsquelchTime", std::make_pair(owner->getNetworkId(), 0));
			Chat::sendToChatServer(setUnsquelchTime);
		}
	}
	else
	{
		CreatureObject * const owner = getCreatureObject();
		if (owner)
			owner->sendControllerMessageToAuthServer(CM_unsquelch, nullptr);
	}
}

// ----------------------------------------------------------------------
// returns 0 if not currently squelched; returns < 0 if indefinitely squelched
int PlayerObject::getSecondsUntilUnsquelched()
{
	// not squelched
	if (!m_squelchedById.get().isValid())
		return 0;

	// squelched, but need to check to see if the squelch period has already expired
	if (m_squelchExpireTime.get() < 0) // squelched indefinitely
		return -1;

	int32 const timeNow = static_cast<int32>(::time(nullptr));
	if (timeNow < m_squelchExpireTime.get()) // still in squelch period
		return (m_squelchExpireTime.get() - timeNow);

	// squelch has expired
	if (isAuthoritative())
	{
		m_squelchedById = NetworkId::cms_invalid;
		m_squelchedByName = std::string();
		m_squelchExpireTime = 0;

		// persist squelch info
		removeObjVarItem(OBJVAR_SQUELCH_ID);
		removeObjVarItem(OBJVAR_SQUELCH_NAME);
		removeObjVarItem(OBJVAR_SQUELCH_EXPIRE);
	}
	return 0;
}

// ----------------------------------------------------------------------

void PlayerObject::modifyShowBackpack(bool b)
{
	if(m_showBackpack.get() != b)
		m_showBackpack = b;
}

// ----------------------------------------------------------------------

void PlayerObject::modifyShowHelmet(bool b)
{
	if(m_showHelmet.get () != b)
		m_showHelmet = b;
}

void PlayerObject::setEnvironmentFlags(EnvironmentInfo const & flags)
{
	m_environmentFlags = flags;
}

// ----------------------------------------------------------------------

void PlayerObject::toggleOutOfCharacter()
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.toggleBit(MatchMakingId::B_outOfCharacter);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}

// ----------------------------------------------------------------------

void PlayerObject::toggleLookingForWork()
{
	if (isAuthoritative())
	{
		MatchMakingId matchMakingId(m_matchMakingCharacterProfileId.get());

		matchMakingId.toggleBit(MatchMakingId::B_lookingForWork);

		setMatchMakingCharacterProfileId(matchMakingId);
	}
}


// ----------------------------------------------------------------------

void PlayerObject::setAccountNumLotsOverLimitSpam()
{
	if (!isAuthoritative())
		return;

	// if over the lot limit, spam warning; otherwise, if no
	// longer over the lot limit, remove lot over limit restriction
	CreatureObject * const owner = getCreatureObject();
	if (!owner)
		return;

	if (m_accountNumLots.get() > owner->getMaxNumberOfLots())
	{
		if (owner->getObjVars().hasItem("lotOverlimit"))
		{
			m_accountNumLotsOverLimitSpam = m_accountNumLots.get() - owner->getMaxNumberOfLots();

			int const timeNow = static_cast<int>(::time(nullptr));
			int violationTime = 0;
			if (!owner->getObjVars().getItem("lotOverlimit.violation_time", violationTime))
			{
				violationTime = timeNow;
				IGNORE_RETURN(owner->setObjVarItem("lotOverlimit.violation_time", violationTime));
			}

			if (owner->getClient())
			{
				// spam once every 5 minutes for 24 hours after the violation, then once every minute after that
				bool shouldSpam = true;
				if ((violationTime + 86400) >= timeNow)
				{
					int timeLastSpam = 0;
					if (owner->getObjVars().getItem("lotOverlimit.last_spam_time", timeLastSpam) && ((timeLastSpam + 300) >= timeNow))
						shouldSpam = false;

					if (shouldSpam)
						IGNORE_RETURN(owner->setObjVarItem("lotOverlimit.last_spam_time", timeNow));
				}
				else
				{
					// over 24 hours after violation, spam every minute (i.e. each time this messageTo fires)
					owner->removeObjVarItem("lotOverlimit.last_spam_time");
				}

				if (shouldSpam)
				{
					Chat::sendSystemMessage(*owner, Unicode::narrowToWide("!!!WARNING!!! !!!WARNING!!! !!!WARNING!!! !!!WARNING!!! !!!WARNING!!!"), Unicode::emptyString);
					Chat::sendSystemMessage(*owner, Unicode::narrowToWide(FormattedString<256>().sprintf("YOU ARE %d LOTS OVER THE LOT LIMIT!!!", m_accountNumLotsOverLimitSpam.get())), Unicode::emptyString);

					std::string lotOverlimitStructureName;
					std::string lotOverlimitStructureLocation;
					IGNORE_RETURN(owner->getObjVars().getItem("lotOverlimit.structure_name", lotOverlimitStructureName));
					IGNORE_RETURN(owner->getObjVars().getItem("lotOverlimit.structure_location", lotOverlimitStructureLocation));
					Chat::sendSystemMessage(*owner, Unicode::narrowToWide(FormattedString<256>().sprintf("THE %s STRUCTURE LOCATED AT %s CAUSED YOU TO EXCEED THE LOT LIMIT!!!", lotOverlimitStructureName.c_str(), lotOverlimitStructureLocation.c_str())), Unicode::emptyString);

					Chat::sendSystemMessage(*owner, Unicode::narrowToWide("!!!WARNING!!! !!!WARNING!!! !!!WARNING!!! !!!WARNING!!! !!!WARNING!!!"), Unicode::emptyString);
				}
			}
		}
		else
		{
			// over the limit, but we shouldn't spam
			m_accountNumLotsOverLimitSpam = 0;
		}
	}
	else
	{
		m_accountNumLotsOverLimitSpam = 0;
		owner->removeObjVarItem("lotOverlimit");
	}
}

// ----------------------------------------------------------------------

bool PlayerObject::canAccessGalacticReserve(real x, real z)
{
	// must be at a bank terminal in Mos Eisley, Theed, or Coronet to access the Galactic Reserve
	std::string const & sceneId = ServerWorld::getSceneId();
	if ((sceneId != "tatooine") && (sceneId != "naboo") && (sceneId != "corellia"))
		return false;

	std::vector<const Region *> result;
	RegionMaster::getRegionsAtPoint(sceneId, x, z, result);
	if (result.empty())
		return false;

	static Unicode::String const regionMosEisley = Unicode::narrowToWide("@tatooine_region_names:mos_eisley");
	static Unicode::String const regionTheed = Unicode::narrowToWide("@naboo_region_names:theed");
	static Unicode::String const regionCoronet = Unicode::narrowToWide("@corellia_region_names:coronet");
	static uint32 const hashMosEisley = Crc::calculate(regionMosEisley.c_str(), regionMosEisley.length()*2);
	static uint32 const hashTheed = Crc::calculate(regionTheed.c_str(), regionTheed.length()*2);
	static uint32 const hashCoronet = Crc::calculate(regionCoronet.c_str(), regionCoronet.length()*2);

	for(std::vector<const Region*>::iterator it = result.begin(); it != result.end(); ++it)
	{
		if (((sceneId == "tatooine") && ((*it)->getNameCrc() == hashMosEisley)) ||
			((sceneId == "naboo") && ((*it)->getNameCrc() == hashTheed)) ||
			((sceneId == "corellia") && ((*it)->getNameCrc() == hashCoronet)))
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

void PlayerObject::depositToGalacticReserve(bool ignoreCooldownAndLocationCheck)
{
	if (!isAuthoritative())
		return;

	CreatureObject * const co = getCreatureObject();
	if (!co)
		return;

	Client const * const c = co->getClient();
	if (c && c->isFreeTrialAccount())
	{
		Chat::sendSystemMessage(*co, Unicode::narrowToWide("Trial account cannot access the Galactic Reserve."), Unicode::emptyString);
		return;
	}

	bool const isGod = (c && c->isGod());

	// check for cooldown
	if (!isGod && !ignoreCooldownAndLocationCheck)
	{
		int cooldown;
		if (co->getObjVars().getItem("galactic_reserve_cooldown", cooldown))
		{
			int const timeNow = static_cast<int>(ServerClock::getInstance().getGameTimeSeconds());
			if (cooldown > timeNow)
			{
				if (c)
					Chat::sendSystemMessage(*co, Unicode::narrowToWide(FormattedString<1024>().sprintf("You must wait %s before you can access the Galactic Reserve again.", CalendarTime::convertSecondsToMS(static_cast<unsigned>(cooldown - timeNow)).c_str())), Unicode::emptyString);

				return;
			}
		}
	}

	// must be at a bank terminal in Mos Eisley, Theed, or Coronet to access the Galactic Reserve
	if (!ignoreCooldownAndLocationCheck)
	{
		Vector const positionW = co->getPosition_w();
		if (!canAccessGalacticReserve(positionW.x, positionW.z))
		{
			if (c)
				Chat::sendSystemMessage(*co, Unicode::narrowToWide("You must be at a bank terminal in Mos Eisley, Theed, or Coronet to access the Galactic Reserve."), Unicode::emptyString);

			return;
		}
	}

	if (static_cast<int>(m_galacticReserveDeposit.get()) >= ConfigServerGame::getMaxGalacticReserveDepositBillion())
	{
		if (c)
			Chat::sendSystemMessage(*co, Unicode::narrowToWide(FormattedString<1024>().sprintf("You already have the maximum %d billion credits in the Galactic Reserve.", ConfigServerGame::getMaxGalacticReserveDepositBillion())), Unicode::emptyString);

		return;
	}

	// transfer as much cash as necessary into bank to get 1 billion in bank
	int amountToTransferToBank = 1000000000 - co->getBankBalance();
	if (amountToTransferToBank > co->getCashBalance())
	{
		if (c)
			Chat::sendSystemMessage(*co, Unicode::narrowToWide("You do not currently have 1 billion credits to deposit to the Galactic Reserve."), Unicode::emptyString);

		return;
	}

	if (amountToTransferToBank > 0)
	{
		co->depositCashToBank(amountToTransferToBank);
	}

	// final sanity check to make sure there's 1 billion in the bank
	if (co->getBankBalance() < 1000000000)
	{
		if (c)
			Chat::sendSystemMessage(*co, Unicode::narrowToWide("You do not currently have 1 billion credits to deposit to the Galactic Reserve."), Unicode::emptyString);

		return;
	}

	// do the transfer
	if (!co->transferBankCreditsTo("galactic_reserve", 1000000000))
	{
		if (c)
			Chat::sendSystemMessage(*co, Unicode::narrowToWide("Failed to transfer 1 billion credits to the Galactic Reserve."), Unicode::emptyString);

		return;
	}

	// update reserve balance
	m_galacticReserveDeposit = m_galacticReserveDeposit.get() + 1;
	co->setObjVarItem("galactic_reserve", static_cast<int>(m_galacticReserveDeposit.get()));

	// set cooldown (15 minutes)
	if (!ignoreCooldownAndLocationCheck)
	{
		co->setObjVarItem("galactic_reserve_cooldown", static_cast<int>(ServerClock::getInstance().getGameTimeSeconds() + (15 * 60)));
	}

	// display confirmation message
	if (c)
		Chat::sendSystemMessage(*co, Unicode::narrowToWide(FormattedString<1024>().sprintf("1 billion credits have been deposited to the Galactic Reserve.  Your new Galactic Reserve balance is %d billion credits.", m_galacticReserveDeposit.get())), Unicode::emptyString);
}

// ----------------------------------------------------------------------

void PlayerObject::withdrawFromGalacticReserve()
{
	if (!isAuthoritative())
		return;

	CreatureObject * const co = getCreatureObject();
	if (!co)
		return;

	Client const * const c = co->getClient();
	if (!c)
		return;

	if (c->isFreeTrialAccount())
	{
		Chat::sendSystemMessage(*co, Unicode::narrowToWide("Trial account cannot access the Galactic Reserve."), Unicode::emptyString);
		return;
	}

	// check for cooldown
	if (!c->isGod())
	{
		int cooldown;
		if (co->getObjVars().getItem("galactic_reserve_cooldown", cooldown))
		{
			int const timeNow = static_cast<int>(ServerClock::getInstance().getGameTimeSeconds());
			if (cooldown > timeNow)
			{
				Chat::sendSystemMessage(*co, Unicode::narrowToWide(FormattedString<1024>().sprintf("You must wait %s before you can access the Galactic Reserve again.", CalendarTime::convertSecondsToMS(static_cast<unsigned>(cooldown - timeNow)).c_str())), Unicode::emptyString);
				return;
			}
		}
	}

	// must be at a bank terminal in Mos Eisley, Theed, or Coronet to access the Galactic Reserve
	Vector const positionW = co->getPosition_w();
	if (!canAccessGalacticReserve(positionW.x, positionW.z))
	{
		Chat::sendSystemMessage(*co, Unicode::narrowToWide("You must be at a bank terminal in Mos Eisley, Theed, or Coronet to access the Galactic Reserve."), Unicode::emptyString);
		return;
	}

	if (m_galacticReserveDeposit.get() <= 0)
	{
		Chat::sendSystemMessage(*co, Unicode::narrowToWide("You do not have any deposit in the Galactic Reserve to withdraw."), Unicode::emptyString);
		return;
	}

	// move all the bank to cash to make room to receive the 1 billion into the bank
	if (co->getBankBalance() > 0)
	{
		if ((co->getCashBalance() + co->getBankBalance()) > ConfigServerGame::getMaxMoney())
		{
			Chat::sendSystemMessage(*co, Unicode::narrowToWide("Withdrawing 1 billion credits from the Galactic Reserve would cause your cash/bank balance to exceed the maximum balance limit."), Unicode::emptyString);
			return;
		}

		co->withdrawCashFromBank(co->getBankBalance());
	}

	// final sanity check to make sure the bank is empty to receive the 1 billion
	if (co->getBankBalance() > 0)
	{
		Chat::sendSystemMessage(*co, Unicode::narrowToWide("Withdrawing 1 billion credits from the Galactic Reserve would cause your cash/bank balance to exceed the maximum balance limit."), Unicode::emptyString);
		return;
	}

	// do the transfer
	if (!co->transferBankCreditsFrom("galactic_reserve", 1000000000))
	{
		Chat::sendSystemMessage(*co, Unicode::narrowToWide("Failed to withdraw 1 billion credits from the Galactic Reserve."), Unicode::emptyString);
		return;
	}

	// update reserve balance
	m_galacticReserveDeposit = m_galacticReserveDeposit.get() - 1;
	if (m_galacticReserveDeposit.get() > 0)
		co->setObjVarItem("galactic_reserve", static_cast<int>(m_galacticReserveDeposit.get()));
	else
		co->removeObjVarItem("galactic_reserve");

	// set cooldown (15 minutes)
	co->setObjVarItem("galactic_reserve_cooldown", static_cast<int>(ServerClock::getInstance().getGameTimeSeconds() + (15 * 60)));

	// display confirmation message
	Chat::sendSystemMessage(*co, Unicode::narrowToWide(FormattedString<1024>().sprintf("1 billion credits have been withdrawn from the Galactic Reserve.  Your new Galactic Reserve balance is %d billion credits.", m_galacticReserveDeposit.get())), Unicode::emptyString);
}

// ----------------------------------------------------------------------

void PlayerObject::depersistGalacticReserveDeposit()
{
	if (!isAuthoritative())
		return;

	m_galacticReserveDeposit = 0;

	CreatureObject const * const co = getCreatureObject();
	if (!co)
		return;

	// depersist galactic reserve 
	int temp;
	if (co->getObjVars().getItem("galactic_reserve", temp))
		m_galacticReserveDeposit = static_cast<int8>(temp);
}

// ----------------------------------------------------------------------

void PlayerObjectNamespace::checkAndSetCharacterAgeTitle(PlayerObject & player, bool thoroughCheck)
{
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> s_vectorCharacterAgeTitle;
	if (s_vectorCharacterAgeTitle.empty())
	{
		// we'll exit this loop as soon as we don't find the "next" character age title collection slot
		for (int i = 1; i <= 1000000; ++i)
		{
			CollectionsDataTable::CollectionInfoSlot const * collectionSlot = CollectionsDataTable::getSlotByName(FormattedString<128>().sprintf("character_age_%dyr", i));
			if (!collectionSlot)
				break;

			FATAL((collectionSlot->collection.name != std::string("character_age_collection")), ("collection slot %s must be in collection character_age_collection", collectionSlot->name.c_str()));
			FATAL((collectionSlot->collection.page.name != std::string("character_age_page")), ("collection slot %s must be in collection page character_age_page", collectionSlot->name.c_str()));
			FATAL((collectionSlot->collection.page.book.name != std::string("character_age_book")), ("collection slot %s must be in collection book character_age_book", collectionSlot->name.c_str()));

			s_vectorCharacterAgeTitle.push_back(collectionSlot);
		}
	}

	int const ageInYears = ((player.getBornDate() > 0) ? (player.getAgeInDays() / 365) : 0);

	if (!thoroughCheck)
	{
		// just do quickie check to see if the player has turned a new year older
		if (ageInYears <= static_cast<int>(s_vectorCharacterAgeTitle.size()))
		{
			if (ageInYears >= 1)
			{
				if (!player.hasCompletedCollectionSlot(*s_vectorCharacterAgeTitle[ageInYears - 1]))
				{
					// this will force a check/set/unset of the proper character age slot for the player
					thoroughCheck = true;
				}
			}
		}
		else
		{
			// we need to add additional character age collection slots

			// this will make sure to unset all previous character age slots the player may have
			thoroughCheck = true;
		}
	}

	// go through all possible character age titles and verify that the appropriate one is set and the others are unset
	if (thoroughCheck)
	{
		for (int i = 0, size = s_vectorCharacterAgeTitle.size(); i < size; ++i)
		{
			if ((i + 1) == ageInYears)
			{
				player.modifyCollectionSlotValue(s_vectorCharacterAgeTitle[i]->name, 1ll);
			}
			else
			{
				player.modifyCollectionSlotValue(s_vectorCharacterAgeTitle[i]->name, -1ll);
			}
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObjectNamespace::checkAndSetAccountAgeTitle(PlayerObject & player)
{
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> s_vectorAccountAgeTitle;
	if (s_vectorAccountAgeTitle.empty())
	{
		// we'll exit this loop as soon as we don't find the "next" account age title collection slot
		for (int i = 1; i <= 1000000; ++i)
		{
			CollectionsDataTable::CollectionInfoSlot const * collectionSlot = CollectionsDataTable::getSlotByName(FormattedString<128>().sprintf("account_age_%dyr", i));
			if (!collectionSlot)
				break;

			FATAL((collectionSlot->collection.name != std::string("account_age_collection")), ("collection slot %s must be in collection account_age_collection", collectionSlot->name.c_str()));
			FATAL((collectionSlot->collection.page.name != std::string("account_age_page")), ("collection slot %s must be in collection page account_age_page", collectionSlot->name.c_str()));
			FATAL((collectionSlot->collection.page.book.name != std::string("account_age_book")), ("collection slot %s must be in collection book account_age_book", collectionSlot->name.c_str()));

			s_vectorAccountAgeTitle.push_back(collectionSlot);
		}
	}

	CreatureObject const * const co = player.getCreatureObject();
	if (!co)
		return;

	Client const * const c = co->getClient();
	if (!c)
		return;

	if (c->isUsingAdminLogin())
		return;

	unsigned int const ageInYears = c->getEntitlementEntitledTime() / 365u;

	// go through all possible account age titles and verify that the appropriate one is set and the others are unset
	for (size_t i = 0, size = s_vectorAccountAgeTitle.size(); i < size; ++i)
	{
		if ((i + 1) == ageInYears)
		{
			player.modifyCollectionSlotValue(s_vectorAccountAgeTitle[i]->name, 1ll);
		}
		else
		{
			player.modifyCollectionSlotValue(s_vectorAccountAgeTitle[i]->name, -1ll);
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObjectNamespace::grantGcwFactionalPresenceScore(std::string const & gcwCategory, PlayerObject const & po, CreatureObject const & co)
{
	// base_points = level * (level_% / 100)
	// bonus_% = GCW_rank * bonus_%_per_rank
	//
	// if (in a same faction player city)
	// {
	//     bonus_% += player_city_bonus_%
	//     bonus_% += (player_city_rank_bonus_% * city_rank)
	//     bonus_% += (player_city_age_bonus_% * city_age_in_years)
	// }
	//
	// final_points = base_points + (base_points * (bonus_% / 100))
	int level = co.getLevel();
	int bonus = 100 + (po.getCurrentGcwRank() * ConfigServerGame::getGcwFactionalPresenceGcwRankBonusPct());
	int mountedPct = 100;

	// for ground, give bonus if in same faction player city
	//
	// for space, give bonus if the player's citizenship city is on the
	// corresponding ground planet and is the same faction as the player
	CityInfo const * ci = nullptr;
	if (!ServerWorld::isSpaceScene())
	{
		Vector const creaturePosition = co.findPosition_w();
		int const cityId = CityInterface::getCityAtLocation(ServerWorld::getSceneId(), static_cast<int>(creaturePosition.x), static_cast<int>(creaturePosition.z), 0);
		if (cityId > 0)
			ci = &(CityInterface::getCityInfo(cityId));

		// for ground, there may be a reduction for being on a vehicle/mount
		if (co.getMountedCreature())
			mountedPct = std::min(std::max(ConfigServerGame::getGcwFactionalPresenceMountedPct(), 1), 100); // clamp to 1 - 100
	}
	else
	{
		// for space, the "level" for a pilot (i.e. character is
		// sitting in the pilot chair) is based on the pilot skill
		//
		// Pilots with one of the following skills should be considered Level 90
		//     pilot_rebel_navy_master
		//     pilot_imperial_navy_master
		//     pilot_neutral_master
		//
		// Pilots with one of the following skills should be considered Level 69
		//     pilot_rebel_navy_droid_03
		//     pilot_imperial_navy_droid_03
		//     pilot_neutral_droid_03
		//
		// Pilots with one of the following skills should be considered Level 45
		//     pilot_rebel_navy_droid_02
		//     pilot_imperial_navy_droid_02
		//     pilot_neutral_droid_02
		//
		// Pilots with one of the following skills should be considered Level 21
		//     pilot_rebel_navy_droid_01
		//     pilot_imperial_navy_droid_01
		//     pilot_neutral_droid_01
		//
		// Else they should be consider level 1
		ServerObject const * const container = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(co));
		if (container && !container->asCellObject())
		{
			static std::list<std::pair<const SkillObject *, int> > sPilotSkillToLevel;
			if (sPilotSkillToLevel.empty())
			{
				const SkillObject * skill = SkillManager::getInstance().getSkill("pilot_rebel_navy_master");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 90));

				skill = SkillManager::getInstance().getSkill("pilot_imperial_navy_master");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 90));

				skill = SkillManager::getInstance().getSkill("pilot_neutral_master");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 90));

				skill = SkillManager::getInstance().getSkill("pilot_rebel_navy_droid_03");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 69));

				skill = SkillManager::getInstance().getSkill("pilot_imperial_navy_droid_03");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 69));

				skill = SkillManager::getInstance().getSkill("pilot_neutral_droid_03");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 69));

				skill = SkillManager::getInstance().getSkill("pilot_rebel_navy_droid_02");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 45));

				skill = SkillManager::getInstance().getSkill("pilot_imperial_navy_droid_02");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 45));

				skill = SkillManager::getInstance().getSkill("pilot_neutral_droid_02");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 45));

				skill = SkillManager::getInstance().getSkill("pilot_rebel_navy_droid_01");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 21));

				skill = SkillManager::getInstance().getSkill("pilot_imperial_navy_droid_01");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 21));

				skill = SkillManager::getInstance().getSkill("pilot_neutral_droid_01");
				if (skill)
					sPilotSkillToLevel.push_back(std::make_pair(skill, 21));
			}

			level = 1;
			CreatureObject::SkillList const & creatureSkill = co.getSkillList();
			for (std::list<std::pair<const SkillObject *, int> >::const_iterator iter = sPilotSkillToLevel.begin(); iter != sPilotSkillToLevel.end(); ++iter)
			{
				if (creatureSkill.count(iter->first) >= 1)
				{
					level = iter->second;
					break;
				}
			}
		}

		std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(co.getNetworkId());
		if (!cityIds.empty())
		{
			ci = &(CityInterface::getCityInfo(cityIds.front()));

			// player city on Rori will be associated with space_naboo
			// player city on Talus will be associated with space_corellia
			std::string const & playerCityPlanet = ci->getPlanet();
			std::string playerCitySpaceZone;
			if (playerCityPlanet == std::string("rori"))
				playerCitySpaceZone = std::string("space_naboo");
			else if (playerCityPlanet == std::string("talus"))
				playerCitySpaceZone = std::string("space_corellia");
			else
				playerCitySpaceZone = std::string("space_") + playerCityPlanet;

			if (playerCitySpaceZone != ServerWorld::getSceneId())
				ci = nullptr;
		}
	}

	if (ci)
	{
		if (!PvpData::isNeutralFactionId(ci->getFaction()) && (ci->getFaction() == co.getPvpFaction()))
		{
			bonus += ConfigServerGame::getGcwFactionalPresenceAlignedCityBonusPct();

			int cityRank = 5;
			if (ci->getRadius() < 200)
				cityRank = 1;
			else if (ci->getRadius() < 300)
				cityRank = 2;
			else if (ci->getRadius() < 400)
				cityRank = 3;
			else if (ci->getRadius() < 450)
				cityRank = 4;

			bonus += (cityRank * ConfigServerGame::getGcwFactionalPresenceAlignedCityRankBonusPct());

			if (ci->getCreationTime() > 0)
				bonus += std::max(0, (static_cast<int>(::time(nullptr)) - std::max(1041408000, ci->getCreationTime())) / 31536000 * ConfigServerGame::getGcwFactionalPresenceAlignedCityAgeBonusPct());
		}
	}

	int const points = std::max(1, level * ConfigServerGame::getGcwFactionalPresenceLevelPct() * std::min(1000, bonus) * mountedPct / 1000000);

	if (PvpData::isImperialFactionId(co.getPvpFaction()))
		ServerUniverse::getInstance().adjustGcwImperialScore(std::string("FactionalPresence"), const_cast<CreatureObject *>(&co), gcwCategory, points);
	else if (PvpData::isRebelFactionId(co.getPvpFaction()))
		ServerUniverse::getInstance().adjustGcwRebelScore(std::string("FactionalPresence"), const_cast<CreatureObject *>(&co), gcwCategory, points);
}

// ======================================================================
