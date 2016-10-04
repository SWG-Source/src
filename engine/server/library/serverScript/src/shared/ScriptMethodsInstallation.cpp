//========================================================================
//
// ScriptMethodsInstallation.cpp - implements script methods dealing with general
// object info.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/HarvesterInstallationObject.h"
#include "swgSharedNetworkMessages/PermissionListCreateMessage.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsInstallationNamespace
// ======================================================================

namespace ScriptMethodsInstallationNamespace
{
	bool install();

	jint         JNICALL getInstalledExtractionRate(JNIEnv *env, jobject self, jlong installation);
	jboolean     JNICALL isHarvesterActive(JNIEnv *env, jobject self, jlong installation);
	jboolean     JNICALL isHarvesterEmpty(JNIEnv *env, jobject self, jlong installation);
	jint         JNICALL getMaxExtractionRate(JNIEnv *env, jobject self, jlong installation);
	jboolean     JNICALL setMaxExtractionRate(JNIEnv *env, jobject self, jlong installation, jint value);
	jint         JNICALL getCurrentExtractionRate(JNIEnv *env, jobject self, jlong installation);
	jboolean     JNICALL setCurrentExtractionRate(JNIEnv *env, jobject self, jlong installation, jint value);
	jint         JNICALL getMaxHopperAmount(JNIEnv *env, jobject self, jlong installation);
	jboolean     JNICALL setMaxHopperAmount(JNIEnv *env, jobject self, jlong installation, jint value);
	jboolean     JNICALL activate(JNIEnv *env, jobject self, jlong installation);
	jboolean     JNICALL deactivate(JNIEnv *env, jobject self, jlong installation);
	void         JNICALL displayStructurePermissionData(JNIEnv *env, jobject self, jlong player, jobjectArray currentMembers, jobjectArray nearbyPeople, jstring listName);
	jfloat       JNICALL getPowerValue(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setPowerValue(JNIEnv *env, jobject self, jlong target, float value);
	jboolean     JNICALL incrementPowerValue(JNIEnv *env, jobject self, jlong target, float value);
	jfloat       JNICALL getPowerRate(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setPowerRate(JNIEnv *env, jobject self, jlong target, float value);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsInstallationNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsInstallationNamespace::c)}
	JF("_getCurrentExtractionRate", "(J)I", getCurrentExtractionRate),
	JF("_setCurrentExtractionRate", "(JI)Z", setCurrentExtractionRate),
	JF("_getInstalledExtractionRate","(J)I",getInstalledExtractionRate),
	JF("_isHarvesterActive","(J)Z",isHarvesterActive),
	JF("_isHarvesterEmpty","(J)Z",isHarvesterEmpty),
	JF("_getMaxExtractionRate","(J)I",getMaxExtractionRate),
	JF("_setMaxExtractionRate","(JI)Z",setMaxExtractionRate),
	JF("_getMaxHopperAmount","(J)I",getMaxHopperAmount),
	JF("_setMaxHopperAmount","(JI)Z",setMaxHopperAmount),
	JF("_activate","(J)Z",activate),
	JF("_deactivate","(J)Z",deactivate),
	JF("_displayStructurePermissionData","(J[Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)V",displayStructurePermissionData),
	JF("_getPowerValue", "(J)F", getPowerValue),
	JF("_setPowerValue", "(JF)Z", setPowerValue),
	JF("_incrementPowerValue", "(JF)Z", incrementPowerValue),
	JF("_getPowerRate", "(J)F", getPowerRate),
	JF("_setPowerRate", "(JF)Z", setPowerRate),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//----------------------------------------------------------------------

jint JNICALL ScriptMethodsInstallationNamespace::getInstalledExtractionRate(JNIEnv *env, jobject self, jlong installation)
{
	UNREF(self);
	HarvesterInstallationObject *obj=0;
	if (!JavaLibrary::getObject(installation, obj))
		return -1;
	return static_cast<int>(obj->getInstalledExtractionRate()); //TODO: do we really want to round this for scripts?
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInstallationNamespace::isHarvesterActive(JNIEnv *env, jobject self, jlong installation)
{
	UNREF(self);
	InstallationObject *obj=0;
	if (!JavaLibrary::getObject(installation, obj))
		return JNI_FALSE;
	return obj->isActive();
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInstallationNamespace::isHarvesterEmpty(JNIEnv *env, jobject self, jlong installation)
{
	UNREF(self);
	Object * obj = 0;
	if (!JavaLibrary::getObject(installation, obj))
	{
		DEBUG_WARNING (true, ("JavaLibrary::isHarvesterEmpty: could not get object for given obj_id"));
		return JNI_FALSE;
	}
	if(!obj)
	{
		DEBUG_WARNING (true, ("JavaLibrary::isHarvesterEmpty: could not get an Object from the given obj_id"));
		return JNI_FALSE;
	}
	HarvesterInstallationObject* harv = dynamic_cast<HarvesterInstallationObject*>(obj);
	if(!harv)
	{
		DEBUG_WARNING (true, ("JavaLibrary::isHarvesterEmpty: obj_id %s does not point to a harvester", obj->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}
	return harv->isEmpty();
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsInstallationNamespace::getMaxExtractionRate(JNIEnv *env, jobject self, jlong installation)
{
	UNREF(self);
	HarvesterInstallationObject *obj=0;
	if (!JavaLibrary::getObject(installation, obj))
		return -1;
	return obj->getMaxExtractionRate();
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInstallationNamespace::setMaxExtractionRate(JNIEnv *env, jobject self, jlong installation, jint value)
{
	UNREF(self);
	HarvesterInstallationObject *obj=0;
	if (!JavaLibrary::getObject(installation, obj))
		return JNI_FALSE;
	obj->setMaxExtractionRate(value);
	return JNI_TRUE;
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsInstallationNamespace::getCurrentExtractionRate(JNIEnv *env, jobject self, jlong installation)
{
	UNREF(self);
	HarvesterInstallationObject *obj=0;
	if (!JavaLibrary::getObject(installation, obj))
		return -1;
	return static_cast<int>(obj->getCurrentExtractionRate());  //TODO: do we really want to round this for scripts?
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInstallationNamespace::setCurrentExtractionRate(JNIEnv *env, jobject self, jlong installation, jint value)
{
	UNREF(self);
	HarvesterInstallationObject *obj=0;
	if (!JavaLibrary::getObject(installation, obj))
		return JNI_FALSE;
	obj->setCurrentExtractionRate(static_cast<float>(value));
	return JNI_TRUE;
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsInstallationNamespace::getMaxHopperAmount(JNIEnv *env, jobject self, jlong installation)
{
	UNREF(self);
	HarvesterInstallationObject *obj=0;
	if (!JavaLibrary::getObject(installation, obj))
		return -1;
	return obj->getMaxHopperAmount();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInstallationNamespace::setMaxHopperAmount(JNIEnv *env, jobject self, jlong installation, jint value)
{
	UNREF(self);
	HarvesterInstallationObject *obj=0;
	if (!JavaLibrary::getObject(installation, obj))
		return JNI_FALSE;
	obj->setMaxHopperAmount(value);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInstallationNamespace::activate(JNIEnv *env, jobject self, jlong installation)
{
	UNREF(self);
	InstallationObject *obj=0;
	if (!JavaLibrary::getObject(installation, obj))
		return JNI_FALSE;
	obj->activate(NetworkId::cms_invalid);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsInstallationNamespace::deactivate(JNIEnv *env, jobject self, jlong installation)
{
	UNREF(self);
	InstallationObject *obj=0;
	if (!JavaLibrary::getObject(installation, obj))
		return JNI_FALSE;
	obj->deactivate();
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsInstallationNamespace::displayStructurePermissionData(JNIEnv *env, jobject self, jlong player, jobjectArray currentMembers, jobjectArray nearbyPeople, jstring listName)
{
	UNREF(env);
	UNREF(self);

	//get the player id from player
	CreatureObject* creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject))
		return;

	//convert the jstring[] to a Unicode::String vector
	Unicode::String tempMember;
	std::vector<Unicode::String> memberVector;
	jsize count = env->GetArrayLength(currentMembers);
	for (jsize i = 0; i < count; ++i)
	{
		JavaStringPtr m = getStringArrayElement(LocalObjectArrayRefParam(currentMembers), i);
		if (m != JavaString::cms_nullPtr)
		{
			if (JavaLibrary::convert(*m, tempMember))
			{
				memberVector.push_back(tempMember);
			}
		}
	}

	//convert the jstring[] to a Unicode::String vector
	Unicode::String tempPerson;
	std::vector<Unicode::String> nearbyVector;
	count = env->GetArrayLength(nearbyPeople);
	for (jsize j = 0; j < count; ++j)
	{
		JavaStringPtr person = getStringArrayElement(LocalObjectArrayRefParam(nearbyPeople), j);
		if (person != JavaString::cms_nullPtr)
		{
			if (JavaLibrary::convert(*person, tempPerson))
			{
				nearbyVector.push_back(tempPerson);
			}
		}
	}

	//convert the listName to unicode
	Unicode::String localListName;
	JavaString l(listName);
	JavaLibrary::convert(l, localListName);

	const Client * client = creatureObject->getClient();
	if (client)
	{
		PermissionListCreateMessage msg(memberVector, nearbyVector, localListName);
		client->send(msg,true);
	}
}

// ----------------------------------------------------------------------

/**
 * Returns an installation's power value.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of the installation
 *
 * @return the power value
 */
jfloat JNICALL ScriptMethodsInstallationNamespace::getPowerValue(JNIEnv *env, jobject self, jlong target)
{
	UNREF(env);
	UNREF(self);

	//get the player id from player
	const InstallationObject * installation = nullptr;
	if (!JavaLibrary::getObject(target, installation))
		return 0;

	return installation->getPower();
}	// JavaLibrary::getPowerValue

// ----------------------------------------------------------------------

/**
 * Sets an installation's power value.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of the installation
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsInstallationNamespace::setPowerValue(JNIEnv *env, jobject self, jlong target, float value)
{
	UNREF(env);
	UNREF(self);

	//get the player id from player
	InstallationObject * installation = nullptr;
	if (!JavaLibrary::getObject(target, installation))
		return JNI_FALSE;

	installation->setPower(value);
	return JNI_TRUE;
}	// JavaLibrary::setPowerValue

// ----------------------------------------------------------------------

/**
 * Changes an installation's power value.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of the installation
 * @param value			the amount to change the power by
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsInstallationNamespace::incrementPowerValue(JNIEnv *env, jobject self, jlong target, float value)
{
	UNREF(env);
	UNREF(self);

	//get the player id from player
	InstallationObject * installation = nullptr;
	if (!JavaLibrary::getObject(target, installation))
		return JNI_FALSE;

	installation->changePower(value);
	return JNI_TRUE;
}	// JavaLibrary::incrementPowerValue

// ----------------------------------------------------------------------

/**
 * Returns an installation's power consumption rate, in units/hour.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of the installation
 *
 * @return the power rate
 */
jfloat JNICALL ScriptMethodsInstallationNamespace::getPowerRate(JNIEnv *env, jobject self, jlong target)
{
	UNREF(env);
	UNREF(self);

	//get the player id from player
	const InstallationObject * installation = nullptr;
	if (!JavaLibrary::getObject(target, installation))
		return 0;

	return installation->getPowerRate();
}	// JavaLibrary::getPowerRate

// ----------------------------------------------------------------------

/**
 * Sets an installation's power consumption rate, in units/hour.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of the installation
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsInstallationNamespace::setPowerRate(JNIEnv *env, jobject self, jlong target, float value)
{
	UNREF(env);
	UNREF(self);

	//get the player id from player
	InstallationObject * installation = nullptr;
	if (!JavaLibrary::getObject(target, installation))
		return JNI_FALSE;

	installation->setPowerRate(value);
	return JNI_TRUE;
}	// JavaLibrary::setPowerRate


// ======================================================================
