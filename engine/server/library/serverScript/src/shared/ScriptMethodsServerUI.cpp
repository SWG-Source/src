//-----------------------------------------------------------------------
// ScriptMethodsServerUI.cpp
// Copyright 2002, Sony Online Entertainment Inc., all rights reserved.
//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/Client.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUIManager.h"
#include "serverGame/ServerUIPage.h"
#include "sharedGame/SuiPageData.h"

#include "sharedUtility/ValueDictionary.h"

// ======================================================================
// ScriptMethodsServerUINamespace
// ======================================================================

namespace ScriptMethodsServerUINamespace
{
	bool install();

	jint         JNICALL createSuiPage             (JNIEnv *env, jobject self, jstring pageName, jlong owner, jlong client);
	jboolean     JNICALL clearSuiDataSource        (JNIEnv *env, jobject self, jint pageId, jstring dataSource);
	jboolean     JNICALL clearSuiDataSourceContainer(JNIEnv *env, jobject self, jint pageId, jstring dataSource);
	jboolean     JNICALL addSuiDataItem            (JNIEnv *env, jobject self, jint pageId, jstring dataSource, jstring dataItemValue);
	jboolean     JNICALL addSuiDataSourceContainer (JNIEnv *env, jobject self, jint pageId, jstring dataSource, jstring dataItemValue);
	jboolean     JNICALL addSuiDataSource		   (JNIEnv *env, jobject self, jint pageId, jstring dataSource, jstring dataItemValue);
	jboolean     JNICALL addSuiChildWidget         (JNIEnv *env, jobject self, jint pageId, jstring widgetType, jstring widgetName, jstring partentWidgetName);
	jboolean     JNICALL setSuiProperty            (JNIEnv *env, jobject self, jint pageId, jstring widgetName, jstring propertyName, jstring propertyValue);
	jboolean     JNICALL subscribeToSuiEvent       (JNIEnv *env, jobject self, jint pageId, jint eventType, jstring widgetName, jstring callback);
	jboolean     JNICALL subscribeToSuiPropertyForEvent (JNIEnv *env, jobject self, jint pageId, jint eventType, jstring eventWidgetName, jstring propertyWidgetName, jstring propertyName);
	jboolean     JNICALL showSuiPage               (JNIEnv *env, jobject self, jint pageId);
	jboolean     JNICALL forceCloseSuiPage         (JNIEnv *env, jobject self, jint pageId);
	jboolean     JNICALL setSuiAssociatedObject    (JNIEnv *env, jobject self, jint pageId, jlong j_associatedObjectId);
	jboolean     JNICALL setSuiAssociatedLocation  (JNIEnv *env, jobject self, jint pageId, jlong j_associatedObjectId);
	jboolean     JNICALL setSuiMaxRangeToObject    (JNIEnv *env, jobject self, jint pageId, jfloat j_maxRange);
	jboolean     JNICALL clientMinigameOpen(JNIEnv *env, jobject self, jlong player, jobject data);
	jboolean     JNICALL clientMinigameClose(JNIEnv *env, jobject self, jlong player, jobject data);

}


//========================================================================
// install
//========================================================================

bool ScriptMethodsServerUINamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsServerUINamespace::c)}
	JF("_createSUIPage",             "(Ljava/lang/String;JJ)I",        createSuiPage),
	JF("clearSUIDataSource",        "(ILjava/lang/String;)Z",                                     clearSuiDataSource),
	JF("clearSUIDataSourceContainer",        "(ILjava/lang/String;)Z",                                     clearSuiDataSourceContainer),
	JF("addSUIDataItem",            "(ILjava/lang/String;Ljava/lang/String;)Z",                   addSuiDataItem),
	JF("addSUIDataSourceContainer", "(ILjava/lang/String;Ljava/lang/String;)Z",                   addSuiDataSourceContainer),
	JF("addSUIDataSource",			"(ILjava/lang/String;Ljava/lang/String;)Z",                   addSuiDataSource),
	JF("addSUIChildWidget",         "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z", addSuiChildWidget),
	JF("setSUIProperty",            "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z", setSuiProperty),
	JF("subscribeToSUIEvent",       "(IILjava/lang/String;Ljava/lang/String;)Z",                  subscribeToSuiEvent),
	JF("subscribeToSUIPropertyForEvent",   "(IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",   subscribeToSuiPropertyForEvent),
	JF("showSUIPage",               "(I)Z",                                                       showSuiPage),
	JF("forceCloseSUIPage",         "(I)Z",                                                       forceCloseSuiPage),
	JF("_setSUIAssociatedObject",   "(IJ)Z",                                                      setSuiAssociatedObject),
	JF("_setSUIAssociatedLocation", "(IJ)Z",                                                      setSuiAssociatedLocation),
	JF("setSUIMaxRangeToObject",    "(IF)Z",                                                      setSuiMaxRangeToObject),
	JF("_clientMinigameOpen",       "(JLscript/dictionary;)Z",                                    clientMinigameOpen),
	JF("_clientMinigameClose",      "(JLscript/dictionary;)Z",                                    clientMinigameClose),

};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsServerUINamespace::createSuiPage(JNIEnv *env, jobject self, jstring pageName, jlong ownerobject, jlong client)
{
	JavaStringParam localPageName(pageName);

	jint failureCode = -1;
	ServerObject* owner = nullptr;
	if(!JavaLibrary::getObject(ownerobject, owner))
	{
		WARNING(true, ("SUI: couldn't get owner ServerObject*, can't create a page"));
		return failureCode;
	}

	ServerObject* so = nullptr;
	if(!JavaLibrary::getObject(client, so))
		return failureCode;

	std::string pn;
	JavaLibrary::convert(localPageName, pn);

	//Still going to check for client even though we don't use it anymore -- ARH
	Client* const c = so->getClient();
	if(!c)
	{
		WARNING(true, ("SUI: couldn't get Client object [%s], can't create a page [%s]",
			so->getDebugInformation(false).c_str(), pn.c_str()));
		return failureCode;
	}

	//make a serverui page for the given client
	int const pId = ServerUIManager::createPage(pn, *owner, so->getNetworkId());
	//fail if can't create page
	if(pId == -1)
	{
		WARNING(true, ("SUI: ServerManager::createPage could not create a SUI page"));
		return failureCode;
	}
	return pId;
}

