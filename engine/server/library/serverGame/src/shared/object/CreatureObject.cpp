//========================================================================
//
// CreatureObject.cpp
// Copyright 2001-2005, Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#pragma warning(disable : 4100 4514)
#include "serverGame/CreatureObject.h"

#include "SwgGameServer/CombatEngine.h"
#include "SwgGameServer/ConfigCombatEngine.h"
#include "UnicodeUtils.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/AttribModNameManager.h"
#include "serverGame/BiographyManager.h"
#include "serverGame/BuffManager.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/Chat.h"
#include "serverGame/CitizenInfo.h"
#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/Client.h"
#include "serverGame/CommandCppFuncs.h"
#include "serverGame/CommandQueue.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GroupMissionCriticalObjectsBuilder.h"
#include "serverGame/GroupStringId.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/GuildMemberInfo.h"
#include "serverGame/GuildObject.h"
#include "serverGame/ServerImageDesignerManager.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/MissionObject.h"
#include "serverGame/MoveSimManager.h"
#include "serverGame/NameManager.h"
#include "serverGame/NewbieTutorial.h"
#include "serverGame/NpcConversation.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PvpInternal.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ReportManager.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerMissionObjectTemplate.h"
#include "serverGame/ServerSecureTrade.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWeaponObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceVisibilityManager.h"
#include "serverGame/TriggerVolume.h"
#include "serverGame/VeteranRewardManager.h"
#include "serverGame/WeaponObject.h"
#include "serverNetworkMessages/AccountFeatureIdResponse.h"
#include "serverNetworkMessages/FeatureIdTransactionRequest.h"
#include "serverNetworkMessages/FeatureIdTransactionResponse.h"
#include "serverNetworkMessages/GroupMemberParam.h"
#include "serverNetworkMessages/LoginUpgradeAccountMessage.h"
#include "serverNetworkMessages/MessageQueueAlterAttribute.h"
#include "serverNetworkMessages/MessageQueueCommandQueueEnqueueFwd.h"
#include "serverNetworkMessages/MessageQueueSetState.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "serverNetworkMessages/TransferReplyMoveValidation.h"
#include "serverNetworkMessages/UnloadedPlayerMessage.h"
#include "serverNetworkMessages/UpdateObjectOnPlanetMessage.h"
#include "serverNetworkMessages/UploadCharacterMessage.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/JavaLibrary.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/AdminAccountManager.h"
#include "serverUtility/ServerClock.h"
#include "Session/CommonAPI/CommonAPI.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/Footprint.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/DynamicVariableListNestedList.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/AppearanceManager.h"
#include "sharedGame/AttribModArchive.h"
#include "sharedGame/Buff.h"
#include "sharedGame/CityData.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedGame/CraftingData.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedGame/CustomizationManager_MorphParameter.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/LfgDataTable.h"
#include "sharedGame/MatchMakingCharacterResult.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedGame/PlayerCreationManager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedGame/ShipSlotIdManager.h"
#include "sharedGame/SlopeEffectProperty.h"
#include "sharedGame/SlowDownProperty.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedGame/VehicleHoverDynamics.h"
#include "sharedLog/Log.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/MxCifQuadTreeBounds.h"
#include "sharedMath/Ray3d.h"
#include "sharedNetworkMessages/ClientMfdStatusUpdateMessage.h"
#include "sharedNetworkMessages/ErrorMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/GrantCommand.h"
#include "sharedNetworkMessages/MessageQueueCombatSpam.h"
#include "sharedNetworkMessages/MessageQueueCommandTimer.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
#include "sharedNetworkMessages/MessageQueueOpponentInfo.h"
#include "sharedNetworkMessages/MessageQueuePosture.h"
#include "sharedNetworkMessages/MessageQueuePushCreature.h"
#include "sharedNetworkMessages/MessageQueueSitOnObject.h"
#include "sharedNetworkMessages/MessageQueueSlowDownEffect.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedNetworkMessages/MessageQueueStringList.h"
#include "sharedNetworkMessages/RevokeCommand.h"
#include "sharedNetworkMessages/SceneChannelMessages.h"
#include "sharedNetworkMessages/SlowDownEffectMessage.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "sharedNetworkMessages/UpdatePostureMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/MovementTable.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedSkillSystem/ExpertiseManager.h"
#include "sharedSkillSystem/LevelManager.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedTerrain/WaterTypeManager.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgSharedNetworkMessages/MessageQueueCombatAction.h"
#include "swgSharedUtility/Attributes.h"
#include "swgSharedUtility/States.h"
#include "sharedUtility/ValueDictionary.h"
#include <algorithm>
#include <deque>
#include <limits>
#include <unordered_set>
#include <set>

//----------------------------------------------------------------------
// static CreatureObject vars

const SharedObjectTemplate * CreatureObject::m_defaultSharedTemplate = nullptr;

//----------------------------------------------------------------------

// Slot names
static const ConstCharCrcLowerString DATAPAD_SLOT_NAME("datapad");

// Crafting objvar names
static const std::string OBJVAR_DISABLE_SCHEMATIC_FILTER("crafting.disableSchematicFiltering");

//objvar for banking
static const std::string OBJVAR_BANK_ID("banking_bankid");
static const std::string OBJVAR_OPEN_BANK_TERMINAL_ID("open_bank_location");

//slot names
static const ConstCharCrcLowerString BANK_SLOT_NAME("bank");
static const ConstCharCrcLowerString MISSION_BAG_SLOT_NAME("mission_bag");

// skill data granted independently from skills
static const std::string OBJVAR_NOT_SKILL_MODS("_notskill.mods");
static const std::string OBJVAR_NOT_SKILL_COMMANDS("_notskill.commands");
static const std::string OBJVAR_NOT_SKILL_SCHEMATICS("_notskill.schematics");

// special skill mods
static const std::string SLOPE_MOD("slope_move");
static const std::string GROUP_SLOPE_MOD("group_slope_move");

// jedi/force power objvars
static const std::string OBJVAR_NEW_JEDI_FORCE("_start_force");
static const std::string OBJVAR_JEDI("jedi");
static const std::string OBJVAR_ADD_JEDI_ACK("jediAddedAcknowlege");
//static const std::string OBJVAR_FORCE_REGEN_RATE("jedi.regenrate");
//static const std::string OBJVAR_FORCE_REGEN_VALUE("jedi.regenvalue");
static const std::string OBJVAR_JEDI_VISIBILITY("jedi.visibility");

// deprecated badge objvars (will be used to move old badges over to the collection system)
static const std::string OBJVAR_BADGES("badge.tracking");
static const std::string OBJVAR_COUNT_BADGES("badge.count");

// residence house id objvar
const std::string OBJVAR_PLAYER_HOUSE_ID("residenceHouseId");

// If a pet accepts a group invitation but is deleted before
// being given the group information, we need to try to remove
// him from the group in the future
static const unsigned MAX_ATTEMPTS_TO_REMOVE_PET_FROM_GROUP = 30;
static const int      TIME_BETWEEN_ATTEMPTS_TO_REMOVE_PET   = 1;

// used to determine if LOS-checks should be skipped during CreatureObject::canManipulateObject
static const std::string OBJVAR_SKIP_LOS_CHECK("skipLosCheck");

// creature regen constants
const static std::string CREATURES_TABLE("datatables/mob/creatures.iff");

//----------------------------------------------------------------------

namespace CreatureObjectNamespace
{
	const int POINTS_PER_TIER = 4;
	// ----------------------------------------------------------------------

	const char* s_inventoryTemplate  = "object/tangible/inventory/character_inventory.iff";
	const char* s_datapadTemplate    = "object/tangible/datapad/character_datapad.iff";
	const char* s_missionBagTemplate = "object/tangible/mission_bag/mission_bag.iff";
	const char* s_bankTemplate       = "object/tangible/bank/character_bank.iff";
	const char* s_appearanceTemplate = "object/tangible/inventory/appearance_inventory.iff";

	SlotId      s_defaultWeaponSlotId    = SlotId::invalid;
	SlotId      s_inventorySlotId        = SlotId::invalid;
	SlotId      s_appearanceSlotId       = SlotId::invalid;
	bool        s_allowNullDefaultWeapon = false;

	// ----------------------------------------------------------------------

	std::map<std::string, std::pair<int, int> > s_instrumentTypeMap;

	// ----------------------------------------------------------------------

	void loadInstrumentTable()
	{
		char const *fileName = ConfigServerGame::getInstrumentDataTableFilename();
		DataTable *t = DataTableManager::getTable(fileName, true);
		if (!t)
		{
			FATAL(true, ("Could not find instrument datatable %s.", fileName));
		}
		for (int row = 0; row < t->getNumRows(); ++row)
		{
			std::string name = t->getStringValue("serverTemplateName", row);
			int visualId = t->getIntValue("instrumentVisualId", row);
			int audioId = t->getIntValue("instrumentAudioId", row);
			IGNORE_RETURN(s_instrumentTypeMap.insert(std::make_pair(name, std::make_pair(visualId, audioId))));
		}
		DataTableManager::close(fileName);
	}

	// ----------------------------------------------------------------------

	int lookupInstrumentVisualId(char const * serverTemplateName)
	{
		std::map<std::string, std::pair<int, int> >::const_iterator i = s_instrumentTypeMap.find(std::string(serverTemplateName));
		if (i != s_instrumentTypeMap.end())
			return (*i).second.first;
		return 0;
	}

	// ----------------------------------------------------------------------

	int lookupInstrumentAudioId(char const * serverTemplateName)
	{
		std::map<std::string, std::pair<int, int> >::const_iterator i = s_instrumentTypeMap.find(std::string(serverTemplateName));
		if (i != s_instrumentTypeMap.end())
			return (*i).second.second;
		return 0;
	}

	// ----------------------------------------------------------------------

	std::map<int, int> s_danceVisualMap;

	// ----------------------------------------------------------------------

	void loadDanceVisualTable()
	{
		char const *fileName = ConfigServerGame::getPerformanceDataTableFilename();
		DataTable *t = DataTableManager::getTable(fileName, true);
		for (int row = 0; row < t->getNumRows(); ++row)
		{
			int visualId = t->getIntValue("danceVisualId", row);
			if (visualId)
			{
				IGNORE_RETURN(s_danceVisualMap.insert(std::make_pair(row+1, visualId)));
			}
		}
		DataTableManager::close(fileName);
	}

	struct CommandSeriesRecord
	{
		std::string commandName;
		std::string baseCommandName;
		int level;
	};
	std::vector<CommandSeriesRecord> s_commandSeriesRecords;

	void loadCommandSeriesTable()
	{
		DataTable *t = DataTableManager::getTable("datatables/command/command_series.iff", true);
		for (int row = 0; row < t->getNumRows(); ++row)
		{
			CommandSeriesRecord csr;
			csr.commandName = t->getStringValue("commandName", row);
			csr.baseCommandName = t->getStringValue("baseCommandName", row);
			csr.level = t->getIntValue("level", row);
			IGNORE_RETURN(s_commandSeriesRecords.push_back(csr));			
		}
		DataTableManager::close("datatables/command/command_series.iff");
	}

	// ----------------------------------------------------------------------

	int lookupDanceVisual(int performanceType)
	{
		std::map<int, int>::const_iterator i = s_danceVisualMap.find(performanceType);
		if (i != s_danceVisualMap.end())
			return (*i).second;
		return 0;
	}

	// ----------------------------------------------------------------------

	bool instrumentTooFar(Vector const &playerPos_w, Vector const &instrumentPos_w)
	{
		static float maxDistSquared = sqr(ConfigServerGame::getMaxInstrumentPlayDistance());
		if (playerPos_w.magnitudeBetweenSquared(instrumentPos_w) < maxDistSquared)
			return false;
		return true;
	}

	// ----------------------------------------------------------------------

	CreatureObject::AllCreaturesSet g_creatureList;

	std::vector<std::pair<CreatureObject *, NetworkId> > gs_missionRequestQueue;
	std::vector<std::pair<CreatureObject *, NetworkId> > gs_missionRequestQueueDeferredRemoves;

	void removeCreatureFromMissionRequestQueue(const CreatureObject * creature)
	{
		std::vector<std::pair<CreatureObject *, NetworkId> >::iterator qiter;
		for(qiter = gs_missionRequestQueue.begin(); qiter != gs_missionRequestQueue.end(); ++qiter)
		{
			if((*qiter).first == creature)
			{
				gs_missionRequestQueueDeferredRemoves.push_back((*qiter));
			}
		}
	}

	std::deque<NetworkId> s_spawnQueue;
	std::deque<NetworkId> s_missionQueue;

	//----------------------------------------------------------------------
	//- @odo: these lists really need to be moved into C++

	bool isBuildingAdminForTarget (const ServerObject & object, const ServerObject & target)
	{

		const TangibleObject* hopperObject = target.asTangibleObject();
		const CreatureObject* player = object.asCreatureObject();
		if (!player || !hopperObject)
			return false;
		return hopperObject->isOnAdminList(*player);
	}

	//----------------------------------------------------------------------

	bool canHopperForTarget (const ServerObject & object, const ServerObject & target)
	{
		const TangibleObject* hopperObject = target.asTangibleObject();
		const CreatureObject* player = object.asCreatureObject();
		if (!player || !hopperObject)
			return false;
		return hopperObject->isOnHopperList(*player);
	}

	//----------------------------------------------------------------------

	bool isTargetBuildingBaseObject (const ServerObject & buildingObject, const ServerObject & target)
	{
		static const std::string objvarname_baseObjects = "player_structure.base_objects";

		const NetworkId & targetId = target.getNetworkId ();

		typedef std::vector<NetworkId> NetworkIdVector;
		static NetworkIdVector ids;
		ids.clear ();

		if (buildingObject.getObjVars ().getItem (objvarname_baseObjects, ids))
		{
			for (NetworkIdVector::const_iterator it = ids.begin (); it != ids.end (); ++it)
			{
				const NetworkId & id = *it;
				if (id == targetId)
					return true;
			}
		}

		return false;
	}

	bool isOnVendorList(const CreatureObject& object, const ServerObject& target)
	{
		static const std::string objvarname_vendorList = "player_structure.vendor.vendorList";

		const Unicode::String & requestorFirstName = object.getAssignedObjectFirstName ();
		typedef std::vector<Unicode::String> StringVector;
		static StringVector sv;
		sv.clear ();

		if (target.getObjVars ().getItem (objvarname_vendorList, sv))
		{
			for (StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
			{
				const Unicode::String & vendorFirstName = *it;
				if (Unicode::caseInsensitiveCompare (vendorFirstName, requestorFirstName))
				{
					return true;
				}
			}
		}
		return false;
	}

	//----------------------------------------------------------------------

	namespace SlotNames
	{
		static const ConstCharCrcLowerString datapad   ("datapad");
		static const ConstCharCrcLowerString inventory ("inventory");
		static const ConstCharCrcLowerString bank      ("bank");
		static const ConstCharCrcLowerString hair      ("hair");
		static const ConstCharCrcLowerString ghost     ("ghost");
		static const ConstCharCrcLowerString appearance ("appearance_inventory");
		static const ConstCharCrcLowerString hangar    ("hangar");
	}

	uint64 s_containmentStatesMask = 0;
	std::map<SlotId, uint64> s_containerSlotStates;

	namespace GroupHelpers
	{
		bool creatureIsContainedInPOBShip(CreatureObject const * creatureObject);
		void findAllTargetsForGroup(CreatureObject * const targetObj, std::vector<CreatureObject *> & targets);
		bool roomInGroup(GroupObject const * groupObj, int additionalMembers);
		GroupMemberParam const buildGroupMemberParam(CreatureObject const * creatureObject);
		void buildGroupMemberParamsFromCreatures(std::vector<CreatureObject *> const & targets, GroupObject::GroupMemberParamVector & targetMemberParams);
	}

	// ----------------------------------------------------------------------

	void skipField(std::string const &source, unsigned int &pos)
	{
		while (source[pos] && source[pos] != '~')
			++pos;
		if (source[pos] == '~')
			++pos;
	}

	// ----------------------------------------------------------------------

	void nextString(std::string const &source, unsigned int &pos, std::string &ret)
	{
		unsigned int oldPos = pos;
		while (source[pos] && source[pos] != '~')
			++pos;
		ret = source.substr(oldPos, pos-oldPos);
		if (source[pos] == '~')
			++pos;
	}

	// ----------------------------------------------------------------------

	void nextOid(std::string const &source, unsigned int &pos, NetworkId &ret)
	{
		unsigned int oldPos = pos;
		skipField(source, pos);
		ret = NetworkId(source.substr(oldPos, pos-oldPos));
	}

	// ----------------------------------------------------------------------

	void nextFloat(std::string const &source, unsigned int &pos, float &ret)
	{
		ret = static_cast<float>(atof(source.c_str()+pos));
		skipField(source, pos);
	}

	// for enforcing height restriction when restoring decoration layout
	std::map<std::pair<uint32, uint32>, std::pair<float, float> > s_mapCellHeightRestriction;

	void restoreItemDecorationLayout(CreatureObject & decorator, PlayerObject & decoratorPlayerObject, CachedNetworkId const & item, TangibleObject const & pob, CachedNetworkId const & cell, Vector const & position, Quaternion const & orientation);
}

//=======================================================================

using namespace CreatureObjectNamespace;

//=======================================================================

void CreatureObject::PostureChangeCallback::modified(CreatureObject &target, Postures::Enumerator oldValue, Postures::Enumerator value, bool) const
{
	UNREF(oldValue);
	// Make creatures not collidable when they're in dead or incapacitated posture
	CollisionProperty * collision = target.getCollisionProperty();
	if (collision)
	{
		bool collidable = (value != Postures::Dead && value != Postures::Incapacitated);
		collision->setCollidable(collidable);
	}

	//-- Retrieve the collection of avialable locomotions for fast/slow/stationary movement speeds.
	//   This info is used on authoritative and proxy objects to make calcLocomotions(), called every alter,
	//   considerably more efficient.
	target.getAvailableLocomotions();
	if (value == Postures::Dead)
		Pvp::handleCreatureDied(target);
}

//=======================================================================

void CreatureObject::LocomotionChangeCallback::modified(CreatureObject &target, Locomotions::Enumerator oldValue, Locomotions::Enumerator value, bool) const
{
	ScriptParams params;
	params.addParam(value);
	params.addParam(oldValue);

	if (target.isAuthoritative())
	{
		GameScriptObject * const script = target.getScriptObject();

		if (script != 0)
		{
			script->trigAllScripts(Scripting::TRIG_LOCOMOTION_CHANGED, params);
		}
	}
}

// ======================================================================

void CreatureObject::StateChangeCallback::modified(CreatureObject &target, uint64 oldValue, uint64 newValue, bool isLocal) const
{
	UNREF(newValue);
	UNREF(isLocal);
	UNREF(oldValue);
	//-- Retrieve the collection of avialable locomotions for fast/slow/stationary movement speeds.
	//   This info is used on authoritative and proxy objects to make calcLocomotions(), called every alter,
	//   considerably more efficient.
	target.getAvailableLocomotions();
}

// ======================================================================

CreatureAttitude::CreatureAttitude() :
	m_behavior(Behaviors::Invalid),
	m_angerState(kAngerCalm),
	m_fearState(kFearCalm)
{
	for (int i = 0; i <MentalStates::NumberOfMentalStates; ++i)
		m_currentValues[i] = 0;
}

// ----------------------------------------------------------------------

bool CreatureAttitude::operator!=(CreatureAttitude const &other) const
{
	if (   m_behavior != other.m_behavior
	    || m_angerState != other.m_angerState
	    || m_fearState != other.m_fearState)
		return true;
	for (int i = 0; i < MentalStates::NumberOfMentalStates; ++i)
		if (m_currentValues[i] != other.m_currentValues[i])
			return true;
	return false;
}

// ----------------------------------------------------------------------

float CreatureAttitude::operator-(CreatureAttitude const &other) const
{
	float result = 0.0f;
	if (m_behavior != other.m_behavior)
		result = 30.0f;
	if (m_angerState != other.m_angerState)
		result += 30.0f;
	if (m_fearState != other.m_fearState)
		result += 30.0f;
	for (int i = 0; i < MentalStates::NumberOfMentalStates; ++i)
		result += std::abs(m_currentValues[i] - other.m_currentValues[i]);
	return result;
}

// ======================================================================

MonitoredCreatureMovement::MonitoredCreatureMovement() :
	m_lastDistance(0),
	m_skittishness(0),
	m_curve(0)
{
}

// ----------------------------------------------------------------------

bool MonitoredCreatureMovement::operator!=(MonitoredCreatureMovement const &other) const
{
	return    m_lastDistance != other.m_lastDistance
	       || m_skittishness != other.m_skittishness
	       || m_curve != other.m_curve;
}

// ----------------------------------------------------------------------

float MonitoredCreatureMovement::operator-(MonitoredCreatureMovement const &other) const
{
	return   std::abs(m_lastDistance - other.m_lastDistance)
	       + std::abs(m_skittishness - other.m_skittishness)
	       + std::abs(m_curve - other.m_curve);
}

// ======================================================================

CreatureObject::CreatureObject(const ServerCreatureObjectTemplate* newTemplate) :
	TangibleObject(newTemplate),
	m_commandQueue(nullptr),
	m_isStatic(false),
	m_shield(nullptr),
	m_regenerationTime(0),
	m_attributes(Attributes::NumberOfAttributes),
	m_maxAttributes(Attributes::NumberOfAttributes),
	m_totalAttributes(Attributes::NumberOfAttributes),
	m_totalMaxAttributes(Attributes::NumberOfAttributes),
	m_attribBonus(Attributes::NumberOfAttributes),
	m_shockWounds(0),
	m_attributeModList(),
	m_cachedCurrentAttributeModValues(Attributes::NumberOfAttributes),
	m_cachedMaxAttributeModValues(Attributes::NumberOfAttributes),
	m_currentAttitude(),
	m_lastBehavior(),
	m_mentalStatesToward(),
	m_maxMentalStates(),
	m_mentalStateDecays(),
	m_scaleFactor(1.0f),
	m_monitoredCreatureMovements(),
	m_guildId(0),
	m_timeToUpdateGuildWarPvpStatus(0),
	m_guildWarEnabled(false),
	m_militiaOfCityId(),
	m_locatedInCityId(),
	m_masterId(),
	m_rank(0),
	m_wsX(0.0),
	m_wsY(0.0),
	m_wsZ(0.0),
//	m_animState(CreatureObject::AS_idle),
	m_mood(0),
	m_sayMode(0),
	m_animationMood("neutral"),
	m_posture(Postures::Upright),
	m_stopWalkRun(2),
	m_states(0),
	m_movementScale(1.0f),
	m_movementPercent(1.0f),
	m_baseWalkSpeed        (-1.0f),
	m_walkSpeed            (safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSpeed(SharedCreatureObjectTemplate::MT_walk)),
	m_baseRunSpeed         (-1.0f),
	m_runSpeed             (safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSpeed(SharedCreatureObjectTemplate::MT_run)),
	m_accelScale           (1.0f),
	m_accelPercent         (1.0f),
	m_turnScale            (1.0f),
	m_slopeModAngle        (safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSlopeModAngle() * PI / 180),
	m_baseSlopeModPercent  (safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSlopeModPercent()),
	m_slopeModPercent      (1.0f),
	m_waterModPercent      (safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getWaterModPercent()),
	m_lookAtTarget                (),
	m_intendedTarget              (),
	m_cover                       (0),
	m_coverVisibility             (true),
	m_currentWeapon               (),
	m_modMap                      (),
	m_skills                      (),
	m_deferComputeTotalAttributes (0),
	m_group                       (),
	m_groupInviter                (),
	m_inviterForPendingGroup      (),
	m_performanceType             (),
	m_performanceStartTime        (),
	m_animatingSkillData          (),
	m_level                       (-1),
	m_previousLevel               (-1),
	m_totalLevelXp                (0),
	m_levelHealthGranted          (0),
	m_invulnerabilityTimer        (),
	m_alternateAppearanceSharedObjectTemplateName (),
	m_clientMfdStatusUpdateMessage(0),
	m_timedMod                    (),
	m_timedModDuration            (),
	m_timedModUpdateTime          (),
	m_addedToSpawnQueue           (false),
	m_numberOfMissionsWantedInMissionBag(0),
	m_fastLocomotion              (Locomotions::Invalid),
	m_slowLocomotion              (Locomotions::Invalid),
	m_stationaryLocomotion        (Locomotions::Invalid),
	m_vehiclePhysicsData          (0),
	m_missionCriticalObjectSet(),
	m_groupMissionCriticalObjectSet(),
	m_triggerVolumeEntered(),
	m_pseudoPlayedTime(0.0f),
	m_buffs(),
	m_persistedBuffs(),
	m_clientUsesAnimationLocomotion(false),
	m_difficulty(static_cast<unsigned char>(D_normal)),
	m_hologramType(-1),
	m_visibleOnMapAndRadar(true),
	m_lookAtYaw(0.0f),
	m_useLookAtYaw(false),
	m_fixedupPersistentBuffsAfterLoading(false),
	m_fixedupLevelXpAfterLoading(false),
	m_lavaResistance(0.0f),
	m_lastWaterDamageTime(0),
	m_isBeast(false),
	m_forceShowHam(false),
	m_regionFlagTimer(ConfigServerGame::getRegionFlagUpdateTimeSecs()),
	m_wearableAppearanceData(),
	m_decoyOrigin(NetworkId::cms_invalid)
{
	addProperty(*(new CommandQueue(*this)));

	// get the shared object template
	const SharedCreatureObjectTemplate * sharedTemplate = safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate());
	NOT_NULL(sharedTemplate);

	// Handle scale.
	// setScaleFactor(safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getScale());
	// Cannot set scale with setScaleFactor() since this object isn't authoratative yet.  Expect many
	// bugs from the lack of authority setting within a constructor.
	const float scale = std::max (0.0f, sharedTemplate->getScale());
	m_scaleFactor = scale;
	setScale (Vector::xyz111 * scale);

	// get the attribute values
	{
		for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
		{
			m_attributes.set(i, static_cast<Attributes::Value>(
				newTemplate->getAttributes(static_cast<
				ServerCreatureObjectTemplate::Attributes>(i))));
			m_maxAttributes.set(i, m_attributes.get(i));
			m_cachedCurrentAttributeModValues.set(i, 0);
			m_cachedMaxAttributeModValues.set(i, 0);
			m_regeneration[i] = 0;
		}
		// store the default regen values in the former faucet attrib slots
		m_attributes.set(Attributes::Constitution, static_cast<int>(ConfigServerGame::getDefaultHealthRegen()));
		m_attributes.set(Attributes::Stamina, static_cast<int>(ConfigServerGame::getDefaultActionRegen()));
		m_attributes.set(Attributes::Willpower, static_cast<int>(ConfigServerGame::getDefaultMindRegen()));
	}

	// fix the size of the mental state vectors
	m_maxMentalStates.resize(MentalStates::NumberOfMentalStates,      0);
	m_mentalStateDecays.resize(MentalStates::NumberOfMentalStates,    0);

	CreatureAttitude initialState;

	// get the mental state values
	{
		for (int i = 0; i < MentalStates::NumberOfMentalStates; ++i)
		{
			const ServerCreatureObjectTemplate::MentalStates state = static_cast<ServerCreatureObjectTemplate::MentalStates>(i);

			m_maxMentalStates.set(i, static_cast<MentalStates::Value>(newTemplate->getMaxMentalStates(state)));
			initialState.m_currentValues[i] = 0;
			m_mentalStateDecays.set(i, float(newTemplate->getMentalStatesDecay(state)));
		}
	}

	m_currentAttitude.set(initialState);
	m_lastBehavior.set(-1);

	m_fearCutoffs[CreatureAttitude::kFearCalm] = 0;
	m_fearCutoffs[CreatureAttitude::kFearAlert] = 25;
	m_fearCutoffs[CreatureAttitude::kFearThreaten] = 50;
	m_fearCutoffs[CreatureAttitude::kFearFlee] = 75;
	m_fearCutoffs[CreatureAttitude::kFearPanic] = 100;

	m_angerCutoffs[CreatureAttitude::kAngerCalm] = 0;
	m_angerCutoffs[CreatureAttitude::kAngerAttack] = 60;
	m_angerCutoffs[CreatureAttitude::kAngerFrenzy] = 100;

	CollisionProperty * pCollision = getCollisionProperty();

	if(pCollision)
	{
		pCollision->setServerSide(true);
	}

	addMembersToPackages();

	m_guildId.setSourceObject(this);
	m_guildWarEnabled.setSourceObject(this);
	m_militiaOfCityId.setSourceObject(this);
	m_locatedInCityId.setSourceObject(this);
	m_group.setSourceObject(this);
	m_posture.setSourceObject(this);
	m_locomotion.setSourceObject(this);
	m_invulnerabilityTimer.setSourceObject(this);

	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		CustomizationData *const cd = fetchCustomizationData();
		if (cd)
		{
			VehicleHoverDynamics::setInitialParams (*cd,
				safe_cast<const SharedCreatureObjectTemplate*> (getSharedTemplate ())->getSpeedMin        (SharedCreatureObjectTemplate::MT_walk),
				safe_cast<const SharedCreatureObjectTemplate*> (getSharedTemplate ())->getSpeed           (SharedCreatureObjectTemplate::MT_run),
				safe_cast<const SharedCreatureObjectTemplate*> (getSharedTemplate ())->getTurnRate        (SharedCreatureObjectTemplate::MT_walk),
				safe_cast<const SharedCreatureObjectTemplate*> (getSharedTemplate ())->getTurnRate        (SharedCreatureObjectTemplate::MT_run),
				safe_cast<const SharedCreatureObjectTemplate*> (getSharedTemplate ())->getAccelerationMax (SharedCreatureObjectTemplate::MT_walk),
				safe_cast<const SharedCreatureObjectTemplate*> (getSharedTemplate ())->getAccelerationMax (SharedCreatureObjectTemplate::MT_run));

			cd->release ();
		}
	}

	m_lavaResistance = WaterTypeManager::getLavaResistance(getObjectTemplateName());

	IGNORE_RETURN(g_creatureList.insert(this));

	ObjectTracker::addCreature();

	m_maxHousingLots = ConfigServerGame::getMaxHousingLots();
}

//-----------------------------------------------------------------------

CreatureObject::~CreatureObject()
{
	//-- This must be the first line in the destructor to invalidate any watchers watching this object
	nullWatchers();

	if (m_vehiclePhysicsData)
	{
		delete m_vehiclePhysicsData;
		m_vehiclePhysicsData = 0;
	}

	if (isAuthoritative())
	{
		GroupObject *group = getGroup();
		if (group)
		{
			group->removeGroupMember(getNetworkId());
		}
		else
		{
			// If we are a pet...
			if ( getMasterId() != NetworkId::cms_invalid )
			{
				// We need to clean up outstanding group invitations since
				// there are race conditions with ServerUniverse group
				// objects and us getting added to them

				// Check for an outstanding group formation
				const NetworkId groupInviterNetworkId = getInviterForPendingGroup();
				if ( groupInviterNetworkId != NetworkId::cms_invalid )
				{
					// Send a message to the group inviter to tell them to remove
					// us from the group...

					// We need to tell the network ID of the pet and we need to keep
					// of how many times this message has been sent to the inviter
					char msgBuffer[100];
					sprintf( msgBuffer, "%s 1", getNetworkId().getValueString().c_str() );

					// Send the message with a delay
					if ( MessageToQueue::isInstalled() )
					{
						MessageToQueue::getInstance().sendMessageToC( groupInviterNetworkId,
																													"C++RemovePetFromGroup",
																													msgBuffer,
																													TIME_BETWEEN_ATTEMPTS_TO_REMOVE_PET,
																													false );  // reliable
					}
				}
			}
		}

		if (getGuildId())
			GuildInterface::onAboutToClearGuildId(*this);
	}

	if (isPlayerControlled())
		removeFromSpawnQueue();

	removeCreatureFromMissionRequestQueue(this);
	if (isInWorld())
		removeFromWorld();

	Controller *controller = getController();
	if (controller)
	{
		CreatureController *creatureController = safe_cast<CreatureController*>(controller);
		ServerSecureTrade *trade = creatureController->getSecureTrade();
		if (trade)
			trade->cancelTrade(*this);
	}
//	AICreatureController * aiController = AICreatureController::asAiCreatureController(controller);
//	if (aiController != nullptr)
//	{
//		aiController->stop();
//	}

	IGNORE_RETURN(g_creatureList.erase(this));

	delete m_clientMfdStatusUpdateMessage;

	// We should have have been removed from all TriggerVolumes by
	// now, but if we haven't, we need to tell any TriggerVolumes
	// that we haven't been removed from to remove us from their
	// contents list, or else those TriggerVolumes will have a
	// pointer to a non-existent object
	for (std::set<TriggerVolume *>::const_iterator i = m_triggerVolumeEntered.begin(); i != m_triggerVolumeEntered.end(); ++i)
	{
		WARNING(true, ("CreatureObject (%s) being destroyed but hasn't been removed from TriggerVolume (%s) owned by (%s)",
			getDebugInformation().c_str(),
			(*i)->getName().c_str(),
			(*i)->getOwner().getDebugInformation().c_str()));

		(*i)->objectDestroyed(*this);
	}

	ObjectTracker::removeCreature();
}


//-----------------------------------------------------------------------

CreatureObject * CreatureObject::asCreatureObject()
{
	return this;
}

//-----------------------------------------------------------------------

CreatureObject const * CreatureObject::asCreatureObject() const
{
	return this;
}

//-----------------------------------------------------------------------

void CreatureObject::install()
{
	loadDanceVisualTable();
	loadInstrumentTable();
	loadCommandSeriesTable();

	installForMounts();

	s_defaultWeaponSlotId = SlotIdManager::findSlotId(ConstCharCrcLowerString("default_weapon"));
	FATAL(s_defaultWeaponSlotId == SlotId::invalid, ("failed to find SlotId for default_weapon: the slot definition file was improperly loaded or modified."));

	s_inventorySlotId = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::inventory);
	FATAL(s_inventorySlotId == SlotId::invalid, ("failed to find SlotId for inventory: the slot definition file was improperly loaded or modified."));

	s_appearanceSlotId = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::appearance);
	FATAL(s_appearanceSlotId == SlotId::invalid, ("Failed to find SlotId for appearance: the slot definition file was improperly loaded or modified."));

	s_containerSlotStates[ShipSlotIdManager::getShipPilotSlotId()]      = States::getStateMask(States::PilotingShip);
	s_containerSlotStates[ShipSlotIdManager::getPobShipPilotSlotId()]   = States::getStateMask(States::PilotingPobShip);
	s_containerSlotStates[ShipSlotIdManager::getShipOperationsSlotId()] = States::getStateMask(States::ShipOperations);
	s_containerSlotStates[ShipSlotIdManager::getPobShipOperationsSlotId()] = States::getStateMask(States::ShipOperations);

	{
		// corresponds to slots in CreatureObject_Mounts.cpp
		char slotName[256];
		s_containerSlotStates[SlotIdManager::findSlotId(ConstCharCrcLowerString("rider"))] = States::getStateMask(States::RidingMount);
		for (int i = 1; i < 8; ++i)
		{
			snprintf(slotName, 255, "rider%d", i);
			SlotId const slot = SlotIdManager::findSlotId(ConstCharCrcLowerString(slotName));
			s_containerSlotStates[slot] = States::getStateMask(States::RidingMount);
		}
	}

	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		s_containerSlotStates[ShipSlotIdManager::getShipGunnerSlotId(weaponIndex)] = States::getStateMask(States::ShipGunner);
		s_containerSlotStates[ShipSlotIdManager::getPobShipGunnerSlotId(weaponIndex)] = States::getStateMask(States::ShipGunner);
	}

	s_containmentStatesMask = States::getStateMask(States::ShipInterior);
	for (std::map<SlotId, uint64>::const_iterator i = s_containerSlotStates.begin(); i != s_containerSlotStates.end(); ++i)
		s_containmentStatesMask |= (*i).second;

	// for enforcing height restriction when restoring decoration layout
	{
		char const * const cellHeightDataTableName = "datatables/structure/cell_height.iff";
		DataTable * table = DataTableManager::getTable(cellHeightDataTableName, true);
		if (table)
		{
			int const columnStructure = table->findColumnNumber("template");
			int const columnCell = table->findColumnNumber("cell");
			int const columnMinHeight = table->findColumnNumber("min_height");
			int const columnMaxHeight = table->findColumnNumber("max_height");
			if ((columnStructure >= 0) && (columnCell >= 0) && (columnMinHeight >= 0) && (columnMaxHeight >= 0))
			{
				std::string structure;
				std::string cell;

				for (int i = 0, numRows = table->getNumRows(); i < numRows; ++i)
				{
					structure = table->getStringValue(columnStructure, i);
					if (structure.empty())
						continue;

					cell = table->getStringValue(columnCell, i);
					if (cell.empty())
						continue;

					s_mapCellHeightRestriction[std::make_pair(CrcLowerString::calculateCrc(structure.c_str()), CrcLowerString::calculateCrc(cell.c_str()))] = std::make_pair(table->getFloatValue(columnMinHeight, i), table->getFloatValue(columnMaxHeight, i));
				}
			}

			DataTableManager::close(cellHeightDataTableName);
		}
	}

	ExitChain::add(CreatureObject::remove, "CreatureObject::remove");
}

//-----------------------------------------------------------------------

void CreatureObject::remove()
{
}

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * CreatureObject::getDefaultSharedTemplate() const
{
static const ConstCharCrcLowerString templateName("object/creature/base/shared_creature_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));

		DEBUG_WARNING(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
			
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "CreatureObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// CreatureObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void CreatureObject::removeDefaultTemplate()
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// CreatureObject::removeDefaultTemplate

//-----------------------------------------------------------------------

bool CreatureObject::isSpawningAllowed() const
{
	ServerObject const * container = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));

	return    isAuthoritative()
	       && getScriptObject()->isScriptListInitialized()
	       &&
	       (
	        	!container || container->asCreatureObject()
	       )
	       && (   ConfigServerGame::getBoundarySpawningAllowed()
	           || getExposedProxyList().empty())
	       && (   ConfigServerGame::getEnableSpawningNearLoadBeacons()
	           || !ServerWorld::isInLoadBeaconRange(getPosition_w()));
}

//-----------------------------------------------------------------------

void CreatureObject::addToSpawnQueue()
{
	PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::addToSpawnQueue");

	if (m_addedToSpawnQueue || !isSpawningAllowed())
		return;

	DEBUG_FATAL(std::find(s_spawnQueue.begin(), s_spawnQueue.end(), getNetworkId()) != s_spawnQueue.end(), ("spawn queue is inconsistant with m_addedToSpawnQueue flag"));

	s_spawnQueue.push_back(getNetworkId());

	m_addedToSpawnQueue = true;
}

//-----------------------------------------------------------------------

void CreatureObject::removeFromSpawnQueue()
{
	if (m_addedToSpawnQueue)
	{
		std::deque<NetworkId>::iterator f = std::find(s_spawnQueue.begin(), s_spawnQueue.end(), getNetworkId());
		if (f != s_spawnQueue.end())
			s_spawnQueue.erase(f);
		m_addedToSpawnQueue = false;
	}
}

//-----------------------------------------------------------------------

void CreatureObject::runSpawnQueue()
{
	static int framesPerSpawn = ConfigServerGame::getFramesPerSpawn();
	static int64 lastSpawnFrame = 0;
	static int64 currentSpawnFrame = 0;

	++currentSpawnFrame;

	if (currentSpawnFrame - lastSpawnFrame >= framesPerSpawn)
	{
		int i = 1;
		if (framesPerSpawn == 0)
			i = ConfigServerGame::getSpawnQueueSize();

		while (i > 0 && !s_spawnQueue.empty())
		{
			NetworkId const id = s_spawnQueue.front();
			s_spawnQueue.pop_front();

			// Only spawn if we can resolve the object, and it believes spawning
			// should be allowed.  If we can resolve the object but spawning is not
			// allowed, do not re-add it to the queue and mark it as such.

			CreatureObject * const spawner = safe_cast<CreatureObject *>(NetworkIdManager::getObjectById(id));

			if (spawner)
			{
				if (spawner->isSpawningAllowed())
				{
					ScriptParams params;
					ScriptDictionaryPtr dictionary;
					GameScriptObject::makeScriptDictionary(params, dictionary);
					if (dictionary.get() != nullptr)
					{
						dictionary->serialize();
						MessageToQueue::getInstance().sendMessageToJava(id, "spawn_Trigger", dictionary->getSerializedData(), 0, false);
					}
					s_spawnQueue.push_back(id);
				}
				else
					spawner->m_addedToSpawnQueue = false;
			}
			--i;
		}
		lastSpawnFrame = currentSpawnFrame;
	}
}

//-----------------------------------------------------------------------

void CreatureObject::showSpawnQueue(Unicode::String & result)
{
	for (std::deque<NetworkId>::iterator i = s_spawnQueue.begin(); i != s_spawnQueue.end(); ++i)
	{
		ServerObject const * const obj = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById((*i)));
		if (obj)
		{
			result += Unicode::narrowToWide((*i).getValueString());
			result += Unicode::narrowToWide(" - ");
			result += obj->getObjectName();
			result += Unicode::narrowToWide("\n");
		}
	}
}

//-----------------------------------------------------------------------
/**
*/
bool CreatureObject::assignMission(MissionObject * missionObject)
{
	bool result = false;
	if(missionObject)
	{
		ServerObject * datapad = getDatapad();
		if(datapad)
		{
			// if this mission is coming from our own mission bag, increment missions wanted
			ServerObject * missionBag = getMissionBag();
			if(missionBag)
			{
				Container * container = ContainerInterface::getContainer(*missionBag);
				ContainerIterator iter;
				bool found = false;
				int i = 0;
				for(iter = container->begin(); iter != container->end(); ++iter)
				{
					MissionObject * mo = safe_cast<MissionObject *>((*iter).getObject());
					if(mo)
					{
						i++;
						if(mo == missionObject)
						{
							found = true;
						}
					}
				}
				if(found)
				{
					setNumberOfMissionsWantedInMissionBag(i + 1);
				}
			}

			Container::ContainerErrorCode tmp = Container::CEC_Success;
			result = ContainerInterface::transferItemToGeneralContainer(*datapad, *missionObject, nullptr, tmp);
			if(result)
			{
				missionObject->setMissionHolderId(getNetworkId());

			}
		}
	}
	return result;
}

//-----------------------------------------------------------------------

void CreatureObject::setMissionBoardUISequenceId(const uint8 sequenceId)
{
	m_missionListRequestSequenceId = sequenceId;
}

//-----------------------------------------------------------------------

std::vector<CachedNetworkId> CreatureObject::getMissions()
{
	std::vector<CachedNetworkId> result;
	ServerObject * const datapad = getDatapad();
	if (datapad)
	{
		Container * const container = ContainerInterface::getContainer(*datapad);
		if (container)
		{
			for (ContainerIterator iter = container->begin(); iter != container->end(); ++iter)
			{
				ServerObject * const serverMissionObject = safe_cast<ServerObject *>((*iter).getObject());
				if (serverMissionObject)
				{
					MissionObject * const missionObject = serverMissionObject->asMissionObject();
					if (missionObject)
						result.push_back(*iter);
				}
			}
		}
	}
	return result;
}

//-----------------------------------------------------------------------

bool CreatureObject::canManipulateObject(ServerObject const &target, bool movingObject, bool doPermissionCheckOnItem, bool doPermissionCheckOnParents, float maxRange, Container::ContainerErrorCode& code, bool skipNoTradeCheck, bool * allowedByGodMode) const
{
	if (allowedByGodMode)
		*allowedByGodMode = false;

	if (getClient() && !ObserveTracker::isObserving(*getClient(), target) && (target.getCacheVersion() == 0))
	{
		code = Container::CEC_Unknown;
		return false;
	}

	code = Container::CEC_Success;
	ServerObjectTemplate const * const targetTemplate = safe_cast<ServerObjectTemplate const *>(target.getObjectTemplate());
	NOT_NULL(targetTemplate);

	bool canMoveInWorld = false;
	bool godCanMove     = false;

	ServerObject const * const serverTarget = safe_cast<ServerObject const *>(&target);
	if (!serverTarget || (movingObject && serverTarget->getCacheVersion() > 0))
	{
		code = Container::CEC_Unmovable;
		return false;
	}

	if (serverTarget->isInBazaarOrVendor() && !(serverTarget->isVendor() || serverTarget->isBazaarTerminal()))
	{
		code = Container::CEC_Unknown;
		return false;
	}

	ServerObject const * const containedBy = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*serverTarget));

	//-- don't transfer from/to factory crates
	if (containedBy && containedBy->getGameObjectType () == SharedObjectTemplate::GOT_misc_factory_crate)
	{
		code = Container::CEC_Unknown;
		return false;
	}

	{
		for (size_t i = 0; i < targetTemplate->getMoveFlagsCount(); ++i)
		{
			if (targetTemplate->getMoveFlags(i) == ServerObjectTemplate::MF_player)
			{
				canMoveInWorld = true;
				godCanMove = true;
			}
			else if (targetTemplate->getMoveFlags(i) == ServerObjectTemplate::MF_gm)
				godCanMove = true;
		}
	}

	//Gods don't have distance checks or anything, so we can exit early if they are a god at this point.
	if ((canMoveInWorld || godCanMove) && getClient() && getClient()->isGod())
	{
		if (allowedByGodMode)
			*allowedByGodMode = true;

		return true;
	}

	//----------------------------------------------------------------------

	// see if the object is flagged as being owner-only manipulatable
	if (!skipNoTradeCheck)
	{
		if (target.markedNoTrade())
		{
			// the item must be owned by this creature

			NetworkId ownerId;
			if (target.asIntangibleObject())
				ownerId = containedBy ? containedBy->getOwnerId() : NetworkId::cms_invalid;
			else
				ownerId = target.getOwnerId();

			if (ownerId != getNetworkId())
			{
				code = Container::CEC_NoPermission;
				return false;
			}
		}
	}

	//----------------------------------------------------------------------

	// exclude these object types from being moved by players
	const int got = serverTarget->getGameObjectType ();

	CreatureObject const * const creatureTarget = serverTarget->asCreatureObject();
	if (movingObject)
	{
        // If the object isn't contained they may be trying to move the object
		// check the flags on the template to see if it can be picked up.
		if (!canMoveInWorld && target.isInWorld ())
		{
//			DEBUG_REPORT_LOG(true, ("canManipulate failed - This object is in the world and is flagged as immobile\n"));
			code = Container::CEC_NoPermission;
			return false;
		}

		// can't move creatures we don't own
		if (creatureTarget && creatureTarget->getOwnerId() != getNetworkId())
		{
			code = Container::CEC_NoPermission;
			return false;
		}

		if (GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_building)      ||
			GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_installation)  ||
			got == SharedObjectTemplate::GOT_lair                                    ||
			got == SharedObjectTemplate::GOT_camp                                    ||
			got == SharedObjectTemplate::GOT_vehicle                                 ||
			got == SharedObjectTemplate::GOT_corpse                                  ||
			serverTarget->asCellObject()                                             ||
			got == SharedObjectTemplate::GOT_data_mission_object)
		{
			code = Container::CEC_UnmovableType;
			return false;
		}

		//-- can't move items in the world cell unless we are explicitly owning them
		if (serverTarget->isInWorldCell () && serverTarget->getOwnerId () != getNetworkId ())
		{
			code = Container::CEC_NoPermission;
			return false;
		}
	}

	//----------------------------------------------------------------------

	ServerObject const * const firstParentInWorld = safe_cast<ServerObject const *>(ContainerInterface::getFirstParentInWorld(target));
	if (firstParentInWorld && firstParentInWorld != ContainerInterface::getFirstParentInWorld(*this) && firstParentInWorld->asTangibleObject())
	{

		if (maxRange > 0.0f)
		{
			float distance = findPosition_w().magnitudeBetween(firstParentInWorld->findPosition_w());
			distance -= getRadius();
			distance -= firstParentInWorld->getRadius();
			// Check range on the server with a 1m fudge factor in it.
			if (distance > maxRange + 1.0f)
			{
				code = Container::CEC_OutOfRange;
				return false;
			}
		}

		// skip LOS check if the item is being restored as part of decoration layout restore or if it has the skipLOS objvar
		PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (!playerObject || (playerObject->getRestoreDecorationObjectBeingRestored() != target.getNetworkId()))
		{
			if (!target.getObjVars().hasItem(OBJVAR_SKIP_LOS_CHECK) && !checkLOSTo(*firstParentInWorld))
			{
				code = Container::CEC_CantSee;
//				DEBUG_REPORT_LOG(true, ("LOS check failed in can manipulate\n"));
				return false;
			}
		}
	}

	//----------------------------------------------------------------------

	bool manipulateOk = true;

	if (target.asTangibleObject() && target.asTangibleObject()->isLocked())
	{
		TangibleObject const * tangibleTarget = target.asTangibleObject();

		bool allow = false;

		if(tangibleTarget->isUserOnAccessList(getNetworkId()) || tangibleTarget->isGuildOnAccessList(getGuildId()))
		{
			// Still have to obey house rules.
			ServerObject const * const topmost = safe_cast<ServerObject const *>(ContainerInterface::getTopmostContainer(*serverTarget));
			if (movingObject && topmost)
			{
				//Handle the case where we are in a designer placed building and a designer placed container (loot chest)
				if (   topmost->getOwnerId() == NetworkId::cms_invalid
					&& (   firstParentInWorld->getOwnerId() == NetworkId::cms_invalid
					|| firstParentInWorld->getOwnerId() == getNetworkId())
					&& serverTarget != firstParentInWorld)
				{
					allow = true;
				}
				//Handle the case where we are in a player building or pob ship
				else if (topmost->getOwnerId() == getNetworkId() || isBuildingAdminForTarget (*this, *topmost))
				{
					const bool isContainerPortallized = topmost->getPortalProperty() != 0;

					allow = (!isContainerPortallized || !movingObject || !isTargetBuildingBaseObject (*topmost, *serverTarget));
				}
				else
					allow = false;
			}
			else
				allow = true;
		}
		else
		{
			// Our target is locked, but allow admins on a building to be able to manipulate the object.

			
			ServerObject const * const topmost = safe_cast<ServerObject const *>(ContainerInterface::getTopmostContainer(*serverTarget));
			if (topmost)
			{
				//Handle the case where we are in a designer placed building and a designer placed container (loot chest)
				if (   topmost->getOwnerId() == NetworkId::cms_invalid
					&& (   firstParentInWorld->getOwnerId() == NetworkId::cms_invalid
					|| firstParentInWorld->getOwnerId() == getNetworkId())
					&& serverTarget != firstParentInWorld)
				{
					allow = true;
				}
				//Handle the case where we are in a player building or pob ship
				else if (topmost->getOwnerId() == getNetworkId())
				{
					const bool isContainerPortallized = topmost->getPortalProperty() != 0;

					allow = (!isContainerPortallized || !movingObject || !isTargetBuildingBaseObject (*topmost, *serverTarget));
				}
				else
					allow = false;
			}

		}

		if(!allow)
			code = Container::CEC_NoPermission;

		return allow;
	}

	//----------------------------------------------------------------------
	//-- see if we need to check permissions of the object itself

	if (doPermissionCheckOnItem)
	{
		//Do not allow players to manipulate other players or things contained by other players
		if (serverTarget->isPlayerControlled() && serverTarget != this)
		{
			code = Container::CEC_NoPermission;
			return false;
		}

		if (serverTarget->getOwnerId () == getNetworkId())
		{
			manipulateOk = true;
		}

		//-- an invalid owner means we can manipulate the item in general,
		//-- e.g. opening a container, but not move it
		else if (movingObject || serverTarget->getOwnerId ().isValid ())
		{
			manipulateOk = false;
		}
	}

	//----------------------------------------------------------------------
	//-- see if the permission check on parents needs to be carried out.

//ParentCheck on item
//if item is a vendor, get topmost and check vendor list
//else
//  get first parent in world
//  if is an installation check hopper list
//  else if it is a creature, check the immediate parent for ownership only (covers corpses, inventory, etc)
//  else check ownership of topmost, with 0 being allowed.
//    if topmost happens to be a player building, check its admin list and make sure the object being moved isn't part of the house
//	  else if topmost happens to be an unowned building, then any sub containers can be looted

	if (doPermissionCheckOnParents)
	{

		// Check our access list.
		if(containedBy && containedBy->asTangibleObject() && containedBy->asTangibleObject()->isLocked())
		{
			TangibleObject const * tangibleParent = containedBy->asTangibleObject();
			
			if(tangibleParent->isUserOnAccessList(getNetworkId()) || tangibleParent->isGuildOnAccessList(getGuildId()))
			{
				manipulateOk = true;
			}
			else
			{
				manipulateOk = false;
				code = Container::CEC_NoPermission;
			}

		}
		else if (!manipulateOk || movingObject)
		{
			//-- can't move items contained by a manufacturing schematic
			{
				if (containedBy && containedBy->getGameObjectType () == SharedObjectTemplate::GOT_data_manufacturing_schematic)
				{
					code = Container::CEC_NoPermission;
					return false;
				}
			}

			if (firstParentInWorld)
			{
				const int containerGot = firstParentInWorld->getGameObjectType ();
				if (GameObjectTypes::isTypeOf (containerGot, SharedObjectTemplate::GOT_installation))
				{
					//-- test admin status on installation
					manipulateOk = canHopperForTarget (*this, *firstParentInWorld);
				}
				else if (firstParentInWorld->asCreatureObject())
				{
					// check for ownership
					manipulateOk = (containedBy && (containedBy->getOwnerId() == getNetworkId() || containedBy->getOwnerId() == NetworkId::cms_invalid));
				}
				else
				{
					ServerObject const * const topmost = safe_cast<ServerObject const *>(ContainerInterface::getTopmostContainer(*serverTarget));
					if (topmost)
					{
						//Handle the case where we are in a designer placed building and a designer placed container (loot chest)
						if (   topmost->getOwnerId() == NetworkId::cms_invalid
						    && (   firstParentInWorld->getOwnerId() == NetworkId::cms_invalid
						        || firstParentInWorld->getOwnerId() == getNetworkId())
						    && serverTarget != firstParentInWorld)
						{
							manipulateOk = true;
						}
						//Handle the case where we are in a player building or pob ship
						else if (topmost->getOwnerId() == getNetworkId() || isBuildingAdminForTarget (*this, *topmost))
						{
							const bool isContainerPortallized = topmost->getPortalProperty() != 0;

							manipulateOk = (!isContainerPortallized || !movingObject || !isTargetBuildingBaseObject (*topmost, *serverTarget));
						}
						else
							manipulateOk = false;
					}
					else
						manipulateOk = false;
				}
			}
			else
				manipulateOk = false;
		}
	}


	//----------------------------------------------------------------------

	if (!manipulateOk && code == Container::CEC_Success)
		code = Container::CEC_NoPermission;

	return manipulateOk;
}

//-----------------------------------------------------------------------
/**
 * Do any processing needed during endBaselines() to handle this object if
 * it is contained by another object.
 *
 * @return  true if object should have addToWorld() called on it; false otherwise.
 */

bool CreatureObject::handleContentsSetup()
{
	bool ret = TangibleObject::handleContentsSetup();
	setStatesDueToContainment();
	return ret;
}

// ----------------------------------------------------------------------

void CreatureObject::setStatesDueToContainment()
{
	uint64 oldStates = m_states.get();
	uint64 states = oldStates&(~s_containmentStatesMask);

	ServerObject const *containingObject = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));
	if (containingObject)
	{
		SlottedContainer const *slottedContainer = containingObject->getSlottedContainerProperty();
		if (slottedContainer)
		{
			SlottedContainmentProperty const *slottedContainmentProperty = ContainerInterface::getSlottedContainmentProperty(*this);
			if (slottedContainmentProperty)
			{
				int currentArrangement = slottedContainmentProperty->getCurrentArrangement();
				if (currentArrangement != -1)
				{
					SlottedContainmentProperty::SlotArrangement const &slotArrangement = slottedContainmentProperty->getSlotArrangement(currentArrangement);
					for (SlottedContainmentProperty::SlotArrangement::const_iterator i = slotArrangement.begin(); i != slotArrangement.end(); ++i)
						states |= s_containerSlotStates[*i];
				}
			}
		}
		else if (containingObject->asCellObject())
		{
			// special case - state set when in the interior of a pob ship
			ServerObject const * const portallizedObject = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingObject));
			if (portallizedObject && portallizedObject->asShipObject())
				states |= States::getStateMask(States::ShipInterior);
		}
	}

	if (states != oldStates)
		m_states = states;
}

//-----------------------------------------------------------------------

void CreatureObject::forwardServerObjectSpecificBaselines() const
{
	TangibleObject::forwardServerObjectSpecificBaselines();

	Property const * const property = getProperty(SlowDownProperty::getClassPropertyId());
	if (property != nullptr)
	{
		SlowDownProperty const * const slowdownProperty = safe_cast<const SlowDownProperty *>(property);

		SlowDownEffectMessage const message(getNetworkId(), slowdownProperty->getTarget(), slowdownProperty->getConeLength(), slowdownProperty->getConeAngle(), slowdownProperty->getSlopeAngle(), slowdownProperty->getExpireTime());
		ServerMessageForwarding::send(message);
	}

	// if we are an ai creature, have our controller send our current ai state
	AICreatureController const * const aiCreatureController = AICreatureController::asAiCreatureController(getController());

	if (aiCreatureController != nullptr)
	{
		aiCreatureController->forwardServerObjectSpecificBaselines();
}
}

//-----------------------------------------------------------------------

void CreatureObject::sendObjectSpecificBaselinesToClient(Client const &client) const
{
	TangibleObject::sendObjectSpecificBaselinesToClient(client);

	Property const * const property = getProperty(SlowDownProperty::getClassPropertyId());
	if (property != nullptr)
	{
		SlowDownProperty const * const slowdownProperty = safe_cast<const SlowDownProperty *>(property);

		SlowDownEffectMessage const message(getNetworkId(), slowdownProperty->getTarget(), slowdownProperty->getConeLength(), slowdownProperty->getConeAngle(), slowdownProperty->getSlopeAngle(), slowdownProperty->getExpireTime());
		client.send(message, true);
	}

	UpdatePostureMessage const updatePostureMessage(getNetworkId(), getPosture());
	client.send(updatePostureMessage, true);
}

//-----------------------------------------------------------------------

/**
 * Initializes a newly created object.
 */
void CreatureObject::initializeFirstTimeObject()
{
	TangibleObject::initializeFirstTimeObject();

	//-- Get ourselves into a reasonable locomotion state.
	updateMovementInfo();

	// create the default weapon
	initializeDefaultWeapon();

	// set the current weapon
	WeaponObject * weapon = getReadiedWeapon();
	if (weapon != nullptr)
		setCurrentWeapon(*weapon);

#ifdef _DEBUG
	//Quick error check since all creatures should have a default weapon
	WARNING_STRICT_FATAL(!getDefaultWeapon(), ("Creature %s (%s) has no default "
		"weapon", getNetworkId().getValueString().c_str(), getTemplateName()));
#else
	getDefaultWeapon(); //silence, dammit
#endif 

	const ServerCreatureObjectTemplate * myTemplate = safe_cast<const ServerCreatureObjectTemplate *>(getObjectTemplate());
	Unicode::String newName = NameManager::getInstance().generateRandomName(ConfigServerGame::getCharacterNameGeneratorDirectory(), myTemplate->getNameGeneratorType());
	if (!newName.empty())
	{
		setObjectName(newName);
	}

	setOwnerId(getNetworkId());
	recomputeSlopeModPercent();

	setupSkillData();
	packWearables();

	ServerObject *bankContainer = getBankContainer();
	if (bankContainer)
		bankContainer->setLoadContents(false);

	// note: this MUST come after TangibleObject::initializeFirstTimeObject() is called
	float defaultAlterTime = 0;
	if (!isPlayerControlled())
		defaultAlterTime = AlterResult::cms_alterQuickly;
//	DEBUG_REPORT_LOG(true, ("[aitest] setting creature %s default alter time to %f\n", getNetworkId().getValueString().c_str(), defaultAlterTime));
	setDefaultAlterTime(defaultAlterTime);
}	// CreatureObject::initializeFirstTimeObject

//-----------------------------------------------------------------------

void CreatureObject::onLoadedFromDatabase()
{
	if (isAuthoritative())
	{
		if (isPlayerControlled())
		{
			onClientAboutToLoad();

			// Initialize A-Tab info if we need to.
			SlottedContainer * const container = ContainerInterface::getSlottedContainer(*this);
			if(container)
			{
				SlotId slot;
				Container::ContainerErrorCode tmp;
				slot = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::appearance);
				if(slot != SlotId::invalid)
				{
					Container::ContainedItem itemId = container->getObjectInSlot(slot, tmp);
					Object* appearanceInventory = itemId.getObject();
					if(appearanceInventory == nullptr)
					{
						DEBUG_WARNING(true, ("Player %s has lost their appearance inventory", getNetworkId().getValueString().c_str()));
						appearanceInventory = ServerWorld::createNewObject(s_appearanceTemplate, *this, slot, false);
						if(!appearanceInventory)
						{
							DEBUG_FATAL(true, ("Could not create an appearance inventory for the player who lost theirs"));
						}
					}
				}
			}


			PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
			if (playerObject)
			{
				// move old badges over to the collection system
				if (getObjVars().hasItem(OBJVAR_BADGES) && (getObjVars().getType(OBJVAR_BADGES) == DynamicVariable::INT_ARRAY))
				{
					std::vector<int> badges;
					getObjVars().getItem(OBJVAR_BADGES, badges);

					if (!badges.empty())
						playerObject->migrateLegacyBadgesToCollection(badges);

					removeObjVarItem(OBJVAR_BADGES);
					removeObjVarItem(OBJVAR_COUNT_BADGES);
				}

				// check for Jedi initialization
				if (getObjVars().hasItem(OBJVAR_NEW_JEDI_FORCE))
				{
					int start_force;
					getObjVars().getItem(OBJVAR_NEW_JEDI_FORCE, start_force);
					removeObjVarItem(OBJVAR_NEW_JEDI_FORCE);
					if (start_force > 0)
					{
						playerObject->setMaxForcePower(start_force);
						playerObject->setForcePowerRegenRate(ConfigServerGame::getMinForcePowerRegenRate());
					}
					else
					{
						WARNING(true, ("Player %s had the %s objvar, but it had a "
							"value of %d", getNetworkId().getValueString().c_str(),
							OBJVAR_NEW_JEDI_FORCE.c_str(), start_force));
					}
				}

				// check the non-pool attributes: their value should be the
				// same as their max value, since we don't regenerate/heal them
				for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
				{
					if (Attributes::isAttribPool(i))
						continue;

					Attributes::Value attrib = getUnmodifiedAttribute(i);
					Attributes::Value max = getUnmodifiedMaxAttribute(i);
					if (attrib != max)
						setAttribute(i, max);
				}

				// initialize the character born date, if necessary
				playerObject->setBornDate();

				// retroactively set CTS history and CTS title(s), if necessary,
				// and fix up the character's born date, if necessary, if it was
				// incorrectly changed from a prior CTS where the character's born
				// date was not transferred;
				//
				// should be done *AFTER* the above statement to initialize the
				// character born date, if necessary
				GameServer::setRetroactiveCtsHistory(*this);

				// set the "born on <galaxy>" title if this galaxy supports
				// it and it hasn't already been set on this character
				if (!getObjVars().hasItem("bornOnGalaxyTitleSet"))
				{
					// lower case cluster name and replace - with _ for use with the collection slot name
					std::string collectionSlotName = std::string("born_on_") + Unicode::toLower(GameServer::getInstance().getClusterName());

					std::string::size_type pos = collectionSlotName.find('-');
					while (pos != std::string::npos)
					{
						collectionSlotName.replace(pos, 1, 1, '_');
						pos = collectionSlotName.find('-');
					}

					CollectionsDataTable::CollectionInfoSlot const * collectionSlot = CollectionsDataTable::getSlotByName(collectionSlotName);
					if (collectionSlot)
					{
						FATAL((collectionSlot->collection.name != std::string("born_on_collection")), ("collection slot %s must be in collection born_on_collection", collectionSlot->name.c_str()));
						FATAL((collectionSlot->collection.page.name != std::string("born_on_page")), ("collection slot %s must be in collection page born_on_page", collectionSlot->name.c_str()));
						FATAL((collectionSlot->collection.page.book.name != std::string("born_on_book")), ("collection slot %s must be in collection book born_on_book", collectionSlot->name.c_str()));

						// see if character transferred from another galaxy, and
						// if yes, then the original galaxy is the home galaxy
						std::string originalSourceGalaxy;
						int earliestTransferTime = -1;

						DynamicVariableList::NestedList const ctsTransactions(getObjVars(), "ctsHistory");
						for (DynamicVariableList::NestedList::const_iterator i = ctsTransactions.begin(); i != ctsTransactions.end(); ++i)
						{
							Unicode::String ctsTransactionDetail;
							if (i.getValue(ctsTransactionDetail))
							{
								Unicode::UnicodeStringVector tokens;
								if (Unicode::tokenize(ctsTransactionDetail, tokens, nullptr, nullptr) && (tokens.size() >= 4))
								{
									int const transferTime = atoi(Unicode::wideToNarrow(tokens[0]).c_str());
									if ((earliestTransferTime == -1) || (transferTime < earliestTransferTime))
									{
										earliestTransferTime = transferTime;
										originalSourceGalaxy = Unicode::wideToNarrow(tokens[1]);
									}
								}
							}
						}

						// character transferred from another galaxy, so
						// use the original galaxy as the home galaxy
						if (!originalSourceGalaxy.empty())
						{
							// lower case cluster name and replace - with _ for use with the collection slot name
							collectionSlotName = std::string("born_on_") + Unicode::toLower(originalSourceGalaxy);

							std::string::size_type pos = collectionSlotName.find('-');
							while (pos != std::string::npos)
							{
								collectionSlotName.replace(pos, 1, 1, '_');
								pos = collectionSlotName.find('-');
							}

							collectionSlot = CollectionsDataTable::getSlotByName(collectionSlotName);
							if (collectionSlot)
							{
								FATAL((collectionSlot->collection.name != std::string("born_on_collection")), ("collection slot %s must be in collection born_on_collection", collectionSlot->name.c_str()));
								FATAL((collectionSlot->collection.page.name != std::string("born_on_page")), ("collection slot %s must be in collection page born_on_page", collectionSlot->name.c_str()));
								FATAL((collectionSlot->collection.page.book.name != std::string("born_on_book")), ("collection slot %s must be in collection book born_on_book", collectionSlot->name.c_str()));
							}
						}
					}

					// set "born on <galaxy>" collection slot and clear all the other "born on <galaxy>" collection slots;
					// this still needs to be run even if collectionSlot is nullptr in order to forcefully clear all of the
					// other "born on <galaxy>" collection slots, since we are reusing deleted/no longer used collection
					// slot bits, and those bits may be left in a set state at the time they were deleted/no longer used
					std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection("born_on_collection");

					for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
					{
						playerObject->modifyCollectionSlotValue((*iter)->name, ((*iter == collectionSlot) ? 1ll : -1ll));
					}

					IGNORE_RETURN(setObjVarItem("bornOnGalaxyTitleSet", 1));
				}
			}

			// wait for all loading/initialization to complete, and then double
			// check the character's title to make sure it is a valid title
			MessageToQueue::getInstance().sendMessageToC(getNetworkId(),
				"C++CheckCharacterTitle", "", 30, false);
		}

		// store the default regen values in the former faucet attrib slots
		m_attributes.set(Attributes::Constitution, static_cast<int>(ConfigServerGame::getDefaultHealthRegen()));
		m_attributes.set(Attributes::Stamina, static_cast<int>(ConfigServerGame::getDefaultActionRegen()));
		m_attributes.set(Attributes::Willpower, static_cast<int>(ConfigServerGame::getDefaultMindRegen()));

		if(!isPlayerControlled())
			fixupPersistentBuffsAfterLoading();
		else
		{
			PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
			if (playerObject && playerObject->isInitialized())
			{
				fixupPersistentBuffsAfterLoading();
				fixupLevelXpAfterLoading();
			}
		}

		// there are cetain states we don't want to be in on start-up
		setState(States::Cover, false);
		setState(States::Combat, false);
		setState(States::Peace, false);
		setState(States::Aiming, false);
		setState(States::Meditate, false);
		setState(States::Berserk, false);
		setState(States::FeignDeath, false);
		setState(States::Tumbling, false);
		setState(States::Rallied, false);
		setState(States::Stunned, false);
		setState(States::Blinded, false);
		setState(States::Dizzy, false);
		setState(States::Intimidated, false);
		setState(States::Immobilized, false);
		setState(States::Crafting, false);
		setState(States::MountedCreature, false);
		setState(States::Disguised, false);
		setStatesDueToContainment();

		// override the speeds stored in the database
		// @todo: make the speeds non-persisted
		setBaseWalkSpeed((safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSpeed(SharedCreatureObjectTemplate::MT_walk)));
		setBaseRunSpeed((safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSpeed(SharedCreatureObjectTemplate::MT_run)));
	}

	// setupSkillData MUST be called before we add the bonuses for equipment!
	setupSkillData();
	packWearables();

	if (isAuthoritative() && isPlayerControlled())
	{
		// make sure the player has no more expertise skills than they should have for their level
		// if they do, log them as a suspected cheater
		int const remainingExpertisePoints = getRemainingExpertisePoints();

		if (remainingExpertisePoints < 0)
		{
			LOG("CustomerService",
				("SuspectedCheaterChannel: %s has more expertises than level %d allows. Amount over limit is %d.",
				PlayerObject::getAccountDescription(this).c_str(), getLevel(), -remainingExpertisePoints)
				);
		}

		// if we are a player, go through all the creature's equipment and update for
		// bonus skill mods
		std::vector<std::pair<std::string, int> > skillModBonuses;
		std::vector<std::pair<int, int> > attribBonuses;
		Container const * const equipment = ContainerInterface::getContainer(*this);
		if (equipment != nullptr)
		{
			for (ContainerConstIterator i(equipment->begin()); i != equipment->end(); ++i)
			{
				ServerObject const * const so = safe_cast<ServerObject const *>((*i).getObject());
				if (so)
				{
					TangibleObject const * const equippedItem = so->asTangibleObject();
					if (equippedItem != nullptr)
					{
						equippedItem->getSkillModBonuses(skillModBonuses);
						int bonusCount = skillModBonuses.size();
						{
						for (int j = 0; j < bonusCount; ++j)
						{
							std::pair<std::string, int> const &bonus = skillModBonuses[j];
							addModBonus(bonus.first, bonus.second);
						}
					}
						equippedItem->getAttribBonuses(attribBonuses);
						bonusCount = attribBonuses.size();
						{
						for (int j = 0; j < bonusCount; ++j)
						{
							std::pair<int, int> const &bonus = attribBonuses[j];
							addAttribBonus(bonus.first, bonus.second);
						}
						}
					}
				}
			}
		}
	}

	TangibleObject::onLoadedFromDatabase();

	if (isAuthoritative())
	{
		// note: this MUST come after TangibleObject::onLoadedFromDatabase() is called
		float defaultAlterTime = 0;
		if (!isPlayerControlled())
			defaultAlterTime = AlterResult::cms_alterQuickly;
//		DEBUG_REPORT_LOG(true, ("[aitest] setting creature %s default alter time to %f\n", getNetworkId().getValueString().c_str(), defaultAlterTime));
		setDefaultAlterTime(defaultAlterTime);

		CreatureController * controller = getCreatureController();
		if (controller != nullptr)
			controller->updateHibernate();
	}
}

//-----------------------------------------------------------------------

/**
 * Initializes a proxy object.
 */
void CreatureObject::endBaselines()
{
	// catch people in potentially deleted newbie halls
	if (NewbieTutorial::getSceneId() == ServerWorld::getSceneId())
		getContainedByProperty()->setContainedBy(NetworkId::cms_invalid);

	TangibleObject::endBaselines();

	if (isAuthoritative())
	{
		// set our initial posture depending if we are incapacitated/dead or not
		Postures::Enumerator posture = getPosture();
		if (isDead())
			posture = Postures::Dead;
		if (getAttribute(Attributes::Health) <= 0)
		{
			// make sure we are incapacitated, unless we are dead
			if (posture != Postures::Dead)
				posture = Postures::Incapacitated;
		}

		if (posture != getPosture())
			setPosture(posture);

		updateMovementInfo();

		// Fix up guildId - set regardless of whether it is already correct, as this
		// triggers entry into the guild chat room.
		int guildId = GuildInterface::getGuildId(getNetworkId());

		// fix up issue there a guild leader gets mistakenly removed
		// from the guild (i.e. the guild leader's permission is reset
		// to 0, meaning he is only a sponsored member, and the guild
		// id won't be returned in GuildInterface::getGuildId()), causing
		// the guild to be leaderless; we only need to get the guild id
		// for the guild that this character is the leader of; setGuildId()
		// will do the rest of the work
		if (guildId == 0)
			guildId = GuildInterface::getGuildIdForGuildLeader(getNetworkId());

		setGuildId(guildId);

		// Fix up guildWarEnabled
		setGuildWarEnabled(GuildInterface::getGuildMemberGuildWarEnabled(getGuildId(), getNetworkId()));

		// Fix up militiaOfCityId
		setMilitiaOfCityId(CityInterface::getMilitiaOfCityId(getNetworkId()));

		// Mount: needs to grab its position from the rider since the rider dictates
		// mount position.
		if (getState(States::MountedCreature))
		{
			CreatureObject const *const rider = getPrimaryMountingRider();
			if (rider)
			{
				// Use parent space since both the rider and mount should be
				// attached to the same thing.  Initially this is limited to
				// the world.
				setPosition_p(rider->getPosition_p());
			}
		}

		// depersist persisted command cooldowns
		CommandQueue * commandQueue = getCommandQueue();
		if (commandQueue)
		{
			commandQueue->depersistCooldown();
		}

		// make sure that factional player characters are at least rank 1 (rating 0)
		// and non-factional player characters have no current GCW points or rating
		PlayerObject * const player = PlayerCreatureController::getPlayerObject(this);
		if (player)
		{
			static int const min = std::numeric_limits<int>::min();
			if (PvpData::isImperialFactionId(getPvpFaction()) || PvpData::isRebelFactionId(getPvpFaction()))
			{
				// decrementing the value by a really large
				// number will cause it to reset to 0
				if (player->getCurrentGcwRating() < Pvp::getMinRatingForRank())
					player->modifyCurrentGcwRating(min, true);
			}
			else
			{
				// decrementing the value by a really large
				// number will cause it to reset to 0
				if (player->getCurrentGcwPoints() > 0)
					player->modifyCurrentGcwPoints(min, false);

				if (player->getCurrentGcwRating() != -1)
					player->modifyCurrentGcwRating(min, false);

				if (player->getCurrentPvpKills() > 0)
					player->modifyCurrentPvpKills(min, false);
			}

			// make sure that m_nextGcwRatingCalcTime is set so that the
			// rating will be recalculated at the next recalculation interval
			player->setNextGcwRatingCalcTime(true);

			// migrate veteran rewards claim tracking information to new system
			VeteranRewardManager::migrateVeteranRewardsClaimInformation(*this);

			// request the list of account feature transactions for the character
			// from the LoginServer DB which will be compared to the list of account
			// feature transactions stored on the character, and if there is any
			// discrepancy, then it probably means there was a rollback, so we would
			// need to grant the character any missing item(s)
			FeatureIdTransactionRequest const fitr(GameServer::getInstance().getProcessId(), player->getStationId(), getNetworkId());
			GameServer::getInstance().sendToCentralServer(fitr);

			// set the guild rank
			GuildMemberInfo const * const guildMemberInfo = GuildInterface::getGuildMemberInfo(getGuildId(), getNetworkId());
			if (guildMemberInfo)
				player->setGuildRank(guildMemberInfo->m_rank);

			// set the citizen rank
			// join the city chat room
			std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(getNetworkId());
			if (!cityIds.empty())
			{
				int const cityId = cityIds.front();
				CitizenInfo const * const citizenInfo = CityInterface::getCitizenInfo(cityId, getNetworkId());
				if (citizenInfo)
					player->setCitizenRank(citizenInfo->m_citizenRank);

				// join the city chat room
				CityInterface::enterCityChatRoom(cityId, *this);
			}

			// set the player's citizenship information
			player->updateCitizenshipInfo();

			// set the player's GCW Region info
			player->updateGcwRegionInfo();

			// set the player's GCW Region Defender info
			player->updateGcwDefenderRegionInfo();

			// fix the citizen name in the citizen roster, if necessary
			CityInterface::verifyCitizenName(getNetworkId(), Unicode::wideToNarrow(getAssignedObjectName()));

			// check for over lot limit condition
			player->setAccountNumLotsOverLimitSpam();

			// fix the pgc chronicler's name in the pgc rating data, if necessary
			CityInterface::verifyPgcChroniclerName(getNetworkId(), Unicode::wideToNarrow(getAssignedObjectName()));

			// depersist galactic reserve 
			{
				// migrate any galactic reserve information that's stored in the PlayerObject into the CreatureObject
				int temp;
				if (player->getObjVars().getItem("galactic_reserve", temp))
				{
					player->removeObjVarItem("galactic_reserve");
					setObjVarItem("galactic_reserve", temp);
				}

				if (player->getObjVars().getItem("galactic_reserve_cooldown", temp))
				{
					player->removeObjVarItem("galactic_reserve_cooldown");
					setObjVarItem("galactic_reserve_cooldown", temp);
				}

				player->depersistGalacticReserveDeposit();
			}

			// depersist mercenary status
			if (!ConfigServerGame::getEnableCovertImperialMercenary())
			{
				player->modifyCollectionSlotValue("covert_imperial_mercenary", -1);
			}

			if (!ConfigServerGame::getEnableOvertImperialMercenary())
			{
				player->modifyCollectionSlotValue("overt_imperial_mercenary", -1);
			}

			if (!ConfigServerGame::getEnableCovertRebelMercenary())
			{
				player->modifyCollectionSlotValue("covert_rebel_mercenary", -1);
			}

			if (!ConfigServerGame::getEnableOvertRebelMercenary())
			{
				player->modifyCollectionSlotValue("overt_rebel_mercenary", -1);
			}

			if (!PvpData::isNeutralFactionId(getPvpFaction()))
			{
				player->modifyCollectionSlotValue("covert_imperial_mercenary", -1);
				player->modifyCollectionSlotValue("overt_imperial_mercenary", -1);
				player->modifyCollectionSlotValue("covert_rebel_mercenary", -1);
				player->modifyCollectionSlotValue("overt_rebel_mercenary", -1);
			}
			else
			{
				if (player->hasCompletedCollectionSlot("covert_imperial_mercenary"))
				{
					Pvp::setNeutralMercenaryFaction(*this, PvpData::getImperialFactionId(), PvpType_Covert);
				}
				else if (player->hasCompletedCollectionSlot("overt_imperial_mercenary"))
				{
					Pvp::setNeutralMercenaryFaction(*this, PvpData::getImperialFactionId(), PvpType_Declared);
				}
				else if (player->hasCompletedCollectionSlot("covert_rebel_mercenary"))
				{
					Pvp::setNeutralMercenaryFaction(*this, PvpData::getRebelFactionId(), PvpType_Covert);
				}
				else if (player->hasCompletedCollectionSlot("overt_rebel_mercenary"))
				{
					Pvp::setNeutralMercenaryFaction(*this, PvpData::getRebelFactionId(), PvpType_Declared);
				}
			}
		}
	}
	else
	{
		getAvailableLocomotions();
	}

	checkAndRestoreRequiredSlots();

	// get the current weapon, which will set it to our held or default weapon
	IGNORE_RETURN(getCurrentWeapon());

	// Rider: needs to not participate in CollisionWorld.
	if (getState(States::RidingMount))
	{
		CollisionProperty *const collisionProperty = getCollisionProperty();
		if (collisionProperty)
		{
			if (isInWorld())
				CollisionWorld::removeObject(this);

			collisionProperty->setDisableCollisionWorldAddRemove(true);
		}
	}

	// Fix up the footprint's swim height to take into consideration any scaling on this creature.
	Footprint *const footprint = getFootprint();
	if (footprint)
		footprint->setSwimHeight(getSwimHeight());

	if (isAuthoritative())
		recomputeSlopeModPercent();

	if (ConfigServerGame::getMoveSimEnabled ())
		MoveSimManager::checkApplySimulation (*this);

	if (isAuthoritative())
		GroupMissionCriticalObjectsBuilder::updateGroupMissionCriticalObjects(*this, true);
}	// CreatureObject::endBaselines

//-----------------------------------------------------------------------

void CreatureObject::checkAndRestoreRequiredSlots()
{
	SlottedContainer * const container = ContainerInterface::getSlottedContainer(*this);
	if (container == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("This creature is not slotted!"));
		return;
	}

	// get the default weapon from it's slot and set the class variable

	bool foundDefaultWeapon = false;
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	Container::ContainedItem const itemId = container->getObjectInSlot(s_defaultWeaponSlotId, tmp);
	Object * const o = itemId.getObject();
	if (o)
	{
		ServerObject * const so = o->asServerObject();
		if (so && so->asWeaponObject())
			foundDefaultWeapon = true;
	}

	if (!foundDefaultWeapon)
	{
		if (isAuthoritative())
		{
			// try to make the default weapon from the template instead
			initializeDefaultWeapon();
		}
		else
		{
			WARNING_STRICT_FATAL(true, ("Unable to find default weapon slot id for %s(%s)",
				getNetworkId().getValueString().c_str(),
				getObjectTemplateName()));
		}
	}

	if (isPlayerControlled() && isAuthoritative())
	{
		//restore inventory, mission bag, and datapad for players

		SlotId slot = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::inventory);
		if (slot != SlotId::invalid)
		{
			Container::ContainedItem itemId = container->getObjectInSlot(slot, tmp);
			Object* inventory = itemId.getObject();
			if (inventory == nullptr)
			{
				WARNING(true, ("Player %s has lost their inventory", getNetworkId().getValueString().c_str()));
				inventory = ServerWorld::createNewObject(s_inventoryTemplate, *this, slot, false);
				if (!inventory)
				{
					DEBUG_FATAL(true, ("Could not create an inventory for the player who lost theirs"));
				}
			}
		}

		slot = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::datapad);
		if (slot != SlotId::invalid)
		{
			Container::ContainedItem itemId = container->getObjectInSlot(slot, tmp);
			Object* datapad = itemId.getObject();
			if (datapad == nullptr)
			{
				WARNING(true, ("Player %s has lost their datapad", getNetworkId().getValueString().c_str()));
				datapad = ServerWorld::createNewObject(s_datapadTemplate, *this, slot, false);
				if (!datapad)
				{
					DEBUG_FATAL(true, ("Could not create an datapad for the player who lost theirs"));
				}
			}
		}

		slot = SlotIdManager::findSlotId(ConstCharCrcLowerString("mission_bag"));
		if (slot != SlotId::invalid)
		{
			Container::ContainedItem itemId = container->getObjectInSlot(slot, tmp);
			ServerObject* missionBag = safe_cast<ServerObject*>(itemId.getObject());
			if (missionBag == nullptr)
			{
				WARNING(true, ("Player %s has lost their mission bag", getNetworkId().getValueString().c_str()));
				missionBag = ServerWorld::createNewObject(s_missionBagTemplate, *this, slot, false);
				if (!missionBag)
				{
					DEBUG_FATAL(true, ("Could not create a mission bag for the player who lost theirs"));
				}
				else
					missionBag->persist();
			}
		}

		slot = SlotIdManager::findSlotId(ConstCharCrcLowerString("bank"));
		if (slot != SlotId::invalid)
		{
			Container::ContainedItem itemId = container->getObjectInSlot(slot, tmp);
			ServerObject* bank = safe_cast<ServerObject*>(itemId.getObject());
			if (bank == nullptr)
			{
				WARNING(true, ("Player %s has lost their bank", getNetworkId().getValueString().c_str()));
				bank = ServerWorld::createNewObject(s_bankTemplate, *this, slot, false);
				if (!bank)
				{
					DEBUG_FATAL(true, ("Could not create a bank for the player who lost theirs"));
				}
				else
				{
					bank->persist();
					bank->setLoadContents(false);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

/**
 * Allows an object to do pre-deletion cleanup.
 */
void CreatureObject::onRemovingFromWorld()
{
	TangibleObject::onRemovingFromWorld();
	if (isAuthoritative())
	{
		// if we are crafting, end the crafting session
		PlayerObject * const player = PlayerCreatureController::getPlayerObject(this);
		if (player != nullptr && player->isCrafting())
			player->stopCrafting(false);

		// exit all notify regions
		Archive::AutoDeltaSet<std::pair<std::string, std::string> >::const_iterator i;
		for(i = m_notifyRegions.begin(); i != m_notifyRegions.end(); ++i)
		{
			ScriptParams params;
			params.addParam(i->first.c_str());
			params.addParam(i->second.c_str());
			if(getScriptObject())
			{
				IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_EXIT_REGION, params));
			}
		}
		m_notifyRegions.clear();
	}

	if (isMountable())
		IGNORE_RETURN(detachAllRiders());
	{
	}
}

void CreatureObject::addToWorld()
{
	if (getKill())
		return;

	if (isInWorld())
	{
		DEBUG_WARNING(true, ("Calling add to world on object already in it %s", getNetworkId().getValueString().c_str()));
		return;
	}
	updateWorldSphere();

	Object::addToWorld();
	updateTriggerVolumes();

	if (getScriptObject() != nullptr)
		getScriptObject()->setOwnerIsLoaded();

	ContainedByProperty * const containedByProperty = getContainedByProperty();
	if (containedByProperty && containedByProperty->getContainedByNetworkId().isValid())
	{
		Object const * const containedByObject = containedByProperty->getContainedBy();
		if (containedByObject && !containedByObject->isInWorld())
		{
			if (!ServerWorld::isSpaceScene())
			{
				// attempt to put the character at the ejection point of the building
				DynamicVariableLocationData ejectionPoint;
				if (getObjVars().getItem("building_ejection_point", ejectionPoint) && (ServerWorld::getSceneId() == ejectionPoint.scene) && (!ejectionPoint.cell.isValid()))
				{
					DEBUG_REPORT_LOG(true,("Ejecting player %s from building %s, because the building does not exist anymore or is not loaded in the world\n",getNetworkId().getValueString().c_str(), containedByProperty->getContainedByNetworkId().getValueString().c_str()));
					LOG("CustomerService",("Login:  Ejecting player %s from building %s, because the building does not exist anymore or is not loaded in the world",getNetworkId().getValueString().c_str(), containedByProperty->getContainedByNetworkId().getValueString().c_str()));
					removeObjVarItem("building_ejection_point");
					teleportObject(ejectionPoint.pos, ejectionPoint.cell, "", ejectionPoint.pos,"");

				}
				else
				// warp the player to the Mos Eisley Starport
				{
					DEBUG_REPORT_LOG(true, ("Tried to load creature %s into invalid container. Moving to Mos Eisley Starport.\n", getNetworkId().getValueString().c_str()));
					LOG("CustomerService",("Login:  Tried to load creature %s into invalid container. Moving to Mos Eisley Starport.\n", getNetworkId().getValueString().c_str()));
					GameServer::getInstance().requestSceneWarp(CachedNetworkId(*this),"tatooine",Vector(3528,5,-4804),NetworkId::cms_invalid,Vector(3528,5,-4804));
				}
			}
		}
	}

	onAddedToWorld();
}

//-----------------------------------------------------------------------

void CreatureObject::initializeNewPlayer()
{
}

//-----------------------------------------------------------------------

/** Set up the inventory object for the creature and initialize with contents from template.
 */
void CreatureObject::setupInventory()
{

}

//-----------------------------------------------------------------------

/**
 * Sets up the creature's skills and related data.
 */
void CreatureObject::setupSkillData()
{
	if (isAuthoritative())
	{
		// clear out our current skill related data
		m_modMap.clear();
		if (isPlayerControlled())
		{
			clearCommands();

			PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
			if (playerObject != nullptr)
			{
				playerObject->clearSchematics();
			}
		}

		m_skills.erase(0);

		// set the skill related data from the skills
		for (SkillList::const_iterator skillIter = m_skills.begin(); skillIter != m_skills.end(); ++skillIter)
		{
			SkillObject const * skill = *skillIter;
			if (skill)
			{
				// init skillmods
				std::vector<std::pair<std::string, int> > const &statMods = skill->getStatisticModifiers();
				std::vector<std::pair<std::string, int> >::const_iterator modIter;
				for (modIter = statMods.begin(); modIter != statMods.end(); ++modIter)
				{
					addModValue((*modIter).first, (*modIter).second, true);
				}
				// init skill commands
				{
					const SkillObject::StringVector & sv = skill->getCommandsProvided ();
					for (SkillObject::StringVector::const_iterator i = sv.begin();
						i != sv.end(); ++i)
					{
						grantCommand(*i, true);
					}
				}
				// init schematics
				{
					const SkillObject::StringVector & sv = skill->getSchematicsGranted ();
					for (SkillObject::StringVector::const_iterator i = sv.begin(); i != sv.end(); ++i)
						grantSchematicGroup("+" + *i, true);
				}
			}
		}

		// read the non-skill data from the objvars and add those to the creature
		// note that we add them as "from skill" so that they don't get added to the
		// objvar list again
		{
			DynamicVariableList::NestedList mods(getObjVars(), OBJVAR_NOT_SKILL_MODS);
			DynamicVariableList::NestedList::const_iterator i(mods.begin());
			int value;
			for (; i != mods.end(); ++i)
			{
				i.getValue(value);
				addModValue(i.getName(), value, true);
			}
		}
		{
			DynamicVariableList::NestedList commands(getObjVars(), OBJVAR_NOT_SKILL_COMMANDS);
			DynamicVariableList::NestedList::const_iterator i(commands.begin());
			for (; i != commands.end(); ++i)
			{
				grantCommand(i.getName(), true);
			}
			if(getClient())
			{
				revokeCommand(AdminAccountManager::getAdminCommandName(), false);

				if (getClient()->isGod())
					grantCommand(AdminAccountManager::getAdminCommandName(), false);
			}
		}
		{
			DynamicVariableList::NestedList schematics(getObjVars(), OBJVAR_NOT_SKILL_SCHEMATICS);
			DynamicVariableList::NestedList::const_iterator i(schematics.begin());
			for (; i != schematics.end(); ++i)
			{
				grantSchematic(strtoul(i.getName().c_str(), nullptr, 10), true);
			}
		}

		// calculate the level based on the skill list
		recalculateLevel();
	}
}	// CreatureObject::setupSkillData

//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* CreatureObject::createDefaultController ()
{
	const ServerCreatureObjectTemplate * myTemplate = safe_cast<
		const ServerCreatureObjectTemplate *>(getObjectTemplate());

	Controller * controller = 0;
	if (myTemplate->getCanCreateAvatar())
	{
		controller = new PlayerCreatureController(this);
	}
	else
	{
		controller = new AICreatureController(this);

		AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(controller);

		if (aiCreatureController != nullptr)
		{
			aiCreatureController->addServerNpAutoDeltaVariables(m_serverPackage_np);
		}
	}

	setController(controller);
	return controller;
}	// CreatureObject::createDefaultController

//-----------------------------------------------------------------------

CreatureController* CreatureObject::getCreatureController()
{
	return safe_cast<CreatureController*>(getController());
}

//----------------------------------------------------------------------

const CreatureController* CreatureObject::getCreatureController() const
{
	return safe_cast<const CreatureController*>(getController());
}

//----------------------------------------------------------------------

void CreatureObject::setAuthority()
{
	// don't recalculate our attribs until all our equipment has been made
 	// authoritative
	++m_deferComputeTotalAttributes;
	TangibleObject::setAuthority();
	--m_deferComputeTotalAttributes;
	computeTotalAttributes();
}

//----------------------------------------------------------------------


void CreatureObject::setAuthServerProcessId(uint32 processId)
{
	uint32 const oldProcess = getAuthServerProcessId();
	TangibleObject::setAuthServerProcessId(processId);

	if (oldProcess != getAuthServerProcessId())
	{
		// we can't trade if we are on different servers, so cancel it
		if (getCreatureController()->getSecureTrade() != nullptr)
		{
			getCreatureController()->getSecureTrade()->cancelTrade(*this);
		}
	}
}

//----------------------------------------------------------------------

/**
 * Updates the creature. Removes it from combat if it is incapacitated.
 *
 * @param time		current game time
 *
 * @return base class result
 */
float CreatureObject::alter(float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::alter");
	float const result = TangibleObject::alter(time);

	m_regionFlagTimer.updateNoReset(time); // Update our notify timer.

	if (isPlayerControlled() && (isIncapacitated() || isDead()))
	{
		// If the player is trading, cancel the trade.
		// We need to force the issue to catch edge cases.
		ServerSecureTrade * const secureTradeObject = getCreatureController()->getSecureTrade();
		if (secureTradeObject != nullptr)
		{
			secureTradeObject->cancelTrade(*this);
		}
	}

	if (!isAuthoritative())
	{
		// Non-authoritative activity.
		removeFromSpawnQueue();
	}
	else
	{
		PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::alter auth stuff");
		//-- Mounts/vehicles: driver moves the server's mount/vehicle position to that of the driver.
		transferRiderPositionToMount();

		updateVehiclePhysicsData ();

		// Authoritative activity.

		// update the invulnerability timer
		if (m_invulnerabilityTimer.get() != 0.0f)
		{
			float const timeLeft = std::max(0.f, m_invulnerabilityTimer.get()-time);
			m_invulnerabilityTimer.set(timeLeft);
		}
		else
		{
			PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::alter commandqueue");
			// update the command queue
			getCommandQueue()->update(time);
		}

		//check timer and migrate stats if necessary
		if (isPlayerControlled())
		{
			PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::alter auth player stuff");
			// keep the spawn queue pumping.
			// todo: if the spawn queue is starving out players because
			// there are too may other players on the queue, make some
			// preliminary checks (e.g. in a city, too many creatures nearby)
			// and reduce the spawn queue size
			addToSpawnQueue();

			{
				PROFILER_AUTO_BLOCK_DEFINE("gradual customizations");
				ServerImageDesignerManager::updateGradualCustomizations(this);
			}

			PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
			if (playerObject)
			{
				PROFILER_AUTO_BLOCK_DEFINE("playerobject stuff");
				playerObject->alterPlayedTime(time);
				playerObject->updateKillMeter(time);
				playerObject->checkStomach();
				playerObject->checkTheater(getPosition_w(), getSceneId(), getRunSpeed(), 128.0f);
				updateGroup();

				if (playerObject->getIsUnsticking() && getPositionChanged())
				{
					Chat::sendSystemMessageSimple(*this, SharedStringIds::unstick_request_cancelled, nullptr);
					playerObject->setIsUnsticking(false);
				}
			}

			if (!ServerWorld::isSpaceScene())
			{
				// if we're in a conversation, end it if we move too far from the npc
				if (m_npcConversation != nullptr)
				{
					PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::alter::npcConvCheck");
					bool endConversation = false;
					// check the distance to the npc
					ServerObject const * const npc = safe_cast<ServerObject const *>(m_npcConversation->getNPC().getObject());
					if (npc != nullptr)
					{
						float distance = findPosition_w().magnitudeBetween(npc->findPosition_w());
						distance -= getRadius();
						distance -= npc->getRadius();
						if (distance > ConfigServerGame::getNpcConversationDistance())
							endConversation = true;
					}
					else
					{
						endConversation = true;
					}
					if (endConversation)
						endNpcConversation();
				}
			}
		}
		alterPseudoPlayedTime(time);
		updateBuffs();

		//-- Handle mounts-specific functionality for an authoritative rider or mount.
		alterAuthoritativeForMounts();
	}

	// see if we are performing a slow down effect
	Property * property = getProperty(SlowDownProperty::getClassPropertyId());
	if (property != nullptr)
	{
		// has it expired?
		SlowDownProperty * slowdown = safe_cast<SlowDownProperty *>(property);
		if (slowdown->getExpireTime() <= ServerClock::getInstance().getGameTimeSeconds())
		{
			// yes, remove the effect
			removeProperty(SlowDownProperty::getClassPropertyId());
		}
		else
		{
			// we need to find every authoritative non-player creature in the effect's
			// area, and and tell them they are moving on our effect "hill" during their next alter
			// (player creatures are handled on the player's client)
			Object * target = slowdown->getTarget().getObject();
			if (target != nullptr)
			{
				std::vector<ServerObject *> found;
				ServerWorld::findAuthoritativeNonPlayerCreaturesInCone(*this, *target, slowdown->getConeLength(), slowdown->getConeAngle(), found);
				if (!found.empty())
				{
					// compute the effective normal vector of our "hill"
					Vector normal(getPosition_w() - target->getPosition_w());
					normal.y = 0;
					normal.normalize();
					normal *= cos(slowdown->getSlopeAngle() + PI_OVER_2);
					normal.y = sin(slowdown->getSlopeAngle() + PI_OVER_2);
					for (std::vector<ServerObject *>::iterator i = found.begin(); i != found.end(); ++i)
					{
						if ((*i)->getNetworkId() != getNetworkId())
							safe_cast<CreatureObject*>(*i)->addTerrainSlopeEffect(normal);
					}
				}
			}
		}
	}

	//-- Handle mounts-specific functionality for all mounts.  We'll do sanity checking in here.
	alterAnyForMounts();

	//-- Handle updateMovementInfo.
	if (getLocalFlag(LocalObjectFlags::CreatureObject_RequiresMovementInfoUpdate))
	{
		updateMovementInfo();
		setLocalFlag(LocalObjectFlags::CreatureObject_RequiresMovementInfoUpdate, false);
	}

	m_locomotion = calcLocomotion();

	if (PlayerCreatureController::getPlayerObject(this) != 0)
	{
		if (m_level.get() != m_previousLevel)
		{
			if (isAuthoritative())
			{
				GameScriptObject * const script = getScriptObject();

				if (script != 0)
				{
					ScriptParams params;
					params.addParam(m_previousLevel);
					params.addParam(m_level.get());
					script->trigAllScripts(Scripting::TRIG_COMBAT_LEVEL_CHANGED, params);
				}
			}
			m_previousLevel = m_level.get();
			levelChanged();
		}
	}

	return result;
}

//-----------------------------------------------------------------------

bool CreatureObject::canDestroy() const
{
	// turn off our default weapon so it won't prevent us from getting
	// destroyed
	WeaponObject * defaultWeapon = getDefaultWeapon();
	if (defaultWeapon != nullptr)
		defaultWeapon->setAsDefaultWeapon(false);

	bool result = TangibleObject::canDestroy();
	if (!result)
	{
		// we're not being destroyed, turn our default weapon back on
		if (defaultWeapon != nullptr)
			defaultWeapon->setAsDefaultWeapon(true);
	}

	return result;
}

//-----------------------------------------------------------------------

float CreatureObject::getPvpRegionCheckTime()
{
	return ConfigServerGame::getCreaturePvpRegionCheckTime();
}

//-----------------------------------------------------------------------

/**
 * Sets the default weapon for a creature. This function bootstraps the default weapon.
 * After this function is called, there should always be a weapon in the default
 * weapon slot.
 */
void CreatureObject::initializeDefaultWeapon()
{
	FATAL(!isAuthoritative(), ("CreatureObject::initializeDefaultWeapon: obj %s, while nonauth", getDebugInformation().c_str()));

	ServerCreatureObjectTemplate const * const myTemplate = safe_cast<ServerCreatureObjectTemplate const *>(getObjectTemplate());
	if (myTemplate != nullptr)
	{
		ServerWeaponObjectTemplate const *weaponTemplate = myTemplate->getDefaultWeapon();
		if (weaponTemplate == nullptr)
		{
			WARNING(true, ("Creature template %s has no valid default weapon!", getTemplateName()));

			// try to use the fallback weapon
			weaponTemplate = dynamic_cast<ServerWeaponObjectTemplate const *>(ObjectTemplateList::fetch(ConfigServerGame::getFallbackDefaultWeapon()));

			FATAL(weaponTemplate == nullptr, ("CreatureObject::initializeFirstTimeObject fallback weapon template %s is bad!", ConfigServerGame::getFallbackDefaultWeapon()));
		}

		WeaponObject * const weapon = safe_cast<WeaponObject *>(ServerWorld::createNewObject(*weaponTemplate, *this, s_defaultWeaponSlotId, false));

		if (weapon != nullptr)
		{
			weapon->setAsDefaultWeapon(true);
		}
#ifdef _DEBUG
		else
		{
			WARNING_STRICT_FATAL(true, ("CreatureObject::initializeDefaultWeapon unable to create default weapon %s for creature %s", weaponTemplate->getName(), getNetworkId().getValueString().c_str()));
		}
#endif
		weaponTemplate->releaseReference();
	}
}

// ----------------------------------------------------------------------

bool CreatureObject::swapDefaultWeapons(WeaponObject &newDefaultWeapon, ServerObject &weaponContainer)
{
	FATAL(!isAuthoritative(), ("CreatureObject::swapDefaultWeapons: obj %s, weapon %s, weaponContainer %s, while obj nonauth", getDebugInformation().c_str(), newDefaultWeapon.getDebugInformation().c_str(), weaponContainer.getDebugInformation().c_str()));
	FATAL(!newDefaultWeapon.isAuthoritative(), ("CreatureObject::swapDefaultWeapons: obj %s, weapon %s, weaponContainer %s, while newDefaultWeapon nonauth", getDebugInformation().c_str(), newDefaultWeapon.getDebugInformation().c_str(), weaponContainer.getDebugInformation().c_str()));
	FATAL(!weaponContainer.isAuthoritative(), ("CreatureObject::swapDefaultWeapons: obj %s, weapon %s, weaponContainer %s, while weaponContainer nonauth", getDebugInformation().c_str(), newDefaultWeapon.getDebugInformation().c_str(), weaponContainer.getDebugInformation().c_str()));

	// There is a window here where the default weapon can be nullptr, so we
	// set a flag that it's ok until we've finished the transfer.
	FATAL(s_allowNullDefaultWeapon, ("CreatureObject::swapDefaultWeapons has been recursively called!"));

	WeaponObject * const oldDefaultWeapon = getDefaultWeapon();
	FATAL(!oldDefaultWeapon, ("CreatureObject::swapDefaultWeapons: obj %s, weapon %s, weaponContainer %s, with no current default weapon", getDebugInformation().c_str(), newDefaultWeapon.getDebugInformation().c_str(), weaponContainer.getDebugInformation().c_str()));

	s_allowNullDefaultWeapon = true;

	// Move the current default weapon into the weapon container
	oldDefaultWeapon->setAsDefaultWeapon(false);
	Container::ContainerErrorCode err = Container::CEC_Success;
	bool const success1 = ContainerInterface::transferItemToVolumeContainer(weaponContainer, *oldDefaultWeapon, 0, err, true);
	FATAL(!success1, ("CreatureObject::swapDefaultWeapons: obj %s, old default weapon %s, new default weapon %s, weaponContainer %s, old default weapon transfer to weaponContainer failed with error %d", getDebugInformation().c_str(), oldDefaultWeapon->getDebugInformation().c_str(), newDefaultWeapon.getDebugInformation().c_str(), weaponContainer.getDebugInformation().c_str(), static_cast<int>(err)));

	// Try to move the new default weapon into the default weapon slot
	bool const success2 = ContainerInterface::transferItemToSlottedContainer(*this, newDefaultWeapon, s_defaultWeaponSlotId, 0, err);
	if (success2)
		newDefaultWeapon.setAsDefaultWeapon(true);
	else
	{
		// We failed to swap in the new default weapon, so we need to move the old one back in.
		bool const success3 = ContainerInterface::transferItemToSlottedContainer(*this, *oldDefaultWeapon, s_defaultWeaponSlotId, 0, err);
		FATAL(!success3, ("CreatureObject::swapDefaultWeapons: obj %s, old default weapon %s, new default weapon %s, weaponContainer %s, old default weapon transfer back to default weapon slot after failure to switch to new default weapon failed with error %d", getDebugInformation().c_str(), oldDefaultWeapon->getDebugInformation().c_str(), newDefaultWeapon.getDebugInformation().c_str(), weaponContainer.getDebugInformation().c_str(), static_cast<int>(err)));
		oldDefaultWeapon->setAsDefaultWeapon(true);
	}

	s_allowNullDefaultWeapon = false;
	return success2;
}

// ----------------------------------------------------------------------

WeaponObject *CreatureObject::getDefaultWeapon() const
{
	SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);
	if (container == nullptr)
	{
		return nullptr;
	}

	WeaponObject * defaultWeapon = nullptr;
	Container::ContainerErrorCode error;
	Container::ContainedItem itemId = container->getObjectInSlot(s_defaultWeaponSlotId, error);
	if (error == Container::CEC_Success)
	{
		Object * const o = itemId.getObject();
		if (o)
		{
			ServerObject * const so = o->asServerObject();
			if (so)
				defaultWeapon = so->asWeaponObject();
		}
		FATAL(!s_allowNullDefaultWeapon && defaultWeapon == nullptr, ("CreatureObject::getDefaultWeapon, weapon is nullptr! Object in default slot is %s", itemId.getValueString().c_str()));
	}
	return defaultWeapon;
}

// ----------------------------------------------------------------------

/**
 * Returns an attribute value, modified by attribute mods.
 *
 * @param attribute		the attribute to get
 */
Attributes::Value CreatureObject::getAttribute(Attributes::Enumerator attribute) const
{
	if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
	{
		DEBUG_REPORT_LOG(true, ("attribute out of range (%d)\n", attribute));
		return 0;
	}

	return getAdjustedAttribute(attribute, m_attributes[attribute]);
}

//-----------------------------------------------------------------------

/**
 * Returns an attribute value, adjusted by wounds, etc.
 *
 * @param attribute		the attribute type
 * @param value			the value to adjust
 *
 * @returns value adjusted by various params
 */
Attributes::Value CreatureObject::getAdjustedAttribute(Attributes::Enumerator attribute,
	Attributes::Value value) const
{
	if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
	{
		DEBUG_REPORT_LOG(true, ("attribute out of range (%d)\n", attribute));
		return value;
	}

	// modify the attribute for attrib mods
	value = static_cast<Attributes::Value>(
		value + m_cachedCurrentAttributeModValues[attribute]);

	// modify for partial regeneration
	value = static_cast<Attributes::Value>(value + static_cast<int>(floor(m_regeneration[attribute])));

	//cap maximum value
	Attributes::Value max = getMaxAttribute(attribute);
	if (value > max)
		value = max;

	return value;
}	// CreatureObject::getAdjustedAttribute

//-----------------------------------------------------------------------

/**
 * Returns a max attribute value, modified by attribute mods.
 *
 * @param attribute		the attribute to get
 */
Attributes::Value CreatureObject::getMaxAttribute(Attributes::Enumerator attribute, bool capStat) const
{
	if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
	{
		DEBUG_REPORT_LOG(true, ("attribute out of range (%d)\n", attribute));
		return 0;
	}

	Attributes::Value value = getUnmodifiedMaxAttribute(attribute);
	value = static_cast<Attributes::Value>(
		value + m_cachedMaxAttributeModValues[attribute] + m_attribBonus[attribute]);

	if (capStat)
	{
		// make sure the value doesn't go below the defined minimum value
		if (Attributes::isAttribPool(attribute) && value < ConfigServerGame::getMinPoolValue())
			value = static_cast<Attributes::Value>(ConfigServerGame::getMinPoolValue());

		// double-check that the max isn't <= 0
		if (value < 1)
			value = 1;
	}
	return value;
}

//-----------------------------------------------------------------------

const std::map<std::string, std::pair<int, int> > & CreatureObject::getModMap() const
{
	return m_modMap.getMap();
}

//-----------------------------------------------------------------------

const int CreatureObject::getModValue(const std::string & modName) const
{
	int result = 0;
	std::map<std::string, std::pair<int, int> >::const_iterator f = m_modMap.find(modName);
	if(f != m_modMap.end())
	{
		result = (*f).second.first;
	}
	return result;
}

//-----------------------------------------------------------------------

/**
 * Returns the value for a skill mod, modified by any special equipment the
 * creature may be wearing.
 *
 * @param statMod		the name of the skill mod
 *
 * @return the mod value
 */
const int CreatureObject::getEnhancedModValue(const std::string & modName) const
{
	int base = 0;
	int bonus = 0;

	std::map<std::string, std::pair<int, int> >::const_iterator f = m_modMap.find(modName);
	if (f != m_modMap.end())
	{
		base = (*f).second.first;
		bonus = (*f).second.second;
	}
	if (bonus > ConfigSharedGame::getMaxCreatureSkillModBonus())
		bonus = ConfigSharedGame::getMaxCreatureSkillModBonus();
	return base + bonus;
}	// CreatureObject::getEnhancedSkillStatMod

//-----------------------------------------------------------------------

/**
 * Returns the value for a skill mod, modified by any special equipment the
 * creature may be wearing.  This version does not respect getMaxCreatureSkillModBonus
 *
 * @param statMod		the name of the skill mod
 *
 * @return the mod value
 */
const int CreatureObject::getEnhancedModValueUncapped(const std::string & modName) const
{
	int base = 0;
	int bonus = 0;

	std::map<std::string, std::pair<int, int> >::const_iterator f = m_modMap.find(modName);
	if (f != m_modMap.end())
	{
		base = (*f).second.first;
		bonus = (*f).second.second;
	}
	return base + bonus;
}	// CreatureObject::getEnhancedSkillStatMod

//-----------------------------------------------------------------------

void CreatureObject::setModValue(const std::string & modName, const int value)
{
	if(isAuthoritative())
	{
		int oldBonus = 0;
		Archive::AutoDeltaMap<std::string, std::pair<int, int> >::const_iterator found =
			m_modMap.find(modName);
		if (found != m_modMap.end())
			oldBonus = (*found).second.second;
		if (value != 0 || oldBonus != 0)
		{
			m_modMap.set(modName, std::make_pair(value, oldBonus));
		}
		else
		{
			m_modMap.erase(modName);
		}

		updateSlopeMovement(modName);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setModValue, new MessageQueueGenericValueType<std::pair<std::string, int> >(std::make_pair(modName, value)));
	}
}

//-----------------------------------------------------------------------

void CreatureObject::addModValue(const std::string & modName, const int value, bool fromSkill)
{
	int currentVal = getModValue(modName);
	setModValue(modName, currentVal + value);
	if (!fromSkill)
	{
		// update the objvars for the value
		std::string objvarName(OBJVAR_NOT_SKILL_MODS + "." + modName);
		int currentValue = 0;
		getObjVars().getItem(objvarName, currentValue);
		currentValue += value;
		if (currentValue != 0)
			setObjVarItem(objvarName, currentValue);
		else
			removeObjVarItem(objvarName);
	}
}

// ----------------------------------------------------------------------

/**
 * Adds a skill mod bonus to the creature.
 *
 * @param statMod		the skill mod name
 * @param bonus			the bonus value
 */
void CreatureObject::addModBonus(const std::string & modName, int bonus)
{
	if(isAuthoritative())
	{
		int oldBase = 0;
		int oldBonus = 0;
		Archive::AutoDeltaMap<std::string, std::pair<int, int> >::const_iterator found =
			m_modMap.find(modName);
		if (found != m_modMap.end())
		{
			oldBase = (*found).second.first;
			oldBonus = (*found).second.second;
		}
		bonus += oldBonus;
		if (bonus != 0 || oldBase != 0)
		{
			m_modMap.set(modName, std::make_pair(oldBase, bonus));
		}
		else
		{
			m_modMap.erase(modName);
		}

		updateSlopeMovement(modName);
	}
	else
	{
//		sendControllerMessageToAuthServer(CM_setModValue, new MessageQueueGenericValueType<std::pair<std::string, int> >(std::make_pair(modName, value)));
	}
}	// CreatureObject::addModBonus

// ----------------------------------------------------------------------

/**
 * Adds an attrib bonus to the creature.
 *
 * @param attrib		the attribute
 * @param bonus			the bonus value
 */
void CreatureObject::addAttribBonus(int attrib, int bonus)
{
	if (isAuthoritative())
	{
		if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
			return;

		int newBonus = m_attribBonus[attrib] + bonus;
		if (newBonus > ConfigServerGame::getMaxTotalAttribBonus())
			newBonus = ConfigServerGame::getMaxTotalAttribBonus();
		else if (newBonus < -ConfigServerGame::getMaxTotalAttribBonus())
			newBonus = -ConfigServerGame::getMaxTotalAttribBonus();
		m_attribBonus.set(attrib, newBonus);
	}
}	// CreatureObject::addAttribBonus

// ----------------------------------------------------------------------

void CreatureObject::updateSlopeMovement(const std::string & modName)
{
	// if I'm modifiying the slope mod, update my speed
	if (modName == SLOPE_MOD)
		recomputeSlopeModPercent();
	// if I'm modifiying the group slope mod and I'm a group leader,
	// update my group's speed
	else if (modName == GROUP_SLOPE_MOD && getGroup() != nullptr &&
		getGroup()->getGroupLeaderId() == getNetworkId())
	{
		const GroupObject::GroupMemberVector & members = getGroup()->getGroupMembers();
		GroupObject::GroupMemberVector::const_iterator iter;
		for (iter = members.begin(); iter != members.end(); ++iter)
		{
			CreatureObject * member = safe_cast<CreatureObject *>(
				NetworkIdManager::getObjectById((*iter).first));
			if (member != nullptr)
				member->recomputeSlopeModPercent();
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Cause the client to dance using the specified dance style.
 *
 * This function will change the server and clients to the SkillAnimating
 * posture.
 *
 * @param enabled     if true, start doing the skill.  If false, stop doing the skill.
 * @param danceStyle  the number of the dance style.
 *
 * @return  true if this function succeeded, false if it failed for any reason.
 */

bool CreatureObject::showDanceVisuals(bool enabled)
{
	//-- Ensure we are authoratative.  This function should only be called
	//   indirectly via the command table, which guarantees it is called on the
	//   authoratative object.
	if (!isAuthoritative())
	{
		DEBUG_WARNING(true, ("showDanceVisuals() should be called only on the authoratative object."));
		return false;
	}

	if (!enabled)
	{
		//-- Set the server object to the upright posture.
		setPosture(Postures::Upright);

		//-- Tell all clients to switch to the specified posture.
		Controller *const controller = getController();
		if (controller)
			controller->appendMessage(CM_setPosture, 0.0f, new MessageQueuePosture(Postures::Upright, true), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_ALL_CLIENT);
		else
			DEBUG_WARNING(true, ("showDanceVisuals(): no controller, could not tell clients to go to SkillAnimating posture."));
	}
	else
	{
		int visualId = lookupDanceVisual(getPerformanceType());
		if (visualId == 0)
			return false;

		//-- Set the server object to animating skill posture.
		setPosture(Postures::SkillAnimating);

		//-- Tell all clients to switch to the specified posture.
		Controller *const controller = getController();
		if (controller)
			controller->appendMessage(CM_setPosture, 0.0f, new MessageQueuePosture(Postures::SkillAnimating, true), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_ALL_CLIENT);
		else
			DEBUG_WARNING(true, ("showDanceVisuals(): no controller, could not tell clients to go to SkillAnimating posture."));

		//-- Modify the animating skill data string.
		char buffer[64];

		sprintf(buffer, "dance_%d", visualId);
		m_animatingSkillData = buffer;
	}

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool CreatureObject::showMusicianVisuals(bool enabled)
{
	//-- Ensure we are authoratative.  This function should only be called
	//   indirectly via the command table, which guarantees it is called on the
	//   authoratative object.
	if (!isAuthoritative())
	{
		DEBUG_WARNING(true, ("showMusicianVisuals() should be called only on the authoratative object."));
		return false;
	}

	if (!enabled)
	{
		//-- Set the server object to the upright posture.
		setPosture(Postures::Upright);

		//-- Tell all clients to switch to the specified posture.
		Controller *const controller = getController();
		if (controller)
			controller->appendMessage(CM_setPosture, 0.0f, new MessageQueuePosture(Postures::Upright, true), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_ALL_CLIENT);
		else
			DEBUG_WARNING(true, ("showDanceVisuals(): no controller, could not tell clients to go to SkillAnimating posture."));
	}
	else
	{
		//-- Determine the type of instrument used by this creature.
		// @todo
		const int instrumentId = getInstrumentVisualId();
		if (instrumentId == 0)
		{
			WARNING(true, ("tried to call showMusicianVisuals without a valid instrument"));
			return false;
		}

		//-- Set the server object to animating skill posture.
		setPosture(Postures::SkillAnimating);

		//-- Tell all clients to switch to the specified posture.
		Controller *const controller = getController();
		if (controller)
			controller->appendMessage(CM_setPosture, 0.0f, new MessageQueuePosture(Postures::SkillAnimating, true), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_ALL_CLIENT);
		else
			DEBUG_WARNING(true, ("showDanceVisuals(): no controller, could not tell clients to go to SkillAnimating posture."));

		//-- Modify the animating skill data string.
		char buffer[64];

		sprintf(buffer, "music_%d", instrumentId);
		m_animatingSkillData = buffer;
	}

	//-- Success.
	return true;
}

//-----------------------------------------------------------------------

void CreatureObject::sendMusicFlourish(int flourishIndex)
{
	Controller *controller = getController();
	if (controller)
	{
		MessageQueueGenericValueType<int> *msg = new MessageQueueGenericValueType<int>(flourishIndex);
		controller->appendMessage(
			CM_musicFlourish,
			0.0f,
			msg,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_ALL_CLIENT);
	}
}

//-----------------------------------------------------------------------

/**
 * Returns a mental state value, modified by mental state mods.
 *
 * @param attribute		the attribute to get
 */
MentalStates::Value CreatureObject::getMentalState(MentalStates::Enumerator attribute) const
{
	if (attribute < 0 || attribute >= MentalStates::NumberOfMentalStates)
	{
		DEBUG_REPORT_LOG(true, ("mental state out of range (%d)\n", attribute));
		return 0.0f;
	}

	MentalStates::Value value = static_cast<MentalStates::Value>(m_currentAttitude.get().m_currentValues[attribute]);
	return value;
}

//-----------------------------------------------------------------------

/**
 * Returns a mental state value, modified by mental state mods.
 *
 * @param attribute		the attribute to get
 */
MentalStates::Value CreatureObject::getMentalStateToward(const NetworkId &target, MentalStates::Enumerator attribute) const
{
	if (attribute < 0 || attribute >= MentalStates::NumberOfMentalStates)
	{
		DEBUG_REPORT_LOG(true, ("mental state out of range (%d)\n", attribute));
		return 0.0f;
	}

	MentalStates::Value value = static_cast<MentalStates::Value>(m_currentAttitude.get().m_currentValues[attribute]);

	CreatureAttitude const * const cs = getAttitudeToward(target);
	if (cs)
	{
		value = static_cast<MentalStates::Value>(value + cs->m_currentValues[attribute]);
	}
	return value;
}

//-----------------------------------------------------------------------

int CreatureObject::getExperiencePoints(const std::string & experienceType) const
{
	if (isPlayerControlled())
	{
		const PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject != nullptr)
			return playerObject->getExperiencePoints(experienceType);
	}
	return 0;
}

//-----------------------------------------------------------------------

const std::map<std::string, int> & CreatureObject::getExperiencePoints() const
{
	static const std::map<std::string, int> emptyResult;
	if(isPlayerControlled())
	{
		const PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if(playerObject != nullptr)
		{
			return playerObject->getExperiencePoints();
		}
	}
	return emptyResult;
}

//-----------------------------------------------------------------------

const CreatureObject::SkillList & CreatureObject::getSkillList() const
{
	return m_skills.get();
}

//-----------------------------------------------------------------------

const int CreatureObject::grantExperiencePoints(const std::string & experienceType, int amount)
{
	if (isPlayerControlled())
	{
		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject != nullptr)
		{
			int const amountGranted = playerObject->grantExperiencePoints(experienceType, amount);

			LevelManager::LevelData levelData(m_level.get(), m_totalLevelXp.get());
			LevelManager::addXpToLevelData(levelData, experienceType, amountGranted);

			setLevelData(levelData.currentLevel, levelData.currentLevelXp, levelData.currentHealth);

			return amountGranted;
		}
	}
	return INT_MIN;
}

// ----------------------------------------------------------------------

const bool CreatureObject::grantSkill(const SkillObject & newSkill)
{
	if(isAuthoritative())
	{
		//-- jww: simply return true if the skill is already in the vector
		//--      this was the previous behavior, dunno if it should return false
		if (hasSkill (newSkill))
			return true;

		// signal scripts that skill is about to be granted.  If the script returns override, cancel skill grant.
		{
			ScriptParams params;
			params.addParam(newSkill.getSkillName().c_str());
			if (getScriptObject()->trigAllScripts(Scripting::TRIG_SKILL_ABOUT_TO_BE_GRANTED, params) == SCRIPT_OVERRIDE)
			{
				return false;
			}
		}

		m_skills.insert(&newSkill);
		if(getClient())
		{
			LOG("CustomerService", ("Skill: Account %s CharacterId %s has acquired skill %s", getClient()->getAccountName().c_str(), getNetworkId().getValueString().c_str(), newSkill.getSkillName().c_str()));
		}

		{
			const SkillObject::StringVector & sv = newSkill.getCommandsProvided ();
			for (SkillObject::StringVector::const_iterator i = sv.begin(); i != sv.end(); ++i)
				grantCommand(*i, true);
		}

		const std::vector<std::pair<std::string, int> > & statMods =  newSkill.getStatisticModifiers();
		std::vector<std::pair<std::string, int> >::const_iterator modIter;
		for(modIter = statMods.begin(); modIter != statMods.end(); ++ modIter)
		{
			addModValue((*modIter).first, (*modIter).second, true);
		}

		{
			const SkillObject::StringVector & sv = newSkill.getSchematicsGranted ();
			for (SkillObject::StringVector::const_iterator i = sv.begin(); i != sv.end(); ++i)
				grantSchematicGroup("+" + *i, true);
		}

		// signal scripts that skill has been granted
		ScriptParams params;
		params.addParam(newSkill.getSkillName().c_str());
		IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_SKILL_GRANTED,
			params));

		LevelManager::LevelData levelData(m_level.get(), m_totalLevelXp.get());
		LevelManager::addSkillToLevelData(levelData, newSkill.getSkillName());

		setLevelData(levelData.currentLevel, levelData.currentLevelXp, levelData.currentHealth);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_grantSkill, new MessageQueueGenericValueType<std::string>(newSkill.getSkillName()));
	}

	return true;
}

//-----------------------------------------------------------------------

void CreatureObject::revokeSkill(const SkillObject & oldSkill, bool silent)
{
	if(isAuthoritative())
	{
		// signal scripts that skill has been revoked
		ScriptParams params;
		params.addParam(oldSkill.getSkillName().c_str());
		if(getScriptObject()->trigAllScripts(Scripting::TRIG_SKILL_ABOUT_TO_BE_REVOKED, params) == SCRIPT_CONTINUE)
		{

			for (SkillList::const_iterator it = m_skills.begin(); it != m_skills.end(); ++it)
			{
				const SkillObject * const skill = *it;

				if (skill)
				{
					const SkillObject::SkillVector &prerequisiteSkills = skill->getPrerequisiteSkills();

					for (SkillObject::SkillVector::const_iterator pit = prerequisiteSkills.begin(); pit != prerequisiteSkills.end(); ++pit)
					{
						const SkillObject * const prereq = *pit;
						NOT_NULL (prereq);

						if (oldSkill.getSkillName() == prereq->getSkillName())
						{
							if (!silent)
							Chat::sendSystemMessage(*this, SharedStringIds::revoke_dependant_skill, Unicode::emptyString);
							return;
						}
					}
				}
			}

			if (m_skills.contains(&oldSkill))
			{
				// remove granted commands
				{
					const SkillObject::StringVector & sv = oldSkill.getCommandsProvided ();
					for (SkillObject::StringVector::const_iterator i = sv.begin(); i != sv.end(); ++i)
						revokeCommand(*i, true);
				}

				// remove granted skill mods
				const std::vector<std::pair<std::string, int> > & statMods =  oldSkill.getStatisticModifiers();
				std::vector<std::pair<std::string, int> >::const_iterator modIter;
				for(modIter = statMods.begin(); modIter != statMods.end(); ++ modIter)
				{
					addModValue((*modIter).first, -(*modIter).second, true);
				}

				// remove schematics that were granted by any schematic groups
				{
					const SkillObject::StringVector & sv = oldSkill.getSchematicsGranted ();
					for (SkillObject::StringVector::const_iterator i = sv.begin(); i != sv.end(); ++i)
					{
						const std::string & grantedGroup = *i;
						grantSchematicGroup("-" + grantedGroup, true);
					}
				}

				m_skills.erase(&oldSkill);
				if(getClient())
				{
					LOG("CustomerService", ("Skill: Account %s CharacterId %s has lost skill %s", getClient()->getAccountName().c_str(), getNetworkId().getValueString().c_str(), oldSkill.getSkillName().c_str()));
				}

				// Remove the current skill title if this is a player and their current title
				// is this skill

				PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);

				if ((playerObject != nullptr) &&
					(oldSkill.getSkillName() == playerObject->getTitle()))
				{
					StringId message("shared", "skill_title_removed");
					Unicode::String outOfBand;
					if (!silent)
					Chat::sendSystemMessage(*this, message, outOfBand);

					playerObject->setTitle("");
				}

				// signal scripts that skill has been revoked
				ScriptParams params;
				params.addParam(oldSkill.getSkillName().c_str());
				IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_SKILL_REVOKED,params));

				// update the level data
				LevelManager::LevelData levelData(m_level.get(), m_totalLevelXp.get());
				LevelManager::removeSkillFromLevelData(levelData, oldSkill.getSkillName());

				setLevelData(levelData.currentLevel, levelData.currentLevelXp, levelData.currentHealth);
			}
		}
	}
	else
	{
		LOG("CustomerService", ("Skill: Could not revoke skill %s from character %s because it is a proxy so we're forwarding this to the auth server.",
			oldSkill.getSkillName().c_str(), getNetworkId().getValueString().c_str()));
		sendControllerMessageToAuthServer(CM_revokeSkill, new MessageQueueGenericValueType<std::string>(oldSkill.getSkillName()));
	}
}

//-----------------------------------------------------------------------

const bool CreatureObject::grantSchematicGroup(const std::string & groupNameWithModifier, bool fromSkill)
{
	if (isPlayerControlled())
	{
		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject != nullptr)
			return playerObject->grantSchematicGroup(groupNameWithModifier, fromSkill);
	}
	return false;
}

//-----------------------------------------------------------------------

const bool CreatureObject::grantSchematic(const std::string & schematicName, bool fromSkill)
{
	return grantSchematic(Crc::calculate(schematicName.c_str()), fromSkill);
}

//-----------------------------------------------------------------------

const bool CreatureObject::grantSchematic(uint32 schematicCrc, bool fromSkill)
{
	if (isPlayerControlled())
	{
		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject != nullptr)
			return playerObject->grantSchematic(schematicCrc, fromSkill);
	}
	return false;
}

//-----------------------------------------------------------------------

const bool CreatureObject::revokeSchematic(const std::string & schematicName, bool fromSkill)
{
	return revokeSchematic(Crc::calculate(schematicName.c_str()), fromSkill);
}

//-----------------------------------------------------------------------

const bool CreatureObject::revokeSchematic(uint32 schematicCrc, bool fromSkill)
{
	if (isPlayerControlled())
	{
		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject != nullptr)
			return playerObject->revokeSchematic(schematicCrc, fromSkill);
	}
	return false;
}

//-----------------------------------------------------------------------

const bool CreatureObject::hasSchematic(const std::string & schematicName)
{
	return hasSchematic(Crc::calculate(schematicName.c_str()));
}

//-----------------------------------------------------------------------

const bool CreatureObject::hasSchematic(uint32 schematicCrc)
{
	if (isPlayerControlled())
	{
		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject != nullptr)
			return playerObject->hasSchematic(schematicCrc);
	}
	return false;
}

//-----------------------------------------------------------------------

const bool CreatureObject::hasSkill(const SkillObject & skill) const
{
	return std::find (m_skills.begin(), m_skills.end (), &skill) != m_skills.end();
}

//-----------------------------------------------------------------------

/**
 * Sets up the object for being in or out of combat.
 * @todo: this should really be in tangible only, we need to have a command queue
 * for tangibles
 *
 * @param inCombat		flag to put the object in combat-ready state or not
 *
 * @return true if state was set, false if something went wrong
 */
void CreatureObject::setInCombat(bool inCombat)
{
	TangibleObject::setInCombat(inCombat);

	if (isAuthoritative())
	{
		if (!inCombat)
		{
			setState(States::Aiming, false);
		}
		else
		{
			// if we are crafting, end the crafting session

			PlayerObject * const player = PlayerCreatureController::getPlayerObject(this);

			if (player != nullptr && player->isCrafting())
			{
				player->stopCrafting(false);
			}
		}
	}
}

//-----------------------------------------------------------------------

/**
 * If this object is in combat, adds an aim to it.
 */
void CreatureObject::addAim ()
{
	TangibleObject::addAim();
	if (isInCombat())
	{
		setState(States::Aiming, true);
	}
}	// CreatureObject::addAim

//-----------------------------------------------------------------------

/**
 * If this object is in combat, clear's it's aims.
 */
void CreatureObject::clearAims ()
{
	TangibleObject::clearAims();
	if (isInCombat())
	{
		setState(States::Aiming, false);
	}
}	// CreatureObject::clearAims

//-----------------------------------------------------------------------

/**
 * Modifies a creature's attribute. No other function should modify the pool indexes
 * of m_attributes. Will incapacitate the creature if an attribute drops to 0.
 *
 * @param attrib				the attribute to modify
 * @param delta					amount to modify the attribute by
 * @param checkIncapacitation	flag to check for incap/recap
 * @param source				who is doing the damage (NetworkId::cms_invalid
 *								for no one)
 * @param force                 force the change, even if we would normally not
 *                              make it
 *
 * @return the amount of damage actually applied
 */
int CreatureObject::alterAttribute(Attributes::Enumerator attrib, int delta,
	bool checkIncapacitation, const NetworkId & source, bool force)
{
	if (isAuthoritative())
	{
		if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		{
			DEBUG_REPORT_LOG(true, ("attribute out of range (%d)\n", attrib));
			return 0;
		}

		if (!Attributes::isAttribPool(attrib))
		{
			// we only alter pool values now, regeneration is set directly
			DEBUG_REPORT_LOG(true, ("CreatureObject::alterAttribute called on %s for non-pool attrib (%d)\n",
				getNetworkId().getValueString().c_str(), attrib));
			return 0;
		}

		if (delta == 0)
			return 0;

		// Use TangibleObject::isInvulnerable so we don't consider invulnerability timers
		if ((isDead() || TangibleObject::isInvulnerable()) && !force)
			return 0;

		if (getMaxAttribute(attrib) <= 0 && !force)
			return 0;

		// if we are taking damage, try and damage any attrib mods instead
		int attribModChange = 0;
		int regenChange = 0;
		if (delta < 0)
		{
			attribModChange = changeAttribModCurrentValue(attrib, delta, false);
			if (attribModChange == delta)
			{
				computeTotalAttributes();
				if (checkIncapacitation)
					testIncapacitation(source);
				return attribModChange;
			}
			else
			{
				delta -= attribModChange;
			}

			// if we are still taking damage, try and reduce any accumulated
			// regeneration points
			if (delta < 0 && m_regeneration[attrib] > 0)
			{
				if (m_regeneration[attrib] >= -delta)
				{
					// regen can absorb it all
					m_regeneration[attrib] += delta;
					computeTotalAttributes();
					if (checkIncapacitation)
						testIncapacitation(source);
					return attribModChange + delta;
				}
				else
				{
					regenChange = -static_cast<int>(floor(m_regeneration[attrib]));
					m_regeneration[attrib] += regenChange;
					delta -= regenChange;
				}
			}
		}

		// Don't set the actual attribute until the end of this function,
		// to avoid changing it multiple times and generating network spam
		int newValue = m_attributes[attrib] + delta;

		// if the attribute is more than the max attribute, reduce it
		int maxAttribLimit = getUnmodifiedMaxAttribute(attrib) + getAttributeBonus(attrib);
		if (newValue > maxAttribLimit)
		{
			int overflow = newValue - maxAttribLimit;
			delta -= overflow;
			newValue = maxAttribLimit;

			// try and heal any attrib mods
			attribModChange = changeAttribModCurrentValue(attrib, overflow, false);
		}

		// cap for min attrib value, depending on attrib type
		int minValue = 0;
		if (isPlayerControlled())
		{
			if (Attributes::isAttribPool(attrib))
				minValue = ConfigServerGame::getMinPoolValue();
		}
		int adjustedNewValue = getAdjustedAttribute(attrib, static_cast<
			Attributes::Value>(newValue));
		if (adjustedNewValue < minValue)
		{
			newValue = minValue - (adjustedNewValue - newValue);
			adjustedNewValue = minValue;
#ifdef _DEBUG
			adjustedNewValue = getAdjustedAttribute(attrib, static_cast<
				Attributes::Value>(newValue));
			if (adjustedNewValue != minValue)
			{
				DEBUG_WARNING(true, ("CreatureObject::alterAttribute, "
					"adjustedNewValue != minValue"));
			}
#endif
		}

		if (newValue != m_attributes[attrib])
		{
			// clip delta so whoever did the damage won't get overkill xp
			if (adjustedNewValue < 0)
				adjustedNewValue = 0;
			delta = adjustedNewValue - getAttribute(attrib);

			// change the attribute
			m_attributes.set(attrib, static_cast<Attributes::Value>(newValue));

			// update the client and check for incapacitation
			computeTotalAttributes();
			if (checkIncapacitation)
				testIncapacitation(source);
		}
		else
			delta = 0;

		return delta + attribModChange + regenChange;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_alterAttribute,
			new MessageQueueAlterAttribute(attrib, delta, checkIncapacitation,
			source));
		return 0;
	}
}	// CreatureObject::alterAttribute

//-----------------------------------------------------------------------

/**
 * Tests to see if we need to be incapacitated/recapacitated.
 *
 * @param attackerId	the object that may have caused our incapacitation.
 */
void CreatureObject::testIncapacitation(const NetworkId & attackerId)
{
	WARNING_STRICT_FATAL(!isAuthoritative(), ("CreatureObject::testIncapacitation invoked on non-authoritative object %s:%s", getObjectTemplateName(), getNetworkId().getValueString().c_str()));

	if (m_deferComputeTotalAttributes <= 0)
	{
		int const health = getAttribute(Attributes::Health);

		if (!isIncapacitated())
		{
			// see if we are incapacitated

			if (health <= 0)
			{
				setIncapacitated(true, attackerId);
			}
		}
		else
		{
			// recapacitate if any attribute is above 0

			if (health > 0)
			{
				setIncapacitated(false, attackerId);
			}
		}
	}
}

//-----------------------------------------------------------------------

/**
 * Applies an attribute modifier to this creature.
 *
 * @param name          the attrib mod name
 * @param attrib		the attribute to modify
 * @param value			the value to apply
 * @param duration		how long the modifier lasts
 * @param attackRate	time to get to the peak modifier value
 * @param decayRate		time for the modifier to be reduced to 0
 * @param flags         flags that define how the mod works
 */
void CreatureObject::addAttributeModifier(const std::string & name,
	Attributes::Enumerator attrib, int value, float duration, float attackRate,
	float decayRate, int flags)
{
	AttribMod::AttribMod mod;

	if (!name.empty())
	{
		AttribModNameManager::getInstance().addAttribModName(name.c_str());
		mod.tag = Crc::calculate(name.c_str());
	}
	else
		mod.tag = 0;
	mod.attrib = attrib;
	mod.value = value;
	mod.attack = attackRate;
	mod.sustain = duration;
	mod.decay = decayRate;
	mod.flags = flags;
	if (!Attributes::isAttribPool(mod.attrib))
	{
		// non-pool mods must change both max and current
		mod.flags |=
			AttribMod::AMF_changeMax |
			AttribMod::AMF_attackCurrent |
			AttribMod::AMF_decayCurrent;
	}
	LOG("CustomerService", ("Attribs: Added attrib mod to %s. Mod data: "
		"%s(%u) %d %d %.2f %.2f %.2f %d",
		PlayerObject::getAccountDescription(this).c_str(),
		name.c_str(), mod.tag, mod.attrib, mod.value, mod.attack, mod.sustain,
		mod.decay, mod.flags));
	addAttributeModifier(mod);
}	// CreatureObject::addAttributeModifier

//-----------------------------------------------------------------------

/**
 * Applies a skillmod modifier to this creature.
 *
 * @param name          the attrib mod name
 * @param skill 		the skillmod to modify
 * @param value			the value to apply
 * @param duration		how long the modifier lasts (-1 = infinite, or until the player logs off)
 * @param flags         flags that define how the mod works
 */
void CreatureObject::addSkillmodModifier(const std::string & name,
	const std::string & skill, int value, float duration, int flags)
{
	AttribMod::AttribMod mod;

	if (skill.empty())
	{
		WARNING_STRICT_FATAL(true, ("CreatureObject::addSkillmodModifier called "
			"with empty string for skill"));
		return;
	}

	if (!name.empty())
	{
		AttribModNameManager::getInstance().addAttribModName(name.c_str());
		mod.tag = Crc::calculate(name.c_str());
	}
	else
		mod.tag = 0;

	AttribModNameManager::getInstance().addAttribModName(skill.c_str());
	mod.skill = Crc::calculate(skill.c_str());
	mod.value = value;
	mod.attack = 0;
	mod.sustain = duration;
	mod.decay = 0;
	mod.flags = flags | AttribMod::AMF_skillMod;

	LOG("CustomerService", ("Attribs: Added skill mod to %s. Mod data: "
		"%s(%u) %s(%u) %d %.2f %d",
		PlayerObject::getAccountDescription(this).c_str(),
		name.c_str(), mod.tag, skill.c_str(), mod.skill,
		mod.value, mod.sustain, mod.flags));
	addAttributeModifier(mod);
}	// CreatureObject::addSkillmodModifier

//-----------------------------------------------------------------------

/**
 * Applies an attribute modifier to this creature.
 *
 * @param mod			the mod to add
 * @param attacker		who is causing the mod to be applied
 *
 * @return the amount the attrib was modified by, if the mod did direct damage
 */
int CreatureObject::addAttributeModifier(const AttribMod::AttribMod & mod,
	ServerObject * attacker)
{
static char internalTagBuf[] = {'_','_','i','n','t','e','r','n','a','l','T','a','g','N','a','m','e','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','\0'};
static const int internalTagBufLen = strlen(internalTagBuf);

	int damage = 0;

	if (isAuthoritative())
	{
		++m_deferComputeTotalAttributes;

		if (AttribMod::isDirectDamage(mod))
		{
			// check if we're making a potentially dangerous mod
			if (!Attributes::isAttribPool(mod.attrib))
			{
 				DEBUG_WARNING(true, ("Creature %s is having non-pool attribute %d instantly "
					"modified! This change will be permanent!",
					getNetworkId().getValueString().c_str(), mod.attrib));
			}

			// apply instantanious damage directly
#ifdef _DEBUG
			if (ConfigCombatEngine::getDebugDamage())
			{
				char debugBuffer[1024];
				snprintf(debugBuffer, 1024, "creature %s attribute %d(%d) modified by %d to %d\n",
					getNetworkId().getValueString().c_str(),
					mod.attrib,
					m_attributes[mod.attrib],
					mod.value,
					m_attributes[mod.attrib] + mod.value);
				DEBUG_REPORT_LOG (true, ("%s", debugBuffer));
				if (attacker != nullptr)
				{
					Client *client = attacker->getClient();
					if (client != nullptr)
						ConsoleMgr::broadcastString(debugBuffer, client);
				}
			}
#endif

			damage = alterAttribute(mod.attrib, mod.value, false,
				attacker ? attacker->getNetworkId() : NetworkId::cms_invalid);
		}
		else if (AttribMod::isAttribMod(mod) && mod.decay == AttribMod::AMDS_antidote)
		{
			removeAttributeModifiers(mod.attrib);
		}
		else
		{
			if (mod.attack >= 0 && (mod.sustain >= 0 || mod.sustain == -1.0f))
			{
				// apply other damage to the attribMod list
				CreatureMod newMod;
				newMod.startTime = ServerClock::getInstance().getGameTimeSeconds();
				newMod.maxVal = 0;
				newMod.currentVal = 0;
				newMod.mod = mod;
				if (newMod.mod.tag == 0)
				{
					// we need to have some sort of tag, so we use an internal
					// name hash
					std::pair<Archive::AutoDeltaMap<uint32, CreatureMod>::const_iterator, bool> insertResult;
					do
					{
						newMod.mod.tag = Crc::calculate(&internalTagBuf[0]);
						if ((internalTagBufLen - 1) > 0) {
							char * c = &internalTagBuf[internalTagBufLen - 1];
							while (++(*c) == '9' + 1)
							{
								*c-- = '0';
							}
							insertResult = m_attributeModList.insert(newMod.mod.tag, newMod);
						}
					} while (!insertResult.second);

					// clear any flags that assume we have a valid tag.
					if (newMod.mod.flags & AttribMod::AMF_triggerOnDone)
					{
						newMod.mod.flags &= ~AttribMod::AMF_triggerOnDone;

						DEBUG_WARNING(true, ("Creature %s has a mod flagged as "
							"trigger when done, but has no mod name.",
							getNetworkId().getValueString().c_str()));
					}
					if (newMod.mod.flags & AttribMod::AMF_visible)
					{
						newMod.mod.flags &= ~AttribMod::AMF_visible;

						DEBUG_WARNING(true, ("Creature %s received a mod flagged as "
							"visible, but has no mod name.",
							getNetworkId().getValueString().c_str()));
					}
				}
				else
				{
					m_attributeModList.insert(mod.tag, newMod);
				}

				if ((mod.flags & AttribMod::AMF_visible) && isPlayerControlled())
				{
					// send the mod name and the mod time length to the player
					// NOTE: we just send the tag value to the client, we expect
					// the client to get the mod name from a datatable
					float time = mod.attack;
					if (mod.decay > 0)
						time += mod.decay;
					if (mod.sustain > 0)
						time += mod.sustain;
					else if (mod.sustain == -1.0f)
						time = -1.0f;
					sendTimedModData(mod.tag, time);
				}
			}
			else
			{
				const char * modName = AttribModNameManager::getInstance().getAttribModName(mod.tag);
				if (modName == nullptr)
					modName = "<nullptr>";
				WARNING(true, ("Creature %s received a mod %s with invalid "
					"attack(%.2f) or duration(%.2f)",
					getNetworkId().getValueString().c_str(), modName, mod.attack,
					mod.sustain));
			}
		}

		--m_deferComputeTotalAttributes;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addAttributeModifier,
			new MessageQueueGenericValueType<AttribMod::AttribMod>(mod));
	}
	return damage;
}	// CreatureObject::addAttributeModifier

//-----------------------------------------------------------------------

/**
 * Checks if the creature has a given attrib/skill mod on them.
 *
 * @param modName		the name of the mod to look for
 *
 * @return tue if the creature has the mod, false if not
 */
bool CreatureObject::hasAttribModifier(const std::string & modName) const
{
	// since the mod name may be a group name, we need to get any names
	// that may be derived from the modName
	std::vector<uint32> nameCrcs;
	AttribModNameManager::getInstance().getAttribModCrcsFromBase(modName, nameCrcs);

	// if we have any name that matches a value in nameCrcs, we can return true
	for (std::vector<uint32>::const_iterator i = nameCrcs.begin();
		i != nameCrcs.end(); ++i)
	{
		if (m_attributeModList.find(*i) != m_attributeModList.end())
			return true;
	}
	return false;
}	// CreatureObject::hasAttribModifier

//-----------------------------------------------------------------------

/**
 * Removes an attribute/skill modifier from this creature.
 *
 * @param modName		the name of the mod to look for
 */
void CreatureObject::removeAttributeModifier (const std::string & modName)
{
	// since the mod name may be a group name, we need to get any names
	// that may be derived from the modName
	std::vector<uint32> nameCrcs;
	AttribModNameManager::getInstance().getAttribModCrcsFromBase(modName, nameCrcs);

	// if we have any name that matches a value in nameCrcs, we can return true
	std::map<uint32, CreatureMod>::const_iterator f;
	for (std::vector<uint32>::const_iterator i = nameCrcs.begin();
		i != nameCrcs.end(); ++i)
	{
		f = m_attributeModList.find(*i);
		if (f != m_attributeModList.end())
		{
			const CreatureMod & m = (*f).second;

			// if the mod is visible, we need to tell the player it's being
			// removed
			if (m.mod.flags & AttribMod::AMF_visible)
			{
				sendCancelTimedMod(m.mod.tag);
			}
			if (AttribMod::isSkillMod(m.mod))
			{
				const char * skillModName = AttribModNameManager::getInstance().getAttribModName(m.mod.skill);
				if (skillModName != nullptr)
					addModValue(skillModName, -m.maxVal, true);
				else
				{
					WARNING(true, ("Unable to remove skillmod mod on player %s due to "
						"unknown skill mod crc. Mod data: %u %u %d %.2f %d",
						PlayerObject::getAccountDescription(this).c_str(),
						m.mod.tag, m.mod.skill, m.mod.value, m.mod.sustain, m.mod.flags));
				}
			}
			m_attributeModList.erase(f);
		}
	}
}	// CreatureObject::removeAttributeModifier

//-----------------------------------------------------------------------

/**
 * Removes all attribute modifiers for a given attribute from this creature.
 *
 * @param attribute		the attribute to clear
 */
void CreatureObject::removeAttributeModifiers(Attributes::Enumerator attribute)
{
	if(isAuthoritative())
	{
		DEBUG_FATAL(attribute < 0 || attribute >= Attributes::NumberOfAttributes,
			("attribute out of range\n"));

		LOG("CustomerService", ("Attribs: Removed all attrib mods affecting "
			"attrib %d from %s", attribute,
			PlayerObject::getAccountDescription(this).c_str()));

		m_cachedCurrentAttributeModValues.set(attribute, 0);
		m_cachedMaxAttributeModValues.set(attribute, 0);

		typedef Archive::AutoDeltaMap<uint32, CreatureMod>::const_iterator ModIter;
		for (ModIter i = m_attributeModList.begin(); i != m_attributeModList.end();)
		{
			if (!AttribMod::isSkillMod((*i).second.mod) &&
				(*i).second.mod.attrib == attribute)
			{
				// if the mod is visible, we need to tell the player it's being
				// removed
				if ((*i).second.mod.flags & AttribMod::AMF_visible)
				{
					sendCancelTimedMod((*i).second.mod.tag);
				}

				i = m_attributeModList.erase(i);
			}
			else
				++i;
		}

		computeTotalAttributes();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeAttributeModifiers, new MessageQueueGenericValueType<Attributes::Enumerator>(attribute));
	}
}	// CreatureObject::removeAttributeModifiers

//-----------------------------------------------------------------------

/**
 * Removes all the attribute modifiers from this creature.
 */
void CreatureObject::removeAllAttributeModifiers ()
{
int i;

	if (isAuthoritative())
	{
		for (i = 0; i < Attributes::NumberOfAttributes; ++i)
		{
			m_cachedCurrentAttributeModValues.set(i, 0);
			m_cachedMaxAttributeModValues.set(i, 0);
		}

		typedef Archive::AutoDeltaMap<uint32, CreatureMod>::const_iterator ModIter;
		for (ModIter i = m_attributeModList.begin(); i != m_attributeModList.end();)
		{
			if (!AttribMod::isSkillMod((*i).second.mod))
			{
				// if the mod is visible, we need to tell the player it's being
				// removed
				if ((*i).second.mod.flags & AttribMod::AMF_visible)
				{
					sendCancelTimedMod((*i).second.mod.tag);
				}

				i = m_attributeModList.erase(i);
			}
			else
				++i;
		}

		computeTotalAttributes();

		LOG("CustomerService", ("Attribs: removed all attrib mods from %s.",
			PlayerObject::getAccountDescription(this).c_str()));
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeAllAttributeModifiers, 0);
	}
}	// CreatureObject::removeAllAttributeModifiers

//-----------------------------------------------------------------------

/**
* Removes all the attribute and skill mod modifiers from this creature.
* Does NOT remove buffs, only the effects so be sure to manually remove
* any buffs for which you need the icons to go away.
*/
void CreatureObject::removeAllAttributeAndSkillmodMods ()
{
	if (isAuthoritative())
	{
		// remove all attrib and skill mod mods
		typedef Archive::AutoDeltaMap<uint32, CreatureMod>::const_iterator ModIter;
		for (ModIter i = m_attributeModList.begin(); i != m_attributeModList.end();)
		{
			const CreatureMod & m = (*i).second;
			if (AttribMod::isSkillMod(m.mod))
			{
				const char * skillModName = AttribModNameManager::getInstance().getAttribModName(m.mod.skill);
				if (skillModName != nullptr)
					addModValue(skillModName, -m.maxVal, true);
			}
			// if the mod is visible, we need to tell the player it's being
			// removed
			if (m.mod.flags & AttribMod::AMF_visible)
			{
				sendCancelTimedMod(m.mod.tag);
			}
			// if the mod is flagged as having a callback, call the script
			if (m.mod.flags & AttribMod::AMF_triggerOnDone)
			{
				const char * modName = AttribModNameManager::getInstance().getAttribModName(m.mod.tag);
				if (modName != nullptr)
				{
					ScriptParams params;
					params.addParam(modName);
					params.addParam(true);
					if (AttribMod::isAttribMod(m.mod))
					{
						IGNORE_RETURN(getScriptObject()->trigAllScripts(
							Scripting::TRIG_ATTRIB_MOD_DONE, params));
					}
					else if (AttribMod::isSkillMod(m.mod))
					{
						IGNORE_RETURN(getScriptObject()->trigAllScripts(
							Scripting::TRIG_SKILL_MOD_DONE, params));
					}

				}
			}

			i = m_attributeModList.erase(i);
		}
		for (int j = 0; j < Attributes::NumberOfAttributes; ++j)
		{
			m_cachedCurrentAttributeModValues.set(j, 0);
			m_cachedMaxAttributeModValues.set(j, 0);
		}
		computeTotalAttributes();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeAllAttributeAndSkillmodModifiers, 0);
	}

}

//-----------------------------------------------------------------------

/**
 * Returns a attrib/skillmod mod with a given name attached to the creature.
 *
 * @param modName		the mod to look for
 *
 * @return the mod, or nullptr if there is no mod with that name attached to us
 */
const AttribMod::AttribMod * CreatureObject::getAttributeModifier(
	const std::string & modName) const
{
	// since the mod name may be a group name, we need to get any names
	// that may be derived from the modName
	std::vector<uint32> nameCrcs;
	AttribModNameManager::getInstance().getAttribModCrcsFromBase(modName, nameCrcs);

	// if we have any name that matches a value in nameCrcs, we can return it
	for (std::vector<uint32>::const_iterator i = nameCrcs.begin();
		i != nameCrcs.end(); ++i)
	{
		std::map<uint32, CreatureMod>::const_iterator found = m_attributeModList.find(*i);
		if (found != m_attributeModList.end())
			return &((*found).second.mod);
	}
	return nullptr;
}	// CreatureObject::getAttributeModifier

//-----------------------------------------------------------------------

/**
 * Returns the attribute modifiers on this creature.
 */
const std::map<uint32, CreatureMod> & CreatureObject::getAttributeModifiers() const
{
	return m_attributeModList.getMap();
}	// CreatureObject::getAttributeModifiers

//-----------------------------------------------------------------------

/**
 * Sends info about a mod that's affecting a player to the client.
 *
 * @param id		the mod id
 * @param time		how long the mod lasts (in sec)
 * @param updateCache whether or not to update the cached list of timed mods that have been sent to the client
 */
void CreatureObject::sendTimedModData(uint32 id, float time, bool updateCache/* = true*/)
{
	if (isPlayerControlled() && getController() != nullptr && id != 0)
	{
		MessageQueueGenericValueType<std::pair<uint32, float> > * const msg =
			new MessageQueueGenericValueType<std::pair<uint32, float> >(
			std::make_pair(id, time));
		getController()->appendMessage(
			static_cast<int>(CM_modData),
			0,
			msg,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}

	if (isPlayerControlled() && isAuthoritative() && updateCache && id != 0)
	{
		const int index = m_timedMod.find(id);

		if (index >= 0)
		{
			m_timedModDuration.set(index, time);
			m_timedModUpdateTime.set(index, ServerClock::getInstance().getGameTimeSeconds());
		}
		else
		{
			m_timedMod.push_back(id);
			m_timedModDuration.push_back(time);
			m_timedModUpdateTime.push_back(ServerClock::getInstance().getGameTimeSeconds());
		}
	}
}	// CreatureObject::sendTimedModData

//-----------------------------------------------------------------------

/**
 * Tells a client to cancel a timed mod.
 *
 * @param id		the mod id
 */
void CreatureObject::sendCancelTimedMod(uint32 id)
{
	if (isPlayerControlled() && getController() != nullptr && id != 0)
	{
		MessageQueueGenericValueType<uint32> * const msg =
			new MessageQueueGenericValueType<uint32>(id);
		getController()->appendMessage(
			static_cast<int>(CM_cancelMod),
			0,
			msg,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}

	if (isPlayerControlled() && isAuthoritative() && id != 0)
	{
		const int index = m_timedMod.find(id);

		if (index >= 0)
		{
			m_timedMod.erase(index);
			m_timedModDuration.erase(index);
			m_timedModUpdateTime.erase(index);
		}
	}
}	// CreatureObject::sendCancelTimedMod

//-----------------------------------------------------------------------

/**
 * Applies damage done during combat to the creature.
 *
 * @param damageData		the damage
 */
void CreatureObject::applyDamage(const CombatEngineData::DamageData &damageData)
{
	WARNING_STRICT_FATAL(!isAuthoritative(), ("CreatureObject::applyDamageTo "
		"invoked on non-authoritative object %s:%s", getObjectTemplateName(),
		getNetworkId().getValueString().c_str()));

	if (!isAuthoritative())
		return;

	//-- vehicles get damaged like tangibles
	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		TangibleObject::applyDamage (damageData);
		return;
	}

	int healthDamage = 0;    // damage info we will send to the client
	int actionDamage = 0;    // damage info we will send to the client
	int mindDamage   = 0;    // damage info we will send to the client

	ServerObject *attacker = safe_cast<ServerObject *>(damageData.attackerId.getObject());

	// if the attacker is a player and we are not, and we are incapped/dead,
	// don't allow additional damage
	if (attacker != nullptr && attacker->isPlayerControlled() && !isPlayerControlled() &&
		(isIncapacitated() || isDead()))
	{
		return;
	}

	++m_deferComputeTotalAttributes;

	std::vector<AttribMod::AttribMod>::const_iterator iter;
	for (iter = damageData.damage.begin(); iter != damageData.damage.end(); ++iter)
	{
		const AttribMod::AttribMod &mod = *iter;
		if (mod.value == 0)
			continue;

		int damage = -addAttributeModifier(mod, attacker);
		if (damage != 0)
		{
			if (mod.attrib == Attributes::Health)
				healthDamage += damage;
			else if (mod.attrib == Attributes::Action)
				actionDamage += damage;
			else if (mod.attrib == Attributes::Mind)
				mindDamage += damage;
		}
	}

	--m_deferComputeTotalAttributes;

	computeTotalAttributes();

	// tell scripts we have taken combat damage
	if (damageData.attackerId != NetworkId::cms_invalid &&
		(healthDamage > 0 || actionDamage > 0 || mindDamage > 0))
	{
		NOT_NULL(getScriptObject());
		int damageArray[] = {healthDamage, 0, actionDamage, 0, mindDamage, 0};
		std::vector<int> damageParam(&damageArray[0], &damageArray[Attributes::NumberOfAttributes]);

		ScriptParams params;
		params.addParam(damageData.attackerId);
		params.addParam(damageData.weaponId);
		params.addParam(damageParam);
		IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_CREATURE_DAMAGED,
			params));
	}
	testIncapacitation(damageData.attackerId);
}	// CreatureObject::applyDamage

//-----------------------------------------------------------------------

/**
 * Changes attrib mod(s) on the creature due to combat damage or healing. Note
 * that only positive attrib mods can be changed
 *
 * @param attrib				the index of the attribute getting damaged
 * @param delta					the amount to change the mod by
 * @param checkPoolOverflow		flag to check if a pool value is over its max value
 *
 * @return the amount the attrib mod(s) changed
 */
int CreatureObject::changeAttribModCurrentValue(Attributes::Enumerator attrib, int delta, bool checkPoolOverflow)
{
	if (!isAuthoritative())
	{
		DEBUG_WARNING(true, ("CreatureObject::damageAttribMods called for "
			"non-authoritative creature %s, attrib %d",
			getNetworkId().getValueString().c_str(), attrib));
		return 0;
	}

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
	{
		DEBUG_REPORT_LOG(true, ("attribute out of range (%d)\n", attrib));
		return 0;
	}

	int startDelta = delta;
	typedef Archive::AutoDeltaMap<uint32, CreatureMod>::const_iterator ModIter;
	for (ModIter i = m_attributeModList.begin(); i != m_attributeModList.end() &&
		delta != 0; ++i)
	{
		CreatureMod m = (*i).second;
		if (AttribMod::isAttribMod(m.mod) && m.mod.attrib == attrib && m.mod.value > 0)
		{
			if (delta > 0)
			{
				// healing mods
				if (m.maxVal > m.currentVal)
				{
					if (m.maxVal - m.currentVal >= delta)
					{
						// this mod can take the entire change
						m.currentVal += delta;
						delta = 0;
					}
					else
					{
						// this mod can only take part of the change
						delta -= (m.maxVal - m.currentVal);
						m.currentVal = m.maxVal;
					}
					m_attributeModList.set((*i).first, m);
				}
			}
			else
			{
				// damaging mods
				if (m.currentVal > 0)
				{
					if (m.currentVal + delta > 0)
					{
						// this mod can take the entire change
						m.currentVal += delta;
						delta = 0;
					}
					else
					{
						// this mod can only take part of the change
						delta += m.currentVal;
						m.currentVal = 0;
					}
					m_attributeModList.set((*i).first, m);
				}
			}
		}
	}

	if (startDelta != delta)
		recomputeAttribModTotals(checkPoolOverflow);

	return startDelta - delta;
}	// CreatureObject::changeAttribModCurrentValue

//-----------------------------------------------------------------------

/**
 * Recomputes the values due to attrib mods to be added to the creature's current
 * and max attribs.
 *
 * @param checkPoolOverflow		flag to check if a pool value is over its max value
 */
void CreatureObject::recomputeAttribModTotals(bool checkPoolOverflow)
{
int i;

	if (!isAuthoritative())
		return;

	int newCachedMaxAttributeModValues[Attributes::NumberOfAttributes];
	int newCachedCurrentAttributeModValues[Attributes::NumberOfAttributes];
	for (i = 0; i < Attributes::NumberOfAttributes; ++i)
	{
		newCachedMaxAttributeModValues[i] = 0;
		newCachedCurrentAttributeModValues[i] = 0;
	}

	typedef Archive::AutoDeltaMap<uint32, CreatureMod>::const_iterator ModIter;
	for (ModIter j = m_attributeModList.begin(); j != m_attributeModList.end(); ++j)
	{
		const CreatureMod & m = (*j).second;
		if (AttribMod::isAttribMod(m.mod))
		{
			if ((m.mod.flags & AttribMod::AMF_changeMax))
				newCachedMaxAttributeModValues[m.mod.attrib] += m.maxVal;
			newCachedCurrentAttributeModValues[m.mod.attrib] += m.currentVal;
		}
	}

	for (i = 0; i < Attributes::NumberOfAttributes; ++i)
	{
		m_cachedMaxAttributeModValues.set(i, static_cast<Attributes::Value>(newCachedMaxAttributeModValues[i]));
		m_cachedCurrentAttributeModValues.set(i, static_cast<Attributes::Value>(newCachedCurrentAttributeModValues[i]));

		if (newCachedCurrentAttributeModValues[i] < 0)
		{
			// don't let the mod prevent us from healing out of incapacitation
			int healedAttrib = getAdjustedAttribute(i, getUnmodifiedMaxAttribute(i));
			if (healedAttrib < 0)
			{
				m_cachedMaxAttributeModValues.set(i, static_cast<Attributes::Value>(
					m_cachedMaxAttributeModValues[i] - healedAttrib + 1));
				m_cachedCurrentAttributeModValues.set(i, static_cast<Attributes::Value>(
					m_cachedCurrentAttributeModValues[i] - healedAttrib + 1));
			}
		}

		if (Attributes::isAttribPool(i) && checkPoolOverflow)
		{
			// since we can modify the max value and not the current value, damage
			// the attribute if it is over the max
			int max = getMaxAttribute(i);
			int current = getUnmodifiedAttribute(i) + m_cachedCurrentAttributeModValues[i];
			if (current > max)
			{
				IGNORE_RETURN(alterAttribute(i, max - current, true));
			}
		}
	}
}	// CreatureObject::recomputeAttribModTotals

//-----------------------------------------------------------------------

/**
 * Advances the timing of attribute buffs and debuffs.  Should be called by the controller from the
 * authoritative object.
 *
 * @param time The amount of time to advance the buff.
 */
void CreatureObject::decayAttributes(float time)
{
	if (!isAuthoritative())
		return;

	PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::decayAttributes");

	// prevent client-seen attributes from being updated until all the attributes
	// have been modified
	++m_deferComputeTotalAttributes;

	const uint32 currentTime = ServerClock::getInstance().getGameTimeSeconds();
	typedef Archive::AutoDeltaMap<uint32, CreatureMod>::const_iterator ModIter;
	{	// contain i because VC isn't compilant
	for (ModIter i = m_attributeModList.begin(); i != m_attributeModList.end();)
	{
		CreatureMod m = (*i).second;
		if (AttribMod::isDirectDamage(m.mod))
		{
			DEBUG_WARNING(true, ("CreatureObject::decayAttributes has direct "
				"damage mod in its list"));
			continue;
		}

		const float deltaTime = static_cast<float>(currentTime - m.startTime);
		float decayBegin = m.mod.attack;
		if (m.mod.sustain > 0)
			decayBegin += m.mod.sustain;
		float totalTime = decayBegin;
		if (m.mod.decay > 0)
			totalTime += m.mod.decay;
		if (m.mod.sustain == -1.0f)
		{
			// mod never expires
			decayBegin = -1.0f;
			totalTime = -1.0f;
		}
		int modValue = 0;

		if (deltaTime >= totalTime && totalTime != -1.0f)
		{			
			// end of the mod time, erase the mod
			if (AttribMod::isSkillMod(m.mod))
			{
				const char * skillModName = AttribModNameManager::getInstance().getAttribModName(m.mod.skill);
				if (skillModName != nullptr)
				{
					addModValue(skillModName, -m.maxVal, true);

					LOG("CustomerService", ("Attribs: Expired skill mod on %s. Mod data: "
						"%u %u %d %.2f %d",
						PlayerObject::getAccountDescription(this).c_str(),
						m.mod.tag, m.mod.skill, m.mod.value, m.mod.sustain, m.mod.flags));
				}
				else
				{
					WARNING(true, ("Unable to expire skillmod mod on player %s due to "
						"unknown skill mod crc. Mod data: %u %u %d %.2f %d",
						PlayerObject::getAccountDescription(this).c_str(),
						m.mod.tag, m.mod.skill, m.mod.value, m.mod.sustain, m.mod.flags));
				}
			}
			else
			{
				LOG("CustomerService", ("Attribs: Expired attrib mod on %s. Mod data: "
					"%u %d %d %.2f %.2f %.2f %d",
					PlayerObject::getAccountDescription(this).c_str(),
					m.mod.tag, m.mod.attrib, m.mod.value, m.mod.attack, m.mod.sustain,
					m.mod.decay, m.mod.flags));
			}

			// remove the mod from ourself
			i = m_attributeModList.erase(i);

			if (m.mod.flags & AttribMod::AMF_triggerOnDone)
			{
				// tell scripts the mod has ended
				const char * modName = AttribModNameManager::getInstance().getAttribModName(m.mod.tag);
				if (modName != nullptr)
				{
					ScriptParams params;
					params.addParam(modName);
					params.addParam(false);
					if (AttribMod::isAttribMod(m.mod))
					{
						IGNORE_RETURN(getScriptObject()->trigAllScripts(
							Scripting::TRIG_ATTRIB_MOD_DONE, params));
					}
					else if (AttribMod::isSkillMod(m.mod))
					{
						IGNORE_RETURN(getScriptObject()->trigAllScripts(
							Scripting::TRIG_SKILL_MOD_DONE, params));
					}
				}
				else
				{
					WARNING(true, ("Unable to trigger mod done on player %s due to "
						"unknown mod name crc. Mod data: %u %u %d %.2f %.2f %.2f %d",
						PlayerObject::getAccountDescription(this).c_str(),
						m.mod.tag, m.mod.attrib, m.mod.value, m.mod.attack,
						m.mod.sustain, m.mod.decay, m.mod.flags));
				}
			}
			if (m.mod.flags & AttribMod::AMF_visible)
			{
				// tell the client to erase the mod icon
				sendCancelTimedMod(m.mod.tag);
			}
		}
		else
		{
			if (deltaTime < m.mod.attack)      // in attack phase
			{
				float t = deltaTime / m.mod.attack;
				modValue = static_cast<int>(m.mod.value * t + 0.5f);
			}
			else if (deltaTime < decayBegin || decayBegin == -1.0f) // in sustain phase
			{
				modValue = m.mod.value;
			}
			else if (deltaTime < totalTime)  // in decay phase
			{
				float t = (deltaTime - decayBegin) / m.mod.decay;
				modValue = static_cast<int>(m.mod.value * (1.0f-t) + 0.5f);
			}

			if (m.maxVal != modValue)
			{
				int delta = modValue - m.maxVal;
				// Even if the mod is flagged as not updating max, we still need
				// to change it here so we can calculate the next frame's delta.
				// We'll check for updating max in recomputeAttribModTotals instead.
				m.maxVal = modValue;

				if (AttribMod::isSkillMod(m.mod))
				{
					// add the skillmod mod as if it were from a skill,
					// which makes it temporary
					const char * skillModName = AttribModNameManager::getInstance().getAttribModName(m.mod.skill);
					if (skillModName != nullptr)
						addModValue(skillModName, delta, true);
					else
					{
						WARNING(true, ("Unable to add skillmod mod on player %s due to "
							"unknown skill mod crc. Mod data: %u %u %d %.2f %d",
							PlayerObject::getAccountDescription(this).c_str(),
							m.mod.tag, m.mod.skill, m.mod.value, m.mod.sustain, m.mod.flags));
					}
				}
				else
				{
					// see if we need to update the mod current value in addition
					// to the max value
					// non-pool attribs must update the current value
					bool updateCurrent = !Attributes::isAttribPool(m.mod.attrib);
					if (!updateCurrent)
					{
						// check the mod flags
						if (deltaTime < decayBegin)
						{
							if ((m.mod.flags & AttribMod::AMF_attackCurrent))
								updateCurrent = true;
						}
						else
						{
							if ((m.mod.flags & AttribMod::AMF_decayCurrent))
								updateCurrent = true;
						}
					}
					if (updateCurrent)
					{
						if ((m.mod.flags & AttribMod::AMF_changeMax) ||
							!Attributes::isAttribPool(m.mod.attrib))
						{
							m.currentVal += delta;
						}
						else
						{
							// we are doing damage/healing over time, modify the
							// attribute directly
							IGNORE_RETURN(alterAttribute(m.mod.attrib, delta, true,
								NetworkId::cms_invalid, false));
						}
					}
				}

				// make sure the mod's current val doesn't get out of range
				if (m.mod.value >= 0)
				{
					if (m.currentVal > m.maxVal)
						m.currentVal = m.maxVal;
					else if (m.currentVal < 0)
						m.currentVal = 0;
				}
				else
				{
					if (m.currentVal < m.maxVal)
						m.currentVal = m.maxVal;
					else if (m.currentVal > 0)
						m.currentVal = 0;
				}
				m_attributeModList.set((*i).first, m);
			}
			++i;
		}
	}
	}

	recomputeAttribModTotals(true);

	// regenerate attributes
	int i;
	float regenerationRate[3] = {0,0,0};
	m_regenerationTime += time;
	for (i = 0; i < 3; ++i)
	{
		int poolAttrib = Attributes::POOLS[i];
		int maxAttrib = getMaxAttribute(poolAttrib);
 		int currentAttrib = getAttribute(poolAttrib);
 		if (currentAttrib < maxAttrib)
		{
			regenerationRate[i] = getRegenRate(poolAttrib);
			m_regeneration[poolAttrib] += regenerationRate[i] * time;
		}
	}

	//--
	//-- update player incapacitation timer
	//--

	if (isPlayerControlled())
	{		
		// regenerate force powers
		if (!isIncapacitated())
		{
			PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
			if (playerObject)
			{
				playerObject->regenerateForcePower(time);
			}
		}
	}

	// update the client
	--m_deferComputeTotalAttributes;
	computeTotalAttributes();
	if (isAuthoritative())
		testIncapacitation(NetworkId::cms_invalid);
}

//-----------------------------------------------------------------------

/**
 * Advances the timing of mental state buffs and debuffs and decays mental states toward
 * their default values.  Should be called by the controller from the
 * authoritative object.
 *
 * @param time The amount of time to advance the buff.
 */

void CreatureObject::decayMentalStates(float time)
{
	if (isDead())
		return;

	// @todo AJS - make this take into account time since objects were unloaded when they are reloaded
	int oldBehavior = m_currentAttitude.get().m_behavior;
	CreatureAttitude newState = m_currentAttitude.get();
	updateBehavior(0, newState);

	CreatureAttitude newTargetState;

	int i;
	// Initialize the decay values for each mental state
	float decays[MentalStates::NumberOfMentalStates];
	for (i=0; i<MentalStates::NumberOfMentalStates; ++i)
	{
		// decay the values
		const float decay = time * 100.0f / m_mentalStateDecays[i];
		decays[i] = decay;
	}
	// decay the main mental states
	for (i=0; i<MentalStates::NumberOfMentalStates; ++i)
	{
		MentalStates::Value newValue = newState.m_currentValues[i];
		newValue = std::max(0.0f, newValue - decays[i]);
		newState.m_currentValues[i] = newValue;
	}

	// decay targetted mental states
	Archive::AutoDeltaMap<NetworkId, CreatureAttitude>::const_iterator targetIterator, targetEnd = m_mentalStatesToward.end();
	for (targetIterator=m_mentalStatesToward.begin(); targetIterator != targetEnd; ++targetIterator)
	{
		newTargetState = targetIterator->second;
		for (i=0; i<MentalStates::NumberOfMentalStates; ++i)
		{
			MentalStates::Value newValue = targetIterator->second.m_currentValues[i];
			if (newValue > 0)
			{
				newValue = std::max(newValue - decays[i], 0.0f);
			}
			else
			{
				newValue = std::min(newValue + decays[i], 0.0f);
			}
			newTargetState.m_currentValues[i] = newValue;
		}
		m_mentalStatesToward.set(targetIterator->first, newTargetState);
	}

	// Now change the behaviors for all of the targetted objects
	bool behaviorChanged[Behaviors::NumberOfBehaviors];

	int behavior;

	// Clear the accumulated values for each behavior
	for (behavior = 0; behavior < Behaviors::NumberOfBehaviors; ++behavior)
	{
		behaviorChanged[behavior] = false;
	}

	clipMentalStatesToward();
	deleteUnusedMentalStatesToward(behaviorChanged);

	int newBehavior = newState.m_behavior;
	if (!m_mentalStatesToward.empty())
	{
		// Update the behavior toward each target, and also update the accumulated state values for each behavior
		Archive::AutoDeltaMap<NetworkId, CreatureAttitude>::const_iterator i = m_mentalStatesToward.begin(), e=m_mentalStatesToward.end();
		for (; i != e; ++i)
		{
			oldBehavior = i->second.m_behavior;
			newTargetState = i->second;
			updateBehavior(&i->first, newTargetState);
			if (newTargetState.m_behavior != oldBehavior)
			{
				if (oldBehavior >= 0)
				{
					m_targetsByBehavior[oldBehavior].erase(i->first);
					behaviorChanged[oldBehavior] = true;
				}
				m_targetsByBehavior[newTargetState.m_behavior].insert(i->first);
				behaviorChanged[newTargetState.m_behavior] = true;
			}
			m_mentalStatesToward.set(i->first, newTargetState);
			if (newTargetState.m_behavior > newBehavior)
				newBehavior = newTargetState.m_behavior;
		}
	}

	m_currentAttitude.set(newState);

	if (m_lastBehavior.get() != newBehavior)
	{
		const int lastBehavior = m_lastBehavior.get();
		m_lastBehavior = newBehavior;
		behaviorChanged[newBehavior] = false;

		notifyTargetChange(lastBehavior, newBehavior, behaviorChanged);
	}
	else
	{
		notifyTargetChange(m_lastBehavior.get(), m_lastBehavior.get(), behaviorChanged);
	}
}

// --------------------------------------------------------------------------

/**
 * Returns the weapon currently being carried by the creature, which may or may
 * not be the weapon it is using. If the creature is not carrying a weapon,
 * returns the default weapon.
 *
 * @return the weapon
 */
WeaponObject *CreatureObject::getReadiedWeapon() const
{
	static SlotId const rightHandSlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("hold_r"));

	// first check the right hand.
	SlottedContainer const * const slottedContainer = ContainerInterface::getSlottedContainer(*this);
	if (slottedContainer)
	{
		if (slottedContainer->hasSlot(rightHandSlot))
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			Object * const o = slottedContainer->getObjectInSlot(rightHandSlot, tmp).getObject();
			if (o)
			{
				ServerObject * const so = o->asServerObject();
				if (so)
				{
					WeaponObject * const wo = so->asWeaponObject();
					if (wo)
						return wo;
				}
			}
		}
	}
	// nothing in the hand, return the default weapon
	return getDefaultWeapon();
}

//----------------------------------------------------------------------

/**
 * Returned the weapon the creature will use for combat.
 *
 * @return the weapon
 */
WeaponObject *CreatureObject::getCurrentWeapon()
{
	Object * const o = m_currentWeapon.get().getObject();
	if (o)
	{
		ServerObject * const so = o->asServerObject();
		if (so)
		{
			WeaponObject * const currentWeapon = so->asWeaponObject();
			if (currentWeapon)
				return currentWeapon;
		}
	}

	WeaponObject * const readiedWeapon = getReadiedWeapon();
	if (readiedWeapon)
		setCurrentWeapon(*readiedWeapon);
	return readiedWeapon;
}

//----------------------------------------------------------------------

/**
 * Sets the weapon the creature will use for combat.
 *
 * @param weapon		the weapon
 */
void CreatureObject::setCurrentWeapon(WeaponObject & weapon)
{
	if(isAuthoritative())
	{
		m_currentWeapon = CachedNetworkId(weapon);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setCurrentWeapon, new MessageQueueGenericValueType<NetworkId>(weapon.getNetworkId()));
	};
}

// --------------------------------------------------------------------------

int CreatureObject::getInstrumentVisualId() const
{
	static SlotId rightHandSlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("hold_r"));
	bool hasWeapon = false;

	// first check the right hand.
	SlottedContainer const * const slottedContainer = ContainerInterface::getSlottedContainer(*this);
	if (slottedContainer)
	{
		if (slottedContainer->hasSlot(rightHandSlot))
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			Object const * const object = slottedContainer->getObjectInSlot(rightHandSlot, tmp).getObject();
			if (object)
			{
				int type = lookupInstrumentVisualId(object->getObjectTemplateName());
				if (type)
					return type;
				ServerObject const * const so = object->asServerObject();
				if (so && so->asWeaponObject())
					hasWeapon = true;
			}
		}
	}

	// then check the intended target if no weapon is equipped
	if(!hasWeapon)
	{
		int visualId = internalGetInstrumentVisualId(getIntendedTarget());
		if(visualId)
			return visualId;
	}

	return 0;
}

// --------------------------------------------------------------------------

int CreatureObject::getInstrumentAudioId() const
{
	static SlotId rightHandSlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("hold_r"));
	bool hasWeapon = false;

	// first check the right hand.
	SlottedContainer const * const slottedContainer = ContainerInterface::getSlottedContainer(*this);
	if (slottedContainer)
	{
		if (slottedContainer->hasSlot(rightHandSlot))
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			Object const * const object = slottedContainer->getObjectInSlot(rightHandSlot, tmp).getObject();
			if (object)
			{
				int type = lookupInstrumentAudioId(object->getObjectTemplateName());
				if (type)
					return type;
				ServerObject const * const so = object->asServerObject();
				if (so && so->asWeaponObject())
					hasWeapon = true;
			}
		}
	}

	// then check the intended target if no weapon is equipped
	if (!hasWeapon)
	{
		int audioId = internalGetInstrumentAudioId(getIntendedTarget());
		if(audioId)
			return audioId;		
	}
	
	return 0;
}

// --------------------------------------------------------------------------

int CreatureObject::internalGetInstrumentAudioId(NetworkId const &id) const
{
	if (id != NetworkId::cms_invalid)
	{
		Object const * const object = NetworkIdManager::getObjectById(id);
		if (   object
			&& !instrumentTooFar(getPosition_w(), object->getPosition_w())
			&& ContainerInterface::getContainedByObject(*this) == ContainerInterface::getContainedByObject(*object))
		{
			ServerObject const * const serverObj = object->asServerObject();
			if (!serverObj->getSharedTemplate()->getArrangementDescriptor())
			{
				int type = lookupInstrumentAudioId(object->getObjectTemplateName());
				if (type)
					return type;
			}
		}
	}
	return 0;
}

// --------------------------------------------------------------------------

int CreatureObject::internalGetInstrumentVisualId(NetworkId const &id) const
{
	if (id != NetworkId::cms_invalid)
	{
		Object const * const object = NetworkIdManager::getObjectById(id);
		if (   object
			&& !instrumentTooFar(getPosition_w(), object->getPosition_w())
			&& ContainerInterface::getContainedByObject(*this) == ContainerInterface::getContainedByObject(*object))
		{
			ServerObject const * const serverObj = object->asServerObject();
			if (!serverObj->getSharedTemplate()->getArrangementDescriptor())
			{
				int type = lookupInstrumentVisualId(object->getObjectTemplateName());
				if (type)
					return type;
			}
		}
	}
	return 0;
}

// --------------------------------------------------------------------------

GroupObject *CreatureObject::getGroup() const
{
	if (m_group.get() != NetworkId::cms_invalid)
	{
		Object *obj = m_group.get().getObject();
		if (obj)
		{
			ServerObject *serverObj = obj->asServerObject();
			return serverObj->asGroupObject();
		}
	}
	return 0;
}

// --------------------------------------------------------------------------

void CreatureObject::setGroup(GroupObject *group, bool disbandingCurrentGroup)
{
	if (isAuthoritative())
	{
		NetworkId const oldGroupId = m_group.get();

		m_group = group ? CachedNetworkId(*group) : CachedNetworkId::cms_cachedInvalid;
		recomputeSlopeModPercent();

		// Our group has finally been established, so don't worry about
		// keeping track of who invited us to the group
		setInviterForPendingGroup( NetworkId::cms_invalid );
		setGroupInviter(NetworkId::cms_invalid, std::string(), NetworkId::cms_invalid);

		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject && (playerObject->getSessionStartPlayTime() != 0))
		{
			std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
			std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = connectedCharacterLfgData.find(getNetworkId());
			if ((iterFind != connectedCharacterLfgData.end()) && (iterFind->second.groupId != m_group.get()))
				ServerUniverse::setConnectedCharacterGroupData(getNetworkId(), m_group.get());
		}

		// if joining a group, and the group currently
		// has an active group pickup point, handle it
		if (group && (oldGroupId != group->getNetworkId()))
		{
			unsigned int const secondsLeftOnGroupPickup = group->getSecondsLeftOnGroupPickup();
			if (secondsLeftOnGroupPickup)
			{
				std::pair<std::string, Vector> const & groupPickupLocation = group->getGroupPickupLocation();
				if (!groupPickupLocation.first.empty())
				{
					// tell group member that there's an active group pickup point
					if (getClient())
					{
						StringId::LocUnicodeString response;
						if (StringId("group", "create_group_pickup_success_new_group_member").localize(response))
						{
							ConsoleMgr::broadcastString(FormattedString<2048>().sprintf(Unicode::wideToNarrow(response).c_str(), CalendarTime::convertSecondsToMS(secondsLeftOnGroupPickup).c_str()),
								getClient());
						}
					}

					// create/update the group member's group pickup point waypoint
					if (playerObject)
					{
						Location const location(groupPickupLocation.second, NetworkId::cms_invalid, Location::getCrcBySceneName(groupPickupLocation.first));
						playerObject->createOrUpdateReusableWaypoint(location, "groupPickupWp", Unicode::narrowToWide("Group Pickup Point"), Waypoint::White);
					}
				}
			}
		}
	}
	else
	{
		NetworkId groupId;
		if(group)
			groupId = group->getNetworkId();

		sendControllerMessageToAuthServer(CM_setGroup, new MessageQueueGenericValueType<std::pair<bool, NetworkId> >(std::make_pair(disbandingCurrentGroup, groupId)));
	}
}

// --------------------------------------------------------------------------

NetworkId const & CreatureObject::getGroupInviterId() const
{
	return m_groupInviter.get().first.first;
}

// --------------------------------------------------------------------------

std::string const & CreatureObject::getGroupInviterName() const
{
	return m_groupInviter.get().first.second;
}

// --------------------------------------------------------------------------

CreatureObject *CreatureObject::getGroupInviter() const
{
	NetworkId const & inviterId = m_groupInviter.get().first.first;
	if (inviterId != NetworkId::cms_invalid)
	{
		Object * const o = NetworkIdManager::getObjectById(inviterId);
		if (o)
		{
			ServerObject * const so = o->asServerObject();
			if (so)
				return so->asCreatureObject();
		}
	}
	return 0;
}

// --------------------------------------------------------------------------

void CreatureObject::setGroupInviter(NetworkId const &inviterId, std::string const& inviterName, NetworkId const &inviterShipId)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, NetworkId> > > *msg = new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, NetworkId> > >(std::make_pair(inviterName, std::make_pair(inviterId, inviterShipId)));
			controller->appendMessage(
				CM_setGroupInviter,
				0.0f,
				msg,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		m_groupInviter = PlayerAndShipPair(std::make_pair(inviterId, inviterName), inviterShipId);
	}
}

// --------------------------------------------------------------------------

NetworkId const &CreatureObject::getInviterForPendingGroup() const
{
	// We agreed to join a group and this is the guy that invited us
	return m_inviterForPendingGroup.get();
}

// --------------------------------------------------------------------------

void CreatureObject::setInviterForPendingGroup(NetworkId const &inviterId)
{
	// Once we agree to join a group, we keep track of who extended
	// the invitation until we get an actual group object
	m_inviterForPendingGroup = inviterId;
}

// --------------------------------------------------------------------------

void CreatureObject::notifyTargetChange(int lastBehavior, int newBehavior, bool * i_targetsChanged)
{
	bool sendMessage = false;
	Scripting::TrigId notification = Scripting::TRIG_TARGET_CHANGE;
	ScriptParams params;
	params.addParam(newBehavior);
	if (lastBehavior != newBehavior)
	{
		notification = Scripting::TRIG_BEHAVIOR_CHANGE;
		params.addParam(lastBehavior);
		sendMessage = true;
	}
	std::vector<int> targetListChanged;
	targetListChanged.reserve(Behaviors::NumberOfBehaviors);
	targetListChanged.resize(Behaviors::NumberOfBehaviors);
	int i;
	for (i=0; i<Behaviors::NumberOfBehaviors; ++i)
	{
		targetListChanged[i]= 0;
		if (i_targetsChanged[i])
		{
			sendMessage = true;
			targetListChanged[i] = true;
		}
	}
	if (sendMessage)
	{
		params.addParam(targetListChanged);
		IGNORE_RETURN(getScriptObject()->trigAllScripts(notification, params));
	}
}

// ----------------------------------------------------------------------

CreatureAttitude const *CreatureObject::getAttitudeToward(NetworkId const &target) const
{
	Archive::AutoDeltaMap<NetworkId, CreatureAttitude>::const_iterator i = m_mentalStatesToward.find(target);
	if (i == m_mentalStatesToward.end())
		return 0;
	return &i->second;
}

// ----------------------------------------------------------------------

void CreatureObject::clipMentalStatesToward()
{
	float maxDeltas[MentalStates::NumberOfMentalStates];
	float minValues[MentalStates::NumberOfMentalStates];
	MentalStates::Enumerator state;
	for (state=0; state<MentalStates::NumberOfMentalStates; ++state)
	{
		MentalStates::Value current = getMentalState(state);
		minValues[state] = -current;
		maxDeltas[state] = m_maxMentalStates[state] - current;
	}

	Archive::AutoDeltaMap<NetworkId, CreatureAttitude>::const_iterator i = m_mentalStatesToward.begin();
	Archive::AutoDeltaMap<NetworkId, CreatureAttitude>::const_iterator e = m_mentalStatesToward.end();
	for (; i != e; ++i)
	{
		CreatureAttitude newState = i->second;
		for (state=0; state<MentalStates::NumberOfMentalStates; ++state)
		{
			float delta = i->second.m_currentValues[state];
			if (delta > maxDeltas[state])
			{
				newState.m_currentValues[state] = maxDeltas[state];
			}
			else if (delta < minValues[state])
			{
				newState.m_currentValues[state] = 0;
			}
		}
		m_mentalStatesToward.set(i->first, newState);
	}
}

//----------------------------------------------------------------------

void CreatureObject::deleteUnusedMentalStatesToward(bool * i_behaviorChanged)
{
	Archive::AutoDeltaMap<NetworkId, CreatureAttitude>::const_iterator i = m_mentalStatesToward.begin();
	Archive::AutoDeltaMap<NetworkId, CreatureAttitude>::const_iterator e = m_mentalStatesToward.end();
	std::vector<NetworkId> to_erase;
	for (; i != e; ++i)
	{
		ServerObject * object = ServerWorld::findObjectByNetworkId(i->first);
		bool erase = true;
		int state;
		if (object)
		{
			for (state=0; state<MentalStates::NumberOfMentalStates && erase; ++state)
			{
				if (i->second.m_currentValues[state] > 0)
					erase = false;
			}
		}
		if (erase)
		{
			int currentObjectBehavior = i->second.m_behavior;
			if (currentObjectBehavior != -1) // if -1, we are deleting an object that was added and removed in the same alter
			{
				if (currentObjectBehavior < Behaviors::Calm || currentObjectBehavior >= Behaviors::NumberOfBehaviors)
				{
					DEBUG_REPORT_LOG(true, ("deleteUnusedMentalStatesToward: currentBehavior %d is invalid", currentObjectBehavior));
				}
				else
				{
					std::set<NetworkId>::iterator currentTargetByBehaviorIter = m_targetsByBehavior[currentObjectBehavior].find(i->first);
					if (currentTargetByBehaviorIter != m_targetsByBehavior[currentObjectBehavior].end())
					{
						i_behaviorChanged[currentObjectBehavior] = true;
						m_targetsByBehavior[currentObjectBehavior].erase(currentTargetByBehaviorIter);
					}
				}
			}
			to_erase.push_back(i->first);
		}
	}
	std::vector<NetworkId>::iterator tei;
	for (tei = to_erase.begin(); tei!=to_erase.end(); ++tei)
	{
		m_mentalStatesToward.erase(*tei);
	}
}

//----------------------------------------------------------------------

void CreatureObject::setMentalStateToward(const NetworkId &target, MentalStates::Enumerator state, MentalStates::Value value)
{
	if(! isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_setMentalStateToward, new MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, MentalStates::Value> > >(std::make_pair(target, std::make_pair(state, value))));
	}
	else
	{
		if (state < 0 || state >= MentalStates::NumberOfMentalStates)
		{
			DEBUG_REPORT_LOG(true, ("mental state out of range (%d)\n", state));
			return;
		}

		ServerObject * object = ServerWorld::findObjectByNetworkId(target);
		if (!object)
			return;

		CreatureAttitude const * const cs = getAttitudeToward(target);
		CreatureAttitude newState;

		if (!cs && value <= 0)
			return;

		if (cs)
		{
			 newState = *cs;
		}
		newState.m_currentValues[state] = value;
		m_mentalStatesToward.set(target, newState);
	}
}

//----------------------------------------------------------------------

void CreatureObject::setMentalStateTowardClampBehavior(const NetworkId &target, MentalStates::Enumerator state, MentalStates::Value value, Behaviors::Enumerator behavior)
{
	if(! isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_setMentalStateTowardClampBehavior, new MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, std::pair<MentalStates::Value, Behaviors::Enumerator> > > >(std::make_pair(target, std::make_pair(state, std::make_pair(value, behavior)))));
	}
	else
	{
		if (state < 0 || state >= MentalStates::NumberOfMentalStates)
		{
			DEBUG_REPORT_LOG(true, ("mental state out of range (%d)\n", state));
			return;
		}

		if (behavior < Behaviors::Calm || behavior >= Behaviors::NumberOfBehaviors)
		{
			DEBUG_REPORT_LOG(true, ("setMentalStateTowardClampBehavior: behavior %d is invalid", behavior));
			return;
		}

		ServerObject * object = ServerWorld::findObjectByNetworkId(target);
		if (!object)
			return;

		CreatureAttitude const * const cs = getAttitudeToward(target);
		if (!cs && value <= 0)
			return;

		CreatureAttitude newState;
		if (cs)
		{
			 newState = *cs;
		}
		float oldValue = newState.m_currentValues[state];

		float maxValue = 0;
		if (behavior >= Behaviors::Alert && behavior <= Behaviors::Retreat) // [1..3] 4 = flee
		{
			if (state != MentalStates::Fear)
			{
				DEBUG_REPORT_LOG(true, ("Bad mental state %d for behavior %d in setMentalStateTowardClampBehavior", state, behavior));
				return;
			}
			maxValue = m_fearCutoffs[behavior - Behaviors::Alert + 2]; // [2..4] out of [0..4]
		}
		else if (behavior == Behaviors::Attack) // [5] 6 = frenzy
		{
			if (state != MentalStates::Anger)
			{
				DEBUG_REPORT_LOG(true, ("Bad mental state %d for behavior %d in setMentalStateTowardClampBehavior", state, behavior));
				return;
			}
			maxValue = m_angerCutoffs[behavior - Behaviors::Attack + 2]; // [2..2] out of [0..2]
		}
		else if (behavior == Behaviors::Calm)
		{
			if (state == MentalStates::Fear)
			{
				maxValue = m_fearCutoffs[behavior+1];
			}
			else
			{
				maxValue = 0.0f;
			}
		}
		else
		{
			maxValue = 1e30f; // allow any value
		}
		maxValue = std::max(0.0f, maxValue - 1.0f);
		if (oldValue < maxValue)
		{
			if (value > maxValue)
				value = maxValue;

			newState.m_currentValues[state] = value;
			m_mentalStatesToward.set(target, newState);
		}
	}
}

//-----------------------------------------------------------------------

/**
* Gets the creatures current state.  0 = calm, 1-4 = afraid, 5-7 = angry.
*/

bool CreatureObject::updateBehavior(const NetworkId * id, CreatureAttitude &state) const
{
	int attitude = state.m_behavior;
	float anger = id ? getMentalStateToward(*id, MentalStates::Anger) : getMentalState(MentalStates::Anger);
	float fear = id ? getMentalStateToward(*id, MentalStates::Fear) : getMentalState(MentalStates::Fear);

	while ((state.m_fearState > 0) && fear <= m_fearCutoffs[state.m_fearState-1]) // much less fearful than current state
	{
		state.m_fearState = static_cast<CreatureAttitude::eFearState>(state.m_fearState - 1);
	}
	while ((state.m_fearState < CreatureAttitude::kFearStateCount-1) && fear >= m_fearCutoffs[state.m_fearState+1])
	{
		state.m_fearState = static_cast<CreatureAttitude::eFearState>(state.m_fearState + 1);
	}
	if (anger == 0)
		state.m_angerState = CreatureAttitude::kAngerCalm;
	else
	{
		while ((state.m_angerState > CreatureAttitude::kAngerAttack) && anger <= m_angerCutoffs[state.m_angerState-1]) // much less angry than current state
		{
			state.m_angerState = static_cast<CreatureAttitude::eAngerState>(state.m_angerState - 1);
		}
		while ((state.m_angerState < CreatureAttitude::kAngerStateCount-1) && anger >= m_angerCutoffs[state.m_angerState+1])
		{
			state.m_angerState = static_cast<CreatureAttitude::eAngerState>(state.m_angerState + 1);
		}
	}

	if (anger > 0)
	{
		if (state.m_angerState == CreatureAttitude::kAngerCalm) // got here somehow, I don't know
			state.m_behavior = CreatureAttitude::kAngerAttack + CreatureAttitude::kFearStateCount-1;
		else
			state.m_behavior = state.m_angerState + CreatureAttitude::kFearStateCount-1;
	}
	else
	{
		if (state.m_fearState == CreatureAttitude::kFearCalm)
			state.m_behavior = 0;
		else
			state.m_behavior = state.m_fearState;
	}

	return state.m_behavior != attitude;
}

//-----------------------------------------------------------------------

/**
 * Returns the list of draft schematics this creature can use.
 *
 * @return the schematic list
 */
const std::map<std::pair<uint32, uint32>,int> & CreatureObject::getDraftSchematics() const
{
static const std::map<std::pair<uint32, uint32>,int> npcSchematics;

	if (isPlayerControlled())
	{
		PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject != nullptr)
			return playerObject->getDraftSchematics();
	}
	return npcSchematics;
}	// CreatureObject::getDraftSchematics

//-----------------------------------------------------------------------

/**
 * Tests if a given ingredient for use in crafting is in this creature's inventory.
 *
 * @param ingredient		the ingredient to test
 *
 * @return true if the ingredient in in our inventory, false if not
 */
bool CreatureObject::isIngredientInInventory(const Object & ingredient) const
{
	const ServerObject * inventory = getInventory();
	if (inventory == nullptr)
		return false;

	const Object * container = ContainerInterface::getContainedByObject(ingredient);
	while (container != nullptr)
	{
		if (inventory->getNetworkId() == container->getNetworkId() ||
			getNetworkId() == container->getNetworkId())
		{
			return true;
		}
		// go up a container level
		container = ContainerInterface::getContainedByObject(*container);
	}
	return false;
}	// CreatureObject::isIngredientInInventory

//-----------------------------------------------------------------------

/**
 * Disables schematic filtering when this creature crafts. Note this only works
 * if the creature is controlled by a god-mode player.
 */
void CreatureObject::disableSchematicFiltering()
{
	if (getClient() == nullptr || !getClient()->isGod())
		return;

	setObjVarItem(OBJVAR_DISABLE_SCHEMATIC_FILTER, 1);
}	// CreatureObject::disableSchematicFiltering

//-----------------------------------------------------------------------

/**
 * Enables schematic filtering when this creature crafts. Note this only works
 * if the creature is controlled by a god-mode player.
 */
void CreatureObject::enableSchematicFiltering()
{
	if (getClient() == nullptr || !getClient()->isGod())
		return;

	removeObjVarItem(OBJVAR_DISABLE_SCHEMATIC_FILTER);
}	// CreatureObject::enableSchematicFiltering

//-----------------------------------------------------------------------

/**
 * Returns if schematic filtering is enabled or not. Note this only works
 * if the creature is controlled by a god-mode player.
 *
 * @return true if filtering is on, false if it is off
 */
bool CreatureObject::isSchematicFilteringEnabled()
{
	if (getClient() == nullptr || !getClient()->isGod())
		return true;

	return (!getObjVars().hasItem(OBJVAR_DISABLE_SCHEMATIC_FILTER));
}	// CreatureObject::isSchematicFilteringEnabled

//-----------------------------------------------------------------------

/**
 * Returns a list of manufacture schematics in this creature's datapad.
 *
 * @param schematics		list to be filled with the schematics
 */
void CreatureObject::getManufactureSchematics(std::vector<const ManufactureSchematicObject *> & schematics)
{
	const ServerObject * datapad = getDatapad();
	if (datapad == nullptr)
		return;
	const VolumeContainer * container = ContainerInterface::getVolumeContainer(*datapad);
	if (container == nullptr)
		return;

	for (ContainerConstIterator iter = container->begin(); iter != container->end(); ++iter)
	{
		const ManufactureSchematicObject * schematic = dynamic_cast<const
			ManufactureSchematicObject *>((*iter).getObject());
		if (schematic != nullptr)
			schematics.push_back(schematic);
	}
}	// CreatureObject::getManufactureSchematics(unfiltered)

//-----------------------------------------------------------------------

/**
 * Returns a list of manufacture schematics in this creature's datapad, filtered
 * by a bitmask of crafting types.
 *
 * @param schematics		list to be filled with the schematics
 * @param craftingTypes		bitmask of valid crafting types
 */
void CreatureObject::getManufactureSchematics(std::vector<const ManufactureSchematicObject *> & schematics, uint32 craftingTypes)
{
	const ServerObject * datapad = getDatapad();
	if (datapad == nullptr)
		return;
	const VolumeContainer * container = ContainerInterface::getVolumeContainer(*datapad);
	if (container == nullptr)
		return;

	for (ContainerConstIterator iter = container->begin(); iter != container->end(); ++iter)
	{
		const ManufactureSchematicObject * schematic = dynamic_cast<const
			ManufactureSchematicObject *>((*iter).getObject());
		if (schematic != nullptr && ((schematic->getCategory() & craftingTypes) != 0))
			schematics.push_back(schematic);
	}
}	// CreatureObject::getManufactureSchematics(filtered)

//-----------------------------------------------------------------------


bool CreatureObject::isItemEquipped(const Object & item) const
{
	Object const * const itemContainedBy = ContainerInterface::getContainedByObject (item);

	if (itemContainedBy)
	{
		return (getNetworkId () == itemContainedBy->getNetworkId ());
	}

	return false;
}

//-----------------------------------------------------------------------

void CreatureObject::getBehaviorTargets(int behavior, std::vector<NetworkId> &targets)
{
	if (behavior < 0 || behavior >= Behaviors::NumberOfBehaviors)
	{
		return;
	}

	targets.clear();
	targets.reserve(m_targetsByBehavior[behavior].size());
	targets.resize(m_targetsByBehavior[behavior].size());
	std::set<NetworkId>::const_iterator  i;
	std::vector<NetworkId>::iterator src = targets.begin();
	for (i=m_targetsByBehavior[behavior].begin(); i!= m_targetsByBehavior[behavior].end(); ++i)
	{
		 *src++=*i;
	}
}

//-----------------------------------------------------------------------

int CreatureObject::getBehavior() const
{
	return m_lastBehavior.get();
}


//-----------------------------------------------------------------------

int CreatureObject::getBehaviorToward(const NetworkId &target) const
{
	CreatureAttitude const * const cs = getAttitudeToward(target);
	if (!cs)
		return 0;
	return cs->m_behavior;
}

//----------------------------------------------------------------------

int CreatureObject::getNiche() const
{
	const SharedCreatureObjectTemplate * sharedTemplate = safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate());
	return static_cast<unsigned int>(sharedTemplate->getNiche());
}

//----------------------------------------------------------------------

SharedCreatureObjectTemplate::Species CreatureObject::getSpecies() const
{
	const SharedCreatureObjectTemplate * sharedTemplate = safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate());
	return static_cast<SharedCreatureObjectTemplate::Species>(sharedTemplate->getSpecies());
}

//----------------------------------------------------------------------

SharedCreatureObjectTemplate::Race CreatureObject::getRace() const
{
	const SharedCreatureObjectTemplate * sharedTemplate = safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate());
	return static_cast<SharedCreatureObjectTemplate::Race>(sharedTemplate->getRace());
}

//----------------------------------------------------------------------
/*
void CreatureObject::setAnimationState(CreatureObject::AnimationState state)
{
    m_animState = static_cast<unsigned char>(state);
}
*/
//----------------------------------------------------------------------

void CreatureObject::setMood(uint32 mood)
{
	if(isAuthoritative())
	{
		DEBUG_WARNING (mood > 255, ("Mood %d out of range", mood));
		m_mood = static_cast<unsigned char>(mood);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setMood, new MessageQueueGenericValueType<unsigned long>(mood));
	}
}

//----------------------------------------------------------------------

/**
 * Sets the incapacitated state on the creature.
 *
 * @param flag		true to incapacitate, false to recapacitate
 * @param attacker	who incapacitated us
 */
void CreatureObject::setIncapacitated(bool flag, const NetworkId & attackerId)
{
	if (isAuthoritative())
	{
		if (flag)
		{
			clearHateList();
		}

		int oldPosture = getPosture();
		if (flag && oldPosture != Postures::Incapacitated)
		{
			ScriptParams params;
			params.addParam(attackerId);
			if (getScriptObject()->trigAllScripts(
				Scripting::TRIG_ABOUT_TO_BE_INCAPACITATED, params) == SCRIPT_OVERRIDE)
			{
				LOG("CustomerService", ("Combat: %s not incapacitated due to script",
					PlayerObject::getAccountDescription(this).c_str()));
				return;
			}
			LOG("CustomerService", ("Combat: %s incapacitated",
				PlayerObject::getAccountDescription(this).c_str()));

			// if we are crafting, end the crafting session
			PlayerObject * const player = PlayerCreatureController::getPlayerObject(this);
			if (player != nullptr && player->isCrafting())
				player->stopCrafting(false);

			if (isInNpcConversation())
				endNpcConversation();

			setPosture(Postures::Incapacitated);
			setState(States::FeignDeath, false);

			// invoke incapacitation script on us
			if (oldPosture != Postures::Dead)
			{
				params.clear();
				params.addParam(attackerId);
				IGNORE_RETURN(getScriptObject()->trigAllScripts(
					Scripting::TRIG_INCAPACITATED, params));
				if (attackerId != NetworkId::cms_invalid)
				{
					// invoke incapacitation script on who incapacitated us
					ServerObject * attacker = safe_cast<ServerObject *>(
						NetworkIdManager::getObjectById(attackerId));
					if (attacker != nullptr)
					{
						params.clear();
						params.addParam(getNetworkId());
						IGNORE_RETURN(attacker->getScriptObject()->trigAllScripts(
							Scripting::TRIG_INCAPACITATE_TARGET, params));

						TangibleObject * const tangibleAttacker = attacker->asTangibleObject();

						if (tangibleAttacker != nullptr)
						{
							tangibleAttacker->verifyHateList();
						}
					}
				}
			}
		}
		else if (!flag && oldPosture == Postures::Incapacitated)
		{
			// allow scripts to prevent recapacitation
			ScriptParams params;
			if (getScriptObject()->trigAllScripts(Scripting::TRIG_RECAPACITATED,
				params) == SCRIPT_OVERRIDE)
			{
				LOG("CustomerService", ("Combat: %s not recapacitated due to script",
					PlayerObject::getAccountDescription(this).c_str()));
				return;
			}

			LOG("CustomerService", ("Combat: %s recapacitated",
				PlayerObject::getAccountDescription(this).c_str()));
			// recapacitate us
			// Force the stand command to execute
			setPosture(Postures::Upright);
		}
		// if we are a player, send us our new posture
		if ((getController() != nullptr) && !isInCombat())
		{
			getController()->appendMessage(
				CM_setPosture,
				0.0f,
				new MessageQueuePosture(getPosture(), true),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_ALL_CLIENT
			);
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setIncapacitated, new MessageQueueGenericValueType<std::pair<bool, NetworkId> >(std::make_pair(flag, attackerId)));
	}
}

//----------------------------------------------------------------------

void CreatureObject::updateMovementInfo()
{
	// TODO RAD - figure out a real fix for this
	if (!canMove() && !isPlayerControlled())
 		m_stopWalkRun = 0;

	//-- Retrieve walk and run multipliers from movement table.  These are based on the creature's posture and states.
	float walkSpeedModifier = 0.0f;
	float runSpeedModifier = 0.0f;

	float accelScale = 0.0f;
	float turnScale = 0.0f;

	getMovementTable()->getAllMovementRateScales(m_posture.get(), m_states.get(), walkSpeedModifier, runSpeedModifier, accelScale, turnScale);
	m_accelScale    = accelScale;
	m_turnScale     = turnScale;

	DEBUG_WARNING((walkSpeedModifier > 0.0f) && (runSpeedModifier > 0.0f) && (walkSpeedModifier != runSpeedModifier), ("updateMovementInfo(): both walk and run speed are nonzero but not equal,walk=[%g],run=[%g].", walkSpeedModifier, runSpeedModifier));
	m_movementScale = std::max(walkSpeedModifier, runSpeedModifier);

	//-- Calculate walk and run speed.
	m_walkSpeed = (walkSpeedModifier > 0.0f) ? getBaseWalkSpeed() : 0.0f;
	m_runSpeed  = (runSpeedModifier > 0.0f)  ? getBaseRunSpeed() : 0.0f;

	// If this creature is mounted, we want this creature's walkSpeed, runSpeed, movementScale and movementPercent set to the mount's
	if (getState(States::RidingMount))
	{
		CreatureObject *const mountedCreature = getMountedCreature();
		if (mountedCreature)
		{
			if (GameObjectTypes::isTypeOf (mountedCreature->getGameObjectType (), SharedObjectTemplate::GOT_vehicle) &&
				mountedCreature->m_vehiclePhysicsData)
			{
				m_runSpeed  = mountedCreature->m_vehiclePhysicsData->m_runSpeed;
				m_walkSpeed = mountedCreature->m_vehiclePhysicsData->m_walkSpeed;
			}
			else
			{
				m_walkSpeed = mountedCreature->getBaseWalkSpeed();
				m_runSpeed  = mountedCreature->getBaseRunSpeed();
			}
		}
	}

	//-- Update fast/slow/stationary locomotion mappings.
	getAvailableLocomotions();

	//-- Choose locomotion.
	m_locomotion = calcLocomotion();

	// if the posture isn't sneak, make sure we don't have cover
	if (getPosture() != Postures::Sneaking && getCover() != 0)
 		setCover(0);

	// if we are prone and have a movement mod > 50, increase our movement rate
	if (getPosture() == Postures::Prone)
	{
		int movementMod = getEnhancedModValue(SLOPE_MOD);
		GroupObject const *group = getGroup();
		if (group)
		{
			// get my group leader skill mod
			NetworkId const &leaderId = group->getGroupLeaderId();
			if (leaderId != NetworkId::cms_invalid)
			{
				Object const *o = NetworkIdManager::getObjectById(leaderId);
				if (o)
				{
					ServerObject const *so = o->asServerObject();
					if (so)
					{
						CreatureObject const *co = so->asCreatureObject();
						if (co)
							movementMod += co->getEnhancedModValue(GROUP_SLOPE_MOD);
					}
				}
			}
		}
		if (movementMod > 50)
		{
			float moveScale, accelScale, turnScale;
			getMovementTable()->getRates(Postures::Upright, moveScale, accelScale, turnScale);

			movementMod -= 50;
			if (movementMod > 50)
				movementMod = 50;
			float modScale = (movementMod / 50.0f) * ((moveScale / m_movementScale.get()) - 1.0f) + 1.0f;
			m_movementScale = m_movementScale.get() * modScale;
		}
	}

	//-- Mounts: update to a mount's movement info needs to propagate to
	//   its rider.
	if (getState(States::MountedCreature))
	{
		typedef std::vector<CreatureObject *> Riders;
		Riders riders;

		getMountingRiders(riders);

		Riders::const_iterator ii = riders.begin();
		Riders::const_iterator iiEnd = riders.end();

		for (; ii != iiEnd; ++ii)
		{
			CreatureObject *const rider = * ii;

			if (rider)
				rider->requestMovementInfoUpdate();
			else
			{
				LOG("mounts-bug", ("CreatureObject::updateMovementInfo(): server id=[%d],mount id=[%s],has MountedCreature state but getMountingRider() returns nullptr.", static_cast<int>(GameServer::getInstance().getProcessId()), getNetworkId().getValueString().c_str()));
				detachAllRiders();
			}
		}
	}

	//-- Prevent player from moving while stunned
	if (getState(States::Stunned))
	{
		m_turnScale = 0.0f;
		m_movementScale = 0.0f;
	}
}

//----------------------------------------------------------------------

/**
 * Sets a creature's posture, if it's allowed for its current state. This function
 * should be called directly if source is CS_player, otherwise it should be called
 * from setCombatState().
 *
 * Note: callers should set isClientImmediate to false (the default) if changing
 * posture during combat.  Combat generally controls when the client visuals for
 * posture will actually change.  Setting isClientImmediate to true causes the client
 * to drop any combat actions currently playing involving this creature and will
 * make the client's visual posture change immediately.
 *
 * @param posture		         the new posture.
 * @param isClientImmediate  if set, the client changes visual posture immediately;
 *                           otherwise, combat is assumed to migrate to this posture
 *                           at the appropriate time (as instructed by the combat action
 *                           message) or the client will revert to this new server posture
 *                           in 5 seconds (as of this writing).
 */
void CreatureObject::setPosture(Postures::Enumerator newPosture, bool isClientImmediate)
{
	//-- vehicles can't be anything but upright
	if (newPosture != Postures::Upright && GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		WARNING (true, ("CreatureObject attempt to set a vehicle to posture [%d].", newPosture));
		return;
	}

	//-- Only do server message processing if the posture changed, but always send the
	//   posture to the clients if isClientImmediate is true --- the client visual posture
	//   is unknown on the server and may be different than the server posture.
	Postures::Enumerator oldPosture = m_posture.get();
	bool const doServerProcessing = (oldPosture != newPosture);

	if (doServerProcessing)
	{
		//-- Process the posture change on the server only if the posture changed.
		if (!isAuthoritative())
		{
			sendControllerMessageToAuthServer(CM_setPosture, new MessageQueuePosture(newPosture, isClientImmediate));
			return;
		}

		if (getMovementTable()->getLocomotion(newPosture, m_states.get(), MovementTable::kStationary) != Locomotions::Invalid)
		{
			Postures::Enumerator oldPosture = m_posture.get();
			
			m_posture = newPosture;
			
			// If we were previously sitting on a chair, clear the flag.
			if(oldPosture == Postures::Sitting)
				setState(States::SittingOnChair, false);

			// @todo revisit this.  updateMoevementInfo() indirectly changes auto delta variables.  Since this isn't
			// guaranteed to be the authoritative object, this is an invalid thing to do.
			requestMovementInfoUpdate();

			if (getScriptObject() != nullptr)
			{
				ScriptParams params;
				params.addParam(oldPosture);
				params.addParam(newPosture);
				IGNORE_RETURN(getScriptObject()->trigAllScripts(
												Scripting::TRIG_CHANGED_POSTURE, params));
			}

			recomputeSlopeModPercent();
		}
	}

	//-- Handle posture changes that should take effect immediately on the client.
	if (isClientImmediate)
	{
		//-- Send if we're the authoritative server OR if we're not passing the message on to the authoritative server.
		//   This guarantees that we only send this message to the clients once.
		bool const sendToClient = isAuthoritative() || !doServerProcessing;
		if (sendToClient)
		{
			Controller *const controller = getController();
			if (controller)
				controller->appendMessage(CM_setPosture, 0.0f, new MessageQueuePosture(newPosture, true), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_ALL_CLIENT);
			else
				DEBUG_WARNING(true, ("setPosture(): called with isClientImmediate but object id=[%s],template=[%s] has no controller.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Call this function to request that movement info be updated at the
 * end of the next frame's alter.
 *
 * This function was needed to delay the updateMovementInfo() until
 * after all transitive mount-related state was complete.  It has the
 * side effect of preventing the call from being made multiple times
 * in the same frame.
 */

void CreatureObject::requestMovementInfoUpdate()
{
	setLocalFlag(LocalObjectFlags::CreatureObject_RequiresMovementInfoUpdate, true);
}

//-----------------------------------------------------------------------

void CreatureObject::setCover(int cover)
{
	if(!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_setCover, new MessageQueueGenericValueType<int>(cover));
	}
	else
	{
		m_cover = cover;
	}
}

// ----------------------------------------------------------------------

/**
 * Sets the value of a state on a creature, if it's allowed for its current
 * state.
 *
 * @param whichState  which state to change
 * @param value       new value of the state
 */
void CreatureObject::setState(States::Enumerator whichState, bool value)
{
	if (isAuthoritative())
	{
		if(whichState == States::Combat)
		{
			setInCombat(value);
			return;
		}

		//-- vehicles can't have most states
		if (value && GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		{
			switch (whichState)
			{
				case States::MountedCreature:
				case States::Swimming:
				case States::Combat:
					// these states are okay for a vehicle to have, continue on with state change processing.
					break;

				default:
					// all other states are not okay for a vehicle to have, warn about them.
					WARNING (true, ("CreatureObject id=[%s] attempt to set state id=[%d], name=[%s] on a vehicle, unsupported.", getNetworkId().getValueString().c_str(), whichState, States::getStateName(whichState)));
					return;
			}
		}

		uint64 stateFlags = m_states.get();

		// enforce a no state change if the creature is a glowing Jedi
		if (value)
			stateFlags |= States::getStateMask(whichState);
		else
			stateFlags &= ~States::getStateMask(whichState);
 		if (m_states.get() != stateFlags)
 		{
 			m_states = stateFlags;
 			requestMovementInfoUpdate();
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setState, new MessageQueueSetState(static_cast<int>(whichState), value));
	}
}

//----------------------------------------------------------------------

void CreatureObject::setSayMode(uint32 sayMode)
{
	if(isAuthoritative())
	{
		DEBUG_WARNING (sayMode > 255, ("Saymode %d out of range", sayMode));
		m_sayMode = static_cast<unsigned char>(sayMode);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setSayMode, new MessageQueueGenericValueType<unsigned long>(sayMode));
	}
}

//----------------------------------------------------------------------

void CreatureObject::setAnimationMood(const std::string & mood)
{
	if(isAuthoritative())
	{
		m_animationMood = mood;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setAnimationMood, new MessageQueueGenericValueType<std::string>(mood));
	}
}

//----------------------------------------------------------------------

void CreatureObject::setAttribute(Attributes::Enumerator attribute, Attributes::Value value)
{
	if(!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_setAttribute, new MessageQueueGenericValueType<std::pair<Attributes::Enumerator, Attributes::Value> >(std::make_pair(attribute, value)));
	}
	else
	{
		if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
		{
			DEBUG_REPORT_LOG(true, ("attribute out of range (%d)\n", attribute));
			return;
		}
		if (!Attributes::isAttribPool(attribute))
		{
			DEBUG_WARNING(true, ("CreatureObject::setAttribute called with non-pool attribute %d on creature %s. If this is a regeneration value, call setRegenRate() instead.",
				attribute, getNetworkId().getValueString().c_str()));
			return;
		}
		const Attributes::Value currentValue = m_attributes[attribute];
		IGNORE_RETURN(alterAttribute(attribute, value - currentValue, true));
	}
}

//----------------------------------------------------------------------

/** Set the maximum value for an attribute.  Optionally revalidate the stats for players.
 */
void CreatureObject::setMaxAttribute(Attributes::Enumerator attribute, Attributes::Value value, bool verifyPlayerMaxStats)
{
	if(! isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_setMaxAttribute, new MessageQueueGenericValueType<std::pair<Attributes::Enumerator, Attributes::Value> >(std::make_pair(attribute, value)));
	}
	else
	{
		if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
		{
			DEBUG_REPORT_LOG(true, ("attribute out of range (%d)\n", attribute));
			return;
		}
		if (!Attributes::isAttribPool(attribute))
		{
			DEBUG_WARNING(true, ("CreatureObject::setMaxAttribute called with non-pool attribute %d on creature %s. If this is a regeneration value, call setRegenRate() instead.",
				attribute, getNetworkId().getValueString().c_str()));
			return;
		}
		m_maxAttributes.set(attribute, value);
	}
}

//----------------------------------------------------------------------

void CreatureObject::setMentalState(MentalStates::Enumerator state, MentalStates::Value value)
{
	if(! isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_setMentalState, new MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, MentalStates::Value> >(std::make_pair(state, value)));
	}
	else
	{
		if (state < 0 || state >= MentalStates::NumberOfMentalStates)
		{
			DEBUG_REPORT_LOG(true, ("mental state out of range (%d)\n", state));
			return;
		}

		if (value > m_maxMentalStates[state])
			value = m_maxMentalStates[state];
		else if (value < 0)
			value = 0;
		CreatureAttitude newState = m_currentAttitude.get();
		newState.m_currentValues[state] = value;
		m_currentAttitude.set(newState);
	}
}

//----------------------------------------------------------------------

void CreatureObject::setMaxMentalState(MentalStates::Enumerator state, MentalStates::Value value)
{
	if(isAuthoritative())
	{
		if (state < 0 || state >= MentalStates::NumberOfMentalStates)
		{
			DEBUG_REPORT_LOG(true, ("mental state out of range (%d)\n", state));
			return;
		}

		m_maxMentalStates.set(state, value);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setMaxMentalState, new MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, MentalStates::Value> >(std::make_pair(state, value)));
	}
}

//----------------------------------------------------------------------

void CreatureObject::setMentalStateDecay(MentalStates::Enumerator state, float value)
{
	if(! isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_setMentalStateDecay, new MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, float> >(std::make_pair(state, value)));
	}
	else
	{
		if (state < 0 || state >= MentalStates::NumberOfMentalStates)
		{
			DEBUG_REPORT_LOG(true, ("mental state out of range (%d)\n", state));
			return;
		}

		m_mentalStateDecays.set(state, value);
	}
}

//----------------------------------------------------------------------

ServerObject *CreatureObject::getObjectInSlot(ConstCharCrcLowerString const &slotName)
{
	SlottedContainer * const s = ContainerInterface::getSlottedContainer(*this);
	if (s)
	{
		SlotId const dp = SlotIdManager::findSlotId(slotName);
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		return safe_cast<ServerObject *>(s->getObjectInSlot(dp, tmp).getObject());
	}

	return 0;
}

//----------------------------------------------------------------------

const ServerObject *  CreatureObject::getObjectInSlot (const ConstCharCrcLowerString & slotName) const
{
	return const_cast<CreatureObject *>(this)->getObjectInSlot (slotName);
}

//-----------------------------------------------------------------------

ServerObject * CreatureObject::getDatapad()
{
	return getObjectInSlot (SlotNames::datapad);
}

//----------------------------------------------------------------------

const ServerObject * CreatureObject::getDatapad() const
{
	return const_cast<CreatureObject *>(this)->getDatapad ();
}

//----------------------------------------------------------------------

ServerObject * CreatureObject::getBank       ()
{
	return getObjectInSlot (SlotNames::bank);
}

//----------------------------------------------------------------------

const ServerObject *  CreatureObject::getBank       () const
{
	return const_cast<CreatureObject *>(this)->getBank ();
}

//-----------------------------------------------------------------------

ServerObject * CreatureObject::getInventory()
{
	return getObjectInSlot (SlotNames::inventory);
}

//----------------------------------------------------------------------

const ServerObject * CreatureObject::getInventory() const
{
	return const_cast<CreatureObject *>(this)->getInventory ();
}

//----------------------------------------------------------------------

ServerObject * CreatureObject::getHair()
{
	return getObjectInSlot (SlotNames::hair);
}

//------------------------------------------------------------------------

const ServerObject * CreatureObject::getHair() const
{
	return const_cast<CreatureObject *>(this)->getHair ();
}

//----------------------------------------------------------------------


ServerObject * CreatureObject::getAppearanceInventory()
{
	return getObjectInSlot (SlotNames::appearance);
}

//----------------------------------------------------------------------

const ServerObject * CreatureObject::getAppearanceInventory() const
{
	return const_cast<CreatureObject *>(this)->getAppearanceInventory ();
}

//----------------------------------------------------------------------


ServerObject * CreatureObject::getHangar()
{
	return getObjectInSlot (SlotNames::hangar);
}

//----------------------------------------------------------------------

const ServerObject * CreatureObject::getHangar() const
{
	return const_cast<CreatureObject *>(this)->getHangar();
}


//-----------------------------------------------------------------------

bool CreatureObject::onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer)
{
	bool const baseResult = TangibleObject::onContainerAboutToTransfer(destination, transferer);
	if (!baseResult)
		return false;

	const PlayerObject * const player = PlayerCreatureController::getPlayerObject(this);
	// let's prevent CS from putting any more players in their inventories
	if(player && destination)
	{
		const CreatureObject * creatureDestination = destination->asCreatureObject();
		if(creatureDestination)
		{
			const PlayerObject * const playerDestination = PlayerCreatureController::getPlayerObject(creatureDestination);
			if(playerDestination)
			{
				return false;
			}
		}
	}
	return onContainerAboutToTransferForMounts(destination, transferer);
}


/**
 * Callback for when the creature is about to equip an item. Checks to make sure
 * we don't equip two shields, and prevents equiping armor that would incapacitate
 * us.
 *
 * @param item			the item being equipped
 * @param transferer	who is causing the item to be equipped
 *
 * @return true to allow the transfer, false to prevent it
 */
int CreatureObject::onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer)
{
	TangibleObject const * const object = item.asTangibleObject();
	if (object != nullptr)
	{
		// See if this item is equippable
		const char *sharedTemplateName = item.getSharedTemplateName();
		if (!isAppearanceEquippable(sharedTemplateName))
		{
			if (getClient() != nullptr)
			{
				StringId message("shared", "item_not_equippable");
				Unicode::String outOfBand;
				Chat::sendSystemMessage(*this, message, outOfBand);
				return Container::CEC_SilentError;
			}
			else
				return Container::CEC_BlockedByDestinationContainer;
		}
	}


	return TangibleObject::onContainerAboutToGainItem(item, transferer);
}	// CreatureObject::onContainerAboutToGainItem

//----------------------------------------------------------------------

/**
 * Callback for when the creature unequips an item.
 *
 * @param destination		where the item was transferred to
 * @param item				the item that was unequipped
 * @param transferer		who caused the transfer
 */
void CreatureObject::onContainerLostItem(ServerObject *destination, ServerObject &item, ServerObject *transferer)
{
	TangibleObject const * const tangibleObject = item.asTangibleObject();
	if (tangibleObject == nullptr)
		return;

	// check if the object applies skill mod bonuses when equipped
	{
	std::vector<std::pair<std::string, int> > skillModBonuses;
	tangibleObject->getSkillModBonuses(skillModBonuses);
	int const bonusCount = skillModBonuses.size();
	if (bonusCount > 0)
	{
		std::vector<const char *> modNames(bonusCount);
		std::vector<int> modValues(bonusCount);
		for (int i = 0; i < bonusCount; ++i)
		{
			std::pair<std::string, int> const &bonus = skillModBonuses[i];
			addModBonus(bonus.first, -bonus.second);
			modNames[i] = bonus.first.c_str();
			modValues[i] = -bonus.second;
		}
		ScriptParams params;
		params.addParam(modNames);
		params.addParam(modValues);
		IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_SKILLMODS_CHANGED, params));
	}
	}

	// check if the object applies attribute bonuses when equipped
	{
	std::vector<std::pair<int, int> > attribBonuses;
	tangibleObject->getAttribBonuses(attribBonuses);
	int const bonusCount = attribBonuses.size();
	for (int i = 0; i < bonusCount; ++i)
	{
		std::pair<int, int> const &bonus = attribBonuses[i];
		addAttribBonus(bonus.first, -bonus.second);
	}
	}



	// if the item is a weapon, make our current weapon our default weapon
	WeaponObject const * const weaponObject = tangibleObject->asWeaponObject();
	if (weaponObject != nullptr && getDefaultWeapon() != nullptr)
		setCurrentWeapon(*getDefaultWeapon());

	// check if the object is our shield
	if (tangibleObject == m_shield)
		m_shield = nullptr;

	//Update wearbles data
	SlottedContainmentProperty* scp = ContainerInterface::getSlottedContainmentProperty(item);
	if (scp)
	{
		WearableEntry e("", scp->getCurrentArrangement(), item.getNetworkId(), 0);
		int index = m_wearableData.find(e);
		if (index >= 0)
		{
			m_wearableData.erase(index);
		}
	}

	TangibleObject::onContainerLostItem(destination, item, transferer);
}	// CreatureObject::onContainerLostItem

//----------------------------------------------------------------------

/**
 * Callback for when the creature equips an item.
 * shields, and weapons.
 *
 * @param item				the item that was equipped
 * @param transferer		who caused the transfer
 */
void CreatureObject::onContainerGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer)
{
	TangibleObject * const tangibleObject = item.asTangibleObject();
	if (tangibleObject == nullptr)
		return;

	// check if the object applies skill mod bonuses when equipped
	{
	std::vector<std::pair<std::string, int> > skillModBonuses;
	tangibleObject->getSkillModBonuses(skillModBonuses);
	int const bonusCount = skillModBonuses.size();
	if (bonusCount > 0)
	{
		std::vector<const char *> modNames(bonusCount);
		std::vector<int> modValues(bonusCount);
		for (int i = 0; i < bonusCount; ++i)
		{
			std::pair<std::string, int> const &bonus = skillModBonuses[i];
			addModBonus(bonus.first, bonus.second);
			modNames[i] = bonus.first.c_str();
			modValues[i] = bonus.second;
		}
		ScriptParams params;
		params.addParam(modNames);
		params.addParam(modValues);
		IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_SKILLMODS_CHANGED, params));
	}
	}

	// check if the object applies attribute bonuses when equipped
	{
	std::vector<std::pair<int, int> > attribBonuses;
	tangibleObject->getAttribBonuses(attribBonuses);
	int const bonusCount = attribBonuses.size();
	for (int i = 0; i < bonusCount; ++i)
	{
		std::pair<int, int> const &bonus = attribBonuses[i];
		addAttribBonus(bonus.first, bonus.second);
	}
	}

	// if the item is a weapon, make it our current weapon
	WeaponObject * const weaponObject = tangibleObject->asWeaponObject();
	if (weaponObject != nullptr)
		setCurrentWeapon(*weaponObject);

	//Update wearables data
	SlottedContainmentProperty* scp = ContainerInterface::getSlottedContainmentProperty(item);
	if (scp)
	{
		Object const * inventory = 0;
		Object const * datapad = 0;

		SlottedContainer const * const myContainer = ContainerInterface::getSlottedContainer(*this);

		if (myContainer)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			static SlotId const inventorySlot = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::inventory);
			if (inventorySlot != SlotId::invalid)
			{
				Container::ContainedItem itemId = myContainer->getObjectInSlot(inventorySlot, tmp);
				inventory = itemId.getObject();
			}
			SlotId datapadSlot = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::datapad);
			if (datapadSlot != SlotId::invalid)
			{
				Container::ContainedItem itemId = myContainer->getObjectInSlot(datapadSlot, tmp);
				datapad = itemId.getObject();
			}
			SlotId appearanceInvSlot = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::appearance);
			if (appearanceInvSlot != SlotId::invalid)
			{
				Container::ContainedItem itemId = myContainer->getObjectInSlot(appearanceInvSlot, tmp);
			}
		}

		if ( scp->isInAppearanceSlot() || tangibleObject == inventory || tangibleObject == datapad)
		{
			ConstCharCrcString clientSharedTemplateNameCrcString = ObjectTemplateList::lookUp(tangibleObject->getClientSharedTemplateName());
			if (clientSharedTemplateNameCrcString.isEmpty())
				WARNING(true, ("Could not find crc for %s.  Wearable will not be streamed to client", tangibleObject->getClientSharedTemplateName()));
			else if (tangibleObject->asWeaponObject() != nullptr)
			{
				addPackedWearable(tangibleObject->getAppearanceData(), scp->getCurrentArrangement(), tangibleObject->getNetworkId(), clientSharedTemplateNameCrcString.getCrc(),
					tangibleObject->createSharedBaselinesMessage(), tangibleObject->createSharedNpBaselinesMessage());
			}
			else
				addPackedWearable(tangibleObject->getAppearanceData(), scp->getCurrentArrangement(), tangibleObject->getNetworkId(), clientSharedTemplateNameCrcString.getCrc());
		}
	}

	TangibleObject::onContainerGainItem(item, source, transferer);
}

// ----------------------------------------------------------------------

void CreatureObject::onContainerTransferComplete(ServerObject *oldContainer, ServerObject *newContainer)
{
	TangibleObject::onContainerTransferComplete(oldContainer, newContainer);

	setStatesDueToContainment();

	if (!isInWorld())
		setMovementStationary();

	ShipObject const * const oldContainingShipObject = ShipObject::getContainingShipObject(oldContainer);
	ShipObject const * const newContainingShipObject = ShipObject::getContainingShipObject(newContainer);

	if (oldContainingShipObject != newContainingShipObject)
	{
		GroupObject * const groupObject = getGroup();

		if (groupObject != 0)
		{
			if (oldContainingShipObject != 0)
			{
				if (oldContainingShipObject->getPortalProperty() != 0)
				{
					if (getNetworkId() == oldContainingShipObject->getOwnerId())
					{
						NetworkId const & shipId = oldContainingShipObject->getNetworkId();
						groupObject->removePOBShip(shipId);
					}
				}
			}

			if (newContainingShipObject != 0)
			{
				if (newContainingShipObject->getPortalProperty() != 0)
				{
					groupObject->addPOBShipAndOwner(newContainingShipObject->getNetworkId(), newContainingShipObject->getOwnerId());
				}
			}

			NetworkId const & shipId = (newContainingShipObject != 0) ? newContainingShipObject->getNetworkId() : NetworkId::cms_invalid;
			groupObject->setShipForMember(getNetworkId(), shipId);
		}
	}

	Client * const client = getClient();
	if (client && ServerWorld::isSpaceScene())
	{
		ServerObject const * const oldTopmost = oldContainer ? safe_cast<ServerObject *>(ContainerInterface::getTopmostContainer(*oldContainer)) : this;
		ServerObject * const newTopmost = newContainer ? safe_cast<ServerObject *>(ContainerInterface::getTopmostContainer(*newContainer)) : this;
		if (oldTopmost != newTopmost)
		{
			SpaceVisibilityManager::removeClient(*client);
			SpaceVisibilityManager::addClient(*client, *newTopmost);
		}
	}
}

// ----------------------------------------------------------------------
// this object got added to a trigger volume
void CreatureObject::onAddedToTriggerVolume(TriggerVolume & triggerVolume)
{
	WARNING(!isInWorld(), ("CreatureObject (%s) not in world but being added to TriggerVolume (%s) owned by (%s)",
		getDebugInformation().c_str(),
		triggerVolume.getName().c_str(),
		triggerVolume.getOwner().getDebugInformation().c_str()));

	IGNORE_RETURN(m_triggerVolumeEntered.insert(&triggerVolume));
}

// ----------------------------------------------------------------------
// this object got removed from a trigger volume
void CreatureObject::onRemovedFromTriggerVolume(TriggerVolume & triggerVolume)
{
	// NOTE: be careful about referencing the TriggerVolume because this
	// function can be called from the TriggerVolume's dtor
	IGNORE_RETURN(m_triggerVolumeEntered.erase(&triggerVolume));
}

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------

/**
 * Computes the total attributes and max attributes for sending to clients,
 */
void CreatureObject::computeTotalAttributes ()
{
int i;

	WARNING_STRICT_FATAL(!isAuthoritative(), ("computeTotalAttributes called on nonauthoritative object %s:%s", getObjectTemplateName(), getNetworkId().getValueString().c_str()));

	if (!isAuthoritative())
		return;

	if (m_deferComputeTotalAttributes <= 0)
	{
		if (m_regenerationTime >= ConfigServerGame::getRegenThreshold())
		{
			// send updates to the client
			m_regenerationTime -= ConfigServerGame::getRegenThreshold();
			for (i = 0; i < 3; ++i)
			{
				int poolAttrib = Attributes::POOLS[i];
				const float delta = static_cast<float>(floor(m_regeneration[
					poolAttrib]));
				if (delta >= 1.0f)
				{
					m_regeneration[poolAttrib] -= delta;
					IGNORE_RETURN(alterAttribute(poolAttrib, static_cast<int>(
						delta), true));
				}
			}
		}

		if (ConfigServerGame::getCheckAttribLimits())
		{
			// check and make sure our current attribs are <= our max attribs,
			// for both unmodified and modified versions
			for (i = 0; i < Attributes::NumberOfAttributes; ++i)
			{
				if (Attributes::isAttribPool(i))
				{
					if (getUnmodifiedAttribute(i) > getUnmodifiedMaxAttribute(i))
					{
						WARNING_STRICT_FATAL(true, ("Creature %s has attribute %d with "
							"an unmodified current value > its max value! We will change "
							"the current value! Current = %d, max = %d, "
							"cur mods = %d, max mods = %d",
							getNetworkId().getValueString().c_str(),
							i,
							static_cast<int>(getUnmodifiedAttribute(i)),
							static_cast<int>(getUnmodifiedMaxAttribute(i)),
							static_cast<int>(m_cachedCurrentAttributeModValues[i]),
							static_cast<int>(m_cachedMaxAttributeModValues[i])));
						m_attributes.set(i, getUnmodifiedMaxAttribute(i));
					}
					if (getAttribute(i) > getMaxAttribute(i))
					{
						WARNING_STRICT_FATAL(true, ("Creature %s has attribute %d with "
							"a current value > its max value! Current = %d(%d), max = "
							"%d(%d), cur mods = %d, max mods = %d",
							getNetworkId().getValueString().c_str(),
							i,
							static_cast<int>(getAttribute(i)),
							static_cast<int>(getUnmodifiedAttribute(i)),
							static_cast<int>(getMaxAttribute(i)),
							static_cast<int>(getUnmodifiedMaxAttribute(i)),
							static_cast<int>(m_cachedCurrentAttributeModValues[i]),
							static_cast<int>(m_cachedMaxAttributeModValues[i])));
					}
				}
			}
		}

		for (i = 0; i < Attributes::NumberOfAttributes; ++i)
		{
			if (Attributes::isAttribPool(i))
			{
				m_totalAttributes.set(i, getAttribute(i) - static_cast<int>(floor(m_regeneration[i])));
				m_totalMaxAttributes.set(i, getMaxAttribute(i));
			}
		}
	}
}	// CreatureObject::computeTotalAttributes

//----------------------------------------------------------------------

void CreatureObject::resetAttribute (Attributes::Enumerator attrib, Attributes::Value value)
{
	setMaxAttribute (attrib, value);
	setAttribute    (attrib, value);
	m_shockWounds   = 0;
}

//----------------------------------------------------------------------

void CreatureObject::initializeAttribute(Attributes::Enumerator attrib, Attributes::Value value)
{
	m_maxAttributes.set(attrib, value);
	m_attributes.set   (attrib, value);
}

//----------------------------------------------------------------------

/**
 * Drains a creature's special abilities, based on it's drain attributes.
 *
 * @param action		amount to drain the creature's action SA
 * @param mind			amount to drain the creature's mind SA
 *
 * @return true if the SA were drained, false if draining them would cause one
 * to be < 0
 */
bool CreatureObject::drainAttributes(Attributes::Value action, Attributes::Value mind)
{
	int i;
	int mods[3] = {0, action, mind};
	int deltas[3];

	// determine the amount to drain and make sure it won't incapaciate the creature
	for (i = 0; i < 3; ++i)
	{
		if (mods[i] > 0)
		{
			deltas[i] = getAttribute(Attributes::POOLS[i]) - mods[i];
			if (deltas[i] < 0)
			return false;
	}
		else
			deltas[i] = -1;
	}

	// drain the attributes
	for (i = 0; i < 3; ++i)
	{
		if (deltas[i] >= 0)
			alterAttribute(Attributes::POOLS[i], -mods[i], false);
	}
	return true;
}	// CreatureObject::drainAttributes

//----------------------------------------------------------------------

/**
 * Tests draining an attribute, but does not actually modify the attrib.
 *
 * @param attribute		the attribute to test
 * @param value			the amount to drain the attribute by
 *
 * @return the amount the attribute would be drained by, or -1 on error
 */
int CreatureObject::testDrainAttribute(Attributes::Enumerator attribute,
	Attributes::Value value) const
{
	// we can only drain action and mind
	if (attribute != Attributes::Action && attribute != Attributes::Mind)
		return -1;

	if (getAttribute(attribute) - value <= 0)
		return -1;

	return static_cast<int>(value);
}	// CreatureObject::testDrainAttribute

//----------------------------------------------------------------------

void CreatureObject::onClientReady(Client *c)
{
	TangibleObject::onClientReady(c);

	// check cell permissions
	Object * const attachedTo = getAttachedTo();
	if (attachedTo)
	{
		ServerObject * const serverAttachedTo = attachedTo->asServerObject();
		if (serverAttachedTo)
		{
			CellObject * const cellObject = serverAttachedTo->asCellObject();
			if (cellObject && !cellObject->isAllowed(*this))
			{
				BuildingObject * const ownerBuilding = cellObject->getOwnerBuilding();
				if (ownerBuilding)
					ownerBuilding->expelObject(*this);
			}
		}
	}

	GroupObject *group = getGroup();
	if (group)
		group->onGroupMemberConnect(getNetworkId());

	// update the client with the list of mods currently on the character

	// but first, remove any mods from the list that has expired
	uint32 const gameTimeSeconds = ServerClock::getInstance().getGameTimeSeconds();
	std::vector<uint32> expiredMods;
	std::vector<uint32>::const_iterator iterTimedMod = m_timedMod.begin();
	std::vector<float>::const_iterator iterTimedModDuration = m_timedModDuration.begin();
	std::vector<uint32>::const_iterator iterTimedModUpdateTime = m_timedModUpdateTime.begin();

	for (; iterTimedMod != m_timedMod.end(); ++iterTimedMod, ++iterTimedModDuration, ++iterTimedModUpdateTime)
	{
		if (*iterTimedModDuration >= 0)
		{
			if ((*iterTimedModUpdateTime + static_cast<uint32>(*iterTimedModDuration)) < gameTimeSeconds)
				expiredMods.push_back(*iterTimedMod);
		}
	}

	int index;
	for (iterTimedMod = expiredMods.begin(); iterTimedMod != expiredMods.end(); ++iterTimedMod)
	{
		index = m_timedMod.find(*iterTimedMod);

		if (index >= 0)
		{
			m_timedMod.erase(index);
			m_timedModDuration.erase(index);
			m_timedModUpdateTime.erase(index);
		}
	}

	// send the mods to the client
	iterTimedMod = m_timedMod.begin();
	iterTimedModDuration = m_timedModDuration.begin();
	iterTimedModUpdateTime = m_timedModUpdateTime.begin();

	for (; iterTimedMod != m_timedMod.end(); ++iterTimedMod, ++iterTimedModDuration, ++iterTimedModUpdateTime)
	{
		if (*iterTimedModDuration >= 0)
			sendTimedModData(*iterTimedMod, *iterTimedModDuration - static_cast<float>(gameTimeSeconds - *iterTimedModUpdateTime), false);
		else
			sendTimedModData(*iterTimedMod, -1.0f, false);
	}

	// update the client with the list of cooldowns currently on the character

	// visual only cooldown timers are stored in objvars
	DynamicVariableList const & objvars = getObjVars();
	std::map<int, std::pair<int, int> > visualCooldownTimers;
	for (DynamicVariableList::MapType::const_iterator iterObjVar = objvars.begin(); iterObjVar != objvars.end(); ++iterObjVar)
	{
		if (iterObjVar->first.find("visualCooldown.beginTime.") == 0)
		{
			int cooldownGroupCrc = 0;
			if (1 == sscanf(iterObjVar->first.c_str(), "visualCooldown.beginTime.%d", &cooldownGroupCrc))
			{
				int beginTime = 0;
				if (iterObjVar->second.get(beginTime))
				{
					std::map<int, std::pair<int, int> >::iterator iterFind = visualCooldownTimers.find(cooldownGroupCrc);
					if (iterFind == visualCooldownTimers.end())
					{
						visualCooldownTimers[cooldownGroupCrc] = std::make_pair(beginTime, 0);
					}
					else
					{
						iterFind->second.first = beginTime;
					}
				}
			}
		}
		else if (iterObjVar->first.find("visualCooldown.endTime.") == 0)
		{
			int cooldownGroupCrc = 0;
			if (1 == sscanf(iterObjVar->first.c_str(), "visualCooldown.endTime.%d", &cooldownGroupCrc))
			{
				int endTime = 0;
				if (iterObjVar->second.get(endTime))
				{
					std::map<int, std::pair<int, int> >::iterator iterFind = visualCooldownTimers.find(cooldownGroupCrc);
					if (iterFind == visualCooldownTimers.end())
					{
						visualCooldownTimers[cooldownGroupCrc] = std::make_pair(0, endTime);
					}
					else
					{
						iterFind->second.second = endTime;
					}
				}
			}
		}
	}

	char buffer[256];
	int const currentGameTime = static_cast<int>(ServerClock::getInstance().getGameTimeSeconds());
	for (std::map<int, std::pair<int, int> >::const_iterator iterCooldown = visualCooldownTimers.begin(); iterCooldown != visualCooldownTimers.end(); ++iterCooldown)
	{
		// send any unexpired cooldowns to the client
		if ((iterCooldown->second.first > 0) && (iterCooldown->second.second > 0) && (iterCooldown->second.first <= currentGameTime) && (currentGameTime < iterCooldown->second.second))
		{
			CreatureController *controller = getCreatureController();
			if (controller)
			{
				MessageQueueCommandTimer *msg = new MessageQueueCommandTimer(
					0,
					iterCooldown->first,
					-1,
					0);
				msg->setCurrentTime(MessageQueueCommandTimer::F_cooldown, static_cast<float>(currentGameTime - iterCooldown->second.first));
				msg->setMaxTime    (MessageQueueCommandTimer::F_cooldown, static_cast<float>(iterCooldown->second.second - iterCooldown->second.first));

				controller->appendMessage(
					static_cast< int >( CM_commandTimer ),
					0.0f,
					msg,
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
		}
		else
		{
			// cleanup the objvars for any expired/invalid cooldowns
			snprintf(buffer, sizeof(buffer)-1, "visualCooldown.beginTime.%d", iterCooldown->first);
			buffer[sizeof(buffer)-1] = '\0';
			removeObjVarItem(buffer);

			snprintf(buffer, sizeof(buffer)-1, "visualCooldown.endTime.%d", iterCooldown->first);
			buffer[sizeof(buffer)-1] = '\0';
			removeObjVarItem(buffer);
		}
	}

	// command cooldown timers are stored in the command queue
	CommandQueue const * commandQueue = getCommandQueue();
	if (commandQueue && getClient())
	{
		double const currentTime = Clock::getCurrentTime();
		uint32 currentCooldownTimeSecond = 0;
		uint32 maxCooldownTimeSecond = 0;
		std::map<uint32, std::pair<double, double> > const & cooldowns = commandQueue->getCooldowns().getMap();
		for (std::map<uint32, std::pair<double, double> >::const_iterator cooldownsIter = cooldowns.begin(); cooldownsIter != cooldowns.end(); ++cooldownsIter)
		{
			if ((cooldownsIter->second.first <= currentTime) && (currentTime < cooldownsIter->second.second))
			{
				// don't bother sending cooldown with less than 1 second left because
				// by the time it gets to the client, it will probably have already expired
				currentCooldownTimeSecond = static_cast<uint32>(currentTime - cooldownsIter->second.first);
				maxCooldownTimeSecond = static_cast<uint32>(cooldownsIter->second.second - cooldownsIter->second.first);

				if (currentCooldownTimeSecond < maxCooldownTimeSecond)
				{
					GenericValueTypeMessage<std::pair<uint32, std::pair<uint32, uint32> > > commandCooldown("SetCommandCooldown", std::make_pair(cooldownsIter->first, std::make_pair(currentCooldownTimeSecond, maxCooldownTimeSecond)));
					getClient()->send(commandCooldown, true);
				}
			}
		}
	}

	// sync chat character count with chat server
	PlayerObject * const player = PlayerCreatureController::getPlayerObject(this);
	if (player && (player->getChatSpamTimeEndInterval() > 0))
	{
		time_t timeUnsquelch = static_cast<time_t>(player->getSecondsUntilUnsquelched());
		if (timeUnsquelch > 0)
			timeUnsquelch += ::time(nullptr);

		GenericValueTypeMessage<std::pair<std::pair<std::pair<NetworkId, int>, int>, std::pair<int, int> > > chatStatistics("ChatStatisticsGS", std::make_pair(std::make_pair(std::make_pair(getNetworkId(), static_cast<int>(timeUnsquelch)), player->getChatSpamTimeEndInterval()), std::make_pair(player->getChatSpamSpatialNumCharacters(), player->getChatSpamNonSpatialNumCharacters())));
		Chat::sendToChatServer(chatStatistics);
	}

	// if necessary, announce TCG monthly bonus booster pack and Swg Tcg Beta
	if (getClient() && !getClient()->isUsingAdminLogin())
	{
		VeteranRewardManager::announceMonthlyBonusBoosterPack(*this);

		if (ConfigServerGame::getAccountFeatureIdForTcgBetaAnnouncement() > 0)
			VeteranRewardManager::announceSwgTcgBeta(*this);
	}
	// Make sure our player has the server's speed maximum
	PlayerCreatureController const * playerController = safe_cast<PlayerCreatureController const *>(getCreatureController());
	if(playerController)
	{
		GenericValueTypeMessage<float> const msg("fca11a62d23041008a4f0df36aa7dca6", playerController->getServerSpeedForPlayer());
		Client const * const client = getClient();
		if (client)
			return client->send(msg, true);
	}

}

// ----------------------------------------------------------------------

void CreatureObject::onClientAboutToLoad()
{
	if (isAuthoritative())
		setInvulnerabilityTimer(ConfigServerGame::getCreatureLoadInvulnerableTimeWithoutClient());

	TangibleObject::onClientAboutToLoad();
}

// ----------------------------------------------------------------------

void CreatureObject::onLoadingScreenComplete()
{
	TangibleObject::onLoadingScreenComplete();

	if (isAuthoritative() && isInvulnerable())
		setInvulnerabilityTimer(ConfigServerGame::getCreatureLoadInvulnerableTimeWithClient());
}

// ----------------------------------------------------------------------

bool CreatureObject::monitorCreatureMovement(const CachedNetworkId &ofTarget, float i_skittishness, float i_curve)
{
	Object * object = ofTarget.getObject();
	if (!object || object == this)
		return false;

	Archive::AutoDeltaMap<NetworkId, MonitoredCreatureMovement>::const_iterator i = m_monitoredCreatureMovements.find(ofTarget);
	MonitoredCreatureMovement newMovement;
	if (i == m_monitoredCreatureMovements.end())
	{
		newMovement.m_lastDistance = object->getPosition_w().magnitudeBetween(getPosition_w());
		newMovement.m_skittishness = i_skittishness;
		newMovement.m_curve = i_curve;
	}
	else
	{
		newMovement = i->second;
		newMovement.m_skittishness = i_skittishness;
		newMovement.m_curve = i_curve;

	}
	if (newMovement != i->second)
	{
		m_monitoredCreatureMovements.set(ofTarget, newMovement);
	}
	return true;
}

//----------------------------------------------------------------------

bool CreatureObject::ignoreCreatureMovement(const CachedNetworkId &ofTarget)
{
	Archive::AutoDeltaMap<NetworkId, MonitoredCreatureMovement>::const_iterator i = m_monitoredCreatureMovements.find(ofTarget);
	if (i == m_monitoredCreatureMovements.end())
		return false;
	m_monitoredCreatureMovements.erase(ofTarget);
	return true;
}

//----------------------------------------------------------------------

void CreatureObject::reportMonitoredCreatures(float time)
{
	Vector ownerPosition = getPosition_w();
	std::vector<NetworkId> toRemove;
	Archive::AutoDeltaMap<NetworkId, MonitoredCreatureMovement>::const_iterator i;
	for (i=m_monitoredCreatureMovements.begin(); i!=m_monitoredCreatureMovements.end(); ++i)
	{
		CachedNetworkId cnid(i->first);
		Object * o = cnid.getObject();
		if (o)
		{
			MonitoredCreatureMovement newMovement = i->second;

			float lastDistance = newMovement.m_lastDistance;
			float currentDistance = o->getPosition_w().magnitudeBetween(ownerPosition);
			float approach = lastDistance - currentDistance;
			//float tangentVelocity = sqrt(((mobDelta - targetDelta)/i_time).magnitudeSquared() - approachVelocity * approachVelocity);

			// calculate the fear delta
			if (approach > 0)
			{
				float approachVelocity = approach / time;
				float totalDistance = getApproachTriggerRange();
				float percent = 1 - ((lastDistance + currentDistance) * 0.5f) / totalDistance;
				if (percent > 1)
					percent = 1;
				else if (percent < 0)
					percent = 0;
				if (i->second.m_curve < 0)
				{
					percent = -percent;
				}
				float absCurve = std::abs(i->second.m_curve);
				float scale = pow(percent, absCurve) * (1+absCurve);
				float speedScale = float(sqrt(approachVelocity / getRunSpeed()));
				float fearDelta = approach * i->second.m_skittishness * speedScale * scale;
				setMentalStateToward(i->first, MentalStates::Fear, getMentalStateToward(i->first, MentalStates::Fear)+fearDelta);
			}
			newMovement.m_lastDistance = currentDistance;
			m_monitoredCreatureMovements.set(i->first, newMovement);
		}
		else
		{
			toRemove.push_back(i->first);
		}
	}
	std::vector<NetworkId>::iterator removeIt;
	for (removeIt = toRemove.begin(); removeIt != toRemove.end(); ++removeIt)
	{
		m_monitoredCreatureMovements.erase(*removeIt);
	}
}

//-----------------------------------------------------------------------

void CreatureObject::doWarmupChecks(Command const &command, NetworkId const &targetId, Unicode::String const &params, Command::ErrorCode &status, int &statusDetail)
{
	if (doesLocomotionInvalidateCommand(command))
	{
		if (ConfigServerGame::getLogAllCommands())
		{
			LOG(
				"Command",
				("%s>%s %s ignored due to locomotion %d",
					getNetworkId().getValueString().c_str(),
					targetId.getValueString().c_str(),
					command.m_commandName.c_str(),
					getLocomotion()));
		}
		status = Command::CEC_Locomotion;
		statusDetail = static_cast<int>(getLocomotion());
	}
	else if (isPlayerControlled() && command.m_characterAbility.size() && !hasCommand(command.m_characterAbility))
	{
		if (ConfigServerGame::getLogAllCommands())
		{
			LOG(
				"Command",
				("%s>%s %s ignored due to ability %s",
					getNetworkId().getValueString().c_str(),
					targetId.getValueString().c_str(),
					command.m_commandName.c_str(),
					command.m_characterAbility.c_str()));
		}
		status = Command::CEC_Ability;
	}
	// target verification - don't execute the command if a target is
	// required and none is present, or if there is never a target and
	// one is specified
	else if (   (command.m_targetType == Command::CTT_None && targetId != NetworkId::cms_invalid)
			     || (command.m_targetType == Command::CTT_Required && targetId == NetworkId::cms_invalid))
	{
		if (ConfigServerGame::getLogAllCommands())
		{
			LOG(
				"Command",
				("%s>%s %s ignored due to target type",
					getNetworkId().getValueString().c_str(),
					targetId.getValueString().c_str(),
					command.m_commandName.c_str()));
		}
		status = Command::CEC_TargetType;
	}
	else
	{
		// target range checking
		if (command.m_targetType != Command::CTT_None && targetId != NetworkId::cms_invalid && command.m_maxRangeToTargetSquared != 0.0f)
		{
			Object *targetObj = NetworkIdManager::getObjectById(targetId);
			if (!targetObj)
			{
				if (ConfigServerGame::getLogAllCommands())
				{
					LOG(
						"Command",
						("%s>%s %s ignored due to range to target",
							getNetworkId().getValueString().c_str(),
							targetId.getValueString().c_str(),
							command.m_commandName.c_str()));
				}
				status = Command::CEC_TargetRange;
			}
			else
			{
				TangibleObject const * const tangible = dynamic_cast<TangibleObject *>(targetObj);
				if(tangible)
				{
					float distance = this->getDistanceBetweenCollisionSpheres_w(*tangible);
					distance = sqr(distance);

					if(distance > command.m_maxRangeToTargetSquared)
					{
						if (ConfigServerGame::getLogAllCommands())
						{
							LOG(
								"Command",
								("%s>%s %s ignored due to range to target",
								getNetworkId().getValueString().c_str(),
								targetId.getValueString().c_str(),
								command.m_commandName.c_str()));
						}
						status = Command::CEC_TargetRange;
					}
				}
				else
				{
					if(findPosition_w().magnitudeBetweenSquared(targetObj->findPosition_w()) > command.m_maxRangeToTargetSquared)
					{
						if (ConfigServerGame::getLogAllCommands())
						{
							LOG(
								"Command",
								("%s>%s %s ignored due to range to target",
								getNetworkId().getValueString().c_str(),
								targetId.getValueString().c_str(),
								command.m_commandName.c_str()));
						}
					
						status = Command::CEC_TargetRange;
					}
				
				}
				
			}
			
		}
		if (status == Command::CEC_Success)
		{
			// make sure that the command is allowed in all states set on the creature currently
			for (States::Enumerator i = 0; i < States::NumberOfStates; ++i)
			{
				// @todo: don't gate npc command usage on swimming
				if (!isPlayerControlled() && i == States::Swimming)
					continue;

				if (getState(i) && !command.m_statePermissions[i])
				{
					if (ConfigServerGame::getLogAllCommands())
					{
						LOG(
							"Command",
							("%s>%s %s ignored due to state %d",
								getNetworkId().getValueString().c_str(),
								targetId.getValueString().c_str(),
								command.m_commandName.c_str(),
								i));
					}
					status = Command::CEC_StateMustNotHave;
					statusDetail = i;
					break;
				}

				if (!getState(i) && command.m_stateRequired[i])
				{
					if (ConfigServerGame::getLogAllCommands())
					{
						LOG(
							"Command",
							("%s>%s %s ignored due to missing required state %d",
							getNetworkId().getValueString().c_str(),
							targetId.getValueString().c_str(),
							command.m_commandName.c_str(),
							i));
					}
					status = Command::CEC_StateMustHave;
					statusDetail = i;
					break;
				}
			}
		}
	}

}

// ----------------------------------------------------------------------

bool CreatureObject::isDisabled() const
{
	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		return TangibleObject::isDisabled ();

	return (isIncapacitated() || isDead());
}

// -------------------------------------------------------------------

bool CreatureObject::makeDead(const NetworkId & killer, const NetworkId & corpse)
{
	if (isAuthoritative())
	{
		if (isDead())
		{
			clearHateList();
			return true;
		}

		ScriptParams params;
		params.addParam(killer);
		params.addParam(corpse);
		if (getScriptObject()->trigAllScripts(Scripting::TRIG_DEATH, params) != SCRIPT_CONTINUE)
			return false;

		clearHateList();

		// if we are crafting, end the crafting session
		PlayerObject * const player = PlayerCreatureController::getPlayerObject(this);
		if (player != nullptr && player->isCrafting())
			player->stopCrafting(false);

		// if we are in a conversation, end it
		if (isInNpcConversation())
			endNpcConversation();

		// remove all attrib and skill mod mods
		typedef Archive::AutoDeltaMap<uint32, CreatureMod>::const_iterator ModIter;
		for (ModIter i = m_attributeModList.begin(); i != m_attributeModList.end();)
		{
			const CreatureMod & m = (*i).second;

			bool remove = true;
			//check to see if this attribute mod comes from a buff that should not be removed on death
			const char * rawModName = AttribModNameManager::getInstance().getAttribModName(m.mod.tag);
			if(rawModName)
			{
				std::string tmpModName(rawModName);

				//strip the attribute number from the mod name to get the raw buff name
				//this is a little fragile because we are looking up the name of the
				//buff, trusting that the mod name will be based on it
				//in BuffManager.cpp, attributes that are assigned by a buff have the
				//mod name as ("%s_%d", buffname, effectNum) so we will check to see if
				//the name matches that format and if the resulting buff name is flagged
				//to not be removed on death.
				int namelength = tmpModName.size();
				if(namelength > 2 && tmpModName[namelength-2] == '_' && tmpModName[namelength-1] >= '0' && tmpModName[namelength-1] <= '9')
				{
					std::string buffname = tmpModName.substr(0,namelength-2);
					uint32 buffNameCrc = Crc::normalizeAndCalculate(buffname.c_str());
					remove = BuffManager::getIsBuffRemovedOnDeath(buffNameCrc);
					
					// Check to see if this is a Pvp related death and we should ignore the removal.
					if(remove && BuffManager::getDoesBuffDecayOnPvPDeath(buffNameCrc)) // We only care about buffs that would normally be removed on death and have the decay flag.
					{
						Object * killerObj = NetworkIdManager::getObjectById(killer);
						if(killerObj && killerObj->asServerObject() && killerObj->asServerObject()->asCreatureObject())
						{
							CreatureObject * killerCreature = killerObj->asServerObject()->asCreatureObject();
							PlayerObject * const playerKiller = PlayerCreatureController::getPlayerObject(killerCreature);
							if(playerKiller)
								remove =  false; // Buffs that decay on PvP death don't get removed on PvP death.
							else if(killerCreature->getMasterId() != NetworkId::cms_invalid) // Check to see if this is some type of pet.
							{
								Object * masterObj = NetworkIdManager::getObjectById(killerCreature->getMasterId());
								if(masterObj && masterObj->asServerObject() && masterObj->asServerObject()->asCreatureObject())
								{
									CreatureObject * masterCreature = masterObj->asServerObject()->asCreatureObject();
									PlayerObject * const masterPlayer = PlayerCreatureController::getPlayerObject(masterCreature); // Pet has a valid player as a master. Pvp related.
									if(masterPlayer)
										remove = false;
								}
							}
						}
					}// End PvP check
				}
			}

			if(remove)
			{
				if (AttribMod::isSkillMod(m.mod))
				{
					const char * skillModName = AttribModNameManager::getInstance().getAttribModName(m.mod.skill);
					if (skillModName != nullptr)
						addModValue(skillModName, -m.maxVal, true);
				}
				// if the mod is visible, we need to tell the player it's being
				// removed
				if (m.mod.flags & AttribMod::AMF_visible)
				{
					sendCancelTimedMod(m.mod.tag);
				}
				// if the mod is flagged as having a callback, call the script
				if (m.mod.flags & AttribMod::AMF_triggerOnDone)
				{
					const char * modName = AttribModNameManager::getInstance().getAttribModName(m.mod.tag);
					if (modName != nullptr)
					{
						ScriptParams params;
						params.addParam(modName);
						params.addParam(true);
						if (AttribMod::isAttribMod(m.mod))
						{
							IGNORE_RETURN(getScriptObject()->trigAllScripts(
								Scripting::TRIG_ATTRIB_MOD_DONE, params));
						}
						else if (AttribMod::isSkillMod(m.mod))
						{
							IGNORE_RETURN(getScriptObject()->trigAllScripts(
								Scripting::TRIG_SKILL_MOD_DONE, params));
						}

					}
				}

				i = m_attributeModList.erase(i);
			}
			else
			{
				++i;
			}
		}
		for (int j = 0; j < Attributes::NumberOfAttributes; ++j)
		{
			m_cachedCurrentAttributeModValues.set(j, 0);
			m_cachedMaxAttributeModValues.set(j, 0);
		}
		computeTotalAttributes();

		LOG("CustomerService", ("Attribs: removed all attrib and skillmod mods "
			"from %s due to death.",
			PlayerObject::getAccountDescription(this).c_str()));

		if (!isPlayerControlled())
			setDefaultAlterTime(AlterResult::cms_keepNoAlter);

		//-- If we're in combat, the client will take care of arranging for
		//   a suitable transition to the dead posture; otherwise, make it
		//   happen immediately.
		setPosture(Postures::Dead, !isInCombat());
	}
	else
	{
		sendControllerMessageToAuthServer(CM_makeDead, new MessageQueueGenericValueType<
			std::pair<NetworkId, NetworkId> >(std::make_pair(killer, corpse)));
	}
	return true;
}

//--------------------------------------------------------------------

bool CreatureObject::makeNotDead()
{
	if (!isDead())
		return true;

	if (!isPlayerControlled())
		setDefaultAlterTime(AlterResult::cms_alterQuickly);

	setPosture(Postures::Prone, true);
	testIncapacitation(NetworkId::cms_invalid);
	return true;
}

//--------------------------------------------------------------------

Locomotions::Enumerator CreatureObject::calcLocomotion() const
{
	Locomotions::Enumerator ret = Locomotions::Invalid;
	if (m_stopWalkRun.get() > 1)
		ret = m_fastLocomotion;
	if (ret == Locomotions::Invalid && m_stopWalkRun.get() > 0)
		ret = m_slowLocomotion;
	if (ret == Locomotions::Invalid)
		ret = m_stationaryLocomotion;
	return ret;
}

//--------------------------------------------------------------------

bool CreatureObject::canMove() const
{
	return getMovementTable()->hasMovingLocomotion(m_posture.get(), m_states.get()) && !getObjVars().hasItem("forceNoMovement");
}

//----------------------------------------------------------------------

void CreatureObject::setScaleFactor(float scale)
{
	if(isAuthoritative())
	{
		scale = std::max (0.0f, scale);
		m_scaleFactor = scale;
		setScale (Vector::xyz111 * scale);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setScaleFactor, new MessageQueueGenericValueType<float>(scale));
	}
}

//----------------------------------------------------------------------

/**
 * Gets the approximate height of the creature, modified by it's posture.
 *
 * @return the creature's height
 */
float CreatureObject::getHeight() const
{
	float height = TangibleObject::getHeight();
	height *= getMovementTable()->getCanSeeHeightMod(m_posture.get(), m_states.get());

	return height;
}	// CreatureObject::getHeight

//----------------------------------------------------------------------

/**
 * Gets the approximate radius of the object.
 *
 * @return the object's radius
 */
float CreatureObject::getRadius() const
{
	const SharedCreatureObjectTemplate * myTemplate = safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate());
	return myTemplate->getCollisionRadius() * std::max(getScale().x, getScale().z);
}	// CreatureObject::getHeight

//----------------------------------------------------------------------

Object const * CreatureObject::getStandingOn() const
{
	CollisionProperty const * collision = getCollisionProperty();

	if(collision)
	{
		return collision->getStandingOn();
	}
	else
	{
		return nullptr;
	}
}

//----------------------------------------------------------------------

float CreatureObject::getFarNetworkUpdateRadius() const
{
	if (ServerWorld::getSceneId() == NewbieTutorial::getSceneId())
		return 0.f;

	float height = TangibleObject::getHeight();
	float radius;

	if (height < ConfigServerGame::getMaxSmallCreatureHeight())
		radius = ConfigServerGame::getSmallCreatureUpdateRadius();
	else if (height < ConfigServerGame::getMaxMediumCreatureHeight())
		radius = ConfigServerGame::getMediumCreatureUpdateRadius();
	else
		radius = ConfigServerGame::getLargeCreatureUpdateRadius();

	return radius;
}

//----------------------------------------------------------------------

void CreatureObject::setShockWounds(int wound)
{
static const StringId SHOCK_WOUND_ID("cbt_spam", "shock_wound");

	if(isAuthoritative())
	{
		// Use TangibleObject::isInvulnerable so we don't account for the invulnerability timer
		if (TangibleObject::isInvulnerable())
			return;

		if (wound < 0)
			wound = 0;
		else if (wound > 1000)
			wound = 1000;
		if (wound != m_shockWounds.get())
		{
			if (isInCombat())
			{
				ProsePackage pp;
				pp.stringId = SHOCK_WOUND_ID;
				pp.digitInteger = wound - m_shockWounds.get();
				Unicode::String oob;
				OutOfBandPackager::pack(pp, 0, oob);
				MessageQueueCombatSpam spam(oob, false, false, false, 1);
				performCombatSpam (spam, true, false, false);
			}
			m_shockWounds = wound;
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setShockWounds, new MessageQueueGenericValueType<int>(wound));
	}
}

//----------------------------------------------------------------------

void CreatureObject::setLookAtTarget(const NetworkId& id)
{
	if(isAuthoritative())
	{
		m_lookAtTarget.set(id);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setLookAtTarget, new MessageQueueGenericValueType<NetworkId>(id));
	}

	ShipObject * const ship = getPilotedShip();
	if(ship)
	{
		ship->setLookAtTargetFromPilot(id);
	}
}

//----------------------------------------------------------------------

void CreatureObject::setIntendedTarget(const NetworkId& id)
{
	if(isAuthoritative())
	{
		DEBUG_WARNING(true, ("CreatureObject::setIntendedTarget: '%s'", id.getValueString().c_str()));
		m_intendedTarget.set(id);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setIntendedTarget, new MessageQueueGenericValueType<NetworkId>(id));
	}
}

//-----------------------------------------------------------------------

void CreatureObject::setMovementStationary()
{
	if (isAuthoritative())
		m_stopWalkRun = 0;
}

//-----------------------------------------------------------------------

void CreatureObject::setMovementWalk()
{
	if (isAuthoritative())
		m_stopWalkRun = 1;
}

//-----------------------------------------------------------------------

void CreatureObject::setMovementRun()
{
	if (isAuthoritative())
		m_stopWalkRun = 2;
}

//--------------------------------------------------------------------

bool CreatureObject::setSlopeModAngle(float angle)
{
	if(isAuthoritative())
	{
		m_slopeModAngle = angle;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setSlopeModAngle, new MessageQueueGenericValueType<float>(angle));
	}
	return true;
}

//--------------------------------------------------------------------

bool CreatureObject::setSlopeModPercent(float percent)
{
	if(isAuthoritative())
	{
		if (percent < 0.0f)
			percent = 0.0f;
		else if (percent > 1.0f)
			percent = 1.0f;
		m_baseSlopeModPercent = percent;

		//
		// modify the percent reduced due to skill mods
		//

		// get my skill mod
		int movementMod = getEnhancedModValue(SLOPE_MOD);
		if (getGroup() != nullptr)
		{
			// get my group leader skill mod
			const NetworkId & leaderId = getGroup()->getGroupLeaderId();
			if (leaderId != NetworkId::cms_invalid)
			{
				const CreatureObject * leader = safe_cast<const CreatureObject *>(
					NetworkIdManager::getObjectById(leaderId));
				if (leader != nullptr)
				{
					movementMod += leader->getEnhancedModValue(GROUP_SLOPE_MOD);
				}
			}
		}
		if (movementMod > 100)
			movementMod = 100;

		switch (getPosture())
		{
			case Postures::Upright:
				if (movementMod > 50)
					movementMod = 50;
				if (movementMod > 0 && percent < 1.0f)
				{
					percent += (1.0f - percent) * ((float)movementMod / 50.0f);
				}
				break;
			case Postures::Prone:
			case Postures::Sneaking:
//				movementMod -= 50;
//				if (movementMod > 0 && percent < 1.0f)
//				{
//					percent += (1.0f - percent) * ((float)movementMod / 50.0f);
//				}
//				break;
			case Postures::Crouched:
			case Postures::Blocking:
			case Postures::Climbing:
			case Postures::Flying:
			case Postures::LyingDown:
			case Postures::Sitting:
			case Postures::SkillAnimating:
			case Postures::DrivingVehicle:
			case Postures::RidingCreature:
			case Postures::KnockedDown:
			case Postures::Incapacitated:
			case Postures::Dead:
			default:
				break;
		}

		m_slopeModPercent = percent;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setSlopeModPercent, new MessageQueueGenericValueType<float>(percent));
	}
	return true;
}

//--------------------------------------------------------------------

bool CreatureObject::setWaterModPercent(float percent)
{
	if(isAuthoritative())
	{
		m_waterModPercent = percent;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setWaterModPercent, new MessageQueueGenericValueType<float>(percent));
	}
	return true;
}

//--------------------------------------------------------------------

bool CreatureObject::setMovementScale(float scale)
{
	if(isAuthoritative())
	{
		m_movementScale = scale;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setMovementScale, new MessageQueueGenericValueType<float>(scale));
	}
	return true;
}

// ----------------------------------------------------------------------

bool CreatureObject::setMovementPercent(float percent)
{
	if(isAuthoritative())
	{
		m_movementPercent = percent;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setMovementPercent, new MessageQueueGenericValueType<float>(percent));
	}
	return true;
}

//--------------------------------------------------------------------

bool CreatureObject::setTurnPercent(float percent)
{
	if(isAuthoritative())
	{
		m_turnScale = percent;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setTurnPercent, new MessageQueueGenericValueType<float>(percent));
	}
	return true;
}

//--------------------------------------------------------------------

bool CreatureObject::setAccelScale(float scale)
{
	if(isAuthoritative())
	{
		m_accelScale = scale;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setAccelScale, new MessageQueueGenericValueType<float>(scale));
	}
	return true;
}

// ----------------------------------------------------------------------

bool CreatureObject::setAccelPercent(float percent)
{
	if(isAuthoritative())
	{
		m_accelPercent = percent;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setAccelPercent, new MessageQueueGenericValueType<float>(percent));
	}
	return true;
}

//--------------------------------------------------------------------
/**
 * Instruct the clients to display the visuals for having this creature
 * sit on the specified chair-like target object in the specified position.
 *
 * For chairs, where there is only a single spot to sit in, the positionIndex
 * should be zero.  For couches where, theoretically, we could have multiple
 * creatures sit, the value should represent the space in which the player
 * is sitting.
 *
 * @param targetObjectId  the network id for the object on which the creature
 *                        will sit.
 * @param positionIndex   the 0-based index of the position of the chair-like object
 *                        in which the creature will sit.  Typically this will be zero.
 */
void CreatureObject::sitOnObject(NetworkId const &chairCellId, Vector const &chairPosition_p)
{
	if (isAuthoritative())
	{
		// you can't sit on something in a different cell than you
		if (ContainerInterface::getContainedByProperty(*this)->getContainedByNetworkId() != chairCellId)
			return;

		//-- If the chair is more than the allowed distance away from the authoritative position for the creature, log the attempt and ignore.
		Vector const creaturePosition_p = getPosition_p();
		float  const separationSquared  = creaturePosition_p.magnitudeBetweenSquared(chairPosition_p);

		if (separationSquared > MessageQueueSitOnObject::cs_maximumChairRangeSquared)
		{
			LOG("move_validation", ("CreatureObject::sitOnObject: object id=[%s] out of range for specified chair position, separation=[%.2f] meters.", getNetworkId().getValueString().c_str(), separationSquared));
			return;
		}

		//-- Get the controller.
		Controller * const controller = getController();
		NetworkController * const networkController = safe_cast<NetworkController *>(controller);
		ServerController * const serverController = safe_cast<ServerController *>(networkController);
		NOT_NULL(serverController);

		// teleport the player to the chairs location
		// orientation doesn't matter in this case
		Transform transform;
		transform.setPosition_p(chairPosition_p);
		serverController->teleport(transform, ContainerInterface::getContainingCellObject(*this));

		//-- Send the SitOnObject controller message to all clients.
		serverController->appendMessage(CM_sitOnObject, 0.0f, new MessageQueueSitOnObject(chairCellId, chairPosition_p), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_ALL_CLIENT);

		//-- Set the server posture to sitting & set SittingInChair state.
		setPosture(Postures::Sitting);
		setState(States::SittingOnChair, true);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_sitOnObject, new MessageQueueSitOnObject(chairCellId, chairPosition_p));
	}
}

// ----------------------------------------------------------------------

void CreatureObject::onBiographyRetrieved(const NetworkId &owner, const Unicode::String &bio)
{
	//TODO:  Biography handling code
	//DEBUG_REPORT_LOG(true,("Biography for object %s received:  %s\n",owner.getValueString().c_str(), Unicode::wideToNarrow(bio).c_str()));

	CreatureController *creatureController = getCreatureController();

	if (creatureController != nullptr)
	{
		typedef std::pair<NetworkId, Unicode::String> Payload;

		MessageQueueGenericValueType<Payload> * const msg = new MessageQueueGenericValueType<Payload>(Payload(owner, bio));

		creatureController->appendMessage(static_cast<int>(CM_biographyRetrieved), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

//-----------------------------------------------------------------------

void CreatureObject::onCharacterMatchRetrieved(MatchMakingCharacterResult const &results)
{
	CreatureController *creatureController = getCreatureController();

	if (creatureController != nullptr)
	{
		MessageQueueGenericValueType<MatchMakingCharacterResult> * const msg = new MessageQueueGenericValueType<MatchMakingCharacterResult>(results);

		creatureController->appendMessage(static_cast<int>(CM_characterMatchRetrieved), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

//-----------------------------------------------------------------------

void CreatureObject::setLocomotion(Locomotions::Enumerator posture)
{
	if (!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_setLocomotion, new MessageQueueGenericValueType<Locomotions::Enumerator>(posture));
	}
	else
	{
		Postures::Enumerator newPosture;
		MovementTable::eLocomotionSpeed speed = getMovementTable()->getLocomotionData(posture, newPosture);
		if (speed != MovementTable::kInvalid)
		{
			setPosture(newPosture);
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::speakText(MessageQueueSpatialChat const &spatialChat)
{
	// a speech type may only be used if it either has no skills listed as providing it, or the creature has
	// at least one of the skills listed.

	std::vector<std::string> const &skillsProviding = SpatialChatManager::getSkillsProviding(spatialChat.getChatType());
	if (!skillsProviding.empty())
	{
		bool found = false;
		for (std::vector<std::string>::const_iterator i = skillsProviding.begin(); i != skillsProviding.end(); ++i)
		{
			if (hasCommand(*i))
			{
				found = true;
				break;
			}
		}
		if (!found)
			return;
	}

	TangibleObject::speakText(spatialChat);
}

// ----------------------------------------------------------------------

int CreatureObject::getPerformanceType() const
{
	return m_performanceType.get();
}

// ----------------------------------------------------------------------

void CreatureObject::setPerformanceType(int performanceType)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			MessageQueueGenericValueType<int> *msg = new MessageQueueGenericValueType<int>(performanceType);
			controller->appendMessage(
				CM_setPerformanceType,
				0.0f,
				msg,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		m_performanceType = performanceType;
	}
}

// ----------------------------------------------------------------------

int CreatureObject::getPerformanceStartTime() const
{
	return m_performanceStartTime.get();
}

// ----------------------------------------------------------------------

void CreatureObject::setPerformanceStartTime(int performanceStartTime)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			MessageQueueGenericValueType<int> *msg = new MessageQueueGenericValueType<int>(performanceStartTime);
			controller->appendMessage(
				CM_setPerformanceStartTime,
				0.0f,
				msg,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		m_performanceStartTime = performanceStartTime;
	}
}

// ----------------------------------------------------------------------

NetworkId const &CreatureObject::getPerformanceListenTarget() const
{
	return m_performanceListenTarget.get();
}

// ----------------------------------------------------------------------

void CreatureObject::setPerformanceListenTarget(NetworkId const &who)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			MessageQueueGenericValueType<NetworkId> *msg = new MessageQueueGenericValueType<NetworkId>(who);
			controller->appendMessage(
				CM_setPerformanceListenTarget,
				0.0f,
				msg,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		m_performanceListenTarget = who;
	}
}

// ----------------------------------------------------------------------

NetworkId const &CreatureObject::getPerformanceWatchTarget() const
{
	return m_performanceWatchTarget.get();
}

// ----------------------------------------------------------------------

void CreatureObject::setPerformanceWatchTarget(NetworkId const &who)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			MessageQueueGenericValueType<NetworkId> *msg = new MessageQueueGenericValueType<NetworkId>(who);
			controller->appendMessage(
				CM_setPerformanceWatchTarget,
				0.0f,
				msg,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		m_performanceWatchTarget = who;
	}
}

//-----------------------------------------------------------------------

void CreatureObject::setGuildId(int guildId)
{
	FATAL(!isAuthoritative(), ("setGuildId called on nonauthoritative object"));
	int oldGuildId = getGuildId();
	if (oldGuildId != guildId)
	{
		if (oldGuildId)
			GuildInterface::onAboutToClearGuildId(*this);
		m_guildId = guildId;
	}
	if (guildId)
		GuildInterface::onGuildIdSet(*this);
}

//-----------------------------------------------------------------------

void CreatureObject::setTimeToUpdateGuildWarPvpStatus(unsigned long timeToUpdateGuildWarPvpStatus)
{
	FATAL(!isAuthoritative(), ("setTimeToUpdateGuildWarPvpStatus called on nonauthoritative object"));
	m_timeToUpdateGuildWarPvpStatus = timeToUpdateGuildWarPvpStatus;
}

//-----------------------------------------------------------------------

void CreatureObject::setGuildWarEnabled(bool guildWarEnabled)
{
	FATAL(!isAuthoritative(), ("setGuildWarEnabled called on nonauthoritative object"));
	m_guildWarEnabled = guildWarEnabled;
	m_timeToUpdateGuildWarPvpStatus = 0;
}

// ----------------------------------------------------------------------

int CreatureObject::getMilitiaOfCityId() const
{
	return m_militiaOfCityId.get();
}

//-----------------------------------------------------------------------

void CreatureObject::setMilitiaOfCityId(int cityId)
{
	FATAL(!isAuthoritative(), ("setMilitiaOfCityId called on nonauthoritative object"));
	m_militiaOfCityId = cityId;
}

// ----------------------------------------------------------------------

int CreatureObject::getLocatedInCityId() const
{
	return m_locatedInCityId.get();
}

//-----------------------------------------------------------------------

void CreatureObject::setLocatedInCityId(int newCityId)
{
	FATAL(!isAuthoritative(), ("setLocatedInCityId called on nonauthoritative object"));
	int oldCityId = m_locatedInCityId.get();
	if (oldCityId != newCityId)
	{
		m_locatedInCityId.set(newCityId);
		if (getScriptObject())
		{
			ScriptParams params;
			params.addParam(oldCityId);
			params.addParam(newCityId);
			IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_CITY_CHANGED, params));
		}
	}
}

// ----------------------------------------------------------------------

NetworkId const &CreatureObject::getMasterId() const
{
	return m_masterId.get();
}

// ----------------------------------------------------------------------

void CreatureObject::setMasterId(NetworkId const &masterId)
{
	if (isAuthoritative())
		m_masterId = masterId;
	else
		sendControllerMessageToAuthServer(CM_setMasterId, new MessageQueueGenericValueType<NetworkId>(masterId));
}

// ----------------------------------------------------------------------

float CreatureObject::getSwimHeight() const
{
	return safe_cast<const SharedCreatureObjectTemplate*>(getSharedTemplate())->getSwimHeight() * getScaleFactor();
}

//-----------------------------------------------------------------------

const CreatureObject::AllCreaturesSet & CreatureObject::getAllCreatures()
{
	return g_creatureList;
}

//-----------------------------------------------------------------------

void CreatureObject::addToMissionRequestQueue(const NetworkId & terminalId)
{
	if (!ConfigServerGame::getDisableMissions())
	{
		std::vector<std::pair<CreatureObject *, NetworkId> >::const_iterator f = std::find(gs_missionRequestQueue.begin(), gs_missionRequestQueue.end(), std::make_pair(this, terminalId));
		if(f == gs_missionRequestQueue.end())
		{
			gs_missionRequestQueue.push_back(std::make_pair(this, terminalId));
		}
	}
}

//-----------------------------------------------------------------------

void CreatureObject::removeFromMissionRequestQueue(const NetworkId & creatureId)
{
	CreatureObject * creature = safe_cast<CreatureObject *>(NetworkIdManager::getObjectById(creatureId));
	if (creature)
	{
		removeCreatureFromMissionRequestQueue(creature);
	}
}

//-----------------------------------------------------------------------

void CreatureObject::updateMissionRequestQueue()
{
	if(! gs_missionRequestQueue.empty())
	{
		// remove pending creatures from previous run
		std::vector<std::pair<CreatureObject *, NetworkId> >::const_iterator remIter;
		std::vector<std::pair<CreatureObject *, NetworkId> >::iterator i;
		for(remIter = gs_missionRequestQueueDeferredRemoves.begin(); remIter != gs_missionRequestQueueDeferredRemoves.end(); ++remIter)
		{
			for(i = gs_missionRequestQueue.begin(); i != gs_missionRequestQueue.end(); ++i)
			{
				if((*i).first == (*remIter).first)
				{
					gs_missionRequestQueue.erase(i);
					break;
				}
			}
		}

		gs_missionRequestQueueDeferredRemoves.clear();
		int count = 0;

		for(i = gs_missionRequestQueue.begin(); i != gs_missionRequestQueue.end(); ++i)
		{
			CreatureObject * creature = (*i).first;

			// if a delete happened while processing the queue, find the
			// creature in the defferred deleted queue. If it's there, skip this
			// creature and move on to the next
			if(! gs_missionRequestQueueDeferredRemoves.empty())
			{
				// is this creature in the queue?
				remIter = std::find(gs_missionRequestQueueDeferredRemoves.begin(), gs_missionRequestQueueDeferredRemoves.end(), std::make_pair(creature, (*i).second));
				if(remIter != gs_missionRequestQueueDeferredRemoves.end())
					continue;
			}

			// build vector of network ids for all mission objects in the mission bag
			ServerObject * missionBag = creature->getMissionBag();
			if(missionBag)
			{
				static std::vector<NetworkId> missionObjects;

				Container * container = ContainerInterface::getContainer(*missionBag);
				ContainerIterator iter;
				for(iter = container->begin(); iter != container->end(); ++iter)
				{
					MissionObject * missionObject = safe_cast<MissionObject *>((*iter).getObject());
					if(missionObject)
					{
						missionObjects.push_back(missionObject->getNetworkId());
					}
				}

				ScriptParams p;
				p.addParam(creature->getNetworkId());
				p.addParam((*i).second);
				p.addParam(missionObjects);
				GameScriptObject * script = creature->getScriptObject();
				if(script)
					IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_PLAYER_REQUEST_MISSION_BOARD, p));
				removeCreatureFromMissionRequestQueue(creature);
				missionObjects.clear();
			}
			++count;
			if (count >= ConfigServerGame::getMissionRequestsPerFrame())
			{
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	TangibleObject::getAttributes(data);
}

//-----------------------------------------------------------------------

bool CreatureObject::isAppearanceEquippable(const char *appearanceTemplateName)
{
	// Returns whether an object template is wearable based on the data in the
	// appearance_table.tab

	bool result = true;

	// Make sure this object has a valid appearance

	if (appearanceTemplateName == nullptr)
	{
		result = false;
	}
	else
	{
		// Check to see if we even care about the creature object depending
		// on its gender/species combination

		SharedCreatureObjectTemplate::Species species = getSpecies();
		SharedCreatureObjectTemplate::Gender gender = getGender();

		int column = -1;

		// Get the species index

		switch (species)
		{
			case SharedCreatureObjectTemplate::SP_human:       { column =  1; } break;
			case SharedCreatureObjectTemplate::SP_rodian:      { column =  3; } break;
			case SharedCreatureObjectTemplate::SP_monCalamari: { column =  5; } break;
			case SharedCreatureObjectTemplate::SP_wookiee:     { column =  7; } break;
			case SharedCreatureObjectTemplate::SP_twilek:      { column =  9; } break;
			case SharedCreatureObjectTemplate::SP_trandoshan:  { column = 11; } break;
			case SharedCreatureObjectTemplate::SP_zabrak:      { column = 13; } break;
			case SharedCreatureObjectTemplate::SP_bothan:      { column = 15; } break;
			case SharedCreatureObjectTemplate::SP_ithorian:    { column = 17; } break;
			case SharedCreatureObjectTemplate::SP_sullustan:   { column = 19; } break;
			default: { } break;
		}

		// Add in the gender index

		switch (gender)
		{
			case SharedCreatureObjectTemplate::GE_male:   { } break;
			case SharedCreatureObjectTemplate::GE_female: { column += 1; } break;
			default: { } break;
		}

		if (column > 0)
		{
			// If the object template is in the list, then check if it is equippable, otherwise
			// there is no restriction so the item must be equippable

			std::string objectTemplateName(appearanceTemplateName);

			if (!objectTemplateName.empty())
			{
				// Strip out just the object template name

				size_t const slashIndex = objectTemplateName.rfind('/');
				size_t const dotIndex   = objectTemplateName.rfind('.');

				std::string fileName;

				if (slashIndex == std::string::npos)
				{
					fileName = objectTemplateName.substr(0, dotIndex);
				}
				else if (dotIndex == std::string::npos)
				{
					fileName = objectTemplateName.substr(slashIndex + 1);
				}
				else
				{
					fileName = objectTemplateName.substr(slashIndex + 1, dotIndex - slashIndex - 1);
				}

				if (AppearanceManager::isAppearanceManaged(fileName))
				{
					// A ':block' token in the data table denotes the item is not equippable

					std::string destAppearancePath;

					AppearanceManager::getAppearanceName(destAppearancePath, fileName, column);

					static const std::string tagBlock = ":block";

					if (destAppearancePath == tagBlock)
					{
						result = false;
					}
				}
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------

/**
 * Sends a message to the Planet Server to update the position of the object.
 */
void CreatureObject::updatePlanetServerInternal(const bool forceUpdate) const
{
	if (forceUpdate || getPositionChanged())
	{
		// If this is a mount with a rider on it, use the larger of the rider and mount's
		// interest radius for the mount.  This allows the planet server to update nodes
		// marked for proxy subscription properly.  Otherwise we'll get a destroy proxy
		// for the mount the first time it moves into a node that hasn't yet been subscribed,
		// immediately followed by a create proxy message for the mount when the rider's
		// position gets updated.

		int effectiveInterestRadius = getInterestRadius();
		if (getState(States::MountedCreature))
		{
			CreatureObject const *const rider = getPrimaryMountingRider();
			if (rider)
				effectiveInterestRadius = std::max(rider->getInterestRadius(), getInterestRadius());
		}

		if (!ContainerInterface::getContainedByObject(*this) || (effectiveInterestRadius > 0))
		{
			bool hibernating = false;
			if(getCreatureController())
				hibernating = getCreatureController()->getHibernate();

			AICreatureController const * const aiCreatureController = dynamic_cast<AICreatureController const *>(getCreatureController());

			Object const * const topmostContainer = ContainerInterface::getTopmostContainer(*this);
			FATAL(!topmostContainer, ("Object %s was contained by something, but getTopmostContainer() returned nullptr", getNetworkId().getValueString().c_str()));
			Vector const &position = topmostContainer->getPosition_p();
			UpdateObjectOnPlanetMessage const msg(
				getNetworkId(),
				topmostContainer == this ? NetworkId::cms_invalid : topmostContainer->getNetworkId(),
				static_cast<int>(position.x),
				static_cast<int>(position.y),
				static_cast<int>(position.z),
				effectiveInterestRadius,
				static_cast<int>(getLocationReservationRadius()),
				false,
				true,
				static_cast<int>(getObjectType()),
				static_cast<int>(getLevel()),
				hibernating,
				getTemplateCrc(),
				aiCreatureController ? aiCreatureController->getMovementType() : -1,
				getIsStatic() == true ? 1 : 0);
			GameServer::getInstance().sendToPlanetServer(msg);
		}
	}
}

// ----------------------------------------------------------------------

int CreatureObject::getCurrentTargetsTotal(const std::map<Attributes::Enumerator, Attributes::Value> & targets)
{
	int sum = 0;
	for(std::map<Attributes::Enumerator, Attributes::Value>::const_iterator i = targets.begin(); i != targets.end(); ++i)
	{
		sum += i->second;
	}
	return sum;
}

// ======================================================================

ServerObject *CreatureObject::getMissionBag() const
{
	SlottedContainer const * const s = ContainerInterface::getSlottedContainer(*this);
	if (s)
	{
		static SlotId const bankId = SlotIdManager::findSlotId(MISSION_BAG_SLOT_NAME);
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		Object * const o = s->getObjectInSlot(bankId, tmp).getObject();
		if (o)
			return o->asServerObject();
	}
	return 0;
}

// ======================================================================

ServerObject *CreatureObject::getBankContainer() const
{
	return const_cast<CreatureObject *>(this)->getBank ();
}

// ======================================================================

namespace PlayerBanks
{
	const static Unicode::String invalidBankName =
		Unicode::narrowToWide("INVALID_BANK_NAME");
}

//----------------------------------------------------------------------

Unicode::String CreatureObject::getBankName() const
{
	Unicode::String bankName = PlayerBanks::invalidBankName;
	getObjVars().getItem(OBJVAR_BANK_ID,bankName);
	return bankName;
}

// ======================================================================

bool CreatureObject::isBankMember(Unicode::String bankName) const
{
	return (bankName.compare(getBankName()) == 0);
}

// ======================================================================

bool CreatureObject::joinBank(Unicode::String bankName)
{
	if (getBankName() == bankName)
	{
		StringId message("system_msg", "already_member_of_bank");
		Unicode::String outOfBand;
		Chat::sendSystemMessage(*this, message, outOfBand);
		return false;
	}
	if (getBankName() != PlayerBanks::invalidBankName)
	{
		StringId message("system_msg", "member_of_different_bank");
		Unicode::String outOfBand;
		Chat::sendSystemMessage(*this, message, outOfBand);
		return false;
	}
	StringId message("system_msg", "succesfully_joined_bank");
	Unicode::String outOfBand;
	Chat::sendSystemMessage(*this, message, outOfBand);
//	printf("Setting item (%s) on objvars (%x)\n", OBJVAR_BANK_ID.c_str(), (int)objVars);
	setObjVarItem(OBJVAR_BANK_ID, bankName);
	/*DynamicVariable *objvar = objVars->getItemByName(OBJVAR_BANK_ID);
	if (objvar)
	{
		DynamicVariableString *stringVar = dynamic_cast<DynamicVariableString *>(objvar);
		stringVar->setValue(bankName);
	}*/
	return true;
}

// ======================================================================

void CreatureObject::quitBank()
{
	ServerObject *bankContainer = getBankContainer();
	if (bankContainer)
	{
		Container *container = ContainerInterface::getContainer(*bankContainer);
		if (container->getNumberOfItems() != 0)
		{
			//Can't quit bank, you still have items in it
			StringId message("system_msg", "bank_not_empty");
			Unicode::String outOfBand;
			Chat::sendSystemMessage(*this, message, outOfBand);
			return;
		}
	}
	StringId message("system_msg", "succesfully_quit_bank");
	Unicode::String outOfBand;
	Chat::sendSystemMessage(*this, message, outOfBand);
	setObjVarItem(OBJVAR_BANK_ID, PlayerBanks::invalidBankName);
	/*DynamicVariable *objvar = objVars->getItemByName(OBJVAR_BANK_ID);
	if (objvar)
	{
		DynamicVariableString *stringVar = dynamic_cast<DynamicVariableString *>(objvar);
		stringVar->setValue(PlayerBanks::invalidBankName);
	}*/
	setObjVarItem(OBJVAR_OPEN_BANK_TERMINAL_ID, NetworkId::cms_invalid);
	if (bankContainer)
	{
		ScriptParams params;
		params.addParam(getNetworkId());
		IGNORE_RETURN (bankContainer->getScriptObject()->trigAllScripts(Scripting::TRIG_CLOSED_CONTAINER, params))
		;
		Client *client = getClient();
		if (client)
		{
			GenericValueTypeMessage<NetworkId> m("ClosedContainerMessage", bankContainer->getNetworkId());
			client->send(m, true);
			ObserveTracker::onClientClosedContainer(*client, *bankContainer);
		}
	}
}

// ----------------------------------------------------------------------

float CreatureObject::getInvulnerabilityTimer() const
{
	return m_invulnerabilityTimer.get();
}

// ----------------------------------------------------------------------

void CreatureObject::setInvulnerabilityTimer(float duration)
{
	if (isAuthoritative())
	{
		DEBUG_REPORT_LOG(true, ("Setting creature invulnerability timer for %s to %g\n", getDebugInformation().c_str(), duration));
		m_invulnerabilityTimer = duration;
	}
}

// ----------------------------------------------------------------------

bool CreatureObject::isInvulnerable() const
{
	if (m_invulnerabilityTimer.get() > 0.f)
		return true;
	return TangibleObject::isInvulnerable();
}

// ----------------------------------------------------------------------

void CreatureObject::handleCMessageTo(MessageToPayload const &message)
{
	if (message.getMethod() == "C++RenameFailed")
	{
		std::string reason(message.getDataAsString());
		if (reason.empty())
			reason = "Rename request failed.";

		Chat::sendSystemMessage(*this,Unicode::narrowToWide(reason),Unicode::String());
	}
	else if ((message.getMethod() == "C++PlayerRenameRequestSubmitted") || (message.getMethod() == "C++PlayerLastNameRenameRequestSubmitted"))
	{
		// set objvar to indicate there's a pending rename request for this character,
		// and the time of the rename request, to enforce the 90 days wait between rename
		if (!getClient() || !getClient()->isGod())
			setObjVarItem("renameCharacterRequest.requestTime", static_cast<int>(::time(nullptr)));

		std::string const newName(message.getDataAsString());
		if (getObjVars().hasItem("renameCharacterRequest.requestTime") && !newName.empty())
		{
			setObjVarItem("renameCharacterRequest.requestNewName", newName);
		}

		// for player requested rename, send message to CentralServer to drop the character,
		// and prevent the character from further login until the rename request has been completed
		PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject)
		{
			GenericValueTypeMessage<std::pair<unsigned int, std::pair<NetworkId, std::pair<std::string, bool> > > > const msg("PlayerRenameRequestSubmitted", std::make_pair(static_cast<unsigned int>(playerObject->getStationId()), std::make_pair(getNetworkId(), std::make_pair(newName, (message.getMethod() == "C++PlayerLastNameRenameRequestSubmitted")))));
			GameServer::getInstance().sendToCentralServer(msg);
		}
	}
	else if (message.getMethod() == "C++GmRenameRequestSubmitted")
	{
		std::string reason(message.getDataAsString());
		if (reason.empty())
			reason = "Rename request submitted.";

		Chat::sendSystemMessage(*this,Unicode::narrowToWide(reason),Unicode::String());
	}
	else if (message.getMethod() == "C++experience")
	{
		std::vector<int8> const & packedData = message.getPackedDataVector();
		std::vector<int8>::const_iterator spacePos = std::find(packedData.begin(), packedData.end(), ' ');
		if (spacePos != packedData.end())
		{
			int amount = atoi(std::string(spacePos + 1, packedData.end()).c_str());
			grantExperiencePoints(std::string(packedData.begin(), spacePos), amount);
		}
	}
	else if (message.getMethod() == "C++AddJediSlot")
	{
		// if we haven't received an addJediToAccountAck(), resend the Jedi request
		if (!getObjVars().hasItem(OBJVAR_ADD_JEDI_ACK))
		{
			PlayerObject * const player = PlayerCreatureController::getPlayerObject(this);
			if (player != nullptr)
			{
				player->addJediToAccount();
				MessageToQueue::getInstance().sendMessageToC(getNetworkId(),
					"C++AddJediSlot", "", 5 * 60, true);
			}
		}
		else
		{
			removeObjVarItem(OBJVAR_ADD_JEDI_ACK);
		}
	}
	else if (message.getMethod() == "ClientMfdStatusUpdateMessage")
	{
		if (getClient() && isAuthoritative())
		{
			if (!message.getPackedDataVector().empty())
			{
				static Archive::ByteStream bs;
				bs.clear();
				bs.put((&(message.getPackedDataVector()[0])), message.getPackedDataVector().size());
				Archive::put(bs, message.getPackedDataVector());
				Archive::ReadIterator ri = bs.begin();
				ClientMfdStatusUpdateMessage mfdUpdate(ri);
				getClient()->send(mfdUpdate, false);
			}
		}
	}
	else if (message.getMethod() == "FindFriend")
	{
		if (getClient() && isAuthoritative())
		{
			if (!message.getPackedDataVector().empty())
			{
				PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
				if (playerObject)
				{
					static Archive::ByteStream bs;
					bs.clear();
					bs.put((&(message.getPackedDataVector()[0])), message.getPackedDataVector().size());
					Archive::put(bs, message.getPackedDataVector());
					Archive::ReadIterator ri = bs.begin();
					NetworkId source;
					std::string sourceName;
					Archive::get(ri, source);
					Archive::get(ri, sourceName);
					playerObject->replyLocationToFriend(sourceName, source);
				}
			}
		}
	}
	else if (message.getMethod() == "LocationToFriend")
	{
		if (getClient() && isAuthoritative())
		{
			if (!message.getPackedDataVector().empty())
			{
				PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
				if (playerObject)
				{
					static Archive::ByteStream bs;
					bs.clear();
					bs.put((&(message.getPackedDataVector()[0])), message.getPackedDataVector().size());
					Archive::put(bs, message.getPackedDataVector());
					Archive::ReadIterator ri = bs.begin();
					Location friendLocation;
					NetworkId friendId;
					Archive::get(ri, friendLocation);
					Archive::get(ri, friendId);
					playerObject->receiveLocationToFriend(friendId, friendLocation);
				}
			}
		}
	}
	else if (message.getMethod() == "FailLocationToFriend")
	{
		if (getClient() && isAuthoritative())
		{
			if (!message.getPackedDataVector().empty())
			{
				PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
				if (playerObject)
				{
					static Archive::ByteStream bs;
					bs.clear();
					bs.put((&(message.getPackedDataVector()[0])), message.getPackedDataVector().size());
					Archive::put(bs, message.getPackedDataVector());
					Archive::ReadIterator ri = bs.begin();
					NetworkId friendId;
					Archive::get(ri, friendId);
					playerObject->receiveFailLocationToFriend(friendId);
				}
			}
		}
	}
	else if (message.getMethod() == "C++TheaterDestroyed")
	{
		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject)
		{
			playerObject->handleCMessageTo(message);
		}
	}
	else if (message.getMethod() == "C++RemovePetFromGroup")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string packedData(message.getPackedDataVector().begin(), message.getPackedDataVector().end());

			// Parse the info from the message
			char     petNetworkIdBuffer[100];
			unsigned msgSendCount;
			sscanf(packedData.c_str(), "%s %u", petNetworkIdBuffer, &msgSendCount);

			// Turn the string into a network Id
			NetworkId petNetworkId( petNetworkIdBuffer );

			// Try to remove the pet from the group
			GroupObject *group = getGroup();
			if (group && group->isGroupMember(petNetworkId))
			{
				group->removeGroupMember(petNetworkId);
			}
			else
			{
				// We will only try to remove the pet so many times...
				if (msgSendCount < MAX_ATTEMPTS_TO_REMOVE_PET_FROM_GROUP)
				{
					// We will try again in the future...
					++msgSendCount;

					// We need to tell the network ID of the pet and we need to keep
					// of how many times this message has been sent to the inviter
					char msgBuffer[100];
					sprintf( msgBuffer, "%s %u", petNetworkId.getValueString().c_str(), msgSendCount );

					// Send the message with a delay
					MessageToQueue::getInstance().sendMessageToC( getNetworkId(),
					                                              "C++RemovePetFromGroup",
					                                              msgBuffer,
					                                              TIME_BETWEEN_ATTEMPTS_TO_REMOVE_PET,
					                                              false );  // reliable
				}
			}
		}
	}
	else if (message.getMethod() == "C++WaitForPatrolPreload")
	{
		if (ServerWorld::isPreloadComplete())
		{
			DEBUG_REPORT_LOG(ConfigServerGame::isAiLoggingEnabled(),("C++WaitForPatrolPreload being handled for %s\n", getNetworkId().getValueString().c_str()));

			MessageToQueue::cancelRecurringMessageTo(getNetworkId(), "C++WaitForPatrolPreload");

			AICreatureController * const aiCreatureController = safe_cast<AICreatureController * const>(getController());
			if (aiCreatureController != nullptr)
			{
				const std::string data = message.getDataAsString();
				std::string::size_type locationStart = 0;
				std::string::size_type  locationEnd = data.find('|', locationStart);
				std::vector<Unicode::String> locations;
				while (locationEnd != std::string::npos)
				{
					locations.push_back(Unicode::narrowToWide(data.substr(locationStart, locationEnd - locationStart)));
					locationStart = locationEnd + 1;
					locationEnd = data.find('|', locationStart);
				}

				bool random = false;
				bool flip = false;
				bool repeat = false;

				if (data.size() > (locationStart+2))
				{
					random = data.at(locationStart++) != '0';
					flip = data.at(locationStart++) != '0';
					repeat = data.at(locationStart++) != '0';
				}
				int startPoint = 0;
				if (data.size() > locationStart && data.at(locationStart) == '*')
				{
					++locationStart;
					startPoint = atoi(data.substr(locationStart).c_str());
				}

				aiCreatureController->patrol(locations, random, flip, repeat, startPoint);
			}
		}
	}
	else if (message.getMethod() == "C++WarnAboutVendorPurge")
	{
		LOG("CustomerService",("Purge:  Sending warning emails to the owner (%s) of vendor (%s) effected by a purge",getNetworkId().getValueString().c_str(),message.getDataAsString().c_str()));

		const StringId subject("player_structure", "vendor_purge_warning_subject");
		const StringId body("player_structure", "vendor_purge_warning_body");
		const Unicode::String mail_subject = Unicode::narrowToWide("@" + subject.getCanonicalRepresentation());
		/*const Unicode::String mail_body = Unicode::narrowToWide("@" + body.getCanonicalRepresentation());*/

		ProsePackage pp;
		pp.stringId     = body;
		pp.other.str    = Unicode::narrowToWide(message.getDataAsString());
			
		Unicode::String oob;
		OutOfBandPackager::pack(pp, -1, oob);

		Chat::sendPersistentMessage("Galactic Housing Authority", Unicode::wideToNarrow(getObjectName()), 
									mail_subject, Unicode::emptyString, oob); 
	}
	else if (message.getMethod() == "C++CheckCharacterTitle")
	{
		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject && !playerObject->getTitle().empty())
		{
			static uint32 const commandHash = Crc::normalizeAndCalculate("setCurrentSkillTitle");
			const Command &command = CommandTable::getCommand(static_cast<uint32>(commandHash));
			if (!command.isNull())
			{
				commandQueueEnqueue(command, NetworkId::cms_invalid, Unicode::narrowToWide(playerObject->getTitle()), 0, false);
			}
		}
	}
	else if (message.getMethod() == "C++CharacterSheetInfoResidenceLocationRsp")
	{
		if (!message.getPackedDataVector().empty())
		{
			// location of residence
			std::string const residenceLoc(message.getPackedDataVector().begin(), message.getPackedDataVector().end());

			// get the city(ies) this character is a citizen of
			std::string citizensOf;
			std::vector<int> const & cityId = CityInterface::getCitizenOfCityId(getNetworkId());
			if (!cityId.empty())
			{
				std::string cityName;
				for (std::vector<int>::const_iterator iterCity = cityId.begin(); iterCity != cityId.end(); ++iterCity)
				{
					CityInfo const & cityInfo = CityInterface::getCityInfo(*iterCity);
					cityName = cityInfo.getCityName();
					if (cityName.empty())
						continue;

					if (!citizensOf.empty())
						citizensOf += ",";

					if (getNetworkId() == cityInfo.getLeaderId())
						citizensOf += "*";

					citizensOf += cityName;
				}
			}

			// send to client
			GenericValueTypeMessage<std::pair<std::string, std::string> > message("CharacterSheetResponseResLoc", std::make_pair(residenceLoc, citizensOf));
			Client * const client = getClient();
			if (client)
				client->send(message, true);
		}
	}
	else if (message.getMethod() == "C++SpammerReq")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			std::string::size_type const pos = params.find(' ');
			if ((pos != std::string::npos) && (params.size() > (pos + 1)))
			{
				NetworkId const actor(params.substr(0, pos));
				std::string const actorName(params.substr(pos + 1));
				PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
				if (playerObject)
				{
					bool canSpammer = true;

					// on a session-authenticated cluster, cannot /spammer a secured login character
					if (getClient() && getClient()->isGodValidated())
					{
						if (!ConfigServerGame::getAdminGodToAll() && !ConfigServerGame::getEnableWardenCanSquelchSecuredLogin())
						{
							canSpammer = false;
						}
					}

					// cannot /spammer self
					if (canSpammer && (actor == getNetworkId()))
					{
						canSpammer = false;
					}

					// cannot /spammer a target that is already squelched
					if (canSpammer && (playerObject->getSecondsUntilUnsquelched() != 0))
					{
						canSpammer = false;
					}

					if (!canSpammer)
					{
						MessageToQueue::getInstance().sendMessageToC(actor,
							"C++SpammerRspCannotSpammerTarget",
							Unicode::wideToNarrow(getAssignedObjectName()),
							0,
							false);
					}
					else
					{
						// do it
						playerObject->squelch(actor, actorName, ConfigServerGame::getWardenSquelchDurationSeconds());

						std::string result = getNetworkId().getValueString();
						result += "|";
						result += Unicode::wideToNarrow(getAssignedObjectName());
						result += "|";
						result += playerObject->getAccountDescription();

						MessageToQueue::getInstance().sendMessageToC(actor,
							"C++SpammerRspSuccess",
							result,
							0,
							false);
					}
				}
				else
				{
					MessageToQueue::getInstance().sendMessageToC(actor,
						"C++SpammerRspInvalidTarget",
						std::string(),
						0,
						false);
				}
			}
		}
	}
	else if (message.getMethod() == "C++SpammerRspTargetNotFound")
	{
		Chat::sendSystemMessage(*this, StringId("warden", "invalid_target"), Unicode::emptyString);
	}
	else if (message.getMethod() == "C++SpammerRspInvalidTarget")
	{
		Chat::sendSystemMessage(*this, StringId("warden", "invalid_target"), Unicode::emptyString);
	}
	else if (message.getMethod() == "C++SpammerRspCannotSpammerTarget")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const targetName(message.getPackedDataVector().begin(), message.getPackedDataVector().end());

			ProsePackage prosePackage;
			prosePackage.stringId = StringId("warden", "cannot_spammer_target");
			prosePackage.target.str = Unicode::narrowToWide(targetName);
			Chat::sendSystemMessage(*this, prosePackage);
		}
	}
	else if (message.getMethod() == "C++SpammerRspSuccess")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const result(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			std::string::size_type const pos1 = result.find('|');
			if ((pos1 != std::string::npos) && (result.size() > (pos1 + 1)))
			{
				std::string::size_type const pos2 = result.find('|', (pos1 + 1));
				if ((pos2 != std::string::npos) && (result.size() > (pos2 + 1)))
				{
					NetworkId const target(result.substr(0, pos1));
					std::string const targetName(result.substr(pos1 + 1, pos2 - pos1 - 1));
					std::string const targetAccountDescription(result.substr(pos2 + 1));

					// request /report
					ReportManager::addReport(getAssignedObjectFirstName(), getNetworkId(), Unicode::narrowToWide(NameManager::normalizeName(targetName)), target, std::string("Warden"));

					// log it
					LOG("CustomerService", ("Warden:%s has spammer %s", PlayerObject::getAccountDescription(this).c_str(), targetAccountDescription.c_str()));

					// display confirmation message
					ProsePackage prosePackage;
					prosePackage.stringId = StringId("warden", "confirm_spammer");
					prosePackage.target.str = Unicode::narrowToWide(targetName);
					Chat::sendSystemMessage(*this, prosePackage);
				}
			}
		}
	}
	else if (message.getMethod() == "C++UnspammerReq")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			NetworkId const actor(params);
			PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
			if (playerObject)
			{
				bool canUnspammer = true;

				// cannot /unspammer a target that is not currently /spammer(ed)
				if (playerObject->getSecondsUntilUnsquelched() <= 0)
				{
					canUnspammer = false;
				}

				// can only /unspammer target that was /spammer(ed) by the actor
				if (canUnspammer && (playerObject->getSquelchedById() != actor))
				{
					canUnspammer = false;
				}

				if (!canUnspammer)
				{
					MessageToQueue::getInstance().sendMessageToC(actor,
						"C++UnspammerRspCannotUnspammerTarget",
						Unicode::wideToNarrow(getAssignedObjectName()),
						0,
						false);
				}
				else
				{
					// do it
					playerObject->unsquelch();

					std::string result = Unicode::wideToNarrow(getAssignedObjectName());
					result += "|";
					result += playerObject->getAccountDescription();

					MessageToQueue::getInstance().sendMessageToC(actor,
						"C++UnspammerRspSuccess",
						result,
						0,
						false);
				}
			}
			else
			{
				MessageToQueue::getInstance().sendMessageToC(actor,
					"C++UnspammerRspInvalidTarget",
					std::string(),
					0,
					false);
			}
		}
	}
	else if (message.getMethod() == "C++UnspammerRspTargetNotFound")
	{
		Chat::sendSystemMessage(*this, StringId("warden", "invalid_target"), Unicode::emptyString);
	}
	else if (message.getMethod() == "C++UnspammerRspInvalidTarget")
	{
		Chat::sendSystemMessage(*this, StringId("warden", "invalid_target"), Unicode::emptyString);
	}
	else if (message.getMethod() == "C++UnspammerRspCannotUnspammerTarget")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const targetName(message.getPackedDataVector().begin(), message.getPackedDataVector().end());

			ProsePackage prosePackage;
			prosePackage.stringId = StringId("warden", "cannot_unspammer_target");
			prosePackage.target.str = Unicode::narrowToWide(targetName);
			Chat::sendSystemMessage(*this, prosePackage);
		}
	}
	else if (message.getMethod() == "C++UnspammerRspSuccess")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const result(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			std::string::size_type const pos = result.find('|');
			if ((pos != std::string::npos) && (result.size() > (pos + 1)))
			{
				std::string const targetName(result.substr(0, pos));
				std::string const targetAccountDescription(result.substr(pos + 1));

				// log it
				LOG("CustomerService", ("Warden:%s has unspammer %s", PlayerObject::getAccountDescription(this).c_str(), targetAccountDescription.c_str()));

				// display confirmation message
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("warden", "confirm_unspammer");
				prosePackage.target.str = Unicode::narrowToWide(targetName);
				Chat::sendSystemMessage(*this, prosePackage);
			}
		}
	}
	else if (message.getMethod() == "C++DeputizeWardenReq")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			NetworkId existingWarden;
			StationId existingWardenStationId;
			bool success = false;

			Unicode::String const delimiters(Unicode::narrowToWide("|"));
			Unicode::UnicodeStringVector tokens;
			if ((Unicode::tokenize(Unicode::narrowToWide(params), tokens, &delimiters, nullptr)) && (tokens.size() == 2))
			{
				existingWarden = NetworkId(Unicode::wideToNarrow(tokens[0]));
				
				// quick way to convert a string to a uint32
				existingWardenStationId = static_cast<StationId>(NetworkId(Unicode::wideToNarrow(tokens[1])).getValue());

				if (existingWarden != getNetworkId())
				{
					PlayerObject * po = PlayerCreatureController::getPlayerObject(this);
					if (po && !po->isWarden() && (po->getStationId() == existingWardenStationId))
					{
						// make warden
						success = true;
						po->setPriviledgedTitle(static_cast<int8>(PlayerDataPriviledgedTitle::Warden));

						// send success message back to existing warden who deputized this warden
						std::string result = Unicode::wideToNarrow(getAssignedObjectName());
						result += "|";
						result += po->getAccountDescription();

						MessageToQueue::getInstance().sendMessageToC(existingWarden,
							"C++DeputizeWardenRspSuccess",
							result,
							0,
							false);
					}
				}
			}

			if (!success)
			{
				MessageToQueue::getInstance().sendMessageToC(existingWarden,
					"C++DeputizeWardenRspCannotDeputize",
					std::string(),
					0,
					false);
			}
		}
	}
	else if (message.getMethod() == "C++DeputizeWardenRspSuccess")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const result(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			std::string::size_type const pos = result.find('|');
			if ((pos != std::string::npos) && (result.size() > (pos + 1)))
			{
				std::string const newWardenName(result.substr(0, pos));
				std::string const newWardenAccountInfo(result.substr(pos + 1));

				// log it
				LOG("CustomerService", ("WardenPlayerDeputize:%s has deputized %s", PlayerObject::getAccountDescription(this).c_str(), newWardenAccountInfo.c_str()));

				// display confirmation message
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("warden", "confirm_deputize");
				prosePackage.target.str = Unicode::narrowToWide(newWardenName);
				Chat::sendSystemMessage(*this, prosePackage);
			}
		}
	}
	else if (message.getMethod() == "C++DeputizeWardenRspCannotDeputize")
	{
		Chat::sendSystemMessage(*this, StringId("warden", "cannot_deputize"), Unicode::emptyString);
	}
	else if (message.getMethod() == "C++UndeputizeWardenReq")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			NetworkId existingWarden;
			StationId existingWardenStationId;
			bool success = false;

			Unicode::String const delimiters(Unicode::narrowToWide("|"));
			Unicode::UnicodeStringVector tokens;
			if ((Unicode::tokenize(Unicode::narrowToWide(params), tokens, &delimiters, nullptr)) && (tokens.size() == 2))
			{
				existingWarden = NetworkId(Unicode::wideToNarrow(tokens[0]));

				// quick way to convert a string to a uint32
				existingWardenStationId = static_cast<StationId>(NetworkId(Unicode::wideToNarrow(tokens[1])).getValue());

				if (existingWarden != getNetworkId())
				{
					PlayerObject * po = PlayerCreatureController::getPlayerObject(this);
					if (po && po->isWarden() && (po->getStationId() == existingWardenStationId))
					{
						// revoke warden
						success = true;
						po->setPriviledgedTitle(static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer));

						// send success message back to existing warden who undeputized this warden
						std::string result = Unicode::wideToNarrow(getAssignedObjectName());
						result += "|";
						result += po->getAccountDescription();

						MessageToQueue::getInstance().sendMessageToC(existingWarden,
							"C++UndeputizeWardenRspSuccess",
							result,
							0,
							false);
					}
				}
			}

			if (!success)
			{
				MessageToQueue::getInstance().sendMessageToC(existingWarden,
					"C++UndeputizeWardenRspCannotUndeputize",
					std::string(),
					0,
					false);
			}
		}
	}
	else if (message.getMethod() == "C++UndeputizeWardenRspSuccess")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const result(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			std::string::size_type const pos = result.find('|');
			if ((pos != std::string::npos) && (result.size() > (pos + 1)))
			{
				std::string const priorWardenName(result.substr(0, pos));
				std::string const pirorWardenAccountInfo(result.substr(pos + 1));

				// log it
				LOG("CustomerService", ("WardenPlayerUndeputize:%s has undeputized %s", PlayerObject::getAccountDescription(this).c_str(), pirorWardenAccountInfo.c_str()));

				// display confirmation message
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("warden", "confirm_undeputize");
				prosePackage.target.str = Unicode::narrowToWide(priorWardenName);
				Chat::sendSystemMessage(*this, prosePackage);
			}
		}
	}
	else if (message.getMethod() == "C++UndeputizeWardenRspCannotUndeputize")
	{
		Chat::sendSystemMessage(*this, StringId("warden", "cannot_undeputize"), Unicode::emptyString);
	}
	else if (message.getMethod() == "C++OnCollectionServerFirst")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const collectionName(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			CollectionsDataTable::CollectionInfoCollection const * const collectionInfo = CollectionsDataTable::getCollectionByName(collectionName);
			if (collectionInfo && collectionInfo->trackServerFirst)
			{
				GameScriptObject * gameScriptObject = const_cast<GameScriptObject *>(getScriptObject());
				if (gameScriptObject)
				{
					ScriptParams params;
					params.addParam(collectionInfo->page.book.name.c_str()); // book
					params.addParam(collectionInfo->page.name.c_str());      // page
					params.addParam(collectionInfo->name.c_str());           // collection

					IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_COLLECTION_SERVER_FIRST, params));
				}
			}
		}
	}
	else if (message.getMethod() == "C++InviteToGroupReq")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());

			bool success = false;
			NetworkId actor, actorShip, group;
			std::string actorName;
			GroupObject const * groupObject = nullptr;
			StringId responseSid;

			Unicode::String const delimiters(Unicode::narrowToWide("|"));
			Unicode::UnicodeStringVector tokens;
			if ((Unicode::tokenize(Unicode::narrowToWide(params), tokens, &delimiters, nullptr)) && (tokens.size() == 4))
			{
				success = true;
				actor = NetworkId(Unicode::wideToNarrow(tokens[0]));
				actorShip = NetworkId(Unicode::wideToNarrow(tokens[1]));
				group = NetworkId(Unicode::wideToNarrow(tokens[2]));
				actorName = Unicode::wideToNarrow(tokens[3]);

				ServerObject const * so = (group.isValid() ? safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(group)) : nullptr);
				groupObject = (so ? so->asGroupObject() : nullptr);
			}

			if (success)
			{
				std::vector<CreatureObject *> targets;
				GroupHelpers::findAllTargetsForGroup(this, targets);

				if (!GroupHelpers::roomInGroup(groupObject, targets.size()))
				{
					responseSid = GroupStringId::SID_GROUP_FULL;
					success = false;
				}
			}

			if (success)
			{
				if ((this->getGroup()) && (!GroupHelpers::creatureIsContainedInPOBShip(this)))
				{
					responseSid = GroupStringId::SID_GROUP_ALREADY_GROUPED;
					success = false;
				}
			}

			if (success)
			{
				GameScriptObject* const gso = getScriptObject();
				if (gso != nullptr && gso->hasScript("ai.beast"))
				{
					responseSid = GroupStringId::SID_GROUP_BEASTS_CANT_JOIN;
					success = false;
				}
			}

			if (success)
			{
				NetworkId const & oldInviter = getGroupInviterId();
				if (oldInviter.isValid())
				{
					if (oldInviter == actor)
						responseSid = GroupStringId::SID_GROUP_CONSIDERING_YOUR_GROUP;
					else
						responseSid = GroupStringId::SID_GROUP_CONSIDERING_OTHER_GROUP;

					success = false;
				}
			}

			if (success)
			{
				PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(this);
				if (playerObject && playerObject->isIgnoring(Unicode::toLower(actorName)))
				{
					responseSid = GroupStringId::SID_GROUP_INVITE_NO_TARGET_SELF;
					success = false;
				}
			}

			if (success)
			{
				// cannot be invited if in combat
				if (getState(States::Combat) && ConfigServerGame::getGroupInviteInviteeCombatRestriction())
				{
					responseSid = GroupStringId::SID_GROUP_INVITE_TARGET_IN_COMBAT;
					success = false;
				}
			}

			if (success)
			{
				ProsePackage pp;
				pp.stringId = GroupStringId::SID_GROUP_INVITE_TARGET;
				pp.target.str = Unicode::narrowToWide(actorName);
				pp.actor.id = getNetworkId();

				Chat::sendSystemMessage(*this, pp);

				setGroupInviter(actor, actorName, actorShip);

				responseSid = GroupStringId::SID_GROUP_INVITE_LEADER;
			}

			std::string const objectName(Unicode::wideToNarrow(getAssignedObjectName()));

			std::string response = responseSid.getCanonicalRepresentation();
			response += "|";

			if (!objectName.empty())
			{
				response += "nameString|";
				response += objectName;
			}
			else
			{
				StringId nameSid = getObjectNameStringId();
				response += "nameStringId|";
				response += nameSid.getCanonicalRepresentation();
			}

			MessageToQueue::getInstance().sendMessageToC(actor,
				"C++GroupOperationGenericRsp",
				response,
				0,
				false);
		}
	}
	else if (message.getMethod() == "C++InviteToGroupRspTargetNotFound")
	{
		Chat::sendSystemMessageSimple(*this, GroupStringId::SID_GROUP_INVITE_NO_TARGET_SELF, nullptr);
	}
	else if (message.getMethod() == "C++GroupOperationGenericRsp")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const result(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			Unicode::String const delimiters(Unicode::narrowToWide("|"));
			Unicode::UnicodeStringVector tokens;
			if ((Unicode::tokenize(Unicode::narrowToWide(result), tokens, &delimiters, nullptr)) && (tokens.size() == 3))
			{
				std::string const response(Unicode::wideToNarrow(tokens[0]));
				std::string const responseParmType(Unicode::wideToNarrow(tokens[1]));
				std::string const responseParm(Unicode::wideToNarrow(tokens[2]));

				ProsePackage pp;
				pp.stringId = StringId(response);

				if (responseParmType == std::string("nameString"))
				{
					pp.target.str = Unicode::narrowToWide(responseParm);
				}
				else
				{
					pp.target.stringId = StringId(responseParm);
				}

				pp.actor.id = getNetworkId();

				Chat::sendSystemMessage(*this, pp);
			}
		}
	}
	else if (message.getMethod() == "C++UninviteFromGroupReq")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());

			bool success = false;
			NetworkId actor;
			std::string actorName;
			StringId responseSid;

			Unicode::String const delimiters(Unicode::narrowToWide("|"));
			Unicode::UnicodeStringVector tokens;
			if ((Unicode::tokenize(Unicode::narrowToWide(params), tokens, &delimiters, nullptr)) && (tokens.size() == 2))
			{
				success = true;
				actor = NetworkId(Unicode::wideToNarrow(tokens[0]));
				actorName = Unicode::wideToNarrow(tokens[1]);
			}

			if (success)
			{
				if (actor != getGroupInviterId())
				{
					responseSid = GroupStringId::SID_GROUP_UNINVITE_NOT_INVITED;
					success = false;
				}
			}

			if (success)
			{
				ProsePackage pp;
				pp.stringId = GroupStringId::SID_GROUP_UNINVITE_TARGET;
				pp.target.str = Unicode::narrowToWide(actorName);
				pp.actor.id = getNetworkId();

				Chat::sendSystemMessage(*this, pp);

				setGroupInviter(NetworkId::cms_invalid, std::string(), NetworkId::cms_invalid);

				responseSid = GroupStringId::SID_GROUP_UNINVITE_SELF;
			}

			std::string const objectName(Unicode::wideToNarrow(getAssignedObjectName()));

			std::string response = responseSid.getCanonicalRepresentation();
			response += "|";

			if (!objectName.empty())
			{
				response += "nameString|";
				response += objectName;
			}
			else
			{
				StringId nameSid = getObjectNameStringId();
				response += "nameStringId|";
				response += nameSid.getCanonicalRepresentation();
			}

			MessageToQueue::getInstance().sendMessageToC(actor,
				"C++GroupOperationGenericRsp",
				response,
				0,
				false);
		}
	}
	else if (message.getMethod() == "C++UninviteFromGroupRspTargetNotFound")
	{
		Chat::sendSystemMessageSimple(*this, GroupStringId::SID_GROUP_UNINVITE_NO_TARGET_SELF, nullptr);
	}
	else if (message.getMethod() == "C++GroupJoinInviterInfoReq")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			NetworkId const invitee(params);
			GroupObject const * const existingGroup = getGroup();
			GroupMemberParam gmp = GroupHelpers::buildGroupMemberParam(this);

			char buffer[1024];
			snprintf(buffer, sizeof(buffer)-1, "%s|%s|%s|%d|%d|%d|%s|%d|%d|%d",
				(existingGroup ? existingGroup->getNetworkId().getValueString().c_str() : "0"),
				gmp.m_memberId.getValueString().c_str(),
				gmp.m_memberName.c_str(),
				gmp.m_memberDifficulty,
				static_cast<int>(gmp.m_memberProfession),
				(gmp.m_memberIsPC ? 1 : 0),
				gmp.m_memberShipId.getValueString().c_str(),
				(gmp.m_memberShipIsPOB ? 1 : 0),
				(gmp.m_memberOwnsPOB ? 1 : 0),
				(getState(States::Combat) ? 1 : 0));

			buffer[sizeof(buffer)-1] = '\0';

			MessageToQueue::getInstance().sendMessageToC(invitee,
				"C++GroupJoinInviterInfoRsp",
				buffer,
				0,
				false);

			// if the inviter had been invited in the meantime, have them decline
			if (getGroupInviterId().isValid())
				setGroupInviter(NetworkId::cms_invalid, std::string(), NetworkId::cms_invalid);
		}
	}
	else if (message.getMethod() == "C++GroupJoinInviterInfoRsp")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			bool success = false;
			NetworkId existingGroupId;
			NetworkId inviterId;
			std::string inviterName;
			int inviterDifficulty = 1;
			LfgCharacterData::Profession inviterProfession = LfgCharacterData::Prof_Unknown;
			bool inviterIsPC = true;
			NetworkId inviterShipId;
			bool inviterShipIsPOB = false;
			bool inviterOwnsPOB = false;
			bool inviterInCombat = false;

			Unicode::String const delimiters(Unicode::narrowToWide("|"));
			Unicode::UnicodeStringVector tokens;
			if ((Unicode::tokenize(Unicode::narrowToWide(params), tokens, &delimiters, nullptr)) && (tokens.size() == 10))
			{
				success = true;
				existingGroupId = NetworkId(Unicode::wideToNarrow(tokens[0]));
				inviterId = NetworkId(Unicode::wideToNarrow(tokens[1]));
				inviterName = Unicode::wideToNarrow(tokens[2]);
				inviterDifficulty = atoi(Unicode::wideToNarrow(tokens[3]).c_str());
				inviterProfession = static_cast<LfgCharacterData::Profession>(atoi(Unicode::wideToNarrow(tokens[4]).c_str()));
				inviterIsPC = (atoi(Unicode::wideToNarrow(tokens[5]).c_str()) != 0);
				inviterShipId = NetworkId(Unicode::wideToNarrow(tokens[6]));
				inviterShipIsPOB = (atoi(Unicode::wideToNarrow(tokens[7]).c_str()) != 0);
				inviterOwnsPOB = (atoi(Unicode::wideToNarrow(tokens[8]).c_str()) != 0);
				inviterInCombat = (atoi(Unicode::wideToNarrow(tokens[9]).c_str()) != 0);
			}

			GroupObject * existingGroup = nullptr;
			if (success)
			{
				if (existingGroupId.isValid())
				{
					ServerObject * so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(existingGroupId));
					existingGroup = (so ? so->asGroupObject() : nullptr);
					if (!existingGroup)
					{
						success = false;
						Chat::sendSystemMessageSimple(*this, GroupStringId::SID_GROUP_EXISTING_GROUP_NOT_FOUND, nullptr);
					}
				}
			}

			if (success)
			{
				// make sure inviter hasn't joined another group since the invite
				if (existingGroup && (inviterId != existingGroup->getGroupLeaderId()))
				{
					success = false;
					ProsePackage pp;
					pp.stringId = GroupStringId::SID_GROUP_JOIN_INVITER_NOT_LEADER;
					pp.target.str = Unicode::narrowToWide(inviterName);
					pp.actor.id = getNetworkId();

					Chat::sendSystemMessage(*this, pp);
				}
			}

			if (success)
			{
				// cannot join if the inviter is in combat
				if (inviterInCombat && ConfigServerGame::getGroupJoinInviterCombatRestriction())
				{
					success = false;
					ProsePackage pp;
					pp.stringId = GroupStringId::SID_GROUP_JOIN_LEADER_IN_COMBAT;
					pp.target.str = Unicode::narrowToWide(inviterName);
					pp.actor.id = getNetworkId();

					Chat::sendSystemMessage(*this, pp);
				}
			}

			std::vector<CreatureObject *> targets;
			if (success)
			{
				GroupHelpers::findAllTargetsForGroup(this, targets);

				if (existingGroup && !GroupHelpers::roomInGroup(existingGroup, targets.size()))
				{
					success = false;
					Chat::sendSystemMessageSimple(*this, GroupStringId::SID_GROUP_JOIN_FULL, nullptr);
				}
			}

			if (success)
			{
				// if we are in a POB group then we need to disband it
				GroupObject * const groupFromPOB = getGroup();
				if (groupFromPOB != 0)
				{
					groupFromPOB->disbandGroup();

					std::vector<CreatureObject *>::const_iterator ii = targets.begin();
					std::vector<CreatureObject *>::const_iterator iiEnd = targets.end();

					for (; ii != iiEnd; ++ii)
					{
						if ((*ii)->getGroup() == groupFromPOB)
							(*ii)->getGroup()->onGroupMemberRemoved((*ii)->getNetworkId(), true);
					}

					if (getGroup() == groupFromPOB)
						getGroup()->onGroupMemberRemoved(getNetworkId(), true);
				}

				if (existingGroup)
				{
					GroupObject::GroupMemberParamVector targetMemberParams;
					GroupHelpers::buildGroupMemberParamsFromCreatures(targets, targetMemberParams);

					int const numberOfTargets = static_cast<int>(targetMemberParams.size());

					for (int i = 0; i < numberOfTargets; ++i)
					{
						existingGroup->addGroupMember(targetMemberParams[i]);
					}
				}
				else
				{
					// if the inviter happens to be in the same ship as you then we need to remove
					// him from the targets list so that he isn't added to the new group twice
					for (std::vector<CreatureObject *>::iterator ii = targets.begin(); ii != targets.end(); ++ii)
					{
						if ((*ii)->getNetworkId() == inviterId)
						{
							targets.erase(ii);
							break;
						}
					}

					GroupObject::GroupMemberParamVector targetMemberParams;
					GroupHelpers::buildGroupMemberParamsFromCreatures(targets, targetMemberParams);

					// group did not already exist, so form it with the inviter as the leader
					if (!targetMemberParams.empty())
						ServerUniverse::getInstance().createGroup(GroupMemberParam(inviterId, inviterName, inviterDifficulty, inviterProfession, inviterIsPC, inviterShipId, inviterShipIsPOB, inviterOwnsPOB), targetMemberParams);
				}

				// Until the targets are actually told they are in a group
				// (the authoritative group object may be on another server)
				// we will keep track of who extended the invitation
				std::vector<CreatureObject *>::const_iterator ii = targets.begin();
				std::vector<CreatureObject *>::const_iterator iiEnd = targets.end();
				for (; ii != iiEnd; ++ii)
				{
					(*ii)->setInviterForPendingGroup(inviterId);
				}
			}
		}
	}
	else if (message.getMethod() == "C++GroupJoinInviterInfoReqInviterNotFound")
	{
		Chat::sendSystemMessageSimple(*this, GroupStringId::SID_GROUP_MUST_BE_INVITED, nullptr);
	}
	else if (message.getMethod() == "C++LeaveGroupReq")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			GroupObject const * const groupObj = getGroup();
			if (groupObj && (groupObj->getNetworkId() == NetworkId(params)) && (groupObj->getGroupLeaderId() != getNetworkId()))
			{
				static uint32 const commandHash = Crc::normalizeAndCalculate("leaveGroup");
				const Command &command = CommandTable::getCommand(static_cast<uint32>(commandHash));
				if (!command.isNull())
				{
					commandQueueEnqueue(command, NetworkId::cms_invalid, Unicode::String(), 0, false, static_cast<Command::Priority>(Command::CP_Front), true);
				}
			}
		}
	}
	else if (message.getMethod() == "C++GroupPickupPointCreated")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());

			Unicode::String const delimiters(Unicode::narrowToWide("|"));
			Unicode::UnicodeStringVector tokens;
			if ((Unicode::tokenize(Unicode::narrowToWide(params), tokens, &delimiters, nullptr)) && (tokens.size() == 5))
			{
				// tell group member that the group pickup point has been created
				if (getClient())
				{
					StringId::LocUnicodeString response;
					if (StringId("group", "create_group_pickup_success_others").localize(response))
					{
						ConsoleMgr::broadcastString(FormattedString<2048>().sprintf(Unicode::wideToNarrow(response).c_str(), Unicode::wideToNarrow(tokens[4]).c_str(), CalendarTime::convertSecondsToMS(static_cast<unsigned int>(ConfigServerGame::getGroupPickupPointTimeLimitSeconds())).c_str()),
							getClient());
					}
				}

				// create/update the group member's group pickup point waypoint
				PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
				if (playerObject)
				{
					std::string const planetName = Unicode::wideToNarrow(tokens[0]);
					int const x = atoi(Unicode::wideToNarrow(tokens[1]).c_str());
					int const y = atoi(Unicode::wideToNarrow(tokens[2]).c_str());
					int const z = atoi(Unicode::wideToNarrow(tokens[3]).c_str());

					Location const location(Vector(static_cast<real>(x), static_cast<real>(y), static_cast<real>(z)), NetworkId::cms_invalid, Location::getCrcBySceneName(planetName));
					playerObject->createOrUpdateReusableWaypoint(location, "groupPickupWp", Unicode::narrowToWide("Group Pickup Point"), Waypoint::White);
				}
			}
		}
	}
	else if (message.getMethod() == "C++OccupyUnlockedSlotRsp")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			LoginUpgradeAccountMessage::OccupyUnlockedSlotResponse const response = static_cast<LoginUpgradeAccountMessage::OccupyUnlockedSlotResponse>(::atoi(params.c_str()));

			if (response == LoginUpgradeAccountMessage::OUSR_success)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s OccupyUnlockedSlot request SUCCESS", PlayerObject::getAccountDescription(this).c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "occupy_unlocked_slot_success"), Unicode::emptyString);

				// let the client know so it can change the character to be properly displayed on the character select screen
				Client * const client = getClient();
				if (client)
				{
					GenericValueTypeMessage<std::pair<std::string, NetworkId> > const characterChangedUnlocked("CharacterChangedUnlocked", std::make_pair(GameServer::getInstance().getClusterName(), getNetworkId()));
					client->send(characterChangedUnlocked, true);
				}
			}
			else if (response == LoginUpgradeAccountMessage::OUSR_db_error)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s OccupyUnlockedSlot request FAILED - internal db error", PlayerObject::getAccountDescription(this).c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "occupy_unlocked_slot_db_error"), Unicode::emptyString);
			}
			else if (response == LoginUpgradeAccountMessage::OUSR_account_has_no_unlocked_slot)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s OccupyUnlockedSlot request FAILED - account doesn't have an unlocked slot", PlayerObject::getAccountDescription(this).c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "occupy_unlocked_slot_db_no_unlocked_slot"), Unicode::emptyString);
			}
			else if (response == LoginUpgradeAccountMessage::OUSR_account_has_no_unoccupied_unlocked_slot)
			{
				LOG("CustomerService",("JediUnlockedSlot:%s OccupyUnlockedSlot request FAILED - account has no unoccupied unlocked slot", PlayerObject::getAccountDescription(this).c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "occupy_unlocked_slot_db_no_unoccupied_unlocked_slot"), Unicode::emptyString);
			}
			else if (response == LoginUpgradeAccountMessage::OUSR_cluster_already_has_unlocked_slot_character)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s OccupyUnlockedSlot request FAILED - cluster already has an unlocked slot character", PlayerObject::getAccountDescription(this).c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "occupy_unlocked_slot_db_has_unlocked_slot"), Unicode::emptyString);
			}
			else
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s OccupyUnlockedSlot request FAILED - unknown result code (%d)", PlayerObject::getAccountDescription(this).c_str(), static_cast<int>(response)));
			}
		}
	}
	else if (message.getMethod() == "C++VacateUnlockedSlotRsp")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			LoginUpgradeAccountMessage::VacateUnlockedSlotResponse const response = static_cast<LoginUpgradeAccountMessage::VacateUnlockedSlotResponse>(::atoi(params.c_str()));

			if (response == LoginUpgradeAccountMessage::VUSR_success)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s VacateUnlockedSlot request SUCCESS", PlayerObject::getAccountDescription(this).c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "vacate_unlocked_slot_success"), Unicode::emptyString);

				// let the client know so it can change the character to be properly displayed on the character select screen
				Client * const client = getClient();
				if (client)
				{
					GenericValueTypeMessage<std::pair<std::string, NetworkId> > const characterChangedNormal("CharacterChangedNormal", std::make_pair(GameServer::getInstance().getClusterName(), getNetworkId()));
					client->send(characterChangedNormal, true);
				}
			}
			else if (response == LoginUpgradeAccountMessage::VUSR_db_error)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s VacateUnlockedSlot request FAILED - internal db error", PlayerObject::getAccountDescription(this).c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "vacate_unlocked_slot_db_error"), Unicode::emptyString);
			}
			else if (response == LoginUpgradeAccountMessage::VUSR_account_has_no_unlocked_slot)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s VacateUnlockedSlot request FAILED - account doesn't have an unlocked slot", PlayerObject::getAccountDescription(this).c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "vacate_unlocked_slot_db_no_unlocked_slot"), Unicode::emptyString);
			}
			else if (response == LoginUpgradeAccountMessage::VUSR_not_unlocked_slot_character)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s VacateUnlockedSlot request FAILED - character is not an unlocked slot character", PlayerObject::getAccountDescription(this).c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "vacate_unlocked_slot_db_not_unlocked_slot_character"), Unicode::emptyString);
			}
			else if (response == LoginUpgradeAccountMessage::VUSR_no_available_normal_character_slot)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s VacateUnlockedSlot request FAILED - no available normal character slot for the account on this galaxy", PlayerObject::getAccountDescription(this).c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "vacate_unlocked_slot_db_no_available_normal_character_slot"), Unicode::emptyString);
			}
			else
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s VacateUnlockedSlot request FAILED - unknown result code (%d)", PlayerObject::getAccountDescription(this).c_str(), static_cast<int>(response)));
			}
		}
	}
	else if (message.getMethod() == "C++SwapUnlockedSlotRsp")
	{
		if (!message.getPackedDataVector().empty())
		{
			Archive::ByteStream bs;
			bs.put(reinterpret_cast<const void *>(&message.getPackedDataVector()[0]), static_cast<unsigned int>(message.getPackedDataVector().size()));

			Archive::ReadIterator ri = bs.begin();
			GenericValueTypeMessage<std::pair<std::pair<int, NetworkId>, std::pair<uint32, std::pair<NetworkId, std::string> > > > const swapUnlockedSlotRsp(ri);

			LoginUpgradeAccountMessage::SwapUnlockedSlotResponse const response = static_cast<LoginUpgradeAccountMessage::SwapUnlockedSlotResponse>(swapUnlockedSlotRsp.getValue().first.first);
			if (response == LoginUpgradeAccountMessage::SUSR_success)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s SwapUnlockedSlot with %s (%s) request SUCCESS", PlayerObject::getAccountDescription(this).c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str(), swapUnlockedSlotRsp.getValue().second.second.second.c_str()));

				ProsePackage prosePackage;
				prosePackage.stringId = StringId("unlocked_slot", "swap_unlocked_slot_success");
				prosePackage.target.str = Unicode::narrowToWide(swapUnlockedSlotRsp.getValue().second.second.second);
				Chat::sendSystemMessage(*this, prosePackage);

				// let the client know so it can change the character to be properly displayed on the character select screen
				Client * const client = getClient();
				if (client)
				{
					GenericValueTypeMessage<std::pair<std::string, NetworkId> > const characterChangedNormal("CharacterChangedNormal", std::make_pair(GameServer::getInstance().getClusterName(), getNetworkId()));
					client->send(characterChangedNormal, true);

					GenericValueTypeMessage<std::pair<std::string, NetworkId> > const characterChangedUnlocked("CharacterChangedUnlocked", std::make_pair(GameServer::getInstance().getClusterName(), swapUnlockedSlotRsp.getValue().second.second.first));
					client->send(characterChangedUnlocked, true);
				}
			}
			else if (response == LoginUpgradeAccountMessage::SUSR_db_error)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s SwapUnlockedSlot with %s request FAILED - internal db error", PlayerObject::getAccountDescription(this).c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "swap_unlocked_slot_db_error"), Unicode::emptyString);
			}
			else if (response == LoginUpgradeAccountMessage::SUSR_account_has_no_unlocked_slot)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s SwapUnlockedSlot with %s request FAILED - account doesn't have an unlocked slot", PlayerObject::getAccountDescription(this).c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "swap_unlocked_slot_db_no_unlocked_slot"), Unicode::emptyString);
			}
			else if (response == LoginUpgradeAccountMessage::SUSR_not_unlocked_slot_character)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s SwapUnlockedSlot with %s request FAILED - source character is not an unlocked slot character", PlayerObject::getAccountDescription(this).c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "swap_unlocked_slot_db_not_unlocked_slot_character"), Unicode::emptyString);
			}
			else if (response == LoginUpgradeAccountMessage::SUSR_invalid_target_character)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s SwapUnlockedSlot with %s request FAILED - target character is either not valid, not on the same account, or not on this galaxy", PlayerObject::getAccountDescription(this).c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "swap_unlocked_slot_db_invalid_target_character"), Unicode::emptyString);
			}
			else if (response == LoginUpgradeAccountMessage::SUSR_target_character_already_unlocked_slot_character)
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s SwapUnlockedSlot with %s request FAILED - target character is already an unlocked slot character", PlayerObject::getAccountDescription(this).c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str()));

				Chat::sendSystemMessage(*this, StringId("unlocked_slot", "swap_unlocked_slot_db_target_is_already_unlocked_character"), Unicode::emptyString);
			}
			else
			{
				// CS log the response
				LOG("CustomerService",("JediUnlockedSlot:%s SwapUnlockedSlot with %s request FAILED - unknown result code (%d)", PlayerObject::getAccountDescription(this).c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str(), swapUnlockedSlotRsp.getValue().first.first));
			}
		}
	}
	else if (message.getMethod() == "C++AccountFeatureIdResponse")
	{
		if (!message.getPackedDataVector().empty())
		{
			Archive::ByteStream bs;
			bs.put(reinterpret_cast<const void *>(&message.getPackedDataVector()[0]), static_cast<unsigned int>(message.getPackedDataVector().size()));

			Archive::ReadIterator ri = bs.begin();
			AccountFeatureIdResponse const msg(ri);

			if ((msg.getTarget() == getNetworkId()) && (msg.getResultCode() == RESULT_SUCCESS))
			{
				Client * client = getClient();
				if (client)
				{
					if (msg.getGameCode() == PlatformGameCode::SWG)
						client->setAccountFeatureIds(msg.getFeatureIds());
				}
			}

			if ((msg.getRequestReason() == AccountFeatureIdRequest::RR_ConsoleCommandReloadRequest) && msg.getRequester().isValid() && (msg.getRequester() == getNetworkId()))
			{
				if (msg.getResultCode() == RESULT_SUCCESS)
				{
					ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("request for feature Id for account (%lu) was successful", msg.getTargetStationId()),
						getClient());

					if (msg.getGameCode() == PlatformGameCode::SWG)
					{
						ConsoleMgr::broadcastString("you can use the /getAccountInfo command on a targeted character to view the character's account SWG feature Id",
							getClient());
					}

					if (msg.getResultCameFromSession())
					{
						std::map<uint32, std::string> const & sessionFeatureIdsData = msg.getSessionFeatureIdsData();

						ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("session/Platform reported successful result code (%u, %s:%s), (%d) feature Ids for account (%lu)", msg.getResultCode(), msg.getSessionResultString().c_str(), msg.getSessionResultText().c_str(), sessionFeatureIdsData.size(), msg.getTargetStationId()),
							getClient());

						for (std::map<uint32, std::string>::const_iterator iter = sessionFeatureIdsData.begin(); iter != sessionFeatureIdsData.end(); ++iter)
						{
							ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("%lu (%s)", iter->first, iter->second.c_str()),
								getClient());
						}
					}
					else if (msg.getGameCode() != PlatformGameCode::SWG)
					{
						std::map<uint32, int> const & featureIds = msg.getFeatureIds();

						ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("(%d) feature Ids for account (%lu)", featureIds.size(), msg.getTargetStationId()),
							getClient());

						for (std::map<uint32, int>::const_iterator iter = featureIds.begin(); iter != featureIds.end(); ++iter)
						{
							ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("%lu (%d)", iter->first, iter->second),
								getClient());
						}
					}
				}
				else
				{
					if (msg.getResultCameFromSession())
						ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("request for feature Id for account (%lu) failed with error code (%u, %s:%s)", msg.getTargetStationId(), msg.getResultCode(), msg.getSessionResultString().c_str(), msg.getSessionResultText().c_str()), getClient());
					else
						ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("request for feature Id for account (%lu) failed with error code (%u)", msg.getTargetStationId(), msg.getResultCode()), getClient());
				}
			}

			if ((msg.getRequestReason() == AccountFeatureIdRequest::RR_ReloadRewardCheck) && msg.getRequester().isValid() && (msg.getRequester() == getNetworkId()) && (msg.getGameCode() == PlatformGameCode::SWG))
			{
				// let script know the that account feature id has been
				// retrieved so it can resume the rewards claim process
				MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), "veteranAccountFeatureIdResponse", std::vector<int8>(), 0, false);
			}
		}
	}
	else if (message.getMethod() == "C++FeatureIdTransactionResponse")
	{
		if (!message.getPackedDataVector().empty())
		{
			Archive::ByteStream bs;
			bs.put(reinterpret_cast<const void *>(&message.getPackedDataVector()[0]), static_cast<unsigned int>(message.getPackedDataVector().size()));

			Archive::ReadIterator ri = bs.begin();
			FeatureIdTransactionResponse const msg(ri);
			VeteranRewardManager::verifyFeatureIdTransactions(*this, msg.getTransactions());
		}
	}
	else if (message.getMethod() == "C++TransferReplyMoveValidation")
	{
		if (!message.getPackedDataVector().empty())
		{
			Archive::ByteStream bs;
			bs.put(reinterpret_cast<const void *>(&message.getPackedDataVector()[0]), static_cast<unsigned int>(message.getPackedDataVector().size()));

			Archive::ReadIterator ri = bs.begin();
			TransferReplyMoveValidation const msg(ri);

			// we should only be here if the validation failed
			TransferRequestMoveValidation::TransferRequestSource const transferRequestSource = msg.getTransferRequestSource();
			if (transferRequestSource == TransferRequestMoveValidation::TRS_console_god_command)
			{
				TransferReplyMoveValidation::TransferReplyMoveValidationResult const result = msg.getResult();

				if (result == TransferReplyMoveValidation::TRMVR_cannot_create_regular_character)
				{
					ConsoleMgr::broadcastString(FormattedString<256>().sprintf("the destination station id (%u) is not allowed to create any more character on the destination galaxy (%s)", msg.getDestinationStationId(), msg.getDestinationGalaxy().c_str()), getClient());
				}
				else if (result == TransferReplyMoveValidation::TRMVR_destination_galaxy_invalid)
				{
					ConsoleMgr::broadcastString(FormattedString<256>().sprintf("either the destination galaxy (%s) is invalid, or the destination galaxy (%s) is not using the same login server as this galaxy", msg.getDestinationGalaxy().c_str(), msg.getDestinationGalaxy().c_str()), getClient());
				}
				else if (result == TransferReplyMoveValidation::TRMVR_destination_galaxy_not_connected)
				{
					ConsoleMgr::broadcastString(FormattedString<256>().sprintf("the destination galaxy (%s) is not currently started", msg.getDestinationGalaxy().c_str()), getClient());
				}
				else if (result == TransferReplyMoveValidation::TRMVR_destination_galaxy_in_loading)
				{
					ConsoleMgr::broadcastString(FormattedString<256>().sprintf("the destination galaxy (%s) is currently loading", msg.getDestinationGalaxy().c_str()), getClient());
				}
				else
				{
					ConsoleMgr::broadcastString(FormattedString<256>().sprintf("transfer validation failed with error code (%d)", static_cast<int>(result)), getClient());
				}
			}
			else if ((transferRequestSource == TransferRequestMoveValidation::TRS_ingame_freects_command_validate) || (transferRequestSource == TransferRequestMoveValidation::TRS_ingame_freects_command_transfer))
			{
				TransferReplyMoveValidation::TransferReplyMoveValidationResult const result = msg.getResult();
				if (result == TransferReplyMoveValidation::TRMVR_cannot_create_regular_character)
				{
					ScriptParams params;
					params.addParam(msg.getDestinationGalaxy().c_str(), "destinationGalaxy");
					ScriptDictionaryPtr dictionary;
					GameScriptObject::makeScriptDictionary(params, dictionary);
					if (dictionary.get() != nullptr)
					{
						dictionary->serialize();
						MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), "handleFreeCtsValidateFailCannotCreateCharacter", dictionary->getSerializedData(), 0, false);
					}
				}
				else if ((result == TransferReplyMoveValidation::TRMVR_destination_galaxy_invalid) ||
				         (result == TransferReplyMoveValidation::TRMVR_destination_galaxy_not_connected) ||
				         (result == TransferReplyMoveValidation::TRMVR_destination_galaxy_in_loading))
				{
					ScriptParams params;
					params.addParam(msg.getDestinationGalaxy().c_str(), "destinationGalaxy");
					ScriptDictionaryPtr dictionary;
					GameScriptObject::makeScriptDictionary(params, dictionary);
					if (dictionary.get() != nullptr)
					{
						dictionary->serialize();
						MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), "handleFreeCtsValidateFailDestGalaxyUnavailable", dictionary->getSerializedData(), 0, false);
					}
				}
				else
				{
					LOG("CustomerService", ("CharacterTransfer: transfer validation failed for [%u, %s, %s (%s)] to [%u, %s, %s] with unexpected error code (%d)", msg.getSourceStationId(), msg.getSourceGalaxy().c_str(), msg.getSourceCharacter().c_str(), msg.getSourceCharacterId().getValueString().c_str(), msg.getDestinationStationId(), msg.getDestinationGalaxy().c_str(), msg.getDestinationCharacter().c_str(), static_cast<int>(result)));
				}
			}
			else if ((transferRequestSource == TransferRequestMoveValidation::TRS_ingame_cts_command_validate) || (transferRequestSource == TransferRequestMoveValidation::TRS_ingame_cts_command_transfer))
			{
				TransferReplyMoveValidation::TransferReplyMoveValidationResult const result = msg.getResult();
				if (result == TransferReplyMoveValidation::TRMVR_cannot_create_regular_character)
				{
					ScriptParams params;
					params.addParam(msg.getDestinationGalaxy().c_str(), "destinationGalaxy");
					ScriptDictionaryPtr dictionary;
					GameScriptObject::makeScriptDictionary(params, dictionary);
					if (dictionary.get() != nullptr)
					{
						dictionary->serialize();
						MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), "handleCtsValidateFailCannotCreateCharacter", dictionary->getSerializedData(), 0, false);
					}
				}
				else if ((result == TransferReplyMoveValidation::TRMVR_destination_galaxy_invalid) ||
					(result == TransferReplyMoveValidation::TRMVR_destination_galaxy_not_connected) ||
					(result == TransferReplyMoveValidation::TRMVR_destination_galaxy_in_loading))
				{
					ScriptParams params;
					params.addParam(msg.getDestinationGalaxy().c_str(), "destinationGalaxy");
					ScriptDictionaryPtr dictionary;
					GameScriptObject::makeScriptDictionary(params, dictionary);
					if (dictionary.get() != nullptr)
					{
						dictionary->serialize();
						MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), "handleCtsValidateFailDestGalaxyUnavailable", dictionary->getSerializedData(), 0, false);
					}
				}
				else
				{
					LOG("CustomerService", ("CharacterTransfer: transfer validation failed for [%u, %s, %s (%s)] to [%u, %s, %s] with unexpected error code (%d)", msg.getSourceStationId(), msg.getSourceGalaxy().c_str(), msg.getSourceCharacter().c_str(), msg.getSourceCharacterId().getValueString().c_str(), msg.getDestinationStationId(), msg.getDestinationGalaxy().c_str(), msg.getDestinationCharacter().c_str(), static_cast<int>(result)));
				}
			}
		}
	}
	else if (message.getMethod() == "C++TransferReplyNameValidation")
	{
		if (!message.getPackedDataVector().empty())
		{
			Archive::ByteStream bs;
			bs.put(reinterpret_cast<const void *>(&message.getPackedDataVector()[0]), static_cast<unsigned int>(message.getPackedDataVector().size()));

			Archive::ReadIterator ri = bs.begin();
			GenericValueTypeMessage<std::pair<std::string, TransferCharacterData> > const msg(ri);

			TransferRequestMoveValidation::TransferRequestSource const transferRequestSource = msg.getValue().second.getTransferRequestSource();

			if (!msg.getValue().second.getIsValidName())
			{
				if (transferRequestSource == TransferRequestMoveValidation::TRS_console_god_command)
				{
					ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("the character name (%s) is not available on galaxy (%s) because of reason (%s)", msg.getValue().second.getDestinationCharacterName().c_str(), msg.getValue().second.getDestinationGalaxy().c_str(), msg.getValue().first.c_str()), getClient());
				}
				else if ((transferRequestSource == TransferRequestMoveValidation::TRS_ingame_freects_command_validate) || (transferRequestSource == TransferRequestMoveValidation::TRS_ingame_freects_command_transfer))
				{
					ScriptParams params;
					params.addParam(msg.getValue().second.getDestinationGalaxy().c_str(), "destinationGalaxy");
					params.addParam(msg.getValue().second.getDestinationCharacterName().c_str(), "destinationCharacterName");
					params.addParam(msg.getValue().first.c_str(), "reason");
					ScriptDictionaryPtr dictionary;
					GameScriptObject::makeScriptDictionary(params, dictionary);
					if (dictionary.get() != nullptr)
					{
						dictionary->serialize();
						MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), "handleFreeCtsValidateFailNameValidation", dictionary->getSerializedData(), 0, false);
					}
				}
				else if ((transferRequestSource == TransferRequestMoveValidation::TRS_ingame_cts_command_validate) || (transferRequestSource == TransferRequestMoveValidation::TRS_ingame_cts_command_transfer))
				{
					ScriptParams params;
					params.addParam(msg.getValue().second.getDestinationGalaxy().c_str(), "destinationGalaxy");
					params.addParam(msg.getValue().second.getDestinationCharacterName().c_str(), "destinationCharacterName");
					params.addParam(msg.getValue().first.c_str(), "reason");
					ScriptDictionaryPtr dictionary;
					GameScriptObject::makeScriptDictionary(params, dictionary);
					if (dictionary.get() != nullptr)
					{
						dictionary->serialize();
						MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), "handleCtsValidateFailNameValidation", dictionary->getSerializedData(), 0, false);
					}
				}
			}
			else if (transferRequestSource == TransferRequestMoveValidation::TRS_ingame_freects_command_validate)
			{
				ScriptParams params;
				params.addParam(msg.getValue().second.getDestinationGalaxy().c_str(), "destinationGalaxy");
				params.addParam(msg.getValue().second.getDestinationCharacterName().c_str(), "destinationCharacterName");
				ScriptDictionaryPtr dictionary;
				GameScriptObject::makeScriptDictionary(params, dictionary);
				if (dictionary.get() != nullptr)
				{
					dictionary->serialize();
					MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), "handleFreeCtsValidateSuccess", dictionary->getSerializedData(), 0, false);
				}
			}
			else if (transferRequestSource == TransferRequestMoveValidation::TRS_ingame_cts_command_validate)
			{
				ScriptParams params;
				params.addParam(msg.getValue().second.getDestinationGalaxy().c_str(), "destinationGalaxy");
				params.addParam(msg.getValue().second.getDestinationCharacterName().c_str(), "destinationCharacterName");
				ScriptDictionaryPtr dictionary;
				GameScriptObject::makeScriptDictionary(params, dictionary);
				if (dictionary.get() != nullptr)
				{
					dictionary->serialize();
					MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), "handleCtsValidateSuccess", dictionary->getSerializedData(), 0, false);
				}
			}
		}
	}
	else if (message.getMethod() == "C++PickupAllRoomItemsIntoInventory")
	{
		// use while(true) loop to use break to discontinue processing and go to the end of loop to run cleanup code
		PlayerObject * playerObject = nullptr;
		while (true)
		{
			playerObject = PlayerCreatureController::getPlayerObject(this);
			if (!playerObject)
				break;

			// player has disconnected
			if (!getClient())
				break;

			// don't allow this in god mode because it could be used accidentally and pick up a bunch of stuff,
			// and god mode allows stuff to be picked up, which wouldn't normally be allowed to be picked up
			if (getClient()->isGod())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because you switched into GOD mode."), Unicode::emptyString);
				break;
			}

			// player is not "active", meaning he's probably doing it with a macro
			if (playerObject->getSessionLastActiveTime() == 0)
				break;

			int operationId = 0;
			if (!getObjVars().getItem("pickupDropAllItemsOperation.operationId", operationId) || (operationId == 0))
				break;

			if (operationId != playerObject->getPickupDropAllItemsOperationId())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because of an unexpected condition.  Please try the operation again."), Unicode::emptyString);
				break;
			}

			NetworkId const & cellId = playerObject->getPickupDropAllItemsCellId();

			ServerObject const * const containingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));
			if (!containingCell || !containingCell->asCellObject() || (containingCell->getNetworkId() != cellId))
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because you have left the room."), Unicode::emptyString);
				break;
			}

			ServerObject const * const containingPOBso = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingCell));
			if (!containingPOBso)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because you have left the room."), Unicode::emptyString);
				break;
			}

			if (!containingPOBso->isInWorldCell())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because you have left the room."), Unicode::emptyString);
				break;
			}

			TangibleObject const * const containingPOB = containingPOBso->asTangibleObject();
			if (!containingPOB)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because of an internal error."), Unicode::emptyString);
				break;
			}

			if (!containingPOB->isOnAdminList(*this))
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because you are no longer the the owner or an admin of the structure or POB ship."), Unicode::emptyString);
				break;
			}

			ServerObject * const targetInventoryObj = getInventory();
			if (!targetInventoryObj)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because of an internal error."), Unicode::emptyString);
				break;
			}

			VolumeContainer * const targetInventoryContainer = ContainerInterface::getVolumeContainer(*targetInventoryObj);
			if (!targetInventoryContainer)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because of an internal error."), Unicode::emptyString);
				break;
			}

			int const inventoryBaseLimit = targetInventoryContainer->getTotalVolume();
			int const inventoryLimit = (playerObject->getPickupAllItemsAllowInventoryOverload() ? std::max(inventoryBaseLimit, (inventoryBaseLimit + ConfigServerGame::getMoveValidationMaxInventoryOverload() - 11)) : inventoryBaseLimit);

			if (targetInventoryContainer->getCurrentVolume() >= inventoryLimit)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because your inventory is full."), Unicode::emptyString);

				// if they didn't run the command with the -overload
				// option, remind them about the -overload option
				if (!playerObject->getPickupAllItemsAllowInventoryOverload())
					Chat::sendSystemMessage(*this, Unicode::narrowToWide("If you wish to pick up more items than available inventory space, you can specify the \"overload\" option with the command by typing \"/pickupAllRoomItemsIntoInventory -overload\" which will pick up additional items and overload your inventory, but not so overloaded as to prevent you from moving."), Unicode::emptyString);

				break;
			}

			Container const * const cellContainer = ContainerInterface::getContainer(*containingCell);
			if (!cellContainer)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because of an internal error."), Unicode::emptyString);
				break;
			}

			std::set<NetworkId> & processedItemsSoFar = playerObject->getPickupDropAllItemsProcessedItemsSoFar();
			std::list<CachedNetworkId> cellContents;
			{
				for (ContainerConstIterator iter = cellContainer->begin(); iter != cellContainer->end(); ++iter)
				{
					if ((*iter != getNetworkId()) && (processedItemsSoFar.count(*iter) <= 0))
						cellContents.push_back(*iter);
				}
			}

			if (cellContents.empty())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because no more items in the room can be picked up or the room is empty."), Unicode::emptyString);
				break;
			}

			// if necessary, temporary increase the inventory limit to allow overload
			if (inventoryLimit > inventoryBaseLimit)
				targetInventoryContainer->debugDoNotUseSetCapacity(inventoryLimit);

			int numberOfItemsPickedUpSoFar = playerObject->getPickupDropAllItemsNumItemsSoFar();

			char buffer[512];
			Vector const pos = Transform::identity.getPosition_p();
			snprintf(buffer, sizeof(buffer)-1, " %s -1 %f %f %f", targetInventoryObj->getNetworkId().getValueString().c_str(), pos.x, pos.y, pos.z);
			buffer[sizeof(buffer)-1] = '\0';
			Unicode::String const commandTransferItemMiscParams = Unicode::narrowToWide(buffer);

			static uint32 const hashTransferItemMisc = Crc::normalizeAndCalculate("transferItemMisc");
			Command const & commandTransferItemMisc = CommandTable::getCommand(hashTransferItemMisc);
			ServerObject const * cellContent;
			bool blockedByNoTrade;
			bool showPickUpMenu;
			Container::ContainerErrorCode cec;
			int previousVolume, newVolume;
			int numberOfItemsToProcessThisRound = ConfigServerGame::getPickDropAllRoomItemsNumberOfItemsPerRound();
			for (std::list<CachedNetworkId>::const_iterator iter = cellContents.begin(); iter != cellContents.end(); ++iter)
			{
				--numberOfItemsToProcessThisRound;
				processedItemsSoFar.insert(*iter);

				cellContent = safe_cast<ServerObject const *>(iter->getObject());
				if (!cellContent)
					continue;

				if (ContainerInterface::getContainedByObject(*cellContent) != containingCell)
					continue;

				// see if the item would normally have the "Pick Up" menu item on it
				if (!PlayerCreatureController::objectMenuRequestCanManipulateObject(*this, *cellContent, blockedByNoTrade, showPickUpMenu))
					continue;

				if (!showPickUpMenu)
					continue;

				// see if item would fit in inventory
				if (!targetInventoryContainer->mayAdd(*cellContent, cec))
					continue;

				// "Pick Up" the item, just as if the player had used the "Pick Up" radial or /pickup command
				previousVolume = targetInventoryContainer->getCurrentVolume();
				CommandCppFuncs::commandFuncTransferMisc(commandTransferItemMisc, getNetworkId(), cellContent->getNetworkId(), commandTransferItemMiscParams);

				// stop trying when inventory is full
				newVolume = targetInventoryContainer->getCurrentVolume();
				if (newVolume >= inventoryLimit)
				{
					Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because your inventory is full."), Unicode::emptyString);

					// if they didn't run the command with the -overload
					// option, remind them about the -overload option
					if (!playerObject->getPickupAllItemsAllowInventoryOverload())
						Chat::sendSystemMessage(*this, Unicode::narrowToWide("If you wish to pick up more items than available inventory space, you can specify the \"overload\" option with the command by typing \"/pickupAllRoomItemsIntoInventory -overload\" which will pick up additional items and overload your inventory, but not so overloaded as to prevent you from moving."), Unicode::emptyString);

					break;
				}

				// update count of how many items have been picked up for the operation
				numberOfItemsPickedUpSoFar += std::max(0, (newVolume - previousVolume));

				// stop trying when enough items have been picked up for the operation
				if (numberOfItemsPickedUpSoFar > inventoryLimit)
				{
					Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because \"enough\" items have been picked up."), Unicode::emptyString);
					break;
				}

				// only do so many items each round, to spread out the load
				if (numberOfItemsToProcessThisRound <= 0)
					break;
			}

			// if necessary, restore the inventory limit
			if (inventoryLimit > inventoryBaseLimit)
				targetInventoryContainer->debugDoNotUseSetCapacity(inventoryBaseLimit);

			// stop if inventory is full
			if (targetInventoryContainer->getCurrentVolume() >= inventoryLimit)
				break;

			// stop if enough items have been picked up for the operation
			if (numberOfItemsPickedUpSoFar > inventoryLimit)
				break;
			else
				playerObject->setPickupDropAllItemsNumItemsSoFar(numberOfItemsPickedUpSoFar);

			// stop if no more items can be picked up
			if (numberOfItemsToProcessThisRound > 0)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /pickupAllRoomItemsIntoInventory operation has been terminated because no more items in the room can be picked up or the room is empty."), Unicode::emptyString);
				break;
			}

			// queue up the next round of pickup
			MessageToQueue::getInstance().sendMessageToC(getNetworkId(),
				"C++PickupAllRoomItemsIntoInventory",
				"",
				1,
				false);

			return;
		}

		// if we reach this point, it means we are stopping the operation, so cleanup
		removeObjVarItem("pickupDropAllItemsOperation");

		if (playerObject)
		{
			playerObject->setPickupDropAllItemsOperationId(0);
			playerObject->setPickupDropAllItemsCellId(NetworkId::cms_invalid);
			playerObject->setPickupAllItemsAllowInventoryOverload(false);
			playerObject->setPickupDropAllItemsNumItemsSoFar(0);
			playerObject->getPickupDropAllItemsProcessedItemsSoFar().clear();
		}
	}
	else if (message.getMethod() == "C++DropAllInventoryItemsIntoRoom")
	{
		// use while(true) loop to use break to discontinue processing and go to the end of loop to run cleanup code
		PlayerObject * playerObject = nullptr;
		while (true)
		{
			playerObject = PlayerCreatureController::getPlayerObject(this);
			if (!playerObject)
				break;

			// player has disconnected
			if (!getClient())
				break;

			// don't allow this in god mode because it could be used accidentally and drop a bunch of stuff,
			// and god mode allows stuff to be dropped, which wouldn't normally be allowed to be dropped
			if (getClient()->isGod())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because you switched into GOD mode."), Unicode::emptyString);
				break;
			}

			// player is not "active", meaning he's probably doing it with a macro
			if (playerObject->getSessionLastActiveTime() == 0)
				break;

			int operationId = 0;
			if (!getObjVars().getItem("pickupDropAllItemsOperation.operationId", operationId) || (operationId == 0))
				break;

			if (operationId != playerObject->getPickupDropAllItemsOperationId())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because of an unexpected condition.  Please try the operation again."), Unicode::emptyString);
				break;
			}

			NetworkId const & cellId = playerObject->getPickupDropAllItemsCellId();

			ServerObject const * const containingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));
			if (!containingCell || !containingCell->asCellObject() || (containingCell->getNetworkId() != cellId))
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because you have left the room."), Unicode::emptyString);
				break;
			}

			ServerObject const * const containingPOBso = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingCell));
			if (!containingPOBso)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because you have left the room."), Unicode::emptyString);
				break;
			}

			if (!containingPOBso->isInWorldCell())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because you have left the room."), Unicode::emptyString);
				break;
			}

			TangibleObject const * const containingPOB = containingPOBso->asTangibleObject();
			if (!containingPOB)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because of an internal error."), Unicode::emptyString);
				break;
			}

			if (!containingPOB->isOnAdminList(*this))
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because you are no longer the the owner or an admin of the structure or POB ship."), Unicode::emptyString);
				break;
			}

			int pobBaseItemLimit = 0;
			int pobAdditionalItems = 0;
			const int pobItemLimit = containingPOB->getPobTotalItemLimit(pobBaseItemLimit, pobAdditionalItems);

			if (containingPOB->getPobItemCount() >= pobItemLimit)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because the structure or POB ship is full."), Unicode::emptyString);
				break;
			}

			ServerObject const * const targetInventoryObj = getInventory();
			if (!targetInventoryObj)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because of an internal error."), Unicode::emptyString);
				break;
			}

			VolumeContainer const * const targetInventoryContainer = ContainerInterface::getVolumeContainer(*targetInventoryObj);
			if (!targetInventoryContainer)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because of an internal error."), Unicode::emptyString);
				break;
			}

			int const inventoryOverloadLimit = targetInventoryContainer->getTotalVolume() + ConfigServerGame::getMoveValidationMaxInventoryOverload();

			std::set<NetworkId> & processedItemsSoFar = playerObject->getPickupDropAllItemsProcessedItemsSoFar();
			std::list<CachedNetworkId> inventoryContents;
			{
				for (ContainerConstIterator iter = targetInventoryContainer->begin(); iter != targetInventoryContainer->end(); ++iter)
				{
					if (processedItemsSoFar.count(*iter) <= 0)
						inventoryContents.push_back(*iter);
				}
			}

			if (inventoryContents.empty())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because no more items in your inventory can be dropped or your inventory is empty."), Unicode::emptyString);
				break;
			}

			int numberOfItemsDroppedSoFar = playerObject->getPickupDropAllItemsNumItemsSoFar();

			char buffer[512];
			Vector const pos = getTransform_o2c().getPosition_p();
			snprintf(buffer, sizeof(buffer)-1, " %s -1 %f %f %f", containingCell->getNetworkId().getValueString().c_str(), pos.x, pos.y, pos.z);
			buffer[sizeof(buffer)-1] = '\0';
			Unicode::String const commandTransferItemMiscParams = Unicode::narrowToWide(buffer);

			static uint32 const hashTransferItemMisc = Crc::normalizeAndCalculate("transferItemMisc");
			Command const & commandTransferItemMisc = CommandTable::getCommand(hashTransferItemMisc);
			ServerObject const * inventoryContent;
			int previousVolume, newVolume;
			int numberOfItemsToProcessThisRound = ConfigServerGame::getPickDropAllRoomItemsNumberOfItemsPerRound();
			for (std::list<CachedNetworkId>::const_iterator iter = inventoryContents.begin(); iter != inventoryContents.end(); ++iter)
			{
				--numberOfItemsToProcessThisRound;
				processedItemsSoFar.insert(*iter);

				inventoryContent = safe_cast<ServerObject const *>(iter->getObject());
				if (!inventoryContent)
					continue;

				if (ContainerInterface::getContainedByObject(*inventoryContent) != targetInventoryObj)
					continue;

				previousVolume = containingPOB->getPobItemCount();
				if ((previousVolume + inventoryContent->getVolume()) > pobItemLimit)
					continue;

				// "Drop" the item, just as if the player had used the "Drop" radial
				CommandCppFuncs::commandFuncTransferMisc(commandTransferItemMisc, getNetworkId(), inventoryContent->getNetworkId(), commandTransferItemMiscParams);

				// stop trying when POB is full
				newVolume = containingPOB->getPobItemCount();
				if (newVolume >= pobItemLimit)
				{
					Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because the structure or POB ship is full."), Unicode::emptyString);
					break;
				}

				// update count of how many items have been dropped for the operation
				numberOfItemsDroppedSoFar += std::max(0, (newVolume - previousVolume));

				// stop trying when enough items have been dropped for the operation
				if ((numberOfItemsDroppedSoFar > inventoryOverloadLimit) && (numberOfItemsDroppedSoFar > ConfigServerGame::getPickDropAllRoomItemsNumberOfItemsPerRound()))
				{
					Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because \"enough\" items have been dropped."), Unicode::emptyString);
					break;
				}

				// only do so many items each round, to spread out the load
				if (numberOfItemsToProcessThisRound <= 0)
					break;
			}

			// stop if POB is full
			if (containingPOB->getPobItemCount() >= pobItemLimit)
				break;

			// stop if enough items have been dropped for the operation
			if ((numberOfItemsDroppedSoFar > inventoryOverloadLimit) && (numberOfItemsDroppedSoFar > ConfigServerGame::getPickDropAllRoomItemsNumberOfItemsPerRound()))
				break;
			else
				playerObject->setPickupDropAllItemsNumItemsSoFar(numberOfItemsDroppedSoFar);

			// stop if no more items can be dropped
			if (numberOfItemsToProcessThisRound > 0)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The /dropAllInventoryItemsIntoRoom operation has been terminated because no more items in your inventory can be dropped or your inventory is empty."), Unicode::emptyString);
				break;
			}

			// queue up the next round of drop
			MessageToQueue::getInstance().sendMessageToC(getNetworkId(),
				"C++DropAllInventoryItemsIntoRoom",
				"",
				1,
				false);

			return;
		}

		// if we reach this point, it means we are stopping the operation, so cleanup
		removeObjVarItem("pickupDropAllItemsOperation");

		if (playerObject)
		{
			playerObject->setPickupDropAllItemsOperationId(0);
			playerObject->setPickupDropAllItemsCellId(NetworkId::cms_invalid);
			playerObject->setPickupAllItemsAllowInventoryOverload(false);
			playerObject->setPickupDropAllItemsNumItemsSoFar(0);
			playerObject->getPickupDropAllItemsProcessedItemsSoFar().clear();
		}
	}
	else if (message.getMethod() == "C++RestoreDecorationLayout")
	{
		// use while(true) loop to use break to discontinue processing and go to the end of loop to run cleanup code
		PlayerObject * playerObject = nullptr;
		while (true)
		{
			playerObject = PlayerCreatureController::getPlayerObject(this);
			if (!playerObject)
				break;

			// player has disconnected
			if (!getClient())
				break;

			// player is not "active", meaning he's probably doing it with a macro
			if (playerObject->getSessionLastActiveTime() == 0)
				break;

			int operationId = 0;
			if (!getObjVars().getItem("restoreDecorationOperation.operationId", operationId) || (operationId == 0))
				break;

			if (operationId != playerObject->getRestoreDecorationOperationId())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The restore decoration layout operation has been terminated because of an unexpected condition.  Please try the operation again."), Unicode::emptyString);
				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("%d items to be moved/restored, %d items attempted, %d items moved/restored.", playerObject->getRestoreDecorationTotalCount(), playerObject->getRestoreDecorationAttemptedCount(), playerObject->getRestoreDecorationSuccessCount())), Unicode::emptyString);
				break;
			}

			ServerObject * const containingCell = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*this));
			if (!containingCell || !containingCell->asCellObject())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The restore decoration layout operation has been terminated because you have left the structure or POB ship."), Unicode::emptyString);
				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("%d items to be moved/restored, %d items attempted, %d items moved/restored.", playerObject->getRestoreDecorationTotalCount(), playerObject->getRestoreDecorationAttemptedCount(), playerObject->getRestoreDecorationSuccessCount())), Unicode::emptyString);
				break;
			}

			ServerObject * const containingPOBso = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*containingCell));
			if (!containingPOBso)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The restore decoration layout operation has been terminated because you have left the structure or POB ship."), Unicode::emptyString);
				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("%d items to be moved/restored, %d items attempted, %d items moved/restored.", playerObject->getRestoreDecorationTotalCount(), playerObject->getRestoreDecorationAttemptedCount(), playerObject->getRestoreDecorationSuccessCount())), Unicode::emptyString);
				break;
			}

			if (!containingPOBso->isInWorldCell())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The restore decoration layout operation has been terminated because you have left the structure or POB ship."), Unicode::emptyString);
				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("%d items to be moved/restored, %d items attempted, %d items moved/restored.", playerObject->getRestoreDecorationTotalCount(), playerObject->getRestoreDecorationAttemptedCount(), playerObject->getRestoreDecorationSuccessCount())), Unicode::emptyString);
				break;
			}

			if (containingPOBso->getNetworkId() != playerObject->getRestoreDecorationPobId())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The restore decoration layout operation has been terminated because you have left the structure or POB ship."), Unicode::emptyString);
				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("%d items to be moved/restored, %d items attempted, %d items moved/restored.", playerObject->getRestoreDecorationTotalCount(), playerObject->getRestoreDecorationAttemptedCount(), playerObject->getRestoreDecorationSuccessCount())), Unicode::emptyString);
				break;
			}

			TangibleObject * const containingPOB = containingPOBso->asTangibleObject();
			if (!containingPOB)
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The restore decoration layout operation has been terminated because of an internal error."), Unicode::emptyString);
				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("%d items to be moved/restored, %d items attempted, %d items moved/restored.", playerObject->getRestoreDecorationTotalCount(), playerObject->getRestoreDecorationAttemptedCount(), playerObject->getRestoreDecorationSuccessCount())), Unicode::emptyString);
				break;
			}

			if (!containingPOB->isOnAdminList(*this))
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The restore decoration layout operation has been terminated because you are no longer the the owner or an admin of the structure or POB ship."), Unicode::emptyString);
				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("%d items to be moved/restored, %d items attempted, %d items moved/restored.", playerObject->getRestoreDecorationTotalCount(), playerObject->getRestoreDecorationAttemptedCount(), playerObject->getRestoreDecorationSuccessCount())), Unicode::emptyString);
				break;
			}

			int numberOfItemsToProcessThisRound = ConfigServerGame::getPickDropAllRoomItemsNumberOfItemsPerRound();
			std::list<std::pair<std::pair<CachedNetworkId, CachedNetworkId>, std::pair<Vector, Quaternion> > > & restoreDecorationLayoutList = playerObject->getRestoreDecorationLayoutList();
			while (!restoreDecorationLayoutList.empty() && (numberOfItemsToProcessThisRound > 0))
			{
				std::pair<std::pair<CachedNetworkId, CachedNetworkId>, std::pair<Vector, Quaternion> > const & front = restoreDecorationLayoutList.front();
				restoreItemDecorationLayout(*this, *playerObject, front.first.first, *containingPOB, front.first.second, front.second.first, front.second.second);
				playerObject->setRestoreDecorationAttemptedCount(playerObject->getRestoreDecorationAttemptedCount() + 1);
				--numberOfItemsToProcessThisRound;
				restoreDecorationLayoutList.pop_front();
			}

			if (restoreDecorationLayoutList.empty())
			{
				Chat::sendSystemMessage(*this, Unicode::narrowToWide("The restore decoration layout operation has been completed."), Unicode::emptyString);
				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("%d items to be moved/restored, %d items attempted, %d items moved/restored.", playerObject->getRestoreDecorationTotalCount(), playerObject->getRestoreDecorationAttemptedCount(), playerObject->getRestoreDecorationSuccessCount())), Unicode::emptyString);
				break;
			}

			// queue up the next round of restore
			MessageToQueue::getInstance().sendMessageToC(getNetworkId(),
				"C++RestoreDecorationLayout",
				"",
				1,
				false);

			return;
		}

		// if we reach this point, it means we are stopping the operation, so cleanup
		removeObjVarItem("restoreDecorationOperation");

		if (playerObject)
		{
			playerObject->setRestoreDecorationOperationId(0);
			playerObject->setRestoreDecorationPobId(NetworkId::cms_invalid);
			playerObject->setRestoreDecorationTotalCount(0);
			playerObject->setRestoreDecorationAttemptedCount(0);
			playerObject->setRestoreDecorationSuccessCount(0);
			playerObject->getRestoreDecorationLayoutList().clear();
		}
	}
	else if (message.getMethod() == "C++CityAddCitizen")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			const int cityId = atoi(params.c_str());
			if (cityId > 0)
				CityInterface::enterCityChatRoom(cityId, *this);
		}
	}
	else if (message.getMethod() == "C++CityRemoveCitizen")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			const int cityId = atoi(params.c_str());
			if (cityId > 0)
				CityInterface::leaveCityChatRoom(cityId, *this);
		}
	}
	else
	{
		TangibleObject::handleCMessageTo(message);
	}
}

//-----------------------------------------------------------------------

bool CreatureObject::alwaysSendReliableTransform() const
{
	return false;
}

//-----------------------------------------------------------------------

bool CreatureObject::immediateLogoutAllowed()
{
	// Notes:
	//   1. This should only be called on authoritative objects.
	//   2. This call is triggered by a client disconnect that we will only see if a client is
	//      already associated with this object.
	//   3. A client is only associated with an object when the object has finished baselines and
	//      is authoritative; therefore, we should never hit a case where a player that is logging
	//      in gets disconnected and causes this function to get called.
	//   4. Even though this function is phrased as a query, it has some serious side effects.
	//      Calling it should be treated as if the caller is saying "Prepare for a disconnect" even
	//      though script can override and prevent an immediate logout.
	DEBUG_FATAL(!isAuthoritative(), ("CreatureObject::immediateLogoutAllowed() should only be called on authoritative objects."));

	// if we are in an npc conversation, end it
	if (isInNpcConversation())
		endNpcConversation();

	// if we are mounted, dismount.
	if (getState(States::RidingMount))
	{
		CreatureObject *const mountedCreature = getMountedCreature();
		if (mountedCreature)
			mountedCreature->detachRider(getNetworkId());
	}

	virtualOnLogout();

	LOG("CustomerService", ("Money:%s logging out with %d money", PlayerObject::getAccountDescription(getNetworkId()).c_str(), getTotalMoney()));

	ScriptParams params;
	return (getScriptObject()->trigAllScripts(Scripting::TRIG_IMMEDIATE_LOGOUT, params) == SCRIPT_CONTINUE);
}

//-----------------------------------------------------------------------

float CreatureObject::getStepHeight() const
{
	return static_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getStepHeight();
}

//----------------------------------------------------------------------

float CreatureObject::getApproachTriggerRange() const
{
	return static_cast<const ServerCreatureObjectTemplate *>(getObjectTemplate())->getApproachTriggerRange();
}

//----------------------------------------------------------------------

void CreatureObject::setIsBeast(bool newValue)
{
	if(m_isBeast.get() != newValue)
	{
		m_isBeast.set(newValue);
	}
}

//----------------------------------------------------------------------

bool CreatureObject::setForceShowHam(bool show)
{
	if(!isAuthoritative())
	{
		return false;
	}

	m_forceShowHam.set(show);
	return true;
}

//----------------------------------------------------------------------

void CreatureObject::setIsStatic(bool newValue)
{
	if(m_isStatic.get() != newValue)
	{
		m_isStatic = newValue;
		if (newValue)
		{
			TerrainObject* const terrain = TerrainObject::getInstance();
			if (terrain && terrain->isReferenceObject(this))
				terrain->removeReferenceObject(this);
			// don't alter static creatures
			if (!isPlayerControlled())
				setDefaultAlterTime(AlterResult::cms_keepNoAlter);
		}
		else
		{
			potentiallyAddToTerrain();
			if (!isPlayerControlled())
				setDefaultAlterTime(AlterResult::cms_alterQuickly);
		}
	}
}

//----------------------------------------------------------------------

bool CreatureObject::potentiallyAddToTerrain()
{
	//@todo don't make reference object if any of the following are true:
	// check is "asleep" (must add when woken up)

	TerrainObject* const terrain = TerrainObject::getInstance ();
	if ( isAuthoritative() && !getIsStatic() && !getCreatureController()->getHibernate() && terrain && !terrain->isReferenceObject(this) && isInWorldCell() )
	{
		terrain->addReferenceObject(this);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void CreatureObject::virtualOnAddedToWorldCell()
{
	TangibleObject::virtualOnAddedToWorldCell();
	potentiallyAddToTerrain();
	m_lastRegionNotifyPosition = getPosition_w();
	m_lastRegionNotifyPosition.x += 1.0f; // force a check
	checkNotifyRegions();
}

//------------------------------------------------------------------------------------------

void CreatureObject::virtualOnRemovedFromWorldCell()
{
	TangibleObject::virtualOnRemovedFromWorldCell();

	TerrainObject* const terrain = TerrainObject::getInstance();
	if (terrain && terrain->isReferenceObject(this))
		terrain->removeReferenceObject(this);
}

//------------------------------------------------------------------------------------------

void CreatureObject::virtualOnSetAuthority()
{
	TangibleObject::virtualOnSetAuthority();

	potentiallyAddToTerrain();

	for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
		m_regeneration[i] = 0;

	if (isPlayerControlled())
	{
		setNumberOfMissionsWantedInMissionBag(ConfigServerGame::getNumberOfMissionsWantedInMissionBag());

		if (isInitialized())
		{
			Client *client = GameServer::getInstance().getClient(getNetworkId());
			if (client)
				client->onLoadPlayer(*this);
		}
	}

	AICreatureController * const controller = AICreatureController::asAiCreatureController(getController());

	if (controller)
		controller->setAuthority(true);

	//-- Update the creature's group mission critical object set
	GroupMissionCriticalObjectsBuilder::updateGroupMissionCriticalObjects(*this, true);
}

//------------------------------------------------------------------------------------------

void CreatureObject::virtualOnReleaseAuthority()
{
	TangibleObject::virtualOnReleaseAuthority();
	TerrainObject* const terrain = TerrainObject::getInstance();
	if (terrain && terrain->isReferenceObject(this))
		terrain->removeReferenceObject(this);

	AICreatureController * const controller = AICreatureController::asAiCreatureController(getController());

	if(controller)
	{
		if (getProperty(SlopeEffectProperty::getClassPropertyId()) != nullptr)
			removeProperty(SlopeEffectProperty::getClassPropertyId());
		controller->setAuthority(false);
	}
}

//------------------------------------------------------------------------------------------

void CreatureObject::virtualOnLogout()
{
	TangibleObject::virtualOnLogout();
	PlayerObject * const player = PlayerCreatureController::getPlayerObject(this);
	if (player != nullptr)
		player->virtualOnLogout();
}

//------------------------------------------------------------------------------------------

bool CreatureObject::wantSawAttackTriggers() const
{
	if (isDisabled())
		return false;
	return TangibleObject::wantSawAttackTriggers();
}

//----------------------------------------------------------------------

void CreatureObject::addPackedWearable(std::string const &appearanceData, int arrangementIndex, NetworkId const &networkId, uint32 sharedTemplateCrcValue, const BaselinesMessage * weaponSharedBaselines, const BaselinesMessage * weaponSharedNpBaselines)
{
	//-- Check if this network id already exists in the wearable list.
	size_t const length = m_wearableData.size();
	for (size_t i = 0; i < length; ++i)
	{
		WearableEntry const &entry = m_wearableData[i];
		if (entry.m_networkId == networkId)
		{
			// Check if this entry exists but in a different configuration.
			WARNING((entry.m_arrangement != arrangementIndex), ("attempted to add object id=[%s] in packed wearable list multiple times in different arrangements, existing=[%d],new=[%d].", appearanceData.c_str(), entry.m_arrangement, arrangementIndex));
			WARNING((static_cast<uint32>(entry.m_objectTemplate) != sharedTemplateCrcValue), ("attempted to add object id=[%s] in packed wearable list multiple times differing only by shared object template crc value, existing=[%d],new=[%d].", appearanceData.c_str(), entry.m_objectTemplate, static_cast<int>(sharedTemplateCrcValue)));
			DEBUG_WARNING((entry.m_appearanceString != appearanceData), ("attempted to add object id=[%s] in packed wearable list multiple times differing only by appearance customization data.", appearanceData.c_str()));

			// Done, this entry is already in the wearable data list.

			// we own the weaponSharedBaselines and weaponSharedBaselines pointers, so if
			// we are not going to add them to the wearables list, we must delete them
			delete weaponSharedBaselines;
			delete weaponSharedNpBaselines;

			return;
		}
	}

	//-- Add the new entry.
	m_wearableData.push_back(WearableEntry(appearanceData, arrangementIndex, networkId, sharedTemplateCrcValue, weaponSharedBaselines, weaponSharedNpBaselines));
}

//----------------------------------------------------------------------

void CreatureObject::packWearables()
{
	// Pack wearables data

	SlottedContainer const * const myContainer = ContainerInterface::getSlottedContainer(*this);

	if (myContainer)
	{
		Object const * inventory = 0;
		Object const * datapad = 0;
		Object const * appearance = 0;
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		static SlotId const inventorySlot = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::inventory);
		if (inventorySlot != SlotId::invalid)
		{
			Container::ContainedItem itemId = myContainer->getObjectInSlot(inventorySlot, tmp);
			inventory = itemId.getObject();
		}
		SlotId datapadSlot = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::datapad);
		if (datapadSlot != SlotId::invalid)
		{
			Container::ContainedItem itemId = myContainer->getObjectInSlot(datapadSlot, tmp);
			datapad = itemId.getObject();
		}
		SlotId appearanceInvSlot = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::appearance);
		if (appearanceInvSlot != SlotId::invalid)
		{
			Container::ContainedItem itemId = myContainer->getObjectInSlot(appearanceInvSlot, tmp);
			appearance = itemId.getObject();
		}

		for (ContainerConstIterator iter = myContainer->begin(); iter != myContainer->end(); ++iter)
		{
			Object const * const o = (*iter).getObject();
			if (o)
			{
				ServerObject const * const so = o->asServerObject();
				if (so)
				{
					TangibleObject const * const tang = so->asTangibleObject();
					if (tang)
					{
						SlottedContainmentProperty const * const scp = ContainerInterface::getSlottedContainmentProperty(*tang);
						if (scp)
						{
							if (scp->isInAppearanceSlot() || tang == inventory || tang == datapad )
							{
								ConstCharCrcString clientSharedTemplateNameCrcString = ObjectTemplateList::lookUp(tang->getClientSharedTemplateName());
								if (clientSharedTemplateNameCrcString.isEmpty())
									WARNING(true, ("Could not find crc for [%s].  Wearable will not be streamed to client", tang->getClientSharedTemplateName()));
								else if (so->asWeaponObject() != nullptr)
								{
									addPackedWearable(tang->getAppearanceData(), scp->getCurrentArrangement(), tang->getNetworkId(), clientSharedTemplateNameCrcString.getCrc(),
										tang->createSharedBaselinesMessage(), tang->createSharedNpBaselinesMessage());
								}
								else
									addPackedWearable(tang->getAppearanceData(), scp->getCurrentArrangement(), tang->getNetworkId(), clientSharedTemplateNameCrcString.getCrc());
							}
						}
					}
				}
			}
		}

		if(appearance)
		{
			SlottedContainer const * const myAppearanceContainer = ContainerInterface::getSlottedContainer(*appearance);
			for(ContainerConstIterator iter = myAppearanceContainer->begin(); iter!= myAppearanceContainer->end(); ++iter)
			{
				Object const * const o = (*iter).getObject();
				if (o)
				{
					ServerObject const * const so = o->asServerObject();
					if (so)
					{
						TangibleObject const * const tang = so->asTangibleObject();
						if (tang)
						{
							SlottedContainmentProperty const * const scp = ContainerInterface::getSlottedContainmentProperty(*tang);
							if (scp)
							{
								if (scp->isInAppearanceSlot())
								{
									ConstCharCrcString clientSharedTemplateNameCrcString = ObjectTemplateList::lookUp(tang->getClientSharedTemplateName());
									if (clientSharedTemplateNameCrcString.isEmpty())
										WARNING(true, ("Could not find crc for [%s].  Wearable will not be streamed to client", tang->getClientSharedTemplateName()));
									else
										addPackedAppearanceWearable(tang->getAppearanceData(), scp->getCurrentArrangement(), tang->getNetworkId(), clientSharedTemplateNameCrcString.getCrc());
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

/**
 * Called when we get an ack back that a Jedi slot has been added to our account.
 * This function should only be called by PlayerObject::addJediToAccountAck()!
 */
void CreatureObject::addJediToAccountAck()
{
	// flag success via an objvar so we can end the timed callback C++AddJediSlot
	setObjVarItem(OBJVAR_ADD_JEDI_ACK, 1);
}	// CreatureObject::addJediToAccountAck


//----------------------------------------------------------------------

void CreatureObject::setNumberOfMissionsWantedInMissionBag(int n)
{
	int i = 0;
	ServerObject * const missionBag = getMissionBag();
	if(missionBag)
	{
		Container * const container = ContainerInterface::getContainer(*missionBag);
		ContainerIterator iter;
		for(iter = container->begin(); iter != container->end(); ++iter)
		{
			Object * const o = (*iter).getObject();
			if (o)
			{
				ServerObject * const so = o->asServerObject();
				if (so && so->asMissionObject())
					++i;
			}
		}

		if (n - i > 0)
		{
			if (m_numberOfMissionsWantedInMissionBag <= 0)
			{
				// place object on mission object create queue
				s_missionQueue.push_back(getNetworkId());
			}
			m_numberOfMissionsWantedInMissionBag = n - i;
		}
		else if (i - n > 0)
		{
			// remove mission objects from the bag
			int j = 0;
			for (iter = container->begin(), j = 0; (iter != container->end() && j < i - n); ++iter, ++j)
			{
				Object * const o = (*iter).getObject();
				if (o)
				{
					ServerObject * const so = o->asServerObject();
					if (so && so->asMissionObject())
						so->kill();
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void CreatureObject::runMissionCreationQueue()
{
	int i = ConfigServerGame::getMissionCreationQueueSize();
	while(i > 0 && !s_missionQueue.empty())
	{
		const NetworkId id = s_missionQueue.front();
		s_missionQueue.pop_front();
		CreatureObject * creature = safe_cast<CreatureObject *>(NetworkIdManager::getObjectById(id));
		if(creature)
		{
			if(creature->isAuthoritative())
			{
				if(MissionObject::createMissionObjectInCreatureMissionBag(creature))
					creature->m_numberOfMissionsWantedInMissionBag--;
				if(creature->m_numberOfMissionsWantedInMissionBag > 0)
					s_missionQueue.push_back(id);
			}
		}
		i--;
	}
}

//----------------------------------------------------------------------

int CreatureObject::getMaxNumberOfLots() const
{
	return m_maxHousingLots;
}

//----------------------------------------------------------------------

NetworkId CreatureObject::getHouse() const
{
	NetworkId houseId;

	// the objvar houseId is per player and is used if it exists
	if (getObjVars().getItem(OBJVAR_PLAYER_HOUSE_ID, houseId))
		return houseId;

	return NetworkId::cms_invalid;
}

//----------------------------------------------------------------------

void CreatureObject::setHouse(const CachedNetworkId & houseId)
{
	if (isAuthoritative())
	{
		// store as an objvar to keep this id per player vs. per account
		setObjVarItem(OBJVAR_PLAYER_HOUSE_ID, houseId);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setHouse, new MessageQueueGenericValueType<NetworkId>(houseId));
	}
}

// ----------------------------------------------------------------------

void CreatureObject::getAvailableLocomotions()
{
	getMovementTable()->getAllLocomotions(m_posture.get(), m_states.get(), m_fastLocomotion, m_slowLocomotion, m_stationaryLocomotion);
}

// ----------------------------------------------------------------------

void CreatureObject::calcPvpableState()
{
	setPvpable(true);
}

// ----------------------------------------------------------------------

float CreatureObject::getBaseWalkSpeed() const
{
	float speed = m_baseWalkSpeed.get();
	if (speed < 0.0f)
		return safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSpeed(SharedCreatureObjectTemplate::MT_walk);
	return speed;
}

// ----------------------------------------------------------------------

void CreatureObject::setBaseWalkSpeed(float speed)
{
	if (isAuthoritative())
	{
		if (m_baseWalkSpeed.get() != speed)
		{
			m_baseWalkSpeed = speed;
			requestMovementInfoUpdate();
		}
	}
	else
	{
		Controller *controller = getController();
		if (controller)
			controller->appendMessage(
				CM_creatureSetBaseWalkSpeed,
				speed,
				0,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

float CreatureObject::getBaseRunSpeed() const
{
	float speed = m_baseRunSpeed.get();
	if (speed < 0.0f)
		return safe_cast<const SharedCreatureObjectTemplate *>(getSharedTemplate())->getSpeed(SharedCreatureObjectTemplate::MT_run);
	return speed;
}

// ----------------------------------------------------------------------

int CreatureObject::loadPackedHouses()
{
	int numHouses = 0;
	ServerObject *datapad = getDatapad();
	if (datapad)
	{
		VolumeContainer const * datapadContainer = ContainerInterface::getVolumeContainer(*datapad);
		if (datapadContainer)
		{
			// iterate over contents.
			for(ContainerConstIterator iter(datapadContainer->begin());
				iter != datapadContainer->end();
				++iter)
			{
				const CachedNetworkId & objId = (*iter);
				const ServerObject * obj = safe_cast<const ServerObject *>(objId.getObject());
				if (obj && (obj->getGameObjectType() == SharedObjectTemplate::GOT_data_house_control_device))
				{
					// if object is a house control device, then go ahead and get the first contained item -
					// should be the house itself.
					SlottedContainer const *hcdContainer = ContainerInterface::getSlottedContainer(*obj);
					if (hcdContainer && hcdContainer->getNumberOfItems() > 0)
					{
						//load the contents for the house.
						for(ContainerConstIterator hcdIter(hcdContainer->begin());
							hcdIter != hcdContainer->end();
							++hcdIter)
						{
							Object * const houseId = (*hcdIter).getObject();
							BuildingObject * const house = (houseId ? houseId->asServerObject()->asBuildingObject() : nullptr);
							if (house && !house->getContentsLoaded())
							{
								LOG("CustomerService", ("CharacterTransfer: starting packed house load (%s) for CTS character %s", house->getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(this).c_str()));

								numHouses++;
								house->loadContents();
								setLoadCTSPackedHouses(true);
							}
						}
					}
				}
			}
		}
	}

	return numHouses;
}

// ----------------------------------------------------------------------

std::vector<unsigned char> CreatureObject::uploadCharacterData(const bool withItems, const bool allowOverride)
{
	std::vector<unsigned char> result;

	if(getClient())
	{
		GameScriptObject * scriptObject = getScriptObject();
		if(scriptObject)
		{
			int resultSize = 0;
			std::vector<unsigned char> tmp;
			tmp.resize(ConfigServerGame::getCtsDefaultScriptDictionarySizeBytes());

			ScriptParams p;
			p.addParam(tmp);
			p.addParam(resultSize);
			p.addParam(withItems);
			p.addParam(allowOverride);
			REPORT_LOG(true, ("Calling OnStartCharacterUpload from CreatureObject.cpp\n"));
			IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_START_CHARACTER_UPLOAD, p));

			resultSize = p.getIntParam(1);
			tmp = p.getByteArrayParam(0);
			int i;
			for(i=0; i < resultSize; ++i)
			{
				result.push_back(tmp[static_cast<size_t>(i)]);
			}
			LOG("CustomerService", ("CharacterTransfer: CreatureObject::uploadCharacterData() yields a buffer %d bytes long", result.size()));
		}
	}
	return result;
}

// ----------------------------------------------------------------------

bool  CreatureObject::receiveCharacterTransferMessage(const std::vector<unsigned char> & packedCharacterData)
{
	bool result = false;
	GameScriptObject * scriptObject = getScriptObject();
	if(scriptObject)
	{
		ScriptParams p;
		p.addParam(packedCharacterData);
		if(scriptObject->trigAllScripts(Scripting::TRIG_DOWNLOAD_CHARACTER, p))
		{
			result = true;
		}
	}

	if (result)
	{
	// tell CentralServer to not allow login for this character
	// until the character has been persisted and unloaded
		GenericValueTypeMessage<std::pair<NetworkId, std::pair<std::string, bool> > > const msg(
			"SetSceneForPlayer",
			std::make_pair(
			getNetworkId(),
			std::make_pair(ServerWorld::getSceneId(), true)));

		GameServer::getInstance().sendToCentralServer(msg);

		// set the idle time so that the character will be disconnected
		// in ConfigServerGame::getIdleLogoutTimeAfterCharacterTransferSec()
		// so that the character will get unloaded after the next save cycle,
		// so that the player can log in the character; this should be much
		// faster than the default idle disconnect behavior where the character
		// would get disconnected in ConfigServerGame::getIdleLogoutTimeSec()
		if (getClient())
			getClient()->resetIdleTimeAfterCharacterTransfer();
	}

	return result;
}

// ----------------------------------------------------------------------

void  CreatureObject::receiveCharacterTransferStatusMessage(const std::string & statusMessage)
{
	GameScriptObject * scriptObject = getScriptObject();
	if(scriptObject)
	{
		ScriptParams p;
		p.addParam(statusMessage.c_str());
		scriptObject->trigAllScripts(Scripting::TRIG_RECEIVE_CHARACTER_TRANSFER_STATUS_MESSAGE, p);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::setBaseRunSpeed(float speed)
{
	if (isAuthoritative())
	{
		if (m_baseRunSpeed.get() != speed)
		{
			m_baseRunSpeed = speed;
			requestMovementInfoUpdate();
		}
	}
	else
	{
		Controller *controller = getController();
		if (controller)
			controller->appendMessage(
				CM_creatureSetBaseRunSpeed,
				speed,
				0,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

/**
 * Tests if this creature has a bounty on another creature.
 * (Virtual function, overriden in SwgCreatureObject)
 *
 * @param target		the creature to test if we have a bounty on
 *
 * @return true if we have a bounty on the target, false if not
 */
bool CreatureObject::hasBounty(const CreatureObject & target) const
{
	//@todo: this should really be a function of the pvp classes, but we
	// need this to be virtual
	return false;
}	// CreatureObject::hasBounty

// ----------------------------------------------------------------------

/**
 * Tests if this creature has a bounty on anything.
 * (Virtual function, overriden in SwgCreatureObject, can't be pure virtual
 * because ServerCreatureObjectTemplate has a function that instantiates a CreatureObject)
 */
bool CreatureObject::hasBounty() const
{
	return false;
}

// ----------------------------------------------------------------------

/**
 * Return a list of all the bounty hunters targeting this creature.
 *
 * Overriden in SwgCreatureObject.
 */
std::vector<NetworkId> const & CreatureObject::getJediBountiesOnMe() const
{
	static const std::vector<NetworkId> nothing;
	return nothing;
}

// ----------------------------------------------------------------------

bool CreatureObject::hasBountyMissionForTarget(const NetworkId & targetId) const
{
	const ServerObject * const datapad = getDatapad();
	if (!datapad)
		return false;

	const Container * const container = ContainerInterface::getContainer(*datapad);
	if (!container)
		return false;

	for (ContainerConstIterator iter = container->begin(); iter != container->end(); ++iter)
	{
		const ServerObject * const serverMissionObject = safe_cast<const ServerObject *>((*iter).getObject());
		if (serverMissionObject)
		{
			const MissionObject * const missionObject = serverMissionObject->asMissionObject();
			if (missionObject)
			{
				static const unsigned int bounty = CrcLowerString::calculateCrc("bounty");
				NetworkId bountyTargetId;

				if ((missionObject->getMissionType() == bounty) && (missionObject->getObjVars().getItem("objTarget", bountyTargetId)) && (bountyTargetId == targetId))
						return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

void CreatureObject::setAlternateAppearance(std::string const &sharedObjectTemplateName)
{
	//-- Ensure we're authoritative.
	if (!isAuthoritative())
	{
		WARNING(true,
			("CreatureObject::setAppearanceFromObjectTemplate(): called on non-authoritative object id=[%s], server id=[%d]",
			getNetworkId().getValueString().c_str(),
			static_cast<int>(GameServer::getInstance().getProcessId())
			));
		return;
	}
	
	// See if we're trying to turn off our alt appearance
	std::string const none("none");
	if(sharedObjectTemplateName.find(none) != std::string::npos && sharedObjectTemplateName.size() == none.size())
	{
		GameScriptObject * const scriptObject = this->getScriptObject();
		if(scriptObject)
		{
			ScriptParams params;
			params.addParam(getNetworkId());
			if(scriptObject->trigAllScripts(Scripting::TRIG_ON_ABOUT_TO_REVERT_APPEARANCE, params) == SCRIPT_OVERRIDE)
				return;
			m_alternateAppearanceSharedObjectTemplateName.set(sharedObjectTemplateName);

			setState(States::Disguised, false);

			params.clear();
			params.addParam(getNetworkId());

			IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_REVERTED_APPEARANCE, params));

		}
		
		return;
	}

	//-- Ensure the serverObjectTemplateName exists.
	if (sharedObjectTemplateName.empty() || !TreeFile::exists(sharedObjectTemplateName.c_str()))
	{
		WARNING(true,
			("CreatureObject::setAppearanceFromObjectTemplate(): called on id=[%s], server id=[%d]: invalid serverObjectTemplateName [%s]",
			getNetworkId().getValueString().c_str(),
			static_cast<int>(GameServer::getInstance().getProcessId()),
			sharedObjectTemplateName.c_str()
			));
		return;
	}


	ObjectTemplate const *sharedObjectTemplate = ObjectTemplateList::fetch(sharedObjectTemplateName);
	SharedCreatureObjectTemplate const *const sharedObjectTemplateAsCreature = dynamic_cast<SharedCreatureObjectTemplate const*>(sharedObjectTemplate);

	if (sharedObjectTemplate && sharedObjectTemplateAsCreature)
	{
		//-- Ensure there is an appearance filename.
		if (!sharedObjectTemplateAsCreature->getAppearanceFilename().empty())
		{
			GameScriptObject * const scriptObject = this->getScriptObject();
			if(scriptObject)
			{
				ScriptParams params;
				params.addParam(getNetworkId());
				params.addParam(sharedObjectTemplateName.c_str());

				if(scriptObject->trigAllScripts(Scripting::TRIG_ON_ABOUT_TO_CHANGE_APPEARANCE, params) == SCRIPT_OVERRIDE)
					return;

				//-- Set the alternate shared object template name since it is different from the natural shared object template name.
				m_alternateAppearanceSharedObjectTemplateName.set(sharedObjectTemplateAsCreature->getAppearanceFilename());

				setState(States::Disguised, true);

				params.clear();
				params.addParam(getNetworkId());

				IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_CHANGED_APPEARANCE, params));
			}

		}

		// Release resources.
		sharedObjectTemplate->releaseReference();
		sharedObjectTemplate = 0;
	}

}

// ======================================================================

bool operator!=(CreatureMod const &a, CreatureMod const &b)
{
	return    a.mod != b.mod
	       || a.startTime != b.startTime
	       || a.maxVal != b.maxVal
	       || a.currentVal != b.currentVal;
}

// ======================================================================

namespace Archive
{
	void put(Archive::ByteStream &target, CreatureAttitude const &state)
	{
		target.put(&state.m_behavior, 4);
		target.put(&state.m_angerState, 4);
		target.put(&state.m_fearState, 4);
		for (int i = 0; i < MentalStates::NumberOfMentalStates; ++i)
			target.put(&state.m_currentValues[i], 4);
	}

	void get(Archive::ReadIterator &source, CreatureAttitude &state)
	{
		source.get(&state.m_behavior, 4);
		source.get(&state.m_angerState, 4);
		source.get(&state.m_fearState, 4);
		for (int i = 0; i < MentalStates::NumberOfMentalStates; ++i)
			source.get(&state.m_currentValues[i], 4);
	}

	//----------------------------------------------------------------------

	void put(Archive::ByteStream &target, MonitoredCreatureMovement const &state)
	{
		target.put(&state.m_lastDistance, sizeof(state.m_lastDistance));
		target.put(&state.m_skittishness, sizeof(state.m_skittishness));
		target.put(&state.m_curve, sizeof(state.m_curve));
	}

	void get(Archive::ReadIterator &source, MonitoredCreatureMovement &state)
	{
		source.get(&state.m_lastDistance, sizeof(state.m_lastDistance));
		source.get(&state.m_skittishness, sizeof(state.m_skittishness));
		source.get(&state.m_curve, sizeof(state.m_curve));
	}

	//----------------------------------------------------------------------

	void put(Archive::ByteStream &target, CreatureMod const &mod)
	{
		target.put(&mod.startTime, sizeof(mod.startTime));
		target.put(&mod.maxVal, sizeof(mod.maxVal));
		target.put(&mod.currentVal, sizeof(mod.currentVal));
		put(target, mod.mod);
	}

	void get(Archive::ReadIterator &source, CreatureMod &mod)
	{
		source.get(&mod.startTime, sizeof(mod.startTime));
		source.get(&mod.maxVal, sizeof(mod.maxVal));
		source.get(&mod.currentVal, sizeof(mod.currentVal));
		get(source, mod.mod);
	}
};

//----------------------------------------------------------------------

float CreatureObject::getWalkSpeed    () const
{
	float walkSpeed = m_walkSpeed.get();

	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		if (m_vehiclePhysicsData)
		{
			walkSpeed = m_vehiclePhysicsData->m_walkSpeed;
		}
	}

	return walkSpeed * getMovementScale() * getMovementPercent();
}

//----------------------------------------------------------------------

float CreatureObject::getRunSpeed     () const
{
	float runSpeed = m_runSpeed.get();

	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		if (m_vehiclePhysicsData)
		{
			runSpeed = m_vehiclePhysicsData->m_runSpeed;
		}
	}

	return runSpeed * getMovementScale() * getMovementPercent();
}

//----------------------------------------------------------------------

float CreatureObject::getAccelerationForSpeed (float currentSpeed) const
{
	float accel = 0.0f;

	if (currentSpeed <= getWalkSpeed ())
		accel = safe_cast<const SharedCreatureObjectTemplate*> (getSharedTemplate ())->getAccelerationMax (SharedCreatureObjectTemplate::MT_walk);
	else
		accel = safe_cast<const SharedCreatureObjectTemplate*> (getSharedTemplate ())->getAccelerationMax (SharedCreatureObjectTemplate::MT_run);

	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		if (m_vehiclePhysicsData)
		{
			if (m_vehiclePhysicsData->m_runSpeed)
			{
				const float speedPercent = std::min (1.0f, currentSpeed / m_vehiclePhysicsData->m_runSpeed);
				accel = m_vehiclePhysicsData->m_accelerationMin + (m_vehiclePhysicsData->m_accelerationMax - m_vehiclePhysicsData->m_accelerationMin) * speedPercent;
			}
		}
	}

	 return accel * getAccelScale() * getAccelPercent();
}

//----------------------------------------------------------------------

float CreatureObject::getTurnRateForSpeed     (float currentSpeed) const
{
	float turnRate = 0.0f;

	if (currentSpeed <= getWalkSpeed ())
		turnRate = safe_cast<const SharedCreatureObjectTemplate*> (getSharedTemplate ())->getTurnRate (SharedCreatureObjectTemplate::MT_walk);
	else
		turnRate = safe_cast<const SharedCreatureObjectTemplate*> (getSharedTemplate ())->getTurnRate (SharedCreatureObjectTemplate::MT_run);

	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		if (m_vehiclePhysicsData)
		{
			if (m_vehiclePhysicsData->m_runSpeed)
			{
				const float speedPercent = std::min (1.0f, currentSpeed / m_vehiclePhysicsData->m_runSpeed);
				turnRate = convertRadiansToDegrees (m_vehiclePhysicsData->m_turnRateMin + (m_vehiclePhysicsData->m_turnRateMax - m_vehiclePhysicsData->m_turnRateMin) * speedPercent);
			}
		}
	}

	return turnRate;
}

//----------------------------------------------------------------------

float CreatureObject::getSlopeMod     () const
{
	return 1.0f;
}

//-----------------------------------------------------------------------

float CreatureObject::getAcceleration(SharedCreatureObjectTemplate::MovementTypes movementType) const
{
	float walk = getAccelerationForSpeed (getWalkSpeed ());
	float run  = getAccelerationForSpeed (getRunSpeed ());

	if (walk < 0.1f)
		walk = 0.1f;
	if (walk < run)
	{
		run = walk;
	}
	if (movementType == SharedCreatureObjectTemplate::MT_walk)
		return walk;
	else
		return run;
}

//-----------------------------------------------------------------------

float CreatureObject::getTurnRate(SharedCreatureObjectTemplate::MovementTypes movementType) const
{
	float walk = getTurnRateForSpeed (getWalkSpeed ());
	float run  = getTurnRateForSpeed (getRunSpeed ());

	if (walk < 30.0f)
		walk = 30.0f;
	if (walk < run)
	{
		run = walk * 0.5f;
	}
	if (movementType == SharedCreatureObjectTemplate::MT_walk)
		return walk;
	else
		return run;
}

//----------------------------------------------------------------------

void CreatureObject::updateVehiclePhysicsData ()
{
	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		if (!m_vehiclePhysicsData)
			m_vehiclePhysicsData = new VehiclePhysicsData;

		const CustomizationData *const cd = fetchCustomizationData();
		if (cd)
		{
			VehicleHoverDynamics::readParamsFromCustomizationData (*cd,
				m_vehiclePhysicsData->m_minMoveSpeed,
				m_vehiclePhysicsData->m_runSpeed,
				m_vehiclePhysicsData->m_turnRateMin,
				m_vehiclePhysicsData->m_turnRateMax,
				m_vehiclePhysicsData->m_accelerationMin,
				m_vehiclePhysicsData->m_accelerationMax);

			cd->release ();
			m_vehiclePhysicsData->m_walkSpeed = 0.5f * m_vehiclePhysicsData->m_runSpeed;
			return;
		}
	}

	if (m_vehiclePhysicsData)
	{
		delete m_vehiclePhysicsData;
		m_vehiclePhysicsData = 0;
	}
}

//----------------------------------------------------------------------

void CreatureObject::unequipAllItems()
{
	//-- Ensure we're authoritative.
	if (!isAuthoritative())
	{
		WARNING(true,
			("unequipAllItems() called on object id=[%s], template=[%s] on server id=[%d] but this object is not authoritative.",
			getNetworkId().getValueString().c_str(),
			getObjectTemplateName(),
			static_cast<int>(GameServer::getInstance().getProcessId())
			));
		return;
	}

	//-- Get the equipment container for the creature.
	SlottedContainer * const equipmentContainer = ContainerInterface::getSlottedContainer(*this);
	if (!equipmentContainer)
	{
		WARNING(true,
			("unequipAllItems() called on object id=[%s], template=[%s] on server id=[%d]: failed to get equipment slotted container, error code=[%d].",
			getNetworkId().getValueString().c_str(),
			getObjectTemplateName(),
			static_cast<int>(GameServer::getInstance().getProcessId())
			));
		return;
	}

	//-- Get the inventory container for the creature.
	Container::ContainerErrorCode errorCode          = Container::CEC_Success;
	Container::ContainedItem  inventoryContainedItem = equipmentContainer->getObjectInSlot(s_inventorySlotId, errorCode);
	Object *const             inventoryObjectBase    = inventoryContainedItem.getObject();
	ServerObject *const       inventoryObject        = inventoryObjectBase ? inventoryObjectBase->asServerObject() : nullptr;
	if (!inventoryObject || (errorCode != Container::CEC_Success))
	{
		WARNING(true,
			("unequipAllItems() called on object id=[%s], template=[%s] on server id=[%d]: failed to get inventory object from equipment, error code=[%d].",
			getNetworkId().getValueString().c_str(),
			getObjectTemplateName(),
			static_cast<int>(GameServer::getInstance().getProcessId()),
			static_cast<int>(errorCode)
			));
		return;
	}

	VolumeContainer *const inventoryContainer = ContainerInterface::getVolumeContainer(*inventoryObject);
	if (!inventoryContainer)
	{
		WARNING(true,
			("unequipAllItems() called on object id=[%s], template=[%s] on server id=[%d]: inventory object is nullptr.",
			getNetworkId().getValueString().c_str(),
			getObjectTemplateName(),
			static_cast<int>(GameServer::getInstance().getProcessId())
			));
		return;
	}

	//-- Collect items to move into inventory.  This includes items in all slots that players can manipulate.
	typedef std::vector<ServerObject*>  ServerObjectVector;
	ServerObjectVector                  itemsToTransfer;

	{
		ContainerIterator const endIt = equipmentContainer->end();
		for (ContainerIterator it = equipmentContainer->begin(); it != endIt; ++it)
		{
			// Get the equipment item.
			Container::ContainedItem containedItem = *it;
			Object *const       objectBase = containedItem.getObject();
			ServerObject *const object     = objectBase ? objectBase->asServerObject() : nullptr;
			if (!object)
			{
				WARNING(true,
					("nullptr object in equipment container for object id=[%s]: equipment item id=[%s].",
					getNetworkId().getValueString().c_str(),
					containedItem.getValueString().c_str()
					));
				continue;
			}

			// Get the slotted containment property so we can check the slot characteristics.
			SlottedContainmentProperty const *const property = ContainerInterface::getSlottedContainmentProperty(*object);
			if (!property)
			{
				WARNING(true,
					("object in equipment container for object id=[%s], equipment item id=[%s] has no SlottedContainmentProperty.",
					getNetworkId().getValueString().c_str(),
					containedItem.getValueString().c_str()
					));
				continue;
			}

			// Check if any of the current arrangement's slots are not player manipulatable.
			if (property->canManipulateArrangement(property->getCurrentArrangement()))
				itemsToTransfer.push_back(object);
		}
	}

	//-- Transfer selected equipped items to inventory.
	{
		ServerObjectVector::iterator const endIt = itemsToTransfer.end();
		for (ServerObjectVector::iterator it = itemsToTransfer.begin(); it != endIt; ++it)
		{
			ServerObject *const object = *it;
			if (!object)
				continue;

			ContainerInterface::transferItemToVolumeContainer(*inventoryObject, *object, nullptr, errorCode);
			WARNING(errorCode != Container::CEC_Success,
				("unequipAllItems(): CreatureObject id=[%s] failed to transfer item id=[%s], template=[%s] from equipment to inventory container, container error code [%d].",
				getNetworkId().getValueString().c_str(),
				object->getNetworkId().getValueString().c_str(),
				object->getObjectTemplateName(),
				static_cast<int>(errorCode)
				));
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::virtualOnSetClient()
{
	TangibleObject::virtualOnSetClient();

	handleTutorialTransition();

	setNumberOfMissionsWantedInMissionBag(ConfigServerGame::getNumberOfMissionsWantedInMissionBag());
}

//----------------------------------------------------------------------

std::vector<WearableEntry> CreatureObject::getWearableData()
{
	return m_wearableData.get();
}

//----------------------------------------------------------------------

void CreatureObject::setWearableData(std::vector<WearableEntry> const & packedWearables)
{
	m_wearableData.set(packedWearables);
}

//----------------------------------------------------------------------

void CreatureObject::onContainedItemAppearanceDataModified(TangibleObject const & target, std::string const & oldValue, std::string const & value)
{
	TangibleObject::onContainedItemAppearanceDataModified(target, oldValue, value);

	if(oldValue != value)
	{
		std::vector<WearableEntry> wearableData = getWearableData();
		for(std::vector<WearableEntry>::iterator i = wearableData.begin(); i != wearableData.end(); ++i)
		{
			if(target.getNetworkId() == i->m_networkId)
			{
				i->m_appearanceString = value;
				setWearableData(wearableData);
				//there will only be one object with the target's networkid, so we can return
				return;
			}
		}
	}
}

//----------------------------------------------------------------------

void CreatureObject::updateGroup()
{
	GroupObject * group = getGroup();
	if(group)
	{
		const std::vector<std::pair<NetworkId, std::string> > & groupMembers = group->getGroupMembers();

		// if there are members that might need an update
		if(! groupMembers.empty())
		{
			// setup the update status message that is sent to the client
			if(! m_clientMfdStatusUpdateMessage)
			{
				// this is the first time an update might need to have been sent
				m_clientMfdStatusUpdateMessage = new ClientMfdStatusUpdateMessage(getNetworkId(), ServerWorld::getSceneId(), findPosition_w(), ConfigFile::getKeyFloat("ServerGame", "groupLocationUpdateTimer", 1.0f), ConfigFile::getKeyFloat("ServerGame", "groupLocationUpdateMinimumMoveDistance", 64.0f));
			}
			else
			{
				// it's already been constructed, update members
				m_clientMfdStatusUpdateMessage->setSceneName(ServerWorld::getSceneId());
				m_clientMfdStatusUpdateMessage->setWorldCoordinates(findPosition_w());
			}

			// if there are still members to update since the last calculation, select
			// the first one in the list and send the message
			if(! m_clientMfdStatusUpdateMessage->getMembersToUpdate().empty())
			{
				if(m_clientMfdStatusUpdateMessage->updateMemberUpdateTimer(Clock::frameTime()))
				{
					// default, assume that the object is observed and no messageTo
					// is needed
					bool sendMessageTo = false;

					while(! sendMessageTo && (!m_clientMfdStatusUpdateMessage->getMembersToUpdate().empty()))
					{
						static Archive::ByteStream bs;
						bs.clear();
						NetworkId memberId = m_clientMfdStatusUpdateMessage->getMembersToUpdate().back();
						m_clientMfdStatusUpdateMessage->getMembersToUpdate().pop_back();

						if(memberId != getNetworkId())
						{
							// cull bad network id's
							if(memberId != NetworkId::cms_invalid)
							{
								Object * obj = NetworkIdManager::getObjectById(memberId);
								if(obj)
								{
									ServerObject * target = obj->asServerObject();
									if(target && target->getClient())
									{
										// check to see whether the target observing this creature
										// if it is, then the client will be updated automatically
										if(! ObserveTracker::isObserving(*target->getClient(), *this))
										{
											// this creature is not being observed by the target player
											// if it is authoritative on this server, however, send
											// the message directly.
											if(target->isAuthoritative())
											{
												target->getClient()->send(*m_clientMfdStatusUpdateMessage, false);
											}
											else
											{
												// the object is valid, but
												// not authoritative on this
												// server, send a messageTO
												sendMessageTo = true;
											}
										} // else normal game operation will take care of updating the client MFD status window
									}
									else if(target && !target->getClient() && !target->isAuthoritative())
									{
										// target is valid, but probably a proxy of a player
										// controlled object. Send a message to the remote
										// server
										sendMessageTo = true;
									}
									else if(! target)
									{
										WARNING_STRICT_FATAL(true, ("While updating group locations, group member ID %s refers to an object that is not a server object!", memberId.getValueString().c_str()));
									}
								}
								else
								{
									// not on this server, send a message
									sendMessageTo = true;
								}
							} // else memberId is an invalid NetworkId
						} // else memberId is self
						if(sendMessageTo)
						{
							// the group member is not locally authoritative
							// on this server and is not being observed, the only
							// way to get data to the group member is by sending
							// a message to.

							bs.clear();
							m_clientMfdStatusUpdateMessage->pack(bs);
							// send the messageTo to the target object
							MessageToQueue::getInstance().sendMessageToC(memberId, "ClientMfdStatusUpdateMessage", std::string(reinterpret_cast<const char * const>(bs.getBuffer()), bs.getSize()), 0, false);
						}
					}
				}
			}
			// If the message hasn't changed since the last attempt, don't
			// bother sending the update
			else if(m_clientMfdStatusUpdateMessage->getHasChanged())
			{
				// add group members to the mfs status update member id list
				m_clientMfdStatusUpdateMessage->getMembersToUpdate().clear();
				std::vector<std::pair<NetworkId, std::string> >::const_iterator i;
				for(i = groupMembers.begin(); i != groupMembers.end(); ++i)
				{
					if(i->first != NetworkId::cms_invalid && i->first != getNetworkId())
						m_clientMfdStatusUpdateMessage->addMemberToUpdate(i->first);
				}
			}
		}
		else
		{
			// the group is empty, clean up any memory that might have been used before
			delete m_clientMfdStatusUpdateMessage;
			m_clientMfdStatusUpdateMessage = 0;
		}
	}
	else
	{
		// the group is empty, clean up any memory that might have been used before
		delete m_clientMfdStatusUpdateMessage;
		m_clientMfdStatusUpdateMessage = 0;
	}

	if(m_clientMfdStatusUpdateMessage)
	{
		m_clientMfdStatusUpdateMessage->setHasChanged(false);
	}
}

//------------------------------------------------------------------------------------------

void CreatureObject::handleTutorialTransition()
{
	// this is where all the code lives for the special transitions for the tutorial...
	// much of the implementation is implemented in NewbieTutorial and is called through here
	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);

	if (playerObject)
	{
		// check if this player is coming from the character creation / selection screen
		bool isFromLogin = playerObject->getIsFromLogin();
		playerObject->setIsFromLogin(false);
		bool startTutorial = !ConfigServerGame::getSkipTutorial();
		if (startTutorial)
		{
			// let the npe phase determine whether or not to do the tutorial
			startTutorial = (NewbieTutorial::shouldStartTutorial(this));
		}

		const std::string currentScene = ServerWorld::getSceneId();
		if (NewbieTutorial::isInTutorialArea(this))
		{
			LOG("npe", ("handleTutorialTransition called for object %s in scene %s at location %f %f %f\n", getNetworkId().getValueString().c_str(), currentScene.c_str(), getPosition_w().x, getPosition_w().y, getPosition_w().z));

			// check and see if they are loading into a tutorial area. If so, make sure they only have objects in the FileManifest
			if (ConfigServerGame::getStripNonFreeAssetsFromPlayersInTutorial())
			{
				LOG("npe", ("handleTutorialTransition removing any non-free assets from object %s\n", getNetworkId().getValueString().c_str()));
				NewbieTutorial::stripNonFreeAssetsFromPlayerInTutorial(this);
			}
		}

		if (startTutorial && currentScene == NewbieTutorial::getSceneId())
		{
			// if we are in the tutorial scene, force a teleportFixup here so we can determine if we are in a container
			handleTeleportFixup(false);

			ServerObject const * container = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));

			// if they are on the tutorial planet, but not in a valid container, create a tutorial instance for them
			if (!container || !container->isAuthoritative() || isFromLogin)
			{
				// the player should be in the tutorial and is in the right place, go ahead and place them
				NewbieTutorial::setupCharacterForTutorial(this);
				Vector newLocation = getPosition_w();
				if (newLocation == Vector(0,0,0))
				{
					LOG("npe", ("handleTutorialTransition setup tutorial at the origin for object %s\n", getNetworkId().getValueString().c_str()));
				}
				ServerObject *tutorial = NewbieTutorial::createTutorial(newLocation);

				FATAL(!tutorial, ("failed to create tutorial for player which already exists."));
				std::string startCellName = NewbieTutorial::getStartCellName();

				Vector startCoords = NewbieTutorial::getStartCoords();
				LOG("npe", ("handleTutorialTransition created newbie tutorial %s (%s authoritative) for %s (%s authoritative)\n", tutorial->getNetworkId().getValueString().c_str(), (tutorial->isAuthoritative()) ? "is" : "not", getNetworkId().getValueString().c_str(), (isAuthoritative()) ? "is" : "not"));

				teleportObject(newLocation, tutorial->getNetworkId(), startCellName, startCoords, "", true);
			}
		}
		else if (
				(startTutorial && isFromLogin)
			)
		{
			// if the creature object is coming from login, and they should start the tutorial
			// or if the player is on a free trial account, but is on a non-free trial planet or location (NOT GOOD!)
			// warp them to the tutorial planet
			LOG("npe", ("Warping player (%s) back to tutorial because they are coming from login (%i) or because they are on a free trial account (%i) and trying to get to a non-free trial planet (%s: %f %f %f)\n", getNetworkId().getValueString().c_str(), isFromLogin, (getClient() && getClient()->isFreeTrialAccount()), ServerWorld::getSceneId().c_str(), getPosition_w().x, getPosition_w().y, getPosition_w().z));
			NewbieTutorial::setupCharacterForTutorial(this);
			GameServer::getInstance().requestSceneWarp(CachedNetworkId(*this), NewbieTutorial::getSceneId(), NewbieTutorial::getTutorialLocation(), NetworkId::cms_invalid, Vector(0,0,0));
		}
	}
}

//------------------------------------------------------------------------------------------

void CreatureObject::onContainerChildLostItem(ServerObject * destination, ServerObject& item, ServerObject* source, ServerObject* transferer)
{
	ServerObject::onContainerChildLostItem(destination, item, source, transferer);

	GameScriptObject * gameScriptObject = getScriptObject();
	if(gameScriptObject)
	{
		NetworkId itemId = item.getNetworkId();
		NetworkId sourceId;
		if(source)
			sourceId = source->getNetworkId();
		NetworkId transfererId;
		if(transferer)
			transfererId = transferer->getNetworkId();
		NetworkId destinationId;
		if(destination)
			destinationId = destination->getNetworkId();

		ScriptParams params;
		params.addParam (itemId);
		params.addParam (sourceId);
		params.addParam (transfererId);
		params.addParam (destinationId);
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_CONTAINER_CHILD_LOST_ITEM, params));
	}

	if(isPlayerControlled()) // Source == Appearance Inventory
	{
		//Update wearble appearance data
		SlottedContainmentProperty* scp = ContainerInterface::getSlottedContainmentProperty(item);
		if (scp)
		{
			WearableEntry e("", scp->getCurrentArrangement(), item.getNetworkId(), 0);
			int index = m_wearableAppearanceData.find(e);
			if (index >= 0)
			{
				m_wearableAppearanceData.erase(index);
			}
		}
	}

}

//------------------------------------------------------------------------------------------

void CreatureObject::onContainerChildGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer)
{
	ServerObject::onContainerChildGainItem(item, source, transferer);

	GameScriptObject * gameScriptObject = getScriptObject();
	if(gameScriptObject)
	{
		NetworkId itemId = item.getNetworkId();
		NetworkId sourceId;
		if(source)
			sourceId = source->getNetworkId();
		NetworkId transfererId;
		if(transferer)
			transfererId = transferer->getNetworkId();

		ScriptParams params;
		params.addParam (itemId);
		params.addParam (sourceId);
		params.addParam (transfererId);

		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_CONTAINER_CHILD_GAIN_ITEM, params));
	}

	if(isPlayerControlled())
	{
		TangibleObject* tangibleObject = item.asTangibleObject();

		if(!tangibleObject)
			return;

		//Update wearables data
		SlottedContainmentProperty* scp = ContainerInterface::getSlottedContainmentProperty(item);
		if (scp)
		{
			Object const * appearance = 0;

			SlottedContainer const * const myContainer = ContainerInterface::getSlottedContainer(*this);

			if (myContainer)
			{
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				SlotId appearanceInvSlot = SlotIdManager::findSlotId(CreatureObjectNamespace::SlotNames::appearance);
				if (appearanceInvSlot != SlotId::invalid)
				{
					Container::ContainedItem itemId = myContainer->getObjectInSlot(appearanceInvSlot, tmp);
					appearance = itemId.getObject();
				}
			}

			Object const * container = ContainerInterface::getContainedByObject(*tangibleObject);
			bool const inAppearanceInv = (container == appearance);
			if (scp->getCurrentArrangement() >= 0 && scp->isInAppearanceSlot() && inAppearanceInv)
			{
				ConstCharCrcString clientSharedTemplateNameCrcString = ObjectTemplateList::lookUp(tangibleObject->getClientSharedTemplateName());
				if (clientSharedTemplateNameCrcString.isEmpty())
					WARNING(true, ("Could not find crc for %s.  Wearable will not be streamed to client", tangibleObject->getClientSharedTemplateName()));
				else
					addPackedAppearanceWearable(tangibleObject->getAppearanceData(), scp->getCurrentArrangement(), tangibleObject->getNetworkId(), clientSharedTemplateNameCrcString.getCrc());
			}
		}
	}
	
}

//------------------------------------------------------------------------------------------

void CreatureObject::forceNotifyRegionsCheck()
{
	m_lastRegionNotifyPosition.x += 1.0f; // force a check
}

//------------------------------------------------------------------------------------------

void CreatureObject::checkNotifyRegions()
{
	// if the object is player controlled, check for a change of regions that have their
	// notify field set to true.
	static const bool allowChecks = ConfigServerGame::getCheckNotifyRegions();
	if(! allowChecks)
		return;

	if(! isPlayerControlled() || !isAuthoritative())
		return;

	if (! isInWorld())
		return;
	bool forceCheckRegions = m_regionFlagTimer.isExpired();

	if(m_lastRegionNotifyPosition == getPosition_w() && !forceCheckRegions)
		return;

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
	if(! playerObject)
		return;

	PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::checkNotifyRegions()");
	// region containers populated by region master getRegionsAtPoint()

	static std::vector<const Region *> newRegions;
	newRegions.clear();
	RegionMaster::getRegionsAtPoint(ServerWorld::getSceneId(), getPosition_w().x, getPosition_w().z, newRegions);

	if(forceCheckRegions)
	{
		m_regionFlagTimer.reset();

		std::vector<const Region *>::iterator regionIter = newRegions.begin();
		EnvironmentInfo flags = static_cast<EnvironmentInfo>(0);
		for(; regionIter != newRegions.end(); ++regionIter)
		{
			if((*regionIter)->getNotify())
				flags = flags | (*regionIter)->getEnvironmentFlags();
		}

		playerObject->setEnvironmentFlags(flags);

		if(m_lastRegionNotifyPosition == getPosition_w())
			return;
	}

	static std::vector<std::string> newNotifyRegions;
	newNotifyRegions.clear();

	static std::vector<std::string> newNotifyRegionsEntered;
	newNotifyRegionsEntered.clear();

	static std::vector<std::pair<std::string, std::string> > notifyRegionsExited;
	notifyRegionsExited.clear();

	// look for region enter events
	std::vector<const Region *>::const_iterator i;

	bool teleport = false;
	static const bool allowOverride = ConfigServerGame::getAllowRegionTriggerOverride();

	// iterate through the new regions looking for a "notify" region that is not in the
	// list of regions found during the last check.
	for(i = newRegions.begin(); i != newRegions.end(); ++i)
	{
		if((*i)->getNotify())
		{
			// this is a region that requires a notification be sent
			// to scripts on the player's creature object
			std::string regionName = Unicode::wideToNarrow((*i)->getName());
			newNotifyRegions.push_back(regionName);

			// check the notify regions from the last pass
			// to see if this is a new region
			if (!m_notifyRegions.contains(std::make_pair(ServerWorld::getSceneId(), regionName)))
			{
				// the region was not in the list, this is a new region
				// and TRIG_ENTER_REGION should be generated
				ScriptParams params;
				params.addParam(ServerWorld::getSceneId().c_str());
				params.addParam(regionName.c_str());
				if(getScriptObject())
				{
					// allow scripts to prevent entry into the region if they
					// return SCRIPT_OVERRIDE
					if (getScriptObject()->trigAllScripts(Scripting::TRIG_ENTER_REGION, params) == SCRIPT_OVERRIDE)
					{
						// don't disallow admins, don't disallow if the server
						// config has it disabled
						if((!getClient() || !getClient()->isGod()) && allowOverride)
						{
							teleport = true;

							Vector teleportTarget = getPosition_w();
							Vector regionCenter = (*i)->getCenter();
							
							// if, somehow, a player starts in a forbidden region, allow them to walk out
							if(teleportTarget.magnitudeXZBetweenSquared(regionCenter) < m_lastRegionNotifyPosition.magnitudeXZBetweenSquared(regionCenter))
							{
								// the current position is further into the region,
								// push the player back
								teleportObject(m_lastRegionNotifyPosition, NetworkId::cms_invalid, "", Vector(), "");
							}
							else
							{
								// the current position is closer to the region edge,
								// allow the player to move normally in this direction
								m_lastRegionNotifyPosition = teleportTarget;
							}

							// if the player is on a mount/vehicle but is
							// not the driver of the mount/vehicle, then
							// eject the (passenger) player so he doesn't
							// keep trying to enter the region because the
							// mount/vehicle will keep taking him into the
							// region as long as he remains mounted
							CreatureObject const * const mountedCreature = getMountedCreature();
							if (mountedCreature && (this != mountedCreature->getPrimaryMountingRider()))
							{
								static uint32 const commandHash = Crc::normalizeAndCalculate("dismount");
								const Command &command = CommandTable::getCommand(static_cast<uint32>(commandHash));
								if (!command.isNull())
								{
									commandQueueEnqueue(command, mountedCreature->getNetworkId(), mountedCreature->getObjectName(), 0, false, static_cast<Command::Priority>(Command::CP_Front), true);
								}
							}
						}
					}
				}

				// if a teleport didn't happen,
				// keep this region on the list
				// of notify regions the client
				// just entered
				if(! teleport)
					newNotifyRegionsEntered.push_back(regionName);
			}
		}
	}

	// look for for region exit events. If a region from the last pass
	// is not in the new region list, the player has left the region
	Archive::AutoDeltaSet<std::pair<std::string, std::string> >::const_iterator i2;
	for(i2 = m_notifyRegions.begin(); i2 != m_notifyRegions.end(); ++i2)
	{
		if((i2->first != ServerWorld::getSceneId()) || (std::find(newNotifyRegions.begin(), newNotifyRegions.end(), i2->second) == newNotifyRegions.end()))
		{
			// the player is exiting the region, send a TRIG_EXIT_REGION
			// to all scripts attached to the player's creature object
			ScriptParams params;
			params.addParam(i2->first.c_str());
			params.addParam(i2->second.c_str());
			if(getScriptObject())
			{
				if(getScriptObject()->trigAllScripts(Scripting::TRIG_EXIT_REGION, params) == SCRIPT_OVERRIDE)
				{
					// don't override movement of god characters, or if the
					// server config disables move overrides
					if((!getClient() || !getClient()->isGod()) && allowOverride && (i2->first == ServerWorld::getSceneId()))
					{
						teleport = true;

						Vector teleportTarget = getPosition_w();

						// if the current position is outside the region, fix the
						// teleport target to prevent the player from exiting
						Region * r = RegionMaster::getRegionByName(i2->first, Unicode::narrowToWide(i2->second));
						if (r)
						{
							if(! r->getBounds().isPointIn(teleportTarget.x, teleportTarget.z))
							{
								// find a point along the path that is in the retion
								Vector heading = r->getCenter() - teleportTarget;
								heading.normalize();

								int tries = 0;
								while(! r->getBounds().isPointIn(teleportTarget.x, teleportTarget.z) && tries < 300)
								{
									++tries;
									teleportTarget += heading;
								}
								teleportObject(teleportTarget, NetworkId::cms_invalid, "", Vector(), "");
							}
						}
						m_lastRegionNotifyPosition = teleportTarget;
					}
				}
				else
				{
					notifyRegionsExited.push_back(std::make_pair(i2->first, i2->second));
				}
			}
		}
	}

	if(!teleport)
	{
		m_lastRegionNotifyPosition = getPosition_w();
	}

	std::vector<std::pair<std::string, std::string> >::const_iterator i3;
	for(i3 = notifyRegionsExited.begin(); i3 != notifyRegionsExited.end(); ++i3)
	{
		m_notifyRegions.erase(*i3);
	}

	std::vector<std::string>::const_iterator i4;
	for(i4 = newNotifyRegionsEntered.begin(); i4 != newNotifyRegionsEntered.end(); ++i4)
	{
		m_notifyRegions.insert(std::make_pair(ServerWorld::getSceneId(), *i4));
	}
}

// ----------------------------------------------------------------------

bool CreatureObject::hasCertificationsForItem(const TangibleObject & item) const
{
	std::vector<std::string> requiredCerts;
	item.getRequiredCertifications(requiredCerts);

	for (std::vector<std::string>::const_iterator i=requiredCerts.begin(); i!=requiredCerts.end(); ++i)
	{
		if (!hasCommand(*i))
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------

CreatureObject * CreatureObject::getCreatureObject(NetworkId const & networkId)
{
	return asCreatureObject(NetworkIdManager::getObjectById(networkId));
}

// ----------------------------------------------------------------------

CreatureObject const * CreatureObject::asCreatureObject(Object const * object)
{
	ServerObject const * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;

	return (serverObject != nullptr) ? serverObject->asCreatureObject() : nullptr;
}

// ----------------------------------------------------------------------

CreatureObject * CreatureObject::asCreatureObject(Object * object)
{
	ServerObject * serverObject = (object != nullptr) ? object->asServerObject() : nullptr;

	return (serverObject != nullptr) ? serverObject->asCreatureObject() : nullptr;
}

// ----------------------------------------------------------------------

void CreatureObject::addMissionCriticalObject(NetworkId const & networkId)
{
	if (isAuthoritative())
	{
		if (m_missionCriticalObjectSet.find(networkId) == m_missionCriticalObjectSet.end())
		{
			m_missionCriticalObjectSet.insert(networkId);
			GroupMissionCriticalObjectsBuilder::updateGroupMissionCriticalObjects(*this, true);
			ServerObject * const serverObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(networkId));
			if (serverObject)
				ObserveTracker::onMissionCriticalObjectAdded(*this, *serverObject);
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::removeMissionCriticalObject(NetworkId const & networkId)
{
	if (isAuthoritative())
	{
		MissionCriticalObjectSet::const_iterator iter = m_missionCriticalObjectSet.find(networkId);
		if (iter != m_missionCriticalObjectSet.end())
		{
			m_missionCriticalObjectSet.erase(iter);
			GroupMissionCriticalObjectsBuilder::updateGroupMissionCriticalObjects(*this, true);
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::clearMissionCriticalObjects()
{
	if (isAuthoritative())
	{
		m_missionCriticalObjectSet.clear();
		GroupMissionCriticalObjectsBuilder::updateGroupMissionCriticalObjects(*this, true);
	}
}

// ----------------------------------------------------------------------

bool CreatureObject::isMissionCriticalObject(NetworkId const & networkId) const
{
	if (isAuthoritative())
		return m_missionCriticalObjectSet.find(networkId) != m_missionCriticalObjectSet.end();

	return false;
}

// ----------------------------------------------------------------------

CreatureObject::MissionCriticalObjectSet const & CreatureObject::getMissionCriticalObjects() const
{
	return m_missionCriticalObjectSet.get();
}

// ----------------------------------------------------------------------

void CreatureObject::setGroupMissionCriticalObjects(GroupMissionCriticalObjectSet const & groupMissionCriticalObjectSet)
{
	DEBUG_FATAL(!isAuthoritative(), ("CreatureObject::setGroupMissionCriticalObjects: called on non-authoritative object %s", getDebugInformation(true).c_str()));
	if (isAuthoritative())
	{
	    m_groupMissionCriticalObjectSet.clear();
		//-- Anything that is not in our set, we need to add
		{
			for (GroupMissionCriticalObjectSet::const_iterator iter = groupMissionCriticalObjectSet.begin(); iter != groupMissionCriticalObjectSet.end(); ++iter)
				m_groupMissionCriticalObjectSet.insert(*iter);
		}
	}
}

// ----------------------------------------------------------------------

bool CreatureObject::getCoverVisibility() const
{
	return m_coverVisibility.get();
}

// ----------------------------------------------------------------------

void CreatureObject::setCoverVisibility(bool isVisible)
{
	if (isAuthoritative())
	{
		m_coverVisibility.set(isVisible);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setCoverVisibility, new MessageQueueGenericValueType<bool>(isVisible));
	}
}

// ----------------------------------------------------------------------

/**
 * Called when an attack pushes us away from the attacker. This gets called
 * due to a message from the client (which sucks, btw).
 *
 * @param attackerId	the attacker
 * @param attackerPos	where the client thought the attacker was
 * @param defenderPos	where the client thought the defender (us) was
 * @param distance		the distance we are pushed away from the attacker
 */
void CreatureObject::pushedMe(const NetworkId & attackerId,
	const Vector & attackerPos, const Vector & defenderPos, float distance)
{
	if (isAuthoritative())
	{
		const ServerObject * attacker = safe_cast<const ServerObject *>(NetworkIdManager::getObjectById(attackerId));

		//@todo: add a whole bunch of security checks here
		// for now, push us away from the defender, using the server positions

		// do a bare minimum sanity check
		static const float MAX_DISTANCE = 20.0f;
		if (distance > MAX_DISTANCE)
		{
			WARNING(true, ("CreatureObject::pushedMe told defender %s was pushed by %s %f meters (max = %f)",
				getNetworkId().getValueString().c_str(), attackerId.getValueString().c_str(), distance, MAX_DISTANCE));
			return;
		}


		UNREF(defenderPos);

		Vector offset(getPosition_w());
		if (attacker != nullptr)
			offset -= attacker->getPosition_w();
		else
			offset -= attackerPos;
		offset.normalize();
		offset *= distance;

		// this is a hack due to the controller move command needing to know the destination cell
		// @todo: change creature movement to always use the CellProperty notification system for moving across cell boundaries
		const Vector oldPos(getPosition_p());

		// note: unlike the client, this call will not cause us to change cells
		move_p(rotate_w2p(offset));

		AICreatureController *controller = dynamic_cast<AICreatureController *>(getController());
		if (controller != nullptr)
		{
			// we only need to do this for npcs, because we'll use the update from a player's client for players
			// we need to do this for npcs to prevent the ai from moving them back to their previous position
			const Vector newPos(getPosition_p());
			float closestPortalT = 0.0f;
			const CellProperty * destinationCell = getParentCell()->getDestinationCell(oldPos, newPos, closestPortalT);
			if (destinationCell == nullptr || destinationCell == getParentCell())
			{
				// no cell change
				controller->warpTo(getParentCell(), newPos);
			}
			else
			{
				Vector warpPos(destinationCell->getOwner().rotateTranslate_w2p(rotateTranslate_p2w(newPos)));
				controller->warpTo(destinationCell, warpPos);
			}
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_pushCreature,
			new MessageQueuePushCreature(attackerId, getNetworkId(), attackerPos, defenderPos, distance));
	}
}

// ----------------------------------------------------------------------

/**
 * Creates a "slow down" effect between this creature and a defender. The effect acts as if
 * there was a hill between the combatants, with the attacker at the top and the defender
 * at the bottom. The area of the effect is a cone between the attacker and defender.
 * The "hill" will be maintained between the combatants even if they move.
 *
 * @param defender		the target that defines the direction of the "hill"
 * @param coneLength	the length of the effect cone
 * @param coneAngle		the angle of the effect cone, in radians
 * @param slopeAngle	the angle of the "hill", in radians
 * @param expireTime	the game time when the effect expires
 */
void CreatureObject::addSlowDownEffect(const TangibleObject & defender, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime)
{
	if (isAuthoritative())
	{
		if (addSlowDownEffectProxy(defender, coneLength, coneAngle, slopeAngle, expireTime))
		{
			// tell everyone who can see us that we are doing the effect
			Controller * const controller = getController();
			if (controller)
			{
				controller->appendMessage(CM_slowDownEffect,
					0,
					new MessageQueueSlowDownEffect(defender.getNetworkId(), coneLength, coneAngle, slopeAngle, expireTime),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_PROXY_SERVER |
					GameControllerMessageFlags::DEST_ALL_CLIENT);
			}
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_slowDownEffect,
			new MessageQueueSlowDownEffect(defender.getNetworkId(), coneLength, coneAngle, slopeAngle, expireTime));
	}
}

// ----------------------------------------------------------------------

/**
 * Creates a "slow down" effect between this creature and a defender. The effect acts as if
 * there was a hill between the combatants, with the attacker at the top and the defender
 * at the bottom. The area of the effect is a cone between the attacker and defender.
 * The "hill" will be maintained between the combatants even if they move.
 *
 * @param defender		the target that defines the direction of the "hill"
 * @param coneLength	the length of the effect cone
 * @param coneAngle		the angle of the effect cone, in radians
 * @param slopeAngle	the angle of the "hill", in radians
 * @param expireTime	the game time when the effect expires
 *
 * @return true if the effect was added, false if the creature already had a slow down effect
 */
bool CreatureObject::addSlowDownEffectProxy(const TangibleObject & defender, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime)
{
	// if we already are doing a slowdown, don't do another
	Property * property = getProperty(SlowDownProperty::getClassPropertyId());
	if (property != nullptr)
		return false;

	property = new SlowDownProperty(*this, CachedNetworkId(defender), coneLength, coneAngle, slopeAngle, expireTime);
	addProperty(*property, true);
	return true;
}

// ----------------------------------------------------------------------

/**
 * Causes any slow down effect being used by us to be canceled early.
 */
void CreatureObject::removeSlowDownEffect()
{
	if (isAuthoritative())
	{
		removeSlowDownEffectProxy();

		// tell all my proxies (client and server) to remove the effect
		Controller * const controller = getController();
		if (controller)
			controller->appendMessage(CM_removeSlowDownEffectProxy, 0, nullptr, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_ALL_CLIENT | GameControllerMessageFlags::DEST_PROXY_SERVER);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeSlowDownEffect, nullptr);
	}
}

// ----------------------------------------------------------------------

/**
 * Causes any slow down effect being used by us to be canceled early.
 */
void CreatureObject::removeSlowDownEffectProxy()
{
	if (getProperty(SlowDownProperty::getClassPropertyId()) != nullptr)
		removeProperty(SlowDownProperty::getClassPropertyId());
}

// ----------------------------------------------------------------------

/**
 * Adds an additional terrain slope effect that will be taken into account the
 * next time we alter this creature. Note that this function only affects non-player
 * creatures.
 *
 * @param normal		the normal vector of the "terrain" that is affecting us
 */
void CreatureObject::addTerrainSlopeEffect(const Vector & normal)
{
	if (isAuthoritative() && !isPlayerControlled())
	{
		Property * property = getProperty(SlopeEffectProperty::getClassPropertyId());
		if (property == nullptr)
		{
			property = new SlopeEffectProperty(*this);
			addProperty(*property, true);
		}
		safe_cast<SlopeEffectProperty*>(property)->addNormal(normal);
	}
}

// ----------------------------------------------------------------------

char const * CreatureObject::getMentalStateString(MentalStates::Enumerator const mentalState)
{
	switch (mentalState)
	{
		case MentalStates::Fear: { return "FEAR"; }
		case MentalStates::Anger: { return "ANGER"; }
		case MentalStates::Interest: { return "INTEREST"; }
		case MentalStates::Distress: { return "DISTRESS"; }
		default: {}
	}

	return "ERROR";
}

// ----------------------------------------------------------------------

char const * CreatureObject::getBehaviorString(Behaviors::Enumerator const behavior)
{
	switch (behavior)
	{
		case Behaviors::Calm: { return "CALM"; }
		case Behaviors::Alert: { return "ALERT"; }
		case Behaviors::Threaten: { return "THREATEN"; }
		case Behaviors::Retreat: { return "RETREAT"; }
		case Behaviors::Flee: { return "FLEE"; }
		case Behaviors::Attack: { return "ATTACK"; }
		case Behaviors::Frenzy: { return "FRENZY"; }
		default: {}
	}

	return "ERROR";
}

// ----------------------------------------------------------------------

void CreatureObject::alterPseudoPlayedTime(float time)
{
	if (isAuthoritative())
	{
		m_pseudoPlayedTime += time;
	}
	else
	{
		WARNING(true, ("CreatureObject::alterPlayedTime called on non-authoritative creature"));
	}
}	// updatePlayedTime

// ----------------------------------------------------------------------

void CreatureObject::setPseudoPlayedTime(float playedTime)
{
	m_pseudoPlayedTime = playedTime;
}

// ----------------------------------------------------------------------

void CreatureObject::addBuff(uint32 buffNameCrc, float duration, float dynamicValue, NetworkId caster, uint32 stackCount)
{
	addBuff( buffNameCrc, duration, dynamicValue, duration, caster, stackCount);
}

void CreatureObject::addBuff(uint32 buffNameCrc, float timeTillRemoval, float dynamicValue, float totalDuration, NetworkId caster, uint32 stackCount)
{
	if (!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_addBuff, new MessageQueueGenericValueType<std::pair<int, std::pair<float, std::pair<float, std::pair< float, NetworkId > > > > >(std::make_pair(static_cast<int>(buffNameCrc), std::make_pair(timeTillRemoval, std::make_pair(dynamicValue, std::make_pair(totalDuration, caster))))));
		return;
	}
	uint32 timestamp = internalGetPlayedTime();

	if (BuffManager::getBuffIsCelestial(buffNameCrc))
	{
		timestamp = ServerClock::getInstance().getGameTimeSeconds();
	}

	int newStackCount = stackCount;

	if(timeTillRemoval < 0.0f)
		timestamp = 0xFFFFFFFF;
	else if (timeTillRemoval == 0.0f)
	{
		float defaultDuration = BuffManager::getBuffDefaultDuration(buffNameCrc);
		if(defaultDuration < 0.0f)
			timestamp = 0xFFFFFFFF;
		else
		{
			timestamp += static_cast<uint32>(defaultDuration);
			totalDuration = defaultDuration;
		}
	}
	else
	{
		timestamp += static_cast<uint32>(timeTillRemoval);
	}
	bool reapply = false;
	if(hasBuff(buffNameCrc))
	{
		Buff b;
		getBuff(buffNameCrc, b);
		
		newStackCount += b.m_stackCount;

		if((timestamp != 0xFFFFFFFF) && (b.m_timestamp > timestamp))
		{
			//We don't need to do anything; the buff we're adding has less time than the one we have
			return;
		}
		std::map<uint32, Buff::PackedBuff>::const_iterator i = m_buffs.find(buffNameCrc);
		if (i != m_buffs.end())
		{
			m_buffs.erase(i);
		}
		m_persistedBuffs.erase(buffNameCrc);
		reapply = true;
	}

	newStackCount = std::max(0, std::min(newStackCount, static_cast<int>(BuffManager::getBuffMaxStacks(buffNameCrc))));

	Buff::PackedBuff packedValue = Buff(buffNameCrc, timestamp, dynamicValue, static_cast<uint32>(totalDuration), caster, newStackCount).getPackedBuffValue();

	m_buffs.insert(buffNameCrc, packedValue);
	if (BuffManager::getIsBuffPersisted(buffNameCrc))
	{
		m_persistedBuffs.insert(buffNameCrc, packedValue);
	}
	if(reapply)
		BuffManager::reapplyBuffEffects(this, buffNameCrc, timeTillRemoval, dynamicValue, newStackCount, caster);
	else
		BuffManager::applyBuffEffects(this, buffNameCrc, timeTillRemoval, dynamicValue, newStackCount, caster);
	//If there was a state, add that state to this creature
	int buffState = BuffManager::getBuffState(buffNameCrc);
	if(buffState != -1)
		setState(static_cast<States::Enumerator>(buffState), true);
}

// ----------------------------------------------------------------------

void CreatureObject::removeBuff(uint32 buffNameCrc)
{
	if (!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_removeBuff, new MessageQueueGenericValueType<int>(static_cast<int>(buffNameCrc)));
		return;
	}

	std::map<uint32, Buff::PackedBuff>::const_iterator i = m_buffs.find(buffNameCrc);
	if (i != m_buffs.end())
	{
		NetworkId caster = (*i).second.caster;
		m_buffs.erase(i);
		int buffState = BuffManager::getBuffState(buffNameCrc);
		BuffManager::removeBuffEffects(this, buffNameCrc, caster);
		if (!stillHasBuffState(buffNameCrc, buffState))
			setState(static_cast<States::Enumerator>(buffState), false);
		std::string const &callback = BuffManager::getBuffCallback(buffNameCrc);
		ScriptParams params;
		params.addParam(static_cast<int>(buffNameCrc), "buffCrc");
		ScriptDictionaryPtr dictionary;
		getScriptObject()->makeScriptDictionary(params, dictionary);
		dictionary->serialize();
		getScriptObject()->handleMessage(callback, dictionary);
	}

	// clean up any persistance
	m_persistedBuffs.erase(buffNameCrc);
}

// ----------------------------------------------------------------------

void CreatureObject::decrementBuff(uint32 const buffNameCrc, uint32 const stacksToRemove)
{
	std::map<const uint32, Buff::PackedBuff>::const_iterator i = m_buffs.find(buffNameCrc);
	if (i != m_buffs.end())
	{
		
		Buff::PackedBuff b = (*i).second;

		b.stackCount -= stacksToRemove;

		removeBuff(buffNameCrc);

		if (static_cast<int32>(b.stackCount) > 0)
		{
			uint32 currentTime = internalGetPlayedTime();

			if (BuffManager::getBuffIsCelestial(buffNameCrc))
			{
				currentTime = ServerClock::getInstance().getGameTimeSeconds();
			}

			float endTime = static_cast<float> (b.endtime) - static_cast<float> (currentTime); 
			addBuff( buffNameCrc, endTime , b.value, static_cast<float> (b.duration), b.caster, b.stackCount);	
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::decayBuff(uint32 buffNameCrc, float decayPercentage)
{
	std::map<const uint32, Buff::PackedBuff>::const_iterator i = m_buffs.find(buffNameCrc);
	if (i != m_buffs.end())
	{
		Buff::PackedBuff b = (*i).second;

		float decayValue = b.duration * decayPercentage; // decayPercentage should be a value between 0.0 and 1.0
		
		removeBuff(buffNameCrc);
		
		uint32 currentTime = internalGetPlayedTime();

		float endTime = (static_cast<float> (b.endtime) - decayValue) - static_cast<float> (currentTime);
		
		if(endTime > 0.0f)
			addBuff( buffNameCrc, endTime , b.value, static_cast<float> (b.duration), b.caster, b.stackCount);	
	}
}

uint32 CreatureObject::internalGetPlayedTime() const
{
	if (isPlayerControlled())
	{
		PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject)
		{
			return static_cast<uint32>(playerObject->getPlayedTime());
		}
	}
	return static_cast<uint32>(ServerClock::getInstance().getGameTimeSeconds());
}

// ----------------------------------------------------------------------

// This functions expires buffs as needed
void CreatureObject::updateBuffs()
{
	uint32 timestamp = internalGetPlayedTime();
	std::list< Buff > buffsToRemove;

	for (std::map<uint32, Buff::PackedBuff>::const_iterator i = m_buffs.begin(); i != m_buffs.end(); )
	{
		const std::pair<uint32, Buff::PackedBuff> & p = *i++;
		Buff b(p.first, p.second);

		if (BuffManager::getBuffIsCelestial(b.m_nameCrc))
		{
			timestamp = ServerClock::getInstance().getGameTimeSeconds();
		}
		else
		{
			timestamp = internalGetPlayedTime();
		}

		if (b.m_timestamp < timestamp)
		{
			buffsToRemove.push_back(b);
		}
	}
	
	std::list< Buff >::iterator iter = buffsToRemove.begin();
	for( ; iter != buffsToRemove.end(); ++iter)
	{
		removeBuff((*iter).m_nameCrc);
	}
	
}

// ----------------------------------------------------------------------

bool CreatureObject::stillHasBuffState(uint32 buffNameCrc, int state) const
{
	for (std::map<uint32, Buff::PackedBuff>::const_iterator i = m_buffs.begin(); i != m_buffs.end(); ++i)
	{
		const std::pair<uint32, Buff::PackedBuff> & p = *i;
		if ((p.first != buffNameCrc) && (BuffManager::getBuffState(p.first) == state))
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------

void CreatureObject::getAllBuffs(std::vector<uint32> & buffCrcs) const
{
	buffCrcs.clear();
	buffCrcs.reserve(m_buffs.size());
	for (std::map<uint32, Buff::PackedBuff>::const_iterator i = m_buffs.begin(); i != m_buffs.end(); ++i)
	{
		const std::pair<uint32, Buff::PackedBuff> & p = *i;
		buffCrcs.push_back(p.first);
	}
}

// ----------------------------------------------------------------------

bool CreatureObject::getBuff(uint32 buffNameCrc, Buff & buff) const
{
	std::map<uint32, Buff::PackedBuff>::const_iterator found = m_buffs.find(buffNameCrc);
	if (found == m_buffs.end())
		return false;

	buff.set((*found).first, (*found).second);
	return true;
}

// ----------------------------------------------------------------------

bool CreatureObject::hasBuff(uint32 buffNameCrc) const
{
	std::map<uint32, Buff::PackedBuff>::const_iterator found = m_buffs.find(buffNameCrc);
	return (found != m_buffs.end());
}

// ----------------------------------------------------------------------

bool CreatureObject::isInTutorial() const
{
	return NewbieTutorial::isInTutorial(this);
}

// ----------------------------------------------------------------------

bool CreatureObject::isJedi() const
{
	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(this);
	if (playerObject)
		return playerObject->isJedi();
	else
		return false;
}

// ----------------------------------------------------------------------

void CreatureObject::setLevel(int level)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_setLevel,
				0.0f,
				new MessageQueueGenericValueType<int>(level),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject == nullptr)
		{
			// this is an ai, so just set the level
			m_level = (int16) level;
		}
		else
		{
			LevelManager::LevelData levelData;
			LevelManager::setLevelDataFromLevel(levelData, level);

			setLevelData(levelData.currentLevel, levelData.currentLevelXp, levelData.currentHealth);
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::recalculateLevel()
{
	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
	if (playerObject != nullptr)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_recalculateLevel,
					0.0f,
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else
		{
			LevelManager::LevelData levelData;
			LevelManager::calculateLevelData(getLevelXp(), levelData);

			setLevelData(levelData.currentLevel, levelData.currentLevelXp, levelData.currentHealth);
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::setLevelData(int16 level, int levelXp, int health)
{
	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
	if (playerObject != nullptr)
	{
		m_totalLevelXp = levelXp;

		m_levelHealthGranted = health;

		m_level = level;

		GroupObject *group = getGroup();
		if (group != 0)
		{
			group->setMemberLevel(getNetworkId(), level);
		}

		std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = connectedCharacterLfgData.find(getNetworkId());
		if ((iterFind != connectedCharacterLfgData.end()) && (iterFind->second.level != static_cast<int16>(level)))
			ServerUniverse::setConnectedCharacterLevelData(getNetworkId(), static_cast<int16>(level));

		// update and sync
		computeTotalAttributes();
	}
}

// ----------------------------------------------------------------------

void CreatureObject::levelChanged() const
{
	return;
}

// ----------------------------------------------------------------------

void CreatureObject::setClientUsesAnimationLocomotion(bool const enabled)
{
	m_clientUsesAnimationLocomotion = enabled;
}

// ----------------------------------------------------------------------

void CreatureObject::fixupPersistentBuffsAfterLoading()
{
	// initialize persisted buffs
	if(m_fixedupPersistentBuffsAfterLoading)
		return;  // don't want to do it twice
	m_fixedupPersistentBuffsAfterLoading = true;
	DEBUG_WARNING(isPlayerControlled() && PlayerCreatureController::getPlayerObject(this) && !PlayerCreatureController::getPlayerObject(this)->isInitialized(),
		("CreatureObject::fixupPersistentBuffsAfterLoading trying to use an uninitialized player object"));
	uint32 currentTime = internalGetPlayedTime();
	std::vector<Buff> tempPersistedHolder;
	for (std::map<uint32, Buff::PackedBuff>::const_iterator ip = m_persistedBuffs.begin(); ip != m_persistedBuffs.end(); ++ip)
	{
		Buff b((*ip).first, (*ip).second);
		tempPersistedHolder.push_back(b);
	}
	m_persistedBuffs.clear();
	for (std::vector<Buff>::const_iterator i = tempPersistedHolder.begin(); i != tempPersistedHolder.end(); ++i)
	{
		const Buff & b = (*i);

		if (BuffManager::getBuffIsCelestial(b.m_nameCrc))
		{
			currentTime = ServerClock::getInstance().getGameTimeSeconds();
		}
		else
			currentTime = internalGetPlayedTime();

		if (b.m_timestamp > currentTime || b.m_timestamp == 0xFFFFFFFF)
		{
			float timeTillRemoval = -1.0f;
			if (b.m_timestamp != 0xFFFFFFFF)
				timeTillRemoval = static_cast<float>(b.m_timestamp - currentTime);
						

			addBuff(b.m_nameCrc, timeTillRemoval, b.m_value, static_cast<float>(b.m_duration), b.m_caster, b.m_stackCount);
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::fixupLevelXpAfterLoading()
{
	// initialize persisted buffs
	if(m_fixedupLevelXpAfterLoading)
		return;  // don't want to do it twice

	m_fixedupLevelXpAfterLoading = true;

	DEBUG_FATAL(!isPlayerControlled(),
		("CreatureObject::fixupLevelXpAfterLoading trying to be called for a non-player controlled object"));

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);

	DEBUG_FATAL(!playerObject,
		("CreatureObject::fixupLevelXpAfterLoading trying to use a non-existent player object"));

	DEBUG_FATAL(playerObject && !playerObject->isInitialized(),
		("CreatureObject::fixupLevelXpAfterLoading trying to use an uninitialized player object"));

	// The level XP and level are not persisted in the database, so when we
	// are done loading, we will calculate the current level XP and level
	if (playerObject && playerObject->isInitialized())
	{
		LevelManager::LevelData levelData;

		// Level XP due to experience points
		std::map<std::string, int> const & experienceMap = playerObject->getExperiencePoints();
		std::map<std::string, int>::const_iterator i;
		for (i = experienceMap.begin(); i != experienceMap.end(); ++i)
		{
			LevelManager::addXpToLevelData(levelData, i->first, i->second);
		}

		// Level XP due to skills
		for (SkillList::const_iterator skillIter = m_skills.begin(); skillIter != m_skills.end(); ++skillIter)
		{
			SkillObject const * skill = *skillIter;
			if (skill)
			{
				LevelManager::addSkillToLevelData(levelData, skill->getSkillName());
			}
		}

		// Update the skill info directly (we do not want scripts triggered)
		m_level              = levelData.currentLevel;
		m_previousLevel      = levelData.currentLevel;
		m_totalLevelXp       = levelData.currentLevelXp;
		m_levelHealthGranted = levelData.currentHealth;

		levelChanged();
	}
}

// ----------------------------------------------------------------------

CreatureObject::Difficulty CreatureObject::getDifficulty() const
{
	return static_cast<Difficulty>(m_difficulty.get());
}

// ----------------------------------------------------------------------

void CreatureObject::setDifficulty(Difficulty const difficulty)
{
	m_difficulty = static_cast<unsigned char>(difficulty);
}

// ----------------------------------------------------------------------

void CreatureObject::setHologramType(int32 const newHologramType)
{
	if (isAuthoritative())
	{
		m_hologramType = newHologramType;
	}
	else
	{	
		sendControllerMessageToAuthServer(CM_modifyHologramType, new MessageQueueGenericValueType<int32 >(newHologramType));				
	}
}

// ----------------------------------------------------------------------

void CreatureObject::setVisibleOnMapAndRadar(bool const newVal)
{
	if (isAuthoritative())
	{
		m_visibleOnMapAndRadar = newVal;
	}
	else
	{	
		sendControllerMessageToAuthServer(CM_modifyVisibleOnMapAndRadar, new MessageQueueGenericValueType<bool>(newVal));				
	}
}

// ----------------------------------------------------------------------

float CreatureObject::getRegenRate(Attributes::Enumerator poolAttrib) const
{
	if (!Attributes::isAttribPool(poolAttrib))
		return 0;

	// we've stored the rate as an int, convert back to float
	int fixedFloatRate = m_attributes[poolAttrib+1];
	return static_cast<float>(fixedFloatRate);
}

// ----------------------------------------------------------------------

void CreatureObject::setRegenRate(Attributes::Enumerator poolAttrib, float value)
{
	if (Attributes::isAttribPool(poolAttrib))
	{
		if (value < 0)
			value = 0;
		if (isAuthoritative())
		{
			// store the value as an int
			m_attributes.set(poolAttrib+1, static_cast<int>(floor(value)));
		}
		else
		{
			sendControllerMessageToAuthServer(CM_setRegenRate, new MessageQueueGenericValueType<std::pair<int, float> >(std::make_pair(static_cast<int>(poolAttrib), value)));
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::setLastWaterDamageTime(unsigned long newTime)
{
	m_lastWaterDamageTime = newTime;
}

// ----------------------------------------------------------------------

bool CreatureObject::doesLocomotionInvalidateCommand(Command const &cmd) const
{
	Locomotions::Enumerator locomotion = getLocomotion();
	if((locomotion != Locomotions::Running) || cmd.m_locomotionPermissions[locomotion])
		return !cmd.m_locomotionPermissions[locomotion];
	if(!isPlayerControlled())
		return true;
	PlayerCreatureController const *playerCreatureController = safe_cast<PlayerCreatureController const *>(getController());
	if(playerCreatureController)
		return (playerCreatureController->getLastSpeed() > ConfigSharedGame::getMovementThresholdForActionValidation());
	return true;
}

// ----------------------------------------------------------------------

CommandQueue * CreatureObject::getCommandQueue() const
{
	if (m_commandQueue == nullptr)
	{
		m_commandQueue = CommandQueue::getCommandQueue(*const_cast<CreatureObject *>(this));
	}

	return m_commandQueue;
}

// ----------------------------------------------------------------------

void CreatureObject::clearCommands()
{
	m_commands.clear();	
}

// ----------------------------------------------------------------------

std::map<std::string, int> const & CreatureObject::getCommandList() const
{
	return m_commands.getMap();
}

// ----------------------------------------------------------------------

/**
	Multiple skills can grant the same command. Commands can be granted
	apart from skills. When a skill is removed, it must remove the command
	it granted, but it probably shouldn't remove the command if is granted
	by another skill or commands granted via other methods
	(items/quests, etc...)
*/
bool CreatureObject::grantCommand(std::string const & commandName, bool const fromSkill)
{
	bool result = false;
	if(isAuthoritative())
	{
		result = true;
		// check if we already have the command
		Archive::AutoDeltaMap<std::string, int>::const_iterator found = m_commands.find(commandName);
		if (found == m_commands.end())
		{
			m_commands.set(commandName, 1);

			if(getClient())
			{
				GrantCommand grant(commandName);
				getClient()->send(grant, true);
			}
			if (!fromSkill)
			{
				// save command as objvar
				setObjVarItem(OBJVAR_NOT_SKILL_COMMANDS + "." + commandName, 1);
			}
		}
		else
		{
			m_commands.set(commandName, (*found).second + 1);
		}
	}
	else
	{
		result = true;
		sendControllerMessageToAuthServer(CM_grantCommand, new MessageQueueGenericValueType<std::string>(commandName));
	}
	return result;
}

//-----------------------------------------------------------------------

void CreatureObject::revokeCommand(std::string const & command, bool const fromSkill, bool const ignoreCount)
{
	if(isAuthoritative())
	{
		Archive::AutoDeltaMap<std::string, int>::const_iterator found = m_commands.find(command);
		if (found != m_commands.end())
		{
			if ((*found).second > 1 && !ignoreCount)
			{
				m_commands.set(command, (*found).second - 1);
			}
			else
			{
				m_commands.erase(found);

				if(getClient())
				{
					RevokeCommand revoke(command);
					getClient()->send(revoke, true);
				}
				if (!fromSkill)
				{
					// remove commands from objvars
					removeObjVarItem(OBJVAR_NOT_SKILL_COMMANDS + "." + command);
				}
			}
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_revokeCommand, new MessageQueueGenericValueType<std::string>(command));
	}
}

//-----------------------------------------------------------------------

bool CreatureObject::hasCommand(std::string const & commandName) const
{
	bool result = false;
	if (commandName == "unskilled")
		result = true;
	else
	{
		const Archive::AutoDeltaMap<std::string, int>::const_iterator f = m_commands.find(commandName);
		result = (f != m_commands.end());
	}
	return result;
}

//-----------------------------------------------------------------------

/**
 * Called when our observer count changes. Will bring us into or out of hibernation
 * if needed.
 */
void CreatureObject::observersCountChanged()
{
	if (isAuthoritative())
	{
		CreatureController * controller = getCreatureController();
		int observers = getObserversCount();
		bool hibernating = controller->getHibernate();
		if ((observers > 0 && hibernating) ||
			(observers <= 0 && !hibernating))
		{
			// attempt to change our hibernate state
			controller->updateHibernate();
		}
	}
}

//-----------------------------------------------------------------------

bool CreatureObject::isStopped() const
{
	return (m_stopWalkRun.get() == 0);
}

//-----------------------------------------------------------------------

bool CreatureObject::isWalking() const
{
	return (m_stopWalkRun.get() == 1);
}

//-----------------------------------------------------------------------

bool CreatureObject::isRunning() const
{
	return (m_stopWalkRun.get() == 2);
}

//-----------------------------------------------------------------------

void CreatureObject::setLookAtYaw(const float lookAtYaw, bool useLookAtYaw)
{
	if(useLookAtYaw && (m_lookAtYaw != lookAtYaw))
		setTransformChanged(true);
	m_lookAtYaw = lookAtYaw;
	m_useLookAtYaw = useLookAtYaw;	
}

//-----------------------------------------------------------------------

float CreatureObject::getLookAtYaw() const
{
	return m_lookAtYaw;
}

//-----------------------------------------------------------------------

bool CreatureObject::getUseLookAtYaw() const
{
	return m_useLookAtYaw;
}

//-----------------------------------------------------------------------

int CreatureObject::getExpertiseRankForPlayer(std::string const & expertiseName)
{
	int result = 0;

	int rankMax = ExpertiseManager::getExpertiseRankMax(expertiseName);

	int tree = ExpertiseManager::getExpertiseTree(expertiseName);
	int tier = ExpertiseManager::getExpertiseTier(expertiseName);
	int grid = ExpertiseManager::getExpertiseGrid(expertiseName);
	int rank = 1;

	while (rank <= rankMax)
	{
		SkillObject const * skill = ExpertiseManager::getExpertiseSkillAt(tree, tier, grid, rank);
		if (skill && hasSkill(*skill))
		{
			result = rank;
			rank++;
		}
		else
		{
			break;
		}
	}
	return result;
}

//-----------------------------------------------------------------------

void CreatureObject::getExpertisesForPlayer(SkillList & expertiseList) const
{
	expertiseList.clear();

	SkillList const & skillList = this->getSkillList();

	for (SkillList::const_iterator i = skillList.begin(); i != skillList.end(); ++i)
	{
		if (*i)
		{
			if (ExpertiseManager::isExpertise(*i))
			{
				expertiseList.insert(*i);
			}
		}
	}
}

//-----------------------------------------------------------------------

int CreatureObject::getExpertisePointsSpentForPlayerInTree(int tree)
{
	int spentPoints = 0;

	SkillList expertiseList;
	getExpertisesForPlayer(expertiseList);

	for (SkillList::const_iterator i = expertiseList.begin(); i != expertiseList.end(); ++i)
	{
		SkillObject const * expertise = *i;
		if (expertise)
		{
			std::string const & expertiseName = expertise->getSkillName();
			if (ExpertiseManager::getExpertiseTree(expertiseName) == tree)
			{
				spentPoints += getExpertiseRankForPlayer(expertiseName);
			}
		}
	}
	return spentPoints;
}

//----------------------------------------------------------------------

int CreatureObject::getRemainingExpertisePoints() const
{
	int const numPointsAvail = ExpertiseManager::getExpertisePointsForLevel(getLevel());

	CreatureObject::SkillList expertiseList;
	getExpertisesForPlayer(expertiseList);
	int const numPointsUsed = expertiseList.size() - 1; // subtract 1 for skill "expertise"

	return numPointsAvail - numPointsUsed;
}

//-----------------------------------------------------------------------

bool CreatureObject::processExpertiseRequest(std::vector<std::string> const &addExpertisesNamesList, bool clearAllExpertisesFirst)
{
	
	// if you are in god mode, grant the expertise without permission checks
	if(getClient()->isGod()) {
		for(std::vector<std::string>::const_iterator i = addExpertisesNamesList.begin(); i != addExpertisesNamesList.end(); ++i) {
			std::string const &s = *i;
			const SkillObject *skill = SkillManager::getInstance().getSkill(s);
			grantSkill(*skill);
			Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("GOD MODE: Granting you expertise skill %s without regard for points, requisites, or permissions, because you are in God Mode.", skill->getSkillName().c_str())), Unicode::emptyString);
		}
		return true;
	}
	
	for(std::vector<std::string>::const_iterator i = addExpertisesNamesList.begin(); i != addExpertisesNamesList.end(); ++i)
	{
		std::string const &s = *i;
		const SkillObject *skill = SkillManager::getInstance().getSkill(s);
		if(skill)
		{
			//Check prerequisites
			SkillObject::SkillVector const prereqs = skill->getPrerequisiteSkills();
			for (SkillObject::SkillVector::const_iterator i = prereqs.begin(); i != prereqs.end(); ++i)
			{
				SkillObject const * prereq = (*i);
				if (!prereq || !hasSkill(*prereq))
				{
					DEBUG_WARNING(true, ("player %s tried to get expertise %s but doesn't have expertise %s", getNetworkId().getValueString().c_str(),
						skill->getSkillName().c_str(), prereq->getSkillName().c_str()));
					return false;
				}
			}
			//Check tier points
			//Check if the player has enough points for a skill of this tier
			int tree = ExpertiseManager::getExpertiseTree(s);
			int pointsInTree = getExpertisePointsSpentForPlayerInTree(tree);
			int tier = ExpertiseManager::getExpertiseTier(s);
			if (pointsInTree < (tier - 1) * POINTS_PER_TIER)
			{
				DEBUG_WARNING(true, ("player %s tried to get expertise %s but only has %d points in tree %d, needs %d", getNetworkId().getValueString().c_str(),
					s.c_str(), pointsInTree, tree, (tier - 1) * POINTS_PER_TIER
					));
				return false;
			}

			if (getRemainingExpertisePoints() < 1)
			{
				LOG("CustomerService", 
					("SuspectedCheaterChannel: %s attempted to gain more expertise than allowed.",
					PlayerObject::getAccountDescription(this).c_str())
					);

				return false;
			}

			//Check faction column

			//Grant skill
			grantSkill(*skill);
		}
	}
	return true;
}

//-----------------------------------------------------------------------

bool CreatureObject::clearAllExpertises()
{
	int numTries = 0;
	while(numTries < 50)
	{
		//@TODO:  guard this however design wants
		const SkillList &skills = getSkillList();
		std::vector<const SkillObject *> skillsToRevoke;
		for(std::set<const SkillObject *>::const_iterator i(skills.begin()); i != skills.end(); ++i)
		{
			const SkillObject *skill = *i;
			if(skill && ExpertiseManager::isExpertise(skill))
				skillsToRevoke.push_back(skill);
		}
		if(skillsToRevoke.empty())
			return true;
		for(std::vector<const SkillObject *>::iterator i2 = skillsToRevoke.begin(); i2 != skillsToRevoke.end(); ++i2)
		{
			const SkillObject *skill = *i2;
			revokeSkill(*skill, true);
		}
		numTries++;
	}
	return false;
}

//-----------------------------------------------------------------------

void CreatureObject::recomputeCommandSeries()
{
	for(std::vector<CommandSeriesRecord>::iterator i = s_commandSeriesRecords.begin(); i != s_commandSeriesRecords.end(); ++i)
	{
		CommandSeriesRecord const & csr = *i;
		bool hasLevel = getLevel() >= csr.level;
		if(csr.baseCommandName.empty())
		{
			if(!hasLevel)
				LOG("CustomerService",("Player %s of level %d has command %s from command series, but should not have it until level %d",
					getNetworkId().getValueString().c_str(), getLevel(), csr.baseCommandName.c_str(), csr.level));
		}
		else
		{
			if(hasCommand(csr.commandName) && (!hasCommand(csr.baseCommandName) || !hasLevel))
			{
				revokeCommand(csr.commandName, true);
			}
			else if(!hasCommand(csr.commandName) && hasCommand(csr.baseCommandName) && hasLevel)
			{
				grantCommand(csr.commandName, true);
			}
		}

	}
}

//-----------------------------------------------------------------------

void CreatureObject::incrementKillMeter(int amount)
{
	if (isPlayerControlled())
	{
		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(this);
		if (playerObject != nullptr)
		{
			playerObject->incrementKillMeter(amount);
		}
	}
}

//-----------------------------------------------------------------------

void CreatureObject::getLfgCharacterData(LfgCharacterData & lfgCharacterData) const
{
	lfgCharacterData.characterId = getNetworkId();
	lfgCharacterData.characterName = getAssignedObjectName();
	lfgCharacterData.characterLowercaseFirstName = Unicode::wideToNarrow(Unicode::toLower(getAssignedObjectFirstName()));

	lfgCharacterData.species = getSpecies();
	lfgCharacterData.gender = getGender();

	std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
	std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = connectedCharacterLfgData.find(getNetworkId());
	if (iterFind != connectedCharacterLfgData.end())
	{
		lfgCharacterData.characterInterests = iterFind->second.characterInterests;
		lfgCharacterData.characterCreateTime = iterFind->second.characterCreateTime;
		lfgCharacterData.characterAge = iterFind->second.characterAge;
	}
	else
	{
		lfgCharacterData.characterInterests.clear();
		lfgCharacterData.characterCreateTime = -1;
		lfgCharacterData.characterAge = -1;
	}

	if (lfgCharacterData.characterCreateTime <= 0)
		lfgCharacterData.characterCreateTime = static_cast<int32>(NameManager::getInstance().getPlayerCreateTime(getNetworkId()));

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(this);
	if (playerObject)
	{
		lfgCharacterData.characterAge = playerObject->getAgeInDays();

		lfgCharacterData.profession = LfgCharacterData::convertSkillTemplateToProfession(playerObject->getSkillTemplate());

		MatchMakingId const & id = playerObject->getMatchMakingCharacterProfileId();
		lfgCharacterData.anonymous = id.isBitSet(MatchMakingId::B_anonymous);
		lfgCharacterData.searchableByCtsSourceGalaxy = id.isBitSet(MatchMakingId::B_searchableByCtsSourceGalaxy);
		lfgCharacterData.displayLocationInSearchResults = id.isBitSet(MatchMakingId::B_displayLocationInSearchResults);

		if (id.isBitSet(MatchMakingId::B_lookingForGroup))
			LfgDataTable::setBit("lfg", lfgCharacterData.characterInterests);
		else
			LfgDataTable::clearBit("lfg", lfgCharacterData.characterInterests);

		if (id.isBitSet(MatchMakingId::B_helper))
			LfgDataTable::setBit("helper", lfgCharacterData.characterInterests);
		else
			LfgDataTable::clearBit("helper", lfgCharacterData.characterInterests);

		if (id.isBitSet(MatchMakingId::B_rolePlay))
			LfgDataTable::setBit("rp", lfgCharacterData.characterInterests);
		else
			LfgDataTable::clearBit("rp", lfgCharacterData.characterInterests);

		if (id.isBitSet(MatchMakingId::B_lookingForWork))
			LfgDataTable::setBit("lfw", lfgCharacterData.characterInterests);
		else
			LfgDataTable::clearBit("lfw", lfgCharacterData.characterInterests);

		lfgCharacterData.active = (playerObject->getSessionLastActiveTime() != 0);
		lfgCharacterData.sessionActivity = playerObject->getSessionActivity();
	}
	else
	{
		lfgCharacterData.profession = LfgCharacterData::Prof_Unknown;
		lfgCharacterData.anonymous = false;
		lfgCharacterData.displayLocationInSearchResults = false;
		lfgCharacterData.active = false;
		lfgCharacterData.sessionActivity = 0;
	}

	SkillObject const * skill = SkillManager::getInstance().getSkill("pilot_rebel_navy_novice");
	if (skill && hasSkill(*skill))
	{
		lfgCharacterData.pilot = LfgCharacterData::Pilot_Rebel;
	}
	else
	{
		skill = SkillManager::getInstance().getSkill("pilot_imperial_navy_novice");
		if (skill && hasSkill(*skill))
		{
			lfgCharacterData.pilot = LfgCharacterData::Pilot_Imperial;
		}
		else
		{
			skill = SkillManager::getInstance().getSkill("pilot_neutral_novice");
			if (skill && hasSkill(*skill))
			{
				lfgCharacterData.pilot = LfgCharacterData::Pilot_Neutral;
			}
			else
			{
				lfgCharacterData.pilot = LfgCharacterData::Pilot_None;
			}
		}
	}

	lfgCharacterData.level = getLevel();
	lfgCharacterData.faction = getPvpFaction();

	int const guildId = getGuildId();
	if (guildId == 0)
	{
		lfgCharacterData.guildName.clear();
		lfgCharacterData.guildAbbrev.clear();
	}
	else
	{
		lfgCharacterData.guildName = GuildInterface::getGuildName(guildId);
		lfgCharacterData.guildAbbrev = GuildInterface::getGuildAbbrev(guildId);
	}

	std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(getNetworkId());
	if (!cityIds.empty())
	{
		lfgCharacterData.citizenOfCity = CityInterface::getCityInfo(cityIds.front()).getCityName();
	}
	else
	{
		lfgCharacterData.citizenOfCity.clear();
	}

	lfgCharacterData.groupId = m_group.get();
	lfgCharacterData.locationPlanet = ServerWorld::getSceneId();

	lfgCharacterData.locationRegion.clear();
	lfgCharacterData.locationFactionalPresenceGcwRegion.clear();
	lfgCharacterData.locationFactionalPresenceGridX = 0;
	lfgCharacterData.locationFactionalPresenceGridZ = 0;
	lfgCharacterData.locationPlayerCity.clear();
	PlanetObject const * const planetObject = ServerUniverse::getInstance().getPlanetByName(lfgCharacterData.locationPlanet);
	if (planetObject)
	{
		Vector const creaturePosition = findPosition_w();
		Region const * const region = RegionMaster::getSmallestVisibleRegionAtPoint(planetObject->getName(), creaturePosition.x, creaturePosition.z);

		if (region != nullptr)
			lfgCharacterData.locationRegion = Unicode::wideToNarrow(region->getName());

		// handle factional presence
		Client const * const client = getClient();
		if (client && (!ConfigServerGame::getGcwFactionalPresenceDisableForSecuredLogin() || !client->isGodValidated()))
		{
			std::string const & playerCurrentGcwRegion = playerObject->getCurrentGcwRegion();
			if (!playerCurrentGcwRegion.empty())
			{
				if (!ServerWorld::isSpaceScene())
				{
					// ground factional presence
					//
					// character contributes to ground GCW factional presence if he is actively
					// playing the game, is declared factional, is not in a cell, is not
					// cloaked/incapped/dead/swimming, and is not hidden from the overhead map/radar
					if (lfgCharacterData.active && !PvpData::isNeutralFactionId(lfgCharacterData.faction) && PvpInternal::isDeclared(*this) && !isHidden() && isVisible() && getCoverVisibility() && !isIncapacitated() && !isDead() && !getState(States::Swimming) && getVisibleOnMapAndRadar())
					{
						bool inCell = false;
						ServerObject const * o = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));
						while (o)
						{
							if (o->asCellObject())
							{
								inCell = true;
								break;
							}

							o = safe_cast<ServerObject const*>(ContainerInterface::getContainedByObject(*o));
						}

						if (!inCell)
						{
							// make sure he (or his vehicle/mount) is standing on the
							// ground, and not on a roof/balcony/storyteller platform/etc
							Object const * playerOrMount = getMountedCreature();
							if (playerOrMount)
							{
								// is factional presence allowed while mounted?
								if (ConfigServerGame::getGcwFactionalPresenceMountedPct() <= 0)
									playerOrMount = nullptr;
							}
							else
							{
								playerOrMount = this;
							}

							if (playerOrMount)
							{
								CollisionProperty const * const collisionProperty = playerOrMount->getCollisionProperty();
								Footprint const * const footprint = (collisionProperty ? collisionProperty->getFootprint() : nullptr);
								bool isOnSolidFloor = (footprint && footprint->isOnSolidFloor());
								if (isOnSolidFloor)
								{
									// make sure the player/mount is actually standing on the floor for it to count
									float groundHeight, floorHeight;
									if (footprint->getGroundHeight(groundHeight) && footprint->getFloorHeight(floorHeight) && (groundHeight == floorHeight))
									{
										// really standing on the floor
									}
									else
									{
										// not really standing on the floor
										isOnSolidFloor = false;
									}
								}

								// standing on the ground
								if (!isOnSolidFloor)
								{
									lfgCharacterData.locationFactionalPresenceGcwRegion = playerCurrentGcwRegion;

									// if there's factional presence, calculate the grid in which it is occurring
									// clamp to -8000 - 7999
									int posX = std::min(std::max(static_cast<int>(creaturePosition.x), -8000), 7999);

									// clamp to -8000 - 7999
									int posZ = std::min(std::max(static_cast<int>(creaturePosition.z), -8000), 7999);

									if (posX < 0)
										posX -= (PvpData::getGcwFactionalPresenceGridSize() - 1);

									if (posZ < 0)
										posZ -= (PvpData::getGcwFactionalPresenceGridSize() - 1);

									lfgCharacterData.locationFactionalPresenceGridX = ((posX / PvpData::getGcwFactionalPresenceGridSize()) * PvpData::getGcwFactionalPresenceGridSize()) + (PvpData::getGcwFactionalPresenceGridSize() / 2);
									lfgCharacterData.locationFactionalPresenceGridZ = ((posZ / PvpData::getGcwFactionalPresenceGridSize()) * PvpData::getGcwFactionalPresenceGridSize()) + (PvpData::getGcwFactionalPresenceGridSize() / 2);
								}
							}
						}
					}
				}
				else if (lfgCharacterData.active && !isIncapacitated() && !isDead())
				{
					// space factional presence
					//
					// character contributes to space GCW factional presence if he is actively playing the
					// game, is not incapped/dead, is on board a player ship that's open to attack from
					// other player ships, and is the same faction as the ship
					ShipObject const * const ship = ShipObject::getContainingShipObject(this);
					if (ship && ship->isPlayerShip() && !ship->isInvulnerable() && PvpInternal::isDeclared(*ship))
					{
						uint32 const shipFaction = ship->getSpaceFaction();
						if ((PvpData::isImperialFactionId(shipFaction) || PvpData::isRebelFactionId(shipFaction)) && (lfgCharacterData.faction == shipFaction))
							lfgCharacterData.locationFactionalPresenceGcwRegion = playerCurrentGcwRegion;
					}
				}
			}
		}

		int const cityId = CityInterface::getCityAtLocation(planetObject->getName(), static_cast<int>(creaturePosition.x), static_cast<int>(creaturePosition.z), 0);
		if (cityId > 0)
			lfgCharacterData.locationPlayerCity = CityInterface::getCityInfo(cityId).getCityName();
	}

	// CTS transferred from galaxy
	lfgCharacterData.ctsSourceGalaxy.clear();
	if (getObjVars().hasItem("ctsHistory"))
	{
		DynamicVariableList::NestedList const ctsTransactions(getObjVars(), "ctsHistory");
		for (DynamicVariableList::NestedList::const_iterator i = ctsTransactions.begin(); i != ctsTransactions.end(); ++i)
		{
			Unicode::String ctsTransactionDetail;
			if (i.getValue(ctsTransactionDetail))
			{
				Unicode::UnicodeStringVector tokens;
				if (Unicode::tokenize(ctsTransactionDetail, tokens, nullptr, nullptr) && (tokens.size() >= 4))
				{
					IGNORE_RETURN(lfgCharacterData.ctsSourceGalaxy.insert(Unicode::wideToNarrow(tokens[1])));
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

bool CreatureObjectNamespace::GroupHelpers::creatureIsContainedInPOBShip(CreatureObject const * const creatureObject)
{
	NOT_NULL(creatureObject);

	ShipObject const * const shipObject = ShipObject::getContainingShipObject(creatureObject);
	return shipObject && shipObject->getPortalProperty();
}

// ----------------------------------------------------------------------

void CreatureObjectNamespace::GroupHelpers::findAllTargetsForGroup(CreatureObject * const targetObj, std::vector<CreatureObject *> & targets)
{
	typedef std::vector<CreatureObject *> Targets;

	NOT_NULL(targetObj);

	ShipObject * const shipObject = ShipObject::getContainingShipObject(targetObj);
	GroupObject const * const groupObject = targetObj->getGroup();

	if ((shipObject != 0) && (groupObject != 0))
	{
		Targets passengers;

		shipObject->findAllPassengers(passengers, true);

		Targets::const_iterator ii = passengers.begin();
		Targets::const_iterator iiEnd = passengers.end();

		for (; ii != iiEnd; ++ii)
		{
			CreatureObject * const passenger = *ii;

			if (passenger != 0)
			{
				NetworkId const & passengerId = passenger->getNetworkId();

				if (groupObject->isGroupMember(passengerId))
				{
					targets.push_back(passenger);
				}
			}
			NOT_NULL(passenger);
		}
	}
	else
	{
		targets.push_back(targetObj);
	}
}

// ----------------------------------------------------------------------

bool CreatureObjectNamespace::GroupHelpers::roomInGroup(GroupObject const * groupObj, int additionalMembers)
{
	if (groupObj != 0)
	{
		return groupObj->doesGroupHaveRoomFor(additionalMembers);
	}

	additionalMembers = std::max(0, additionalMembers);
	return additionalMembers < GroupObject::maximumMembersInGroup();
}

// ----------------------------------------------------------------------

GroupMemberParam const CreatureObjectNamespace::GroupHelpers::buildGroupMemberParam(CreatureObject const * const creatureObject)
{
	NOT_NULL(creatureObject);

	ShipObject const * const shipObject = ShipObject::getContainingShipObject(creatureObject);
	NetworkId const & memberId = creatureObject->getNetworkId();
	std::string const & name = Unicode::wideToNarrow(creatureObject->getEncodedObjectName());
	int const level = creatureObject->getLevel();
	bool const memberIsPC = creatureObject->isPlayerControlled();
	LfgCharacterData::Profession profession = LfgCharacterData::Prof_Unknown;
	if (!memberIsPC)
	{
		profession = LfgCharacterData::Prof_NPC;
	}
	else
	{
		PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
		if (playerObject)
			profession = LfgCharacterData::convertSkillTemplateToProfession(playerObject->getSkillTemplate());
	}

	NetworkId const & shipId = (shipObject != 0) ? shipObject->getNetworkId() : NetworkId::cms_invalid;
	bool const shipIsPOB = shipObject != 0 && shipObject->getPortalProperty() != 0;
	bool const ownsPOB = shipObject != 0 && shipObject->getOwnerId() == memberId;

	return GroupMemberParam(memberId, name, level, profession, memberIsPC, shipId, shipIsPOB, ownsPOB);
}

// ----------------------------------------------------------------------

void CreatureObjectNamespace::GroupHelpers::buildGroupMemberParamsFromCreatures(std::vector<CreatureObject *> const & targets, GroupObject::GroupMemberParamVector & targetMemberParams)
{
	std::vector<CreatureObject *>::const_iterator ii = targets.begin();
	std::vector<CreatureObject *>::const_iterator iiEnd = targets.end();

	for (; ii != iiEnd; ++ii)
	{
		CreatureObject const * const creatureObject = *ii;
		if (creatureObject != 0)
		{
			targetMemberParams.push_back(GroupHelpers::buildGroupMemberParam(creatureObject));
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObjectNamespace::restoreItemDecorationLayout(CreatureObject & decorator, PlayerObject & decoratorPlayerObject, CachedNetworkId const & item, TangibleObject const & pob, CachedNetworkId const & cell, Vector const & position, Quaternion const & orientation)
{
	// make sure item is around
	ServerObject const * const itemSo = safe_cast<ServerObject const *>(item.getObject());
	if (!itemSo)
		return;

	// make sure item is immediately in a cell/room
	ServerObject const * const itemContainingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*itemSo));
	if (!itemContainingCell || !itemContainingCell->asCellObject())
		return;

	// make sure item is in the pob
	if (&pob != ContainerInterface::getContainedByObject(*itemContainingCell))
		return;

	// make sure the cell is around
	ServerObject const * const cellSo = safe_cast<ServerObject const *>(cell.getObject());
	if (!cellSo)
		return;

	CellObject const * const cellCo = cellSo->asCellObject();
	if (!cellCo)
		return;

	// make sure cell is in the pob
	if (&pob != ContainerInterface::getContainedByObject(*cellCo))
		return;

	// item is not currently in the target room, need to move it
	bool needToMoveItem = false;
	bool needToRotateItem = false;
	Transform const * itemCurrentTransform = nullptr;
	if (itemContainingCell->getNetworkId() != cellSo->getNetworkId())
	{
		needToMoveItem = true;
	}
	else
	{
		// if the item is not currently at the desired position, need to move it
		itemCurrentTransform = &(itemSo->getTransform_o2p());
		Vector const currentPosition = itemCurrentTransform->getPosition_p();
		if (currentPosition.magnitudeBetweenSquared(position) > 0.0025) // not within 0.05m
		{
			needToMoveItem = true;
		}
		else
		{
			// if the item is not currently at the desired orientation, need to rotate it
			Quaternion const currentOrientation(*itemCurrentTransform);
			if (!WithinEpsilonInclusive(currentOrientation.w, orientation.w, 0.01f) ||
				!WithinEpsilonInclusive(currentOrientation.x, orientation.x, 0.01f) ||
				!WithinEpsilonInclusive(currentOrientation.y, orientation.y, 0.01f) ||
				!WithinEpsilonInclusive(currentOrientation.z, orientation.z, 0.01f))
			{
				needToRotateItem = true;
			}
		}
	}

	if (!needToMoveItem && !needToRotateItem)
		return;

	// if the item needs to be moved, check to see if it needs to be rotated as well
	if (needToMoveItem)
	{
		if (!itemCurrentTransform)
			itemCurrentTransform = &(itemSo->getTransform_o2p());

		Quaternion const currentOrientation(*itemCurrentTransform);
		if (!WithinEpsilonInclusive(currentOrientation.w, orientation.w, 0.01f) ||
			!WithinEpsilonInclusive(currentOrientation.x, orientation.x, 0.01f) ||
			!WithinEpsilonInclusive(currentOrientation.y, orientation.y, 0.01f) ||
			!WithinEpsilonInclusive(currentOrientation.z, orientation.z, 0.01f))
		{
			needToRotateItem = true;
		}
	}

	// if the item needs to be moved, find the "best" valid position for the item
	if (needToMoveItem)
	{
		CellProperty const * const cellProperty = cellCo->getCellProperty();
		if (!cellProperty)
			return;

		Floor const * const floor = cellProperty->getFloor();
		if (!floor)
			return;

		BaseExtent const * const floorExtent = floor->getExtent_l();
		if (!floorExtent)
			return;

		AxialBox const floorExtentBoundingBox = floorExtent->getBoundingBox();
		Vector floorExtentBoundingBoxCenter = floorExtentBoundingBox.getCenter();
		floorExtentBoundingBoxCenter.y = position.y + 1.0f;

		Vector goodLocation(position);
		goodLocation.y = floorExtentBoundingBoxCenter.y;

		bool foundGoodPoint = true;
		FloorLocator flResult;
		if (!floor->intersectClosest(Ray3d(goodLocation, Vector::negativeUnitY), flResult))
		{
			// do a "binary" search of points on the line starting at the floor bounding
			// box center location (which may be a good location) and ending at the target 
			// location (which is a bad location) until a point on the line is found
			// that is a good location, and that point is "up against" the wall of the cell
			Vector badLocation(goodLocation);
			goodLocation = floorExtentBoundingBoxCenter;
			foundGoodPoint = floor->intersectClosest(Ray3d(goodLocation, Vector::negativeUnitY), flResult);

			Vector midpointPosition;
			while (true)
			{
				// stop when the 2 closest "good" and "bad" location points are .2m apart,
				// meaning we have gotten close enough to "up against" the wall of the cell
				if (goodLocation.magnitudeXZBetweenSquared(badLocation) <= 0.04f)
					break;

				midpointPosition = Vector::midpoint(goodLocation, badLocation);
				midpointPosition.y = floorExtentBoundingBoxCenter.y;
				if (floor->intersectClosest(Ray3d(midpointPosition, Vector::negativeUnitY), flResult))
				{
					foundGoodPoint = true;
					goodLocation = midpointPosition;
				}
				else
				{
					badLocation = midpointPosition;
				}
			}
		}

		// couldn't find any valid position for the item
		if (!foundGoodPoint)
			return;

		// set the height of the item
		goodLocation.y = position.y;

		// trigger script to move the item, which will perform the remaining validation
		GameScriptObject * const gameScriptObject = decorator.getScriptObject();
		if (!gameScriptObject)
			return;

		ScriptParams sp;
		sp.addParam(itemSo->getNetworkId());
		sp.addParam(cellSo->getNetworkId());
		sp.addParam(goodLocation);
		sp.addParam(needToRotateItem);
		sp.addParam(orientation.w);
		sp.addParam(orientation.x);
		sp.addParam(orientation.y);
		sp.addParam(orientation.z);

		// set the id of the object being restored, so that we can bypass the LOS check for it
		decoratorPlayerObject.setRestoreDecorationObjectBeingRestored(itemSo->getNetworkId());
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_DO_RESTORE_ITEM_DECORATION_LAYOUT, sp));
		decoratorPlayerObject.setRestoreDecorationObjectBeingRestored(NetworkId::cms_invalid);

		// check to see if the item got restored
		bool itemGotRestored = true;
		if (itemContainingCell->getNetworkId() == cellSo->getNetworkId())
		{
			Transform const & itemCurrentTransform = itemSo->getTransform_o2p();
			Vector const currentPosition = itemCurrentTransform.getPosition_p();
			if (currentPosition.magnitudeBetweenSquared(goodLocation) > 0.0025) // not within 0.05m
			{
				itemGotRestored = false;
			}
			else if (needToRotateItem)
			{
				Quaternion const currentOrientation(itemCurrentTransform);
				if (!WithinEpsilonInclusive(currentOrientation.w, orientation.w, 0.01f) ||
					!WithinEpsilonInclusive(currentOrientation.x, orientation.x, 0.01f) ||
					!WithinEpsilonInclusive(currentOrientation.y, orientation.y, 0.01f) ||
					!WithinEpsilonInclusive(currentOrientation.z, orientation.z, 0.01f))
				{
					itemGotRestored = false;
				}
			}
		}
		else if (cellSo != ContainerInterface::getContainedByObject(*itemSo))
		{	
			itemGotRestored = false;
		}

		if (itemGotRestored)
			decoratorPlayerObject.setRestoreDecorationSuccessCount(decoratorPlayerObject.getRestoreDecorationSuccessCount() + 1);
	}
	else if (needToRotateItem)
	{
		// trigger script to rotate the item, which will perform the remaining validation
		GameScriptObject * const gameScriptObject = decorator.getScriptObject();
		if (!gameScriptObject)
			return;

		ScriptParams sp;
		sp.addParam(itemSo->getNetworkId());
		sp.addParam(orientation.w);
		sp.addParam(orientation.x);
		sp.addParam(orientation.y);
		sp.addParam(orientation.z);

		// set the id of the object being restored, so that we can bypass the LOS check for it
		decoratorPlayerObject.setRestoreDecorationObjectBeingRestored(itemSo->getNetworkId());
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_DO_RESTORE_ITEM_DECORATION_LAYOUT_ROTATION_ONLY, sp));
		decoratorPlayerObject.setRestoreDecorationObjectBeingRestored(NetworkId::cms_invalid);

		// check to see if the item got restored
		bool itemGotRestored = true;
		Transform const & itemCurrentTransform = itemSo->getTransform_o2p();
		Quaternion const currentOrientation(itemCurrentTransform);
		if (!WithinEpsilonInclusive(currentOrientation.w, orientation.w, 0.01f) ||
			!WithinEpsilonInclusive(currentOrientation.x, orientation.x, 0.01f) ||
			!WithinEpsilonInclusive(currentOrientation.y, orientation.y, 0.01f) ||
			!WithinEpsilonInclusive(currentOrientation.z, orientation.z, 0.01f))
		{
			itemGotRestored = false;
		}

		if (itemGotRestored)
			decoratorPlayerObject.setRestoreDecorationSuccessCount(decoratorPlayerObject.getRestoreDecorationSuccessCount() + 1);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::clientMinigameOpen(ValueDictionary const & messageData)
{
	typedef ValueDictionary Payload;
	MessageQueueGenericValueType<Payload> * msg = new MessageQueueGenericValueType<Payload>(messageData);

	if (isAuthoritative())
	{
		appendMessage(CM_clientMinigameOpen, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_clientMinigameOpen, msg);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::clientMinigameClose(ValueDictionary const & messageData)
{
	typedef ValueDictionary Payload;
	MessageQueueGenericValueType<Payload> * msg = new MessageQueueGenericValueType<Payload>(messageData);

	if (isAuthoritative())
	{
		appendMessage(CM_clientMinigameClose, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_clientMinigameClose, msg);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::addPackedAppearanceWearable(std::string const &appearanceData, int arrangementIndex, NetworkId const &networkId, uint32 sharedTemplateCrcValue)
{
	//-- Check if this network id already exists in the wearable list.
	size_t const length = m_wearableAppearanceData.size();
	for (size_t i = 0; i < length; ++i)
	{
		WearableEntry const &entry = m_wearableAppearanceData[i];
		if (entry.m_networkId == networkId)
		{
			// Check if this entry exists but in a different configuration.
			WARNING((entry.m_arrangement != arrangementIndex), ("attempted to add object id=[%s] in packed wearable list multiple times in different arrangements, existing=[%d],new=[%d].", appearanceData.c_str(), entry.m_arrangement, arrangementIndex));
			WARNING((static_cast<uint32>(entry.m_objectTemplate) != sharedTemplateCrcValue), ("attempted to add object id=[%s] in packed wearable list multiple times differing only by shared object template crc value, existing=[%d],new=[%d].", appearanceData.c_str(), entry.m_objectTemplate, static_cast<int>(sharedTemplateCrcValue)));
			DEBUG_WARNING((entry.m_appearanceString != appearanceData), ("attempted to add object id=[%s] in packed wearable list multiple times differing only by appearance customization data.", appearanceData.c_str()));

			return;
		}
	}
	//-- Add the new entry.
	m_wearableAppearanceData.push_back(WearableEntry(appearanceData, arrangementIndex, networkId, sharedTemplateCrcValue, nullptr, nullptr));
}

// ----------------------------------------------------------------------

void CreatureObject::saveDecorationLayout(ServerObject const & pobSourceObject, int saveSlotNumber, std::string const & description)
{
	int debugNumItems = 0;
	const unsigned long debugStartTimeMs = Clock::timeMs();

	if (!isAuthoritative())
		return;

	if (!getClient())
		return;

	// only slots 1-3 are currently supported
	if ((saveSlotNumber < 1) || (saveSlotNumber > 3))
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	if (!pobSourceObject.isAuthoritative())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be standing inside the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	PlayerObject * const playerObj = PlayerCreatureController::getPlayerObject(this);
	if (!playerObj)
		return;

	ServerObject const * const containingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));
	if (!containingCell || !containingCell->asCellObject())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be standing inside the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	ServerObject const * const containingPOBso = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingCell));
	if (!containingPOBso)
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be standing inside the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (!containingPOBso->isInWorldCell())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be standing inside the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (containingPOBso != &pobSourceObject)
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be standing inside the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (containingPOBso->asBuildingObject() && !containingPOBso->asBuildingObject()->getContentsLoaded())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("Please wait until the building has loaded its contents and try again."), Unicode::emptyString);
		return;
	}

	TangibleObject const * const containingPOB = containingPOBso->asTangibleObject();
	if (!containingPOB)
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	if (!containingPOB->isOnAdminList(*this))
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be the owner or an admin of the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	Container const * const pobContainer = ContainerInterface::getContainer(*containingPOB);
	if (pobContainer)
	{
		std::string objvarValue;
		int objvarValueIndex = 0;

		char buffer1[64];
		buffer1[sizeof(buffer1)-1] = '\0';

		char buffer2[1024];
		buffer2[sizeof(buffer2)-1] = '\0';

		std::set<std::string> rooms;

		snprintf(buffer1, sizeof(buffer1)-1, "savedDecoration%d", saveSlotNumber);
		playerObj->removeObjVarItem(buffer1);

		for (ContainerConstIterator i = pobContainer->begin(); i != pobContainer->end(); ++i)
		{
			ServerObject const * const cell = safe_cast<ServerObject const *>((*i).getObject());
			if (cell && cell->asCellObject())
			{
				CellProperty const * const cellContainer = cell->getCellProperty();
				if (cellContainer)
				{
					std::map<std::pair<uint32, uint32>, std::pair<float, float> >::const_iterator const iterFindCellHeight = s_mapCellHeightRestriction.find(std::make_pair(containingPOB->getTemplateCrc(), cellContainer->getCellNameCrc()));
					if (iterFindCellHeight == s_mapCellHeightRestriction.end())
						continue;

					Floor const * const floor = cellContainer->getFloor();
					if (!floor)
						continue;

					BaseExtent const * const floorExtent = floor->getExtent_l();
					if (!floorExtent)
						continue;

					AxialBox const floorExtentBoundingBox = floorExtent->getBoundingBox();
					Vector const floorExtentBoundingBoxCenter = floorExtentBoundingBox.getCenter();

					for (ContainerConstIterator j = cellContainer->begin(); j != cellContainer->end(); ++j)
					{
						ServerObject const * const cellContent = safe_cast<ServerObject const *>((*j).getObject());
						if (cellContent && cellContent->isPersisted() && !cellContent->asCreatureObject() && !cellContent->isVendor())
						{
							++debugNumItems;
							IGNORE_RETURN(rooms.insert(cellContainer->getCellName()));

							Transform const & transform = cellContent->getTransform_o2p();
							Vector const position = transform.getPosition_p();
							Quaternion const quaternion(transform);

							snprintf(buffer2, sizeof(buffer2)-1, "%s~%s~%.10g~%.10g~%.10g~%.10g~%.10g~%.10g~%.10g", cellContent->getNetworkId().getValueString().c_str(), cellContainer->getCellName(), position.x - floorExtentBoundingBoxCenter.x, position.y - iterFindCellHeight->second.first, position.z - floorExtentBoundingBoxCenter.z, quaternion.w, quaternion.x, quaternion.y, quaternion.z);
							if ((objvarValue.size() + ::strlen(buffer2) + 2)  > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
							{
								snprintf(buffer1, sizeof(buffer1)-1, "savedDecoration%d.%d", saveSlotNumber, objvarValueIndex);
								playerObj->setObjVarItem(buffer1, objvarValue);
								++objvarValueIndex;
								objvarValue = buffer2;
							}
							else
							{
								if (!objvarValue.empty())
									objvarValue += "~";

								objvarValue += buffer2;
							}
						}
					}
				}
			}
		}

		if (rooms.empty())
		{
			snprintf(buffer1, sizeof(buffer1)-1, "savedDecoration%d", saveSlotNumber);
			playerObj->removeObjVarItem(buffer1);

			Chat::sendSystemMessage(*this, Unicode::narrowToWide("There is currently no decoration in the structure or POB ship to save."), Unicode::emptyString);
		}
		else
		{
			if (!objvarValue.empty())
			{
				snprintf(buffer1, sizeof(buffer1)-1, "savedDecoration%d.%d", saveSlotNumber, objvarValueIndex);
				playerObj->setObjVarItem(buffer1, objvarValue);
			}

			snprintf(buffer1, sizeof(buffer1)-1, "savedDecoration%d.saveTime", saveSlotNumber);
			playerObj->setObjVarItem(buffer1, static_cast<int>(::time(nullptr)));

			snprintf(buffer1, sizeof(buffer1)-1, "savedDecoration%d.pobName", saveSlotNumber);
			playerObj->setObjVarItem(buffer1, containingPOB->getObjectNameStringId().localize());

			std::vector<Unicode::String> roomsVector;
			for (std::set<std::string>::const_iterator iterRoom = rooms.begin(); iterRoom != rooms.end(); ++iterRoom)
				roomsVector.push_back(Unicode::narrowToWide(*iterRoom));

			snprintf(buffer1, sizeof(buffer1)-1, "savedDecoration%d.rooms", saveSlotNumber);
			playerObj->setObjVarItem(buffer1, roomsVector);

			std::string validatedDescription = description;
			if (!validatedDescription.empty())
			{
				if (validatedDescription.size() > 40)
					validatedDescription = validatedDescription.substr(0, 40);

				snprintf(buffer1, sizeof(buffer1)-1, "savedDecoration%d.description", saveSlotNumber);
				playerObj->setObjVarItem(buffer1, validatedDescription);
			}

			if (validatedDescription.empty())
				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("The decoration layout of the structure or POB ship has been saved in slot %d.", saveSlotNumber)), Unicode::emptyString);
			else
				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("The decoration layout of the structure or POB ship has been saved in slot %d (%s).", saveSlotNumber, validatedDescription.c_str())), Unicode::emptyString);
		}
	}
	else
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
	}

	if ((debugNumItems > 0) && getClient()->isGod())
	{
		const unsigned long debugEndTimeMs = Clock::timeMs();
		Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("!!!GOD MODE STATISTICS!!! %d items saved in %lums", debugNumItems, (debugEndTimeMs - debugStartTimeMs))), Unicode::emptyString);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::restoreDecorationLayout(ServerObject const & pobTargetObject, int saveSlotNumber)
{
	const unsigned long debugStartTimeMs = Clock::timeMs();

	if (!isAuthoritative())
		return;

	if (!getClient())
		return;

	// only slots 1-3 are currently supported
	if ((saveSlotNumber < 1) || (saveSlotNumber > 3))
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	if (!pobTargetObject.isAuthoritative())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be standing inside the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	PlayerObject * const playerObj = PlayerCreatureController::getPlayerObject(this);
	if (!playerObj)
		return;

	ServerObject const * const containingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));
	if (!containingCell || !containingCell->asCellObject())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be standing inside the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	ServerObject const * const containingPOBso = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingCell));
	if (!containingPOBso)
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be standing inside the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (!containingPOBso->isInWorldCell())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be standing inside the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (containingPOBso != &pobTargetObject)
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be standing inside the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	// don't allow another restore on the pob while one is still in progress
	int const timeNow = static_cast<int>(::time(nullptr));
	int restoreDecorationOperationTimeout = 0;
	if (getObjVars().getItem("restoreDecorationOperation.timeout", restoreDecorationOperationTimeout) && (restoreDecorationOperationTimeout > timeNow))
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<1024>().sprintf("You are already performing a restore decoration layout operation.  Please wait for the current operation to complete, or if the current operation does not complete in %s, you can try another operation.", CalendarTime::convertSecondsToMS(static_cast<unsigned int>(restoreDecorationOperationTimeout - timeNow)).c_str())), Unicode::emptyString);
		return;
	}

	if (containingPOBso->asBuildingObject() && !containingPOBso->asBuildingObject()->getContentsLoaded())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("Please wait until the building has loaded its contents and try again."), Unicode::emptyString);
		return;
	}

	TangibleObject const * const containingPOB = containingPOBso->asTangibleObject();
	if (!containingPOB)
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	// check for lot overlimit drop restriction
	if ((playerObj->getAccountNumLots() > getMaxNumberOfLots()) && getObjVars().hasItem("lotOverlimit.structure_id"))
	{
		NetworkId lotOverlimitStructure;
		if (getObjVars().getItem("lotOverlimit.structure_id", lotOverlimitStructure) && lotOverlimitStructure.isValid())
		{
			if (GameObjectTypes::isTypeOf(containingPOB->getGameObjectType(), SharedObjectTemplate::GOT_building) && (containingPOB->getNetworkId() != lotOverlimitStructure))
			{
				std::string lotOverlimitStructureName;
				std::string lotOverlimitStructureLocation;

				IGNORE_RETURN(getObjVars().getItem("lotOverlimit.structure_name", lotOverlimitStructureName));
				IGNORE_RETURN(getObjVars().getItem("lotOverlimit.structure_location", lotOverlimitStructureLocation));

				Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<1024>().sprintf("You are currently over the lot limit, and cannot drop any item into (or move around items within) any factory or structure, other than the %s structure located at %s that caused you to exceed the lot limit.", lotOverlimitStructureName.c_str(), lotOverlimitStructureLocation.c_str())), Unicode::emptyString);
				return;
			}
		}
	}

	if (!containingPOB->isOnAdminList(*this))
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("You must be the owner or an admin of the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	// build the list of rooms in the target pob
	std::map<std::string, std::pair<CellObject const *, std::pair<std::pair<float, float>, std::pair<float, float> > > > pobTargetObjectRoomsToCells;
	Container const * const pobContainer = ContainerInterface::getContainer(*containingPOB);
	if (pobContainer)
	{
		for (ContainerConstIterator i = pobContainer->begin(); i != pobContainer->end(); ++i)
		{
			ServerObject const * const cell = safe_cast<ServerObject const *>((*i).getObject());
			if (!cell || !cell->asCellObject())
				continue;

			CellProperty const * const cellContainer = cell->getCellProperty();
			if (!cellContainer)
				continue;

			std::map<std::pair<uint32, uint32>, std::pair<float, float> >::const_iterator const iterFindCellHeight = s_mapCellHeightRestriction.find(std::make_pair(containingPOB->getTemplateCrc(), cellContainer->getCellNameCrc()));
			if (iterFindCellHeight == s_mapCellHeightRestriction.end())
				continue;

			Floor const * const floor = cellContainer->getFloor();
			if (!floor)
				continue;

			BaseExtent const * const floorExtent = floor->getExtent_l();
			if (!floorExtent)
				continue;

			AxialBox const floorExtentBoundingBox = floorExtent->getBoundingBox();
			Vector const floorExtentBoundingBoxCenter = floorExtentBoundingBox.getCenter();

			pobTargetObjectRoomsToCells[cellContainer->getCellName()] = std::make_pair(cell->asCellObject(), std::make_pair(std::make_pair(iterFindCellHeight->second.first, iterFindCellHeight->second.second), std::make_pair(floorExtentBoundingBoxCenter.x, floorExtentBoundingBoxCenter.z)));
		}
	}
	else
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	// build the list of source room to target room mapping, telling us where to move/restore the items
	DynamicVariableList const & objVars = playerObj->getObjVars();
	char buffer[64];
	buffer[sizeof(buffer)-1] = '\0';

	std::vector<Unicode::String> savedSourceRooms;
	snprintf(buffer, sizeof(buffer)-1, "savedDecoration%d.rooms", saveSlotNumber);
	if (!objVars.getItem(buffer, savedSourceRooms) || savedSourceRooms.empty())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	std::vector<Unicode::String> savedTargetRooms;
	snprintf(buffer, sizeof(buffer)-1, "savedDecoration%d.targetRooms", saveSlotNumber);
	if (!objVars.getItem(buffer, savedTargetRooms) || savedTargetRooms.empty())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	if (savedSourceRooms.size() != savedTargetRooms.size())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	std::map<std::string, std::string> sourceRoomToTargetRoomMapping;
	for (size_t i = 0, size = savedSourceRooms.size(); i < size; ++i)
	{
		if (savedTargetRooms[i].empty())
			continue;

		std::string savedTargetRoomNarrow = Unicode::wideToNarrow(savedTargetRooms[i]);
		if (pobTargetObjectRoomsToCells.count(savedTargetRoomNarrow) <= 0)
			continue;

		sourceRoomToTargetRoomMapping[Unicode::wideToNarrow(savedSourceRooms[i])] = savedTargetRoomNarrow;
	}

	if (sourceRoomToTargetRoomMapping.empty())
	{
		Chat::sendSystemMessage(*this, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	int debugNumItems = 0;
	int numItemsToBeMoved = 0;

	unsigned int pos;
	NetworkId itemId;
	ServerObject const * itemObj;
	ServerObject const * itemObjContainingCell;
	std::string room;
	Vector position, currentPosition;
	Quaternion orientation;
	std::map<std::string, std::string>::const_iterator iterFindSourceRoomToTargetRoomMapping;
	std::map<std::string, std::pair<CellObject const *, std::pair<std::pair<float, float>, std::pair<float, float> > > >::const_iterator iterFindPobTargetObjectRoomsToCells;

	std::string objvarValue;
	int objvarValueIndex = 0;

	std::list<std::pair<std::pair<CachedNetworkId, CachedNetworkId>, std::pair<Vector, Quaternion> > > & restoreDecorationLayoutList = playerObj->getRestoreDecorationLayoutList();
	restoreDecorationLayoutList.clear();

	snprintf(buffer, sizeof(buffer)-1, "savedDecoration%d.%d", saveSlotNumber, objvarValueIndex);
	while (objVars.getItem(buffer, objvarValue) && !objvarValue.empty())
	{
		pos = 0;
		while (objvarValue[pos])
		{
			++debugNumItems;

			nextOid(objvarValue, pos, itemId);
			nextString(objvarValue, pos, room);
			nextFloat(objvarValue, pos, position.x);
			nextFloat(objvarValue, pos, position.y);
			nextFloat(objvarValue, pos, position.z);
			nextFloat(objvarValue, pos, orientation.w);
			nextFloat(objvarValue, pos, orientation.x);
			nextFloat(objvarValue, pos, orientation.y);
			nextFloat(objvarValue, pos, orientation.z);

			itemObj = ServerWorld::findObjectByNetworkId(itemId);
			if (!itemObj)
				continue;

			itemObjContainingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*itemObj));
			if (!itemObjContainingCell || !itemObjContainingCell->asCellObject())
				continue;

			if (ContainerInterface::getContainedByObject(*itemObjContainingCell) != containingPOB)
				continue;

			// what destination room should this item go in?
			iterFindSourceRoomToTargetRoomMapping = sourceRoomToTargetRoomMapping.find(room);
			if (iterFindSourceRoomToTargetRoomMapping == sourceRoomToTargetRoomMapping.end())
			{
				// no destination room specified, item should not be moved
				continue;
			}

			// find the cell in the target structure corresponding to the target room
			iterFindPobTargetObjectRoomsToCells = pobTargetObjectRoomsToCells.find(iterFindSourceRoomToTargetRoomMapping->second);
			if (iterFindPobTargetObjectRoomsToCells == pobTargetObjectRoomsToCells.end())
			{
				// target room not found, cannot move the item
				continue;
			}

			// the height is saved as the offset from the floor
			position.y += iterFindPobTargetObjectRoomsToCells->second.second.first.first;

			if (position.y < iterFindPobTargetObjectRoomsToCells->second.second.first.first)
				position.y = iterFindPobTargetObjectRoomsToCells->second.second.first.first;
			else if (position.y > iterFindPobTargetObjectRoomsToCells->second.second.first.second)
				position.y = iterFindPobTargetObjectRoomsToCells->second.second.first.second;

			// x and z are saved as offset from the "center"
			position.x += iterFindPobTargetObjectRoomsToCells->second.second.second.first;
			position.z += iterFindPobTargetObjectRoomsToCells->second.second.second.second;

			// item is not currently in the target room, need to move it
			if (itemObjContainingCell->getNetworkId() != iterFindPobTargetObjectRoomsToCells->second.first->getNetworkId())
			{
				++numItemsToBeMoved;
				restoreDecorationLayoutList.push_back(std::make_pair(std::make_pair(CachedNetworkId(*itemObj), CachedNetworkId(*iterFindPobTargetObjectRoomsToCells->second.first)), std::make_pair(position, orientation)));
				continue;
			}

			// if the item is not currently at the desired position, need to move it
			Transform const & itemCurrentTransform = itemObj->getTransform_o2p();
			currentPosition = itemCurrentTransform.getPosition_p();
			if (currentPosition.magnitudeBetweenSquared(position) > 0.0025) // not within 0.05m
			{
				++numItemsToBeMoved;
				restoreDecorationLayoutList.push_back(std::make_pair(std::make_pair(CachedNetworkId(*itemObj), CachedNetworkId(*iterFindPobTargetObjectRoomsToCells->second.first)), std::make_pair(position, orientation)));
				continue;
			}

			// if the item is not currently at the desired orientation, need to move it
			Quaternion const currentOrientation(itemCurrentTransform);
			if (!WithinEpsilonInclusive(currentOrientation.w, orientation.w, 0.01f) ||
				!WithinEpsilonInclusive(currentOrientation.x, orientation.x, 0.01f) ||
				!WithinEpsilonInclusive(currentOrientation.y, orientation.y, 0.01f) ||
				!WithinEpsilonInclusive(currentOrientation.z, orientation.z, 0.01f))
			{
				++numItemsToBeMoved;
				restoreDecorationLayoutList.push_back(std::make_pair(std::make_pair(CachedNetworkId(*itemObj), CachedNetworkId(*iterFindPobTargetObjectRoomsToCells->second.first)), std::make_pair(position, orientation)));
				continue;
			}
		}

		++objvarValueIndex;
		snprintf(buffer, sizeof(buffer)-1, "savedDecoration%d.%d", saveSlotNumber, objvarValueIndex);
	}

	snprintf(buffer, sizeof(buffer)-1, "savedDecoration%d.description", saveSlotNumber);
	std::string saveSlotDescription;
	IGNORE_RETURN(objVars.getItem(buffer, saveSlotDescription));
	if (restoreDecorationLayoutList.empty())
	{
		if (saveSlotDescription.empty())
			Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<1024>().sprintf("No items in the structure or POB ship could be (or need to be) moved/restored using the saved decoration layout in slot %d.", saveSlotNumber)), Unicode::emptyString);
		else
			Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<1024>().sprintf("No items in the structure or POB ship could be (or need to be) moved/restored using the saved decoration layout in slot %d (%s).", saveSlotNumber, saveSlotDescription.c_str())), Unicode::emptyString);
	}
	else
	{
		if (saveSlotDescription.empty())
			Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<1024>().sprintf("Please remain in the structure or POB ship while the system attempts to move/restore %d items using the saved decoration layout in slot %d.  You will be notified when the operation is complete.", numItemsToBeMoved, saveSlotNumber)), Unicode::emptyString);
		else
			Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<1024>().sprintf("Please remain in the structure or POB ship while the system attempts to move/restore %d items using the saved decoration layout in slot %d (%s).  You will be notified when the operation is complete.", numItemsToBeMoved, saveSlotNumber, saveSlotDescription.c_str())), Unicode::emptyString);

		// start messageTo loop to do the decoration layout restore,
		// spread out over a period of time so it doesn't lag out the server
		int restoreDecorationOperationId = ::rand();
		while (restoreDecorationOperationId == 0)
			restoreDecorationOperationId = ::rand();

		playerObj->setRestoreDecorationOperationId(restoreDecorationOperationId);
		playerObj->setRestoreDecorationPobId(containingPOB->getNetworkId());
		playerObj->setRestoreDecorationTotalCount(numItemsToBeMoved);
		playerObj->setRestoreDecorationAttemptedCount(0);
		playerObj->setRestoreDecorationSuccessCount(0);

		IGNORE_RETURN(setObjVarItem("restoreDecorationOperation.timeout", timeNow + (5 * 60)));
		IGNORE_RETURN(setObjVarItem("restoreDecorationOperation.operationId", restoreDecorationOperationId));

		MessageToQueue::getInstance().sendMessageToC(getNetworkId(),
			"C++RestoreDecorationLayout",
			"",
			0,
			false);
	}

	if ((debugNumItems > 0) && getClient()->isGod())
	{
		const unsigned long debugEndTimeMs = Clock::timeMs();
		Chat::sendSystemMessage(*this, Unicode::narrowToWide(FormattedString<256>().sprintf("!!!GOD MODE STATISTICS!!! %d items read (%d will be moved) in %lums", debugNumItems, numItemsToBeMoved, (debugEndTimeMs - debugStartTimeMs))), Unicode::emptyString);
	}
}

// ======================================================================
