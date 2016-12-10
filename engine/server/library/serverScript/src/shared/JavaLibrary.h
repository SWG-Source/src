//========================================================================
//
// JavaLibrary.h - interface to the JVM via JNI.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_JavaLibrary_H
#define _INCLUDED_JavaLibrary_H

#ifdef WIN32
#pragma warning (disable: 4100)
#pragma warning (disable: 4786)
#endif

#include <jni.h>
#include "serverScript/JNIWrappers.h"
#include "serverScript/ScriptDictionary.h"
#include "sharedUtility/Location.h"

class Container;
class CreatureObject;
class CustomizationData;
class CustomizationVariable;
class DraftSchematicObject;
class DynamicVariableList;
class DynamicVariableListNestedList;
class GameScriptObject;
class ManufactureObjectInterface;
class NetworkId;
class PaletteColorCustomizationVariable;
class RangedIntCustomizationVariable;
class Region;
class ScriptListEntry;
class ScriptParams;
class Semaphore;
class ServerObject;
class ShipObject;
class TangibleObject;
class Thread;
class Transform;
class ValueDictionary;
class Vector;
class WeaponObject;
struct ObjectMenuRequestData;

namespace AttribMod
{
	struct AttribMod;
};

namespace CellPermissions
{
	class PermissionObject;
};

namespace Crafting
{
	struct IngredientSlot;
};

namespace Archive
{
	//	struct DefaultObjectType;
	template <typename T, typename U> class AutoDeltaVector;
}

typedef std::vector<ScriptListEntry> ScriptList;
typedef Unicode::String           String_t;
typedef std::vector <String_t> StringVector_t;
typedef std::vector<ObjectMenuRequestData> MenuDataVector;

//========================================================================
// C<->Java conversion functions
//========================================================================

namespace ScriptConversion
{
	const bool convert(const Location & sourceLoc, LocalRefPtr & target);
	const bool convert(const LocalRefParam & sourceLoc, Location & target);
	const bool convert(const LocalObjectArrayRefParam & sourceLoc, std::vector<Location> & target);
	const bool convert(const std::vector<const Unicode::String *> & source, LocalObjectArrayRefPtr & target);
	const bool convert(const jobject source, StringId & target);
	const bool convert(const LocalRefParam & source, StringId & target);
	const bool convert(const StringId & source, LocalRefPtr & target);
	const bool getClassName(const jobject & source, std::string & target);
	const bool convert(const std::vector<const char *> & source, LocalObjectArrayRefPtr & strArray);
	const bool convert(const std::vector<std::string> & source, LocalObjectArrayRefPtr & strArray);
	const bool convert(const jobjectArray & source, std::vector<std::string> & strArray);
	const bool convert(const std::set<CellPermissions::PermissionObject> & source, LocalObjectArrayRefPtr & strArray);
	const bool convert(const std::vector<ServerObject *> & source, LocalLongArrayRefPtr & target);
	const bool convert(const std::vector<NetworkId> & source, LocalLongArrayRefPtr & target);
	const bool convert(const jlongArray & source, std::vector<ServerObject *> & target);
	const bool convert(const jlongArray & source, std::vector<NetworkId> & target);
	const bool convert(const jobject & source, Vector & target, NetworkId & targetCell);
	const bool convertWorld(const jobject & source, Vector & target);
	const bool convertWorld(const jlong & source, Vector & target);
	const bool convertWorld(const LocalRefParam & source, Vector & target);
	const bool convert(const Vector & source, const NetworkId & sourceCell, LocalRefPtr & target);
	const bool convert(const jobject & source, Vector & targetLoc, std::string & targetSceneId, NetworkId & targetCell);
	const bool convertWorld(const jobject & source, Vector & targetLoc, std::string & targetSceneId);
	const bool convert(const Vector & sourceLoc, const std::string & sourceSceneId, const NetworkId & sourceCell, LocalRefPtr & target);
	const bool convertWorld(const jobjectArray & source, std::vector<Vector> & target);
	const bool convert(const std::vector<const Vector *> & source, LocalObjectArrayRefPtr & target);
	const bool convert(const LocalRefParam & source, const Region * & target);
	const bool convert(const jobject & source, const Region * & target);
	const bool convert(const Region & source, LocalRefPtr & target);
	const bool convert(const AttribMod::AttribMod & source, LocalRefPtr & target);
	const bool convert(const jobject & source, AttribMod::AttribMod & target);
	const bool convert(const LocalRefParam & source, AttribMod::AttribMod & target);
	const bool convert(const std::vector<AttribMod::AttribMod> & source, LocalObjectArrayRefPtr & target);
	const bool convert(const jobjectArray & source, std::vector<AttribMod::AttribMod> & target);
	const bool convert(const jobject & source, Vector & target, NetworkId & targetCell, const Vector & i_default);
	const bool convert(const jbyteArray & source, std::vector<int8> & target);
	const bool convert(const LocalByteArrayRef & source, std::vector<int8> & target);
	const bool convert(const std::vector<int8> & source, LocalByteArrayRefPtr & target);
	const bool convert(const Transform & sourceTransform, LocalRefPtr & target);
	const bool convert(const jobject & sourceTransform, Transform & target);
	const bool convert(const LocalRefParam & sourceTransform, Transform & target);
	const bool convert(const std::vector<Transform> & source, LocalObjectArrayRefPtr & target);
	const bool convert(const jobjectArray & source, std::vector<Transform> & target);
	const bool convert(const Vector & sourceVector, LocalRefPtr & target);
	const bool convert(const jobject & sourceVector, Vector & target);
	const bool convert(const LocalRefParam & sourceVector, Vector & target);
}

//========================================================================
// class JavaLibrary
//========================================================================

class JavaLibrary
{
	//	friend jobject convertDynamicVariableListToObject(JNIEnv *env, const DynamicVariableList& list);
	friend const bool ScriptConversion::convert(const Location & sourceLoc, LocalRefPtr & target);
	friend const bool ScriptConversion::convert(const LocalRefParam & sourceLoc, Location & target);
	friend const bool ScriptConversion::convert(const LocalObjectArrayRefParam & sourceLoc, std::vector<Location> & target);
	friend const bool ScriptConversion::convert(const std::vector<const Unicode::String *> & source, LocalObjectArrayRefPtr & target);
	friend const bool ScriptConversion::convert(const jobject source, StringId & target);
	friend const bool ScriptConversion::convert(const LocalRefParam & source, StringId & target);
	friend const bool ScriptConversion::convert(const StringId & source, LocalRefPtr & target);
	friend const bool ScriptConversion::convert(const std::vector<ServerObject *> & source, LocalLongArrayRefPtr & target);
	friend const bool ScriptConversion::convert(const std::vector<NetworkId> & source, LocalLongArrayRefPtr & target);
	friend const bool ScriptConversion::convert(const std::vector<const char *> & source, LocalObjectArrayRefPtr & strArray);
	friend const bool ScriptConversion::convert(const jlongArray & source, std::vector<ServerObject *> & target);
	friend const bool ScriptConversion::convert(const jlongArray & source, std::vector<NetworkId> & target);
	friend const bool ScriptConversion::convert(const jobject & source, Vector & target, NetworkId & targetCell);
	friend const bool ScriptConversion::convertWorld(const jobject & source, Vector & target);
	friend const bool ScriptConversion::convertWorld(const jlong & source, Vector & target);
	friend const bool ScriptConversion::convertWorld(const LocalRefParam & source, Vector & target);
	friend const bool ScriptConversion::convert(const Vector & source, const NetworkId & sourceCell, LocalRefPtr & target);
	friend const bool ScriptConversion::convert(const jobject & source, Vector & targetLoc, std::string & targetSceneId, NetworkId & targetCell);
	friend const bool ScriptConversion::convertWorld(const jobject & source, Vector & targetLoc, std::string & targetSceneId);
	friend const bool ScriptConversion::convert(const Vector & sourceLoc, const std::string & sourceSceneId, const NetworkId & sourceCell, LocalRefPtr & target);
	friend const bool ScriptConversion::convertWorld(const jobjectArray & source, std::vector<Vector> & target);
	friend const bool ScriptConversion::convert(const std::vector<const Vector *> & source, LocalObjectArrayRefPtr & target);
	friend const bool ScriptConversion::convert(const jobject & source, const Region* &target);
	friend const bool ScriptConversion::convert(const Region & source, LocalRefPtr & target);
	friend const bool ScriptConversion::convert(const std::vector<std::string> & source, LocalObjectArrayRefPtr & strArray);
	friend const bool ScriptConversion::convert(const jobjectArray & source, std::vector<std::string> & strArray);
	friend const bool ScriptConversion::convert(const std::set<CellPermissions::PermissionObject> & source, LocalObjectArrayRefPtr & strArray);
	friend const bool ScriptConversion::convert(const AttribMod::AttribMod & source, LocalRefPtr & target);
	friend const bool ScriptConversion::convert(const jobject & source, AttribMod::AttribMod & target);
	friend const bool ScriptConversion::convert(const LocalRefParam & source, AttribMod::AttribMod & target);
	friend const bool ScriptConversion::convert(const std::vector<AttribMod::AttribMod> & source, LocalObjectArrayRefPtr & target);
	friend const bool ScriptConversion::convert(const jobjectArray & source, std::vector<AttribMod::AttribMod> & target);
	friend const bool ScriptConversion::convert(const jbyteArray & source, std::vector<int8> & target);
	friend const bool ScriptConversion::convert(const LocalByteArrayRef & source, std::vector<int8> & target);
	friend const bool ScriptConversion::convert(const std::vector<int8> & source, LocalByteArrayRefPtr & target);
	friend const bool ScriptConversion::convert(const Transform & sourceTransform, LocalRefPtr & target);
	friend const bool ScriptConversion::convert(const jobject & sourceTransform, Transform & target);
	friend const bool ScriptConversion::convert(const LocalRefParam & sourceTransform, Transform & target);
	friend const bool ScriptConversion::convert(const std::vector<Transform> & source, LocalObjectArrayRefPtr & target);
	friend const bool ScriptConversion::convert(const jobjectArray & source, std::vector<Transform> & target);
	friend const bool ScriptConversion::convert(const Vector & sourceVector, LocalRefPtr & target);
	friend const bool ScriptConversion::convert(const jobject & sourceVector, Vector & target);
	friend const bool ScriptConversion::convert(const LocalRefParam & sourceVector, Vector & target);

	friend JavaDictionaryPtr JNIWrappersNamespace::createNewDictionary();
	friend JavaStringPtr     JNIWrappersNamespace::callStringMethod(const LocalRefParam & object, jmethodID methodID, ...);

public:
	enum JavaVMs
	{
		JV_none,
		JV_ibm,
		JV_sun
	};

public:

	static void throwScriptException(char const * format, ...);

