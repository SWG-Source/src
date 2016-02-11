//========================================================================
//
// ScriptMethodsInteriors.cpp - implements script methods dealing with cells and interiors
//
// copyright 2002 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedMath/Ray3d.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsInteriorsNamespace
// ======================================================================

namespace ScriptMethodsInteriorsNamespace
{
	bool install();

	jlong        JNICALL createObjectInCell(JNIEnv *env, jobject self, jlong source, jlong building, jstring cellName, jobject pos);
	jlong        JNICALL createObjectInCellString(JNIEnv *env, jobject self, jobject source, jlong building, jstring cellName, jobject pos);
	jlong        JNICALL _createObjectInCellInternal(JNIEnv *env, jobject self, std::string & templateName, jlong building, jstring cellName, jobject pos);
	jlong        JNICALL createObjectInCellAnywhere(JNIEnv *env, jobject self, jlong original, jlong building, jstring cellName);
	jlong        JNICALL createObjectInCellAnywhereString(JNIEnv *env, jobject self, jobject original, jlong building, jstring cellName);
	jlong        JNICALL _createObjectInCellAnywhereInternal(JNIEnv *env, jobject self, std::string & templateName, jlong building, jstring cellName);
	jobjectArray JNICALL getCellNames(JNIEnv *env, jobject self, jlong target);
	jstring      JNICALL getCellName(JNIEnv *env, jobject self, jlong target);
	jlongArray   JNICALL getCellIds(JNIEnv *env, jobject self, jlong target);
	jlong        JNICALL getCellId(JNIEnv *env, jobject self, jlong building, jstring cellName);
	jobject      JNICALL getLocationForWorldCoordinate(JNIEnv *env, jobject self, jfloat x, jfloat y, jfloat z);
	jboolean     JNICALL hasCell(JNIEnv *env, jobject self, jlong building, jstring cellName);
	jobject      JNICALL getGoodLocationInterior(JNIEnv *env, jobject self, jlong building, jstring cellName);
	jobject      JNICALL getBuildingEjectLocation(JNIEnv *env, jobject self, jlong building);
	jboolean     JNICALL isValidInteriorLocation (JNIEnv * env, jobject self, jobject location);
	jlong        JNICALL moveHouseItemToPlayer(JNIEnv *env, jobject self, jlong building, jlong player, int index);
	jlong        JNICALL moveHouseItemToPlayer2(JNIEnv *env, jobject self, jlong building, jlong player, jlong item);
	void         JNICALL deleteAllHouseItems(JNIEnv *env, jobject self, jlong building, jlong player);
	jboolean     JNICALL areAllContentsLoaded(JNIEnv *env, jobject self, jlong building);
	void         JNICALL loadBuildingContents(JNIEnv *env, jobject self, jlong player, jlong building);
	jint         JNICALL getPobBaseItemLimit(JNIEnv *env, jobject self, jlong building);
	jboolean     JNICALL isAtPendingLoadRequestLimit(JNIEnv * env, jobject self);

	jstring      JNICALL getCellLabel(JNIEnv * env, jobject self, jlong target);
	jboolean     JNICALL setCellLabel(JNIEnv * env, jobject self, jlong target, jstring cellLabel);
	jboolean     JNICALL setCellLabelOffset(JNIEnv * env, jobject self, jlong target, jfloat x, jfloat y, jfloat z);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsInteriorsNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsInteriorsNamespace::c)}
	JF("_createObjectInCell", "(Ljava/lang/String;JLjava/lang/String;Lscript/location;)J", createObjectInCellString),
	JF("_createObjectInCell", "(JJLjava/lang/String;Lscript/location;)J", createObjectInCell),
	JF("_createObjectInCell", "(Ljava/lang/String;JLjava/lang/String;)J", createObjectInCellAnywhereString),
	JF("_createObjectInCell", "(JJLjava/lang/String;)J", createObjectInCellAnywhere),
	JF("_getCellNames", "(J)[Ljava/lang/String;", getCellNames),
	JF("_getCellName", "(J)Ljava/lang/String;", getCellName),
	JF("_getCellIds", "(J)[J", getCellIds),
	JF("_getCellId", "(JLjava/lang/String;)J", getCellId),
	JF("getLocationForWorldCoordinate", "(FFF)Lscript/location;", getLocationForWorldCoordinate),
	JF("_hasCell", "(JLjava/lang/String;)Z", hasCell),
	JF("_getGoodLocation", "(JLjava/lang/String;)Lscript/location;", getGoodLocationInterior),
	JF("_getBuildingEjectLocation", "(J)Lscript/location;", getBuildingEjectLocation),
	JF("isValidInteriorLocation", "(Lscript/location;)Z", isValidInteriorLocation),
	JF("_moveHouseItemToPlayer", "(JJI)J", moveHouseItemToPlayer),
	JF("_moveHouseItemToPlayer", "(JJJ)J", moveHouseItemToPlayer2),
	JF("_deleteAllHouseItems", "(JJ)V", deleteAllHouseItems),
	JF("_areAllContentsLoaded", "(J)Z", areAllContentsLoaded),
	JF("_loadBuildingContents", "(JJ)V", loadBuildingContents),
	JF("_getPobBaseItemLimit", "(J)I", getPobBaseItemLimit),
	JF("isAtPendingLoadRequestLimit", "()Z", isAtPendingLoadRequestLimit),
	JF("_getCellLabel", "(J)Ljava/lang/String;", getCellLabel),
	JF("_setCellLabel", "(JLjava/lang/String;)Z", setCellLabel),
	JF("_setCellLabelOffset", "(JFFF)Z", setCellLabelOffset),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI Container methods
