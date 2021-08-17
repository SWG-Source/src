//========================================================================
//
// ScriptFuncTable.h - Default values for initializing the script function list.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ScriptFuncTable_H
#define _INCLUDED_ScriptFuncTable_H

#include <unordered_map>

namespace Scripting
{

	/* This is the id list for supported trigger types.
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * When you add to this list, make sure you enter your trigger in 
	 * ScriptFunctionTable.cpp
	 * base_class.java
	 * base_script.script
	 * =================================================================
	 * I MEAN IT!
	 */
enum TrigId
{
	TRIG_ATTACH = 0,
	TRIG_DETACH = 1,
	TRIG_SPEAKING = 2,
	TRIG_HEAR_SPEECH_STRING = 3,
	TRIG_HEAR_SPEECH_STRINGID = 4,
	TRIG_INSPECTED = 5,
	TRIG_LOGOUT = 6,
	TRIG_LOGIN = 7,
	TRIG_ITEM_EQUIP = 8,
	TRIG_ITEM_UNEQUIP = 9,
	TRIG_TARGET_EQUIPS_ITEM = 10,
	TRIG_TARGET_UNEQUIPS_ITEM = 11,
	TRIG_ABOUT_TO_RECEIVE_ITEM = 12,
	TRIG_RECEIVED_ITEM = 13,
	TRIG_ABOUT_TO_BE_XFERRED = 14,
	TRIG_XFERRED = 15,
	TRIG_ABOUT_TO_LOSE_ITEM = 16,
	TRIG_LOST_ITEM = 17,
	TRIG_LOADED_FROM_DB = 18,
	TRIG_UNLOADED_FROM_MEMORY = 19,
	TRIG_DECAY = 20,
	TRIG_DESTROY = 21,
	TRIG_SERVER_TRANSFER = 22,
	TRIG_MADE_AUTHORITATIVE = 23,
	TRIG_INCAPACITATED = 24,
	TRIG_ABOUT_TO_DIE = 25,
	TRIG_DEATH = 26,
//	TRIG_COMBAT_LOOP = 27,
	TRIG_DEFENDER_COMBAT_ACTION = 28,
	TRIG_ATTACKER_COMBAT_ACTION = 29,
	TRIG_WEAPON_COMBAT_ACTION = 30,
//	TRIG_WAS_HIT = 31,
	TRIG_INCAPACITATE_TARGET = 32,
//	TRIG_SAW_INCAPACITATION = 33,
//	TRIG_SAW_DEATH = 34,
//	TRIG_SAW_ATTACK = 35,
	//TRIG_FUMBLE = 36,
	TRIG_ADDED_TO_WORLD = 37,
	TRIG_REMOVING_FROM_WORLD = 38,
	TRIG_PRELOAD_COMPLETE = 39,
	//TRIG_REPATH_COMPLETE = 40,
	TRIG_GET_STATE_INFO = 41,
	TRIG_BEHAVIOR_CHANGE = 46,
	//TRIG_PATH_COMPLETE = 47,
	//TRIG_TARGET_LOST = 48,
	TRIG_START_NPC_CONVERSATION = 49,
	TRIG_END_NPC_CONVERSATION = 50,
	TRIG_NPC_CONVERSATION_RESPONSE = 51,
	TRIG_TARGET_CHANGE = 52,
	TRIG_RESOURCE_HARVESTER_SET_ACTIVE = 53,
	TRIG_RESOURCE_HARVESTER_REQUEST_STATUS = 54,
	TRIG_RESOURCE_HARVESTER_REQUEST_RESOURCE_DATA = 55,
	TRIG_OBJECT_MENU_SELECT = 56,
	TRIG_OBJECT_MENU_REQUEST = 57,
//	TRIG_MY_WEAPON_IS_MISSING = 58,
//	TRIG_COMBAT_QUEUE_EMPTIED = 59,
//	TRIG_WAS_MISSED = 60,
//	TRIG_IS_MISSING = 61,
	TRIG_RECAPACITATED = 62,
	TRIG_WANDER_MOVING = 63,
	TRIG_WANDER_WAYPOINT = 64,
	TRIG_WANDER_WAITING = 65,
	TRIG_WANDER_PATH_NOT_FOUND = 66,
	TRIG_LOITER_MOVING = 67,
	TRIG_LOITER_WAYPOINT = 68,
	TRIG_LOITER_WAITING = 69,
	TRIG_FOLLOW_TARGET_LOST = 75,
	TRIG_FOLLOW_WAITING = 76,
	TRIG_FOLLOW_MOVING = 77,
	TRIG_FOLLOW_PATH_NOT_FOUND = 79,
	TRIG_FLEE_TARGET_LOST = 80,
	TRIG_FLEE_WAYPOINT = 81,
	TRIG_FLEE_PATH_NOT_FOUND = 48,
	TRIG_MOVE_PATH_COMPLETE = 47,
	TRIG_MOVE_MOVING = 40,
	TRIG_MOVE_PATH_NOT_FOUND = 36,
	TRIG_INITIALIZE = 82,
	TRIG_REQUEST_DRAFT_SCHEMATICS = 83,
	TRIG_MANUFACTURING_SCHEMATIC_CREATION = 84,
	TRIG_CRAFTING_EXPERIMENT = 85,
	TRIG_MANUFACTURE_OBJECT = 86,
	TRIG_CRAFTING_DONE = 87,
	TRIG_ARRIVE_AT_LOCATION = 88,
//	TRIG_EXITED_LOCATION = 89,
	TRIG_GRANT_MISSION = 90,
	TRIG_COMBAT_TARGETED = 91,
	TRIG_COMBAT_UNTARGETED = 92,
	TRIG_PLAYER_REQUEST_MISSION_BOARD = 93,
	TRIG_CRAFTING_ADD_RESOURCE = 94,
	TRIG_MONITORED_CREATURE_MOVEMENT = 96,
	TRIG_OUT_OF_AMMO = 97,
	TRIG_OBJECT_DISABLED = 98,
	TRIG_VOLUME_ENTERED = 99,
	TRIG_VOLUME_EXITED = 100,
	TRIG_MODIFY_WEAPON_DAMAGE = 102,
	TRIG_ENTERED_COMBAT = 103,
	TRIG_EXITED_COMBAT = 104,
	TRIG_CHANGED_POSTURE = 105,
	TRIG_COMBAT_DAMAGED = 106,
	TRIG_COMBAT_LOST_TARGET = 107,
	TRIG_SAW_EMOTE = 108,
	TRIG_OPENED_CONTAINER = 110,
	TRIG_CLOSED_CONTAINER = 111,
	TRIG_SPAWN_HEARTBEAT = 112,
	TRIG_PVP_TYPE_CHANGED = 113,
	TRIG_PVP_FACTION_CHANGED = 114,
	TRIG_CITY_CHANGED = 115,
	TRIG_ENTER_SWIMMING = 116,
	TRIG_EXIT_SWIMMING = 117,
	TRIG_PLACE_STRUCTURE = 118,
	TRIG_ABOUT_TO_OPEN_CONTAINER = 119,
	TRIG_REQUEST_SURVEY = 120,
	TRIG_PERMISSION_LIST_MODIFY = 121,
	TRIG_PURCHASE_TICKET = 122,
	TRIG_GROUP_LEADER_CHANGED = 123,
	TRIG_REMOVED_FROM_GROUP = 124,
	TRIG_GROUP_DISBANDED = 125,
	TRIG_ADDED_TO_GROUP = 126,
	TRIG_GROUP_FORMED = 127,
	TRIG_GET_ATTRIBS = 128,
	TRIG_REQUEST_CORESAMPLE = 129,
	TRIG_UNIVERSE_COMPLETE = 130,
	TRIG_AUCTION_ITEM = 131,
	TRIG_MISSION_ENTRIES_CREATED = 132,
	TRIG_NEWBIE_TUTORIAL_RESPONSE = 133,
	TRIG_IMMEDIATE_LOGOUT = 134,
	TRIG_CREATURE_DAMAGED = 135,
	TRIG_OBJECT_DAMAGED = 136,
	TRIG_SAW_ATTACK = 137,
	TRIG_GIVE_ITEM = 138,
	TRIG_SURVEY_DATA_RECEIVED = 139,
	TRIG_APPLY_POWERUP = 140,
	TRIG_IMAGE_DESIGN_VALIDATE = 141,
	TRIG_IMAGE_DESIGN_COMPLETED = 142,
	TRIG_HIBERNATE_BEGIN = 143,
	TRIG_HIBERNATE_END = 144,
	TRIG_SOCKET_USED = 145,
	TRIG_QUERY_AUCTIONS = 146,
	TRIG_REQUEST_AUCTION_FEE = 147,
//	TRIG_IMAGE_DESIGN_REQUESTCONSENT = 148,
	TRIG_GET_RESPAWN_LOC = 149,
	TRIG_WAYPOINT_CREATED = 150,
	TRIG_WAYPOINT_DESTROYED = 151,
	TRIG_WAYPOINT_ON_GET_ATTRIBUTES = 152,
	TRIG_FINALIZE_SCHEMATIC = 153,
	TRIG_SKILL_GRANTED = 154,
	TRIG_SKILL_REVOKED = 155,
	TRIG_MAKE_CRAFTED_ITEM = 156,
	TRIG_START_CHARACTER_UPLOAD = 157,
	TRIG_RECEIVE_CHARACTER_TRANSFER_STATUS_MESSAGE = 158,
	TRIG_LOCATION_RECEIVED = 159,
	TRIG_ATTRIB_MOD_DONE = 160,
	TRIG_SKILL_MOD_DONE = 161,
	TRIG_STOMACH_UPDATE = 162,
	TRIG_RECEIVE_CLUSTER_WIDE_DATA_RESPONSE = 163,
	TRIG_IMAGE_DESIGN_CANCELED = 164,
	TRIG_DOWNLOAD_CHARACTER = 165,
	TRIG_PERFORM_EMOTE = 166,
	TRIG_CHAT_ON_LOGIN = 167,
	TRIG_ABOUT_TO_BE_INCAPACITATED = 168,
	TRIG_SKILL_ABOUT_TO_BE_GRANTED = 169,
	TRIG_SKILL_ABOUT_TO_BE_REVOKED = 170,
	TRIG_CONTAINER_CHILD_GAIN_ITEM = 171,
	TRIG_CONTAINER_CHILD_LOST_ITEM = 172,
	TRIG_QUEST_ACTIVATED = 173,
	TRIG_CRAFTED_PROTOTYPE = 174,
	TRIG_THEATER_CREATED = 175,
	TRIG_ENTER_REGION = 176,
	TRIG_EXIT_REGION = 177,
	TRIG_VENDOR_ITEM_COUNT_REPLY = 178,
	TRIG_VENDOR_STATUS_CHANGE = 179,
	TRIG_PLAYER_THEATER_FAIL = 180,
	TRIG_GRANT_SCHEMATIC = 181,
	TRIG_REVOKE_SCHEMATIC = 182,
	TRIG_PLAYER_VENDOR_COUNT_REPLY = 183,
	TRIG_SHIP_HIT = 200,
	TRIG_SHIP_HITTING = 201,
	TRIG_SHIP_BEHAVIOR_CHANGED = 202,
	TRIG_SHIP_COMPONENT_POWER_SUFFICIENT = 203,
	TRIG_SHIP_COMPONENT_POWER_INSUFFICIENT = 204,
	TRIG_FORM_CREATEOBJECT = 205,
	TRIG_FORM_EDITOBJECT = 206,
	TRIG_FORM_REQUESTDATA = 207,
	TRIG_SHIP_COMPONENT_INSTALLING = 208,
	TRIG_SHIP_COMPONENT_INSTALLED = 209,
	TRIG_SHIP_COMPONENT_UNINSTALLING = 210,
	TRIG_SHIP_COMPONENT_UNINSTALLED = 211,
	TRIG_SHIP_COMPONENT_ITEM_DROPPED_ON_SLOT = 212,
	TRIG_SHIP_TRY_TO_EQUIP_DROID_CONTROL_DEVICE_IN_SHIP = 213,
	TRIG_SHIP_HIT_BY_LIGHTNING = 214,
	TRIG_SHIP_HIT_BY_ENVIRONMENT = 215,
	TRIG_SHIP_FIRED_MISSILE = 216,
	TRIG_SHIP_TARGETED_BY_MISSILE = 217,
	TRIG_SHIP_FIRED_COUNTERMEASURE = 218,
	TRIG_SPACE_UNIT_MOVE_TO_COMPLETE = 219,
	TRIG_SPACE_UNIT_BEHAVIOR_CHANGED = 220,
	TRIG_SPACE_UNIT_FOLLOW_LOST = 221,
	TRIG_SHIP_INTERNAL_DAMAGE_OVER_TIME_REMOVED = 222,
	TRIG_SHIP_DAMAGED_BY_INTERNAL_DAMAGE_OVER_TIME = 223,
	TRIG_SPACE_UNIT_DOCKED = 224,
	TRIG_SPACE_UNIT_UNDOCKED = 225,
	TRIG_SPACE_UNIT_ENTER_COMBAT = 226,
	TRIG_ABOUT_TO_LAUNCH_TO_SPACE = 227,
	TRIG_SPACE_UNIT_START_UNDOCK = 228,
	TRIG_SPACE_EJECT_PLAYER_FROM_SHIP = 229,
	TRIG_TASK_ACTIVATED = 230,
	TRIG_TASK_COMPLETED = 231,
	TRIG_TASK_FAILED = 232,
	TRIG_TASK_CLEARED = 233,
	TRIG_PROGRAM_DROID_COMMANDS = 234,
	TRIG_START_CONVERSATION = 235,
	TRIG_END_CONVERSATION = 236,
	TRIG_BEGIN_WARMUP = 237,
	TRIG_LOOT_LOTTERY_SELECTED = 238,
	TRIG_AI_PRIMARY_WEAPON_EQUIPPED = 239,
	TRIG_AI_SECONDARY_WEAPON_EQUIPPED = 240,
	TRIG_BUILDOUT_OBJECT_REGISTER_WITH_CONTROLLER = 241,
	TRIG_CYBERNETIC_CHANGE_REQUEST = 242,
	TRIG_SOME_TASK_ACTIVATED = 243,
	TRIG_SOME_TASK_COMPLETED = 244,
	TRIG_SOME_TASK_FAILED = 245,
	TRIG_SOME_TASK_CLEARED = 246,
	TRIG_SPACE_MINING_SELL_RESOURCE = 247,
	TRIG_AI_RETREAT_COMPLETE = 248,
	TRIG_HATE_TARGET_CHANGED = 249,
	TRIG_HATE_TARGET_ADDED = 250,
	TRIG_HATE_TARGET_REMOVED = 251,
	TRIG_AI_RETREAT_START = 252,
	TRIG_DO_STRUCTURE_ROLLUP = 253,
	TRIG_GET_SCRIPTVARS = 254,
	//TRIG_DEFAULT_ATTACK = 255,
	TRIG_INVULNERABLE_CHANGED = 256,
	TRIG_UNSTICKING = 257,
	TRIG_AI_COMBAT_FRAME = 258,
	TRIG_UPLOAD_CHARACTER = 259,
	TRIG_REQUEST_RESOURCE_WEIGHTS = 260,
	TRIG_FSQUEST_COMPLETED = 261,
	TRIG_MOVE_PATH_BLOCKED = 262,
	TRIG_QUEST_COMPLETED = 263,
	TRIG_QUEST_CLEARED = 264,
	TRIG_GROUP_MEMBERS_CHANGED= 265,
	TRIG_ENVIRONMENTAL_DEATH=266,
	TRIG_LOCOMOTION_CHANGED= 267,
	TRIG_REQUEST_STATIC_ITEM_DATA = 268,
	TRIG_SKILL_TEMPLATE_CHANGED = 269,
	TRIG_WORKING_SKILL_CHANGED = 270,
	TRIG_COMBAT_LEVEL_CHANGED = 271,
	TRIG_QUEST_GRANT_REWARD = 272,
	TRIG_SKILLMODS_CHANGED = 273,
	TRIG_GET_STATIC_ITEM_ATTRIBS = 274,
	TRIG_PURCHASE_TICKET_INSTANT_TRAVEL = 275,
	TRIG_HYPERSPACE_TO_HOME_LOCATION = 276,
	TRIG_CS_CREATE_STATIC_ITEM = 277,
	TRIG_ON_CUSTOMIZE_FINISHED = 278,
	TRIG_PVP_RANKING_CHANGED = 279,
	TRIG_BUFF_BUILDER_VALIDATE = 280,
	TRIG_BUFF_BUILDER_COMPLETED = 281,
	TRIG_BUFF_BUILDER_CANCELED = 282,
	TRIG_BUILDING_CONTENTS_LOADED = 283,
	TRIG_INCUBATOR_COMMITTED = 284,
	TRIG_INCUBATOR_CANCELED = 285,
	TRIG_COLLECTION_SLOT_MODIFIED = 286,
	TRIG_COLLECTION_SERVER_FIRST = 287,
	TRIG_CREATE_VETERAN_REWARD = 288,
	TRIG_GROUND_TARGET_LOC = 289,
	TRIG_ON_DUEL_REQUEST = 290,
	TRIG_ON_DUEL_START = 291,
	TRIG_ON_ABOUT_TO_CHANGE_APPEARANCE = 292,
	TRIG_ON_ABOUT_TO_REVERT_APPEARANCE = 293,
	TRIG_ABOUT_TO_TRAVEL_TO_GROUP_PICKUP_POINT = 294,
	TRIG_TRAVEL_TO_GROUP_PICKUP_POINT = 295,
	TRIG_CHANGED_APPEARANCE = 296,
	TRIG_REVERTED_APPEARANCE = 297,
	TRIG_ON_DYNAMIC_SPAWN_REGION_CREATED = 298,
	TRIG_DO_RESTORE_ITEM_DECORATION_LAYOUT = 299,
	TRIG_DO_RESTORE_ITEM_DECORATION_LAYOUT_ROTATION_ONLY = 300,
	TRIG_ON_RATING_FINISHED = 301,
	TRIG_ON_ABANDON_PLAYER_QUEST = 302,
	TRIG_ON_GCW_SCORE_CATEGORY_PERCENTILE_CHANGE = 303,
	TRIG_WAYPOINT_WARP = 304,
	TRIG_ON_PLAYER_REPORTED_CHAT = 305,
	TRIG_ON_SET_GOD_MODE_ON = 306,
	TRIG_ON_SET_GOD_MODE_OFF = 307,
	TRIG_LAST_TRIGGER
};

// This is a sample object for describing the specifics of a supported trigger.
// It contains the Trigger id, the name of the function to be invoked for that
// trigger, and the argument type list. Argument types are described using the
// Python specification without the return type.  All triggers are assumed to
// return an int indicating stop or continue.  e.g. "is" would specify that the
// function took an integer for the first parameter, and a string for the second.
struct ScriptFuncTable
{
	TrigId id;
	const char *name;
	const char *argList;
};

typedef std::unordered_map< int, const ScriptFuncTable * > _ScriptFuncHashMap;
extern _ScriptFuncHashMap *ScriptFuncHashMap;
extern void InitScriptFuncHashMap(void);
extern void RemoveScriptFuncHashMap(void);

}

#endif	// _INCLUDED_ScriptFuncTable_H