	static void throwScriptException(char const * format, va_list va);

public:
	// singleton methods
	static void install(void);
	static void remove(void);
	static JavaLibrary* instance(void);

	static bool registerNatives(const JNINativeMethod natives[], int count);

public:
	static void                 onWatching(ServerObject & observer, ServerObject & subject);
	static void                 onStopWatching(ServerObject & observer, ServerObject & subject);

	// obj_id functions
//	static void            removeJavaId(void * id);
	static void            removeJavaId(const NetworkId & id);
	static void            flagDestroyed(const NetworkId & id);
	void                   setObjIdAuthoritative(const NetworkId & id, bool authoritative, uint32 pid);
	void                   setObjIdLoaded(const NetworkId & id);
	void                   setObjIdInitialized(const NetworkId & id);
	void                   setObjIdLoggedIn(const NetworkId & id, bool loggedIn);
	void                   attachScriptToObjId(const NetworkId &object, const std::string & script);
	void                   attachScriptsToObjId(const NetworkId &object, const ScriptList & scripts);
	void                   detachScriptFromObjId(const NetworkId &object, const std::string & script);
	void                   detachAllScriptsFromObjId(const NetworkId &object);
	static void            setScriptVar(const ServerObject &object, const std::string & name, int value);
	static void            setScriptVar(const ServerObject &object, const std::string & name, float value);
	static void            setScriptVar(const ServerObject &object, const std::string & name, const std::string & value);
	static void            clearScriptVars(const ServerObject & source);
	static void            unpackScriptVars(const ServerObject & target, const std::vector<int8> & data);
	static void            unpackDeltaScriptVars(const ServerObject & target, const std::vector<int8> & data);
	static void            packScriptVars(const ServerObject & source, std::vector<int8> & data);
	static void            packAllDeltaScriptVars();

	// accessor functions
	static JNIEnv *        getEnv(void);
	static int             getJavaVmType();
	static const bool      getClassName(const jobject & source, std::string & target);
	static const bool      getClassName(const jclass & sourceClass, std::string & target);

	// debug functions
	static jlong           getFreeJavaMemory();
	static void            printJavaStack();

	static ServerObject *  findObjectByNetworkId(const NetworkId & id);

	static CreatureObject *getCreatureThrow(JNIEnv *env, jlong objId, char const *errorDescription, bool throwIfNotOnServer = true);
	static ShipObject *    getShipThrow(JNIEnv *env, jlong objId, char const *errorDescription, bool throwIfNotOnServer = true);

	template <class OBJPTR>
	static bool getObject(const jobject obj, OBJPTR &result)
	{
		if (obj != 0)
		{
			NetworkId id(getNetworkId(obj));
			result = dynamic_cast<OBJPTR>(findObjectByNetworkId(id));
			return result != 0;
		}
		return 0;
	}

	template <class OBJPTR>
	static bool getObject(const jlong obj, OBJPTR &result)
	{
		if (obj != 0)
		{
			NetworkId id(static_cast<NetworkId::NetworkIdType>(obj));
			result = dynamic_cast<OBJPTR>(findObjectByNetworkId(id));
			return result != 0;
		}
		return 0;
	}

	template <class OBJPTR>
	static bool getObject(const LocalRefParam & obj, OBJPTR &result)
	{
		if (obj.getValue() != 0)
		{
			NetworkId id(getNetworkId(obj));
			result = dynamic_cast<OBJPTR>(findObjectByNetworkId(id));
			return result != 0;
		}
		return 0;
	}

	template <class OBJPTR, class CTLPTR>
	static bool getObjectController(const jobject obj, OBJPTR &object, CTLPTR &controller)
	{
		if (getObject(obj, object) && object)
		{
			controller = dynamic_cast<CTLPTR>(object->getController());
			return controller != 0;
		}
		return 0;
	}

	template <class OBJPTR, class CTLPTR>
	static bool getObjectController(const jlong obj, OBJPTR &object, CTLPTR &controller)
	{
		if (getObject(obj, object) && object)
		{
			controller = dynamic_cast<CTLPTR>(object->getController());
			return controller != 0;
		}
		return 0;
	}

	// efficiency functions
	bool queryScriptFunctions(const std::string & scriptName);
	void enableLogging(bool enable) const;
	void enableNewJediTracking(bool enableTracking);

	// conversion functions
	static const bool convert(const std::string & source, JavaString & target);
	static const bool convert(const JavaStringParam & source, std::string & target);
	static const bool convert(const Unicode::String & source, JavaString & target);
	static const bool convert(const JavaStringParam & source, Unicode::String & target);
	static const bool convert(const JavaDictionary & source, std::vector<int8> & target);
	static const bool convert(const LocalRefParamPtr & source, MenuDataVector & target);
	static const bool convert(const MenuDataVector & source, LocalRefParamPtr & target);
	static const bool convert(const std::map<std::string, int> & source, JavaDictionaryPtr & target);
	static const bool convert(const std::vector<std::pair<std::string, int> > & source, JavaDictionaryPtr & target);
	static const bool convert(const std::vector<std::pair<std::string, std::pair<int, int> > > & source, JavaDictionaryPtr & target);
	static const bool convert(const std::vector<std::pair<std::string, bool> > & source, JavaDictionaryPtr & target);
	static LocalRefPtr convert(const ValueDictionary & source);
	static void        convert(const jobject & source, ValueDictionary & target);

	// trigger functions
	int runScripts(const NetworkId & caller, const std::string& method, const std::string& argList, ScriptParams &args);
	int runScripts(const NetworkId & caller, const std::string& method, const std::string& argList, const StringVector_t &args);
	int runScript(const NetworkId & caller, const std::string& script, const std::string& method, const std::string& argList, ScriptParams &args);
	int runScript(const NetworkId & caller, const std::string& script, const std::string& method, const std::string& argList, const StringVector_t &args);
	int runScript(const NetworkId & caller, const std::string& script, const std::string& method, jobjectArray args);
	std::string callScriptConsoleHandler(const std::string & script,
		const std::string & method, const std::string & argList, ScriptParams & args);
	bool reloadScript(const std::string& scriptName);

	// message functions
	int callMessages(const NetworkId & caller, const std::string & method, const ScriptDictionaryPtr & data);
	int callMessage(const NetworkId & caller, const std::string & script, const std::string & method, const ScriptDictionary & data);
	void packDictionary(const ScriptDictionary & dictionary, std::vector<int8> & packedData);
	bool unpackDictionary(const std::vector<int8> & packedData, ScriptDictionaryPtr & dictionary);
	void convert(const ScriptParams & params, JavaDictionaryPtr & dictionary);

	static void         setupWeaponCombatData(JNIEnv *env, const WeaponObject * weapon, jobject weaponData);
	static LocalRefPtr  createObjectAttribute(const ManufactureObjectInterface & manfSchematic, const DraftSchematicObject & draftSchematic, int attribIndex);
	static LocalRefPtr  createExperimentAttribute(const ManufactureObjectInterface & manfSchematic, const StringId & attribName);
	static LocalRefPtr  convert(const ManufactureObjectInterface & source);
	static LocalRefPtr  convert(const ManufactureObjectInterface & schematic, const Crafting::IngredientSlot & source, int amountRequired, const std::string & appearance, const std::string & requiredIngredient);
	static jobject      convert(const DraftSchematicObject & source);
	static jboolean     updateDynamicVariableList(JNIEnv *env, DynamicVariableList &list, jobject data);

	static void throwInternalScriptError(const char * message);

public:
	// JNI class and id accessors
	static jclass    getClsAttribute();
	static jmethodID getMidAttribute();
	static jfieldID  getFidAttributeType();
	static jfieldID  getFidAttributeValue();

	static jclass    getClsBaseClassRangeInfo();
	static jfieldID  getFidBaseClassRangeInfoMinRange();
	static jfieldID  getFidBaseClassRangeInfoMaxRange();
	static jfieldID  getFidBaseClassAttackerResultsId();
	static jfieldID  getFidBaseClassAttackerResultsWeapon();
	static jfieldID  getFidBaseClassAttackerResultsPosture();
	static jfieldID  getFidBaseClassAttackerResultsTrailBits();
	static jfieldID  getFidBaseClassAttackerResultsClientEffectId();
	static jfieldID  getFidBaseClassAttackerResultsActionName();
	static jfieldID  getFidBaseClassAttackerResultsUseLocation();
	static jfieldID  getFidBaseClassAttackerResultsTargetLocation();
	static jfieldID  getFidBaseClassAttackerResultsTargetCell();
	static jfieldID  getFidBaseClassDefenderDamageAmount();
	static jfieldID  getFidBaseClassDefenderResultsHitLocation();
	static jfieldID  getFidBaseClassDefenderResultsId();
	static jfieldID  getFidBaseClassDefenderResultsPosture();
	static jfieldID  getFidBaseClassDefenderResultsResult();
	static jfieldID  getFidBaseClassDefenderResultsClientEffectId();

	static jclass    getClsColor();
	static jmethodID getMidColor();

	static jfieldID  getFidCombatEngineAttackerDataWeaponSkill();
	static jfieldID  getFidCombatEngineAttackerDataAims();
	static jfieldID  getFidCombatEngineCombatantDataPos();
	static jfieldID  getFidCombatEngineCombatantDataWorldPos();
	static jfieldID  getFidCombatEngineCombatantDataRadius();
	static jfieldID  getFidCombatEngineCombatantDataIsCreature();
	static jfieldID  getFidCombatEngineCombatantDataPosture();
	static jfieldID  getFidCombatEngineCombatantDataLocomotion();
	static jfieldID  getFidCombatEngineCombatantDataScriptMod();
	static jfieldID  getFidCombatEngineDefenderDataCombatSkeleton();
	static jfieldID  getFidCombatEngineDefenderDataCover();
	static jfieldID  getFidCombatEngineDefenderDataHitLocationChances();
	static jfieldID  getFidCombatEngineHitResultRawDamage();
	static jfieldID  getFidCombatEngineHitResultDamageType();
	static jfieldID  getFidCombatEngineHitResultElementalDamage();
	static jfieldID  getFidCombatEngineHitResultElementalDamageType();
	static jfieldID  getFidCombatEngineHitResultBleedDamage();
	static jfieldID  getFidCombatEngineHitResultCritDamage();
	static jfieldID  getFidCombatEngineHitResultBlockedDamage();
	static jfieldID  getFidCombatEngineHitResultDamage();
	static jfieldID  getFidCombatEngineHitResultHitLocation();
	static jfieldID  getFidCombatEngineHitResultSuccess();
	static jfieldID  getFidCombatEngineHitResultCritical();
	static jfieldID  getFidCombatEngineHitResultGlancing();
	static jfieldID  getFidCombatEngineHitResultCrushing();
	static jfieldID  getFidCombatEngineHitResultStrikethrough();
	static jfieldID  getFidCombatEngineHitResultStrikethroughAmount();
	static jfieldID  getFidCombatEngineHitResultEvadeResult();
	static jfieldID  getFidCombatEngineHitResultEvadeAmount();
	static jfieldID  getFidCombatEngineHitResultBlockResult();
	static jfieldID  getFidCombatEngineHitResultBlock();
	static jfieldID  getFidCombatEngineHitResultDodge();
	static jfieldID  getFidCombatEngineHitResultParry();
	static jfieldID  getFidCombatEngineHitResultProc();
	static jfieldID  getFidCombatEngineHitResultBlockingArmor();

