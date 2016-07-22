//========================================================================
//
// JavaLibrary.cpp - interface to the JVM via JNI.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/AttribModNameManager.h"
#include "serverGame/CellPermissions.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/ConfigServerScript.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptListEntry.h"
#include "serverScript/ScriptParameters.h"
#include "sharedDebug/DebugHelp.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/TextManager.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ObjectMenuRequestData.h" //@todo move this header into a different library (sharedUserInterface)
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedSynchronization/Semaphore.h"
#include "sharedThread/RunThread.h"
#include "sharedUtility/ValueDictionary.h"

using namespace JNIWrappersNamespace;

#if !defined(linux) && !defined(WIN32)
#error Unsupported platform
#endif

#ifdef linux
// shared library includes
#include <dlfcn.h>
#include <signal.h>
#endif

//========================================================================
// macros
//========================================================================


#define GET_CLASS(var, name) tempClass = ms_env->FindClass(name); if (ms_env->ExceptionCheck()) { ms_env->ExceptionDescribe(); WARNING(true, ("Unable to find Java class "#name)); return false; } var = static_cast<jclass>(ms_env->NewGlobalRef(tempClass)); ms_env->DeleteLocalRef(tempClass); if (var == 0) { DEBUG_WARNING(true, ("Unable to create global reference for JavaLibrary " #var )); return false; }
#define GET_FIELD(var, clazz, name, sig) var = ms_env->GetFieldID(clazz, name, sig); if (ms_env->ExceptionCheck()) { ms_env->ExceptionDescribe(); WARNING(true, ("Unable to find Java field "#name" for class "#clazz)); return false; }
#define GET_METHOD(var, clazz, name, sig) var = ms_env->GetMethodID(clazz, name, sig); if (ms_env->ExceptionCheck()) { ms_env->ExceptionDescribe(); WARNING(true, ("Unable to find Java method "#name" for class "#clazz)); return false; }
#define GET_STATIC_METHOD(var, clazz, name, sig) var = ms_env->GetStaticMethodID(clazz, name, sig); if (ms_env->ExceptionCheck()) { ms_env->ExceptionDescribe(); WARNING(true, ("Unable to find Java static method "#name" for class "#clazz)); return false; }

#define FREE_CLASS(var) if (ms_env != nullptr && var != nullptr) { ms_env->DeleteGlobalRef(var); var = nullptr; }

//========================================================================
// local constants
//========================================================================


// set path to jvm
#if defined(WIN32)
#define PATH_SEPARATOR ";"
const char *JNI_DLL_PATH = "jvm.dll";
#else
#define PATH_SEPARATOR ":"
const char *JNI_DBG_DLL_PATH = "libjvm.so";//"libjvm_g.so";
const char *JNI_DLL_PATH = "libjvm.so";
#endif

extern "C" {
typedef jint (JNICALL *JNI_CREATEJAVAVMPROC)(JavaVM**, void**, void*);
}

//========================================================================
// JNI native function namespaces
//========================================================================


namespace ScriptMethodsActionStatesNamespace
{
	bool install();
}
namespace ScriptMethodsAiNamespace
{
	bool install();
}
namespace ScriptMethodsAnimationNamespace
{
	bool install();
}
namespace ScriptMethodsAttributesNamespace
{
	bool install();
}
namespace ScriptMethodsAuctionNamespace
{
	bool install();
}
namespace ScriptMethodsBankNamespace
{
	bool install();
}
namespace ScriptMethodsBeastMasterNamespace
{
	bool install();
}
namespace ScriptMethodsBroadcastingNamespace
{
	bool install();
}
namespace ScriptMethodsBuffBuilderNamespace
{
	bool install();
}
namespace ScriptMethodsBuffNamespace
{
	bool install();
}
namespace ScriptMethodsChatNamespace
{
	bool install();
}
namespace ScriptMethodsCityNamespace
{
	bool install();
}
namespace ScriptMethodsClientEffectNamespace
{
	bool install();
}
namespace ScriptMethodsClusterWideDataNamespace
{
	bool install();
}
namespace ScriptMethodsCollectionNamespace
{
	bool install();
}
namespace ScriptMethodsCombatNamespace
{
	bool install();
}
namespace ScriptMethodsCommandQueueNamespace
{
	bool install();
}
namespace ScriptMethodsConsoleNamespace
{
	bool install();
}
namespace ScriptMethodsContainersNamespace
{
	bool install();
}
namespace ScriptMethodsCraftingNamespace
{
	bool install();
}
namespace ScriptMethodsDataTableNamespace
{
	bool install();
}
namespace ScriptMethodsDebugNamespace
{
	bool install();
}
namespace ScriptMethodsDynamicVariableNamespace
{
	bool install();
}
namespace ScriptMethodsFormNamespace
{
	bool install();
}
namespace ScriptMethodsGlobalQueryNamespace
{
	bool install();
}
namespace ScriptMethodsGuildNamespace
{
	bool install();
}
namespace ScriptMethodsHateListNamespace
{
	bool install();
}
namespace ScriptMethodsHolocubeNamespace
{
	bool install();
}
namespace ScriptMethodsHyperspaceNamespace
{
	bool install();
}
namespace ScriptMethodsImageDesignNamespace
{
	bool install();
}
namespace ScriptMethodsInstallationNamespace
{
	bool install();
}
namespace ScriptMethodsInterestNamespace
{
	bool install();
}
namespace ScriptMethodsInteriorsNamespace
{
	bool install();
}
namespace ScriptMethodsJediNamespace
{
	bool install();
}
namespace ScriptMethodsMapNamespace
{
	bool install();
}
namespace ScriptMethodsMentalStatesNamespace
{
	bool install();
}
namespace ScriptMethodsMenuNamespace
{
	bool install();
}
namespace ScriptMethodsMissionNamespace
{
	bool install();
}
namespace ScriptMethodsMoneyNamespace
{
	bool install();
}
namespace ScriptMethodsMountNamespace
{
	bool install();
}
namespace ScriptMethodsNewbieTutorialNamespace
{
	bool install();
}
namespace ScriptMethodsNotificationNamespace
{
	bool install();
}
namespace ScriptMethodsNpcNamespace
{
	bool install();
}
namespace ScriptMethodsObjectCreateNamespace
{
	bool install();
}
namespace ScriptMethodsObjectInfoNamespace
{
	bool install();
}
namespace ScriptMethodsObjectMoveNamespace
{
	bool install();
}
namespace ScriptMethodsPermissionsNamespace
{
	bool install();
}
namespace ScriptMethodsPilotNamespace
{
	bool install();
}
namespace ScriptMethodsPlanetNamespace
{
	bool install();
}
namespace ScriptMethodsPlayerAccountNamespace
{
	bool install();
}
namespace ScriptMethodsPlayerQuestNamespace
{
	bool install();
}
namespace ScriptMethodsPvpNamespace
{
	bool install();
}
namespace ScriptMethodsQuestNamespace
{
	bool install();
}
namespace ScriptMethodsRegionNamespace
{
	bool install();
}
namespace ScriptMethodsRegion3dNamespace
{
	bool install();
}
namespace ScriptMethodsRemoteDebugNamespace
{
	bool install();
}
namespace ScriptMethodsResourceNamespace
{
	bool install();
}
namespace ScriptMethodsScriptNamespace
{
	bool install();
}
namespace ScriptMethodsServerUINamespace
{
	bool install();
}
namespace ScriptMethodsShipNamespace
{
	bool install();
}
namespace ScriptMethodsSkillNamespace
{
	bool install();
}
namespace ScriptMethodsSpawnerNamespace
{
	bool install();
}
namespace ScriptMethodsStringNamespace
{
	bool install();
}
namespace ScriptMethodsSystemNamespace
{
	bool install();
}
namespace ScriptMethodsTerrainNamespace
{
	bool install();
}
namespace ScriptMethodsTriggerVolumeNamespace
{
	bool install();
}
namespace ScriptMethodsVeteranNamespace
{
	bool install();
}
namespace ScriptMethodsWaypointNamespace
{
	bool install();
}
namespace ScriptMethodsWorldInfoNamespace
{
	bool install();
}

//========================================================================
// class JavaLibrary static members
//========================================================================

JavaLibrary* JavaLibrary::ms_instance = nullptr;
int          JavaLibrary::ms_javaVmType = JV_none;
//void*        JavaLibrary::ms_libHandle = nullptr;
JavaVM*      JavaLibrary::ms_jvm = nullptr;
JNIEnv*      JavaLibrary::ms_env = nullptr;
Thread *     JavaLibrary::m_initializerThread = nullptr;
int          JavaLibrary::ms_envCount = 0;
int          JavaLibrary::ms_currentRecursionCount = 0;
bool         JavaLibrary::ms_resetJava = false;
jclass       JavaLibrary::ms_clsScriptEntry = nullptr;
jobject      JavaLibrary::ms_scriptEntry = nullptr;
jmethodID    JavaLibrary::ms_midScriptEntryGetOwnerContext = nullptr;
jmethodID    JavaLibrary::ms_midScriptEntryEnableLogging = nullptr;
jmethodID    JavaLibrary::ms_midScriptEntryEnableNewJediTracking = nullptr;
jmethodID    JavaLibrary::ms_midScriptEntryGetFreeMem = nullptr;
jclass       JavaLibrary::ms_clsObject = nullptr;
jclass       JavaLibrary::ms_clsClass = nullptr;
jmethodID    JavaLibrary::ms_midClassGetName = nullptr;
jmethodID    JavaLibrary::ms_midClassGetMethods = nullptr;
jclass       JavaLibrary::ms_clsMethod = nullptr;
jmethodID    JavaLibrary::ms_midMethodGetName = nullptr;
jclass       JavaLibrary::ms_clsBoolean = nullptr;
jclass       JavaLibrary::ms_clsBooleanArray = nullptr;
jmethodID    JavaLibrary::ms_midBoolean = nullptr;
jmethodID    JavaLibrary::ms_midBooleanBooleanValue = nullptr;
jclass       JavaLibrary::ms_clsInteger = nullptr;
jclass       JavaLibrary::ms_clsIntegerArray = nullptr;
jmethodID    JavaLibrary::ms_midInteger = nullptr;
jmethodID    JavaLibrary::ms_midIntegerIntValue = nullptr;
jclass       JavaLibrary::ms_clsModifiableInt = nullptr;
jmethodID    JavaLibrary::ms_midModifiableInt = nullptr;
jfieldID     JavaLibrary::ms_fidModifiableIntData = nullptr;
jclass       JavaLibrary::ms_clsFloat = nullptr;
jclass       JavaLibrary::ms_clsFloatArray = nullptr;
jmethodID    JavaLibrary::ms_midFloat = nullptr;
jmethodID    JavaLibrary::ms_midFloatFloatValue = nullptr;
jclass       JavaLibrary::ms_clsModifiableFloat = nullptr;
jmethodID    JavaLibrary::ms_midModifiableFloat = nullptr;
jfieldID     JavaLibrary::ms_fidModifiableFloatData = nullptr;
jclass       JavaLibrary::ms_clsString = nullptr;
jclass       JavaLibrary::ms_clsStringArray = nullptr;
jclass       JavaLibrary::ms_clsMap = nullptr;
jmethodID    JavaLibrary::ms_midMapPut = nullptr;
jmethodID    JavaLibrary::ms_midMapGet = nullptr;
jclass       JavaLibrary::ms_clsHashtable = nullptr;
jmethodID    JavaLibrary::ms_midHashtable = nullptr;
jclass       JavaLibrary::ms_clsThrowable = nullptr;
jclass       JavaLibrary::ms_clsError = nullptr;
jclass       JavaLibrary::ms_clsStackOverflowError = nullptr;
jmethodID    JavaLibrary::ms_midThrowableGetMessage = nullptr;
jclass       JavaLibrary::ms_clsThread = nullptr;
jmethodID    JavaLibrary::ms_midThreadDumpStack = nullptr;
jclass       JavaLibrary::ms_clsInternalScriptError = nullptr;
jclass       JavaLibrary::ms_clsInternalScriptSeriousError = nullptr;
jfieldID     JavaLibrary::ms_fidInternalScriptSeriousErrorError = nullptr;
jclass       JavaLibrary::ms_clsDictionary = nullptr;
jmethodID    JavaLibrary::ms_midDictionary = nullptr;
jmethodID    JavaLibrary::ms_midDictionaryPack = nullptr;
jmethodID    JavaLibrary::ms_midDictionaryUnpack = nullptr;
jmethodID    JavaLibrary::ms_midDictionaryKeys = nullptr;
jmethodID    JavaLibrary::ms_midDictionaryValues = nullptr;
jmethodID    JavaLibrary::ms_midDictionaryPut = nullptr;
jmethodID    JavaLibrary::ms_midDictionaryPutInt = nullptr;
jmethodID    JavaLibrary::ms_midDictionaryPutFloat = nullptr;
jmethodID    JavaLibrary::ms_midDictionaryPutBool = nullptr;
jmethodID    JavaLibrary::ms_midDictionaryGet = nullptr;
jclass       JavaLibrary::ms_clsCollection = nullptr;
jmethodID    JavaLibrary::ms_midCollectionToArray = nullptr;
jclass       JavaLibrary::ms_clsEnumeration = nullptr;
jmethodID    JavaLibrary::ms_midEnumerationHasMoreElements = nullptr;
jmethodID    JavaLibrary::ms_midEnumerationNextElement = nullptr;
jclass       JavaLibrary::ms_clsBaseClassRangeInfo = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassRangeInfoMinRange = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassRangeInfoMaxRange = nullptr;
jclass       JavaLibrary::ms_clsBaseClassAttackerResults = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassAttackerResultsId = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassAttackerResultsWeapon = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassAttackerResultsPosture = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassAttackerResultsTrailBits = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassAttackerResultsClientEffectId = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassAttackerResultsActionName = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassAttackerResultsUseLocation = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassAttackerResultsTargetLocation = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassAttackerResultsTargetCell = nullptr;
jclass       JavaLibrary::ms_clsBaseClassDefenderResults = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassDefenderResultsId = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassDefenderResultsPosture = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassDefenderResultsResult = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassDefenderResultsClientEffectId = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassDefenderResultsHitLocation = nullptr;
jfieldID     JavaLibrary::ms_fidBaseClassDefenderDamageAmount = nullptr;
jclass       JavaLibrary::ms_clsDynamicVariable = nullptr;
jfieldID     JavaLibrary::ms_fidDynamicVariableName = nullptr;
jfieldID     JavaLibrary::ms_fidDynamicVariableData = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableInt = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableIntArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableFloat = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableFloatArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableString = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableStringArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableObjId = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableObjIdArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableLocation = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableLocationArray = nullptr;
jclass       JavaLibrary::ms_clsDynamicVariableList = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableList = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSet = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetInt = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetIntArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetFloat = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetFloatArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetString = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetStringArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetObjId = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetObjIdArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetLocation = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetLocationArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetStringId = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetStringIdArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetTransform = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetTransformArray = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetVector = nullptr;
jmethodID    JavaLibrary::ms_midDynamicVariableListSetVectorArray = nullptr;
jclass       JavaLibrary::ms_clsObjId = nullptr;
jclass       JavaLibrary::ms_clsObjIdArray = nullptr;
jmethodID    JavaLibrary::ms_midObjIdGetValue = nullptr;
jmethodID    JavaLibrary::ms_midObjIdGetObjId = nullptr;
jmethodID    JavaLibrary::ms_midObjIdClearObjId = nullptr;
jmethodID    JavaLibrary::ms_midObjIdFlagDestroyed = nullptr;
jmethodID    JavaLibrary::ms_midObjIdSetAuthoritative = nullptr;
jmethodID    JavaLibrary::ms_midObjIdSetLoaded = nullptr;
jmethodID    JavaLibrary::ms_midObjIdSetInitialized = nullptr;
jmethodID    JavaLibrary::ms_midObjIdSetLoggedIn = nullptr;
jmethodID    JavaLibrary::ms_midObjIdPackAllDeltaScriptVars = nullptr;
jmethodID    JavaLibrary::ms_midObjIdSetScriptVarInt = nullptr;
jmethodID    JavaLibrary::ms_midObjIdSetScriptVarFloat = nullptr;
jmethodID    JavaLibrary::ms_midObjIdSetScriptVarString = nullptr;
jmethodID    JavaLibrary::ms_midObjIdClearScriptVars = nullptr;
jmethodID    JavaLibrary::ms_midObjIdPackScriptVars = nullptr;
jmethodID    JavaLibrary::ms_midObjIdUnpackDeltaScriptVars = nullptr;
jmethodID    JavaLibrary::ms_midObjIdUnpackScriptVars = nullptr;
jmethodID    JavaLibrary::ms_midObjIdAttachScript = nullptr;
jmethodID    JavaLibrary::ms_midObjIdAttachScripts = nullptr;
jmethodID    JavaLibrary::ms_midObjIdDetachScript = nullptr;
jmethodID    JavaLibrary::ms_midObjIdDetachAllScripts = nullptr;
jclass       JavaLibrary::ms_clsStringId = nullptr;
jmethodID    JavaLibrary::ms_midStringId = nullptr;
jclass       JavaLibrary::ms_clsStringIdArray = nullptr;
jfieldID     JavaLibrary::ms_fidStringIdTable = nullptr;
jfieldID     JavaLibrary::ms_fidStringIdAsciiId = nullptr;
jfieldID     JavaLibrary::ms_fidStringIdIndexId = nullptr;
jclass       JavaLibrary::ms_clsModifiableStringId = nullptr;
jclass       JavaLibrary::ms_clsAttribute = nullptr;
jmethodID    JavaLibrary::ms_midAttribute = nullptr;
jfieldID     JavaLibrary::ms_fidAttributeType = nullptr;
jfieldID     JavaLibrary::ms_fidAttributeValue = nullptr;
jclass       JavaLibrary::ms_clsAttribMod = nullptr;
jmethodID    JavaLibrary::ms_midAttribMod = nullptr;
jfieldID     JavaLibrary::ms_fidAttribModName = nullptr;
jfieldID     JavaLibrary::ms_fidAttribModSkill = nullptr;
jfieldID     JavaLibrary::ms_fidAttribModType = nullptr;
jfieldID     JavaLibrary::ms_fidAttribModValue = nullptr;
jfieldID     JavaLibrary::ms_fidAttribModTime = nullptr;
jfieldID     JavaLibrary::ms_fidAttribModAttack = nullptr;
jfieldID     JavaLibrary::ms_fidAttribModDecay = nullptr;
jfieldID     JavaLibrary::ms_fidAttribModFlags = nullptr;
jclass       JavaLibrary::ms_clsMentalState = nullptr;
jmethodID    JavaLibrary::ms_midMentalState = nullptr;
jfieldID     JavaLibrary::ms_fidMentalStateType = nullptr;
jfieldID     JavaLibrary::ms_fidMentalStateValue = nullptr;
jclass       JavaLibrary::ms_clsMentalStateMod = nullptr;
jmethodID    JavaLibrary::ms_midMentalStateMod = nullptr;
jfieldID     JavaLibrary::ms_fidMentalStateModType = nullptr;
jfieldID     JavaLibrary::ms_fidMentalStateModValue = nullptr;
jfieldID     JavaLibrary::ms_fidMentalStateModTime = nullptr;
jfieldID     JavaLibrary::ms_fidMentalStateModAttack = nullptr;
jfieldID     JavaLibrary::ms_fidMentalStateModDecay = nullptr;
jclass       JavaLibrary::ms_clsLocation = nullptr;
jclass       JavaLibrary::ms_clsLocationArray = nullptr;
jfieldID     JavaLibrary::ms_fidLocationX = nullptr;
jfieldID     JavaLibrary::ms_fidLocationY = nullptr;
jfieldID     JavaLibrary::ms_fidLocationZ = nullptr;
jfieldID     JavaLibrary::ms_fidLocationArea = nullptr;
jfieldID     JavaLibrary::ms_fidLocationCell = nullptr;
jmethodID    JavaLibrary::ms_midRunOne = nullptr;
jmethodID    JavaLibrary::ms_midRunAll = nullptr;
jmethodID    JavaLibrary::ms_midCallMessages = nullptr;
jmethodID    JavaLibrary::ms_midRunConsoleHandler = nullptr;
jmethodID    JavaLibrary::ms_midUnload = nullptr;
jmethodID    JavaLibrary::ms_midGetClass = nullptr;
jmethodID    JavaLibrary::ms_midGetScriptFunctions = nullptr;
jclass       JavaLibrary::ms_clsMenuInfo = nullptr;
jmethodID    JavaLibrary::ms_midMenuInfo = nullptr;
jmethodID    JavaLibrary::ms_midMenuInfoSetMenuItemsInternal = nullptr;
jmethodID    JavaLibrary::ms_midMenuInfoGetMenuItemsInternal = nullptr;
jclass       JavaLibrary::ms_clsMenuInfoData = nullptr;
jmethodID    JavaLibrary::ms_midMenuInfoData = nullptr;
jfieldID     JavaLibrary::ms_fidMenuInfoDataId;
jfieldID     JavaLibrary::ms_fidMenuInfoDataParent;
jfieldID     JavaLibrary::ms_fidMenuInfoDataType;
jfieldID     JavaLibrary::ms_fidMenuInfoDataLabel;
jfieldID     JavaLibrary::ms_fidMenuInfoDataEnabled;
jfieldID     JavaLibrary::ms_fidMenuInfoDataServerNotify;
jclass       JavaLibrary::ms_clsCustomVar;
jmethodID    JavaLibrary::ms_midCustomVarObjIdStringInt;
jclass       JavaLibrary::ms_clsRangedIntCustomVar;
jmethodID    JavaLibrary::ms_midRangedIntCustomVar;
jclass       JavaLibrary::ms_clsPalcolorCustomVar;
jmethodID    JavaLibrary::ms_midPalcolorCustomVar;
jclass       JavaLibrary::ms_clsColor;
jmethodID    JavaLibrary::ms_midColor;
jclass       JavaLibrary::ms_clsDraftSchematic = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicCategory = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicComplexity = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSlots = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicAttribs = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicExperimentalAttribs = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicCustomizations = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicAttribMap = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicObjectTemplateCreated = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicScripts = nullptr;
jclass       JavaLibrary::ms_clsDraftSchematicSlot = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSlotName = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSlotOption = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSlotIngredientType = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSlotIngredientName = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSlotIngredients = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSlotComplexity = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSlotAmountRequired = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSlotAppearance = nullptr;
jclass       JavaLibrary::ms_clsDraftSchematicAttrib = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicAttribName = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicAttribMinValue = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicAttribMaxValue = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicAttribResourceMaxValue = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicAttribCurrentValue = nullptr;
jclass       JavaLibrary::ms_clsDraftSchematicSimpleIngredient = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSimpleIngredientIngredient = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSimpleIngredientCount = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSimpleIngredientSource = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSimpleIngredientXpType = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicSimpleIngredientXpAmount = nullptr;
jclass       JavaLibrary::ms_clsDraftSchematicCustom = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicCustomName = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicCustomValue = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicCustomMinValue = nullptr;
jfieldID     JavaLibrary::ms_fidDraftSchematicCustomMaxValue = nullptr;
//jfieldID     JavaLibrary::ms_fidDraftSchematicCustomLocked = nullptr;
jclass       JavaLibrary::ms_clsMapLocation = nullptr;
jmethodID    JavaLibrary::ms_midMapLocation = nullptr;
jclass       JavaLibrary::ms_clsRegion = nullptr;
jmethodID    JavaLibrary::ms_midRegion = nullptr;
jfieldID     JavaLibrary::ms_fidRegionName = nullptr;
jfieldID     JavaLibrary::ms_fidRegionPlanet = nullptr;
jclass       JavaLibrary::ms_clsCombatEngine = nullptr;
jclass       JavaLibrary::ms_clsCombatEngineCombatantData = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineCombatantDataPos = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineCombatantDataWorldPos = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineCombatantDataRadius = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineCombatantDataIsCreature = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineCombatantDataPosture = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineCombatantDataLocomotion = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineCombatantDataScriptMod = nullptr;
jclass       JavaLibrary::ms_clsCombatEngineAttackerData = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineAttackerDataWeaponSkill = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineAttackerDataAims = nullptr;
jclass       JavaLibrary::ms_clsCombatEngineDefenderData = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineDefenderDataCombatSkeleton = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineDefenderDataCover = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineDefenderDataHitLocationChances = nullptr;
jclass       JavaLibrary::ms_clsCombatEngineWeaponData = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataId = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataMinDamage = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataMaxDamage = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataWeaponType = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataAttackType = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataDamageType = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataElementalType = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataElementalValue = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataAttackSpeed = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataWoundChance = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataAccuracy = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataMinRange = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataMaxRange = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataDamageRadius = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataAttackCost = nullptr;
jfieldID     JavaLibrary::ms_fidCombatEngineWeaponDataIsDisabled = nullptr;
jclass       JavaLibrary::ms_clsCombatEngineHitResult;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultSuccess;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultCritical;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultGlancing;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultCrushing;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultStrikethrough;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultStrikethroughAmount;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultEvadeResult;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultEvadeAmount;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultBlockResult;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultBlock;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultDodge;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultParry;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultProc;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultBaseRoll;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultFinalRoll;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultAttackVal;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultCanSee;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultHitLocation;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultDamage;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultDamageType;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultRawDamage;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultElementalDamage;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultElementalDamageType;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultBleedDamage;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultCritDamage;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultBlockedDamage;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultBlockingArmor;
jfieldID     JavaLibrary::ms_fidCombatEngineHitResultBleedingChance;
jclass       JavaLibrary::ms_clsTransform = nullptr;
jclass       JavaLibrary::ms_clsTransformArray = nullptr;
jmethodID    JavaLibrary::ms_midTransform = nullptr;
jfieldID     JavaLibrary::ms_fidTransformMatrix = nullptr;
jclass       JavaLibrary::ms_clsVector = nullptr;
jclass       JavaLibrary::ms_clsVectorArray = nullptr;
jfieldID     JavaLibrary::ms_fidVectorX = nullptr;
jfieldID     JavaLibrary::ms_fidVectorY = nullptr;
jfieldID     JavaLibrary::ms_fidVectorZ = nullptr;
jclass       JavaLibrary::ms_clsResourceDensity = nullptr;
jfieldID     JavaLibrary::ms_fidResourceDensityResourceType = nullptr;
jfieldID     JavaLibrary::ms_fidResourceDensityDensity = nullptr;
jclass       JavaLibrary::ms_clsResourceAttribute = nullptr;
jfieldID     JavaLibrary::ms_fidResourceAttributeName = nullptr;
jfieldID     JavaLibrary::ms_fidResourceAttributeValue = nullptr;
jclass	     JavaLibrary::ms_clsLibrarySpaceTransition = nullptr;
jmethodID    JavaLibrary::ms_midLibrarySpaceTransitionSetPlayerOvert = nullptr;
jmethodID    JavaLibrary::ms_midLibrarySpaceTransitionClearOvertStatus = nullptr;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// CS handlers
jclass       JavaLibrary::ms_clsLibraryDump = nullptr;
jmethodID    JavaLibrary::ms_midLibraryDumpDumpTargetInfo = nullptr;

jclass	     JavaLibrary::ms_clsLibraryGMLib = nullptr;
jmethodID    JavaLibrary::ms_midLibraryGMLibFreeze = nullptr;
jmethodID    JavaLibrary::ms_midLibraryGMLibUnfreeze = nullptr;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int          JavaLibrary::ms_loaded = 0;
Semaphore *  JavaLibrary::ms_shutdownJava = nullptr;

int     JavaLibrary::GlobalInstances::ms_stringIdIndex = 0;
int     JavaLibrary::GlobalInstances::ms_attribModIndex = 0;
int     JavaLibrary::GlobalInstances::ms_mentalStateModIndex = 0;
int     JavaLibrary::GlobalInstances::ms_modifiableIntIndex = 0;
int     JavaLibrary::GlobalInstances::ms_modifiableFloatIndex = 0;
int     JavaLibrary::GlobalInstances::ms_modifiableStringIdIndex = 0;
GlobalRefPtr JavaLibrary::GlobalInstances::ms_stringIds[MAX_STRING_ID_PARAMS];
GlobalRefPtr JavaLibrary::GlobalInstances::ms_attribMods[MAX_ATTRIB_MOD_PARAMS];
GlobalRefPtr JavaLibrary::GlobalInstances::ms_mentalStateMods[MAX_MENTAL_STATE_MOD_PARAMS];
GlobalRefPtr JavaLibrary::GlobalInstances::ms_modifiableInts[MAX_MODIFIABLE_INT_PARAMS];
GlobalRefPtr JavaLibrary::GlobalInstances::ms_modifiableFloats[MAX_MODIFIABLE_FLOAT_PARAMS];
GlobalRefPtr JavaLibrary::GlobalInstances::ms_modifiableStringIds[MAX_MODIFIABLE_STRING_ID_PARAMS];
GlobalRefPtr JavaLibrary::GlobalInstances::ms_menuInfo;
GlobalArrayRefPtr JavaLibrary::ms_attribModList[MAX_RECURSION_COUNT];
GlobalArrayRefPtr JavaLibrary::ms_mentalStateModList[MAX_RECURSION_COUNT];


// ======================================================================

std::set<std::string> s_profileSections;

//========================================================================
// class JavaLibrary methods
//========================================================================

void JavaLibrary::throwScriptException(char const * const format, ...)
{
	va_list va;
	va_start(va, format);

	throwScriptException(format, va);

	va_end(va);
}

void JavaLibrary::throwScriptException(char const * const format, va_list va)
{
	DEBUG_FATAL(!ms_env, ("JavaLibrary::throwScriptException: ms_env is nullptr"));
	if (ms_env)
	{
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), format, va);
		buffer[sizeof(buffer) - 1] = 0;
		throwInternalScriptError(buffer);
	}
}

