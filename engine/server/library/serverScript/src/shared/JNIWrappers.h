//========================================================================
//
// JNIWrappers.h - Classes and functions to wrap the JNI interface
//
// copyright 2004 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_JNIWrappers_H
#define _INCLUDED_JNIWrappers_H

#include <jni.h>
#include "serverScript/ScriptDictionary.h"
#include <memory>

namespace boost
{
	template <class T> class shared_ptr;
}

class LocalRefParam;
class LocalArrayRefParam;
class LocalObjectArrayRefParam;
class LocalRef;
class LocalArrayRef;
class LocalObjectArrayRef;
class LocalByteArrayRef;
class LocalIntArrayRef;
class LocalFloatArrayRef;
class LocalBooleanArrayRef;
class LocalLongArrayRef;
class GlobalRef;
class GlobalArrayRef;
class JavaStringParam;
class JavaString;
class JavaDictionary;
typedef std::shared_ptr<LocalRefParam>            LocalRefParamPtr;
typedef std::shared_ptr<LocalArrayRefParam>       LocalArrayRefParamPtr;
typedef std::shared_ptr<LocalObjectArrayRefParam> LocalObjectArrayRefParamPtr;
typedef std::shared_ptr<LocalRef>                 LocalRefPtr;
typedef std::shared_ptr<LocalArrayRef>            LocalArrayRefPtr;
typedef std::shared_ptr<LocalObjectArrayRef>      LocalObjectArrayRefPtr;
typedef std::shared_ptr<LocalByteArrayRef>        LocalByteArrayRefPtr;
typedef std::shared_ptr<LocalIntArrayRef>         LocalIntArrayRefPtr;
typedef std::shared_ptr<LocalFloatArrayRef>       LocalFloatArrayRefPtr;
typedef std::shared_ptr<LocalBooleanArrayRef>     LocalBooleanArrayRefPtr;
typedef std::shared_ptr<LocalLongArrayRef>        LocalLongArrayRefPtr;
typedef std::shared_ptr<GlobalRef>                GlobalRefPtr;
typedef std::shared_ptr<GlobalArrayRef>           GlobalArrayRefPtr;
typedef std::shared_ptr<JavaStringParam>          JavaStringParamPtr;
typedef std::shared_ptr<JavaString>               JavaStringPtr;
typedef std::shared_ptr<JavaDictionary>           JavaDictionaryPtr;


//========================================================================