	static jclass    getClsCustomVar();
	static jmethodID getMidCustomVarObjIdStringInt();

	static jclass    getClsDraftSchematic();
	static jclass    getClsDraftSchematicAttrib();
	static jfieldID  getFidDraftSchematicAttribs();
	static jfieldID  getFidDraftSchematicAttribMap();
	static jfieldID  getFidDraftSchematicAttribName();
	static jfieldID  getFidDraftSchematicAttribCurrentValue();
	static jfieldID  getFidDraftSchematicAttribResourceMaxValue();
	static jfieldID  getFidDraftSchematicAttribMinValue();
	static jfieldID  getFidDraftSchematicAttribMaxValue();
	static jfieldID  getFidDraftSchematicCustomName();
	static jfieldID  getFidDraftSchematicCustomValue();
	static jfieldID  getFidDraftSchematicCustomMinValue();
	static jfieldID  getFidDraftSchematicCustomMaxValue();
	static jfieldID  getFidDraftSchematicExperimentalAttribs();

	static jclass    getClsDynamicVariable();
	static jmethodID getMidDynamicVariableFloat();
	static jmethodID getMidDynamicVariableFloatArray();
	static jmethodID getMidDynamicVariableInt();
	static jmethodID getMidDynamicVariableIntArray();
	static jmethodID getMidDynamicVariableLocation();
	static jmethodID getMidDynamicVariableLocationArray();
	static jmethodID getMidDynamicVariableObjId();
	static jmethodID getMidDynamicVariableObjIdArray();
	static jmethodID getMidDynamicVariableString();
	static jmethodID getMidDynamicVariableStringArray();

	static jclass    getClsDynamicVariableList();
	static jmethodID getMidDynamicVariableList();
	static jmethodID getMidDynamicVariableListSet();
	static jmethodID getMidDynamicVariableListSetFloat();
	static jmethodID getMidDynamicVariableListSetFloatArray();
	static jmethodID getMidDynamicVariableListSetInt();
	static jmethodID getMidDynamicVariableListSetIntArray();
	static jmethodID getMidDynamicVariableListSetLocation();
	static jmethodID getMidDynamicVariableListSetLocationArray();
	static jmethodID getMidDynamicVariableListSetObjId();
	static jmethodID getMidDynamicVariableListSetObjIdArray();
	static jmethodID getMidDynamicVariableListSetString();
	static jmethodID getMidDynamicVariableListSetStringArray();
	static jmethodID getMidDynamicVariableListSetStringId();
	static jmethodID getMidDynamicVariableListSetStringIdArray();
	static jmethodID getMidDynamicVariableListSetTransform();
	static jmethodID getMidDynamicVariableListSetTransformArray();
	static jmethodID getMidDynamicVariableListSetVector();
	static jmethodID getMidDynamicVariableListSetVectorArray();

	static jclass    getClsBoolean();
	static jmethodID getMidBooleanBooleanValue();

	static jclass    getClsDictionary();
	static jmethodID getMidDictionary();
	static jmethodID getMidDictionaryGet();
	static jmethodID getMidDictionaryKeys();
	static jmethodID getMidDictionaryPut();
	static jmethodID getMidDictionaryPutFloat();
	static jmethodID getMidDictionaryPutInt();

	static jmethodID getMidEnumerationHasMoreElements();
	static jmethodID getMidEnumerationNextElement();

	static jclass    getClsFloat();
	static jmethodID getMidFloatFloatValue();

	static jclass    getClsHashtable();
	static jmethodID getMidHashtable();

	static jclass    getClsInteger();
	static jmethodID getMidIntegerIntValue();

	static jclass    getClsLocation();
	static jclass    getClsLocationArray();
	static jfieldID  getFidLocationArea();
	static jfieldID  getFidLocationCell();
	static jfieldID  getFidLocationX();
	static jfieldID  getFidLocationY();
	static jfieldID  getFidLocationZ();

	static jmethodID getMidMapPut();

	static jclass    getClsMapLocation();
	static jmethodID getMidMapLocation();

	static jclass    getClsMentalState();
	static jmethodID getMidMentalState();
	static jfieldID  getFidMentalStateType();
	static jfieldID  getFidMentalStateValue();

	static jmethodID getMidObjIdGetValue();
	static jmethodID getMidObjIdSetAuthoritative();

	static jclass    getClsPalcolorCustomVar();
	static jmethodID getMidPalcolorCustomVar();

	static jclass    getClsRangedIntCustomVar();
	static jmethodID getMidRangedIntCustomVar();

	static jclass    getClsRegion();

	static jclass    getClsResourceAttribute();
	static jfieldID  getFidResourceAttributeName();
	static jfieldID  getFidResourceAttributeValue();

	static jclass    getClsResourceDensity();
	static jfieldID  getFidResourceDensityDensity();
	static jfieldID  getFidResourceDensityResourceType();

	static jclass    getClsScriptEntry();
	static jmethodID getMidScriptEntryGetOwnerContext();

	static jclass    getClsString();
	static jclass    getClsStringArray();
	static jclass    getClsStringId();
	static jclass    getClsStringIdArray();
	static jfieldID  getFidStringIdAsciiId();
	static jfieldID  getFidStringIdTable();

	static jclass    getClsTransform();
	static jclass    getClsTransformArray();

	static jclass    getClsVector();
	static jclass    getClsVectorArray();

protected:
	JavaLibrary(void);
	virtual ~JavaLibrary();

private:
	enum
	{
		MAX_STRING_ID_PARAMS = 10,
		MAX_ATTRIB_MOD_PARAMS = 12,
		MAX_MENTAL_STATE_MOD_PARAMS = 8,
		MAX_MODIFIABLE_INT_PARAMS = 2,
		MAX_MODIFIABLE_FLOAT_PARAMS = 1,
		MAX_MODIFIABLE_STRING_ID_PARAMS = 2,
		MAX_RECURSION_COUNT = 10
	};

	// Function to make the java output quiet
	static void fatalHandler(int);

	// class global instances stores a pool of commonly used java classes so we
	// don't have to keep allocating/deallocating them
	class GlobalInstances;
	friend class GlobalInstances;
	class GlobalInstances
	{
		friend class JavaLibrary;
	public:
		GlobalInstances(void);
		~GlobalInstances();

		GlobalRefPtr getNextStringId(void);
		GlobalRefPtr getNextAttribMod(void);
		GlobalRefPtr getNextMentalStateMod(void);
		GlobalRefPtr getNextModifiableInt(void);
		GlobalRefPtr getNextModifiableFloat(void);
		GlobalRefPtr getNextModifiableStringId(void);
		GlobalRefPtr getMenuInfo();

	private:
		static int      ms_stringIdIndex;           // next string_id instance to use
		static int      ms_attribModIndex;          // next attrib_mod instance to use
		static int      ms_mentalStateModIndex;     // next mental_state instance to use
		static int      ms_modifiableIntIndex;      // next modifiable_int instance to use
		static int      ms_modifiableFloatIndex;    // next modifiable_float instance to use
		static int      ms_modifiableStringIdIndex; // next modifiable_string_id instance to use
		static GlobalRefPtr ms_stringIds[MAX_STRING_ID_PARAMS];		// global string_id instances for paramaters
		static GlobalRefPtr ms_attribMods[MAX_ATTRIB_MOD_PARAMS];	// global attrib_mod instances for paramaters
		static GlobalRefPtr ms_mentalStateMods[MAX_MENTAL_STATE_MOD_PARAMS];    // global mental_state instances for parameters
		static GlobalRefPtr ms_modifiableInts[MAX_MODIFIABLE_INT_PARAMS];	    // global modifiable_int instances for other paramaters
		static GlobalRefPtr ms_modifiableFloats[MAX_MODIFIABLE_FLOAT_PARAMS];	// global modifiable_float instances for paramaters
		static GlobalRefPtr ms_modifiableStringIds[MAX_MODIFIABLE_STRING_ID_PARAMS];	// global modifiable_string_id instances for paramaters
		static GlobalRefPtr ms_menuInfo;
		int             m_stringIdCount;	// next string_id instance to use
		int             m_attribModCount;	// next attrib_mod instance to use
		int             m_mentalStateModCount;  // next mental_state instance to use
		int             m_modifiableIntCount;	// next modifiable_int instance to use
		int             m_modifiableFloatCount;	// next modifiable_float instance to use
		int             m_modifiableStringIdCount;	// next modifiable_string_id instance to use
	};

private:

	static JavaLibrary*    ms_instance;			// singleton instance
	static int             ms_javaVmType;       // what kind of vm we are running
//	static void*           ms_libHandle;		// handle to the Java shared library
	static JavaVM*         ms_jvm;				// Java Virtual Machine
	static JNIEnv*         ms_env;				// Java environment
	static Thread *        m_initializerThread; // thread that initializes the jvm
	static int             ms_envCount;         // number of times we've reset the ms_env pointer
	static int             ms_currentRecursionCount;   // number of times we've called into runScript
	static bool            ms_resetJava;        // flag that we need to reset the Java connection as soon as we're all the way out of Java
	static jclass          ms_clsScriptEntry;   // entry point for all scripts
	static jobject         ms_scriptEntry;      // instance of class script_entry, used to initialize the class
	static jmethodID       ms_midScriptEntryGetOwnerContext; // the current game object ID of the running script
	static jmethodID       ms_midScriptEntryEnableLogging;   // turn script logging on/off
	static jmethodID       ms_midScriptEntryEnableNewJediTracking; // turns tracking of players to see if they can become Jedi on or off
	static jmethodID       ms_midScriptEntryGetFreeMem; // gets the amount of free memory left in Java
	static jclass          ms_clsObject;		// reference to java.lang.Object
	static jclass          ms_clsClass;			// reference to java.lang.Class
	static jmethodID       ms_midClassGetName;	// reference to java.lang.Class function getName
	static jmethodID       ms_midClassGetMethods;	// reference to java.lang.Class function getMethods
	static jclass          ms_clsMethod;		// reference to java.reflect.Method
	static jmethodID       ms_midMethodGetName;	// reference to java.reflect.Method function getName
	static jclass          ms_clsBoolean;		// reference to java.lang.Boolean
	static jclass          ms_clsBooleanArray;	// reference to java.lang.Boolean[]
	static jmethodID       ms_midBoolean;		// methodID for java.lang.Boolean constructor
	static jmethodID       ms_midBooleanBooleanValue;	// methodID for java.lang.Boolean booleanValue()
	static jclass          ms_clsInteger;		// reference to java.lang.Integer
	static jclass          ms_clsIntegerArray;	// reference to java.lang.Integer[]
	static jmethodID       ms_midInteger;		// methodID for java.lang.Integer constructor
	static jmethodID       ms_midIntegerIntValue;	// methodID for java.lang.Integer intValue()
	static jclass          ms_clsModifiableInt;	// reference to modifiable_int
	static jmethodID       ms_midModifiableInt;	// methodID for modifiable_int constructor
	static jfieldID        ms_fidModifiableIntData;	// reference to m_data field of modifiable_int
	static jclass          ms_clsFloat;			// reference to java.lang.Float
	static jclass          ms_clsFloatArray;	// reference to java.lang.Float[]
	static jmethodID       ms_midFloat;			// methodID for java.lang.Float constructor
	static jmethodID       ms_midFloatFloatValue;	// methodID for java.lang.Float floatValue()
	static jclass          ms_clsModifiableFloat;	// reference to modifiable_float
	static jmethodID       ms_midModifiableFloat;	// methodID for modifiable_float constructor
	static jfieldID        ms_fidModifiableFloatData;	// reference to m_data field of modifiable_float
	static jclass          ms_clsString;		    // reference to java.lang.String
	static jclass          ms_clsStringArray;		// reference to java.lang.String[]
	static jclass          ms_clsMap;               // reference to java.util.Map
	static jmethodID       ms_midMapPut;            // methodID for java.util.Map.put()
	static jmethodID       ms_midMapGet;            // methodID for java.util.Map.get()
	static jclass          ms_clsHashtable;         // reference to java.util.Hashtable
	static jmethodID       ms_midHashtable;         // reference to java.util.Hashtable constructor
	static jclass          ms_clsThrowable;         // reference to java.lang.Throwable
	static jclass          ms_clsError;             // reference to java.lang.Error
	static jclass          ms_clsStackOverflowError;// reference to java.lang.StackOverflowError
	static jmethodID       ms_midThrowableGetMessage;// reference to java.lang.Throwable.getMessage()
	static jclass          ms_clsThread;             // reference to java.lang.Thread
	static jmethodID       ms_midThreadDumpStack;    // reference to java.lang.Thread.dumpStack()

	static jclass          ms_clsInternalScriptError;             // reference to internal_script_exception
	static jclass          ms_clsInternalScriptSeriousError;      // reference to internal_script_error
	static jfieldID        ms_fidInternalScriptSeriousErrorError; // fieldID for internal_script_error.wrappedError

	static jclass          ms_clsDictionary;		// reference to dictionary
	static jmethodID       ms_midDictionary;        // method for the dictionary constructor
	static jmethodID       ms_midDictionaryPack;    // method for dictionary::pack()
	static jmethodID       ms_midDictionaryUnpack;  // method for dictionary::unpack()

	static jmethodID       ms_midDictionaryKeys;    // methodID for keys()
	static jmethodID       ms_midDictionaryValues;  // methodID for values()
	static jmethodID       ms_midDictionaryGet;     // methodID for dictionary::get()
	static jmethodID       ms_midDictionaryPut;     // methodID for dictionary::put()
	static jmethodID       ms_midDictionaryPutInt;
	static jmethodID       ms_midDictionaryPutFloat;
	static jmethodID       ms_midDictionaryPutBool;
	static jclass          ms_clsCollection;		// reference to Collection
	static jmethodID       ms_midCollectionToArray;	// methodID for toArray()
	static jclass          ms_clsEnumeration;		// reference to Enumeration
	static jmethodID       ms_midEnumerationHasMoreElements;	// methodID for hasMoreElements()
	static jmethodID       ms_midEnumerationNextElement;	// methodID for nextElement()

	static jclass          ms_clsBaseClassRangeInfo;               // reference to base_class.range_info
	static jfieldID        ms_fidBaseClassRangeInfoMinRange;       // reference to minRange field of base_class.range_info
	static jfieldID        ms_fidBaseClassRangeInfoMaxRange;       // reference to maxRange field of base_class.range_info
	static jclass          ms_clsBaseClassAttackerResults;         // reference to base_class.attacker_results
	static jfieldID        ms_fidBaseClassAttackerResultsId;       // reference to id field of base_class.attacker_results
	static jfieldID        ms_fidBaseClassAttackerResultsWeapon;   // reference to weapon field of base_class.attacker_results
	static jfieldID        ms_fidBaseClassAttackerResultsPosture;  // reference to endPosture field of base_class.attacker_results
	static jfieldID        ms_fidBaseClassAttackerResultsTrailBits;// reference to m_trailBits field of base_class.attacker_results
	static jfieldID        ms_fidBaseClassAttackerResultsClientEffectId;  // reference to clientEffectId field of base_class.attacker_results
	static jfieldID        ms_fidBaseClassAttackerResultsActionName;  // reference to actionName field of base_class.attacker_results
	static jfieldID        ms_fidBaseClassAttackerResultsUseLocation;  // reference to useLocation field of base_class.attacker_results
	static jfieldID        ms_fidBaseClassAttackerResultsTargetLocation;  // reference to targetLocation field of base_class.attacker_results
	static jfieldID        ms_fidBaseClassAttackerResultsTargetCell;  // reference to targetCell field of base_class.attacker_results
	static jclass          ms_clsBaseClassDefenderResults;         // reference to base_class.defender_results
	static jfieldID        ms_fidBaseClassDefenderResultsId;       // reference to id field of base_class.defender_results
	static jfieldID        ms_fidBaseClassDefenderResultsPosture;  // reference to endPosture field of base_class.defender_results
	static jfieldID        ms_fidBaseClassDefenderResultsResult;   // reference to result field of base_class.defender_results
	static jfieldID        ms_fidBaseClassDefenderResultsClientEffectId;  // reference to clientEffectId field of base_class.defender_results
	static jfieldID        ms_fidBaseClassDefenderResultsHitLocation;  // reference to hitLocation field of base_class.defender_results
	static jfieldID        ms_fidBaseClassDefenderDamageAmount;  // reference to damage amount field of base_class.defender_results

	static jclass          ms_clsDynamicVariable;		// reference to objVar
	static jfieldID        ms_fidDynamicVariableName;	// reference to m_name field of objVar
	static jfieldID        ms_fidDynamicVariableData;	// reference to m_data field of objVar
	static jmethodID       ms_midDynamicVariableInt;		    // methodID for integer objVar constructor
	static jmethodID       ms_midDynamicVariableIntArray;	    // methodID for integer array objVar constructor
	static jmethodID       ms_midDynamicVariableFloat;	        // methodID for float objVar constructor
	static jmethodID       ms_midDynamicVariableFloatArray;	    // methodID for float array objVar constructor
	static jmethodID       ms_midDynamicVariableString;	        // methodID for string objVar constructor
	static jmethodID       ms_midDynamicVariableStringArray;    // methodID for string array objVar constructor
	static jmethodID       ms_midDynamicVariableObjId;	        // methodID for obj_id objVar constructor
	static jmethodID       ms_midDynamicVariableObjIdArray;     // methodID for obj_id array objVar constructor
	static jmethodID       ms_midDynamicVariableLocation;       // methodID for location objVar constructor
	static jmethodID       ms_midDynamicVariableLocationArray;  // methodID for location array objVar constructor

	static jclass          ms_clsDynamicVariableList;			// reference to objVarList
	static jmethodID       ms_midDynamicVariableList;			// methodID for integer objVarList constructor
	static jmethodID       ms_midDynamicVariableListSet;			  // methodID for setDynamicVariable(objVar data)
	static jmethodID       ms_midDynamicVariableListSetInt;		      // methodID for setDynamicVariable(String name, int data)
	static jmethodID       ms_midDynamicVariableListSetIntArray;      // methodID for setDynamicVariable(String name, int[] data)
	static jmethodID       ms_midDynamicVariableListSetFloat;	      // methodID for setDynamicVariable(String name, float data)
	static jmethodID       ms_midDynamicVariableListSetFloatArray;	  // methodID for setDynamicVariable(String name, float[] data)
	static jmethodID       ms_midDynamicVariableListSetString;	      // methodID for setDynamicVariable(String name, string data)
	static jmethodID       ms_midDynamicVariableListSetStringArray;	  // methodID for setDynamicVariable(String name, string[] data)
	static jmethodID       ms_midDynamicVariableListSetObjId;	      // methodID for setDynamicVariable(String name, obj_id data)
	static jmethodID       ms_midDynamicVariableListSetObjIdArray;    // methodID for setDynamicVariable(String name, obj_id[] data)
	static jmethodID       ms_midDynamicVariableListSetLocation;      // methodID for setDynamicVariable(String name, location data)
	static jmethodID       ms_midDynamicVariableListSetLocationArray; // methodID for setDynamicVariable(String name, location[] data)
	static jmethodID       ms_midDynamicVariableListSetStringId;      // methodID for setDynamicVariable(String name, string_id data)
	static jmethodID       ms_midDynamicVariableListSetStringIdArray; // methodID for setDynamicVariable(String name, string_id[] data)
	static jmethodID       ms_midDynamicVariableListSetTransform;      // methodID for setDynamicVariable(String name, transform data)
	static jmethodID       ms_midDynamicVariableListSetTransformArray; // methodID for setDynamicVariable(String name, transform[] data)
	static jmethodID       ms_midDynamicVariableListSetVector;      // methodID for setDynamicVariable(String name, vector data)
	static jmethodID       ms_midDynamicVariableListSetVectorArray; // methodID for setDynamicVariable(String name, vector[] data)

