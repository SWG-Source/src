//========================================================================
//
// ScriptMethodsDynamicVariable.cpp - implements script methods dealing with objvars.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "UnicodeUtils.h"

using namespace JNIWrappersNamespace;


//========================================================================
// debugging non-authoritative objvar stuff
//
// we keep track of any objvars we set on a non-authoritative object in a frame,
// and warn/fatal if we try to read the objvar back in the same frame

namespace NonAuthObjvarNamespace
{
	unsigned long lastObjvarSetFrame = 0;
	std::map<NetworkId, std::set<std::string> > ObjvarsSet;

	void addSetObjvar(const ServerObject & obj, const char * name)
	{
		if (!ConfigServerGame::getTrackNonAuthoritativeObjvarSets())
			return;

		if (name == nullptr || *name == '\0')
			return;
		if (obj.isAuthoritative())
			return;

		unsigned long currentFrame = ServerClock::getInstance().getServerFrame();
		if (currentFrame > lastObjvarSetFrame)
		{
			// new frame, reset our data
			lastObjvarSetFrame = currentFrame;
			ObjvarsSet.clear();
		}
		ObjvarsSet[obj.getNetworkId()].insert(name);
	}

	void testIsSafeToReadObjvar(const ServerObject & obj, const std::string & name)
	{
		if (!ConfigServerGame::getTrackNonAuthoritativeObjvarSets())
			return;

		// we can read if the object is authoritative
		if (obj.isAuthoritative())
			return;

		// we can read if we're on a different frame than the last time an objvar
		// set
		unsigned long currentFrame = ServerClock::getInstance().getServerFrame();
		if (currentFrame > lastObjvarSetFrame)
			return;

		// we can read if we haven't set the objvar
		std::map<NetworkId, std::set<std::string> >::const_iterator found = ObjvarsSet.find(
			obj.getNetworkId());
		if (found == ObjvarsSet.end())
			return;

		if ((*found).second.find(name) == (*found).second.end())
			return;

		// !!! we are trying to read an objvar on a non-authoritative object we
		// have set this frame !!!
		fprintf(stderr, "Trying to read objvar %s on non-authoritative "
			"object %s that we have set this frame!\n", name.c_str(),
			obj.getNetworkId().getValueString().c_str());
		JavaLibrary::printJavaStack();
		WARNING_STRICT_FATAL(true, ("Trying to read objvar %s on non-authoritative object %s that we have set this frame!",
			name.c_str(), obj.getNetworkId().getValueString().c_str()));
	}
}
using namespace NonAuthObjvarNamespace;


// ======================================================================
// ScriptMethodsDynamicVariableNamespace
// ======================================================================

namespace ScriptMethodsDynamicVariableNamespace
{
	bool install();
	LocalRefPtr                 convertDynamicVariableListToObject(JNIEnv *env, const DynamicVariableListNestedList &list);
	ServerObject *              getObjectAndName(JNIEnv *env, jlong objId, jstring name, char * buffer, size_t bufferSize);
	const DynamicVariableList * getObjvarsAndName(JNIEnv *env, jlong objId, jstring name, char * buffer, size_t bufferSize);

	jobject      JNICALL getDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jint         JNICALL getIntDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jintArray    JNICALL getIntArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jfloat       JNICALL getFloatDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jfloatArray  JNICALL getFloatArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jstring      JNICALL getStringDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jobjectArray JNICALL getStringArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jlong        JNICALL getObjIdDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jlongArray   JNICALL getObjIdArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jobject      JNICALL getLocationDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jobjectArray JNICALL getLocationArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jobject      JNICALL getStringIdDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jobjectArray JNICALL getStringIdArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jobject      JNICALL getTransformDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jobjectArray JNICALL getTransformArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jobject      JNICALL getVectorDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jobjectArray JNICALL getVectorArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jobject      JNICALL getDynamicVariableList(JNIEnv *env, jobject self, jlong objId, jstring name);
	void         JNICALL removeDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	void         JNICALL removeAllDynamicVariables(JNIEnv *env, jobject self, jlong objId);
	jboolean     JNICALL hasDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name);
	jboolean     JNICALL setDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobject data);
	jboolean     JNICALL setIntDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jint data);
	jboolean     JNICALL setIntArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jintArray data);
	jboolean     JNICALL setFloatDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jfloat data);
	jboolean     JNICALL setFloatArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jfloatArray data);
	jboolean     JNICALL setStringDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jstring data);
	jboolean     JNICALL setStringArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobjectArray data);
	jboolean     JNICALL setObjIdDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jlong data);
	jboolean     JNICALL setObjIdArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jlongArray data);
	jboolean     JNICALL setLocationDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobject data);
	jboolean     JNICALL setLocationArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobjectArray data);
	jboolean     JNICALL setStringIdDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobject data);
	jboolean     JNICALL setStringIdArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobjectArray data);
	jboolean     JNICALL setTransformDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobject data);
	jboolean     JNICALL setTransformArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobjectArray data);
	jboolean     JNICALL setVectorDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobject data);
	jboolean     JNICALL setVectorArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobjectArray data);
	jboolean     JNICALL setDynamicVariableList(JNIEnv *env, jobject self, jlong objId, jstring name);
	jboolean     JNICALL copyDynamicVariable(JNIEnv *env, jobject self, jlong fromId, jlong toId, jstring name);
	jstring      JNICALL getPackedObjvars(JNIEnv *env, jobject self, jlong source, jstring jprefix);
	void         JNICALL setPackedObjvars(JNIEnv *env, jobject self, jlong target, jstring packedVars);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsDynamicVariableNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsDynamicVariableNamespace::c)}
	JF("_getObjVar", "(JLjava/lang/String;)Lscript/obj_var;", getDynamicVariable),
	JF("_getIntObjVar", "(JLjava/lang/String;)I", getIntDynamicVariable),
	JF("_getIntArrayObjVar", "(JLjava/lang/String;)[I", getIntArrayDynamicVariable),
	JF("_getFloatObjVar", "(JLjava/lang/String;)F", getFloatDynamicVariable),
	JF("_getFloatArrayObjVar", "(JLjava/lang/String;)[F", getFloatArrayDynamicVariable),
	JF("_getStringObjVar", "(JLjava/lang/String;)Ljava/lang/String;", getStringDynamicVariable),
	JF("_getStringArrayObjVar", "(JLjava/lang/String;)[Ljava/lang/String;", getStringArrayDynamicVariable),
	JF("_getObjIdObjVar", "(JLjava/lang/String;)J", getObjIdDynamicVariable),
	JF("_getObjIdArrayObjVar", "(JLjava/lang/String;)[J", getObjIdArrayDynamicVariable),
	JF("_getLocationObjVar", "(JLjava/lang/String;)Lscript/location;", getLocationDynamicVariable),
	JF("_getLocationArrayObjVar", "(JLjava/lang/String;)[Lscript/location;", getLocationArrayDynamicVariable),
	JF("_getStringIdObjVar", "(JLjava/lang/String;)Lscript/string_id;", getStringIdDynamicVariable),
	JF("_getStringIdArrayObjVar", "(JLjava/lang/String;)[Lscript/string_id;", getStringIdArrayDynamicVariable),
	JF("_getTransformObjVar", "(JLjava/lang/String;)Lscript/transform;", getTransformDynamicVariable),
	JF("_getTransformArrayObjVar", "(JLjava/lang/String;)[Lscript/transform;", getTransformArrayDynamicVariable),
	JF("_getVectorObjVar", "(JLjava/lang/String;)Lscript/vector;", getVectorDynamicVariable),
	JF("_getVectorArrayObjVar", "(JLjava/lang/String;)[Lscript/vector;", getVectorArrayDynamicVariable),
	JF("_getObjVarList", "(JLjava/lang/String;)Lscript/obj_var_list;", getDynamicVariableList),
	JF("_removeObjVar", "(JLjava/lang/String;)V", removeDynamicVariable),
	JF("_removeAllObjVars", "(J)V", removeAllDynamicVariables),
	JF("_hasObjVar", "(JLjava/lang/String;)Z", hasDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;Lscript/obj_var;)Z", setDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;I)Z", setIntDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;[I)Z", setIntArrayDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;F)Z", setFloatDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;[F)Z", setFloatArrayDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;Ljava/lang/String;)Z", setStringDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;[Ljava/lang/String;)Z", setStringArrayDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;J)Z", setObjIdDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;[J)Z", setObjIdArrayDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;Lscript/location;)Z", setLocationDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;[Lscript/location;)Z", setLocationArrayDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;Lscript/string_id;)Z", setStringIdDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;[Lscript/string_id;)Z", setStringIdArrayDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;Lscript/transform;)Z", setTransformDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;[Lscript/transform;)Z", setTransformArrayDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;Lscript/vector;)Z", setVectorDynamicVariable),
	JF("_setObjVar", "(JLjava/lang/String;[Lscript/vector;)Z", setVectorArrayDynamicVariable),
	JF("_setObjVarList", "(JLjava/lang/String;)Z", setDynamicVariableList),
	JF("_copyObjVar", "(JJLjava/lang/String;)Z", copyDynamicVariable),
	JF("_getPackedObjvars", "(JLjava/lang/String;)Ljava/lang/String;", getPackedObjvars),
	JF("_setPackedObjvars", "(JLjava/lang/String;)V", setPackedObjvars),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary objVar functions
