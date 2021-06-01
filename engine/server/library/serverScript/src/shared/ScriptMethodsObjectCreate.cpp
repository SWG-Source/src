//========================================================================
//
// ScriptMethodsObjectCreate.cpp - implements script methods dealing with object
// creation/destruction.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CellObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/FactoryObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/IntangibleObject.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/FlagObjectForDeleteMessage.h"
#include "serverPathfinding/CityPathGraphManager.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedLog/Log.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedTerrain/TerrainModificationHelper.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "sharedFoundation/CrcConstexpr.hpp"

using namespace JNIWrappersNamespace;


//========================================================================
// constants
//========================================================================

static const uint32 THEATER_DATATABLE_TAG = constcrc("THEATER");
static const std::string THEATER_TEMPLATE("object/intangible/theater/base_theater.iff");
static const std::string THEATER_FLATTEN_LAYER("terrain/poi_small.lay");


// ======================================================================
// ScriptMethodsObjectCreateNamespace
// ======================================================================

namespace ScriptMethodsObjectCreateNamespace
{
	bool  install();
	jlong createTheater(const std::vector<jint> & crcs, const std::vector<Vector> & positions, const std::vector<float> & headings, const std::vector<std::string> & scripts, const Vector & center, const std::string & script, const NetworkId & creator, const std::string & name, int locationType);