	static jclass          ms_clsObjId;			// reference to objId
	static jclass          ms_clsObjIdArray;	// reference to objId[]
	static jobjectArray    ms_objIdList[MAX_RECURSION_COUNT]; // global objId array for sendings lists of objIds
	static jmethodID       ms_midObjIdGetValue;		// methodID for objId.getValue
	static jmethodID       ms_midObjIdGetObjId;		// methodID for objId.getObjId
	static jmethodID       ms_midObjIdClearObjId;   // methodID for objId.clearObjId
	static jmethodID       ms_midObjIdFlagDestroyed;// methodID for objId.flagDestroyed
	static jmethodID       ms_midObjIdSetAuthoritative; // methodID for objId.setAuthoritative
	static jmethodID       ms_midObjIdSetLoaded;        // methodID for objId.setLoaded
	static jmethodID       ms_midObjIdSetInitialized;   // methodID for objId.setInitialized
	static jmethodID       ms_midObjIdSetLoggedIn;      // methodID for objId.setLoggedIn
	static jmethodID       ms_midObjIdAttachScript;     // methodID for objId.attachScript(String)
	static jmethodID       ms_midObjIdAttachScripts;    // methodID for objId.attachScript(String[])
	static jmethodID       ms_midObjIdDetachScript;     // methodID for objId.detachScript
	static jmethodID       ms_midObjIdDetachAllScripts; // methodID for objId.detachAllScripts
	static jmethodID       ms_midObjIdPackAllDeltaScriptVars;   // methodID for String obj_id.packAllScriptVars()
	static jmethodID       ms_midObjIdSetScriptVarInt;    // methodID for objId.setScriptVar(int)
	static jmethodID       ms_midObjIdSetScriptVarFloat;  // methodID for objId.setScriptVar(float)
	static jmethodID       ms_midObjIdSetScriptVarString; // methodID for objId.setScriptVar(String)
	static jmethodID       ms_midObjIdClearScriptVars;  // methodID for objId.clearScriptVars()
	static jmethodID       ms_midObjIdPackScriptVars; // methoID for String obj_id.packScriptVars()
	static jmethodID       ms_midObjIdPackDeltaScriptVars; // methodID for String obj_id.packDeltaScriptVars()
	static jmethodID       ms_midObjIdUnpackScriptVars; // methodID for obj_id.unpackScriptVars(String)
	static jmethodID       ms_midObjIdUnpackDeltaScriptVars; // methodID for obj_id.unpackDeltaScriptVars(String)
	static jclass          ms_clsStringId;		    // reference to string_id
	static jmethodID       ms_midStringId;		    // methodID for string_id constructor
	static jclass          ms_clsStringIdArray;		// reference to string_id[]
	static jfieldID        ms_fidStringIdTable;	    // reference to m_table field of string_id
	static jfieldID        ms_fidStringIdAsciiId;   // reference to m_asciiId field of string_id
	static jfieldID        ms_fidStringIdIndexId;   // reference to m_indexId field of string_id
	static jclass          ms_clsModifiableStringId;// reference to modifiable_string_id

//	static jobject         ms_stringIdArray[MAX_STRINGID_PARAMS]; // global stringId instances for paramaters
	static jclass          ms_clsAttribute;       // reference to attribute
	static jmethodID       ms_midAttribute;       // methodID for attribute constructor
	static jfieldID        ms_fidAttributeType;   // reference to m_type field of attribute
	static jfieldID        ms_fidAttributeValue;  // reference to m_value field of attribute
	static jclass          ms_clsAttribMod;       // reference to attribMod
	static jmethodID       ms_midAttribMod;       // methodID for attribMod constructor
	static jfieldID        ms_fidAttribModName;   // reference to m_name field of attribMod
	static jfieldID        ms_fidAttribModSkill;  // reference to m_skill field of attribMod
	static jfieldID        ms_fidAttribModType;   // reference to m_attrib field of attribMod
	static jfieldID        ms_fidAttribModValue;  // reference to m_value field of attribMod
	static jfieldID        ms_fidAttribModTime;   // reference to m_duration field of attribMod
	static jfieldID        ms_fidAttribModAttack; // reference to m_attack field of attribMod
	static jfieldID        ms_fidAttribModDecay;  // reference to m_decay field of attribMod
	static jfieldID        ms_fidAttribModFlags;  // reference to m_flags field of attribMod
	static GlobalArrayRefPtr ms_attribModList[MAX_RECURSION_COUNT];	// global attribMod array for sending list of attribMods

	static jclass          ms_clsMentalState;		// reference to MentalState
	static jmethodID       ms_midMentalState;		// methodID for MentalState constructor
	static jfieldID        ms_fidMentalStateType;	// reference to m_type field of MentalState
	static jfieldID        ms_fidMentalStateValue;// reference to m_value field of MentalState
	static jclass          ms_clsMentalStateMod;		// reference to MentalStateMod
	static jmethodID       ms_midMentalStateMod;		// methodID for MentalStateMod constructor
	static jfieldID        ms_fidMentalStateModType;	// reference to m_attrib field of MentalStateMod
	static jfieldID        ms_fidMentalStateModValue;// reference to m_value field of MentalStateMod
	static jfieldID        ms_fidMentalStateModTime;	// reference to m_duration field of MentalStateMod
	static jfieldID        ms_fidMentalStateModAttack;// reference to m_attack field of MentalStateMod
	static jfieldID        ms_fidMentalStateModDecay;// reference to m_decay field of MentalStateMod
	static GlobalArrayRefPtr ms_mentalStateModList[MAX_RECURSION_COUNT];	// global MentalStateMod array for sending list of MentalStateMods

	static jclass          ms_clsLocation;		// reference to location
	static jclass          ms_clsLocationArray;	// reference to location[]
	static jfieldID        ms_fidLocationX;		// reference to x field of location
	static jfieldID        ms_fidLocationY;		// reference to y field of location
	static jfieldID        ms_fidLocationZ;		// reference to z field of location
	static jfieldID        ms_fidLocationArea;  // reference to planetName field of location
	static jfieldID        ms_fidLocationCell;  // reference to cell field of location

	static jmethodID       ms_midRunOne;            // methodID for script_entry.runScript()
	static jmethodID       ms_midRunAll;            // methodID for script_entry.runScripts()
	static jmethodID       ms_midCallMessages;      // methodID for script_entry.callMessageHandlers()
	static jmethodID       ms_midRunConsoleHandler; // methodID for script_entry.runConsoleHandler()
	static jmethodID       ms_midUnload;		    // methodID for script_entry.unloadClass()
	static jmethodID       ms_midGetClass;		    // methodID for script_entry.getClass()
	static jmethodID       ms_midGetScriptFunctions;// methodID for script_entry.getScriptFunctions()

	static jclass          ms_clsMenuInfo;
	static jmethodID       ms_midMenuInfo;
	static jmethodID       ms_midMenuInfoSetMenuItemsInternal;
	static jmethodID       ms_midMenuInfoGetMenuItemsInternal;
	static jclass          ms_clsMenuInfoData;
	static jmethodID       ms_midMenuInfoData;
	static jfieldID        ms_fidMenuInfoDataId;
	static jfieldID        ms_fidMenuInfoDataParent;
	static jfieldID        ms_fidMenuInfoDataType;
	static jfieldID        ms_fidMenuInfoDataLabel;
	static jfieldID        ms_fidMenuInfoDataEnabled;
	static jfieldID        ms_fidMenuInfoDataServerNotify;

	// customization-related class, method, field specification
	static jclass          ms_clsCustomVar;
	static jmethodID       ms_midCustomVarObjIdStringInt;
	static jclass          ms_clsRangedIntCustomVar;
	static jmethodID       ms_midRangedIntCustomVar;

	static jclass          ms_clsPalcolorCustomVar;
	static jmethodID       ms_midPalcolorCustomVar;

	static jclass          ms_clsColor;
	static jmethodID       ms_midColor;

	static jclass          ms_clsDraftSchematic;                    // reference to draft_schematic
	static jfieldID        ms_fidDraftSchematicCategory;            // reference to m_category field of draft_schematic
	static jfieldID        ms_fidDraftSchematicComplexity;			// reference to m_complexity field of draft_schematic
	static jfieldID        ms_fidDraftSchematicSlots;               // reference to m_slots field of draft_schematic
	static jfieldID        ms_fidDraftSchematicAttribs;             // reference to m_attribs field of draft_schematic
	static jfieldID        ms_fidDraftSchematicExperimentalAttribs; // reference to m_attribs field of draft_schematic
	static jfieldID        ms_fidDraftSchematicCustomizations;      // reference to m_customizations field of draft_schematic
	static jfieldID        ms_fidDraftSchematicAttribMap;           // reference to m_attribMap field of draft_schematic
	static jfieldID        ms_fidDraftSchematicObjectTemplateCreated;// reference to m_objectTemplateCreated field of draft_schematic
	static jfieldID        ms_fidDraftSchematicScripts;             // reference to m_scripts field of draft_schematic
	static jclass          ms_clsDraftSchematicSlot;                // reference to draft_schematic.slot
	static jfieldID        ms_fidDraftSchematicSlotName;            // reference to name field of draft_schematic.slot
	static jfieldID        ms_fidDraftSchematicSlotOption;          // reference to slotOption field of draft_schematic.slot
	static jfieldID        ms_fidDraftSchematicSlotIngredientType;  // reference to ingredientType field of draft_schematic.slot
	static jfieldID        ms_fidDraftSchematicSlotIngredientName;  // reference to ingredientName field of draft_schematic.slot
	static jfieldID        ms_fidDraftSchematicSlotIngredients;     // reference to ingredients field of draft_schematic.slot
	static jfieldID        ms_fidDraftSchematicSlotComplexity;      // reference to complexity field of draft_schematic.slot
	static jfieldID        ms_fidDraftSchematicSlotAmountRequired;  // reference to amountRequired field of draft_schematic.slot
	static jfieldID        ms_fidDraftSchematicSlotAppearance;      // reference to appearance field of draft_schematic.slot
	static jclass          ms_clsDraftSchematicAttrib;              // reference to draft_schematic.attribute
	static jfieldID        ms_fidDraftSchematicAttribName;          // reference to name field of draft_schematic.attribute
	static jfieldID        ms_fidDraftSchematicAttribMinValue;      // reference to minValue field of draft_schematic.attribute
	static jfieldID        ms_fidDraftSchematicAttribMaxValue;      // reference to maxValue field of draft_schematic.attribute
	static jfieldID        ms_fidDraftSchematicAttribResourceMaxValue;// reference to resourceMaxValue field of draft_schematic.attribute
	static jfieldID        ms_fidDraftSchematicAttribCurrentValue;  // reference to currentValue field of draft_schematic.attribute
	static jclass          ms_clsDraftSchematicSimpleIngredient;    // reference to draft_schematic.simple_ingredient
	static jfieldID        ms_fidDraftSchematicSimpleIngredientIngredient;  // reference to ingredient field of draft_schematic.simple_ingredient
	static jfieldID        ms_fidDraftSchematicSimpleIngredientCount;       // reference to count field of draft_schematic.simple_ingredient
	static jfieldID        ms_fidDraftSchematicSimpleIngredientSource;      // reference to source field of draft_schematic.simple_ingredient
	static jfieldID        ms_fidDraftSchematicSimpleIngredientXpType;      // reference to xpType field of draft_schematic.simple_ingredient
	static jfieldID        ms_fidDraftSchematicSimpleIngredientXpAmount;    // reference to xpAmount field of draft_schematic.simple_ingredient
	static jclass          ms_clsDraftSchematicCustom;              // reference to draft_schematic.custom
	static jfieldID        ms_fidDraftSchematicCustomName;          // reference to name field of draft_schematic.custom
	static jfieldID        ms_fidDraftSchematicCustomValue;         // reference to value field of draft_schematic.custom
	static jfieldID        ms_fidDraftSchematicCustomMinValue;      // reference to minValue field of draft_schematic.custom
	static jfieldID        ms_fidDraftSchematicCustomMaxValue;      // reference to maxValue field of draft_schematic.custom
//	static jfieldID        ms_fidDraftSchematicCustomLocked;        // reference to locked field of draft_schematic.custom