//-----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::clearSuiDataSource(JNIEnv *env, jobject self, jint pageId, jstring dataSource)
{
	JavaStringParam localDataSource(dataSource);

	ServerUIPage * const p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("SUI: Could not find page %d, cannot clear datasource", pageId));
		return JNI_FALSE;
	}

	std::string ds;
	JavaLibrary::convert(localDataSource, ds);

	p->clearDataSource(ds);
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsServerUINamespace::clearSuiDataSourceContainer(JNIEnv * env, jobject self, jint pageId, jstring dataSourceContainer)
{
	JavaStringParam localDataSource(dataSourceContainer);

	ServerUIPage * const p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("SUI: Could not find page %d, cannot clear datasource", pageId));
		return JNI_FALSE;
	}

	std::string ds;
	JavaLibrary::convert(localDataSource, ds);

	p->clearDataSourceContainer(ds);
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::addSuiDataItem(JNIEnv *env, jobject self, jint pageId, jstring dataSource, jstring dataItemValue)
{
	JavaStringParam localDataSource(dataSource);
	JavaStringParam localDataItemValue(dataItemValue);

	ServerUIPage* p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("SUI: Could not find page %d, cannot add the item", pageId));
		return JNI_FALSE;
	}

	std::string ds;
	JavaLibrary::convert(localDataSource, ds);

	Unicode::String div;
	JavaLibrary::convert(localDataItemValue, div);

	std::string     din = "Name";
	p->addDataItem(ds, din, div);
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::addSuiDataSourceContainer(JNIEnv *env, jobject self, jint pageId, jstring dataSource, jstring dataItemValue)
{
    JavaStringParam localDataSource(dataSource);
    JavaStringParam localDataItemValue(dataItemValue);

    ServerUIPage* p = ServerUIManager::getPage(pageId);
    if(!p)
    {
        DEBUG_WARNING(true, ("SUI: Could not find page %d, cannot add the item", pageId));
        return JNI_FALSE;
    }

    std::string ds;
    JavaLibrary::convert(localDataSource, ds);

    Unicode::String div;
    JavaLibrary::convert(localDataItemValue, div);

    std::string     din = "Name";
    p->addDataSourceContainer(ds, din, div);
    return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::addSuiDataSource(JNIEnv *env, jobject self, jint pageId, jstring dataSource, jstring dataItemValue)
{
	JavaStringParam localDataSource(dataSource);
	JavaStringParam localDataItemValue(dataItemValue);

	ServerUIPage* p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("SUI: Could not find page %d, cannot add the item", pageId));
		return JNI_FALSE;
	}

	std::string ds;
	JavaLibrary::convert(localDataSource, ds);

	Unicode::String div;
	JavaLibrary::convert(localDataItemValue, div);

	std::string     din = "Name";
	p->addDataSource(ds, din, div);
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::addSuiChildWidget(JNIEnv *env, jobject self, jint pageId, jstring widgetType, jstring widgetName, jstring parentWidgetName)
{
	JavaStringParam localWidgetType(widgetType);
	JavaStringParam localWidgetName(widgetName);
	JavaStringParam localParentWidgetName(parentWidgetName);

	ServerUIPage* p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("SUI: Could not find page %d, cannot add a child widget", pageId));
		return JNI_FALSE;
	}

	std::string wt;
	JavaLibrary::convert(localWidgetType, wt);
	std::string wn;
	JavaLibrary::convert(localWidgetName, wn);
	std::string pwn;
	JavaLibrary::convert(localParentWidgetName, pwn);

	p->addChildWidget(wt, wn, pwn);
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::setSuiProperty(JNIEnv *env, jobject self, jint pageId, jstring widgetName, jstring propertyName, jstring propertyValue)
{
	JavaStringParam localWidgetName(widgetName);
	JavaStringParam localPropertyName(propertyName);
	JavaStringParam localPropertyValue(propertyValue);

	ServerUIPage* p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("SUI: Could not find page %d, cannot set the property", pageId));
		return JNI_FALSE;
	}

	std::string     wn;
	JavaLibrary::convert(localWidgetName, wn);
	std::string     pn;
	JavaLibrary::convert(localPropertyName, pn);
	Unicode::String pv;
	JavaLibrary::convert(localPropertyValue, pv);

	p->setProperty(wn, pn, pv);
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::subscribeToSuiEvent(JNIEnv *env, jobject self, jint pageId, jint eventType, jstring eventWidgetName, jstring callback)
{
	ServerUIPage* p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("SUI: Could not find page %d, cannot subscribe to the property", pageId));
		return JNI_FALSE;
	}

	std::string     ewn;
	if (eventWidgetName != nullptr)
	{
		JavaStringParam localEventWidgetName(eventWidgetName);
		JavaLibrary::convert(localEventWidgetName, ewn);
	}

	JavaStringParam localCallback(callback);

	std::string     cb;
	JavaLibrary::convert(localCallback, cb);

	p->getPageData().subscribeToEvent(eventType, ewn, cb);
	return JNI_TRUE;
}
//-----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::subscribeToSuiPropertyForEvent(JNIEnv *env, jobject self, jint pageId, jint eventType, jstring eventWidgetName,  jstring propertyWidgetName, jstring propertyName)
{

	ServerUIPage* p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("SUI: Could not find page %d, cannot subscribe to the property", pageId));
		return JNI_FALSE;
	}

	std::string     ewn;
	if (eventWidgetName != nullptr)
	{
		JavaStringParam localEventWidgetName(eventWidgetName);
		JavaLibrary::convert(localEventWidgetName, ewn);
	}

	JavaStringParam localPropertyWidgetName(propertyWidgetName);
	JavaStringParam localPropertyName(propertyName);

	std::string     pwn;
	JavaLibrary::convert(localPropertyWidgetName, pwn);

	std::string     pn;
	JavaLibrary::convert(localPropertyName, pn);

	p->getPageData().subscribeToPropertyForEvent(eventType, ewn, pwn, pn);
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsServerUINamespace::showSuiPage(JNIEnv *env, jobject self, jint pageId)
{
	if (ServerUIManager::showPage(pageId))
		return JNI_TRUE;

	return JNI_FALSE;
}