//========================================================================

jlong JNICALL ScriptMethodsInteriorsNamespace::createObjectInCellString(JNIEnv *env, jobject self, jobject source, jlong building, jstring cellName, jobject pos)
{
	UNREF(self);
	NOT_NULL(env);

	if (source == 0)
		return 0;
	if (building == 0)
		return 0;

	std::string templateName;
	if (env->IsInstanceOf(source, JavaLibrary::getClsString()) == JNI_TRUE)
	{
		JavaStringParam localSource(static_cast<jstring>(source));
		JavaLibrary::convert(localSource, templateName);
		return _createObjectInCellInternal(env, self, templateName, building, cellName, pos);
	}
	else
		return 0;
}

jlong JNICALL ScriptMethodsInteriorsNamespace::createObjectInCell(JNIEnv *env, jobject self, jlong source, jlong building, jstring cellName, jobject pos)
{
	UNREF(self);
	NOT_NULL(env);

	if (source == 0)
		return 0;
	if (building == 0)
		return 0;

	std::string templateName;
	ServerObject* sourceObject = nullptr;
	if (!JavaLibrary::getObject(source, sourceObject))
		return 0;
	templateName = sourceObject->getTemplateName();

	return _createObjectInCellInternal(env, self, templateName, building, cellName, pos);
}

jlong JNICALL ScriptMethodsInteriorsNamespace::_createObjectInCellInternal(JNIEnv *env, jobject self, std::string & templateName, jlong building, jstring cellName, jobject pos)
{
	JavaStringParam localCellName(cellName);

	ServerObject * portallizedObject = nullptr;
	if (!JavaLibrary::getObject(building, portallizedObject))
		return 0;

	std::string cellNameString;
	JavaLibrary::convert(localCellName, cellNameString);

	PortalProperty* portalProp = portallizedObject->getPortalProperty();
	if (!portalProp)
		return 0;

	ServerObject* cellObject = (portalProp->getCell(cellNameString.c_str()))->getOwner().asServerObject();

	if (!cellObject)
		return 0;

	CellProperty * cellProperty = ContainerInterface::getCell(*cellObject);

	if (!cellProperty)
		return 0;

	// This code is a little iffy - The position object we get contains a
	// vector and a cell ID, but when we create the object we use the vector
	// and a different cell ID. It's possible that we would need to transform
	// newPos from the

	Vector newPos;
	NetworkId newCell;
	ScriptConversion::convert(pos, newPos, newCell);
	UNREF(newCell);

	Transform tr;
	tr.setPosition_p(newPos);
	ServerObject *newObject = ServerWorld::createNewObject(templateName, tr, cellObject, false);
	if (newObject == nullptr)
		return 0;

	//@todo do we need to snap to floor or something?
	FloorLocator dropLoc;
	Floor const * floor = cellProperty->getFloor();

	if (!floor)
		return 0;

	bool foundLoc = floor->findClosestLocation(newPos,dropLoc);
	if(foundLoc)
	{
		newObject->setPosition_p(dropLoc.getPosition_p());
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("Danger, could not find floor under location in createNewObjectInCell\n"));
	}

	// create an networkId to return
	NetworkId netId = (newObject->getNetworkId());
	if (netId == NetworkId::cms_invalid)
	{
		delete newObject;
		return 0;
	}
	return netId.getValue();
}
//--------------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsInteriorsNamespace::createObjectInCellAnywhere(JNIEnv *env, jobject self, jlong source, jlong building, jstring cellName)
{
	//@todo need to find a way to get a good location in a cell.  For now just use the coordinates of the cell?
	UNREF(self);
	NOT_NULL(env);

	if (source == 0)
		return 0;
	if (building == 0)
		return 0;

	std::string templateName;
	ServerObject* sourceObject = nullptr;
	if (!JavaLibrary::getObject(source, sourceObject))
		return 0;
	templateName = sourceObject->getTemplateName();

	return _createObjectInCellAnywhereInternal(env, self, templateName, building, cellName);
}

