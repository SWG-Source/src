#ifndef COMMON_API_STRINGS_H
#define COMMON_API_STRINGS_H

#include <map>
#include <string>
#include "CommonAPI.h"

#define result_text std::pair<apiResult,const char *>
#define gamecode_text std::pair<apiGamecode,const char *>
#define text_gamecode std::pair<std::string, apiGamecode>

static const int _ResultCount = 38;
static const result_text _resultString[_ResultCount] = 
{  
    result_text(RESULT_SUCCESS,                         "RESULT_SUCCESS"),
    result_text(RESULT_CANCELLED,                       "RESULT_CANCELLED"),
    result_text(RESULT_TIMEOUT,                         "RESULT_TIMEOUT"),
    result_text(RESULT_FUNCTION_DEPRICATED,             "RESULT_FUNCTION_DEPRICATED"),
    result_text(RESULT_FUNCTION_NOT_SUPPORTED,          "RESULT_FUNCTION_NOT_SUPPORTED"),
    result_text(RESULT_INVALID_NAME_OR_PASSWORD,        "RESULT_INVALID_NAME_OR_PASSWORD"),
    result_text(RESULT_INVALID_ACCOUNT_ID,              "RESULT_INVALID_USER_ID"),
    result_text(RESULT_INVALID_SESSION,                 "RESULT_INVALID_SESSION"),
    result_text(RESULT_INVALID_PARENT_SESSION,          "RESULT_INVALID_PARENT_SESSION"),
    result_text(RESULT_INVALID_SESSION_TYPE,            "RESULT_INVALID_SESSION_TYPE"),
    result_text(RESULT_INVALID_GAMECODE,                "RESULT_INVALID_GAMECODE"),
    result_text(RESULT_REQUIRES_VALID_SUBSCRIPTION,     "RESULT_REQUIRES_VALID_SUBSCRIPTION"),
    result_text(RESULT_REQUIRES_ACTIVE_ACCOUNT,         "RESULT_REQUIRES_ACTIVE_ACCOUNT"),
    result_text(RESULT_REQUIRES_CLIENT_LOGOUT,          "RESULT_REQUIRES_CLIENT_LOGOUT"),
    result_text(RESULT_SESSION_ALREADY_CONSUMED,        "RESULT_SESSION_ALREADY_CONSUMED"),
    result_text(RESULT_REQUIRES_SECURE_ID_NEXT_CODE,    "RESULT_REQUIRES_SECURE_ID_NEXT_CODE"),
    result_text(RESULT_REQUIRES_SECURE_ID_NEW_PIN,      "RESULT_REQUIRES_SECURE_ID_NEW_PIN"),
    result_text(RESULT_SECURE_ID_PIN_ACCEPTED,          "RESULT_SECURE_ID_PIN_ACCEPTED"),
    result_text(RESULT_SECURE_ID_PIN_REJECTED,          "RESULT_SECURE_ID_PIN_REJECTED"),
    result_text(RESULT_USAGE_LIMIT_DENIED_LOGIN,        "RESULT_USAGE_LIMIT_DENIED_LOGIN"),
    result_text(RESULT_INVALID_FEATURE,					"RESULT_INVALID_FEATURE"),
    result_text(RESULT_INVALID_AVATAR,                  "RESULT_INVALID_AVATAR"),
    result_text(RESULT_INVALID_CHATROOM,                "RESULT_INVALID_CHATROOM"),
    result_text(RESULT_INVALID_MESSAGE,                 "RESULT_INVALID_MESSAGE"),
    result_text(RESULT_INVALID_FANCLUB,                 "RESULT_INVALID_FANCLUB"),
    result_text(RESULT_INVALID_ROOM_BANNER,             "RESULT_INVALID_ROOM_BANNER"),
    result_text(RESULT_REQUIRES_FANCLUB_MEMBERSHIP,     "RESULT_REQUIRES_FANCLUB_MEMBERSHIP"),
    result_text(RESULT_REQUIRES_ONLINE_AVATAR,          "RESULT_REQUIRES_ONLINE_AVATAR"),
    result_text(RESULT_REQUIRES_MODERATOR_PRIVILEGES,   "RESULT_REQUIRES_MODERATOR_PRIVILEGES"),
    result_text(RESULT_REQUIRES_VALID_MODERATION_STATE, "RESULT_REQUIRES_VALID_MODERATION_STATE"),
    result_text(RESULT_BANNED_FROM_ROOM,                "RESULT_BANNED_FROM_ROOM"),
	result_text(RESULT_INVALID_NAMESPACE,               "RESULT_INVALID_NAMESPACE"),
    result_text(RESULT_INVALID_CLIENT_IP,               "RESULT_INVALID_CLIENT_IP"),
    result_text(RESULT_USER_LOCKOUT,                    "RESULT_USER_LOCKOUT"),
	result_text(RESULT_INVALID_KICK_REASON,             "RESULT_INVALID_KICK_REASON"),
	result_text(RESULT_INVALID_PLAN_ID,					"RESULT_INVALID_PLAN_ID"),
	result_text(RESULT_INVALID_FEATURE_MATCH,			"RESULT_INVALID_FEATURE_MATCH"),
	result_text(RESULT_INVALID_SUBSCRIPTION,			"RESULT_INVALID_SUBSCRIPTION"),
	
};
static std::map<apiResult,const char *> ResultString((const std::map<apiResult,const char *>::value_type *)&_resultString[0],(const std::map<apiResult,const char *>::value_type *)&_resultString[_ResultCount]);

