//========================================================================
//
// ScriptMethodsClusterWideData.cpp - implements script methods dealing
// with cluster wide data management
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/ClusterWideDataClient.h"
#include "sharedUtility/ValueDictionary.h"
#include "sharedUtility/ValueTypeBool.h"
#include "sharedUtility/ValueTypeFloat.h"
#include "sharedUtility/ValueTypeObjId.h"
#include "sharedUtility/ValueTypeSignedInt.h"
#include "sharedUtility/ValueTypeString.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsClusterWideDataNamespace
// ======================================================================

namespace ScriptMethodsClusterWideDataNamespace
{
	bool install();

	jint JNICALL getClusterWideData(JNIEnv *env, jobject self, jstring managerName, jstring elementNameRegex, jboolean lockElements, jlong callbackObject);
	void JNICALL releaseClusterWideDataLock(JNIEnv *env, jobject self, jstring managerName, jint lockKey);
	void JNICALL removeClusterWideData(JNIEnv *env, jobject self, jstring managerName, jstring elementNameRegex, jint lockKey);
	void JNICALL updateClusterWideData(JNIEnv *env, jobject self, jstring managerName, jstring elementNameRegex, jobject data, jint lockKey);
	void JNICALL replaceClusterWideData(JNIEnv *env, jobject self, jstring managerName, jstring elementNameRegex, jobject data, jboolean autoRemove, jint lockKey);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsClusterWideDataNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsClusterWideDataNamespace::c)}
	JF("_getClusterWideData", "(Ljava/lang/String;Ljava/lang/String;ZJ)I", getClusterWideData),
	JF("releaseClusterWideDataLock", "(Ljava/lang/String;I)V", releaseClusterWideDataLock),
	JF("removeClusterWideData", "(Ljava/lang/String;Ljava/lang/String;I)V", removeClusterWideData),
	JF("updateClusterWideData", "(Ljava/lang/String;Ljava/lang/String;Lscript/dictionary;I)V", updateClusterWideData),
	JF("replaceClusterWideData", "(Ljava/lang/String;Ljava/lang/String;Lscript/dictionary;ZI)V", replaceClusterWideData),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI conversion methods
//========================================================================

/**
 * Converts a C++ ValueDictionary to a Java dictionary
 *
 * @param source		the ValueDictionary to convert
 *
 * @return a new dictionary object
 */
LocalRefPtr JavaLibrary::convert(const ValueDictionary & source)
{
	if (ms_env == nullptr)
		return LocalRef::cms_nullPtr;

	// create the dictionary
	LocalRefPtr target = createNewObject(ms_clsDictionary, ms_midDictionary);
	if (target == LocalRef::cms_nullPtr)
		return LocalRef::cms_nullPtr;

	for (DictionaryValueMap::const_iterator iter = source.get().begin(); iter != source.get().end(); ++iter)
	{
		JavaString paramName(iter->first.c_str());

		if (iter->second->getType() == ValueTypeBool::ms_type)
		{
			const ValueTypeBool * value = static_cast<const ValueTypeBool *>(iter->second);

			callObjectMethod(*target,
				ms_midDictionaryPutBool, paramName.getValue(),
				value->getValue());
		}
		else if (iter->second->getType() == ValueTypeFloat::ms_type)
		{
			const ValueTypeFloat * value = static_cast<const ValueTypeFloat *>(iter->second);

			callObjectMethod(*target,
				ms_midDictionaryPutFloat, paramName.getValue(),
				value->getValue());
		}
		else if (iter->second->getType() == ValueTypeObjId::ms_type)
		{
			const ValueTypeObjId * value = static_cast<const ValueTypeObjId *>(iter->second);

			LocalRefPtr arg = getObjId(value->getValue());
			if (arg == LocalRef::cms_nullPtr)
				return LocalRef::cms_nullPtr;

			callObjectMethod(*target,
				ms_midDictionaryPut, paramName.getValue(), arg->getValue());
		}
		else if (iter->second->getType() == ValueTypeSignedInt::ms_type)
		{
			const ValueTypeSignedInt * value = static_cast<const ValueTypeSignedInt *>(iter->second);

			callObjectMethod(*target,
				ms_midDictionaryPutInt, paramName.getValue(),
				value->getValue());

		}
		else if (iter->second->getType() == ValueTypeString::ms_type)
		{
			const ValueTypeString * value = static_cast<const ValueTypeString *>(iter->second);

			JavaString jvString(value->getValue().c_str());
			callObjectMethod(*target,
				ms_midDictionaryPut, paramName.getValue(), jvString.getValue());
		}
		else
		{
			DEBUG_REPORT_LOG(true, ("Unknown/unhandled data type "
				"(%s) while parsing ValueDictionary\n", iter->second->getType().c_str()));
			return LocalRef::cms_nullPtr;
		}
	}

	return target;
}	// JavaLibrary::convert(const ValueDictionary &)