//========================================================================

// ----------------------------------------------------------------------

/**
 * Converts an DynamicVariableList to a jobject.
 *
 * @param env		Java environment
 * @param list		the list to convert
 *
 * @return the converted object
 */
LocalRefPtr ScriptMethodsDynamicVariableNamespace::convertDynamicVariableListToObject(JNIEnv *env, const DynamicVariableList::NestedList &list)
{
	int count = list.getCount();

	JavaString listName(list.getName().c_str());

	LocalRefPtr newDynamicVariableList = createNewObject(JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableList(),
		listName.getValue(), count);
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		newDynamicVariableList = LocalRef::cms_nullPtr;
	}
	if (newDynamicVariableList == LocalRef::cms_nullPtr)
		return LocalRef::cms_nullPtr;

	jboolean result;
	for (DynamicVariableList::NestedList::const_iterator objvar(list.begin()); objvar != list.end(); ++objvar)
	{
		result = JNI_FALSE;
		switch (objvar.getType())
		{
			case DynamicVariable::INT:
				{
					int value;
					objvar.getValue(value);
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList,
						JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetInt(),
						name.getValue(), value);
				}
				break;
			case DynamicVariable::INT_ARRAY:
				{
					std::vector<int> value;
					objvar.getValue(value);
					LocalIntArrayRefPtr valueArray = createNewIntArray(value.size());
					if (value.size() > 0)
						setIntArrayRegion(*valueArray, 0, value.size(), reinterpret_cast<jint *>(const_cast<int *>(&value[0])));
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList,
						JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetIntArray(),
						name.getValue(), valueArray->getValue());
				}
				break;
			case DynamicVariable::REAL:
				{
					real value = 0;
					objvar.getValue(value);
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList,
						JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetFloat(),
						name.getValue(), value);
				}
				break;
			case DynamicVariable::REAL_ARRAY:
				{
					std::vector<real> value;
					objvar.getValue(value);
					LocalFloatArrayRefPtr valueArray = createNewFloatArray(value.size());
					if (value.size() > 0 && valueArray != LocalFloatArrayRef::cms_nullPtr)
						setFloatArrayRegion(*valueArray, 0, value.size(), const_cast<jfloat *>(&value[0]));
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList,
						JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetFloatArray(),
						name.getValue(), valueArray->getValue());
				}
				break;
			case DynamicVariable::STRING:
				{
					Unicode::String value;
					objvar.getValue(value);
					// convert the Unicode string to a JavaString
					JavaString jval(value);
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList,
						JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetString(),
						name.getValue(), jval.getValue());
				}
				break;
			case DynamicVariable::STRING_ARRAY:
				{
					std::vector<Unicode::String> value;
					objvar.getValue(value);
					size_t count = value.size();
					LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsString());
					for (size_t i = 0; i < count; ++i)
					{
						JavaString jval(value[i]);
						setObjectArrayElement(*valueArray, i, jval);
					}
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList,
						JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetStringArray(),
						name.getValue(), valueArray->getValue());
				}
				break;
			case DynamicVariable::NETWORK_ID:
				{
					NetworkId value;
					objvar.getValue(value);
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList,
						JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetObjId(),
						name.getValue(), value.getValue());
				}
				break;
			case DynamicVariable::NETWORK_ID_ARRAY:
				{
					std::vector<NetworkId> value;
					objvar.getValue(value);
					size_t count = value.size();
					LocalLongArrayRefPtr valueArray = createNewLongArray(count);
					for (size_t i = 0; i < count; ++i)
					{
						jlong jvalue = value[i].getValue();
						setLongArrayRegion(*valueArray, i, 1, &jvalue);
					}
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList,
						JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetObjIdArray(),
						name.getValue(), valueArray->getValue());
				}
				break;
			case DynamicVariable::LOCATION:
				{
					DynamicVariableLocationData value;
					objvar.getValue(value);
					LocalRefPtr jvalue;
					if (ScriptConversion::convert(value.pos, value.scene, value.cell, jvalue))
					{
						JavaString name(objvar.getName().c_str());
						result = callNonvirtualBooleanMethod(*newDynamicVariableList,
							JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetLocation(),
							name.getValue(), jvalue->getValue());
					}
				}
				break;
			case DynamicVariable::LOCATION_ARRAY:
				{
					std::vector<DynamicVariableLocationData> value;
					objvar.getValue(value);
					size_t count = value.size();
					LocalObjectArrayRefPtr valueArray = createNewObjectArray(count,
						JavaLibrary::getClsLocation());
					LocalRefPtr jvalue;
					for (size_t i = 0; i < count; ++i)
					{
						const DynamicVariableLocationData &valueItem = value[i];
						if (ScriptConversion::convert(valueItem.pos, valueItem.scene,
							valueItem.cell, jvalue))
						{
							setObjectArrayElement(*valueArray, i, *jvalue);
						}
					}
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList,
						JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetLocationArray(),
						name.getValue(), valueArray->getValue());
				}
				break;
			case DynamicVariable::LIST:
				{
					DynamicVariableList::NestedList nested(objvar.getNestedList());
					LocalRefPtr newDynamicVariable = convertDynamicVariableListToObject(env,nested);
					if (newDynamicVariable != LocalRef::cms_nullPtr)
					{
						result = callNonvirtualBooleanMethod(*newDynamicVariableList,
							JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSet(),
							newDynamicVariable->getValue());
					}
				}
				break;
			case DynamicVariable::STRING_ID:
				{
					StringId value;
					objvar.getValue(value);
					LocalRefPtr jvalue;
					if (ScriptConversion::convert(value, jvalue))
					{
						JavaString name(objvar.getName().c_str());
						result = callNonvirtualBooleanMethod(*newDynamicVariableList,
															  JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetStringId(),
															  name.getValue(), jvalue->getValue());
					}
				}
				break;
			case DynamicVariable::STRING_ID_ARRAY:
				{
					std::vector<StringId> value;
					objvar.getValue(value);
					size_t count = value.size();
					LocalObjectArrayRefPtr valueArray = createNewObjectArray(count,
						JavaLibrary::getClsStringId());
					for (size_t i = 0; i < count; ++i)
					{
						LocalRefPtr jvalue;
						if (ScriptConversion::convert(value[i], jvalue))
							setObjectArrayElement(*valueArray, i, *jvalue);
					}
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList,
						JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetStringIdArray(),
						name.getValue(), valueArray->getValue());
				}
				break;
			case DynamicVariable::TRANSFORM:
				{
					Transform value;
					objvar.getValue(value);
					LocalRefPtr jvalue;
					if (ScriptConversion::convert(value, jvalue))
					{
						JavaString name(objvar.getName().c_str());
						result = callNonvirtualBooleanMethod(*newDynamicVariableList, JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetTransform(), name.getValue(), jvalue->getValue());
					}
				}
				break;
			case DynamicVariable::TRANSFORM_ARRAY:
				{
					std::vector<Transform> value;
					objvar.getValue(value);
					size_t count = value.size();
					LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsTransform());
					for (size_t i = 0; i < count; ++i)
					{
						LocalRefPtr jvalue;
						if (ScriptConversion::convert(value[i], jvalue))
							setObjectArrayElement(*valueArray, i, *jvalue);
					}
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList, JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetTransformArray(), name.getValue(), valueArray->getValue());
				}
				break;
			case DynamicVariable::VECTOR:
				{
					Vector value;
					objvar.getValue(value);
					LocalRefPtr jvalue;
					if (ScriptConversion::convert(value, jvalue))
					{
						JavaString name(objvar.getName().c_str());
						result = callNonvirtualBooleanMethod(*newDynamicVariableList, JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetVector(), name.getValue(), jvalue->getValue());
					}
				}
				break;
			case DynamicVariable::VECTOR_ARRAY:
				{
					std::vector<Vector> value;
					objvar.getValue(value);
					size_t count = value.size();
					LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsVector());
					for (size_t i = 0; i < count; ++i)
					{
						LocalRefPtr jvalue;
						if (ScriptConversion::convert(value[i], jvalue))
							setObjectArrayElement(*valueArray, i, *jvalue);
					}
					JavaString name(objvar.getName().c_str());
					result = callNonvirtualBooleanMethod(*newDynamicVariableList, JavaLibrary::getClsDynamicVariableList(), JavaLibrary::getMidDynamicVariableListSetVectorArray(), name.getValue(), valueArray->getValue());
				}
				break;
		}
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			result = JNI_FALSE;
		}
		if (result == JNI_FALSE)
		{
			return LocalRef::cms_nullPtr;
		}
	}
	return newDynamicVariableList;
}	// JavaLibrary::convertDynamicVariableListToObject

