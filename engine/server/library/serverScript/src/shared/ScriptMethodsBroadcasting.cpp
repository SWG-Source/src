//========================================================================
//
// ScriptMethodsBroadcasting.cpp - implements script methods dealing
// with broadcasting
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/ServerObject.h"
#include "sharedObject/CachedNetworkId.h"

#include <limits>

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsBroadcastingNamespace
// ======================================================================

namespace ScriptMethodsBroadcastingNamespace
{
	bool install();

	void JNICALL listenToMessage(JNIEnv *env, jobject self, jlong listener, jlong emitter, jstring messageHandlerName);
	void JNICALL stopListeningToMessage(JNIEnv *env, jobject self, jlong listener, jlong emitter, jstring messageHandlerName);
	jlongArray JNICALL getMessageListeners(JNIEnv *env, jobject self, jlong emitter, jstring messageHandlerName);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsBroadcastingNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsBroadcastingNamespace::c)}
	JF("_listenToMessage", "(JJLjava/lang/String;)V", listenToMessage),
	JF("_stopListeningToMessage", "(JJLjava/lang/String;)V", stopListeningToMessage),
	JF("_getMessageListeners", "(JLjava/lang/String;)[J", getMessageListeners),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}

//--------------------------------------------------------------------------------------

void JNICALL ScriptMethodsBroadcastingNamespace::listenToMessage(JNIEnv *env, jobject self, jlong listener, jlong emitter, jstring messageHandlerName)
{
	UNREF(self);

	if (listener == 0 || emitter == 0 || messageHandlerName == 0)
		return;

	JavaStringParam localMessageHandlerName(messageHandlerName);

	CachedNetworkId listenerId(listener);
	CachedNetworkId emitterId(emitter);

	ServerObject* listenerObject = dynamic_cast<ServerObject*>(listenerId.getObject());
	if (listenerObject == nullptr)
		return;

	std::string messageHandlerNameString;
	JavaLibrary::convert(localMessageHandlerName, messageHandlerNameString);
	listenerObject->listenToBroadcastMessage(messageHandlerNameString, emitterId);

	return;
}	// JavaLibrary::listenToMessage

//--------------------------------------------------------------------------------------

void JNICALL ScriptMethodsBroadcastingNamespace::stopListeningToMessage(JNIEnv *env, jobject self, jlong listener, jlong emitter, jstring messageHandlerName)
{
	UNREF(self);

	if (listener == 0 || emitter == 0 || messageHandlerName == 0)
		return;

	JavaStringParam localMessageHandlerName(messageHandlerName);

	CachedNetworkId listenerId(listener);
	CachedNetworkId emitterId(emitter);

	ServerObject* listenerObject = dynamic_cast<ServerObject*>(listenerId.getObject());
	if (listenerObject == nullptr)
		return;

	std::string messageHandlerNameString;
	JavaLibrary::convert(localMessageHandlerName, messageHandlerNameString);
	listenerObject->stopListeningToBroadcastMessage(messageHandlerNameString, emitterId);

	return;
}	// JavaLibrary::stopListeningToMessage

//--------------------------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsBroadcastingNamespace::getMessageListeners(JNIEnv *env, jobject self, jlong emitter, jstring messageHandlerName)
{
	UNREF(self);

	if (emitter == 0 || messageHandlerName == 0)
		return 0;

	JavaStringParam localMessageHandlerName(messageHandlerName);

	CachedNetworkId emitterId(emitter);

	ServerObject* emitterObject = dynamic_cast<ServerObject*>(emitterId.getObject());
	if (emitterObject == nullptr)
		return 0;

	std::string messageHandlerNameString;
	JavaLibrary::convert(localMessageHandlerName, messageHandlerNameString);

	// the listener list is a set of <messageHandlerName, NetworkId>, and we
	// need to quickly locate the first entry in the set that contains the
	// messageHandlerName we are looking for
	static const NetworkId s_minNetworkId(std::numeric_limits<NetworkId::NetworkIdType>::min());
	ServerObject::BroadcastMap const & listeners = emitterObject->getBroadcastListeners();
	std::vector<NetworkId> listenerIds;

	for (ServerObject::BroadcastMap::const_iterator iter = listeners.lower_bound(std::make_pair(messageHandlerNameString, s_minNetworkId)); ((iter != listeners.end()) && (iter->first == messageHandlerNameString)); ++iter)
		listenerIds.push_back(iter->second);

	if (listenerIds.empty())
		return 0;

	size_t count = listenerIds.size();
	LocalLongArrayRefPtr listenerIdsArray = createNewLongArray(count);
	if (listenerIdsArray == LocalLongArrayRef::cms_nullPtr)
		return 0;

	jlong jlongTmp;
	for (size_t i = 0; i < count; ++i)
	{
		jlongTmp = listenerIds[i].getValue();
		setLongArrayRegion(*listenerIdsArray, i, 1, &jlongTmp);
	}

	return listenerIdsArray->getReturnValue();
}	// JavaLibrary::getMessageListeners