	static jclass          ms_clsMapLocation;
	static jmethodID       ms_midMapLocation;

	static jclass          ms_clsRegion;
	static jmethodID       ms_midRegion;
	static jfieldID        ms_fidRegionName;
	static jfieldID        ms_fidRegionPlanet;

	static jclass          ms_clsCombatEngine;
	static jclass          ms_clsCombatEngineCombatantData;
	static jfieldID        ms_fidCombatEngineCombatantDataPos;
	static jfieldID        ms_fidCombatEngineCombatantDataWorldPos;
	static jfieldID        ms_fidCombatEngineCombatantDataRadius;
	static jfieldID        ms_fidCombatEngineCombatantDataIsCreature;
	static jfieldID        ms_fidCombatEngineCombatantDataPosture;
	static jfieldID        ms_fidCombatEngineCombatantDataLocomotion;
	static jfieldID        ms_fidCombatEngineCombatantDataScriptMod;
	static jclass          ms_clsCombatEngineAttackerData;
	static jfieldID        ms_fidCombatEngineAttackerDataWeaponSkill;
	static jfieldID        ms_fidCombatEngineAttackerDataAims;
	static jclass          ms_clsCombatEngineDefenderData;
	static jfieldID        ms_fidCombatEngineDefenderDataCombatSkeleton;
	static jfieldID        ms_fidCombatEngineDefenderDataCover;
	static jfieldID        ms_fidCombatEngineDefenderDataHitLocationChances;
	static jclass          ms_clsCombatEngineWeaponData;
	static jfieldID        ms_fidCombatEngineWeaponDataId;
	static jfieldID        ms_fidCombatEngineWeaponDataMinDamage;
	static jfieldID        ms_fidCombatEngineWeaponDataMaxDamage;
	static jfieldID        ms_fidCombatEngineWeaponDataWeaponType;
	static jfieldID        ms_fidCombatEngineWeaponDataAttackType;
	static jfieldID        ms_fidCombatEngineWeaponDataDamageType;
	static jfieldID        ms_fidCombatEngineWeaponDataElementalType;
	static jfieldID        ms_fidCombatEngineWeaponDataElementalValue;
	static jfieldID        ms_fidCombatEngineWeaponDataAttackSpeed;
	static jfieldID        ms_fidCombatEngineWeaponDataWoundChance;
	static jfieldID        ms_fidCombatEngineWeaponDataAccuracy;
	static jfieldID        ms_fidCombatEngineWeaponDataMinRange;
	static jfieldID        ms_fidCombatEngineWeaponDataMaxRange;
	static jfieldID        ms_fidCombatEngineWeaponDataDamageRadius;
	static jfieldID        ms_fidCombatEngineWeaponDataAttackCost;
	static jfieldID        ms_fidCombatEngineWeaponDataIsDisabled;
	static jclass          ms_clsCombatEngineHitResult;
	static jfieldID        ms_fidCombatEngineHitResultSuccess;
	static jfieldID        ms_fidCombatEngineHitResultCritical;
	static jfieldID        ms_fidCombatEngineHitResultGlancing;
	static jfieldID        ms_fidCombatEngineHitResultCrushing;
	static jfieldID        ms_fidCombatEngineHitResultStrikethrough;
	static jfieldID        ms_fidCombatEngineHitResultStrikethroughAmount;
	static jfieldID        ms_fidCombatEngineHitResultEvadeResult;
	static jfieldID        ms_fidCombatEngineHitResultEvadeAmount;
	static jfieldID        ms_fidCombatEngineHitResultBlockResult;
	static jfieldID        ms_fidCombatEngineHitResultBlock;
	static jfieldID        ms_fidCombatEngineHitResultDodge;
	static jfieldID        ms_fidCombatEngineHitResultParry;
	static jfieldID        ms_fidCombatEngineHitResultProc;
	static jfieldID        ms_fidCombatEngineHitResultBaseRoll;
	static jfieldID        ms_fidCombatEngineHitResultFinalRoll;
	static jfieldID        ms_fidCombatEngineHitResultAttackVal;
	static jfieldID        ms_fidCombatEngineHitResultCanSee;
	static jfieldID        ms_fidCombatEngineHitResultHitLocation;
	static jfieldID        ms_fidCombatEngineHitResultDamage;
	static jfieldID        ms_fidCombatEngineHitResultDamageType;
	static jfieldID        ms_fidCombatEngineHitResultRawDamage;
	static jfieldID        ms_fidCombatEngineHitResultElementalDamage;
	static jfieldID        ms_fidCombatEngineHitResultElementalDamageType;
	static jfieldID        ms_fidCombatEngineHitResultBleedDamage;
	static jfieldID        ms_fidCombatEngineHitResultCritDamage;
	static jfieldID        ms_fidCombatEngineHitResultBlockedDamage;
	static jfieldID        ms_fidCombatEngineHitResultBlockingArmor;
	static jfieldID        ms_fidCombatEngineHitResultBleedingChance;

	static jclass          ms_clsTransform;
	static jclass          ms_clsTransformArray;
	static jmethodID       ms_midTransform;
	static jfieldID        ms_fidTransformMatrix;

	static jclass          ms_clsVector;
	static jclass          ms_clsVectorArray;
	static jfieldID        ms_fidVectorX;
	static jfieldID        ms_fidVectorY;
	static jfieldID        ms_fidVectorZ;

	static jclass          ms_clsResourceDensity;
	static jfieldID        ms_fidResourceDensityResourceType;
	static jfieldID        ms_fidResourceDensityDensity;

	static jclass          ms_clsResourceAttribute;
	static jfieldID        ms_fidResourceAttributeName;
	static jfieldID        ms_fidResourceAttributeValue;

	static jclass	       ms_clsLibrarySpaceTransition;
	static jmethodID       ms_midLibrarySpaceTransitionSetPlayerOvert;
	static jmethodID       ms_midLibrarySpaceTransitionClearOvertStatus;

	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
		// java classes/methods needed by CS Handlers.  These classes may not be CS specific
		// if there is no previous call into them specifically.

	static jclass          ms_clsLibraryDump;
	static jmethodID       ms_midLibraryDumpDumpTargetInfo;

	static jclass	       ms_clsLibraryGMLib;
	static jmethodID       ms_midLibraryGMLibFreeze;
	static jmethodID       ms_midLibraryGMLibUnfreeze;

	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////

		// flag that the JVM was loaded; once it is, it can never be loaded again
		// without restarting the program
	volatile static int         ms_loaded;
	static Semaphore * ms_shutdownJava;

	// Java initialization functions
	void initializeJavaThread();
	bool connectToJava();
	void disconnectFromJava();
	void resetJavaConnection();

	// entry point to Java VM
	static jint handleScriptEntryCleanup(jint result);
	static jint callScriptEntry(const JavaStringParam & script, const JavaStringParam & method, jobjectArray params);
	static jint callScriptEntry(const JavaStringParam & method, jobjectArray params);
	static jstring callScriptConsoleHandlerEntry(const JavaStringParam & script, const JavaStringParam & method, jobjectArray params);

	// obj_id functions
public:
	static LocalRefPtr     getObjId(const ServerObject & object);
	static LocalRefPtr     getObjId(const NetworkId::NetworkIdType & id);
	static LocalRefPtr     getObjId(const NetworkId & id);

	static const NetworkId getNetworkId(const jobject & source);
	static const NetworkId getNetworkId(const LocalRefParam & source);

	static LocalRefPtr     getVector(Vector const & vector);

	/////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////
	static void	       spaceMakeOvert(const NetworkId &player);
	static void	       spaceClearOvert(const NetworkId &ship);

	static std::string     getObjectDumpInfo(NetworkId id);

	static void	       freezePlayer(const NetworkId &id);
	static void	       unFreezePlayer(const NetworkId &id);

	/////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////

private:

	// misc support functions
	static LocalObjectArrayRefPtr convert(const NetworkId & caller, const std::string& argList, const ScriptParams &args);
	static LocalObjectArrayRefPtr convert(const std::string& argList, const ScriptParams &args);
	static bool                   convert(LocalObjectArrayRefPtr & javaParams, int startIndex, const std::string& argList, const ScriptParams &args);
	static void                   alterScriptParams(jobjectArray jparams, const std::string& argList, ScriptParams &args);
};

// ----------------------------------------------------------------------
inline JavaLibrary* JavaLibrary::instance(void)
{
	return ms_instance;
}	// JavaLibrary::Instance

inline JavaLibrary::GlobalInstances::GlobalInstances(void) :
	m_stringIdCount(0),
	m_attribModCount(0),
	m_mentalStateModCount(0),
	m_modifiableIntCount(0),
	m_modifiableFloatCount(0),
	m_modifiableStringIdCount(0)
{
}	// JavaLibrary::GlobalInstances::GlobalInstances

inline JavaLibrary::GlobalInstances::~GlobalInstances()
{
	ms_stringIdIndex -= m_stringIdCount;
	ms_attribModIndex -= m_attribModCount;
	ms_mentalStateModIndex -= m_mentalStateModCount;
	ms_modifiableIntIndex -= m_modifiableIntCount;
	ms_modifiableFloatIndex -= m_modifiableFloatCount;
	ms_modifiableStringIdIndex -= m_modifiableStringIdCount;
}	// JavaLibrary::GlobalInstances::~GlobalInstances

inline GlobalRefPtr JavaLibrary::GlobalInstances::getNextStringId(void)
{
	if (ms_stringIdIndex == MAX_STRING_ID_PARAMS)
	{
		DEBUG_REPORT_LOG(true, ("Too many stringId params, max = %d\n",
			MAX_STRING_ID_PARAMS));
		return GlobalRef::cms_nullPtr;
	}
	++m_stringIdCount;
	return ms_stringIds[ms_stringIdIndex++];
}	// JavaLibrary::GlobalInstances::getNextStringId

inline GlobalRefPtr JavaLibrary::GlobalInstances::getNextAttribMod(void)
{
	if (ms_attribModIndex == MAX_ATTRIB_MOD_PARAMS)
	{
		DEBUG_REPORT_LOG(true, ("Too many attribMod params, max = %d\n",
			MAX_ATTRIB_MOD_PARAMS));
		return GlobalRef::cms_nullPtr;
	}
	++m_attribModCount;
	return ms_attribMods[ms_attribModIndex++];
}	// JavaLibrary::GlobalInstances::getNextAttribMod