static const result_text _resultText[_ResultCount] = 
{  
    result_text(RESULT_SUCCESS,                     "The operation completed successfully."),
    result_text(RESULT_CANCELLED,                   "The operation was cancelled by the requestor."),
    result_text(RESULT_TIMEOUT,                     "The operation could not be processed within a reasonable time. Please try again."),
    result_text(RESULT_FUNCTION_DEPRICATED,         "This operation is no longer supported."),
    result_text(RESULT_FUNCTION_NOT_SUPPORTED,      "This operation is not supported yet."),
    result_text(RESULT_INVALID_NAME_OR_PASSWORD,    "The sepecifed user name or password is invalid."),
    result_text(RESULT_INVALID_ACCOUNT_ID,          "The sepecifed account identifier is invalid."),
    result_text(RESULT_INVALID_SESSION,             "The sepecifed session identifier is invalid."),
    result_text(RESULT_INVALID_PARENT_SESSION,      "The sepecifed parent session identifier is invalid."),
    result_text(RESULT_INVALID_SESSION_TYPE,        "The sepecifed session type is invalid."),
    result_text(RESULT_INVALID_GAMECODE,            "The specified gamecode is invalid."),
    result_text(RESULT_REQUIRES_VALID_SUBSCRIPTION, "This operation requires a valid subscription to complete."),
    result_text(RESULT_REQUIRES_ACTIVE_ACCOUNT,     "RESULT_REQUIRES_ACTIVE_ACCOUNT"),
    result_text(RESULT_REQUIRES_CLIENT_LOGOUT,      "RESULT_REQUIRES_CLIENT_LOGOUT"),
    result_text(RESULT_SESSION_ALREADY_CONSUMED,    "RESULT_SESSION_ALREADY_CONSUMED"),
    result_text(RESULT_REQUIRES_SECURE_ID_NEXT_CODE,"RESULT_REQUIRES_SECURE_ID_NEXT_CODE"),
    result_text(RESULT_REQUIRES_SECURE_ID_NEW_PIN,  "RESULT_REQUIRES_SECURE_ID_NEW_PIN"),
    result_text(RESULT_SECURE_ID_PIN_ACCEPTED,      "RESULT_SECURE_ID_PIN_ACCEPTED"),
    result_text(RESULT_SECURE_ID_PIN_REJECTED,      "RESULT_SECURE_ID_PIN_REJECTED"),
    result_text(RESULT_USAGE_LIMIT_DENIED_LOGIN,        "RESULT_USAGE_LIMIT_DENIED_LOGIN"),
    result_text(RESULT_INVALID_FEATURE,					"RESULT_INVALID_FEATURE"),
    result_text(RESULT_INVALID_AVATAR,              "The specified chat avatar name is invalid."),
    result_text(RESULT_INVALID_CHATROOM,            "The specified chat room name is invalid."),
    result_text(RESULT_INVALID_MESSAGE,             "The specified chat message is invalid."),
    result_text(RESULT_INVALID_FANCLUB,             "The specified fan club is invalid."),
    result_text(RESULT_INVALID_ROOM_BANNER,         "RESULT_INVALID_ROOM_BANNER"),
    result_text(RESULT_REQUIRES_FANCLUB_MEMBERSHIP, "This operation requires membership inb the specified fan club."),
    result_text(RESULT_REQUIRES_ONLINE_AVATAR,      "RESULT_REQUIRES_ONLINE_AVATAR"),
    result_text(RESULT_REQUIRES_MODERATOR_PRIVILEGES, "RESULT_REQUIRES_MODERATOR_PRIVILEGES"),
    result_text(RESULT_REQUIRES_VALID_MODERATION_STATE, "RESULT_REQUIRES_VALID_MODERATION_STATE"),
    result_text(RESULT_BANNED_FROM_ROOM,            "RESULT_BANNED_FROM_ROOM"),
	result_text(RESULT_INVALID_NAMESPACE,           "RESULT_INVALID_NAMESPACE"),
	result_text(RESULT_INVALID_PLAN_ID,				"RESULT_INVALID_PLAN_ID"),
	result_text(RESULT_INVALID_FEATURE_MATCH,		"RESULT_INVALID_FEATURE_MATCH"),
	result_text(RESULT_INVALID_SUBSCRIPTION,		"RESULT_INVALID_SUBSCRIPTION"),
};
static std::map<apiResult,const char *> ResultText((const std::map<apiResult,const char *>::value_type *)&_resultText[0],(const std::map<apiResult,const char *>::value_type *)&_resultText[_ResultCount]);

