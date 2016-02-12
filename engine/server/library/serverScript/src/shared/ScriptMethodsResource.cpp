//========================================================================
//
// ScriptMethodsResource.cpp - implements script methods dealing with resource
// movement.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "UnicodeUtils.h"
#include "serverGame/Client.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourcePoolObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/SurveySystem.h"
#include "sharedGame/ResourceClassObject.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ResourceHarvesterActivatePageMessage.h"
#include "sharedObject/NetworkIdManager.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsResourceNamespace
// ======================================================================

namespace ScriptMethodsResourceNamespace
{
	bool install();

	void         JNICALL activateHarvesterExtractionPage (JNIEnv *env, jobject self, jlong player, jlong harvester);
	jboolean     JNICALL requestResourceListForSurvey(JNIEnv *env, jobject self, jlong player, jlong tool, jstring parentResourceClass);
	jboolean     JNICALL requestSurvey(JNIEnv *env, jobject self, jlong player, jstring parentResourceClass, jstring resourceTypeName, jint surveyRange, jint numPoints);
	jlong        JNICALL pickRandomNonDepeletedResource(JNIEnv *env, jobject self, jstring parentResourceClass);
	jint         JNICALL oneTimeHarvest(JNIEnv *env, jobject self, jlong resourceType, jint amount, jobject where);
	jlong        JNICALL getResourceContainerResourceType(JNIEnv *env, jobject self, jlong resourceContainer);
	jint         JNICALL getResourceContainerQuantity(JNIEnv *env, jobject self, jlong resourceContainer);
	jstring      JNICALL getResourceCtsData(JNIEnv *env, jobject self, jlong resourceContainer);
	void         JNICALL setResourceCtsData(JNIEnv *env, jobject self, jlong resourceContainer, jint quantity, jstring resourceData);
	jboolean     JNICALL addResourceToContainer(JNIEnv *env, jobject self, jlong resourceContainer, jlong resourceType, jint amount, jlong sourcePlayer);
	jboolean     JNICALL removeResourceFromContainer(JNIEnv *env, jobject self, jlong resourceContainer, jlong resourceType, jint amount);
	jboolean     JNICALL transferToResourceContainer(JNIEnv *env, jobject self, jlong sourceResourceContainer, jlong destinationResourceContainer, jint amount);
	jfloat       JNICALL getResourceEfficiency(JNIEnv *env, jobject self, jlong resourceType, jobject where);
	jboolean     JNICALL isResourceDerivedFrom(JNIEnv *env, jobject self, jlong resourceType, jstring parentResourceClass);
	jboolean     JNICALL isResourceClassDerivedFrom(JNIEnv *env, jobject self, jstring resourceClass, jstring parentResourceClass);
	jlong        JNICALL getResourceTypeByName(JNIEnv *env, jobject self, jstring typeName);
	jstring      JNICALL getResourceContainerForType(JNIEnv *env, jobject self, jlong resourceType);
	jstring      JNICALL getResourceName(JNIEnv *env, jobject self, jlong resourceType);
	jobjectArray JNICALL getResourceNames(JNIEnv *env, jobject self, jlongArray resourceTypes);
	jstring      JNICALL getResourceClassName(JNIEnv *env, jobject self, jstring resourceClass);
	jobjectArray JNICALL getResourceClassNames(JNIEnv *env, jobject self, jobjectArray resourceClasses);
	jlongArray   JNICALL getResourceTypes(JNIEnv *env, jobject self, jstring resourceClass);
	jstring      JNICALL getResourceClass(JNIEnv *env, jobject self, jlong resourceType);
	jstring      JNICALL getResourceParentClass(JNIEnv *env, jobject self, jstring resourceClass);
	jobjectArray JNICALL getResourceChildClasses(JNIEnv *env, jobject self, jstring resourceClass);
	jobjectArray JNICALL getImmediateResourceChildClasses(JNIEnv *env, jobject self, jstring resourceClass);
	jobjectArray JNICALL getLeafResourceChildClasses(JNIEnv *env, jobject self, jstring resourceClass);
	jboolean     JNICALL hasResourceType(JNIEnv *env, jobject self, jstring resourceClass);
	jlong        JNICALL createResourceCrate(JNIEnv *env, jobject self, jlong resourceType, jint amount, jlong destination);
	jobjectArray JNICALL requestResourceList(JNIEnv *env, jobject self, jobject loc, jfloat minDensity, jfloat maxDensity, jstring resourceClass);
	jobjectArray JNICALL getResourceAttributes(JNIEnv *env, jobject self, jlong resourceType);
	jobjectArray JNICALL getScaledResourceAttributes(JNIEnv *env, jobject self, jlong resourceType, jstring resourceClass);
	jint         JNICALL getResourceAttribute(JNIEnv *env, jobject self, jlong resourceType, jstring attributeName);
	jlong        JNICALL getRecycledVersionOfResourceType(JNIEnv * env, jobject self, jlong resourceType);
	jboolean     JNICALL isValidResourceId(JNIEnv *env, jobject self, jlong resourceType);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsResourceNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsResourceNamespace::c)}
	JF("_activateHarvesterExtractionPage", "(JJ)V", activateHarvesterExtractionPage),
	JF("_requestResourceListForSurvey","(JJLjava/lang/String;)Z",requestResourceListForSurvey),
	JF("_requestSurvey","(JLjava/lang/String;Ljava/lang/String;II)Z",requestSurvey),
	JF("_pickRandomNonDepeletedResource","(Ljava/lang/String;)J",pickRandomNonDepeletedResource),
	JF("_oneTimeHarvest","(JILscript/location;)I",oneTimeHarvest),
	JF("_addResourceToContainer","(JJIJ)Z",addResourceToContainer),
	JF("_removeResourceFromContainer","(JJI)Z",removeResourceFromContainer),
	JF("_transferToResourceContainer","(JJI)Z",transferToResourceContainer),
	JF("_getResourceEfficiency","(JLscript/location;)F",getResourceEfficiency),
	JF("_isResourceDerivedFrom","(JLjava/lang/String;)Z",isResourceDerivedFrom),
	JF("isResourceClassDerivedFrom","(Ljava/lang/String;Ljava/lang/String;)Z",isResourceClassDerivedFrom),
	JF("_getResourceTypeByName","(Ljava/lang/String;)J",getResourceTypeByName),
	JF("_getResourceContainerForType","(J)Ljava/lang/String;",getResourceContainerForType),
	JF("_getResourceName", "(J)Ljava/lang/String;", getResourceName),
	JF("_getResourceNames", "([J)[Ljava/lang/String;", getResourceNames),
	JF("getResourceClassName", "(Ljava/lang/String;)Ljava/lang/String;", getResourceClassName),
	JF("getResourceClassNames", "([Ljava/lang/String;)[Ljava/lang/String;", getResourceClassNames),
	JF("_getResourceTypes", "(Ljava/lang/String;)[J", getResourceTypes),
	JF("_getResourceClass", "(J)Ljava/lang/String;", getResourceClass),
	JF("getResourceParentClass", "(Ljava/lang/String;)Ljava/lang/String;", getResourceParentClass),
	JF("getResourceChildClasses", "(Ljava/lang/String;)[Ljava/lang/String;", getResourceChildClasses),
	JF("getImmediateResourceChildClasses", "(Ljava/lang/String;)[Ljava/lang/String;", getImmediateResourceChildClasses),
	JF("getLeafResourceChildClasses", "(Ljava/lang/String;)[Ljava/lang/String;", getLeafResourceChildClasses),
	JF("hasResourceType", "(Ljava/lang/String;)Z", hasResourceType),
	JF("_createResourceCrate", "(JIJ)J", createResourceCrate),
	JF("requestResourceList", "(Lscript/location;FFLjava/lang/String;)[Lscript/resource_density;", requestResourceList),
	JF("_getResourceAttributes", "(J)[Lscript/resource_attribute;", getResourceAttributes),
	JF("_getScaledResourceAttributes", "(JLjava/lang/String;)[Lscript/resource_attribute;", getScaledResourceAttributes),
	JF("_getResourceAttribute", "(JLjava/lang/String;)I", getResourceAttribute),
	JF("_getRecycledVersionOfResourceType", "(J)J", getRecycledVersionOfResourceType),
	JF("_isValidResourceId", "(J)Z", isValidResourceId),
	JF("_getResourceContainerResourceType", "(J)J", getResourceContainerResourceType),
	JF("_getResourceContainerQuantity", "(J)I", getResourceContainerQuantity),
	JF("_getResourceCtsData", "(J)Ljava/lang/String;", getResourceCtsData),
	JF("_setResourceCtsData", "(JILjava/lang/String;)V", setResourceCtsData),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