namespace JNIWrappersNamespace
{
jboolean                isInstanceOf(const LocalRefParam & object, jclass clazz);
jsize                   getArrayLength(const LocalArrayRefParam & array);
jsize                   getStringLength(const JavaStringParam & string);
LocalRefPtr             allocObject(jclass clazz);
LocalRefPtr             createNewObject(jclass clazz, jmethodID constructorID, ...);
JavaStringPtr           createNewString(const char * bytes);
JavaStringPtr           createNewString(const jchar * unicodeChars, jsize len);
JavaDictionaryPtr       createNewDictionary();
LocalObjectArrayRefPtr  createNewObjectArray(jsize length, jclass clazz);
LocalByteArrayRefPtr    createNewByteArray(jsize length);
LocalIntArrayRefPtr     createNewIntArray(jsize length);
LocalFloatArrayRefPtr   createNewFloatArray(jsize length);
LocalBooleanArrayRefPtr createNewBooleanArray(jsize length);
LocalLongArrayRefPtr    createNewLongArray(jsize length);
LocalRefPtr             callObjectMethod(const LocalRefParam & object, jmethodID methodID, ...);
LocalObjectArrayRefPtr  callObjectArrayMethod(const LocalRefParam & object, jmethodID methodID, ...);
LocalByteArrayRefPtr    callByteArrayMethod(const LocalRefParam & object, jmethodID methodID, ...);
jint                    callIntMethod(const LocalRefParam & object, jmethodID methodID, ...);
jlong                   callLongMethod(const LocalRefParam & object, jmethodID methodID, ...);
jfloat                  callFloatMethod(const LocalRefParam & object, jmethodID methodID, ...);
jboolean                callBooleanMethod(const LocalRefParam & object, jmethodID methodID, ...);
void                    callVoidMethod(const LocalRefParam & object, jmethodID methodID, ...);
jboolean                callNonvirtualBooleanMethod(const LocalRefParam & object, jclass clazz, jmethodID methodID, ...);
void                    callStaticVoidMethod(jclass clazz, jmethodID methodID, ...);
LocalRefPtr             callStaticObjectMethod(jclass clazz, jmethodID methodID, ...);
LocalObjectArrayRefPtr  callStaticObjectArrayMethod(jclass clazz, jmethodID methodID, ...);
LocalByteArrayRefPtr    callStaticByteArrayMethod(jclass clazz, jmethodID methodID, ...);
JavaStringPtr           callStringMethod(const LocalRefParam & object, jmethodID methodID, ...);
JavaStringPtr           callStaticStringMethod(jclass clazz, jmethodID methodID, ...);
LocalRefPtr             getObjectArrayElement(const LocalObjectArrayRefParam & array, int index);
JavaStringPtr           getStringArrayElement(const LocalObjectArrayRefParam & array, int index);
LocalArrayRefPtr        getArrayArrayElement(const LocalObjectArrayRefParam & array, int index);
LocalObjectArrayRefPtr  getObjectArrayArrayElement(const LocalObjectArrayRefParam & array, int index);
LocalIntArrayRefPtr     getIntArrayArrayElement(const LocalObjectArrayRefParam & array, int index);
LocalFloatArrayRefPtr   getFloatArrayArrayElement(const LocalObjectArrayRefParam & array, int index);
LocalRefPtr             getObjectField(const LocalRefParam & object, jfieldID fieldID);
LocalObjectArrayRefPtr  getArrayObjectField(const LocalRefParam & object, jfieldID fieldID);
JavaStringPtr           getStringField(const LocalRefParam & object, jfieldID fieldID);
void                    setObjectArrayElement(const LocalObjectArrayRefParam & array, jsize index, const LocalRefParam & value);
void                    setObjectField(const LocalRefParam & object, jfieldID fieldID, const LocalRefParam & value);
jint                    getIntField(const LocalRefParam & object, jfieldID fieldID);
jfloat                  getFloatField(const LocalRefParam & object, jfieldID fieldID);
jboolean                getBooleanField(const LocalRefParam & object, jfieldID fieldID);
void                    setIntField(const LocalRefParam & object, jfieldID fieldID, jint value);
void                    setFloatField(const LocalRefParam & object, jfieldID fieldID, jfloat value);
void                    setBooleanField(const LocalRefParam & object, jfieldID fieldID, jboolean value);
void                    getByteArrayRegion(const LocalByteArrayRef & array, jsize start, jsize length, jbyte * buf);
void                    getIntArrayRegion(const LocalIntArrayRef & array, jsize start, jsize length, jint * buf);
void                    getFloatArrayRegion(const LocalFloatArrayRef & array, jsize start, jsize length, jfloat * buf);
void                    getBooleanArrayRegion(const LocalBooleanArrayRef & array, jsize start, jsize length, jboolean * buf);
void                    getLongArrayRegion(const LocalLongArrayRef & array, jsize start, jsize length, jlong * buf);
void                    setByteArrayRegion(const LocalByteArrayRef & array, jsize start, jsize length, jbyte * buf);
void                    setIntArrayRegion(const LocalIntArrayRef & array, jsize start, jsize length, jint * buf);
void                    setFloatArrayRegion(const LocalFloatArrayRef & array, jsize start, jsize length, jfloat * buf);
void                    setBooleanArrayRegion(const LocalBooleanArrayRef & array, jsize start, jsize length, jboolean * buf);
void                    setLongArrayRegion(const LocalLongArrayRef & array, jsize start, jsize length, jlong * buf);

// these functions don't match a JNI function, but are useful for optimization reasons
void                    getStringArray(const LocalObjectArrayRefParam & array, std::vector<const char *> & strings);
void                    getStringArray(const LocalObjectArrayRefParam & array, std::vector<const Unicode::String *> & strings);
}


//========================================================================