inline GlobalRefPtr JavaLibrary::GlobalInstances::getNextModifiableInt(void)
{
	if (ms_modifiableIntIndex == MAX_MODIFIABLE_INT_PARAMS)
	{
		DEBUG_REPORT_LOG(true, ("Too many modifiableInt params, max = %d\n",
			MAX_MODIFIABLE_INT_PARAMS));
		return GlobalRef::cms_nullPtr;
	}
	++m_modifiableIntCount;
	return ms_modifiableInts[ms_modifiableIntIndex++];
}	// JavaLibrary::GlobalInstances::getNextModifiableInt

inline GlobalRefPtr JavaLibrary::GlobalInstances::getNextModifiableFloat(void)
{
	if (ms_modifiableFloatIndex == MAX_MODIFIABLE_FLOAT_PARAMS)
	{
		DEBUG_REPORT_LOG(true, ("Too many modifiableFloat params, max = %d\n",
			MAX_MODIFIABLE_FLOAT_PARAMS));
		return GlobalRef::cms_nullPtr;
	}
	++m_modifiableFloatCount;
	return ms_modifiableFloats[ms_modifiableFloatIndex++];
}	// JavaLibrary::GlobalInstances::getNextModifiableFloat

inline GlobalRefPtr JavaLibrary::GlobalInstances::getNextModifiableStringId(void)
{
	if (ms_modifiableStringIdIndex == MAX_MODIFIABLE_STRING_ID_PARAMS)
	{
		DEBUG_REPORT_LOG(true, ("Too many modifiableStringId params, max = %d\n",
			MAX_MODIFIABLE_STRING_ID_PARAMS));
		return GlobalRef::cms_nullPtr;
	}
	++m_modifiableStringIdCount;
	return ms_modifiableStringIds[ms_modifiableStringIdIndex++];
}	// JavaLibrary::GlobalInstances::getNextModifiableStringId

inline GlobalRefPtr JavaLibrary::GlobalInstances::getNextMentalStateMod(void)
{
	if (ms_mentalStateModIndex == MAX_MENTAL_STATE_MOD_PARAMS)
	{
		DEBUG_REPORT_LOG(true, ("Too many mental_state_mod params, max = %d\n",
			MAX_MENTAL_STATE_MOD_PARAMS));
		return GlobalRef::cms_nullPtr;
	}
	++m_mentalStateModCount;
	return ms_mentalStateMods[ms_mentalStateModIndex++];
}	// JavaLibrary::GlobalInstances::getNextMentalStateMod

inline GlobalRefPtr JavaLibrary::GlobalInstances::getMenuInfo()
{
	return ms_menuInfo;
}

// ----------------------------------------------------------------------

inline JNIEnv * JavaLibrary::getEnv(void)
{
	return ms_env;
}

// ----------------------------------------------------------------------

inline int JavaLibrary::getJavaVmType()
{
	return ms_javaVmType;
}

// ----------------------------------------------------------------------

namespace ScriptConversion
{
	inline const bool getClassName(const jobject & source, std::string & target)
	{
		return JavaLibrary::getClassName(source, target);
	}
};

//----------------------------------------------------------------------

#define JAVA_THROW_SCRIPT_EXCEPTION(a, b) ((a) ? JavaLibrary::throwScriptException b : NOP)

//----------------------------------------------------------------------

inline jclass    JavaLibrary::getClsAttribute()
{
	return ms_clsAttribute;
}

inline jmethodID JavaLibrary::getMidAttribute()
{
	return ms_midAttribute;
}

inline jfieldID  JavaLibrary::getFidAttributeType()
{
	return ms_fidAttributeType;
}

inline jfieldID  JavaLibrary::getFidAttributeValue()
{
	return ms_fidAttributeValue;
}

inline jclass    JavaLibrary::getClsBaseClassRangeInfo()
{
	return ms_clsBaseClassRangeInfo;
}

inline jfieldID  JavaLibrary::getFidBaseClassRangeInfoMinRange()
{
	return ms_fidBaseClassRangeInfoMinRange;
}

inline jfieldID  JavaLibrary::getFidBaseClassRangeInfoMaxRange()
{
	return ms_fidBaseClassRangeInfoMaxRange;
}

inline jfieldID  JavaLibrary::getFidBaseClassAttackerResultsId()
{
	return ms_fidBaseClassAttackerResultsId;
}

inline jfieldID  JavaLibrary::getFidBaseClassAttackerResultsWeapon()
{
	return ms_fidBaseClassAttackerResultsWeapon;
}

inline jfieldID  JavaLibrary::getFidBaseClassAttackerResultsPosture()
{
	return ms_fidBaseClassAttackerResultsPosture;
}

inline jfieldID  JavaLibrary::getFidBaseClassAttackerResultsTrailBits()
{
	return ms_fidBaseClassAttackerResultsTrailBits;
}

inline jfieldID  JavaLibrary::getFidBaseClassAttackerResultsClientEffectId()
{
	return ms_fidBaseClassAttackerResultsClientEffectId;
}

inline jfieldID  JavaLibrary::getFidBaseClassAttackerResultsActionName()
{
	return ms_fidBaseClassAttackerResultsActionName;
}

inline jfieldID  JavaLibrary::getFidBaseClassAttackerResultsUseLocation()
{
	return ms_fidBaseClassAttackerResultsUseLocation;
}

inline jfieldID  JavaLibrary::getFidBaseClassAttackerResultsTargetLocation()
{
	return ms_fidBaseClassAttackerResultsTargetLocation;
}

inline jfieldID  JavaLibrary::getFidBaseClassAttackerResultsTargetCell()
{
	return ms_fidBaseClassAttackerResultsTargetCell;
}

inline jfieldID  JavaLibrary::getFidBaseClassDefenderDamageAmount()
{
	return ms_fidBaseClassDefenderDamageAmount;
}

inline jfieldID  JavaLibrary::getFidBaseClassDefenderResultsHitLocation()
{
	return ms_fidBaseClassDefenderResultsHitLocation;
}

inline jfieldID  JavaLibrary::getFidBaseClassDefenderResultsId()
{
	return ms_fidBaseClassDefenderResultsId;
}

inline jfieldID  JavaLibrary::getFidBaseClassDefenderResultsPosture()
{
	return ms_fidBaseClassDefenderResultsPosture;
}

inline jfieldID  JavaLibrary::getFidBaseClassDefenderResultsResult()
{
	return ms_fidBaseClassDefenderResultsResult;
}

inline jfieldID  JavaLibrary::getFidBaseClassDefenderResultsClientEffectId()
{
	return ms_fidBaseClassDefenderResultsClientEffectId;
}

inline jclass    JavaLibrary::getClsColor()
{
	return ms_clsColor;
}

inline jmethodID JavaLibrary::getMidColor()
{
	return ms_midColor;
}

inline jfieldID  JavaLibrary::getFidCombatEngineAttackerDataWeaponSkill()
{
	return ms_fidCombatEngineAttackerDataWeaponSkill;
}

inline jfieldID  JavaLibrary::getFidCombatEngineAttackerDataAims()
{
	return ms_fidCombatEngineAttackerDataAims;
}

inline jfieldID  JavaLibrary::getFidCombatEngineCombatantDataPos()
{
	return ms_fidCombatEngineCombatantDataPos;
}

inline jfieldID  JavaLibrary::getFidCombatEngineCombatantDataWorldPos()
{
	return ms_fidCombatEngineCombatantDataWorldPos;
}

inline jfieldID  JavaLibrary::getFidCombatEngineCombatantDataRadius()
{
	return ms_fidCombatEngineCombatantDataRadius;
}

inline jfieldID  JavaLibrary::getFidCombatEngineCombatantDataIsCreature()
{
	return ms_fidCombatEngineCombatantDataIsCreature;
}

inline jfieldID  JavaLibrary::getFidCombatEngineCombatantDataPosture()
{
	return ms_fidCombatEngineCombatantDataPosture;
}

inline jfieldID  JavaLibrary::getFidCombatEngineCombatantDataLocomotion()
{
	return ms_fidCombatEngineCombatantDataLocomotion;
}

inline jfieldID  JavaLibrary::getFidCombatEngineCombatantDataScriptMod()
{
	return ms_fidCombatEngineCombatantDataScriptMod;
}

inline jfieldID  JavaLibrary::getFidCombatEngineDefenderDataCombatSkeleton()
{
	return ms_fidCombatEngineDefenderDataCombatSkeleton;
}

inline jfieldID  JavaLibrary::getFidCombatEngineDefenderDataCover()
{
	return ms_fidCombatEngineDefenderDataCover;
}

