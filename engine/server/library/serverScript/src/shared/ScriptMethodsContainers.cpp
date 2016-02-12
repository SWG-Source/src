//========================================================================
//
// ScriptMethodsContainers.cpp - implements script methods dealing with object
// creation/destruction.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/ContainerInterface.h"
#include "serverGame/GameServer.h"
#include "serverGame/NameManager.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TangibleObject.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/Container.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/VolumeContainer.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsContainersNamespace
// ======================================================================

namespace ScriptMethodsContainersNamespace
{
	bool install();

	jint         JNICALL getContainerType(JNIEnv *env, jobject self, jlong container);
	jlongArray   JNICALL getContents(JNIEnv *env, jobject self, jlong container);
	jlong        JNICALL getContainedBy(JNIEnv *env, jobject self, jlong item);
	jboolean     JNICALL contains(JNIEnv *env, jobject self, jlong container, jlong item);
	jlong        JNICALL getObjectInSlot(JNIEnv *env, jobject self, jlong container, jstring slot);
	jlong        JNICALL getFirstParentInWorld(JNIEnv *env, jobject self, jlong item);
	jlong        JNICALL getTopMostContainer(JNIEnv *env, jobject self, jlong item);
	jint         JNICALL getNumItemsIn(JNIEnv *env, jobject self, jlong container);
	void         JNICALL destroyContents(JNIEnv *env, jobject self, jlong container);
	jboolean     JNICALL putIn(JNIEnv *env, jobject self, jlong item, jlong container);
	jboolean     JNICALL putInPosition(JNIEnv *env, jobject self, jlong item, jlong container, jobject pos);
	jboolean     JNICALL putInAndSendError(JNIEnv *env, jobject self, jlong item, jlong container, jlong player);
	jboolean     JNICALL putInOverloaded(JNIEnv *env, jobject self, jlong item, jlong container);
	jboolean     JNICALL equip(JNIEnv *env, jobject self, jlong item, jlong container);
	jboolean     JNICALL equipIn(JNIEnv *env, jobject self, jlong item, jlong container, jstring slot);
	jboolean     JNICALL equipOverride(JNIEnv *env, jobject self, jlong item, jlong container);
	jint         JNICALL getVolume(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getFilledVolume(JNIEnv *env, jobject self, jlong container);
	jint         JNICALL getTotalVolume(JNIEnv *env, jobject self, jlong container);
	jint         JNICALL canPutIn(JNIEnv *env, jobject self, jlong item, jlong container);
	jint         JNICALL canPutInSlot(JNIEnv *env, jobject self, jlong item, jlong container, jstring slot);
	void         JNICALL sendContainerErrorToClient(JNIEnv *env, jobject self, jlong player, jint errorCode);
	void         JNICALL moveToOfflinePlayerInventoryAndUnload(JNIEnv *env, jobject self, jlong target, jlong player);
	void         JNICALL moveToOfflinePlayerBankAndUnload(JNIEnv *env, jobject self, jlong target, jlong player);
	void         JNICALL moveToOfflinePlayerDatapadAndUnload(JNIEnv *env, jobject self, jlong target, jlong player, jint maxDepth);
	jint         JNICALL moveContents(JNIEnv *env, jobject self, jlong containerA, jlong containerB);
	jint         JNICALL moveObjects(JNIEnv *env, jobject self, jlongArray targets, jlong container);
	jint         JNICALL getVolumeFree(JNIEnv *env, jobject self, jlong container);
	jboolean     JNICALL isInSecureTrade(JNIEnv *env, jobject self, jlong item);
	void         JNICALL fixLoadWith(JNIEnv *env, jobject self, jlong topmostObject, jlong startingLoadWith, jint maxDepth);
	jboolean     JNICALL addUserToAccessList(JNIEnv *env, jobject self, jlong object, jlong user);
	jboolean     JNICALL removeUserFromAccessList(JNIEnv * env, jobject self, jlong object, jlong user);
	jboolean     JNICALL clearUserAccessList(JNIEnv * env, jobject self, jlong object);
	jboolean     JNICALL addGuildToAccessList(JNIEnv *env, jobject self, jlong object, jint guild);
	jboolean     JNICALL removeGuildFromAccessList(JNIEnv * env, jobject self, jlong object, jint guild);
	jboolean     JNICALL clearGuildAccessList(JNIEnv * env, jobject self, jlong object);
	jlongArray   JNICALL getUserAccessList(JNIEnv * env, jobject self, jlong object);
	jintArray    JNICALL getGuildAccessList(JNIEnv * env, jobject self, jlong object);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsContainersNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsContainersNamespace::c)}
	JF("_getContainerType", "(J)I", getContainerType),
	JF("_getContents", "(J)[J", getContents),
	JF("_getContainedBy", "(J)J", getContainedBy),
	JF("_contains", "(JJ)Z", contains),
	JF("_getObjectInSlot", "(JLjava/lang/String;)J", getObjectInSlot),
	JF("_getFirstParentInWorld", "(J)J", getFirstParentInWorld),
	JF("_getTopMostContainer", "(J)J", getTopMostContainer),
	JF("_getNumItemsIn", "(J)I",getNumItemsIn),
	JF("_destroyContents", "(J)V", destroyContents),
	JF("_putIn", "(JJ)Z", putIn),
	JF("_putInPosition", "(JJLscript/location;)Z", putInPosition),
	JF("_putIn", "(JJJ)Z", putInAndSendError),
	JF("_putInOverloaded", "(JJ)Z", putInOverloaded),
	JF("_canPutIn", "(JJ)I",canPutIn),
	JF("_canPutInSlot", "(JJLjava/lang/String;)I",canPutInSlot),
	JF("_sendContainerErrorToClient", "(JI)V",sendContainerErrorToClient),
	JF("_equip", "(JJ)Z", equip),
	JF("_equip", "(JJLjava/lang/String;)Z", equipIn),
	JF("_equipOverride", "(JJ)Z", equipOverride),
	JF("_moveToOfflinePlayerInventoryAndUnload","(JJ)V", moveToOfflinePlayerInventoryAndUnload),
	JF("_moveToOfflinePlayerBankAndUnload","(JJ)V", moveToOfflinePlayerBankAndUnload),
	JF("_moveToOfflinePlayerDatapadAndUnload","(JJI)V", moveToOfflinePlayerDatapadAndUnload),
	JF("_moveContents", "(JJ)I",moveContents),
	JF("_moveObjects", "([JJ)I",moveObjects),
	JF("_getVolumeFree", "(J)I",getVolumeFree),
	JF("_getFilledVolume", "(J)I",getFilledVolume),
	JF("_getTotalVolume", "(J)I",getTotalVolume),
	JF("_isInSecureTrade", "(J)Z",isInSecureTrade),
	JF("_fixLoadWith","(JJI)V", fixLoadWith),
	JF("_addUserToAccessList", "(JJ)Z", addUserToAccessList),
	JF("_removeUserFromAccessList", "(JJ)Z", removeUserFromAccessList),
	JF("_clearUserAccessList", "(J)Z", clearUserAccessList),
	JF("_addGuildToAccessList", "(JI)Z", addGuildToAccessList),
	JF("_removeGuildFromAccessList", "(JI)Z", removeGuildFromAccessList),
	JF("_clearGuildAccessList", "(J)Z", clearGuildAccessList),
	JF("_getUserAccessList", "(J)[J", getUserAccessList),
	JF("_getGuildAccessList", "(J)[I", getGuildAccessList),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI Container methods
