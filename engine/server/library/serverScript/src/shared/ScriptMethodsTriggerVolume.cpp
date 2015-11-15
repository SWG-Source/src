// ScriptMethodsTriggerVolume.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "JavaLibrary.h"
#include "serverGame/AiLogManager.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/TriggerVolume.h"
#include "sharedLog/Log.h"
#include "sharedTerrain/TerrainObject.h"
#include "UnicodeUtils.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsTriggerVolumeNamespace
// ======================================================================

namespace ScriptMethodsTriggerVolumeNamespace
{
	bool install();

	void         JNICALL createTriggerVolume(JNIEnv * env, jobject script, jlong target, jstring triggerVolumeName, jfloat radius, jboolean isPromiscuous);
	jlongArray   JNICALL getTriggerVolumeContents(JNIEnv * env, jobject script, jlong target, jstring triggerVolumeName);
	jboolean     JNICALL isInTriggerVolume(JNIEnv * env, jobject script, jlong owner, jstring triggerVolumeName, jlong target);
	jfloat       JNICALL getTriggerVolumeRadius(JNIEnv * env, jobject script, jlong targetObject, jstring triggerVolumeName);
	jboolean     JNICALL hasTriggerVolume(JNIEnv * env, jobject script, jlong target, jstring triggerVolumeName);
	jboolean     JNICALL isTriggerVolumePromiscuous(JNIEnv * env, jobject script, jlong me, jstring triggerVolumeName);
	void         JNICALL listenToTriggerEventsFrom(JNIEnv * env, jobject script, jlong self, jstring volumeName, jlong who);
	void         JNICALL removeTriggerVolume(JNIEnv * env, jobject script, jlong target, jstring triggerVolumeName);
	void         JNICALL setTriggerVolumeIsPromiscuous(JNIEnv * env, jobject script, jlong self, jstring triggerVolumeName, jboolean isPromiscuous);
	void         JNICALL stopListeningToTriggerEventsFrom(JNIEnv * env, jobject script, jlong self, jstring volumeName, jlong who);
	jboolean     JNICALL expelFromTriggerVolume(JNIEnv * env, jobject script, jlong self, jstring triggerVolumeName, jlong targetObject);
	jboolean     JNICALL confineToTriggerVolume(JNIEnv * env, jobject script, jlong self, jstring triggerVolumeName, jlong targetObject);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsTriggerVolumeNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsTriggerVolumeNamespace::c)}
	JF("__createTriggerVolume", "(JLjava/lang/String;FZ)V", createTriggerVolume),
	JF("_getTriggerVolumeContents", "(JLjava/lang/String;)[J", getTriggerVolumeContents),
	JF("_isInTriggerVolume", "(JLjava/lang/String;J)Z", isInTriggerVolume),
	JF("_getTriggerVolumeRadius", "(JLjava/lang/String;)F", getTriggerVolumeRadius),
	JF("_hasTriggerVolume", "(JLjava/lang/String;)Z", hasTriggerVolume),
	JF("_isTriggerVolumePromiscuous", "(JLjava/lang/String;)Z", isTriggerVolumePromiscuous),
	JF("__addTriggerVolumeEventSource", "(JLjava/lang/String;J)V", listenToTriggerEventsFrom),
	JF("__removeTriggerVolume", "(JLjava/lang/String;)V", removeTriggerVolume),
	JF("__setTriggerVolumePromiscuous", "(JLjava/lang/String;Z)V", setTriggerVolumeIsPromiscuous),
	JF("__removeTriggerVolumeEventSource", "(JLjava/lang/String;J)V", stopListeningToTriggerEventsFrom),
	JF("_expelFromTriggerVolume", "(JLjava/lang/String;J)Z", expelFromTriggerVolume),
	JF("_confineToTriggerVolume", "(JLjava/lang/String;J)Z", confineToTriggerVolume),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

void JNICALL ScriptMethodsTriggerVolumeNamespace::createTriggerVolume(JNIEnv * env, jobject script, jlong targetObject, jstring triggerVolumeName, jfloat radius, jboolean isPromiscuous)
{
	JavaStringParam localTriggerVolumeName(triggerVolumeName);

	if (script == 0 || env == 0)
		return;

	ServerObject * target = 0;
	if (!JavaLibrary::getObject(targetObject, target))
		return;

	std::string name;
	if(!JavaLibrary::convert(localTriggerVolumeName, name))
		return;

	float r = radius;
	target->createTriggerVolume(r, name, isPromiscuous);

	LOGC(AiLogManager::isLogging(target->getNetworkId()), "debug_ai", ("ScriptMethodsTriggerVolume::createTriggerVolume() object(%s:%s) volumeName(%s) radius(%.2f) promiscuous(%s)", target->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(target->getEncodedObjectName()).c_str(), name.c_str(), radius, isPromiscuous ? "yes" : "no"));
}

