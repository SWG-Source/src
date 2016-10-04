// ======================================================================
//
// ScriptMethodsHateList.cpp
// Copyright 2005 Sony Online Eentertainment, Inc.
// All rights reserved.
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/AiLogManager.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/HateList.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/TangibleObject.h"
#include "sharedLog/Log.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsHateListNamespace
// ======================================================================

namespace ScriptMethodsHateListNamespace
{
	bool install();

	void       JNICALL setAggroImmuneDuration(JNIEnv * env, jobject self, jlong player, jint duration);
	jboolean   JNICALL isAggroImmune(JNIEnv * env, jobject self, jlong player);
	void       JNICALL addHate(JNIEnv * env, jobject self, jlong object, jlong hateTarget, jfloat hate);
	void       JNICALL addHateDot(JNIEnv * env, jobject self, jlong object, jlong hateTarget, jfloat hate, jint seconds);
	void       JNICALL setHate(JNIEnv * env, jobject self, jlong object, jlong hateTarget, jfloat hate);
	void       JNICALL removeHateTarget(JNIEnv * env, jobject self, jlong object, jlong hateTarget);
	jfloat     JNICALL getHate(JNIEnv * env, jobject self, jlong object, jlong hateTarget);
	jfloat     JNICALL getMaxHate(JNIEnv * env, jobject self, jlong object);
	void       JNICALL clearHateList(JNIEnv * env, jobject self, jlong object);
	jlong      JNICALL getHateTarget(JNIEnv * env, jobject self, jlong object);
	jlongArray JNICALL getHateList(JNIEnv * env, jobject self, jlong object);
	jboolean   JNICALL isOnHateList(JNIEnv * env, jobject self, jlong object, jlong target);
	void       JNICALL resetHateTimer(JNIEnv * env, jobject self, jlong object);
	void       JNICALL setAILeashTime(JNIEnv * env, jobject self, jlong object, jfloat time);
	jfloat     JNICALL getAILeashTime(JNIEnv * env, jobject self, jlong object);
	void       JNICALL forceHateTarget(JNIEnv * env, jobject self, jlong object, jlong target);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsHateListNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsHateListNamespace::c)}
	JF("_setAggroImmuneDuration", "(JI)V", setAggroImmuneDuration),
	JF("_isAggroImmune", "(J)Z", isAggroImmune),
	JF("_addHate", "(JJF)V", addHate),
	JF("_addHateDot", "(JJFI)V", addHateDot),
	JF("_setHate", "(JJF)V", setHate),
	JF("_removeHateTarget", "(JJ)V", removeHateTarget),
	JF("_getHate", "(JJ)F", getHate),
	JF("_getMaxHate", "(J)F", getMaxHate),
	JF("_clearHateList", "(J)V", clearHateList),
	JF("_getHateTarget", "(J)J", getHateTarget),
	JF("_getHateList", "(J)[J", getHateList),
	JF("_isOnHateList", "(JJ)Z", isOnHateList),
	JF("_resetHateTimer", "(J)V", resetHateTimer),
	JF("_setAILeashTime", "(JF)V", setAILeashTime),
	JF("_getAILeashTime", "(J)F", getAILeashTime),
	JF("_forceHateTarget", "(JJ)V", forceHateTarget),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================