	jlong        JNICALL createNewObjectAt(JNIEnv *env, jobject self, jstring source, jlong target);
	jlong        JNICALL createNewObjectInWorld(JNIEnv *env, jobject self, jlong source, jobject location);
	jlong        JNICALL createNewObjectInWorldString(JNIEnv *env, jobject self, jobject source, jobject location);
	jlong        JNICALL createNewObjectInContainer(JNIEnv *env, jobject self, jlong source, jlong container, jstring slot);
	jlong        JNICALL createNewObjectInContainerString(JNIEnv *env, jobject self, jobject source, jlong container, jstring slot);
	jlong        JNICALL createNewObjectInContainerOverloaded(JNIEnv *env, jobject self, jlong source, jlong container);
	jlong        JNICALL createNewObjectInContainerOverloadedString(JNIEnv *env, jobject self, jobject source, jlong container);
	jlong        JNICALL createNewObjectInInventoryOverloaded(JNIEnv *env, jobject self, jstring sourceName, jlong target);
	jlong        JNICALL createNewObjectCrc(JNIEnv *env, jobject self, int sourceCrc, jobject location);
	jlong        JNICALL createNewObjectInContainerCrc(JNIEnv *env, jobject self, int sourceCrc, jlong container, jstring slot);
	jlong        JNICALL createNewObjectInContainerOverloadedCrc(JNIEnv *env, jobject self, int sourceCrc, jlong container);
	jlong        JNICALL createNewObjectInInventoryOverloadedCrc(JNIEnv *env, jobject self, int sourceCrc, jlong target);
	jlong        JNICALL createNewObjectAtCrc(JNIEnv *env, jobject self, int sourceCrc, jlong target);
	jlong        JNICALL createNewObjectSimulator(JNIEnv *env, jobject self, jstring source, jobject location);
	jboolean     JNICALL destroyObjectSimulator(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL destroyObject(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL destroyObjectHyperspace(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL persistObject(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isObjectPersisted(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL reloadPathNodes(JNIEnv *env, jobject self, jlongArray target);
	jstring      JNICALL getObjectTemplateName(JNIEnv *env, jobject self, jint crc);
	jint         JNICALL getObjectTemplateCrc(JNIEnv *env, jobject self, jstring name);
	jlong        JNICALL createNewObjectTransform(JNIEnv *env, jobject self, jstring templateName, jobject transform, jlong cell);
	jlong        JNICALL createNewObjectTransformCrc(JNIEnv *env, jobject self, jint templateCrc, jobject transform, jlong cell);
	jlong        JNICALL createTheaterDatatableOnly(JNIEnv *env, jobject self, jstring datatable, jlong caller, jstring name, jint locationType);
	jlong        JNICALL createTheaterDatatable(JNIEnv *env, jobject self, jstring datatable, jobject center, jstring script, jlong caller, jstring name, jint locationType);
	jboolean     JNICALL createRemoteTheaterDatatableOnly(JNIEnv *env, jobject self, jstring datatable, jlong caller, jstring name, jint locationType);
	jboolean     JNICALL createRemoteTheaterDatatable(JNIEnv *env, jobject self, jstring datatable, jobject center, jstring script, jlong caller, jstring name, jint locationType);
	jlong        JNICALL createTheaterCrc(JNIEnv *env, jobject self, jintArray crcs, jobjectArray positions, jobject center, jstring script, jlong caller);
	jlong        JNICALL createTheaterString(JNIEnv *env, jobject self, jobjectArray templates, jobjectArray positions, jobject center, jstring script, jlong caller);
	jboolean     JNICALL assignTheaterToPlayer(JNIEnv *env, jobject self, jlong player, jstring datatable, jlong caller, jstring name, jint locationType);
	jboolean     JNICALL assignTheaterToPlayerLocation(JNIEnv *env, jobject self, jlong player, jstring datatable, jobject basePosition, jstring script, jlong caller, jstring name, jint locationType);
	jboolean     JNICALL unassignTheaterFromPlayer(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL hasTheaterAssigned(JNIEnv *env, jobject self, jlong player);
	jstring      JNICALL getTheaterName(JNIEnv *env, jobject self, jlong theater);
	jlong        JNICALL findTheater(JNIEnv *env, jobject self, jstring name);
	jlong        JNICALL createSchematicString(JNIEnv *env, jobject self, jstring draftSchematic, jlong container);
	jlong        JNICALL createSchematicCrc(JNIEnv *env, jobject self, jint draftSchematicCrc, jlong container);
	jlong        JNICALL createNewObjectTransformHyperspace(JNIEnv *env, jobject self, jstring templateName, jobject transform, jlong cell);
	jlong        JNICALL createNewObjectTransformCrcHyperspace(JNIEnv *env, jobject self, jint templateCrc, jobject transform, jlong cell);
	jlong        JNICALL createNewObjectTransformInternal(JNIEnv *env, jobject self, jstring templateName, jobject transform, jlong cell, bool hyperspace =  false);
	jlong        JNICALL createNewObjectTransformCrcInternal(JNIEnv *env, jobject self, jint templateCrc, jobject transform, jlong cell, bool hyperspace = false);
	jboolean     JNICALL updateNetworkTriggerVolume(JNIEnv *env, jobject self, jlong target, jfloat radius);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsObjectCreateNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsObjectCreateNamespace::c)}
	JF("_createObject", "(Ljava/lang/String;Lscript/location;)J", createNewObjectInWorldString),
	JF("_createObject", "(JLscript/location;)J", createNewObjectInWorld),
	JF("_createObject", "(Ljava/lang/String;JLjava/lang/String;)J", createNewObjectInContainerString),
	JF("_createObject", "(JJLjava/lang/String;)J", createNewObjectInContainer),
	JF("_createObjectOverloaded", "(Ljava/lang/String;J)J", createNewObjectInContainerOverloadedString),
	JF("_createObjectOverloaded", "(JJ)J", createNewObjectInContainerOverloaded),
	JF("_createObjectInInventoryAllowOverload", "(Ljava/lang/String;J)J", createNewObjectInInventoryOverloaded),
	JF("_createObjectAt","(Ljava/lang/String;J)J", createNewObjectAt),
	JF("_createObject", "(ILscript/location;)J", createNewObjectCrc),
	JF("_createObject", "(IJLjava/lang/String;)J", createNewObjectInContainerCrc),
	JF("_createObjectOverloaded", "(IJ)J", createNewObjectInContainerOverloadedCrc),
	JF("_createObjectInInventoryAllowOverload", "(IJ)J", createNewObjectInInventoryOverloadedCrc),
	JF("_createObjectAt","(IJ)J", createNewObjectAtCrc),
	JF("_createObjectSimulator", "(Ljava/lang/String;Lscript/location;)J", createNewObjectSimulator),
	JF("_destroyObjectSimulator", "(J)Z", destroyObjectSimulator),
	JF("__destroyObject", "(J)Z", destroyObject),
	JF("__destroyObjectHyperspace", "(J)Z", destroyObjectHyperspace),
	JF("_persistObject", "(J)Z", persistObject),
	JF("_isObjectPersisted", "(J)Z", isObjectPersisted),
	JF("_reloadPathNodes", "([J)Z", reloadPathNodes),
	JF("getObjectTemplateName", "(I)Ljava/lang/String;", getObjectTemplateName),
	JF("getObjectTemplateCrc", "(Ljava/lang/String;)I", getObjectTemplateCrc),
	JF("_createObject", "(Ljava/lang/String;Lscript/transform;J)J", createNewObjectTransform),
	JF("_createObject", "(ILscript/transform;J)J", createNewObjectTransformCrc),
	JF("_createTheater", "(Ljava/lang/String;JLjava/lang/String;I)J", createTheaterDatatableOnly),
	JF("_createTheater", "(Ljava/lang/String;Lscript/location;Ljava/lang/String;JLjava/lang/String;I)J", createTheaterDatatable),
	JF("_createRemoteTheater", "(Ljava/lang/String;JLjava/lang/String;I)Z", createRemoteTheaterDatatableOnly),
	JF("_createRemoteTheater", "(Ljava/lang/String;Lscript/location;Ljava/lang/String;JLjava/lang/String;I)Z", createRemoteTheaterDatatable),
	JF("_createTheater", "([I[Lscript/location;Lscript/location;Ljava/lang/String;J)J", createTheaterCrc),
	JF("_createTheater", "([Ljava/lang/String;[Lscript/location;Lscript/location;Ljava/lang/String;J)J", createTheaterString),
	JF("_assignTheaterToPlayer", "(JLjava/lang/String;JLjava/lang/String;I)Z", assignTheaterToPlayer),
	JF("_assignTheaterToPlayer", "(JLjava/lang/String;Lscript/location;Ljava/lang/String;JLjava/lang/String;I)Z", assignTheaterToPlayerLocation),
	JF("_unassignTheaterFromPlayer", "(J)Z", unassignTheaterFromPlayer),
	JF("_hasTheaterAssigned", "(J)Z", hasTheaterAssigned),
	JF("_getTheaterName", "(J)Ljava/lang/String;", getTheaterName),
	JF("_findTheater", "(Ljava/lang/String;)J", findTheater),
	JF("_createSchematic", "(Ljava/lang/String;J)J", createSchematicString),
	JF("_createSchematic", "(IJ)J", createSchematicCrc),
	JF("_createObjectHyperspace", "(Ljava/lang/String;Lscript/transform;J)J", createNewObjectTransformHyperspace),
	JF("_createObjectHyperspace", "(ILscript/transform;J)J", createNewObjectTransformCrcHyperspace),
	JF("_updateNetworkTriggerVolume", "(JF)Z", updateNetworkTriggerVolume),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// utility functions
//========================================================================

/**
 * Creates a group of objects centered at a given position.
 *
 * @param crcs			template crcs of the objects to create
 * @param positions		where to create the objects, relative to the theater center
 * @param headings		the orientation of the objects
 * @param scripts       list of scripts that will be attached to the created objects
 * @param center		the location of the master theater object - may or may
 *						not the the actual center of the theater
 * @param script		optional script to be attached to the master theater object;
 *						the trigger OnTheaterCreated will be called when all the
 *						theater objects have been created
 * @param creator		the object that is creating the theater
 * @param name          the theater's name
 * @param locationType	how to create the theater
 *
 * @return the obj_id of the theater object
 */
jlong ScriptMethodsObjectCreateNamespace::createTheater(const std::vector<jint> & crcs,
	const std::vector<Vector> & positions, const std::vector<float> & headings,
	const std::vector<std::string> & scripts, const Vector & center,
	const std::string & script, const NetworkId & creator, const std::string & name,
	int locationType)
{
	if (crcs.empty())
	{
		WARNING(true, ("JavaLibrary::createTheater crc list empty"));
		return 0;
	}
	if (crcs.size() != positions.size())
	{
		WARNING(true, ("JavaLibrary::createTheater positions length does not "
			"match crcs"));
		return 0;
	}
	if (!headings.empty() && crcs.size() != headings.size())
	{
		WARNING(true, ("JavaLibrary::createTheater headings length does not "
			"match crcs"));
		return 0;
	}
	if (!scripts.empty() && crcs.size() != scripts.size())
	{
		WARNING(true, ("JavaLibrary::createTheater scripts length does not "
			"match crcs"));
		return 0;
	}
	if (!name.empty())
	{
		// make sure the theater doesn't already exist
		const NetworkId & oldTheater = ServerUniverse::getInstance().findTheaterId(name);
		if (oldTheater != NetworkId::cms_invalid)
		{
			WARNING(true, ("JavaLibrary::createTheater asked to create theater "
				"with name %s which is already assigned to theater %s",
				name.c_str(), oldTheater.getValueString().c_str()));
			return 0;
		}
	}

	// Find out the area the theater will take up and use getGoodLocation
	// to try and find somewhere where we can place the theater. Our search
	// area is the size of the theater * 3.
	float minx = FLT_MAX;
	float maxx = -FLT_MIN;
	float minz = FLT_MAX;
	float maxz = -FLT_MIN;
	size_t count = positions.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (positions[i].x < minx)
			minx = positions[i].x;
		else if (positions[i].x > maxx)
			maxx = positions[i].x;
		if (positions[i].z < minz)
			minz = positions[i].z;
		else if (positions[i].z > maxz)
			maxz = positions[i].z;
	}
	float dx = maxx - minx;
	float dz = maxz - minz;
	float centerX = minx + dx / 2.0f + center.x;
	float centerZ = minz + dz / 2.0f + center.z;

	TerrainGenerator::Layer * layer = nullptr;
	if (locationType == IntangibleObject::TLT_flatten)
	{
		layer = TerrainModificationHelper::importLayer(THEATER_FLATTEN_LAYER.c_str());
		if (layer == nullptr)
		{
			WARNING (true, ("Layer %s not found for theater, using getGoodLocation "
				"instead", THEATER_FLATTEN_LAYER.c_str()));
			locationType = IntangibleObject::TLT_getGoodLocation;
		}
	}

	Vector goodLoc(centerX, 0, centerZ);
	if (locationType == IntangibleObject::TLT_getGoodLocation)
	{
		goodLoc = ServerWorld::getGoodLocation(dx, dz, Vector(minx - dx + center.x,
			0, minz - dz + center.z), Vector(maxx + dx + center.x, 0, maxz + dz + center.z),
			false, false);
	}
	if (goodLoc.x == 0 && goodLoc.z == 0)
	{
		WARNING(true, ("JavaLibrary::createTheater could not find good location "
			"of size %f %f around point %f %f", dx, dz, centerX, centerZ));
		return 0;
	}
	DEBUG_LOG("Theater", ("JavaLibrary::createTheater found goodloc at %f %f",
		goodLoc.x, goodLoc.z));

	// create an object at the center of the theater that will be used to represent
	// the entire theater
	Transform tr;
	tr.setPosition_p(goodLoc.x + center.x - centerX, 0, goodLoc.z + center.z - centerZ);
	IntangibleObject * theater = safe_cast<IntangibleObject *>(ServerWorld::createNewObject(THEATER_TEMPLATE, tr, 0, false));
	NOT_NULL(theater);
	theater->setTheater();
	if (layer != nullptr)
	{
		theater->setLayer(layer);
	}

	// this is freaking ugly, but a jint is defined as a long in Windows and an
	// int in Linux, so we use our int32 definition
	theater->setObjects(*reinterpret_cast<const std::vector<int32> *>(&crcs),
		positions, headings, scripts);
	theater->setTheaterCreator(creator);
	theater->setTheaterName(name);
	theater->addToWorld();
	theater->scheduleForAlter();

	if (!script.empty())
	{
		NOT_NULL(theater->getScriptObject());
		theater->getScriptObject()->attachScript(script, true);
	}

	return (theater->getNetworkId()).getValue();
}	// JavaLibrary::createTheater


//========================================================================
// class JavaLibrary JNI object creation callback methods
//========================================================================

/**
 * Creates a new object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param source		the template id or the object id to create the object with
 * @param location		where to put the object
 *
 * @return the new object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectInWorld(JNIEnv *env, jobject self, jlong source, jobject location)
{
	if (source == 0)
		return 0;
	if (location == 0)
		return 0;

	// determine what source is and create the object
	std::string localName;

	ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(source, object))
		return 0;
	localName = object->getTemplateName();

	ConstCharCrcString crcName(ObjectTemplateList::lookUp(localName.c_str()));
	if (crcName.getCrc() == 0)
	{
		WARNING(true, ("[designer bug] JavaLibrary::createNewObject called with "
			"unknown template %s", localName.c_str()));
		fprintf(stderr, "WARNING: Could not create object from template %s\n",
			localName.c_str());
		JavaLibrary::printJavaStack();
		return 0;
	}
	return createNewObjectCrc(env, self, crcName.getCrc(), location);
}	// JavaLibrary::createNewObject

/**
 * Creates a new object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param source		the template id or the object id to create the object with
 * @param location		where to put the object
 *
 * @return the new object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectInWorldString(JNIEnv *env, jobject self, jobject source, jobject location)
{
	if (source == 0)
		return 0;
	if (location == 0)
		return 0;

	// determine what source is and create the object
	std::string localName;
	if (env->IsInstanceOf(source, JavaLibrary::getClsString()) == JNI_TRUE)
	{
		JavaStringParam localSource(static_cast<jstring>(source));
		JavaLibrary::convert(localSource, localName);
	}
	else
		return 0;

	ConstCharCrcString crcName(ObjectTemplateList::lookUp(localName.c_str()));
	if (crcName.getCrc() == 0)
	{
		WARNING(true, ("[designer bug] JavaLibrary::createNewObject called with "
			"unknown template %s", localName.c_str()));
		fprintf(stderr, "WARNING: Could not create object from template %s\n",
			localName.c_str());
		JavaLibrary::printJavaStack();
		return 0;
	}

	jlong result = createNewObjectCrc(env, self, crcName.getCrc(), location);

	if (result == 0)
	{
		fprintf(stderr, "WARNING: Could not create object from crc(%d) string(%s)\n", static_cast<int>(crcName.getCrc()), (!crcName.isEmpty() ? crcName.getString() : "No String"));
		JavaLibrary::printJavaStack();
	}

	return result;
}	// JavaLibrary::createNewObject

//--------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectAt(JNIEnv *env, jobject self, jstring source, jlong target)
{
	if (source == 0)
		return 0;
	if (target == 0)
		return 0;

	// get the template name into an std::string
	JavaStringParam localSource(source);
	std::string templateName;
	JavaLibrary::convert(localSource, templateName);

	ConstCharCrcString crcName(ObjectTemplateList::lookUp(templateName.c_str()));
	if (crcName.getCrc() == 0)
	{
		WARNING(true, ("[designer bug] JavaLibrary::createNewObjectAt called with "
			"unknown template %s", templateName.c_str()));
		fprintf(stderr, "WARNING: Could not create object from template %s\n",
			templateName.c_str());
		JavaLibrary::printJavaStack();
		return 0;
	}
	return createNewObjectAtCrc(env, self, crcName.getCrc(), target);
}

//--------------------------------------------------------------------------------

/**
 * Creates a new object and puts it in another object's container slot.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param source		the template id or the object id to create the object with
 * @param container		the object we want to put our new object into
 * @param slot			the container slot we want to put the object in
 *
 * @return the new object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectInContainer(JNIEnv *env, jobject self, jlong source, jlong container, jstring slot)
{
	if (source == 0)
		return 0;
	if (container == 0)
		return 0;

	// determine what source is and create the object
	std::string templateName;

	const ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(source, object))
		return 0;
	templateName = object->getTemplateName();

	ConstCharCrcString crcName(ObjectTemplateList::lookUp(templateName.c_str()));
	if (crcName.getCrc() == 0)
	{
		WARNING(true, ("[designer bug] JavaLibrary::createNewObjectInContainer "
			"called with unknown template %s", templateName.c_str()));
		fprintf(stderr, "WARNING: Could not create object from template %s\n",
			templateName.c_str());
		JavaLibrary::printJavaStack();
		return 0;
	}

	jlong const result = createNewObjectInContainerCrc(env, self, crcName.getCrc(), container, slot);

	if (result == 0)
	{
		fprintf(stderr, "WARNING: Could not create object from crc(%d) string(%s)\n", static_cast<int>(crcName.getCrc()), (!crcName.isEmpty() ? crcName.getString() : "No String"));
		JavaLibrary::printJavaStack();
	}

	return result;
}

//--------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectInContainerString(JNIEnv *env, jobject self, jobject source, jlong container, jstring slot)
{
	if (source == 0)
		return 0;
	if (container == 0)
		return 0;

	// determine what source is and create the object
	std::string templateName;
	if (env->IsInstanceOf(source, JavaLibrary::getClsString()) == JNI_TRUE)
	{
		JavaStringParam localSource(static_cast<jstring>(source));
		JavaLibrary::convert(localSource, templateName);
	}
	else
		return 0;

	ConstCharCrcString crcName(ObjectTemplateList::lookUp(templateName.c_str()));
	if (crcName.getCrc() == 0)
	{
		WARNING(true, ("[designer bug] JavaLibrary::createNewObjectInContainer "
			"called with unknown template %s", templateName.c_str()));
		fprintf(stderr, "WARNING: Could not create object from template %s\n",
			templateName.c_str());
		JavaLibrary::printJavaStack();
		return 0;
	}

	jlong const result = createNewObjectInContainerCrc(env, self, crcName.getCrc(), container, slot);

	if (result == 0)
	{
		fprintf(stderr, "WARNING: Could not create object from crc(%d) string(%s)\n", static_cast<int>(crcName.getCrc()), (!crcName.isEmpty() ? crcName.getString() : "No String"));
		JavaLibrary::printJavaStack();
	}

	return result;
}

//--------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectInContainerOverloaded(JNIEnv *env, jobject self, jlong source, jlong container)
{
	if (source == 0)
		return 0;
	if (container == 0)
		return 0;

	// determine what source is and create the object
	std::string templateName;
	const ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(source, object))
		return 0;
	templateName = object->getTemplateName();

	ConstCharCrcString crcName(ObjectTemplateList::lookUp(templateName.c_str()));
	if (crcName.getCrc() == 0)
	{
		WARNING(true, ("[designer bug] JavaLibrary::createNewObjectInContainerOverloaded "
			"called with unknown template %s", templateName.c_str()));
		fprintf(stderr, "WARNING: Could not create object from template %s\n",
			templateName.c_str());
		JavaLibrary::printJavaStack();
		return 0;
	}
	return createNewObjectInContainerOverloadedCrc(env, self, crcName.getCrc(),
		container);
}

//--------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectInContainerOverloadedString(JNIEnv *env, jobject self, jobject source, jlong container)
{
	if (source == 0)
		return 0;
	if (container == 0)
		return 0;

	// determine what source is and create the object
	std::string templateName;
	if (env->IsInstanceOf(source, JavaLibrary::getClsString()) == JNI_TRUE)
	{
		JavaStringParam localSource(static_cast<jstring>(source));
		JavaLibrary::convert(localSource, templateName);
	}
	else
		return 0;

	ConstCharCrcString crcName(ObjectTemplateList::lookUp(templateName.c_str()));
	if (crcName.getCrc() == 0)
	{
		WARNING(true, ("[designer bug] JavaLibrary::createNewObjectInContainerOverloaded "
			"called with unknown template %s", templateName.c_str()));
		fprintf(stderr, "WARNING: Could not create object from template %s\n",
			templateName.c_str());
		JavaLibrary::printJavaStack();
		return 0;
	}
	return createNewObjectInContainerOverloadedCrc(env, self, crcName.getCrc(),
		container);
}

//--------------------------------------------------------------------------------

/**
 * Creates a new object in a creature's inventory, not failing if full.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param sourceName	the template name used to create the object
 * @param target		the creature to create the object in
 *
 * @return the obj_id of the created object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectInInventoryOverloaded(JNIEnv *env, jobject self, jstring sourceName, jlong target)
{
	if (sourceName == 0 || target == 0)
		return 0;

	JavaStringParam jsourceName(sourceName);
	std::string objectTemplate;
	if (!JavaLibrary::convert(jsourceName, objectTemplate))
		return 0;

	return createNewObjectInInventoryOverloadedCrc(env, self, Crc::calculate(
		objectTemplate.c_str()), target);
}	// JavaLibrary::createNewObjectInInventoryOverloaded

//--------------------------------------------------------------------------------

/**
 * Creates a new object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param sourceCrc		the template crc to create the object with
 * @param location		where to put the object
 *
 * @return the new object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectCrc(JNIEnv *env, jobject self, int sourceCrc, jobject location)
{
	UNREF(self);

	if (sourceCrc == 0)
		return 0;
	if (location == 0)
		return 0;

	// @todo: check to see if the location area matches our scene id
	Vector newPos;
	NetworkId cellId;
	if (!ScriptConversion::convert(location, newPos, cellId))
		return 0;

	CellObject * cell = 0;
	if (cellId.isValid())
	{
		ServerObject * const so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(cellId));
		if (so)
			cell = so->asCellObject();
		if (!cell)
			return 0;
	}

	jlong result = 0;

	Transform tr;
	tr.setPosition_p(newPos);
	ServerObject *newObject = ServerWorld::createNewObject(sourceCrc, tr, cell, false);
	if (newObject != nullptr)
	{
		// get the networkId to return
		NetworkId netId = newObject->getNetworkId();
		if (netId == NetworkId::cms_invalid)
		{
			WARNING(true, ("JavaLibrary::createNewObject created object %s, but "
				"could not get Java obj_id for it!", newObject->getNetworkId().getValueString().c_str()));
			newObject->permanentlyDestroy(DeleteReasons::SetupFailed);
		}
		else
		{
			if (!cell)
				newObject->addToWorld();

			result = netId.getValue();
		}
	}
	else
	{
		fprintf(stderr, "WARNING: Could not create object from crc %d\n", sourceCrc);
		JavaLibrary::printJavaStack();
	}
	return result;
}	// JavaLibrary::createNewObjectCrc

//--------------------------------------------------------------------------------

/**
 * Creates a new object and puts it in another object's container slot.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param sourceCrc		the template crc to create the object with
 * @param container		the object we want to put our new object into
 * @param slot			the container slot we want to put the object in
 *
 * @return the new object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectInContainerCrc(JNIEnv *env, jobject self, int sourceCrc, jlong container, jstring slot)
{
	UNREF(self);

	JavaStringParam localSlot(slot);

	if (sourceCrc == 0)
		return 0;
	if (container == 0)
		return 0;

	// make sure the container exists
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return 0;
	if (ContainerInterface::getContainer(*containerOwner) == nullptr)
		return 0;

	//Create the object
	ServerObject* target = 0;

	SlottedContainer * const slottedContainer = ContainerInterface::getSlottedContainer(*containerOwner);
	if (slottedContainer)
	{
		std::string slotName;
		if (!JavaLibrary::convert(localSlot, slotName))
		{
			delete target;
			return 0;
		}
		if (slotName == "")
		{
			target = ServerWorld::createNewObject(sourceCrc, *containerOwner, false);
		}
		else
		{
			const SlotId slotId = SlotIdManager::findSlotId(CrcLowerString(slotName.c_str()));
			target = ServerWorld::createNewObject(sourceCrc, *containerOwner, slotId, false);
		}
	}
	else
	{
		target = ServerWorld::createNewObject(sourceCrc, *containerOwner, false);
	}

	if (!target)
	{
		fprintf(stderr, "WARNING: Could not create object from crc %d\n", sourceCrc);
		JavaLibrary::printJavaStack();
		return 0;
	}

	//Create its java id
	NetworkId netId = target->getNetworkId();
	if (netId == NetworkId::cms_invalid)
	{
		delete target;
		return 0;
	}

	return netId.getValue();
}	// JavaLibrary::createNewObjectInContainerCrc

//--------------------------------------------------------------------------------

/**
 *
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectInContainerOverloadedCrc(JNIEnv *env, jobject self, int sourceCrc, jlong container)
{
	UNREF(self);

	if (sourceCrc == 0)
		return 0;
	if (container == 0)
		return 0;

	// make sure the container exists
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return 0;
	if (ContainerInterface::getVolumeContainer(*containerOwner) == nullptr)
	{
		DEBUG_WARNING(true, ("createObjectin an overloaded container only works on volume containers"));
		return 0;
	}

	ServerObject* target = ServerWorld::createNewObject(sourceCrc, *containerOwner, false, true);
	if (!target)
	{
		fprintf(stderr, "WARNING: Could not create object from crc %d\n", sourceCrc);
		JavaLibrary::printJavaStack();
		return 0;
	}

	//Create its java id
	NetworkId netId = target->getNetworkId();
	if (netId == NetworkId::cms_invalid)
	{
		delete target;
		return 0;
	}

	return netId.getValue();
}	// JavaLibrary::createNewObjectInContainerOverloadedCrc

//--------------------------------------------------------------------------------

/**
 * Creates a new object in a creature's inventory, not failing if full.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param sourceCrc		the template crc used to create the object
 * @param target		the creature to create the object in
 *
 * @return the obj_id of the created object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectInInventoryOverloadedCrc(
	JNIEnv *env, jobject self, int sourceCrc, jlong target)
{
	if (sourceCrc == 0 || target == 0)
		return 0;

	CreatureObject * targetObject = nullptr;
	if (!JavaLibrary::getObject(target, targetObject))
		return 0;

	ServerObject * inventory = targetObject->getInventory();
	if (inventory == nullptr)
		return 0;

	VolumeContainer * volContainer = ContainerInterface::getVolumeContainer(*inventory);
	if (volContainer == nullptr)
		return 0;

	int oldCapacity = volContainer->debugDoNotUseSetCapacity(-1);

	ServerObject * newObject = ServerWorld::createNewObject(sourceCrc, *inventory, false, true);

	volContainer->debugDoNotUseSetCapacity(oldCapacity);
	volContainer->recalculateVolume();

	if (newObject == nullptr)
		return 0;

	return (newObject->getNetworkId()).getValue();
}	// JavaLibrary::createNewObjectInInventoryOverloadedCrc

//--------------------------------------------------------------------------------

/**
 * Creates a new object at exactly the location specified by target
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param sourceCrc     the template crc of the new object to create
 * @param target        an object whose location and container/cell will be used to create the object.
 * @return the obj_id of the created object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectAtCrc(JNIEnv *env, jobject self, int sourceCrc, jlong target)
{
	UNREF(self);

	if (sourceCrc == 0)
		return 0;
	if (target == 0)
		return 0;

	ServerObject *sourceObject = nullptr;
	if (!JavaLibrary::getObject(target, sourceObject))
		return 0;

	if (!sourceObject->isInWorld())
		return 0;

	ServerObject *cell = 0;
	CellProperty * const cellProperty = sourceObject->getParentCell();
	if (cellProperty && cellProperty != CellProperty::getWorldCellProperty())
		cell = safe_cast<ServerObject *>(&cellProperty->getOwner());

	Transform tr;
	tr.setPosition_p(sourceObject->getPosition_p());
	ServerObject *newObject = ServerWorld::createNewObject(sourceCrc, tr, cell, false);
	if (newObject == nullptr)
	{
		fprintf(stderr, "WARNING: Could not create object from crc %d\n", sourceCrc);
		JavaLibrary::printJavaStack();
		return 0;
	}

	// create an objId to return
	NetworkId netId = newObject->getNetworkId();
	if (netId == NetworkId::cms_invalid)
	{
		delete newObject;
		return 0;
	}

	if (!cell)
		newObject->addToWorld();

	return netId.getValue();
}	// JavaLibrary::createNewObjectAtCrc

//--------------------------------------------------------------------------------


/**

 * Creates a new object, and gives it a playerObject, so it can be used to simulate players
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param source		the template id or the object id to create the object with
 * @param location		where to put the object
 *
 * @return the new object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectSimulator(JNIEnv *env, jobject self, jstring source, jobject location)
{
	if (source == 0)
		return 0;
	if (location == 0)
		return 0;

	// determine what source is and create the object
	std::string localName;
	if (env->IsInstanceOf(source, JavaLibrary::getClsString()) == JNI_TRUE)
	{
		JavaStringParam localSource(static_cast<jstring>(source));
		JavaLibrary::convert(localSource, localName);
	}
	else
		return 0;

	ConstCharCrcString crcName(ObjectTemplateList::lookUp(localName.c_str()));
	if (crcName.getCrc() == 0)
	{
		WARNING(true, ("[designer bug] JavaLibrary::createNewObject called with unknown template %s", localName.c_str()));
		fprintf(stderr, "WARNING: Could not create object from template %s\n", localName.c_str());
		JavaLibrary::printJavaStack();
		return 0;
	}

	int sourceCrc = crcName.getCrc();
	UNREF(self);

	// @todo: check to see if the location area matches our scene id
	Vector newPos;
	NetworkId cellId;
	if (!ScriptConversion::convert(location, newPos, cellId))
		return 0;

	CellObject * cell = 0;
	if (cellId.isValid())
	{
		ServerObject * const so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(cellId));
		if (so)
			cell = so->asCellObject();
		if (!cell)
			return 0;
	}

	jlong result = 0;

	Transform tr;
	tr.setPosition_p(newPos);
	ServerObject *newObject = ServerWorld::createNewObject(sourceCrc, tr, cell, false);
	if (newObject != nullptr)
	{
		// add on the player object
		CreatureObject * creature = dynamic_cast<CreatureObject *>(newObject);
		ServerWorld::createNewObject (ConfigServerGame::getPlayerObjectTemplate (), *creature, false);
		creature->setPlayerControlled (true);
		creature->setAuthoritative (true);

		// create a networkId to return
		NetworkId netId = newObject->getNetworkId();
		if (netId == NetworkId::cms_invalid)
		{
			WARNING(true, ("JavaLibrary::createNewObjectSimulator created object %s, but could not get Java obj_id for it!", newObject->getNetworkId().getValueString().c_str()));
			creature->setPlayerControlled (false);
			newObject->permanentlyDestroy(DeleteReasons::SetupFailed);
		}
		else
		{
			if (!cell)
				newObject->addToWorld();

			result = netId.getValue();
		}
	}
	else
	{
		fprintf(stderr, "WARNING: Could not create object from crc %d\n", sourceCrc);
		JavaLibrary::printJavaStack();
	}
	return result;

}	// JavaLibrary::createNewObjectSimulator

/**
 * Destroys a simulator object. Should only be used for simulator objects
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object to destroy
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::destroyObjectSimulator(JNIEnv *env, jobject self, jlong target)
{
	CreatureObject *playerObject = nullptr;
	// get the object
	if (!JavaLibrary::getObject(target, playerObject))
	{
		WARNING(true, ("JavaLibrary::destroyObjectSimulator : bad player object"));
		return JNI_FALSE;
	}

	// verify that it is a simulator object - it is playerControlled with no client
	if (playerObject->getClient() || !playerObject->isPlayerControlled())
	{
		WARNING(true, ("JavaLibrary::destroyObjectSimulator : the object you are trying to destroy (%s) is not a simulator object", playerObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	playerObject->setPlayerControlled(false);

	return destroyObject(env, self, target);

}   // JavaLibrary::destroyObjectSimulator

/**
 * Destroys an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object to destroy
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::destroyObject(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);
	NOT_NULL(env);

	const NetworkId targetId(target);
	if (targetId == NetworkId::cms_invalid)
		return JNI_FALSE;

	ServerObject* object = nullptr;
	bool retval = JavaLibrary::getObject(target, object);

	if (!retval || !object) // || object->isPersisted())  will be done in permanently destroy
	{
		// flag the object deleted in the database
		FlagObjectForDeleteMessage const dm(targetId, DeleteReasons::Script,false,false,false);
		GameServer::getInstance().sendToDatabaseServer(dm);
	}
	// remove the object from the game
	if (object)
	{
		// check if the object is in a FactoryObject crate
		Object * container = ContainerInterface::getContainedByObject(*object);
		if (container != nullptr && dynamic_cast<FactoryObject *>(container) != nullptr)
		{
			// destroy an object in the factory; if it is the last object,
			// the factory will destroy itself
			FactoryObject * factory = safe_cast<FactoryObject *>(container);
			factory->deleteContents(1);
			return JNI_TRUE;
		}
		else
			return object->permanentlyDestroy(DeleteReasons::Script);
	}

	MessageToQueue::getInstance().sendMessageToC(targetId, "C++DestroySelf", "", 0, false);
	return JNI_TRUE;
}	// JavaLibrary::destroyObject

//--------------------------------------------------------------------------------

/**
 * Destroys an object by hyperspacing it away
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object to destroy
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::destroyObjectHyperspace(JNIEnv *env, jobject self, jlong target)
{
	const NetworkId targetId(target);
	if (targetId == NetworkId::cms_invalid)
		return JNI_FALSE;

	ServerObject * object = nullptr;
	bool retval = JavaLibrary::getObject(target, object);
	if(retval && object)
	{
		object->setHyperspaceOnDestroy(true);
		return destroyObject(env, self, target);
	}
	else
		return JNI_FALSE;
} // JavaLibrary::destroyObjectHyperspace

//--------------------------------------------------------------------------------

/**
 * Persists an object to the database.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object to persist
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::persistObject(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);
	NOT_NULL(env);

	ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;
	object->persist();
	return JNI_TRUE;
}	// JavaLibrary::persistObject

/**
 * Checks if an object has been persisted to the database.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object to check
 *
 * @return JNI_TRUE if the object is persisted, JNI_FALSE if not
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::isObjectPersisted(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);
	NOT_NULL(env);

	ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	if (object->isPersisted())
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::isObjectPersisted

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectCreateNamespace::reloadPathNodes(JNIEnv *env, jobject self, jlongArray target)
{
	UNREF(self);
	NOT_NULL(env);

	std::vector<NetworkId> targetIds;
	if (!ScriptConversion::convert(target, targetIds))
		return JNI_FALSE;

	std::vector<ServerObject *> objectList;

	int targetCount = targetIds.size();
	for(int i = 0; i < targetCount; i++)
	{
		ServerObject * serverObject = ServerWorld::findObjectByNetworkId(targetIds[i]);

		if(serverObject) objectList.push_back(serverObject);
	}

	CityPathGraphManager::reloadPathNodes(objectList);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsObjectCreateNamespace::getObjectTemplateName(JNIEnv *env, jobject self, jint crc)
{
	JavaString str(ObjectTemplateList::lookUp(crc).getString());
	return str.getReturnValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectCreateNamespace::getObjectTemplateCrc(JNIEnv *env, jobject self, jstring name)
{
	JavaStringParam localName(name);
	std::string templateName;
	JavaLibrary::convert(localName, templateName);
	return ObjectTemplateList::lookUp(templateName.c_str()).getCrc();
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectTransform(JNIEnv *env, jobject self, jstring templateName, jobject transform, jlong cell)
{
	return createNewObjectTransformInternal(env, self, templateName, transform, cell, false);
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectTransformHyperspace(JNIEnv *env, jobject self, jstring templateName, jobject transform, jlong cell)
{
	return createNewObjectTransformInternal(env, self, templateName, transform, cell, true);
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectTransformInternal(JNIEnv *env, jobject self, jstring templateName, jobject transform, jlong cell, bool const hyperspace)
{
	std::string localName;
	JavaStringParam localSource(templateName);
	if (!JavaLibrary::convert(localSource, localName))
		JavaLibrary::throwInternalScriptError("JavaLibrary::createNewObjectTransform - bad template name");
	else
	{
		ConstCharCrcString crcName(ObjectTemplateList::lookUp(localName.c_str()));
		if (crcName.getCrc() == 0)
		{
			char buf[256];
			snprintf(buf, sizeof(buf), "JavaLibrary::createNewObjectTransform - unknown template %s", localName.c_str());
			JavaLibrary::throwInternalScriptError(buf);
		}
		else
			return createNewObjectTransformCrcInternal(env, self, crcName.getCrc(), transform, cell, hyperspace);
	}
	return 0;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectTransformCrc(JNIEnv *env, jobject self, jint templateCrc, jobject transform, jlong cell)
{
	return createNewObjectTransformCrcInternal(env, self, templateCrc, transform, cell, false);
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectTransformCrcHyperspace(JNIEnv *env, jobject self, jint templateCrc, jobject transform, jlong cell)
{
	return createNewObjectTransformCrcInternal(env, self, templateCrc, transform, cell, true);
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectCreateNamespace::createNewObjectTransformCrcInternal(JNIEnv *env, jobject self, jint templateCrc, jobject transform, jlong jcell, bool const hyperspace)
{
	NetworkId const cellId(jcell);
	CellObject *cell = 0;

	if (cellId.isValid())
	{
		Object * const o = NetworkIdManager::getObjectById(cellId);
		if (o)
		{
			ServerObject * so = o->asServerObject();
			if (so)
				cell = so->asCellObject();
		}
		if (!cell)
			return 0;
	}

	Transform newTransform;
	if (!ScriptConversion::convert(transform, newTransform))
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "JavaLibrary::createNewObjectTransform - bad transform (template crc %08x)", templateCrc);
		JavaLibrary::throwInternalScriptError(buf);
	}
	else
	{
		ServerObject *newObject = ServerWorld::createNewObject(templateCrc, newTransform, cell, false, hyperspace);
		if (!newObject)
		{
			char buf[256];
			snprintf(buf, sizeof(buf), "JavaLibrary::createNewObjectTransform - ServerWorld::createNewObject failed (template crc %08x, cell %s)", templateCrc, cell ? cell->getNetworkId().getValueString().c_str() : "0");
			JavaLibrary::throwInternalScriptError(buf);
		}
		else
		{
			// create an objId to return
			NetworkId netId = newObject->getNetworkId();
			if (netId == NetworkId::cms_invalid)
			{
				char buf[256];
				snprintf(buf, sizeof(buf), "JavaLibrary::createNewObjectTransform - object %s created but could not get java obj_id for it", newObject->getNetworkId().getValueString().c_str());
				JavaLibrary::throwInternalScriptError(buf);
				newObject->permanentlyDestroy(DeleteReasons::SetupFailed);
			}
			else
			{
				if (!cell)
					newObject->addToWorld();
				return netId.getValue();
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

/**
 * Creates a group of objects centered at a given position. The first entry in
 * the datatable will define the base position of the theater and an optional
 * script to be attached to the master theater object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param datatable		file name of the datatable to read the objects from
 * @param caller        who's creating the theater
 * @param name          the name of the theater
 * @param locationType	how to create the theater
 *
 * @return the id of the master theater object, or nullptr on error (bad datatable name or position)
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createTheaterDatatableOnly(JNIEnv *env, jobject self, jstring datatable, jlong caller, jstring name, jint locationType)
{
int i;

	if (datatable == 0)
		return 0;

	// convert the Java data to C
	JavaStringParam jdatatable(datatable);
	std::string datatableName;
	if (!JavaLibrary::convert(jdatatable, datatableName))
		return 0;

	NetworkId callerId(caller);

	std::string theaterName;
	if (name != 0)
	{
		JavaStringParam jname(name);
		if (!JavaLibrary::convert(jname, theaterName))
			return 0;
	}

	// read the object data from the datatable
	DataTable * dt = DataTableManager::getTable(datatableName, true);
	if (dt == nullptr)
	{
		WARNING(true, ("JavaLibrary::createTheaterDatatable could not open "
			"datatable %s", datatableName.c_str()));
		return 0;
	}
	int rows = dt->getNumRows();
	if (rows <= 0)
	{
		WARNING(true, ("JavaLibrary::createTheaterDatatable datatable %s has no "
			"rows", datatableName.c_str()));
		return 0;
	}
	// the 1st row is master object data so there needs to be at least 2 rows
	if (rows == 1)
	{
		WARNING(true, ("JavaLibrary::createTheaterDatatable datatable %s has no "
			"object rows", datatableName.c_str()));
		return 0;
	}

	int templateColumn = dt->findColumnNumber("template");
	int xColumn        = dt->findColumnNumber("x");
	int yColumn        = dt->findColumnNumber("y");
	int zColumn        = dt->findColumnNumber("z");
	int headingColumn  = dt->findColumnNumber("heading");
	int scriptColumn   = dt->findColumnNumber("script");

	Vector theaterCenter;
	std::vector<jint> objectCrcs;
	std::vector<Vector> objectPositions(rows - 1);
	std::vector<float> objectHeadings(rows - 1);
	std::vector<std::string> objectScripts(rows - 1);
	std::vector<float> tempPos;

	dt->getIntColumn(templateColumn, objectCrcs);
	objectCrcs.erase(objectCrcs.begin());

	dt->getFloatColumn(xColumn, tempPos);
	theaterCenter.x = tempPos[0];
	for (i = 1; i < rows; ++i)
		objectPositions[i-1].x = tempPos[i];
	dt->getFloatColumn(yColumn, tempPos);
	theaterCenter.y = tempPos[0];
	for (i = 1; i < rows; ++i)
		objectPositions[i-1].y = tempPos[i];
	dt->getFloatColumn(zColumn, tempPos);
	theaterCenter.z = tempPos[0];
	for (i = 1; i < rows; ++i)
		objectPositions[i-1].z = tempPos[i];
	dt->getFloatColumn(headingColumn, tempPos);
	for (i = 1; i < rows; ++i)
		objectHeadings[i-1] = tempPos[i];

	std::string theaterScript;
	std::vector<const char *> scripts;
	dt->getStringColumn(scriptColumn, scripts);
	theaterScript = scripts[0];
	for (i = 1; i < rows; ++i)
	{
		objectScripts[i-1] = scripts[i];
	}

	// start the creation process
	return createTheater(objectCrcs, objectPositions, objectHeadings, objectScripts, theaterCenter, theaterScript, callerId, theaterName, locationType);
}	// JavaLibrary::createTheaterDatatableOnly

/**
 * Creates a group of objects centered at a given position.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param datatable		file name of the datatable to read the objects from
 * @param basePosition	the base position of the theater; all objects will be
 *						placed relative to here
 * @param script		optional script to be attached to the master theater object;
 *						the trigger OnTheaterCreated will be called when all the
 *						theater objects have been created
 * @param caller        who's creating the theater
 * @param name          the name of the theater
 * @param locationType	how to create the theater
 *
 * @return the id of the master theater object, or nullptr on error (bad datatable name or position)
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createTheaterDatatable(JNIEnv *env, jobject self, jstring datatable, jobject basePosition, jstring script, jlong caller, jstring name, jint locationType)
{
int i;

	if (datatable == 0 || basePosition == 0)
		return 0;

	// convert the Java data to C
	JavaStringParam jdatatable(datatable);
	std::string datatableName;
	Vector theaterCenter;
	std::string theaterScript;
	if (!JavaLibrary::convert(jdatatable, datatableName))
		return 0;
	if (!ScriptConversion::convertWorld(basePosition, theaterCenter))
		return 0;
	if (script != 0)
	{
		JavaStringParam jscript(script);
		if (!JavaLibrary::convert(jscript, theaterScript))
			return 0;
	}

	NetworkId callerId(caller);

	std::string theaterName;
	if (name != 0)
	{
		JavaStringParam jname(name);
		if (!JavaLibrary::convert(jname, theaterName))
			return 0;
	}

	// read the object data from the datatable
	DataTable * dt = DataTableManager::getTable(datatableName, true);
	if (dt == nullptr)
	{
		WARNING(true, ("JavaLibrary::createTheaterDatatable could not open "
			"datatable %s", datatableName.c_str()));
		return 0;
	}
	int rows = dt->getNumRows();
	if (rows <= 0)
	{
		WARNING(true, ("JavaLibrary::createTheaterDatatable datatable %s has no "
			"rows", datatableName.c_str()));
		return 0;
	}

	int templateColumn = dt->findColumnNumber("template");
	int xColumn        = dt->findColumnNumber("x");
	int yColumn        = dt->findColumnNumber("y");
	int zColumn        = dt->findColumnNumber("z");
	int headingColumn  = dt->findColumnNumber("heading");
	int scriptColumn   = dt->findColumnNumber("script");

	std::vector<jint> objectCrcs;
	std::vector<Vector> objectPositions(rows);
	std::vector<float> objectHeadings;
	std::vector<std::string> objectScripts(rows);
	std::vector<float> tempPos;

	dt->getIntColumn(templateColumn, objectCrcs);
	dt->getFloatColumn(headingColumn, objectHeadings);
	dt->getFloatColumn(xColumn, tempPos);
	for (i = 0; i < rows; ++i)
		objectPositions[i].x = tempPos[i];
	dt->getFloatColumn(yColumn, tempPos);
	for (i = 0; i < rows; ++i)
		objectPositions[i].y = tempPos[i];
	dt->getFloatColumn(zColumn, tempPos);
	for (i = 0; i < rows; ++i)
	{
		objectPositions[i].z = tempPos[i];
		objectScripts[i] = dt->getStringValue(scriptColumn, i);
	}

	// start the creation process
	return createTheater(objectCrcs, objectPositions, objectHeadings, objectScripts, theaterCenter, theaterScript, callerId, theaterName, locationType);
}	// JavaLibrary::createTheaterDatatable

// ----------------------------------------------------------------------

/**
 * Creates a group of objects centered at a given position. The first entry in
 * the datatable will define the base position of the theater and an optional
 * script to be attached to the master theater object. The difference between
 * this command and createTheater is that this will work if the theater is created
 * on a different server than the one the command is called from, although the caller
 * and the theater must still be on the same planet.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param datatable		file name of the datatable to read the objects from
 * @param caller        who's creating the theater
 * @param name          the name of the theater
 * @param locationType	how to create the theater
 *
 * @return true on success, or false on error (bad datatable name)
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::createRemoteTheaterDatatableOnly(JNIEnv *env, jobject self, jstring datatable, jlong caller, jstring name, jint locationType)
{
	if (datatable == 0)
		return JNI_FALSE;

	// convert the Java data to C
	JavaStringParam jdatatable(datatable);
	std::string datatableName;
	if (!JavaLibrary::convert(jdatatable, datatableName))
		return JNI_FALSE;

	NetworkId callerId(caller);

	std::string theaterName;
	if (name != 0)
	{
		JavaStringParam jname(name);
		if (!JavaLibrary::convert(jname, theaterName))
			return JNI_FALSE;
		if (!theaterName.empty())
		{
			// make sure the theater doesn't exist
			const NetworkId & oldTheater = ServerUniverse::getInstance().findTheaterId(theaterName);
			if (oldTheater != NetworkId::cms_invalid)
			{
				WARNING(true, ("JavaLibrary::createRemoteTheaterDatatableOnly "
					"asked to create theater with name %s which is already "
					"assigned to theater %s", theaterName.c_str(),
					oldTheater.getValueString().c_str()));
				return JNI_FALSE;
			}
		}
	}

	// read the object data from the datatable
	DataTable * dt = DataTableManager::getTable(datatableName, true);
	if (dt == nullptr)
	{
		WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable could not open "
			"datatable %s", datatableName.c_str()));
		return JNI_FALSE;
	}
	int rows = dt->getNumRows();
	if (rows <= 0)
	{
		WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable datatable %s "
			"has no rows", datatableName.c_str()));
		return JNI_FALSE;
	}
	// the 1st row is master object data so there needs to be at least 2 rows
	if (rows == 1)
	{
		WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable datatable %s "
			"has no object rows", datatableName.c_str()));
		return JNI_FALSE;
	}

	if (static_cast<uint32>(dt->getIntValue("template", 0)) != Crc::normalizeAndCalculate("THEATER"))
	{
		WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable datatable %s "
			"first entry not THEATER", datatableName.c_str()));
		return JNI_FALSE;
	}

	Vector location;
	location.x = dt->getFloatValue("x", 0);
	location.y = dt->getFloatValue("y", 0);
	location.z = dt->getFloatValue("z", 0);

	//
	// create a "theater spawner" at the destination, and send it a message
	// telling it what theater to spawn
	//

	// create the packed dictionary we are going to send to the theater spawner
	ScriptParams params;
	params.addParam(datatableName.c_str(), "datatable");
	params.addParam(callerId, "caller");
	params.addParam(theaterName.c_str(), "name");
	params.addParam(locationType, "locationType");
	ScriptDictionaryPtr dictionary;
	GameScriptObject::makeScriptDictionary(params, dictionary);
	if (dictionary.get() == nullptr)
	{
		WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable could not "
			"create dictionary for theater datatable %s", datatableName.c_str()));
		return JNI_FALSE;
	}

	// create the spawner
	Transform tr;
	tr.setPosition_p(location);
	ServerObject * spawner = ServerWorld::createNewObject("object/tangible/spawning/remote_theater_spawner.iff", tr, 0, false);
	if (spawner == nullptr)
	{
		WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable unable to "
			"create remote theater spawner for datatable %s", datatableName.c_str()));
		return JNI_FALSE;
	}
	spawner->addToWorld();

	// tell the spawner what to create
	dictionary->serialize();
	MessageToQueue::getInstance().sendMessageToJava(spawner->getNetworkId(),
		"handleCreateRemoteTheater", dictionary->getSerializedData(), 1, false);
	return JNI_TRUE;
}	// JavaLibrary::createRemoteTheaterDatatableOnly

// ----------------------------------------------------------------------

/**
 * Creates a group of objects centered at a given position. The difference between this command and createTheater
 * is that this will work if the theater is created on a different server than the one the command is called from,
 * although the caller and the theater must still be on the same planet.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param datatable		file name of the datatable to read the objects from
 * @param basePosition	the base position of the theater; all objects will be placed relative to here
 * @param script		optional script to be attached to the master theater object; the trigger OnTheaterCreated
 *                   	will be called when all the theater objects have been created
 * @param caller        who's creating the theater
 * @param name          the name of the theater
 * @param locationType	how to create the theater
 *
 * @return true on success, or false on error (bad datatable name)
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::createRemoteTheaterDatatable(JNIEnv *env, jobject self, jstring datatable, jobject basePosition, jstring script, jlong caller, jstring name, jint locationType)
{
	if (datatable == 0 || basePosition == 0)
		return JNI_FALSE;

	// convert the Java data to C
	JavaStringParam jdatatable(datatable);
	std::string datatableName;
	Vector theaterCenter;
	std::string theaterScript;
	if (!JavaLibrary::convert(jdatatable, datatableName))
		return JNI_FALSE;
	if (!ScriptConversion::convertWorld(basePosition, theaterCenter))
		return JNI_FALSE;
	if (script != 0)
	{
		JavaStringParam jscript(script);
		if (!JavaLibrary::convert(jscript, theaterScript))
			return JNI_FALSE;
	}

	NetworkId callerId(caller);

	std::string theaterName;
	if (name != 0)
	{
		JavaStringParam jname(name);
		if (!JavaLibrary::convert(jname, theaterName))
			return JNI_FALSE;
		if (!theaterName.empty())
		{
			// make sure the theater doesn't exist
			const NetworkId & oldTheater = ServerUniverse::getInstance().findTheaterId(theaterName);
			if (oldTheater != NetworkId::cms_invalid)
			{
				WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable asked "
					"to create theater with name %s which is already assigned "
					"to theater %s", theaterName.c_str(),
					oldTheater.getValueString().c_str()));
				return JNI_FALSE;
			}
		}
	}

	// read the object data from the datatable
	DataTable * dt = DataTableManager::getTable(datatableName, true);
	if (dt == nullptr)
	{
		WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable could not open "
			"datatable %s", datatableName.c_str()));
		return JNI_FALSE;
	}
	int rows = dt->getNumRows();
	if (rows <= 0)
	{
		WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable datatable %s has no "
			"rows", datatableName.c_str()));
		return JNI_FALSE;
	}

	//
	// create a "theater spawner" at the destination, and send it a message
	// telling it what theater to spawn
	//

	// create the packed dictionary we are going to send to the theater spawner
	ScriptParams params;
	params.addParam(datatableName.c_str(), "datatable");
	params.addParam(theaterCenter.x, "x");
	params.addParam(theaterCenter.z, "z");
	params.addParam(theaterScript.c_str(), "script");
	params.addParam(callerId, "caller");
	params.addParam(theaterName.c_str(), "name");
	params.addParam(locationType, "locationType");
	ScriptDictionaryPtr dictionary;
	GameScriptObject::makeScriptDictionary(params, dictionary);
	if (dictionary.get() == nullptr)
	{
		WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable could not "
			"create dictionary for theater datatable %s", datatableName.c_str()));
		return JNI_FALSE;
	}
	LOG("theater", ("createRemoteTheaterDatatable will create theater at %f, %f\n",
		theaterCenter.x, theaterCenter.z));

	// create the spawner
	Transform tr;
	tr.setPosition_p(theaterCenter);
	ServerObject * spawner = ServerWorld::createNewObject("object/tangible/spawning/remote_theater_spawner.iff", tr, 0, false);
	if (spawner == nullptr)
	{
		WARNING(true, ("JavaLibrary::createRemoteTheaterDatatable unable to "
			"create remote theater spawner for datatable %s", datatableName.c_str()));
		return JNI_FALSE;
	}
	spawner->addToWorld();

	// tell the spawner what to create
	dictionary->serialize();
	MessageToQueue::getInstance().sendMessageToJava(spawner->getNetworkId(),
		"handleCreateRemoteTheater", dictionary->getSerializedData(), 1, false);
	return JNI_TRUE;
}	// JavaLibrary::createRemoteTheaterDatatable

// ----------------------------------------------------------------------

/**
 * Creates a group of objects centered at a given position.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param crcs			template crcs of the objects to create
 * @param positions		where to create the objects, relative to the theater center
 * @param basePosition	the base position of the theater; all objects will be
 *						placed relative to here
 * @param script		optional script to be attached to the master theater object;
 *						the trigger OnTheaterCreated will be called when all the
 *						theater objects have been created
 * @param caller        who's creating the theater
 *
 * @return the id of the master theater object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createTheaterCrc(JNIEnv *env, jobject self, jintArray crcs, jobjectArray positions, jobject basePosition, jstring script, jlong caller)
{
	if (crcs == 0 || positions == 0 || basePosition == 0)
		return 0;

	// make sure the arrays are good
	jsize crcCount = env->GetArrayLength(crcs);
	jsize posCount = env->GetArrayLength(positions);
	if (crcCount != posCount)
	{
		WARNING(true, ("JavaLibrary::createTheaterCrc crc and position arrays do "
			"not match"));
		return 0;
	}
	if (crcCount == 0 || posCount == 0)
	{
		WARNING(true, ("JavaLibrary::createTheaterCrc 0-length crc or position "
			"arrays"));
		return 0;
	}

	// convert Java data to C
	std::vector<jint> objectCrcs(crcCount);
	std::vector<Vector> objectPositions;
	Vector theaterCenter;
	std::string theaterScript;
	env->GetIntArrayRegion(crcs, 0, crcCount, &objectCrcs[0]);
	if (!ScriptConversion::convertWorld(positions, objectPositions))
		return 0;
	if (!ScriptConversion::convertWorld(basePosition, theaterCenter))
		return 0;
	if (script != 0)
	{
		JavaStringParam jscript(script);
		if (!JavaLibrary::convert(jscript, theaterScript))
			return 0;
	}

	NetworkId callerId(caller);

	// start the creation process
	std::vector<float> const emptyFloatVector;
	std::vector<std::string> const emptyStringVector;
	std::string const emptyString;
	return createTheater(objectCrcs, objectPositions, emptyFloatVector, emptyStringVector, theaterCenter, theaterScript, callerId, emptyString, IntangibleObject::TLT_getGoodLocation);
}	// JavaLibrary::createTheaterCrc

// ----------------------------------------------------------------------

/**
 * Creates a group of objects centered at a given position.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param templates		template names of the objects to create
 * @param positions		where to create the objects, relative to the theater center
 * @param basePosition	the base position of the theater; all objects will be
 *						placed relative to here
 * @param script		optional script to be attached to the master theater object;
 *						the trigger OnTheaterCreated will be called when all the
 *						theater objects have been created
 * @param caller        who's creating the theater
 *
 * @return the id of the master theater object, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createTheaterString(JNIEnv *env, jobject self, jobjectArray templates, jobjectArray positions, jobject basePosition, jstring script, jlong caller)
{
	if (templates == 0 || positions == 0 || basePosition == 0)
		return 0;

	// make sure the arrays are good
	jsize templatesCount = env->GetArrayLength(templates);
	jsize posCount = env->GetArrayLength(positions);
	if (templatesCount != posCount)
	{
		WARNING(true, ("JavaLibrary::createTheaterString template and position "
			"arrays do not match"));
		return 0;
	}
	if (templatesCount == 0 || posCount == 0)
	{
		WARNING(true, ("JavaLibrary::createTheaterString 0-length tempalte or "
			"position arrays"));
		return 0;
	}

	// convert Java data to C
	std::vector<std::string> objectTemplates;
	std::vector<jint> objectCrcs(templatesCount);
	std::vector<Vector> objectPositions;
	Vector theaterCenter;
	std::string theaterScript;
	if (!ScriptConversion::convert(templates, objectTemplates))
		return 0;
	if (!ScriptConversion::convertWorld(positions, objectPositions))
		return 0;
	if (!ScriptConversion::convertWorld(basePosition, theaterCenter))
		return 0;
	if (script != 0)
	{
		JavaStringParam jscript(script);
		if (!JavaLibrary::convert(jscript, theaterScript))
			return 0;
	}

	NetworkId callerId(caller);

	// convert the template names to crcs
	for (jsize i = 0; i < templatesCount; ++i)
		objectCrcs[i] = Crc::calculate(objectTemplates[i].c_str());

	// start the creation process
	std::vector<float> const emptyFloatVector;
	std::vector<std::string> const emptyStringVector;
	std::string const emptyString;
	return createTheater(objectCrcs, objectPositions, emptyFloatVector, emptyStringVector, theaterCenter, theaterScript, callerId, emptyString, IntangibleObject::TLT_getGoodLocation);
}	// JavaLibrary::createTheaterString

// ----------------------------------------------------------------------

/**
 * Causes a theater to be created when a player gets near its location. We only allow one theater to
 * be assigned to a player at a time.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param playerId		the player who will cause the theater to be created
 * @param datatable		the datatable to read the theater data from; the 1st entry in the
 *                      datatable defines its location, see createTheater(datatable) above
 * @param caller        who's creating the theater
 * @param name          the name of the theater
 * @param locationType	how to create the theater
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::assignTheaterToPlayer(JNIEnv *env, jobject self, jlong playerId, jstring datatable, jlong caller, jstring name, jint locationType)
{
	if (playerId == 0 || datatable == 0)
		return JNI_FALSE;

	JavaStringParam jdatatable(datatable);

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(playerId, playerCreature))
		return JNI_FALSE;

	PlayerObject * const player = PlayerCreatureController::getPlayerObject(
		playerCreature);
	if (player == nullptr)
		return JNI_FALSE;

	if (player->hasTheater())
		return JNI_FALSE;

	std::string datatableName;
	if (!JavaLibrary::convert(jdatatable, datatableName))
		return JNI_FALSE;

	NetworkId callerId(caller);

	std::string theaterName;
	if (name != 0)
	{
		JavaStringParam jname(name);
		if (!JavaLibrary::convert(jname, theaterName))
			return JNI_FALSE;
		if (!theaterName.empty())
		{
			// make sure the theater doesn't exist
			const NetworkId & oldTheater = ServerUniverse::getInstance().findTheaterId(theaterName);
			if (oldTheater != NetworkId::cms_invalid)
			{
				WARNING(true, ("JavaLibrary::assignTheaterToPlayer asked to create "
					"theater with name %s which is already assigned to theater %s",
					theaterName.c_str(), oldTheater.getValueString().c_str()));
				return JNI_FALSE;
			}
		}
	}

	// read the object data from the datatable
	DataTable * dt = DataTableManager::getTable(datatableName, true);
	if (dt == nullptr)
	{
		WARNING(true, ("JavaLibrary::assignTheaterToPlayer could not open "
			"datatable %s", datatableName.c_str()));
		return JNI_FALSE;
	}
	int rows = dt->getNumRows();
	if (rows <= 0)
	{
		WARNING(true, ("JavaLibrary::assignTheaterToPlayer datatable %s has no "
			"rows", datatableName.c_str()));
		return JNI_FALSE;
	}

	int templateColumn = dt->findColumnNumber("template");
	int xColumn        = dt->findColumnNumber("x");
	int yColumn        = dt->findColumnNumber("y");
	int zColumn        = dt->findColumnNumber("z");
	int sceneColumn    = dt->findColumnNumber("scene");
	int scriptColumn   = dt->findColumnNumber("script");

	if (static_cast<uint32>(dt->getIntValue(templateColumn, 0)) == THEATER_DATATABLE_TAG)
		return JNI_FALSE;

	Vector position;
	std::string scene;
	std::string script;

	position.x = dt->getFloatValue(xColumn, 0);
	position.y = dt->getFloatValue(yColumn, 0);
	position.z = dt->getFloatValue(zColumn, 0);
	scene = dt->getStringValue(sceneColumn, 0);
	script = dt->getStringValue(scriptColumn, 0);

	return player->setTheater(datatableName, position, scene, script, callerId,
		theaterName, static_cast<IntangibleObject::TheaterLocationType>(locationType));
}	// JavaLibrary::assignTheaterToPlayer

// ----------------------------------------------------------------------

/**
 * Causes a theater to be created when a player gets near its location.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param playerId		the player who will cause the theater to be created
 * @param datatable		the datatable to read the theater data from
 * @param basePosition	the base position of the theater; all objects will be placed relative to here
 * @param script		optional script to be attached to the master theater object; the trigger OnTheaterCreated
 *                   	will be called when all the theater objects have been created
 * @param caller        who's creating the theater
 * @param name          the name of the theater
 * @param locationType	how to create the theater
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::assignTheaterToPlayerLocation(JNIEnv *env, jobject self, jlong playerId, jstring datatable, jobject basePosition, jstring script, jlong caller, jstring name, jint locationType)
{
	if (playerId == 0 || datatable == 0 || basePosition == 0)
		return JNI_FALSE;

	JavaStringParam jdatatable(datatable);
	JavaStringParam jscript(script);

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(playerId, playerCreature))
		return JNI_FALSE;

	PlayerObject * const player = PlayerCreatureController::getPlayerObject(
		playerCreature);
	if (player == nullptr)
		return JNI_FALSE;

	std::string datatableName;
	if (!JavaLibrary::convert(jdatatable, datatableName))
		return JNI_FALSE;

	Vector position;
	std::string scene;
	if (!ScriptConversion::convertWorld(basePosition, position, scene))
		return JNI_FALSE;

	std::string scriptName;
	if (script != 0)
	{
		if (!JavaLibrary::convert(jscript, scriptName))
			return JNI_FALSE;
	}

	NetworkId callerId(caller);

	std::string theaterName;
	if (name != 0)
	{
		JavaStringParam jname(name);
		if (!JavaLibrary::convert(jname, theaterName))
			return JNI_FALSE;
		if (!theaterName.empty())
		{
			// make sure the theater doesn't exist
			const NetworkId & oldTheater = ServerUniverse::getInstance().findTheaterId(theaterName);
			if (oldTheater != NetworkId::cms_invalid)
			{
				WARNING(true, ("JavaLibrary::assignTheaterToPlayerLocation asked "
					"to create theater with name %s which is already assigned "
					"to theater %s", theaterName.c_str(),
					oldTheater.getValueString().c_str()));
				return JNI_FALSE;
			}
		}
	}

	return player->setTheater(datatableName, position, scene, scriptName,
		callerId, theaterName, static_cast<IntangibleObject::TheaterLocationType>(locationType));
}	// JavaLibrary::assignTheaterToPlayerLocation

// ----------------------------------------------------------------------

/**
 * Causes any theater assigned to a player to be unassigned.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param playerId	 the player to remove the theater from
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::unassignTheaterFromPlayer(JNIEnv *env, jobject self, jlong playerId)
{
	if (playerId == 0)
		return JNI_FALSE;

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(playerId, playerCreature))
		return JNI_FALSE;

	PlayerObject * const player = PlayerCreatureController::getPlayerObject(
		playerCreature);
	if (player == nullptr)
		return JNI_FALSE;

	player->clearTheater();
	return JNI_TRUE;
}	// JavaLibrary::unassignTheaterFromPlayer

// ----------------------------------------------------------------------

/**
 * Checks if a player has a theater assigned to him.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param playerId	 the player to check
 *
 * @return true if the player is assigned a theater, false if not
 */
jboolean JNICALL ScriptMethodsObjectCreateNamespace::hasTheaterAssigned(JNIEnv *env, jobject self, jlong playerId)
{
	if (playerId == 0)
		return JNI_FALSE;

	const CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(playerId, playerCreature))
		return JNI_FALSE;

	const PlayerObject * player = PlayerCreatureController::getPlayerObject(
		playerCreature);
	if (player == nullptr)
		return JNI_FALSE;

	return player->hasTheater();
}	// JavaLibrary::hasTheaterAssigned

// ----------------------------------------------------------------------

/**
 * Returns the name of a theater.
 *
 * @param theater    the theater id
 *
 * @return the theater's name, or nullptr if it doesn't have one
 */
jstring JNICALL ScriptMethodsObjectCreateNamespace::getTheaterName(JNIEnv *env, jobject self, jlong theater)
{
	if (theater == 0)
		return 0;

	const NetworkId theaterId(theater);
	if (theaterId == NetworkId::cms_invalid)
		return 0;

	const std::string & theaterName = ServerUniverse::getInstance().findTheaterName(theaterId);
	if (theaterName.empty())
		return 0;

	return JavaString(theaterName).getReturnValue();
}	// JavaLibrary::getTheaterName

// ----------------------------------------------------------------------

/**
 * Finds a theater with a given name.
 *
 * @param name    the theater name to look for
 *
 * @return the theater's id, or nullptr if the theater doesn't exist
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::findTheater(JNIEnv *env, jobject self, jstring name)
{
	if (name == 0)
		return 0;

	JavaStringParam jname(name);
	std::string theaterName;
	if (!JavaLibrary::convert(jname, theaterName))
		return 0;

	const NetworkId & theater = ServerUniverse::getInstance().findTheaterId(theaterName);
	if (theater == NetworkId::cms_invalid)
		return 0;

	return (theater).getValue();
}	// JavaLibrary::findTheater

// ----------------------------------------------------------------------

/**
 * Creates a manufacturing schematic from a draft schematic template.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param draftSchematic	the draft schematic template to create the manufacturing schematic from
 * @param container			the container to put the manufacturing schematic in
 *
 * @return the id of the manufacturing schematic, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createSchematicString(JNIEnv *env, jobject self, jstring draftSchematic, jlong container)
{
	if (draftSchematic == 0 || container == 0)
		return 0;

	JavaStringParam jdraftSchematic(draftSchematic);
	std::string schematicName;
	if (!JavaLibrary::convert(jdraftSchematic, schematicName))
		return 0;

	return createSchematicCrc(env, self, Crc::calculate(schematicName.c_str()),
		container);
}	// JavaLibrary::createSchematicString

// ----------------------------------------------------------------------

/**
 * Creates a manufacturing schematic from a draft schematic template.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param draftSchematicCrc	the draft schematic template crc value to create the manufacturing schematic from
 * @param container			the container to put the manufacturing schematic in
 *
 * @return the id of the manufacturing schematic, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectCreateNamespace::createSchematicCrc(JNIEnv *env, jobject self, jint draftSchematicCrc, jlong container)
{
	if (draftSchematicCrc == 0 || container == 0)
		return 0;

	ServerObject * containerObject = nullptr;
	if (!JavaLibrary::getObject(container, containerObject))
		return 0;

	const DraftSchematicObject * const draftSchematic = DraftSchematicObject::getSchematic(
		draftSchematicCrc);
	if (draftSchematic == nullptr)
		return 0;

	ManufactureSchematicObject * manfSchematic = ServerWorld::createNewManufacturingSchematic(
		*draftSchematic, *containerObject, false);
	if (manfSchematic == nullptr)
		return 0;

	return (manfSchematic->getNetworkId()).getValue();
}	// JavaLibrary::createSchematicCrc

// ----------------------------------------------------------------------
/**
* Updates the Far Network Trigger volume for a given object. This causes the client to be notified about it earlier, causing it to load sooner.
*
* @param env				Java environment
* @param self				class calling this function
* @param target             Target whose trigger volume will be changed
* @param radius             Radius of the new trigger volume.
*
* @return true or false whether it completed properly or not.
*/

jboolean JNICALL ScriptMethodsObjectCreateNamespace::updateNetworkTriggerVolume(JNIEnv *env, jobject self, jlong target, jfloat radius)
{
	ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	if(object->getNetworkTriggerVolume() && object->isNetworkUpdateFarTriggerVolume(*object->getNetworkTriggerVolume()))
		object->destroyFarNetworkUpdateVolume();

	object->createFarNetworkUpdateVolume(radius);

	return JNI_TRUE;
}
// ======================================================================