inline jfieldID  JavaLibrary::getFidCombatEngineDefenderDataHitLocationChances()
{
	return ms_fidCombatEngineDefenderDataHitLocationChances;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultRawDamage()
{
	return ms_fidCombatEngineHitResultRawDamage;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultDamageType()
{
	return ms_fidCombatEngineHitResultDamageType;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultElementalDamage()
{
	return ms_fidCombatEngineHitResultElementalDamage;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultElementalDamageType()
{
	return ms_fidCombatEngineHitResultElementalDamageType;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultBleedDamage()
{
	return ms_fidCombatEngineHitResultBleedDamage;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultCritDamage()
{
	return ms_fidCombatEngineHitResultCritDamage;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultBlockedDamage()
{
	return ms_fidCombatEngineHitResultBlockedDamage;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultDamage()
{
	return ms_fidCombatEngineHitResultDamage;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultHitLocation()
{
	return ms_fidCombatEngineHitResultHitLocation;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultSuccess()
{
	return ms_fidCombatEngineHitResultSuccess;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultCritical()
{
	return ms_fidCombatEngineHitResultCritical;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultGlancing()
{
	return ms_fidCombatEngineHitResultGlancing;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultCrushing()
{
	return ms_fidCombatEngineHitResultCrushing;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultStrikethrough()
{
	return ms_fidCombatEngineHitResultStrikethrough;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultStrikethroughAmount()
{
	return ms_fidCombatEngineHitResultStrikethroughAmount;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultEvadeResult()
{
	return ms_fidCombatEngineHitResultEvadeResult;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultEvadeAmount()
{
	return ms_fidCombatEngineHitResultEvadeAmount;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultBlockResult()
{
	return ms_fidCombatEngineHitResultBlockResult;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultBlock()
{
	return ms_fidCombatEngineHitResultBlock;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultDodge()
{
	return ms_fidCombatEngineHitResultDodge;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultParry()
{
	return ms_fidCombatEngineHitResultParry;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultProc()
{
	return ms_fidCombatEngineHitResultProc;
}

inline jfieldID  JavaLibrary::getFidCombatEngineHitResultBlockingArmor()
{
	return ms_fidCombatEngineHitResultBlockingArmor;
}

inline jclass    JavaLibrary::getClsCustomVar()
{
	return ms_clsCustomVar;
}

inline jmethodID JavaLibrary::getMidCustomVarObjIdStringInt()
{
	return ms_midCustomVarObjIdStringInt;
}

inline jclass    JavaLibrary::getClsDraftSchematic()
{
	return ms_clsDraftSchematic;
}

inline jclass    JavaLibrary::getClsDraftSchematicAttrib()
{
	return ms_clsDraftSchematicAttrib;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicAttribs()
{
	return ms_fidDraftSchematicAttribs;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicAttribMap()
{
	return ms_fidDraftSchematicAttribMap;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicAttribName()
{
	return ms_fidDraftSchematicAttribName;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicAttribCurrentValue()
{
	return ms_fidDraftSchematicAttribCurrentValue;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicAttribResourceMaxValue()
{
	return ms_fidDraftSchematicAttribResourceMaxValue;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicAttribMinValue()
{
	return ms_fidDraftSchematicAttribMinValue;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicAttribMaxValue()
{
	return ms_fidDraftSchematicAttribMaxValue;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicCustomName()
{
	return ms_fidDraftSchematicCustomName;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicCustomValue()
{
	return ms_fidDraftSchematicCustomValue;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicCustomMinValue()
{
	return ms_fidDraftSchematicCustomMinValue;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicCustomMaxValue()
{
	return ms_fidDraftSchematicCustomMaxValue;
}

inline jfieldID  JavaLibrary::getFidDraftSchematicExperimentalAttribs()
{
	return ms_fidDraftSchematicExperimentalAttribs;
}

inline jclass    JavaLibrary::getClsDynamicVariable()
{
	return ms_clsDynamicVariable;
}

inline jmethodID JavaLibrary::getMidDynamicVariableFloat()
{
	return ms_midDynamicVariableFloat;
}

inline jmethodID JavaLibrary::getMidDynamicVariableFloatArray()
{
	return ms_midDynamicVariableFloatArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableInt()
{
	return ms_midDynamicVariableInt;
}

inline jmethodID JavaLibrary::getMidDynamicVariableIntArray()
{
	return ms_midDynamicVariableIntArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableLocation()
{
	return ms_midDynamicVariableLocation;
}

inline jmethodID JavaLibrary::getMidDynamicVariableLocationArray()
{
	return ms_midDynamicVariableLocationArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableObjId()
{
	return ms_midDynamicVariableObjId;
}

inline jmethodID JavaLibrary::getMidDynamicVariableObjIdArray()
{
	return ms_midDynamicVariableObjIdArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableString()
{
	return ms_midDynamicVariableString;
}

inline jmethodID JavaLibrary::getMidDynamicVariableStringArray()
{
	return ms_midDynamicVariableStringArray;
}

inline jclass    JavaLibrary::getClsDynamicVariableList()
{
	return ms_clsDynamicVariableList;
}

inline jmethodID JavaLibrary::getMidDynamicVariableList()
{
	return ms_midDynamicVariableList;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSet()
{
	return ms_midDynamicVariableListSet;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetFloat()
{
	return ms_midDynamicVariableListSetFloat;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetFloatArray()
{
	return ms_midDynamicVariableListSetFloatArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetInt()
{
	return ms_midDynamicVariableListSetInt;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetIntArray()
{
	return ms_midDynamicVariableListSetIntArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetLocation()
{
	return ms_midDynamicVariableListSetLocation;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetLocationArray()
{
	return ms_midDynamicVariableListSetLocationArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetObjId()
{
	return ms_midDynamicVariableListSetObjId;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetObjIdArray()
{
	return ms_midDynamicVariableListSetObjIdArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetString()
{
	return ms_midDynamicVariableListSetString;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetStringArray()
{
	return ms_midDynamicVariableListSetStringArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetStringId()
{
	return ms_midDynamicVariableListSetStringId;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetStringIdArray()
{
	return ms_midDynamicVariableListSetStringIdArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetTransform()
{
	return ms_midDynamicVariableListSetTransform;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetTransformArray()
{
	return ms_midDynamicVariableListSetTransformArray;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetVector()
{
	return ms_midDynamicVariableListSetVector;
}

inline jmethodID JavaLibrary::getMidDynamicVariableListSetVectorArray()
{
	return ms_midDynamicVariableListSetVectorArray;
}

inline jclass    JavaLibrary::getClsBoolean()
{
	return ms_clsBoolean;
}

inline jmethodID JavaLibrary::getMidBooleanBooleanValue()
{
	return ms_midBooleanBooleanValue;
}

inline jclass    JavaLibrary::getClsDictionary()
{
	return ms_clsDictionary;
}

inline jmethodID JavaLibrary::getMidDictionary()
{
	return ms_midDictionary;
}

inline jmethodID JavaLibrary::getMidDictionaryGet()
{
	return ms_midDictionaryGet;
}

inline jmethodID JavaLibrary::getMidDictionaryKeys()
{
	return ms_midDictionaryKeys;
}

inline jmethodID JavaLibrary::getMidDictionaryPut()
{
	return ms_midDictionaryPut;
}

inline jmethodID JavaLibrary::getMidDictionaryPutFloat()
{
	return ms_midDictionaryPutFloat;
}

inline jmethodID JavaLibrary::getMidDictionaryPutInt()
{
	return ms_midDictionaryPutInt;
}

inline jmethodID JavaLibrary::getMidEnumerationHasMoreElements()
{
	return ms_midEnumerationHasMoreElements;
}

inline jmethodID JavaLibrary::getMidEnumerationNextElement()
{
	return ms_midEnumerationNextElement;
}

inline jclass    JavaLibrary::getClsFloat()
{
	return ms_clsFloat;
}

inline jmethodID JavaLibrary::getMidFloatFloatValue()
{
	return ms_midFloatFloatValue;
}

inline jclass    JavaLibrary::getClsHashtable()
{
	return ms_clsHashtable;
}

inline jmethodID JavaLibrary::getMidHashtable()
{
	return ms_midHashtable;
}

inline jclass    JavaLibrary::getClsInteger()
{
	return ms_clsInteger;
}

inline jmethodID JavaLibrary::getMidIntegerIntValue()
{
	return ms_midIntegerIntValue;
}

inline jclass    JavaLibrary::getClsLocation()
{
	return ms_clsLocation;
}

inline jclass    JavaLibrary::getClsLocationArray()
{
	return ms_clsLocationArray;
}

inline jfieldID  JavaLibrary::getFidLocationArea()
{
	return ms_fidLocationArea;
}

inline jfieldID  JavaLibrary::getFidLocationCell()
{
	return ms_fidLocationCell;
}

inline jfieldID  JavaLibrary::getFidLocationX()
{
	return ms_fidLocationX;
}

inline jfieldID  JavaLibrary::getFidLocationY()
{
	return ms_fidLocationY;
}

inline jfieldID  JavaLibrary::getFidLocationZ()
{
	return ms_fidLocationZ;
}

inline jmethodID JavaLibrary::getMidMapPut()
{
	return ms_midMapPut;
}

inline jclass    JavaLibrary::getClsMapLocation()
{
	return ms_clsMapLocation;
}

inline jmethodID JavaLibrary::getMidMapLocation()
{
	return ms_midMapLocation;
}

inline jclass    JavaLibrary::getClsMentalState()
{
	return ms_clsMentalState;
}

inline jmethodID JavaLibrary::getMidMentalState()
{
	return ms_midMentalState;
}

inline jfieldID  JavaLibrary::getFidMentalStateType()
{
	return ms_fidMentalStateType;
}

inline jfieldID  JavaLibrary::getFidMentalStateValue()
{
	return ms_fidMentalStateValue;
}

inline jmethodID JavaLibrary::getMidObjIdGetValue()
{
	return ms_midObjIdGetValue;
}

inline jmethodID JavaLibrary::getMidObjIdSetAuthoritative()
{
	return ms_midObjIdSetAuthoritative;
}

inline jclass    JavaLibrary::getClsPalcolorCustomVar()
{
	return ms_clsPalcolorCustomVar;
}

inline jmethodID JavaLibrary::getMidPalcolorCustomVar()
{
	return ms_midPalcolorCustomVar;
}

inline jclass    JavaLibrary::getClsRangedIntCustomVar()
{
	return ms_clsRangedIntCustomVar;
}

inline jmethodID JavaLibrary::getMidRangedIntCustomVar()
{
	return ms_midRangedIntCustomVar;
}

inline jclass    JavaLibrary::getClsRegion()
{
	return ms_clsRegion;
}

inline jclass    JavaLibrary::getClsResourceAttribute()
{
	return ms_clsResourceAttribute;
}

inline jfieldID  JavaLibrary::getFidResourceAttributeName()
{
	return ms_fidResourceAttributeName;
}

inline jfieldID  JavaLibrary::getFidResourceAttributeValue()
{
	return ms_fidResourceAttributeValue;
}

inline jclass    JavaLibrary::getClsResourceDensity()
{
	return ms_clsResourceDensity;
}

inline jfieldID  JavaLibrary::getFidResourceDensityDensity()
{
	return ms_fidResourceDensityDensity;
}

inline jfieldID  JavaLibrary::getFidResourceDensityResourceType()
{
	return ms_fidResourceDensityResourceType;
}

inline jclass    JavaLibrary::getClsScriptEntry()
{
	return ms_clsScriptEntry;
}

inline jmethodID JavaLibrary::getMidScriptEntryGetOwnerContext()
{
	return ms_midScriptEntryGetOwnerContext;
}

inline jclass    JavaLibrary::getClsString()
{
	return ms_clsString;
}

inline jclass    JavaLibrary::getClsStringArray()
{
	return ms_clsStringArray;
}

inline jclass    JavaLibrary::getClsStringId()
{
	return ms_clsStringId;
}

inline jclass    JavaLibrary::getClsStringIdArray()
{
	return ms_clsStringIdArray;
}

inline jfieldID  JavaLibrary::getFidStringIdAsciiId()
{
	return ms_fidStringIdAsciiId;
}

inline jfieldID  JavaLibrary::getFidStringIdTable()
{
	return ms_fidStringIdTable;
}

inline jclass    JavaLibrary::getClsTransform()
{
	return ms_clsTransform;
}

inline jclass    JavaLibrary::getClsTransformArray()
{
	return ms_clsTransformArray;
}

inline jclass    JavaLibrary::getClsVector()
{
	return ms_clsVector;
}

inline jclass    JavaLibrary::getClsVectorArray()
{
	return ms_clsVectorArray;
}

#endif	// _INCLUDED_JavaLibrary_H