/**
 * Given a Java obj_id and objvar name, converts them to a C ServerObject *
 * and string. If there is an error we do a Java stack dump to help find which script
 * made the bad call.
 *
 * @param env			Java environment
 * @param jlong			the obj_id to convert to a ServerObject *
 * @param name			the Java objvar name to convert to C
 * @param buffer		a buffer to be filled in with the converted C name
 * @param bufferSize	the length of buffer
 *
 * @return the ServerObject * for the obj_id, or nullptr on error
 */
ServerObject * ScriptMethodsDynamicVariableNamespace::getObjectAndName(JNIEnv *env, jlong objId, jstring name, char * buffer, size_t bufferSize)
{
	NOT_NULL(buffer);

	ServerObject * object = nullptr;
	JavaStringParam localName(name);
	if (localName.fillBuffer(buffer, bufferSize) > 1)
	{
		if (!JavaLibrary::getObject(objId, object))
		{
			WARNING((!ConfigServerGame::getDisableObjvarNullCheck()), ("WARNING: Could not get object for objvar %s call\n", buffer));
		}
	}
	else
	{
		WARNING(true, ("Could not get objvar name."));
	}

	if (object == nullptr && !ConfigServerGame::getDisableObjvarNullCheck())
		JavaLibrary::printJavaStack();

	return object;
}

/**
 * Given a Java obj_id and objvar name, converts them to a C DynamicVariableList *
 * and string. If there is an error we do a Java stack dump to help find which script
 * made the bad call.
 *
 * @param env			Java environment
 * @param jlong			the obj_id to convert to a ServerObject *
 * @param name			the Java objvar name to convert to C
 * @param buffer		a buffer to be filled in with the converted C name
 * @param bufferSize	the length of buffer
 *
 * @return the DynamicVariableList * for the obj_id, or nullptr on error
 */
const DynamicVariableList * ScriptMethodsDynamicVariableNamespace::getObjvarsAndName(JNIEnv *env, jlong objId, jstring name, char * buffer, size_t bufferSize)
{
	NOT_NULL(buffer);

	const DynamicVariableList * objvars = nullptr;
	const ServerObject * object = getObjectAndName(env, objId, name, buffer, bufferSize);
	if (object != nullptr)
	{
		testIsSafeToReadObjvar(*object, buffer);
		objvars = &object->getObjVars();
	}
	return objvars;
}