#ifdef linux
static struct sigaction OrgSa;		// original signal handler, used to generate core files
static struct sigaction JavaSa;		// Java signal handler, used to generate Java exceptions
static struct sigaction OurSa;		// our signal handler
#endif

void JavaLibrary::fatalHandler(int signum)
{
#ifdef linux
	fprintf(stderr, "In JavaLibrary::fatalHandler, signal %d: ", signum);
	if (signum == SIGSEGV && ms_jvm)
	{

		// try to see if our call stack came from C or Java
		void *frameAddress = __builtin_frame_address(0);
		// the address that generated the segfault is stored at an offset of 0x44 from
		// the frame address
		// @todo: find documentation on the offset to verify it is stable
		void *crashAddress1 = *((void**)((static_cast<char *>(frameAddress)) + 0x44));
		// pass the crash address to addr2line to get the file name where the crash occurred
		static const int BUFLEN = 2048;
		char lib1[BUFLEN], lib2[BUFLEN];
		char file1[BUFLEN], file2[BUFLEN];
		int line1, line2;
		bool result1 = DebugHelp::lookupAddress(reinterpret_cast<uint32>(crashAddress1), lib1, file1, BUFLEN, line1);

		// do a second test based on the return address
		// it turns out that in some java crashes we don't even have 2 return
		// addresses, so check 0 and 1 just to make sure
		bool result2 = false;
		void *crashAddress2a = nullptr;
		void *crashAddress2b = nullptr;
		void *crashAddress2c = nullptr;
		void *frameAddressA = nullptr;
		void *frameAddressB = nullptr;
		uint32 frameAddressHigh = (reinterpret_cast<uint32>(frameAddress) >> 16);
		crashAddress2a = __builtin_return_address(0);
		if (crashAddress2a != nullptr)
		{
			frameAddressA = __builtin_frame_address(1);
			if (frameAddressA != nullptr && 
				(reinterpret_cast<uint32>(frameAddressA) >> 16 == frameAddressHigh))
			{
				crashAddress2b = __builtin_return_address(1);
				if (crashAddress2b != nullptr)
				{
					frameAddressB = __builtin_frame_address(2);
					if (frameAddressB != nullptr && 
						(reinterpret_cast<uint32>(frameAddressB) >> 16 == frameAddressHigh))
					{
						crashAddress2c = __builtin_return_address(2);
						if (crashAddress2c != nullptr)
						{
							result2 = DebugHelp::lookupAddress(reinterpret_cast<uint32>(
								crashAddress2c), lib2, file2, BUFLEN, line2);
						}
					}
				}
			}
		}

		bool javaCrash = true;
		if ((result1 || result2) && strstr(lib1, "libjvm.so") == nullptr && strstr(lib2, "libjvm.so") == nullptr)
		{
			if (result1 && result2)
			{
				fprintf(stderr, "I crashed in a file: <%s>, line %d, or in file "
					"<%s>, line %d, going to core.\n", file1, line1, file2, line2);
			}
			else if (result1)
			{
				fprintf(stderr, "I crashed in a file: <%s>, line %d, going to core.\n",
					file1, line1);
			}
			else
			{
				fprintf(stderr, "I crashed in a file: <%s>, line %d, going to core.\n",
					file2, line2);
			}
			javaCrash = false;
		}

		if (javaCrash)
		{
			fprintf(stderr, "I think I crashed in Java, calling the Java segfault hanlder.\n");
			IGNORE_RETURN(sigaction(SIGSEGV, &JavaSa, nullptr));
			IGNORE_RETURN(pthread_kill(pthread_self(), SIGSEGV));
		}
		else
		{
			// destroy Java threads
			// this pthread method is not in later versions of glibc as the kernel should handle the kill
			//pthread_kill_other_threads_np();
			ms_instance = nullptr;
			ms_env = nullptr;
			ms_jvm = nullptr;
			// restore original signal handler and rethrow signal
			IGNORE_RETURN(sigaction(SIGSEGV, &OrgSa, nullptr));
			IGNORE_RETURN(pthread_kill(pthread_self(), SIGSEGV));
		}
	}
	else
		::abort();
#endif
}	// JavaLibrary::fatalHandler

/**
 * Class constructor.
 */
JavaLibrary::JavaLibrary(void)
{
int i;

	if (ms_instance != nullptr || ms_loaded != 0)
		return;

	ms_shutdownJava = new Semaphore();

	for (i = 0; i < MAX_RECURSION_COUNT; ++i)
	{
		ms_attribModList[i] = GlobalArrayRef::cms_nullPtr;
		ms_mentalStateModList[i] = GlobalArrayRef::cms_nullPtr;
	}
	// initialize the GlobalInstances class static members
	for (i = 0; i < MAX_STRING_ID_PARAMS; ++i)
		GlobalInstances::ms_stringIds[i] = GlobalRef::cms_nullPtr;
	for (i = 0; i < MAX_ATTRIB_MOD_PARAMS; ++i)
		GlobalInstances::ms_attribMods[i] = GlobalRef::cms_nullPtr;
	for (i = 0; i < MAX_MENTAL_STATE_MOD_PARAMS; ++i)
		GlobalInstances::ms_mentalStateMods[i] = GlobalRef::cms_nullPtr;
	for (i = 0; i < MAX_MODIFIABLE_INT_PARAMS; ++i)
		GlobalInstances::ms_modifiableInts[i] = GlobalRef::cms_nullPtr;
	for (i = 0; i < MAX_MODIFIABLE_FLOAT_PARAMS; ++i)
		GlobalInstances::ms_modifiableFloats[i] = GlobalRef::cms_nullPtr;
	for (i = 0; i < MAX_MODIFIABLE_STRING_ID_PARAMS; ++i)
		GlobalInstances::ms_modifiableStringIds[i] = GlobalRef::cms_nullPtr;


	m_initializerThread = new MemberFunctionThreadZero<JavaLibrary>("JavaLibrary", *this, &JavaLibrary::initializeJavaThread);
	ThreadHandle tempThreadHandle(m_initializerThread); // for some obscure reason, the thread doesn't run unless you create a handle, but we don't need the handle for anything
	UNREF(tempThreadHandle);

	// wait until ms_loaded != 0; if it is 1, we have successfully initialized Java,
	// if it is not 1, there was an error
	while (ms_loaded == 0)
		Os::sleep(100);
	if (ms_loaded != 1)
		return;

	if (!connectToJava())
		return;

	ms_instance = this;
	DEBUG_REPORT_LOG(true, ("Java initialized\n"));
}	// JavaLibrary::JavaLibrary

/**
 * Class destructor.
 */
JavaLibrary::~JavaLibrary()
{
	disconnectFromJava();	

	if (ms_shutdownJava != nullptr)
	{
		// tell the initialize thread to shut down
		if (ms_loaded > 0)
		{
			ms_shutdownJava->signal();
			while (ms_loaded > 0)
				Os::sleep(100);
		}
		delete ms_shutdownJava;
		ms_shutdownJava = nullptr;
	}

	ms_instance = nullptr;
}	// JavaLibrary::~JavaLibrary

//----------------------------------------------------------------------

/**
 * Creates the singleton JavaLibrary instance.
 */
void JavaLibrary::install(void)
{
	if (ms_instance == nullptr)
	{
		JavaLibrary *lib = new JavaLibrary;
		if (lib != ms_instance)
		{
			if (ms_instance == nullptr)
			{
				delete lib;
				if (ms_javaVmType != JV_none)
					FATAL (true, ("Unable to initialize Java"));
			}
		}
	}
	else
		DEBUG_REPORT_LOG(true, ("JavaLibrary::JavaLibrary error: dup instance\n"));
}	// JavaLibrary::install

//----------------------------------------------------------------------

/**
 * Destroys the singleton JavaLibrary instance.
 */
void JavaLibrary::remove(void)
{
	if (ms_instance != nullptr)
	{
		JavaLibrary * temp = ms_instance;
		ms_instance = nullptr;
		delete temp;
		s_profileSections.clear();
	}
}	// JavaLibrary::remove

//----------------------------------------------------------------------

/**
 * Entry point for the Java initialization thread. We create the Java VM from here,
 * and then wait until we get a signal to shut down Java. The main game thread will
 * connect to our VM via the connectToJava function.
 */
void JavaLibrary::initializeJavaThread()
{
	if (ms_loaded != 0)
	{
		WARNING(true, ("JavaLibrary::initializeJavaThread called when already loaded!"));
		return;
	}

#ifdef linux
	// get the default signal handler
	IGNORE_RETURN(sigaction(SIGSEGV, nullptr, &OrgSa));
#endif	// linux

	// dynamically load the jni dll and JNI_CreateJavaVM
	void * libHandle = nullptr;
	JNI_CREATEJAVAVMPROC JNI_CreateJavaVMProc;
#if defined(WIN32)
	std::string dllPath = ConfigServerGame::getJavaLibPath();
	HINSTANCE hVm = LoadLibrary(dllPath.c_str());
	if (hVm == nullptr)
	{
		FATAL(true, ("jvm open fail error: could not open %s", dllPath.c_str()));
		ms_loaded = -1;
		return;
	}
	libHandle = hVm;
	JNI_CreateJavaVMProc =
		(JNI_CREATEJAVAVMPROC)GetProcAddress(hVm, "JNI_CreateJavaVM");		//lint !e1924 C-style cast
#else
	void *libVM = nullptr;
	std::string dllPath = ConfigServerGame::getJavaLibPath();
	libVM = dlopen(dllPath.c_str(), RTLD_LAZY);
	if (libVM == nullptr)
	{
		FATAL(true, ("jvm open fail! error: %s", dlerror()));
		ms_loaded = -1;
		return;
	}
	libHandle = libVM;
	JNI_CreateJavaVMProc =
		(JNI_CREATEJAVAVMPROC)dlsym(libVM, "JNI_CreateJavaVM");
#endif
	if (JNI_CreateJavaVMProc == nullptr)
	{
		FATAL(true, ("Error getting JNI_CreateJavaVM from jvm shared library"));
		ms_loaded = -1;
		return;
	}

	// set up the args to initialize the jvm
	std::string classPath = "-Djava.class.path=";
	classPath += ConfigServerGame::getScriptPath();

	JavaVMInitArgs vm_args;
	JavaVMOption tempOption = {nullptr, nullptr};
	std::vector<JavaVMOption> options;
	char *jdwpBuffer = nullptr;

	UNREF(jdwpBuffer);

	// append any config file specified options...our defaults below will override them
	// as after testing i see them as sane defaults -darth
	if (ConfigServerScript::hasJavaOptions())
	{
		int const numberOfJavaOptions = ConfigServerScript::getNumberOfJavaOptions();
		for (int i = 0; i < numberOfJavaOptions; ++i)
		{
			char const * p = ConfigServerScript::getJavaOptions(i);
			tempOption.optionString = const_cast<char *>(p);
			options.push_back(tempOption);
		}
	}

	// initial and minimum jvm allocation size
	tempOption.optionString = "-Xms128m";
	options.push_back(tempOption);

	// maximum jvm allocation - max 512m on 32-bit
	tempOption.optionString = "-Xmx512m";
	options.push_back(tempOption);

	// thread stack size, that is, size per thread
	tempOption.optionString = "-Xss768k";
	options.push_back(tempOption);

	// java 1.8 and higher uses metaspace...which is apparently unlimited by default
	// we have to consider it with our 512m max above so 96 on 32-bit is as high as we go
        tempOption.optionString = "-XX:MaxMetaspaceSize=96m";
        options.push_back(tempOption);

	// rice options!!!!1! yay - actually after much trial and error these are a good mix for speed and efficiency
	// i should split these someday into separate optionStrings...or not
	// some may be default, not needed, or ignored by whatever version we're using -darth
	tempOption.optionString = "-Xrs -XX:-UsePerfData -XX:-AllowUserSignalHandlers -XX:UseSSE=3 -XX+DoEscapeAnalysis -XX:AutoBoxCacheMax:2000 -XX:+OptimizeStringConcat -XX:+OptimizeFill -XX:+EliminateAutoBox -XX:+UseCompressedStrings -XX:+UseCompressedOops -XX:+EliminateLocks -XX:UseFastAccessorMethods -XX:+UseStringCache -XgcPrio:throughput -XXkeepAreaRatio:1 -XXlazyUnlocking -XXcallProfiling -XXcompactRatio:1";
	options.push_back(tempOption);

	// left over from SOE, below...probably won't use these most of the time
	if (ConfigServerGame::getUseJavaXcheck())
	{
		tempOption.optionString = "-Xcheck:jni";
		options.push_back(tempOption);
	}

	// if at all
	if (ConfigServerGame::getCompileScripts())
	{
		tempOption.optionString = "-Xint";
		options.push_back(tempOption);
	}

	// ...
	if (ConfigServerGame::getUseVerboseJava())
	{
		tempOption.optionString = "-verbose:jni";
		options.push_back(tempOption);
		tempOption.optionString = "-verbose:gc";
		options.push_back(tempOption);
		tempOption.optionString = "-verbose:class";
		options.push_back(tempOption);
	}

	// not sure how logging GC runs is even useful personally
	if (ConfigServerGame::getLogJavaGc())
	{
		tempOption.optionString = "-Xloggc:javagc.log";
		options.push_back(tempOption);
	}

	tempOption.optionString = const_cast<char *>(classPath.c_str());
	options.push_back(tempOption);

// TODO: this really sucks as the jvm won't start without the param
// there's a dynamic method but requires the jvm to already be running, wtf?
// so we'll support the dev and stable versions
#ifdef JNI_VERSION_1_9
        vm_args.version = JNI_VERSION_1_9;
#define JNIVERSET = 1
#endif

#if !defined(JNIVERSET) && defined(JNI_VERSION_1_8)
	vm_args.version = JNI_VERSION_1_8;
#define JNIVERSET = 1 
#endif

#ifdef JNIVERSET
#undef JNIVERSET
#else
#error JNI version not found/set!
#endif

	vm_args.options = &options[0];
	vm_args.nOptions = options.size();
	vm_args.ignoreUnrecognized = JNI_TRUE; // ok let's ignore whatever isn't understood instead of crashing like a loser

	// create the JVM
	JNIEnv * env = nullptr;
	jint result = (*JNI_CreateJavaVMProc)(&ms_jvm, reinterpret_cast<void**>(&env), &vm_args);	

	if (result != 0)
	{
		FATAL (true, ("Failed to CreateJavaVMProc: %d", result));
		ms_loaded = -1;
		return;
	}
	ms_loaded = 1;

	// i don't think this bit functions anymore with new java?
	if (ConfigServerGame::getTrapScriptCrashes())
	{	
		//set up signal handler for fatals in linux
		OurSa.sa_handler = fatalHandler;
		sigemptyset(&OurSa.sa_mask);
		OurSa.sa_flags = 0;
		IGNORE_RETURN(sigaction(SIGSEGV, &OurSa, &JavaSa));
	}

	// wait until the main thread tells us to shutdown
	ms_shutdownJava->wait();

	// clean up
	ms_jvm->DestroyJavaVM();
	Os::sleep(2000); // give java a chance to end it's miserable existence
	ms_jvm = nullptr;

	if (ConfigServerGame::getTrapScriptCrashes())
	{
		// restore the default signal handler
		IGNORE_RETURN(sigaction(SIGSEGV, &OrgSa, NULL));
	}

#if defined(_WIN32)
	IGNORE_RETURN(FreeLibrary(static_cast<HMODULE>(libHandle)));
#elif defined(linux)
	dlclose(libHandle);
#endif

	ms_loaded = 0;
}	// JavaLibrary::initializeJavaThread

//----------------------------------------------------------------------

/**
 * Connects this thread to the Java VM and initializes our member vars that 
 * reference Java objects.
 *
 * @return true on success, false if we were unable to connect to Java or 
 * initialize our members
 */