//========================================================================


jint JNICALL ScriptMethodsContainersNamespace::getContainerType(JNIEnv *env, jobject self, jlong container)
{
	//@todo use enum
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return 0;

	if (ContainerInterface::getSlottedContainer(*containerOwner))
		return 1;
	if (ContainerInterface::getVolumeContainer(*containerOwner))
		return 2;
	return 0;
}

//--------------------------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsContainersNamespace::getContents(JNIEnv *env, jobject self, jlong containerObj)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(containerObj, containerOwner))
		return 0;

	Container* container = ContainerInterface::getContainer(*containerOwner);

	if (!container)
		return 0;

	int numContents = container->getNumberOfItems();

	// create the objId array
	LocalLongArrayRefPtr idArray = createNewLongArray(numContents);
	if (idArray == LocalLongArrayRef::cms_nullPtr)
	{
		if (env->ExceptionCheck())
			env->ExceptionDescribe();
		return 0;
	}

	ContainerIterator iter = container->begin();
	for (int count = 0; iter != container->end(); ++iter, ++count)
	{
		if (*iter == NetworkId::cms_invalid)
		{
			WARNING_STRICT_FATAL(true, ("Iteration through a container should never return 0\n"));
			continue;
		}

		jlong jlongTmp = (*iter).getValue();
		setLongArrayRegion(*idArray, count, 1, &jlongTmp);
	}
	return idArray->getReturnValue();
}

