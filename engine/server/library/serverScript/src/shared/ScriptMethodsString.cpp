//=======================================================================
//
// ScriptMethodsString.cpp
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved.
//
//=======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/NameManager.h"
#include "sharedGame/TextManager.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsStringNamespace
// ======================================================================

namespace ScriptMethodsStringNamespace
{
	bool install();

	jstring      JNICALL getString(JNIEnv *env, jobject self, jobject id);
	jobjectArray JNICALL getStrings(JNIEnv *env, jobject self, jobjectArray ids);
	jboolean     JNICALL isObscene(JNIEnv *env, jobject self, jstring data);
	jboolean     JNICALL isAppropriateText(JNIEnv *env, jobject self, jstring data);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsStringNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsStringNamespace::c)}
	JF("getString", "(Lscript/string_id;)Ljava/lang/String;", getString),
	JF("getStrings", "([Lscript/string_id;)[Ljava/lang/String;", getStrings),
	JF("isObscene", "(Ljava/lang/String;)Z", isObscene),
	JF("isAppropriateText", "(Ljava/lang/String;)Z", isAppropriateText),	
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//----------------------------------------------------------------------

/**
 * Returns the string associated with a stringId.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param id			the stringId to find
 *
 * @return the string, or nullptr on error
 */
jstring JNICALL ScriptMethodsStringNamespace::getString(JNIEnv *env, jobject self, jobject id)
{
	UNREF(self);

	if (id == 0)
	{
		WARNING(true, ("JavaLibrary::log getString is nullptr."));
		return 0;
	}

	// convert the string id to a char string
	if (!env->IsInstanceOf(id, JavaLibrary::getClsStringId()))
	{
		WARNING(true, ("JavaLibrary::log IsInstanceOf failed."));
		 return 0;
	}

	// get the table
	JavaStringPtr tempString = getStringField(LocalRefParam(id), JavaLibrary::getFidStringIdTable());
	std::string table;
	if(! JavaLibrary::convert(*tempString, table))
	{
		WARNING(true, ("JavaLibrary::log failed to get the string table."));
		return 0;
	}

	std::string asciiId;
	JavaStringPtr tempAsciiIdString = getStringField(LocalRefParam(id), JavaLibrary::getFidStringIdAsciiId());
	if(! JavaLibrary::convert(*tempAsciiIdString, asciiId))
	{
		WARNING(true, ("JavaLibrary::log failed to get the string asciiId."));
		return 0;
	}

	const StringId stringId(table, asciiId);
	Unicode::String localized;

	if (stringId.localize(localized))
	{
		JavaString result(localized);
		return result.getReturnValue();
	}

	//TODO? this warning is annoying and likely just a remnant of the japanese language version of the game
	//DEBUG_WARNING(true, ("JavaLibrary::log failed to localize the stringId(%s, %s)", table.c_str(), asciiId.c_str()));
	
	return 0;

}	// JavaLibrary::getString

//----------------------------------------------------------------------

/**
 * Returns the strings associated with an array of stringIds.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param ids			array of stringIds to find
 *
 * @return an array of strings, or nullptr on error
 */
jobjectArray JNICALL ScriptMethodsStringNamespace::getStrings(JNIEnv *env, jobject self, jobjectArray ids)
{
	UNREF(self);

	if (ids == 0)
		return 0;

	if (!env->IsInstanceOf(ids, JavaLibrary::getClsStringIdArray()))
		 return 0;

	jsize count = env->GetArrayLength(ids);

	// create the string array to return
	LocalObjectArrayRefPtr strings = createNewObjectArray(count, JavaLibrary::getClsString());
	if (strings == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	// fill the array
	for (jsize i = 0; i < count; ++i)
	{
		LocalRefPtr id = getObjectArrayElement(LocalObjectArrayRefParam(ids), i);
		setObjectArrayElement(*strings, i, LocalRef(getString(env, self, id->getValue())));
	}

	return strings->getReturnValue();
}	// JavaLibrary::getStrings

//----------------------------------------------------------------------

/**
 * Finds if a string is obscene.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param data			string to test
 *
 * @return true if the string is obscene, false if not
 */
jboolean JNICALL ScriptMethodsStringNamespace::isObscene(JNIEnv *env, jobject self, jstring data)
{
	if (data == 0)
		return JNI_FALSE;

	JavaStringParam jtext(data);
	Unicode::String text;
	if (JavaLibrary::convert(jtext, text))
	{
		return NameManager::getInstance().isNameReserved(text);
	}

	return JNI_FALSE;
}	// JavaLibrary::isObscene

//----------------------------------------------------------------------

/**
 * Tests is a string is appropriate, whatever that means.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param data			string to test
 *
 * @return true if the string is appropriate, false if not
 */
jboolean JNICALL ScriptMethodsStringNamespace::isAppropriateText(JNIEnv *env, jobject self, jstring data)
{
	if (data == 0)
		return JNI_FALSE;

	JavaStringParam jtext(data);
	Unicode::String text;
	if (JavaLibrary::convert(jtext, text))
	{
		return TextManager::isAppropriateText(text);
	}

	return JNI_FALSE;
}	// JavaLibrary::isAppropriateText

//----------------------------------------------------------------------