bool JavaLibrary::connectToJava()
{
	if (ms_jvm == 0)
		return false;

	// attach our thread to the VM
	jint result = ms_jvm->AttachCurrentThread(reinterpret_cast<void**>(&ms_env), nullptr);
	if (result != 0)
	{
		FATAL(true, ("Failed to attach to the Java VM! Error code returned = %d", result));
		return false;
	}

	int i;
	jmethodID constructor;
	LocalRefPtr localInstance;
	LocalObjectArrayRefPtr localArrayInstance;
	jclass tempClass;

	// set the local reference count limit
	int localRefLimit = ConfigServerGame::getJavaLocalRefLimit();
	if (localRefLimit < 16)
		localRefLimit = 16;
	ms_env->EnsureLocalCapacity(localRefLimit);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return false;
	}

	// get the script_entry class
	GET_CLASS(ms_clsScriptEntry, "script/script_entry");
	GET_METHOD(constructor, ms_clsScriptEntry, "<init>", "(Ljava/lang/String;ZZIIIZ)V");
	GET_STATIC_METHOD(ms_midScriptEntryGetOwnerContext, ms_clsScriptEntry, "getOwnerContext", "()Lscript/obj_id;");
	GET_STATIC_METHOD(ms_midScriptEntryEnableLogging, ms_clsScriptEntry, "enableScriptLogging", "(Z)V");
	GET_STATIC_METHOD(ms_midScriptEntryEnableNewJediTracking, ms_clsScriptEntry, "enableNewJediTracking", "(Z)V");
	GET_STATIC_METHOD(ms_midScriptEntryGetFreeMem, ms_clsScriptEntry, "getFreeMemory", "()J");

	// create an instance of script_entry class to initialze the system on the Java side
	JavaString scriptPath(ConfigServerGame::getScriptPath());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return false;
	}
	jobject localEntry = ms_env->NewObject(ms_clsScriptEntry, constructor, 
		scriptPath.getValue(),
		ConfigServerGame::getJavaConsoleDebugMessages(),
		ConfigServerGame::getCrashOnScriptError(), 
		ConfigServerGame::getScriptWatcherWarnTime(),
		ConfigServerGame::getScriptWatcherInterruptTime(),
		ConfigServerGame::getScriptStackErrorLimit(),
		ConfigServerGame::getScriptStackErrorLevel() == 1);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return false;
	}
	ms_scriptEntry = ms_env->NewGlobalRef(localEntry);
	ms_env->DeleteLocalRef(localEntry);

	// get the methodIDs for the runScript() and the unloadClass() methods
	GET_STATIC_METHOD(ms_midRunOne,ms_clsScriptEntry, "runScript", "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/Object;)I");
	GET_STATIC_METHOD(ms_midRunAll, ms_clsScriptEntry, "runScripts", "(Ljava/lang/String;[Ljava/lang/Object;)I");
	GET_STATIC_METHOD(ms_midCallMessages, ms_clsScriptEntry, "callMessageHandlers", "(Ljava/lang/String;JLscript/dictionary;)I");
	GET_STATIC_METHOD(ms_midRunConsoleHandler, ms_clsScriptEntry, "runConsoleHandler", "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/String;");
	GET_STATIC_METHOD(ms_midUnload, ms_clsScriptEntry, "unloadClass", "(Ljava/lang/String;)Z");
	GET_STATIC_METHOD(ms_midGetClass, ms_clsScriptEntry, "getClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	GET_STATIC_METHOD(ms_midGetScriptFunctions, ms_clsScriptEntry, "getScriptFunctions", "(Ljava/lang/String;)[Ljava/lang/String;");

	// get the class and method ids for java.lang.Object
	GET_CLASS(ms_clsObject, "java/lang/Object");

	// get the class and method ids for java.lang.Class
	GET_CLASS(ms_clsClass, "java/lang/Class");
	GET_METHOD(ms_midClassGetName, ms_clsClass, "getName", "()Ljava/lang/String;");
	GET_METHOD(ms_midClassGetMethods, ms_clsClass, "getMethods", "()[Ljava/lang/reflect/Method;");

	// get the class and method ids for java.reflect.Method
	GET_CLASS(ms_clsMethod, "java/lang/reflect/Method");
	GET_METHOD(ms_midMethodGetName, ms_clsMethod, "getName", "()Ljava/lang/String;");

	// get the class and constructor methodID for java.lang.Boolean
	GET_CLASS(ms_clsBoolean, "java/lang/Boolean");
	GET_CLASS(ms_clsBooleanArray, "[Ljava/lang/Boolean;");
	GET_METHOD(ms_midBoolean, ms_clsBoolean, "<init>", "(Z)V");
	GET_METHOD(ms_midBooleanBooleanValue, ms_clsBoolean, "booleanValue", "()Z");

	// get the class and constructor methodID for java.lang.Integer
	GET_CLASS(ms_clsInteger, "java/lang/Integer");
	GET_CLASS(ms_clsIntegerArray, "[Ljava/lang/Integer;");
	GET_METHOD(ms_midInteger, ms_clsInteger, "<init>", "(I)V");
	GET_METHOD(ms_midIntegerIntValue, ms_clsInteger, "intValue", "()I");

	// get the class and constructor methodID for modifiable_int
	GET_CLASS(ms_clsModifiableInt, "script/modifiable_int");
	GET_FIELD(ms_fidModifiableIntData, ms_clsModifiableInt, "m_data", "I");
	GET_METHOD(ms_midModifiableInt, ms_clsModifiableInt, "<init>", "()V");

	// create some global modifiable_int class instances to be used parameters for
	// events
	for (i = 0; i < MAX_MODIFIABLE_INT_PARAMS; ++i)
	{
		localInstance = createNewObject(ms_clsModifiableInt, ms_midModifiableInt);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
		GlobalInstances::ms_modifiableInts[i] = GlobalRefPtr(new GlobalRef(*localInstance));
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
	}

	// get the class and constructor methodID for java.lang.Float
	GET_CLASS(ms_clsFloat, "java/lang/Float");
	GET_CLASS(ms_clsFloatArray, "[Ljava/lang/Float;");
	GET_METHOD(ms_midFloat, ms_clsFloat, "<init>", "(F)V");
	GET_METHOD(ms_midFloatFloatValue, ms_clsFloat, "floatValue", "()F");

	// get the class and constructor methodID for modifiable_float
	GET_CLASS(ms_clsModifiableFloat, "script/modifiable_float");
	GET_FIELD(ms_fidModifiableFloatData, ms_clsModifiableFloat, "m_data", "F");
	GET_METHOD(ms_midModifiableFloat, ms_clsModifiableFloat, "<init>", "()V");

	// create some global modifiable_float class instances to be used parameters for
	// events
	for (i = 0; i < MAX_MODIFIABLE_FLOAT_PARAMS; ++i)
	{
		localInstance = createNewObject(ms_clsModifiableFloat, ms_midModifiableFloat);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
		GlobalInstances::ms_modifiableFloats[i] = GlobalRefPtr(new GlobalRef(*localInstance));
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
	}

	// get the class for java.lang.String
	GET_CLASS(ms_clsString, "java/lang/String");
	GET_CLASS(ms_clsStringArray, "[Ljava/lang/String;");

	// get the class and methods for java.util.Map and java.util.Hashtable
	GET_CLASS(ms_clsMap, "java/util/Map");
	GET_METHOD(ms_midMapPut, ms_clsMap, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	GET_METHOD(ms_midMapGet, ms_clsMap, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

	GET_CLASS(ms_clsHashtable, "java/util/Hashtable");
	GET_METHOD(ms_midHashtable, ms_clsHashtable, "<init>", "()V");

	GET_CLASS(ms_clsThrowable, "java/lang/Throwable");
	GET_CLASS(ms_clsError, "java/lang/Error");
	GET_CLASS(ms_clsStackOverflowError, "java/lang/StackOverflowError");
	GET_METHOD(ms_midThrowableGetMessage, ms_clsThrowable, "getMessage", "()Ljava/lang/String;");

	GET_CLASS(ms_clsThread, "java/lang/Thread");
	GET_STATIC_METHOD(ms_midThreadDumpStack, ms_clsThread, "dumpStack", "()V");

	GET_CLASS(ms_clsInternalScriptError, "script/internal_script_exception");
	GET_CLASS(ms_clsInternalScriptSeriousError, "script/internal_script_error");
	GET_FIELD(ms_fidInternalScriptSeriousErrorError, ms_clsInternalScriptSeriousError, "wrappedError", "Ljava/lang/Error;");

	// get the class and methods for dictionary
	GET_CLASS(ms_clsDictionary, "script/dictionary");
	GET_METHOD(ms_midDictionary, ms_clsDictionary, "<init>", "()V");
	GET_METHOD(ms_midDictionaryPack, ms_clsDictionary, "pack", "()[B");
	GET_METHOD(ms_midDictionaryKeys, ms_clsDictionary, "keys", "()Ljava/util/Enumeration;");
	GET_METHOD(ms_midDictionaryValues, ms_clsDictionary, "values", "()Ljava/util/Collection;");
	GET_METHOD(ms_midDictionaryPut, ms_clsDictionary, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	GET_METHOD(ms_midDictionaryPutInt, ms_clsDictionary, "put", "(Ljava/lang/Object;I)Ljava/lang/Object;");
	GET_METHOD(ms_midDictionaryPutBool, ms_clsDictionary, "put", "(Ljava/lang/Object;Z)Ljava/lang/Object;");
	GET_METHOD(ms_midDictionaryPutFloat, ms_clsDictionary, "put", "(Ljava/lang/Object;F)Ljava/lang/Object;");
	GET_METHOD(ms_midDictionaryGet, ms_clsDictionary, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
	GET_STATIC_METHOD(ms_midDictionaryUnpack, ms_clsDictionary, "unpack", "([B)Lscript/dictionary;");

	// get the class and methods for java.util.Collection
	GET_CLASS(ms_clsCollection, "java/util/Collection");
	GET_METHOD(ms_midCollectionToArray, ms_clsCollection, "toArray", "()[Ljava/lang/Object;");

	// get the class and methods for java.util.Enumeration
	GET_CLASS(ms_clsEnumeration, "java/util/Enumeration");
	GET_METHOD(ms_midEnumerationHasMoreElements, ms_clsEnumeration, "hasMoreElements", "()Z");
	GET_METHOD(ms_midEnumerationNextElement, ms_clsEnumeration, "nextElement", "()Ljava/lang/Object;");

	// get the class, field ids and constructor methodIDs for obj_var
	GET_CLASS(ms_clsDynamicVariable, "script/obj_var");
	GET_FIELD(ms_fidDynamicVariableName, ms_clsDynamicVariable, "m_name", "Ljava/lang/String;");
	GET_FIELD(ms_fidDynamicVariableData, ms_clsDynamicVariable, "m_data", "Ljava/lang/Object;");
	GET_METHOD(ms_midDynamicVariableInt, ms_clsDynamicVariable, "<init>", "(Ljava/lang/String;I)V");
	GET_METHOD(ms_midDynamicVariableIntArray, ms_clsDynamicVariable, "<init>", "(Ljava/lang/String;[I)V");
	GET_METHOD(ms_midDynamicVariableFloat, ms_clsDynamicVariable, "<init>", "(Ljava/lang/String;F)V");
	GET_METHOD(ms_midDynamicVariableFloatArray, ms_clsDynamicVariable, "<init>", "(Ljava/lang/String;[F)V");
	GET_METHOD(ms_midDynamicVariableString, ms_clsDynamicVariable, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
	GET_METHOD(ms_midDynamicVariableStringArray, ms_clsDynamicVariable, "<init>", "(Ljava/lang/String;[Ljava/lang/String;)V");
	GET_METHOD(ms_midDynamicVariableObjId, ms_clsDynamicVariable, "<init>", "(Ljava/lang/String;J)V");
	GET_METHOD(ms_midDynamicVariableObjIdArray, ms_clsDynamicVariable, "<init>", "(Ljava/lang/String;[J)V");
	GET_METHOD(ms_midDynamicVariableLocation, ms_clsDynamicVariable, "<init>", "(Ljava/lang/String;Lscript/location;)V");
	GET_METHOD(ms_midDynamicVariableLocationArray, ms_clsDynamicVariable, "<init>", "(Ljava/lang/String;[Lscript/location;)V");

	// get the class, field ids and constructor methodIDs for obj_var_list
	GET_CLASS(ms_clsDynamicVariableList, "script/obj_var_list");
	GET_METHOD(ms_midDynamicVariableList, ms_clsDynamicVariableList, "<init>", "(Ljava/lang/String;I)V");
	GET_METHOD(ms_midDynamicVariableListSet, ms_clsDynamicVariableList, "setObjVar", "(Lscript/obj_var;)Z");
	GET_METHOD(ms_midDynamicVariableListSetInt, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;I)Z");
	GET_METHOD(ms_midDynamicVariableListSetIntArray, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;[I)Z");
	GET_METHOD(ms_midDynamicVariableListSetFloat, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;F)Z");
	GET_METHOD(ms_midDynamicVariableListSetFloatArray, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;[F)Z");
	GET_METHOD(ms_midDynamicVariableListSetString, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;Ljava/lang/String;)Z");
	GET_METHOD(ms_midDynamicVariableListSetStringArray, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;[Ljava/lang/String;)Z");
	GET_METHOD(ms_midDynamicVariableListSetObjId, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;J)Z");
	GET_METHOD(ms_midDynamicVariableListSetObjIdArray, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;[J)Z");
	GET_METHOD(ms_midDynamicVariableListSetLocation, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;Lscript/location;)Z");
	GET_METHOD(ms_midDynamicVariableListSetLocationArray, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;[Lscript/location;)Z");
	GET_METHOD(ms_midDynamicVariableListSetStringId, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;Lscript/string_id;)Z");
	GET_METHOD(ms_midDynamicVariableListSetStringIdArray, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;[Lscript/string_id;)Z");
	GET_METHOD(ms_midDynamicVariableListSetTransform, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;Lscript/transform;)Z");
	GET_METHOD(ms_midDynamicVariableListSetTransformArray, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;[Lscript/transform;)Z");
	GET_METHOD(ms_midDynamicVariableListSetVector, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;Lscript/vector;)Z");
	GET_METHOD(ms_midDynamicVariableListSetVectorArray, ms_clsDynamicVariableList, "setObjVar", "(Ljava/lang/String;[Lscript/vector;)Z");

	// get the class, field ids and constructor methodID for attribute
	GET_CLASS(ms_clsAttribute, "script/attribute");
	GET_FIELD(ms_fidAttributeType, ms_clsAttribute, "m_type", "I");
	GET_FIELD(ms_fidAttributeValue, ms_clsAttribute, "m_value", "I");
	GET_METHOD(ms_midAttribute, ms_clsAttribute, "<init>", "(II)V");

	// get the class, field ids and constructor methodID for attribMod
	GET_CLASS(ms_clsAttribMod, "script/attrib_mod");
	GET_FIELD(ms_fidAttribModName, ms_clsAttribMod, "m_name", "Ljava/lang/String;");
	GET_FIELD(ms_fidAttribModSkill, ms_clsAttribMod, "m_skill", "Ljava/lang/String;");
	GET_FIELD(ms_fidAttribModType, ms_clsAttribMod, "m_attrib", "I");
	GET_FIELD(ms_fidAttribModValue, ms_clsAttribMod, "m_value", "I");
	GET_FIELD(ms_fidAttribModTime, ms_clsAttribMod, "m_duration", "F");
	GET_FIELD(ms_fidAttribModAttack, ms_clsAttribMod, "m_attack", "F");
	GET_FIELD(ms_fidAttribModDecay, ms_clsAttribMod, "m_decay", "F");
	GET_FIELD(ms_fidAttribModFlags, ms_clsAttribMod, "m_flags", "I");
	GET_METHOD(ms_midAttribMod, ms_clsAttribMod, "<init>", "()V");

	// create some attribMod instances for use as script paramters
	for (i = 0; i < MAX_ATTRIB_MOD_PARAMS; ++i)
	{
		localInstance = createNewObject(ms_clsAttribMod, ms_midAttribMod);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
		GlobalInstances::ms_attribMods[i] = GlobalRefPtr(new GlobalRef(*localInstance));
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
	}
	// create the attribModList
	for (i = 0; i < MAX_RECURSION_COUNT; ++i)
	{
		localArrayInstance = createNewObjectArray(MAX_ATTRIB_MOD_PARAMS, ms_clsAttribMod);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
		ms_attribModList[i] = GlobalArrayRefPtr(new GlobalArrayRef(*localArrayInstance));
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
	}

	//-------
	// get the class, field ids and constructor methodID for mental state
	GET_CLASS(ms_clsMentalState, "script/mental_state");
	GET_FIELD(ms_fidMentalStateType, ms_clsMentalState, "m_type", "I");
	GET_FIELD(ms_fidMentalStateValue, ms_clsMentalState, "m_value", "F");
	GET_METHOD(ms_midMentalState, ms_clsMentalState, "<init>", "(IF)V");

	// get the class, field ids and constructor methodID for mental_state_mod
	GET_CLASS(ms_clsMentalStateMod, "script/mental_state_mod");
	GET_FIELD(ms_fidMentalStateModType, ms_clsMentalStateMod, "m_mental_state", "I");
	GET_FIELD(ms_fidMentalStateModValue, ms_clsMentalStateMod, "m_value", "F");
	GET_FIELD(ms_fidMentalStateModTime, ms_clsMentalStateMod, "m_duration", "F");
	GET_FIELD(ms_fidMentalStateModAttack, ms_clsMentalStateMod, "m_attack", "F");
	GET_FIELD(ms_fidMentalStateModDecay, ms_clsMentalStateMod, "m_decay", "F");
	GET_METHOD(ms_midMentalStateMod, ms_clsMentalStateMod, "<init>", "(IFFFF)V");

	// create some mentalStateMod instances for use as script paramters
	for (i = 0; i < MAX_MENTAL_STATE_MOD_PARAMS; ++i)
	{
		localInstance = createNewObject(ms_clsMentalStateMod, ms_midMentalStateMod,
			0, 0, 0, 0, 0);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
		GlobalInstances::ms_mentalStateMods[i] = GlobalRefPtr(new GlobalRef(*localInstance));
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
	}
	// create the mentalStateModList
	for (i = 0; i < MAX_RECURSION_COUNT; ++i)
	{
		localArrayInstance = createNewObjectArray(MAX_MENTAL_STATE_MOD_PARAMS, ms_clsMentalStateMod);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
		ms_mentalStateModList[i] = GlobalArrayRefPtr(new GlobalArrayRef(*localArrayInstance));
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
	}

	//----------------------------------------------------------------------
	// MenuInfo

	GET_CLASS(ms_clsMenuInfo, "script/menu_info");
	GET_METHOD(ms_midMenuInfo, ms_clsMenuInfo, "<init>", "()V");
	GET_METHOD(ms_midMenuInfoSetMenuItemsInternal, ms_clsMenuInfo, "setMenuItemsInternal", "([Lscript/menu_info_data;)Z");
	GET_METHOD(ms_midMenuInfoGetMenuItemsInternal, ms_clsMenuInfo, "getMenuItemsInternal", "()[Ljava/lang/Object;");

	//-- create the global instance for the menu_info
	{
		localInstance = createNewObject(ms_clsMenuInfo, ms_midMenuInfo);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
		GlobalInstances::ms_menuInfo = GlobalRefPtr(new GlobalRef(*localInstance));
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
	}

	//-- menu_info_data
	//
	GET_CLASS(ms_clsMenuInfoData, "script/menu_info_data");
	GET_FIELD(ms_fidMenuInfoDataId, ms_clsMenuInfoData, "m_id", "I");
	GET_FIELD(ms_fidMenuInfoDataParent, ms_clsMenuInfoData, "m_parent", "I");
	GET_FIELD(ms_fidMenuInfoDataLabel, ms_clsMenuInfoData, "m_label", "Lscript/string_id;");
	GET_FIELD(ms_fidMenuInfoDataType, ms_clsMenuInfoData, "m_type", "I");
	GET_FIELD(ms_fidMenuInfoDataEnabled, ms_clsMenuInfoData, "m_enabled", "Z");
	GET_FIELD(ms_fidMenuInfoDataServerNotify, ms_clsMenuInfoData, "m_serverNotify", "Z");
	GET_METHOD(ms_midMenuInfoData, ms_clsMenuInfoData, "<init>", "()V");

	//
	// get class and fields for class draft_schematic
	//
	GET_CLASS(ms_clsDraftSchematic, "script/draft_schematic");
	GET_FIELD(ms_fidDraftSchematicCategory, ms_clsDraftSchematic, "m_category", "I");
	GET_FIELD(ms_fidDraftSchematicComplexity, ms_clsDraftSchematic, "m_complexity", "F");
	GET_FIELD(ms_fidDraftSchematicSlots, ms_clsDraftSchematic, "m_slots", "[Lscript/draft_schematic$slot;");
	GET_FIELD(ms_fidDraftSchematicAttribs, ms_clsDraftSchematic, "m_attribs", "[Lscript/draft_schematic$attribute;");
	GET_FIELD(ms_fidDraftSchematicExperimentalAttribs, ms_clsDraftSchematic, "m_experimentalAttribs", "[Lscript/draft_schematic$attribute;");
	GET_FIELD(ms_fidDraftSchematicCustomizations, ms_clsDraftSchematic, "m_customizations", "[Lscript/draft_schematic$custom;");
	GET_FIELD(ms_fidDraftSchematicAttribMap, ms_clsDraftSchematic, "m_attribMap", "Ljava/util/Hashtable;");
	GET_FIELD(ms_fidDraftSchematicObjectTemplateCreated, ms_clsDraftSchematic, "m_objectTemplateCreated", "I");
	GET_FIELD(ms_fidDraftSchematicScripts, ms_clsDraftSchematic, "m_scripts", "[Ljava/lang/String;");

	// get the draft_schematic.slot data
	GET_CLASS(ms_clsDraftSchematicSlot, "script/draft_schematic$slot");
	GET_FIELD(ms_fidDraftSchematicSlotName, ms_clsDraftSchematicSlot, "name", "Lscript/string_id;");
	GET_FIELD(ms_fidDraftSchematicSlotOption, ms_clsDraftSchematicSlot, "slotOption", "I");
	GET_FIELD(ms_fidDraftSchematicSlotIngredientType, ms_clsDraftSchematicSlot, "ingredientType", "I");
	GET_FIELD(ms_fidDraftSchematicSlotIngredientName, ms_clsDraftSchematicSlot, "ingredientName", "Ljava/lang/String;");
	GET_FIELD(ms_fidDraftSchematicSlotIngredients, ms_clsDraftSchematicSlot, "ingredients", "[Lscript/draft_schematic$simple_ingredient;");
	GET_FIELD(ms_fidDraftSchematicSlotComplexity, ms_clsDraftSchematicSlot, "complexity", "F");
	GET_FIELD(ms_fidDraftSchematicSlotAmountRequired, ms_clsDraftSchematicSlot, "amountRequired", "I");
	GET_FIELD(ms_fidDraftSchematicSlotAppearance, ms_clsDraftSchematicSlot, "appearance", "Ljava/lang/String;");

	// get the draft_schematic.attribute data
	GET_CLASS(ms_clsDraftSchematicAttrib, "script/draft_schematic$attribute");
	GET_FIELD(ms_fidDraftSchematicAttribName, ms_clsDraftSchematicAttrib, "name", "Lscript/string_id;");
	GET_FIELD(ms_fidDraftSchematicAttribMinValue, ms_clsDraftSchematicAttrib, "minValue", "F");
	GET_FIELD(ms_fidDraftSchematicAttribMaxValue, ms_clsDraftSchematicAttrib, "maxValue", "F");
	GET_FIELD(ms_fidDraftSchematicAttribResourceMaxValue, ms_clsDraftSchematicAttrib, "resourceMaxValue", "F");
	GET_FIELD(ms_fidDraftSchematicAttribCurrentValue, ms_clsDraftSchematicAttrib, "currentValue", "F");

	// get the draft_schematic.simple_ingredient data
	GET_CLASS(ms_clsDraftSchematicSimpleIngredient, "script/draft_schematic$simple_ingredient");
	GET_FIELD(ms_fidDraftSchematicSimpleIngredientIngredient, ms_clsDraftSchematicSimpleIngredient, "ingredient", "Lscript/obj_id;");
	GET_FIELD(ms_fidDraftSchematicSimpleIngredientCount, ms_clsDraftSchematicSimpleIngredient, "count", "I");
	GET_FIELD(ms_fidDraftSchematicSimpleIngredientSource, ms_clsDraftSchematicSimpleIngredient, "source", "Lscript/obj_id;");
	GET_FIELD(ms_fidDraftSchematicSimpleIngredientXpType, ms_clsDraftSchematicSimpleIngredient, "xpType", "I");
	GET_FIELD(ms_fidDraftSchematicSimpleIngredientXpAmount, ms_clsDraftSchematicSimpleIngredient, "xpAmount", "I");

	// get the draft_schematic.custom data
	GET_CLASS(ms_clsDraftSchematicCustom, "script/draft_schematic$custom");
	GET_FIELD(ms_fidDraftSchematicCustomName, ms_clsDraftSchematicCustom, "name", "Ljava/lang/String;");
	GET_FIELD(ms_fidDraftSchematicCustomValue, ms_clsDraftSchematicCustom, "value", "I");
	GET_FIELD(ms_fidDraftSchematicCustomMinValue, ms_clsDraftSchematicCustom, "minValue", "I");
	GET_FIELD(ms_fidDraftSchematicCustomMaxValue, ms_clsDraftSchematicCustom, "maxValue", "I");

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// customization data
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	GET_CLASS(ms_clsCustomVar, "script/custom_var");
	GET_METHOD(ms_midCustomVarObjIdStringInt, ms_clsCustomVar, "<init>", "(JLjava/lang/String;I)V");

	GET_CLASS(ms_clsRangedIntCustomVar, "script/ranged_int_custom_var");
	GET_METHOD(ms_midRangedIntCustomVar, ms_clsRangedIntCustomVar, "<init>", "(JLjava/lang/String;III)V");

	GET_CLASS(ms_clsPalcolorCustomVar, "script/palcolor_custom_var");
	GET_METHOD(ms_midPalcolorCustomVar, ms_clsPalcolorCustomVar, "<init>", "(JLjava/lang/String;III)V");

	GET_CLASS(ms_clsColor, "script/color");
	GET_METHOD(ms_midColor, ms_clsColor, "<init>", "(IIII)V");

	//----------------------------------------------------------------------

	// get the class, field ids and constructor methodID for location
	GET_CLASS(ms_clsLocation, "script/location");
	GET_CLASS(ms_clsLocationArray, "[Lscript/location;");
	GET_FIELD(ms_fidLocationX, ms_clsLocation, "x", "F");
	GET_FIELD(ms_fidLocationY, ms_clsLocation, "y", "F");
	GET_FIELD(ms_fidLocationZ, ms_clsLocation, "z", "F");
	GET_FIELD(ms_fidLocationArea, ms_clsLocation, "area", "Ljava/lang/String;");
	GET_FIELD(ms_fidLocationCell, ms_clsLocation, "cell", "Lscript/obj_id;");

	// get class and fields for map location
	GET_CLASS(ms_clsMapLocation, "script/map_location");
	GET_METHOD(ms_midMapLocation, ms_clsMapLocation, "<init>", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;JJB)V");

	// get class and fields for class region
	GET_CLASS(ms_clsRegion, "script/region");
	GET_FIELD(ms_fidRegionName, ms_clsRegion, "m_name", "Ljava/lang/String;");
	GET_FIELD(ms_fidRegionPlanet, ms_clsRegion, "m_planetName", "Ljava/lang/String;");
	GET_METHOD(ms_midRegion, ms_clsRegion, "<init>", "(Ljava/lang/String;IIIIIIIILjava/lang/String;)V");

	// get class and fields for base_class.range_info
	GET_CLASS(ms_clsBaseClassRangeInfo, "script/base_class$range_info");
	GET_FIELD(ms_fidBaseClassRangeInfoMinRange, ms_clsBaseClassRangeInfo, "minRange", "F");
	GET_FIELD(ms_fidBaseClassRangeInfoMaxRange, ms_clsBaseClassRangeInfo, "maxRange", "F");

	// get class and fields for base_class.attacker_results
	GET_CLASS(ms_clsBaseClassAttackerResults, "script/base_class$attacker_results");
	GET_FIELD(ms_fidBaseClassAttackerResultsId, ms_clsBaseClassAttackerResults, "id", "Lscript/obj_id;");
	GET_FIELD(ms_fidBaseClassAttackerResultsWeapon, ms_clsBaseClassAttackerResults, "weapon", "Lscript/obj_id;");
	GET_FIELD(ms_fidBaseClassAttackerResultsPosture, ms_clsBaseClassAttackerResults, "endPosture", "I");
	GET_FIELD(ms_fidBaseClassAttackerResultsTrailBits, ms_clsBaseClassAttackerResults, "m_trailBits", "I");
	GET_FIELD(ms_fidBaseClassAttackerResultsClientEffectId, ms_clsBaseClassAttackerResults, "clientEffectId", "I");
	GET_FIELD(ms_fidBaseClassAttackerResultsActionName, ms_clsBaseClassAttackerResults, "actionName", "I");
	GET_FIELD(ms_fidBaseClassAttackerResultsUseLocation, ms_clsBaseClassAttackerResults, "useLocation", "Z");
	GET_FIELD(ms_fidBaseClassAttackerResultsTargetLocation, ms_clsBaseClassAttackerResults, "targetLocation", "Lscript/vector;");
	GET_FIELD(ms_fidBaseClassAttackerResultsTargetCell, ms_clsBaseClassAttackerResults, "targetCell", "Lscript/obj_id;");

	// get class and fields for base_class.defender_results
	GET_CLASS(ms_clsBaseClassDefenderResults, "script/base_class$defender_results");
	GET_FIELD(ms_fidBaseClassDefenderResultsId, ms_clsBaseClassDefenderResults, "id", "Lscript/obj_id;");
	GET_FIELD(ms_fidBaseClassDefenderResultsPosture, ms_clsBaseClassDefenderResults, "endPosture", "I");
	GET_FIELD(ms_fidBaseClassDefenderResultsResult, ms_clsBaseClassDefenderResults, "result", "I");
	GET_FIELD(ms_fidBaseClassDefenderResultsClientEffectId, ms_clsBaseClassDefenderResults, "clientEffectId", "I");
	GET_FIELD(ms_fidBaseClassDefenderResultsHitLocation, ms_clsBaseClassDefenderResults, "hitLocation", "I");
	GET_FIELD(ms_fidBaseClassDefenderDamageAmount, ms_clsBaseClassDefenderResults, "damageAmount", "I");

	// get class obj_id info
	GET_CLASS(ms_clsObjId, "script/obj_id");
	GET_CLASS(ms_clsObjIdArray, "[Lscript/obj_id;");
	GET_METHOD(ms_midObjIdGetValue, ms_clsObjId, "getValue", "()J");
	GET_METHOD(ms_midObjIdSetAuthoritative, ms_clsObjId, "setAuthoritative", "(ZI)V");
	GET_METHOD(ms_midObjIdSetLoaded, ms_clsObjId, "setLoaded", "()V");
	GET_METHOD(ms_midObjIdSetInitialized, ms_clsObjId, "setInitialized", "()V");
	GET_METHOD(ms_midObjIdSetLoggedIn, ms_clsObjId, "setLoggedIn", "(Z)V");
	GET_METHOD(ms_midObjIdAttachScript, ms_clsObjId, "attachScript", "(Ljava/lang/String;)V");
	GET_METHOD(ms_midObjIdAttachScripts, ms_clsObjId, "attachScript", "([Ljava/lang/String;)V");
	GET_METHOD(ms_midObjIdDetachScript, ms_clsObjId, "detachScript", "(Ljava/lang/String;)V");
	GET_METHOD(ms_midObjIdDetachAllScripts, ms_clsObjId, "detachAllScripts", "()V");
	GET_METHOD(ms_midObjIdSetScriptVarInt, ms_clsObjId, "setScriptVar", "(Ljava/lang/String;I)V");
	GET_METHOD(ms_midObjIdSetScriptVarFloat, ms_clsObjId, "setScriptVar", "(Ljava/lang/String;F)V");
	GET_METHOD(ms_midObjIdSetScriptVarString, ms_clsObjId, "setScriptVar", "(Ljava/lang/String;Ljava/lang/String;)V");
	GET_METHOD(ms_midObjIdClearScriptVars, ms_clsObjId, "clearScriptVars", "()V");
	GET_STATIC_METHOD(ms_midObjIdGetObjId, ms_clsObjId, "getObjId", "(J)Lscript/obj_id;");
	GET_STATIC_METHOD(ms_midObjIdClearObjId, ms_clsObjId, "clearObjId", "(J)V");
	GET_STATIC_METHOD(ms_midObjIdFlagDestroyed, ms_clsObjId, "flagDestroyed", "(J)V");
	GET_STATIC_METHOD(ms_midObjIdPackAllDeltaScriptVars, ms_clsObjId, "packAllScriptVars", "()V");
	GET_STATIC_METHOD(ms_midObjIdPackScriptVars, ms_clsObjId, "packScriptVars", "(J)[B");
	GET_STATIC_METHOD(ms_midObjIdUnpackDeltaScriptVars, ms_clsObjId, "unpackDeltaScriptVars", "(J[B)V");
	GET_STATIC_METHOD(ms_midObjIdUnpackScriptVars, ms_clsObjId, "unpackScriptVars", "(J[B)V");

	// get class combat_engine info
	GET_CLASS(ms_clsCombatEngine, "script/combat_engine");
	GET_CLASS(ms_clsCombatEngineCombatantData, "script/combat_engine$combatant_data");
	GET_FIELD(ms_fidCombatEngineCombatantDataPos, ms_clsCombatEngineCombatantData, "pos", "Lscript/location;");
	GET_FIELD(ms_fidCombatEngineCombatantDataWorldPos, ms_clsCombatEngineCombatantData, "worldPos", "Lscript/location;");
	GET_FIELD(ms_fidCombatEngineCombatantDataRadius, ms_clsCombatEngineCombatantData, "radius", "F");
	GET_FIELD(ms_fidCombatEngineCombatantDataIsCreature, ms_clsCombatEngineCombatantData, "isCreature", "Z");
	GET_FIELD(ms_fidCombatEngineCombatantDataPosture, ms_clsCombatEngineCombatantData, "posture", "I");
	GET_FIELD(ms_fidCombatEngineCombatantDataLocomotion, ms_clsCombatEngineCombatantData, "locomotion", "I");
	GET_FIELD(ms_fidCombatEngineCombatantDataScriptMod, ms_clsCombatEngineCombatantData, "scriptMod", "I");
	
	GET_CLASS(ms_clsCombatEngineAttackerData, "script/combat_engine$attacker_data");
	GET_FIELD(ms_fidCombatEngineAttackerDataWeaponSkill, ms_clsCombatEngineAttackerData, "weaponSkillMod", "I");
	GET_FIELD(ms_fidCombatEngineAttackerDataAims, ms_clsCombatEngineAttackerData, "aims", "I");

	GET_CLASS(ms_clsCombatEngineDefenderData, "script/combat_engine$defender_data");
	GET_FIELD(ms_fidCombatEngineDefenderDataCombatSkeleton, ms_clsCombatEngineDefenderData, "combatSkeleton", "I");
	GET_FIELD(ms_fidCombatEngineDefenderDataCover, ms_clsCombatEngineDefenderData, "cover", "I");
	GET_FIELD(ms_fidCombatEngineDefenderDataHitLocationChances, ms_clsCombatEngineDefenderData, "hitLocationChances", "[I");

	GET_CLASS(ms_clsCombatEngineWeaponData, "script/combat_engine$weapon_data");
	GET_FIELD(ms_fidCombatEngineWeaponDataId, ms_clsCombatEngineWeaponData, "id", "Lscript/obj_id;");
	GET_FIELD(ms_fidCombatEngineWeaponDataMinDamage, ms_clsCombatEngineWeaponData, "minDamage", "I");
	GET_FIELD(ms_fidCombatEngineWeaponDataMaxDamage, ms_clsCombatEngineWeaponData, "maxDamage", "I");
	GET_FIELD(ms_fidCombatEngineWeaponDataWeaponType, ms_clsCombatEngineWeaponData, "weaponType", "I");
	GET_FIELD(ms_fidCombatEngineWeaponDataAttackType, ms_clsCombatEngineWeaponData, "attackType", "I");
	GET_FIELD(ms_fidCombatEngineWeaponDataDamageType, ms_clsCombatEngineWeaponData, "damageType", "I");
	GET_FIELD(ms_fidCombatEngineWeaponDataElementalType, ms_clsCombatEngineWeaponData, "elementalType", "I");
	GET_FIELD(ms_fidCombatEngineWeaponDataElementalValue, ms_clsCombatEngineWeaponData, "elementalValue", "I");
	GET_FIELD(ms_fidCombatEngineWeaponDataAttackSpeed, ms_clsCombatEngineWeaponData, "attackSpeed", "F");
	GET_FIELD(ms_fidCombatEngineWeaponDataWoundChance, ms_clsCombatEngineWeaponData, "woundChance", "F");
    GET_FIELD(ms_fidCombatEngineWeaponDataAccuracy, ms_clsCombatEngineWeaponData, "accuracy", "I");
	GET_FIELD(ms_fidCombatEngineWeaponDataMinRange, ms_clsCombatEngineWeaponData, "minRange", "F");
	GET_FIELD(ms_fidCombatEngineWeaponDataMaxRange, ms_clsCombatEngineWeaponData, "maxRange", "F");
	GET_FIELD(ms_fidCombatEngineWeaponDataDamageRadius, ms_clsCombatEngineWeaponData, "damageRadius", "F");
	GET_FIELD(ms_fidCombatEngineWeaponDataAttackCost, ms_clsCombatEngineWeaponData, "attackCost", "I");
	GET_FIELD(ms_fidCombatEngineWeaponDataIsDisabled, ms_clsCombatEngineWeaponData, "isDisabled", "Z");

	GET_CLASS(ms_clsCombatEngineHitResult, "script/combat_engine$hit_result");
	GET_FIELD(ms_fidCombatEngineHitResultSuccess, ms_clsCombatEngineHitResult, "success", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultCritical, ms_clsCombatEngineHitResult, "critical", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultGlancing, ms_clsCombatEngineHitResult, "glancing", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultCrushing, ms_clsCombatEngineHitResult, "crushing", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultStrikethrough, ms_clsCombatEngineHitResult, "strikethrough", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultStrikethroughAmount, ms_clsCombatEngineHitResult, "strikethroughAmmount", "F");
	GET_FIELD(ms_fidCombatEngineHitResultEvadeResult, ms_clsCombatEngineHitResult, "evadeResult", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultEvadeAmount, ms_clsCombatEngineHitResult, "evadeAmmount", "F");
	GET_FIELD(ms_fidCombatEngineHitResultBlockResult, ms_clsCombatEngineHitResult, "blockResult", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultBlock, ms_clsCombatEngineHitResult, "block", "I");
	GET_FIELD(ms_fidCombatEngineHitResultDodge, ms_clsCombatEngineHitResult, "dodge", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultParry, ms_clsCombatEngineHitResult, "parry", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultProc, ms_clsCombatEngineHitResult, "proc", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultBaseRoll, ms_clsCombatEngineHitResult, "baseRoll", "I");
	GET_FIELD(ms_fidCombatEngineHitResultFinalRoll, ms_clsCombatEngineHitResult, "finalRoll", "I");
	GET_FIELD(ms_fidCombatEngineHitResultAttackVal, ms_clsCombatEngineHitResult, "attackVal", "F");
	GET_FIELD(ms_fidCombatEngineHitResultCanSee, ms_clsCombatEngineHitResult, "canSee", "Z");
	GET_FIELD(ms_fidCombatEngineHitResultHitLocation, ms_clsCombatEngineHitResult, "hitLocation", "I");
	GET_FIELD(ms_fidCombatEngineHitResultDamage, ms_clsCombatEngineHitResult, "damage", "I");
	GET_FIELD(ms_fidCombatEngineHitResultDamageType, ms_clsCombatEngineHitResult, "damageType", "I");
	GET_FIELD(ms_fidCombatEngineHitResultRawDamage, ms_clsCombatEngineHitResult, "rawDamage", "I");
	GET_FIELD(ms_fidCombatEngineHitResultElementalDamage, ms_clsCombatEngineHitResult, "elementalDamage", "I");
	GET_FIELD(ms_fidCombatEngineHitResultElementalDamageType, ms_clsCombatEngineHitResult, "elementalDamageType", "I");
	GET_FIELD(ms_fidCombatEngineHitResultBleedDamage, ms_clsCombatEngineHitResult, "bleedDamage", "I");
	GET_FIELD(ms_fidCombatEngineHitResultCritDamage, ms_clsCombatEngineHitResult, "critDamage", "I");
	GET_FIELD(ms_fidCombatEngineHitResultBlockedDamage, ms_clsCombatEngineHitResult, "blockedDamage", "I");
	GET_FIELD(ms_fidCombatEngineHitResultBlockingArmor, ms_clsCombatEngineHitResult, "blockingArmor", "Lscript/obj_id;");
	GET_FIELD(ms_fidCombatEngineHitResultBleedingChance, ms_clsCombatEngineHitResult, "bleedingChance", "I");

	// be used parameters for events
	GET_CLASS(ms_clsStringId, "script/string_id");
	GET_CLASS(ms_clsStringIdArray, "[Lscript/string_id;");
	GET_CLASS(ms_clsModifiableStringId, "script/modifiable_string_id");
	GET_FIELD(ms_fidStringIdTable, ms_clsStringId, "m_table", "Ljava/lang/String;");
	GET_FIELD(ms_fidStringIdAsciiId, ms_clsStringId, "m_asciiId", "Ljava/lang/String;");
	GET_FIELD(ms_fidStringIdIndexId, ms_clsStringId, "m_indexId", "I");
	GET_METHOD(ms_midStringId, ms_clsStringId, "<init>", "(Ljava/lang/String;I)V");

	for (i = 0; i < MAX_STRING_ID_PARAMS; ++i)
	{
		localInstance = createNewObject(ms_clsStringId, ms_midStringId, nullptr, -1);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
		GlobalInstances::ms_stringIds[i] = GlobalRefPtr(new GlobalRef(*localInstance));
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
	}

	GET_METHOD(constructor, ms_clsModifiableStringId, "<init>", "(Ljava/lang/String;I)V");
	for (i = 0; i < MAX_MODIFIABLE_STRING_ID_PARAMS; ++i)
	{
		localInstance = createNewObject(ms_clsModifiableStringId, constructor,
			nullptr, -1);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
		GlobalInstances::ms_modifiableStringIds[i] = GlobalRefPtr(new GlobalRef(*localInstance));
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return false;
		}
	}

	// get class and fields for class transform
	GET_CLASS(ms_clsTransform, "script/transform");
	GET_CLASS(ms_clsTransformArray, "[Lscript/transform;");
	GET_METHOD(ms_midTransform, ms_clsTransform, "<init>", "(FFFFFFFFFFFF)V");
	GET_FIELD(ms_fidTransformMatrix, ms_clsTransform, "matrix", "[[F");

	// get class and fields for class vector
	GET_CLASS(ms_clsVector, "script/vector");
	GET_CLASS(ms_clsVectorArray, "[Lscript/vector;");
	GET_FIELD(ms_fidVectorX, ms_clsVector, "x", "F");
	GET_FIELD(ms_fidVectorY, ms_clsVector, "y", "F");
	GET_FIELD(ms_fidVectorZ, ms_clsVector, "z", "F");

	GET_CLASS(ms_clsResourceDensity, "script/resource_density");
	GET_FIELD(ms_fidResourceDensityResourceType, ms_clsResourceDensity, "m_resourceType", "Lscript/obj_id;");
	GET_FIELD(ms_fidResourceDensityDensity, ms_clsResourceDensity, "m_density", "F");

	GET_CLASS(ms_clsResourceAttribute, "script/resource_attribute");
	GET_FIELD(ms_fidResourceAttributeName, ms_clsResourceAttribute, "m_name", "Ljava/lang/String;");
	GET_FIELD(ms_fidResourceAttributeValue, ms_clsResourceAttribute, "m_value", "I");

	GET_CLASS(ms_clsLibrarySpaceTransition, "script/library/space_transition");
	GET_STATIC_METHOD(ms_midLibrarySpaceTransitionSetPlayerOvert, ms_clsLibrarySpaceTransition, "setPlayerOvert", "(J)V");
	GET_STATIC_METHOD(ms_midLibrarySpaceTransitionClearOvertStatus, ms_clsLibrarySpaceTransition, "clearOvertStatus", "(J)V");
	
	///////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////
	
	// get class and field info needed by the CS Handler stuff.  These may not have been
	// written specifically for the CS Handler.
	
	
	
	GET_CLASS( ms_clsLibraryDump, "script/library/dump" );
	GET_STATIC_METHOD( ms_midLibraryDumpDumpTargetInfo, ms_clsLibraryDump, "getTargetInfoStringByLong", "(J)Ljava/lang/String;" );
	
	GET_CLASS( ms_clsLibraryGMLib, "script/library/gmlib" );
	GET_STATIC_METHOD( ms_midLibraryGMLibFreeze, ms_clsLibraryGMLib, "freezePlayer", "(J)Ljava/lang/String;" );
	GET_STATIC_METHOD( ms_midLibraryGMLibUnfreeze, ms_clsLibraryGMLib, "unFreezePlayer", "(J)V" );

	
	/////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////
	// register our native methods
	bool registerSuccess = true;
	registerSuccess &= ScriptMethodsActionStatesNamespace::install();
	registerSuccess &= ScriptMethodsAiNamespace::install();
	registerSuccess &= ScriptMethodsAnimationNamespace::install();
	registerSuccess &= ScriptMethodsAttributesNamespace::install();
	registerSuccess &= ScriptMethodsAuctionNamespace::install();
	registerSuccess &= ScriptMethodsBankNamespace::install();
	registerSuccess &= ScriptMethodsBeastMasterNamespace::install();
	registerSuccess &= ScriptMethodsBroadcastingNamespace::install();
	registerSuccess &= ScriptMethodsBuffBuilderNamespace::install();
	registerSuccess &= ScriptMethodsBuffNamespace::install();
	registerSuccess &= ScriptMethodsChatNamespace::install();
	registerSuccess &= ScriptMethodsCityNamespace::install();
	registerSuccess &= ScriptMethodsClientEffectNamespace::install();
	registerSuccess &= ScriptMethodsClusterWideDataNamespace::install();
	registerSuccess &= ScriptMethodsCollectionNamespace::install();
	registerSuccess &= ScriptMethodsCombatNamespace::install();
	registerSuccess &= ScriptMethodsCommandQueueNamespace::install();
	registerSuccess &= ScriptMethodsConsoleNamespace::install();
	registerSuccess &= ScriptMethodsContainersNamespace::install();
	registerSuccess &= ScriptMethodsCraftingNamespace::install();
	registerSuccess &= ScriptMethodsDataTableNamespace::install();
	registerSuccess &= ScriptMethodsDebugNamespace::install();
	registerSuccess &= ScriptMethodsDynamicVariableNamespace::install();
	registerSuccess &= ScriptMethodsFormNamespace::install();
	registerSuccess &= ScriptMethodsGlobalQueryNamespace::install();
	registerSuccess &= ScriptMethodsGuildNamespace::install();
	registerSuccess &= ScriptMethodsHateListNamespace::install();
	registerSuccess &= ScriptMethodsHolocubeNamespace::install();
	registerSuccess &= ScriptMethodsHyperspaceNamespace::install();
	registerSuccess &= ScriptMethodsImageDesignNamespace::install();
	registerSuccess &= ScriptMethodsInstallationNamespace::install();
	registerSuccess &= ScriptMethodsInterestNamespace::install();
	registerSuccess &= ScriptMethodsInteriorsNamespace::install();
	registerSuccess &= ScriptMethodsJediNamespace::install();
	registerSuccess &= ScriptMethodsMapNamespace::install();
	registerSuccess &= ScriptMethodsMentalStatesNamespace::install();
	registerSuccess &= ScriptMethodsMenuNamespace::install();
	registerSuccess &= ScriptMethodsMissionNamespace::install();
	registerSuccess &= ScriptMethodsMoneyNamespace::install();
	registerSuccess &= ScriptMethodsMountNamespace::install();
	registerSuccess &= ScriptMethodsNewbieTutorialNamespace::install();
	registerSuccess &= ScriptMethodsNotificationNamespace::install();
	registerSuccess &= ScriptMethodsNpcNamespace::install();
	registerSuccess &= ScriptMethodsObjectCreateNamespace::install();
	registerSuccess &= ScriptMethodsObjectInfoNamespace::install();
	registerSuccess &= ScriptMethodsObjectMoveNamespace::install();
	registerSuccess &= ScriptMethodsPermissionsNamespace::install();
	registerSuccess &= ScriptMethodsPilotNamespace::install();
	registerSuccess &= ScriptMethodsPlanetNamespace::install();
	registerSuccess &= ScriptMethodsPlayerAccountNamespace::install();
	registerSuccess &= ScriptMethodsPlayerQuestNamespace::install();
	registerSuccess &= ScriptMethodsPvpNamespace::install();
	registerSuccess &= ScriptMethodsQuestNamespace::install();
	registerSuccess &= ScriptMethodsRegionNamespace::install();
	registerSuccess &= ScriptMethodsRegion3dNamespace::install();
	registerSuccess &= ScriptMethodsRemoteDebugNamespace::install();
	registerSuccess &= ScriptMethodsResourceNamespace::install();
	registerSuccess &= ScriptMethodsScriptNamespace::install();
	registerSuccess &= ScriptMethodsServerUINamespace::install();
	registerSuccess &= ScriptMethodsShipNamespace::install();
	registerSuccess &= ScriptMethodsSkillNamespace::install();
	registerSuccess &= ScriptMethodsSpawnerNamespace::install();
	registerSuccess &= ScriptMethodsStringNamespace::install();
	registerSuccess &= ScriptMethodsSystemNamespace::install();
	registerSuccess &= ScriptMethodsTerrainNamespace::install();
	registerSuccess &= ScriptMethodsTriggerVolumeNamespace::install();
	registerSuccess &= ScriptMethodsVeteranNamespace::install();
	registerSuccess &= ScriptMethodsWaypointNamespace::install();
	registerSuccess &= ScriptMethodsWorldInfoNamespace::install();

	if (!registerSuccess)
	{
		FATAL(true, ("JavaLibrary RegisterNatives fail!\n"));
		return false;
	}

	++ms_envCount;
	return true;
}	// JavaLibrary::connectToJava

//----------------------------------------------------------------------

bool JavaLibrary::registerNatives(const JNINativeMethod natives[], int count)
{
	jclass baseClass = ms_env->FindClass("script/base_class");
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return false;
	}

	int result = 0;
	for (int i = 0; i < count; ++i)
	{
		if (!natives[i].signature)
		{
			DEBUG_REPORT_LOG(true, ("RegisterNatives failed: %s - nullptr signature\n", natives[i].name));
			result = 1;
			continue;
		}
		int lresult = ms_env->RegisterNatives(baseClass, &natives[i], 1);
		if (lresult != 0)
		{
			ms_env->ExceptionClear();
			DEBUG_REPORT_LOG(true, ("RegisterNatives failed: %s: %s\n", natives[i].name, natives[i].signature));
			result = lresult;
		}
	}
	
	ms_env->DeleteLocalRef(baseClass);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return false;
	}
	if (result != 0)
	{
		FATAL(true, ("JavaLibrary RegisterNatives fail!\n"));
		return false;
	}
	return true;
}

//----------------------------------------------------------------------

/**
 * Disconnects this thread from the Java VM. Any vars that reference Java objects 
 * will be invalid after this call.
 */
void JavaLibrary::disconnectFromJava()
{
int i;

	if (ms_jvm == 0 || ms_env == 0)
		return;

	FREE_CLASS(ms_clsScriptEntry);
	FREE_CLASS(ms_clsObject);
	FREE_CLASS(ms_clsClass);
	FREE_CLASS(ms_clsMethod);
	FREE_CLASS(ms_clsBoolean);
	FREE_CLASS(ms_clsBooleanArray);
	FREE_CLASS(ms_clsInteger);
	FREE_CLASS(ms_clsIntegerArray);
	FREE_CLASS(ms_clsModifiableInt);
	FREE_CLASS(ms_clsFloat);
	FREE_CLASS(ms_clsFloatArray);
	FREE_CLASS(ms_clsModifiableFloat);
	FREE_CLASS(ms_clsString);
	FREE_CLASS(ms_clsStringArray);
	FREE_CLASS(ms_clsMap);
	FREE_CLASS(ms_clsHashtable);
	FREE_CLASS(ms_clsThrowable);
	FREE_CLASS(ms_clsError);
	FREE_CLASS(ms_clsStackOverflowError);
	FREE_CLASS(ms_clsThread);
	FREE_CLASS(ms_clsInternalScriptError);
	FREE_CLASS(ms_clsInternalScriptSeriousError);
	FREE_CLASS(ms_clsDictionary);
	FREE_CLASS(ms_clsCollection);
	FREE_CLASS(ms_clsDynamicVariable);
	FREE_CLASS(ms_clsDynamicVariableList);
	FREE_CLASS(ms_clsAttribute);
	FREE_CLASS(ms_clsAttribMod);
	FREE_CLASS(ms_clsMentalState);
	FREE_CLASS(ms_clsMentalStateMod);
	FREE_CLASS(ms_clsMenuInfo);
	FREE_CLASS(ms_clsMenuInfoData);
	FREE_CLASS(ms_clsDraftSchematic);
	FREE_CLASS(ms_clsDraftSchematicSlot);
	FREE_CLASS(ms_clsDraftSchematicAttrib);
	FREE_CLASS(ms_clsDraftSchematicSimpleIngredient);
	FREE_CLASS(ms_clsDraftSchematicCustom);
	FREE_CLASS(ms_clsCustomVar);
	FREE_CLASS(ms_clsRangedIntCustomVar);
	FREE_CLASS(ms_clsPalcolorCustomVar);
	FREE_CLASS(ms_clsColor);
	FREE_CLASS(ms_clsLocation);
	FREE_CLASS(ms_clsLocationArray);
	FREE_CLASS(ms_clsRegion);
	FREE_CLASS(ms_clsObjId);
	FREE_CLASS(ms_clsObjIdArray);
	FREE_CLASS(ms_clsStringId);
	FREE_CLASS(ms_clsStringIdArray);
	FREE_CLASS(ms_clsModifiableStringId);
	FREE_CLASS(ms_clsBaseClassRangeInfo);
	FREE_CLASS(ms_clsBaseClassAttackerResults);
	FREE_CLASS(ms_clsBaseClassDefenderResults);
	FREE_CLASS(ms_clsCombatEngine);
	FREE_CLASS(ms_clsCombatEngineCombatantData);
	FREE_CLASS(ms_clsCombatEngineAttackerData);
	FREE_CLASS(ms_clsCombatEngineDefenderData);
	FREE_CLASS(ms_clsCombatEngineWeaponData);
	FREE_CLASS(ms_clsCombatEngineHitResult);
	FREE_CLASS(ms_clsTransform);
	FREE_CLASS(ms_clsTransformArray);
	FREE_CLASS(ms_clsVector);
	FREE_CLASS(ms_clsVectorArray);
	FREE_CLASS(ms_clsResourceDensity);
	FREE_CLASS(ms_clsResourceAttribute);
	FREE_CLASS(ms_clsLibrarySpaceTransition);
	FREE_CLASS(ms_clsLibraryDump);
	FREE_CLASS(ms_clsLibraryGMLib);

	if (ms_scriptEntry != nullptr)
	{
		if (ms_env)
			ms_env->DeleteGlobalRef(ms_scriptEntry);
		ms_scriptEntry = nullptr;
	}

	for (i = 0; i < MAX_RECURSION_COUNT; ++i)
	{
		ms_attribModList[i] = GlobalArrayRef::cms_nullPtr;
		ms_mentalStateModList[i] = GlobalArrayRef::cms_nullPtr;
	}
	for (i = 0; i < MAX_STRING_ID_PARAMS; ++i)
		GlobalInstances::ms_stringIds[i] = GlobalRef::cms_nullPtr;
	for (i = 0; i < MAX_ATTRIB_MOD_PARAMS; ++i)
		GlobalInstances::ms_attribMods[i] = GlobalRef::cms_nullPtr;
	for (i = 0; i < MAX_MENTAL_STATE_MOD_PARAMS; ++i)
		GlobalInstances::ms_mentalStateMods[i] = GlobalRef::cms_nullPtr;
	for (i = 0; i < MAX_MODIFIABLE_INT_PARAMS; ++i)
		GlobalInstances::ms_modifiableInts[i] = GlobalRef::cms_nullPtr;
	for (i = 0; i < MAX_MODIFIABLE_FLOAT_PARAMS; ++i)
		GlobalInstances::ms_modifiableFloats[i] = GlobalRef::cms_nullPtr;
	for (i = 0; i < MAX_MODIFIABLE_STRING_ID_PARAMS; ++i)
		GlobalInstances::ms_modifiableStringIds[i] = GlobalRef::cms_nullPtr;
	GlobalInstances::ms_menuInfo = GlobalRef::cms_nullPtr;

	IGNORE_RETURN(ms_jvm->DetachCurrentThread());
	ms_env = nullptr;
}	// JavaLibrary::disconnectFromJava

//----------------------------------------------------------------------

/**
 * Resets our connection to the Java VM.
 */
void JavaLibrary::resetJavaConnection()
{
	if (ms_currentRecursionCount == 0)
	{
		LOG("ScriptInvestigation", ("Reconnecting to Java after VM error"));
		ms_instance->disconnectFromJava();
		if (!ms_instance->connectToJava())
		{
			FATAL(true, ("Unable to reconnect to Java after error, fataling"));
		}
		ms_resetJava = false;
		LOG("ScriptInvestigation", ("Reconnection succeeded"));
	}
	else
		ms_resetJava = true;
}	// JavaLibrary::resetJavaConnection()

//----------------------------------------------------------------------

/**
 * Gets a list of public methods of a given script, and adds the info to the
 * GameScriptObject class so we don't have to go into the JVM to see if a
 * script handles a given trigger, etc.
 *
 * @param scriptName		the name of the script to query
 * @return false on failure.  True on success.
 */
bool JavaLibrary::queryScriptFunctions(const std::string & scriptName)
{
	if (ms_instance == nullptr)
		return false;

	JavaString scriptClassName(("script." + scriptName).c_str());
	LocalObjectArrayRefPtr scriptMethods = callStaticObjectArrayMethod(
		ms_clsScriptEntry, ms_midGetScriptFunctions, scriptClassName.getValue());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return false;
	}
	if (scriptMethods == LocalObjectArrayRef::cms_nullPtr)
		return false;

	std::set<std::string> & functionList = GameScriptObject::getScriptFunctions(
		scriptName);
	functionList.clear();

//	DEBUG_REPORT_LOG(true, ("Querying script %s methods:\n", scriptName.c_str()));

	// get the name of each method
	bool result = true;
	int count = getArrayLength(*scriptMethods);
	for (int i = 0; i < count; ++i)
	{
		JavaStringPtr methodName = getStringArrayElement(*scriptMethods, i);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			result = false;
			break;
		}
		if (methodName == JavaString::cms_nullPtr)
		{
			result = false;
			break;
		}

		// convert the method name to a C string and store it
		std::string localName;
		convert(*methodName, localName);
//		DEBUG_REPORT_LOG(true, ("\t%s\n", localName));
		functionList.insert(localName);
	}
	return result;
}	// JavaLibrary::queryScriptFunctions

/**
 * Returns the amount of free Java heap space left.
 *
 * @return the amount of Java memory free
 */
jlong JavaLibrary::getFreeJavaMemory()
{
	if (ms_instance == nullptr || ms_env == nullptr)
		return 0;
	
	return ms_env->CallStaticLongMethod(ms_clsScriptEntry, ms_midScriptEntryGetFreeMem);
}	// JavaLibrary::getFreeJavaMemory

/**
 * Prints the current java stack to stdout.
 */
void JavaLibrary::printJavaStack()
{
	ms_env->CallStaticVoidMethod(ms_clsThread, ms_midThreadDumpStack);
}

/**
 * Turns script logging on or off.
 *
 * @param enable		flag to turn logging on or off
 */
void JavaLibrary::enableLogging(bool enable) const
{
	if (ms_instance == nullptr || ms_env == nullptr)
		return;	

	ms_env->CallStaticVoidMethod(ms_clsScriptEntry,
		ms_midScriptEntryEnableLogging, enable);
}	// JavaLibrary::enableLogging

/**
 * Turns tracking of players to see if they can become Jedi on or off.
 *
 * @param enableTracking	flag to turn tracking on or off
 */
void JavaLibrary::enableNewJediTracking(bool enableTracking)
{
	if (ms_instance == nullptr || ms_env == nullptr)
		return;	

	ms_env->CallStaticVoidMethod(ms_clsScriptEntry,
		ms_midScriptEntryEnableNewJediTracking, enableTracking);
}	// JavaLibrary::enableNewJediTracking

/**
 * Asks the Java script library for a obj_id mapped to a ServerObject.
 *
 * @param object	the NetworkId::NetworkIdType we want to map
 *
 * @return the jobject obj_id
 */
LocalRefPtr JavaLibrary::getObjId(const ServerObject & object)
{
	return getObjId(object.getNetworkId());
}	// JavaLibrary::getObjId(const ServerObject &)

/**
 * Asks the Java script library for a obj_id mapped to a NetworkId::NetworkIdType.
 *
 * @param id		the NetworkId::NetworkIdType we want to map
 *
 * @return the jobject obj_id
 */
LocalRefPtr JavaLibrary::getObjId(const NetworkId::NetworkIdType & id)
{
	return getObjId(NetworkId(id));
}	// JavaLibrary::getObjId(const NetworkId::NetworkIdType &)

/**
 * Asks the Java script library for a obj_id mapped to a NetworkId.
 *
 * @param id		the NetworkId we want to map
 *
 * @return the jobject obj_id
 */
LocalRefPtr JavaLibrary::getObjId(const NetworkId & id)
{
	if (ms_env != nullptr && ms_instance != nullptr)
		return callStaticObjectMethod(ms_clsObjId, ms_midObjIdGetObjId, id.getValue());
	return LocalRef::cms_nullPtr;
}	// JavaLibrary::getObjId(const CachedNetworkId &)

void JavaLibrary::spaceMakeOvert(const NetworkId & player)
{
	callStaticVoidMethod(ms_clsLibrarySpaceTransition, ms_midLibrarySpaceTransitionSetPlayerOvert, player.getValue());
}

void JavaLibrary::spaceClearOvert(const NetworkId & ship)
{
	callStaticVoidMethod(ms_clsLibrarySpaceTransition, ms_midLibrarySpaceTransitionClearOvertStatus, ship.getValue());
}

std::string JavaLibrary::getObjectDumpInfo( NetworkId id )
{
	
	JavaStringPtr s = callStaticStringMethod(ms_clsLibraryDump, ms_midLibraryDumpDumpTargetInfo, id.getValue() );

	std::string retval;
	convert( *s, retval );
	
	return retval;
}

void JavaLibrary::freezePlayer( const NetworkId & id )
{
	DEBUG_REPORT_LOG( true, ( "calling freeze code in script" ) );
	JavaStringPtr s = callStaticStringMethod( ms_clsLibraryGMLib, ms_midLibraryGMLibFreeze, id.getValue() );
	DEBUG_REPORT_LOG( true, ( "done with script" ) );
	std::string retval;
	convert( *s, retval );
	DEBUG_REPORT_LOG( true, ("Got back %s", retval.c_str() ) );
}

void JavaLibrary::unFreezePlayer( const NetworkId & id )
{
	callStaticVoidMethod( ms_clsLibraryGMLib, ms_midLibraryGMLibUnfreeze, id.getValue() );
}

/**
 * Returns the NetworkId associated with a Java obj_id.
 *
 * @param source		the obj_id
 *
 * @return the NetworkId
 */
const NetworkId JavaLibrary::getNetworkId(const jobject & source)
{
	NetworkId result(NetworkId::cms_invalid);
	if (ms_env && source != 0 && ms_env->IsInstanceOf(source, ms_clsObjId))
	{
		result = NetworkId(static_cast<NetworkId::NetworkIdType>(ms_env->CallLongMethod(source, ms_midObjIdGetValue)));
	}
	return result;
}	// getNetworkId(const jobject &)

/**
 * Returns the NetworkId associated with a Java obj_id.
 *
 * @param source		the obj_id
 *
 * @return the NetworkId
 */
const NetworkId JavaLibrary::getNetworkId(const LocalRefParam & source)
{
	NetworkId result(NetworkId::cms_invalid);
	if (ms_env && source.getValue() != 0 && isInstanceOf(source, ms_clsObjId))
	{
		result = NetworkId(static_cast<NetworkId::NetworkIdType>(callLongMethod(source, ms_midObjIdGetValue)));
	}
	return result;
}	// getNetworkId(const LocalRef &)

LocalRefPtr JavaLibrary::getVector(Vector const & vector)
{
	LocalRefPtr temp;
	ScriptConversion::convert(vector, temp);
	return temp;
}

/**
 * Frees a Java obj_id matching a NetwrokId.
 *
 * @param id		the NetworkId we want to remove
 */
void JavaLibrary::removeJavaId(const NetworkId & id)
{
	if (ms_env == nullptr || ms_clsObjId == nullptr || ms_midObjIdClearObjId == nullptr)
	{
		return;
	}

	if (id.getValue())
	{
		ms_env->CallStaticVoidMethod(ms_clsObjId, ms_midObjIdClearObjId, id.getValue());
	}
}	// JavaLibrary::removeJavaId

/**
 * Flag the object in script as going to be destroyed, but hasn't actually been removed from the game yet.
 *
 * @param id		the NetworkId of the object
 */
void JavaLibrary::flagDestroyed(const NetworkId & id)
{
	if (ms_env == nullptr || ms_clsObjId == nullptr || ms_midObjIdFlagDestroyed == nullptr)
	{
		return;
	}

	if (id.getValue())
	{
		ms_env->CallStaticVoidMethod(ms_clsObjId, ms_midObjIdFlagDestroyed, id.getValue());
	}
}	// JavaLibrary::flagDestroyed

/**
 * Sets the authoritative flag of an obj_id.
 *
 * @param object			the object we want to flag
 * @param authoritative		authoritative flag
 */
void JavaLibrary::setObjIdAuthoritative(const NetworkId & object, bool authoritative, uint32 pid)
{
	if (ms_env == nullptr || ms_midObjIdSetAuthoritative == nullptr)
	{
		return;
	}
	LocalRefPtr id = getObjId(object);
	if (id == LocalRef::cms_nullPtr)
	{
		DEBUG_WARNING(true, ("JavaLibrary::setObjIdAuthoritative: could not get id for object %s",
			object.getValueString().c_str()));
		return;
	}
	callVoidMethod(*id, ms_midObjIdSetAuthoritative,
		static_cast<jboolean>(authoritative ? JNI_TRUE : JNI_FALSE), pid);
}	// JavaLibrary::setObjIdAuthoritative

/**
 * Sets the loaded flag of an obj_id. You cannot unload an object, it is marked
 * unloaded when it is deleted.
 *
 * @param object		the object we want to flag as loaded
 */
void JavaLibrary::setObjIdLoaded(const NetworkId &object)
{
	if (ms_env == nullptr || ms_midObjIdSetLoaded == nullptr)
	{
		return;
	}
	LocalRefPtr id = getObjId(object);
	if (id == LocalRef::cms_nullPtr)
	{
		DEBUG_WARNING(true, ("JavaLibrary::setObjIdLoaded: could not get id for object %s",
			object.getValueString().c_str()));
		return;
	}
	callVoidMethod(*id, ms_midObjIdSetLoaded);
}	// JavaLibrary::setObjIdLoaded

/**
 * Sets the initialized flag of an obj_id. You cannot uninitialize an object, 
 * it is marked uninitialized when it is deleted.
 *
 * @param object		the object we want to flag as initialized
 */
void JavaLibrary::setObjIdInitialized(const NetworkId &object)
{
	if (ms_env == nullptr || ms_midObjIdSetInitialized == nullptr)
	{
		return;
	}
	LocalRefPtr id = getObjId(object);
	if (id == LocalRef::cms_nullPtr)
	{
		DEBUG_WARNING(true, ("JavaLibrary::setObjIdInitialized: could not get "
			"id for object %s", object.getValueString().c_str()));
		return;
	}
	callVoidMethod(*id, ms_midObjIdSetInitialized);
}	// JavaLibrary::setObjIdInitialized

/**
 * Sets the logged in flag of an obj_id.
 *
 * @param object		the object we want to flag
 * @param loggedIn		true if logged in, false if logged out
 */
void JavaLibrary::setObjIdLoggedIn(const NetworkId & object, bool loggedIn)
{
	if (ms_env == nullptr || ms_midObjIdSetLoggedIn == nullptr)
	{
		return;
	}
	LocalRefPtr id = getObjId(object);
	if (id == LocalRef::cms_nullPtr)
	{
		DEBUG_WARNING(true, ("JavaLibrary::setObjIdLoggedIn: could not get "
			"id for object %s", object.getValueString().c_str()));
		return;
	}
	callVoidMethod(*id, ms_midObjIdSetLoggedIn, loggedIn);
}	// JavaLibrary::setObjIdLoggedIn

/**
 * Adds a script name to an obj_id.
 *
 * @param object		object we want to attach the script to
 * @param script		name of the script to attach
 */
void JavaLibrary::attachScriptToObjId(const NetworkId &object,
	const std::string & script)
{
	if (ms_env != nullptr)
	{
		LocalRefPtr obj_id = getObjId(object);
		if (obj_id == LocalRef::cms_nullPtr)
		return;

	JavaString jscript(script.c_str());

		callVoidMethod(*obj_id, ms_midObjIdAttachScript, jscript.getValue());
	}
}	// JavaLibrary::attachScriptToObjId

/**
 * Adds script names to an obj_id.
 *
 * @param object		object we want to attach the scripts to
 * @param scripts		names of the scripts to attach
 */
void JavaLibrary::attachScriptsToObjId(const NetworkId &object,
	const ScriptList & scripts)
{
	if (scripts.size() == 0 || ms_env == nullptr)
		return;

	LocalRefPtr obj_id = getObjId(object);
	if (obj_id == LocalRef::cms_nullPtr)
		return;

	LocalObjectArrayRefPtr jscripts = createNewObjectArray(scripts.size(), ms_clsString);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return;
	}
	int index = 0;
	for (ScriptList::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
	{
		if ((*i).isValid())
		{
			JavaString js((*i).getScriptName());
			if (ms_env->ExceptionCheck())
			{
				ms_env->ExceptionDescribe();
				return;
			}
			setObjectArrayElement(*jscripts, index, js);
			if (ms_env->ExceptionCheck())
			{
				ms_env->ExceptionDescribe();
				return;
			}
			++index;
		}
	}

	callVoidMethod(*obj_id, ms_midObjIdAttachScripts, jscripts->getValue());
}	// JavaLibrary::attachScriptsToObjId

/**
 * Removes a script name from an obj_id.
 *
 * @param object		object we want to detach the script from
 * @param script		name of the script to detach
 */
void JavaLibrary::detachScriptFromObjId(const NetworkId &object,
	const std::string & script)
{
	if (ms_env != nullptr)
	{
		LocalRefPtr obj_id = getObjId(object);
		if (obj_id == LocalRef::cms_nullPtr)
		return;

	JavaString jscript(script.c_str());

		callVoidMethod(*obj_id, ms_midObjIdDetachScript, jscript.getValue());
	}
}	// JavaLibrary::detachScriptFromObjId

/**
 * Removes all the scripts from an obj_id.
 *
 * @param object		object we want to detach the script from
 */
void JavaLibrary::detachAllScriptsFromObjId(const NetworkId &object)
{
	if (ms_env != nullptr)
	{
		LocalRefPtr obj_id = getObjId(object);
		if (obj_id == LocalRef::cms_nullPtr)
		return;

		callVoidMethod(*obj_id, ms_midObjIdDetachAllScripts);
	}
}	// JavaLibrary::detachAllScriptsFromObjId

/**
 * Handles errors after calling into script_entry.java.
 *
 * @param result		the result we got back from script_entry
 *
 * @return the result to be returned to the calling trigger
 */
jint JavaLibrary::handleScriptEntryCleanup(jint result)
{
	if (ms_resetJava)
	{
		ms_instance->resetJavaConnection();
		result = SCRIPT_CONTINUE;
	}
	else if (result == SCRIPT_INTERNAL_ERROR || ms_env->ExceptionCheck())
	{
		if (ms_env->ExceptionCheck())
		{
			// if this is an internal error, clear it so we don't print the error
			// message later
			jthrowable err = ms_env->ExceptionOccurred();
			ms_env->ExceptionClear();
			if (ms_env->IsInstanceOf(err, ms_clsError))
			{
				if (ms_env->IsInstanceOf(err, ms_clsInternalScriptError))
				{
					LOG("ScriptInvestigation", ("call to script_entry.java failed because of a script internal error"));
					if (ConfigServerGame::getScriptStackErrorLevel() == 2)
					{
						// if the internal error was caused by a stack overflow, fatal
						LocalRefPtr wrappedErr = getObjectField(LocalRefParam(err), ms_fidInternalScriptSeriousErrorError);
						if (wrappedErr != LocalRef::cms_nullPtr && isInstanceOf(*wrappedErr, ms_clsStackOverflowError))
						{
							FATAL(true, ("Java stack overflow error, fataling"));
						}
					}
				}
				// we are going to reset our connection to the Java VM
				ms_instance->resetJavaConnection();
				result = SCRIPT_CONTINUE;
			}
			else
				ms_env->DeleteLocalRef(err);
		}
		else if (result == SCRIPT_INTERNAL_ERROR)
		{
			LOG("ScriptInvestigation", ("call to script_entry.java failed because of a script internal exception"));
			result = SCRIPT_CONTINUE;
		}
#ifdef linux
		if (ConfigServerGame::getTrapScriptCrashes())
		{
			// the script threw an error or exception, restore our segfault handler
			IGNORE_RETURN(sigaction(SIGSEGV, &OurSa, nullptr));
		}
#endif
	}
	return result;
}	// JavaLibrary::handleScriptEntryCleanup

/**
 * Entry point into the Java VM for scripts. All functions should use this to call
 * into the VM. This version of the function executes on a single script.
 *
 * @param script		the script name to use
 * @param method		the script method to call
 * @param params		parameters to be sent to the script method
 *
 * @return return value from the script method, usually SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
jint JavaLibrary::callScriptEntry(const JavaStringParam & script,
	const JavaStringParam & method, jobjectArray params)
{
	if (ms_env == nullptr || ms_clsScriptEntry == nullptr || ms_midRunOne == nullptr)
	{
		LOG("ScriptInvestigation", ("callScriptEntry failed because something was nullptr"));
		if (!ms_env)
		{
			LOG("ScriptInvestigation", ("callScriptEntry failed because ms_env was nullptr"));
		}
		if (!ms_clsScriptEntry)
		{
			LOG("ScriptInvestigation", ("callScriptEntry failed because scriptEntry was nullptr"));
		}
		if (!ms_midRunOne)
		{
			LOG("ScriptInvestigation", ("callScriptEntry failed because runOne was nullptr"));
		}
		
		return SCRIPT_OVERRIDE;
	}

	PROFILER_BLOCK_DEFINE(p, "JavaLibrary::callScriptEntry1");
	PROFILER_BLOCK_ENTER(p);

	++ms_currentRecursionCount;
	if (ms_currentRecursionCount > 3)
	{
		LOG("ScriptRecursion", ("callScriptEntry1 recursion %d", ms_currentRecursionCount));
	}
  	jint result = ms_env->CallStaticIntMethod(ms_clsScriptEntry, ms_midRunOne,
		script.getValue(), method.getValue(), params);
  	--ms_currentRecursionCount;

	result = handleScriptEntryCleanup(result);

	PROFILER_BLOCK_LOST_CHECK(p);
	PROFILER_BLOCK_LEAVE(p);

	return result;
}	// JavaLibrary::callScriptEntry(JavaStringParam, JavaStringParam, jobjectArray)

/**
 * Entry point into the Java VM for scripts. All functions should use this to call
 * into the VM. This version of the function executes on all scripts attached to
 * an object.
 *
 * @param method		the script method to call
 * @param params		parameters to be sent to the script method
 *
 * @return return value from the script method, usually SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
jint JavaLibrary::callScriptEntry(const JavaStringParam & method, jobjectArray params)
{
	if (ms_env == nullptr || ms_clsScriptEntry == nullptr || ms_midRunAll == nullptr)
	{
		LOG("ScriptInvestigation", ("callScriptEntry2 failed because something was nullptr"));
		if (!ms_env)
		{
			LOG("ScriptInvestigation", ("callScriptEntry2 failed because ms_env was nullptr"));
		}
		if (!ms_clsScriptEntry)
		{
			LOG("ScriptInvestigation", ("callScriptEntry2 failed because scriptEntry was nullptr"));
		}
		if (!ms_midRunOne)
		{
			LOG("ScriptInvestigation", ("callScriptEntry2 failed because runOne was nullptr"));
		}
		return SCRIPT_OVERRIDE;
	}

	PROFILER_BLOCK_DEFINE(p, "JavaLibrary::callScriptEntry2");
	PROFILER_BLOCK_ENTER(p);

	++ms_currentRecursionCount;
	if (ms_currentRecursionCount > 3)
	{
		LOG("ScriptRecursion", ("callScriptEntry2 recursion %d", ms_currentRecursionCount));
	}
  	jint result = ms_env->CallStaticIntMethod(ms_clsScriptEntry, ms_midRunAll,
		method.getValue(), params);
  	--ms_currentRecursionCount;

	result = handleScriptEntryCleanup(result);

	PROFILER_BLOCK_LOST_CHECK(p);
	PROFILER_BLOCK_LEAVE(p);

	return result;
}	// JavaLibrary::callScriptEntry(JavaStringParam, jobjectArray)

/**
 * Entry point into the Java VM for calling console handler script functions.
 *
 * @param script		the script name to use
 * @param method		the script method to call
 * @param params		parameters to be sent to the script method
 *
 * @return string returned by the console handler
 */
jstring JavaLibrary::callScriptConsoleHandlerEntry(const JavaStringParam & script, const JavaStringParam & method, jobjectArray params)
{
	if (ms_env == nullptr || ms_clsScriptEntry == nullptr || ms_midRunConsoleHandler == nullptr)
	{
		LOG("ScriptInvestigation", ("callScriptConsoleHandlerEntry failed because something was nullptr"));
		if (!ms_env)
		{
			LOG("ScriptInvestigation", ("callScriptConsoleHandlerEntry failed because ms_env was nullptr"));
		}
		if (!ms_clsScriptEntry)
		{
			LOG("ScriptInvestigation", ("callScriptConsoleHandlerEntry failed because scriptEntry was nullptr"));
		}
		if (!ms_midRunConsoleHandler)
		{
			LOG("ScriptInvestigation", ("callScriptConsoleHandlerEntry failed because runConsoleHandler was nullptr"));
		}
		
		return 0;
	}

	PROFILER_BLOCK_DEFINE(p, "JavaLibrary::callScriptConsoleHandlerEntry");
	PROFILER_BLOCK_ENTER(p);

	++ms_currentRecursionCount;
	if (ms_currentRecursionCount > 3)
	{
		LOG("ScriptRecursion", ("callScriptConsoleHandlerEntry recursion %d", ms_currentRecursionCount));
	}
  	jstring result = static_cast<jstring>(ms_env->CallStaticObjectMethod(
		ms_clsScriptEntry, ms_midRunConsoleHandler, script.getValue(), method.getValue(), params));
  	--ms_currentRecursionCount;

	IGNORE_RETURN(handleScriptEntryCleanup(SCRIPT_CONTINUE));

	PROFILER_BLOCK_LOST_CHECK(p);
	PROFILER_BLOCK_LEAVE(p);

	return result;
}	// JavaLibrary::callScriptConsoleHandlerEntry

// ----------------------------------------------------------------------

/**
 * Converts ScriptParams into a Java script.dictionary class.
 *
 * @param params		the ScriptParams to convert
 * @param dictionary	pointer that will be set to the dictionary. The caller
 *						is responsible for deleting the pointer.
 */
void JavaLibrary::convert(const ScriptParams & params, JavaDictionaryPtr & dictionary)
{
	dictionary.reset();

	if (ms_env == nullptr)
		return;

	// create the dictionary
	JavaDictionaryPtr localDictionary(createNewDictionary());
	if (localDictionary == JavaDictionary::cms_nullPtr)
		return;

	for (int i = 0; i < params.getParamCount(); ++i)
	{
		JavaString paramName(params.getParamName(i).c_str());
		switch (params.getParamType(i))
		{
			case Param::BOOL:
				{
					callObjectMethod(*localDictionary,
						ms_midDictionaryPutBool, paramName.getValue(),
						params.getBoolParam(i));
				}
				break;
			case Param::BOOL_ARRAY:
				{
					const std::deque<bool> & boolArray = params.getBoolArrayParam(i);
					LocalBooleanArrayRefPtr array = createNewBooleanArray(boolArray.size());
					if (array != LocalBooleanArrayRef::cms_nullPtr)
					{
						if (boolArray.size() > 0)
						{
							// we need the array of bool to be contiguous in order to 
							// "memcopy" it into the Java return buffer; a deque<bool>
							// does not store the array of bool contiguously; neither
							// does a vector<bool>; in fact, if you find an stl container
							// of bool that stores the array of bool contiguously, you
							// should  change this code to use it; for now, we allocate
							// a temporary bool array and copy the deque's bool array
							// into it, and  pass that to the JNI call to do the "memcopy"
							bool * tempBoolArray = new bool[boolArray.size()];
							int indexBoolArray = 0;
							for (std::deque<bool>::const_iterator iter = boolArray.begin(); iter != boolArray.end(); ++iter, ++indexBoolArray)
								tempBoolArray[indexBoolArray] = *iter;

							setBooleanArrayRegion(*array, 0, boolArray.size(), 
								const_cast<jboolean *>(reinterpret_cast<const jboolean *>(&tempBoolArray[0])));

							delete [] tempBoolArray;
						}

						callObjectMethod(*localDictionary,
							ms_midDictionaryPut, paramName.getValue(), array->getValue()
							);
					}
				}
				break;
			case Param::INT:
				{
					callObjectMethod(*localDictionary,
						ms_midDictionaryPutInt, paramName.getValue(),
						params.getIntParam(i));
				}
				break;
			case Param::INT_ARRAY:
				{
					const std::vector<int> & intArray = params.getIntArrayParam(i);
					LocalIntArrayRefPtr array = createNewIntArray(intArray.size());
					if (array != LocalIntArrayRef::cms_nullPtr)
					{
						if (intArray.size() > 0)
						{
#if INT_MAX == LONG_MAX
							setIntArrayRegion(*array, 0, intArray.size(), const_cast<jint *>(
								reinterpret_cast<const jint *>(&intArray[0])));
#else
							std::vector<jint> intArray2(intArray.begin(), intArray.end());
							setIntArrayRegion(*array, 0, intArray.size(), const_cast<jint *>(&intArray2[0]));
#endif
						}

						callObjectMethod(*localDictionary,
							ms_midDictionaryPut, paramName.getValue(), array->getValue()
							);
					}
				}
				break;
			case Param::FLOAT:
				{
					callObjectMethod(*localDictionary,
						ms_midDictionaryPutFloat, paramName.getValue(),
						params.getFloatParam(i));
				}
				break;
			case Param::STRING:
				{
					JavaString value(params.getStringParam(i));
					callObjectMethod(*localDictionary,
						ms_midDictionaryPut, paramName.getValue(), value.getValue());
				}
				break;
			case Param::STRING_ARRAY:
				{
					LocalObjectArrayRefPtr value;
					ScriptConversion::convert(params.getStringArrayParam(i), value);
					callObjectMethod(*localDictionary,
						ms_midDictionaryPut, paramName.getValue(), value->getValue());
				}
				break;
			case Param::UNICODE:
				{
					JavaString value(params.getUnicodeParam(i));
					callObjectMethod(*localDictionary,
						ms_midDictionaryPut, paramName.getValue(), value.getValue());
				}
				break;
			case Param::UNICODE_ARRAY:
				{
					LocalObjectArrayRefPtr value;
					ScriptConversion::convert(params.getUnicodeArrayParam(i), value);
					callObjectMethod(*localDictionary,
						ms_midDictionaryPut, paramName.getValue(), value->getValue());
				}
				break;
			case Param::OBJECT_ID:
				{
					LocalRefPtr arg = getObjId(params.getObjIdParam(i));
					if (arg == LocalRef::cms_nullPtr)
						return;
					callObjectMethod(*localDictionary,
						ms_midDictionaryPut, paramName.getValue(), arg->getValue());
				}
				break;
			case Param::OBJECT_ID_ARRAY:
				{
					const std::vector<NetworkId> & objIds =
						params.getObjIdArrayParam(i);
					LocalObjectArrayRefPtr array = createNewObjectArray(objIds.size(), ms_clsObjId);
					if (array != LocalObjectArrayRef::cms_nullPtr)
					{
						int count = objIds.size();
						for (int j = 0; j < count; ++j)
						{
							LocalRefPtr id = getObjId(objIds[j]);
							if (id != LocalRef::cms_nullPtr)
							{
								setObjectArrayElement(*array, j, *id);
							}
						}
						callObjectMethod(*localDictionary,
							ms_midDictionaryPut, paramName.getValue(), array->getValue());
					}
					else
					{
						if (ms_env->ExceptionCheck())
							ms_env->ExceptionDescribe();
						return;
					}
				}
				break;
			case Param::CACHED_OBJECT_ID_ARRAY:
				{
					const std::vector<CachedNetworkId> & objIds =
						params.getCachedObjIdArrayParam(i);
					LocalObjectArrayRefPtr array = createNewObjectArray(objIds.size(), ms_clsObjId);
					if (array != LocalObjectArrayRef::cms_nullPtr)
					{
						int count = objIds.size();
						for (int j = 0; j < count; ++j)
						{
							LocalRefPtr id = getObjId(objIds[j]);
							if (id != LocalRef::cms_nullPtr)
							{
								setObjectArrayElement(*array, j, *id);
							}
						}
						callObjectMethod(*localDictionary,
							ms_midDictionaryPut, paramName.getValue(), array->getValue());
					}
					else
					{
						if (ms_env->ExceptionCheck())
							ms_env->ExceptionDescribe();
						return;
					}
				}
				break;
			case Param::OBJECT_ID_ARRAY_ARRAY:
				{
					const std::vector<const std::vector<NetworkId> *> & objIds =
						params.getObjIdArrayArrayParam(i);
					LocalObjectArrayRefPtr array = createNewObjectArray(objIds.size(), ms_clsObjIdArray);
					if (array != LocalObjectArrayRef::cms_nullPtr)
					{
						int count = objIds.size();
						for (int j = 0; j < count; ++j)
						{
							const std::vector<NetworkId> * inner = objIds[j];
							if (inner != nullptr)
							{
								LocalObjectArrayRefPtr innerArray = createNewObjectArray(inner->size(), ms_clsObjId);
								if (innerArray != LocalObjectArrayRef::cms_nullPtr)
								{
									int innerCount = inner->size();
									for (int k = 0; k < innerCount; ++k)
									{
										LocalRefPtr id = getObjId(inner->at(k));
										if (id != LocalRef::cms_nullPtr)
										{
											setObjectArrayElement(*innerArray, k, *id);
										}
									}
									setObjectArrayElement(*array, j, *innerArray);
								}
								else
								{
									if (ms_env->ExceptionCheck())
										ms_env->ExceptionDescribe();
									return;
								}
							}
							else
							{
								setObjectArrayElement(*array, j, *LocalRef::cms_nullPtr);
						}
						}
						callObjectMethod(*localDictionary,
							ms_midDictionaryPut, paramName.getValue(), array->getValue());
					}
					else
					{
						if (ms_env->ExceptionCheck())
							ms_env->ExceptionDescribe();
						return;
					}
				}
				break;
			case Param::LOCATION:
				{
					LocalRefPtr location;
					if (ScriptConversion::convert(params.getLocationParam(i), "", NetworkId::cms_invalid, location))
					{
						callObjectMethod(*localDictionary,
							ms_midDictionaryPut, paramName.getValue(), location->getValue());
					}
				}
				break;
			case Param::LOCATION_ARRAY:
				{
					LocalObjectArrayRefPtr locations;
					if (ScriptConversion::convert(params.getLocationArrayParam(i), locations))
					{
						callObjectMethod(*localDictionary,
							ms_midDictionaryPut, paramName.getValue(), locations->getValue());
					}
				}
				break;
			case Param::DICTIONARY:
				{
					LocalRefPtr dictionaryArg = convert(params.getValueDictionaryParam(i));
					if (dictionaryArg != LocalRef::cms_nullPtr)
					{
						callObjectMethod(*localDictionary,
							ms_midDictionaryPut, paramName.getValue(), dictionaryArg->getValue());
					}
					else
					{
						if (ms_env->ExceptionCheck())
							ms_env->ExceptionDescribe();
						return;
					}		
				}
				break;
			default:
				{
					DEBUG_REPORT_LOG(true, ("Unknown/unhandled parameter type "
						"%d while parsing ScriptParameters\n", static_cast<int>(
						params.getParamType(i))));
					return;
				}
				break;
		}
	}

	dictionary = localDictionary;
}	// JavaLibrary::convert(const ScriptParams &, ScriptDictionaryPtr &)

// ----------------------------------------------------------------------

/**
 * Converts ScriptParams to an array of Java objects. The first parameter of the
 * objects will be the NetworkId of the object we are running scripts on ("self").
 *
 * @param self		the network id that scripts are attached to
 * @param argList	argument list description
 * @param args		the parameters we want to convert
 *
 * @return self and args converted to Java data, or 0 on error
 */
LocalObjectArrayRefPtr JavaLibrary::convert(const NetworkId & self, const std::string& argList, const ScriptParams &args)
{
	if (ms_env == nullptr)
		return LocalObjectArrayRef::cms_nullPtr;

	// convert the params to jobjects
	LocalObjectArrayRefPtr jparams = createNewObjectArray(static_cast<jsize>(args.getParamCount() + 1), ms_clsObject);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return LocalObjectArrayRef::cms_nullPtr;
	}
	// add the "self" parameter
	{
		LocalRefPtr jself = getObjId(self);
		setObjectArrayElement(*jparams, 0, *jself);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return LocalObjectArrayRef::cms_nullPtr;
		}
	}

	if (convert(jparams, 1, argList, args))
		return jparams;
	
	return LocalObjectArrayRef::cms_nullPtr;
}	// JavaLibrary::convert

/**
 * Converts ScriptParams to an array of Java objects.
 *
 * @param argList	argument list description
 * @param args		the parameters we want to convert
 *
 * @return args converted to Java data, or 0 on error
 */
LocalObjectArrayRefPtr JavaLibrary::convert(const std::string& argList, const ScriptParams &args)
{
	if (ms_env == nullptr)
		return LocalObjectArrayRef::cms_nullPtr;

	// convert the params to jobjects
	LocalObjectArrayRefPtr jparams = createNewObjectArray(static_cast<jsize>(
		args.getParamCount()), ms_clsObject);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return LocalObjectArrayRef::cms_nullPtr;
	}

	if (convert(jparams, 0, argList, args))
		return jparams;
	return LocalObjectArrayRef::cms_nullPtr;
}	// JavaLibrary::convert

/**
 * Converts ScriptParams to an array of Java objects.
 *
 * @param javaParams	Java object array to be filled in with the params
 * @param startIndex	index to start inserting params into the javaParams
 * @param argList		argument list description
 * @param args			the parameters we want to convert
 *
 * @return true on success, false if there was an error
 */
bool JavaLibrary::convert(LocalObjectArrayRefPtr & javaParams, int startIndex, const std::string& argList, const ScriptParams &args)
{
	if (ms_env == nullptr)
		return 0;

	GlobalInstances globals;

	PROFILER_AUTO_BLOCK_CHECK_DEFINE("JavaLibrary::convert(ScriptParams)");

	// add the rest of the parameters
	jsize paramIndex = 0;
	for (unsigned i = 0; i < argList.size(); ++paramIndex)
	{
		const char argType = argList[i];
		int dimensions = 0;
		int modifiable = false;
		while (argList[i])
		{
			++i;
			if (argList[i] == '*')
				modifiable = true;
			else if (argList[i] != '[')
				break;
			else
				 ++dimensions;
		}

		if (paramIndex >= args.getParamCount())
		{
			REPORT_LOG(true, ("JavaLibrary::convert(argList, ScriptParams) called with too few parameters: expected %s, got %d\n", argList.c_str(), args.getParamCount()));
			return 0;
		}
		LocalRefParamPtr arg = LocalRefParam::cms_nullPtr;
		switch (argType)
		{
			case 'b':
				if (dimensions == 0)
				{
					if (args.getParamType(paramIndex) != Param::BOOL)
						break;
					jboolean param = args.getBoolParam(paramIndex);
					arg = createNewObject(ms_clsBoolean, ms_midBoolean, param);
				}
				else
				{
					if (args.getParamType(paramIndex) != Param::BOOL_ARRAY)
						break;
					const std::deque<bool> & param = args.getBoolArrayParam(paramIndex);
					int count = param.size();
					LocalBooleanArrayRefPtr localInstance = createNewBooleanArray(count);

					if (count > 0)
					{
						// we need the array of bool to be contiguous in order to 
						// "memcopy" it into the Java return buffer; a deque<bool>
						// does not store the array of bool contiguously; neither
						// does a vector<bool>; in fact, if you find an stl container
						// of bool that stores the array of bool contiguously, you
						// should  change this code to use it; for now, we allocate
						// a temporary bool array and copy the deque's bool array
						// into it, and  pass that to the JNI call to do the "memcopy"
						bool * tempBoolArray = new bool[count];
						int indexBoolArray = 0;
						for (std::deque<bool>::const_iterator iter = param.begin(); iter != param.end(); ++iter, ++indexBoolArray)
							tempBoolArray[indexBoolArray] = *iter;

						setBooleanArrayRegion(*localInstance, 0, count,
							const_cast<jboolean *>(reinterpret_cast<const jboolean *>(&tempBoolArray[0])));

						delete [] tempBoolArray;
					}

					if (ms_env->ExceptionCheck())
					{
						ms_env->ExceptionDescribe();
						return 0;
					}
					arg = localInstance;
				}
				break;
			case 'i':
				if (dimensions == 0)
				{
					if (args.getParamType(paramIndex) != Param::INT)
						break;
					if (modifiable)
					{
						arg = globals.getNextModifiableInt();
						if (arg == GlobalRef::cms_nullPtr)
							break;
						jint param = args.getIntParam(paramIndex);
						setIntField(*arg, ms_fidModifiableIntData, param);
					}
					else
					{
						jint param = args.getIntParam(paramIndex);
						arg = createNewObject(ms_clsInteger, ms_midInteger, param);
					}
				}
				else
				{
					if (args.getParamType(paramIndex) != Param::INT_ARRAY)
						break;
					const std::vector<int> & param = args.getIntArrayParam(paramIndex);
					int count = param.size();
					LocalIntArrayRefPtr localInstance = createNewIntArray(count);
					jint * destination = ms_env->GetIntArrayElements(localInstance->getValue(), 0);
					if (destination)
					{
						int i;
						for (i=0; i<count; ++i)
						{
							destination[i] = param[i];
						}
						ms_env->ReleaseIntArrayElements(localInstance->getValue(), destination, 0);
					}
					if (ms_env->ExceptionCheck())
					{
						ms_env->ExceptionDescribe();
						return 0;
					}
					arg = localInstance;
				}
				break;
			case 'U':
				{
					if (dimensions != 0)
					{
						if (args.getParamType(paramIndex) != Param::BYTE_ARRAY)
							break;
						const std::vector<unsigned char> & param = args.getByteArrayParam(paramIndex);
						int count = param.size();
						LocalByteArrayRefPtr localInstance = createNewByteArray(count);
						jbyte * destination = ms_env->GetByteArrayElements(localInstance->getValue(), 0);
						if (destination)
						{
							int i;
							for (i=0; i<count; ++i)
							{
								destination[i] = param[i];
							}
							ms_env->ReleaseByteArrayElements(localInstance->getValue(), destination, 0);
						}
						if (ms_env->ExceptionCheck())
						{
							ms_env->ExceptionDescribe();
							return 0;
						}
						arg = localInstance;
					}
				}
				break;
			case 'f':
				if (dimensions == 0)
				{
					if (args.getParamType(paramIndex) != Param::FLOAT)
						break;
					if (modifiable)
					{
						arg = globals.getNextModifiableFloat();
						if (arg == GlobalRef::cms_nullPtr)
							break;
						jfloat param = args.getFloatParam(paramIndex);
						setFloatField(*arg, ms_fidModifiableFloatData, param);
					}
					else
					{
						jfloat param = args.getFloatParam(paramIndex);
						arg = createNewObject(ms_clsFloat, ms_midFloat, param);
					}
				}
				else
				{
					if (args.getParamType(paramIndex) != Param::FLOAT_ARRAY)
						break;
					const std::vector<float> & floats = args.getFloatArrayParam(paramIndex);
					LocalFloatArrayRefPtr farray = createNewFloatArray(floats.size());
					if (farray != LocalFloatArrayRef::cms_nullPtr)
					{
						if (floats.size() > 0)
							setFloatArrayRegion(*farray, 0, floats.size(), const_cast<float *>(&floats[0]));
						arg = farray;
					}
				}
				break;
			case 's':
				if (dimensions == 0)
				{
					if (args.getParamType(paramIndex) != Param::STRING)
						break;
					arg = createNewString(args.getStringParam(paramIndex));
				}
				else
				{
					if (args.getParamType(paramIndex) != Param::STRING_ARRAY)
						break;

					const std::vector<const char *> & strings =
						args.getStringArrayParam(paramIndex);
					LocalObjectArrayRefPtr localInstance = createNewObjectArray(strings.size(), ms_clsString);
					int i;
					std::vector<const char *>::const_iterator iter;
					for (i = 0, iter = strings.begin(); iter != strings.end();
						++i, ++iter)
					{
						if (*iter)
						{
							JavaString newString(*iter);
							setObjectArrayElement(*localInstance, i, newString);
						}
					}
					arg = localInstance;
				}
				break;
			case 'u':
				if (dimensions == 0)
				{
					if (args.getParamType(paramIndex) != Param::UNICODE)
						break;
					if (modifiable)
						break;
					const String_t &param = args.getUnicodeParam(paramIndex);
					arg = createNewString(param.c_str(), static_cast<jsize>(param.size()));
				}
				else
				{
					if (args.getParamType(paramIndex) != Param::UNICODE_ARRAY)
						break;

					const std::vector<const Unicode::String *> & strings =
						args.getUnicodeArrayParam(paramIndex);
					LocalObjectArrayRefPtr localInstance = createNewObjectArray(strings.size(), ms_clsString);
					int i;
					std::vector<const Unicode::String *>::const_iterator iter;
					for (i = 0, iter = strings.begin(); iter != strings.end();
						++i, ++iter)
					{
						if (*iter != nullptr)
						{
							JavaString newString(**iter);
							setObjectArrayElement(*localInstance, i, newString);
						}
					}
					arg = localInstance;
				}
				break;
			case 'L':
				if (dimensions == 0)
				{
					if (args.getParamType(paramIndex) != Param::LOCATION)
						break;
					NetworkId cell;
					LocalRefPtr target;
					if (!ScriptConversion::convert(args.getLocationParam(paramIndex), cell, target))
						break;
					arg = target;
				}
				else
				{
					if (args.getParamType(paramIndex) != Param::LOCATION_ARRAY)
						break;
					const std::vector<const Vector *> & locations = args.getLocationArrayParam(paramIndex);
					LocalObjectArrayRefPtr target;
					if (!ScriptConversion::convert(locations, target))
						break;
					arg = target;
				}
				break;
			case 'O':
			{
					if (modifiable || dimensions > 1)
						 break;
					if (dimensions == 1)
					{
						 // obj_id array
					if (args.getParamType(paramIndex) == Param::OBJECT_ID_ARRAY)
					{
						const std::vector<NetworkId> & param = args.getObjIdArrayParam(paramIndex);
						 int count = param.size();
						 int paramCount = 0;
						LocalObjectArrayRefPtr localInstance = createNewObjectArray(count, ms_clsObjId);
						 for (int j = 0; j < count; ++j)
						 {
							LocalRefPtr id = getObjId(param[j]);
							if (id == LocalRef::cms_nullPtr)
								break;
							setObjectArrayElement(*localInstance, static_cast<jsize>(paramCount), *id);
							if (ms_env->ExceptionCheck())
							{	
								ms_env->ExceptionDescribe();
								return 0;
							}
							++paramCount;
						}
						arg = localInstance;
							}
					else if (args.getParamType(paramIndex) == Param::CACHED_OBJECT_ID_ARRAY)
					{
						const std::vector<CachedNetworkId> & param = args.getCachedObjIdArrayParam(paramIndex);
						int count = param.size();
						int paramCount = 0;
						LocalObjectArrayRefPtr localInstance = createNewObjectArray(count, ms_clsObjId);
						for (int j = 0; j < count; ++j)
							{
							LocalRefPtr id = getObjId(param[j]);
							if (id == LocalRef::cms_nullPtr)
									 break;
							setObjectArrayElement(*localInstance, static_cast<jsize>(paramCount), *id);
							if (ms_env->ExceptionCheck())
							{
								ms_env->ExceptionDescribe();
								return 0;
							}
							++paramCount;
						 }
						 arg = localInstance;
					}
					}
					else
					{
						if (args.getParamType(paramIndex) != Param::OBJECT_ID)
							break;
					arg = getObjId(args.getObjIdParam(paramIndex));
					}
			}
			break;

			//----------------------------------------------------------------------
			//-- menu info

			case 'm':
				{
					if (dimensions != 0)
						break;

					if (args.getParamType(paramIndex) != Param::OBJECT_MENU_INFO)
						break;

					const MenuDataVector & menuVector = args.getObjectMenuRequestDataArrayParam (paramIndex);

					arg = globals.getMenuInfo ();

					if (!convert (menuVector, arg))
					{
						return 0;
					}
				}
				break;

			//----------------------------------------------------------------------

			 case 'S':
				if (dimensions == 0)
				{
					if (args.getParamType(paramIndex) != Param::STRING_ID)
						break;
					if (modifiable)
						 arg = globals.getNextModifiableStringId();
					else
						 arg = globals.getNextStringId();
					if (arg == GlobalArrayRef::cms_nullPtr)
						break;
					const StringId & param = args.getStringIdParam(paramIndex);
					JavaString text(param.getTable().c_str());
					setObjectField(*arg, ms_fidStringIdTable, text);
					if (param.getText().empty())
					{
						setIntField(*arg, ms_fidStringIdIndexId,
							static_cast<jint>(param.getTextIndex()));
						JavaString text("");
						setObjectField(*arg, ms_fidStringIdAsciiId, text);
					}
					else
					{
						JavaString text(param.getText().c_str());
						setObjectField(*arg, ms_fidStringIdAsciiId, text);
					}
				}
				else
				{
					if (args.getParamType(paramIndex) != Param::STRING_ID_ARRAY)
						break;
					if (modifiable)
						break;

					const std::vector<const StringId *> & strings = args.getStringIdArrayParam(paramIndex);
					LocalObjectArrayRefPtr localInstance = createNewObjectArray(strings.size(), ms_clsStringId);

					int i;
					std::vector<const StringId *>::const_iterator iter;
					for (i = 0, iter = strings.begin(); iter != strings.end(); ++i, ++iter)
					{
						const StringId * param = *iter;
						// get the next available stringId from the pool
						GlobalRefPtr stringIdObject = globals.getNextStringId();
						if (stringIdObject == GlobalRef::cms_nullPtr)
							break;
						// set the stringId elements
						JavaString text(param->getTable().c_str());
						setObjectField(*stringIdObject, ms_fidStringIdTable, text);
						if (param->getText().empty())
						{
							setIntField(*stringIdObject, ms_fidStringIdIndexId,
								static_cast<jint>(param->getTextIndex()));
							JavaString text("");
							setObjectField(*stringIdObject, ms_fidStringIdAsciiId, text);
						}
						else
						{
							JavaString text(param->getText().c_str());
							setObjectField(*stringIdObject, ms_fidStringIdAsciiId, text);
						}
						// add the string id to the array
						setObjectArrayElement(*localInstance, i, *stringIdObject);
					}
					arg = localInstance;
				}
				break;
			case 'E':
				//@todo: implement this
				break;
			case 'A':
				 if (modifiable || dimensions > 1)
						break;
				if (dimensions == 1)
				{
					// attribMod array
					if (args.getParamType(paramIndex) != Param::ATTRIB_MOD_ARRAY)
						break;
					const std::vector<AttribMod::AttribMod> & param = args.getAttribModArrayParam(paramIndex);
					int count = param.size();
					int paramCount = 0;
					for (int j = 0; j < count; ++j)
					{
						// only send attribMods that actually do something
						if (param[j].value != 0)
						{
							arg = globals.getNextAttribMod();
							if (arg == GlobalRef::cms_nullPtr)
								return 0;

							JavaString name(AttribModNameManager::getInstance(
								).getAttribModName(param[j].tag));
							setObjectField(*arg, ms_fidAttribModName, name);
							if (!AttribMod::isSkillMod(param[j]))
							{
								setIntField(*arg, ms_fidAttribModType, static_cast<jint>(param[j].attrib));
							}
							else
							{
								JavaString skill(AttribModNameManager::getInstance(
									).getAttribModName(param[j].skill));
								setObjectField(*arg, ms_fidAttribModSkill, skill);
							}
							setIntField(*arg, ms_fidAttribModValue, param[j].value);
							setFloatField(*arg, ms_fidAttribModTime, param[j].sustain);
							setFloatField(*arg, ms_fidAttribModAttack, param[j].attack);
							setFloatField(*arg, ms_fidAttribModDecay, param[j].decay);
							setIntField(*arg, ms_fidAttribModFlags, param[j].flags);
							setObjectArrayElement(*ms_attribModList[ms_currentRecursionCount], static_cast<jsize>(paramCount), *arg);
							if (ms_env->ExceptionCheck())
							{
								ms_env->ExceptionDescribe();
								return 0;
							}
							++paramCount;
						}
					}
					// fill in the rest of ms_attribModList with nullptr
					for (; paramCount < MAX_ATTRIB_MOD_PARAMS; ++paramCount)
					{
						setObjectArrayElement(*ms_attribModList[ms_currentRecursionCount], static_cast<jsize>(paramCount), *LocalRefParam::cms_nullPtr);
						if (ms_env->ExceptionCheck())
						{
							ms_env->ExceptionDescribe();
							return 0;
						}
					}
					arg = ms_attribModList[ms_currentRecursionCount];
				}
				else
				{
					// single attribMod
					if (args.getParamType(paramIndex) != Param::ATTRIB_MOD)
						break;
					const AttribMod::AttribMod &param = args.getAttribModParam(paramIndex);
					arg = globals.getNextAttribMod();
					if (arg == GlobalRef::cms_nullPtr)
						return 0;

					JavaString name(AttribModNameManager::getInstance(
						).getAttribModName(param.tag));
					setObjectField(*arg, ms_fidAttribModName, name);
					if (!AttribMod::isSkillMod(param))
					{
						setIntField(*arg, ms_fidAttribModType, param.attrib);
					}
					else
					{
						JavaString skill(AttribModNameManager::getInstance(
							).getAttribModName(param.skill));
						setObjectField(*arg, ms_fidAttribModSkill, skill);
					}
					setIntField(*arg, ms_fidAttribModValue, param.value);
					setFloatField(*arg, ms_fidAttribModTime, param.sustain);
					setFloatField(*arg, ms_fidAttribModAttack, param.attack);
					setFloatField(*arg, ms_fidAttribModDecay, param.decay);
					setIntField(*arg, ms_fidAttribModFlags, param.flags);
				}
				break;

			case 'M':
				if (modifiable || dimensions > 1)
					break;
				if (dimensions == 1)
				{
					// mental_state_mod array
					if (args.getParamType(paramIndex) != Param::MENTAL_STATE_MOD_ARRAY)
						break;
					const std::vector<ServerObjectTemplate::MentalStateMod> & param = args.getMentalStateModArrayParam(paramIndex);
					int count = param.size();
					int paramCount = 0;
					for (int j = 0; j < count; ++j)
					{
						// only send mental_state_mods that actually do something
						if (param[j].value != 0)
						{
							arg = globals.getNextMentalStateMod();
							if (arg == GlobalRef::cms_nullPtr)
								return 0;

							setIntField(*arg, ms_fidMentalStateModType, param[j].target);
							setFloatField(*arg, ms_fidMentalStateModValue, param[j].value);
							setFloatField(*arg, ms_fidMentalStateModTime, param[j].timeAtValue);
							setFloatField(*arg, ms_fidMentalStateModAttack, param[j].time);
							setFloatField(*arg, ms_fidMentalStateModDecay, param[j].decay);
							setObjectArrayElement(
								*ms_mentalStateModList[ms_currentRecursionCount],
								static_cast<jsize>(paramCount), *arg);
							if (ms_env->ExceptionCheck())
							{
								ms_env->ExceptionDescribe();
								return 0;
							}
							++paramCount;
						}
					}
					// fill in the rest of ms_mentalStateModList with nullptr
					for (; paramCount < MAX_MENTAL_STATE_MOD_PARAMS; ++paramCount)
					{
						setObjectArrayElement(
							*ms_mentalStateModList[ms_currentRecursionCount],
							static_cast<jsize>(paramCount), *LocalRefParam::cms_nullPtr);
						if (ms_env->ExceptionCheck())
						{
							ms_env->ExceptionDescribe();
							return 0;
						}
					}
					arg = ms_mentalStateModList[ms_currentRecursionCount];
				}
				else
				{
					// single mental_state_mod
					if (args.getParamType(paramIndex) != Param::MENTAL_STATE_MOD)
						break;
					const ServerObjectTemplate::MentalStateMod &param = args.getMentalStateModParam(paramIndex);
					arg = globals.getNextMentalStateMod();
					if (arg == GlobalRef::cms_nullPtr)
						return 0;

					setIntField(*arg, ms_fidMentalStateModType, param.target);
					setFloatField(*arg, ms_fidMentalStateModValue, param.value);
					setFloatField(*arg, ms_fidMentalStateModTime, param.timeAtValue);
					setFloatField(*arg, ms_fidMentalStateModAttack, param.time);
					setFloatField(*arg, ms_fidMentalStateModDecay, param.decay);
				}
				break;

			case 'D':
				if (!modifiable && dimensions == 0)
				{
					const ManufactureObjectInterface & schematic = args.getManufactureSchematicParam(paramIndex);
					arg = convert(schematic);
				}
				break;

			case 'I':
				if (!modifiable && dimensions == 0)
				{
					const Crafting::IngredientSlot & slot = args.getIngredientSlotParam(paramIndex);
					const ManufactureObjectInterface & schematic = args.getIngredientSlotParamSchematic(paramIndex);
					int amountRequired = args.getIngredientSlotParamAmountRequired(paramIndex);
					const std::string & appearance = args.getIngredientSlotParamAppearance(paramIndex);
					arg = convert(schematic, slot, amountRequired, appearance, "");
				}
				break;

			case 'V':
				if (!modifiable)
				{
					if (dimensions == 0)
					{
						const ValueDictionary & dictionary = args.getValueDictionaryParam(paramIndex);
						arg = convert(dictionary);
					}
					else if (dimensions == 1)
					{
						const std::vector<ValueDictionary> & param = args.getValueDictionaryArrayParam(paramIndex);
						LocalObjectArrayRefPtr localInstance = createNewObjectArray(param.size(), ms_clsDictionary);

						int paramCount = 0;
						std::vector<ValueDictionary>::const_iterator iter;
						for (iter = param.begin(); iter != param.end(); ++iter)
						{
							setObjectArrayElement(*localInstance, static_cast<jsize>(paramCount++), *convert(*iter));

							if (ms_env->ExceptionCheck())
							{
								ms_env->ExceptionDescribe();
								return 0;
							}
						}
						arg = localInstance;
					}
				}
				break;

			default:
				DEBUG_REPORT_LOG(true, ("unknown parameter type %c(%#x)\n", argType,
					static_cast<unsigned>(argType)));		//lint !e571 suspicious cast
				return 0;
		}
		if (arg.get() == nullptr || arg == LocalRef::cms_nullPtr)
		{
			DEBUG_REPORT_LOG(true, ("bad parameter, %c%s%d%s\n", argType,
							modifiable ? "*" : "",
							dimensions,
							dimensions > 0 ? " dimensions" : ""
				));
			return 0;
		}
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return 0;
		}
		setObjectArrayElement(*javaParams, static_cast<jsize>(paramIndex + startIndex), *arg);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			return 0;
		}
	}
	return true;
}	// JavaLibrary::convert

/**
 * Copies any modifiable params from Java to C. This is called after a script
 * has run.
 *
 * @param jparams		Java parameters
 * @param argList		argument list description
 * @param args			arguments that were passed to the script
 */
void JavaLibrary::alterScriptParams(jobjectArray jparams, const std::string& argList,
	ScriptParams &args)
{
	if (ms_env == nullptr)
		return;

	PROFILER_AUTO_BLOCK_CHECK_DEFINE("JavaLibrary::alterScriptParams");

	int paramIndex = 0;
	for (unsigned i = 0; i < argList.size(); ++i, ++paramIndex)
	{
		switch (argList[i])
		{
			case 'i':
				if (i < argList.size() - 1 && argList[i+1] == '*')
				{
					++i;
					if (i == argList.size() || argList[i+1] != '[')
					{
						LocalRefPtr arg = getObjectArrayElement(LocalObjectArrayRefParam(jparams), paramIndex + 1);
						if (arg != LocalRef::cms_nullPtr)
						{
							int value = getIntField(*arg, ms_fidModifiableIntData);
							args.changeParam(paramIndex, value);
						}
						else
						{
							WARNING_STRICT_FATAL(true, ("Error getting back integer param on script return paramIndex=%d, i=%d", paramIndex, i));
						}
					}
					else if (i < argList.size() - 1 && argList[i+1] == '[')
					{
						++i;

						LocalArrayRefPtr arg = getArrayArrayElement(LocalObjectArrayRefParam(jparams), paramIndex + 1);
						if (arg != LocalArrayRef::cms_nullPtr)
						{
							int count = getArrayLength(*arg);
							std::vector<int> * values = new std::vector<int>(count, 0);
							jint * jvalues = static_cast<jint *>(ms_env->GetPrimitiveArrayCritical(static_cast<jintArray>(arg->getValue()), nullptr));
							for (int j = 0; j < count; ++j)
							{
								values->at(j) = static_cast<int>(jvalues[j]);
							}
							ms_env->ReleasePrimitiveArrayCritical(static_cast<jintArray>(arg->getValue()), jvalues, JNI_ABORT);
							args.changeParam(paramIndex, *values, true);
						}
						else
						{
							WARNING_STRICT_FATAL(true, ("Error getting back integer array param on script return"));
						}
					}
				}
				break;
			case 'U':
				if (i < argList.size() - 1 && argList[i+1] == '*')
				{
					++i;
					if (i == argList.size() || argList[i+1] != '[')
					{
						WARNING_STRICT_FATAL(true, ("Error getting back vector<unsigned char> param on script return paramIndex=%d, i=%d", paramIndex, i));
					}
					else if (i < argList.size() - 1 && argList[i+1] == '[')
					{
						++i;

						LocalArrayRefPtr arg = getArrayArrayElement(LocalObjectArrayRefParam(jparams), paramIndex + 1);
						if (arg != LocalArrayRef::cms_nullPtr)
						{
							int count = getArrayLength(*arg);
							std::vector<unsigned char> * values = new std::vector<unsigned char>(count, 0);
							jbyte * jvalues = static_cast<jbyte *>(ms_env->GetPrimitiveArrayCritical(static_cast<jbyteArray>(arg->getValue()), nullptr));
							for (int j = 0; j < count; ++j)
							{
								values->at(j) = static_cast<unsigned char>(jvalues[j]);
							}
							ms_env->ReleasePrimitiveArrayCritical(static_cast<jbyteArray>(arg->getValue()), jvalues, JNI_ABORT);
							args.changeParam(paramIndex, *values, true);
						}
						else
						{
							WARNING_STRICT_FATAL(true, ("Error getting back integer array param on script return"));
						}
					}
				}
				break;
				
			case 'f':
				if (i < argList.size() - 1 && argList[i+1] == '*')
				{
					++i;
					LocalRefPtr arg = getObjectArrayElement(LocalObjectArrayRefParam(jparams), paramIndex + 1);
					if (arg != LocalRef::cms_nullPtr)
					{
						args.changeParam(paramIndex, getFloatField(*arg, ms_fidModifiableFloatData));
					}
					else
					{
						WARNING_STRICT_FATAL(true, ("Error getting back float param on script return"));
					}
				}
				break;
			case 's':
				if (i < argList.size() - 1 && argList[i+1] == '*')
				{
					++i;
					if (i < argList.size() - 1 && argList[i+1] == '[')
					{
						++i;
						LocalObjectArrayRefPtr arg = getObjectArrayArrayElement(LocalObjectArrayRefParam(jparams), paramIndex + 1);
						if (arg != LocalObjectArrayRef::cms_nullPtr)
						{
							std::vector<const char *> * strings = new std::vector<const char *>();
							getStringArray(*arg, *strings);
							args.changeParam(paramIndex, *strings, true);
						}
						else
						{
							WARNING_STRICT_FATAL(true, ("Error getting back string array param on script return"));
						}
					}
				}
				break;
			case 'u':
				if (i < argList.size() - 1 && argList[i+1] == '*')
				{
					++i;
					if (i < argList.size() - 1 && argList[i+1] == '[')
					{
						++i;
						LocalObjectArrayRefPtr arg = getObjectArrayArrayElement(LocalObjectArrayRefParam(jparams), paramIndex + 1);
						if (arg != LocalObjectArrayRef::cms_nullPtr)
						{
							std::vector<const Unicode::String *> * strings = new std::vector<const Unicode::String *>();
							getStringArray(*arg, *strings);
							args.changeParam(paramIndex, *strings, true);
						}
						else
						{
							WARNING_STRICT_FATAL(true, ("Error getting back string array param on script return"));
						}
					}
				}
				break;
			case 'S':
				if (i < argList.size() - 1 && argList[i+1] == '*')
				{
					++i;
					StringId * value = new StringId();
					LocalRefPtr arg = getObjectArrayElement(LocalObjectArrayRefParam(jparams), paramIndex + 1);
					if (arg != LocalRef::cms_nullPtr)
					{
						// get the string id table
						JavaStringPtr table = getStringField(*arg, ms_fidStringIdTable);
						std::string localString;
						convert(*table, localString);
						value->setTable(localString);
						// get the string id text, if it is not nullptr/empty, use it
						localString.clear();
						JavaStringPtr text = getStringField(*arg, ms_fidStringIdAsciiId);
						if (text != JavaString::cms_nullPtr)
							convert(*text, localString);
						if (localString[0] != '\0')
							value->setText(localString);
						else
						{
							// use the textIndex field instead
							jint textIndex = getIntField(*arg, ms_fidStringIdIndexId);
							value->setTextIndex(textIndex);
						}
						args.changeParam(paramIndex, *value, true);
					}
					else
					{
						WARNING_STRICT_FATAL(true, ("Error getting back string id param on script return"));
					}
				}
				break;


			//----------------------------------------------------------------------
			//-- menu info

			case 'm':
				if (i < argList.size() - 1 && argList[i+1] == '*')
				{
					++i;

					LocalRefPtr arg = getObjectArrayElement(LocalObjectArrayRefParam(jparams), paramIndex + 1);
					MenuDataVector * menuVector = new MenuDataVector;
					convert(arg, *menuVector);
					args.changeParam (paramIndex, *menuVector, true);
				}
				break;

			//----------------------------------------------------------------------

			default:
				break;
		}
		// skip extra characters in the arglist
		if (i < argList.size() - 1)
		{
			if (argList[i+1] == '[' || argList[i+1] == '*')
				++i;
		}
	}
}	// JavaLibrary::alterScriptParams

/**
 * Executes all the scripts attached to an object; invoked from C code.
 *
 * @param caller		object id of the network object invoking this script
 * @param method		script method name
 * @param argList		argument list description
 * @param args			arguments to be passed to the script
 *
 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
int JavaLibrary::runScripts(const NetworkId & caller,
	const std::string& method, const std::string& argList, ScriptParams & args)
{
	// get the current env count, in case we reset our Java connection
	int currentEnvCount = ms_envCount;

	PROFILER_AUTO_BLOCK_CHECK_DEFINE("JavaLibrary::runScripts");

	GlobalInstances globals;

	DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
		"JavaLibrary::runScripts enter, self = %s, method = %s\n",
		caller.getValueString().c_str(), method.c_str()));

	if (ms_env == nullptr || ms_clsObject == nullptr)
 	{
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::runScripts exit, self = %s, method = %s\n",
			caller.getValueString().c_str(), method.c_str()));
		if (!ms_env)
		{
			LOG("ScriptInvestigation", ("runSCripts failed because env was nullptr"));
		}
		else
		{
			LOG("ScriptInvestigation", ("runSCripts failed because clsObject was nullptr"));
		}
		return SCRIPT_OVERRIDE;
	}

	if (ms_currentRecursionCount >= MAX_RECURSION_COUNT)
	{
		DEBUG_REPORT_LOG(true, ("JavaLibrary::runScripts (C version) max recursion "
			"count reached, method = %s, object id = %s, dumping script!\n", 
			method.c_str(), caller.getValueString().c_str()));
		LOG("ScriptInvestigation", ("runSCripts failed because max recursion count reached"));		
		return SCRIPT_OVERRIDE;
	}

	// convert the script and method strings to jstrings
	JavaString jmethod(method.c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::runScripts exit, self = %s, method = %s\n",
			caller.getValueString().c_str(), method.c_str()));
		LOG("ScriptInvestigation", ("runScripts failed trying to convert method "
			"name to jstring"));
		return SCRIPT_OVERRIDE;
	}

	// convert the params to jobjects
	LocalObjectArrayRefPtr jparams = convert(caller, argList, args);
	if (jparams == LocalObjectArrayRef::cms_nullPtr)
	{
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::runScripts exit, self = %s, method = %s\n",
			caller.getValueString().c_str(), method.c_str()));
		LOG("ScriptInvestigation", ("runSCripts failed because could not convert params"));		
		return SCRIPT_OVERRIDE;
	}

	// invoke the script method
	jint result = callScriptEntry(jmethod, jparams->getValue());

	//==========================================================================
	// !!IMPORTANT after this point our env pointer may have changed, we can't 
	// call any JNI functions on objects created with the old pointer!!
	//==========================================================================

	if (result == SCRIPT_CONTINUE && argList.find('*') != std::string::npos)
	{
		if (currentEnvCount == ms_envCount)
		{
			// copy any modifiable params from Java to C
			alterScriptParams(jparams->getValue(), argList, args);
		}
		else
		{
			LOG("ScriptInvestigation", ("JavaLibrary::runScripts env has changed, "
				"we can't get return params for trigger %s", method.c_str()));
		}
	}

	if (currentEnvCount == ms_envCount)
	{
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
				"JavaLibrary::runScripts exit, self = %s, method = %s\n",
				caller.getValueString().c_str(), method.c_str()));
			LOG("ScriptInvestigation", ("runSCripts failed because exception after delete"));		
			return SCRIPT_OVERRIDE;
		}
	}
	else
	{
		DEBUG_LOG("ScriptInvestigation", ("JavaLibrary::runScripts env changed after trigger %s", method.c_str()));
		jmethod.emergencyClear();
	}

	DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
		"JavaLibrary::runScripts exit, self = %s, method = %s\n",
		caller.getValueString().c_str(), method.c_str()));
	return result;
}	// JavaLibrary::runScripts(const ScriptParams & args)

/**
 * Executes all the scripts attached to an object; invoked from the console.
 *
 * @param caller		object id of the network object invoking this script
 * @param method		script method name
 * @param argList		argument list description
 * @param args			arguments to be passed to the script
 *
 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
int JavaLibrary::runScripts(const NetworkId & caller, 
	const std::string& method, const std::string& argList, const StringVector_t &args)
{
	DEBUG_WARNING(true, ("Running scripts from the console not currently supported"));
	LOG("ScriptInvestigation", ("runScripts from console is not supported"));
	return SCRIPT_OVERRIDE;
}	// JavaLibrary::runScripts(StringVector_t args)

/**
 * Executes a script invoked from C code.
 *
 * @param caller		object id of the network object invoking this script
 * @param script		script name
 * @param method		script method name
 * @param argList		argument list description
 * @param args			arguments to be passed to the script
 *
 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
int JavaLibrary::runScript(const NetworkId & caller, const std::string& script,
	const std::string& method, const std::string& argList, ScriptParams & args)
{
	// get the current env count, in case we reset our Java connection
	int currentEnvCount = ms_envCount;

	PROFILER_AUTO_BLOCK_CHECK_DEFINE("JavaLibrary::runScript");

	GlobalInstances globals;

	DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
		"JavaLibrary::runScript %s enter, self = %s, method = %s\n",
		script.c_str(), caller.getValueString().c_str(), method.c_str()));

	if (ms_env == nullptr || ms_midRunOne == nullptr || ms_clsObject == nullptr)
 	{
		if (!ms_env)
		{
			LOG("ScriptInvestigation", ("runSCripts2 failed because env was nullptr"));
		}
		else if (!ms_midRunOne)
		{
			LOG("ScriptInvestigation", ("runSCripts2 failed because midRunOne was nullptr"));
		}
		else
		{
			LOG("ScriptInvestigation", ("runSCripts2 failed because clsObject was nullptr"));
		}
				
		return SCRIPT_OVERRIDE;
	}

	if (ms_currentRecursionCount >= MAX_RECURSION_COUNT)
	{
		DEBUG_REPORT_LOG(true, ("JavaLibrary::runScript (C version) max recursion "
			"count reached, script = %s, method = %s, object id = %s, dumping "
			"script!\n", script.c_str(), method.c_str(), caller.getValueString().c_str()));
		LOG("ScriptInvestigation", ("runSCripts2 failed because max recursion count reached"));		
		return SCRIPT_OVERRIDE;
	}

	// convert the script and method strings to jstrings
	JavaString jscript(("script." + script).c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("runSCripts2 failed because of exception"));		
		return SCRIPT_OVERRIDE;
	}
	JavaString jmethod(method.c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("runSCripts2 failed because of exception2"));		
		return SCRIPT_OVERRIDE;
	}

	// convert the params to jobjects
	LocalObjectArrayRefPtr jparams = convert(caller, argList, args);
	if (jparams == LocalObjectArrayRef::cms_nullPtr)
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("runSCripts2 failed because of exception3"));		
		return SCRIPT_OVERRIDE;
	}

	// invoke the script method
	jint result = callScriptEntry(jscript, jmethod, jparams->getValue());
	
	//==========================================================================
	// !!IMPORTANT after this point our env pointer may have changed, we can't 
	// call any JNI functions on objects created with the old pointer!!
	//==========================================================================

	if (result == SCRIPT_DEFAULT)
		result = SCRIPT_CONTINUE;
	if (result == SCRIPT_CONTINUE && argList.find('*') != std::string::npos)
	{
		if (currentEnvCount == ms_envCount)
		{
			// copy any modifiable params from Java to C
			alterScriptParams(jparams->getValue(), argList, args);
		}
		else
		{
			LOG("ScriptInvestigation", ("JavaLibrary::runScript env has changed, "
				"we can't get return params for trigger %s", method.c_str()));
		}
	}

	if (currentEnvCount == ms_envCount)
	{
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			LOG("ScriptInvestigation", ("runScript failed because of exception4"));		
			return SCRIPT_OVERRIDE;
		}
	}
	else
	{
		DEBUG_LOG("ScriptInvestigation", ("JavaLibrary::runScript env changed after trigger %s", method.c_str()));
		jscript.emergencyClear();
		jmethod.emergencyClear();
	}

	DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
		"JavaLibrary::runScript %s exit, self = %s, method = %s\n",
		script.c_str(), caller.getValueString().c_str(), method.c_str()));

	return result;
}	// JavaLibrary::runScript(const ScriptParams & args)

/**
 * Executes a script invoked from Java code.
 *
 * @param caller		object id of the network object invoking this script
 * @param script		script name
 * @param method		script method name
 * @param args			arguments to be passed to the script
 *
 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
int JavaLibrary::runScript(const NetworkId & caller, const std::string& script,
	const std::string& method, jobjectArray args)
{
	// get the current env count, in case we reset our Java connection
	int currentEnvCount = ms_envCount;

	PROFILER_AUTO_BLOCK_CHECK_DEFINE("JavaLibrary::runScript");

	DEBUG_REPORT_LOG(true, ("JavaLibrary::runScript %s enter, method = %s\n",
		script.c_str(),	method.c_str()));

	if (ms_env == nullptr || ms_midRunOne == nullptr || ms_clsObject == nullptr)
	{
		if (!ms_env)
		{
			LOG("ScriptInvestigation", ("runSCripts3 failed because env was nullptr"));
		}
		else if (!ms_midRunOne)
		{
			LOG("ScriptInvestigation", ("runSCripts3 failed because midRunOne was nullptr"));
		}
		else
		{
			LOG("ScriptInvestigation", ("runSCripts3 failed because clsObject was nullptr"));
		}
		return SCRIPT_OVERRIDE;
	}

	if (ms_currentRecursionCount >= MAX_RECURSION_COUNT)
	{
		DEBUG_REPORT_LOG(true, ("JavaLibrary::runScript (Java version) max "
			"recursion count reached, script = %s, method = %s, object id = %s, "
			"dumping script!\n", script.c_str(), method.c_str(), 
			caller.getValueString().c_str()));
		LOG("ScriptInvestigation", ("runSCripts3 failed because of max recursion count"));		
		return SCRIPT_OVERRIDE;
	}

	// convert the script and method strings to jstrings
	JavaString jscript(("script." + script).c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("runSCripts3 failed because of exception"));		
		return SCRIPT_OVERRIDE;
	}
	JavaString jmethod(method.c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("runSCripts3 failed because of exception2"));		
		return SCRIPT_OVERRIDE;
	}

	// copy the args to a new array that has the object id tacked on
	int argsCount = 1;
	if (args != 0)
		argsCount += ms_env->GetArrayLength(args);

	LocalObjectArrayRefPtr jparams = createNewObjectArray(argsCount, ms_clsObject);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("runSCripts3 failed because of exception3"));		
		return SCRIPT_OVERRIDE;
	}
	for (int i = 1; i < argsCount; ++i)
	{
		LocalRefPtr element = getObjectArrayElement(LocalObjectArrayRefParam(args), i - 1);
		setObjectArrayElement(*jparams, i, *element);
	}

	// add the "self" parameter as the 1st argument
	LocalRefPtr self = getObjId(caller);
	setObjectArrayElement(*jparams, 0, *self);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("runSCripts3 failed because of exception4"));		
		return SCRIPT_OVERRIDE;
	}

	// invoke the script method
	jint result = callScriptEntry(jscript, jmethod, jparams->getValue());

	//==========================================================================
	// !!IMPORTANT after this point our env pointer may have changed, we can't 
	// call any JNI functions on objects created with the old pointer!!
	//==========================================================================

	if (currentEnvCount == ms_envCount)
	{
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			LOG("ScriptInvestigation", ("runSCripts3 failed because of exception5"));		
			return SCRIPT_OVERRIDE;
		}
	}
	else
	{
		DEBUG_LOG("ScriptInvestigation", ("JavaLibrary::runScript(from java) env changed after trigger %s", method.c_str()));
		self->emergencyClear();
		jparams->emergencyClear();
		jscript.emergencyClear();
		jmethod.emergencyClear();
	}

	DEBUG_REPORT_LOG(true, ("JavaLibrary::runScript %s exit, method = %s\n",
		script.c_str(),	method.c_str()));

	return result;
}	// JavaLibrary::runScript(jobjectArray args)

/**
 * Executes a script invoked from the console.
 *
 * @param caller		object id of the network object invoking this script
 * @param script		script name
 * @param method		script method name
 * @param argList		argument list description
 * @param args			arguments to be passed to the script
 *
 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
int JavaLibrary::runScript(const NetworkId & caller, const std::string& script,
	const std::string& method, const std::string& argList, const StringVector_t &args)
{
	// get the current env count, in case we reset our Java connection
	int currentEnvCount = ms_envCount;

	PROFILER_AUTO_BLOCK_CHECK_DEFINE("JavaLibrary::runScript");

	GlobalInstances globals;

	DEBUG_REPORT_LOG(true, ("JavaLibrary::runScript %s enter, method = %s\n", script.c_str(),
		method.c_str()));

	if (ms_env == nullptr || ms_midRunOne == nullptr || ms_clsObject == nullptr)
	{
		if (!ms_env)
		{
			LOG("ScriptInvestigation", ("runSCripts4 failed because env was nullptr"));
		}
		else if (!ms_midRunOne)
		{
			LOG("ScriptInvestigation", ("runSCripts4 failed because midRunOne was nullptr"));
		}
		else
		{
			LOG("ScriptInvestigation", ("runSCripts4 failed because clsObject was nullptr"));
		}
				
		return SCRIPT_OVERRIDE;
	}

	if (ms_currentRecursionCount >= MAX_RECURSION_COUNT)
	{
		DEBUG_REPORT_LOG(true, ("JavaLibrary::runScript (console version) max "
			"recursion count reached, script = %s, method = %s, object id = %s, "
			"dumping script!\n", script.c_str(), method.c_str(), 
			caller.getValueString().c_str()));
		LOG("ScriptInvestigation", ("runSCripts4 failed because of max recursion count"));		
		return SCRIPT_OVERRIDE;
	}

	// convert the script and method strings to jstrings
	JavaString jscript(("script." + script).c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("runSCripts4 failed because of exception"));		
		return SCRIPT_OVERRIDE;
	}
	JavaString jmethod(method.c_str());
	if (ms_env->ExceptionCheck())
	{
		LOG("ScriptInvestigation", ("runSCripts4 failed because of exception2"));		
		ms_env->ExceptionDescribe();
		return SCRIPT_OVERRIDE;
	}

	// convert the params to jobjects
	LocalObjectArrayRefPtr jparams = createNewObjectArray(static_cast<jsize>(argList.size() + 1),
		ms_clsObject);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("runSCripts4 failed because of exception3"));		
		return SCRIPT_OVERRIDE;
	}
	// add the "self" parameter
	LocalRefPtr self = getObjId(caller);
	setObjectArrayElement(*jparams, 0, *self);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("runSCripts4 failed because of exception4"));		
		return SCRIPT_OVERRIDE;
	}

	// add the rest of the parameters
	for (unsigned i = 0; i < argList.size(); ++i)
	{
		LocalRefParamPtr arg = LocalRefParam::cms_nullPtr;
		const char *stringArg = Unicode::wideToNarrow(args[i]).c_str();
		switch (argList[i])
		{
			case 'i':
				{
					jint param = atoi(stringArg);
					arg = createNewObject(ms_clsInteger, ms_midInteger, param);
				}
				break;
			case 'f':
				{
					jfloat param = static_cast<jfloat>(atof(stringArg));
					arg = createNewObject(ms_clsFloat, ms_midFloat, param);
				}
				break;
			case 's':
				{
					arg = createNewString(stringArg);
				}
				break;
			case 'u':
				{
					arg = createNewString(args[i].c_str(), static_cast<jsize>(args[i].size()));
				}
				break;
			case 'O':
				{
					arg = getObjId(NetworkId(stringArg));
					if (arg == LocalRef::cms_nullPtr)
					{
						LOG("ScriptInvestigation", ("runSCripts4 failed because of bad objid"));		
						return SCRIPT_OVERRIDE;
					}
				}
				break;
			default:
				DEBUG_REPORT_LOG(true, ("unknown parameter type %c(%#x)\n", argList[i],
					static_cast<unsigned>(argList[i])));		//lint !e571 suspicious cast
				LOG("ScriptInvestigation", ("runSCripts4 failed because of param"));		
				return SCRIPT_OVERRIDE;
		}
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			LOG("ScriptInvestigation", ("runSCripts4 failed because of exception5"));		
			return SCRIPT_OVERRIDE;
		}
		setObjectArrayElement(*jparams, static_cast<jsize>(i + 1), *arg);
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			LOG("ScriptInvestigation", ("runSCripts4 failed because of exception6"));		
			return SCRIPT_OVERRIDE;
		}
	}

	// invoke the script method
	jint result = callScriptEntry(jscript, jmethod, jparams->getValue());

	//==========================================================================
	// !!IMPORTANT after this point our env pointer may have changed, we can't 
	// call any JNI functions on objects created with the old pointer!!
	//==========================================================================

	if (currentEnvCount == ms_envCount)
	{
		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			LOG("ScriptInvestigation", ("runSCripts4 failed because of exception7"));		
			return SCRIPT_OVERRIDE;
		}
	}
	else
	{
		DEBUG_LOG("ScriptInvestigation", ("JavaLibrary::runScript(from console) env changed after trigger %s", method.c_str()));
		self->emergencyClear();
		jparams->emergencyClear();
		jscript.emergencyClear();
		jmethod.emergencyClear();
	}

	return result;
}	// JavaLibrary::runScript(StringVector_t args)

/**
 * Calls a special function on a script that returns a string instead of an 
 * integer. The script function has no "self" associated with it.
 *
 * @param script		the name of the script to call
 * @param method		the function name in the script
 * @param argList		the argument types that are being passed to the script
 * @param args			the arguments that are being passed to the script
 *
 * @return the string returned from the script function
 */
std::string JavaLibrary::callScriptConsoleHandler(const std::string & script, 
	const std::string & method, const std::string & argList, ScriptParams & args)
{
static const std::string errorReturnString;

	// get the current env count, in case we reset our Java connection
	int currentEnvCount = ms_envCount;

	PROFILER_AUTO_BLOCK_CHECK_DEFINE("JavaLibrary::callScriptConsoleHandler");

	GlobalInstances globals;

	DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
		"JavaLibrary::runScript %s enter, method = %s\n", script.c_str(), 
		method.c_str()));

	if (ms_env == nullptr || ms_midRunConsoleHandler == nullptr || ms_clsObject == nullptr)
 	{
		if (!ms_env)
		{
			LOG("ScriptInvestigation", ("runSCripts2 failed because env was nullptr"));
		}
		else if (!ms_midRunConsoleHandler)
		{
			LOG("ScriptInvestigation", ("runSCripts2 failed because midRunConsoleHandler was nullptr"));
		}
		else
		{
			LOG("ScriptInvestigation", ("runSCripts2 failed because clsObject was nullptr"));
		}
				
		return errorReturnString;
	}

	if (ms_currentRecursionCount >= MAX_RECURSION_COUNT)
	{
		DEBUG_REPORT_LOG(true, ("JavaLibrary::callScriptConsoleHandler max recursion "
			"count reached, script = %s, method = %s, dumping "
			"script!\n", script.c_str(), method.c_str()));
		LOG("ScriptInvestigation", ("callScriptConsoleHandler failed because max recursion count reached"));
		return errorReturnString;
	}

	// convert the script and method strings to jstrings
	JavaString jscript(("script." + script).c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("callScriptConsoleHandler failed because of exception"));		
		return errorReturnString;
	}
	JavaString jmethod(method.c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("callScriptConsoleHandler failed because of exception2"));		
		return errorReturnString;
	}

	// convert the params to jobjects
	LocalObjectArrayRefPtr jparams = convert(argList, args);
	if (jparams == LocalObjectArrayRef::cms_nullPtr)
	{
		ms_env->ExceptionDescribe();
		LOG("ScriptInvestigation", ("callScriptConsoleHandler failed because of exception3"));		
		return errorReturnString;
	}

	// invoke the script method
	jstring result = callScriptConsoleHandlerEntry(jscript, jmethod, jparams->getValue());

	//==========================================================================
	// !!IMPORTANT after this point our env pointer may have changed, we can't 
	// call any JNI functions on objects created with the old pointer!!
	//==========================================================================

	std::string returnResult;
	if (currentEnvCount == ms_envCount)
	{
		JavaString resultString(result);

		if (argList.find('*') != std::string::npos)
		{
			// copy any modifiable params from Java to C
			alterScriptParams(jparams->getValue(), argList, args);
		}

		if (ms_env->ExceptionCheck())
		{
			ms_env->ExceptionDescribe();
			LOG("ScriptInvestigation", ("callScriptConsoleHandler failed because of exception4"));
			return errorReturnString;
		}
		convert(resultString, returnResult);
	}
	else
	{
		DEBUG_LOG("ScriptInvestigation", ("JavaLibrary::callScriptConsoleHandler env changed after trigger %s", method.c_str()));
		jscript.emergencyClear();
		jmethod.emergencyClear();
	}

	DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
		"JavaLibrary::callScriptConsoleHandler %s exit, method = %s\n",
		script.c_str(), method.c_str()));

	return returnResult;
}	// JavaLibrary::callScriptConsoleHandler