/**
 * Wraps a jobject passed into a JNI callback function from Java.
 */
class LocalRefParam
{
public:
	static const LocalRefParamPtr cms_nullPtr;

public:
	explicit  LocalRefParam(jobject src);
	virtual  ~LocalRefParam();

	// getValue should be used when passing the wrapped object to a JNI function
	jobject   getValue(void) const;
	// getReturnValue should be used when returning the wrapped object from a
	// JNICALL function back to Java
	jobject   getReturnValue(void);

	void      emergencyClear();

protected:
	jobject m_ref;

private:
	LocalRefParam();
	LocalRefParam(const LocalRefParam &);
};

// ----------------------------------------------------------------------

inline jobject LocalRefParam::getValue(void) const
{
	return m_ref;
}


//========================================================================

/**
 * Wraps a jarray passed into a JNI callback function from Java.
 */
class LocalArrayRefParam : public LocalRefParam
{
public:
	static const LocalArrayRefParamPtr cms_nullPtr;

public:
	explicit  LocalArrayRefParam(jarray src);
	virtual  ~LocalArrayRefParam();

	// getValue should be used when passing the wrapped object to a JNI function
	jarray   getValue(void) const;
	// getReturnValue should be used when returning the wrapped object from a
	// JNICALL function back to Java
	jarray   getReturnValue(void);

private:
	LocalArrayRefParam();
	LocalArrayRefParam(const LocalArrayRefParam &);
};

// ----------------------------------------------------------------------

inline jarray LocalArrayRefParam::getValue(void) const
{
	return static_cast<jarray>(LocalRefParam::getValue());
}

inline jarray LocalArrayRefParam::getReturnValue(void)
{
	return static_cast<jarray>(LocalRefParam::getReturnValue());
}


//========================================================================

/**
 * Wraps a jobjectArray passed into a JNI callback function from Java.
 */
class LocalObjectArrayRefParam : public LocalArrayRefParam
{
public:
	static const LocalObjectArrayRefParamPtr cms_nullPtr;

public:
	explicit  LocalObjectArrayRefParam(jobjectArray src);
	virtual  ~LocalObjectArrayRefParam();

	// getValue should be used when passing the wrapped object to a JNI function
	jobjectArray   getValue(void) const;
	// getReturnValue should be used when returning the wrapped object from a
	// JNICALL function back to Java
	jobjectArray   getReturnValue(void);

private:
	LocalObjectArrayRefParam();
	LocalObjectArrayRefParam(const LocalObjectArrayRefParam &);
};

// ----------------------------------------------------------------------

inline jobjectArray LocalObjectArrayRefParam::getValue(void) const
{
	return static_cast<jobjectArray>(LocalArrayRefParam::getValue());
}

inline jobjectArray LocalObjectArrayRefParam::getReturnValue(void)
{
	return static_cast<jobjectArray>(LocalArrayRefParam::getReturnValue());
}

//========================================================================

/**
 * Wraps a jobject created/returned by a JNI function.
 */
class LocalRef : public LocalRefParam
{
public:
	static const LocalRefPtr cms_nullPtr;

public:
	explicit  LocalRef(jobject src);
	virtual  ~LocalRef();

private:
	LocalRef();
	LocalRef(const LocalRef &);
};


//========================================================================

/**
 * Wraps a jarray created/returned by a JNI function.
 */
class LocalArrayRef : public LocalArrayRefParam
{
public:
	static const LocalArrayRefPtr cms_nullPtr;

public:
	explicit  LocalArrayRef(jarray src);
	virtual  ~LocalArrayRef();

private:
	LocalArrayRef();
	LocalArrayRef(const LocalArrayRef &);
};


//========================================================================

/**
 * Wraps a jobjectArray created/returned by a JNI function.
 */
class LocalObjectArrayRef : public LocalObjectArrayRefParam
{
public:
	static const LocalObjectArrayRefPtr cms_nullPtr;

public:
	explicit  LocalObjectArrayRef(jobjectArray src);
	virtual  ~LocalObjectArrayRef();

private:
	LocalObjectArrayRef();
	LocalObjectArrayRef(const LocalObjectArrayRef &);
};


