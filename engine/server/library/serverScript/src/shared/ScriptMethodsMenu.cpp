//========================================================================
//
// ScriptMethodsMenu.cpp - implements script methods dealing with object
// movement.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "sharedGame/RadialMenuManager.h"
#include "sharedNetworkMessages/ObjectMenuRequestData.h"
#include <vector>

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsMenuNamespace
// ======================================================================

namespace ScriptMethodsMenuNamespace
{
	bool install();

	jfloat JNICALL getRangeForMenuType(JNIEnv *env, jobject self, jint menuType);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsMenuNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsMenuNamespace::c)}
	JF("getRangeForMenuType", "(I)F", getRangeForMenuType),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}

 
//----------------------------------------------------------------------
//-- todo move this into the proper implementation file

/**
* Convert a java menu_info to a vector of C++ ObjectMenuRequestData objects
*/
const bool JavaLibrary::convert(const LocalRefParamPtr & source, MenuDataVector & target)
{
	LocalObjectArrayRefPtr arrayOfMenuInfoDatas = callObjectArrayMethod (*source, ms_midMenuInfoGetMenuItemsInternal);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return false;
	}
	
	const int count = getArrayLength (*arrayOfMenuInfoDatas);
	target.reserve (count);
	
	for (int j = 0; j < count; ++j)
	{
		ObjectMenuRequestData omrd;
		LocalRefPtr element = getObjectArrayElement(*arrayOfMenuInfoDatas, j);
		
		//-- id
		const int id     = getIntField (*element, JavaLibrary::ms_fidMenuInfoDataId);
		if (id < 0 || id >= 255)
			return false;

		//-- parent
		const int parent = getIntField (*element, JavaLibrary::ms_fidMenuInfoDataParent);
		if (parent < 0 || parent >= 255)
			return false;
		
		//-- type
		const int type = getIntField (*element, JavaLibrary::ms_fidMenuInfoDataType);
		if (type < 0 || type >= 65535)
			return false;

		omrd.m_parent  = static_cast<uint8>(parent);
		omrd.m_id      = static_cast<uint8>(id);
		omrd.m_menuItemType = static_cast<uint16>(type);

		//-- label
		LocalRefPtr labelObject = getObjectField(*element, ms_fidMenuInfoDataLabel);
		if (labelObject == LocalRef::cms_nullPtr)
			return false;
		StringId labelId;
		if (ScriptConversion::convert (*labelObject, labelId))
			omrd.m_labelId = labelId;
		else
			WARNING (true, ("Unable to convert object menu labelId from Java string_id to C++ StringId."));
		
		//-- flags (enabled, serverNotify)

		const bool enabled      = getBooleanField (*element, ms_fidMenuInfoDataEnabled) != 0;
		const bool serverNotify = getBooleanField (*element, ms_fidMenuInfoDataServerNotify) != 0;

		if (enabled)
			omrd.m_flags |= ObjectMenuRequestData::F_enabled;
		else
			omrd.m_flags &= ~(ObjectMenuRequestData::F_enabled);

		if (serverNotify)
			omrd.m_flags |= ObjectMenuRequestData::F_serverNotify;
		else
			omrd.m_flags &= ~(ObjectMenuRequestData::F_serverNotify);

		target.push_back (omrd);
	}

	return true;
}

//----------------------------------------------------------------------
//-- todo move this into the proper implementation file

/**
* Convert a C++ MenuDataVector to a java menu_info
*/
const bool JavaLibrary::convert(const MenuDataVector & source, LocalRefParamPtr & target)
{
	const int count = static_cast<int>(source.size ());
	
	LocalObjectArrayRefPtr arrayOfMenuInfoDatas = createNewObjectArray(count, ms_clsMenuInfoData);

	if (arrayOfMenuInfoDatas == LocalObjectArrayRef::cms_nullPtr)
	{
		WARNING (true, ("java unable to create array of menu info datas."));
		return false;
	}

	if (getArrayLength (*arrayOfMenuInfoDatas) != count)
	{
		WARNING (true, ("java created wrong size array."));
		return false;
	}

	bool retval = true;

	for (int j = 0; j < count; ++j)
	{
		const ObjectMenuRequestData & omrd = source [j];

		LocalRefPtr element = createNewObject(ms_clsMenuInfoData, ms_midMenuInfoData);
		if (element == LocalRef::cms_nullPtr)
		{
			WARNING (true, ("java unable to create new menu array element %d from size %d array.", j, count));
			retval = false;
			continue;
		}

		setIntField     (*element, JavaLibrary::ms_fidMenuInfoDataId,           omrd.m_id);
		setIntField     (*element, JavaLibrary::ms_fidMenuInfoDataParent,       omrd.m_parent);
		setIntField     (*element, JavaLibrary::ms_fidMenuInfoDataType,         omrd.m_menuItemType);
		setBooleanField (*element, JavaLibrary::ms_fidMenuInfoDataEnabled,      omrd.hasFlag (ObjectMenuRequestData::F_enabled));
		setBooleanField (*element, JavaLibrary::ms_fidMenuInfoDataServerNotify, omrd.hasFlag (ObjectMenuRequestData::F_serverNotify));

		LocalRefPtr labelIdObject;
		if (ScriptConversion::convert (omrd.m_labelId, labelIdObject))
		{
			setObjectField(*element, ms_fidMenuInfoDataLabel, *labelIdObject);
		}
		else
			WARNING (true, ("Unable to convert object menu m_labelId from C++ StringId to Java string_id."));
		setObjectArrayElement (*arrayOfMenuInfoDatas, j, *element);
	}
	
	if (retval)
	{
		retval = callBooleanMethod(*target, JavaLibrary::ms_midMenuInfoSetMenuItemsInternal, arrayOfMenuInfoDatas->getValue()) != 0;
	}

	return retval;
}

//----------------------------------------------------------------------

/**
 * Returns the distance a player must be from a target to use a menu item.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param menuType		the id of the menu item (from menu_info_types.java)
 *
 * @return the distance, or -1 on unknown menu type
 */
jfloat JNICALL ScriptMethodsMenuNamespace::getRangeForMenuType(JNIEnv *env, jobject self, jint menuType)
{
	UNREF(env);
	UNREF(self);

	float range = 0;
	if (RadialMenuManager::getRangeForMenuType(menuType, range))
		return range;

	return -1.0f;
}	// JavaLibrary::getRangeForMenuType

//----------------------------------------------------------------------


