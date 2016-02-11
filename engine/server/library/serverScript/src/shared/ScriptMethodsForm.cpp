// ======================================================================
//
// ScriptMethodsForm.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/FormManagerServer.h"
#include "serverGame/ServerObject.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsFormNamespace
// ======================================================================

namespace ScriptMethodsFormNamespace
{
	bool install();

	jboolean   JNICALL editFormData(JNIEnv * env, jobject self, jlong player, jlong objectToEdit, jobjectArray keys, jobjectArray values);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsFormNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsFormNamespace::c)}
	JF("_editFormData", "(JJ[Ljava/lang/String;[Ljava/lang/String;)Z", editFormData),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jboolean JNICALL ScriptMethodsFormNamespace::editFormData(JNIEnv * env, jobject self, jlong player, jlong jObjectToEdit, jobjectArray jKeys, jobjectArray jValues)
{
	UNREF(self);
	ServerObject * playerServerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerServerObject) || !playerServerObject)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("[script bug] can't get player in editFormData"));
		return JNI_FALSE;
	}

	CreatureObject * const playerCreatureObject = playerServerObject->asCreatureObject();;
	if (!playerCreatureObject)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("[script bug] player is not a CreatureObject in editFormData"));
		return JNI_FALSE;
	}

	ServerObject * objectToEditServerObject = nullptr;
	if (!JavaLibrary::getObject(jObjectToEdit, objectToEditServerObject) || !objectToEditServerObject)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("[script bug] objectToEdit is not a ServerObject in editFormData"));
		return JNI_FALSE;
	}

	int const keysCount   = env->GetArrayLength(jKeys);
	int const valuesCount = env->GetArrayLength(jValues);

	if(keysCount != valuesCount)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("[script bug] keys and values vectors are different sizes in editFormData"));
		return JNI_FALSE;
	}

	FormManager::UnpackedFormData dataMap;

	std::string key;
	std::string value;
	std::vector<std::string> values;
	for (int i = 0; i < keysCount; ++i)
	{
		JavaString jKey(static_cast<jstring>(env->GetObjectArrayElement(jKeys, i)));
		if (jKey.getValue() != 0)
		{
			if (!JavaLibrary::convert(jKey, key))
			{
				JAVA_THROW_SCRIPT_EXCEPTION(true, ("[script bug] can't convert key value in editFormData"));
				return JNI_FALSE;
			}
		}
		JavaString jValue(static_cast<jstring>(env->GetObjectArrayElement(jValues, i)));
		if (jValue.getValue() != 0)
		{
			if (!JavaLibrary::convert(jValue, value))
			{
				JAVA_THROW_SCRIPT_EXCEPTION(true, ("[script bug] can't convert value in editFormData"));
				return JNI_FALSE;
			}
		}
		values.clear();
		values.push_back(value);
		/*TODO handle lists*/
		dataMap[key] = values;
	}

	//override what we've been given with data from any appropriate objvar bindings

	//get matching form
	FormManager::Form const * const form = FormManagerServer::getFormForServerObjectTemplate(objectToEditServerObject->getTemplateName());
	if(!form)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("[script bug] objectToEdit does not have a form binding, cannot modify"));
		return JNI_FALSE;
	}


	//build a list of all the fields that DON'T have objvar bindings (since script will need to tell us about them in JavaLibrary::editFormData
	DynamicVariableList const & objvars = objectToEditServerObject->getObjVars();
	std::vector<FormManager::Field const *> const & fields = form->getOrderedFieldList();
	for(std::vector<FormManager::Field const *>::const_iterator i2 = fields.begin(); i2 != fields.end(); ++i2)
	{
		FormManager::Field const * const field = *i2;
		if(!field)
			continue;

		std::string const & objvarBinding = field->getObjvarBinding();
		if(!objvarBinding.empty())
		{
			//TODO handle non-strings
			objvars.getItem(objvarBinding, value);
		}
		values.clear();
		values.push_back(value);
		/*TODO handle lists*/
		dataMap[field->getName()] = values;
	}

	FormManagerServer::sendEditObjectDataToClient(playerCreatureObject->getNetworkId(),objectToEditServerObject->getNetworkId(), dataMap);
	return JNI_TRUE;
}

// ======================================================================