//--------------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsContainersNamespace::getContainedBy(JNIEnv *env, jobject self, jlong item)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return 0;

	ServerObject * target = dynamic_cast<ServerObject*>(ContainerInterface::getContainedByObject(*itemObj));
	if (!target)
		return 0;

	return (target->getNetworkId()).getValue();
}

//--------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsContainersNamespace::contains(JNIEnv *env, jobject self, jlong container, jlong item)
{
	if (!container)
		return 0;
	NetworkId container_id(container);
	if (container_id.getValue() == 0)
		return JNI_FALSE;

	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return JNI_FALSE;

	ServerObject * target = dynamic_cast<ServerObject*>(ContainerInterface::getContainedByObject(*itemObj));
	if (!target)
		return JNI_FALSE;

	return (target->getNetworkId() == container_id);
}

//--------------------------------------------------------------------------------------
jint JNICALL ScriptMethodsContainersNamespace::canPutIn(JNIEnv *env, jobject self, jlong target, jlong jcontainer)
{
	const int unknownError = static_cast<int>(Container::CEC_Unknown);
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(jcontainer, containerOwner))
		return unknownError;

	ServerObject * item = nullptr;
	if (!JavaLibrary::getObject(target, item))
		return unknownError;

	Container* container = ContainerInterface::getContainer(*containerOwner);
	if (!container)
		return unknownError;

	Container::ContainerErrorCode tmp = Container::CEC_Success;
	IGNORE_RETURN (container->mayAdd(*item, tmp) );
	return static_cast<int>(tmp);
}

//--------------------------------------------------------------------------------------

jint JNICALL ScriptMethodsContainersNamespace::canPutInSlot(JNIEnv *env, jobject self, jlong target, jlong jcontainer, jstring slot)
{
	const int unknownError = static_cast<int>(Container::CEC_Unknown);
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(jcontainer, containerOwner))
		return unknownError;

	ServerObject * item = nullptr;
	if (!JavaLibrary::getObject(target, item))
		return unknownError;

	Container::ContainerErrorCode tmp = Container::CEC_Success;

	JavaStringParam localSlot(slot);
	std::string slotString;
	if (!JavaLibrary::convert(localSlot, slotString))
		return static_cast<int>(Container::CEC_NoSlot);


	SlotId slotId = SlotIdManager::findSlotId(CrcLowerString(slotString.c_str()));
	if (slotId == SlotId::invalid)
		return static_cast<int>(Container::CEC_NoSlot);

	IGNORE_RETURN(ContainerInterface::canTransferToSlot(*containerOwner, *item, slotId, nullptr, tmp));

	return static_cast<int>(tmp);
}

//--------------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsContainersNamespace::getObjectInSlot(JNIEnv *env, jobject self, jlong container, jstring slot)
{
	JavaStringParam localSlot(slot);

	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return 0;

	SlottedContainer * const slotContainer = ContainerInterface::getSlottedContainer(*containerOwner);

	if (!slotContainer)
		return 0;

	std::string localName;
	if (!JavaLibrary::convert(localSlot, localName))
		return 0;

	Container::ContainerErrorCode tmp = Container::CEC_Success;
	const SlotId slotName = SlotIdManager::findSlotId(CrcLowerString(localName.c_str()));
	return (slotContainer->getObjectInSlot(slotName, tmp)).getValue();
}