// ----------------------------------------------------------------------

/**
 * Converts a Java dictionary to a C++ ValueDictionary
 *
 * @param source		the ValueDictionary to convert
 *
 * @return a new dictionary object
 */
void JavaLibrary::convert(const jobject & source, ValueDictionary & target)
{
	target.clear();

	JNIEnv * env = getEnv();
	if (env == nullptr)
		return;

	if (!env->IsInstanceOf(source, ms_clsDictionary))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::convert from dictionary to ValueDictionary has a bad dictionary"));
		return;
	}

	// get the list of keys in the dictionary
	LocalRefPtr keyEnumeration = callObjectMethod(LocalRefParam(source), ms_midDictionaryKeys);

	while (callBooleanMethod(*keyEnumeration, ms_midEnumerationHasMoreElements))
	{
		LocalRefPtr key = callObjectMethod(*keyEnumeration, ms_midEnumerationNextElement);

		if (!isInstanceOf(*key, ms_clsString))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::convert from dictionary to ValueDictionary has a bad key"));
			return;
		}

		JavaStringParam localKeyStr(*key);
		std::string keyStr;
		IGNORE_RETURN(convert(localKeyStr, keyStr));

		// convert the value into a C++ ValueType object
		LocalRefPtr value = callObjectMethod(LocalRefParam(source), ms_midDictionaryGet, key->getValue());

		if (isInstanceOf(*value, ms_clsBoolean))
		{
			jboolean booleanValue = callBooleanMethod(*value, ms_midBooleanBooleanValue);
			target.insert(keyStr, ValueTypeBool(booleanValue ? true : false));
		}
		else if (isInstanceOf(*value, ms_clsFloat))
		{
			jfloat floatValue = callFloatMethod(*value, ms_midFloatFloatValue);
			target.insert(keyStr, ValueTypeFloat(static_cast<float>(floatValue)));
		}
		else if (isInstanceOf(*value, ms_clsObjId))
		{
			NetworkId networkId(getNetworkId(*value));
			target.insert(keyStr, ValueTypeObjId(networkId));
		}
		else if (isInstanceOf(*value, ms_clsInteger))
		{
			jint intValue = callIntMethod(*value, ms_midIntegerIntValue);
			target.insert(keyStr, ValueTypeSignedInt(static_cast<signed int>(intValue)));
		}
		else if (isInstanceOf(*value, ms_clsString))
		{
			JavaStringParam localValueStr(*value);
			std::string valueStr;
			IGNORE_RETURN(convert(localValueStr, valueStr));

			target.insert(keyStr, ValueTypeString(valueStr));
		}
		else
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::convert from dictionary to ValueDictionary - key (%s) is an unsupported type", keyStr.c_str()));
			return;
		}
	}
}

//--------------------------------------------------------------------------------------

