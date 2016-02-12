//-----------------------------------------------------------------------
// ScriptMethodsClientEffect.cpp
// Copyright 2002, Sony Online Entertainment Inc., all rights reserved.
//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/GameServer.h"
#include "serverGame/ServerObject.h"
#include "serverGame/TangibleObject.h"
#include "sharedObject/CellProperty.h"
#include "sharedMath/PackedArgb.h"
#include "sharedNetworkMessages/ClientEffectMessages.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/PlayMusicMessage.h"


// ======================================================================
// ScriptMethodsClientEffectNamespace
// ======================================================================

namespace ScriptMethodsClientEffectNamespace
{
	bool install();

	jboolean     JNICALL playClientEventObj(JNIEnv *env, jobject self, jlong client, jstring eventType, jlong obj, jstring hardpoint, jobject jTransform);
	jboolean     JNICALL playClientEventObjLimited(JNIEnv *env, jobject self, jlongArray clients, jstring eventType, jlong obj, jstring hardpoint, jobject jTransform);
	jboolean     JNICALL playClientEventLoc(JNIEnv *env, jobject self, jlong client, jstring eventSourceType, jstring eventDestType, jobject loc, jfloat terrainDelta);
	jboolean     JNICALL playClientEventLocLimited(JNIEnv *env, jobject self, jlongArray client, jstring eventSourceType, jstring eventDestType, jobject loc, jfloat terrainDelta);
	jboolean     JNICALL playUiEffect(JNIEnv *env, jobject self, jlong client, jstring uiEffectString);
	jboolean     JNICALL playClientEffectObj(JNIEnv *env, jobject self, jlong client, jstring effectName, jlong obj, jstring hardpoint, jobject jTransform, jstring labelName);
	jboolean     JNICALL playClientEffectObjLimited(JNIEnv *env, jobject self, jlongArray clients, jstring effectName, jlong obj, jstring hardpoint, jobject jTransform, jstring labelName);
	jboolean     JNICALL playClientEffectLoc(JNIEnv *env, jobject self, jlong client, jstring effectName, jobject loc, jfloat terrainDelta, jstring labelName);
	jboolean     JNICALL playClientEffectLocLimited(JNIEnv *env, jobject self, jlongArray clients, jstring effectName, jobject loc, jfloat terrainDelta, jstring labelName);
	jboolean     JNICALL stopClientEffectObjByLabel(JNIEnv *env, jobject self, jlong client, jlong objectEffectIsOn, jstring labelName, jboolean softTerminate);
	jboolean     JNICALL stopClientEffectObjByLabelLimited(JNIEnv *env, jobject self, jlongArray clients, jlong objectEffectIsOn, jstring labelName, jboolean softTerminate);
	jboolean     JNICALL play2dNonLoopingMusic(JNIEnv *env, jobject self, jlong client, jstring musicName);
	jboolean     JNICALL play2dNonLoopingSound(JNIEnv *env, jobject self, jlong client, jstring musicName);
	jboolean     JNICALL playMusicWithParms(JNIEnv *env, jobject self, jlong client, jlong source, jstring musicName, jint playtype, bool loop);
	void         JNICALL sendMusicFlourish(JNIEnv *env, jobject self, jlong performer, jint flourishIndex);
	jboolean     JNICALL createClientProjectile(JNIEnv *env, jobject self, jlong client, jstring weaponObjectTemplateName, jobject start, jobject end, jfloat speed, jfloat expiration, jboolean trail, jint r, jint g, jint b, jint a);
    jboolean     JNICALL createClientProjectileObjectToObject(JNIEnv *env, jobject self, jlong client, jstring weaponObjectTemplateName, jlong source, jstring sourceHardpoint, jlong target, jstring targetHardpoint, jfloat speed, jfloat expiration, jboolean trail, jint r, jint g, jint b, jint a);
    jboolean     JNICALL createClientProjectileLocationToObject(JNIEnv *env, jobject self, jlong client, jstring weaponObjectTemplateName, jobject source, jlong target, jstring targetHardpoint, jfloat speed, jfloat expiration, jboolean trail, jint r, jint g, jint b, jint a);
    jboolean     JNICALL createClientProjectileObjectToLocation(JNIEnv *env, jobject self, jlong client, jstring weaponObjectTemplateName, jlong source, jstring sourceHardpoint, jobject target, jfloat speed, jfloat expiration, jboolean trail, jint r, jint g, jint b, jint a);
	void		 JNICALL addObjectEffect(JNIEnv * env, jobject self, jlong obj, jstring filename, jstring hardpoint, jobject offset, jfloat scale, jstring label);
	void		 JNICALL removeObjectEffect(JNIEnv * env, jobject self, jlong obj, jstring label);
	void         JNICALL removeAllObjectEffects(JNIEnv * env, jobject self, jlong obj);
	jboolean     JNICALL hasObjectEffect(JNIEnv * env, jobject self, jlong obj, jstring label);
}