//--------------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsContainersNamespace::getFirstParentInWorld(JNIEnv *env, jobject self, jlong item)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return 0;

	ServerObject * target = dynamic_cast<ServerObject*>(ContainerInterface::getFirstParentInWorld(*itemObj));
	if (!target)
		return 0;

	return (target->getNetworkId()).getValue();
}

//--------------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsContainersNamespace::getTopMostContainer(JNIEnv *env, jobject self, jlong item)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return 0;

	ServerObject * target = dynamic_cast<ServerObject*>(ContainerInterface::getTopmostContainer(*itemObj));
	if (!target)
		return 0;

	return (target->getNetworkId()).getValue();
}

//--------------------------------------------------------------------------------------

jint JNICALL ScriptMethodsContainersNamespace::getNumItemsIn(JNIEnv *env, jobject self, jlong container)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return 0;

	Container* cont = ContainerInterface::getContainer(*containerOwner);
	if (!cont)
		return 0;

	return cont->getNumberOfItems();
}

//--------------------------------------------------------------------------------------

void JNICALL ScriptMethodsContainersNamespace::destroyContents(JNIEnv *env, jobject self, jlong container)
{
#if 0 //@todo implement
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return;

	//call an engine function to do this.
#endif
}

//--------------------------------------------------------------------------------------

jint JNICALL ScriptMethodsContainersNamespace::moveContents(JNIEnv *env, jobject self, jlong containerA, jlong containerB)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(containerA, containerOwner))
		return 0;

	ServerObject * targetContainer = nullptr;
	if (!JavaLibrary::getObject(containerB, targetContainer))
		return 0;

	Container * container = ContainerInterface::getContainer(*containerOwner);
	if (!container)
		return 0;

	int count = 0;
	ContainerIterator iter = container->begin();
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	for (; iter != container->end(); ++iter)
	{
		ServerObject* item = safe_cast<ServerObject*>((*iter).getObject());
		if (item && ContainerInterface::transferItemToGeneralContainer(*targetContainer, *item, nullptr, tmp))
		{
			++count;
		}
	}
	return count;
}

//--------------------------------------------------------------------------------------

jint JNICALL ScriptMethodsContainersNamespace::moveObjects(JNIEnv *env, jobject self, jlongArray targets, jlong container)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return 0;

	int count = 0;
	for (int i = 0; i < env->GetArrayLength(targets); ++i)
	{
		ServerObject * itemObj = nullptr;
		jlong jlongTmp;
		env->GetLongArrayRegion(targets, i, 1, &jlongTmp);
		if (JavaLibrary::getObject(jlongTmp, itemObj))
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			if (ContainerInterface::transferItemToGeneralContainer(*containerOwner, *itemObj, nullptr, tmp))
			{
				++count;
			}
		}
	}
	return count;
}

//--------------------------------------------------------------------------------------



jboolean JNICALL ScriptMethodsContainersNamespace::putIn(JNIEnv *env, jobject self, jlong item, jlong container)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return JNI_FALSE;

	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return JNI_FALSE;
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	return ContainerInterface::transferItemToGeneralContainer(*containerOwner, *itemObj, nullptr, tmp);

}

//--------------------------------------------------------------------------------------


jboolean JNICALL ScriptMethodsContainersNamespace::putInPosition(JNIEnv *env, jobject self, jlong item, jlong container, jobject pos)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return JNI_FALSE;

	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return JNI_FALSE;

	Vector newPos;
	NetworkId newCell;
	ScriptConversion::convert(pos, newPos, newCell);
	UNREF(newCell);

	Transform tr;
	tr.setPosition_p(newPos);

	Container::ContainerErrorCode tmp = Container::CEC_Success;
	return ContainerInterface::transferItemToCell(*containerOwner, *itemObj, tr, nullptr, tmp);
}

//--------------------------------------------------------------------------------------