jlong JNICALL ScriptMethodsInteriorsNamespace::createObjectInCellAnywhereString(JNIEnv *env, jobject self, jobject source, jlong building, jstring cellName)
{
	//@todo need to find a way to get a good location in a cell.  For now just use the coordinates of the cell?
	UNREF(self);
	NOT_NULL(env);

	if (source == 0)
		return 0;
	if (building == 0)
		return 0;

	std::string templateName;
	if (env->IsInstanceOf(source, JavaLibrary::getClsString()) == JNI_TRUE)
	{
		JavaStringParam localSource(static_cast<jstring>(source));
		JavaLibrary::convert(localSource, templateName);
		return _createObjectInCellAnywhereInternal(env, self, templateName, building, cellName);
	}
	else
		return 0;
}

jlong JNICALL ScriptMethodsInteriorsNamespace::_createObjectInCellAnywhereInternal(JNIEnv *env, jobject self, std::string & templateName, jlong building, jstring cellName)
{
	//@todo need to find a way to get a good location in a cell.  For now just use the coordinates of the cell?
	JavaStringParam localCellName(cellName);

	ServerObject *portallizedObject = nullptr;
	if (!JavaLibrary::getObject(building, portallizedObject))
		return 0;

	std::string cellNameString;
	JavaLibrary::convert(localCellName, cellNameString);

	PortalProperty* portalProp = portallizedObject->getPortalProperty();
	if (!portalProp)
		return 0;

	CellProperty* cellProp = portalProp->getCell(cellNameString.c_str());
	if (!cellProp)
		return 0;

	ServerObject* cellObject = cellProp->getOwner().asServerObject();
	if (!cellObject)
		return 0;

	Vector createPosition;
	Floor const * floor = cellProp->getFloor();
	bool foundLoc = floor->getGoodLocation(0.5f,createPosition);
	if(!foundLoc)
		return 0;

	Transform tr;
	tr.setPosition_p(createPosition);
	ServerObject *newObject = ServerWorld::createNewObject(templateName, tr, cellObject, false);
	if (newObject == nullptr)
		return 0;

	// create an networkId to return
	NetworkId netId = (newObject->getNetworkId());
	if (netId == NetworkId::cms_invalid)
	{
		delete newObject;
		return 0;
	}
	return netId.getValue();
}

//--------------------------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsInteriorsNamespace::getCellNames(JNIEnv *env, jobject self, jlong target)
{
	ServerObject const *portallizedObject = nullptr;
	if (!JavaLibrary::getObject(target, portallizedObject))
		return 0;

	PortalProperty const *cellProp = portallizedObject->getPortalProperty();
	if (!cellProp)
		return 0;

	PortalProperty::CellNameList const &nameList = cellProp->getCellNames();
	if (nameList.empty())
		return 0;

	LocalObjectArrayRefPtr nameArray;
	if (!ScriptConversion::convert(nameList, nameArray))
		return 0;

	return nameArray->getReturnValue();
}

//--------------------------------------------------------------------------------------