static const char * AccountStatusString[ACCOUNT_STATUS_END] = 
{
    "ACCOUNT_STATUS_NULL",
    "ACCOUNT_STATUS_ACTIVE",
    "ACCOUNT_STATUS_CLOSED"
};

static const char * SessionTypeString[SESSION_TYPE_END] = 
{
    "SESSION_TYPE_NULL",                             
    "SESSION_TYPE_LAUNCH_PAD",
    "SESSION_TYPE_STATION_UNSECURE",
    "SESSION_TYPE_STATION_SECURE",
    "SESSION_TYPE_TANARUS",
    "SESSION_TYPE_INFANTRY",
    "SESSION_TYPE_COSMIC_RIFT",
    "SESSION_TYPE_EVERQUEST",
    "SESSION_TYPE_EVERQUEST_2",
    "SESSION_TYPE_STARWARS",
    "SESSION_TYPE_PLANETSIDE",
    "SESSION_TYPE_EVERQUEST_ONLINE_ADVENTURES_BETA",
    "SESSION_TYPE_EVERQUEST_ONLINE_ADVENTURES",
    "SESSION_TYPE_EVERQUEST_INSTANT_MESSENGER",
    "SESSION_TYPE_REALM_SERVER",
    "SESSION_TYPE_EVERQUEST_MACINTOSH",
	"SESSION_TYPE_MATRIX_ONLINE",
	"SESSION_TYPE_HARRY_POTTER",
	"SESSION_TYPE_NEO_PETS",
	"SESSION_TYPE_GOODLIFE",
	"SESSION_TYPE_EQ2_GUILDS",
	"SESSION_TYPE_SWG_JP_BETA",
	"SESSION_TYPE_MARVEL",
	"SESSION_TYPE_EQ2_JAPAN",
	"SESSION_TYPE_EQ2_TAIWAN",
	"SESSION_TYPE_EQ2_CHINA",
	"SESSION_TYPE_EQ2_KOREA",
	"SESSION_TYPE_VGD",
	"SESSION_TYPE_PIRATE",
	"SESSION_TYPE_STAR_CHAMBER",
	"SESSION_TYPE_STARGATE",
	"SESSION_TYPE_DCU_ONLINE",
	"SESSION_TYPE_NORRATH_CSG",
};