jboolean JNICALL ScriptMethodsContainersNamespace::putInAndSendError(JNIEnv *env, jobject self, jlong item, jlong container, jlong player)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return JNI_FALSE;

	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return JNI_FALSE;


	Container::ContainerErrorCode tmp = Container::CEC_Success;
	bool retval = ContainerInterface::transferItemToGeneralContainer(*containerOwner, *itemObj, nullptr, tmp);
	if (!retval)
	{
		ServerObject * playerObj = nullptr;
		if (!JavaLibrary::getObject(player, playerObj))
			return JNI_FALSE;
		ContainerInterface::sendContainerMessageToClient(*playerObj, tmp);
	}
	return retval;
}

//--------------------------------------------------------------------------------------
jboolean JNICALL ScriptMethodsContainersNamespace::putInOverloaded(JNIEnv *env, jobject self, jlong item, jlong container)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return JNI_FALSE;

	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return JNI_FALSE;

	Container::ContainerErrorCode tmp = Container::CEC_Success;
	bool retval = false;
	if (!ContainerInterface::getVolumeContainer(*containerOwner))
	{
		DEBUG_WARNING(true, ("putInOverloaded only works on volume containers"));
		retval = false;
	}
	else
	{
		retval =  ContainerInterface::transferItemToVolumeContainer(*containerOwner, *itemObj, nullptr, tmp, true);
	}

	return retval;
}

//--------------------------------------------------------------------------------------
jboolean JNICALL ScriptMethodsContainersNamespace::equip(JNIEnv *env, jobject self, jlong item, jlong container)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return JNI_FALSE;

	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return JNI_FALSE;

	SlottedContainer * const test = ContainerInterface::getSlottedContainer(*containerOwner);
	if (!test)
		return JNI_FALSE;
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	return ContainerInterface::transferItemToGeneralContainer(*containerOwner, *itemObj, nullptr, tmp);

}

//--------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsContainersNamespace::equipIn(JNIEnv *env, jobject self, jlong item, jlong container, jstring slot)
{
	JavaStringParam localSlot(slot);

	//@todo better error checking for invalid slot name.  Do it above too.
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return JNI_FALSE;

	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return JNI_FALSE;

	SlottedContainer * const test = ContainerInterface::getSlottedContainer(*containerOwner);
	if (!test)
		return JNI_FALSE;

	std::string localName;
	if (!JavaLibrary::convert(localSlot, localName))
		return JNI_FALSE;

	const SlotId slotName = SlotIdManager::findSlotId(CrcLowerString(localName.c_str()));

	SlottedContainmentProperty * slotted = ContainerInterface::getSlottedContainmentProperty(*itemObj);
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	int arrangement = slotted ? slotted->getBestArrangementForSlot(slotName) : -1;

	return ContainerInterface::transferItemToSlottedContainer(*containerOwner, *itemObj, arrangement, nullptr, tmp);

}

//--------------------------------------------------------------------------------------