jstring JNICALL ScriptMethodsInteriorsNamespace::getCellName(JNIEnv *env, jobject self, jlong target)
{
	CellObject const *cellObject = nullptr;
	if (!JavaLibrary::getObject(target, cellObject))
		return 0;

	// remember: cell object indexes start with 1, but the cell names are 0-based
	int cellIndex = cellObject->getCell();
	if (cellIndex <= 0)
	{
		// cell is uninitialized
		return 0;
	}

	ServerObject const * const portallizedObject = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*cellObject));
	if (portallizedObject == nullptr)
		return 0;

	PortalProperty const * const cellProp = portallizedObject->getPortalProperty();
	if (!cellProp)
		return 0;

	PortalProperty::CellNameList const &nameList = cellProp->getCellNames();
	if (nameList.empty())
		return 0;

	if (static_cast<size_t>(cellIndex) > nameList.size())
	{
		WARNING(true, ("JavaLibrary::getCellName cell %s has a cell index of %d"
			"(base-1), when its owner building %s has only %u cells(base-0)",
			cellObject->getNetworkId().getValueString().c_str(), cellIndex,
			portallizedObject->getNetworkId().getValueString().c_str(), nameList.size()));
		return 0;
	}

	JavaString cellName(nameList[cellIndex-1]);
	return cellName.getReturnValue();
}

//--------------------------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsInteriorsNamespace::getCellIds(JNIEnv *env, jobject self, jlong target)
{
	ServerObject const *portallizedObject = nullptr;
	if (!JavaLibrary::getObject(target, portallizedObject))
		return 0;

	PortalProperty const * const portalProp = portallizedObject->getPortalProperty();
	if (!portalProp)
		return 0;

	PortalProperty::CellNameList const &nameList = portalProp->getCellNames();
	if (nameList.empty())
		return 0;

	int count = nameList.size();
	std::vector<NetworkId> cellIds;
	cellIds.reserve(count);
	for (int i = 0; i < count; ++i)
	{
		ServerObject const * cellObject = nullptr;
		CellProperty const * const cellProp = portalProp->getCell(nameList[i]);
		if (cellProp != nullptr)
			cellObject = cellProp->getOwner().asServerObject();
		if (cellObject != nullptr)
		{
			cellIds.push_back(cellObject->getNetworkId());
		}
		else
			cellIds.push_back(NetworkId::cms_invalid);
	}

	LocalLongArrayRefPtr idArray;
	if (!ScriptConversion::convert(cellIds, idArray))
		return 0;

	return idArray->getReturnValue();
}

//--------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInteriorsNamespace::hasCell(JNIEnv *env, jobject self, jlong portallizedObject, jstring cellName)
{
	JavaStringParam localCellName(cellName);

	ServerObject const * serverObject = nullptr;
	if (!JavaLibrary::getObject(portallizedObject, serverObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::hasCell passed invalid object"));
		return 0;
	}

	PortalProperty const * const portalProp = serverObject->getPortalProperty();
	if (!portalProp)
	{
		DEBUG_WARNING(true, ("JavaLibrary::hasCell server object %s has no portal property", serverObject->getDebugInformation().c_str()));
		return 0;
	}

	std::string cellNameString;
	if (!JavaLibrary::convert(localCellName, cellNameString))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::hasCell error converting cell name to C string"));
		return 0;
	}

	bool found = false;

	CellProperty const * const cellProp = portalProp->getCell(cellNameString.c_str());
	if (cellProp && cellProp->getOwner().asServerObject())
		found = true;

	return static_cast<jboolean>(found);
}