//========================================================================
// class JavaLibrary JNI objvar callback methods
//========================================================================

/**
 * Gets an objvar.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 *
 * @todo add support for objvar lists
 */
jobject JNICALL ScriptMethodsDynamicVariableNamespace::getDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvars = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvars == nullptr)
		return 0;
	const std::string objvarName(buffer);

	if (!objvars->hasItem(objvarName))
		return 0;

	// create the parameters for the java objVar constructor and create it
	LocalRefPtr newDynamicVariable;
	switch (objvars->getType(objvarName))
	{
		case DynamicVariable::INT:
			{
				int value;
				objvars->getItem(objvarName,value);
				newDynamicVariable = createNewObject(JavaLibrary::getClsDynamicVariable(),
					JavaLibrary::getMidDynamicVariableInt(), name, value);
			}
			break;
		case DynamicVariable::INT_ARRAY:
			{
				std::vector<int> value;
				objvars->getItem(objvarName,value);
				LocalIntArrayRefPtr valueArray = createNewIntArray(value.size());
				if (value.size() > 0)
				{
					setIntArrayRegion(*valueArray, 0, value.size(),
						reinterpret_cast<jint *>(const_cast<int *>(&value[0])));
				}
				newDynamicVariable = createNewObject(JavaLibrary::getClsDynamicVariable(),
					JavaLibrary::getMidDynamicVariableIntArray(), name, valueArray->getValue());
			}
			break;
		case DynamicVariable::REAL:
			{
				float value;
				objvars->getItem(objvarName,value);
				newDynamicVariable = createNewObject(JavaLibrary::getClsDynamicVariable(),
					JavaLibrary::getMidDynamicVariableFloat(), name, value);
			}
			break;
		case DynamicVariable::REAL_ARRAY:
			{
				std::vector<float> value;
				objvars->getItem(objvarName,value);
				LocalFloatArrayRefPtr valueArray = createNewFloatArray(value.size());
				if (value.size() > 0 && valueArray != LocalFloatArrayRef::cms_nullPtr)
				{
					setFloatArrayRegion(*valueArray, 0, value.size(),
						const_cast<jfloat *>(&value[0]));
				}
				newDynamicVariable = createNewObject(JavaLibrary::getClsDynamicVariable(),
					JavaLibrary::getMidDynamicVariableFloatArray(), name, valueArray->getValue());
			}
			break;
		case DynamicVariable::STRING:
			{
				Unicode::String value;
				objvars->getItem(objvarName,value);
				// convert the Unicode string to a jstring
				JavaString jval(value);
				newDynamicVariable = createNewObject(JavaLibrary::getClsDynamicVariable(),
					JavaLibrary::getMidDynamicVariableString(), name, jval.getValue());
			}
			break;
		case DynamicVariable::STRING_ARRAY:
			{
				std::vector<Unicode::String> value;
				objvars->getItem(objvarName,value);
				size_t count = value.size();
				LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsString());
				for (size_t i = 0; i < count; ++i)
				{
					JavaString jval(value[i]);
					setObjectArrayElement(*valueArray, i, jval);
				}
				newDynamicVariable = createNewObject(JavaLibrary::getClsDynamicVariable(),
					JavaLibrary::getMidDynamicVariableStringArray(), name, valueArray->getValue());
			}
			break;
		case DynamicVariable::NETWORK_ID:
			{
				NetworkId value;
				objvars->getItem(objvarName,value);
				newDynamicVariable = createNewObject(JavaLibrary::getClsDynamicVariable(),
					JavaLibrary::getMidDynamicVariableObjId(), name, value.getValue());
			}
			break;
		case DynamicVariable::NETWORK_ID_ARRAY:
			{
				std::vector<NetworkId> value;
				objvars->getItem(objvarName,value);
				size_t count = value.size();
				LocalLongArrayRefPtr valueArray = createNewLongArray(count);
				for (size_t i = 0; i < count; ++i)
				{
					jlong jval = value[i].getValue();
					setLongArrayRegion(*valueArray, i, 1, &jval);
				}
				newDynamicVariable = createNewObject(JavaLibrary::getClsDynamicVariable(),
					JavaLibrary::getMidDynamicVariableObjIdArray(), name, valueArray->getValue());
			}
			break;
		case DynamicVariable::LOCATION:
			{
				DynamicVariableLocationData value;
				objvars->getItem(objvarName,value);
				LocalRefPtr jvalue;
				if (ScriptConversion::convert(value.pos, value.scene, value.cell, jvalue))
				{
					newDynamicVariable = createNewObject(JavaLibrary::getClsDynamicVariable(),
						JavaLibrary::getMidDynamicVariableLocation(), name, jvalue->getValue());
				}
			}
			break;
		case DynamicVariable::LOCATION_ARRAY:
			{
				std::vector<DynamicVariableLocationData> value;
				objvars->getItem(objvarName,value);
				size_t count = value.size();
				LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsLocation());
				LocalRefPtr jvalue;
				for (size_t i = 0; i < count; ++i)
				{
					const DynamicVariableLocationData &valueItem = value[i];
					if (ScriptConversion::convert(valueItem.pos, valueItem.scene,
						valueItem.cell, jvalue))
					{
						setObjectArrayElement(*valueArray, i, *jvalue);
					}
				}
				newDynamicVariable = createNewObject(JavaLibrary::getClsDynamicVariable(),
					JavaLibrary::getMidDynamicVariableLocationArray(), name, valueArray->getValue());
			}
			break;
		case DynamicVariable::LIST:
			newDynamicVariable = convertDynamicVariableListToObject(env, DynamicVariableList::NestedList(*objvars, objvarName));
			break;
		default:
			newDynamicVariable = LocalRef::cms_nullPtr;
			break;
	}
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return 0;
	}
	return newDynamicVariable->getReturnValue();
}	// JavaLibrary::getDynamicVariable