static const char * GamecodeString[GAMECODE_END] = 
{
    "GAMECODE_NULL",                                 
    "GAMECODE_STATION_PASS",
    "GAMECODE_EVERQUEST",
    "GAMECODE_EVERQUEST_2",
    "GAMECODE_STARWARS",
    "GAMECODE_PLANETSIDE",
    "GAMECODE_EVERQUEST_ONLINE_ADVENTURES_BETA",
    "GAMECODE_EVERQUEST_ONLINE_ADVENTURES",
    "GAMECODE_EVERQUEST_INSTANT_MESSENGER",
    "GAMECODE_EVERQUEST_MACINTOSH",
    "GAMECODE_MATRIX_ONLINE",
	"GAMECODE_HARRY_POTTER",
	"GAMECODE_NEO_PETS",
	"GAMECODE_GOODLIFE",
	"GAMECODE_SWG_JP_BETA",
	"GAMECODE_MARVEL",
	"GAMECODE_EQ2_JAPAN",
	"GAMECODE_EQ2_TAIWAN",
	"GAMECODE_EQ2_CHINA",
	"GAMECODE_EQ2_KOREA",
	"GAMECODE_VGD",
	"GAMECODE_PIRATE",
	"GAMECODE_STAR_CHAMBER",
	"GAMECODE_STARGATE",
	"GAMECODE_DCU_ONLINE",
	"GAMECODE_NORRATH_CSG"
};

static const char * SubscriptionStatusString[SUBSCRIPTION_STATUS_END] = 
{
    "SUBSCRIPTION_STATUS_NULL", 
    "SUBSCRIPTION_STATUS_NO_ACCOUNT",
    "SUBSCRIPTION_STATUS_ACTIVE",
    "SUBSCRIPTION_STATUS_PENDING",
    "SUBSCRIPTION_STATUS_CANCEL",
    "SUBSCRIPTION_STATUS_TRIAL_ACTIVE",
    "SUBSCRIPTION_STATUS_TRIAL_EXPIRED",
    "SUBSCRIPTION_STATUS_SUSPENDED",
    "SUBSCRIPTION_STATUS_BANNED",
    "SUBSCRIPTION_STATUS_RENTAL_ACTIVE",
    "SUBSCRIPTION_STATUS_RENTAL_CLOSED"
	"SUBSCRIPTION_STATUS_ACTIVE_COMBO",
	"SUBSCRIPTION_STATUS_BANNED_CHARGEBACK",
};