/**
 * Tells the script engine to reload a script file.
 *
 * @param scriptName		the script name
 *
 * @return true on success, false if the script wasn't found
 */
bool JavaLibrary::reloadScript(const std::string& scriptName)
{
	if (ms_env == nullptr || ms_clsScriptEntry == nullptr || ms_midUnload == nullptr)
		return false;

	// convert the script name to jstring
	JavaString jscript(("script." + scriptName).c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return false;
	}

	// call the unload method
	jboolean result = ms_env->CallStaticBooleanMethod(ms_clsScriptEntry, ms_midUnload,
		jscript.getValue());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		return false;
	}
	return result;
}	// JavaLibrary::reloadScript

/**
 * Calls a user-defined message method on all scripts attached to an object.
 *
 * @param caller		object id of the network object invoking this script
 * @param method		script message method name
 * @param data          data to be passed to the message
 *
 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
int JavaLibrary::callMessages(const NetworkId & caller, const std::string & method, const ScriptDictionaryPtr & data)
{
	// get the current env count, in case we reset our Java connection
	int currentEnvCount = ms_envCount;

	DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
		"JavaLibrary::callMessages enter, self = %s, method = %s\n",
		caller.getValueString().c_str(), method.c_str()));

	int result = SCRIPT_CONTINUE;

	if (ms_env == nullptr || ms_midRunOne == nullptr || ms_clsObject == nullptr)
	{
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::callMessages exit, self = %s, method = %s\n",
			caller.getValueString().c_str(), method.c_str()));
		if (!ms_env)
		{
			LOG("ScriptInvestigation", ("callMessages failed because env was nullptr"));
		}
		else if (!ms_midRunOne)
		{
			LOG("ScriptInvestigation", ("callMessages failed because midRunOne was nullptr"));
		}
		else
		{
			LOG("ScriptInvestigation", ("callMessages failed because clsObject was nullptr"));
		}
				
		return SCRIPT_OVERRIDE;
	}

	if (ms_currentRecursionCount >= MAX_RECURSION_COUNT)
	{
		DEBUG_REPORT_LOG(true, ("JavaLibrary::callMessages max recursion count reached, "
			"method = %s, object id = %s, dumping script!\n",
			method.c_str(), caller.getValueString().c_str()));
		LOG("ScriptInvestigation", ("callMessages failed because max recurion count"));
		return SCRIPT_OVERRIDE;
	}

	const JavaDictionary * dictionary = safe_cast<const JavaDictionary *>(data.get());
	jobject jdictionary = 0;
	if (dictionary != nullptr)
		jdictionary = dictionary->getValue();

	// convert the method string to jstrings
	JavaString jmethod(method.c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::callMessages exit, self = %s, method = %s\n",
			caller.getValueString().c_str(), method.c_str()));
		LOG("ScriptInvestigation", ("callMessages failed because of exception"));
		return SCRIPT_OVERRIDE;
	}

	++ms_currentRecursionCount;
	if (ms_currentRecursionCount > 3)
	{
		LOG("ScriptRecursion", ("callMessages recursion %d", ms_currentRecursionCount));
	}
  	result = ms_env->CallStaticIntMethod(ms_clsScriptEntry, ms_midCallMessages, jmethod.getValue(), caller.getValue(), jdictionary);
  	--ms_currentRecursionCount;

	result = handleScriptEntryCleanup(result);

	//==========================================================================
	// !!IMPORTANT after this point our env pointer may have changed, we can't 
	// call any JNI functions on objects created with the old pointer!!
	//==========================================================================

	if (currentEnvCount != ms_envCount)
	{
		jmethod.emergencyClear();
	}

	DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
		"JavaLibrary::callMessages exit, self = %s, method = %s\n",
		caller.getValueString().c_str(), method.c_str()));

	return result;
}	// JavaLibrary::callMessages

/**
 * Calls a user-defined message method on a script attached to an object.
 *
 * @param caller		object id of the network object invoking this script
 * @param script		script name
 * @param method		script message method name
 * @param data          data to be passed to the message
 *
 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
int JavaLibrary::callMessage(const NetworkId & caller, const std::string & script,
	const std::string & method, const ScriptDictionary & data)
{
	// get the current env count, in case we reset our Java connection
	int currentEnvCount = ms_envCount;

	DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
		"JavaLibrary::callMessage %s enter, self = %s, method = %s\n",
		script.c_str(), caller.getValueString().c_str(), method.c_str()));

	if (ms_env == nullptr || ms_midRunOne == nullptr || ms_clsObject == nullptr)
	{
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::callMessage %s exit, self = %s, method = %s\n",
			script.c_str(), caller.getValueString().c_str(), method.c_str()));
		if (!ms_env)
		{
			LOG("ScriptInvestigation", ("callMessage failed because env was nullptr"));
		}
		else if (!ms_midRunOne)
		{
			LOG("ScriptInvestigation", ("callMessage failed because midRunOne was nullptr"));
		}
		else
		{
			LOG("ScriptInvestigation", ("callMessage failed because clsObject was nullptr"));
		}

		return SCRIPT_OVERRIDE;
	}

	if (ms_currentRecursionCount >= MAX_RECURSION_COUNT)
	{
		DEBUG_REPORT_LOG(true, ("JavaLibrary::callMessage max recursion count reached, "
			"script = %s, method = %s, object id = %s, dumping script!\n",
			script.c_str(), method.c_str(), caller.getValueString().c_str()));
		LOG("ScriptInvestigation", ("callMessage failed because max recursion count"));
		return SCRIPT_OVERRIDE;
	}	

	const JavaDictionary * dictionary = dynamic_cast<const JavaDictionary *>(&data);
	if (dictionary == nullptr)
	{
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::callMessage %s exit, self = %s, method = %s\n",
			script.c_str(), caller.getValueString().c_str(), method.c_str()));
		LOG("ScriptInvestigation", ("callMessage failed because bad dictionary"));
		return SCRIPT_OVERRIDE;
	}

	// convert the script and method strings to jstrings
	JavaString jscript(script.c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::callMessage %s exit, self = %s, method = %s\n",
			script.c_str(), caller.getValueString().c_str(), method.c_str()));
		LOG("ScriptInvestigation", ("callMessage failed because exception"));
		return SCRIPT_OVERRIDE;
	}
	JavaString jmethod(method.c_str());
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::callMessage %s exit, self = %s, method = %s\n",
			script.c_str(), caller.getValueString().c_str(), method.c_str()));
		LOG("ScriptInvestigation", ("callMessage failed because exception2"));
		return SCRIPT_OVERRIDE;
	}

	// set up the parameters
	LocalObjectArrayRefPtr jparams = createNewObjectArray(2, ms_clsObject);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::callMessage %s exit, self = %s, method = %s\n",
			script.c_str(), caller.getValueString().c_str(), method.c_str()));
		LOG("ScriptInvestigation", ("callMessage failed because exception3"));
		return SCRIPT_OVERRIDE;
	}
	// add the data parameter
	setObjectArrayElement(*jparams, 1, *dictionary);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::callMessage %s exit, self = %s, method = %s\n",
			script.c_str(), caller.getValueString().c_str(), method.c_str()));
		LOG("ScriptInvestigation", ("callMessage failed because exception4"));
		return SCRIPT_OVERRIDE;
	}

	// add the "self" parameter
	LocalRefPtr self = getObjId(caller);
	setObjectArrayElement(*jparams, 0, *self);
	if (ms_env->ExceptionCheck())
	{
		ms_env->ExceptionDescribe();
		DEBUG_REPORT_LOG(ConfigServerGame::getJavaConsoleDebugMessages(), (
			"JavaLibrary::callMessage %s exit, self = %s, method = %s\n",
			script.c_str(), caller.getValueString().c_str(), method.c_str()));
		LOG("ScriptInvestigation", ("callMessage failed because exception5"));
		return SCRIPT_OVERRIDE;
	}

	int result = callScriptEntry(jscript, jmethod, jparams->getValue());

	//==========================================================================
	// !!IMPORTANT after this point our env pointer may have changed, we can't 
	// call any JNI functions on objects created with the old pointer!!
	//==========================================================================

	if (currentEnvCount == ms_envCount)
	{
		if (ms_env->ExceptionCheck())
			ms_env->ExceptionDescribe();
	}
	else
	{
		DEBUG_LOG("ScriptInvestigation", ("JavaLibrary::callMessage env changed after message %s", method.c_str()));
		self->emergencyClear();
		jparams->emergencyClear();
		jscript.emergencyClear();
		jmethod.emergencyClear();
	}

	return result;
}	// JavaLibrary::callMessage

//========================================================================
// class JavaLibrary string/stringId methods
//========================================================================

/**
 * Gets the name of a java class/script.
 *
 * @param sourceClass		the class we want the name of
 * @param target			string that will be filled in with the name
 *
 * @reurn true on success, false on error
 */