//--------------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsInteriorsNamespace::getCellId(JNIEnv *env, jobject self, jlong portallizedObject, jstring cellName)
{
	JavaStringParam localCellName(cellName);

	ServerObject const * serverObject = nullptr;
	if (!JavaLibrary::getObject(portallizedObject, serverObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("[script bug] JavaLibrary::getCellId passed invalid object"));
		return 0;
	}

	PortalProperty const * const portalProp = serverObject->getPortalProperty();
	if (!portalProp)
	{
		//DEBUG_WARNING(true, ("[script bug] JavaLibrary::getCellId server object %s has no portal property", serverObject->getDebugInformation().c_str()));
		return 0;
	}

	std::string cellNameString;
	if (!JavaLibrary::convert(localCellName, cellNameString))
	{
		DEBUG_WARNING(true, ("[programmer bug] JavaLibrary::getCellId error converting cell name to C string"));
		return 0;
	}

	CellProperty const * const cellProp = portalProp->getCell(cellNameString.c_str());
	if (!cellProp)
	{
		//DEBUG_WARNING(true, ("[script bug] JavaLibrary::getCellId passed bad cell name %s for portallized object %s", cellNameString.c_str(), serverObject->getDebugInformation().c_str()));
		return 0;
	}

	ServerObject const * const cellObject = cellProp->getOwner().asServerObject();
	if (!cellObject)
	{
		DEBUG_WARNING(true, ("[programmer bug] JavaLibrary::getCellId error getting cell object from cell property"));
		return 0;
	}

	return (cellObject->getNetworkId()).getValue();
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsInteriorsNamespace::getLocationForWorldCoordinate(JNIEnv *env, jobject self, jfloat x, jfloat y, jfloat z)
{
	UNREF(self);
	NOT_NULL(env);

	Vector position_w(x, y, z);

	LocalRefPtr location;
	Vector position_l(position_w);
	NetworkId cellId;

	CellProperty const * cellProperty = ServerWorld::findCellAtLocation(position_w);
	if (cellProperty) 
	{
		Object const & cellObject = cellProperty->getOwner();

		position_l = cellObject.rotateTranslate_w2o(position_w);
		cellId = cellObject.getNetworkId();
		
		if (!ScriptConversion::convert(position_l, ServerWorld::getSceneId(), cellId, location))
		{
			DEBUG_WARNING(true, ("getLocationForWorldCoordinate failed creating location object"));
			return 0;
		}
	}

	return location->getReturnValue();

}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsInteriorsNamespace::getGoodLocationInterior(JNIEnv *env, jobject self, jlong building, jstring cellName)
{
	UNREF(self);
	NOT_NULL(env);

	JavaStringParam localCellName(cellName);

	if (building == 0)
		return 0;

	ServerObject const *portallizedObject = 0;
	if (!JavaLibrary::getObject(building, portallizedObject))
		return 0;

	std::string cellNameString;
	JavaLibrary::convert(localCellName, cellNameString);

	PortalProperty const *portalProp = portallizedObject->getPortalProperty();
	if (!portalProp)
		return 0;

	CellProperty const *cellProp = portalProp->getCell(cellNameString.c_str());
	if (!cellProp)
		return 0;

	ServerObject const *cellObject = cellProp->getOwner().asServerObject();
	if (!cellObject)
		return 0;

	Vector location;
	Floor const *floor = cellProp->getFloor();
	bool foundLoc = floor->getGoodLocation(0.5f,location);
	if (!foundLoc)
		return 0;

	LocalRefPtr goodLocation;
	if (!ScriptConversion::convert(location, cellObject->getSceneId(), cellObject->getNetworkId(), goodLocation))
		return 0;

	return goodLocation->getReturnValue();
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsInteriorsNamespace::getBuildingEjectLocation(JNIEnv *env, jobject self, jlong building)
{
	UNREF(self);
	NOT_NULL(env);

	if (building == 0)
	{
		DEBUG_WARNING(true, ("[designer bug] getBuildingEjectLocation was passed a nullptr building objid"));
		return 0;
	}

	ServerObject const *portallizedObject = 0;
	if (!JavaLibrary::getObject(building, portallizedObject))
	{
		DEBUG_WARNING(true, ("[designer bug] getBuildingEjectLocation was passed a non-ServerObject objid"));
		return 0;
	}

	PortalProperty const *portalProp = portallizedObject->getPortalProperty();
	if (!portalProp)
	{
		return 0;
	}

	PortalPropertyTemplate const &ppTemplate = portalProp->getPortalPropertyTemplate();

	Transform ejectionTransform(Transform::IF_none);
	ejectionTransform.multiply(portallizedObject->getTransform_o2w(), ppTemplate.getEjectionLocationTransform());

	Vector ejectionWorldPos = ejectionTransform.getPosition_p();

	CellProperty const *worldCell = CellProperty::getWorldCellProperty();
	if (!worldCell)
	{
		DEBUG_WARNING(true, ("getBuildingEjectLocation get a nullptr worldCell back from CellProperty::getWorldCellProperty()"));
		return 0;
	}

	NetworkId const &worldCellId = worldCell->getOwner().getNetworkId();

	LocalRefPtr goodLocation;
	if (!ScriptConversion::convert(ejectionWorldPos, portallizedObject->getSceneId(), worldCellId, goodLocation))
		return 0;

	return goodLocation->getReturnValue();
}

//--------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInteriorsNamespace::isValidInteriorLocation (JNIEnv * /*env*/, jobject /*self*/, jobject jobject_location)
{
	NetworkId cellNetworkId;
	Vector position_p;
	if (!ScriptConversion::convert (jobject_location, position_p, cellNetworkId))
	{
		DEBUG_WARNING (true, ("isValidInteriorLocation (): DB failed to convert parameter 1 (location) to cellNetworkId and position_w"));
		return JNI_FALSE;
	}

	Object const * const object = NetworkIdManager::getObjectById (cellNetworkId);
	if (!object)
	{
		DEBUG_WARNING (true, ("isValidInteriorLocation (): DB could not find cell with object id=%s", cellNetworkId.getValueString ().c_str ()));
		return JNI_FALSE;
	}

	CellObject const * const cellObject = dynamic_cast<CellObject const *> (object);
	if (!cellObject)
	{
		DEBUG_WARNING (true, ("isValidInteriorLocation (): DB object id=%s is not a cell", cellNetworkId.getValueString ().c_str ()));
		return JNI_FALSE;
	}

	CellProperty const * const cellProperty = cellObject->getCellProperty ();
	if (!cellProperty)
	{
		DEBUG_WARNING (true, ("isValidInteriorLocation (): PB could not find cellProperty for cell id=%s name=%s", cellNetworkId.getValueString ().c_str (), cellProperty->getCellName ()));
		return JNI_FALSE;
	}

	Floor const * const floor = cellProperty->getFloor ();
	if (!floor)
	{
		DEBUG_WARNING (true, ("isValidInteriorLocation (): DB could not find floor for cell id=%s name=%s", cellNetworkId.getValueString ().c_str (), cellProperty->getCellName ()));
		return JNI_FALSE;
	}

	FloorLocator result;
	Vector const direction_p = Vector::negativeUnitY;
	Ray3d ray_p (position_p + Vector::unitY, direction_p);
	if (floor->intersectClosest (ray_p, result))
		return JNI_TRUE;

	return JNI_FALSE;
}

//--------------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsInteriorsNamespace::moveHouseItemToPlayer(JNIEnv * /*env*/, jobject /*self*/, jlong building, jlong player, int index)
{
	if (building == 0)
	{
		DEBUG_WARNING(true, ("[designer bug] moveHouseItemToPlayer was passed a nullptr building objid"));
		return 0;
	}

	TangibleObject *pob = 0;
	if (!JavaLibrary::getObject(building, pob) || !pob->getPortalProperty())
	{
		DEBUG_WARNING(true, ("[designer bug] moveHouseItemToPlayer was passed a non-tangible-pob building objid"));
		return 0;
	}

	CreatureObject *creatureObject = 0;
	if (!JavaLibrary::getObject(player, creatureObject))
	{
		DEBUG_WARNING(true, ("[designer bug] moveHouseItemToPlayer was passed a non-CreatureObject player"));
		return 0;
	}
	
	return pob->movePobItemToPlayer(*creatureObject, index, false).getValue();
}

//--------------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsInteriorsNamespace::moveHouseItemToPlayer2(JNIEnv * /*env*/, jobject /*self*/, jlong building, jlong player, jlong item)
{
	if (building == 0)
	{
		DEBUG_WARNING(true, ("[designer bug] moveHouseItemToPlayer2 was passed a nullptr building objid"));
		return 0;
	}

	TangibleObject *pob = 0;
	if (!JavaLibrary::getObject(building, pob) || !pob->getPortalProperty())
	{
		DEBUG_WARNING(true, ("[designer bug] moveHouseItemToPlayer2 was passed a non-tangible-pob building objid"));
		return 0;
	}

	CreatureObject *creatureObject = 0;
	if (!JavaLibrary::getObject(player, creatureObject))
	{
		DEBUG_WARNING(true, ("[designer bug] moveHouseItemToPlayer2 was passed a non-CreatureObject player"));
		return 0;
	}

	return pob->movePobItemToPlayer(*creatureObject, NetworkId(static_cast<NetworkId::NetworkIdType>(item)), false).getValue();
}

//--------------------------------------------------------------------------------------

void JNICALL ScriptMethodsInteriorsNamespace::deleteAllHouseItems(JNIEnv * /*env*/, jobject /*self*/, jlong building, jlong player)

{
	if (building == 0)
	{
		DEBUG_WARNING(true, ("[designer bug] deleteAllHouseItems was passed a nullptr building objid"));
		return ;
	}

	TangibleObject *pob = 0;
	if (!JavaLibrary::getObject(building, pob) || !pob->getPortalProperty())
	{
		DEBUG_WARNING(true, ("[designer bug] deleteAllHouseItems was passed a non-tangible-pob building objid"));
		return;
	}

	CreatureObject const *creatureObject = 0;
	if (!JavaLibrary::getObject(player, creatureObject))
	{
		DEBUG_WARNING(true, ("[designer bug] deleteAllHouseItems was passed a non-CreatereObject player"));
		return;
	}
	pob->deletePobPersistedContents(creatureObject, DeleteReasons::Player);
}

//--------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInteriorsNamespace::areAllContentsLoaded(JNIEnv * /*env*/, jobject /*self*/, jlong building)
{
	if (building == 0)
	{
		DEBUG_WARNING(true, ("[designer bug] areAllContentsLoaded was passed a nullptr building objid"));
		return JNI_FALSE;
	}

	const BuildingObject * buildingObject = nullptr;
	if (!JavaLibrary::getObject(building, buildingObject))
	{
		DEBUG_WARNING(true, ("[designer bug] areAllContentsLoaded was passed a non-ServerObject objid"));
		return JNI_FALSE;
	}

	return static_cast<jboolean>(buildingObject->getContentsLoaded() ? JNI_TRUE : JNI_FALSE);
}

//--------------------------------------------------------------------------------------

void JNICALL ScriptMethodsInteriorsNamespace::loadBuildingContents(JNIEnv * /*env*/, jobject /*self*/, jlong player, jlong building)
{
	if (building == 0)
	{
		DEBUG_WARNING(true, ("[designer bug] loadBuildingContents was passed a nullptr building objid"));
		return;
	}

	BuildingObject * buildingObject = nullptr;
	if (!JavaLibrary::getObject(building, buildingObject))
	{
		DEBUG_WARNING(true, ("[designer bug] loadBuildingContents was passed a non-ServerObject objid"));
		return;
	}

	if (GameServer::isAtPendingLoadRequestLimit())
	{
		WARNING(true, ("loadBuildingContents attempted to load building contents while at the request limit"));
		return;
	}

	GameServer::addPendingLoadRequest(buildingObject->getNetworkId());

	// Check for loading objvar first
	buildingObject->loadContents();
}

//--------------------------------------------------------------------------------------

jint JNICALL ScriptMethodsInteriorsNamespace::getPobBaseItemLimit(JNIEnv * /*env*/, jobject /*self*/, jlong building)
{
	TangibleObject *pob = 0;
	if(!JavaLibrary::getObject(building, pob) || !pob->getPortalProperty())
	{
		DEBUG_WARNING(true, ("[designer bug] getPobBaseItemLimit() was passed a non-tangible-pob building objid"));
		return 0;
	}

	return pob->getPobBaseItemLimit();
}

//--------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInteriorsNamespace::isAtPendingLoadRequestLimit(JNIEnv * /*env*/, jobject /*self*/)
{
	return GameServer::isAtPendingLoadRequestLimit();
}

//--------------------------------------------------------------------------------------

jstring JNICALL ScriptMethodsInteriorsNamespace::getCellLabel(JNIEnv * env, jobject self, jlong target)
{
	CellObject const *cellObject = nullptr;
	if (!JavaLibrary::getObject(target, cellObject))
		return nullptr;

	JavaString cellLabel(cellObject->getCellLabel());
	return cellLabel.getReturnValue();
}

//--------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInteriorsNamespace::setCellLabel(JNIEnv * env, jobject self, jlong target, jstring cellLabel)
{
	JavaStringParam localCellLabel(cellLabel);

	Unicode::String cellLabelString;
	if (!JavaLibrary::convert(localCellLabel, cellLabelString))
		return JNI_FALSE;

	CellObject * cellObject = nullptr;
	if (!JavaLibrary::getObject(target, cellObject))
		return JNI_FALSE;

	cellObject->setCellLabel(cellLabelString);

	return JNI_TRUE;
}

//--------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInteriorsNamespace::setCellLabelOffset(JNIEnv * env, jobject self, jlong target, jfloat x, jfloat y, jfloat z)
{
	CellObject * cellObject = nullptr;
	if (!JavaLibrary::getObject(target, cellObject))
		return JNI_FALSE;

	Vector offset(x, y, z);

	cellObject->setLabelLocationOffset(offset);

	return JNI_TRUE;
}

//--------------------------------------------------------------------------------------
