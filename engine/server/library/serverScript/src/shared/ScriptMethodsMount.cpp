// ======================================================================
//
// ScriptMethodsMount.cpp
// Copyright 2003, Sony Online Entertainment, Inc.
// All rights reserved.
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerObjectTemplate.h"
#include "sharedGame/MountValidScaleRangeTable.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"

using namespace JNIWrappersNamespace;

#define LOCAL_LOG_CHANNEL  "mount-script-interface"


// ======================================================================
// ScriptMethodsMountNamespace
// ======================================================================

namespace ScriptMethodsMountNamespace
{
	bool install();

	jboolean     JNICALL getMountsEnabled(JNIEnv *env, jobject self);
	jboolean     JNICALL getMountsMultiSeaterEnabled(JNIEnv *env, jobject self);
	jboolean     JNICALL makePetMountable(JNIEnv *env, jobject self, jlong petId);
	void         JNICALL updateMountWearableVisuals(JNIEnv *env, jobject self, jlong mountId);
	jboolean     JNICALL mountCreature(JNIEnv *env, jobject self, jlong riderId, jlong mountId);
	void         JNICALL dismountCreature(JNIEnv *env, jobject self, jlong riderId);
	jlong        JNICALL getMountId(JNIEnv *env, jobject self, jlong riderId);
	jlong        JNICALL getRiderId(JNIEnv *env, jobject self, jlong mountId);
	jint         JNICALL couldPetBeMadeMountable(JNIEnv *env, jobject self, jlong petId);
	jint         JNICALL couldObjectTemplateBeMadeMountable(JNIEnv *env, jobject self, jstring serverObjectTemplateName, jfloat objectScale);
	void         JNICALL makeContainerStateInconsistentTestOnly(JNIEnv *env, jobject self, jlong containedObjectId);
	jboolean     JNICALL doesMountHaveRoom(JNIEnv * env, jobject self, jlong mountId);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsMountNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsMountNamespace::c)}
	JF("getMountsEnabled", "()Z", getMountsEnabled),
	JF("getMountsMultiSeaterEnabled", "()Z", getMountsMultiSeaterEnabled),
	JF("_makePetMountable", "(J)Z", makePetMountable),
	JF("_updateMountWearableVisuals", "(J)V", updateMountWearableVisuals),
	JF("_mountCreature", "(JJ)Z", mountCreature),
	JF("_dismountCreature", "(J)V", dismountCreature),
	JF("_getMountId", "(J)J", getMountId),
	JF("_getRiderId", "(J)J", getRiderId),
	JF("_couldPetBeMadeMountable", "(J)I", couldPetBeMadeMountable),
	JF("couldObjectTemplateBeMadeMountable", "(Ljava/lang/String;F)I", couldObjectTemplateBeMadeMountable),
	JF("_makeContainerStateInconsistentTestOnly", "(J)V", makeContainerStateInconsistentTestOnly),
	JF("_doesMountHaveRoom", "(J)Z", doesMountHaveRoom),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jboolean JNICALL ScriptMethodsMountNamespace::getMountsEnabled(JNIEnv *env, jobject self)
{
	UNREF(env);
	UNREF(self);
	return static_cast<jboolean>(ConfigServerGame::getMountsEnabled() ? JNI_TRUE : JNI_FALSE);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsMountNamespace::getMountsMultiSeaterEnabled(JNIEnv * /*env*/, jobject /*self*/)
{
	LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::getMountsMultiSeaterEnabled(): is a deprecated call."));

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsMountNamespace::makePetMountable(JNIEnv *env, jobject self, jlong petId)
{
	UNREF(self);

	//-- Ignore calls if mounts are disabled.
	if (!ConfigServerGame::getMountsEnabled())
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::makePetMountable(): ignoring call, [GameServer] mountsEnabled is false."));
		return JNI_FALSE;
	}

	//-- Get CreatureObject for pet.
	CreatureObject *const petObject = JavaLibrary::getCreatureThrow(env, petId, "makePetMountable(): error in petId arg");
	if (!petObject)
		return JNI_FALSE;

	//-- Check if creature can be converted to a mount.
	if (petObject->getMountabilityStatus() != static_cast<int>(MountValidScaleRangeTable::MS_creatureMountable))
	{
		char buffer[512];
		IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "makePetMountable(): creature id=[%s],object template=[%s] cannot be made mountable.", petObject->getNetworkId().getValueString().c_str(), petObject->getObjectTemplateName()));
		buffer[sizeof(buffer) - 1] = '\0';
		JavaLibrary::throwInternalScriptError(buffer);
		LOG(LOCAL_LOG_CHANNEL, (buffer));
		return JNI_FALSE;
	}

	//-- Check if creature is already mountable.
	if (petObject->isMountable())
	{
		char buffer[512];
		IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "makePetMountable(): creature id=[%s],object template=[%s] already is mountable.", petObject->getNetworkId().getValueString().c_str(), petObject->getObjectTemplateName()));
		buffer[sizeof(buffer) - 1] = '\0';
		JavaLibrary::throwInternalScriptError(buffer);
		LOG(LOCAL_LOG_CHANNEL, (buffer));
		return JNI_FALSE;
	}

	//-- Do it.
	petObject->makePetMountable();
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsMountNamespace::updateMountWearableVisuals(JNIEnv *env, jobject self, jlong mountId)
{
	UNREF(env);
	UNREF(self);
	UNREF(mountId);

	LOG(LOCAL_LOG_CHANNEL, ("updateMountWearableVisuals called."));
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsMountNamespace::mountCreature(JNIEnv *env, jobject self, jlong riderId, jlong mountId)
{
	UNREF(self);

	//-- Ignore calls if mounts are disabled.
	if (!ConfigServerGame::getMountsEnabled())
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::mountCreature(): ignoring call, [GameServer] mountsEnabled is false."));
		return JNI_FALSE;
	}

	//-- Get the rider object.
	CreatureObject *const riderObject = JavaLibrary::getCreatureThrow(env, riderId, "mountCreature(): error in riderId arg");
	if (!riderObject)
		return JNI_FALSE;

	//-- Get the mount object.
	CreatureObject *const mountObject = JavaLibrary::getCreatureThrow(env, mountId, "mountCreature(): error in mountId arg");
	if (!mountObject)
		return JNI_FALSE;

	//-- Ensure that the rider is not already riding a mount.
	if (riderObject->getState(States::RidingMount))
	{
		char buffer[512];

		IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "JavaLibrary::mountCreature(): rider id=[%s] is already riding a mount.", riderObject->getNetworkId().getValueString().c_str()));
		buffer[sizeof(buffer) - 1] = '\0';
		JavaLibrary::throwInternalScriptError(buffer);
		LOG(LOCAL_LOG_CHANNEL, (buffer));
		return JNI_FALSE;
	}

	//-- Check if mount is mountable.
	if (!mountObject->isMountable())
	{
		char buffer[512];
		IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "mountCreature(): creature id=[%s],object template=[%s] is not a mount.", mountObject->getNetworkId().getValueString().c_str(), mountObject->getObjectTemplateName()));
		buffer[sizeof(buffer) - 1] = '\0';
		JavaLibrary::throwInternalScriptError(buffer);
		LOG(LOCAL_LOG_CHANNEL, (buffer));
		return JNI_FALSE;
	}

	//-- Check if mount has room;
	if (mountObject->isMountableAndHasRoomForAnother())
	{
		//-- Have the rider mount the creature.
		bool const mountResult = riderObject->mountCreature(*mountObject);
		return static_cast<jboolean>(mountResult);
	}

	return JNI_FALSE;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsMountNamespace::dismountCreature(JNIEnv *env, jobject self, jlong riderId)
{
	UNREF(env);
	UNREF(self);
	UNREF(riderId);

	//-- Ignore calls if mounts are disabled.
	if (!ConfigServerGame::getMountsEnabled())
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::dismountCreature(): ignoring call, [GameServer] mountsEnabled is false."));
		return;
	}

	//-- Get the rider.
	CreatureObject *const riderObject = JavaLibrary::getCreatureThrow(env, riderId, "getMountId(): error in riderId arg");
	if (!riderObject)
		return;

	//-- Ensure the rider is on a mount.
	if (!riderObject->getState(States::RidingMount))
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::dismountCreature(): rider id=[%s] is not riding a mount.", riderObject->getNetworkId().getValueString().c_str()));
		return;
	}

	//-- Get the mount.
	CreatureObject *const mountObject = riderObject->getMountedCreature();
	if (!mountObject)
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::dismountCreature(): server id=[%d],rider id=[%s] has RidingMount state but CreatureObject::getMountedCreature() returns nullptr, skipping dismount.", GameServer::getInstance().getProcessId(), riderObject->getNetworkId().getValueString().c_str()));
		return;
	}

	//-- Detach the rider from the mount.
	IGNORE_RETURN(mountObject->detachRider(riderObject->getNetworkId()));
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsMountNamespace::getMountId(JNIEnv *env, jobject self, jlong riderId)
{
	UNREF(env);
	UNREF(self);

	//-- Ignore calls if mounts are disabled.
	if (!ConfigServerGame::getMountsEnabled())
		return 0;

	//-- Get the rider object.
	bool const throwIfNotOnServer = false;
	CreatureObject const *const riderObject = JavaLibrary::getCreatureThrow(env, riderId, "getMountId(): error in riderId arg", throwIfNotOnServer);
	if (!riderObject)
		return 0;

	CreatureObject const *const mountedCreature = riderObject->getMountedCreature();
	NetworkId const mountNetworkId = (mountedCreature ? mountedCreature->getNetworkId() : NetworkId::cms_invalid);

	return mountNetworkId.getValue();
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsMountNamespace::getRiderId(JNIEnv *env, jobject self, jlong mountId)
{
	UNREF(env);
	UNREF(self);

	//-- Ignore calls if mounts are disabled.
	if (!ConfigServerGame::getMountsEnabled())
		return 0;

	//-- Get the rider object.
	bool const throwIfNotOnServer = false;
	CreatureObject const *const mountObject = JavaLibrary::getCreatureThrow(env, mountId, "getRiderId(): error in mountId arg", throwIfNotOnServer);
	if (!mountObject)
		return 0;

	CreatureObject const *const riderObject = mountObject->getPrimaryMountingRider();
	NetworkId const riderNetworkId = (riderObject ? riderObject->getNetworkId() : NetworkId::cms_invalid);

	return riderNetworkId.getValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsMountNamespace::couldPetBeMadeMountable(JNIEnv *env, jobject self, jlong petId)
{
	UNREF(env);
	UNREF(self);
	UNREF(petId);

	//-- Ignore calls if mounts are disabled.
	if (!ConfigServerGame::getMountsEnabled())
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::couldPetBeMadeMountable(): ignoring call, [GameServer] mountsEnabled is false."));
		return static_cast<int>(MountValidScaleRangeTable::MS_speciesUnmountable);
	}

	//-- Get CreatureObject for pet.
	CreatureObject const *const petObject = JavaLibrary::getCreatureThrow(env, petId, "couldPetBeMadeMountable(): error in petId arg");
	if (!petObject)
		return static_cast<int>(MountValidScaleRangeTable::MS_speciesUnmountable);

	//-- Check if creature can be converted to a mount.
	return static_cast<jint>(petObject->getMountabilityStatus());
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsMountNamespace::couldObjectTemplateBeMadeMountable(JNIEnv *env, jobject self, jstring objectTemplateName, jfloat objectScale)
{
	UNREF(env);
	UNREF(self);

	//-- Ignore calls if mounts are disabled.
	if (!ConfigServerGame::getMountsEnabled())
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::couldObjectTemplateBeMadeMountable(): ignoring call, [GameServer] mountsEnabled is false."));
		return static_cast<int>(MountValidScaleRangeTable::MS_speciesUnmountable);
	}

	//-- Convert objectTemplateName from jstring to something else.
	JavaStringParam objectTemplateNameParam(objectTemplateName);
	std::string     objectTemplateNameNarrow;
	IGNORE_RETURN(JavaLibrary::convert(objectTemplateNameParam, objectTemplateNameNarrow));

	//-- Get the appearance name.
	//... first grab the server object template name.
	ObjectTemplate const *const objectTemplate = ObjectTemplateList::fetch(objectTemplateNameNarrow);
	if (!objectTemplate)
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::couldObjectTemplateBeMadeMountable(): object template [%s] could not be fetched.", objectTemplateNameNarrow.c_str()));
		return static_cast<int>(MountValidScaleRangeTable::MS_speciesUnmountable);
	}

	//... grab the shared object template name from it.
	ServerObjectTemplate const *const serverObjectTemplate = dynamic_cast<ServerObjectTemplate const*>(objectTemplate);
	if (!serverObjectTemplate)
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::couldObjectTemplateBeMadeMountable(): object template [%s] is not a ServerObjectTemplate.", objectTemplateNameNarrow.c_str()));
		objectTemplate->releaseReference();
		return static_cast<int>(MountValidScaleRangeTable::MS_speciesUnmountable);
	}

	std::string const &sharedObjectTemplateName = serverObjectTemplate->getSharedTemplate();

	//... get the shared object template.
	ObjectTemplate const *const sharedObjectTemplateBase = ObjectTemplateList::fetch(sharedObjectTemplateName);

	if (!sharedObjectTemplateBase)
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::couldObjectTemplateBeMadeMountable(): could not load shared object template [%s].", sharedObjectTemplateName.c_str()));
		objectTemplate->releaseReference();
		return static_cast<int>(MountValidScaleRangeTable::MS_speciesUnmountable);
	}

	SharedObjectTemplate const *const sharedObjectTemplate = sharedObjectTemplateBase->asSharedObjectTemplate();
	if (!sharedObjectTemplate)
	{
		LOG(LOCAL_LOG_CHANNEL, ("JavaLibrary::couldObjectTemplateBeMadeMountable(): shared object template [%s] is not derived from class SharedObjectTemplate.", sharedObjectTemplateName.c_str()));
		objectTemplate->releaseReference();
		sharedObjectTemplateBase->releaseReference();
		return static_cast<int>(MountValidScaleRangeTable::MS_speciesUnmountable);
	}

	//... get the appearance name.
	TemporaryCrcString  appearanceTemplateName(sharedObjectTemplate->getAppearanceFilename().c_str(), true);

	//-- Check if creature can be converted to a mount.
	int const saddleCapacity = 1;
	MountValidScaleRangeTable::MountabilityStatus const msResult = MountValidScaleRangeTable::doesCreatureSupportScaleAndSaddleCapacity(appearanceTemplateName, static_cast<float>(objectScale), saddleCapacity);

	//-- Release resources.
	objectTemplate->releaseReference();
	sharedObjectTemplateBase->releaseReference();

	//-- Return result.
	return static_cast<jint>(msResult);
}

