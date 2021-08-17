//========================================================================
//
// ScriptFuncTable.cpp - Default values for initializing the script function list.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/ScriptFunctionTable.h"


//========================================================================
// local constants
//========================================================================

//
// values for script parameters:
// b = boolean
// i = integer         (4 byte)
// f = float           (4 byte)
// s = string          (const char *)
// u = unicode string  (const uint16 *)
// V = class ValueDictionary
// O = class objId
// S = class stringId
// E = class slotData
// A = class attribMod
// M = class mentalStateMod
// L = class location
// m = class menu_info
// D = class draft_schematic
// I = struct Crafting::IngredientSlot
// U = unsigned char
// [ = previous identifier was an array
// * = previous identifier was the modifiable version of the data (currently int, float, and stringId)
//

static const Scripting::ScriptFuncTable ScriptFuncList[] =
{
	{Scripting::TRIG_ATTACH,				    "OnAttach",				""},
	{Scripting::TRIG_DETACH,				    "OnDetach",				""},
	{Scripting::TRIG_INITIALIZE,                "OnInitialize",         ""},
	{Scripting::TRIG_SPEAKING,				    "OnSpeaking",			"u"},
	{Scripting::TRIG_HEAR_SPEECH_STRING,	    "OnHearSpeech",			"Ou"},
	{Scripting::TRIG_HEAR_SPEECH_STRINGID,	    "OnHearSpeech",			"OS"},
	{Scripting::TRIG_PERFORM_EMOTE,             "OnPerformEmote",       "u"},
	{Scripting::TRIG_SAW_EMOTE,                 "OnSawEmote",           "Ou"},
	{Scripting::TRIG_INSPECTED,				    "OnInspected",			"OS"},
	{Scripting::TRIG_LOGOUT,				    "OnLogout",				""},
	{Scripting::TRIG_LOGIN,					    "OnLogin",				""},
	{Scripting::TRIG_ITEM_EQUIP,			    "OnItemEquip",			"OE["},
	{Scripting::TRIG_ITEM_UNEQUIP,			    "OnItemUneqip",			"OE["},
	{Scripting::TRIG_TARGET_EQUIPS_ITEM,	    "OnTargetEquipsItem",	"OE["},
	{Scripting::TRIG_TARGET_UNEQUIPS_ITEM,	    "OnTargetUnequipsItem",	"OE["},
	{Scripting::TRIG_ABOUT_TO_RECEIVE_ITEM,	    "OnAboutToReceiveItem", "OOO"},
	{Scripting::TRIG_RECEIVED_ITEM,			    "OnReceivedItem",		"OOO"},
	{Scripting::TRIG_ABOUT_TO_BE_XFERRED,	    "OnAboutToBeTransferred","OO"},
	{Scripting::TRIG_XFERRED,				    "OnTransferred",		"OOO"},
	{Scripting::TRIG_ABOUT_TO_LOSE_ITEM,	    "OnAboutToLoseItem",	"OOO"},
	{Scripting::TRIG_LOST_ITEM,				    "OnLostItem",			"OOO"},
	{Scripting::TRIG_LOADED_FROM_DB,		    "OnLoadedFromDb",		""},
	{Scripting::TRIG_UNLOADED_FROM_MEMORY,	    "OnUnloadedFromMemory",	""},
	{Scripting::TRIG_DECAY,					    "OnDecay",				""},
	{Scripting::TRIG_OBJECT_DISABLED,           "OnObjectDisabled",     "O"},
	{Scripting::TRIG_DESTROY,				    "OnDestroy",			""},
	{Scripting::TRIG_SERVER_TRANSFER,		    "OnServerTransfer",		""},
	{Scripting::TRIG_MADE_AUTHORITATIVE,	    "OnMadeAuthoritative",	""},
	{Scripting::TRIG_ABOUT_TO_BE_INCAPACITATED, "OnAboutToBeIncapacitated",	"O"},
	{Scripting::TRIG_INCAPACITATED,			    "OnIncapacitated",		"O"},
	{Scripting::TRIG_RECAPACITATED,             "OnRecapacitated",		""},
	{Scripting::TRIG_ABOUT_TO_DIE,			    "OnAboutToDie",			"O"},
	{Scripting::TRIG_DEATH,					    "OnDeath",				"OO"},
	{Scripting::TRIG_ENVIRONMENTAL_DEATH,       "OnEnvironmentalDeath", ""},
	{Scripting::TRIG_ENTERED_COMBAT,            "OnEnteredCombat",      ""},
	{Scripting::TRIG_EXITED_COMBAT,				"OnExitedCombat",       ""},
	{Scripting::TRIG_DEFENDER_COMBAT_ACTION,	"OnDefenderCombatAction","OOi"},
	{Scripting::TRIG_ATTACKER_COMBAT_ACTION,	"OnAttackerCombatAction","OO"},
	{Scripting::TRIG_WEAPON_COMBAT_ACTION,		"OnWeaponCombatAction", "OO"},
	{Scripting::TRIG_CHANGED_POSTURE,           "OnChangedPosture",     "ii"},
	{Scripting::TRIG_COMBAT_DAMAGED,            "OnCombatDamaged",      "OOi"},
	{Scripting::TRIG_CREATURE_DAMAGED,          "OnCreatureDamaged",    "OOi["},
	{Scripting::TRIG_OBJECT_DAMAGED,            "OnObjectDamaged",      "OOi"},
	{Scripting::TRIG_INCAPACITATE_TARGET,	    "OnIncapacitateTarget",	"O"},
	{Scripting::TRIG_ADDED_TO_WORLD,		    "OnAddedToWorld",		""},
	{Scripting::TRIG_REMOVING_FROM_WORLD,	    "OnRemovingFromWorld",	""},
	{Scripting::TRIG_PRELOAD_COMPLETE,          "OnPreloadComplete",          ""},
	{Scripting::TRIG_BEHAVIOR_CHANGE,           "OnBehaviorChange",     "iii["},
	{Scripting::TRIG_TARGET_CHANGE,             "OnBehaviorTargetChange", "ii["},
	{Scripting::TRIG_START_NPC_CONVERSATION,    "OnStartNpcConversation","O"},
	{Scripting::TRIG_END_NPC_CONVERSATION,      "OnEndNpcConversation",	"O"},
	{Scripting::TRIG_START_CONVERSATION ,       "OnStartConversation",  "O"},
	{Scripting::TRIG_END_CONVERSATION,          "OnEndConversation",	  "O"},
	{Scripting::TRIG_ARRIVE_AT_LOCATION,        "OnArrivedAtLocation",  "u"},
	{Scripting::TRIG_NPC_CONVERSATION_RESPONSE, "OnNpcConversationResponse", "sOS"},
	{Scripting::TRIG_COMBAT_TARGETED,           "OnTargeted",            "O"},
	{Scripting::TRIG_COMBAT_UNTARGETED,         "OnUntargeted",          "O"},
	{Scripting::TRIG_COMBAT_LOST_TARGET,        "OnLostTarget",          "O"},
	{Scripting::TRIG_PLAYER_REQUEST_MISSION_BOARD, "OnPlayerRequestMissionBoard", "OOO["},
	{Scripting::TRIG_OUT_OF_AMMO,               "OnOutOfAmmo",          "O"},
	{Scripting::TRIG_PVP_TYPE_CHANGED,          "OnPvpTypeChanged",     "ii"},
	{Scripting::TRIG_PVP_FACTION_CHANGED,       "OnPvpFactionChanged",  "ii"},
	{Scripting::TRIG_CITY_CHANGED,              "OnCityChanged",        "ii"},
	{Scripting::TRIG_ENTER_SWIMMING,            "OnEnterSwimming",      ""},
	{Scripting::TRIG_EXIT_SWIMMING,             "OnExitSwimming",       ""},
	{Scripting::TRIG_PLACE_STRUCTURE,           "OnPlaceStructure",     "OOLi"},
	{Scripting::TRIG_PURCHASE_TICKET,           "OnPurchaseTicket",     "Ouuuub"},
	{Scripting::TRIG_PURCHASE_TICKET_INSTANT_TRAVEL, "OnPurchaseTicketInstantTravel", "Ouuuub"},
	{Scripting::TRIG_LOCATION_RECEIVED,         "OnLocationReceived",   "sOLf"},
	{Scripting::TRIG_GET_ATTRIBS,               "OnGetAttributes",      "Os*[u*["},
	{Scripting::TRIG_GET_SCRIPTVARS,            "OnGetScriptvars",      "Os*[u*["},
	{Scripting::TRIG_SOCKET_USED,               "OnSocketUsed",         "s[i["},
	{Scripting::TRIG_GET_RESPAWN_LOC,           "OnGetRespawnLocation", "i*"},
	{Scripting::TRIG_SKILL_ABOUT_TO_BE_GRANTED, "OnSkillAboutToBeGranted", "s"},
	{Scripting::TRIG_SKILL_ABOUT_TO_BE_REVOKED, "OnSkillAboutToBeRevoked", "s"},
	{Scripting::TRIG_SKILL_GRANTED,             "OnSkillGranted",       "s"},
	{Scripting::TRIG_SKILL_REVOKED,             "OnSkillRevoked",       "s"},
	{Scripting::TRIG_ATTRIB_MOD_DONE,           "OnAttribModDone",      "sb"},
	{Scripting::TRIG_SKILL_MOD_DONE,            "OnSkillModDone",       "sb"},
	{Scripting::TRIG_SKILLMODS_CHANGED,         "OnSkillModsChanged",   "s[i["},
	{Scripting::TRIG_THEATER_CREATED,           "OnTheaterCreated",     "O[OO"},
	{Scripting::TRIG_PLAYER_THEATER_FAIL,       "OnPlayerTheaterFail",  "ss"},
	{Scripting::TRIG_GRANT_SCHEMATIC,           "OnGrantedSchematic",   "ib"},
	{Scripting::TRIG_REVOKE_SCHEMATIC,          "OnRevokedSchematic",   "ib"},
	{Scripting::TRIG_DO_STRUCTURE_ROLLUP,       "OnDoStructureRollup",  "Ob"},

	//- added when changing to scripting system that verifies triggers

	{Scripting::TRIG_UPLOAD_CHARACTER,          "OnUploadCharacter",    "V"},
	{Scripting::TRIG_REQUEST_RESOURCE_WEIGHTS,  "OnRequestResourceWeights", "Os[s[i[i[i["},
	{Scripting::TRIG_FSQUEST_COMPLETED,         "OnForceSensitiveQuestCompleted",     "sb"},

	{Scripting::TRIG_QUEST_COMPLETED,           "OnQuestCompleted", "i"},
	{Scripting::TRIG_QUEST_CLEARED,             "OnQuestCleared",   "i"},


	//----------------------------------------------------------------------
	//-- resource harvesting

	{Scripting::TRIG_RESOURCE_HARVESTER_SET_ACTIVE,            "OnResourceHarvesterSetActive", "Oi" },
	{Scripting::TRIG_RESOURCE_HARVESTER_REQUEST_STATUS,        "OnResourceHarvesterRequestStatus", "O" },
	{Scripting::TRIG_RESOURCE_HARVESTER_REQUEST_RESOURCE_DATA, "OnResourceHarvesterRequestResourceData", "O" },

	//----------------------------------------------------------------------

	{Scripting::TRIG_OBJECT_MENU_SELECT,                         "OnObjectMenuSelect", "Oi" },
	{Scripting::TRIG_OBJECT_MENU_REQUEST,                        "OnObjectMenuRequest", "Om*" },

	// combat state triggers
	{Scripting::TRIG_GET_STATE_INFO,        "GetStateDescription",  "S*S*"},

	//----------------------------------------------------------------------
	//-- Action state triggers
	{Scripting::TRIG_WANDER_MOVING, "OnWanderMoving", ""},
	{Scripting::TRIG_WANDER_WAYPOINT, "OnWanderWaypoint", ""},
	{Scripting::TRIG_WANDER_WAITING, "OnWanderWaiting", "f*"},
	{Scripting::TRIG_WANDER_PATH_NOT_FOUND, "OnWanderPathNotFound", ""},

	{Scripting::TRIG_LOITER_MOVING, "OnLoiterMoving", ""},
	{Scripting::TRIG_LOITER_WAYPOINT, "OnLoiterWaypoint", ""},
	{Scripting::TRIG_LOITER_WAITING, "OnLoiterWaiting", "f*"},

	{Scripting::TRIG_FOLLOW_TARGET_LOST, "OnFollowTargetLost", "O"},
	{Scripting::TRIG_FOLLOW_WAITING, "OnFollowWaiting", "O"},
	{Scripting::TRIG_FOLLOW_MOVING, "OnFollowMoving", "O"},
	{Scripting::TRIG_FOLLOW_PATH_NOT_FOUND, "OnFollowPathNotFound", "O"},

	{Scripting::TRIG_FLEE_TARGET_LOST, "OnFleeTargetLost", "O"},
	{Scripting::TRIG_FLEE_WAYPOINT, "OnFleeWaypoint", "O"},
	{Scripting::TRIG_FLEE_PATH_NOT_FOUND, "OnFleePathNotFound", "O"},

	{Scripting::TRIG_MOVE_PATH_COMPLETE, "OnMovePathComplete", ""},
	{Scripting::TRIG_MOVE_MOVING, "OnMoveMoving", ""},
	{Scripting::TRIG_MOVE_PATH_NOT_FOUND, "OnMovePathNotFound", ""},
	{Scripting::TRIG_MOVE_PATH_BLOCKED, "OnMovePathBlocked", ""},

	//----------------------------------------------------------------------
	//-- crafting triggers
	{Scripting::TRIG_REQUEST_DRAFT_SCHEMATICS,         "OnRequestDraftSchematics",         "Oi[f["},
	{Scripting::TRIG_CRAFTING_ADD_RESOURCE,            "OnCraftingAddResource",            "OOIi*["},
	{Scripting::TRIG_MANUFACTURING_SCHEMATIC_CREATION, "OnManufacturingSchematicCreation", "OODi*i*"},
	{Scripting::TRIG_CRAFTING_EXPERIMENT,              "OnCraftingExperiment",             "OOS[i[ii*i"},
	{Scripting::TRIG_FINALIZE_SCHEMATIC,               "OnFinalizeSchematic",              "OOD"},
	{Scripting::TRIG_MANUFACTURE_OBJECT,               "OnManufactureObject",              "OODbb"},
	{Scripting::TRIG_CRAFTING_DONE,                    "OnCraftingDone",                   "Osib"},
	{Scripting::TRIG_MAKE_CRAFTED_ITEM,                "OnMakeCraftedItem",                "ODf"},

	//-----------------------------------------------------------------------
	//-- mission triggers
	{Scripting::TRIG_GRANT_MISSION, "OnGrantMission", "O"},

	//-----------------------------------------------------------------------
	//-- monitored creature movement
	{Scripting::TRIG_MONITORED_CREATURE_MOVEMENT, "OnMonitoredCreatureMoved", "OLLf"},

	{Scripting::TRIG_VOLUME_ENTERED, "OnTriggerVolumeEntered", "sO"},
	{Scripting::TRIG_VOLUME_EXITED, "OnTriggerVolumeExited",   "sO"},

	{Scripting::TRIG_ABOUT_TO_OPEN_CONTAINER, "OnAboutToOpenContainer", "O"},
	{Scripting::TRIG_OPENED_CONTAINER, "OnOpenedContainer", "O"},
	{Scripting::TRIG_CLOSED_CONTAINER, "OnClosedContainer", "O"},
	{Scripting::TRIG_SPAWN_HEARTBEAT, "OnSpawnHeartbeat", ""},

	{Scripting::TRIG_REQUEST_SURVEY, "OnRequestSurvey", "Ou"},
	{Scripting::TRIG_REQUEST_CORESAMPLE, "OnRequestCoreSample", "Ou"},

	{Scripting::TRIG_PERMISSION_LIST_MODIFY, "OnPermissionListModify", "Ouuu"},

	{Scripting::TRIG_LOCOMOTION_CHANGED, "OnLocomotionChanged", "ii"},

	//----------------------------------------------------------------------
	//-- group triggers
	{Scripting::TRIG_GROUP_LEADER_CHANGED,      "OnGroupLeaderChanged", "OOO"},
	{Scripting::TRIG_REMOVED_FROM_GROUP,        "OnRemovedFromGroup",   "O"},
	{Scripting::TRIG_GROUP_DISBANDED,           "OnGroupDisbanded",     "O"},
	{Scripting::TRIG_ADDED_TO_GROUP,            "OnAddedToGroup",       "O"},
	{Scripting::TRIG_GROUP_FORMED,              "OnGroupFormed",        "O"},
	{Scripting::TRIG_GROUP_MEMBERS_CHANGED,     "OnGroupMembersChanged", "OO[O[O["},

	{Scripting::TRIG_LOOT_LOTTERY_SELECTED, "OnLootLotterySelected", "OO["},

	//------------------------------------------------------------------------
	//-- Universe Object triggers
	{Scripting::TRIG_UNIVERSE_COMPLETE,         "OnUniverseComplete", ""},

	//------------------------------------------------------------------------
	//-- Auction triggers
	{Scripting::TRIG_AUCTION_ITEM,              "OnAuctionItem",		"Oi*"},
	{Scripting::TRIG_REQUEST_AUCTION_FEE,       "OnRequestAuctionFee",	"OOObi*"},
	{Scripting::TRIG_QUERY_AUCTIONS,            "OnQueryAuctions",		"OO"},
	{Scripting::TRIG_MISSION_ENTRIES_CREATED,   "OnMissionEntriesCreated", "O"},

	{Scripting::TRIG_NEWBIE_TUTORIAL_RESPONSE,  "OnNewbieTutorialResponse", "s"},
	{Scripting::TRIG_IMMEDIATE_LOGOUT,          "OnImmediateLogout", ""},
	{Scripting::TRIG_SAW_ATTACK,                "OnSawAttack", "OO["},
	{Scripting::TRIG_GIVE_ITEM,                 "OnGiveItem", "OO"},

	{Scripting::TRIG_SURVEY_DATA_RECEIVED,      "OnSurveyDataReceived", "f[f[f["},

	{Scripting::TRIG_APPLY_POWERUP,             "OnApplyPowerup", "OO"},

	{Scripting::TRIG_IMAGE_DESIGN_VALIDATE,      "OnImageDesignValidate", "OOOiibssiibs[f[s[i[s"},
	{Scripting::TRIG_IMAGE_DESIGN_COMPLETED,     "OnImageDesignCompleted", "OOOiibssiibs[f[s[i[s"},
	{Scripting::TRIG_IMAGE_DESIGN_CANCELED,      "OnImageDesignCanceled", ""},

	//------------------------------------------------------------------------
	//-- Hibernation triggers

	{Scripting::TRIG_HIBERNATE_BEGIN,           "OnHibernateBegin", ""},
	{Scripting::TRIG_HIBERNATE_END,             "OnHibernateEnd", ""},

	//------------------------------------------------------------------------
	//-- Waypoint triggers
	{Scripting::TRIG_WAYPOINT_CREATED,          "OnWaypointCreated", "O"},
	{Scripting::TRIG_WAYPOINT_DESTROYED,        "OnWaypointDestroyed", "O"},
	{Scripting::TRIG_WAYPOINT_ON_GET_ATTRIBUTES, "OnWaypointGetAttributes", "OOs*[u*["},

	//------------------------------------------------------------------------
	//-- Character transfer triggers
	{Scripting::TRIG_START_CHARACTER_UPLOAD,    "OnStartCharacterUpload", "U*[i*bb"},
	{Scripting::TRIG_RECEIVE_CHARACTER_TRANSFER_STATUS_MESSAGE, "OnReceiveCharacterTransferStatusMessage", "s"},
	{Scripting::TRIG_DOWNLOAD_CHARACTER,    "OnDownloadCharacter", "U*["},

	//------------------------------------------------------------------------
	//-- Stomach update
	{Scripting::TRIG_STOMACH_UPDATE,            "OnStomachUpdate", ""},

	//------------------------------------------------------------------------
	//-- Cluster wide data manager
	{Scripting::TRIG_RECEIVE_CLUSTER_WIDE_DATA_RESPONSE, "OnClusterWideDataResponse", "ssis[V[i"},

	//------------------------------------------------------------------------
	//-- Space triggers
	{Scripting::TRIG_SHIP_HIT,                  "OnShipWasHit", "Oibiibfff"},
	{Scripting::TRIG_SHIP_HITTING,              "OnShipIsHitting", "Oii"},
	{Scripting::TRIG_SHIP_BEHAVIOR_CHANGED,     "OnShipBehaviorChanged", "ii"},
	{Scripting::TRIG_SHIP_COMPONENT_POWER_SUFFICIENT,     "OnShipComponentPowerSufficient",   "if"},
	{Scripting::TRIG_SHIP_COMPONENT_POWER_INSUFFICIENT,   "OnShipComponentPowerInsufficient", "iff"},
	{Scripting::TRIG_FORM_CREATEOBJECT,         "OnFormCreateObject", "sfffOs[s["},
	{Scripting::TRIG_FORM_EDITOBJECT,           "OnFormEditObject", "Os[s["},
	{Scripting::TRIG_FORM_REQUESTDATA,          "OnFormRequestEditObject", "Os["},

	{Scripting::TRIG_SHIP_COMPONENT_INSTALLING,     "OnShipComponentInstalling",   "OOi"},
	{Scripting::TRIG_SHIP_COMPONENT_INSTALLED,      "OnShipComponentInstalled",    "Oi"},
	{Scripting::TRIG_SHIP_COMPONENT_UNINSTALLING,   "OnShipComponentUninstalling", "OiO"},
	{Scripting::TRIG_SHIP_COMPONENT_UNINSTALLED,    "OnShipComponentUninstalled",  "OOiO"},

	{Scripting::TRIG_SHIP_COMPONENT_ITEM_DROPPED_ON_SLOT, "OnDroppedItemOntoShipComponent",  "iOO"},
	{Scripting::TRIG_SHIP_TRY_TO_EQUIP_DROID_CONTROL_DEVICE_IN_SHIP, "OnTryToEquipDroidControlDeviceInShip",  "OO"},
	{Scripting::TRIG_SHIP_HIT_BY_LIGHTNING, "OnShipHitByLightning", "if"},
	{Scripting::TRIG_SHIP_HIT_BY_ENVIRONMENT, "OnShipHitByEnvironment", "if"},
	{Scripting::TRIG_SHIP_FIRED_MISSILE, "OnShipFiredMissile", "iiiOOi"},
	{Scripting::TRIG_SHIP_TARGETED_BY_MISSILE, "OnShipTargetedByMissile", "iiiOOi"},
	{Scripting::TRIG_SHIP_FIRED_COUNTERMEASURE, "OnShipFiredCountermeasure", "iO"},

	{Scripting::TRIG_SPACE_UNIT_MOVE_TO_COMPLETE, "OnSpaceUnitMoveToComplete", ""},
	{Scripting::TRIG_SPACE_UNIT_BEHAVIOR_CHANGED, "OnSpaceUnitBehaviorChanged", "ii"},
	{Scripting::TRIG_SPACE_UNIT_FOLLOW_LOST, "OnSpaceUnitFollowLost", "O"},
	{Scripting::TRIG_SPACE_UNIT_DOCKED, "OnSpaceUnitDocked", "O"},
	{Scripting::TRIG_SPACE_UNIT_UNDOCKED, "OnSpaceUnitUnDocked", "Ob"},
	{Scripting::TRIG_SPACE_UNIT_ENTER_COMBAT, "OnSpaceUnitEnterCombat", "O"},
	{Scripting::TRIG_ABOUT_TO_LAUNCH_TO_SPACE, "OnAboutToLaunchIntoSpace", "OOO[ss"},
	{Scripting::TRIG_SPACE_UNIT_START_UNDOCK, "OnSpaceUnitStartUnDock", "O"},
	{Scripting::TRIG_SPACE_EJECT_PLAYER_FROM_SHIP, "OnSpaceEjectPlayerFromShip", ""},
	{Scripting::TRIG_AI_PRIMARY_WEAPON_EQUIPPED, "OnAiPrimaryWeaponEquipped", "O"},
	{Scripting::TRIG_AI_SECONDARY_WEAPON_EQUIPPED, "OnAiSecondaryWeaponEquipped", "O"},
	{Scripting::TRIG_AI_RETREAT_COMPLETE, "OnAiTetherComplete", ""},
	{Scripting::TRIG_HATE_TARGET_CHANGED, "OnHateTargetChanged", "O"},
	{Scripting::TRIG_HATE_TARGET_ADDED, "OnHateTargetAdded", "O"},
	{Scripting::TRIG_HATE_TARGET_REMOVED, "OnHateTargetRemoved", "O"},
	{Scripting::TRIG_AI_RETREAT_START, "OnAiTetherStart", ""},
	{Scripting::TRIG_INVULNERABLE_CHANGED, "OnInvulnerableChanged", "b"},
	{Scripting::TRIG_AI_COMBAT_FRAME, "OnAiCombatFrame", ""},

	{Scripting::TRIG_PROGRAM_DROID_COMMANDS, "OnCommitDroidProgramCommands", "Os[O[O["},

	{Scripting::TRIG_SHIP_INTERNAL_DAMAGE_OVER_TIME_REMOVED, "OnShipInternalDamageOverTimeRemoved", "iff"},
	{Scripting::TRIG_SHIP_DAMAGED_BY_INTERNAL_DAMAGE_OVER_TIME, "OnShipDamagedByInternalDamageOverTime", "ifff"},

	//----------------------------------------------------------------------
	//-- Chat
	{Scripting::TRIG_CHAT_ON_LOGIN,             "OnChatLogin", ""},

	{Scripting::TRIG_CONTAINER_CHILD_GAIN_ITEM, "OnContainerChildGainItem", "OOO"},
	{Scripting::TRIG_CONTAINER_CHILD_LOST_ITEM, "OnContainerChildLostItem", "OOOO"},

	{Scripting::TRIG_QUEST_ACTIVATED, "OnQuestActivated", "i"},

	{Scripting::TRIG_CRAFTED_PROTOTYPE, "OnCraftedPrototype", "OD"},

	{Scripting::TRIG_ENTER_REGION, "OnEnterRegion", "ss"},
	{Scripting::TRIG_EXIT_REGION, "OnExitRegion", "ss"},
	{Scripting::TRIG_ON_PLAYER_REPORTED_CHAT, "OnPlayerReportedChat", "Ou"},

	//----------------------------------------------------------------------
	{Scripting::TRIG_VENDOR_ITEM_COUNT_REPLY, "OnVendorItemCountReply", "ii"},
	{Scripting::TRIG_VENDOR_STATUS_CHANGE, "OnVendorStatusChange", "i"},
	{Scripting::TRIG_PLAYER_VENDOR_COUNT_REPLY, "OnPlayerVendorCountReply", "i"},
 
	{Scripting::TRIG_TASK_ACTIVATED, "OnTaskActivated", "ii"},
	{Scripting::TRIG_TASK_COMPLETED, "OnTaskCompleted", "ii"},
	{Scripting::TRIG_TASK_FAILED, "OnTaskFailed", "ii"},
	{Scripting::TRIG_TASK_CLEARED, "OnTaskCleared", "ii"},
	{Scripting::TRIG_BEGIN_WARMUP, "OnBeginWarmup", "siif[sO" },

	//----------------------------------------------------------------------

	{Scripting::TRIG_BUILDOUT_OBJECT_REGISTER_WITH_CONTROLLER, "OnBuildoutObjectRegisterWithController", "O"},
	{Scripting::TRIG_CYBERNETIC_CHANGE_REQUEST, "OnCyberneticChangeRequest", "OiO" },

	{Scripting::TRIG_SOME_TASK_ACTIVATED, "OnSomeTaskActivated", "ii"},
	{Scripting::TRIG_SOME_TASK_COMPLETED, "OnSomeTaskCompleted", "ii"},
	{Scripting::TRIG_SOME_TASK_FAILED, "OnSomeTaskFailed", "ii"},
	{Scripting::TRIG_SOME_TASK_CLEARED, "OnSomeTaskCleared", "ii"},

	// params: player, ship, station, resource, amount
	{Scripting::TRIG_SPACE_MINING_SELL_RESOURCE, "OnSpaceMiningSellResource", "OOOOi"},

	{Scripting::TRIG_UNSTICKING, "OnUnsticking", ""},

	{Scripting::TRIG_REQUEST_STATIC_ITEM_DATA, "OnRequestStaticItemData", "s"},
	
	//----------------------------------------------------------------------
	//-- Skills
	{Scripting::TRIG_SKILL_TEMPLATE_CHANGED, "OnSkillTemplateChanged", "sb"},
	{Scripting::TRIG_WORKING_SKILL_CHANGED, "OnWorkingSkillChanged", "sb"},
	{Scripting::TRIG_COMBAT_LEVEL_CHANGED, "OnCombatLevelChanged", "ii"},

	{Scripting::TRIG_QUEST_GRANT_REWARD, "OnQuestReceivedReward", "is"},

	{Scripting::TRIG_GET_STATIC_ITEM_ATTRIBS, "OnGetStaticItemsAttributes",      "ss*[u*["},
	
	{Scripting::TRIG_HYPERSPACE_TO_HOME_LOCATION, "OnHyperspaceToHomeLocation", ""},

	//----------------------------------------------------------------------
	//-- CS stuff
	{Scripting::TRIG_CS_CREATE_STATIC_ITEM, "OnCSCreateItem", "Os"},

	{Scripting::TRIG_ON_CUSTOMIZE_FINISHED, "OnCustomizeFinished",  "Os"},

	{Scripting::TRIG_PVP_RANKING_CHANGED, "OnPvpRankingChanged", "ii"},

    {Scripting::TRIG_ON_SET_GOD_MODE_ON, "OnSetGodModeOn", ""},
    {Scripting::TRIG_ON_SET_GOD_MODE_OFF, "OnSetGodModeOff", ""},


	//-----------------------------------------------------------------------
	//-- buff builder
	{Scripting::TRIG_BUFF_BUILDER_VALIDATE,      "OnBuffBuilderValidate",   "OOiiibs[i["},
	{Scripting::TRIG_BUFF_BUILDER_COMPLETED,     "OnBuffBuilderCompleted", "OOiiibs[i["},
	{Scripting::TRIG_BUFF_BUILDER_CANCELED,      "OnBuffBuilderCanceled", ""},

	//----------------------------------------------------------------------
	//-- Building packing
	{Scripting::TRIG_BUILDING_CONTENTS_LOADED, "OnContentsLoadedForBuildingToBePacked", "O"},

	//-----------------------------------------------------------------------
	//-- incubator
	{Scripting::TRIG_INCUBATOR_COMMITTED, "OnIncubatorCommitted", "OOOOOOiiiiiiiiiiiiiii"},
	{Scripting::TRIG_INCUBATOR_CANCELED, "OnIncubatorCancelled","OO"},

	//-----------------------------------------------------------------------
	//-- collection
	{Scripting::TRIG_COLLECTION_SLOT_MODIFIED, "OnCollectionSlotModified", "ssssbiiib"},
	{Scripting::TRIG_COLLECTION_SERVER_FIRST, "OnCollectionServerFirst", "sss"},

	//-----------------------------------------------------------------------
	//-- veteran reward
	{Scripting::TRIG_CREATE_VETERAN_REWARD, "DoCreateVeteranReward", "s*["},

	//-----------------------------------------------------------------------
	//-- Storyteller location trigger
	{Scripting::TRIG_GROUND_TARGET_LOC, "OnGroundTargetLoc", "Oifff"},

	// Duel Trigger
	{Scripting::TRIG_ON_DUEL_REQUEST, "OnDuelRequest","OO"},
	{Scripting::TRIG_ON_DUEL_START, "OnDuelStart", "OO"},

	{Scripting::TRIG_ON_ABOUT_TO_CHANGE_APPEARANCE, "OnAboutToChangeAppearance","Os"},
	{Scripting::TRIG_ON_ABOUT_TO_REVERT_APPEARANCE, "OnAboutToRevertAppearance","O"},
	
	// Group pickup point travel
	{Scripting::TRIG_ABOUT_TO_TRAVEL_TO_GROUP_PICKUP_POINT, "OnAboutToTravelToGroupPickupPoint", ""},
	{Scripting::TRIG_TRAVEL_TO_GROUP_PICKUP_POINT, "OnTravelToGroupPickupPoint", "ss"},

	{Scripting::TRIG_CHANGED_APPEARANCE, "OnChangedAppearance", "O"},
	{Scripting::TRIG_REVERTED_APPEARANCE, "OnRevertedAppearance", "O"},

	{Scripting::TRIG_ON_DYNAMIC_SPAWN_REGION_CREATED, "OnDynamicSpawnRegionCreated", "Osfff"},

	{Scripting::TRIG_DO_RESTORE_ITEM_DECORATION_LAYOUT, "DoRestoreItemDecorationLayout", "OOLbffff"},
	{Scripting::TRIG_DO_RESTORE_ITEM_DECORATION_LAYOUT_ROTATION_ONLY, "DoRestoreItemDecorationLayoutRotationOnly", "Offff"},

	{Scripting::TRIG_ON_RATING_FINISHED, "OnRatingFinished", "i"},
	{Scripting::TRIG_ON_ABANDON_PLAYER_QUEST, "OnAbandonPlayerQuest", "O"},

	{Scripting::TRIG_ON_GCW_SCORE_CATEGORY_PERCENTILE_CHANGE, "OnGcwScoreCategoryPercentileChange", "sii"},
	
	{Scripting::TRIG_WAYPOINT_WARP, "OnWaypointWarpRequested", "O"},

//-- finish it up

	{Scripting::TRIG_LAST_TRIGGER,			nullptr,					nullptr}
};

const int NUM_SCRIPT_FUNCTIONS = sizeof(ScriptFuncList) / sizeof(ScriptFuncList[0]) - 1;


//========================================================================
// globals
//========================================================================

Scripting::_ScriptFuncHashMap *Scripting::ScriptFuncHashMap = nullptr;


//========================================================================
// initialize the ScriptFuncHashMap
//========================================================================

/**
 * Initialize the ScriptFuncHashMap
 */
void Scripting::InitScriptFuncHashMap(void)
{
    Scripting::ScriptFuncHashMap = new Scripting::_ScriptFuncHashMap(NUM_SCRIPT_FUNCTIONS);

    for (int i = 0; i < NUM_SCRIPT_FUNCTIONS; ++i)
		IGNORE_RETURN(Scripting::ScriptFuncHashMap->insert(std::make_pair(static_cast<int>(ScriptFuncList[i].id), &ScriptFuncList[i])));
}	// InitScriptFuncHashMap

/**
 * Cleans up memory.
 */
void Scripting::RemoveScriptFuncHashMap(void)
{
    delete Scripting::ScriptFuncHashMap;
    Scripting::ScriptFuncHashMap = nullptr;
}

