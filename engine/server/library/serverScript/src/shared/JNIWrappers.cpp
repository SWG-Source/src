//========================================================================
//
// JNIWrappers.cpp - Classes and functions to wrap the JNI interface
//
// copyright 2004 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JNIWrappers.h"
#include "serverScript/JavaLibrary.h"
#include "sharedFoundation/Crc.h"


const LocalRefParamPtr            LocalRefParam::cms_nullPtr(new LocalRefParam(static_cast<jobject>(0)));
const LocalArrayRefParamPtr       LocalArrayRefParam::cms_nullPtr(new LocalArrayRefParam(static_cast<jarray>(0)));
const LocalObjectArrayRefParamPtr LocalObjectArrayRefParam::cms_nullPtr(new LocalObjectArrayRefParam(static_cast<jobjectArray>(0)));
const LocalRefPtr                 LocalRef::cms_nullPtr(new LocalRef(static_cast<jobject>(0)));
const LocalArrayRefPtr            LocalArrayRef::cms_nullPtr(new LocalArrayRef(static_cast<jarray>(0)));
const LocalObjectArrayRefPtr      LocalObjectArrayRef::cms_nullPtr(new LocalObjectArrayRef(static_cast<jobjectArray>(0)));
const LocalByteArrayRefPtr        LocalByteArrayRef::cms_nullPtr(new LocalByteArrayRef(static_cast<jbyteArray>(0)));
const LocalIntArrayRefPtr         LocalIntArrayRef::cms_nullPtr(new LocalIntArrayRef(static_cast<jintArray>(0)));
const LocalFloatArrayRefPtr       LocalFloatArrayRef::cms_nullPtr(new LocalFloatArrayRef(static_cast<jfloatArray>(0)));
const LocalBooleanArrayRefPtr     LocalBooleanArrayRef::cms_nullPtr(new LocalBooleanArrayRef(static_cast<jbooleanArray>(0)));
const LocalLongArrayRefPtr        LocalLongArrayRef::cms_nullPtr(new LocalLongArrayRef(static_cast<jlongArray>(0)));
const GlobalRefPtr                GlobalRef::cms_nullPtr(new GlobalRef(*LocalRefParam::cms_nullPtr));
const GlobalArrayRefPtr           GlobalArrayRef::cms_nullPtr(new GlobalArrayRef(*LocalObjectArrayRefParam::cms_nullPtr));
const JavaStringParamPtr          JavaStringParam::cms_nullPtr(new JavaStringParam(static_cast<jstring>(0)));
const JavaStringPtr               JavaString::cms_nullPtr(new JavaString(static_cast<jstring>(0)));
const JavaDictionaryPtr           JavaDictionary::cms_nullPtr(new JavaDictionary(static_cast<jobject>(0)));


//========================================================================

