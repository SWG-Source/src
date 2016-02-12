// ======================================================================
//
// ScriptMethodsVeteran.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/VeteranRewardManager.h"
#include "serverNetworkMessages/AccountFeatureIdRequest.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedObject/NetworkIdManager.h"
#include "UnicodeUtils.h"

// ======================================================================

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsVeteranNamespace
// ======================================================================

namespace ScriptMethodsVeteranNamespace
{
	bool install();

	void         JNICALL veteranWriteAccountDataToObjvars(JNIEnv * env, jobject self, jlong player);
	jobjectArray JNICALL veteranGetTriggeredEvents(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL veteranAccountFeatureIdRequest(JNIEnv * env, jobject self, jlong player);
	jobjectArray JNICALL veteranGetRewardChoicesDescriptions(JNIEnv *env, jobject self, jlong player, jstring event);
	jobjectArray JNICALL veteranGetRewardChoicesTags(JNIEnv *env, jobject self, jlong player, jstring event);
	jboolean     JNICALL veteranClaimReward(JNIEnv *env, jobject self, jlong player, jstring event, jstring rewardTag);
	jstring      JNICALL veteranGetEventAnnouncement(JNIEnv *env, jobject self, jstring event);
	jstring      JNICALL veteranGetEventDescription(JNIEnv *env, jobject self, jstring event);
	jstring      JNICALL veteranGetEventUrl(JNIEnv *env, jobject self, jstring event);
	jboolean     JNICALL veteranIsEventAccountUnique(JNIEnv *env, jobject self, jstring event);
 	jboolean     JNICALL veteranIsItemAccountUnique(JNIEnv *env, jobject self, jstring item);
	jboolean     JNICALL veteranIsItemAccountUniqueFeatureId(JNIEnv *env, jobject self, jstring item);
	jboolean     JNICALL veteranCanTradeInReward(JNIEnv *env, jobject self, jlong player, jlong item);
	void         JNICALL veteranTradeInReward(JNIEnv *env, jobject self, jlong player, jlong item);
	void         JNICALL adjustSwgTcgAccountFeatureId(JNIEnv *env, jobject self, jlong player, jlong item, jint featureId, jint adjustment);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsVeteranNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsVeteranNamespace::c)}
	JF("_veteranWriteAccountDataToObjvars",             "(J)V", veteranWriteAccountDataToObjvars),
	JF("_veteranGetTriggeredEvents",                    "(J)[Ljava/lang/String;", veteranGetTriggeredEvents),
	JF("_veteranAccountFeatureIdRequest",               "(J)Z", veteranAccountFeatureIdRequest),
	JF("_veteranGetRewardChoicesDescriptions",          "(JLjava/lang/String;)[Ljava/lang/String;", veteranGetRewardChoicesDescriptions),
	JF("_veteranGetRewardChoicesTags",                  "(JLjava/lang/String;)[Ljava/lang/String;", veteranGetRewardChoicesTags),
	JF("_veteranClaimReward",                           "(JLjava/lang/String;Ljava/lang/String;)Z", veteranClaimReward),
	JF("veteranGetEventAnnouncement",                  "(Ljava/lang/String;)Ljava/lang/String;", veteranGetEventAnnouncement),
	JF("veteranGetEventDescription",                   "(Ljava/lang/String;)Ljava/lang/String;", veteranGetEventDescription),
	JF("veteranGetEventUrl",                           "(Ljava/lang/String;)Ljava/lang/String;", veteranGetEventUrl),
	JF("veteranIsEventAccountUnique",                  "(Ljava/lang/String;)Z", veteranIsEventAccountUnique),
	JF("veteranIsItemAccountUnique",                   "(Ljava/lang/String;)Z", veteranIsItemAccountUnique),
	JF("veteranIsItemAccountUniqueFeatureId",          "(Ljava/lang/String;)Z", veteranIsItemAccountUniqueFeatureId),
	JF("_veteranCanTradeInReward",                     "(JJ)Z", veteranCanTradeInReward),
	JF("_veteranTradeInReward",                        "(JJ)V", veteranTradeInReward),
	JF("_adjustSwgTcgAccountFeatureId",                "(JJII)V", adjustSwgTcgAccountFeatureId),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jobjectArray JNICALL ScriptMethodsVeteranNamespace::veteranGetTriggeredEvents(JNIEnv * /*env*/, jobject /*self*/, jlong player)
{
	ServerObject const * const so = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(NetworkId(player)));
	CreatureObject const * const playerCreature = so ? so->asCreatureObject() : nullptr;

	if (playerCreature)
	{
		std::vector<std::string> eventsIds;
		VeteranRewardManager::getTriggeredEventsIds(*playerCreature, eventsIds);

		if (eventsIds.empty())
			return nullptr;

		int i = 0;
		LocalObjectArrayRefPtr valueArray = createNewObjectArray(eventsIds.size(), JavaLibrary::getClsString());
		for (std::vector<std::string>::const_iterator iter = eventsIds.begin(); iter != eventsIds.end(); ++iter)
		{
			JavaString jval(*iter);
			setObjectArrayElement(*valueArray, i++, jval);
		}

		return valueArray->getReturnValue();
	}

	return nullptr;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsVeteranNamespace::veteranAccountFeatureIdRequest(JNIEnv * /*env*/, jobject /*self*/, jlong player)
{
	ServerObject * const so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(NetworkId(player)));
	CreatureObject * const playerCreature = so ? so->asCreatureObject() : nullptr;

	if (playerCreature)
	{
		Client * const client = playerCreature->getClient();
		if (client && !client->isUsingAdminLogin())
		{
			AccountFeatureIdRequest const req(playerCreature->getNetworkId(), GameServer::getInstance().getProcessId(), playerCreature->getNetworkId(), static_cast<StationId>(client->getStationId()), PlatformGameCode::SWG, AccountFeatureIdRequest::RR_ReloadRewardCheck);
			client->sendToConnectionServer(req);
			return JNI_TRUE;
		}
	}

	return JNI_FALSE;
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsVeteranNamespace::veteranGetRewardChoicesDescriptions(JNIEnv * /*env*/, jobject /*self*/, jlong player, jstring event)
{
	ServerObject const * const so = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(NetworkId(player)));
	CreatureObject const * const playerCreature = so ? so->asCreatureObject() : nullptr;

	if (playerCreature)
	{
		std::string eventString;
		JavaStringParam temp(event);
		if (JavaLibrary::convert(temp,eventString))
		{
			std::vector<std::string> value;

			VeteranRewardManager::getRewardChoicesDescriptions(*playerCreature, eventString, value);
			LocalObjectArrayRefPtr valueArray = createNewObjectArray(static_cast<long>(value.size()), JavaLibrary::getClsString());
			for (size_t i = 0; i < value.size(); ++i)
			{
				JavaString jval(value[i]);
				setObjectArrayElement(*valueArray, static_cast<long>(i), jval);
			}
			return valueArray->getReturnValue();
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsVeteranNamespace::veteranGetRewardChoicesTags(JNIEnv * /*env*/, jobject /*self*/, jlong player, jstring event)
{
	ServerObject const * const so = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(NetworkId(player)));
	CreatureObject const * const playerCreature = so ? so->asCreatureObject() : nullptr;

	if (playerCreature)
	{
		std::string eventString;
		JavaStringParam temp(event);
		if (JavaLibrary::convert(temp,eventString))
		{
			std::vector<Unicode::String> value;

			VeteranRewardManager::getRewardChoicesTags(*playerCreature, eventString, value);
			LocalObjectArrayRefPtr valueArray = createNewObjectArray(static_cast<long>(value.size()), JavaLibrary::getClsString());
			for (size_t i = 0; i < value.size(); ++i)
			{
				JavaString jval(value[i]);
				setObjectArrayElement(*valueArray, static_cast<long>(i), jval);
			}
			return valueArray->getReturnValue();
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsVeteranNamespace::veteranClaimReward(JNIEnv * /*env*/, jobject /*self*/, jlong player, jstring event, jstring rewardTag)
{
	ServerObject const * const so = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(NetworkId(player)));
	CreatureObject const * const playerCreature = so ? so->asCreatureObject() : nullptr;

	if (playerCreature)
	{
		std::string eventString;
		JavaStringParam temp(event);
		if (JavaLibrary::convert(temp,eventString))
		{
			std::string rewardTagString;
			JavaStringParam temp2(rewardTag);
			if (JavaLibrary::convert(temp2,rewardTagString))
			{
				std::string debugMessage;
				if (VeteranRewardManager::claimRewards(*playerCreature, eventString, rewardTagString, &debugMessage))
					return true;
				else
					DEBUG_REPORT_LOG(true,("Claim failed:  %s",debugMessage.c_str()));
			}
		}
	}
	return false;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsVeteranNamespace::veteranGetEventAnnouncement(JNIEnv * /*env*/, jobject /*self*/, jstring event)
{
	std::string eventString;
	JavaStringParam temp(event);
	if (JavaLibrary::convert(temp,eventString))
	{
		StringId const * const eventStringId = VeteranRewardManager::getEventAnnouncement(eventString);
		if (eventStringId)
		{
			JavaString eventAnnouncement(Unicode::narrowToWide(std::string("@") + eventStringId->getCanonicalRepresentation()));
			return eventAnnouncement.getReturnValue();
		}
	}
	return nullptr;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsVeteranNamespace::veteranGetEventDescription(JNIEnv * /*env*/, jobject /*self*/, jstring event)
{
	std::string eventString;
	JavaStringParam temp(event);
	if (JavaLibrary::convert(temp,eventString))
	{
		StringId const * const eventStringId = VeteranRewardManager::getEventDescription(eventString);
		if (eventStringId)
		{
			JavaString eventDescription(Unicode::narrowToWide(std::string("@") + eventStringId->getCanonicalRepresentation()));
			return eventDescription.getReturnValue();
		}
	}
	return nullptr;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsVeteranNamespace::veteranGetEventUrl(JNIEnv * /*env*/, jobject /*self*/, jstring event)
{
	std::string eventString;
	JavaStringParam temp(event);
	if (JavaLibrary::convert(temp,eventString))
	{
		std::string const * const eventUrl = VeteranRewardManager::getEventUrl(eventString);
		if (eventUrl)
		{
			JavaString temp2(*eventUrl);
			return temp2.getReturnValue();
		}
	}
	return nullptr;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsVeteranNamespace::veteranIsEventAccountUnique(JNIEnv * /*env*/, jobject /*self*/, jstring event)
{
	std::string eventString;
	JavaStringParam temp(event);
	if (JavaLibrary::convert(temp,eventString))
	{
		return VeteranRewardManager::isEventAccountUnique(eventString);
	}
	else
		return false;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsVeteranNamespace::veteranIsItemAccountUnique(JNIEnv * /*env*/, jobject /*self*/, jstring item)
{
	std::string itemString;
	JavaStringParam temp(item);
	if (JavaLibrary::convert(temp,itemString))
	{
		return VeteranRewardManager::isItemAccountUnique(itemString);
	}
	else
		return false;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsVeteranNamespace::veteranIsItemAccountUniqueFeatureId(JNIEnv * /*env*/, jobject /*self*/, jstring item)
{
	std::string itemString;
	JavaStringParam temp(item);
	if (JavaLibrary::convert(temp,itemString))
	{
		return VeteranRewardManager::isItemAccountUniqueFeatureId(itemString);
	}
	else
		return false;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsVeteranNamespace::veteranWriteAccountDataToObjvars(JNIEnv * /*env*/, jobject /*self*/, jlong player)
{
	ServerObject * const so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(NetworkId(player)));
	CreatureObject * const playerCreature = so ? so->asCreatureObject() : nullptr;

	if (playerCreature)
		VeteranRewardManager::writeAccountDataToObjvars(*playerCreature);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsVeteranNamespace::veteranCanTradeInReward(JNIEnv *env, jobject self, jlong player, jlong item)
{
	CreatureObject const * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature) || !playerCreature)
		return JNI_FALSE;

	ServerObject * itemObject = nullptr;
	if (!JavaLibrary::getObject(item, itemObject) || !itemObject)
		return JNI_FALSE;

	// just validate the trade in
	return VeteranRewardManager::tradeInReward(*playerCreature, *itemObject, true);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsVeteranNamespace::veteranTradeInReward(JNIEnv *env, jobject self, jlong player, jlong item)
{
	CreatureObject const * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature) || !playerCreature)
		return;

	ServerObject * itemObject = nullptr;
	if (!JavaLibrary::getObject(item, itemObject) || !itemObject)
		return;

	// validate and perform the trade in
	VeteranRewardManager::tradeInReward(*playerCreature, *itemObject, false);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsVeteranNamespace::adjustSwgTcgAccountFeatureId(JNIEnv *env, jobject self, jlong player, jlong item, jint featureId, jint adjustment)
{
	CreatureObject const * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature) || !playerCreature)
		return;

	ServerObject * itemObject = nullptr;
	if (!JavaLibrary::getObject(item, itemObject) || !itemObject)
		return;

	VeteranRewardManager::tcgRedemption(*playerCreature, *itemObject, featureId, adjustment);
}

// ======================================================================
