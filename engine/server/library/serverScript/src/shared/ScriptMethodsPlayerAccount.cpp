// ======================================================================
//
// ScriptMethodsPlayerAccount.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlayerCreationManagerServer.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/NameErrors.h"
#include "serverNetworkMessages/TransferRequestMoveValidation.h"
#include "serverNetworkMessages/VerifyAndLockName.h"
#include "serverScript/GameScriptObject.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/DataTable.h"
#include "serverUtility/FreeCtsDataTable.h"
#include "UnicodeUtils.h"

#include <unordered_set>

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsPlayerAccountNamespace
// ======================================================================

namespace ScriptMethodsPlayerAccountNamespace
{
	bool install();

	jlong        JNICALL getPlayerObject(JNIEnv *env, jobject self, jlong creature);
	jboolean     JNICALL isIgnoring(JNIEnv *env, jobject self, jlong player, jstring who);
	jboolean     JNICALL adjustLotCount(JNIEnv *env, jobject self, jlong player, jint adjustment);
	jint         JNICALL getAccountNumLots(JNIEnv *env, jobject self, jlong player);
	jint         JNICALL getMaxHousingLots(JNIEnv *env, jobject self);
	jint         JNICALL getGameFeatureBits(JNIEnv *env, jobject self, jlong player);
	jint         JNICALL getSubscriptionFeatureBits(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL isUsingAdminLogin(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL isWarden(JNIEnv *env, jobject self, jlong player);
	jobject      JNICALL getAccountTimeData(JNIEnv * env, jobject self, jlong target);
	jint         JNICALL getPlayerLastLoginTime(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL isFreeTrialAccount(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL setCompletedTutorial(JNIEnv *env, jobject self, jlong player, jboolean value);
	void         JNICALL logActivity(JNIEnv *env, jobject self, jlong player, jint activityType);
	jboolean     JNICALL isAccountQualifiedForHousePackup(JNIEnv *env, jobject self, jlong target);
	jobject      JNICALL getCharacterCtsHistory(JNIEnv * env, jobject self, jlong player);
	jobjectArray JNICALL getCharacterRetroactiveCtsObjvars(JNIEnv * env, jobject self, jlong player);
	jobjectArray JNICALL qualifyForFreeCts(JNIEnv *env, jobject self, jlong player);
	void         JNICALL validateFreeCts(JNIEnv *env, jobject self, jlong player, jstring destinationGalaxy, jstring destinationCharacterName);
	void         JNICALL performFreeCts(JNIEnv *env, jobject self, jlong player, jstring destinationGalaxy, jstring destinationCharacterName);
	void         JNICALL validateCts(JNIEnv *env, jobject self, jlong player, jstring destinationGalaxy, jstring destinationCharacterName);
	void         JNICALL performCts(JNIEnv *env, jobject self, jlong player, jstring destinationGalaxy, jstring destinationCharacterName);
	void         JNICALL validateRenameCharacter(JNIEnv *env, jobject self, jlong player, jstring newName);
	void         JNICALL renameCharacterReleaseNameReservation(JNIEnv *env, jobject self, jlong player);
	void         JNICALL renameCharacter(JNIEnv *env, jobject self, jlong player, jstring newName);
	jint         JNICALL getPlayerStationId(JNIEnv *env, jobject self, jlong player);
	jstring      JNICALL getPlayerUsername(JNIEnv *env, jobject self, jlong player);
}

//========================================================================
// install
//========================================================================

bool ScriptMethodsPlayerAccountNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsPlayerAccountNamespace::c)}
	JF("_getPlayerObject", "(J)J", getPlayerObject),
	JF("_isIgnoring", "(JLjava/lang/String;)Z", isIgnoring),
	JF("_adjustLotCount", "(JI)Z", adjustLotCount),
	JF("_getAccountNumLots", "(J)I", getAccountNumLots),
	JF("_getMaxHousingLots", "()I", getMaxHousingLots),
	JF("_getGameFeatureBits", "(J)I", getGameFeatureBits),
	JF("_getSubscriptionFeatureBits", "(J)I", getSubscriptionFeatureBits),
	JF("_isUsingAdminLogin", "(J)Z", isUsingAdminLogin),
	JF("_isWarden", "(J)Z", isWarden),
	JF("_getAccountTimeData", "(J)Lscript/dictionary;", getAccountTimeData),
	JF("_getPlayerLastLoginTime", "(J)I", getPlayerLastLoginTime),
	JF("_isFreeTrialAccount", "(J)Z", isFreeTrialAccount),
	JF("_setCompletedTutorial", "(JZ)Z", setCompletedTutorial),
	JF("__logActivity", "(JI)V", logActivity),
	JF("_isAccountQualifiedForHousePackup", "(J)Z", isAccountQualifiedForHousePackup),
	JF("_getCharacterCtsHistory", "(J)Lscript/dictionary;", getCharacterCtsHistory),
	JF("_getCharacterRetroactiveCtsObjvars", "(J)[Lscript/dictionary;", getCharacterRetroactiveCtsObjvars),
	JF("_qualifyForFreeCts", "(J)[Ljava/lang/String;", qualifyForFreeCts),
	JF("_validateFreeCts", "(JLjava/lang/String;Ljava/lang/String;)V", validateFreeCts),
	JF("_performFreeCts", "(JLjava/lang/String;Ljava/lang/String;)V", performFreeCts),
	JF("_validateCts", "(JLjava/lang/String;Ljava/lang/String;)V", validateCts),
	JF("_performCts", "(JLjava/lang/String;Ljava/lang/String;)V", performCts),
	JF("_validateRenameCharacter", "(JLjava/lang/String;)V", validateRenameCharacter),
	JF("_renameCharacterReleaseNameReservation", "(J)V", renameCharacterReleaseNameReservation),
	JF("_renameCharacter", "(JLjava/lang/String;)V", renameCharacter),
	JF("_getPlayerStationId", "(J)I", getPlayerStationId),
	JF("_getPlayerUsernameDoNotUse", "(J)Ljava/lang/String;", getPlayerUsername),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================
// Methods for dealing with PlayerObjects and (Station) account data

jlong JNICALL ScriptMethodsPlayerAccountNamespace::getPlayerObject(JNIEnv *env, jobject self, jlong creature)
{
	UNREF(self);
	NOT_NULL(env);

	jlong objId = 0;

	ServerObject * creatureServerObject = nullptr;
	if (JavaLibrary::getObject(creature,creatureServerObject))
	{
		SlotId slot = SlotIdManager::findSlotId(ConstCharCrcLowerString("ghost"));
		if (slot != SlotId::invalid)
		{
			SlottedContainer const * const slots = ContainerInterface::getSlottedContainer(*creatureServerObject);
			if (slots)
			{
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				CachedNetworkId playerObjectId = slots->getObjectInSlot(slot, tmp);
				objId = playerObjectId.getValue();
			}
		}
	}
	return objId;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlayerAccountNamespace::isIgnoring(JNIEnv *env, jobject self, jlong player, jstring who)
{
	PlayerObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		DEBUG_WARNING(true, ("JavaLibrary::isIgnoring: bad player object"));
		return JNI_FALSE;
	}
	std::string whoStr;
	if (!JavaLibrary::convert(JavaStringParam(who), whoStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::isIgnoring: bad who string"));
		return JNI_FALSE;
	}

	if (playerObject->isIgnoring(whoStr))
		return JNI_TRUE;
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlayerAccountNamespace::adjustLotCount(JNIEnv *env, jobject self, jlong player, jint adjustment)
{
	PlayerObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		DEBUG_WARNING(true, ("JavaLibrary::adjustLotCount:  bad player object"));
		return JNI_FALSE;
	}

	return playerObject->adjustLotCount(adjustment);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPlayerAccountNamespace::getAccountNumLots(JNIEnv *env, jobject self, jlong player)
{
	PlayerObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		DEBUG_WARNING(true, ("JavaLibrary::getAccountNumLots:  bad player object"));
		return 0;
	}

	return playerObject->getAccountNumLots();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPlayerAccountNamespace::getMaxHousingLots(JNIEnv *env, jobject self)
{
	UNREF(env);
	UNREF(self);
	return ConfigServerGame::getMaxHousingLots();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPlayerAccountNamespace::getGameFeatureBits(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		DEBUG_WARNING(true, ("JavaLibrary::getGameFeatureBits:  bad player object"));
		return 0;
	}

	if (!playerObject->getClient())
	{
		DEBUG_WARNING(true, ("JavaLibrary::getGameFeatureBits:  player object had no client"));
		return 0;
	}

	return playerObject->getClient()->getGameFeatures();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPlayerAccountNamespace::getSubscriptionFeatureBits(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		DEBUG_WARNING(true, ("JavaLibrary::GetSubscriptionFeatureBits:  bad player object"));
		return 0;
	}

	if (!playerObject->getClient())
	{
		DEBUG_WARNING(true, ("JavaLibrary::GetSubscriptionFeatureBits:  player object had no client"));
		return 0;
	}

	return playerObject->getClient()->getSubscriptionFeatures();
}

// ----------------------------------------------------------------------

/**
 * Returns info on how long a player has been playing the game.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the id of the player
 *
 * @return a dictionary with the player's account time info:
 *		"total_subscription_time"     the total amount of time since the player first subscribed
 *		"total_entitled_time"         the total amount of time the player has been billed
 *		"last_login_time"             the amount of time passed since the player last logged in
 *		"entitled_login_time"         the amount of billing time since the player last logged in
 */
jobject JNICALL ScriptMethodsPlayerAccountNamespace::getAccountTimeData(JNIEnv * env, jobject self, jlong target)
{
	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	const PlayerObject * player = PlayerCreatureController::getPlayerObject(
		creature);
	if (player == nullptr)
		return 0;

	const Client * client = creature->getClient();
	if (client == nullptr)
		return 0;

	// get the values that came from Platform
	int total_time = static_cast<int>(client->getEntitlementTotalTime());
	int entitled_time = static_cast<int>(client->getEntitlementEntitledTime());
	int login_time = static_cast<int>(client->getEntitlementTotalTimeSinceLastLogin());
	int entitled_login_time = static_cast<int>(client->getEntitlementEntitledTimeSinceLastLogin());

	// override with any global configuration specified values
	int temp = ConfigServerGame::getMinEntitledTime();

	if ((temp >= 0) && (temp > entitled_time))
		entitled_time = temp;

	// override with any manually created test values on the creature
	if (creature->getObjVars().getItem("_testVeteran.total_time", temp))
		total_time = temp;

	if (creature->getObjVars().getItem("_testVeteran.entitled_time", temp))
		entitled_time = temp;

	if (creature->getObjVars().getItem("_testVeteran.login_time", temp))
		login_time = temp;

	if (creature->getObjVars().getItem("_testVeteran.entitled_login_time", temp))
		entitled_login_time = temp;

	// don't return any information if all the values are 0 because it
	// indicates that Platform does not have the information available
	// at this time; we'll try again the next time the player logs in.
	if ((total_time == 0) && (entitled_time == 0) && (login_time == 0) && (entitled_login_time == 0))
		return 0;

	LocalRefPtr dictionary = createNewObject(JavaLibrary::getClsDictionary(), JavaLibrary::getMidDictionary());
	if (dictionary != LocalRef::cms_nullPtr)
	{
		callObjectMethod(*dictionary, JavaLibrary::getMidDictionaryPutInt(), JavaString("total_subscription_time").getValue(), total_time);
		callObjectMethod(*dictionary, JavaLibrary::getMidDictionaryPutInt(), JavaString("total_entitled_time").getValue(), entitled_time);
		callObjectMethod(*dictionary, JavaLibrary::getMidDictionaryPutInt(), JavaString("last_login_time").getValue(), login_time);
		callObjectMethod(*dictionary, JavaLibrary::getMidDictionaryPutInt(), JavaString("entitled_login_time").getValue(), entitled_login_time);
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return 0;
		}
	}
	return dictionary->getReturnValue();
}	// JavaLibrary::getAccountTimeData

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPlayerAccountNamespace::getPlayerLastLoginTime(JNIEnv *env, jobject self, jlong player)
{
	NetworkId playerId(player);
        return NameManager::getInstance().getPlayerLastLoginTime(playerId);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlayerAccountNamespace::isUsingAdminLogin(JNIEnv *env, jobject self, jlong  player)
{
	CreatureObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		DEBUG_WARNING(true, ("JavaLibrary::isUsingAdminLogin:  bad player object"));
		return false;
	}

	if (!playerObject->getClient())
	{
		DEBUG_WARNING(true, ("JavaLibrary::isUsingAdminLogin:  player object had no client"));
		return false;
	}

	return playerObject->getClient()->isUsingAdminLogin();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlayerAccountNamespace::isWarden(JNIEnv *env, jobject self, jlong  player)
{
	CreatureObject *creatureObject = 0;
	if (!JavaLibrary::getObject(player, creatureObject))
	{
		DEBUG_WARNING(true, ("JavaLibrary::isWarden:  bad creature object"));
		return false;
	}

	PlayerObject *playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (!playerObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::isWarden:  creature object had no player object"));
		return false;
	}

	return playerObject->isWarden();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlayerAccountNamespace::isFreeTrialAccount(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		DEBUG_WARNING(true, ("JavaLibrary::isFreeTrialAccount:  bad player object"));
		return false;
	}

	if (!playerObject->getClient())
	{
		DEBUG_WARNING(true, ("JavaLibrary::isFreeTrialAccount:  player object had no client"));
		return false;
	}

	return playerObject->getClient()->isFreeTrialAccount();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlayerAccountNamespace::setCompletedTutorial(JNIEnv *env, jobject self, jlong player, jboolean value)
{
	CreatureObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		DEBUG_WARNING(true, ("JavaLibrary::setCompletedTutorial:  bad player object"));
		return false;
	}

	if (!playerObject->getClient())
	{
		DEBUG_WARNING(true, ("JavaLibrary::setCompletedTutorial:  player object had no client"));
		return false;
	}

	if (playerObject->getClient()->isFreeTrialAccount())
	{
		DEBUG_WARNING(true, ("JavaLibrary::setCompletedTutorial:  client is from a free trial and CANNOT have completed tutorial bit set"));
		return false;
	}

	const unsigned int stationId = playerObject->getClient()->getStationId();

	LOG("CustomerService", ("Setting tutorial bit to %s for stationId %i\n", (value) ? "true" : "false", stationId));
	GenericValueTypeMessage< std::pair<unsigned int, bool> > const updateTutorial("LoginToggleCompletedTutorial", std::pair<unsigned int, bool>(stationId, (bool)value));
	GameServer::getInstance().sendToCentralServer(updateTutorial);
	return true;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPlayerAccountNamespace::logActivity(JNIEnv *env, jobject self, jlong player, jint activityType)
{
	UNREF(self);

	CreatureObject * creatureObject = 0;
	if (!JavaLibrary::getObject(player, creatureObject))
		return;

	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (playerObject)
		playerObject->addSessionActivity(static_cast<uint32>(activityType));
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlayerAccountNamespace::isAccountQualifiedForHousePackup(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);
	NOT_NULL(env);

	static std::unordered_set<StationId> accountsQualifiedForHousePackup;
	if (accountsQualifiedForHousePackup.empty())
	{
		DataTable * table = DataTableManager::getTable(ConfigServerGame::getHousePackupAccountListDataTableName(), true);
		if (table)
		{
			int const columnId = table->findColumnNumber("stationId");
			if (columnId >= 0)
			{
				int const numRows = table->getNumRows();
				for (int i = 0; i < numRows; ++i)
				{
					IGNORE_RETURN(accountsQualifiedForHousePackup.insert(static_cast<StationId>(table->getIntValue(columnId, i))));
				}
			}

			DataTableManager::close(ConfigServerGame::getHousePackupAccountListDataTableName());
		}
	}

	return (accountsQualifiedForHousePackup.count(static_cast<StationId>(NameManager::getInstance().getPlayerStationId(NetworkId(static_cast<NetworkId::NetworkIdType>(target))))) >= 1);
}

/**
* Gets the CTS history of the specified character
*
* @returns a dictionary that contains the following data in paralled arrays
* with the oldest CTS transaction first and the newest CTS transaction last
*
* @returns nullptr if the character doesn't have any CTS history
*
* string[] character_name        full name of the source character
* string[] cluster_name          name of the source cluster
* int[]    transfer_time         calendar time of the transfer
*/
jobject JNICALL ScriptMethodsPlayerAccountNamespace::getCharacterCtsHistory(JNIEnv * env, jobject self, jlong player)
{
	CreatureObject const * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		DEBUG_WARNING(true, ("JavaLibrary::getCharacterCtsHistory:  bad player object"));
		return 0;
	}

	if (!playerObject->getObjVars().hasItem("ctsHistory"))
		return 0;

	// we need to return the CTS history ordered by transfer time
	std::multimap<int, std::pair<const Unicode::String *, const Unicode::String *> > orderedCtsHistory;

	DynamicVariableList::NestedList const ctsTransactions(playerObject->getObjVars(), "ctsHistory");
	for (DynamicVariableList::NestedList::const_iterator i = ctsTransactions.begin(); i != ctsTransactions.end(); ++i)
	{
		Unicode::String ctsTransactionDetail;
		if (i.getValue(ctsTransactionDetail))
		{
			Unicode::UnicodeStringVector tokens;
			if (Unicode::tokenize(ctsTransactionDetail, tokens, nullptr, nullptr) && (tokens.size() >= 4))
			{
				Unicode::String * characterName = new Unicode::String();
				for (size_t i = 3, j = tokens.size(); i < j; ++i)
				{
					if (!characterName->empty())
						*characterName += Unicode::narrowToWide(" ");

					*characterName += tokens[i];
				}

				IGNORE_RETURN(orderedCtsHistory.insert(std::make_pair(atoi(Unicode::wideToNarrow(tokens[0]).c_str()), std::make_pair(characterName, new Unicode::String(tokens[1])))));
			}
		}
	}

	if (orderedCtsHistory.empty())
		return 0;

	std::vector<const Unicode::String *> * sourceCharacterName = new std::vector<const Unicode::String *>;
	std::vector<const Unicode::String *> * sourceClusterName = new std::vector<const Unicode::String *>;
	std::vector<int> * transferTime = new std::vector<int>;

	for (std::multimap<int, std::pair<const Unicode::String *, const Unicode::String *> >::const_iterator iter = orderedCtsHistory.begin(); iter != orderedCtsHistory.end(); ++iter)
	{
		transferTime->push_back(iter->first);
		sourceCharacterName->push_back(iter->second.first);
		sourceClusterName->push_back(iter->second.second);
	}

	ScriptParams params;
	params.addParam(*sourceCharacterName, "character_name", true);
	params.addParam(*sourceClusterName, "cluster_name", true);
	params.addParam(*transferTime, "transfer_time", true);

	JavaDictionaryPtr dictionary;
	JavaLibrary::instance()->convert(params, dictionary);
	return dictionary->getReturnValue();
}	// JavaLibrary::requestJedi

/**
* Gets the retroactive CTS objvars history of the specified character
*
* @returns an array of dictionaries each of which contains the retroactive CTS objvars
* for the particular CTS source character that this character at one time transferred from
* with the oldest CTS transaction first and the newest CTS transaction last
*
* @returns nullptr if the character doesn't have any retroactive CTS objvars history
*/
jobjectArray JNICALL ScriptMethodsPlayerAccountNamespace::getCharacterRetroactiveCtsObjvars(JNIEnv * env, jobject self, jlong player)
{
	std::vector<std::vector<std::pair<std::string, DynamicVariable> > const *> const & characterRetroactiveCtsObjvars = GameServer::getRetroactiveCtsHistoryObjvars(NetworkId(static_cast<NetworkId::NetworkIdType>(player)));
	if (characterRetroactiveCtsObjvars.empty())
		return nullptr;

	LocalObjectArrayRefPtr results = createNewObjectArray(characterRetroactiveCtsObjvars.size(), JavaLibrary::getClsDictionary());
	for (size_t i = 0, size = characterRetroactiveCtsObjvars.size(); i < size; ++i)
	{
		bool addedAnything = false;
		ScriptParams params;

		if (characterRetroactiveCtsObjvars[i] && !characterRetroactiveCtsObjvars[i]->empty())
		{
			for (std::vector<std::pair<std::string, DynamicVariable> >::const_iterator iterObjvar = characterRetroactiveCtsObjvars[i]->begin(); iterObjvar != characterRetroactiveCtsObjvars[i]->end(); ++iterObjvar)
			{
				// can only handle INT (type 0) and INT_ARRAY (type 1) objvar type at this time
				if (iterObjvar->second.getType() == DynamicVariable::INT)
				{
					int iValue;
					iterObjvar->second.get(iValue);
					params.addParam(iValue, iterObjvar->first);
					addedAnything = true;
				}
				else if (iterObjvar->second.getType() == DynamicVariable::INT_ARRAY)
				{
					// iaValue will be owned and deleted by params
					std::vector<int> * iaValue = new std::vector<int>();
					iterObjvar->second.get(*iaValue);
					params.addParam(*iaValue, iterObjvar->first, true);
					addedAnything = true;
				}
			}
		}

		if (!addedAnything)
		{
			setObjectArrayElement(*results, i, *JavaDictionary::cms_nullPtr);
		}
		else
		{
			JavaDictionaryPtr dictionary;
			JavaLibrary::instance()->convert(params, dictionary);
			setObjectArrayElement(*results, i, *dictionary);
		}
	}

	return results->getReturnValue();
}

/**
* Checks to see if the specified character qualifies for free CTS
*
* @returns an array of strings containing the galaxies to which this character can transfer for free
*/
jobjectArray JNICALL ScriptMethodsPlayerAccountNamespace::qualifyForFreeCts(JNIEnv * /*env*/, jobject /*self*/, jlong player)
{
	time_t const characterCreateTime = static_cast<time_t>(NameManager::getInstance().getPlayerCreateTime(NetworkId(player)));
	FreeCtsDataTable::FreeCtsInfo const * freeCtsInfo = FreeCtsDataTable::getFreeCtsInfoForCharacter(characterCreateTime, GameServer::getInstance().getClusterName(), false);

	// see if we can/should bypass the free CTS time restriction
	if (!freeCtsInfo && ConfigServerGame::getAllowIgnoreFreeCtsTimeRestriction())
	{
		CreatureObject const * playerObject = nullptr;
		if (JavaLibrary::getObject(player, playerObject) && playerObject && playerObject->getClient() && playerObject->getClient()->isGod())
		{
			freeCtsInfo = FreeCtsDataTable::getFreeCtsInfoForCharacter(characterCreateTime, GameServer::getInstance().getClusterName(), true);
			if (freeCtsInfo)
			{
				Chat::sendSystemMessage(*playerObject, Unicode::narrowToWide("Bypassing free CTS time restriction for GOD MODE!!!"), Unicode::emptyString);
			}
		}
	}

	if (!freeCtsInfo || freeCtsInfo->targetCluster.empty())
		return nullptr;

	LocalObjectArrayRefPtr valueArray = createNewObjectArray(freeCtsInfo->targetCluster.size(), JavaLibrary::getClsString());

	int i = 0;
	for (std::map<std::string, std::string>::const_iterator iter = freeCtsInfo->targetCluster.begin(); iter != freeCtsInfo->targetCluster.end(); ++iter)
	{
		JavaString jval(iter->second);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

/**
* Validate the free CTS information and messageTo back to the object to indicate status of the validation
*/
void JNICALL ScriptMethodsPlayerAccountNamespace::validateFreeCts(JNIEnv * /*env*/, jobject /*self*/, jlong player, jstring destinationGalaxy, jstring destinationCharacterName)
{
	CreatureObject const * creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject) || !creatureObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateFreeCts: bad CreatureObject"));
		return;
	}

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (!playerObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateFreeCts: bad PlayerObject"));
		return;
	}

	std::string destinationGalaxyString;
	JavaStringParam temp1(destinationGalaxy);
	if (!JavaLibrary::convert(temp1, destinationGalaxyString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateFreeCts: bad destinationGalaxy"));
		return;
	}

	std::string destinationCharacterNameString;
	JavaStringParam temp2(destinationCharacterName);
	if (!JavaLibrary::convert(temp2, destinationCharacterNameString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateFreeCts: bad destinationCharacterName"));
		return;
	}

	TransferRequestMoveValidation const trmv(TransferRequestMoveValidation::TRS_ingame_freects_command_validate, GameServer::getInstance().getProcessId(), playerObject->getStationId(), playerObject->getStationId(), GameServer::getInstance().getClusterName(), destinationGalaxyString, Unicode::wideToNarrow(creatureObject->getAssignedObjectName()), creatureObject->getNetworkId(), creatureObject->getTemplateCrc(), destinationCharacterNameString, std::string("en"));
	GameServer::getInstance().sendToCentralServer(trmv);
}

/**
* Validate the free CTS information and initiate the transfer process if the validation passes, or
* messageTo back to the object to indicate status of the validation if the validation fails
*/
void JNICALL ScriptMethodsPlayerAccountNamespace::performFreeCts(JNIEnv * /*env*/, jobject /*self*/, jlong player, jstring destinationGalaxy, jstring destinationCharacterName)
{
	CreatureObject const * creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject) || !creatureObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::performFreeCts: bad CreatureObject"));
		return;
	}

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (!playerObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::performFreeCts: bad PlayerObject"));
		return;
	}

	std::string destinationGalaxyString;
	JavaStringParam temp1(destinationGalaxy);
	if (!JavaLibrary::convert(temp1, destinationGalaxyString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::performFreeCts: bad destinationGalaxy"));
		return;
	}

	std::string destinationCharacterNameString;
	JavaStringParam temp2(destinationCharacterName);
	if (!JavaLibrary::convert(temp2, destinationCharacterNameString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::performFreeCts: bad destinationCharacterName"));
		return;
	}

	TransferRequestMoveValidation const trmv(TransferRequestMoveValidation::TRS_ingame_freects_command_transfer, GameServer::getInstance().getProcessId(), playerObject->getStationId(), playerObject->getStationId(), GameServer::getInstance().getClusterName(), destinationGalaxyString, Unicode::wideToNarrow(creatureObject->getAssignedObjectName()), creatureObject->getNetworkId(), creatureObject->getTemplateCrc(), destinationCharacterNameString, std::string("en"));
	GameServer::getInstance().sendToCentralServer(trmv);
}

/**
* Validate the CTS information and messageTo back to the object to indicate status of the validation
*/
void JNICALL ScriptMethodsPlayerAccountNamespace::validateCts(JNIEnv * /*env*/, jobject /*self*/, jlong player, jstring destinationGalaxy, jstring destinationCharacterName)
{
	CreatureObject const * creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject) || !creatureObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateCts: bad CreatureObject"));
		return;
	}

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (!playerObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateCts: bad PlayerObject"));
		return;
	}

	std::string destinationGalaxyString;
	JavaStringParam temp1(destinationGalaxy);
	if (!JavaLibrary::convert(temp1, destinationGalaxyString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateCts: bad destinationGalaxy"));
		return;
	}

	std::string destinationCharacterNameString;
	JavaStringParam temp2(destinationCharacterName);
	if (!JavaLibrary::convert(temp2, destinationCharacterNameString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateCts: bad destinationCharacterName"));
		return;
	}

	TransferRequestMoveValidation const trmv(TransferRequestMoveValidation::TRS_ingame_cts_command_validate, GameServer::getInstance().getProcessId(), playerObject->getStationId(), playerObject->getStationId(), GameServer::getInstance().getClusterName(), destinationGalaxyString, Unicode::wideToNarrow(creatureObject->getAssignedObjectName()), creatureObject->getNetworkId(), creatureObject->getTemplateCrc(), destinationCharacterNameString, std::string("en"));
	GameServer::getInstance().sendToCentralServer(trmv);
}

/**
* Validate the CTS information and initiate the transfer process if the validation passes, or
* messageTo back to the object to indicate status of the validation if the validation fails
*/
void JNICALL ScriptMethodsPlayerAccountNamespace::performCts(JNIEnv * /*env*/, jobject /*self*/, jlong player, jstring destinationGalaxy, jstring destinationCharacterName)
{
	CreatureObject const * creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject) || !creatureObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::performCts: bad CreatureObject"));
		return;
	}

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (!playerObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::performCts: bad PlayerObject"));
		return;
	}

	std::string destinationGalaxyString;
	JavaStringParam temp1(destinationGalaxy);
	if (!JavaLibrary::convert(temp1, destinationGalaxyString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::performCts: bad destinationGalaxy"));
		return;
	}

	std::string destinationCharacterNameString;
	JavaStringParam temp2(destinationCharacterName);
	if (!JavaLibrary::convert(temp2, destinationCharacterNameString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::performCts: bad destinationCharacterName"));
		return;
	}

	TransferRequestMoveValidation const trmv(TransferRequestMoveValidation::TRS_ingame_cts_command_transfer, GameServer::getInstance().getProcessId(), playerObject->getStationId(), playerObject->getStationId(), GameServer::getInstance().getClusterName(), destinationGalaxyString, Unicode::wideToNarrow(creatureObject->getAssignedObjectName()), creatureObject->getNetworkId(), creatureObject->getTemplateCrc(), destinationCharacterNameString, std::string("en"));
	GameServer::getInstance().sendToCentralServer(trmv);
}

/**
 * Validate the new character name for the rename operation and messageTo back to the object to indicate status of the validation
 */
void JNICALL ScriptMethodsPlayerAccountNamespace::validateRenameCharacter(JNIEnv *env, jobject self, jlong player, jstring newName)
{
	CreatureObject const * creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject) || !creatureObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateRenameCharacter: bad CreatureObject"));
		return;
	}

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (!playerObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateRenameCharacter: bad PlayerObject"));
		return;
	}

	Unicode::String newNameString;
	JavaStringParam temp(newName);
	if (!JavaLibrary::convert(temp, newNameString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::validateRenameCharacter: bad newName"));
		return;
	}

	// if this is a last name change only, to validate that the last name is valid,
	// generate a random unique first name, and validate that with the new last name
	bool lastNameChangeOnly = false;
	static Unicode::String const delimiters(Unicode::narrowToWide(" "));
	Unicode::UnicodeStringVector newNameTokens;
	if (!Unicode::tokenize(newNameString, newNameTokens, &delimiters, nullptr))
		newNameTokens.clear();

	size_t const newNameTokensCount = newNameTokens.size();

	if ((newNameTokensCount == 1) || (newNameTokensCount == 2))
	{
		if (Unicode::caseInsensitiveCompare(creatureObject->getAssignedObjectFirstName(), newNameTokens[0]))
		{
			lastNameChangeOnly = true;

			// no last name specified, so validation passes
			if (newNameTokensCount == 1)
			{
				MessageToQueue::getInstance().sendMessageToJava(creatureObject->getNetworkId(), "renameCharacterNameValidationSuccess", std::vector<int8>(), 0, false);
				return;
			}
			else
			{
				const ObjectTemplate * const objectTemplate = ObjectTemplateList::fetch(creatureObject->getObjectTemplateName());
				if (objectTemplate)
				{
					const ServerCreatureObjectTemplate * const creatureTemplate = dynamic_cast<const ServerCreatureObjectTemplate *>(objectTemplate);
					if (creatureTemplate)
					{
						Unicode::String const uniqueRandomName = NameManager::getInstance().generateUniqueRandomName(ConfigServerGame::getCharacterNameGeneratorDirectory(), creatureTemplate->getNameGeneratorType());
						Unicode::UnicodeStringVector uniqueRandomNameTokens;
						if (!Unicode::tokenize(uniqueRandomName, uniqueRandomNameTokens, &delimiters, nullptr))
							uniqueRandomNameTokens.clear();

						newNameString = ((uniqueRandomNameTokens.size() >= 1) ? uniqueRandomNameTokens[0] : delimiters) + delimiters + newNameTokens[1];
					}
				}
			}
		}
	}
	else
	{
		ScriptParams params;
		params.addParam("@ui:name_declined_racially_inappropriate", "reason");
		ScriptDictionaryPtr dictionary;
		GameScriptObject::makeScriptDictionary(params, dictionary);
		if (dictionary.get() != nullptr)
		{
			dictionary->serialize();
			MessageToQueue::getInstance().sendMessageToJava(creatureObject->getNetworkId(), "renameCharacterNameValidationFail", dictionary->getSerializedData(), 0, false);
		}

		return;
	}

	VerifyAndLockNameRequest const verifyRequest(playerObject->getStationId(), creatureObject->getNetworkId(), creatureObject->getObjectTemplateName(), newNameString, 0xFFFFFFFF);
	StringId const verifyResult = GameServer::getInstance().handleVerifyAndLockNameRequest(verifyRequest, false, !lastNameChangeOnly);
	if (verifyResult != NameErrors::nameApproved)
	{
		std::string const errorText(std::string("@") + verifyResult.getCanonicalRepresentation());
		ScriptParams params;
		params.addParam(errorText.c_str(), "reason");
		ScriptDictionaryPtr dictionary;
		GameScriptObject::makeScriptDictionary(params, dictionary);
		if (dictionary.get() != nullptr)
		{
			dictionary->serialize();
			MessageToQueue::getInstance().sendMessageToJava(creatureObject->getNetworkId(), "renameCharacterNameValidationFail", dictionary->getSerializedData(), 0, false);
		}
	}
	else if (lastNameChangeOnly)
	{
		// last name passes validation
		MessageToQueue::getInstance().sendMessageToJava(creatureObject->getNetworkId(), "renameCharacterNameValidationSuccess", std::vector<int8>(), 0, false);
	}
	else
	{
		// nothing to do here, as it means the request has been forwarded
		// to the DB server for additional verification, so when that
		// verification is complete, we'll send the response from there
	}
}

/**
* Release the character name that had been reserved for renaming the specified character; the character
* name is reserved during the character rename process, even if the name doesn't end up being used,
* so we want to return the name back to the pool; just make sure you don't call this function during
* the character rename process, or before the character rename process has completed
*/
void JNICALL ScriptMethodsPlayerAccountNamespace::renameCharacterReleaseNameReservation(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject const * creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject) || !creatureObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::renameCharacterReleaseNameReservation: bad CreatureObject"));
		return;
	}

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (!playerObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::renameCharacterReleaseNameReservation: bad PlayerObject"));
		return;
	}

	ReleaseNameMessage const rnm(playerObject->getStationId(), creatureObject->getNetworkId());
	GameServer::getInstance().sendToDatabaseServer(rnm);
}

/**
* submit request to rename character; the name is assumed to have already been validated with validateRenameCharacter()
*/
void JNICALL ScriptMethodsPlayerAccountNamespace::renameCharacter(JNIEnv *env, jobject self, jlong player, jstring newName)
{
	CreatureObject const * creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject) || !creatureObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::renameCharacter: bad CreatureObject"));
		return;
	}

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (!playerObject)
	{
		DEBUG_WARNING(true, ("JavaLibrary::renameCharacter: bad PlayerObject"));
		return;
	}

	Unicode::String newNameString;
	JavaStringParam temp(newName);
	if (!JavaLibrary::convert(temp, newNameString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::renameCharacter: bad newName"));
		return;
	}

	PlayerCreationManagerServer::renamePlayer(static_cast<int8>(RenameCharacterMessageEx::RCMS_player_request), playerObject->getStationId(), creatureObject->getNetworkId(), newNameString, creatureObject->getAssignedObjectName(), NetworkId::cms_invalid);
}

// ----------------------------------------------------------------------

/**
* Get the station id of the player.
*
* @param env		    Java environment
* @param self		    class calling this function
* @param player         the id of the player
*
* @return the station id of the player
*/

jint JNICALL ScriptMethodsPlayerAccountNamespace::getPlayerStationId(JNIEnv *env, jobject self, jlong player)
{
	UNREF(self);
	NOT_NULL(env);

	return NameManager::getInstance().getPlayerStationId(NetworkId(static_cast<NetworkId::NetworkIdType>(player)));
}

// ----------------------------------------------------------------------

/**
 * Get the account username of a player.
 * Note: The player must be online to use this.
 * See base_class.java implementation for more info.
 *
 * @param player the object id of the player
 * @return string of the player's account username
 */
jstring JNICALL ScriptMethodsPlayerAccountNamespace::getPlayerUsername(JNIEnv *env, jobject self, jlong player) {
    UNREF(self);
    NOT_NULL(env);

    CreatureObject const * creatureObject = nullptr;
    if (!JavaLibrary::getObject(player, creatureObject) || !creatureObject)
    {
        DEBUG_WARNING(true, ("JavaLibrary::getPlayerUsername: bad CreatureObject"));
        return nullptr;
    }

    if(creatureObject)
    {
        const Client * playerClient = creatureObject->getClient();
        if(playerClient)
        {
            return JavaString(playerClient->getAccountName()).getReturnValue();
        }
        else {
            DEBUG_WARNING(true, ("JavaLibrary::getPlayerUsername: bad playerClient"));
            return nullptr;
        }
    }

}

// ======================================================================