namespace JNIWrappersNamespace
{

jboolean isInstanceOf(const LocalRefParam & object, jclass clazz)
{
	if (object.getValue() != 0)
		return JavaLibrary::getEnv()->IsInstanceOf(object.getValue(), clazz);
	return JNI_FALSE;
}

//-----------------------------------------------------------------------

jsize getArrayLength(const LocalArrayRefParam & array)
{
	if (array.getValue() != 0)
		return JavaLibrary::getEnv()->GetArrayLength(array.getValue());
	return 0;
}

//-----------------------------------------------------------------------

jsize getStringLength(const JavaStringParam & string)
{
	if (string.getValue() != 0)
		return JavaLibrary::getEnv()->GetStringLength(string.getValue());
	return 0;
}

//-----------------------------------------------------------------------

LocalRefPtr allocObject(jclass clazz)
{
	LocalRefPtr result(new LocalRef(JavaLibrary::getEnv()->AllocObject(clazz)));
	if (result->getValue() == 0)
		return LocalRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

LocalRefPtr createNewObject(jclass clazz, jmethodID constructorID, ...)
{
	va_list marker;
	va_start(marker, constructorID);

	LocalRefPtr result(new LocalRef(JavaLibrary::getEnv()->NewObjectV(clazz, constructorID, marker)));

	va_end(marker);
	
	if (result->getValue() == 0)
		return LocalRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

JavaStringPtr createNewString(const char * bytes)
{
	if (bytes != nullptr)
	{
		JavaStringPtr result(new JavaString(JavaLibrary::getEnv()->NewStringUTF(bytes)));
		if (result->getValue() != 0)
			return result;
	}
	return JavaString::cms_nullPtr;
}

//-----------------------------------------------------------------------

JavaStringPtr createNewString(const jchar * unicodeChars, jsize len)
{
	if (unicodeChars != nullptr && len >= 0)
	{
		JavaStringPtr result(new JavaString(JavaLibrary::getEnv()->NewString(unicodeChars, len)));
		if (result->getValue() != 0)
			return result;
	}
	return JavaString::cms_nullPtr;
}

//-----------------------------------------------------------------------

JavaDictionaryPtr createNewDictionary()
{
	JavaDictionaryPtr result(new JavaDictionary(JavaLibrary::getEnv()->NewObject(JavaLibrary::ms_clsDictionary, JavaLibrary::ms_midDictionary)));
	if (result->getValue() != 0)
		return result;
	return JavaDictionary::cms_nullPtr;
}

//-----------------------------------------------------------------------

LocalObjectArrayRefPtr createNewObjectArray(jsize length, jclass clazz)
{
	LocalObjectArrayRefPtr result(new LocalObjectArrayRef(JavaLibrary::getEnv()->NewObjectArray(length, clazz, 0)));
	if (result->getValue() == 0)
		return LocalObjectArrayRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

LocalByteArrayRefPtr createNewByteArray(jsize length)
{
	LocalByteArrayRefPtr result(new LocalByteArrayRef(JavaLibrary::getEnv()->NewByteArray(length)));
	if (result->getValue() == 0)
		return LocalByteArrayRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

LocalIntArrayRefPtr createNewIntArray(jsize length)
{
	LocalIntArrayRefPtr result(new LocalIntArrayRef(JavaLibrary::getEnv()->NewIntArray(length)));
	if (result->getValue() == 0)
		return LocalIntArrayRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

LocalFloatArrayRefPtr createNewFloatArray(jsize length)
{
	LocalFloatArrayRefPtr result(new LocalFloatArrayRef(JavaLibrary::getEnv()->NewFloatArray(length)));
	if (result->getValue() == 0)
		return LocalFloatArrayRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

LocalBooleanArrayRefPtr createNewBooleanArray(jsize length)
{
	LocalBooleanArrayRefPtr result(new LocalBooleanArrayRef(JavaLibrary::getEnv()->NewBooleanArray(length)));
	if (result->getValue() == 0)
		return LocalBooleanArrayRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

LocalLongArrayRefPtr createNewLongArray(jsize length)
{
	LocalLongArrayRefPtr result(new LocalLongArrayRef(JavaLibrary::getEnv()->NewLongArray(length)));
	if (result->getValue() == 0)
		return LocalLongArrayRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

LocalRefPtr callObjectMethod(const LocalRefParam & object, jmethodID methodID, ...)
{
	if (object.getValue() != 0)
	{
		va_list marker;
		va_start(marker, methodID);
	
		LocalRefPtr result(new LocalRef(JavaLibrary::getEnv()->CallObjectMethodV(object.getValue(), methodID, marker)));

		va_end(marker);
		
		if (result->getValue() != 0)
			return result;
	}
	return LocalRef::cms_nullPtr;
}

//-----------------------------------------------------------------------

LocalObjectArrayRefPtr callObjectArrayMethod(const LocalRefParam & object, jmethodID methodID, ...)
{
	if (object.getValue() != 0)
	{
		va_list marker;
		va_start(marker, methodID);

		LocalObjectArrayRefPtr result(new LocalObjectArrayRef(static_cast<jobjectArray>(JavaLibrary::getEnv()->CallObjectMethodV(object.getValue(), methodID, marker))));

		va_end(marker);

		if (result->getValue() != 0)
			return result;
	}
	return LocalObjectArrayRef::cms_nullPtr;
}

//-----------------------------------------------------------------------

LocalByteArrayRefPtr callByteArrayMethod(const LocalRefParam & object, jmethodID methodID, ...)
{
	if (object.getValue() != 0)
	{
		va_list marker;
		va_start(marker, methodID);

		LocalByteArrayRefPtr result(new LocalByteArrayRef(static_cast<jbyteArray>(JavaLibrary::getEnv()->CallObjectMethodV(object.getValue(), methodID, marker))));

		va_end(marker);
		
		if (result->getValue() != 0)
			return result;
	}
	return LocalByteArrayRef::cms_nullPtr;
}

//-----------------------------------------------------------------------

jint callIntMethod(const LocalRefParam & object, jmethodID methodID, ...)
{
	if (object.getValue() != 0)
	{
		va_list marker;
		va_start(marker, methodID);

		jint result = JavaLibrary::getEnv()->CallIntMethodV(object.getValue(), methodID, marker);

		va_end(marker); 
		
		return result;
	}
	return 0;
}

//-----------------------------------------------------------------------

jlong callLongMethod(const LocalRefParam & object, jmethodID methodID, ...)
{
	if (object.getValue() != 0)
	{
		va_list marker;
		va_start(marker, methodID);

		jlong result = JavaLibrary::getEnv()->CallLongMethodV(object.getValue(), methodID, marker);

		va_end(marker); 
		
		return result;
	}
	return 0;
}

//-----------------------------------------------------------------------

jfloat callFloatMethod(const LocalRefParam & object, jmethodID methodID, ...)
{
	if (object.getValue() != 0)
	{
		va_list marker;
		va_start(marker, methodID);

		jfloat result = JavaLibrary::getEnv()->CallFloatMethodV(object.getValue(), methodID, marker);

		va_end(marker); 
		
		return result;
	}
	return 0;
}

//-----------------------------------------------------------------------

jboolean callBooleanMethod(const LocalRefParam & object, jmethodID methodID, ...)
{
	if (object.getValue() != 0)
	{
		va_list marker;
		va_start(marker, methodID);

		jboolean result = JavaLibrary::getEnv()->CallBooleanMethodV(object.getValue(), methodID, marker);

		va_end(marker); 
		
		return result;
	}
	return JNI_FALSE;
}

//-----------------------------------------------------------------------

void callVoidMethod(const LocalRefParam & object, jmethodID methodID, ...)
{
	if (object.getValue() != 0)
	{
		va_list marker;
		va_start(marker, methodID);

		JavaLibrary::getEnv()->CallVoidMethodV(object.getValue(), methodID, marker);

		va_end(marker);
	}
}

//-----------------------------------------------------------------------

jboolean callNonvirtualBooleanMethod(const LocalRefParam & object, jclass clazz, jmethodID methodID, ...)
{
	if (object.getValue() != 0)
	{
		va_list marker;
		va_start(marker, methodID);

		jboolean result = JavaLibrary::getEnv()->CallNonvirtualBooleanMethodV(object.getValue(), clazz, methodID, marker);

		va_end(marker);
		
		return result;
	}
	return JNI_FALSE;
}

//-----------------------------------------------------------------------

void callStaticVoidMethod(jclass clazz, jmethodID methodID, ...)
{
	va_list marker;
	va_start(marker, methodID);

	JavaLibrary::getEnv()->CallStaticVoidMethodV(clazz, methodID, marker);

	va_end(marker);
}

//-----------------------------------------------------------------------

LocalRefPtr callStaticObjectMethod(jclass clazz, jmethodID methodID, ...)
{
	va_list marker;
	va_start(marker, methodID);

	LocalRefPtr result(new LocalRef(JavaLibrary::getEnv()->CallStaticObjectMethodV(clazz, methodID, marker)));

	va_end(marker);
	
	if (result->getValue() == 0)
		return LocalRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

LocalObjectArrayRefPtr callStaticObjectArrayMethod(jclass clazz, jmethodID methodID, ...)
{
	va_list marker;
	va_start(marker, methodID);

	LocalObjectArrayRefPtr result(new LocalObjectArrayRef(static_cast<jobjectArray>(JavaLibrary::getEnv()->CallStaticObjectMethodV(clazz, methodID, marker))));

	va_end(marker); 
	
	if (result->getValue() == 0)
		return LocalObjectArrayRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

LocalByteArrayRefPtr callStaticByteArrayMethod(jclass clazz, jmethodID methodID, ...)
{
	va_list marker;
	va_start(marker, methodID);

	LocalByteArrayRefPtr result(new LocalByteArrayRef(static_cast<jbyteArray>(JavaLibrary::getEnv()->CallStaticObjectMethodV(clazz, methodID, marker))));

	va_end(marker);
	
	if (result->getValue() == 0)
		return LocalByteArrayRef::cms_nullPtr;
	return result;
}

//-----------------------------------------------------------------------

LocalRefPtr getObjectArrayElement(const LocalObjectArrayRefParam & array, int index)
{
	if (array.getValue() != 0 && index >= 0)
	{
		LocalRefPtr result(new LocalRef(JavaLibrary::getEnv()->GetObjectArrayElement(array.getValue(), static_cast<jsize>(index))));
		if (result->getValue() != 0)
			return result;
	}
	return LocalRef::cms_nullPtr;
}

//-----------------------------------------------------------------------

LocalRefPtr getObjectField(const LocalRefParam & object, jfieldID fieldID)
{
	if (object.getValue() != 0)
	{
		LocalRefPtr result(new LocalRef(JavaLibrary::getEnv()->GetObjectField(object.getValue(), fieldID)));
		if (result->getValue() != 0)
			return result;
	}
	return LocalRef::cms_nullPtr;
}
//-----------------------------------------------------------------------

LocalObjectArrayRefPtr getArrayObjectField(const LocalRefParam & object, jfieldID fieldID)
{
	if (object.getValue() != 0)
	{
		LocalObjectArrayRefPtr result(new LocalObjectArrayRef(static_cast<jobjectArray>(JavaLibrary::getEnv()->GetObjectField(object.getValue(), fieldID))));
		if (result->getValue() != 0)
			return result;
	}
	return LocalObjectArrayRef::cms_nullPtr;
}

//-----------------------------------------------------------------------

JavaStringPtr callStringMethod(const LocalRefParam & object, jmethodID methodID, ...)
{
	if (object.getValue() != 0)
	{
		va_list marker;
		va_start(marker, methodID);

		jobject result = JavaLibrary::getEnv()->CallObjectMethodV(object.getValue(), methodID, marker);

		va_end(marker);

		if (result != 0)
		{
			if (!JavaLibrary::getEnv()->IsInstanceOf(result, JavaLibrary::ms_clsString))
			{
				WARNING_STRICT_FATAL(true, ("JavaLibrary::callStringMethod called with non-string result"));
				JavaLibrary::getEnv()->DeleteLocalRef(result);
				return JavaString::cms_nullPtr;
			}
			return JavaStringPtr(new JavaString(static_cast<jstring>(result)));
		}
	}
	return JavaString::cms_nullPtr;
}

//-----------------------------------------------------------------------

JavaStringPtr callStaticStringMethod(jclass clazz, jmethodID methodID, ...)
{
	va_list marker;
	va_start(marker, methodID);

	JavaStringPtr result(new JavaString(static_cast<jstring>(JavaLibrary::getEnv()->CallStaticObjectMethodV(clazz,
		methodID, marker))));

	va_end(marker);
	
	if (result->getValue() != 0)
		return result;
	return JavaString::cms_nullPtr;
}

//-----------------------------------------------------------------------

JavaStringPtr getStringArrayElement(const LocalObjectArrayRefParam & array, int index)
{
	if (array.getValue() != 0 && index >= 0)
	{
		JavaStringPtr result(new JavaString(static_cast<jstring>(JavaLibrary::getEnv()->GetObjectArrayElement(array.getValue(),
			static_cast<jsize>(index)))));
		if (result->getValue() != 0)
			return result;
	}
	return JavaString::cms_nullPtr;
}

//-----------------------------------------------------------------------

LocalArrayRefPtr getArrayArrayElement(const LocalObjectArrayRefParam & array, int index)
{
	if (array.getValue() != 0 && index >= 0)
	{
		LocalArrayRefPtr result(new LocalArrayRef(static_cast<jarray>(JavaLibrary::getEnv()->GetObjectArrayElement(array.getValue(),
			static_cast<jsize>(index)))));
		if (result->getValue() != 0)
			return result;
	}
	return LocalArrayRef::cms_nullPtr;
}

//-----------------------------------------------------------------------

LocalObjectArrayRefPtr getObjectArrayArrayElement(const LocalObjectArrayRefParam & array, int index)
{
	if (array.getValue() != 0 && index >= 0)
	{
		LocalObjectArrayRefPtr result(new LocalObjectArrayRef(static_cast<jobjectArray>(JavaLibrary::getEnv()->GetObjectArrayElement(array.getValue(),
			static_cast<jsize>(index)))));
		if (result->getValue() != 0)
			return result;
	}
	return LocalObjectArrayRef::cms_nullPtr;
}

//-----------------------------------------------------------------------

LocalIntArrayRefPtr getIntArrayArrayElement(const LocalObjectArrayRefParam & array, int index)
{
	if (array.getValue() != 0 && index >= 0)
	{
		LocalIntArrayRefPtr result(new LocalIntArrayRef(static_cast<jintArray>(JavaLibrary::getEnv()->GetObjectArrayElement(array.getValue(),
			static_cast<jsize>(index)))));
		if (result->getValue() != 0)
			return result;
	}
	return LocalIntArrayRef::cms_nullPtr;
}

//-----------------------------------------------------------------------

LocalFloatArrayRefPtr getFloatArrayArrayElement(const LocalObjectArrayRefParam & array, int index)
{
	if (array.getValue() != 0 && index >= 0)
	{
		LocalFloatArrayRefPtr result(new LocalFloatArrayRef(static_cast<jfloatArray>(JavaLibrary::getEnv()->GetObjectArrayElement(array.getValue(),
			static_cast<jsize>(index)))));
		if (result->getValue() != 0)
			return result;
	}
	return LocalFloatArrayRef::cms_nullPtr;
}

//-----------------------------------------------------------------------

JavaStringPtr getStringField(const LocalRefParam & object, jfieldID fieldID)
{
	if (object.getValue() != 0)
	{
		JavaStringPtr result(new JavaString(static_cast<jstring>(JavaLibrary::getEnv()->GetObjectField(object.getValue(), fieldID))));
		if (result->getValue() != 0)
			return result;
	}
	return JavaString::cms_nullPtr;
}

//-----------------------------------------------------------------------

void setObjectArrayElement(const LocalObjectArrayRefParam & array, jsize index, const LocalRefParam & value)
{
	if (array.getValue() != 0 && index >= 0)
	{
		if (index >= JavaLibrary::getEnv()->GetArrayLength(array.getValue()))
		{
#ifdef _DEBUG 
			WARNING(true, ("JavaLibrary::setObjectArrayElement array index %d "
				"out of bounds", static_cast<int>(index)));
#endif
			return;
		}
		JavaLibrary::getEnv()->SetObjectArrayElement(array.getValue(), index, value.getValue());
	}
}

//-----------------------------------------------------------------------

void setObjectField(const LocalRefParam & object, jfieldID fieldID, const LocalRefParam & value)
{
	if (object.getValue() != 0)
	{
		JavaLibrary::getEnv()->SetObjectField(object.getValue(), fieldID, value.getValue());
	}
}

//-----------------------------------------------------------------------

jint getIntField(const LocalRefParam & object, jfieldID fieldID)
{
	if (object.getValue() != 0)
	{
		return JavaLibrary::getEnv()->GetIntField(object.getValue(), fieldID);
	}
	return 0;
}

//-----------------------------------------------------------------------

jfloat getFloatField(const LocalRefParam & object, jfieldID fieldID)
{
	if (object.getValue() != 0)
	{
		return JavaLibrary::getEnv()->GetFloatField(object.getValue(), fieldID);
	}
	return 0;
}

//-----------------------------------------------------------------------

jboolean getBooleanField(const LocalRefParam & object, jfieldID fieldID)
{
	if (object.getValue() != 0)
	{
		return JavaLibrary::getEnv()->GetBooleanField(object.getValue(), fieldID);
	}
	return false;
}

//-----------------------------------------------------------------------

void setIntField(const LocalRefParam & object, jfieldID fieldID, jint value)
{
	if (object.getValue() != 0)
	{
		JavaLibrary::getEnv()->SetIntField(object.getValue(), fieldID, value);
	}
}

//-----------------------------------------------------------------------

void setFloatField(const LocalRefParam & object, jfieldID fieldID, jfloat value)
{
	if (object.getValue() != 0)
	{
		JavaLibrary::getEnv()->SetFloatField(object.getValue(), fieldID, value);
	}
}

//-----------------------------------------------------------------------

void setBooleanField(const LocalRefParam & object, jfieldID fieldID, jboolean value)
{
	if (object.getValue() != 0)
	{
		JavaLibrary::getEnv()->SetBooleanField(object.getValue(), fieldID, value);
	}
}

//-----------------------------------------------------------------------

void getByteArrayRegion(const LocalByteArrayRef & array, jsize start, jsize length, jbyte * buf)
{
	if (array.getValue() != 0)
	{
		JavaLibrary::getEnv()->GetByteArrayRegion(array.getValue(), start, length, buf);
	}
}

//-----------------------------------------------------------------------

void getIntArrayRegion(const LocalIntArrayRef & array, jsize start, jsize length, jint * buf)
{
	if (array.getValue() != 0)
	{
		JavaLibrary::getEnv()->GetIntArrayRegion(array.getValue(), start, length, buf);
	}
}

//-----------------------------------------------------------------------

void getFloatArrayRegion(const LocalFloatArrayRef & array, jsize start, jsize length, jfloat * buf)
{
	if (array.getValue() != 0)
	{
		JavaLibrary::getEnv()->GetFloatArrayRegion(array.getValue(), start, length, buf);
	}
}

//-----------------------------------------------------------------------

void getBooleanArrayRegion(const LocalBooleanArrayRef & array, jsize start, jsize length, jboolean * buf)
{
	if (array.getValue() != 0)
	{
		JavaLibrary::getEnv()->GetBooleanArrayRegion(array.getValue(), start, length, buf);
	}
}

//-----------------------------------------------------------------------

void getLongArrayRegion(const LocalLongArrayRef & array, jsize start, jsize length, jlong * buf)
{
	if (array.getValue() != 0)
	{
		JavaLibrary::getEnv()->GetLongArrayRegion(array.getValue(), start, length, buf);
	}
}

//-----------------------------------------------------------------------

void setByteArrayRegion(const LocalByteArrayRef & array, jsize start, jsize length, jbyte * buf)
{
	if (array.getValue() != 0)
	{
		JavaLibrary::getEnv()->SetByteArrayRegion(array.getValue(), start, length, buf);
	}
}

//-----------------------------------------------------------------------

void setIntArrayRegion(const LocalIntArrayRef & array, jsize start, jsize length, jint * buf)
{
	if (array.getValue() != 0)
	{
		JavaLibrary::getEnv()->SetIntArrayRegion(array.getValue(), start, length, buf);
	}
}

//-----------------------------------------------------------------------

void setFloatArrayRegion(const LocalFloatArrayRef & array, jsize start, jsize length, jfloat * buf)
{
	if (array.getValue() != 0)
	{
		JavaLibrary::getEnv()->SetFloatArrayRegion(array.getValue(), start, length, buf);
	}
}

//-----------------------------------------------------------------------

void setBooleanArrayRegion(const LocalBooleanArrayRef & array, jsize start, jsize length, jboolean * buf)
{
	if (array.getValue() != 0)
	{
		JavaLibrary::getEnv()->SetBooleanArrayRegion(array.getValue(), start, length, buf);
	}
}

//-----------------------------------------------------------------------

void setLongArrayRegion(const LocalLongArrayRef & array, jsize start, jsize length, jlong * buf)
{
	if (array.getValue() != 0)
	{
		JavaLibrary::getEnv()->SetLongArrayRegion(array.getValue(), start, length, buf);
	}
}

//-----------------------------------------------------------------------

/**
 * Converts a Java array of strings to a vector of const char *. NOTE: The caller is
 * responsible for freeing the returned strings.
 */
void getStringArray(const LocalObjectArrayRefParam & array, std::vector<const char *> & strings)
{
	jsize count = getArrayLength(array);
	strings.resize(count, 0);

	jstring stringElement;
	jsize stringLength;
	jsize bufferLength;
	char * newString;

	JNIEnv * env = JavaLibrary::getEnv();
	jobjectArray arrayRef = array.getValue();
	std::vector<const char *>::iterator stringsIter = strings.begin();
	for (jsize i = 0; i < count; ++i, ++stringsIter)
	{
		if (*stringsIter != 0)
			delete[] const_cast<char *>(*stringsIter);

		newString = 0;
		stringElement = static_cast<jstring>(env->GetObjectArrayElement(arrayRef, i));
		if (stringElement != 0)
		{
			stringLength = env->GetStringLength(stringElement);
			bufferLength = env->GetStringUTFLength(stringElement);
			newString = new char[bufferLength+1];
			newString[bufferLength] = 0;
			env->GetStringUTFRegion(stringElement, 0, stringLength, newString);
			env->DeleteLocalRef(stringElement);
		}
		*stringsIter = newString;
	}
}

//-----------------------------------------------------------------------

/**
 * Converts a Java array of strings to a vector of const Unicode::String *. NOTE:
 * The caller is responsible for freeing the returned strings.
 */
void getStringArray(const LocalObjectArrayRefParam & array, std::vector<const Unicode::String *> & strings)
{
	jsize count = getArrayLength(array);
	strings.resize(count, 0);

	jsize stringLength;
	jstring stringElement;
	const jchar * newString;

	JNIEnv * env = JavaLibrary::getEnv();
	jobjectArray arrayRef = array.getValue();
	std::vector<const Unicode::String *>::iterator stringsIter = strings.begin();
	for (jsize i = 0; i < count; ++i, ++stringsIter)
	{
		delete *stringsIter;

		stringElement = static_cast<jstring>(env->GetObjectArrayElement(arrayRef, i));
		if (stringElement != 0)
		{
			stringLength = env->GetStringLength(stringElement);
			newString = env->GetStringCritical(stringElement, 0);
			*stringsIter = new Unicode::String(newString, stringLength);
			env->ReleaseStringCritical(stringElement, newString);
			env->DeleteLocalRef(stringElement);
		}
		else
			*stringsIter = 0;
	}
}


//-----------------------------------------------------------------------

}	// namespace JNIWrappersNamespace


//========================================================================

LocalRefParam::LocalRefParam(jobject src) :
	m_ref(src)
{
}

LocalRefParam::~LocalRefParam()
{
}

jobject LocalRefParam::getReturnValue(void)
{
	jobject return_val = m_ref;
	m_ref = nullptr;
	return return_val;
}

/**
 * Only call this function if you really know what you're doing! Memory leaks
 * may result!
 */
void LocalRefParam::emergencyClear()
{
	m_ref = nullptr;
}


//========================================================================

LocalArrayRefParam::LocalArrayRefParam(jarray src) :
	LocalRefParam(src)
{
}

LocalArrayRefParam::~LocalArrayRefParam()
{
}


//========================================================================

LocalObjectArrayRefParam::LocalObjectArrayRefParam(jobjectArray src) :
	LocalArrayRefParam(src)
{
}

LocalObjectArrayRefParam::~LocalObjectArrayRefParam()
{
}


//========================================================================

LocalRef::LocalRef(jobject src) :
	LocalRefParam(src)
{
}

LocalRef::~LocalRef()
{
	if (m_ref != nullptr && JavaLibrary::getEnv() != nullptr) 
		JavaLibrary::getEnv()->DeleteLocalRef(m_ref);
	m_ref = nullptr;
}


//========================================================================

LocalArrayRef::LocalArrayRef(jarray src) :
	LocalArrayRefParam(src)
{
}

LocalArrayRef::~LocalArrayRef()
{
	if (m_ref != nullptr && JavaLibrary::getEnv() != nullptr)
		JavaLibrary::getEnv()->DeleteLocalRef(m_ref);
	m_ref = nullptr;
}


//========================================================================

LocalObjectArrayRef::LocalObjectArrayRef(jobjectArray src) :
	LocalObjectArrayRefParam(src)
{
}

LocalObjectArrayRef::~LocalObjectArrayRef()
{
	if (m_ref != nullptr && JavaLibrary::getEnv() != nullptr)
		JavaLibrary::getEnv()->DeleteLocalRef(m_ref);
	m_ref = nullptr;
}


//========================================================================

LocalByteArrayRef::LocalByteArrayRef(jbyteArray src) :
	LocalArrayRef(src)
{
}

LocalByteArrayRef::~LocalByteArrayRef()
{
}


//========================================================================

LocalIntArrayRef::LocalIntArrayRef(jintArray src) :
	LocalArrayRef(src)
{
}

LocalIntArrayRef::~LocalIntArrayRef()
{
}

//========================================================================

LocalFloatArrayRef::LocalFloatArrayRef(jfloatArray src) :
	LocalArrayRef(src)
{
}

LocalFloatArrayRef::~LocalFloatArrayRef()
{
}


//========================================================================

LocalBooleanArrayRef::LocalBooleanArrayRef(jbooleanArray src) :
	LocalArrayRef(src)
{
}

LocalBooleanArrayRef::~LocalBooleanArrayRef()
{
}


//========================================================================

LocalLongArrayRef::LocalLongArrayRef(jlongArray src) :
	LocalArrayRef(src)
{
}

LocalLongArrayRef::~LocalLongArrayRef()
{
}


//========================================================================

GlobalRef::GlobalRef(const LocalRefParam & src) :
	LocalRefParam(static_cast<jobject>(0))
{
	if (src.getValue() != 0 && JavaLibrary::getEnv() != nullptr)
		m_ref = JavaLibrary::getEnv()->NewGlobalRef(src.getValue());
}

GlobalRef::~GlobalRef()
{
	if (m_ref != nullptr && JavaLibrary::getEnv() != nullptr)
		JavaLibrary::getEnv()->DeleteGlobalRef(m_ref);
	m_ref = nullptr;
}


//========================================================================

GlobalArrayRef::GlobalArrayRef(const LocalObjectArrayRefParam & src) :
	LocalObjectArrayRefParam(static_cast<jobjectArray>(0))
{
	if (src.getValue() != 0 && JavaLibrary::getEnv() != nullptr)
		m_ref = JavaLibrary::getEnv()->NewGlobalRef(src.getValue());
}

GlobalArrayRef::~GlobalArrayRef()
{
	if (m_ref != nullptr && JavaLibrary::getEnv() != nullptr)
		JavaLibrary::getEnv()->DeleteGlobalRef(m_ref);
	m_ref = nullptr;
}

//========================================================================

JavaStringParam::JavaStringParam(jstring src) :
	LocalRefParam(src)
{
}

JavaStringParam::JavaStringParam(const LocalRefParam & src) :
	LocalRefParam(src.getValue())
{
}

JavaStringParam::~JavaStringParam()
{
}

int JavaStringParam::fillBuffer(char * buffer, int size) const
{
	if (m_ref != nullptr && buffer != nullptr && JavaLibrary::getEnv() != nullptr)
	{
		// Get the number of storage bytes required to convert this Java string into a UTF-8 string.
		// Include the terminating nullptr byte in the required buffer size.
		int requiredBufferSize = JavaLibrary::getEnv()->GetStringUTFLength(static_cast<jstring>(m_ref)) + 1;
		if (requiredBufferSize <= size)
		{
			int stringLength = JavaLibrary::getEnv()->GetStringLength(static_cast<jstring>(m_ref));

			JavaLibrary::getEnv()->GetStringUTFRegion(static_cast<jstring>(m_ref), 0, stringLength, buffer);

			// Null terminate the string.  requiredBufferSize already includes the byte count for the nullptr terminator.
			buffer[requiredBufferSize - 1] = '\0';

			return requiredBufferSize;
		}
	}
	return 0;
}


//========================================================================

JavaString::JavaString(jstring src) :
	JavaStringParam(src)
{
}

JavaString::JavaString(const char * src) :
	JavaStringParam(JavaLibrary::getEnv()->NewStringUTF(src != nullptr ? src : ""))
{
}

JavaString::JavaString(const std::string & src) :
	JavaStringParam(JavaLibrary::getEnv()->NewStringUTF(src.c_str()))
{
}

JavaString::JavaString(const Unicode::String & src) :
	JavaStringParam(JavaLibrary::getEnv()->NewString(src.data(), src.size()))
{
}

JavaString::~JavaString()
{
	if (m_ref != nullptr && JavaLibrary::getEnv() != nullptr) 
		JavaLibrary::getEnv()->DeleteLocalRef(m_ref);
	m_ref = nullptr;
}


//========================================================================

JavaDictionary::JavaDictionary(jobject src) :
	LocalRef(src)
{
}

JavaDictionary::~JavaDictionary()
{
}

/**
 * Converts the data in a JavaDictionary to a string.
 */
void JavaDictionary::serialize(void)
{
	JavaLibrary::convert(*this, m_serializedData);
	if (!m_serializedData.empty())
		m_crc = Crc::calculate(&m_serializedData[0], m_serializedData.size());
	else
		m_crc = 0;
}