//========================================================================

/**
 * Wraps a jbyteArray created/returned by a JNI function.
 */
class LocalByteArrayRef : public LocalArrayRef
{
public:
	static const LocalByteArrayRefPtr cms_nullPtr;

public:
	explicit  LocalByteArrayRef(jbyteArray src);
	virtual  ~LocalByteArrayRef();

	// getValue should be used when passing the wrapped object to a JNI function
	jbyteArray   getValue(void) const;
	// getReturnValue should be used when returning the wrapped object from a
	// JNICALL function back to Java
	jbyteArray   getReturnValue(void);

private:
	LocalByteArrayRef();
	LocalByteArrayRef(const LocalByteArrayRef &);
};

// ----------------------------------------------------------------------

inline jbyteArray LocalByteArrayRef::getValue(void) const
{
	return static_cast<jbyteArray>(LocalArrayRef::getValue());
}

inline jbyteArray LocalByteArrayRef::getReturnValue(void)
{
	return static_cast<jbyteArray>(LocalArrayRef::getReturnValue());
}


//========================================================================

/**
 * Wraps a jintArray created/returned by a JNI function.
 */
class LocalIntArrayRef : public LocalArrayRef
{
public:
	static const LocalIntArrayRefPtr cms_nullPtr;

public:
	explicit  LocalIntArrayRef(jintArray src);
	virtual  ~LocalIntArrayRef();

	// getValue should be used when passing the wrapped object to a JNI function
	jintArray   getValue(void) const;
	// getReturnValue should be used when returning the wrapped object from a
	// JNICALL function back to Java
	jintArray   getReturnValue(void);

private:
	LocalIntArrayRef();
	LocalIntArrayRef(const LocalIntArrayRef &);
};

// ----------------------------------------------------------------------

inline jintArray LocalIntArrayRef::getValue(void) const
{
	return static_cast<jintArray>(LocalArrayRef::getValue());
}

inline jintArray LocalIntArrayRef::getReturnValue(void)
{
	return static_cast<jintArray>(LocalArrayRef::getReturnValue());
}


//========================================================================

/**
 * Wraps a jfloatArray created/returned by a JNI function.
 */
class LocalFloatArrayRef : public LocalArrayRef
{
public:
	static const LocalFloatArrayRefPtr cms_nullPtr;

public:
	explicit  LocalFloatArrayRef(jfloatArray src);
	virtual  ~LocalFloatArrayRef();

	// getValue should be used when passing the wrapped object to a JNI function
	jfloatArray   getValue(void) const;
	// getReturnValue should be used when returning the wrapped object from a
	// JNICALL function back to Java
	jfloatArray   getReturnValue(void);

private:
	LocalFloatArrayRef();
	LocalFloatArrayRef(const LocalFloatArrayRef &);
};

// ----------------------------------------------------------------------

inline jfloatArray LocalFloatArrayRef::getValue(void) const
{
	return static_cast<jfloatArray>(LocalArrayRef::getValue());
}

inline jfloatArray LocalFloatArrayRef::getReturnValue(void)
{
	return static_cast<jfloatArray>(LocalArrayRef::getReturnValue());
}


//========================================================================

/**
 * Wraps a jbooleanArray created/returned by a JNI function.
 */
class LocalBooleanArrayRef : public LocalArrayRef
{
public:
	static const LocalBooleanArrayRefPtr cms_nullPtr;

public:
	explicit  LocalBooleanArrayRef(jbooleanArray src);
	virtual  ~LocalBooleanArrayRef();

	// getValue should be used when passing the wrapped object to a JNI function
	jbooleanArray   getValue(void) const;
	// getReturnValue should be used when returning the wrapped object from a
	// JNICALL function back to Java
	jbooleanArray   getReturnValue(void);

private:
	LocalBooleanArrayRef();
	LocalBooleanArrayRef(const LocalBooleanArrayRef &);
};

// ----------------------------------------------------------------------

inline jbooleanArray LocalBooleanArrayRef::getValue(void) const
{
	return static_cast<jbooleanArray>(LocalArrayRef::getValue());
}