/**
 * Gets an objvar of type int.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jint JNICALL ScriptMethodsDynamicVariableNamespace::getIntDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	int localValue=0;
	if (!objvarList->getItem(buffer,localValue))
		return 0;

	jint value = localValue;
	return value;
}	// JavaLibrary::getIntDynamicVariable

/**
 * Gets an objvar of type int[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jintArray JNICALL ScriptMethodsDynamicVariableNamespace::getIntArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	std::vector<int> value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	LocalIntArrayRefPtr valueArray = createNewIntArray(value.size());
	if (value.size() > 0)
	{
		setIntArrayRegion(*valueArray, 0, value.size(), reinterpret_cast<jint *>
			(const_cast<int *>(&value[0])));
	}
	return valueArray->getReturnValue();
}	// JavaLibrary::getIntArrayDynamicVariable

/**
 * Gets an objvar of type float.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jfloat JNICALL ScriptMethodsDynamicVariableNamespace::getFloatDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	float localValue = 0;
	if (!objvarList->getItem(buffer,localValue))
		return 0;

	jfloat value = localValue;
	return value;
}	// JavaLibrary::getFloatDynamicVariable

/**
 * Gets an objvar of type float[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jfloatArray JNICALL ScriptMethodsDynamicVariableNamespace::getFloatArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	std::vector<real> value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	LocalFloatArrayRefPtr valueArray = createNewFloatArray(value.size());
	if (value.size() > 0 && valueArray != LocalFloatArrayRef::cms_nullPtr)
		setFloatArrayRegion(*valueArray, 0, value.size(), const_cast<jfloat *>(&value[0]));
	return valueArray->getReturnValue();
}	// JavaLibrary::getFloatArrayDynamicVariable

/**
 * Gets an objvar of type string.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jstring JNICALL ScriptMethodsDynamicVariableNamespace::getStringDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	Unicode::String value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	// convert EString to jstring
	return JavaString(value).getReturnValue();
}	// JavaLibrary::getStringDynamicVariable

/**
 * Gets an objvar of type string[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jobjectArray JNICALL ScriptMethodsDynamicVariableNamespace::getStringArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	std::vector<Unicode::String> value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	size_t count = value.size();
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsString());
	for (size_t i = 0; i < count; ++i)
	{
		JavaString jval(value[i]);
		setObjectArrayElement(*valueArray, i, jval);
	}
	return valueArray->getReturnValue();
}	// JavaLibrary::getStringArrayDynamicVariable

/**
 * Gets an objvar of type obj_id.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jlong JNICALL ScriptMethodsDynamicVariableNamespace::getObjIdDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	NetworkId localValue;
	if (!objvarList->getItem(buffer,localValue))
		return 0;
	return localValue.getValue();
}	// JavaLibrary::getObjIdDynamicVariable

/**
 * Gets an objvar of type obj_id[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jlongArray JNICALL ScriptMethodsDynamicVariableNamespace::getObjIdArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList *objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	std::vector<NetworkId> value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	size_t count = value.size();
	LocalLongArrayRefPtr valueArray = createNewLongArray(count);
	jlong jlongTmp;
	if (valueArray != LocalLongArrayRef::cms_nullPtr)
	{
		for (size_t i = 0; i < count; ++i)
		{
			jlongTmp = value[i].getValue();
			setLongArrayRegion(*valueArray, i, 1, &jlongTmp);
		}
	}

	return valueArray->getReturnValue();
}	// JavaLibrary::getObjIdArrayDynamicVariable

/**
 * Gets an objvar of type location.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jobject JNICALL ScriptMethodsDynamicVariableNamespace::getLocationDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	DynamicVariableLocationData value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	LocalRefPtr jloc;
	if (!ScriptConversion::convert(value.pos, value.scene, value.cell, jloc))
		return 0;

	return jloc->getReturnValue();
}	// JavaLibrary::getLocationDynamicVariable

/**
 * Gets an objvar of type location[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jobjectArray JNICALL ScriptMethodsDynamicVariableNamespace::getLocationArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	std::vector<DynamicVariableLocationData> value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	size_t count = value.size();
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsLocation());
	if (valueArray == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	LocalRefPtr jvalue;
	for (size_t i = 0; i < count; ++i)
	{
		const DynamicVariableLocationData &valueItem = value[i];
		if (ScriptConversion::convert(valueItem.pos, valueItem.scene, valueItem.cell,
			jvalue))
		{
			setObjectArrayElement(*valueArray, i, *jvalue);
		}
		else
			setObjectArrayElement(*valueArray, i, LocalRef(0));
	}
	return valueArray->getReturnValue();
}	// JavaLibrary::getLocationArrayDynamicVariable

/**
 * Gets an objvar of type string_id.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jobject JNICALL ScriptMethodsDynamicVariableNamespace::getStringIdDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	StringId value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	LocalRefPtr string_id;
	if (!ScriptConversion::convert(value, string_id))
		return 0;

	return string_id->getReturnValue();
}	// JavaLibrary::getStringIdDynamicVariable

/**
 * Gets an objvar of type string_id[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jobjectArray JNICALL ScriptMethodsDynamicVariableNamespace::getStringIdArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	std::vector<StringId> value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	size_t count = value.size();
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsStringId());
	if (valueArray == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	LocalRefPtr jvalue;
	for (size_t i = 0; i < count; ++i)
	{
		const StringId & valueItem = value[i];
		if (ScriptConversion::convert(valueItem, jvalue))
			setObjectArrayElement(*valueArray, i, *jvalue);
		else
			setObjectArrayElement(*valueArray, i, LocalRef(0));
	}
	return valueArray->getReturnValue();
}	// JavaLibrary::getStringIdArrayDynamicVariable

/**
 * Gets an objvar of type transform.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jobject JNICALL ScriptMethodsDynamicVariableNamespace::getTransformDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	Transform value;
	if (!objvarList->getItem(buffer, value))
		return 0;

	LocalRefPtr transform;
	if (!ScriptConversion::convert(value, transform))
		return 0;

	return transform->getReturnValue();
}

/**
 * Gets an objvar of type transform[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jobjectArray JNICALL ScriptMethodsDynamicVariableNamespace::getTransformArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	std::vector<Transform> value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	size_t count = value.size();
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsTransform());
	if (valueArray == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	LocalRefPtr jvalue;
	for (size_t i = 0; i < count; ++i)
	{
		Transform const &valueItem = value[i];
		if (ScriptConversion::convert(valueItem, jvalue))
			setObjectArrayElement(*valueArray, i, *jvalue);
		else
			setObjectArrayElement(*valueArray, i, LocalRef(0));
	}
	return valueArray->getReturnValue();
}

/**
 * Gets an objvar of type vector.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jobject JNICALL ScriptMethodsDynamicVariableNamespace::getVectorDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	Vector value;
	if (!objvarList->getItem(buffer, value))
		return 0;

	LocalRefPtr vec;
	if (!ScriptConversion::convert(value, vec))
		return 0;

	return vec->getReturnValue();
}

/**
 * Gets an objvar of type vector[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return the objvar value
 */
jobjectArray JNICALL ScriptMethodsDynamicVariableNamespace::getVectorArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const DynamicVariableList * objvarList = getObjvarsAndName(env, objId, name, buffer, sizeof(buffer));
	if (objvarList == nullptr)
		return 0;

	std::vector<Vector> value;
	if (!objvarList->getItem(buffer,value))
		return 0;

	size_t count = value.size();
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsVector());
	if (valueArray == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	LocalRefPtr jvalue;
	for (size_t i = 0; i < count; ++i)
	{
		Vector const &valueItem = value[i];
		if (ScriptConversion::convert(valueItem, jvalue))
			setObjectArrayElement(*valueArray, i, *jvalue);
		else
			setObjectArrayElement(*valueArray, i, LocalRef(0));
	}
	return valueArray->getReturnValue();
}