//-----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsTriggerVolumeNamespace::getTriggerVolumeContents(JNIEnv * env, jobject script, jlong targetObject, jstring triggerVolumeName)
{
	JavaStringParam localTriggerVolumeName(triggerVolumeName);

	if(script == 0 || env == 0)
		return 0;

	ServerObject * target = 0;
	if (!JavaLibrary::getObject(targetObject, target))
		return 0;

	std::string name;
	if(!JavaLibrary::convert(localTriggerVolumeName, name))
		return 0;

	const TriggerVolume * t = target->getTriggerVolume(name);
	if(t == 0)
		return 0;

	int count = t->getContents().size();
	LocalLongArrayRefPtr items = createNewLongArray(count);
	TriggerVolume::ContentsSet::const_iterator begin = t->getContents().begin();
	TriggerVolume::ContentsSet::const_iterator end = t->getContents().end();
	TriggerVolume::ContentsSet::const_iterator i;
	int index = 0;
	jlong jlongTmp;
	for (i = begin; i != end; ++i)
	{
		jlongTmp = ((*i)->getNetworkId()).getValue();
		setLongArrayRegion(*items, index, 1, &jlongTmp);
		index++;
	}

	return items->getReturnValue();
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTriggerVolumeNamespace::isInTriggerVolume(JNIEnv * /*env*/, jobject /*script*/, jlong owner, jstring triggerVolumeName, jlong target)
{
	ServerObject * ownerServerObject = 0;
	if (!JavaLibrary::getObject(owner, ownerServerObject))
		return JNI_FALSE;

	ServerObject * targetServerObject = 0;
	if (!JavaLibrary::getObject(target, targetServerObject))
		return JNI_FALSE;

	JavaStringParam localTriggerVolumeName(triggerVolumeName);
	std::string name;
	if(!JavaLibrary::convert(localTriggerVolumeName, name))
		return JNI_FALSE;

	TriggerVolume const * const triggerVolume = ownerServerObject->getTriggerVolume(name);
	if(triggerVolume == 0)
		return JNI_FALSE;

	TriggerVolume::ContentsSet const & contents = triggerVolume->getContents();

	return (contents.find(targetServerObject) != contents.end());
}

//-----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsTriggerVolumeNamespace::getTriggerVolumeRadius(JNIEnv * env, jobject script, jlong targetObject, jstring triggerVolumeName)
{
	JavaStringParam localTriggerVolumeName(triggerVolumeName);

	if(script == 0 || env == 0)
		return 0;

	ServerObject * target = 0;
	if (!JavaLibrary::getObject(targetObject, target))
		return 0;

	std::string name;
	if(!JavaLibrary::convert(localTriggerVolumeName, name))
		return 0;

	const TriggerVolume * t = target->getTriggerVolume(name);
	if(t == 0)
		return 0;

	return t->getRadius();
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTriggerVolumeNamespace::hasTriggerVolume(JNIEnv * env, jobject script, jlong targetObject, jstring triggerVolumeName)
{
	JavaStringParam localTriggerVolumeName(triggerVolumeName);

	if(script == 0 || env == 0)
		return JNI_FALSE;

	ServerObject * target = 0;
	if (!JavaLibrary::getObject(targetObject, target))
		return JNI_FALSE;

	std::string name;
	if(!JavaLibrary::convert(localTriggerVolumeName, name))
		 return JNI_FALSE;

	const TriggerVolume * t = target->getTriggerVolume(name);

	if(t == 0)
		 return JNI_FALSE;

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTriggerVolumeNamespace::isTriggerVolumePromiscuous(JNIEnv * env, jobject script, jlong me, jstring triggerVolumeName)
{
	JavaStringParam localTriggerVolumeName(triggerVolumeName);

	jboolean result = JNI_FALSE;
	if(script == 0 || env == 0)
		return result;

	ServerObject * target = 0;
	if (!JavaLibrary::getObject(me, target))
		return result;

	std::string name;
	if(!JavaLibrary::convert(localTriggerVolumeName, name))
		 return result;

	const TriggerVolume * t = target->getTriggerVolume(name);

	if(t == 0)
		 return result;

	result = t->getPromiscuous();

	return result;
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsTriggerVolumeNamespace::listenToTriggerEventsFrom(JNIEnv * env, jobject script, jlong self, jstring volumeName, jlong who)
{
	JavaStringParam localVolumeName(volumeName);

	ServerObject * me = 0;
	if (!JavaLibrary::getObject(self, me))
		return;

	ServerObject * target = 0;
	if(!JavaLibrary::getObject(who, target))
		return;

	std::string name;
	if(!JavaLibrary::convert(localVolumeName, name))
		return;

	TriggerVolume * t = me->getTriggerVolume(name);
	if(! t)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsTriggerVolume::listenToTriggerEventsFrom() Unable to find triggerVolume(%s) for %s listening for %s", name.c_str(), me->getNetworkId().getValueString().c_str(), target->getNetworkId().getValueString().c_str()));
		return;
	}

	t->addEventSource(target->getNetworkId());

	LOGC(AiLogManager::isLogging(me->getNetworkId()), "debug_ai", ("ScriptMethodsTriggerVolume::listenToTriggerEventsFrom() object(%s:%s) from(%s:%s) volumeName(%s)", me->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(me->getEncodedObjectName()).c_str(), target->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(target->getEncodedObjectName()).c_str(), name.c_str()));
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsTriggerVolumeNamespace::removeTriggerVolume(JNIEnv * env, jobject script, jlong targetObject, jstring triggerVolumeName)
{
	JavaStringParam localTriggerVolumeName(triggerVolumeName);

	if(script == 0 || env == 0)
		return;

	ServerObject * target = 0;
	if (!JavaLibrary::getObject(targetObject, target))
		return;

	std::string name;
	if(!JavaLibrary::convert(localTriggerVolumeName, name))
		return;

	LOGC(AiLogManager::isLogging(target->getNetworkId()), "debug_ai", ("ScriptMethodsTriggerVolume::removeTriggerVolume() object(%s:%s) volumeName(%s)", target->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(target->getEncodedObjectName()).c_str(), name.c_str()));

	target->removeTriggerVolume(name);
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsTriggerVolumeNamespace::setTriggerVolumeIsPromiscuous(JNIEnv * env, jobject script, jlong self, jstring volumeName, jboolean isPromiscuous)
{
	JavaStringParam localVolumeName(volumeName);

	ServerObject * me = 0;
	if (!JavaLibrary::getObject(self, me))
		return;

	std::string name;
	if(!JavaLibrary::convert(localVolumeName, name))
		return;

	TriggerVolume * t = me->getTriggerVolume(name);
	if(! t)
		return;

	t->setPromiscuous(isPromiscuous);
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsTriggerVolumeNamespace::stopListeningToTriggerEventsFrom(JNIEnv * env, jobject script, jlong self, jstring volumeName, jlong who)
{
	JavaStringParam localVolumeName(volumeName);

	ServerObject * me = 0;
	if (!JavaLibrary::getObject(self, me))
		return;

	ServerObject * target = 0;
	if(!JavaLibrary::getObject(who, target))
		return;

	std::string name;
	if(!JavaLibrary::convert(localVolumeName, name))
		return;

	TriggerVolume * t = me->getTriggerVolume(name);
	if(! t)
		return;

	t->removeEventSource(target->getNetworkId());

	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("ScriptMethodsTriggerVolume::stopListeningToTriggerEventsFrom() object(%s:%s) from(%s:%s) volumeName(%s)", me->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(me->getEncodedObjectName()).c_str(), target->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(target->getEncodedObjectName()).c_str(), name.c_str()));
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTriggerVolumeNamespace::expelFromTriggerVolume(JNIEnv * env, jobject script, jlong self, jstring triggerVolumeName, jlong targetObject)
{
	const float EXPEL_DISTANCE = 10.0f;

	JavaStringParam localTriggerVolumeName(triggerVolumeName);

	if (script == 0 || env == 0)
		return JNI_FALSE;

	ServerObject * me = 0;
	if (!JavaLibrary::getObject(self, me))
		return JNI_FALSE;

	ServerObject * target = 0;
	if (!JavaLibrary::getObject(targetObject, target))
		return JNI_FALSE;

	std::string name;
	if (!JavaLibrary::convert(localTriggerVolumeName, name))
		return JNI_FALSE;

	const TriggerVolume * triggerVolume = me->getTriggerVolume(name);
	if (triggerVolume == 0)
		return JNI_FALSE;

	Vector const &trigLoc = triggerVolume->getOwner().getPosition_w();
	Vector newLoc(target->getPosition_w());
	newLoc -= trigLoc;
	newLoc.normalize();
	newLoc *= triggerVolume->getRadius()+EXPEL_DISTANCE;
	newLoc += trigLoc;
	IGNORE_RETURN(TerrainObject::getInstance()->getHeightForceChunkCreation(newLoc, newLoc.y));


	target->teleportObject(newLoc, NetworkId::cms_invalid, "", Vector::zero, "");
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTriggerVolumeNamespace::confineToTriggerVolume(JNIEnv * env, jobject script, jlong self, jstring triggerVolumeName, jlong targetObject)
{
	const float CONFINE_DISTANCE = 10.0f;

	JavaStringParam localTriggerVolumeName(triggerVolumeName);

	if (script == 0 || env == 0)
		return JNI_FALSE;

	ServerObject * me = 0;
	if (!JavaLibrary::getObject(self, me))
		return JNI_FALSE;

	ServerObject * target = 0;
	if (!JavaLibrary::getObject(targetObject, target))
		return JNI_FALSE;

	std::string name;
	if (!JavaLibrary::convert(localTriggerVolumeName, name))
		return JNI_FALSE;

	const TriggerVolume * triggerVolume = me->getTriggerVolume(name);
	if (triggerVolume == 0)
		return JNI_FALSE;

	Vector const &trigLoc = triggerVolume->getOwner().getPosition_w();
	Vector newLoc(target->getPosition_w());
	newLoc -= trigLoc;
	newLoc.normalize();
	newLoc *= triggerVolume->getRadius()-CONFINE_DISTANCE;
	newLoc += trigLoc;

	target->teleportObject(newLoc, NetworkId::cms_invalid, "", Vector::zero, "");
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