const bool JavaLibrary::getClassName(const jclass & sourceClass, std::string & target)
{
	bool result = false;
	if(ms_env)
	{
		const LocalRefParamPtr classPtr(new LocalRefParam(sourceClass));
		JavaStringPtr s = callStringMethod(*classPtr, JavaLibrary::ms_midClassGetName);
		if (s != JavaString::cms_nullPtr)
		{
			if (convert(*s, target))
			{
				//@todo Make sure this is the correct way to do things
				//we need to strip script. from the beginning of the class string
				const std::string scriptString("script.");
				size_t position = target.find(scriptString);
				if (position == 0)
				{
					target.erase(position, scriptString.length());
				}
				result = true;
			}
		}
	}
	return result;
}	// JavaLibrary::getClassName(const jclass &)

/**
 * Gets the name of a java class/script.
 *
 * @param source		the object we want the class name of
 * @param target		string that will be filled in with the name
 *
 * @reurn true on success, false on error
 */
const bool JavaLibrary::getClassName(const jobject & source, std::string & target)
{
	bool result = false;
	if (ms_env)
	{
		jclass tempClass = ms_env->GetObjectClass(source);
		result = getClassName(tempClass, target);
	}
	return result;
}	// JavaLibrary::getClassName(const jobject &)

//-----------------------------------------------------------------------