//
// ScriptMethodsHateList
//
// ======================================================================

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsHateListNamespace::setAggroImmuneDuration(JNIEnv * /*env*/, jobject /*self*/, jlong player, jint duration)
{
	NetworkId const playerNetworkId(player);
	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(playerNetworkId));

	if (playerObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::setAggroImmuneDuration() player(%s) Unable to resolve the object to a PlayerObject.", playerNetworkId.getValueString().c_str()));
		return;
	}

	playerObject->setAggroImmuneDuration(duration);
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsHateListNamespace::isAggroImmune(JNIEnv * /*env*/, jobject /*self*/, jlong player)
{
	NetworkId const playerNetworkId(player);
	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(playerNetworkId));

	if (playerObject == nullptr)
	{
		return JNI_FALSE;
	}

	return playerObject->isAggroImmune();
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsHateListNamespace::addHate(JNIEnv * /*env*/, jobject /*self*/, jlong object, jlong hateTarget, jfloat hate)
{
	NetworkId const networkId(object);
	NetworkId const hateTargetNetworkId(hateTarget);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::addHate() object(%s) hateTarget(%s) hate(%.2f) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str(), hate));
		return;
	}

	if (hateTargetNetworkId == NetworkId::cms_invalid)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::addHate() object(%s) hate(%.2f) Unable to resolve the hateTarget(%s) to a NetworkId.", networkId.getValueString().c_str(), hate, hateTargetNetworkId.getValueString().c_str()));
		return;
	}

	if (networkId == hateTargetNetworkId)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::addHate() object(%s) hateTarget(%s) hate(%.2f) The object is trying to add itself to its hate list.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str(), hate));
		return;
	}

	objectTangibleObject->addHate(hateTargetNetworkId, hate);
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsHateListNamespace::addHateDot(JNIEnv * /*env*/, jobject /*self*/, jlong object, jlong hateTarget, jfloat hate, jint seconds)
{
	NetworkId const networkId(object);
	NetworkId const hateTargetNetworkId(hateTarget);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::addHateDot() object(%s) hateTarget(%s) hate(%.2f) seconds(%d) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str(), hate, seconds));
		return;
	}

	if (hateTargetNetworkId == NetworkId::cms_invalid)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::addHateDot() object(%s) hate(%.2f) seconds(%d) Unable to resolve the hateTarget(%s) to a NetworkId.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str(), hate, seconds));
		return;
	}

	if (networkId == hateTargetNetworkId)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::addHateDot() object(%s) hateTarget(%s) hate(%.2f) seconds(%d) The object is trying to add itself to its hate list.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str(), hate, seconds));
		return;
	}

	LOGC(AiLogManager::isLogging(networkId), "debug_ai", ("ScriptMethodsHateList::addHateDot() object(%s) hateTarget(%s) hate(%.2f+%.2f) seconds(%d)", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str(), objectTangibleObject->getHate(hateTargetNetworkId), hate, seconds));

	objectTangibleObject->addHateOverTime(hateTargetNetworkId, hate, seconds);
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsHateListNamespace::setHate(JNIEnv * /*env*/, jobject /*self*/, jlong object, jlong hateTarget, jfloat hate)
{
	NetworkId const networkId(object);
	NetworkId const hateTargetNetworkId(hateTarget);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		WARNING(true, ("ScriptMethodsHateList::setHate() object(%s) hateTarget(%s) hate(%.2f) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str(), hate));
		return;
	}

	if (hateTargetNetworkId == NetworkId::cms_invalid)
	{
		WARNING(true, ("ScriptMethodsHateList::setHate() object(%s) hateTarget(%s) hate(%.2f) Unable to resolve the hateTarget to a NetworkId.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str(), hate));
		return;
	}

	if (networkId == hateTargetNetworkId)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::setHate() object(%s) hateTarget(%s) hate(%.2f) The object is trying to add itself to its hate list.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str(), hate));
		return;
	}

	LOGC(AiLogManager::isLogging(networkId), "debug_ai", ("ScriptMethodsHateList::setHate() object(%s) hateTarget(%s) hate(%.2f)", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str(), hate));

	objectTangibleObject->setHate(hateTargetNetworkId, hate);
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsHateListNamespace::removeHateTarget(JNIEnv * /*env*/, jobject /*self*/, jlong object, jlong hateTarget)
{
	NetworkId const networkId(object);
	NetworkId const hateTargetNetworkId(hateTarget);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		WARNING(true, ("ScriptMethodsHateList::removeHateTarget() object(%s) hateTarget(%s) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str()));
		return;
	}

	if (hateTargetNetworkId == NetworkId::cms_invalid)
	{
		WARNING(true, ("ScriptMethodsHateList::removeHateTarget() object(%s) Unable to resolve the hateTarget(%s) to a NetworkId.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str()));
		return;
	}

	if (networkId == hateTargetNetworkId)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::removeHateTarget() object(%s) hateTarget(%s) The object is trying to add itself to its hate list.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str()));
		return;
	}

	LOGC(AiLogManager::isLogging(networkId), "debug_ai", ("ScriptMethodsHateList::removeHateTarget() object(%s) hateTarget(%s)", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str()));

	objectTangibleObject->removeHateTarget(hateTargetNetworkId);
}

// ----------------------------------------------------------------------
jfloat JNICALL ScriptMethodsHateListNamespace::getHate(JNIEnv * /*env*/, jobject /*self*/, jlong object, jlong hateTarget)
{
	NetworkId const networkId(object);
	NetworkId const hateTargetNetworkId(hateTarget);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::getHate() object(%s) hateTarget(%s) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str()));
		return 0.0f;
	}

	if (hateTargetNetworkId == NetworkId::cms_invalid)
	{
		WARNING(true, ("ScriptMethodsHateList::getHate() object(%s) Unable to resolve the hateTarget(%s) to a NetworkId.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str()));
		return 0.0f;
	}

	if (networkId == hateTargetNetworkId)
	{
		return 0.0f;
	}

	return objectTangibleObject->getHate(hateTargetNetworkId);
}

// ----------------------------------------------------------------------
jfloat JNICALL ScriptMethodsHateListNamespace::getMaxHate(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::getMaxHate() object(%s) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str()));
		return 0.0f;
	}

	float const maxHate = objectTangibleObject->getMaxHate();

	return maxHate;
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsHateListNamespace::clearHateList(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::clearHateList() Unable to resolve the object(%s) to a TangibleObject.", networkId.getValueString().c_str()));
		return;
	}

	LOGC(AiLogManager::isLogging(networkId), "debug_ai", ("ScriptMethodsHateList::clearHateList() object(%s)", networkId.getValueString().c_str()));

	objectTangibleObject->clearHateList();
}