//========================================================================
// install
//========================================================================

bool ScriptMethodsClientEffectNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsClientEffectNamespace::c)}
	JF("_playClientEventObj", "(JLjava/lang/String;JLjava/lang/String;Lscript/transform;)Z", playClientEventObj),
	JF("_playClientEventObj", "([JLjava/lang/String;JLjava/lang/String;Lscript/transform;)Z", playClientEventObjLimited),
	JF("_playClientEventLoc", "(JLjava/lang/String;Ljava/lang/String;Lscript/location;F)Z", playClientEventLoc),
	JF("_playClientEventLoc", "([JLjava/lang/String;Ljava/lang/String;Lscript/location;F)Z", playClientEventLocLimited),
	JF("_playUiEffect", "(JLjava/lang/String;)Z", playUiEffect),
	JF("_playClientEffectObj", "(JLjava/lang/String;JLjava/lang/String;Lscript/transform;Ljava/lang/String;)Z", playClientEffectObj),
	JF("_playClientEffectObj", "([JLjava/lang/String;JLjava/lang/String;Lscript/transform;Ljava/lang/String;)Z", playClientEffectObjLimited),
	JF("_playClientEffectLoc", "(JLjava/lang/String;Lscript/location;FLjava/lang/String;)Z", playClientEffectLoc),
	JF("_playClientEffectLoc", "([JLjava/lang/String;Lscript/location;FLjava/lang/String;)Z", playClientEffectLocLimited),
	JF("_stopClientEffectObjByLabel", "(JJLjava/lang/String;Z)Z", stopClientEffectObjByLabel),
	JF("_stopClientEffectObjByLabel", "([JJLjava/lang/String;Z)Z", stopClientEffectObjByLabelLimited),
	JF("_play2dNonLoopingMusic", "(JLjava/lang/String;)Z", play2dNonLoopingMusic),
	JF("_play2dNonLoopingSound", "(JLjava/lang/String;)Z", play2dNonLoopingSound),
	JF("_playMusic", "(JJLjava/lang/String;IZ)Z", playMusicWithParms),
	JF("_createClientProjectile", "(JLjava/lang/String;Lscript/location;Lscript/location;FFZIIII)Z", createClientProjectile),
	JF("_createClientProjectileObjectToObject", "(JLjava/lang/String;JLjava/lang/String;JLjava/lang/String;FFZIIII)Z", createClientProjectileObjectToObject),
	JF("_createClientProjectileLocationToObject", "(JLjava/lang/String;Lscript/location;JLjava/lang/String;FFZIIII)Z", createClientProjectileLocationToObject),
	JF("_createClientProjectileObjectToLocation", "(JLjava/lang/String;JLjava/lang/String;Lscript/location;FFZIIII)Z", createClientProjectileObjectToLocation),
	JF("_addObjectEffect", "(JLjava/lang/String;Ljava/lang/String;Lscript/vector;FLjava/lang/String;)V", addObjectEffect),
	JF("_removeObjectEffect", "(JLjava/lang/String;)V", removeObjectEffect),
	JF("_removeAllObjectEffects", "(J)V", removeAllObjectEffects),
	JF("_hasObjectEffect", "(JLjava/lang/String;)Z", hasObjectEffect),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::playClientEventObj(JNIEnv *env, jobject self, jlong client, jstring eventType, jlong objectToPlayEventOn, jstring jHardpoint, jobject jTransform)
{
	JavaStringParam localEventType(eventType);

	ServerObject* so = nullptr;
	if(!JavaLibrary::getObject(client, so))
		return JNI_FALSE;
	Client* c = so->getClient();
	if(!c)
		return JNI_FALSE;
	if (!objectToPlayEventOn)
		return JNI_FALSE;

	ServerObject* target = nullptr;
	if(!JavaLibrary::getObject(objectToPlayEventOn, target))
		return JNI_FALSE;
	if(!target)
		return JNI_FALSE;

	//get the event
	std::string event;
	JavaLibrary::convert(localEventType, event);
	if (event.empty())
		return JNI_FALSE;

	//get the hardpoint
	std::string hp;
	if (nullptr != jHardpoint)
	{
		JavaStringParam localHardpoint(jHardpoint);
		JavaLibrary::convert(localHardpoint, hp);
	}

	//get the transform
	Transform transform;
	if (nullptr != jTransform)
	{
		ScriptConversion::convert(jTransform, transform);
	}

	JAVA_THROW_SCRIPT_EXCEPTION(!hp.empty() && transform != Transform::identity, ("playClientEventObj received valid hardpoint AND valid transform."));

	const NetworkId nid(objectToPlayEventOn);

	if (hp.empty() && transform != Transform::identity)
	{
		PlayClientEventObjectTransformMessage const pceotm(nid, event, transform);
		so->sendToClientsInUpdateRange(pceotm, true);
	}
	else
	{
		PlayClientEventObjectMessage const pceom(nid, event, hp);
		so->sendToClientsInUpdateRange(pceom, true);
	}

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::playClientEventObjLimited(JNIEnv *env, jobject self, jlongArray clients, jstring eventType, jlong objectToPlayEventOn, jstring jHardpoint, jobject jTransform)
{
	JavaStringParam localEventType(eventType);

	std::vector<NetworkId> clientIds;
	if (!ScriptConversion::convert(clients, clientIds))
		return JNI_FALSE;

	if (!objectToPlayEventOn)
		return JNI_FALSE;

	ServerObject* target = nullptr;
	if(!JavaLibrary::getObject(objectToPlayEventOn, target))
		return JNI_FALSE;
	if(!target)
		return JNI_FALSE;

	//get the event
	std::string event;
	JavaLibrary::convert(localEventType, event);
	if (event.empty())
		return JNI_FALSE;

	//get the hardpoint
	std::string hp;
	if (nullptr != jHardpoint)
	{
		JavaStringParam localHardpoint(jHardpoint);
		JavaLibrary::convert(localHardpoint, hp);
	}

	//get the transform
	Transform transform;
	if (nullptr != jTransform)
	{
		ScriptConversion::convert(jTransform, transform);
	}

	JAVA_THROW_SCRIPT_EXCEPTION(!hp.empty() && transform != Transform::identity, ("playClientEventObj received valid hardpoint AND valid transform."));

	const NetworkId nid(objectToPlayEventOn);

	if (hp.empty() && transform != Transform::identity)
	{
		PlayClientEventObjectTransformMessage const pceotm(nid, event, transform);
		ServerObject::sendToSpecifiedClients(pceotm, true, clientIds);
	}
	else
	{
		PlayClientEventObjectMessage const pceom(nid, event, hp);
		ServerObject::sendToSpecifiedClients(pceom, true, clientIds);
	}

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::playClientEventLoc(JNIEnv *env, jobject self, jlong client, jstring eventSourceType, jstring eventDestType, jobject loc, jfloat terrainDelta)
{
	JavaStringParam localEventSourceType(eventSourceType);
	JavaStringParam localEventDestType(eventDestType);

	ServerObject* so = nullptr;
	if(!JavaLibrary::getObject(client, so))
		return JNI_FALSE;
	Client* c = so->getClient();
	if(!c)
		return JNI_FALSE;
	//get the events
	std::string eventSource;
	JavaLibrary::convert(localEventSourceType, eventSource);
	if(eventSource.empty())
		return JNI_FALSE;
	std::string eventDest;
	JavaLibrary::convert(localEventDestType, eventDest);
	if(eventDest.empty())
		return JNI_FALSE;

	//get the location
	Vector locationVec;
	std::string sceneId;
	NetworkId cell;
	if (!ScriptConversion::convert(loc, locationVec, sceneId, cell))
		return JNI_FALSE;

	PlayClientEventLocMessage const pcelm(eventSource, eventDest, locationVec, sceneId, cell, terrainDelta);

	so->sendToClientsInUpdateRange(pcelm, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::playClientEventLocLimited(JNIEnv *env, jobject self, jlongArray clients, jstring eventSourceType, jstring eventDestType, jobject loc, jfloat terrainDelta)
{
	JavaStringParam localEventSourceType(eventSourceType);
	JavaStringParam localEventDestType(eventDestType);

	std::vector<NetworkId> clientIds;
	if (!ScriptConversion::convert(clients, clientIds))
		return JNI_FALSE;

	//get the events
	std::string eventSource;
	JavaLibrary::convert(localEventSourceType, eventSource);
	if(eventSource.empty())
		return JNI_FALSE;
	std::string eventDest;
	JavaLibrary::convert(localEventDestType, eventDest);
	if(eventDest.empty())
		return JNI_FALSE;

	//get the location
	Vector locationVec;
	std::string sceneId;
	NetworkId cell;
	if (!ScriptConversion::convert(loc, locationVec, sceneId, cell))
		return JNI_FALSE;

	PlayClientEventLocMessage const pcelm(eventSource, eventDest, locationVec, sceneId, cell, terrainDelta);

	ServerObject::sendToSpecifiedClients(pcelm, true, clientIds);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::playClientEffectObj(JNIEnv *env, jobject self, jlong client, jstring effectName, jlong objectToPlayEffectOn, jstring jHardpoint, jobject jTransform, jstring labelName)
{
	JavaStringParam localEffectName(effectName);

	ServerObject* so = nullptr;
	if(!JavaLibrary::getObject(objectToPlayEffectOn, so))
		return JNI_FALSE;
	/* TPERRY - This isn't used
	 @TODO: We should take the client parameter out of this function and only use objectToPlayEffectOn
	Client* c = so->getClient();
	if(!c)
		return JNI_FALSE;
	*/
	if (!objectToPlayEffectOn)
		return JNI_FALSE;

	ServerObject* target = nullptr;
	if(!JavaLibrary::getObject(objectToPlayEffectOn, target))
		return JNI_FALSE;
	if(!target)
		return JNI_FALSE;

	//get the effect
	std::string effect;
	JavaLibrary::convert(localEffectName, effect);
	if(effect.empty())
		return JNI_FALSE;

	//get the hardpoint
	std::string hp;
	if (nullptr != jHardpoint)
	{
		JavaStringParam localHardpoint(jHardpoint);
		JavaLibrary::convert(localHardpoint, hp);
	}

	//get the transform
	Transform transform;
	if (nullptr != jTransform)
	{
		ScriptConversion::convert(jTransform, transform);
	}

	//get the optional labelName
	std::string strLabelName;
	if (nullptr != labelName)
	{
		JavaStringParam localLabel(labelName);
		JavaLibrary::convert(localLabel, strLabelName);
	}

	JAVA_THROW_SCRIPT_EXCEPTION(!hp.empty() && transform != Transform::identity, ("playClientEffectObj received valid hardpoint AND valid transform."));


	const NetworkId nid(objectToPlayEffectOn);

	if (hp.empty() && transform != Transform::identity)
	{
		PlayClientEffectObjectTransformMessage const pceotm(nid, effect, transform, strLabelName);
		so->sendToClientsInUpdateRange(pceotm, true);
	}
	else
	{
		PlayClientEffectObjectMessage const pceom(nid, effect, hp, strLabelName);
		so->sendToClientsInUpdateRange(pceom, true);
	}

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::playClientEffectObjLimited(JNIEnv *env, jobject self, jlongArray clients, jstring effectName, jlong objectToPlayEffectOn, jstring jHardpoint, jobject jTransform, jstring labelName)
{
	JavaStringParam localEffectName(effectName);

	std::vector<NetworkId> clientIds;
	if (!ScriptConversion::convert(clients, clientIds))
		return JNI_FALSE;

	if (!objectToPlayEffectOn)
		return JNI_FALSE;

	ServerObject* target = nullptr;
	if(!JavaLibrary::getObject(objectToPlayEffectOn, target))
		return JNI_FALSE;
	if(!target)
		return JNI_FALSE;

	//get the effect
	std::string effect;
	JavaLibrary::convert(localEffectName, effect);
	if(effect.empty())
		return JNI_FALSE;

	//get the hardpoint
	std::string hp;
	if (nullptr != jHardpoint)
	{
		JavaStringParam localHardpoint(jHardpoint);
		JavaLibrary::convert(localHardpoint, hp);
	}

	//get the transform
	Transform transform;
	if (nullptr != jTransform)
	{
		ScriptConversion::convert(jTransform, transform);
	}

	//get the optional labelName
	std::string strLabelName;
	if (nullptr != labelName)
	{
		JavaStringParam localLabel(labelName);
		JavaLibrary::convert(localLabel, strLabelName);
	}

	JAVA_THROW_SCRIPT_EXCEPTION(!hp.empty() && transform != Transform::identity, ("playClientEffectObj received valid hardpoint AND valid transform."));

	const NetworkId nid(objectToPlayEffectOn);

	if (hp.empty() && transform != Transform::identity)
	{
		PlayClientEffectObjectTransformMessage const pceotm(nid, effect, transform, strLabelName);
		ServerObject::sendToSpecifiedClients(pceotm, true, clientIds);
	}
	else
	{
		PlayClientEffectObjectMessage const pceom(nid, effect, hp, strLabelName);
		ServerObject::sendToSpecifiedClients(pceom, true, clientIds);
	}

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::playClientEffectLoc(JNIEnv *env, jobject self, jlong client, jstring effectName, jobject loc, jfloat terrainDelta, jstring labelName)
{
	JavaStringParam localEffectName(effectName);

	ServerObject* so = nullptr;
	if(!JavaLibrary::getObject(client, so))
		return JNI_FALSE;

	/* TPERRY - This isn't used
	 @TODO: We should take the client parameter out of this function and only use objectToPlayEffectOn
	Client* c = so->getClient();
	if(!c)
		return JNI_FALSE;
	*/
	//get the effect
	std::string effect;
	JavaLibrary::convert(localEffectName, effect);
	if(effect.empty())
		return JNI_FALSE;

	//get the location
	Vector locationVec;
	std::string sceneId;
	NetworkId cell;
	if (!ScriptConversion::convert(loc, locationVec, sceneId, cell))
		return JNI_FALSE;

	//get the optional labelName
	std::string strLabelName;
	if (nullptr != labelName)
	{
		JavaStringParam localLabel(labelName);
		JavaLibrary::convert(localLabel, strLabelName);
	}

	PlayClientEffectLocMessage const pcelm(effect, locationVec, sceneId, cell, terrainDelta, strLabelName);

	so->sendToClientsInUpdateRange(pcelm, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::playClientEffectLocLimited(JNIEnv *env, jobject self, jlongArray clients, jstring effectName, jobject loc, jfloat terrainDelta, jstring labelName)
{
	JavaStringParam localEffectName(effectName);

	std::vector<NetworkId> clientIds;
	if (!ScriptConversion::convert(clients, clientIds))
		return JNI_FALSE;

	//get the effect
	std::string effect;
	JavaLibrary::convert(localEffectName, effect);
	if(effect.empty())
		return JNI_FALSE;

	//get the location
	Vector locationVec;
	std::string sceneId;
	NetworkId cell;
	if (!ScriptConversion::convert(loc, locationVec, sceneId, cell))
		return JNI_FALSE;

	//get the optional labelName
	std::string strLabelName;
	if (nullptr != labelName)
	{
		JavaStringParam localLabel(labelName);
		JavaLibrary::convert(localLabel, strLabelName);
	}

	PlayClientEffectLocMessage const pcelm(effect, locationVec, sceneId, cell, terrainDelta, strLabelName);

	ServerObject::sendToSpecifiedClients(pcelm, true, clientIds);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::playUiEffect(JNIEnv * /*env*/, jobject /*self*/, jlong client, jstring uiEffectString)
{
	JavaStringParam localUiEffectString(uiEffectString);

	ServerObject* so = nullptr;
	if(!JavaLibrary::getObject(client, so))
	{
		NetworkId const networkId(client);
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("playUiEffect - could not find player %s.", networkId.getValueString().c_str()));
		return JNI_FALSE;
	}

	//get the event
	std::string uiEffect;
	JavaLibrary::convert(localUiEffectString, uiEffect);
	if (uiEffect.empty())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("playUiEffect - empty effect string for %s.", so->getDebugInformation().c_str()));
		return JNI_FALSE;
	}

	//format the message.
	MessageQueueGenericValueType<std::pair<NetworkId, std::string> > * const msg = new MessageQueueGenericValueType<std::pair<NetworkId, std::string> >(std::make_pair(so->getNetworkId(), uiEffect));
	so->appendMessage(CM_uiPlayEffect, 0.0f, msg);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::stopClientEffectObjByLabel(JNIEnv *env, jobject self, jlong client, jlong objectEffectIsOn, jstring labelName, jboolean softTerminate)
{
	JavaStringParam localLabel(labelName);

	ServerObject* so = nullptr;
	if(!JavaLibrary::getObject(objectEffectIsOn, so))
		return JNI_FALSE;

	/* TPERRY - This isn't used
	 @TODO: We should take the client parameter out of this function and only use objectToPlayEffectOn
	Client* c = so->getClient();
	if(!c)
		return JNI_FALSE;
	*/

	if (!objectEffectIsOn)
		return JNI_FALSE;

	ServerObject* target = nullptr;
	if(!JavaLibrary::getObject(objectEffectIsOn, target))
		return JNI_FALSE;
	if(!target)
		return JNI_FALSE;

	//get the effect
	std::string strLabelName;
	JavaLibrary::convert(localLabel, strLabelName);
	if(strLabelName.empty())
		return JNI_FALSE;

	const NetworkId nid(objectEffectIsOn);

	StopClientEffectObjectByLabelMessage const pceotm(nid, strLabelName, softTerminate);
	so->sendToClientsInUpdateRange(pceotm, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::stopClientEffectObjByLabelLimited(JNIEnv *env, jobject self, jlongArray clients, jlong objectEffectIsOn, jstring labelName, jboolean softTerminate)
{
	JavaStringParam localLabel(labelName);

	std::vector<NetworkId> clientIds;
	if (!ScriptConversion::convert(clients, clientIds))
		return JNI_FALSE;

	if (!objectEffectIsOn)
		return JNI_FALSE;

	ServerObject* target = nullptr;
	if(!JavaLibrary::getObject(objectEffectIsOn, target))
		return JNI_FALSE;
	if(!target)
		return JNI_FALSE;

	//get the effect
	std::string strLabelName;
	JavaLibrary::convert(localLabel, strLabelName);
	if(strLabelName.empty())
		return JNI_FALSE;

	const NetworkId nid(objectEffectIsOn);

	StopClientEffectObjectByLabelMessage const pceotm(nid, strLabelName, softTerminate);
	ServerObject::sendToSpecifiedClients(pceotm, true, clientIds);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::play2dNonLoopingMusic(JNIEnv *env, jobject self, jlong client, jstring musicName)
{
	JavaStringParam localMusicName(musicName);

	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(client, so))
		return JNI_FALSE;
	Client* c = so->getClient();
	if (!c)
		return JNI_FALSE;

	std::string music;
	JavaLibrary::convert(localMusicName, music);
	if (music.empty())
		return JNI_FALSE;

	PlayMusicMessage const msg(music, NetworkId::cms_invalid, 0, false);
	c->send(msg, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::play2dNonLoopingSound(JNIEnv *env, jobject self, jlong client, jstring musicName)
{
	JavaStringParam localMusicName(musicName);

	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(client, so))
		return JNI_FALSE;
	Client* c = so->getClient();
	if (!c)
		return JNI_FALSE;

	std::string music;
	JavaLibrary::convert(localMusicName, music);
	if (music.empty())
		return JNI_FALSE;

	PlayMusicMessage const msg(music, NetworkId::cms_invalid, 1, false);
	c->send(msg, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::playMusicWithParms(JNIEnv *env, jobject self, jlong client, jlong source, jstring musicName, jint playType, bool loop)
{
	JavaStringParam localMusicName(musicName);

	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(client, so))
		return JNI_FALSE;
	Client* c = so->getClient();
	if (!c)
		return JNI_FALSE;

	std::string music;
	JavaLibrary::convert(localMusicName, music);
	if (music.empty())
		return JNI_FALSE;

	PlayMusicMessage const msg(music, NetworkId(source), playType, loop);
	c->send(msg, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::createClientProjectile(JNIEnv *env, jobject self, jlong client, jstring weaponObjectTemplateName, jobject start, jobject end, jfloat speed, jfloat expiration, jboolean trail, jint r, jint g, jint b, jint a)
{
	JavaStringParam localWeaponObjectTemplateName(weaponObjectTemplateName);

	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(client, so))
		return JNI_FALSE;

	// verify a client exists
	Client * c = so->getClient();
	if (!c)
		return JNI_FALSE;

	// get the weapon object template name string
	std::string weaponObjectTemplateNameString;
	JavaLibrary::convert(localWeaponObjectTemplateName, weaponObjectTemplateNameString);
	if (weaponObjectTemplateNameString.empty())
		return JNI_FALSE;

	// get the start location
	Vector startLocationVec;
	std::string startSceneId;
	NetworkId startCell;
	if (!ScriptConversion::convert(start, startLocationVec, startSceneId, startCell))
		return JNI_FALSE;

	// get the end location
	Vector endLocationVec;
	std::string endSceneId;
	NetworkId endCell;
	if (!ScriptConversion::convert(end, endLocationVec, endSceneId, endCell))
		return JNI_FALSE;

	CreateClientProjectileMessage const ccpm(weaponObjectTemplateNameString, startLocationVec, startCell, endLocationVec,
		speed, expiration, trail, PackedArgb(static_cast<uint8>(a), static_cast<uint8>(r), static_cast<uint8>(g), static_cast<uint8>(b)).getArgb());

	so->sendToClientsInUpdateRange(ccpm, true);

	return JNI_TRUE;
}
//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::createClientProjectileObjectToObject(JNIEnv *env, jobject self, jlong client, jstring weaponObjectTemplateName, jlong source, jstring sourceHardpoint, jlong target, jstring targetHardpoint, jfloat speed, jfloat expiration, jboolean trail, jint r, jint g, jint b, jint a)
{
	JavaStringParam localWeaponObjectTemplateName(weaponObjectTemplateName);

	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(client, so))
		return JNI_FALSE;

	// verify a client exists
	Client * c = so->getClient();
	if (!c)
		return JNI_FALSE;

	// get the weapon object template name string
	std::string weaponObjectTemplateNameString;
	JavaLibrary::convert(localWeaponObjectTemplateName, weaponObjectTemplateNameString);
	if (weaponObjectTemplateNameString.empty())
		return JNI_FALSE;

	// Get our source object
	ServerObject* sourceObject = nullptr;
	if (!JavaLibrary::getObject(source, sourceObject))
		return JNI_FALSE;

	NetworkId sourceId = sourceObject->getNetworkId();

	// The target of our effect
	ServerObject* targetObject = nullptr;
	if (!JavaLibrary::getObject(target, targetObject))
		return JNI_FALSE;

	NetworkId targetId = targetObject->getNetworkId();

	// Optional Source hardpoint
	JavaStringParam localSourceHardpointName(sourceHardpoint);
	std::string sourceHardpointString;
	JavaLibrary::convert(localSourceHardpointName, sourceHardpointString);

	// Optional Target hardpoint
	JavaStringParam localTargetHardpointName(targetHardpoint);
	std::string targetHardpointString;
	JavaLibrary::convert(localTargetHardpointName, targetHardpointString);

	// Source's Cell ID
	CellProperty const * const cellProperty = sourceObject->getParentCell();
	Object const * const cellObject = (cellProperty != nullptr) ? &cellProperty->getOwner() : nullptr;
	NetworkId const & networkIdForCellOrWorld = (cellObject != nullptr) ? cellObject->getNetworkId() : NetworkId::cms_invalid;

	CreateClientProjectileObjectToObjectMessage const ccpmoto(weaponObjectTemplateNameString, sourceId, sourceHardpointString, networkIdForCellOrWorld, targetId, targetHardpointString,
		speed, expiration, trail, PackedArgb(static_cast<uint8>(a), static_cast<uint8>(r), static_cast<uint8>(g), static_cast<uint8>(b)).getArgb());

	so->sendToClientsInUpdateRange(ccpmoto, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL  ScriptMethodsClientEffectNamespace::createClientProjectileObjectToLocation(JNIEnv *env, jobject self, jlong client, jstring weaponObjectTemplateName, jlong source, jstring sourceHardpoint, jobject target, jfloat speed, jfloat expiration, jboolean trail, jint r, jint g, jint b, jint a)
{
	JavaStringParam localWeaponObjectTemplateName(weaponObjectTemplateName);

	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(client, so))
		return JNI_FALSE;

	// verify a client exists
	Client * c = so->getClient();
	if (!c)
		return JNI_FALSE;

	// get the weapon object template name string
	std::string weaponObjectTemplateNameString;
	JavaLibrary::convert(localWeaponObjectTemplateName, weaponObjectTemplateNameString);
	if (weaponObjectTemplateNameString.empty())
		return JNI_FALSE;

	// Get our source object
	ServerObject* sourceObject = nullptr;
	if (!JavaLibrary::getObject(source, sourceObject))
		return JNI_FALSE;

	NetworkId sourceId = sourceObject->getNetworkId();

	// Optional Source hardpoint
	JavaStringParam localSourceHardpointName(sourceHardpoint);
	std::string sourceHardpointString;
	JavaLibrary::convert(localSourceHardpointName, sourceHardpointString);

	// The target of our effect
	Vector targetLocationVec;
	std::string startSceneId;
	NetworkId startCell;
	if (!ScriptConversion::convert(target, targetLocationVec, startSceneId, startCell))
		return JNI_FALSE;

	// Source's Cell ID
	CellProperty const * const cellProperty = sourceObject->getParentCell();
	Object const * const cellObject = (cellProperty != nullptr) ? &cellProperty->getOwner() : nullptr;
	NetworkId const & networkIdForCellOrWorld = (cellObject != nullptr) ? cellObject->getNetworkId() : NetworkId::cms_invalid;

	CreateClientProjectileObjectToLocationMessage const ccpmotl(weaponObjectTemplateNameString, sourceId, sourceHardpointString, networkIdForCellOrWorld, targetLocationVec,
		speed, expiration, trail, PackedArgb(static_cast<uint8>(a), static_cast<uint8>(r), static_cast<uint8>(g), static_cast<uint8>(b)).getArgb());

	so->sendToClientsInUpdateRange(ccpmotl, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsClientEffectNamespace::createClientProjectileLocationToObject(JNIEnv *env, jobject self, jlong client, jstring weaponObjectTemplateName, jobject source, jlong target, jstring targetHardpoint, jfloat speed, jfloat expiration, jboolean trail, jint r, jint g, jint b, jint a)
{
	JavaStringParam localWeaponObjectTemplateName(weaponObjectTemplateName);

	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(client, so))
		return JNI_FALSE;

	// verify a client exists
	Client * c = so->getClient();
	if (!c)
		return JNI_FALSE;

	// get the weapon object template name string
	std::string weaponObjectTemplateNameString;
	JavaLibrary::convert(localWeaponObjectTemplateName, weaponObjectTemplateNameString);
	if (weaponObjectTemplateNameString.empty())
		return JNI_FALSE;

	// Get our target object
	ServerObject* targetObject = nullptr;
	if (!JavaLibrary::getObject(target, targetObject))
		return JNI_FALSE;

	NetworkId targetId = targetObject->getNetworkId();


	// Optional Target hardpoint
	JavaStringParam localTargetHardpointName(targetHardpoint);
	std::string targetHardpointString;
	JavaLibrary::convert(localTargetHardpointName, targetHardpointString);

	Vector sourceLocationVec;
	std::string startSceneId;
	NetworkId startCell;
	if (!ScriptConversion::convert(source, sourceLocationVec, startSceneId, startCell))
		return JNI_FALSE;

	// Target's Cell ID
	CellProperty const * const cellProperty = targetObject->getParentCell();
	Object const * const cellObject = (cellProperty != nullptr) ? &cellProperty->getOwner() : nullptr;
	NetworkId const & networkIdForCellOrWorld = (cellObject != nullptr) ? cellObject->getNetworkId() : NetworkId::cms_invalid;

	CreateClientProjectileLocationToObjectMessage const ccpmlto(weaponObjectTemplateNameString, sourceLocationVec,  networkIdForCellOrWorld, targetId, targetHardpointString,
		speed, expiration, trail, PackedArgb(static_cast<uint8>(a), static_cast<uint8>(r), static_cast<uint8>(g), static_cast<uint8>(b)).getArgb());

	so->sendToClientsInUpdateRange(ccpmlto, true);

	return JNI_TRUE;
}

void JNICALL ScriptMethodsClientEffectNamespace::addObjectEffect(JNIEnv * env, jobject self, jlong obj, jstring filename, jstring hardpoint, jobject offset, jfloat scale, jstring label)
{
	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(obj, so))
		return;

	TangibleObject * tang = so->asTangibleObject();
	if(!tang)
		return;

	JavaStringParam javaFilename(filename);
	std::string localFilename;
	JavaLibrary::convert(javaFilename, localFilename);

	JavaStringParam javaLabel(label);
	std::string localLabel;
	JavaLibrary::convert(javaLabel, localLabel);

	JavaStringParam javaHardpoint(hardpoint);
	std::string localHardpoint;
	JavaLibrary::convert(javaHardpoint, localHardpoint);

	Vector offsetVector;
	ScriptConversion::convert(offset, offsetVector);

	if(localLabel.empty() || localFilename.empty())
		return;

	tang->addObjectEffect(localFilename, localHardpoint, offsetVector, scale, localLabel);

}
void JNICALL ScriptMethodsClientEffectNamespace::removeObjectEffect(JNIEnv * env, jobject self, jlong obj, jstring label)
{
	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(obj, so))
		return;

	TangibleObject * tang = so->asTangibleObject();
	if(!tang)
		return;

	JavaStringParam javaLabel(label);
	std::string localLabel;
	JavaLibrary::convert(javaLabel, localLabel);

	if(localLabel.empty())
		return;

	tang->removeObjectEffect(localLabel);
}

void JNICALL ScriptMethodsClientEffectNamespace::removeAllObjectEffects(JNIEnv * env, jobject self, jlong obj)
{
	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(obj, so))
		return;

	TangibleObject * tang = so->asTangibleObject();
	if(!tang)
		return;

	tang->removeAllObjectEffects();
}

jboolean JNICALL ScriptMethodsClientEffectNamespace::hasObjectEffect(JNIEnv * env, jobject self, jlong obj, jstring label)
{
	ServerObject* so = nullptr;
	if (!JavaLibrary::getObject(obj, so))
		return JNI_FALSE;

	TangibleObject * tang = so->asTangibleObject();
	if(!tang)
		return JNI_FALSE;

	JavaStringParam javaLabel(label);
	std::string localLabel;
	JavaLibrary::convert(javaLabel, localLabel);

	if(localLabel.empty())
		return JNI_FALSE;

	return tang->hasObjectEffect(localLabel);
}

//-----------------------------------------------------------------------