/**
 * Packs a Java dictionary class into a string for network transmission.
 *
 * @param dictionary	the dictionary to pack
 * @param packedData	the string to be filled with the packed dictionary
 */
void JavaLibrary::packDictionary(const ScriptDictionary & dictionary,
	std::vector<int8> & packedData)
{
	const_cast<ScriptDictionary &>(dictionary).serialize();
	packedData = dictionary.getSerializedData();
}	// JavaLibrary::packDictionary

/**
 * Unpacks a Java dictionary class from a string.
 *
 * @param packedData	the string to unpack
 * @param dictionary	pointer to be set to the unpacked dictionary
 *
 * @return true on success, false on error
 */
bool JavaLibrary::unpackDictionary(const std::vector<int8> & packedData,
	ScriptDictionaryPtr & dictionary)
{
	if (ms_env == nullptr)
		return false;

	dictionary = JavaDictionary::cms_nullPtr;

	if (!packedData.empty())
	{
		// get the crc from the data
		uint32 crc = 0;
		int dataLen = packedData.size();
		const int8 * data = &packedData[0];
		
		std::vector<int8>::const_iterator result = std::find(packedData.begin(), 
			packedData.end(), '*');
		if (result != packedData.end())
		{
			// double-check: verify all the characters before the marker are 
			// digits
			int i;
			int markerPos = packedData.end() - result;
			for (i = 0; i < markerPos; ++i)
			{
				if (!isdigit(data[i]))
					break;
				crc = crc * 10 + (data[i] - '0');
			}
			if (i == markerPos)
			{
				data = &data[markerPos+1];
				// verify the crc
				dataLen -= markerPos + 1;
				uint32 dataCrc = Crc::calculate(data, dataLen);
				if (crc != dataCrc)
				{
					WARNING(true, ("JavaLibrary::unpackDictionary got packed data with "
						"bad crc! Expected: %lu, got %lu.", crc, dataCrc));
					return false;
				}
			}
		}
		
		if (data != nullptr && *data != '\0')
		{
			LocalByteArrayRefPtr jdata = createNewByteArray(dataLen);
			if (jdata != LocalByteArrayRef::cms_nullPtr)
			{
				setByteArrayRegion(*jdata, 0, dataLen, const_cast<int8 *>(data));
				JavaDictionaryPtr lref(new JavaDictionary(ms_env->CallStaticObjectMethod(ms_clsDictionary, ms_midDictionaryUnpack, jdata->getValue())));
				if (lref == JavaDictionary::cms_nullPtr)
				{
					WARNING(true, ("JavaLibrary::unpackDictionary error unpacking dictionary"));
					return false;
				}
				dictionary = lref;
			}
		}
	}
	return true;
}	// JavaLibrary::unpackDictionary

