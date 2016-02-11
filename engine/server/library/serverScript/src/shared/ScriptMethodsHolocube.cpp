//-----------------------------------------------------------------------
// ScriptMethodsHolocube.cpp
// Copyright 2002, Sony Online Entertainment Inc., all rights reserved.
//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/GameServer.h"
#include "serverGame/ServerObject.h"
#include "sharedNetworkMessages/OpenHolocronToPageMessage.h"
#include "sharedNetworkMessages/CloseHolocronMessage.h"


// ======================================================================
// ScriptMethodsHolocubeNamespace
// ======================================================================

namespace ScriptMethodsHolocubeNamespace
{
	bool install();

	jboolean     JNICALL openHolocronToPage(JNIEnv *env, jobject self, jlong client, jstring jpage);
	jboolean     JNICALL closeHolocron(JNIEnv *env, jobject self, jlong client);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsHolocubeNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsHolocubeNamespace::c)}
	JF("_openHolocronToPage", "(JLjava/lang/String;)Z", openHolocronToPage),
	JF("_closeHolocron", "(J)Z", closeHolocron),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsHolocubeNamespace::openHolocronToPage(JNIEnv *env, jobject self, jlong client, jstring jpage)
{
	JavaStringParam localPage(jpage);

	ServerObject* so = nullptr;
	if(!JavaLibrary::getObject(client, so))
	{
		WARNING (true, ("[Script bug] Could not get ServerObject for value passed in for paramter:client in JavaLibrary::openHolocronToPage"));
		return JNI_FALSE;
	}
	Client* c = so->getClient();
	if(!c)
	{
		WARNING (true, ("[Script bug] Value given for paramter:client is not a player in JavaLibrary::openHolocronToPage, associated ServerObject NetworkId is: %s", so->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}
	std::string page;
	JavaLibrary::convert(localPage, page);

	//an empty page is fine, that tells the client just to open the holocube

	OpenHolocronToPageMessage ohtpm(page);

	if(c)
		c->send(ohtpm, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsHolocubeNamespace::closeHolocron(JNIEnv *env, jobject self, jlong client)
{
	ServerObject* so = nullptr;
	if(!JavaLibrary::getObject(client, so))
	{
		WARNING (true, ("[Script bug] Could not get ServerObject for value passed in for paramter:client in JavaLibrary::closeHolocron"));
		return JNI_FALSE;
	}
	Client* c = so->getClient();
	if(!c)
	{
		WARNING (true, ("[Script bug] Value given for paramter:client is not a player in JavaLibrary::closeHolocron, associated ServerObject NetworkId is: %s", so->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	CloseHolocronMessage chm;

	if(c)
		c->send(chm, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------