jint JNICALL ScriptMethodsClusterWideDataNamespace::getClusterWideData(JNIEnv *, jobject, jstring managerName, jstring elementNameRegex, jboolean lockElements, jlong callbackObject)
{
	JavaStringParam localJstringManagerName(managerName);
	std::string localManagerName;
	IGNORE_RETURN(JavaLibrary::convert(localJstringManagerName, localManagerName));

	JavaStringParam localJstringElementNameRegex(elementNameRegex);
	std::string localElementNameRegex;
	IGNORE_RETURN(JavaLibrary::convert(localJstringElementNameRegex, localElementNameRegex));

	return static_cast<int>(ClusterWideDataClient::getClusterWideData(localManagerName, localElementNameRegex, (lockElements == JNI_TRUE), NetworkId(callbackObject)));
}

//--------------------------------------------------------------------------------------

void JNICALL ScriptMethodsClusterWideDataNamespace::releaseClusterWideDataLock(JNIEnv *, jobject, jstring managerName, jint lockKey)
{
	JavaStringParam localJstringManagerName(managerName);
	std::string localManagerName;
	IGNORE_RETURN(JavaLibrary::convert(localJstringManagerName, localManagerName));

	ClusterWideDataClient::releaseClusterWideDataLock(localManagerName, static_cast<unsigned long>(lockKey));

	return;
}

//--------------------------------------------------------------------------------------

void JNICALL ScriptMethodsClusterWideDataNamespace::removeClusterWideData(JNIEnv *, jobject, jstring managerName, jstring elementNameRegex, jint lockKey)
{
	JavaStringParam localJstringManagerName(managerName);
	std::string localManagerName;
	IGNORE_RETURN(JavaLibrary::convert(localJstringManagerName, localManagerName));

	JavaStringParam localJstringElementNameRegex(elementNameRegex);
	std::string localElementNameRegex;
	IGNORE_RETURN(JavaLibrary::convert(localJstringElementNameRegex, localElementNameRegex));

	ClusterWideDataClient::removeClusterWideData(localManagerName, localElementNameRegex, static_cast<unsigned long>(lockKey));

	return;
}

//--------------------------------------------------------------------------------------

void JNICALL ScriptMethodsClusterWideDataNamespace::updateClusterWideData(JNIEnv *, jobject, jstring managerName, jstring elementNameRegex, jobject data, jint lockKey)
{
	JavaStringParam localJstringManagerName(managerName);
	std::string localManagerName;
	IGNORE_RETURN(JavaLibrary::convert(localJstringManagerName, localManagerName));

	JavaStringParam localJstringElementNameRegex(elementNameRegex);
	std::string localElementNameRegex;
	IGNORE_RETURN(JavaLibrary::convert(localJstringElementNameRegex, localElementNameRegex));

	ValueDictionary valueDictionary;
	JavaLibrary::convert(data, valueDictionary);

	ClusterWideDataClient::updateClusterWideData(localManagerName, localElementNameRegex, valueDictionary, static_cast<unsigned long>(lockKey));

	return;
}

//--------------------------------------------------------------------------------------

void JNICALL ScriptMethodsClusterWideDataNamespace::replaceClusterWideData(JNIEnv *, jobject, jstring managerName, jstring elementNameRegex, jobject data, jboolean autoRemove, jint lockKey)
{
	JavaStringParam localJstringManagerName(managerName);
	std::string localManagerName;
	IGNORE_RETURN(JavaLibrary::convert(localJstringManagerName, localManagerName));

	JavaStringParam localJstringElementNameRegex(elementNameRegex);
	std::string localElementNameRegex;
	IGNORE_RETURN(JavaLibrary::convert(localJstringElementNameRegex, localElementNameRegex));

	ValueDictionary valueDictionary;
	JavaLibrary::convert(data, valueDictionary);

	ClusterWideDataClient::replaceClusterWideData(localManagerName, localElementNameRegex, valueDictionary, (autoRemove == JNI_TRUE), static_cast<unsigned long>(lockKey));

	return;
}