//-----------------------------------------------------------------------

/**
 * Convert a JavaString to a std::string.
 */
const bool JavaLibrary::convert(const JavaStringParam & source, std::string & target)
{
	bool result = false;
	if (ms_env)
	{
		if (source.getValue() != 0)
		{
			jboolean isCopy(false);
			const char * temp = ms_env->GetStringUTFChars(source.getValue(), &isCopy);
			if(temp)
			{
				target = temp;
				ms_env->ReleaseStringUTFChars(source.getValue(), temp);
				result = true;
			}
		}
	}
	return result;
}	// JavaLibrary::convert(const JavaStringParam &, std::string &)

/**
 * Convert a JavaString to a Unicode::String.
 */
const bool JavaLibrary::convert(const JavaStringParam & source, Unicode::String & target)
{
	bool result = false;
	if (ms_env)
	{
		if (source.getValue() != 0)
		{
			jboolean isCopy = JNI_FALSE;
			jsize len = getStringLength(source);
			const jchar * temp = ms_env->GetStringCritical(source.getValue(), &isCopy);
			if (temp)
			{
				target.assign(temp, len);
				ms_env->ReleaseStringCritical(source.getValue(), temp);
				result = true;
			}
		}
	}
	return result;
}	// JavaLibrary::convert(const JavaStringParam &, Unicode::String &)

//-----------------------------------------------------------------------

/**
 * Serializes a JavaDictionary into a string.
 */
const bool JavaLibrary::convert(const JavaDictionary & source, std::vector<int8> & target)
{
	bool result = false;
	if (ms_env != nullptr)
	{
		if (source.getValue() != 0)
		{
			// convert the dictionary to a jstring
			LocalByteArrayRefPtr packedData = callByteArrayMethod(source, ms_midDictionaryPack);
			if (ms_env->ExceptionCheck())
			{
				ms_env->ExceptionDescribe();
			}
			else if (packedData != LocalByteArrayRef::cms_nullPtr)
			{
				// convert the Java array to a vector
				int count = getArrayLength(*packedData);
				target.reserve(count);
				target.resize(count);
				getByteArrayRegion(*packedData, 0, count, &target[0]);
				result = true;
			}
		}
	}
	return result;
}	// JavaLibrary::convert(const JavaDictionary &, std::string &)


//=======================================================================

const bool JavaLibrary::convert(const std::map<std::string, int> & source, JavaDictionaryPtr & target)
{
	std::vector<std::pair<std::string, int> > sourceData;
	std::map<std::string, int>:: const_iterator i;
	for(i = source.begin(); i != source.end(); ++i)
	{
		sourceData.push_back(*i);
	}
	return convert(sourceData, target);
}

//-----------------------------------------------------------------------

const bool JavaLibrary::convert(const std::vector<std::pair<std::string, int> > & source, JavaDictionaryPtr & target)
{
	if (!JavaLibrary::getEnv())
		return false;

	target = createNewDictionary();
	if (target == JavaDictionary::cms_nullPtr)
		return false;

	std::vector<std::pair<std::string, int> >::const_iterator i;
	for(i = source.begin(); i != source.end(); ++i)
	{
		int value = (*i).second;
		JavaString name((*i).first.c_str());
		callObjectMethod(*target, ms_midDictionaryPutInt, name.getValue(), value);
	}
	return true;
}

//-----------------------------------------------------------------------

const bool JavaLibrary::convert(const std::vector<std::pair<std::string, std::pair<int, int> > > & source, JavaDictionaryPtr & target)
{
	if (!JavaLibrary::getEnv())
		return false;

	target = createNewDictionary();
	if (target == JavaDictionary::cms_nullPtr)
		return false;

	std::vector<std::pair<std::string, std::pair<int, int> > >::const_iterator i;
	for(i = source.begin(); i != source.end(); ++i)
	{
		int value = (*i).second.first;
		JavaString name((*i).first.c_str());
		callObjectMethod(*target, ms_midDictionaryPutInt, name.getValue(), value);
	}
	return true;
}

//-----------------------------------------------------------------------

const bool JavaLibrary::convert(const std::vector<std::pair<std::string, bool> > & source, JavaDictionaryPtr & target)
{
	if (!JavaLibrary::getEnv())
		return false;

	target = createNewDictionary();
	if (target == JavaDictionary::cms_nullPtr)
		return false;

	std::vector<std::pair<std::string, bool> >::const_iterator i;
	for(i = source.begin(); i != source.end(); ++i)
{
		JavaString name((*i).first.c_str());
		bool value = (*i).second;
		callObjectMethod(*target, ms_midDictionaryPutBool, name.getValue(), value);
	}
	return true;
}

//=======================================================================

namespace ScriptConversion {

const bool convert(const jobject & source, Vector & target, NetworkId & targetCell, const Vector & i_default)
{
	if (!convert(source, target, targetCell))
	{
		target = i_default;
		targetCell = NetworkId::cms_invalid;
	}
	return true;
}

const bool convert(const std::vector<const Unicode::String *> & source, LocalObjectArrayRefPtr & target)
{
	if (!JavaLibrary::getEnv())
		return false;

	bool result = false;
		int count = source.size();
	target = createNewObjectArray(count, JavaLibrary::ms_clsString);
	if (target != LocalObjectArrayRef::cms_nullPtr)
		{
			result = true;
			for (int i = 0; i < count; ++i)
			{
				if (source[i] != nullptr)
				{
				JavaString targetElement(*source[i]);
				setObjectArrayElement(*target, i, targetElement);
			}
		}
	}
	return result;
}

const bool convert(const jobject source, StringId & target)
{
	if (source == 0)
		return false;

	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (env->IsInstanceOf(source, JavaLibrary::ms_clsStringId) == JNI_FALSE)
		return false;

	// @todo: it would be nice if we had a known max length for the table
	// name and text in order to use buffers to get the strings

	// get the table
	{
		JavaStringPtr tempString = getStringField(LocalRefParam(source), JavaLibrary::ms_fidStringIdTable);
		std::string localString;
		JavaLibrary::convert(*tempString, localString);
		target.setTable(localString);
	}

	// get the text
	{
		JavaStringPtr tempString = getStringField(LocalRefParam(source), JavaLibrary::ms_fidStringIdAsciiId);
		if (tempString != JavaString::cms_nullPtr && getStringLength(*tempString) > 0)
		{
			std::string localString;
			JavaLibrary::convert(*tempString, localString);
			target.setText(localString);
		}
		else
		{
			// get the text index
			jint textIndex = env->GetIntField(source, JavaLibrary::ms_fidStringIdIndexId);
			target.setTextIndex(textIndex);
		}
	}

	return true;
}

const bool convert(const LocalRefParam & source, StringId & target)
{
	return convert(source.getValue(), target);
}

const bool convert(const StringId & source, LocalRefPtr & target)
{
	if (!JavaLibrary::getEnv())
		return false;

	target = allocObject(JavaLibrary::ms_clsStringId);
	if (target == LocalRef::cms_nullPtr)
		return false;

	// convert the table
	{
		JavaString tempString(source.getTable().c_str());
		setObjectField(*target, JavaLibrary::ms_fidStringIdTable, tempString);
	}

	// convert the text
	{
		JavaString tempString(source.getText().c_str());
		setObjectField(*target, JavaLibrary::ms_fidStringIdAsciiId, tempString);
	}

	// convert the index
	setIntField(*target, JavaLibrary::ms_fidStringIdIndexId, source.getTextIndex());
	return true;
}

const bool convert(const std::vector<std::string> & source, LocalObjectArrayRefPtr & strArray)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return false;
	}

	strArray = createNewObjectArray(static_cast<jsize>(source.size()), JavaLibrary::ms_clsString);
	std::vector<std::string>::const_iterator i;
	int index = 0;
	for(i = source.begin(); i != source.end(); ++i)
	{
		const char * s = (*i).c_str();
		JavaString js(s);
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return false;
		}
		setObjectArrayElement(*strArray, index, js);
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return false;
		}
		++index;
	}
	return true;
}

const bool convert(const std::vector<const char *> & source, LocalObjectArrayRefPtr & strArray)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return false;
	}

	strArray = createNewObjectArray(static_cast<jsize>(source.size()), JavaLibrary::ms_clsString);
	std::vector<const char *>::const_iterator i;
	int index = 0;
	for(i = source.begin(); i != source.end(); ++i)
	{
		JavaString js(*i);
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return false;
		}
		setObjectArrayElement(*strArray, index, js);
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return false;
		}
		++index;
	}
	return true;
}

const bool convert(const std::set<CellPermissions::PermissionObject> & source, LocalObjectArrayRefPtr & strArray)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return false;
	}

	strArray = createNewObjectArray(static_cast<jsize>(source.size()), JavaLibrary::ms_clsString);
	std::set<CellPermissions::PermissionObject>::const_iterator i;
	int index = 0;
	for(i = source.begin(); i != source.end(); ++i)
	{
		const std::string  name = (*i).getName();
		const char * const s    = name.c_str();

		JavaString js(s);
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return false;
		}
		setObjectArrayElement(*strArray, index, js);
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return false;
		}
		++index;
	}

	return true;
}