jboolean  JNICALL ScriptMethodsContainersNamespace::equipOverride(JNIEnv *env, jobject self, jlong item, jlong container)
{
	//This function will equip and object into the first valid arrangement, deleting objects if necessary.
	//Before it deletes things, it looks for a valid unoccupied arrangement.

	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
	{
		DEBUG_WARNING(true, ("JNI: EquipOverride param container could not be found"));
		return JNI_FALSE;
	}

	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
	{
		DEBUG_WARNING(true, ("JNI: EquipOverride param item could not be found"));
		return JNI_FALSE;
	}

	SlottedContainer * const slotContainer = ContainerInterface::getSlottedContainer(*containerOwner);
	if (!slotContainer)
	{
		DEBUG_WARNING(true, ("JNI: EquipOverride param container was not slotted"));
		return JNI_FALSE;
	}

	//First check for a valid unoccupied arrangement
	int arrangement = -1;
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	if (slotContainer->getFirstUnoccupiedArrangement(*itemObj, arrangement, tmp))
	{
		//Found one
		return ContainerInterface::transferItemToSlottedContainer(*containerOwner, *itemObj, arrangement, nullptr, tmp);
	}

	//Couldn't find an unoccupied arrangement, so it's time to delete objects to make room for this one

	//-- get list of valid arrangements
	std::vector<int> vtmp;
	slotContainer->getValidArrangements(*itemObj, vtmp, tmp, true, false);

	if (vtmp.empty())
	{
		//In this case there was no valid arrangement
		DEBUG_WARNING(true, ("JNI: EquipOverride item could not ever fit into this container"));
		return JNI_FALSE;
	}
	arrangement = vtmp.front();
	if (arrangement < 0)
	{
		DEBUG_WARNING(true, ("JNI: EquipOverride item system error"));
		return JNI_FALSE;
	}

	SlottedContainmentProperty * scp = ContainerInterface::getSlottedContainmentProperty(*itemObj);
	if (!scp)
	{
		DEBUG_WARNING(true, ("JNI: EquipOverride invalid item"));
		return JNI_FALSE;
	}

	const SlottedContainmentProperty::SlotArrangement & slotList = scp->getSlotArrangement(arrangement);

	for (SlottedContainmentProperty::SlotArrangement::const_iterator i = slotList.begin(); i != slotList.end(); ++i)
	{
		const CachedNetworkId & id = slotContainer->getObjectInSlot(*i, tmp);
		if (id != NetworkId::cms_invalid)
		{
			ServerObject* objectToDelete = safe_cast<ServerObject*>(id.getObject());
			if (objectToDelete)
				objectToDelete->permanentlyDestroy(DeleteReasons::Script);
		}
	}

	return ContainerInterface::transferItemToSlottedContainer(*containerOwner, *itemObj, arrangement, nullptr, tmp);

}

//--------------------------------------------------------------------------------------
jint JNICALL ScriptMethodsContainersNamespace::getFilledVolume(JNIEnv *env, jobject self, jlong container)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return 0;

	VolumeContainer* volContainer = ContainerInterface::getVolumeContainer(*containerOwner);
	if (!volContainer)
		return 0;

	return volContainer->getCurrentVolume();
}

//--------------------------------------------------------------------------------------

jint JNICALL ScriptMethodsContainersNamespace::getTotalVolume(JNIEnv *env, jobject self, jlong container)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return 0;

	VolumeContainer* volContainer = ContainerInterface::getVolumeContainer(*containerOwner);
	if (!volContainer)
		return 0;

	return volContainer->getTotalVolume();
}


//--------------------------------------------------------------------------------------

jint JNICALL ScriptMethodsContainersNamespace::getVolumeFree(JNIEnv *env, jobject self, jlong container)
{
	ServerObject * containerOwner = nullptr;
	if (!JavaLibrary::getObject(container, containerOwner))
		return 0;

	VolumeContainer* volContainer = ContainerInterface::getVolumeContainer(*containerOwner);
	if (!volContainer)
		return 0;

	return volContainer->getTotalVolume() - volContainer->getCurrentVolume();
}

//--------------------------------------------------------------------------------------