// ----------------------------------------------------------------------

void JNICALL JNICALL ScriptMethodsMountNamespace::makeContainerStateInconsistentTestOnly(JNIEnv *env, jobject self, jlong containedObjectId)
{
	//-- Get the ServerObject from the object id.
	if (!containedObjectId)
	{
		LOG("script-bug", ("makeContainerStateInconsistentTestOnly(): containedObjectId is nullptr"));
		return;
	}

	NetworkId networkId(containedObjectId);
	if (networkId == NetworkId::cms_invalid)
	{
		LOG("script-bug", ("makeContainerStateInconsistentTestOnly(): containedObjectId is invalid"));
		return;
	}

	Object *const containedObject = NetworkIdManager::getObjectById(networkId);
	if (!containedObject)
	{
		LOG("script-bug", ("makeContainerStateInconsistentTestOnly(): containedObjectId [%s] is not on this server", networkId.getValueString().c_str()));
		return;
	}

	if (!containedObject->isAuthoritative())
	{
		LOG("script-bug", ("makeContainerStateInconsistentTestOnly(): containedObjectId [%s] is not authoritative on this server", networkId.getValueString().c_str()));
		return;
	}

	//-- Get the container.
	Object* containerObject = ContainerInterface::getContainedByObject(*containedObject);
	if (!containerObject)
	{
		LOG("script-bug", ("makeContainerStateInconsistentTestOnly(): containedObjectId [%s] is not contained by anything", networkId.getValueString().c_str()));
		return;
	}

	Container* container = ContainerInterface::getContainer(*containerObject);
	if (!container)
	{
		LOG("script-bug", ("makeContainerStateInconsistentTestOnly(): containerObject [%s] returned a nullptr container", containerObject->getNetworkId().getValueString().c_str()));
		return;
	}

	//-- Break the containment relationship.
	container->internalItemRemoved(*containedObject);
	LOG("script-bug", ("makeContainerStateInconsistentTestOnly(): broke containment relationship for containedObjectId [%s], container [%s] now has invalid information", networkId.getValueString().c_str(), containerObject->getNetworkId().getValueString().c_str()));
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsMountNamespace::doesMountHaveRoom(JNIEnv * env, jobject self, jlong mountId)
{
	//-- Ignore calls if mounts are disabled.
	if (!ConfigServerGame::getMountsEnabled())
	{
		return JNI_FALSE;
	}

	//-- Get the rider object.
	bool const throwIfNotOnServer = false;
	CreatureObject const * const mountObject = JavaLibrary::getCreatureThrow(env, mountId, "getRiderId(): error in mountId arg", throwIfNotOnServer);

	if (mountObject == 0)
	{
		return JNI_FALSE;
	}

	return static_cast<jboolean>(mountObject->isMountableAndHasRoomForAnother());
}

// ======================================================================