const bool convert(const jobjectArray & source, stdvector<std::string>::fwd & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env || !source)
		return false;

	int count = env->GetArrayLength(source);
	target.resize(count, "");

	std::string tempString;
	for (int i = 0; i < count; ++i)
	{
		JavaStringPtr s = getStringArrayElement(LocalObjectArrayRefParam(source), i);
		if (s != JavaString::cms_nullPtr)
{
			if (JavaLibrary::convert(*s, tempString))
				target[i] = tempString;
	}
	}
	return true;
}

const bool convert(const jlongArray & source, std::vector<NetworkId> &results)
{
	bool result = false;
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env || !source)
		return false;

		jsize count = env->GetArrayLength(source);
		jsize i;
		result = true;
	jlong jlongTmp;
		for (i=0; i<count; ++i)
		{
		 env->GetLongArrayRegion(source, i, 1, &jlongTmp);
		 NetworkId nid(jlongTmp);
			 if (!nid)
			 {
				result = false;
			 }
			 else
			 {
				results.push_back(nid);
			 }
		}

	return result;
}

const bool convert(const jlongArray & source, std::vector<ServerObject *> &results)
{
	 JNIEnv * env = JavaLibrary::getEnv();
	 if (!env || !source)
			return false;

	bool result = false;

		jsize count = env->GetArrayLength(source);
		jsize i;
		result = true;
		for (i=0; i<count; ++i)
		{
		jlong id;
		env->GetLongArrayRegion(source, i, 1, &id);
			ServerObject * object = 0;
		if (!JavaLibrary::getObject(id, object))
			{
				result = false;
			}
			else
			{
				results.push_back(object);
			}
		}

	return result;
}

const bool convert(const std::vector<ServerObject *> &source, LocalLongArrayRefPtr & result)
{
	 JNIEnv * env = JavaLibrary::getEnv();
	 if (!env)
			return false;

	int count = source.size();
	bool rv = true;
	int resultCount = 0;
	for (int i = 0; i < count; ++i)
	{
		if (source[i])
		{
			resultCount++;
		}
	}
	result = createNewLongArray(resultCount);
	for (int j = 0; j < count; ++j)
	{
		if (source[j])
		{
			jlong arg = (source[j]->getNetworkId()).getValue();
			if (arg == 0)
			{
				rv = false;
				break;
			}
			setLongArrayRegion(*result, static_cast<jsize>(--resultCount), 1, &arg);
		}
	}
	if (env->ExceptionCheck())
	{
		 env->ExceptionDescribe();
		 rv = false;
	}
	return rv;
}

const bool convert(const std::vector<NetworkId> &source, LocalLongArrayRefPtr & result)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	int count = source.size();
	bool rv = true;
	int i;
	int resultCount = 0;
	for (i = 0; i < count; ++i)
	{
		if (source[i].isValid())
		{
			resultCount++;
		}
	}
	result = createNewLongArray(resultCount);
	jlong jlongTmp;
	for (int j = 0; j < count; ++j)
	{
		if (source[j].isValid())
		{
			jlongTmp = source[j].getValue();

			setLongArrayRegion(*result, static_cast<jsize>(--resultCount), 1, &jlongTmp);
			}
		else
		{
			rv = false;
		}
	}
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		rv = false;
	}
	return rv;
}

const bool convert(const jobject & source, Vector &target, NetworkId & targetCell)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (!source)
		return false;

	ServerObject * object = 0;

	if (env->IsInstanceOf(source, JavaLibrary::ms_clsLocation))
	{
		target.x = env->GetFloatField(source, JavaLibrary::ms_fidLocationX);
		target.y = env->GetFloatField(source, JavaLibrary::ms_fidLocationY);
		target.z = env->GetFloatField(source, JavaLibrary::ms_fidLocationZ);

		LocalRefPtr cell = getObjectField(LocalRefParam(source), JavaLibrary::ms_fidLocationCell);
		targetCell = JavaLibrary::getNetworkId(*cell);
		return true;
	}
	else if (JavaLibrary::getObject(source, object))
	{
		target = object->getPosition_p();
		const Object * cell = ContainerInterface::getContainedByObject(*object);
		targetCell = (cell) ? cell->getNetworkId() : NetworkId::cms_invalid;
		return true;
	}
	return false;
}

const bool convertWorld(const jobject & source, Vector &target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (!source)
		return false;

	ServerObject * object = 0;

	if (env->IsInstanceOf(source, JavaLibrary::ms_clsLocation))
	{
		target.x = env->GetFloatField(source, JavaLibrary::ms_fidLocationX);
		target.y = env->GetFloatField(source, JavaLibrary::ms_fidLocationY);
		target.z = env->GetFloatField(source, JavaLibrary::ms_fidLocationZ);

		LocalRefPtr cell = getObjectField(LocalRefParam(source), JavaLibrary::ms_fidLocationCell);
		Object * cellObject = 0;
		if (JavaLibrary::getObject(*cell, cellObject))
		{
			target = cellObject->rotateTranslate_o2w(target);
		}
		return true;
	}
	else if (JavaLibrary::getObject(source, object))
	{
		target = object->getPosition_w();
		return true;
	}
	return false;
}

const bool convertWorld(const jlong & source, Vector &target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (source == 0)
		return false;

	ServerObject * object = 0;

	if (JavaLibrary::getObject(source, object))
	{
		target = object->getPosition_w();
		return true;
	}
	return false;
}

const bool convertWorld(const LocalRefParam & source, Vector &target)
{
	return convertWorld(source.getValue(), target);
}

const bool convert(const Vector & source, const NetworkId & sourceCell, LocalRefPtr & target)
{
	return convert(source, ServerWorld::getSceneId(), sourceCell, target);
}

const bool convert(const jobject & source, Vector & targetLoc, std::string & targetSceneId, NetworkId & targetCell)
{
	 JNIEnv * env = JavaLibrary::getEnv();
	 if (!env)
			return false;

	if (!source)
		return false;

	Object * object = 0;

	if (env->IsInstanceOf(source, JavaLibrary::ms_clsLocation))
	{
		// get the xyz coords
		targetLoc.x = env->GetFloatField(source, JavaLibrary::ms_fidLocationX);
		targetLoc.y = env->GetFloatField(source, JavaLibrary::ms_fidLocationY);
		targetLoc.z = env->GetFloatField(source, JavaLibrary::ms_fidLocationZ);

		// get the scene
		//@todo EAS what's the right answer here?
		JavaStringPtr sceneId = getStringField(LocalRefParam(source), JavaLibrary::ms_fidLocationArea);
		if (sceneId == JavaString::cms_nullPtr)
		{
			targetSceneId.clear();
			return false;
		}
		JavaLibrary::convert(*sceneId, targetSceneId);

		// get the cell
		LocalRefPtr cell = getObjectField(LocalRefParam(source), JavaLibrary::ms_fidLocationCell);
		targetCell = JavaLibrary::getNetworkId(*cell);
		return true;
	}
	else if (JavaLibrary::getObject(source, object))
	{
		targetLoc = object->getPosition_p();
		targetSceneId = ServerWorld::getSceneId();
		const Object * cell = ContainerInterface::getContainedByObject(*object);
		targetCell = (cell) ? cell->getNetworkId() : NetworkId::cms_invalid;
		return true;
	}
	return false;
}

const bool convertWorld(const jobject & source, Vector & targetLoc, std::string & targetSceneId)
{
	 JNIEnv * env = JavaLibrary::getEnv();
	 if (!env)
			return false;

	if (!source)
		return false;

	Object * object = 0;

	if (env->IsInstanceOf(source, JavaLibrary::ms_clsLocation))
	{
		// get the xyz coords
		targetLoc.x = env->GetFloatField(source, JavaLibrary::ms_fidLocationX);
		targetLoc.y = env->GetFloatField(source, JavaLibrary::ms_fidLocationY);
		targetLoc.z = env->GetFloatField(source, JavaLibrary::ms_fidLocationZ);

		// get the scene
		//@todo EAS what's the right answer here?
		JavaStringPtr sceneId = getStringField(LocalRefParam(source), JavaLibrary::ms_fidLocationArea);
		if (sceneId == JavaString::cms_nullPtr)
		{
			targetSceneId.clear();
			return false;
		}
		JavaLibrary::convert(*sceneId, targetSceneId);

		// get the cell
		LocalRefPtr cell = getObjectField(LocalRefParam(source), JavaLibrary::ms_fidLocationCell);
		Object * cellObject = 0;
		if (JavaLibrary::getObject(*cell, cellObject))
		{
			targetLoc = cellObject->rotateTranslate_o2w(targetLoc);
		}
		return true;
	}
	else if (JavaLibrary::getObject(source, object))
	{
		targetLoc = object->getPosition_w();
		targetSceneId = ServerWorld::getSceneId();
		return true;
	}
	return false;
}

const bool convert(const Location & sourceLoc, LocalRefPtr  & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	target = allocObject(JavaLibrary::ms_clsLocation);
	if (target == LocalRef::cms_nullPtr)
		return false;

	// set xyz
	setFloatField(*target, JavaLibrary::ms_fidLocationX, sourceLoc.getCoordinates().x);
	setFloatField(*target, JavaLibrary::ms_fidLocationY, sourceLoc.getCoordinates().y);
	setFloatField(*target, JavaLibrary::ms_fidLocationZ, sourceLoc.getCoordinates().z);
	// set scene id
	JavaString area(sourceLoc.getSceneId()); //ServerWorld::getSceneId().c_str());
	setObjectField(*target, JavaLibrary::ms_fidLocationArea, area);
	// set cell
	LocalRefPtr cell = JavaLibrary::getObjId(sourceLoc.getCell());
	setObjectField(*target, JavaLibrary::ms_fidLocationCell, *cell);
	return true;
}

const bool convert(const LocalRefParam & sourceLoc, Location & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (sourceLoc.getValue() == 0)
		return false;

	float x =  getFloatField(sourceLoc, JavaLibrary::ms_fidLocationX);
	float y =  getFloatField(sourceLoc, JavaLibrary::ms_fidLocationY);
	float z =  getFloatField(sourceLoc, JavaLibrary::ms_fidLocationZ);

	std::string planetName;
	JavaStringPtr jplanetName = getStringField(sourceLoc, JavaLibrary::ms_fidLocationArea);
	if (!JavaLibrary::convert(*jplanetName, planetName))
		return false;
	
	LocalRefPtr cell = getObjectField(sourceLoc, JavaLibrary::ms_fidLocationCell);
	NetworkId targetCell = JavaLibrary::getNetworkId(*cell);

	target.setCoordinates(Vector(x, y, z));
	target.setSceneId(planetName.c_str());
	target.setCell(targetCell);
	return true;
}

const bool convert(const LocalObjectArrayRefParam & sourceLoc, std::vector<Location> & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (sourceLoc.getValue() == 0)
		return false;

	jsize count = getArrayLength(sourceLoc);
	target.resize(count);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr location(getObjectArrayElement(sourceLoc, i));
		if (location == LocalRef::cms_nullPtr || !convert(*location, target.at(i)))
			return false;
	}
	return true;
}

const bool convert(const Vector & sourceLoc, const std::string & sourceSceneId, const NetworkId & sourceCell, LocalRefPtr & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	target = allocObject(JavaLibrary::ms_clsLocation);
	if (target == LocalRef::cms_nullPtr)
		return false;

	// set xyz
	setFloatField(*target, JavaLibrary::ms_fidLocationX, sourceLoc.x);
	setFloatField(*target, JavaLibrary::ms_fidLocationY, sourceLoc.y);
	setFloatField(*target, JavaLibrary::ms_fidLocationZ, sourceLoc.z);
	// set scene id
	JavaString area(sourceSceneId.c_str()); //ServerWorld::getSceneId().c_str());
	setObjectField(*target, JavaLibrary::ms_fidLocationArea, area);
	// set cell
	LocalRefPtr cell = JavaLibrary::getObjId(sourceCell);
	setObjectField(*target, JavaLibrary::ms_fidLocationCell, *cell);
	return true;
}

const bool convertWorld(const jobjectArray & source, std::vector<Vector> &target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (!source || !env->IsInstanceOf(source, JavaLibrary::ms_clsLocationArray)) return false;

	jsize count = env->GetArrayLength(source);
	jsize i;
	Vector v;
	NetworkId cell;
	bool result = true;
	for (i=0; i<count; ++i)
	{
		LocalRefPtr element = getObjectArrayElement(LocalObjectArrayRefParam(source), i);
		if (convertWorld(*element, v))
		{
			target.push_back(v);
		}
		else
		{
			result = false;
		}
	}

	return result && !target.empty();
}

const bool convert(const std::vector<const Vector *> & source, LocalObjectArrayRefPtr & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (source.empty())
		return false;

	int count = source.size();
	int i;

	target = createNewObjectArray(source.size(), JavaLibrary::ms_clsLocation);
	if (target == LocalObjectArrayRef::cms_nullPtr)
		return false;

	NetworkId cell(NetworkId::cms_invalid);
	bool result = true;
	for (i=0; i < count; ++i)
	{
		LocalRefPtr element;
		if (convert(*source[i], cell, element))
		{
			 setObjectArrayElement(*target, i, *element);
		}
		else
		{
			 result = false;
		}
	}

	return result;
}

const bool convert(const LocalRefParam & source, const Region * & target)
{
	return convert(source.getValue(), target);
}

/** This method converts a java script.region class into a C++ Region class
 */
const bool convert(const jobject & source, const Region * &target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (env == nullptr || source == nullptr)
		return false;
	if (!env->IsInstanceOf(source, JavaLibrary::ms_clsRegion))
		return false;

	std::string planetName;
	JavaStringPtr jplanetName = getStringField(LocalRefParam(source), JavaLibrary::ms_fidRegionPlanet);
	if (!JavaLibrary::convert(*jplanetName, planetName))
		return false;

	Unicode::String regionName;
	JavaStringPtr jregionName = getStringField(LocalRefParam(source), JavaLibrary::ms_fidRegionName);
	if (!JavaLibrary::convert(*jregionName, regionName))
		return false;

	target = RegionMaster::getRegionByName(planetName, regionName);
	return target != 0;
}

/** This method converts a C++ RegionObject class into a Java script.region class
 */
const bool convert(const Region & source, LocalRefPtr & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (!env)
		return false;

	int pvp         = source.getPvp();
	int mun         = source.getMunicipal();
	int build       = source.getBuildable();
	int geo         = source.getGeography();
	int minDiff     = source.getMinDifficulty();
	int maxDiff     = source.getMaxDifficulty();
	int spawn       = source.getSpawn();
	int mission     = source.getMission();

	const Unicode::String name(source.getName());
	JavaString jname(name);

	const std::string planetName(source.getPlanet());
	JavaString jplanetName(planetName);

	target = createNewObject(JavaLibrary::ms_clsRegion, JavaLibrary::ms_midRegion,
		jname.getValue(), pvp, build, mun, geo, minDiff, maxDiff, spawn, mission, jplanetName.getValue());
	return (target != LocalRef::cms_nullPtr);
}

// ----------------------------------------------------------------------

const bool convert(Transform const &sourceTransform, LocalRefPtr &target)
{
	JNIEnv * const env = JavaLibrary::getEnv();
	if (!env)
		return false;

	Vector const i(sourceTransform.getLocalFrameI_p());
	Vector const j(sourceTransform.getLocalFrameJ_p());
	Vector const k(sourceTransform.getLocalFrameK_p());
	Vector const p(sourceTransform.getPosition_p());
	target = createNewObject(JavaLibrary::ms_clsTransform, JavaLibrary::ms_midTransform,
		i.x, j.x, k.x, p.x,
		i.y, j.y, k.y, p.y,
		i.z, j.z, k.z, p.z);
	return (target != LocalRef::cms_nullPtr);
}

// ----------------------------------------------------------------------

const bool convert(const jobject &sourceTransform, Transform &target)
{
	JNIEnv * const env = JavaLibrary::getEnv();
	if (!env || !sourceTransform)
		return false;
	if (!env->IsInstanceOf(sourceTransform, JavaLibrary::ms_clsTransform))
		return false;

	LocalObjectArrayRefPtr matrix = getArrayObjectField(LocalRefParam(sourceTransform), JavaLibrary::ms_fidTransformMatrix);
	LocalFloatArrayRefPtr row0 = getFloatArrayArrayElement(*matrix, 0);
	LocalFloatArrayRefPtr row1 = getFloatArrayArrayElement(*matrix, 1);
	LocalFloatArrayRefPtr row2 = getFloatArrayArrayElement(*matrix, 2);

	float * const row0Elements = env->GetFloatArrayElements(row0->getValue(), 0);
	float * const row1Elements = env->GetFloatArrayElements(row1->getValue(), 0);
	float * const row2Elements = env->GetFloatArrayElements(row2->getValue(), 0);

	target.setLocalFrameIJK_p(
		Vector(row0Elements[0], row1Elements[0], row2Elements[0]),
		Vector(row0Elements[1], row1Elements[1], row2Elements[1]),
		Vector(row0Elements[2], row1Elements[2], row2Elements[2]));
	target.setPosition_p(row0Elements[3], row1Elements[3], row2Elements[3]);

	env->ReleaseFloatArrayElements(row2->getValue(), row2Elements, JNI_ABORT);
	env->ReleaseFloatArrayElements(row1->getValue(), row1Elements, JNI_ABORT);
	env->ReleaseFloatArrayElements(row0->getValue(), row0Elements, JNI_ABORT);

	return true;
}

// ----------------------------------------------------------------------

const bool convert(const LocalRefParam &sourceTransform, Transform &target)
{
	return convert(sourceTransform.getValue(), target);
}

// ----------------------------------------------------------------------

const bool convert(const jobjectArray & source, std::vector<Transform> &target)
{
	JNIEnv * const env = JavaLibrary::getEnv();
	if (!env)
		return false;

	if (!source || !env->IsInstanceOf(source, JavaLibrary::ms_clsTransformArray)) return false;

	jsize const count = env->GetArrayLength(source);
	bool result = true;
	for (jsize i = 0; i < count; ++i)
	{
		LocalRefPtr element = getObjectArrayElement(LocalObjectArrayRefParam(source), i);

		Transform t;
		if (convert(*element, t))
		{
			target.push_back(t);
		}
		else
	{
			result = false;
		}
	}

	return result && !target.empty();
	}

// ----------------------------------------------------------------------

const bool convert(const std::vector<Transform> &source, LocalObjectArrayRefPtr & target)
{
	JNIEnv * const env = JavaLibrary::getEnv();
	if (!env)
		return false;

	int const count = source.size();

	bool result = true;
	
	target = createNewObjectArray(count, JavaLibrary::ms_clsObjId);

	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr temp;
		if (convert(source[i],temp))
		{
			setObjectArrayElement(*target, i, *temp);
		}
		else
		{
			result = false;
			target = LocalObjectArrayRef::cms_nullPtr;
			break;
		}
	}
	
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		result = false;
	}

	return result;
}



// ----------------------------------------------------------------------

const bool convert(const Vector &sourceVector, LocalRefPtr &target)
{
	JNIEnv *env = JavaLibrary::getEnv();
	if (!env)
		return false;

	target = allocObject(JavaLibrary::ms_clsVector);
	if (target == LocalRef::cms_nullPtr)
		return false;

	setFloatField(*target, JavaLibrary::ms_fidVectorX, sourceVector.x);
	setFloatField(*target, JavaLibrary::ms_fidVectorY, sourceVector.y);
	setFloatField(*target, JavaLibrary::ms_fidVectorZ, sourceVector.z);
	return true;
}

// ----------------------------------------------------------------------

const bool convert(const jobject & sourceVector, Vector & target)
{
	JNIEnv *env = JavaLibrary::getEnv();
	if (!env || !sourceVector)
		return false;
	if (!env->IsInstanceOf(sourceVector, JavaLibrary::ms_clsVector))
		return false;

	target.x = env->GetFloatField(sourceVector, JavaLibrary::ms_fidVectorX);
	target.y = env->GetFloatField(sourceVector, JavaLibrary::ms_fidVectorY);
	target.z = env->GetFloatField(sourceVector, JavaLibrary::ms_fidVectorZ);
	return true;
}

// ----------------------------------------------------------------------

const bool convert(const LocalRefParam & sourceVector, Vector & target)
{
	return convert(sourceVector.getValue(), target);
}

// ----------------------------------------------------------------------

/**
 * Convert a C++ AttribMod to a Java attrib_mod.
 */
const bool convert(const AttribMod::AttribMod & source, LocalRefPtr & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (env == nullptr)
		return false;

	target = allocObject(JavaLibrary::ms_clsAttribMod);
	if (target == LocalRef::cms_nullPtr)
		return false;

	JavaString name(AttribModNameManager::getInstance(
		).getAttribModName(source.tag));
	setObjectField(*target, JavaLibrary::ms_fidAttribModName, name);
	if (!AttribMod::isSkillMod(source))
	{
		setIntField(*target, JavaLibrary::ms_fidAttribModType, source.attrib);
	}
	else
	{
		JavaString skill(AttribModNameManager::getInstance(
			).getAttribModName(source.skill));
		setObjectField(*target, JavaLibrary::ms_fidAttribModSkill, skill);
	}
	setIntField(*target, JavaLibrary::ms_fidAttribModValue,	source.value);
	setFloatField(*target, JavaLibrary::ms_fidAttribModTime, source.sustain);
	setFloatField(*target, JavaLibrary::ms_fidAttribModAttack, source.attack);
	setFloatField(*target, JavaLibrary::ms_fidAttribModDecay, source.decay);
	setIntField(*target, JavaLibrary::ms_fidAttribModFlags,	source.flags);
	return true;
}

/**
 * Convert a Java attrib_mod to a C++ AttribMod.
 */
const bool convert(const jobject & source, AttribMod::AttribMod & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (env == nullptr)
		return false;

	if (source == 0 || env->IsInstanceOf(source, JavaLibrary::ms_clsAttribMod) == JNI_FALSE)
		return false;

	std::string modName;
	JavaStringPtr name = getStringField(LocalRefParam(source), JavaLibrary::ms_fidAttribModName);
	if (name != JavaString::cms_nullPtr)
	{
		if (!JavaLibrary::convert(*name, modName))
			return false;
		
		target.tag = Crc::calculate(modName.c_str());
		AttribModNameManager::getInstance().addAttribModName(modName.c_str());
	}
	else
		target.tag = 0;
	
	target.flags = env->GetIntField(source, JavaLibrary::ms_fidAttribModFlags);
	if (!AttribMod::isSkillMod(target))
	{
		target.attrib = env->GetIntField(source, JavaLibrary::ms_fidAttribModType);
	}
	else
	{
		std::string skillModName;
		JavaStringPtr skillName = getStringField(LocalRefParam(source), JavaLibrary::ms_fidAttribModSkill);
		
		// a skillmod mod must have a skillmod name
		if (skillName == JavaString::cms_nullPtr)
			return false;
		if (!JavaLibrary::convert(*skillName, skillModName))
			return false;
		
		target.skill = Crc::calculate(skillModName.c_str());
		AttribModNameManager::getInstance().addAttribModName(skillModName.c_str());
	}
	target.value = env->GetIntField(source, JavaLibrary::ms_fidAttribModValue);
	target.sustain = env->GetFloatField(source, JavaLibrary::ms_fidAttribModTime);
	target.attack = env->GetFloatField(source, JavaLibrary::ms_fidAttribModAttack);
	target.decay = env->GetFloatField(source, JavaLibrary::ms_fidAttribModDecay);
	return true;
}

/**
 * Convert a Java attrib_mod to a C++ AttribMod.
 */
const bool convert(const LocalRefParam & source, AttribMod::AttribMod & target)
{
	return convert(source.getValue(), target);
}

/**
 * Convert a C++ AttribMod vector to a Java attrib_mod[].
 */
const bool convert(const std::vector<AttribMod::AttribMod> & source, LocalObjectArrayRefPtr & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (env == nullptr)
		return false;

	int count = source.size();
	target = createNewObjectArray(count, JavaLibrary::ms_clsAttribMod);
	if (target == LocalObjectArrayRef::cms_nullPtr)
		return false;

	LocalRefPtr mod;
	for (int i = 0; i < count; ++i)
	{
		if (!convert(source[i], mod))
		{
			target = LocalObjectArrayRef::cms_nullPtr;
			return false;
		}
		setObjectArrayElement(*target, i, *mod);
	}
	return true;
}

/**
 * Convert a Java attrib_mod[] to a C++ AttribMod vector.
 */
const bool convert(const jobjectArray & source, std::vector<AttribMod::AttribMod> & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (env == nullptr)
		return false;

	if (source == 0)
		return false;

	AttribMod::AttribMod mod;
	int count = env->GetArrayLength(source);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr element = getObjectArrayElement(LocalObjectArrayRefParam(source), i);
		bool result = convert(*element, mod);
		if (!result)
		{
			target.clear();
			return false;
		}
		target.push_back(mod);
	}
	return true;
}

const bool convert(const jbyteArray & source, std::vector<int8> & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (env == nullptr)
		return false;

	if (source == 0)
		return false;

	int size = env->GetArrayLength(source);
	target.reserve(size);
	target.resize(size);
	env->GetByteArrayRegion(source, 0, size, &target[0]);
	return true;
}

const bool convert(const LocalByteArrayRef & source, std::vector<int8> & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (env == nullptr)
		return false;

	if (source.getValue() == 0)
		return false;

	int size = getArrayLength(source);
	target.reserve(size);
	target.resize(size);
	getByteArrayRegion(source, 0, size, &target[0]);
	return true;
}

const bool convert(const std::vector<int8> & source, LocalByteArrayRefPtr & target)
{
	JNIEnv * env = JavaLibrary::getEnv();
	if (env == nullptr)
		return false;

	int count = source.size();
	target = createNewByteArray(count);
	if (target == LocalByteArrayRef::cms_nullPtr)
		return false;
	
	if (count > 0)
		setByteArrayRegion(*target, 0, count, const_cast<int8 *>(&source[0]));
	return true;
}

}//namespace ScriptConversion

//----------------------------------------------------------------------

ServerObject *  JavaLibrary::findObjectByNetworkId (const NetworkId & id)
{
	return ServerWorld::findObjectByNetworkId(id);
}

//----------------------------------------------------------------------

CreatureObject *JavaLibrary::getCreatureThrow(JNIEnv *env, jlong objId, char const *errorDescription, bool throwIfNotOnServer)
{
	NOT_NULL(env);
	
	char buffer[512];
	
	if (!objId)
	{
		IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "%s: nullptr object id from script.", errorDescription));
		buffer[sizeof(buffer) - 1] = '\0';
		throwInternalScriptError(buffer);
		return nullptr;
	}
	else
	{
		NetworkId networkId(objId);
		if (!networkId.isValid())
		{
			IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "%s: invalid or non-convertable network id [%s].", errorDescription, networkId.getValueString().c_str()));
			buffer[sizeof(buffer) - 1] = '\0';
			throwInternalScriptError(buffer);
			return nullptr;
		}
		else
		{
			Object *const object = NetworkIdManager::getObjectById(networkId);
			if (!object)
			{
				if (!throwIfNotOnServer)
				{
					// The specified object is not on this server.  It might not even be a real object.
					// Since the caller does not want this to throw, simply return nullptr.
					return nullptr;
				}

				IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "%s: object for network id [%s] not on this server.", errorDescription, networkId.getValueString().c_str()));
				buffer[sizeof(buffer) - 1] = '\0';
				throwInternalScriptError(buffer);
				return nullptr;
			}
			else
			{
				ServerObject   *const serverObject   = object->asServerObject();
				CreatureObject *const creatureObject = serverObject ? serverObject->asCreatureObject() : nullptr;

				if (creatureObject)
					return creatureObject;
				else
				{
					IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "%s: object id=[%s], template=[%s] is not a CreatureObject.", errorDescription, networkId.getValueString().c_str(), object->getObjectTemplateName()));
					buffer[sizeof(buffer) - 1] = '\0';
					throwInternalScriptError(buffer);
					return nullptr;
				}
			}
		}
	}
}

//----------------------------------------------------------------------

ShipObject *JavaLibrary::getShipThrow(JNIEnv *env, jlong objId, char const *errorDescription, bool throwIfNotOnServer)
{
	NOT_NULL(env);
	
	char buffer[512];
	
	if (objId == 0)
	{
		IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "%s: nullptr object id from script.", errorDescription));
		buffer[sizeof(buffer) - 1] = '\0';
		throwInternalScriptError(buffer);
		return nullptr;
	}
	else
	{
		NetworkId networkId(objId);
		if (!networkId.isValid())
		{
			IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "%s: invalid or non-convertable network id [%s].", errorDescription, networkId.getValueString().c_str()));
			buffer[sizeof(buffer) - 1] = '\0';
			throwInternalScriptError(buffer);
			return nullptr;
		}
		else
		{
			Object *const object = NetworkIdManager::getObjectById(networkId);
			if (!object)
			{
				if (!throwIfNotOnServer)
				{
					// The specified object is not on this server.  It might not even be a real object.
					// Since the caller does not want this to throw, simply return nullptr.
					return nullptr;
				}

				IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "%s: object for network id [%s] not on this server.", errorDescription, networkId.getValueString().c_str()));
				buffer[sizeof(buffer) - 1] = '\0';
				throwInternalScriptError(buffer);
				return nullptr;
			}
			else
			{
				ServerObject   *const serverObject   = object->asServerObject();
				ShipObject     *const shipObject     = serverObject ? serverObject->asShipObject() : nullptr;

				if (shipObject)
					return shipObject;
				else
				{
					IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "%s: object id=[%s], template=[%s] is not a ShipObject.", errorDescription, networkId.getValueString().c_str(), object->getObjectTemplateName()));
					buffer[sizeof(buffer) - 1] = '\0';
					throwInternalScriptError(buffer);
					return nullptr;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

/**
 * Causes an internal_script_error to be thrown in the Java engine.
 *
 * @param message	the message to be returned by internal_script_error.getMessage()
 */
void JavaLibrary::throwInternalScriptError(const char * message)
{
	if (ms_env != nullptr && message != nullptr)
	{
		ms_env->ThrowNew(ms_clsInternalScriptError, message);
	}
}

//-----------------------------------------------------------------------