#ifdef UNIX
const std::pair<std::string, apiGamecode> _gamecodeID[GAMECODE_END] = 
{  
    text_gamecode(std::string("SP"),		GAMECODE_STATION_PASS),
    text_gamecode(std::string("EQ"),		GAMECODE_EVERQUEST),
    text_gamecode(std::string("EQ2"),		GAMECODE_EVERQUEST_2),
    text_gamecode(std::string("SWG"),		GAMECODE_STARWARS),
    text_gamecode(std::string("PS"),		GAMECODE_PLANETSIDE),
    text_gamecode(std::string("EQOA-beta"), GAMECODE_EVERQUEST_ONLINE_ADVENTURES_BETA),
    text_gamecode(std::string("EQOA"),		GAMECODE_EVERQUEST_ONLINE_ADVENTURES),
    text_gamecode(std::string("EQIM"),		GAMECODE_EVERQUEST_INSTANT_MESSENGER),
    text_gamecode(std::string("EQM"),		GAMECODE_EVERQUEST_MACINTOSH),
    text_gamecode(std::string("MXO"),		GAMECODE_MATRIX_ONLINE),
    text_gamecode(std::string("HPO"),		GAMECODE_HARRY_POTTER),
    text_gamecode(std::string("NP"),		GAMECODE_NEO_PETS),
    text_gamecode(std::string("GL"),		GAMECODE_GOODLIFE),
    text_gamecode(std::string("SWG-JP"),    GAMECODE_SWG_JP_BETA),
	text_gamecode(std::string("MRVL"),		GAMECODE_MARVEL),
	text_gamecode(std::string("EQ2-JP"),    GAMECODE_EQ2_JAPAN),
	text_gamecode(std::string("EQ2-TW"),    GAMECODE_EQ2_TAIWAN),
	text_gamecode(std::string("EQ2-CN"),    GAMECODE_EQ2_CHINA),
	text_gamecode(std::string("EQ2-KR"),    GAMECODE_EQ2_KOREA),
	text_gamecode(std::string("VGD"),		GAMECODE_VGD),
	text_gamecode(std::string("POCSG"),		GAMECODE_PIRATE),
	text_gamecode(std::string("StarChamber"),	GAMECODE_STAR_CHAMBER),
	text_gamecode(std::string("Stargate"),		GAMECODE_STARGATE),
	text_gamecode(std::string("DCO"),		GAMECODE_DCU_ONLINE),
	text_gamecode(std::string("NCSG"),		GAMECODE_NORRATH_CSG)
};
static std::map<std::string,apiGamecode> GamecodeID((const std::map<std::string,apiGamecode>::value_type *)&_gamecodeID[0],(const std::map<std::string,apiGamecode>::value_type *)&_gamecodeID[GAMECODE_END]);

const gamecode_text _gamecodeName[GAMECODE_END] = 
{  
    gamecode_text(GAMECODE_NULL,                        ""),
    gamecode_text(GAMECODE_STATION_PASS,                "SP"),
    gamecode_text(GAMECODE_EVERQUEST,                   "EQ"),
    gamecode_text(GAMECODE_EVERQUEST_2,                 "EQ2"),
    gamecode_text(GAMECODE_STARWARS,                    "SWG"),
    gamecode_text(GAMECODE_PLANETSIDE,                  "PS"),
    gamecode_text(GAMECODE_EVERQUEST_ONLINE_ADVENTURES_BETA, "EQOA-beta"),
    gamecode_text(GAMECODE_EVERQUEST_ONLINE_ADVENTURES, "EQOA"),
    gamecode_text(GAMECODE_EVERQUEST_INSTANT_MESSENGER, "EQIM"),
    gamecode_text(GAMECODE_EVERQUEST_MACINTOSH,         "EQM"),
    gamecode_text(GAMECODE_MATRIX_ONLINE,				"MXO"),
    gamecode_text(GAMECODE_HARRY_POTTER,				"HPO"),
    gamecode_text(GAMECODE_NEO_PETS,					"NP"),
    gamecode_text(GAMECODE_GOODLIFE,					"GL"),
    gamecode_text(GAMECODE_SWG_JP_BETA,					"SWG-JP"),
	gamecode_text(GAMECODE_MARVEL,						"MRVL"),
	gamecode_text(GAMECODE_EQ2_JAPAN,					"EQ2-JP"),
	gamecode_text(GAMECODE_EQ2_TAIWAN,					"EQ2-TW"),
	gamecode_text(GAMECODE_EQ2_CHINA,					"EQ2-CN"),
	gamecode_text(GAMECODE_EQ2_KOREA,					"EQ2-KR"),
	gamecode_text(GAMECODE_VGD,							"VGD"),
	gamecode_text(GAMECODE_PIRATE,						"POCSG"),
	gamecode_text(GAMECODE_STAR_CHAMBER,				"StarChamber"),
	gamecode_text(GAMECODE_STARGATE,					"Stargate"),
	gamecode_text(GAMECODE_DCU_ONLINE,					"DCO"),
	gamecode_text(GAMECODE_NORRATH_CSG,					"NCSG")
};
static std::map<apiGamecode,const char *> GamecodeName((const std::map<apiGamecode,const char *>::value_type *)&_gamecodeName[0],(const std::map<apiGamecode,const char *>::value_type *)&_gamecodeName[GAMECODE_END]);

#endif
#endif