void JNICALL ScriptMethodsContainersNamespace::sendContainerErrorToClient(JNIEnv *env, jobject self, jlong player, jint errorCode)
{
	//If there is no error, do nothing.
	if (errorCode == 0)
		return;

	ServerObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		DEBUG_WARNING(true, ("Player object not found in sendContainerErrorToClient"));
		return;
	}


	if (errorCode < 0 || errorCode >= static_cast<int>(Container::CEC_Last))
	{
		DEBUG_WARNING(true, ("sendContainerErrorCodeToClient called with invalid error code"));
		return;
	}

	Container::ContainerErrorCode code = static_cast<Container::ContainerErrorCode>(errorCode);
	ContainerInterface::sendContainerMessageToClient(*playerObject, code);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsContainersNamespace::moveToOfflinePlayerInventoryAndUnload(JNIEnv *env, jobject self, jlong target, jlong player)
{
	ServerObject *object = 0;
	if (!JavaLibrary::getObject(target, object))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("moveToOfflinePlayerInventoryAndUnload - could not find target object %s", NetworkId(target).getValueString().c_str()));
	else
	{
		NetworkId playerId(player);
		if (!NameManager::getInstance().isPlayer(playerId))
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("moveToOfflinePlayerInventoryAndUnload - target %s was not a valid player", playerId.getValueString().c_str()));
		else
			object->moveToPlayerAndUnload(playerId);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsContainersNamespace::moveToOfflinePlayerBankAndUnload(JNIEnv *env, jobject self, jlong target, jlong player)
{
	ServerObject *object = 0;
	if (!JavaLibrary::getObject(target, object))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("moveToOfflinePlayerBankAndUnload - could not find target object %s", NetworkId(target).getValueString().c_str()));
	else
	{
		NetworkId playerId(player);
		if (!NameManager::getInstance().isPlayer(playerId))
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("moveToOfflinePlayerBankAndUnload - target %s was not a valid player", playerId.getValueString().c_str()));
		else
			object->moveToPlayerBankAndUnload(playerId);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsContainersNamespace::moveToOfflinePlayerDatapadAndUnload(JNIEnv *env, jobject self, jlong target, jlong player, jint maxDepth)
{
	ServerObject *object = 0;
	if (!JavaLibrary::getObject(target, object))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("moveToOfflinePlayerDatapadAndUnload - could not find target object %s", NetworkId(target).getValueString().c_str()));
	else
	{
		NetworkId playerId(player);
		if (!NameManager::getInstance().isPlayer(playerId))
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("moveToOfflinePlayerDatapadAndUnload - player %s was not a valid player", playerId.getValueString().c_str()));
		else
		{
				object->moveToPlayerDatapadAndUnload(playerId, maxDepth);
		}
	}
}


//--------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsContainersNamespace::isInSecureTrade(JNIEnv *env, jobject self, jlong item)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(item, itemObj))
		return JNI_FALSE;

	bool const secureTradeFlag = itemObj ? itemObj->isInSecureTrade() : false;

	return secureTradeFlag;
}

// ----------------------------------------------------------------------

/**
 * Fix the load_with for topmostObject and its contents by setting them to startingLoadWith
 */