inline jbooleanArray LocalBooleanArrayRef::getReturnValue(void)
{
	return static_cast<jbooleanArray>(LocalArrayRef::getReturnValue());
}


//========================================================================

/**
 * Wraps a jlongArray created/returned by a JNI function.
 */
class LocalLongArrayRef : public LocalArrayRef
{
public:
	static const LocalLongArrayRefPtr cms_nullPtr;

public:
	explicit  LocalLongArrayRef(jlongArray src);
	virtual  ~LocalLongArrayRef();

	// getValue should be used when passing the wrapped object to a JNI function
	jlongArray   getValue(void) const;
	// getReturnValue should be used when returning the wrapped object from a
	// JNICALL function back to Java
	jlongArray   getReturnValue(void);

private:
	LocalLongArrayRef();
	LocalLongArrayRef(const LocalLongArrayRef &);
};

// ----------------------------------------------------------------------

inline jlongArray LocalLongArrayRef::getValue(void) const
{
	return static_cast<jlongArray>(LocalArrayRef::getValue());
}

inline jlongArray LocalLongArrayRef::getReturnValue(void)
{
	return static_cast<jlongArray>(LocalArrayRef::getReturnValue());
}


//========================================================================

/**
 * Creates a global reference from a local reference.
 */
class GlobalRef : public LocalRefParam
{
public:
	static const GlobalRefPtr cms_nullPtr;

public:
	explicit  GlobalRef(const LocalRefParam & src);
	virtual  ~GlobalRef();

private:
	GlobalRef();
	GlobalRef(const GlobalRef & src);
};


//========================================================================

/**
 * Creates a global array reference from a local array reference.
 */
class GlobalArrayRef : public LocalObjectArrayRefParam
{
public:
	static const GlobalArrayRefPtr cms_nullPtr;

public:
	explicit  GlobalArrayRef(const LocalObjectArrayRefParam & src);
	virtual  ~GlobalArrayRef();

private:
	GlobalArrayRef();
	GlobalArrayRef(const GlobalArrayRef &);
};


//========================================================================

/**
 * Wraps a jstring passed into a JNI callback function from Java.
 */
class JavaStringParam : public LocalRefParam
{
public:
	static const JavaStringParamPtr cms_nullPtr;

public:
	explicit  JavaStringParam(jstring src);
	explicit  JavaStringParam(const LocalRefParam & src);
	virtual  ~JavaStringParam();

	// getValue should be used when passing the wrapped object to a JNI function
	jstring   getValue(void) const;
	// getReturnValue should be used when returning the wrapped object from a
	// JNICALL function back to Java
	jstring   getReturnValue(void);

	int       fillBuffer(char * buffer, int size) const;

private:
	JavaStringParam();
	JavaStringParam(const JavaStringParam & src);
};

// ----------------------------------------------------------------------

inline jstring JavaStringParam::getValue(void) const
{
	return static_cast<jstring>(LocalRefParam::getValue());
}

inline jstring JavaStringParam::getReturnValue(void)
{
	return static_cast<jstring>(LocalRefParam::getReturnValue());
}


//========================================================================

/**
 * Wraps a jstring created/returned by a JNI function.
 */
class JavaString : public JavaStringParam
{
public:
	static const JavaStringPtr cms_nullPtr;

public:
	explicit  JavaString(jstring src);
	explicit  JavaString(const char * src);
	explicit  JavaString(const std::string & src);
	explicit  JavaString(const Unicode::String & src);
	virtual  ~JavaString();

private:
	JavaString();
	JavaString(const JavaString & src);
};


//========================================================================

/**
 * Wraps a jobject that represents a Java script.dictionary object.
 */
class JavaDictionary : public LocalRef, public ScriptDictionary
{
public:
	static const JavaDictionaryPtr cms_nullPtr;

public:
	explicit  JavaDictionary(jobject src);
	virtual  ~JavaDictionary();

	virtual void serialize(void);

private:
	JavaDictionary(const JavaDictionary &);
	JavaDictionary & operator = (const JavaDictionary &);
};


//========================================================================

#endif	// _INCLUDED_JNIWrappers_H



