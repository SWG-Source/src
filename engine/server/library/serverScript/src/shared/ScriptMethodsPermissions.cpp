// ======================================================================
//
// ScriptMethodsPermissions.cpp - implements script methods dealing with
//   cell and building permissions
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerObject.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/UpdateCellPermissionMessage.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsPermissionsNamespace
// ======================================================================

namespace ScriptMethodsPermissionsNamespace
{
	bool install();

	jobjectArray JNICALL permissionsGetBanned(JNIEnv *env, jobject self, jlong target);
	jobjectArray JNICALL permissionsGetAllowed(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL permissionsIsPublic(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL permissionsIsAllowed(JNIEnv *env, jobject self, jlong target, jlong who);
	void         JNICALL permissionsAddAllowed(JNIEnv *env, jobject self, jlong target, jstring name);
	void         JNICALL permissionsRemoveAllAllowed(JNIEnv *env, jobject self, jlong target);
	void         JNICALL permissionsRemoveAllowed(JNIEnv *env, jobject self, jlong target, jstring name);
	void         JNICALL permissionsAddBanned(JNIEnv *env, jobject self, jlong target, jstring name);
	void         JNICALL permissionsRemoveBanned(JNIEnv *env, jobject self, jlong target, jstring name);
	void         JNICALL permissionsRemoveAllBanned(JNIEnv *env, jobject self, jlong target);
	void         JNICALL permissionsMakePublic(JNIEnv *env, jobject self, jlong target);
	void         JNICALL permissionsMakePrivate(JNIEnv *env, jobject self, jlong target);
	void         JNICALL expelFromBuilding(JNIEnv *env, jobject self, jlong target);
	void         JNICALL sendDirtyCellPermissionsUpdateToClient(JNIEnv *env, jobject self, jlong cell, jlong player, jboolean isAllowed);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsPermissionsNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsPermissionsNamespace::c)}
	JF("_permissionsGetBanned", "(J)[Ljava/lang/String;", permissionsGetBanned),
	JF("_permissionsGetAllowed", "(J)[Ljava/lang/String;", permissionsGetAllowed),
	JF("_permissionsIsPublic", "(J)Z", permissionsIsPublic),
	JF("_permissionsIsAllowed", "(JJ)Z", permissionsIsAllowed),
	JF("_permissionsAddAllowed", "(JLjava/lang/String;)V", permissionsAddAllowed),
	JF("_permissionsRemoveAllowed", "(JLjava/lang/String;)V", permissionsRemoveAllowed),
	JF("_permissionsRemoveAllAllowed", "(J)V", permissionsRemoveAllAllowed),
	JF("_permissionsAddBanned", "(JLjava/lang/String;)V", permissionsAddBanned),
	JF("_permissionsRemoveBanned", "(JLjava/lang/String;)V", permissionsRemoveBanned),
	JF("_permissionsRemoveAllBanned", "(J)V", permissionsRemoveAllBanned),
	JF("_permissionsMakePublic", "(J)V", permissionsMakePublic),
	JF("_permissionsMakePrivate", "(J)V", permissionsMakePrivate),
	JF("_expelFromBuilding", "(J)V", expelFromBuilding),
	JF("_sendDirtyCellPermissionsUpdateToClient", "(JJZ)V", sendDirtyCellPermissionsUpdateToClient),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================
// class JavaLibrary JNI command table methods
// ======================================================================

/**
 * Get the banned list for a cell or building
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the cell or building to get permissions for
 *
 * @return the banned list
 */
jobjectArray JNICALL ScriptMethodsPermissionsNamespace::permissionsGetBanned(JNIEnv *env, jobject self, jlong target)
{
	ServerObject *serverObj = 0;
	if (!JavaLibrary::getObject(target, serverObj))
		return nullptr;

	LocalObjectArrayRefPtr strArray;
	BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
	if (buildingObj)
		if (ScriptConversion::convert(buildingObj->getBanned(), strArray))
			return strArray->getReturnValue();
	CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
	if (cellObj)
		if (ScriptConversion::convert(cellObj->getBanned(), strArray))
			return strArray->getReturnValue();
	return nullptr;
}

// ----------------------------------------------------------------------

/**
 * Get the allowed list for a cell or building
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the cell or building to get permissions for
 *
 * @return the allowed list
 */
jobjectArray JNICALL ScriptMethodsPermissionsNamespace::permissionsGetAllowed(JNIEnv *env, jobject self, jlong target)
{
	ServerObject *serverObj = 0;
	if (!JavaLibrary::getObject(target, serverObj))
		return nullptr;

	LocalObjectArrayRefPtr strArray;
	BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
	if (buildingObj)
		if (ScriptConversion::convert(buildingObj->getAllowed(), strArray))
			return strArray->getReturnValue();
	CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
	if (cellObj)
		if (ScriptConversion::convert(cellObj->getAllowed(), strArray))
			return strArray->getReturnValue();
	return nullptr;
}

// ----------------------------------------------------------------------

/**
 * Get whether a cell or building is public
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the cell or building to get permissions for
 *
 * @return true if public, false if not found or private
 */
jboolean JNICALL ScriptMethodsPermissionsNamespace::permissionsIsPublic(JNIEnv *env, jobject self, jlong target)
{
	ServerObject *serverObj = 0;
	if (!JavaLibrary::getObject(target, serverObj))
		return JNI_FALSE;

	bool isPublic = false;
	BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
	if (buildingObj)
		isPublic = buildingObj->getIsPublic();
	else
	{
		CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
		if (cellObj)
			isPublic = cellObj->getIsPublic();
	}

	if (isPublic)
		return JNI_TRUE;
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

/**
 * check if someone is allowed in a cell/building
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the cell or building to test permissions on
 * @param who      the person to test permissions for
 */
jboolean JNICALL ScriptMethodsPermissionsNamespace::permissionsIsAllowed(JNIEnv *env, jobject self, jlong target, jlong who)
{
	CreatureObject *person = 0;
	if (JavaLibrary::getObject(who, person))
	{
		NetworkId buildingId(target);
		if(buildingId == NetworkId::cms_invalid)
		{
			// target "building" is the world cell - permissions are always allowed

			return JNI_TRUE;
		}

		ServerObject *serverObj = 0;
		if (JavaLibrary::getObject(target, serverObj))
		{
			BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
			if (buildingObj)
			{
				if (buildingObj->isAllowed(*person))
					return JNI_TRUE;
			}
			else
			{
				CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
				if (cellObj && cellObj->isAllowed(*person))
					return JNI_TRUE;
			}
		}
	}
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

/**
 * add someone to a cell or building's allowed list
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the cell or building to set permissions for
 * @param name     name or id string to add to allowed list
 */
void JNICALL ScriptMethodsPermissionsNamespace::permissionsAddAllowed(JNIEnv *env, jobject self, jlong target, jstring name)
{
	JavaStringParam localName(name);
	std::string strName;
	if (!JavaLibrary::convert(localName, strName))
		return;

	ServerObject *serverObj = 0;
	if (JavaLibrary::getObject(target, serverObj))
	{
		BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
		if (buildingObj)
			buildingObj->addAllowed(strName);
		else
		{
			CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
			if (cellObj)
				cellObj->addAllowed(strName);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * remove someone from a cell or building's allowed list
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the cell or building to set permissions for
 * @param name     name or id string to remove from allowed list
 */
void JNICALL ScriptMethodsPermissionsNamespace::permissionsRemoveAllowed(JNIEnv *env, jobject self, jlong target, jstring name)
{
	JavaStringParam localName(name);
	std::string strName;
	if (!JavaLibrary::convert(localName, strName))
		return;

	ServerObject *serverObj = 0;
	if (JavaLibrary::getObject(target, serverObj))
	{
		BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
		if (buildingObj)
			buildingObj->removeAllowed(strName);
		else
		{
			CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
			if (cellObj)
				cellObj->removeAllowed(strName);
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPermissionsNamespace::permissionsRemoveAllAllowed(JNIEnv *env, jobject self, jlong target)
{
	ServerObject *serverObj = 0;
	if (JavaLibrary::getObject(target, serverObj))
	{
		BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
		if (buildingObj)
			buildingObj->removeAllAllowed();
		else
		{
			CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
			if (cellObj)
				cellObj->removeAllAllowed();
		}
	}
}

// ----------------------------------------------------------------------

/**
 * add someone to a cell or building's banned list
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the cell or building to set permissions for
 * @param name     name or id string to add to banned list
 */
void JNICALL ScriptMethodsPermissionsNamespace::permissionsAddBanned(JNIEnv *env, jobject self, jlong target, jstring name)
{
	JavaStringParam localName(name);
	std::string strName;
	if (!JavaLibrary::convert(localName, strName))
		return;

	ServerObject *serverObj = 0;
	if (JavaLibrary::getObject(target, serverObj))
	{
		BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
		if (buildingObj)
			buildingObj->addBanned(strName);
		else
		{
			CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
			if (cellObj)
				cellObj->addBanned(strName);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * remove someone from a cell or building's banned list
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the cell or building to set permissions for
 * @param name     name or id string to remove from banned list
 */
void JNICALL ScriptMethodsPermissionsNamespace::permissionsRemoveBanned(JNIEnv *env, jobject self, jlong target, jstring name)
{
	JavaStringParam localName(name);
	std::string strName;
	if (!JavaLibrary::convert(localName, strName))
		return;

	ServerObject *serverObj = 0;
	if (JavaLibrary::getObject(target, serverObj))
	{
		BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
		if (buildingObj)
			buildingObj->removeBanned(strName);
		else
		{
			CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
			if (cellObj)
				cellObj->removeBanned(strName);
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPermissionsNamespace::permissionsRemoveAllBanned(JNIEnv *env, jobject self, jlong target)
{
	ServerObject *serverObj = 0;
	if (JavaLibrary::getObject(target, serverObj))
	{
		BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
		if (buildingObj)
			buildingObj->removeAllBanned();
		else
		{
			CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
			if (cellObj)
				cellObj->removeAllBanned();
		}
	}
}

// ----------------------------------------------------------------------

/**
 * make a cell or building public
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the cell or building to set permissions for
 */
void JNICALL ScriptMethodsPermissionsNamespace::permissionsMakePublic(JNIEnv *env, jobject self, jlong target)
{
	ServerObject *serverObj = 0;
	if (JavaLibrary::getObject(target, serverObj))
	{
		BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
		if (buildingObj)
			buildingObj->setIsPublic(true);
		else
		{
			CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
			if (cellObj)
				cellObj->setIsPublic(true);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * make a cell or building private
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the cell or building to set permissions for
 */
void JNICALL ScriptMethodsPermissionsNamespace::permissionsMakePrivate(JNIEnv *env, jobject self, jlong target)
{
	ServerObject *serverObj = 0;
	if (JavaLibrary::getObject(target, serverObj))
	{
		BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(serverObj);
		if (buildingObj)
			buildingObj->setIsPublic(false);
		else
		{
			CellObject *cellObj = dynamic_cast<CellObject*>(serverObj);
			if (cellObj)
				cellObj->setIsPublic(false);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * expel an object from a building
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param target   the object to expel
 */
void JNICALL ScriptMethodsPermissionsNamespace::expelFromBuilding(JNIEnv *env, jobject self, jlong target)
{
	ServerObject *serverObj = 0;
	if (JavaLibrary::getObject(target, serverObj))
	{
		BuildingObject *buildingObj = dynamic_cast<BuildingObject*>(ContainerInterface::getTopmostContainer(*serverObj));
		if (buildingObj)
			buildingObj->expelObject(*serverObj);
	}
}

// ----------------------------------------------------------------------

/**
 * Sends a dirty cell permissions update message directly to a player's client to forcibly
 * update the client's cell permissions cache with regards to the cell in question as a means of
 * more quickly granting/revoking access to a cell (e.g. in Death Watch Bunker).
 *
 * @param cell the obj_id of the cell you are updating permissions for
 * @param player the obj_id of the player (and thus their client) to send the notification to
 * @param isAllowed true if they are being added to enter a cell, false if they are being removed/banned
 */
void JNICALL ScriptMethodsPermissionsNamespace::sendDirtyCellPermissionsUpdateToClient
(JNIEnv *env, jobject self, jlong cell, jlong player, jboolean isAllowed)
{

    PlayerObject *playerObject = nullptr;
    if (!JavaLibrary::getObject(player, playerObject))
    {
        WARNING(true, ("JavaLibrary::sendDirtyCellPermissionsUpdateToClient:  bad player object"));
        return;
    }

    ServerObject *serverObj = nullptr;
    if (!JavaLibrary::getObject(cell, serverObj)) {
        WARNING(true, ("JavaLibrary::sendDirtyCellPermissionsUpdateToClient:  bad cell object"));
    }

    auto *cellObj = dynamic_cast<CellObject*>(serverObj);
    UpdateCellPermissionMessage const message(cellObj->getNetworkId(), isAllowed);
    playerObject->getClient()->send(message, true);

}

// ======================================================================