void JNICALL ScriptMethodsContainersNamespace::fixLoadWith(JNIEnv *env, jobject self, jlong topmostObject, jlong startingLoadWith, jint maxDepth)
{
	NetworkId topmostObjectId(topmostObject);
	if (!topmostObjectId.isValid())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("fixLoadWith - topmostObject %s is not valid", topmostObjectId.getValueString().c_str()));
		return;
	}

	NetworkId startingLoadWithId(startingLoadWith);
	if (!startingLoadWithId.isValid())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("fixLoadWith - startingLoadWith %s is not valid", startingLoadWithId.getValueString().c_str()));
		return;
	}

	GenericValueTypeMessage<std::pair<std::pair<NetworkId, NetworkId>, int> > const FixLoadWithMessage(
		"FixLoadWith",
		std::make_pair(std::make_pair(topmostObjectId, startingLoadWithId), maxDepth));
	GameServer::getInstance().sendToDatabaseServer(FixLoadWithMessage);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsContainersNamespace::addUserToAccessList(JNIEnv *env, jobject self, jlong object, jlong user)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(object, itemObj))
		return JNI_FALSE;
	
	TangibleObject * tangibleObj = itemObj->asTangibleObject();
	if(!tangibleObj)
	{
		DEBUG_WARNING(true, ("AddUserToAccessList: Source Object must be a Tangible Object."));
		return JNI_FALSE;
	}

	NetworkId userNetworkId(user);
	
	tangibleObj->addUserToAccessList(userNetworkId);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsContainersNamespace::removeUserFromAccessList(JNIEnv * env, jobject self, jlong object, jlong user)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(object, itemObj))
		return JNI_FALSE;

	TangibleObject * tangibleObj = itemObj->asTangibleObject();
	if(!tangibleObj)
	{
		DEBUG_WARNING(true, ("RemoveUserFromAccessList: Source Object must be a Tangible Object."));
		return JNI_FALSE;
	}

	NetworkId userNetworkId(user);

	tangibleObj->removeUserFromAccessList(userNetworkId);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsContainersNamespace::clearUserAccessList(JNIEnv * env, jobject self, jlong object)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(object, itemObj))
		return JNI_FALSE;

	TangibleObject * tangibleObj = itemObj->asTangibleObject();
	if(!tangibleObj)
	{
		DEBUG_WARNING(true, ("ClearUserAccessList: Source Object must be a Tangible Object."));
		return JNI_FALSE;
	}

	tangibleObj->clearUserAccessList();

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsContainersNamespace::addGuildToAccessList(JNIEnv *env, jobject self, jlong object, jint guild)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(object, itemObj))
		return JNI_FALSE;

	TangibleObject * tangibleObj = itemObj->asTangibleObject();
	if(!tangibleObj)
	{
		DEBUG_WARNING(true, ("AddUserToAccessList: Source Object must be a Tangible Object."));
		return JNI_FALSE;
	}

	tangibleObj->addGuildToAccessList(guild);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsContainersNamespace::removeGuildFromAccessList(JNIEnv * env, jobject self, jlong object, jint guild)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(object, itemObj))
		return JNI_FALSE;

	TangibleObject * tangibleObj = itemObj->asTangibleObject();
	if(!tangibleObj)
	{
		DEBUG_WARNING(true, ("RemoveUserFromAccessList: Source Object must be a Tangible Object."));
		return JNI_FALSE;
	}

	tangibleObj->removeGuildFromAccessList(guild);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsContainersNamespace::clearGuildAccessList(JNIEnv * env, jobject self, jlong object)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(object, itemObj))
		return JNI_FALSE;

	TangibleObject * tangibleObj = itemObj->asTangibleObject();
	if(!tangibleObj)
	{
		DEBUG_WARNING(true, ("ClearUserAccessList: Source Object must be a Tangible Object."));
		return JNI_FALSE;
	}

	tangibleObj->clearGuildAccessList();

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsContainersNamespace::getUserAccessList(JNIEnv * env, jobject self, jlong object)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(object, itemObj))
		return JNI_FALSE;

	TangibleObject * tangibleObj = itemObj->asTangibleObject();
	if(!tangibleObj)
	{
		DEBUG_WARNING(true, ("getUserAccessList: Source Object must be a Tangible Object."));
		return JNI_FALSE;
	}

	std::vector<NetworkId> users;
	tangibleObj->getUserAccessList(users);

	// create the objId array
	LocalLongArrayRefPtr idArray = createNewLongArray(users.size());
	if (idArray == LocalLongArrayRef::cms_nullPtr)
	{
		if (env->ExceptionCheck())
			env->ExceptionDescribe();
		return 0;
	}

	std::vector<NetworkId>::iterator iter = users.begin();
	int count = 0;
	for (; iter != users.end(); ++iter)
	{
		jlong jlongTmp = (*iter).getValue();
		setLongArrayRegion(*idArray, count, 1, &jlongTmp);
		++count;
	}

	return idArray->getReturnValue();
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsContainersNamespace::getGuildAccessList(JNIEnv * env, jobject self, jlong object)
{
	ServerObject * itemObj = nullptr;
	if (!JavaLibrary::getObject(object, itemObj))
		return JNI_FALSE;

	TangibleObject * tangibleObj = itemObj->asTangibleObject();
	if(!tangibleObj)
	{
		DEBUG_WARNING(true, ("getUserAccessList: Source Object must be a Tangible Object."));
		return JNI_FALSE;
	}

	std::vector<int> guilds;
	tangibleObj->getGuildAccessList(guilds);

	LocalIntArrayRefPtr idArray = createNewIntArray(guilds.size());
	if (idArray == LocalIntArrayRef::cms_nullPtr)
	{
		if (env->ExceptionCheck())
			env->ExceptionDescribe();
		return 0;
	}

	std::vector<int>::iterator iter = guilds.begin();
	int count = 0;
	for (; iter != guilds.end(); ++iter)
	{
		jint jintTemp = (*iter);
		setIntArrayRegion(*idArray, count, 1, &jintTemp);
		++count;
	}

	return idArray->getReturnValue();
}

// ======================================================================