// ----------------------------------------------------------------------
jlong JNICALL ScriptMethodsHateListNamespace::getHateTarget(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::getHateTarget() Unable to resolve the object(%s) to a TangibleObject.", networkId.getValueString().c_str()));
		return 0;
	}

	CachedNetworkId const & hateTarget = objectTangibleObject->getHateTarget();

	return hateTarget.getValue();
}

// ----------------------------------------------------------------------
jlongArray JNICALL ScriptMethodsHateListNamespace::getHateList(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	LOGC(AiLogManager::isLogging(networkId), "debug_ai", ("ScriptMethodsHateList::getHateList() object(%s)", networkId.getValueString().c_str()));

	if (objectTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::getHateList() Unable to resolve the object(%s) to a TangibleObject.", networkId.getValueString().c_str()));
		return 0;
	}

	HateList::SortedList sortedHateList;
	objectTangibleObject->getSortedHateList(sortedHateList);

	LocalLongArrayRefPtr hateList = createNewLongArray(static_cast<long>(sortedHateList.size()));
	HateList::SortedList::const_iterator iterSortedHateList = sortedHateList.begin();
	int index = 0;

	jlong jlongTmp;
	for (; iterSortedHateList != sortedHateList.end(); ++iterSortedHateList)
	{
		jlongTmp = (iterSortedHateList->first).getValue();
		setLongArrayRegion(*hateList, index, 1, &jlongTmp);
		++index;
	}

	return hateList->getReturnValue();
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsHateListNamespace::isOnHateList(JNIEnv * /*env*/, jobject /*self*/, jlong object, jlong target)
{
	NetworkId const networkId(object);
	NetworkId const targetNetworkId(target);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::isOnHateList() object(%s) target(%s) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str(), targetNetworkId.getValueString().c_str()));
		return JNI_FALSE;
	}

	if (targetNetworkId == NetworkId::cms_invalid)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::isOnHateList() object(%s) Unable to resolve the target(%s) to a NetworkId.", networkId.getValueString().c_str(), targetNetworkId.getValueString().c_str()));
		return JNI_FALSE;
	}

	return objectTangibleObject->isOnHateList(targetNetworkId);
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsHateListNamespace::resetHateTimer(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::resetHateTimer() object(%s) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str()));
		return;
	}

	objectTangibleObject->resetHateTimer();
}


// ----------------------------------------------------------------------
void JNICALL ScriptMethodsHateListNamespace::setAILeashTime(JNIEnv * /*env*/, jobject /*self*/, jlong object, jfloat time )
{
        NetworkId const networkId(object);
        TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

        if (objectTangibleObject == nullptr)
        {
                JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::setAILeashTime() object(%s)  time(%.2f) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str(), time));
                return;
        }

        LOGC(AiLogManager::isLogging(networkId), "debug_ai", 
		("ScriptMethodsHateList::setAILeashTime() object(%s) time(%f)", 
		networkId.getValueString().c_str(), 
		time
	));

        objectTangibleObject->setAILeashTime(time);
}



// ----------------------------------------------------------------------
jfloat JNICALL ScriptMethodsHateListNamespace::getAILeashTime(JNIEnv * /*env*/, jobject /*self*/, jlong object )
{
        NetworkId const networkId(object);
        TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

        if (objectTangibleObject == nullptr)
        {
                JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::setAILeashTime() object(%s)  time(%.2f) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str(), time));
                return 0.0;
        }

        LOGC(AiLogManager::isLogging(networkId), "debug_ai", 
		("ScriptMethodsHateList::setAILeashTime() object(%s) time(%f)", 
		networkId.getValueString().c_str(), 
		time
	));

        return objectTangibleObject->getAILeashTime();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsHateListNamespace::forceHateTarget(JNIEnv * env, jobject self, jlong object, jlong target)
{
	UNREF(env);
	UNREF(self);

	NetworkId const networkId(object);
	NetworkId const hateTargetNetworkId(target);
	TangibleObject * const objectTangibleObject = TangibleObject::getTangibleObject(networkId);

	if (objectTangibleObject == nullptr)
	{
		WARNING(true, ("ScriptMethodsHateList::forceHateTarget() object(%s) hateTarget(%s) Unable to resolve the object to a TangibleObject.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str()));
		return;
	}

	if (hateTargetNetworkId == NetworkId::cms_invalid)
	{
		WARNING(true, ("ScriptMethodsHateList::forceHateTarget() object(%s) Unable to resolve the hateTarget(%s) to a NetworkId.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str()));
		return;
	}

	if (networkId == hateTargetNetworkId)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsHateList::forceHateTarget() object(%s) hateTarget(%s) The object is trying to add itself to its hate list.", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str()));
		return;
	}

	LOGC(AiLogManager::isLogging(networkId), "debug_ai", ("ScriptMethodsHateList::forceHateTarget() object(%s) hateTarget(%s)", networkId.getValueString().c_str(), hateTargetNetworkId.getValueString().c_str()));

	objectTangibleObject->forceHateTarget(hateTargetNetworkId);
}

// ======================================================================