void JNICALL ScriptMethodsResourceNamespace::activateHarvesterExtractionPage (JNIEnv *env, jobject self, jlong player, jlong harvester)
{
	UNREF(self);
	UNREF (env);

	const NetworkId harvesterId (harvester);

	ServerObject *  playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
		return;

	Client * const client = playerObject->getClient ();
	if (!client)
		return;

	ResourceHarvesterActivatePageMessage const msg(harvesterId);
	client->send(msg, true);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsResourceNamespace::requestResourceListForSurvey(JNIEnv * /*env*/, jobject /*self*/, jlong player, jlong tool, jstring parentResourceClass)
{
	const NetworkId myPlayer(player);
	Unicode::String myParentResourceClass;
	const NetworkId surveyTool(tool);

	JavaString temp(parentResourceClass);
	if (!JavaLibrary::convert(temp,myParentResourceClass))
		return JNI_FALSE;

	SurveySystem::getInstance().requestResourceListForSurvey(myPlayer,surveyTool,Unicode::wideToNarrow(myParentResourceClass));
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsResourceNamespace::requestSurvey(JNIEnv * /*env*/, jobject /*self*/, jlong player, jstring parentResourceClass, jstring resourceTypeName, jint surveyRange, jint numPoints)
{
	const NetworkId myPlayer(player);
	const Object*   myPlayerObject = ServerWorld::findObjectByNetworkId(myPlayer);
	Unicode::String myParentResourceClass;
	std::string     myResourceTypeName;
	const int       mySurveyRange = static_cast<int>(surveyRange);
	const int       myNumPoints   = static_cast<int>(numPoints);

	JavaString temp(parentResourceClass);
	if (!JavaLibrary::convert(temp,myParentResourceClass))
		return JNI_FALSE;
	JavaString temp2(resourceTypeName);
	if (!JavaLibrary::convert(temp2,myResourceTypeName))
		return JNI_FALSE;
	if (!myPlayerObject)
		return JNI_FALSE;

	SurveySystem::getInstance().requestSurvey(myPlayer, Unicode::wideToNarrow(myParentResourceClass), myResourceTypeName, myPlayerObject->getPosition_w(), mySurveyRange, myNumPoints);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsResourceNamespace::pickRandomNonDepeletedResource(JNIEnv * /*env*/, jobject /*self*/, jstring parentResourceClass)
{
	std::string myParentResourceClass;
	JavaString temp(parentResourceClass);
	if (!JavaLibrary::convert(temp,myParentResourceClass))
		return 0;

	ResourceTypeObject const * const typeObj = ServerUniverse::getInstance().pickRandomNonDepletedResource(myParentResourceClass);
	if (typeObj)
	{
		return typeObj->getNetworkId().getValue();
	}
	else
		return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsResourceNamespace::oneTimeHarvest(JNIEnv * /*env*/, jobject /*self*/, jlong resourceType, jint amount, jobject where)
{
	ResourceTypeObject const * const typeObj = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!typeObj)
		return 0;

	Vector pos;
	NetworkId cell;
	if (!ScriptConversion::convert(where, pos, cell))
		return 0;

	if (cell != NetworkId::cms_invalid)
	{
		DEBUG_REPORT_LOG(true,("Can't call oneTimeHarvest() with a location in a cell.\n"));
		return 0;
	}

	ResourcePoolObject const * const poolObj=typeObj->getPoolForCurrentPlanet();
	if (poolObj)
		return poolObj->oneTimeHarvest(amount, pos);
	else
		return 0;
}

// ----------------------------------------------------------------------

/**
 * Get the ResourceType contained in a container
 *
 * @param env                 Java environment
 * @param self                class calling this function
 * @param resourceContainer   the resource container
 * @return A reference to the ResourceTypeObject
 * @todo Maybe make a ScriptMethodsResource file, when there are a few more resource-related script methods.
 */
jlong JNICALL ScriptMethodsResourceNamespace::getResourceContainerResourceType(JNIEnv * /*env*/, jobject /*self*/, jlong resourceContainer)
{
	ResourceContainerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(resourceContainer, sourceObj))
		return JNI_FALSE;

	NetworkId const & result = sourceObj->getResourceTypeId();

	return result.getValue();
}	// JavaLibrary::getResourceContainerResourceType

// ----------------------------------------------------------------------

/**
 * Get the quantity of resources in a container.
 *
 * @param env                 Java environment
 * @param self                class calling this function
 * @param resourceContainer   the resource container
 * @return the amount
 */
jint JNICALL ScriptMethodsResourceNamespace::getResourceContainerQuantity(JNIEnv * /*env*/, jobject /*self*/, jlong resourceContainer)
{
	ResourceContainerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(resourceContainer, sourceObj))
		return JNI_FALSE;

	return (sourceObj->getQuantity());
}	// JavaLibrary::getResourceContainerQuantity

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsResourceNamespace::getResourceCtsData(JNIEnv *env, jobject self, jlong resourceContainer)
{
	ResourceContainerObject const * sourceObj = 0;
	if (!JavaLibrary::getObject(resourceContainer, sourceObj))
		return 0;

	std::string resourceData = sourceObj->getResourceCtsData();
	if (resourceData.empty())
		return 0;

	return JavaString(resourceData).getReturnValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsResourceNamespace::setResourceCtsData(JNIEnv *env, jobject self, jlong resourceContainer, jint quantity, jstring resourceData)
{
	ResourceContainerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(resourceContainer, sourceObj))
		return;

	if (resourceData == 0)
		return;

	JavaStringParam jspResourceData(resourceData);
	std::string sResourceData;
	if (!JavaLibrary::convert(jspResourceData, sResourceData))
		return;

	if (sResourceData.empty())
		return;

	sourceObj->setResourceCtsData(quantity, sResourceData);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsResourceNamespace::addResourceToContainer(JNIEnv * /*env*/, jobject /*self*/, jlong resourceContainer, jlong resourceType, jint amount, jlong sourcePlayer)
{
	ResourceContainerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(resourceContainer, sourceObj))
		return JNI_FALSE;

	NetworkId resourceTypeId(resourceType);
	NetworkId sourcePlayerId(sourcePlayer);

	if (sourceObj->addResource(resourceTypeId, amount, sourcePlayerId))
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsResourceNamespace::removeResourceFromContainer(JNIEnv * /*env*/, jobject /*self*/, jlong resourceContainer, jlong resourceType, jint amount)
{
	ResourceContainerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(resourceContainer, sourceObj))
		return JNI_FALSE;

	NetworkId resourceTypeId(resourceType);

	if (sourceObj->removeResourceWithoutExperience(resourceTypeId, amount))
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsResourceNamespace::transferToResourceContainer(JNIEnv * /*env*/, jobject /*self*/, jlong sourceResourceContainer, jlong destinationResourceContainer, jint amount)
{
	ResourceContainerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(sourceResourceContainer, sourceObj))
		return JNI_FALSE;

	ResourceContainerObject * destObj = 0;
	if (!JavaLibrary::getObject(destinationResourceContainer, destObj))
		return JNI_FALSE;

	if (sourceObj->transferTo(*destObj, amount))
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsResourceNamespace::getResourceEfficiency(JNIEnv * /*env*/, jobject /*self*/, jlong resourceType, jobject where)
{
	ResourceTypeObject const * const typeObj = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!typeObj)
		return 0;

	Vector pos;
	NetworkId cell;
	if (!ScriptConversion::convert(where, pos, cell))
		return 0;

	if (cell != NetworkId::cms_invalid)
	{
		DEBUG_REPORT_LOG(true,("Can't call getResourceEfficiency() with a location in a cell.\n"));
		return 0;
	}

	if (!typeObj->isDepleted())
	{
		ResourcePoolObject const * const poolObj=typeObj->getPoolForCurrentPlanet();
		if (poolObj)
			return poolObj->getEfficiencyAtLocation(pos.x,pos.z);
		else
			return 0;
	}
	else
		return 0;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsResourceNamespace::isResourceDerivedFrom(JNIEnv * /*env*/, jobject /*self*/, jlong resourceType, jstring parentResourceClass)
{
	std::string myParentResourceClass;
	JavaStringParam temp(parentResourceClass);
	if (!JavaLibrary::convert(temp,myParentResourceClass))
		return false;

	ResourceTypeObject const * const typeObj = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!typeObj)
		return 0;

	const ResourceClassObject *classObj=Universe::getInstance().getResourceClassByName(myParentResourceClass);
	if (!classObj)
		return false;

	return typeObj->isDerivedFrom(*classObj);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsResourceNamespace::isResourceClassDerivedFrom(JNIEnv * /*env*/, jobject /*self*/, jstring resourceClass, jstring parentResourceClass)
{
	std::string myResourceClass;
	JavaStringParam temp(resourceClass);
	if (!JavaLibrary::convert(temp,myResourceClass))
		return false;

	std::string myParentResourceClass;
	JavaStringParam temp2(parentResourceClass);
	if (!JavaLibrary::convert(temp2,myParentResourceClass))
		return false;

	const ResourceClassObject *classObj=ServerUniverse::getInstance().getResourceClassByName(myResourceClass);
	if (!classObj)
		return false;

	const ResourceClassObject *parentClassObj=Universe::getInstance().getResourceClassByName(myParentResourceClass);
	if (!parentClassObj)
		return false;

	return classObj->isDerivedFrom(*parentClassObj);
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsResourceNamespace::getResourceTypeByName(JNIEnv * /*env*/, jobject /*self*/, jstring typeName)
{
	std::string myTypeName;
	JavaStringParam temp(typeName);
	if (!JavaLibrary::convert(temp,myTypeName))
		return 0;

	ResourceTypeObject const * const typeObj = ServerUniverse::getInstance().getResourceTypeByName(myTypeName);
	if (typeObj)
		return (typeObj->getNetworkId()).getValue();
	else
	{
		WARNING(true,("getResourceTypeByName called for resource %s, which could not be found.",myTypeName.c_str()));
		return 0;
	}
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsResourceNamespace::getResourceContainerForType(JNIEnv * /*env*/, jobject /*self*/, jlong resourceType)
{
	ResourceTypeObject const * const typeObj = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!typeObj)
		return nullptr;

	std::string templateName;
	typeObj->getCrateTemplate(templateName);

	return JavaString(Unicode::narrowToWide(templateName)).getReturnValue();
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsResourceNamespace::getResourceName(JNIEnv * /*env*/, jobject /*self*/, jlong resourceType)
{
	if (resourceType == 0)
	{
		WARNING(true, ("JavaLibrary::getResourceName passed nullptr resource type"));
		return 0;
	}

	ResourceTypeObject const * const rt = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!rt)
	{
		WARNING(true, ("JavaLibrary::getResourceName passed unknown resource type"));
		return 0;
	}

	JavaString resourceTypeName(rt->getResourceName());
	return resourceTypeName.getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsResourceNamespace::getResourceNames(JNIEnv *env, jobject self, jlongArray resourceTypes)
{
	if (resourceTypes == 0)
	{
		WARNING(true, ("JavaLibrary::getResourceNames passed nullptr resource types"));
		return 0;
	}

	int count = env->GetArrayLength(resourceTypes);
	LocalObjectArrayRefPtr names = createNewObjectArray(count, JavaLibrary::getClsString());
	jlong jlongTmp;
	for (int i = 0; i < count; ++i)
	{
		env->GetLongArrayRegion(resourceTypes, i, 1, &jlongTmp);
		LocalRef resName(getResourceName(env, self, jlongTmp));
		setObjectArrayElement(*names, i, resName);
	}

	return names->getReturnValue();
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsResourceNamespace::getResourceClassName(JNIEnv *env, jobject self, jstring resourceClass)
{
	if (resourceClass == 0)
	{
		WARNING(true, ("JavaLibrary::getResourceClassName passed nullptr resource class"));
		return 0;
	}

	JavaStringParam jresourceClass(resourceClass);
	std::string resourceClassName;
	if (!JavaLibrary::convert(jresourceClass, resourceClassName))
	{
		WARNING(true, ("JavaLibrary::getResourceClassName cannot convert resource class name"));
		return 0;
	}

	const ServerResourceClassObject * resClass = safe_cast<const ServerResourceClassObject *>(
		ServerUniverse::getInstance().getResourceClassByName(resourceClassName));
	if (resClass == nullptr)
	{
		WARNING(true, ("JavaLibrary::getResourceClassName cannot find resource class for %s", resourceClassName.c_str()));
		return 0;
	}

	JavaString friendlyName("@" + resClass->getFriendlyName().getCanonicalRepresentation());
	return friendlyName.getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsResourceNamespace::getResourceClassNames(JNIEnv *env, jobject self, jobjectArray resourceClasses)
{
	if (resourceClasses == 0)
	{
		WARNING(true, ("JavaLibrary::getResourceClassNames passed nullptr resource classes"));
		return 0;
	}

	int count = env->GetArrayLength(resourceClasses);
	LocalObjectArrayRefPtr names = createNewObjectArray(count, JavaLibrary::getClsString());
	for (int i = 0; i < count; ++i)
	{
		LocalRef resClass(env->GetObjectArrayElement(resourceClasses, i));
		LocalRef resName(getResourceClassName(env, self, static_cast<jstring>(resClass.getValue())));
		setObjectArrayElement(*names, i, resName);
	}

	return names->getReturnValue();
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsResourceNamespace::getResourceTypes(JNIEnv * env, jobject /*self*/, jstring resourceClass)
{
	if (resourceClass == 0)
	{
		WARNING(true, ("JavaLibrary::getResourceTypes passed nullptr resource class"));
		return 0;
	}

	JavaStringParam jresourceClass(resourceClass);
	std::string resourceClassName;
	if (!JavaLibrary::convert(jresourceClass, resourceClassName))
	{
		WARNING(true, ("JavaLibrary::getResourceTypes cannot convert resource class name"));
		return 0;
	}

	const ServerResourceClassObject * resClass = safe_cast<const ServerResourceClassObject *>(
		ServerUniverse::getInstance().getResourceClassByName(resourceClassName));
	if (resClass == nullptr)
	{
		WARNING(true, ("JavaLibrary::getResourceTypes cannot find resource class for %s", resourceClassName.c_str()));
		return 0;
	}

	std::vector<const ResourceTypeObject *> types;
	resClass->getResourceTypes(types);

	size_t count = types.size();

	LocalLongArrayRefPtr jtypes = createNewLongArray(count);
	if (jtypes == LocalLongArrayRef::cms_nullPtr)
	{
		WARNING(true, ("JavaLibrary::getResourceTypes cannot create obj_id array of size %d", count));
		return 0;
	}

	jlong jlongTmp;
	for (size_t i = 0; i < count; ++i)
	{
		if (types[i] != nullptr)
		{
			jlongTmp = (types[i]->getNetworkId()).getValue();
			setLongArrayRegion(*jtypes, i, 1, &jlongTmp);
		}
	}
	return jtypes->getReturnValue();
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsResourceNamespace::getResourceClass(JNIEnv * /*env*/, jobject /*self*/, jlong resourceType)
{
	if (resourceType == 0)
	{
		WARNING(true, ("JavaLibrary::getResourceClass passed nullptr resource type"));
		return 0;
	}

	ResourceTypeObject const * const rt = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!rt)
	{
		WARNING(true, ("JavaLibrary::getResourceClass passed unknown resource type"));
		return 0;
	}

	ResourceClassObject  const & resourceClass = rt->getParentClass();

	JavaString resourceClassName(resourceClass.getResourceClassName());
	return resourceClassName.getReturnValue();
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsResourceNamespace::getResourceParentClass(JNIEnv * /*env*/, jobject /*self*/, jstring resourceClass)
{
	if (resourceClass == 0)
	{
		WARNING(true, ("JavaLibrary::getResourceParentClass passed nullptr resource class"));
		return 0;
	}

	JavaStringParam jresourceClass(resourceClass);
	std::string resourceClassName;
	if (!JavaLibrary::convert(jresourceClass, resourceClassName))
	{
		WARNING(true, ("JavaLibrary::getResourceParentClass cannot convert resource class name"));
		return 0;
	}

	const ResourceClassObject * resClass = ServerUniverse::getInstance().getResourceClassByName(resourceClassName);
	if (resClass == nullptr)
	{
		WARNING(true, ("JavaLibrary::getResourceParentClass cannot find resource class for %s", resourceClassName.c_str()));
		return 0;
	}

	const ResourceClassObject * parentClass = resClass->getParent();
	if (parentClass == nullptr)
		return 0;

	JavaString parentClassName(parentClass->getResourceClassName());
	return parentClassName.getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsResourceNamespace::getResourceChildClasses(JNIEnv * env, jobject /*self*/, jstring resourceClass)
{
	if (resourceClass == 0)
	{
		WARNING(true, ("JavaLibrary::getResourceChildClasses passed nullptr resource class"));
		return 0;
	}

	JavaStringParam jresourceClass(resourceClass);
	std::string resourceClassName;
	if (!JavaLibrary::convert(jresourceClass, resourceClassName))
	{
		WARNING(true, ("JavaLibrary::getResourceChildClasses cannot convert resource class name"));
		return 0;
	}

	const ResourceClassObject * resClass = ServerUniverse::getInstance().getResourceClassByName(resourceClassName);
	if (resClass == nullptr)
	{
		WARNING(true, ("JavaLibrary::getResourceChildClasses cannot find resource class for %s", resourceClassName.c_str()));
		return 0;
	}

	std::vector<const ResourceClassObject *> children;
	resClass->getChildren(children, true);

	size_t count = children.size();
	LocalObjectArrayRefPtr childrenArray = createNewObjectArray(static_cast<int>(count), JavaLibrary::getClsString());
	for (size_t i = 0; i < count; ++i)
	{
		if (children[i] != nullptr)
		{
			JavaString name(children[i]->getResourceClassName());
			setObjectArrayElement(*childrenArray, static_cast<int>(i), name);
		}
	}
	return childrenArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsResourceNamespace::getImmediateResourceChildClasses(JNIEnv *env, jobject self, jstring resourceClass)
{
	if (resourceClass == 0)
	{
		WARNING(true, ("JavaLibrary::getImmediateResourceChildClasses passed nullptr resource class"));
		return 0;
	}

	JavaStringParam jresourceClass(resourceClass);
	std::string resourceClassName;
	if (!JavaLibrary::convert(jresourceClass, resourceClassName))
	{
		WARNING(true, ("JavaLibrary::getImmediateResourceChildClasses cannot convert resource class name"));
		return 0;
	}

	const ResourceClassObject * resClass = ServerUniverse::getInstance().getResourceClassByName(resourceClassName);
	if (resClass == nullptr)
	{
		WARNING(true, ("JavaLibrary::getImmediateResourceChildClasses cannot find resource class for %s", resourceClassName.c_str()));
		return 0;
	}

	std::vector<const ResourceClassObject *> children;
	resClass->getChildren(children, false);

	int count = children.size();
	LocalObjectArrayRefPtr childrenArray = createNewObjectArray(count, JavaLibrary::getClsString());
	for (int i = 0; i < count; ++i)
	{
		if (children[i] != nullptr)
		{
			JavaString name(children[i]->getResourceClassName());
			setObjectArrayElement(*childrenArray, i, name);
		}
	}
	return childrenArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsResourceNamespace::getLeafResourceChildClasses(JNIEnv *env, jobject self, jstring resourceClass)
{
	if (resourceClass == 0)
	{
		WARNING(true, ("JavaLibrary::getLeafResourceChildClasses passed nullptr resource class"));
		return 0;
	}

	JavaStringParam jresourceClass(resourceClass);
	std::string resourceClassName;
	if (!JavaLibrary::convert(jresourceClass, resourceClassName))
	{
		WARNING(true, ("JavaLibrary::getLeafResourceChildClasses cannot convert resource class name"));
		return 0;
	}

	const ResourceClassObject * resClass = ServerUniverse::getInstance().getResourceClassByName(resourceClassName);
	if (resClass == nullptr)
	{
		WARNING(true, ("JavaLibrary::getLeafResourceChildClasses cannot find resource class for %s", resourceClassName.c_str()));
		return 0;
	}

	std::vector<const ResourceClassObject *> children;
	resClass->getLeafChildren(children);

	int count = children.size();
	LocalObjectArrayRefPtr childrenArray = createNewObjectArray(count, JavaLibrary::getClsString());
	for (int i = 0; i < count; ++i)
	{
		if (children[i] != nullptr)
		{
			JavaString name(children[i]->getResourceClassName());
			setObjectArrayElement(*childrenArray, i, name);
		}
	}
	return childrenArray->getReturnValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsResourceNamespace::hasResourceType(JNIEnv *env, jobject self, jstring resourceClass)
{
	if (resourceClass == 0)
	{
		WARNING(true, ("JavaLibrary::hasResourceType passed nullptr resource class"));
		return JNI_FALSE;
	}

	JavaStringParam jresourceClass(resourceClass);
	std::string resourceClassName;
	if (!JavaLibrary::convert(jresourceClass, resourceClassName))
	{
		WARNING(true, ("JavaLibrary::hasResourceType cannot convert resource class name"));
		return JNI_FALSE;
	}

	ServerResourceClassObject const * const resClass = safe_cast<ServerResourceClassObject const *>(ServerUniverse::getInstance().getResourceClassByName(resourceClassName));
	if (resClass == nullptr)
	{
		WARNING(true, ("JavaLibrary::hasResourceType cannot find resource class for %s", resourceClassName.c_str()));
		return JNI_FALSE;
	}

	return (resClass->hasDerivedResourceType()) ? static_cast<jboolean>(JNI_TRUE) : static_cast<jboolean>(JNI_FALSE);
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsResourceNamespace::createResourceCrate(JNIEnv * /*env*/, jobject /*self*/, jlong resourceType, jint amount, jlong destination)
{
	if (resourceType == 0)
	{
		WARNING(true, ("JavaLibrary::createResourceCrate passed nullptr resource type"));
		return 0;
	}

	if (destination == 0)
	{
		WARNING(true, ("JavaLibrary::createResourceCrate passed nullptr destination"));
		return 0;
	}

	if (amount <= 0)
	{
		WARNING(true, ("JavaLibrary::createResourceCrate passed amount %d <= 0", amount));
		return 0;
	}

	ResourceTypeObject const * const rt = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!rt)
	{
		WARNING(true, ("JavaLibrary::createResourceCrate passed unknown resource type"));
		return 0;
	}

	ServerObject * container = nullptr;
	if (!JavaLibrary::getObject(destination, container))
	{
		WARNING(true, ("JavaLibrary::createResourceCrate cannot find destination object"));
		return 0;
	}

	std::string crateTemplateName;
	rt->getCrateTemplate(crateTemplateName);

	ServerObject * object = ServerWorld::createNewObject(crateTemplateName, *container, true);
	if (object == nullptr)
	{
		WARNING(true, ("JavaLibrary::createResourceCrate cannot create crate from template %s", crateTemplateName.c_str()));
		return 0;
	}

	ResourceContainerObject * crate = dynamic_cast<ResourceContainerObject *>(object);
	if (crate == nullptr)
	{
		IGNORE_RETURN(object->permanentlyDestroy(DeleteReasons::SetupFailed));
		WARNING(true, ("JavaLibrary::createResourceCrate crate %s is not a resource container", crateTemplateName.c_str()));
		return 0;
	}

	IGNORE_RETURN(crate->addResource(rt->getNetworkId(), amount, NetworkId::cms_invalid));

	// cause the new crate to auto-combine with anything else in the container
	crate = safe_cast<ResourceContainerObject *>(container->combineResourceContainers(*crate));

	return (crate->getNetworkId()).getValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsResourceNamespace::requestResourceList(JNIEnv * env, jobject /*self*/, jobject loc, jfloat minDensity, jfloat maxDensity, jstring resourceClass)
{
	if (loc == 0)
	{
		WARNING(true, ("JavaLibrary::requestResourceList passed nullptr location"));
		return 0;
	}

	if (resourceClass == 0)
	{
		WARNING(true, ("JavaLibrary::requestResourceList passed nullptr resource class"));
		return 0;
	}

	if (minDensity > maxDensity)
	{
		WARNING(true, ("JavaLibrary::requestResourceList minDensity %f > maxDensity %f", minDensity, maxDensity));
		return 0;
	}

	JavaStringParam jresourceClass(resourceClass);
	std::string resourceClassName;
	if (!JavaLibrary::convert(jresourceClass, resourceClassName))
	{
		WARNING(true, ("JavaLibrary::requestResourceList passed unknown resource class"));
		return 0;
	}

	Location location;
	if (!ScriptConversion::convert(LocalRefParam(loc), location))
	{
		WARNING(true, ("JavaLibrary::requestResourceList cannot convert Java location to C"));
		return 0;
	}
	const Vector & locationPos = location.getCoordinates();

	const PlanetObject * planet = ServerUniverse::getInstance().getPlanetByName(location.getSceneId());
	if (planet == nullptr)
	{
		WARNING(true, ("JavaLibrary::requestResourceList cannot find planet %s", location.getSceneId()));
		return 0;
	}

	// get the resource class and all its children
	ServerResourceClassObject const * const resClass = safe_cast<ServerResourceClassObject const *>(ServerUniverse::getInstance().getResourceClassByName(resourceClassName));
	if (resClass == nullptr)
	{
		WARNING(true, ("JavaLibrary::requestResourceList cannot find resource class %s", resourceClassName.c_str()));
		return 0;
	}

	std::vector<ResourceTypeObject const *> types;
	resClass->getAllDerivedResourceTypes(types);

	// create parallel vectors to store the resource types and their densities in
	std::vector<ResourceTypeObject const *> goodTypes;
	std::vector<float> goodDensities;
	goodTypes.reserve(types.size());
	goodDensities.reserve(types.size());

	// for each resource type, see if its density is in the desired range, and
	// add it to goodTypes/goodDensities if it is
	for (std::vector<ResourceTypeObject const *>::const_iterator i=types.begin(); i!=types.end(); ++i)
	{
		if (!(*i)->isDepleted())
		{
			ResourcePoolObject const * const pool = (*i)->getPoolForPlanet(*planet);
			if (pool != nullptr)
			{
				float density = pool->getEfficiencyAtLocation(locationPos.x, locationPos.z);
				if (density >= minDensity && density <= maxDensity)
				{
					goodTypes.push_back(*i);
					goodDensities.push_back(density);
				}
			}
		}
	}

	size_t count = goodTypes.size();
	LocalObjectArrayRefPtr densityArray = createNewObjectArray(count, JavaLibrary::getClsResourceDensity());
	if (densityArray == LocalObjectArrayRef::cms_nullPtr)
	{
		WARNING(true, ("JavaLibrary::requestResourceList cannot create resource density array of size %d", count));
		return 0;
	}

	{
		for (size_t i = 0; i < count; ++i)
		{
			LocalRefPtr densityObject = allocObject(JavaLibrary::getClsResourceDensity());
			if (densityObject != LocalRef::cms_nullPtr)
			{
				LocalRefPtr oid = JavaLibrary::getObjId(goodTypes[i]->getNetworkId());
				setObjectField(*densityObject, JavaLibrary::getFidResourceDensityResourceType(), *oid);
				setFloatField(*densityObject, JavaLibrary::getFidResourceDensityDensity(), goodDensities[i]);
				setObjectArrayElement(*densityArray, i, *densityObject);
			}
		}
	}

	return densityArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsResourceNamespace::getResourceAttributes(JNIEnv * env, jobject /*self*/, jlong resourceType)
{
	ResourceTypeObject const * const resourceTypeObject = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!resourceTypeObject)
	{
		WARNING(true, ("JavaLibrary::getResourceAttributes passed invalid resource type %s",NetworkId(resourceType).getValueString().c_str()));
		return 0;
	}

	const std::map<std::string, int> & attribs = resourceTypeObject->getResourceAttributes();

	LocalObjectArrayRefPtr attribArray = createNewObjectArray(attribs.size(), JavaLibrary::getClsResourceAttribute());
	if (attribArray == LocalObjectArrayRef::cms_nullPtr)
	{
		WARNING(true, ("JavaLibrary::getResourceAttributes could not create Java resource array of size %u", attribs.size()));
		return 0;
	}

	int j = 0;
	for (std::map<std::string, int>::const_iterator i = attribs.begin(); i != attribs.end(); ++i, ++j)
	{
		const std::pair<std::string, int> & data = *i;
		JavaString name(data.first);

		LocalRefPtr attrib = allocObject(JavaLibrary::getClsResourceAttribute());
		if (attrib == LocalRef::cms_nullPtr)
		{
			WARNING(true, ("JavaLibrary::getResourceAttributes could not allocate memory for a Java resource_attribute"));
			return 0;
		}

		setObjectField(*attrib, JavaLibrary::getFidResourceAttributeName(), name);
		setIntField(*attrib, JavaLibrary::getFidResourceAttributeValue(), data.second);
		setObjectArrayElement(*attribArray, j, *attrib);
	}

	return attribArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsResourceNamespace::getScaledResourceAttributes(JNIEnv * env, jobject /*self*/, jlong resourceType, jstring resourceClass)
{
	// convert the params to C
	ResourceTypeObject const * const resourceTypeObject = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!resourceTypeObject)
	{
		WARNING(true, ("JavaLibrary::getScaledResourceAttributes passed invalid resource type %s",NetworkId(resourceType).getValueString().c_str()));
		return 0;
	}

	JavaStringParam jresourceClass(resourceClass);
	std::string resourceClassName;
	if (!JavaLibrary::convert(jresourceClass, resourceClassName))
	{
		WARNING(true, ("JavaLibrary::getScaledResourceAttributes cannot convert resource class name"));
		return 0;
	}
	ServerResourceClassObject const * const resClass = safe_cast<ServerResourceClassObject const *>(ServerUniverse::getInstance().getResourceClassByName(resourceClassName));
	if (resClass == nullptr)
	{
		WARNING(true, ("JavaLibrary::getScaledResourceAttributes cannot find resource class %s", resourceClassName.c_str()));
		return 0;
	}

	// make sure the resource type is derived from the resource class
	if (!resourceTypeObject->isDerivedFrom(*resClass))
	{
		WARNING(true, ("JavaLibrary::getScaledResourceAttributes passed resource "
			"type %s that is not derived from resource class %s",
			resourceTypeObject->getResourceName().c_str(), resourceClassName.c_str()));
		return 0;
	}

	// rescale the resource type values to 1-1000, based on the resource class ranges
	std::map<std::string, int> attribs(resourceTypeObject->getResourceAttributes());
	{
	const ServerResourceClassObject::ResourceAttributeRangesType & classAttribs = resClass->getResourceAttributeRanges();
	for (std::map<std::string, int>::iterator i = attribs.begin(); i != attribs.end(); ++i)
	{
		ServerResourceClassObject::ResourceAttributeRangesType::const_iterator found = classAttribs.find((*i).first);
		if (found != classAttribs.end())
		{
			int const min = (*found).second.first;
			int const max = (*found).second.second;
			if ((max != min) && (max > 0))
			{
				int scaledAttrib = ((*i).second * 1000) / max;
				if (((*i).second * 1000) % max)
					++scaledAttrib;

				if (scaledAttrib > 1000)
					scaledAttrib = 1000;
				else if (scaledAttrib < 1)
					scaledAttrib = 1;

				DEBUG_LOG("crafting", ("Scaled resource %s attrib %s from %d to %d\n",
					resourceTypeObject->getResourceName().c_str(), (*i).first.c_str(),
					(*i).second, scaledAttrib));
				(*i).second = scaledAttrib;
			}
		}
		else
		{
			WARNING(true, ("JavaLibrary::getScaledResourceAttributes, resource "
				"type %s has attribute %s that is not in resource class %s",
				resourceTypeObject->getResourceName().c_str(), (*i).first.c_str(),
				resourceClassName.c_str()));
		}

		if ((*i).second > 1000)
			(*i).second = 1000;
		else if ((*i).second < 1)
			(*i).second = 1;
	}
	}

	// conver the C data to Java
	LocalObjectArrayRefPtr attribArray = createNewObjectArray(attribs.size(), JavaLibrary::getClsResourceAttribute());
	if (attribArray == LocalObjectArrayRef::cms_nullPtr)
	{
		WARNING(true, ("JavaLibrary::getScaledResourceAttributes could not create Java resource array of size %u", attribs.size()));
		return 0;
	}

	{
	int j = 0;
	for (std::map<std::string, int>::const_iterator i = attribs.begin(); i != attribs.end(); ++i, ++j)
	{
		const std::pair<std::string, int> & data = *i;
		JavaString name(data.first);

		LocalRefPtr attrib = allocObject(JavaLibrary::getClsResourceAttribute());
		if (attrib == LocalRef::cms_nullPtr)
		{
			WARNING(true, ("JavaLibrary::getScaledResourceAttributes could not allocate memory for a Java resource_attribute"));
			return 0;
		}

		setObjectField(*attrib, JavaLibrary::getFidResourceAttributeName(), name);
		setIntField(*attrib, JavaLibrary::getFidResourceAttributeValue(), data.second);
		setObjectArrayElement(*attribArray, j, *attrib);
	}
	}

	return attribArray->getReturnValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsResourceNamespace::getResourceAttribute(JNIEnv * /*env*/, jobject /*self*/, jlong resourceType, jstring attributeName)
{
	JavaStringParam jattributeName(attributeName);
	std::string attributeNameStr;
	if (!JavaLibrary::convert(jattributeName, attributeNameStr))
	{
		WARNING(true, ("JavaLibrary::getResourceAttribute cannot convert attribute name"));
		return -1;
	}

	if (resourceType == 0)
	{
		WARNING(true, ("JavaLibrary::getResourceAttribute passed nullptr resource type"));
		return -1;
	}

	ResourceTypeObject const * const resourceTypeObject = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!resourceTypeObject)
	{
		WARNING(true, ("JavaLibrary::getResourceAttribute passed invalid resource type %s",NetworkId(resourceType).getValueString().c_str()));
		return -1;
	}

	return (resourceTypeObject->getAttribute(attributeNameStr));
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsResourceNamespace::getRecycledVersionOfResourceType(JNIEnv * /*env*/, jobject /*self*/, jlong resourceType)
{
	ResourceTypeObject const * const resourceTypeObject = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	ResourceTypeObject const * const recycledTypeObject = resourceTypeObject ? resourceTypeObject->getRecycledVersion() : nullptr;
	if (recycledTypeObject)
		return (recycledTypeObject->getNetworkId()).getValue();
	else
		return 0;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsResourceNamespace::isValidResourceId(JNIEnv * /*env*/, jobject /*self*/, jlong resourceType)
{
	if (resourceType == 0)
	{
		return JNI_FALSE;
	}

	ResourceTypeObject const * const rt = ServerUniverse::getInstance().getResourceTypeById(NetworkId(resourceType));
	if (!rt)
	{
		return JNI_FALSE;
	}
	return JNI_TRUE;
}

// ======================================================================