/**
 * Returns an objvar list.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class to get the list from
 * @param name			name of the list
 *
 * @return the objvar list
 */
jobject JNICALL ScriptMethodsDynamicVariableNamespace::getDynamicVariableList(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return 0;

	JavaStringParam localName(name);

	const ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(objId, object))
		return 0;

	const DynamicVariableList &objvarList = object->getObjVars();

	LocalRefPtr result;
	if (name == 0 || getStringLength(localName) == 0)
	{
		// do the whole list
		result = convertDynamicVariableListToObject(env, DynamicVariableList::NestedList(objvarList));
	}
	else
	{
		// convert jstrings to stl strings
		char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
		if (localName.fillBuffer(buffer, sizeof(buffer)) > 0 && objvarList.hasItem(buffer))
		{
			result = convertDynamicVariableListToObject(env, DynamicVariableList::NestedList(objvarList, buffer));
		}
	}

	if (result.get() == nullptr)
		return 0;
	return result->getReturnValue();
}	// JavaLibrary::getDynamicVariableList

/**
 * Removes an objvar from an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of object containing the objvar
 * @param name			name of the objvar
 */
void JNICALL ScriptMethodsDynamicVariableNamespace::removeDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return;

	object->removeObjVarItem(buffer);
}	// JavaLibrary::removeDynamicVariable

/**
 * Removes all the objvars from an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    the object
 */
void JNICALL ScriptMethodsDynamicVariableNamespace::removeAllDynamicVariables(JNIEnv *env, jobject self, jlong objId)
{
	UNREF(self);

	if (objId == 0)
		return;

	ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(objId, object))
		return;

	object->eraseObjVarListEntries(".");
}	// JavaLibrary::removeAllDynamicVariables

/**
 * Determines if an object has an objvar.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 *
 * @return true if the objvar exists, false if not
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::hasDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	const ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return 0;

	const DynamicVariableList &objvarList = object->getObjVars();

	if (objvarList.hasItem(buffer))
		return JNI_TRUE;
	else
		return JNI_FALSE;
}	// JavaLibrary::hasDynamicVariable

/**
 * Sets an objvar of type obj_var.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobject data)
{
	UNREF(self);

	if (objId == 0 || name == 0 || data == 0)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	const DynamicVariableList *objvarList = &(object->getObjVars());
	if (objvarList == nullptr)
		return JNI_FALSE;

	// determine what type the data is
/*
	if (env->IsInstanceOf(data, JavaLibrary::getClsDynamicVariable()) == JNI_TRUE)
	{
		// name is the name of an objvar list we will add data to
		DynamicVariable *objvar = objvarList->getItemByName(localName);
		if (objvar == nullptr)
			objvar = objvarList->addNestedList(localName);
		if (objvar != nullptr && objvar->getType() == DynamicVariable::LIST)
			return updateDynamicVariableList(env, *dynamic_cast<DynamicVariableList*>(objvar), data);
		return JNI_FALSE;
	}
	else if (env->IsInstanceOf(data, ms_clsInteger) == JNI_TRUE)
	{
		if (objvarList->setItem(localName, static_cast<int>(env->CallIntMethod(data, ms_midIntegerIntValue))))
			return JNI_TRUE;
		return JNI_FALSE;
	}
	else if (env->IsInstanceOf(data, ms_clsFloat) == JNI_TRUE)
	{
		if (objvarList->setItem(localName, env->CallFloatMethod(data, ms_midFloatFloatValue)))
			return JNI_TRUE;
		return JNI_FALSE;
	}
	else if (env->IsInstanceOf(data, ms_clsString) == JNI_TRUE)
	{
		if (objvarList->setItem(localName, Unicode::String(env->GetStringChars(static_cast<jstring>(data), nullptr))))
		#error must release characters
			return JNI_TRUE;
		return JNI_FALSE;
	}
*/
	return JNI_FALSE;
}	// JavaLibrary::setDynamicVariable