//-----------------------------------------------------------------------

jboolean   JNICALL ScriptMethodsServerUINamespace::forceCloseSuiPage(JNIEnv *env, jobject self, jint pageId)
{
	bool result = ServerUIManager::closePage(pageId);
	if(result)
		return JNI_TRUE;
	else
	{
		DEBUG_WARNING(true, ("SUI: Couldn't force close the page", pageId));
		return JNI_FALSE;
	}
}

//----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::setSuiAssociatedObject(JNIEnv *env, jobject self, jint pageId, jlong j_associatedObjectId)
{
	ServerUIPage * const p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("Could not find page %d, cannot set property", pageId));
		return JNI_FALSE;
	}

	if (!j_associatedObjectId)
	{
		DEBUG_WARNING(true, ("bad associatedObjectId sent to setAssociatedObject", pageId));
		return JNI_FALSE;
	}

	NetworkId id(j_associatedObjectId);
	if (id == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("could not convert associatedObjectid", pageId));
		return JNI_FALSE;
	}

	p->setAssociatedObjectId (id);
	return JNI_TRUE;
}

//----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::setSuiAssociatedLocation(JNIEnv *env, jobject self, jint pageId, jlong j_associatedObjectId)
{
	ServerUIPage * const p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("Could not find page %d, cannot set property", pageId));
		return JNI_FALSE;
	}

	if (!j_associatedObjectId)
	{
		DEBUG_WARNING(true, ("bad associatedObjectId sent to setAssociatedObject"));
		return JNI_FALSE;
	}

	ServerObject* associatedObject = nullptr;
	if(!JavaLibrary::getObject(j_associatedObjectId, associatedObject))
	{
		DEBUG_WARNING(true, ("could not find object for associatedObjectid"));
		return JNI_FALSE;
	}

	p->setAssociatedLocation (associatedObject->findPosition_w());
	return JNI_TRUE;
}

//----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsServerUINamespace::setSuiMaxRangeToObject(JNIEnv *env, jobject self, jint pageId, jfloat j_maxRange)
{
	ServerUIPage * const p = ServerUIManager::getPage(pageId);
	if(!p)
	{
		DEBUG_WARNING(true, ("Could not find page %d, cannot set property", pageId));
		return JNI_FALSE;
	}

	p->setMaxRangeFromObject (j_maxRange);
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsServerUINamespace::clientMinigameOpen(JNIEnv *env, jobject self, jlong player, jobject data)
{
	ServerObject* playerServerObject = nullptr;
	if(!JavaLibrary::getObject(player, playerServerObject))
		return JNI_FALSE;

	CreatureObject * creatureObject = playerServerObject->asCreatureObject();
	if(creatureObject != nullptr)
	{
		ValueDictionary gameData;
		JavaLibrary::convert(data, gameData);

		creatureObject->clientMinigameOpen(gameData);
		return JNI_TRUE;
	}

	return JNI_FALSE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsServerUINamespace::clientMinigameClose(JNIEnv *env, jobject self, jlong player, jobject data)
{
	ServerObject* playerServerObject = nullptr;
	if(!JavaLibrary::getObject(player, playerServerObject))
		return JNI_FALSE;

	CreatureObject * creatureObject = playerServerObject->asCreatureObject();
	if(creatureObject != nullptr)
	{
		ValueDictionary gameData;
		JavaLibrary::convert(data, gameData);

		creatureObject->clientMinigameClose(gameData);
		return JNI_TRUE;
	}

	return JNI_FALSE;
}