/**
 * Sets an objvar of type int.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setIntDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jint value)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	addSetObjvar(*object, buffer);

	return object->setObjVarItem(buffer, static_cast<int>(value));
}	// JavaLibrary::setIntDynamicVariable

/**
 * Sets an objvar of type int[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setIntArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jintArray value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	size_t size = env->GetArrayLength(value);
	if (size == 0)
	{
		object->removeObjVarItem(buffer);
		if (ConfigServerGame::getExceptionOnObjVarZeroLengthArray())
		{
			JavaLibrary::throwInternalScriptError("JavaLibrary::setIntArrayDynamicVariable "
				"trying to set an array objvar to 0 length. The objvar has been removed instead.");
		}
		return JNI_FALSE;
	}

	addSetObjvar(*object, buffer);

	std::vector<int> valueArray;
	valueArray.resize(size);
	env->GetIntArrayRegion(value, 0, size, reinterpret_cast<jint*>(&valueArray[0]));
	return object->setObjVarItem(buffer, valueArray);
}	// JavaLibrary::setIntArrayDynamicVariable

/**
 * Sets an objvar of type float.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setFloatDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jfloat value)
{
	UNREF(self);

	if (objId == 0 || name == 0)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	addSetObjvar(*object, buffer);

	return object->setObjVarItem(buffer, value);
}	// JavaLibrary::setFloatDynamicVariable

/**
 * Sets an objvar of type float[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setFloatArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jfloatArray value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	size_t size = env->GetArrayLength(value);
	if (size == 0)
	{
		object->removeObjVarItem(buffer);
		if (ConfigServerGame::getExceptionOnObjVarZeroLengthArray())
		{
			JavaLibrary::throwInternalScriptError("JavaLibrary::setFloatArrayDynamicVariable "
				"trying to set an array objvar to 0 length. The objvar has been removed instead.");
		}
		return false;
	}

	addSetObjvar(*object, buffer);

	std::vector<real> valueArray;
	valueArray.resize(size);
	env->GetFloatArrayRegion(value, 0, size, &valueArray[0]);
	return object->setObjVarItem(buffer, valueArray);
}	// JavaLibrary::setFloatArrayDynamicVariable

/**
 * Sets an objvar of type string.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setStringDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jstring value)
{
	UNREF(self);

	if (objId == 0)
		return JNI_FALSE;

	if (name == 0)
	{
		DEBUG_WARNING(true, ("nullptr name passed from script to JavaLibrary::setStringDynamicValue"));
		return JNI_FALSE;
	}

	JavaStringParam localValue(value);

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	if (value == 0)
	{
		DEBUG_WARNING(true, ("nullptr string passed from script to JavaLibrary::setStringDynamicValue (%s)",buffer));
		return JNI_FALSE;
	}

	const DynamicVariableList *objvarList = &(object->getObjVars());
	if (objvarList == nullptr)
		return JNI_FALSE;

	Unicode::String valueString;
	if (!JavaLibrary::convert(localValue, valueString))
		return JNI_FALSE;

	addSetObjvar(*object, buffer);

	return object->setObjVarItem(buffer, valueString);
}	// JavaLibrary::setStringDynamicVariable

/**
 * Sets an objvar of type string[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setStringArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobjectArray value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsStringArray()) == JNI_FALSE)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	std::vector<Unicode::String> valueArray;
	size_t size = env->GetArrayLength(value);
	if (size == 0)
	{
		object->removeObjVarItem(buffer);
		if (ConfigServerGame::getExceptionOnObjVarZeroLengthArray())
		{
			JavaLibrary::throwInternalScriptError("JavaLibrary::setStringArrayDynamicVariable "
				"trying to set an array objvar to 0 length. The objvar has been removed instead.");
		}
		return JNI_FALSE;
	}

	valueArray.resize(size);
	for (size_t i = 0; i < size; ++i)
	{
		JavaStringPtr item = getStringArrayElement(LocalObjectArrayRefParam(value), i);
		if (item != JavaString::cms_nullPtr)
			JavaLibrary::convert(*item, valueArray[i]);
		else
			valueArray[i] = Unicode::String();
	}

	addSetObjvar(*object, buffer);

	// For some reason, returning the value directly from the function call here
	// makes gcc 2.95.3 think the end of the function can be reached without
	// returning a value in release builds, so we assign the result and return
	// it to work around the bug.
	bool const ret = object->setObjVarItem(buffer, valueArray);
	return static_cast<jboolean>(ret);
}	// JavaLibrary::setStringArrayDynamicVariable

/**
 * Sets an objvar of type obj_id.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setObjIdDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jlong value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	NetworkId oidValue(static_cast<NetworkId::NetworkIdType>
		(env->CallLongMethod(JavaLibrary::getObjId(value)->getReturnValue(), JavaLibrary::getMidObjIdGetValue())));

	addSetObjvar(*object, buffer);

	return object->setObjVarItem(buffer, oidValue);
}	// JavaLibrary::setObjIdDynamicVariable

/**
 * Sets an objvar of type obj_id[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setObjIdArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jlongArray value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	std::vector<NetworkId> valueArray;
	size_t size = env->GetArrayLength(value);
	if (size == 0)
	{
		object->removeObjVarItem(buffer);
		if (ConfigServerGame::getExceptionOnObjVarZeroLengthArray())
		{
			JavaLibrary::throwInternalScriptError("JavaLibrary::setObjIdArrayDynamicVariable "
				"trying to set an array objvar to 0 length. The objvar has been removed instead.");
		}
		return JNI_FALSE;
	}

	valueArray.resize(size);
	jlong jlongTmp;
	for (size_t i = 0; i < size; ++i)
	{
		env->GetLongArrayRegion(value, i, 1, &jlongTmp);
			valueArray[i] = NetworkId(static_cast<NetworkId::NetworkIdType>(jlongTmp));
	}

	addSetObjvar(*object, buffer);

	bool result = object->setObjVarItem(buffer, valueArray);
	return result;
}	// JavaLibrary::setObjIdArrayDynamicVariable

/**
 * Sets an objvar of type location.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setLocationDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobject value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsLocation()) == JNI_FALSE)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	DynamicVariableLocationData locValue;
	locValue.pos.x = env->GetFloatField(value, JavaLibrary::getFidLocationX());
	locValue.pos.y = env->GetFloatField(value, JavaLibrary::getFidLocationY());
	locValue.pos.z = env->GetFloatField(value, JavaLibrary::getFidLocationZ());
	JavaStringPtr area = getStringField(LocalRefParam(value), JavaLibrary::getFidLocationArea());
	JavaLibrary::convert(*area, locValue.scene);
	LocalRefPtr cell = getObjectField(LocalRefParam(value), JavaLibrary::getFidLocationCell());
	locValue.cell = JavaLibrary::getNetworkId(*cell);

	addSetObjvar(*object, buffer);

	return object->setObjVarItem(buffer, locValue);
}	// JavaLibrary::setLocationDynamicVariable

/**
 * Sets an objvar of type location[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setLocationArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobjectArray value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsLocationArray()) == JNI_FALSE)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	std::vector<DynamicVariableLocationData> valueArray;
	size_t size = env->GetArrayLength(value);
	if (size == 0)
	{
		object->removeObjVarItem(buffer);
		if (ConfigServerGame::getExceptionOnObjVarZeroLengthArray())
		{
			JavaLibrary::throwInternalScriptError("JavaLibrary::setLocationArrayDynamicVariable "
				"trying to set an array objvar to 0 length. The objvar has been removed instead.");
		}
		return JNI_FALSE;
	}

	valueArray.resize(size);
	for (size_t i = 0; i < size; ++i)
	{
		LocalRefPtr item = getObjectArrayElement(LocalObjectArrayRefParam(value), i);
		if (item != LocalRef::cms_nullPtr)
		{
			DynamicVariableLocationData &valueItem = valueArray[i];
			valueItem.pos.x = getFloatField(*item, JavaLibrary::getFidLocationX());
			valueItem.pos.y = getFloatField(*item, JavaLibrary::getFidLocationY());
			valueItem.pos.z = getFloatField(*item, JavaLibrary::getFidLocationZ());
			JavaStringPtr area = getStringField(*item, JavaLibrary::getFidLocationArea());
			JavaLibrary::convert(*area, valueItem.scene);
			LocalRefPtr cell = getObjectField(*item, JavaLibrary::getFidLocationCell());
			valueItem.cell = JavaLibrary::getNetworkId(*cell);
		}
		else
			valueArray[i] = DynamicVariableLocationData();
	}

	addSetObjvar(*object, buffer);

	bool result = object->setObjVarItem(buffer, valueArray);
	return result;
}	// JavaLibrary::setLocationArrayDynamicVariable

/**
 * Sets an objvar of type string_id.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setStringIdDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobject value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsStringId()) == JNI_FALSE)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	StringId locValue;
	if (!ScriptConversion::convert(value, locValue))
		return JNI_FALSE;

	addSetObjvar(*object, buffer);

	return object->setObjVarItem(buffer, locValue);
}	// JavaLibrary::setStringIdDynamicVariable

/**
 * Sets an objvar of type string_id[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setStringIdArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobjectArray value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsStringIdArray()) == JNI_FALSE)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	std::vector<StringId> valueArray;
	size_t size = env->GetArrayLength(value);
	if (size == 0)
	{
		object->removeObjVarItem(buffer);
		if (ConfigServerGame::getExceptionOnObjVarZeroLengthArray())
		{
			JavaLibrary::throwInternalScriptError("JavaLibrary::setStringIdArrayDynamicVariable "
				"trying to set an array objvar to 0 length. The objvar has been removed instead.");
		}
		return false;
	}

	valueArray.resize(size);
	for (size_t i = 0; i < size; ++i)
	{
		LocalRefPtr item = getObjectArrayElement(LocalObjectArrayRefParam(value), i);
		if (item == LocalRef::cms_nullPtr)
			return JNI_FALSE;
		StringId &valueItem = valueArray[i];
		if (!ScriptConversion::convert(*item, valueItem))
			return JNI_FALSE;
	}

	addSetObjvar(*object, buffer);

	bool result = object->setObjVarItem(buffer, valueArray);
	return result;
}	// JavaLibrary::setStringIdArrayDynamicVariable

/**
 * Sets an objvar of type transform.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setTransformDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobject value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsTransform()) == JNI_FALSE)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	Transform locValue;
	if (!ScriptConversion::convert(value, locValue))
		return JNI_FALSE;

	addSetObjvar(*object, buffer);

	return object->setObjVarItem(buffer, locValue);
}

/**
 * Sets an objvar of type transform[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setTransformArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobjectArray value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsTransformArray()) == JNI_FALSE)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	std::vector<Transform> valueArray;
	size_t size = env->GetArrayLength(value);
	if (size == 0)
	{
		object->removeObjVarItem(buffer);
		JavaLibrary::throwInternalScriptError("JavaLibrary::setTransformArrayDynamicVariable trying to set an array objvar to 0 length. The objvar has been removed instead.");
		return false;
	}

	valueArray.resize(size);
	for (size_t i = 0; i < size; ++i)
	{
		LocalRefPtr item = getObjectArrayElement(LocalObjectArrayRefParam(value), i);
		if (item == LocalRef::cms_nullPtr)
			return JNI_FALSE;
		Transform &valueItem = valueArray[i];
		if (!ScriptConversion::convert(*item, valueItem))
			return JNI_FALSE;
	}

	addSetObjvar(*object, buffer);

	bool result = object->setObjVarItem(buffer, valueArray);
	return result;
}

/**
 * Sets an objvar of type vector.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setVectorDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobject value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsVector()) == JNI_FALSE)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	Vector locValue;
	if (!ScriptConversion::convert(value, locValue))
		return JNI_FALSE;

	addSetObjvar(*object, buffer);

	return object->setObjVarItem(buffer, locValue);
}

/**
 * Sets an objvar of type vector[].
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the objvar
 * @param value         value to set the objvar to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setVectorArrayDynamicVariable(JNIEnv *env, jobject self, jlong objId, jstring name, jobjectArray value)
{
	UNREF(self);

	if (objId == 0 || name == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsVectorArray()) == JNI_FALSE)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	ServerObject * object = getObjectAndName(env, objId, name, buffer, sizeof(buffer));
	if (object == nullptr)
		return JNI_FALSE;

	std::vector<Vector> valueArray;
	size_t size = env->GetArrayLength(value);
	if (size == 0)
	{
		object->removeObjVarItem(buffer);
		JavaLibrary::throwInternalScriptError("JavaLibrary::setVectorArrayDynamicVariable trying to set an array objvar to 0 length. The objvar has been removed instead.");
		return false;
	}

	valueArray.resize(size);
	for (size_t i = 0; i < size; ++i)
	{
		LocalRefPtr item = getObjectArrayElement(LocalObjectArrayRefParam(value), i);
		if (item == LocalRef::cms_nullPtr)
			return JNI_FALSE;
		Vector &valueItem = valueArray[i];
		if (!ScriptConversion::convert(*item, valueItem))
			return JNI_FALSE;
	}

	addSetObjvar(*object, buffer);

	bool result = object->setObjVarItem(buffer, valueArray);
	return result;
}

/**
 * Creates an objvar list.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class containing the objvar
 * @param name			name of the list
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::setDynamicVariableList(JNIEnv *, jobject, jlong, jstring)
{
	// This is now a no-op, left for compatibility with older scripts.
	return JNI_TRUE;
}	// JavaLibrary::setDynamicVariableList

/**
 * Copies an objvar from one object to another.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param fromId        id of object to copy from
 * @param toId          id of object to copy to
 * @param name			name of the objvar to copy
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsDynamicVariableNamespace::copyDynamicVariable(JNIEnv *env, jobject self, jlong fromId, jlong toId, jstring name)
{
	UNREF(self);

	JavaStringParam localName(name);

	if (fromId == 0 || toId == 0 || name == 0)
		return JNI_FALSE;

	CachedNetworkId from(fromId);
	CachedNetworkId to(toId);

	if (from == to)
		return JNI_TRUE;

	ServerObject* fromObject = dynamic_cast<ServerObject*>(from.getObject());
	if (fromObject == nullptr)
		return JNI_FALSE;
	ServerObject* toObject = dynamic_cast<ServerObject*>(to.getObject());
	if (toObject == nullptr)
		return JNI_FALSE;

	char buffer[DynamicVariable::MAX_DYNAMIC_VARIABLE_NAME_LEN];
	if (localName.fillBuffer(buffer, sizeof(buffer)) <= 0)
		return JNI_FALSE;

	if(toObject->isAuthoritative())
	{
		toObject->copyObjVars(buffer, *fromObject, buffer);
	}
	else
	{
		std::string stringBuffer(buffer);
		toObject->sendControllerMessageToAuthServer(CM_copyDynamicVariable, new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, NetworkId> > >(std::make_pair(stringBuffer, std::make_pair(from, to))));
	}
	return JNI_TRUE;
}	// JavaLibrary::copyDynamicVariable

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsDynamicVariableNamespace::getPackedObjvars(JNIEnv *env, jobject self, jlong source, jstring jprefix)
{
	ServerObject *sourceObj = 0;
	if (!JavaLibrary::getObject(source, sourceObj))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getPackedObjvars: could not find source object"));
	else
	{
		std::string prefix;
		if (jprefix)
		{
			JavaStringParam prefixParam(jprefix);
			if (!JavaLibrary::convert(prefixParam, prefix))
			{
				JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getPackedObjvars: bad prefix string"));
				return 0;
			}
		}

		std::string const &result = sourceObj->getPackedObjVars(prefix);

		return JavaString(result).getReturnValue();
	}
	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsDynamicVariableNamespace::setPackedObjvars(JNIEnv *env, jobject self, jlong target, jstring packedVars)
{
	ServerObject *targetObj = 0;
	if (!JavaLibrary::getObject(target, targetObj))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setPackedObjvars: could not find target object"));
	else
	{
		JavaStringParam localPackedVars(packedVars);
		std::string packedVarString;
		if (!JavaLibrary::convert(localPackedVars, packedVarString))
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setPackedObjvars: bad packed var string"));
		else if (!targetObj->setPackedObjVars(packedVarString))
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setPackedObjvars: bad packed var string"));
	}
}

// ======================================================================

