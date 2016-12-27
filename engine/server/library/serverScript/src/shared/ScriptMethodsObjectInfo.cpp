//========================================================================
//
// ScriptMethodsObjInfo.cpp - implements script methods dealing with general
// object info.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "UnicodeUtils.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/FactoryObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GroupObject.h"
#include "serverGame/IntangibleObject.h"
#include "serverGame/NameManager.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/NamedObjectManager.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "serverNetworkMessages/SynchronizeScriptVarDeltasMessage.h"
#include "serverScript/GameScriptObject.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedLog/Log.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueShowFlyText.h"
#include "sharedNetworkMessages/MessageQueueShowCombatText.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/VolumeContainer.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsObjectInfoNamespace
// ======================================================================

namespace ScriptMethodsObjectInfoNamespace
{
	bool                install();
	void                getGoodItemsFromContainer(const Container & container, std::vector<NetworkId> & goodItems);
	jobjectArray        getNamesFromCrcs(JNIEnv *env, const std::vector<jint> & templateCrcs);
	LocalRefPtr         createCustomVar(const jlong &objId, const std::string &variablePathName, CustomizationVariable &variable);
	LocalRefPtr         createRangedIntCustomVar(const jlong &objId, const std::string &variablePathName, RangedIntCustomizationVariable &rangedIntVariable);
	LocalRefPtr         createPalcolorCustomVar(const jlong &objId, const std::string &variablePathName, PaletteColorCustomizationVariable &variable);
	LocalRefPtr         createColor(int r, int g, int b, int a);
	CustomizationData * fetchCustomizationDataFromObjId(jlong objId);

	jboolean     JNICALL setNameFromString   (JNIEnv *env, jobject self, jlong target, jstring name);
	jboolean     JNICALL setNameFromStringId (JNIEnv *env, jobject self, jlong target, jobject nameId);
	jstring      JNICALL getName             (JNIEnv *env, jobject self, jlong target);
	jstring      JNICALL getPlayerName          (JNIEnv *env, jobject self, jlong player);
	jstring      JNICALL getPlayerFullName      (JNIEnv *env, jobject self, jlong player);
	jstring      JNICALL getAssignedName     (JNIEnv *env, jobject self, jlong target);
	void         JNICALL setCreatureName        (JNIEnv *env, jobject self, jlong creature, jstring creatureName);
	jstring      JNICALL getCreatureName        (JNIEnv *env, jobject self, jlong creature);
	jobject      JNICALL getNameStringId     (JNIEnv *env, jobject self, jlong target);
	jobject      JNICALL getNameFromTemplate (JNIEnv *env, jobject self, jstring templateName);
	jobject      JNICALL getNameFromTemplateCrc (JNIEnv *env, jobject self, jint templateCrc);
	jobjectArray JNICALL getNamesFromTemplates  (JNIEnv *env, jobject self, jobjectArray templateNames);
	jobjectArray JNICALL getNamesFromTemplateCrcs(JNIEnv *env,jobject self, jintArray templateCrcs);
	jobjectArray JNICALL getCtsDestinationClusters(JNIEnv *env, jobject self);
	jstring      JNICALL getCurrentSceneName (JNIEnv *env, jobject self);
	jstring      JNICALL getClusterName         (JNIEnv *env, jobject self);
	void         JNICALL setDescriptionStringId (JNIEnv *env, jobject self, jlong target, jobject descriptionId);
	jobject      JNICALL getDescriptionStringId (JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL internalIsAuthoritative(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL hasProxyOrAuthObject(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getGender(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isSameGender(JNIEnv *env, jobject self, jlong target1, jlong target2);
	jboolean     JNICALL isRace(JNIEnv *env, jobject self, jlong target, jint species, jint race);
	jboolean     JNICALL isSpecies(JNIEnv *env, jobject self, jlong target, jint race);
	jboolean     JNICALL isNiche(JNIEnv *env, jobject self, jlong target, jint niche);
	jboolean     JNICALL isNicheMask(JNIEnv *env, jobject self, jobject target, jint niche, jint mask);
	jint         JNICALL getNiche(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getSpecies(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getRace(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isMob(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isTangible(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isPlayer(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isAwayFromKeyBoard(JNIEnv *env, jobject self, jlong player);
	jint         JNICALL getObjType(JNIEnv *env, jobject self, jlong target);
	jstring      JNICALL getTemplateId(JNIEnv *env, jobject self, jlong target);
	jstring      JNICALL getSharedObjectTemplateName(JNIEnv *env, jobject self, jlong target);
	jlong        JNICALL getLookAtTarget(JNIEnv *env, jobject self, jlong object);
	jboolean     JNICALL setLookAtTarget(JNIEnv *env, jobject self, jlong object, jlong target);	
	jlong        JNICALL getIntendedTarget(JNIEnv *env, jobject self, jlong object);
	jint         JNICALL getPosture(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setPosture(JNIEnv *env, jobject self, jlong target, jint posture);
	jboolean     JNICALL setPostureClientImmediate(JNIEnv *env, jobject self, jlong target, jint posture);
	jint         JNICALL getLocomotion(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setLocomotion(JNIEnv *env, jobject self, jlong target, jint posture);
	jint         JNICALL getState(JNIEnv *env, jobject self, jlong target, jint whichState);
	jboolean     JNICALL setState(JNIEnv *env, jobject self, jlong target, jint whichState, jboolean value);
	jlong        JNICALL getOwner(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setOwner(JNIEnv *env, jobject self, jlong target, jlong owner);
	jboolean     JNICALL isIncapacitated(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isDead(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isNpcCreature(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isDisabled(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setCraftedId(JNIEnv *env, jobject self, jlong target, jlong id);
	jboolean     JNICALL isCrafted(JNIEnv *env, jobject self, jlong target);
	jlong        JNICALL getCrafter(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setCrafter(JNIEnv *env, jobject self, jlong target, jlong crafter);
	jfloat       JNICALL getScale(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setScale(JNIEnv *env, jobject self, jlong target, jfloat scale);
	jfloat       JNICALL getDefaultScaleFromObjectTemplate(JNIEnv *env, jobject self, jstring serverObjectTemplateName);
	jboolean     JNICALL kill(JNIEnv *env, jobject self, jlong target);
	jobject      JNICALL killPlayer(JNIEnv *env, jobject self, jobject player, jobject killer);
	jboolean     JNICALL resurrect(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL removeAllAttributeAndSkillmodMods(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setInvulnerable(JNIEnv *env, jobject self, jlong target, jboolean invulnerable);
	jboolean     JNICALL isInvulnerable(JNIEnv *env, jobject self, jlong target);
	jfloat       JNICALL getDistance(JNIEnv *env, jobject self, jlong target1, jlong target2);
	jfloat       JNICALL getLocationDistance(JNIEnv *env, jobject self, jobject loc1, jobject loc2);
	jboolean     JNICALL isLocationInConicalFrustum(JNIEnv *env, jobject self, jobject jTestLoc, jobject jStartLoc, jobject jEndLoc, jfloat startRadius, jfloat endRadius, jboolean use2d);
	jboolean     JNICALL isLocationInCone(JNIEnv *env, jobject self, jobject jTestLoc, jobject jStartLoc, jobject jDirectionLoc, jfloat range, jfloat halfAngle, jboolean use2d);
	jfloat       JNICALL getComplexity(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setComplexity(JNIEnv *env, jobject self, jlong target, jfloat complexity);
	jboolean     JNICALL isGod(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getGodLevel(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getCount(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setCount(JNIEnv *env, jobject self, jlong target, jint value);
	jboolean     JNICALL incrementCount(JNIEnv *env, jobject self, jlong target, jint delta);
	jint         JNICALL getCondition(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL hasCondition(JNIEnv *env, jobject self, jlong target, jint condition);
	jboolean     JNICALL setCondition(JNIEnv *env, jobject self, jlong target, jint condition);
	jboolean     JNICALL clearCondition(JNIEnv *env, jobject self, jlong target, jint condition);
	jboolean     JNICALL sendScriptVarsToProxies(JNIEnv * env, jobject self, jlong obj, jbyteArray buffer);
	jstring      JNICALL getAppearance(JNIEnv * env, jobject self, jlong target);
	jboolean     JNICALL isInsured(JNIEnv * env, jobject self, jlong target);
	jboolean     JNICALL isAutoInsured(JNIEnv * env, jobject self, jlong target);
	jboolean     JNICALL isUninsurable(JNIEnv * env, jobject self, jlong target);
	jlongArray   JNICALL getInventoryAndEquipment(JNIEnv * env, jobject self, jlong player);
	jboolean     JNICALL setCheaterLevel(JNIEnv * env, jobject self, jlong player, jint level);
	jint         JNICALL getCheaterLevel(JNIEnv * env, jobject self, jlong player);
	jboolean     JNICALL setHouseId(JNIEnv * env, jobject self, jlong player, jlong houseId);
	jlong        JNICALL getHouseId(JNIEnv * env, jobject self, jlong player);
	jstring      JNICALL getDraftSchematic(JNIEnv * env, jobject self, jlong manfSchematic);
	jint         JNICALL getDraftSchematicCrc(JNIEnv * env, jobject self, jlong manfSchematic);
	jint         JNICALL getSourceDraftSchematic(JNIEnv * env, jobject self, jlong object);
	jint         JNICALL getPlayerBirthDate(JNIEnv * env, jobject self, jlong playerId);
	jint         JNICALL getCurrentBirthDate(JNIEnv * env, jobject self);
	jint         JNICALL getPlayerPlayedTime(JNIEnv * env, jobject self, jlong playerId);
	jint         JNICALL getBuildingCityId(JNIEnv *env, jobject self, jlong building);
	void         JNICALL setBuildingCityId(JNIEnv *env, jobject self, jlong building, jint cityId);
	jobject      JNICALL getProductNameFromSchematic(JNIEnv *env, jobject self, jstring draftSchematic);
	jobject      JNICALL getProductNameFromSchematicCrc(JNIEnv *env, jobject self, jint draftSchematic);
	jstring      JNICALL getTemplateCreatedFromSchematic(JNIEnv *env, jobject self, jstring draftSchematic);
	jstring      JNICALL getTemplateCreatedFromSchematicCrc(JNIEnv *env, jobject self, jint draftSchematicCrc);
	jint         JNICALL getTemplateCrcCreatedFromSchematic(JNIEnv *env, jobject self, jstring draftSchematic);
	jint         JNICALL getTemplateCrcCreatedFromSchematicCrc(JNIEnv *env, jobject self, jint draftSchematicCrc);
	void         JNICALL registerNamedObject(JNIEnv *env, jobject self, jstring name, jlong namedObject);
	jlong        JNICALL getNamedObject(JNIEnv *env, jobject self, jstring name);
	void         JNICALL findObjectAnywhere(JNIEnv *env, jobject self, jlong target, jlong objectToNotify);
	void         JNICALL findObjectAnywhereByTemplate(JNIEnv *env, jobject self, jstring templateName, jlong objectToNotify);
	void         JNICALL findPlayerAnywhereByPartialName(JNIEnv *env, jobject self, jstring targetName, jlong objectToNotify);
	void         JNICALL findWardenAnywhere(JNIEnv *env, jobject self, jlong objectToNotify);
	void         JNICALL findCreatureAnywhere(JNIEnv *env, jobject self, jstring creatureName, jlong objectToNotify);
	void         JNICALL requestPreloadCompleteTrigger(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL canTrade(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isNoTradeShared(JNIEnv *env, jobject self, jlong target);
	jlong        JNICALL getLastSpawnedTheater(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL setBioLink(JNIEnv *env, jobject self, jlong target, jlong link);
	jboolean     JNICALL clearBioLink(JNIEnv *env, jobject self, jlong target);
	jlong        JNICALL getBioLink(JNIEnv *env, jobject self, jlong target);
	float        JNICALL getObjectCollisionRadius(JNIEnv * env, jobject self, jlong obj);
	jint         JNICALL getGameObjectType(JNIEnv *env, jobject self, jlong obj);
	jint         JNICALL getGameObjectTypeFromTemplate(JNIEnv *env, jobject self, jstring templateName);
	jint         JNICALL getGameObjectTypeFromTemplateCrc(JNIEnv *env, jobject self, jint templateCrc);
	jstring      JNICALL getGameObjectTypeName(JNIEnv *env, jobject self, jint type);
	jint         JNICALL getGameObjectTypeFromName(JNIEnv *env, jobject self, jstring typeName);
	jint         JNICALL getVolume(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isFacing(JNIEnv *env, jobject self, jlong mob, jobject target);
	jboolean     JNICALL exists(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isInWorld(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isInWorldCell(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL areFacingEachOther(JNIEnv *env, jobject self, jlong mob1, jlong mob2);
	void         JNICALL watch(JNIEnv * env, jobject self, jlong target);
	void         JNICALL stopWatching(JNIEnv * env, jobject self, jlong target);
	jobjectArray JNICALL getAllCustomVars(JNIEnv *env, jclass selfClass, jlong target);
	jobject      JNICALL getCustomVarByName(JNIEnv *env, jclass selfClass, jlong target, jstring varPathName);
	jint         JNICALL getRangedIntCustomVarValue(JNIEnv *env, jclass selfClass, jlong target, jstring varPathName);
	void         JNICALL setRangedIntCustomVarValue(JNIEnv *env, jclass selfClass, jlong target, jstring varPathName, jint newValue);
	jobject      JNICALL getPalcolorCustomVarSelectedColor(JNIEnv *env, jclass selfClass, jlong target, jstring varPathName);
	void         JNICALL setPalcolorCustomVarClosestColor(JNIEnv *env, jclass selfClass, jlong target, jstring varPathName, jint r, jint g, jint b, jint a);
	jobjectArray JNICALL getPalcolorCustomVarColors(JNIEnv *env, jclass selfClass, jlong target, jstring varPathName);
	jlong        JNICALL getGroupObject(JNIEnv *env, jobject self, jlong obj);
	jstring      JNICALL getGroupName(JNIEnv *env, jobject self, jlong obj);
	jlongArray   JNICALL getGroupMemberIds(JNIEnv *env, jobject self, jlong obj);
	jobjectArray JNICALL getGroupMemberNames(JNIEnv *env, jobject self, jlong obj);
	jlong        JNICALL getGroupLeaderId(JNIEnv *env, jobject self, jlong group);
	jint         JNICALL getGroupSize(JNIEnv *env, jobject self, jlong group);
	jint         JNICALL getPCGroupSize(JNIEnv *env, jobject self, jlong group);
	jlong        JNICALL getGroupMasterLooterId(JNIEnv *env, jobject self, jlong group);
	jint         JNICALL getGroupLootRule(JNIEnv *env, jobject self, jlong group);
	jboolean     JNICALL setGroupLootRule(JNIEnv *env, jobject self, jlong target, jint value);
	void         JNICALL openLotteryWindow(JNIEnv *env, jobject self, jlong playerToSendTo, jlong container);
	jint         JNICALL getPerformanceType(JNIEnv *env, jobject self, jlong target);
	void         JNICALL setPerformanceType(JNIEnv *env, jobject self, jlong target, jint performanceType);
	jint         JNICALL getPerformanceStartTime(JNIEnv *env, jobject self, jlong target);
	void         JNICALL setPerformanceStartTime(JNIEnv *env, jobject self, jlong target, jint performanceStartTime);
	jlong        JNICALL getPerformanceListenTarget(JNIEnv *env, jobject self, jlong actor);
	void         JNICALL setPerformanceListenTarget(JNIEnv *env, jobject self, jlong actor, jlong target);
	jlong        JNICALL getPerformanceWatchTarget(JNIEnv *env, jobject self, jlong actor);
	void         JNICALL setPerformanceWatchTarget(JNIEnv *env, jobject self, jlong actor, jlong target);
	jint         JNICALL getInstrumentVisualId(JNIEnv *env, jobject self, jlong who);
	jint         JNICALL getInstrumentAudioId(JNIEnv *env, jobject self, jlong who);
	void         JNICALL sendMusicFlourish(JNIEnv *env, jobject self, jlong performer, jint flourishIndex);
	void         JNICALL showFlyText           (JNIEnv *env, jobject self, jlong emitterId, jobject outputTextId, jfloat scale, jint r, jint g, jint b);
	void         JNICALL showFlyTextPrivate    (JNIEnv *env, jobject self, jlong emitterId, jlong receiverId, jobject outputTextId, jfloat scale, jint r, jint g, jint b, jboolean showInChatBox);
	void         JNICALL showCombatText        (JNIEnv *env, jobject self, jlong defenderId, jlong attackerId, jobject outputTextId, jfloat scale, jint r, jint g, jint b);
	void         JNICALL showCombatTextPrivate (JNIEnv *env, jobject self, jlong defenderId, jlong attackerId, jlong emitterId, jobject outputTextId, jfloat scale, jint r, jint g, jint b);
	void         JNICALL showFlyTextProse           (JNIEnv *env, jobject self, jlong emitterId, jstring outputTextOOB, jfloat scale, jint r, jint g, jint b);
	void         JNICALL showFlyTextPrivateProseWithFlags  (JNIEnv *env, jobject self, jlong emitterId, jlong receiverId, jstring outputTextOOB, jfloat scale, jint r, jint g, jint b, jint flags);
	void         JNICALL showFlyTextPrivateProse    (JNIEnv *env, jobject self, jlong emitterId, jlong receiverId, jstring outputTextOOB, jfloat scale, jint r, jint g, jint b, jboolean showInChatBox);
	void         JNICALL showCombatTextProse        (JNIEnv *env, jobject self, jlong defenderId, jlong attackerId, jstring outputTextOOB, jfloat scale, jint r, jint g, jint b);
	void         JNICALL showCombatTextPrivateProse (JNIEnv *env, jobject self, jlong defenderId, jlong attackerId, jlong emitterId, jstring outputTextOOB, jfloat scale, jint r, jint g, jint b);
	jlong        JNICALL getMaster          (JNIEnv *env, jobject self, jlong target);
	void         JNICALL setMaster          (JNIEnv *env, jobject self, jlong target, jlong master);
	jint         JNICALL getNumAI(JNIEnv *env, jobject self);
	jint         JNICALL getNumCreatures(JNIEnv *env, jobject self);
	jint         JNICALL getNumPlayers(JNIEnv *env, jobject self);
	jint         JNICALL getNumRunTimeRules(JNIEnv *env, jobject self);
	jint         JNICALL getNumDynamicAI(JNIEnv *env, jobject self);
	jint         JNICALL getNumStaticAI(JNIEnv *env, jobject self);
	jint         JNICALL getNumCombatAI(JNIEnv *env, jobject self);
	jint         JNICALL getNumHibernatingAI(JNIEnv *env, jobject self);
	jint         JNICALL getGroupLevel      (JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getLevel(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setLevel(JNIEnv *env, jobject self, jlong target, jint forcedLevel);
	jboolean     JNICALL recalculateLevel(JNIEnv *env, jobject self, jlong target);
	void         JNICALL sendDirtyObjectMenuNotification    (JNIEnv *env, jobject self, jlong jTarget);
	void         JNICALL sendDirtyAttributesNotification    (JNIEnv *env, jobject self, jlong jTarget);
	void         JNICALL activateQuest(JNIEnv * env, jobject self, jlong target, jint questId);
	void         JNICALL completeQuest(JNIEnv * env, jobject self, jlong target, jint questId);
	void         JNICALL clearCompletedQuest(JNIEnv * env, jobject self, jlong target, jint questId);
	void         JNICALL deactivateQuest(JNIEnv * env, jobject self, jlong target, jint questId);
	jboolean     JNICALL isQuestActive(JNIEnv * env, jobject self, jlong target, jint questId);
	jboolean     JNICALL isQuestComplete(JNIEnv * env, jobject self, jlong target, jint questId);
	jbyteArray   JNICALL getByteStreamFromAutoVariable(JNIEnv * env, jobject self, jlong target, jstring variableName);
	void         JNICALL setAutoVariableFromByteStream(JNIEnv * env, jobject self, jlong target, jstring variableName, jbyteArray data);
	jstring      JNICALL getStaticItemName(JNIEnv * env, jobject self, jlong target);
	jint         JNICALL getStaticItemVersion(JNIEnv * env, jobject self, jlong target);
	void         JNICALL setStaticItemName(JNIEnv * env, jobject self, jlong target, jstring staticItemName);
	void         JNICALL setStaticItemVersion(JNIEnv * env, jobject self, jlong target, jint staticItemVersion);
	jint         JNICALL getConversionId(JNIEnv * env, jobject self, jlong target);
	void         JNICALL setConversionId(JNIEnv * env, jobject self, jlong target, jint conversionId);
	jboolean     JNICALL canEquipWearable(JNIEnv * env, jobject self, jlong player, jlong target);
	void         JNICALL openCustomizationWindow(JNIEnv *env, jobject self, jlong player, jlong object, jstring customVarName1, jint minVar1, jint maxVar1, jstring customVarName2, jint minVar2, jint maxVar2, jstring customVarName3, jint minVar3, jint maxVar3, jstring customVarName4, jint minVar4, jint maxVar4);
	jint         JNICALL getHologramType(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setHologramType(JNIEnv *env, jobject self, jlong target, jint type);
	jboolean     JNICALL setVisibleOnMapAndRadar(JNIEnv *env, jobject self, jlong target, jboolean visible);
	jboolean     JNICALL getVisibleOnMapAndRadar(JNIEnv *env, jobject self, jlong target);
	jlong        JNICALL getBeastmasterPet(JNIEnv *env, jobject self, jlong object);
	jboolean     JNICALL setBeastmasterPet(JNIEnv *env, jobject self, jlong object, jlong target);
	jboolean	 JNICALL isPlayerBackpackHidden(JNIEnv *env, jobject self, jlong player);
	jboolean	 JNICALL isPlayerHelmetHidden(JNIEnv *env, jobject self, jlong player);
	jfloat       JNICALL getDefaultScaleFromSharedObjectTemplate(JNIEnv *env, jobject self, jstring sharedObjectTemplateName);
	jboolean     JNICALL setOverrideMapColor(JNIEnv * env, jobject self, jlong object, jint r, jint g, jint b);
	jboolean     JNICALL clearOverrideMapColor(JNIEnv * env, jobject self, jlong object);
	jobject      JNICALL getOverrideMapColor(JNIEnv * env, jobject self, jlong object);
	jboolean     JNICALL setForceShowHam(JNIEnv * env, jobject self, jlong object, jboolean show);
	jboolean     JNICALL isContainedByPlayerAppearanceInventory(JNIEnv *env, jobject self, jlong player, jlong item);
	jlongArray   JNICALL getAllItemsFromAppearanceInventory(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL isAPlayerAppearanceInventoryContainer(JNIEnv *env, jobject self, jlong container);
	jlongArray   JNICALL getAllWornItems(JNIEnv *env, jobject self, jlong player, jboolean ignoreAppearanceItems);
	jlong        JNICALL getAppearanceInventory(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL setDecoyOrigin(JNIEnv *env, jobject self, jlong creature, jlong origin);
	jlong        JNICALL getDecoyOrigin(JNIEnv *env, jobject self, jlong creature);
	jboolean     JNICALL openRatingWindow(JNIEnv * env, jobject self, jlong player, jstring title, jstring description);
	void         JNICALL openExamineWindow(JNIEnv * env, jobject self, jlong player, jlong item);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsObjectInfoNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsObjectInfoNamespace::c)}
	JF("_setName",         "(JLjava/lang/String;)Z", setNameFromString),
	JF("_setName",         "(JLscript/string_id;)Z", setNameFromStringId),
	JF("_getName",         "(J)Ljava/lang/String;",  getName),
	JF("_getPlayerName",   "(J)Ljava/lang/String;",  getPlayerName),
	JF("_getPlayerFullName", "(J)Ljava/lang/String;",  getPlayerFullName),
	JF("_getAssignedName", "(J)Ljava/lang/String;",  getAssignedName),
	JF("_setCreatureName", "(JLjava/lang/String;)V",  setCreatureName),
	JF("_getCreatureName", "(J)Ljava/lang/String;",  getCreatureName),
	JF("_getNameStringId", "(J)Lscript/string_id;",  getNameStringId),
	JF("getNameFromTemplate", "(Ljava/lang/String;)Lscript/string_id;", getNameFromTemplate),
	JF("getNameFromTemplate", "(I)Lscript/string_id;", getNameFromTemplateCrc),
	JF("getNamesFromTemplates", "([Ljava/lang/String;)[Lscript/string_id;", getNamesFromTemplates),
	JF("getNamesFromTemplates", "([I)[Lscript/string_id;", getNamesFromTemplateCrcs),
	JF("_setDescriptionStringId", "(JLscript/string_id;)V", setDescriptionStringId),
	JF("_getDescriptionStringId", "(J)Lscript/string_id;",  getDescriptionStringId),
	JF("__internalIsAuthoritative", "(J)Z", internalIsAuthoritative),
	JF("_hasProxyOrAuthObject", "(J)Z", hasProxyOrAuthObject),
	JF("_getGender", "(J)I", getGender),
	JF("_isSameGender", "(JJ)Z", isSameGender),
	JF("_isRace", "(JII)Z", isRace),
	JF("_isSpecies", "(JI)Z", isSpecies),
	JF("_isNiche", "(JI)Z", isNiche),
	JF("_getRace", "(J)I", getRace),
	JF("_getNiche", "(J)I", getNiche),
	JF("_getSpecies", "(J)I", getSpecies),
	JF("_isMob", "(J)Z", isMob),
	JF("_isTangible", "(J)Z", isTangible),
	JF("_isPlayer", "(J)Z", isPlayer),
	JF("_isAwayFromKeyBoard", "(J)Z", isAwayFromKeyBoard),
	JF("_getObjType", "(J)I", getObjType),
	JF("_getLookAtTarget", "(J)J", getLookAtTarget),
	JF("_setLookAtTarget", "(JJ)Z", setLookAtTarget),	
	JF("_getIntendedTarget", "(J)J", getIntendedTarget),
	JF("_getPosture", "(J)I", getPosture),
	JF("_setPosture", "(JI)Z", setPosture),
	JF("_setPostureClientImmediate", "(JI)Z", setPostureClientImmediate),
	JF("_getLocomotion", "(J)I", getLocomotion),
	JF("_setLocomotion", "(JI)Z", setLocomotion),
	JF("_getState", "(JI)I", getState),
	JF("_setState", "(JIZ)Z", setState),
	JF("_getOwner", "(J)J", getOwner),
	JF("_setOwner", "(JJ)Z", setOwner),
	JF("_isIncapacitated", "(J)Z", isIncapacitated),
	JF("_isDead", "(J)Z", isDead),
	JF("_isNpcCreature", "(J)Z", isNpcCreature),
	JF("_isDisabled", "(J)Z", isDisabled),
	JF("_setCraftedId", "(JJ)Z", setCraftedId),
	JF("_isCrafted", "(J)Z", isCrafted),
	JF("_getCrafter", "(J)J", getCrafter),
	JF("_setCrafter", "(JJ)Z", setCrafter),
	JF("_getScale", "(J)F", getScale),
	JF("_setScale", "(JF)Z", setScale),
	JF("getDefaultScaleFromObjectTemplate", "(Ljava/lang/String;)F", getDefaultScaleFromObjectTemplate),
	JF("_getTemplateName", "(J)Ljava/lang/String;", getTemplateId),
	JF("_getSharedObjectTemplateName", "(J)Ljava/lang/String;", getSharedObjectTemplateName),
	JF("_kill", "(J)Z", kill),
	JF("_resurrect", "(J)Z", resurrect),
	JF("_removeAllAttributeAndSkillmodMods", "(J)Z", removeAllAttributeAndSkillmodMods),
	JF("_getDistance", "(JJ)F",getDistance),
	JF("_getDistance", "(Lscript/location;Lscript/location;)F", getLocationDistance),
	JF("_isLocationWithinConicalFrustum", "(Lscript/location;Lscript/location;Lscript/location;FFZ)Z", isLocationInConicalFrustum),
	JF("_isLocationInCone", "(Lscript/location;Lscript/location;Lscript/location;FFZ)Z", isLocationInCone),
	JF("_setInvulnerable", "(JZ)Z", setInvulnerable),
	JF("_isInvulnerable", "(J)Z", isInvulnerable),
	JF("_getComplexity", "(J)F", getComplexity),
	JF("_setComplexity", "(JF)Z", setComplexity),
	JF("_isGod", "(J)Z", isGod),
	JF("_getGodLevel", "(J)I", getGodLevel),
	JF("_getCount", "(J)I", getCount),
	JF("_setCount", "(JI)Z", setCount),
	JF("_incrementCount", "(JI)Z", incrementCount),
	JF("_getCondition", "(J)I", getCondition),
	JF("_hasCondition", "(JI)Z", hasCondition),
	JF("_setCondition", "(JI)Z", setCondition),
	JF("_clearCondition", "(JI)Z", clearCondition),
	JF("_getAppearance", "(J)Ljava/lang/String;", getAppearance),
	JF("_isInsured", "(J)Z", isInsured),
	JF("_isAutoInsured", "(J)Z", isAutoInsured),
	JF("_isUninsurable", "(J)Z", isUninsurable),
	JF("_getInventoryAndEquipment", "(J)[J", getInventoryAndEquipment),
	JF("_setCheaterLevel", "(JI)Z", setCheaterLevel),
	JF("_getCheaterLevel", "(J)I", getCheaterLevel),
	JF("_setHouseId", "(JJ)Z", setHouseId),
	JF("_getHouseId", "(J)J", getHouseId),
	JF("_getDraftSchematic", "(J)Ljava/lang/String;", getDraftSchematic),
	JF("_getDraftSchematicCrc", "(J)I", getDraftSchematicCrc),
	JF("_getSourceDraftSchematic", "(J)I", getSourceDraftSchematic),
	JF("_getPlayerBirthDate", "(J)I", getPlayerBirthDate),
	JF("getCurrentBirthDate", "()I", getCurrentBirthDate),
	JF("_getPlayerPlayedTime", "(J)I", getPlayerPlayedTime),
	JF("_getBuildingCityId", "(J)I", getBuildingCityId),
	JF("_setBuildingCityId", "(JI)V", setBuildingCityId),
	JF("getProductNameFromSchematic", "(Ljava/lang/String;)Lscript/string_id;", getProductNameFromSchematic),
	JF("getProductNameFromSchematic", "(I)Lscript/string_id;", getProductNameFromSchematicCrc),
	JF("getTemplateCreatedFromSchematic", "(Ljava/lang/String;)Ljava/lang/String;", getTemplateCreatedFromSchematic),
	JF("getTemplateCreatedFromSchematic", "(I)Ljava/lang/String;", getTemplateCreatedFromSchematicCrc),
	JF("getTemplateCrcCreatedFromSchematic", "(Ljava/lang/String;)I", getTemplateCrcCreatedFromSchematic),
	JF("getTemplateCrcCreatedFromSchematic", "(I)I", getTemplateCrcCreatedFromSchematicCrc),
	JF("_registerNamedObject", "(Ljava/lang/String;J)V", registerNamedObject),
	JF("_getNamedObject", "(Ljava/lang/String;)J", getNamedObject),
	JF("_findObjectAnywhere", "(JJ)V", findObjectAnywhere),
	JF("_findObjectAnywhereByTemplate", "(Ljava/lang/String;J)V", findObjectAnywhereByTemplate),
	JF("_findPlayerAnywhereByPartialName", "(Ljava/lang/String;J)V", findPlayerAnywhereByPartialName),
	JF("_findWardenAnywhere", "(J)V", findWardenAnywhere),
	JF("_findCreatureAnywhere", "(Ljava/lang/String;J)V", findCreatureAnywhere),
	JF("_requestPreloadCompleteTrigger", "(J)V", requestPreloadCompleteTrigger),
	JF("_canTrade", "(J)Z", canTrade),
	JF("_isNoTradeShared", "(J)Z", isNoTradeShared),
	JF("_getLastSpawnedTheater", "(J)J", getLastSpawnedTheater),
	JF("_setBioLink", "(JJ)Z", setBioLink),
	JF("_clearBioLink", "(J)Z", clearBioLink),
	JF("_getBioLink", "(J)J", getBioLink),
	JF("_getObjectCollisionRadius",                     "(J)F", getObjectCollisionRadius),
	JF("_getGameObjectType",     "(J)I",    getGameObjectType),
	JF("getGameObjectTypeFromTemplate", "(Ljava/lang/String;)I", getGameObjectTypeFromTemplate),
	JF("getGameObjectTypeFromTemplate", "(I)I", getGameObjectTypeFromTemplateCrc),
	JF("getGameObjectTypeName", "(I)Ljava/lang/String;", getGameObjectTypeName),
	JF("getGameObjectTypeFromName", "(Ljava/lang/String;)I", getGameObjectTypeFromName),
	JF("_getVolume", "(J)I",getVolume),
	JF("_isFacing", "(JLscript/location;)Z", isFacing),
	JF("_exists", "(J)Z", exists),
	JF("_isInWorld", "(J)Z",isInWorld),
	JF("_isInWorldCell", "(J)Z", isInWorldCell),
	JF("_areFacingEachOther", "(JJ)Z",areFacingEachOther),
	JF("_watch",        "(J)V", watch),
	JF("_stopWatching", "(J)V", stopWatching),
	JF("_getAllCustomVars",                 "(J)[Lscript/custom_var;",                                   getAllCustomVars),
	JF("_getCustomVarByName",               "(JLjava/lang/String;)Lscript/custom_var;",                  getCustomVarByName),
	JF("_getRangedIntCustomVarValue",       "(JLjava/lang/String;)I",                             getRangedIntCustomVarValue),
	JF("_setRangedIntCustomVarValue",       "(JLjava/lang/String;I)V",                            setRangedIntCustomVarValue),
	JF("_getPalcolorCustomVarSelectedColor","(JLjava/lang/String;)Lscript/color;",                       getPalcolorCustomVarSelectedColor),
	JF("_setPalcolorCustomVarClosestColor", "(JLjava/lang/String;IIII)V",                         setPalcolorCustomVarClosestColor),
	JF("_getPalcolorCustomVarColors",       "(JLjava/lang/String;)[Lscript/color;",                      getPalcolorCustomVarColors),
	JF("_getGroupObject", "(J)J", getGroupObject),
	JF("_getGroupName", "(J)Ljava/lang/String;", getGroupName),
	JF("_getGroupMemberIds", "(J)[J", getGroupMemberIds),
	JF("_getGroupMemberNames", "(J)[Ljava/lang/String;", getGroupMemberNames),
	JF("_getGroupLeaderId", "(J)J", getGroupLeaderId),
	JF("_getGroupSize", "(J)I", getGroupSize),
	JF("_getPCGroupSize", "(J)I", getPCGroupSize),
	JF("_getGroupMasterLooterId", "(J)J", getGroupMasterLooterId),
	JF("_getGroupLootRule", "(J)I", getGroupLootRule),
	JF("_setGroupLootRule", "(JI)Z", setGroupLootRule),
	JF("_openLotteryWindow", "(JJ)V", openLotteryWindow),
	JF("_getPerformanceType", "(J)I", getPerformanceType),
	JF("_setPerformanceType", "(JI)V", setPerformanceType),
	JF("_getPerformanceStartTime", "(J)I", getPerformanceStartTime),
	JF("_setPerformanceStartTime", "(JI)V", setPerformanceStartTime),
	JF("_getPerformanceListenTarget", "(J)J", getPerformanceListenTarget),
	JF("_setPerformanceListenTarget", "(JJ)V", setPerformanceListenTarget),
	JF("_getPerformanceWatchTarget", "(J)J", getPerformanceWatchTarget),
	JF("_setPerformanceWatchTarget", "(JJ)V", setPerformanceWatchTarget),
	JF("_getInstrumentVisualId", "(J)I", getInstrumentVisualId),
	JF("_getInstrumentAudioId", "(J)I", getInstrumentAudioId),
	JF("_sendMusicFlourish", "(JI)V", sendMusicFlourish),
	JF("_showFlyText", "(JLscript/string_id;FIII)V", showFlyText),
	JF("_showFlyTextPrivate", "(JJLscript/string_id;FIIIZ)V", showFlyTextPrivate),
	JF("_showFlyTextProse", "(JLjava/lang/String;FIII)V", showFlyTextProse),
	JF("_showFlyTextPrivateProse", "(JJLjava/lang/String;FIIIZ)V", showFlyTextPrivateProse),
	JF("_showCombatText", "(JJLscript/string_id;FIII)V", showCombatText),
	JF("_showCombatTextPrivate", "(JJJLscript/string_id;FIII)V", showCombatTextPrivate),
	JF("_showCombatTextProse", "(JJLjava/lang/String;FIII)V", showCombatTextProse),
	JF("_showCombatTextPrivateProse", "(JJJLjava/lang/String;FIII)V", showCombatTextPrivateProse),
	JF("_showFlyTextPrivateProseWithFlags", "(JJLjava/lang/String;FIIII)V", showFlyTextPrivateProseWithFlags),
	JF("_getMaster", "(J)J",          getMaster),
	JF("_setMaster", "(JJ)V",         setMaster),
	JF("getNumAI", "()I", getNumAI),
	JF("getNumCreatures", "()I", getNumCreatures),
	JF("getNumPlayers", "()I", getNumPlayers),
	JF("getNumRunTimeRules", "()I", getNumRunTimeRules),
	JF("getNumDynamicAI", "()I", getNumDynamicAI),
	JF("getNumStaticAI", "()I", getNumStaticAI),
	JF("getNumCombatAI", "()I", getNumCombatAI),
	JF("getNumHibernatingAI", "()I", getNumHibernatingAI),
	JF("_getGroupObjectLevel",    "(J)I",           getGroupLevel),
	JF("_getLevel", "(J)I", getLevel),
	JF("_setLevel", "(JI)Z", setLevel),
	JF("_recalculateLevel", "(J)Z", recalculateLevel),
	JF("_sendDirtyObjectMenuNotification",    "(J)V",     sendDirtyObjectMenuNotification),
	JF("_sendDirtyAttributesNotification",    "(J)V",     sendDirtyAttributesNotification),
	JF("_activateQuest", "(JI)V", activateQuest),
	JF("_completeQuest", "(JI)V", completeQuest),
	JF("_clearCompletedQuest", "(JI)V", clearCompletedQuest),
	JF("_deactivateQuest", "(JI)V", deactivateQuest),
	JF("_isQuestActive", "(JI)Z", isQuestActive),
	JF("_isQuestComplete", "(JI)Z", isQuestComplete),
	JF("_getByteStreamFromAutoVariable", "(JLjava/lang/String;)[B", getByteStreamFromAutoVariable),
	JF("_setAutoVariableFromByteStream", "(JLjava/lang/String;[B)V", setAutoVariableFromByteStream),
	JF("_getStaticItemName", "(J)Ljava/lang/String;", getStaticItemName),
	JF("_getStaticItemVersion", "(J)I", getStaticItemVersion),
	JF("_setStaticItemName", "(JLjava/lang/String;)V", setStaticItemName),
	JF("_setStaticItemVersion", "(JI)V", setStaticItemVersion),
	JF("_getConversionId", "(J)I", getConversionId),
	JF("_setConversionId", "(JI)V", setConversionId),
	JF("getCtsDestinationClusters", "()[Ljava/lang/String;", getCtsDestinationClusters),
	JF("getCurrentSceneName", "()Ljava/lang/String;", getCurrentSceneName),
	JF("getClusterName", "()Ljava/lang/String;", getClusterName),
	JF("_sendScriptVarsToProxies", "(J[B)Z", sendScriptVarsToProxies),
	JF("_canEquipWearable", "(JJ)Z", canEquipWearable),
	JF("_openCustomizationWindow", "(JJLjava/lang/String;IILjava/lang/String;IILjava/lang/String;IILjava/lang/String;II)V", openCustomizationWindow),
	JF("_getHologramType", "(J)I", getHologramType),
	JF("_setHologramType", "(JI)Z", setHologramType),
	JF("_setVisibleOnMapAndRadar", "(JZ)Z", setVisibleOnMapAndRadar),
	JF("_getVisibleOnMapAndRadar", "(J)Z", getVisibleOnMapAndRadar),
	JF("_getBeastmasterPet", "(J)J", getBeastmasterPet),
	JF("_setBeastmasterPet", "(JJ)Z", setBeastmasterPet),
	JF("_isPlayerBackpackHidden", "(J)Z", isPlayerBackpackHidden),
	JF("_isPlayerHelmetHidden", "(J)Z", isPlayerHelmetHidden),
	JF("getDefaultScaleFromSharedObjectTemplate", "(Ljava/lang/String;)F", getDefaultScaleFromSharedObjectTemplate),
	JF("_setOverrideMapColor", "(JIII)Z", setOverrideMapColor),
	JF("_clearOverrideMapColor", "(J)Z", clearOverrideMapColor),
	JF("_getOverrideMapColor", "(J)Lscript/color;", getOverrideMapColor),
	JF("_setForceShowHam", "(JZ)Z", setForceShowHam),
	JF("_isContainedByPlayerAppearanceInventory", "(JJ)Z", isContainedByPlayerAppearanceInventory),
	JF("_getAllItemsFromAppearanceInventory", "(J)[J", getAllItemsFromAppearanceInventory),
	JF("_isAPlayerAppearanceInventoryContainer", "(J)Z", isAPlayerAppearanceInventoryContainer),
	JF("_getAllWornItems", "(JZ)[J", getAllWornItems),
	JF("_getAppearanceInventory", "(J)J", getAppearanceInventory),
	JF("_setDecoyOrigin", "(JJ)Z", setDecoyOrigin),
	JF("_getDecoyOrigin", "(J)J", getDecoyOrigin),
	JF("_openRatingWindow", "(JLjava/lang/String;Ljava/lang/String;)Z", openRatingWindow),
	JF("_openExamineWindow", "(JJ)V", openExamineWindow),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary internal methods
//========================================================================

/**
 * Goes through all the items in a container and finds the "good" ones. A good
 * item must be a visible TangibleObject, and not a special item such as hair.
 * If a good item is a container itself, we will recursively go through its
 * contents.
 *
 * @param container		the container to look through
 * @param goodItems		list that will be filled with the good items
 */
void ScriptMethodsObjectInfoNamespace::getGoodItemsFromContainer(const Container & container,
	std::vector<NetworkId> & goodItems)
{
	const ServerObject * owner = safe_cast<const ServerObject *>(
		ContainerInterface::getFirstParentInWorld(container.getOwner()));
	const ServerObject * ownerInventory = nullptr;
	const ServerObject * ownerDatapad = nullptr;
	const ServerObject * ownerAppearanceInventory = nullptr;
	const ServerObject * ownerHangar = nullptr;

	if (owner != nullptr)
	{
		const CreatureObject * creature =  owner->asCreatureObject();
		if (creature != nullptr)
		{
			ownerInventory = creature->getInventory();
			ownerDatapad = creature->getDatapad();
			ownerAppearanceInventory = creature->getAppearanceInventory();
			ownerHangar = creature->getHangar();
		}
	}

	for (ContainerConstIterator i(container.begin()); i != container.end(); ++i)
	{
		const CachedNetworkId & itemId = *i;
		const ServerObject * item = safe_cast<const ServerObject *>(
			itemId.getObject());
		if (item != nullptr && item->asTangibleObject() != nullptr &&
			item->asTangibleObject()->isVisible())
		{
			//
			// test for special items
			//

			// no player inventory
			if (ownerInventory != nullptr && ownerInventory->getNetworkId() ==
				item->getNetworkId())
			{
				continue;
			}

			// no player datapad
			if (ownerDatapad != nullptr && ownerDatapad->getNetworkId() ==
				item->getNetworkId())
			{
				continue;
			}

			// no player appearance inventories!
			if(ownerAppearanceInventory && ownerAppearanceInventory->getNetworkId() == item->getNetworkId())
				continue;

			// TCG Ship hangar
			if(ownerHangar && ownerHangar->getNetworkId() == item->getNetworkId())
				continue;

			// no creatures (pets/droids)
			if (item->asCreatureObject() != nullptr)
				continue;

			// no hair
			const ServerObjectTemplate * itemTemplate = safe_cast<
				const ServerObjectTemplate *>(item->getObjectTemplate());
			NOT_NULL(itemTemplate);
			if (strstr(itemTemplate->getName(), "tangible/hair") != nullptr)
				continue;

			// no Trandoshan feet



			goodItems.push_back(itemId);

			// see if the item is a container and go through its contents
			const Container * itemContainer = ContainerInterface::getContainer(*item);
			if (itemContainer != nullptr)
				getGoodItemsFromContainer(*itemContainer, goodItems);
		}
	}
}	// JavaLibrary::getGoodItemsFromContainer

/**
 * Returns the default names of objects that would be created by given templates.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param jtemplateCrcs		the template crcs
 *
 * @return the names, or nullptr on error
 */
jobjectArray ScriptMethodsObjectInfoNamespace::getNamesFromCrcs(JNIEnv *env,
	const std::vector<jint> & templateCrcs)
{
	int count = templateCrcs.size();
	LocalObjectArrayRefPtr names = createNewObjectArray(count, JavaLibrary::getClsStringId());
	if (names == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	for (int i = 0; i < count; ++i)
	{
		if (templateCrcs[i] == 0)
			continue;

		// get the server template
		const ObjectTemplate * ot = ObjectTemplateList::fetch(templateCrcs[i]);
		if (ot == nullptr)
		{
			WARNING(true, ("JavaLibrary::getNamesFromTemplateCrcs: Could not "
				"find object template for crc %d", templateCrcs[i]));
			continue;
		}

		// get the shared template
		const ServerObjectTemplate * serverOt = ot->asServerObjectTemplate();
		if (serverOt != nullptr)
		{
			const std::string sharedTemplateName(serverOt->getSharedTemplate());
			serverOt->releaseReference();
			serverOt = nullptr;
			ot = ObjectTemplateList::fetch(sharedTemplateName);
			if (ot == nullptr)
			{
				WARNING(true, ("JavaLibrary::getNamesFromTemplateCrcs: Could not "
					"find shared object template %s", sharedTemplateName.c_str()));
				continue;
			}
		}

		const SharedObjectTemplate * sharedOt = ot->asSharedObjectTemplate();
		if (sharedOt == nullptr)
		{
			WARNING(true, ("JavaLibrary::getNamesFromTemplateCrcs: template %s "
				"is not a shared template", ot->getName()));
			ot->releaseReference();
			continue;
		}
		ot = nullptr;

		const StringId objectName(sharedOt->getObjectName());
		sharedOt->releaseReference();
		sharedOt = nullptr;

		LocalRefPtr jobjectName;
		if (ScriptConversion::convert(objectName, jobjectName))
		{
			setObjectArrayElement(*names, i, *jobjectName);
		}
		else
		{
			WARNING(true, ("JavaLibrary::getNamesFromTemplateCrcs: could not "
				"convert stringId %s to Java",
				objectName.getCanonicalRepresentation().c_str()));
		}

	}
	return names->getReturnValue();
}	// JavaLibrary::getNamesFromTemplateCrcs


//========================================================================
// class JavaLibrary JNI object name callback methods
//========================================================================

/**
 * Sets an object's name from a string.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object to change name
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setNameFromString(JNIEnv *env, jobject self, jlong target, jstring name)
{
	UNREF(self);

	JavaStringParam localName(name);

	ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	std::string nameString;
	JavaLibrary::convert(localName, nameString);
	object->setObjectName(Unicode::utf8ToWide(nameString));

	return JNI_TRUE;
}	// JavaLibrary::setNameFromString

/**
 * Sets an object's name from a stringId.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object to change name
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setNameFromStringId(JNIEnv *env, jobject, jlong target, jobject nameId)
{
	ServerObject * object = 0;
	if(!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	JavaStringPtr tempString = getStringField(LocalRefParam(nameId), JavaLibrary::getFidStringIdTable());
	std::string table;
	if(! JavaLibrary::convert(*tempString, table))
		return JNI_FALSE;

	std::string asciiId;
	JavaStringPtr tempAsciiIdString = getStringField(LocalRefParam(nameId), JavaLibrary::getFidStringIdAsciiId());
	if(! JavaLibrary::convert(*tempAsciiIdString, asciiId))
		return JNI_FALSE;

	StringId stringId(table, asciiId);
	object->setObjectNameStringId(stringId);

	return JNI_TRUE;
}	// JavaLibrary::setNameFromStringId

/**
* Sets an object's description from a stringId.
*
* @param env		    Java environment
* @param self		    class calling this function
* @param target	        id of object to change name
* @param descriptionId  string id of the new description
*
* @return true on success, false on fail
*/
void JNICALL ScriptMethodsObjectInfoNamespace::setDescriptionStringId(JNIEnv * env, jobject self, jlong target, jobject descriptionId)
{
	ServerObject * object = 0;
	if(! JavaLibrary::getObject(target, object))
		return;

	JavaStringPtr tempString = getStringField(LocalRefParam(descriptionId), JavaLibrary::getFidStringIdTable());
	std::string table;
	if(! JavaLibrary::convert(*tempString, table))
		return;

	std::string asciiId;
	JavaStringPtr tempAsciiIdString = getStringField(LocalRefParam(descriptionId), JavaLibrary::getFidStringIdAsciiId());
	if(! JavaLibrary::convert(*tempAsciiIdString, asciiId))
		return;

	StringId stringId(table, asciiId);
	object->setDescriptionStringId(stringId);
}

/**
* Returns an object's description.
*
* @param env		    Java environment
* @param self		    class calling this function
* @param target	        id of object whose name to get
*
* @return the description, or nullptr on error
*/
jobject JNICALL ScriptMethodsObjectInfoNamespace::getDescriptionStringId(JNIEnv * env, jobject self, jlong target)
{
	ServerObject * object = 0;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	StringId const & descriptionId = object->getDescriptionStringId();

	LocalRefPtr result;
	if (ScriptConversion::convert(descriptionId, result))
		return result->getReturnValue();

	return 0;
}

/**
 * Returns an object's name.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object whose name to get
 *
 * @return the name, or nullptr on error
 */
jstring JNICALL ScriptMethodsObjectInfoNamespace::getName(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	const Unicode::String & name = object->getObjectName();
	return JavaString(name).getReturnValue();
}	// JavaLibrary::getName

/**
 * Returns a player's base name. The base name is the player's first name,
 * all lowercase.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		id of the player to get
 *
 * @return the name, or nullptr on error
 */
jstring JNICALL ScriptMethodsObjectInfoNamespace::getPlayerName(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const NetworkId id(target);
	if (id == NetworkId::cms_invalid)
		return 0;

	const std::string & name = NameManager::getInstance().getPlayerName(id);
	if (name.empty())
		return 0;
	return JavaString(name).getReturnValue();
}	// JavaLibrary::getName

jstring JNICALL ScriptMethodsObjectInfoNamespace::getPlayerFullName(JNIEnv *env, jobject, jlong target)
{
	NetworkId const id(target);
	if (id == NetworkId::cms_invalid)
		return 0;

	std::string const &name = NameManager::getInstance().getPlayerFullName(id);
	if (name.empty())
		return 0;
	return JavaString(name).getReturnValue();
}

/**
 * Returns an object's name.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object whose name to get
 *
 * @return the name, or nullptr on error
 */
jstring JNICALL ScriptMethodsObjectInfoNamespace::getAssignedName(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	const Unicode::String & name = object->getAssignedObjectName();
	return JavaString(name).getReturnValue();
}	// JavaLibrary::getName

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setCreatureName(JNIEnv * /*env*/, jobject /*self*/, jlong creature, jstring creatureName)
{
	NetworkId const networkId(creature);
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(networkId);

	if (creatureObject == nullptr)
	{
		WARNING(true, ("ERROR: ScriptMethodsObjectInfo::setCreatureName() Unable to resolve the object(%s) to a CreatureObject.", networkId.getValueString().c_str()));
		return;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(creatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		WARNING(true, ("ERROR: ScriptMethodsObjectInfo::setCreatureName() Unable to resolve the object's(%s) controller to an AiCreatureController.", creatureObject->getDebugInformation().c_str()));
		return;
	}

	std::string nameString;
	JavaLibrary::convert(JavaStringParam(creatureName), nameString);
	aiCreatureController->setCreatureName(nameString);
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsObjectInfoNamespace::getCreatureName(JNIEnv * /*env*/, jobject /*self*/, jlong creature)
{
	NetworkId const networkId(creature);
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(networkId);

	if (creatureObject == nullptr)
	{
		return 0;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(creatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		return 0;
	}

	PersistentCrcString const & creatureName = aiCreatureController->getCreatureName();

	if (creatureName.isEmpty())
	{
		WARNING(true, ("ScriptMethodsObjectInfo::getCreatureName() Somehow this AI(%s) is and calling this function but it was not created using the proper create.scriptlib::initializeCreature().", creatureObject->getDebugInformation().c_str()));

		return 0;
	}

	return JavaString(creatureName.getString()).getReturnValue();
}

/**
 * Returns an object's name.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object whose name to get
 *
 * @return the name, or nullptr on error
 */
jobject JNICALL ScriptMethodsObjectInfoNamespace::getNameStringId(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	const StringId & nameId = object->getObjectNameStringId();

	LocalRefPtr result;
	if (ScriptConversion::convert(nameId, result))
		return result->getReturnValue();
	return 0;
}	// JavaLibrary::getName

/**
 * Returns the default name of an object that would be created by a given template.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param jtemplateName		the template name
 *
 * @return the name, or nullptr if the template doesn't exist
 */
jobject JNICALL ScriptMethodsObjectInfoNamespace::getNameFromTemplate(JNIEnv *env, jobject self,
	jstring jtemplateName)
{
	JavaStringParam templateNameParam(jtemplateName);

	std::string templateName;
	if (!JavaLibrary::convert(templateNameParam, templateName))
		return 0;

	jint templateCrc = Crc::calculate(templateName.c_str());

	return getNameFromTemplateCrc(env, self, templateCrc);
}	// JavaLibrary::getNameFromTemplate

/**
 * Returns the default name of an object that would be created by a given template.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param templateCrc		the template crc
 *
 * @return the name, or nullptr if the template doesn't exist
 */
jobject JNICALL ScriptMethodsObjectInfoNamespace::getNameFromTemplateCrc(JNIEnv *env, jobject self,
	jint templateCrc)
{
	const ObjectTemplate * ot = ObjectTemplateList::fetch(templateCrc);
	if (ot == nullptr)
		return 0;

	// the name is stored in the shared template, so if this is a server template,
	// get the shared one from it
	const SharedObjectTemplate * sharedOt = nullptr;
	const ServerObjectTemplate * serverOt = dynamic_cast<const ServerObjectTemplate *>(
		ot);
	if (serverOt != nullptr)
	{
		const std::string sharedTemplateName(serverOt->getSharedTemplate());
		serverOt->releaseReference();
		serverOt = nullptr;
		ot = ObjectTemplateList::fetch(sharedTemplateName);
		if (ot == nullptr)
			return 0;
	}

	sharedOt = dynamic_cast<const SharedObjectTemplate *>(ot);
	if (sharedOt == nullptr)
	{
		ot->releaseReference();
		return 0;
	}
	ot = nullptr;

	const StringId objectName(sharedOt->getObjectName());
	sharedOt->releaseReference();
	sharedOt = nullptr;

	LocalRefPtr jobjectName;
	if (!ScriptConversion::convert(objectName, jobjectName))
		return 0;
	return jobjectName->getReturnValue();
}	// JavaLibrary::getNameFromTemplate

/**
 * Returns the default names of objects that would be created by given templates.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param jtemplateNames	the template names
 *
 * @return the names, or nullptr on error
 */
jobjectArray JNICALL ScriptMethodsObjectInfoNamespace::getNamesFromTemplates(JNIEnv *env, jobject self,
	jobjectArray jtemplateNames)
{
	if (jtemplateNames == 0)
		return 0;

	// convert the Java template names to C
	std::vector<std::string> templateNames;
	if (!ScriptConversion::convert(jtemplateNames, templateNames))
		return 0;

	// convert the C template strings to crcs
	std::vector<jint> templateCrcs;
	templateCrcs.resize(templateNames.size());
	int j = 0;
	for (std::vector<std::string>::const_iterator i = templateNames.begin();
		i != templateNames.end(); ++i)
	{
		templateCrcs[j++] = Crc::calculate((*i).c_str());
	}

	return getNamesFromCrcs(env, templateCrcs);
}	// JavaLibrary::getNamesFromTemplates

/**
 * Returns the default names of objects that would be created by given templates.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param jtemplateCrcs		the template crcs
 *
 * @return the names, or nullptr on error
 */
jobjectArray JNICALL ScriptMethodsObjectInfoNamespace::getNamesFromTemplateCrcs(JNIEnv *env, jobject self,
	jintArray jtemplateCrcs)
{
	if (jtemplateCrcs == 0)
		return 0;

	jsize length = env->GetArrayLength(jtemplateCrcs);
	std::vector<jint> templateCrcs;
	templateCrcs.resize(length);
	if (length > 0)
		env->GetIntArrayRegion(jtemplateCrcs, 0, length, &templateCrcs[0]);

	return getNamesFromCrcs(env, templateCrcs);
}	// JavaLibrary::getNamesFromTemplates


//========================================================================
// class JavaLibrary JNI object info callback methods
//========================================================================

/**
 * Checks if C thinks an object is authoritative. We usually use the flag on
 * obj_id, but we've noticed cases where they are getting out of synch for some
 * unknown reason. This function will reset the obj_id flag to the appropriate
 * value.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param id		id of the object to test
 *
 * @return true if the object is authoritative, false if not
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::internalIsAuthoritative(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	jboolean authoritative = static_cast<jboolean>(object->isAuthoritative() ?
		JNI_TRUE : JNI_FALSE);
	const uint32 pid = GameServer::getInstance().getProcessId();

	LocalRefPtr targetRefPtr = JavaLibrary::getObjId(target);

	if (targetRefPtr != LocalRef::cms_nullPtr)
	{
		callVoidMethod(*targetRefPtr, JavaLibrary::getMidObjIdSetAuthoritative(), authoritative, pid);
	}
	else
	{
		WARNING(true, ("ScriptMethodsObjectInfo::internalIsAuthoritative() network id could not be resolved to an object id"));
	}

	return authoritative;
}	// JavaLibrary::internalIsAuthoritative

/**
* Checks to see if the object has a proxy object or an authoritative object on another game server
* @param id        id of the object to test
* @return true or false
*/
jboolean JNICALL ScriptMethodsObjectInfoNamespace::hasProxyOrAuthObject(JNIEnv *env, jobject self, jlong target)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::hasProxyOrAuthObject");

	UNREF(self);

	const ServerObject * object = nullptr;

	// if I'm not authoritative, then I must have an authoritative object on another game server
	// if I'm authoritative, then see if I'm proxied on any other game server
	return (JavaLibrary::getObject(target, object) && (!object->isAuthoritative() || !object->getExposedProxyList().empty()));
}	// JavaLibrary::hasProxyOrAuthObject

/**
 * Returns the gender of a creature
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to check
 *
 * @return the gender, or -1 on error
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getGender(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	return creature->getGender();
}	// JavaLibrary::getGender

/**
 * Tests if two creatures are the same gender.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target1		1st creature
 * @param target2		2nd creature
 *
 * @return true if they are the same gender, false if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isSameGender(JNIEnv *env, jobject self, jlong target1, jlong target2)
{
	UNREF(self);

	const CreatureObject *creature1 = 0;
	if (!JavaLibrary::getObject(target1, creature1))
		return JNI_FALSE;
	const CreatureObject *creature2 = 0;
	if (!JavaLibrary::getObject(target2, creature2))
		return JNI_FALSE;

	return static_cast<jboolean>(creature1->getGender() == creature2->getGender() ?
		JNI_TRUE : JNI_FALSE);
}	// JavaLibrary::isSameGender

/**
 * Tests if a creature is a given species.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to check
 * @param race			race we want to check against
 *
 * @return true if the creature is the race, false if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isSpecies(JNIEnv *env, jobject self, jlong target, jint race)
{
	UNREF(self);

	const CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	return static_cast<jboolean>(creature->getSpecies() == race ? JNI_TRUE : JNI_FALSE);
}	// JavaLibrary::isRace

/**
 * Tests if a creature is a given race.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to check
 * @param race			race we want to check against
 *
 * @return true if the creature is the race, false if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isRace(JNIEnv *env, jobject self, jlong target, jint species, jint race)
{
	UNREF(self);

	const CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	return static_cast<jboolean>(creature->getSpecies() == species && creature->getRace() == race ? JNI_TRUE : JNI_FALSE);
}	// JavaLibrary::isRace

/**
 * Tests if a creature is a given niche.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to check
 * @param race			race we want to check against
 *
 * @return true if the creature is the race, false if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isNiche(JNIEnv *env, jobject self, jlong target, jint niche)
{
	UNREF(self);

	const CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	return static_cast<jboolean>(creature->getNiche() == niche ? JNI_TRUE : JNI_FALSE);
}	// JavaLibrary::isRace

/**
 * Tests if a creature is a given niche using a mask.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to check
 * @param race			race we want to check against
 *
 * @return true if the creature is the race, false if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isNicheMask(JNIEnv *env, jobject self, jobject target, jint niche, jint mask)
{
	UNREF(self);

	const CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	return static_cast<jboolean>((creature->getNiche() & mask) == (niche & mask) ? JNI_TRUE : JNI_FALSE);
}	// JavaLibrary::isRace

/**
 * Returns the speciesof a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to check
 *
 * @return the race, or -1 on error
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getSpecies(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	return creature->getSpecies();
}	// JavaLibrary::getRace


/**
 * Returns the race of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to check
 *
 * @return the race, or -1 on error
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getRace(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	return creature->getRace();
}	// JavaLibrary::getRace

/**
 * Returns the niche of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to check
 *
 * @return the race, or -1 on error
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getNiche(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	return creature->getNiche();
}	// JavaLibrary::getRace

/**
 * Returns if an object is a mobile/creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to check
 *
 * @return true if it is, false if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isMob(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;
	return JNI_TRUE;
}	// JavaLibrary::isMob

/**
 * Returns if an object is a tangible.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to check
 *
 * @return true if it is, false if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isTangible(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject *tangible = 0;
	if (!JavaLibrary::getObject(target, tangible))
		return JNI_FALSE;
	return JNI_TRUE;
}	// JavaLibrary::isTangible

/**
 * Returns if an object is a player.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to check
 *
 * @return true if it is, false if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isPlayer(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	// adding back in old functionality for isplayer - causing issues when players aren't fully loaded yet in npe
	// NEEDS TO BE ADDRESSED WITH NAME MANAGER
	const ServerObject *player = 0;
	if (JavaLibrary::getObject(target, player) && player->isPlayerControlled())
		return JNI_TRUE;

	// Use the name manager so that we can identify players that are not on this server
	if (NameManager::getInstance().isPlayer(NetworkId(target)))
		return JNI_TRUE;

	return JNI_FALSE;
}	// JavaLibrary::isPlayer

/**
 * Returns if the player is AFK
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		player we want to check
 *
 * @return true if the player is currently AFK, false if not
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isAwayFromKeyBoard(JNIEnv *env, jobject self, jlong player)
{
	UNREF(self);

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return JNI_FALSE;

	PlayerObject * const playerObj = PlayerCreatureController::getPlayerObject(playerCreature);
	if (playerObj != nullptr && playerObj->isAwayFromKeyBoard())
	{
		return JNI_TRUE;
	}
	return JNI_FALSE;

}	// JavaLibrary::isAwayFromKeyBoard

/**
 * Returns the type of an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to check
 *
 * @return the object type, or -1 on error
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getObjType(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const Object *object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return -1;

	return object->getObjectType();
}	// JavaLibrary::getObjType

/**
 * Returns the template id of an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to check
 *
 * @return the template type, or nullptr on error
 */
jstring JNICALL ScriptMethodsObjectInfoNamespace::getTemplateId(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const Object *object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	return JavaString(object->getObjectTemplateName()).getReturnValue();
}	// JavaLibrary::getTemplateId

/**
	* Returns the shared template id of an object.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param target		object we want to check
	*
	* @return the template type, or nullptr on error
*/

jstring JNICALL ScriptMethodsObjectInfoNamespace::getSharedObjectTemplateName(JNIEnv * /*env*/, jobject /*self*/, jlong target)
{
	Object const * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	ObjectTemplate const * const objectTemplate = object->getObjectTemplate();
	if (!objectTemplate)
		return 0;

	ServerObjectTemplate const * const serverObjectTemplate = objectTemplate->asServerObjectTemplate();
	if (!serverObjectTemplate)
		return 0;

	std::string sharedObjectTemplateName = serverObjectTemplate->getSharedTemplate();

	char const * const debug = sharedObjectTemplateName.c_str();

	return JavaString(debug).getReturnValue();
}	// JavaLibrary::getSharedObjectTemplateName

/**
 * Returns a creature's lookat target.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param object		creature we want to check
 *
 * @return the lookat target, 0 if error
 */

jlong JNICALL ScriptMethodsObjectInfoNamespace::getLookAtTarget (JNIEnv* /*env*/, jobject /*self*/, jlong object)
{
	//-- make sure we have a creature object
	const CreatureObject* creatureObject = 0;
	const ShipObject* shipObject = 0;
	if (!JavaLibrary::getObject (object, creatureObject))
	{
		if (!JavaLibrary::getObject (object, shipObject))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("getLookAtTarget (): jobject object is not a creature object"));
			return 0;
		}
	}

	//-- if our network id is invalid, we have no lookat target
	NetworkId networkId;
	if(creatureObject)
		networkId = creatureObject->getLookAtTarget ();
	else if(shipObject)
		networkId = shipObject->getPilotLookAtTarget ();
	if (networkId == NetworkId::cms_invalid)
		return 0;

	//-- return the network id
	return networkId.getValue();
}

/**
 * Sets the creature's lookat target
 *
 * @param env         Java environment
 * @param self        class calling this function
 * @param object      object we want to set the target of
 * @param target      new target for the object
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setLookAtTarget (JNIEnv* /*env*/, jobject /*self*/, jlong object, jlong target)
{
	//-- make sure we have a creature object
	CreatureObject* creatureObject = 0;
	ShipObject* shipObject = 0;

	if (!JavaLibrary::getObject(object, creatureObject))
	{
		if (!JavaLibrary::getObject(object, shipObject))
		{
			WARNING(true, ("ScriptMethodsObjectInfo::setLookAtTarget() Object could not be resolved to a CreatureObject or ShipObject"));
			return JNI_FALSE;
		}
	}

	NetworkId const networkId(target);

	//-- set our lookat target
	if(creatureObject)
		creatureObject->setLookAtTarget (networkId);
	else if(shipObject)
		shipObject->setPilotLookAtTarget (networkId);

	return JNI_TRUE;
}


/**
* Returns a creature's beastmaster pet.
*
* @param env		    Java environment
* @param self		    class calling this function
* @param object		player we want to get pet id of
*
* @return the pet id, 0 if error
*/

jlong JNICALL ScriptMethodsObjectInfoNamespace::getBeastmasterPet (JNIEnv* /*env*/, jobject /*self*/, jlong object)
{
	//-- make sure we have a creature object
	const CreatureObject* creatureObject = 0;
	if (!JavaLibrary::getObject (object, creatureObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("getBeastmasterPet (): jobject object is not a creature object"));
		return 0;
	}
	
	PlayerObject const *po = PlayerCreatureController::getPlayerObject(creatureObject);
	if(!po)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("getBeastmasterPet (): jobject object does not have a player object"));
		return 0;
	}

	NetworkId networkId = po->getPetId();
	if (networkId == NetworkId::cms_invalid)
		return 0;

	//-- return the network id
	return networkId.getValue();
}

/**
* Sets the creature's beastmaster pet
*
* @param env         Java environment
* @param self        class calling this function
* @param object      object we want to set the pet of
* @param pet         new pet id
*
* @return JNI_TRUE on success, JNI_FALSE on fail
*/
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setBeastmasterPet (JNIEnv* /*env*/, jobject /*self*/, jlong object, jlong pet)
{
	//-- make sure we have a creature object
	CreatureObject* creatureObject = 0;

	if (!JavaLibrary::getObject(object, creatureObject))
	{
		WARNING(true, ("ScriptMethodsObjectInfo::setBeastmasterPet() Object could not be resolved to a CreatureObject"));
		return JNI_FALSE;
	}

	PlayerObject *po = PlayerCreatureController::getPlayerObject(creatureObject);

	if(!po)
	{
		WARNING(true, ("ScriptMethodsObjectInfo::setBeastmasterPet() Object has no player object"));
		return JNI_FALSE;
	}
	NetworkId const networkId(pet);
	
	po->setPetId(networkId);

	// set the bool for being a beast on the CreatureObject.  This lets the client
	// handle it slightly differently.
	if(networkId.isValid())
	{
		CreatureObject *petObject = 0;
		if (!JavaLibrary::getObject(pet, petObject))
		{
			WARNING(true, ("ScriptMethodsObjectInfo::setBeastmasterPet() Pet object could not be resolved to a CreatureObject"));
			return JNI_FALSE;
		}
		petObject->setIsBeast(true);

	}

	return JNI_TRUE;
}


/**
* Returns a creature's intended target.
*
* @param env		    Java environment
* @param self		    class calling this function
* @param object		creature we want to check
*
* @return the intended target, 0 if error
*/

jlong JNICALL ScriptMethodsObjectInfoNamespace::getIntendedTarget (JNIEnv* /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const objectId(object);
	CreatureObject const * creatureObject = 0;
	if (!JavaLibrary::getObject (object, creatureObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsObjectInfo::getIntendedTarget() The specified object(%s) is not a creatureObject", objectId.getValueString().c_str()));
		return 0;		
	}

	//-- if our network id is invalid, we have no intended target
	NetworkId networkId;
	if(creatureObject)
		networkId = creatureObject->getIntendedTarget();
	if (networkId == NetworkId::cms_invalid)
		return 0;

	//-- return the network id
	return networkId.getValue();
}

/**
 * Returns a creature's posture.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to check
 *
 * @return the posture, 0 if error/target not creature
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getPosture(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	return creature->getPosture();
}	// JavaLibrary::getPosture

/**
 * Sets a creature's posture.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to change
 * @param posture       the new posture
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail/target not creature
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setPosture(JNIEnv *env, jobject self, jlong target, jint posture)
{
	UNREF(self);

	if (posture < 0 || posture >= Postures::NumberOfPostures)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (! JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	// take care of special postures
	if (posture == Postures::Dead)
		creature->makeDead(NetworkId::cms_invalid, NetworkId::cms_invalid);
	else if (posture == Postures::Incapacitated)
		creature->setIncapacitated(true, NetworkId::cms_invalid);
	else
	{
		if (creature->isIncapacitated())
			creature->setIncapacitated(false, NetworkId::cms_invalid);
		creature->setPosture(static_cast<Postures::Enumerator>(posture));
	}
	return JNI_TRUE;
}	// JavaLibrary::setPosture

/**
 * Sets a creature's posture, taking effect immediately on all observing clients.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to change
 * @param posture       the new posture
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail/target not creature
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setPostureClientImmediate(JNIEnv *env, jobject self, jlong target, jint posture)
{
	UNREF(self);

	if (posture < 0 || posture >= Postures::NumberOfPostures)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (! JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	// take care of special postures
	if (posture == Postures::Dead)
		creature->makeDead(NetworkId::cms_invalid, NetworkId::cms_invalid);
	else if (posture == Postures::Incapacitated)
		creature->setIncapacitated(true, NetworkId::cms_invalid);
	else
	{
		if (creature->isIncapacitated())
			creature->setIncapacitated(false, NetworkId::cms_invalid);
		creature->setPosture(static_cast<Postures::Enumerator>(posture));
	}

	//-- Whatever posture was resolved, set that posture in immediate mode on the client.
	creature->setPosture(creature->getPosture(), true);

	return JNI_TRUE;
}	// JavaLibrary::setPosture

/**
 * Returns a creature's locomotion.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to check
 *
 * @return the posture, 0 if error/target not creature
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getLocomotion(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	return creature->getLocomotion();
}	// JavaLibrary::getLocomotion

/**
 * Sets a creature's locomotion.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to change
 * @param posture       the new posture
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail/target not creature
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setLocomotion(JNIEnv *env, jobject, jlong target, jint posture)
{
	if (posture < 0 || posture >= Locomotions::NumberOfLocomotions)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if(!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;
	creature->setLocomotion(static_cast<Locomotions::Enumerator>(posture));
	return JNI_TRUE;
}	// JavaLibrary::setLocomotion

/**
 * Returns a whether state is set on a creature.
 *
 * @param env         Java environment
 * @param self        class calling this function
 * @param target      creature we want to check
 * @param whichState  state we wish to check
 *
 * @return 0 if not set, 1 if set, -1 on error/target not creature
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getState(JNIEnv *env, jobject self, jlong target, jint whichState)
{
	UNREF(self);

	if (whichState < 0 || whichState >= States::NumberOfStates)
		return -1;

	const CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	return creature->getState(static_cast<States::Enumerator>(whichState)) ? 1 : 0;
}	// JavaLibrary::getState

/**
 * Sets or clears a state on a creature. NOTE: Do not call this function with state
 * "Combat". Use the setInCombat()/removeFromCombat() functions instead.
 *
 * @param env         Java environment
 * @param self        class calling this function
 * @param target      creature we want to change
 * @param whichState  the state identifier to deal with
 * @param value       the new value for the state (set/clear)
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail/target not creature
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setState(JNIEnv *env, jobject self, jlong target, jint whichState, jboolean value)
{
	UNREF(self);

	if (whichState < 0 || whichState >= States::NumberOfStates)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (! JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (whichState == States::Combat)
	{
		WARNING_STRICT_FATAL(true, ("WARNING: Calling JavaLibrary::setState with "
			"state=Combat on object %s", creature->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	creature->setState(static_cast<States::Enumerator>(whichState), value);

	return JNI_TRUE;
}	// JavaLibrary::setState

/**
 * Returns the owner of a tangible object
 *
 * @param env         Java environment
 * @param self        class calling this function
 * @param target      object whose owner we want
 *
 * @return id of owner
 */
jlong JNICALL ScriptMethodsObjectInfoNamespace::getOwner(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);
	const TangibleObject *tobj = 0;
	if (!JavaLibrary::getObject(target, tobj))
		return (NetworkId::cms_invalid).getValue();
	return (tobj->getOwnerId()).getValue();
}	// JavaLibrary::getOwner

/**
 * Sets the owner of a tangible object
 *
 * @param env         Java environment
 * @param self        class calling this function
 * @param target      object we want to set the owner of
 * @param owner       new owner for the object
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail/target not tangible
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setOwner(JNIEnv *env, jobject, jlong target, jlong owner)
{
	ServerObject *tobj = 0;
	if (! JavaLibrary::getObject(target, tobj))
		return JNI_FALSE;

	tobj->setOwnerId(NetworkId(owner));
	return JNI_TRUE;
}	// JavaLibrary::setOwner

/**
 * Returns if a creature is incapacitated.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to know about
 *
 * @return JNI_TRUE if the creature is incapaciated, JNI_FALSE if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isIncapacitated(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (creature->isIncapacitated())
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::isIncapacitated

/**
 * Returns if a creature is incapacitated.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to know about
 *
 * @return JNI_TRUE if the creature is incapaciated, JNI_FALSE if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isDead(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (creature->isDead())
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::isIncapacitated

/**
 * Returns if a creature is an npc (i.e. not a player).
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to know about
 *
 * @return JNI_TRUE if the creature is an npc, JNI_FALSE if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isNpcCreature(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (creature->isPlayerControlled())
		return JNI_FALSE;
	return JNI_TRUE;
}


/**
 * Returns if an object is disabled.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to know about
 *
 * @return JNI_TRUE if the object is disabled, JNI_FALSE if not
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isDisabled(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = 0;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	// make sure the object isn't a creature or is a vehicle
	if (object->asCreatureObject () != nullptr &&
		!GameObjectTypes::isTypeOf (object->getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		return JNI_FALSE;

	if (object->isDisabled())
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::isDisabled

// ----------------------------------------------------------------

/**
 * Sets the source id an object will be marked as crafted with. If the id is 0,
 * the object will be considered not crafted.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to set as (not)crafted
 * @param id			the id that will be used as the crafted id
 *
 * @return JNI_TRUE in success, JNI_FALSE if the target is invalid
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setCraftedId(JNIEnv *env, jobject self, jlong target, jlong id)
{
	TangibleObject * object = 0;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	NetworkId craftedId(id);

	object->setCraftedId(craftedId);
	return JNI_TRUE;
}	// JavaLibrary::setCraftedId

// ----------------------------------------------------------------

/**
 * Returns if an object is crafted.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to know about
 *
 * @return JNI_TRUE if the object is crafted, JNI_FALSE if not
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isCrafted(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = 0;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	// make sure the object isn't a creature
	if (dynamic_cast<const CreatureObject *>(object) != nullptr)
		return JNI_FALSE;

	if (object->isCrafted())
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::isCrafted

// ----------------------------------------------------------------

/**
 * Returns the id of the player who crafted an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to know about
 *
 * @return the crafter id, or nullptr on error or if the item was not crafted
 */
jlong JNICALL ScriptMethodsObjectInfoNamespace::getCrafter(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = 0;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	const NetworkId crafterId(object->getCreatorId());
	if (crafterId == NetworkId::cms_invalid)
		return 0;

	return crafterId.getValue();
}	// JavaLibrary::getCrafter

// ----------------------------------------------------------------

/**
 * Changes the id of the player who crafted an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to know about
 * @param crafter		the id of the crafter
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setCrafter(JNIEnv *env, jobject self, jlong target, jlong crafter)
{
	UNREF(self);

	TangibleObject * object = 0;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	const NetworkId crafterId(crafter);
	if (crafterId == NetworkId::cms_invalid)
		return JNI_FALSE;

	object->setCreatorId(crafterId);
	return JNI_TRUE;
}	// JavaLibrary::setCrafter

// ----------------------------------------------------------------

/**
 * Returns the scale of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to know about
 *
 * @return the scale, or -1 on error
 */
jfloat JNICALL ScriptMethodsObjectInfoNamespace::getScale(JNIEnv *env, jobject self, jlong target)
{
	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return -1.0f;

	return creature->getScaleFactor();
}	// JavaLibrary::getScale

/**
 * Sets the scale of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to set
 * @param scale			the creature's new scale
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setScale(JNIEnv *env, jobject self, jlong target, jfloat scale)
{
	if (scale <= 0)
		return JNI_FALSE;

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->setScaleFactor(scale);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsObjectInfoNamespace::getDefaultScaleFromObjectTemplate(JNIEnv *env, jobject self, jstring serverObjectTemplateName)
{
	UNREF(env);
	UNREF(self);

	//-- Retrieve the server object template name.
	JavaStringParam serverObjectTemplateNameParam(serverObjectTemplateName);
	std::string     serverObjectTemplateNameNarrow;
	if (!JavaLibrary::convert(serverObjectTemplateNameParam, serverObjectTemplateNameNarrow))
	{
		char const *const errorMessage = "getDefaultScaleFromObjectTemplate(): error handling serverObjectTemplateName arg.";
		LOG("ScriptMethodsObjectInfo", (errorMessage));
		JavaLibrary::throwInternalScriptError(errorMessage);
		return static_cast<jfloat>(0.0f);
	}

	//-- Open the server object template.
	ObjectTemplate const *const baseServerObjectTemplate = ObjectTemplateList::fetch(serverObjectTemplateNameNarrow);
	if (!baseServerObjectTemplate)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "getDefaultScaleFromObjectTemplate(): failed to open object template [%s].", serverObjectTemplateNameNarrow.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		LOG("ScriptMethodsObjectInfo", (buffer));
		JavaLibrary::throwInternalScriptError(buffer);
		return static_cast<jfloat>(0.0f);
	}

	//-- Convert base object template to server object template.
	ServerObjectTemplate const *const serverObjectTemplate = dynamic_cast<ServerObjectTemplate const*>(baseServerObjectTemplate);
	if (!serverObjectTemplate)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "getDefaultScaleFromObjectTemplate(): object template [%s] is not derived from ServerObjectTemplate.", serverObjectTemplateNameNarrow.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		LOG("ScriptMethodsObjectInfo", (buffer));
		JavaLibrary::throwInternalScriptError(buffer);

		baseServerObjectTemplate->releaseReference();
		return static_cast<jfloat>(0.0f);
	}

	//-- Open the shared object template.
	std::string const &sharedObjectTemplateName = serverObjectTemplate->getSharedTemplate();
	ObjectTemplate const *const baseSharedObjectTemplate = ObjectTemplateList::fetch(sharedObjectTemplateName);
	if (!baseSharedObjectTemplate)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "getDefaultScaleFromObjectTemplate(): failed to open shared object template [%s].", sharedObjectTemplateName.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		LOG("ScriptMethodsObjectInfo", (buffer));
		JavaLibrary::throwInternalScriptError(buffer);

		baseServerObjectTemplate->releaseReference();
		return static_cast<jfloat>(0.0f);
	}

	//-- Convert to a shared object template.
	SharedObjectTemplate const *const sharedObjectTemplate = baseSharedObjectTemplate->asSharedObjectTemplate();
	if (!sharedObjectTemplate)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "getDefaultScaleFromObjectTemplate(): object template [%s] is not derived from SharedObjectTemplate.", sharedObjectTemplateName.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		LOG("ScriptMethodsObjectInfo", (buffer));
		JavaLibrary::throwInternalScriptError(buffer);

		baseSharedObjectTemplate->releaseReference();
		baseServerObjectTemplate->releaseReference();
		return static_cast<jfloat>(0.0f);
	}

	//-- Get scale.
	jfloat const returnScale = static_cast<jfloat>(sharedObjectTemplate->getScale());

	baseSharedObjectTemplate->releaseReference();
	baseServerObjectTemplate->releaseReference();

	return returnScale;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getVolume(JNIEnv *env, jobject self, jlong target)
{
	ServerObject *obj;
	if (!JavaLibrary::getObject(target,obj))
		return 0;
	else
		return obj->getVolume();
}

jboolean JNICALL ScriptMethodsObjectInfoNamespace::isFacing(JNIEnv *env, jobject self, jlong mob, jobject target)
{
	Object * object = 0;
	if (!JavaLibrary::getObject(mob, object))
		return JNI_FALSE;
	Vector v;
	if (!ScriptConversion::convertWorld(target, v))
	{
		return JNI_FALSE;
	}

	Vector positionInObjectSpace = object->rotateTranslate_w2o (v);
	real theta = positionInObjectSpace.theta ();

	if (std::abs(theta) <= PI_OVER_4)
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

/**
 * Returns the distance between two objects. Note that this is the distance
 * between the bounding-sphere edges, not the centers.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target1		one of the objects
 * @param target2		the other object
 *
 * @return the distance, or -1 on error
 */
jfloat JNICALL ScriptMethodsObjectInfoNamespace::getDistance(JNIEnv *env, jobject self, jlong target1, jlong target2)
{
	ServerObject const * object1 = 0;
	if (!JavaLibrary::getObject(target1, object1))
		return -1.0f;
	ServerObject const * object2 = 0;
	if (!JavaLibrary::getObject(target2, object2))
		return -1.0f;

	jfloat distance = object1->getDistanceBetweenCollisionSpheres_w(*object2);
	return distance;
}	// JavaLibrary::getDistance

/**
 * Returns the distance between two locations. The locations should be in
 * different cells.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param loc1			the 1st location
 * @param loc2			the 2st location
 *
 * @return the distance between the points, or -1 on error
 */
jfloat JNICALL ScriptMethodsObjectInfoNamespace::getLocationDistance(JNIEnv *env, jobject self, jobject loc1, jobject loc2)
{
	if (loc1 == 0 || loc2 == 0)
		return -1.0f;

	// get the location data
	Vector targetLoc1;
	std::string targetSceneId1;
	NetworkId targetCell1;
	if (!ScriptConversion::convert(loc1, targetLoc1, targetSceneId1, targetCell1))
		return -1.0f;

	Vector targetLoc2;
	std::string targetSceneId2;
	NetworkId targetCell2;
	if (!ScriptConversion::convert(loc2, targetLoc2, targetSceneId2, targetCell2))
		return -1.0f;

	// make sure we're on the same planet
	if (targetSceneId1 != targetSceneId2)
		return -1.0f;

	// translate local coordinates to world coordinates
	Object * cell1 = NetworkIdManager::getObjectById(targetCell1);
	if (cell1 == nullptr)
		return -1;
	const Vector & worldLoc1 = cell1->rotateTranslate_o2w(targetLoc1);
	Object * cell2 = NetworkIdManager::getObjectById(targetCell2);
	if (cell2 == nullptr)
		return -1;
	const Vector & worldLoc2 = cell2->rotateTranslate_o2w(targetLoc2);

	float dx = worldLoc1.x - worldLoc2.x;
	float dy = worldLoc1.y - worldLoc2.y;
	float dz = worldLoc1.z - worldLoc2.z;

	return sqrt(dx * dx + dy * dy + dz * dz);
}	// JavaLibrary::getLocationDistance

/**
* Returns true if a point is within a conical frustum.
*
* @param env		    Java environment
* @param self		    class calling this function
* @param jTestLoc		the location to be tested
* @param jStartLoc		the center of one end of the conical frustum
* @param jEndLoc		the center of the second end of the conical frustum
* @param startRadius	the radius at the first point
* @param endRadius		the radius at the second point
*
* @return true if the point is within the conical frustum
*/
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isLocationInConicalFrustum(JNIEnv *env, jobject self, jobject jTestLoc, jobject jStartLoc, jobject jEndLoc, jfloat startRadius, jfloat endRadius, jboolean use2d)
{
	if (jTestLoc == 0 || jStartLoc == 0 || jEndLoc == 0)
		return false;

	// get the location data
	Vector testLoc;
	std::string testSceneId;
	NetworkId testCellId;
	if (!ScriptConversion::convert(jTestLoc, testLoc, testSceneId, testCellId))
		return false;

	Vector startLoc;
	std::string startSceneId;
	NetworkId startCellId;
	if (!ScriptConversion::convert(jStartLoc, startLoc, startSceneId, startCellId))
		return false;

	Vector endLoc;
	std::string endSceneId;
	NetworkId endCellId;
	if (!ScriptConversion::convert(jEndLoc, endLoc, endSceneId, endCellId))
		return false;

	// make sure we're all on the same planet
	if (testSceneId != startSceneId || testSceneId != endSceneId)
		return false;

	// translate local coordinates to world coordinates if they are in cells
	Object * testCell = NetworkIdManager::getObjectById(testCellId);
	Vector testWorldLoc = (testCell != nullptr) ? testCell->rotateTranslate_o2w(testLoc) : testLoc;

	Object * startCell = NetworkIdManager::getObjectById(startCellId);
	Vector startWorldLoc = (startCell != nullptr) ? startCell->rotateTranslate_o2w(startLoc) : startLoc;

	Object * endCell = NetworkIdManager::getObjectById(endCellId);
	Vector endWorldLoc = (endCell != nullptr) ? endCell->rotateTranslate_o2w(endLoc) : endLoc;

	if(use2d)
	{
		testWorldLoc.y = 0.0f;
		startWorldLoc.y = 0.0f;
		endWorldLoc.y = 0.0f;
	}

	float t;
	Vector const closestPointOnAxis = testWorldLoc.findClosestPointOnLine(startWorldLoc, endWorldLoc, &t);
	
	if(t < 0.0f || t > 1.0f)
		return false;

	float const distanceFromAxisSquared = testWorldLoc.magnitudeBetweenSquared(closestPointOnAxis);
	float const frustumRadius = (endRadius - startRadius) * t + startRadius;

	return (frustumRadius*frustumRadius) > distanceFromAxisSquared;

}	// JavaLibrary::isLocationInConicalFrustum

/**
* Returns true if a point is within a cone
*
* @param env		    Java environment
* @param self		    class calling this function
* @param jTestLoc		the location to be tested
* @param jStartLoc		the center of one end of the conical frustum
* @param jDirectionLoc  a point in the direction of the cone
* @param range	        the range of the cone
* @param halfAngle		the half angle of the cone in degrees
*
* @return true if the point is within the cone
*/
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isLocationInCone(JNIEnv *env, jobject self, jobject jTestLoc, jobject jStartLoc, jobject jDirectionLoc, jfloat range, jfloat halfAngle, jboolean use2d)
{
	if (jTestLoc == 0 || jStartLoc == 0 || jDirectionLoc == 0)
		return false;

	// get the location data
	Vector testLoc;
	std::string testSceneId;
	NetworkId testCellId;
	if (!ScriptConversion::convert(jTestLoc, testLoc, testSceneId, testCellId))
		return false;

	Vector startLoc;
	std::string startSceneId;
	NetworkId startCellId;
	if (!ScriptConversion::convert(jStartLoc, startLoc, startSceneId, startCellId))
		return false;

	Vector endLoc;
	std::string endSceneId;
	NetworkId endCellId;
	if (!ScriptConversion::convert(jDirectionLoc, endLoc, endSceneId, endCellId))
		return false;

	// make sure we're all on the same planet
	if (testSceneId != startSceneId || testSceneId != endSceneId)
		return false;

	// translate local coordinates to world coordinates if they are in cells
	Object * testCell = NetworkIdManager::getObjectById(testCellId);
	Vector const& testWorldLoc = (testCell != nullptr) ? testCell->rotateTranslate_o2w(testLoc) : testLoc;

	Object * startCell = NetworkIdManager::getObjectById(startCellId);
	Vector const & startWorldLoc = (startCell != nullptr) ? startCell->rotateTranslate_o2w(startLoc) : startLoc;

	Object * endCell = NetworkIdManager::getObjectById(endCellId);
	Vector const & endWorldLoc = (endCell != nullptr) ? endCell->rotateTranslate_o2w(endLoc) : endLoc;

	Vector testPointConeSpace = testWorldLoc - startWorldLoc;
	if(use2d)
	{
		testPointConeSpace.y = 0.0f;
	}

	float const distanceSquared = testPointConeSpace.magnitudeSquared();

	if(distanceSquared > sqr(range))
		return false;

	Vector coneAxisVector = endWorldLoc - startWorldLoc;

	if(use2d)
	{
		coneAxisVector.y = 0.0f;
	}

	if(!coneAxisVector.normalize())
		return false;

	//If the vector is too small to normalize then the test location
	//must be within some small epsilon of the start location so allow
	//it to pass
	if(!testPointConeSpace.normalize())
		return true;

	float const cosAngle = cos(convertDegreesToRadians(halfAngle));
	const float dotProduct = coneAxisVector.dot(testPointConeSpace);
	const bool  withinCone = (dotProduct >= cosAngle);

	return withinCone;

}	// JavaLibrary::isLocationInCone

jboolean JNICALL ScriptMethodsObjectInfoNamespace::areFacingEachOther(JNIEnv *env, jobject self, jlong mob1, jlong mob2)
{
	Object * object1 = 0;
	if (!JavaLibrary::getObject(mob1, object1))
		return JNI_FALSE;
	Object * object2 = 0;
	if (!JavaLibrary::getObject(mob2, object2))
		return JNI_FALSE;

	Vector positionInObjectSpace = object1->rotateTranslate_w2o (object2->getPosition_w());
	real theta = positionInObjectSpace.theta ();

	if (std::abs(theta) > PI_OVER_4)
		return JNI_FALSE;
	else
	{
		// object1 is facing object2, so see if object2 is facing object1
		Vector positionInObjectSpace2 = object2->rotateTranslate_w2o (object1->getPosition_w());
		real theta2 = positionInObjectSpace2.theta ();

		if (std::abs(theta2) > PI_OVER_4)
			return JNI_FALSE;
		else
			return JNI_TRUE;
	}
}

jboolean JNICALL ScriptMethodsObjectInfoNamespace::exists(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	Object * object = 0;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;
	else
		return JNI_TRUE;
}

jboolean JNICALL ScriptMethodsObjectInfoNamespace::isInWorld(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;
	else
	{
		if (object->isInWorld())
			return JNI_TRUE;
		else
			return JNI_FALSE;
	}
}

jboolean JNICALL ScriptMethodsObjectInfoNamespace::isInWorldCell(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);
	NOT_NULL(env);

	ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	return (object->isInWorldCell());
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::watch(JNIEnv * env, jobject self, jlong target)
{
	UNREF(env);
	UNREF(self);
	UNREF(target);
	WARNING_STRICT_FATAL(true, ("This function has been depricated.  Do not use\n"));
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::stopWatching(JNIEnv * env, jobject self, jlong target)
{
	UNREF(env);
	UNREF(self);
	UNREF(target);
	WARNING_STRICT_FATAL(true, ("This function has been depricated.  Do not use\n"));

}

//-----------------------------------------------------------------------

void JavaLibrary::onStopWatching(ServerObject & observer, ServerObject & subject)
{
	DEBUG_FATAL(true, ("DO NOT CALL WATCHING/STOP WATCHING! REQUIRES NON-EXISTENT MESSAGE INTERFACE!"));
	static const std::string dictEntry("subject");

	LocalRefPtr params = createNewObject(ms_clsDictionary, ms_midDictionary);
	LocalRefPtr s = JavaLibrary::getObjId(subject);
	JavaString paramName(dictEntry.c_str());
	callObjectMethod(*params, ms_midDictionaryPut, paramName.getValue(), s->getValue());

	GameScriptObject * scripts = observer.getScriptObject();
	if(scripts)
	{
		//depricated
//		scripts->addMessage("OnStopWatching", params, 0);
	}
}

//-----------------------------------------------------------------------

void JavaLibrary::onWatching(ServerObject & observer, ServerObject & subject)
{
	DEBUG_FATAL(true, ("DO NOT CALL WATCHING/STOP WATCHING! REQUIRES NON-EXISTENT MESSAGE INTERFACE!"));
	static const std::string dictEntry("subject");

	LocalRefPtr params = createNewObject(ms_clsDictionary, ms_midDictionary);
	LocalRefPtr s = JavaLibrary::getObjId(subject);
	JavaString paramName(dictEntry.c_str());
	callObjectMethod(*params, ms_midDictionaryPut, paramName.getValue(), s->getValue());

	GameScriptObject * scripts = observer.getScriptObject();
	if(scripts)
	{
		// depricated
//		scripts->addMessage("OnWatching", params, 0);
	}
}

//-----------------------------------------------------------------------
// private local namespace used for CustomizationData support

namespace
{
	typedef std::vector<std::string>             StringVector;
	typedef std::vector<CustomizationVariable*>  CustomizationVariableVector;

	// custom_var TypeId definitions
	const jint CVT_UNKNOWN      = static_cast<jint>(0);
	const jint CVT_RANGED_INT   = static_cast<jint>(2);
	const jint CVT_PALCOLOR     = static_cast<jint>(3);

	struct CustomizationVariableIteratorData
	{
		StringVector                 m_variableNames;
		CustomizationVariableVector  m_customizationVariables;
	};

	// NOTE: this is inappropriate if we will allow multithreaded Java script execution
	CustomizationVariableIteratorData  s_customizationVariableIteratorData;

	//-----------------------------------------------------------------------

	void CustomizationVariableCollector(const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context)
	{
		//-- validate arguments
		if (!customizationVariable || !context)
		{
			DEBUG_FATAL(true, ("programmer error: callback made with nullptr arguments.\n"));
			return;
		}

		//-- convert context to customization variable collection
		CustomizationVariableIteratorData *const iteratorData = reinterpret_cast<CustomizationVariableIteratorData*>(context);

		//-- add CustomizationVariable to the container
		iteratorData->m_variableNames.push_back(fullVariablePathName);
		iteratorData->m_customizationVariables.push_back(customizationVariable);
	}

	// ----------------------------------------------------------------------

}

// ----------------------------------------------------------------------
/**
 * Create a Java custom_var given a CustomizationVariable instance
 * and its name.
 *
 * The Java custom_var instance will provide information regarding the
 * given CustomizationVariable instance, and will allow access to modify
 * the CustomizationVariable if applicable.
 *
 * @param objId             the Java obj_id for the Object influenced by
 *                          the newly-created customization variable.
 * @param variablePathName  the full pathname for the variable to create.
 * @param variable          the variable for which we will construct a
 *                          Java custom_var.
 *
 * @return  the Java-accessible custom_var instance, the script-side counterpart to
 *          the given C++-side CustomizationVariable.  Will return nullptr if
 *          a Java exception or other error occurs.
 */
LocalRefPtr ScriptMethodsObjectInfoNamespace::createCustomVar(const jlong &objId, const std::string &variablePathName, CustomizationVariable &variable)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::createCustomVar");
	//-- handle variables of type BasicRangedIntCustomVariable
	BasicRangedIntCustomizationVariable *const briVariable = dynamic_cast<BasicRangedIntCustomizationVariable*>(&variable);
	if (briVariable)
		return createRangedIntCustomVar(objId, variablePathName, *briVariable);

	//-- handle variables of type PaletteColorCustomizationVariable
	PaletteColorCustomizationVariable *const pcVariable = dynamic_cast<PaletteColorCustomizationVariable*>(&variable);
	if (pcVariable)
		return createPalcolorCustomVar(objId, variablePathName, *pcVariable);

	//-- create Java string for custom_var name arg
	JavaString customVarName(variablePathName.c_str());
	if (customVarName.getValue() == 0)
	{
		// exception thrown.
		return LocalRef::cms_nullPtr;
	}

	//-- construct a basic custom_var
	return createNewObject(JavaLibrary::getClsCustomVar(), JavaLibrary::getMidCustomVarObjIdStringInt(), objId, customVarName.getValue(), CVT_UNKNOWN);
}

/**
 * Create a Java ranged_int_custom_var given a RangedIntCustomizationVariable
 * instance and its name.
 *
 * The Java custom_var instance will provide information regarding the
 * given CustomizationVariable instance, and will allow access to modify
 * the CustomizationVariable if applicable.
 *
 * @param objId             the Java obj_id for the Object influenced by
 *                          the newly-created customization variable.
 * @param variablePathName  the full pathname for the variable to create.
 * @param variable          the variable for which we will construct a
 *                          Java custom_var.
 *
 * @return  the Java-accessible ranged_int_custom_var instance, the script-side counterpart to
 *          the given C++-side CustomizationVariable.  Will return nullptr if
 *          a Java exception or other error occurs.
 */
LocalRefPtr ScriptMethodsObjectInfoNamespace::createRangedIntCustomVar(const jlong &objId, const std::string &variablePathName, RangedIntCustomizationVariable &rangedIntVariable)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::createRangedIntCustomVar");
	//-- create Java string for custom_var name arg
	JavaString customVarName(variablePathName.c_str());
	if (customVarName.getValue() == 0)
	{
		// exception thrown.
		return LocalRef::cms_nullPtr;
	}

	//-- get the variable range
	int minRangeInclusive = 0;
	int maxRangeExclusive = 0;

	rangedIntVariable.getRange(minRangeInclusive, maxRangeExclusive);

	//-- create the variable
	return createNewObject(JavaLibrary::getClsRangedIntCustomVar(), JavaLibrary::getMidRangedIntCustomVar(), objId, customVarName.getValue(), CVT_RANGED_INT, static_cast<jint>(minRangeInclusive), static_cast<jint>(maxRangeExclusive));
}

/**
 * Create a Java palcolor_custom_var given a PaletteColorCustomizationVariable
 * instance and its name.
 *
 * The Java custom_var instance will provide information regarding the
 * given CustomizationVariable instance, and will allow access to modify
 * the CustomizationVariable if applicable.
 *
 * @param objId             the Java obj_id for the Object influenced by
 *                          the newly-created customization variable.
 * @param variablePathName  the full pathname for the variable to create.
 * @param variable          the variable for which we will construct a
 *                          Java custom_var.
 *
 * @return  the Java-accessible palcolor_custom_var instance, the script-side counterpart to
 *          the given C++-side CustomizationVariable.  Will return nullptr if
 *          a Java exception or other error occurs.
 */
LocalRefPtr ScriptMethodsObjectInfoNamespace::createPalcolorCustomVar(const jlong &objId, const std::string &variablePathName, PaletteColorCustomizationVariable &variable)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::createPalcolorCustomVar");
	//-- create Java string for custom_var name arg
	JavaString customVarName(variablePathName.c_str());
	if (customVarName.getValue() == 0)
	{
		// exception thrown.
		return LocalRef::cms_nullPtr;
	}

	//-- get the variable range
	int minRangeInclusive = 0;
	int maxRangeExclusive = 0;

	variable.getRange(minRangeInclusive, maxRangeExclusive);

	//-- create the variable
	return createNewObject(JavaLibrary::getClsPalcolorCustomVar(), JavaLibrary::getMidPalcolorCustomVar(), objId, customVarName.getValue(), CVT_PALCOLOR, static_cast<jint>(minRangeInclusive), static_cast<jint>(maxRangeExclusive));
}

// ----------------------------------------------------------------------

LocalRefPtr ScriptMethodsObjectInfoNamespace::createColor(int r, int g, int b, int a)
{
	return createNewObject(JavaLibrary::getClsColor(), JavaLibrary::getMidColor(), static_cast<jint>(r), static_cast<jint>(g), static_cast<jint>(b), static_cast<jint>(a));
}

// ----------------------------------------------------------------------
/**
 * Fetch the CustomizationData instance associated with the Object
 * specified by the given obj_id.
 *
 * This function may return nullptr if the specified object doesn't have
 * customization data or if some other error occurs.
 *
 * The caller must call CustomizationData::release() on the non-nullptr return
 * value when the reference no longer is needed.  Failure to do so will cause
 * a memory leak.
 *
 * @param objId  the Java obj_id representing the Object for which the
 *               CustomizationData instance should be retrieved.
 *
 * @return  the CustomizationData instance associated with the specified
 *          Object.  May return nullptr if the Object doesn't have customization
 *          data or if an error occurs.
 */
CustomizationData *ScriptMethodsObjectInfoNamespace::fetchCustomizationDataFromObjId(jlong objId)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::fetchCustomizationDataFromObjId");
	if (!objId)
		return nullptr;

	//-- Get the target TangibleObject.
	TangibleObject *object = 0;
	if (!JavaLibrary::getObject(objId, object))
	{
		// this Object doesn't exist or isn't derived from TangibleObject
		return nullptr;
	}

	//-- Fetch the CustomizationData.
	CustomizationDataProperty *const cdProperty = safe_cast<CustomizationDataProperty*>(object->getProperty(CustomizationDataProperty::getClassPropertyId()));
	if (!cdProperty)
	{
		// this Object doesn't expose any customization data
		return nullptr;
	}

	CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
	if (!customizationData)
	{
		// this shouldn't happen
		DEBUG_WARNING(true, ("CustomizationDataProperty returned nullptr CustomizationData on fetch().\n"));
		return nullptr;
	}

	//-- return the CustomizationData instance.
	return customizationData;
}

//-----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsObjectInfoNamespace::getAllCustomVars(JNIEnv * env, jclass /* selfClass */, jlong target)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getAllCustomVars");
	//-- get the CustomizationData instance for the target object.
	CustomizationData *const customizationData = fetchCustomizationDataFromObjId(target);
	if (!customizationData)
		return nullptr;

	//-- collect all local CustomizationVariable instances.
	// NOTE: if we allow multithreaded access to this code from script, we cannot use this static
	//       iterator data structure.
	s_customizationVariableIteratorData.m_customizationVariables.clear();
	s_customizationVariableIteratorData.m_variableNames.clear();

	customizationData->iterateOverVariables(CustomizationVariableCollector, &s_customizationVariableIteratorData, false);
	DEBUG_FATAL(s_customizationVariableIteratorData.m_variableNames.size() != s_customizationVariableIteratorData.m_customizationVariables.size(), ("mismatched return data sizes: %u/%u", s_customizationVariableIteratorData.m_variableNames.size(), s_customizationVariableIteratorData.m_customizationVariables.size()));

	//-- create a Java custom_var for each CustomizationVariable instance.
	LocalObjectArrayRefPtr customVarArray;

	if (!s_customizationVariableIteratorData.m_variableNames.empty())
	{
		// create the Java return array
		const int variableCount = static_cast<int>(s_customizationVariableIteratorData.m_variableNames.size());
		customVarArray = createNewObjectArray(variableCount, JavaLibrary::getClsCustomVar());
		if (customVarArray != LocalObjectArrayRef::cms_nullPtr)
		{
			// convert each C++ CustomizationVariable into a Java custom_var
			for (int i = 0; i < variableCount; ++i)
			{
				CustomizationVariable *const variable = s_customizationVariableIteratorData.m_customizationVariables[static_cast<size_t>(i)];
				NOT_NULL(variable);

				LocalRefPtr customVar = createCustomVar(target, s_customizationVariableIteratorData.m_variableNames[static_cast<size_t>(i)], *variable);
				if (customVar == LocalRef::cms_nullPtr)
				{
					// ran into problem, bail.
					customVarArray = LocalObjectArrayRef::cms_nullPtr;
					break;
				}

				// set new customVar into return array
				setObjectArrayElement(*customVarArray, i, *customVar);
				if (env->ExceptionCheck())
				{
					// ran into a problem, bail.
					customVarArray = LocalObjectArrayRef::cms_nullPtr;
					break;
				}
			}
		}
	}

	//-- release local CustomizationData reference
	customizationData->release();

	//-- return result
	if (customVarArray.get() != nullptr)
		return customVarArray->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsObjectInfoNamespace::getCustomVarByName(JNIEnv * /* env */, jclass /* selfClass */, jlong target, jstring varPathName)
{
	JavaStringParam localVarPathName(varPathName);

	//-- get the CustomizationData instance for the target object.
	CustomizationData *const customizationData = fetchCustomizationDataFromObjId(target);
	if (!customizationData)
		return nullptr;

	//-- get the CustomizationVariable for the specified variable name
	std::string nativeVarPathName;
	JavaLibrary::convert(localVarPathName, nativeVarPathName);

	CustomizationVariable *const variable = customizationData->findVariable(nativeVarPathName);
	if (!variable)
		return nullptr;

	//-- create a Java custom_var based on this CustomizationVariable
	LocalRefPtr newCustomVar = createCustomVar(target, nativeVarPathName, *variable);

	//-- release local references
	customizationData->release();

	return newCustomVar->getReturnValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getRangedIntCustomVarValue(JNIEnv * /* env */, jclass /* selfClass */, jlong target, jstring varPathName)
{
	JavaStringParam localVarPathName(varPathName);

	//-- get the CustomizationData instance for the target object.
	CustomizationData *const customizationData = fetchCustomizationDataFromObjId(target);
	if (!customizationData)
		return 0;

	//-- get the CustomizationVariable for the specified variable name
	std::string nativeVarPathName;
	JavaLibrary::convert(localVarPathName, nativeVarPathName);

	const RangedIntCustomizationVariable *const variable = dynamic_cast<const RangedIntCustomizationVariable*>(customizationData->findConstVariable(nativeVarPathName));
	if (!variable)
	{
		DEBUG_WARNING(true, ("getRangedIntCustomVarValue() tried to access var [%s] as ranged int, but doesn't exist or not right type.\n", nativeVarPathName.c_str()));
		return 0;
	}

	//-- retrieve the variable value
	const jint returnValue = static_cast<jint>(variable->getValue());

	//-- release local references
	customizationData->release();

	//-- return value
	return returnValue;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setRangedIntCustomVarValue(JNIEnv * /* env */, jclass /* selfClass */, jlong target, jstring varPathName, jint newValue)
{
	JavaStringParam localVarPathName(varPathName);

	//-- get the CustomizationData instance for the target object.
	CustomizationData *const customizationData = fetchCustomizationDataFromObjId(target);
	if (!customizationData)
		return;

	//-- get the CustomizationVariable for the specified variable name
	std::string nativeVarPathName;
	JavaLibrary::convert(localVarPathName, nativeVarPathName);

	RangedIntCustomizationVariable *const variable = dynamic_cast<RangedIntCustomizationVariable*>(customizationData->findVariable(nativeVarPathName));
	if (!variable)
	{
		DEBUG_WARNING(true, ("setRangedIntCustomVarValue() tried to access var [%s] as ranged int, but not right type.\n", nativeVarPathName.c_str()));
		return;
	}

	//-- set the variable value
	variable->setValue(static_cast<int>(newValue));

	//-- release local references
	customizationData->release();
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsObjectInfoNamespace::getPalcolorCustomVarSelectedColor(JNIEnv * /* env */, jclass /* selfClass */, jlong target, jstring varPathName)
{
	JavaStringParam localVarPathName(varPathName);

	//-- get the CustomizationData instance for the target object.
	CustomizationData *const customizationData = fetchCustomizationDataFromObjId(target);
	if (!customizationData)
		return 0;

	//-- get the CustomizationVariable for the specified variable name
	std::string nativeVarPathName;
	JavaLibrary::convert(localVarPathName, nativeVarPathName);

	const PaletteColorCustomizationVariable *const variable = dynamic_cast<const PaletteColorCustomizationVariable*>(customizationData->findConstVariable(nativeVarPathName));
	if (!variable)
	{
		DEBUG_WARNING(true, ("getPalcolorCustomVarSelectedColor() tried to access var [%s] as palette color var, but doesn't exist or is not the right type.\n", nativeVarPathName.c_str()));
		return 0;
	}

	//-- get the PackedArgb value for the color
	const int paletteIndex = variable->getValue();

	const PaletteArgb *const palette = variable->fetchPalette();
	bool error = false;
	const PackedArgb         color   = palette->getEntry(paletteIndex, error);
	palette->release();

	//-- release local references
	customizationData->release();

	if (error)
		return nullptr;

	//-- return value
	return createColor(color.getR(), color.getG(), color.getB(), color.getA())->getReturnValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setPalcolorCustomVarClosestColor(JNIEnv * /* env */, jclass /* selfClass */, jlong target, jstring varPathName, jint r, jint g, jint b, jint a)
{
	JavaStringParam localVarPathName(varPathName);

	CustomizationData *const customizationData = fetchCustomizationDataFromObjId(target);
	if (!customizationData)
	{
		DEBUG_WARNING(true, ("setPalcolorCustomVarClosestColor(): object doesn't have any customization variables.\n"));
		return;
	}

	//-- get the CustomizationVariable for the specified variable name
	std::string nativeVarPathName;
	JavaLibrary::convert(localVarPathName, nativeVarPathName);

	PaletteColorCustomizationVariable *const variable = dynamic_cast<PaletteColorCustomizationVariable*>(customizationData->findVariable(nativeVarPathName));
	if (!variable)
	{
		DEBUG_WARNING(true, ("setPalcolorCustomVarClosestColor() tried to access var [%s] as palette color var, but doesn't exist or is not the right type.\n", nativeVarPathName.c_str()));
		return;
	}

	//-- set variable to the palette entry closest to the given color
	variable->setClosestColor(PackedArgb(static_cast<uint8>(a), static_cast<uint8>(r), static_cast<uint8>(g), static_cast<uint8>(b)));
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsObjectInfoNamespace::getPalcolorCustomVarColors(JNIEnv * env, jclass /* selfClass */, jlong target, jstring varPathName)
{
	JavaStringParam localVarPathName(varPathName);

	//-- get the CustomizationData instance for the target object.
	CustomizationData *const customizationData = fetchCustomizationDataFromObjId(target);
	if (!customizationData)
		return 0;

	//-- get the CustomizationVariable for the specified variable name
	std::string nativeVarPathName;
	JavaLibrary::convert(localVarPathName, nativeVarPathName);

	const PaletteColorCustomizationVariable *const variable = dynamic_cast<const PaletteColorCustomizationVariable*>(customizationData->findConstVariable(nativeVarPathName));
	if (!variable)
	{
		DEBUG_WARNING(true, ("getPalcolorCustomVarColors() tried to access var [%s] as palette color var, but doesn't exist or is not the right type.\n", nativeVarPathName.c_str()));
		return 0;
	}

	//-- get the PackedArgb value for the color
	const PaletteArgb *const palette = variable->fetchPalette();

	//-- create the array of colors
	LocalObjectArrayRefPtr colorArray;
	const int    colorCount = palette->getEntryCount();

	if (colorCount > 0)
	{
		// create the Java return array
		colorArray = createNewObjectArray(colorCount, JavaLibrary::getClsColor());
		if (colorArray != LocalObjectArrayRef::cms_nullPtr)
		{
			// convert each palette entry color into a Java color instance
			for (int i = 0; i < colorCount; ++i)
			{
				bool error = false;
				const PackedArgb color     = palette->getEntry(i, error);

				if (error)
				{
					colorArray = LocalObjectArrayRef::cms_nullPtr;
					break;
				}

				LocalRefPtr      javaColor = createColor(color.getR(), color.getG(), color.getB(), color.getA());

				if (javaColor == LocalRef::cms_nullPtr)
				{
					// ran into problem, bail.
					colorArray = LocalObjectArrayRef::cms_nullPtr;
					break;
				}

				// set new color into return array
				setObjectArrayElement(*colorArray, i, *javaColor);
				if (env->ExceptionCheck())
				{
					// ran into a problem, bail.
					colorArray = LocalObjectArrayRef::cms_nullPtr;
					break;
				}
			}
		}
	}

	//-- release local references
	palette->release();
	customizationData->release();

	//-- return result
	if (colorArray.get() != nullptr)
		return colorArray->getReturnValue();
	return 0;
}

//-----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsObjectInfoNamespace::getCtsDestinationClusters(JNIEnv * /*env*/, jobject /*self*/)
{
	static std::set<std::string> * s_ctsDestinationClusters = nullptr;
	if (!s_ctsDestinationClusters)
	{
		s_ctsDestinationClusters = new std::set<std::string>;

		bool thisClusterAllowedToUploadCharacterData = false;
		
		{
			int index = 0;
			char const * result = 0;
			do
			{
				result = ConfigFile::getKeyString("TransferServer", "serverAllowedToUploadCharacterData", index++, 0);
				if ((result != 0) && (GameServer::getInstance().getClusterName() == std::string(result)))
					thisClusterAllowedToUploadCharacterData = true;
			}
			while (result && !thisClusterAllowedToUploadCharacterData);
		}

		if (thisClusterAllowedToUploadCharacterData)
		{
			int index = 0;
			char const * result = 0;
			do
			{
				result = ConfigFile::getKeyString("TransferServer", "serverAllowedToDownloadCharacterData", index++, 0);
				if ((result != 0) && (GameServer::getInstance().getClusterName() != std::string(result)))
					s_ctsDestinationClusters->insert(std::string(result));
			}
			while (result);
		}
	}

	if (s_ctsDestinationClusters->empty())
		return 0;

	LocalObjectArrayRefPtr valueArray = createNewObjectArray(s_ctsDestinationClusters->size(), JavaLibrary::getClsString());

	int i = 0;
	for (std::set<std::string>::const_iterator iter = s_ctsDestinationClusters->begin(); iter != s_ctsDestinationClusters->end(); ++iter)
	{
		JavaString jval(*iter);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsObjectInfoNamespace::getCurrentSceneName(JNIEnv * env, jobject self)
{
	return JavaString(ServerWorld::getSceneId().c_str()).getReturnValue();
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsObjectInfoNamespace::getClusterName(JNIEnv * env, jobject self)
{
	return JavaString(GameServer::getInstance().getClusterName().c_str()).getReturnValue();
}

//-----------------------------------------------------------------------

void JavaLibrary::setScriptVar(const ServerObject &object, const std::string & name, int value)
{
	LocalRefPtr obj_id = getObjId(object);
	if (obj_id == LocalRef::cms_nullPtr)
		return;

	JavaString jname(name);
	callVoidMethod(*obj_id, ms_midObjIdSetScriptVarInt, jname.getValue(), value);
}

//-----------------------------------------------------------------------

void JavaLibrary::setScriptVar(const ServerObject &object, const std::string & name, float value)
{
	LocalRefPtr obj_id = getObjId(object);
	if (obj_id == LocalRef::cms_nullPtr)
		return;

	JavaString jname(name);
	callVoidMethod(*obj_id, ms_midObjIdSetScriptVarFloat, jname.getValue(), value);
}

//-----------------------------------------------------------------------

void JavaLibrary::setScriptVar(const ServerObject &object, const std::string & name, const std::string & value)
{
	LocalRefPtr obj_id = getObjId(object);
	if (obj_id == LocalRef::cms_nullPtr)
		return;

	JavaString jname(name);
	JavaString jvalue(value);
	callVoidMethod(*obj_id, ms_midObjIdSetScriptVarString, jname.getValue(), jvalue.getValue());
}

//-----------------------------------------------------------------------

void JavaLibrary::clearScriptVars(const ServerObject & source)
{
	LocalRefPtr obj_id = getObjId(source);
	if (obj_id == LocalRef::cms_nullPtr)
		return;
	callVoidMethod(*obj_id, ms_midObjIdClearScriptVars);
}

//-----------------------------------------------------------------------

void JavaLibrary::packScriptVars(const ServerObject & source, std::vector<int8> & data)
{
	LocalByteArrayRefPtr jdata = callStaticByteArrayMethod(
		ms_clsObjId, ms_midObjIdPackScriptVars, (source.getNetworkId()).getValue());
	ScriptConversion::convert(*jdata, data);

	WARNING(data.size() > 60000, ("JavaLibrary::packScriptVars: Packing "
		"scriptvars for object %s, packed data size = %d",
		source.getNetworkId().getValueString().c_str(),
		static_cast<int>(data.size())));
}

//-----------------------------------------------------------------------

void JavaLibrary::packAllDeltaScriptVars()
{
	callStaticVoidMethod(ms_clsObjId, ms_midObjIdPackAllDeltaScriptVars);
}

//-----------------------------------------------------------------------

void JavaLibrary::unpackDeltaScriptVars(const ServerObject & target, const std::vector<int8> & data)
{
	LocalByteArrayRefPtr jdata;
	ScriptConversion::convert(data, jdata);
	callStaticVoidMethod(ms_clsObjId, ms_midObjIdUnpackDeltaScriptVars, (target.getNetworkId()).getValue(), jdata->getValue());
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::canEquipWearable(JNIEnv * env, jobject self, jlong player, jlong wearable)
{
	NetworkId const & wearableId = NetworkId(wearable);
	CreatureObject * const creatureObject = JavaLibrary::getCreatureThrow(env, player, "canEquipWearable(): player did not resolve to a CreatureObject", false);
	if (!creatureObject)
		return JNI_FALSE;

	Object * const wearableObj = NetworkIdManager::getObjectById(wearableId);
	if(!wearableObj)
	{
		DEBUG_WARNING(true, ("canEquipWearable got a bad object id %s", wearableId.getValueString().c_str()));
		return JNI_FALSE;
	}
	return (creatureObject->isAppearanceEquippable(wearableObj->asServerObject()->getSharedTemplateName())) ? JNI_TRUE : JNI_FALSE;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::sendScriptVarsToProxies(JNIEnv * env, jobject self, jlong obj, jbyteArray buffer)
{
    PROFILER_AUTO_BLOCK_DEFINE("JNI::sendScriptVarsToProxies");

    jboolean res = JNI_FALSE;

    if (obj != 0 && buffer != 0)
    {
	ServerObject * object = 0;
        if (JavaLibrary::getObject(obj, object))
        {
            ProxyList const &proxyList = object->getExposedProxyList();
            if (!proxyList.empty())
            {
                std::vector<int8> data;
                if (ScriptConversion::convert(buffer, data))
                {
                    if(data.size() > 0)
                    {
                        WARNING(data.size() > 60000, ("JavaLibrary::sendScriptVarsToProxies: "
                                "Packing scriptvars for object %s, packed data size = %d",
                                object->getNetworkId().getValueString().c_str(),
                                static_cast<int>(data.size())
                            )
                        );

                        uint32 const myProcessId = GameServer::getInstance().getProcessId();
                        uint32 const authProcessId = object->getAuthServerProcessId();

                        ProxyList syncServers;
			
			for (auto i = proxyList.begin(); i!=proxyList.end(); ++i) {
				if (myProcessId != authProcessId) {
					if (*i != myProcessId) {
						syncServers.insert(*i);
					}
				} else {
					syncServers.insert(*i);
				}
			}

			if (myProcessId != authProcessId && syncServers.find(authProcessId) == syncServers.end())
				syncServers.insert(authProcessId);

                        ServerMessageForwarding::begin(std::vector<uint32>(syncServers.begin(), syncServers.end()));

                        SynchronizeScriptVarDeltasMessage const deltasMessage(object->getNetworkId(), data);
                        ServerMessageForwarding::send(deltasMessage);

                        ServerMessageForwarding::end();
                    }
                }
            }
	}
	res = JNI_TRUE;
    }
    return res;
}


//-----------------------------------------------------------------------

void JavaLibrary::unpackScriptVars(const ServerObject & target, const std::vector<int8> & buffer)
{
	LocalByteArrayRefPtr jbuffer;
	ScriptConversion::convert(buffer, jbuffer);
	callStaticVoidMethod(ms_clsObjId, ms_midObjIdUnpackScriptVars, (target.getNetworkId()).getValue(), jbuffer->getValue());
}

//--------------------------------------------------------------------

/**
 * Kills a creature and returns true if it was killed or already dead
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to know about
 *
 * @return JNI_TRUE if the creature is incapaciated, JNI_FALSE if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::kill(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (creature->makeDead(NetworkId::cms_invalid, NetworkId::cms_invalid))
		return JNI_TRUE;
	return JNI_FALSE;
}

//----------------------------------------------------------------------

/**
 * Kills a player.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player
 * @param killer		who killed the player (may be nullptr)
 *
 * @return the corpse id of the player, or nullptr on error
 */
jobject JNICALL ScriptMethodsObjectInfoNamespace::killPlayer(JNIEnv *env, jobject self, jobject player, jobject killer)
{
	static const std::string corpseTemplateName("object/tangible/container/corpse/player_corpse.iff");

	CreatureObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return 0;

	NetworkId killerId;
	if (killer != 0)
		killerId = JavaLibrary::getNetworkId(killer);

	// make a corpse object for the player
	ServerObject *cell = 0;
	CellProperty * const cellProperty = playerObject->getParentCell();
	if (cellProperty && cellProperty != CellProperty::getWorldCellProperty())
		cell = safe_cast<ServerObject *>(&cellProperty->getOwner());

	Transform tr;
	tr.setPosition_p(playerObject->getPosition_p());
	ServerObject * corpse = ServerWorld::createNewObject(corpseTemplateName, tr, cell, true);
	if (corpse == nullptr)
		return 0;

	if (playerObject->makeDead(killerId, corpse->getNetworkId()))
	{
		if (!cell)
			corpse->addToWorld();
		LocalRefPtr corpseId = JavaLibrary::getObjId(*corpse);
		return corpseId->getReturnValue();
	}
	else
	{
		corpse->permanentlyDestroy(DeleteReasons::SetupFailed);
		return 0;
	}
}	// JavaLibrary::killPlayer

//----------------------------------------------------------------------

/**
 * Resurrects a creature and returns true if it is resurrected or not already dead
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to know about
 *
 * @return JNI_TRUE if the creature is incapaciated, JNI_FALSE if not or error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::resurrect(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (creature->makeNotDead())
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::resurrect

//----------------------------------------------------------------------

/**
* Removes all the attribute and skill mod modifiers from a creature.
* Does NOT remove buffs, only the effects so be sure to manually remove
* any buffs for which you need the icons to go away.
*
* @param env		    Java environment
* @param self		    class calling this function
* @param target		creature we want to remove all mods from
*
* @return JNI_TRUE if the creature is incapaciated, JNI_FALSE if not or error
*/
jboolean JNICALL ScriptMethodsObjectInfoNamespace::removeAllAttributeAndSkillmodMods(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->removeAllAttributeAndSkillmodMods();
	return JNI_TRUE;
}	// JavaLibrary::removeAllAttributeAndSkillmodMods

//----------------------------------------------------------------------


/**
 * Sets the invulnerable flag on an object.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param target			object we want to set
 * @param invulnerable		the new invulnerable state of the object
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setInvulnerable(JNIEnv *env, jobject self, jlong target, jboolean invulnerable)
{
	UNREF(self);

	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	object->setInvulnerable(invulnerable);
	return JNI_TRUE;
}	// JavaLibrary::setInvulnerable

//----------------------------------------------------------------------

/**
 * Returns the invulnerable flag of an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to get
 *
 * @return the invulnerable flag on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isInvulnerable(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	return object->isInvulnerable();
}	// JavaLibrary::isInvulnerable

//----------------------------------------------------------------------

/**
 * Returns the complexity of an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to get
 *
 * @return the complexity, or -1 on error
 */
jfloat JNICALL ScriptMethodsObjectInfoNamespace::getComplexity(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return -1;

	return object->getComplexity();
}	// JavaLibrary::getComplexity

//----------------------------------------------------------------------

/**
 * Sets the complexity of an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		object we want to set
 * @param complexity	complexity of the object
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setComplexity(JNIEnv *env, jobject self, jlong target, jfloat complexity)
{
	UNREF(self);

	ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	object->setComplexity(complexity);
	return JNI_TRUE;
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getGameObjectType(JNIEnv *env, jobject self, jlong obj)
{
	UNREF (self);

	ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(obj, object))
		return JNI_FALSE;

	return object->getGameObjectType ();
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getGameObjectTypeFromTemplate(JNIEnv *env, jobject self, jstring templateName)
{
	std::string templateString;
	if (!JavaLibrary::convert(JavaStringParam(templateName), templateString))
		return 0;

	return getGameObjectTypeFromTemplateCrc(env, self, Crc::calculate(templateString.c_str()));
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getGameObjectTypeFromTemplateCrc(JNIEnv *env, jobject self, jint templateCrc)
{
	const ObjectTemplate * objectTemplate = ObjectTemplateList::fetch(templateCrc);
	if (objectTemplate == nullptr)
		return 0;

	const std::string & sharedTemplateName = safe_cast<const ServerObjectTemplate *>(objectTemplate)->getSharedTemplate();
	const ObjectTemplate * sharedTemplate = ObjectTemplateList::fetch(sharedTemplateName);
	objectTemplate->releaseReference();
	if (sharedTemplate == nullptr)
		return 0;

	jint got = safe_cast<const SharedObjectTemplate *>(sharedTemplate)->getGameObjectType();
	sharedTemplate->releaseReference();
	return got;
}

//----------------------------------------------------------------------

jstring  JNICALL ScriptMethodsObjectInfoNamespace::getGameObjectTypeName(JNIEnv *env, jobject self, jint type)
{
	UNREF (self);

	return JavaString(GameObjectTypes::getCanonicalName (type)).getReturnValue();
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getGameObjectTypeFromName(JNIEnv *, jobject , jstring typeName)
{
	std::string typeNameString;
	if (!JavaLibrary::convert(JavaStringParam(typeName), typeNameString))
		return 0;
	return GameObjectTypes::getGameObjectType(typeNameString);
}

//----------------------------------------------------------------------

/**
 * Returns if an object is an object being controlled by a god-mode client.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object to test
 *
 * @return true if the object is god-mode controlled, false if not
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isGod(JNIEnv *env, jobject self, jlong target)
{
	UNREF (self);

	const ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	if (object->getClient() == nullptr)
		return JNI_FALSE;

	return object->getClient()->isGod();
}	// JavaLibrary::isGod

//----------------------------------------------------------------------

/**
 * Returns the god-level of a player.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object to test
 *
 * @return the god level
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getGodLevel(JNIEnv *env, jobject self, jlong target)
{
	UNREF (self);

	const ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	if (object->getClient() == nullptr)
		return 0;

	if (object->getClient()->isGod())
		return object->getClient()->getGodLevel();
	return 0;
}	// JavaLibrary::getGodLevel

//----------------------------------------------------------------------

/**
 * Returns an object's counter.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 *
 * @return the counter value
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getCount(JNIEnv *env, jobject self, jlong target)
{
	// both Tangible and Intangible have counters, so we have to test for both
	// cases

	const TangibleObject * tangibleObject = nullptr;
	if (JavaLibrary::getObject(target, tangibleObject))
	{
		return tangibleObject->getCount();
	}

	const IntangibleObject * intangibleObject = nullptr;
	if (JavaLibrary::getObject(target, intangibleObject))
	{
		return intangibleObject->getCount();
	}

	return 0;
}	// JavaLibrary::getCount

//----------------------------------------------------------------------

/**
 * Sets an object's counter.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 * @param value			the counter value
 *
 * @return true if the counter was set, false if there was an error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setCount(JNIEnv *env, jobject self, jlong target, jint value)
{
	// both Tangible and Intangible have counters, so we have to test for both
	// cases

	TangibleObject * tangibleObject = nullptr;
	if (JavaLibrary::getObject(target, tangibleObject))
	{
		tangibleObject->setCount(value);
		return JNI_TRUE;
	}

	IntangibleObject * intangibleObject = nullptr;
	if (JavaLibrary::getObject(target, intangibleObject))
	{
		intangibleObject->setCount(value);
		return JNI_TRUE;
	}

	return JNI_FALSE;
}	// JavaLibrary::setCount

//----------------------------------------------------------------------

/**
 * Increments (or decrements, if passed a negative value) an object's counter.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 * @param delta			number to add to the current count
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::incrementCount(JNIEnv *env, jobject self, jlong target, jint delta)
{
	// both Tangible and Intangible have counters, so we have to test for both
	// cases

	TangibleObject * tangibleObject = nullptr;
	if (JavaLibrary::getObject(target, tangibleObject))
	{
		tangibleObject->incrementCount(delta);
		return JNI_TRUE;
	}

	IntangibleObject * intangibleObject = nullptr;
	if (JavaLibrary::getObject(target, intangibleObject))
	{
		intangibleObject->incrementCount(delta);
		return JNI_TRUE;
	}

	return JNI_FALSE;
}	// JavaLibrary::incrementCount

//----------------------------------------------------------------------

/**
 * Returns an object's condition flags.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 *
 * @return the object's condition flags
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getCondition(JNIEnv *env, jobject self, jlong target)
{
	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	return object->getCondition();
}	// JavaLibrary::getCondition

//----------------------------------------------------------------------

/**
 * Tests if an object has a condition flag set.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 * @param condition		the condition flag to test
 *
 * @return true if the condition flag is set, false if not
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::hasCondition(JNIEnv *env, jobject self, jlong target, jint condition)
{
	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	return object->hasCondition(condition);
}	// JavaLibrary::hasCondition

//----------------------------------------------------------------------

/**
 * Sets a condition flag on an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 * @param condition		the condition flag
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setCondition(JNIEnv *env, jobject self, jlong target, jint condition)
{
	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	object->setCondition(condition);
	return JNI_TRUE;
}	// JavaLibrary::setCondition

//----------------------------------------------------------------------

/**
 * Clears a condition flag on an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 * @param condition		the condition flag
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::clearCondition(JNIEnv *env, jobject self, jlong target, jint condition)
{
	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	object->clearCondition(condition);
	return JNI_TRUE;
}	// JavaLibrary::clearCondition

//----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectInfoNamespace::getGroupObject(JNIEnv *env, jobject self, jlong obj)
{
	UNREF(self);

	CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(obj, creature))
		return 0;

	GroupObject *group = creature->getGroup();
	NetworkId networkId = group ? group->getNetworkId() : NetworkId::cms_invalid;
	return networkId.getValue();
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsObjectInfoNamespace::getGroupName(JNIEnv *env, jobject self, jlong obj)
{
	UNREF(self);

	GroupObject *group = 0;
	if (!JavaLibrary::getObject(obj, group))
		return 0;

	return JavaString(group->getGroupName().c_str()).getReturnValue();
}

//----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsObjectInfoNamespace::getGroupMemberIds(JNIEnv *env, jobject self, jlong obj)
{
	UNREF(self);

	GroupObject *group = 0;
	if (!JavaLibrary::getObject(obj, group))
		return 0;

	GroupObject::GroupMemberVector const &members = group->getGroupMembers();
	LocalLongArrayRefPtr items = createNewLongArray(members.size());
	if (items == LocalLongArrayRef::cms_nullPtr)
		return 0;

	int index = 0;
	jlong jlongTmp;
	for (GroupObject::GroupMemberVector::const_iterator i = members.begin(); i != members.end(); ++i)
	{
		jlongTmp = ((*i).first).getValue();
		setLongArrayRegion(*items, index, 1, &jlongTmp);
		++index;
	}

	return items->getReturnValue();
}

//----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsObjectInfoNamespace::getGroupMemberNames(JNIEnv *env, jobject self, jlong obj)
{
	UNREF(self);

	GroupObject *group = 0;
	if (!JavaLibrary::getObject(obj, group))
		return 0;

	GroupObject::GroupMemberVector const &members = group->getGroupMembers();
	// create the string array to return
	LocalObjectArrayRefPtr strings = createNewObjectArray(members.size(), JavaLibrary::getClsString());
	if (strings == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	int index = 0;
	for (GroupObject::GroupMemberVector::const_iterator i = members.begin(); i != members.end(); ++i)
	{
		JavaString jval((*i).second.c_str());
		setObjectArrayElement(*strings, index, jval);
		++index;
	}

	return strings->getReturnValue();
}

//----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectInfoNamespace::getGroupLeaderId(JNIEnv *env, jobject self, jlong group)
{
	UNREF(self);

	GroupObject *groupObj = 0;
	if (!JavaLibrary::getObject(group, groupObj))
		return 0;

	return (groupObj->getGroupLeaderId()).getValue();
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getGroupSize(JNIEnv *env, jobject self, jlong group)
{
	UNREF(self);

	GroupObject *groupObj = 0;
	if (!JavaLibrary::getObject(group, groupObj))
		return 0;

	return static_cast<jint>(groupObj->getGroupMembers().size());
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getPCGroupSize(JNIEnv *env, jobject self, jlong group)
{
	UNREF(self);

	GroupObject *groupObj = 0;
	if (!JavaLibrary::getObject(group, groupObj))
		return 0;

	return static_cast<jint>(groupObj->getPCMemberCount());
}

//----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectInfoNamespace::getGroupMasterLooterId(JNIEnv *env, jobject /*self*/, jlong group)
{
	GroupObject * groupObject = 0;
	if (!JavaLibrary::getObject(group, groupObject))
	{
		return 0;
	}

	return (groupObject->getLootMasterId()).getValue();
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getGroupLootRule(JNIEnv *env, jobject /*self*/, jlong group)
{
	GroupObject * groupObject = 0;
	if (!JavaLibrary::getObject(group, groupObject))
	{
		return 0;
	}

	return static_cast<jint>(groupObject->getLootRule());
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::setGroupLootRule(JNIEnv *env, jobject /*self*/, jlong group, jint value)
{
	GroupObject * groupObject = 0;
	if (!JavaLibrary::getObject(group, groupObject))
	{
		return JNI_FALSE;
	}

	groupObject->setLootRule(value);
	return JNI_TRUE;
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::openLotteryWindow(JNIEnv *env, jobject /*self*/, jlong playerToSendTo, jlong containerToObserve)
{
	ServerObject * container = 0;
	if (!JavaLibrary::getObject(containerToObserve, container))
	{
		DEBUG_REPORT_LOG(true, ("openLotteryWindow: container == 0.\n"));
		return;
	}

	CreatureObject * player = 0;
	if (!JavaLibrary::getObject(playerToSendTo, player))
	{
		DEBUG_REPORT_LOG(true, ("openLotteryWindow: player == 0.\n"));
		return;
	}

	if (ContainerInterface::getContainer(*container) == 0)
	{
		DEBUG_REPORT_LOG(true, ("commandFuncOpenLotteryContainer: ContainerInterface::getContainer == 0.\n"));
		return;
	}

	Client *client = player->getClient();
	if (client != 0)
	{
		int const sequence = -1;
		std::string const slotName;
		client->observeContainer(*container, sequence, slotName);
	}
	else
		{
		DEBUG_REPORT_LOG(true, ("commandFuncOpenLotteryContainer: unable to get client\n"));
	}

	Controller * const controller = player->getController();

			if (controller != 0)
			{
				typedef MessageQueueGenericValueType<NetworkId> Message;
		Message * const message = new Message(container->getNetworkId());

				controller->appendMessage(
			CM_groupOpenLotteryWindowOnClient,
					0.0f,
					message,
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getPerformanceType(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;
	return creature->getPerformanceType();
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setPerformanceType(JNIEnv *env, jobject self, jlong target, jint performanceType)
{
	UNREF(self);

	CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return;
	creature->setPerformanceType(performanceType);
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getPerformanceStartTime(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;
	return creature->getPerformanceStartTime();
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setPerformanceStartTime(JNIEnv *env, jobject self, jlong target, jint performanceStartTime)
{
	UNREF(self);

	CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return;
	creature->setPerformanceStartTime(performanceStartTime);
}

//----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectInfoNamespace::getPerformanceListenTarget(JNIEnv *env, jobject self, jlong actor)
{
	UNREF(self);

	CreatureObject *actorCreature = 0;
	if (!JavaLibrary::getObject(actor, actorCreature))
		return 0;
	return (actorCreature->getPerformanceListenTarget()).getValue();
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setPerformanceListenTarget(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	UNREF(self);

	CreatureObject *actorCreature = 0;
	if (!JavaLibrary::getObject(actor, actorCreature))
		return;
	CreatureObject *targetCreature = 0;
	JavaLibrary::getObject(target, targetCreature);
	actorCreature->setPerformanceListenTarget(targetCreature ? targetCreature->getNetworkId() : NetworkId::cms_invalid);
}

//----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectInfoNamespace::getPerformanceWatchTarget(JNIEnv *env, jobject self, jlong actor)
{
	UNREF(self);

	CreatureObject *actorCreature = 0;
	if (!JavaLibrary::getObject(actor, actorCreature))
		return 0;
	return (actorCreature->getPerformanceWatchTarget()).getValue();
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setPerformanceWatchTarget(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	UNREF(self);

	CreatureObject *actorCreature = 0;
	if (!JavaLibrary::getObject(actor, actorCreature))
		return;
	CreatureObject *targetCreature = 0;
	JavaLibrary::getObject(target, targetCreature);
	actorCreature->setPerformanceWatchTarget(targetCreature ? targetCreature->getNetworkId() : NetworkId::cms_invalid);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getInstrumentVisualId(JNIEnv *env, jobject self, jlong who)
{
	UNREF(self);

	CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(who, creature))
		return 0;
	return creature->getInstrumentVisualId();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getInstrumentAudioId(JNIEnv *env, jobject self, jlong who)
{
	UNREF(self);

	CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(who, creature))
		return 0;
	return creature->getInstrumentAudioId();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::sendMusicFlourish(JNIEnv *env, jobject self, jlong performer, jint flourishIndex)
{
	CreatureObject *creature = 0;
	if (JavaLibrary::getObject(performer, creature))
		creature->sendMusicFlourish(flourishIndex);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::showFlyText(JNIEnv *env, jobject self, jlong emitterId, jobject outputTextId, jfloat scale, jint r, jint g, jint b)
{
	//-- Get the TangibleObject for the emitter.
	TangibleObject *emitterObject = 0;
	if (!JavaLibrary::getObject(emitterId, emitterObject) || !emitterObject)
	{
		DEBUG_WARNING(true, ("showFlyText: emitterId parameter is not a TangibleObject, cannot show fly text."));
		return;
	}

	//-- Get the StringId from Java.
	StringId  gameOutputTextId;
	if (!ScriptConversion::convert(outputTextId, gameOutputTextId))
	{
		DEBUG_WARNING(true, ("showFlyText: failed to retrieve string_id outputTextId from Java."));
		return;
	}

	//-- Request the object to render the text.
	emitterObject->showFlyText(gameOutputTextId, scale, r, g, b);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::showFlyTextPrivate(JNIEnv *env, jobject self, jlong emitterId, jlong recipientId, jobject outputTextId, jfloat scale, jint r, jint g, jint b, jboolean showInChatBox)
{
	//-- Get the Object for the recipient.
	ServerObject *recipientObject = 0;
	if (!JavaLibrary::getObject(recipientId, recipientObject) || !recipientObject)
	{
		DEBUG_WARNING(true, ("showFlyTextPrivate: recipientObject parameter is not a ServerObject, cannot show fly text."));
		return;
	}

	//-- Ensure recipient is player-controlled.
	if (!recipientObject->isPlayerControlled())
		return;

	//-- Get the TangibleObject for the emitter.
	TangibleObject *emitterObject = 0;
	if (!JavaLibrary::getObject(emitterId, emitterObject) || !emitterObject)
	{
		DEBUG_WARNING(true, ("showFlyTextPrivate: emitterId parameter is not a TangibleObject, cannot show fly text."));
		return;
	}

	//-- Get the StringId from Java.
	StringId  gameOutputTextId;
	if (!ScriptConversion::convert(outputTextId, gameOutputTextId))
	{
		DEBUG_WARNING(true, ("showFlyTextPrivate: failed to retrieve string_id outputTextId from Java."));
		return;
	}

	//-- Request the recipient's client to have the emitter object render the text.
	Controller *const controller = recipientObject->getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("showFlyTextPrivate: object [%s] has no controller.", recipientObject->getNetworkId().getValueString().c_str()));
		return;
	}

	int textFlags = MessageQueueShowFlyText::F_private;

	if (showInChatBox)
		textFlags |= MessageQueueShowFlyText::F_showInChatBox;

	MessageQueueShowFlyText * const msg = new MessageQueueShowFlyText (emitterObject->getNetworkId(), gameOutputTextId, scale, r, g, b, textFlags);

	//-- Send the showFlyText message only to the authoratative recipient client.
	controller->appendMessage(static_cast<int>(CM_showFlyText), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::showCombatText(JNIEnv *env, jobject self, jlong defenderId, jlong attackerId, jobject outputTextId, jfloat scale, jint r, jint g, jint b)
{
	//-- Get the TangibleObject for the emitter.
	TangibleObject *defenderObject = 0;
	if (!JavaLibrary::getObject(defenderId, defenderObject) || !defenderObject)
	{
		DEBUG_WARNING(true, ("showCombatText: defenderId parameter is not a TangibleObject, cannot show combat text."));
		return;
	}

	TangibleObject *attackerObject = 0;
	if (!JavaLibrary::getObject(attackerId, attackerObject) || !attackerObject)
	{
		DEBUG_WARNING(true, ("showCombatText: attackerId parameter is not a TangibleObject, cannot show combat text."));
		return;
	}

	//-- Get the StringId from Java.
	StringId  gameOutputTextId;
	if (!ScriptConversion::convert(outputTextId, gameOutputTextId))
	{
		DEBUG_WARNING(true, ("showCombatText: failed to retrieve string_id outputTextId from Java."));
		return;
	}

	//-- Request the object to render the text.
	defenderObject->showCombatText(*attackerObject, gameOutputTextId, scale, r, g, b);
}

void JNICALL ScriptMethodsObjectInfoNamespace::showCombatTextPrivate(JNIEnv *env, jobject self, jlong defenderId, jlong attackerId, jlong clientObjectId, jobject outputTextId, jfloat scale, jint r, jint g, jint b)
{
	//-- Get the Object for the recipient.
	ServerObject *clientObject = 0;
	if (!JavaLibrary::getObject(clientObjectId, clientObject) || !clientObject)
	{
		DEBUG_WARNING(true, ("showCombatTextPrivate: clientObject parameter is not a ServerObject, cannot show fly text."));
		return;
	}

	//-- Ensure recipient is player-controlled.
	if (!clientObject->isPlayerControlled())
		return;

	//-- Get the TangibleObject for the emitter.
	TangibleObject *defenderObject = 0;
	if (!JavaLibrary::getObject(defenderId, defenderObject) || !defenderObject)
	{
		DEBUG_WARNING(true, ("showCombatTextPrivate: defenderId parameter is not a TangibleObject, cannot show combat text."));
		return;
	}

	TangibleObject *attackerObject = 0;
	if (!JavaLibrary::getObject(attackerId, attackerObject) || !attackerObject)
	{
		DEBUG_WARNING(true, ("showCombatText: attackerId parameter is not a TangibleObject, cannot show combat text."));
		return;
	}


	//-- Get the StringId from Java.
	StringId  gameOutputTextId;
	if (!ScriptConversion::convert(outputTextId, gameOutputTextId))
	{
		DEBUG_WARNING(true, ("showCombatTextPrivate: failed to retrieve string_id outputTextId from Java."));
		return;
	}

	//-- Request the recipient's client to have the emitter object render the text.
	Controller *const controller = clientObject->getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("showCombatTextPrivate: object [%s] has no controller.", clientObject->getNetworkId().getValueString().c_str()));
		return;
	}

	int textFlags = MessageQueueShowFlyText::F_private;

	MessageQueueShowCombatText * const msg = new MessageQueueShowCombatText (defenderObject->getNetworkId(), attackerObject->getNetworkId(), gameOutputTextId, scale, r, g, b, textFlags);

	//-- Send the showCombatText message only to the authoratative recipient client.
	controller->appendMessage(static_cast<int>(CM_showCombatText), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::showFlyTextProse           (JNIEnv *env, jobject self, jlong emitterId, jstring jStringOutputTextOOB, jfloat scale, jint r, jint g, jint b)
{
	//-- Get the TangibleObject for the emitter.
	TangibleObject *emitterObject = 0;
	if (!JavaLibrary::getObject(emitterId, emitterObject) || !emitterObject)
	{
		DEBUG_WARNING(true, ("showFlyTextProse: emitterId parameter is not a TangibleObject, cannot show fly text."));
		return;
	}

	JavaStringParam joutputTextOOB(jStringOutputTextOOB);
	Unicode::String outputTextOOB;
	if (!JavaLibrary::convert(joutputTextOOB, outputTextOOB))
	{
		DEBUG_WARNING(true, ("showFlyTextProse: could not convert outputTextProse to a string."));
		return;
	}

	//-- Request the object to render the text.
	emitterObject->showFlyText(outputTextOOB, scale, r, g, b);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::showFlyTextPrivateProse    (JNIEnv *env, jobject self, jlong emitterId, jlong recipientId, jstring jStringOutputTextOOB, jfloat scale, jint r, jint g, jint b, jboolean showInChatBox)
{
	//-- Get the Object for the recipient.
	ServerObject *recipientObject = 0;
	if (!JavaLibrary::getObject(recipientId, recipientObject) || !recipientObject)
	{
		DEBUG_WARNING(true, ("showFlyTextPrivateProse: recipientObject parameter is not a ServerObject, cannot show fly text."));
		return;
	}

	//-- Ensure recipient is player-controlled.
	if (!recipientObject->isPlayerControlled())
		return;

	//-- Get the TangibleObject for the emitter.
	TangibleObject *emitterObject = 0;
	if (!JavaLibrary::getObject(emitterId, emitterObject) || !emitterObject)
	{
		DEBUG_WARNING(true, ("showFlyTextPrivateProse: emitterId parameter is not a TangibleObject, cannot show fly text."));
		return;
	}

	JavaStringParam joutputTextOOB(jStringOutputTextOOB);
	Unicode::String outputTextOOB;
	if (!JavaLibrary::convert(joutputTextOOB, outputTextOOB))
	{
		DEBUG_WARNING(true, ("showFlyTextPrivateProse: could not convert outputTextProse to a string."));
		return;
	}

	//-- Request the recipient's client to have the emitter object render the text.
	Controller *const controller = recipientObject->getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("showFlyTextPrivateProse: object [%s] has no controller.", recipientObject->getNetworkId().getValueString().c_str()));
		return;
	}

	int textFlags = MessageQueueShowFlyText::F_private;

	if (showInChatBox)
		textFlags |= MessageQueueShowFlyText::F_showInChatBox;

	MessageQueueShowFlyText * const msg = new MessageQueueShowFlyText (emitterObject->getNetworkId(), outputTextOOB, scale, r, g, b, textFlags);

	//-- Send the showFlyText message only to the authoratative recipient client.
	controller->appendMessage(static_cast<int>(CM_showFlyText), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::showFlyTextPrivateProseWithFlags    (JNIEnv *env, jobject self, jlong emitterId, jlong recipientId, jstring jStringOutputTextOOB, jfloat scale, jint r, jint g, jint b, jint flags)
{
	//-- Get the Object for the recipient.
	ServerObject *recipientObject = 0;
	if (!JavaLibrary::getObject(recipientId, recipientObject) || !recipientObject)
	{
		DEBUG_WARNING(true, ("showFlyTextPrivateProse: recipientObject parameter is not a ServerObject, cannot show fly text."));
		return;
	}

	//-- Ensure recipient is player-controlled.
	if (!recipientObject->isPlayerControlled())
		return;

	//-- Get the TangibleObject for the emitter.
	TangibleObject *emitterObject = 0;
	if (!JavaLibrary::getObject(emitterId, emitterObject) || !emitterObject)
	{
		DEBUG_WARNING(true, ("showFlyTextPrivateProse: emitterId parameter is not a TangibleObject, cannot show fly text."));
		return;
	}

	JavaStringParam joutputTextOOB(jStringOutputTextOOB);
	Unicode::String outputTextOOB;
	if (!JavaLibrary::convert(joutputTextOOB, outputTextOOB))
	{
		DEBUG_WARNING(true, ("showFlyTextPrivateProse: could not convert outputTextProse to a string."));
		return;
	}

	//-- Request the recipient's client to have the emitter object render the text.
	Controller *const controller = recipientObject->getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("showFlyTextPrivateProse: object [%s] has no controller.", recipientObject->getNetworkId().getValueString().c_str()));
		return;
	}

	int textFlags = MessageQueueShowFlyText::F_private | flags;

	MessageQueueShowFlyText * const msg = new MessageQueueShowFlyText (emitterObject->getNetworkId(), outputTextOOB, scale, r, g, b, textFlags);

	//-- Send the showFlyText message only to the authoratative recipient client.
	controller->appendMessage(static_cast<int>(CM_showFlyText), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::showCombatTextProse        (JNIEnv *env, jobject self, jlong defenderId, jlong attackerId, jstring jStringOutputTextOOB, jfloat scale, jint r, jint g, jint b)
{
	//-- Get the TangibleObject for the emitter.
	TangibleObject *defenderObject = 0;
	if (!JavaLibrary::getObject(defenderId, defenderObject) || !defenderObject)
	{
		DEBUG_WARNING(true, ("showCombatTextProse: defenderId parameter is not a TangibleObject, cannot show combat text."));
		return;
	}

	TangibleObject *attackerObject = 0;
	if (!JavaLibrary::getObject(attackerId, attackerObject) || !attackerObject)
	{
		DEBUG_WARNING(true, ("showCombatTextProse: attackerId parameter is not a TangibleObject, cannot show combat text."));
		return;
	}

	JavaStringParam joutputTextOOB(jStringOutputTextOOB);
	Unicode::String outputTextOOB;
	if (!JavaLibrary::convert(joutputTextOOB, outputTextOOB))
	{
		DEBUG_WARNING(true, ("showCombatTextProse: could not convert outputTextProse to a string."));
		return;
	}

	//-- Request the object to render the text.
	defenderObject->showCombatText(*attackerObject, outputTextOOB, scale, r, g, b);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::showCombatTextPrivateProse (JNIEnv *env, jobject self, jlong defenderId, jlong attackerId, jlong clientObjectId, jstring jStringOutputTextOOB, jfloat scale, jint r, jint g, jint b)
{
	//-- Get the Object for the recipient.
	ServerObject *clientObject = 0;
	if (!JavaLibrary::getObject(clientObjectId, clientObject) || !clientObject)
	{
		DEBUG_WARNING(true, ("showCombatTextPrivateProse: clientObject parameter is not a ServerObject, cannot show fly text."));
		return;
	}

	//-- Ensure recipient is player-controlled.
	if (!clientObject->isPlayerControlled())
		return;

	//-- Get the TangibleObject for the emitter.
	TangibleObject *defenderObject = 0;
	if (!JavaLibrary::getObject(defenderId, defenderObject) || !defenderObject)
	{
		DEBUG_WARNING(true, ("showCombatTextPrivateProse: defenderId parameter is not a TangibleObject, cannot show combat text."));
		return;
	}

	TangibleObject *attackerObject = 0;
	if (!JavaLibrary::getObject(attackerId, attackerObject) || !attackerObject)
	{
		DEBUG_WARNING(true, ("showCombatTextPrivateProse: attackerId parameter is not a TangibleObject, cannot show combat text."));
		return;
	}

	JavaStringParam joutputTextOOB(jStringOutputTextOOB);
	Unicode::String outputTextOOB;
	if (!JavaLibrary::convert(joutputTextOOB, outputTextOOB))
	{
		DEBUG_WARNING(true, ("showCombatTextPrivateProse: could not convert outputTextProse to a string."));
		return;
	}

	//-- Request the recipient's client to have the emitter object render the text.
	Controller *const controller = clientObject->getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("showCombatTextPrivateProse: object [%s] has no controller.", clientObject->getNetworkId().getValueString().c_str()));
		return;
	}

	int textFlags = MessageQueueShowFlyText::F_private;

	MessageQueueShowCombatText * const msg = new MessageQueueShowCombatText (defenderObject->getNetworkId(), attackerObject->getNetworkId(), outputTextOOB, scale, r, g, b, textFlags);

	//-- Send the showCombatText message only to the authoratative recipient client.
	controller->appendMessage(static_cast<int>(CM_showCombatText), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------


jlong JNICALL ScriptMethodsObjectInfoNamespace::getMaster(JNIEnv *env, jobject self, jlong target)
{
	CreatureObject const *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
	{
		ServerObject const *obj = 0;
		if (JavaLibrary::getObject(target, obj))
		{
			char errbuf[1024];
			snprintf(errbuf, sizeof(errbuf), "JavaLibrary::getMasterId called with non-creature target %s (%s)", obj->getNetworkId().getValueString().c_str(), obj->getObjectTemplateName());
			errbuf[sizeof(errbuf)-1] = '\0';
			JavaLibrary::throwInternalScriptError(errbuf);
		}
		else
			JavaLibrary::throwInternalScriptError("JavaLibrary::getMasterId called with target which could not be resolved");
		return 0;
	}
	return (creature->getMasterId()).getValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setMaster(JNIEnv *env, jobject self, jlong target, jlong master)
{
	CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		JavaLibrary::throwInternalScriptError("JavaLibrary::setMasterId called with bad target");
	else
		creature->setMasterId(NetworkId(master));
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getNumAI(JNIEnv *env, jobject self)
{
	return ObjectTracker::getNumAI();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getNumCreatures(JNIEnv *env, jobject self)
{
	return ObjectTracker::getNumCreatures();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getNumPlayers(JNIEnv *env, jobject self)
{
	return ObjectTracker::getNumPlayers();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getNumRunTimeRules(JNIEnv *env, jobject self)
{
	return ObjectTracker::getNumRunTimeRules();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getNumDynamicAI(JNIEnv *env, jobject self)
{
	return ObjectTracker::getNumDynamicAI();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getNumStaticAI(JNIEnv *env, jobject self)
{
	return ObjectTracker::getNumStaticAI();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getNumCombatAI(JNIEnv *env, jobject self)
{
	return ObjectTracker::getNumCombatAI();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getNumHibernatingAI(JNIEnv *env, jobject self)
{
	return ObjectTracker::getNumHibernatingAI();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getGroupLevel(JNIEnv *, jobject, jlong target)
{
	if (!target)
	{
		DEBUG_WARNING (true, ("JavaLibrary::getGroupLevel nullptr target "));
		return 0;
	}

	NetworkId targetId(target);
	if (targetId == NetworkId::cms_invalid)
	{
		DEBUG_WARNING (true, ("JavaLibrary::getGroupLevel invalid target "));
		return 0;
	}

	const GroupObject * const group = dynamic_cast<const GroupObject *>(ServerWorld::findObjectByNetworkId (targetId));
	if (!group)
	{
		DEBUG_WARNING (true, ("JavaLibrary::getGroupLevel target is not a group"));
		return 0;
	}

	return group->getGroupLevel ();
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getLevel(JNIEnv *env, jobject self, jlong target)
{
	jint result = 0;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return result;

	result = creature->getLevel();
	return result;
	}

jboolean JNICALL ScriptMethodsObjectInfoNamespace::setLevel(JNIEnv *env, jobject self, jlong target, jint forcedLevel)
{
	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return false;

	creature->setLevel(forcedLevel);
	return true;
	}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::recalculateLevel(JNIEnv *env, jobject self, jlong target)
	{
	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return false;

	creature->recalculateLevel();
	return true;
}


//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getHologramType(JNIEnv *env, jobject self, jlong target)
{
	jint result = -1;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return result;

	result = creature->getHologramType();
	return result;
}

jboolean JNICALL ScriptMethodsObjectInfoNamespace::setHologramType(JNIEnv *env, jobject self, jlong target, jint type)
{
	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return false;

	creature->setHologramType(type);
	return true;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::setVisibleOnMapAndRadar(JNIEnv *env, jobject self, jlong target, jboolean visible)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return false;

	creature->setVisibleOnMapAndRadar(visible);
	return true;
}

jboolean JNICALL ScriptMethodsObjectInfoNamespace::getVisibleOnMapAndRadar(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return true; // true is the default value - return true on failure case

	return creature->getVisibleOnMapAndRadar();
}	

//----------------------------------------------------------------------

/**
 * Returns the appearance file name of an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 *
 * @return the appearance name, or nullptr on error
 */
jstring JNICALL ScriptMethodsObjectInfoNamespace::getAppearance(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	const ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	const SharedObjectTemplate * sharedTemplate = object->getSharedTemplate();
	if (sharedTemplate == nullptr)
		return 0;

	JavaString appearance(sharedTemplate->getAppearanceFilename());
	return appearance.getReturnValue();
}	// JavaLibrary::getAppearance

// ----------------------------------------------------------------------

/**
 * Returns if an object is insured.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 *
 * @return true if the object is insured, false if not
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isInsured(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	return object->isInsured();
}	// JavaLibrary::isInsured

// ----------------------------------------------------------------------

/**
 * Returns if an object is auto-insured.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 *
 * @return true if the object is auto-insured, false if not
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isAutoInsured(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	return object->isInsured() && object->isUninsurable();
}	// JavaLibrary::isAutoInsured

// ----------------------------------------------------------------------

/**
 * Returns if an object is uninsurable.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the object
 *
 * @return true if the object is uninsurable, false if not
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::isUninsurable(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	return object->isUninsurable();
}	// JavaLibrary::isUninsurable

// ----------------------------------------------------------------------

/**
 * Returns all the objects that are in a player's (or creature's) inventory and
 * equipment. Special items, such as hair and Trandoshan feet, will not be
 * returned. Items that are not visible to the player will also not be returned.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player to get objects from
 *
 * @return an array of objects, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsObjectInfoNamespace::getInventoryAndEquipment(JNIEnv * env, jobject self, jlong player)
{
	UNREF(self);

	const CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return 0;

	std::vector<NetworkId> objectIds;

	// go through the player's inventory
	const ServerObject * inventoryObject = playerCreature->getInventory();
	if (inventoryObject != nullptr)
	{
		const VolumeContainer * inventoryContainer =
			ContainerInterface::getVolumeContainer(*inventoryObject);
		if (inventoryContainer != nullptr)
		{
			getGoodItemsFromContainer(*inventoryContainer, objectIds);
		}
		else
		{
			DEBUG_WARNING(true, ("JavaLibrary::getInventoryAndEquipment creature %s "
				"inventory object %s has no volume container",
				playerCreature->getNetworkId().getValueString().c_str(),
				inventoryObject->getNetworkId().getValueString().c_str()));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("JavaLibrary::getInventoryAndEquipment creature %s has "
			"no inventory object",
			playerCreature->getNetworkId().getValueString().c_str()));
	}

	// go through the player's equipment
	SlottedContainer const * const equipmentContainer = ContainerInterface::getSlottedContainer(*playerCreature);
	if (equipmentContainer != nullptr)
		getGoodItemsFromContainer(*equipmentContainer, objectIds);
	else
	{
		DEBUG_WARNING(true, ("JavaLibrary::getInventoryAndEquipment creature %s has "
			"no slotted container",
			playerCreature->getNetworkId().getValueString().c_str()));
	}

	if (!objectIds.empty())
	{
		LocalLongArrayRefPtr returnedIds;
		if (ScriptConversion::convert(objectIds, returnedIds))
			return returnedIds->getReturnValue();
	}
	return 0;
}	// JavaLibrary::getInventoryAndEquipment

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::setCheaterLevel(JNIEnv * env, jobject self, jlong player, jint level)
{
	UNREF(self);

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return JNI_FALSE;

	PlayerObject * const playerObj = PlayerCreatureController::getPlayerObject(playerCreature);
	if (playerObj != nullptr && playerObj->isAuthoritative())
	{
		playerObj->setCheaterLevel(static_cast<float>(level));
		return JNI_TRUE;
	}
	return JNI_FALSE;

}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getCheaterLevel(JNIEnv * env, jobject self, jlong player)
{
	UNREF(self);

	const CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return 0;

	PlayerObject const * const playerObj = PlayerCreatureController::getPlayerObject(playerCreature);
	if (playerObj != nullptr)
	{
		return static_cast<int>(playerObj->getCheaterLevel());
	}

	return 0;
}

// ----------------------------------------------------------------------

/**
 * Sets the house that a player owns.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player
 * @param houseId		the house
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setHouseId(JNIEnv * env, jobject self, jlong player, jlong houseId)
{
	UNREF(self);

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return JNI_FALSE;

	CachedNetworkId house(houseId);

	playerCreature->setHouse(house);
	return JNI_TRUE;
}	// JavaLibrary::setHouseId

// ----------------------------------------------------------------------

/**
 * Sets the house that a player owns.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player
 *
 * @return the house id, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectInfoNamespace::getHouseId(JNIEnv * env, jobject self, jlong player)
{
	UNREF(self);

	const CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return 0;

	return (playerCreature->getHouse()).getValue();
}	// JavaLibrary::getHouseId

// ----------------------------------------------------------------------

/**
 * Returns the draft schematic template name used to create a manufacture
 * schematic or factory crate.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param object		the manf schematic or factory id
 *
 * @return the draft schematic name, or nullptr on error
 */
jstring JNICALL ScriptMethodsObjectInfoNamespace::getDraftSchematic(JNIEnv * env, jobject self, jlong object)
{
	UNREF(self);

	const ManufactureObjectInterface * craftable = nullptr;

	const FactoryObject * factoryObject = nullptr;
	const ManufactureSchematicObject * manfSchematicObject = nullptr;
	if (JavaLibrary::getObject(object, manfSchematicObject))
	{
		craftable = manfSchematicObject;
	}
	else if (JavaLibrary::getObject(object, factoryObject))
	{
		craftable = factoryObject;
	}
	else
		return 0;

	const ConstCharCrcString draftSchematic(ObjectTemplateList::lookUp(
		craftable->getDraftSchematic()));
	if (draftSchematic.getString() == nullptr)
		return 0;

	return JavaString(draftSchematic.getString()).getReturnValue();
}	// JavaLibrary::getDraftSchematic

// ----------------------------------------------------------------------

/**
 * Returns the draft schematic template name crc used to create a manufacture
 * schematic or factory crate.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param object		the manf schematic or factory id
 *
 * @return the draft schematic name crc, or nullptr on error
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getDraftSchematicCrc(JNIEnv * env, jobject self, jlong object)
{
	UNREF(self);

	const ManufactureObjectInterface * craftable = nullptr;

	const FactoryObject * factoryObject = nullptr;
	const ManufactureSchematicObject * manfSchematicObject = nullptr;
	if (JavaLibrary::getObject(object, manfSchematicObject))
	{
		craftable = manfSchematicObject;
	}
	else if (JavaLibrary::getObject(object, factoryObject))
	{
		craftable = factoryObject;
	}
	else
		return 0;

	const ConstCharCrcString draftSchematic(ObjectTemplateList::lookUp(
		craftable->getDraftSchematic()));
	if (draftSchematic.getString() == nullptr)
		return 0;

	return draftSchematic.getCrc();
}	// JavaLibrary::getDraftSchematicCrc

// ----------------------------------------------------------------------

/**
 * Returns the draft schematic template name crc used to create an object.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object we want the source schematic of
 *
 * @return the draft schematic name crc, or 0 on error or if the object wasn't crafted
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getSourceDraftSchematic(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	return static_cast<jint>(object->getSourceDraftSchematic());
}	// JavaLibrary::getSourceDraftSchematic

// ----------------------------------------------------------------------

/**
 * Returns the "birth" date of a player character.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param playerId		the id of the player we want
 *
 * @return returns the birth date, in days since Jan 1st, 2001 (Jan 1st, 2001 = 0);
 *		returns -1 on error
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getPlayerBirthDate(JNIEnv * env, jobject self, jlong playerId)
{
	UNREF(self);

	const CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(playerId, playerCreature))
		return -1;

	const PlayerObject * player = PlayerCreatureController::getPlayerObject(
		playerCreature);
	if (player == nullptr)
		return -1;

	return player->getBornDate();
}	// JavaLibrary::getPlayerBirthDate

// ----------------------------------------------------------------------

/**
 * Returns the "birth" date that a player being created now would get.
 *
 * @param env			Java environment
 * @param self			class calling this function
 *
 * @return returns the birth date, in days since Jan 1st, 2001 (Jan 1st, 2001 = 0)
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getCurrentBirthDate(JNIEnv * env, jobject self)
{
	UNREF(self);

	return PlayerObject::getCurrentBornDate();
}	// JavaLibrary::getCurrentBirthDate

// ----------------------------------------------------------------------

/**
 * Returns the amount of time a player has been online.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param playerId		the id of the player we want
 *
 * @return
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getPlayerPlayedTime(JNIEnv * env, jobject self, jlong playerId)
{
	UNREF(self);

	const CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(playerId, playerCreature))
		return -1;

	const PlayerObject * player = PlayerCreatureController::getPlayerObject(
		playerCreature);
	if (player == nullptr)
		return -1;

	return player->getPlayedTime();
}	// JavaLibrary::getPlayerPlayedTime

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getBuildingCityId(JNIEnv *env, jobject self, jlong building)
{
	UNREF(self);
	BuildingObject *buildingObj = 0;
	if (JavaLibrary::getObject(building, buildingObj))
		return buildingObj->getCityId();
	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setBuildingCityId(JNIEnv *env, jobject self, jlong building, jint cityId)
{
	UNREF(self);
	BuildingObject *buildingObj = 0;
	if (JavaLibrary::getObject(building, buildingObj))
		buildingObj->setCityId(cityId);
}

// ----------------------------------------------------------------------

/**
 * Returns the default name that would be given an object created by a given
 * draft schematic.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param jschematicName	the schematic name
 *
 * @return the object name, or nullptr if the schematic doesn't exist
 */
jobject JNICALL ScriptMethodsObjectInfoNamespace::getProductNameFromSchematic(JNIEnv *env, jobject self,
	jstring jschematicName)
{
	JavaStringParam schematicNameParam(jschematicName);

	std::string schematicName;
	if (!JavaLibrary::convert(schematicNameParam, schematicName))
		return 0;

	const DraftSchematicObject * schematic = DraftSchematicObject::getSchematic(
		schematicName);
	if (schematic == nullptr)
		return 0;

	const ServerObjectTemplate * serverOt = schematic->getCraftedObjectTemplate();
	if (serverOt == nullptr)
		return 0;

	const std::string sharedTemplateName(serverOt->getSharedTemplate());
	const ObjectTemplate * ot = ObjectTemplateList::fetch(sharedTemplateName);
	if (ot == nullptr)
		return 0;

	const SharedObjectTemplate * sharedOt = dynamic_cast<const SharedObjectTemplate *>(
		ot);
	if (sharedOt == nullptr)
	{
		ot->releaseReference();
		return 0;
	}
	ot = nullptr;

	const StringId objectName(sharedOt->getObjectName());
	sharedOt->releaseReference();
	sharedOt = nullptr;

	LocalRefPtr jobjectName;
	if (!ScriptConversion::convert(objectName, jobjectName))
		return 0;
	return jobjectName->getReturnValue();
}	// JavaLibrary::getProductNameFromSchematic

// ----------------------------------------------------------------------

/**
 * Returns the default name that would be given an object created by a given
 * draft schematic.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param schematicCrc		the schematic name crc
 *
 * @return the object name, or nullptr if the schematic doesn't exist
 */
jobject JNICALL ScriptMethodsObjectInfoNamespace::getProductNameFromSchematicCrc(JNIEnv *env, jobject self,
	jint schematicCrc)
{
	const DraftSchematicObject * schematic = DraftSchematicObject::getSchematic(
		schematicCrc);
	if (schematic == nullptr)
		return 0;

	const ServerObjectTemplate * serverOt = schematic->getCraftedObjectTemplate();
	if (serverOt == nullptr)
		return 0;

	const std::string sharedTemplateName(serverOt->getSharedTemplate());
	const ObjectTemplate * ot = ObjectTemplateList::fetch(sharedTemplateName);
	if (ot == nullptr)
		return 0;

	const SharedObjectTemplate * sharedOt = dynamic_cast<const SharedObjectTemplate *>(
		ot);
	if (sharedOt == nullptr)
	{
		ot->releaseReference();
		return 0;
	}
	ot = nullptr;

	const StringId objectName(sharedOt->getObjectName());
	sharedOt->releaseReference();
	sharedOt = nullptr;

	LocalRefPtr jobjectName;
	if (!ScriptConversion::convert(objectName, jobjectName))
		return 0;
	return jobjectName->getReturnValue();
}	// JavaLibrary::getProductNameFromSchematicCrc

// ----------------------------------------------------------------------

/**
 * Gets the template of the item a draft schematic creates.
 *
 * @param env		     Java environment
 * @param self		     class calling this function
 * @param draftSchematic the draft schematic's template
 *
 * @return the template for the item the schematic creates, or nullptr on error
 */
jstring JNICALL ScriptMethodsObjectInfoNamespace::getTemplateCreatedFromSchematic(JNIEnv *env, jobject self,
	jstring draftSchematic)
{
	if (draftSchematic == 0)
		return 0;

	JavaStringParam jdraftSchematic(draftSchematic);
	std::string schematicName;
	if (!JavaLibrary::convert(jdraftSchematic, schematicName))
		return 0;

	return getTemplateCreatedFromSchematicCrc(env, self, Crc::calculate(
		schematicName.c_str()));
}	// JavaLibrary::getTemplateCreatedFromSchematic

// ----------------------------------------------------------------------

/**
 * Gets the template of the item a draft schematic creates.
 *
 * @param env		        Java environment
 * @param self		        class calling this function
 * @param draftSchematicCrc the draft schematic's template crc
 *
 * @return the template for the item the schematic creates, or nullptr on error
 */
jstring JNICALL ScriptMethodsObjectInfoNamespace::getTemplateCreatedFromSchematicCrc(JNIEnv *env, jobject self,
	jint draftSchematicCrc)
{
	UNREF(self);

	if (draftSchematicCrc == 0)
		return 0;

	const DraftSchematicObject * schematic = DraftSchematicObject::getSchematic(
		draftSchematicCrc);
	if (schematic == nullptr)
		return 0;

	const ServerObjectTemplate * serverOt = schematic->getCraftedObjectTemplate();
	if (serverOt == nullptr)
		return 0;

	JavaString templateName(serverOt->getName());
	return templateName.getReturnValue();
}	// JavaLibrary::getTemplateCreatedFromSchematicCrc

// ----------------------------------------------------------------------

/**
 * Gets the template of the item a draft schematic creates.
 *
 * @param env		     Java environment
 * @param self		     class calling this function
 * @param draftSchematic the draft schematic's template
 *
 * @return the template crc for the item the schematic creates, or 0 on error
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getTemplateCrcCreatedFromSchematic(JNIEnv *env, jobject self,
	jstring draftSchematic)
{
	if (draftSchematic == 0)
		return 0;

	JavaStringParam jdraftSchematic(draftSchematic);
	std::string schematicName;
	if (!JavaLibrary::convert(jdraftSchematic, schematicName))
		return 0;

	return getTemplateCrcCreatedFromSchematicCrc(env, self, Crc::calculate(
		schematicName.c_str()));
}	// JavaLibrary::getTemplateCrcCreatedFromSchematic

// ----------------------------------------------------------------------

/**
 * Gets the template of the item a draft schematic creates.
 *
 * @param env		        Java environment
 * @param self		        class calling this function
 * @param draftSchematicCrc the draft schematic's template crc
 *
 * @return the template crc for the item the schematic creates, or 0 on error
 */
jint JNICALL ScriptMethodsObjectInfoNamespace::getTemplateCrcCreatedFromSchematicCrc(JNIEnv *env, jobject self,
	jint draftSchematicCrc)
{
	UNREF(self);

	if (draftSchematicCrc == 0)
		return 0;

	const DraftSchematicObject * schematic = DraftSchematicObject::getSchematic(
		draftSchematicCrc);
	if (schematic == nullptr)
		return 0;

	const ServerObjectTemplate * serverOt = schematic->getCraftedObjectTemplate();
	if (serverOt == nullptr)
		return 0;

	return Crc::calculate(serverOt->getName());
}	// JavaLibrary::getTemplateCrcCreatedFromSchematicCrc

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::findObjectAnywhere(JNIEnv *env, jobject self, jlong target, jlong objectToNotify)
{
	NetworkId targetId(target);
	NetworkId notifyId(objectToNotify);

	if (targetId == NetworkId::cms_invalid || notifyId == NetworkId::cms_invalid)
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("findObjectAnywhere received a bad object in its parameters"));
	else
	{
		GenericValueTypeMessage<std::pair<uint32, std::pair<NetworkId, NetworkId> > > locateObject(
			"LocateObject",
			std::make_pair(
				GameServer::getInstance().getProcessId(),
				std::make_pair(targetId, notifyId)));
		GameServer::getInstance().sendToCentralServer(locateObject);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::findObjectAnywhereByTemplate(JNIEnv *env, jobject self, jstring templateName, jlong objectToNotify)
{
	NetworkId notifyId(objectToNotify);

	if (notifyId == NetworkId::cms_invalid)
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::findObjectAnywhereByTemplate received a bad notify object in its parameters"));
	else
	{
		JavaStringParam jspTemplateName(templateName);
		std::string     templateNameString;
		if (!JavaLibrary::convert(jspTemplateName, templateNameString))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::findObjectAnywhereByTemplate given bad template name string"));
		}
		else
		{
			if (!templateNameString.empty())
			{
				GenericValueTypeMessage<std::pair<uint32, std::pair<uint32, NetworkId> > > locatePlayer(
					"LocateObjectByTemplateName",
					std::make_pair(
					GameServer::getInstance().getProcessId(),
					std::make_pair(CrcLowerString::calculateCrc(templateNameString.c_str()), notifyId)));
				GameServer::getInstance().sendToCentralServer(locatePlayer);
			}
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::findPlayerAnywhereByPartialName(JNIEnv *env, jobject self, jstring targetName, jlong objectToNotify)
{
	NetworkId notifyId(objectToNotify);

	if (notifyId == NetworkId::cms_invalid)
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::findPlayerAnywhereByPartialName received a bad notify object in its parameters"));
	else
	{
		JavaStringParam playerName(targetName);
		std::string     playerNameString;
		if (!JavaLibrary::convert(playerName, playerNameString))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::findPlayerAnywhereByPartialName given bad player name string"));
		}
		else
		{
			if (!playerNameString.empty())
			{
				GenericValueTypeMessage<std::pair<uint32, std::pair<std::string, NetworkId> > > locatePlayer(
					"LocatePlayerByPartialName",
					std::make_pair(
						GameServer::getInstance().getProcessId(),
						std::make_pair(playerNameString, notifyId)));
				GameServer::getInstance().sendToCentralServer(locatePlayer);
			}
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::findWardenAnywhere(JNIEnv *env, jobject self, jlong objectToNotify)
{
	NetworkId notifyId(objectToNotify);

	if (notifyId == NetworkId::cms_invalid)
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::findWardenAnywhere received a bad notify object in its parameters"));
	else
	{
		GenericValueTypeMessage<std::pair<uint32, NetworkId> > locateWarden(
			"LocateWarden",
			std::make_pair(GameServer::getInstance().getProcessId(), notifyId));
		GameServer::getInstance().sendToCentralServer(locateWarden);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::findCreatureAnywhere(JNIEnv *env, jobject self, jstring creatureName, jlong objectToNotify)
{
	NetworkId notifyId(objectToNotify);

	if (notifyId == NetworkId::cms_invalid)
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::findCreatureAnywhere received a bad notify object in its parameters"));
	else
	{
		JavaStringParam jspCreatureName(creatureName);
		std::string     creatureNameString;
		if (!JavaLibrary::convert(jspCreatureName, creatureNameString))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::findCreatureAnywhere given bad creature name string"));
		}
		else
		{
			if (!creatureNameString.empty())
			{
				GenericValueTypeMessage<std::pair<uint32, std::pair<std::string, NetworkId> > > locatePlayer(
					"LocateCreatureByCreatureName",
					std::make_pair(
					GameServer::getInstance().getProcessId(),
					std::make_pair(creatureNameString, notifyId)));
				GameServer::getInstance().sendToCentralServer(locatePlayer);
			}
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::sendDirtyObjectMenuNotification(JNIEnv *env, jobject self, jlong jTarget)
{
	UNREF (env);
	UNREF (self);
	ServerObject *target = 0;
	if (JavaLibrary::getObject(jTarget, target) && target)
		target->sendDirtyObjectMenuNotification ();
	else
		DEBUG_WARNING(true, ("[designer bug] sendDirtyObjectMenuNotification script hook was passed an invalid target reference"));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::sendDirtyAttributesNotification(JNIEnv *env, jobject self, jlong jTarget)
{
	UNREF (env);
	UNREF (self);
	ServerObject *target = 0;
	if (JavaLibrary::getObject(jTarget, target) && target)
		target->sendDirtyAttributesNotification ();
	else
		DEBUG_WARNING(true, ("[designer bug] sendDirtyAttributesNotification script hook was passed an invalid target reference"));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::registerNamedObject(JNIEnv *env, jobject self, jstring name, jlong namedObject)
{
	JavaStringParam localName(name);
	std::string objectNameString;
	if (!JavaLibrary::convert(localName, objectNameString))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::registerNamedObject: bad name string"));
	else
	{
		NetworkId netId(namedObject);
		NamedObjectManager::registerNamedObjectId(objectNameString, netId);
	}
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectInfoNamespace::getNamedObject(JNIEnv *env, jobject self, jstring name)
{
	JavaStringParam localName(name);
	std::string objectNameString;
	if (!JavaLibrary::convert(localName, objectNameString))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getNamedObject: bad name string"));
	else
	{
		NetworkId const &namedObjectId = NamedObjectManager::getNamedObjectId(objectNameString);
		if (namedObjectId != NetworkId::cms_invalid)
			return namedObjectId.getValue();
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getNamedObject: no such object named '%s'", objectNameString.c_str()));
	}
	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::requestPreloadCompleteTrigger(JNIEnv *env, jobject self, jlong target)
{
	ServerObject *targetObject = 0;
	if (!JavaLibrary::getObject(target, targetObject))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::requestPreloadCompleteTrigger: bad target object"));
	else
		ServerWorld::requestPreloadCompleteTrigger(*targetObject);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::activateQuest(JNIEnv * env, jobject self, jlong target, jint questId)
{
	CreatureObject * playerCreature = nullptr;
	if (JavaLibrary::getObject(target, playerCreature))
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(playerCreature);
		if(player)
		{
			player->activateQuest(questId);
		}
	}

}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::deactivateQuest(JNIEnv * env, jobject self, jlong target, jint questId)
{
	CreatureObject * playerCreature = nullptr;
	if (JavaLibrary::getObject(target, playerCreature))
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(playerCreature);
		if(player)
		{
			player->deactivateQuest(questId);
		}
	}

}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::completeQuest(JNIEnv * env, jobject self, jlong target, jint questId)
{
	CreatureObject * playerCreature = nullptr;
	if (JavaLibrary::getObject(target, playerCreature))
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(playerCreature);
		if(player)
		{
			player->completeQuest(questId);
		}
	}

}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::isQuestComplete(JNIEnv * env, jobject self, jlong target, jint questId)
{
	jboolean result = false;
	const CreatureObject * playerCreature = nullptr;
	if (JavaLibrary::getObject(target, playerCreature))
	{
		const PlayerObject * player = PlayerCreatureController::getPlayerObject(playerCreature);
		if(player)
		{
			result = player->isQuestComplete(questId);
		}
	}
	return result;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::isQuestActive(JNIEnv * env, jobject self, jlong target, jint questId)
{
	jboolean result = false;
	const CreatureObject * playerCreature = nullptr;
	if (JavaLibrary::getObject(target, playerCreature))
	{
		const PlayerObject * player = PlayerCreatureController::getPlayerObject(playerCreature);
		if(player)
		{
			result = player->isQuestActive(questId);
		}
	}
	return result;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::clearCompletedQuest(JNIEnv * env, jobject self, jlong target, jint questId)
{
	if(questId >= 0)
	{
		CreatureObject * playerCreature = nullptr;
		if (JavaLibrary::getObject(target, playerCreature))
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(playerCreature);
			if(player)
			{
				player->clearCompletedQuest(questId);
			}
		}
	}
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::canTrade(JNIEnv *env, jobject self, jlong target)
{
	const ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	return static_cast<jboolean>(object->canTrade() ? JNI_TRUE : JNI_FALSE);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::isNoTradeShared(JNIEnv *env, jobject self, jlong target)
{
	const ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	return static_cast<jboolean>(object->markedNoTradeShared(true) ? JNI_TRUE : JNI_FALSE);
}

// ----------------------------------------------------------------------

/**
 * Returns the id of the last theater spawned for a player. Note that we do not
 * keep track of any theaters spawned except for the last one.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player to check
 *
 * @return the theater id, or nullptr on error
 */
jlong JNICALL ScriptMethodsObjectInfoNamespace::getLastSpawnedTheater(JNIEnv *env, jobject self, jlong player)
{
	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	const PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (playerObject == nullptr)
		return 0;

	return (playerObject->getTheater()).getValue();
}	// JavaLibrary::getLastSpawnedTheater

// ----------------------------------------------------------------------

jbyteArray JNICALL ScriptMethodsObjectInfoNamespace::getByteStreamFromAutoVariable(JNIEnv * env, jobject self, jlong target, jstring variableName)
{
	ServerObject * object = 0;
	if(JavaLibrary::getObject(target, object))
	{
		JavaStringParam localName(variableName);
		std::string nameString;
		JavaLibrary::convert(localName, nameString);
		Archive::ByteStream bs;
		object->getByteStreamFromAutoVariable(nameString, bs);
		LocalByteArrayRefPtr localInstance;
		std::vector<int8> source;

		size_t i;

		const unsigned char * const p = bs.getBuffer();
		if(p)
		{
			for(i = 0; i < bs.getSize(); ++i)
			{
				source.push_back(static_cast<int8>(*(p + i)));
			}
			if(ScriptConversion::convert(source, localInstance))
			{
				return localInstance->getReturnValue();
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setAutoVariableFromByteStream(JNIEnv * env, jobject self, jlong target, jstring variableName, jbyteArray data)
{
	std::vector<int8> cdata;
	if(ScriptConversion::convert(data, cdata))
	{
		Archive::ByteStream bs(reinterpret_cast<const unsigned char *>(&cdata[0]), cdata.size());
		ServerObject * object = 0;
		if(JavaLibrary::getObject(target, object))
		{
			JavaStringParam localName(variableName);

			std::string nameString;
			JavaLibrary::convert(localName, nameString);

			object->setAutoVariableFromByteStream(nameString, bs);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Sets a bio-link on an object. If the object is equippable, only the player that
 * the object is linked to will be able to equip it. If the object is used as a
 * component during crafting, the crafted object will also be linked to the player.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the item to be bio-linked
 * @param link			the id of the player to link the item to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::setBioLink(JNIEnv *env, jobject self, jlong target, jlong link)
{
	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	NetworkId bioLink(link);
	if (bioLink != NetworkId::cms_invalid)
		object->setBioLink(bioLink);
	else
		object->setPendingBioLink();
	return JNI_TRUE;
}	// JavaLibrary::setBioLink

// ----------------------------------------------------------------------

/**
 * Removes a bio-link from an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the item to remove the link from
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectInfoNamespace::clearBioLink(JNIEnv *env, jobject self, jlong target)
{
	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	object->clearBioLink();
	return JNI_TRUE;
}	// JavaLibrary::clearBioLink

// ----------------------------------------------------------------------

/**
 * Returns the bio-link attached to an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the item to get the link from
 *
 * @return the bio-link id, nullptr if the item isn't linked
 */
jlong JNICALL ScriptMethodsObjectInfoNamespace::getBioLink(JNIEnv *env, jobject self, jlong target)
{
	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	NetworkId link(object->getBioLink());
	if (link == NetworkId::cms_invalid)
		return 0;
	return link.getValue();
}	// JavaLibrary::getBioLink

//----------------------------------------------------------------------

float JNICALL ScriptMethodsObjectInfoNamespace::getObjectCollisionRadius(JNIEnv * env, jobject self, jlong jobject_obj)
{
	const Object * object = nullptr;
	if (!JavaLibrary::getObject(jobject_obj, object))
		return 0.0f;

	Sphere const & sphere = object->getCollisionSphereExtent_o();

	return sphere.getRadius();
}

// ----------------------------------------------------------------------

/**
 * Returns the static item name (if any) of the object
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		id of the object
 *
 * @return the name, or nullptr on error
 */
jstring JNICALL ScriptMethodsObjectInfoNamespace::getStaticItemName(JNIEnv * /*env*/, jobject /*self*/, jlong target)
{
	const ServerObject * o = nullptr;
	if (!JavaLibrary::getObject(target, o))
		return 0;

	const std::string & name = o->getStaticItemName();
	if (name.empty())
		return 0;
	return JavaString(name).getReturnValue();
}	

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getStaticItemVersion(JNIEnv *env, jobject /*self*/, jlong target)
{
	const ServerObject * o = nullptr;
	if (!JavaLibrary::getObject(target, o))
		return 0;

	return o->getStaticItemVersion();
}	

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setStaticItemName(JNIEnv * /*env*/, jobject /*self*/, jlong target, jstring staticItemName)
{
	NetworkId const networkId(target);
	ServerObject * const o = ServerObject::getServerObject(networkId);

	if (o == nullptr)
	{
		WARNING(true, ("ERROR: ScriptMethodsObjectInfo::setStaticItemName() Unable to resolve the object(%s).", networkId.getValueString().c_str()));
		return;
	}

	std::string nameString;
	JavaLibrary::convert(JavaStringParam(staticItemName), nameString);
	o->setStaticItemName(nameString);	
}	

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setStaticItemVersion(JNIEnv * /*env*/, jobject /*self*/, jlong target, jint staticItemVersion)
{
	NetworkId const networkId(target);
	ServerObject * const o = ServerObject::getServerObject(networkId);

	if (o == nullptr)
	{
		WARNING(true, ("ERROR: ScriptMethodsObjectInfo::setStaticItemVersion() Unable to resolve the object(%s).", networkId.getValueString().c_str()));
		return;
	}

	o->setStaticItemVersion(staticItemVersion);
}	

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectInfoNamespace::getConversionId(JNIEnv * /*env*/, jobject /*self*/, jlong target)
{
	const ServerObject * o = nullptr;
	if (!JavaLibrary::getObject(target, o))
		return 0;

	return o->getConversionId();
}	

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::setConversionId(JNIEnv * /*env*/, jobject /*self*/, jlong target, jint conversionId)
{
	NetworkId const networkId(target);
	ServerObject * const o = ServerObject::getServerObject(networkId);

	if (o == nullptr)
	{
		WARNING(true, ("ERROR: ScriptMethodsObjectInfo::setConversionId() Unable to resolve the object(%s).", networkId.getValueString().c_str()));
		return;
	}

	o->setConversionId(conversionId);
}	

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::openCustomizationWindow(JNIEnv *env, jobject self, jlong player, jlong object, jstring customVarName1, jint minVar1, jint maxVar1, jstring customVarName2, jint minVar2, jint maxVar2, jstring customVarName3, jint minVar3, jint maxVar3, jstring customVarName4, jint minVar4, jint maxVar4)
{
	CreatureObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return;
	std::string customVarName1String;
	JavaLibrary::convert(JavaStringParam(customVarName1), customVarName1String);
	std::string customVarName2String;
	JavaLibrary::convert(JavaStringParam(customVarName2), customVarName2String);
	std::string customVarName3String;
	JavaLibrary::convert(JavaStringParam(customVarName3), customVarName3String);
	std::string customVarName4String;
	JavaLibrary::convert(JavaStringParam(customVarName4), customVarName4String);

	std::string value;
	value.append(customVarName1String);
	char tmp[512];
	sprintf(tmp, " %d %d ", minVar1, maxVar1);
	value.append(tmp);
	if(!customVarName2String.empty())
	{
		value.append(" ");
		value.append(customVarName2String);
		char tmp[512];
		sprintf(tmp, " %d %d ", minVar2, maxVar2);
		value.append(tmp);
	}
	if(!customVarName3String.empty())
	{
		value.append(" ");
		value.append(customVarName3String);
		char tmp[512];
		sprintf(tmp, " %d %d ", minVar3, maxVar3);
		value.append(tmp);
	}
	if(!customVarName4String.empty())
	{
		value.append(" ");
		value.append(customVarName4String);
		char tmp[512];
		sprintf(tmp, " %d %d ", minVar4, maxVar4);
		value.append(tmp);
	}

	NetworkId const objectId(object);

	if (playerObject->isPlayerControlled() && playerObject->getController() != nullptr)
	{		
		playerObject->getController()->appendMessage(
			static_cast<int>(CM_openCustomizationWindow),
			0,
			new MessageQueueGenericValueType<std::pair<NetworkId, std::string> >(std::make_pair(objectId, value)),
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
	return;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::isPlayerBackpackHidden(JNIEnv *env, jobject self, jlong player)
{
	UNREF(self);

	const CreatureObject *playerObj = 0;
	if (JavaLibrary::getObject(player, playerObj) && playerObj->isPlayerControlled())
	{
		const PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(playerObj);
		if(playerObject)
		{
			if(playerObject->getShowBackpack())
				return JNI_FALSE;
			else
				return JNI_TRUE;
		}
	}

	return JNI_FALSE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::isPlayerHelmetHidden(JNIEnv *env, jobject self, jlong player)
{
	UNREF(self);

	const CreatureObject *playerObj = 0;
	if (JavaLibrary::getObject(player, playerObj) && playerObj->isPlayerControlled())
	{
		const PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(playerObj);
		if(playerObject)
		{
			if(playerObject->getShowHelmet())
				return JNI_FALSE;
			else
				return JNI_TRUE;
		}
	}

	return JNI_FALSE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsObjectInfoNamespace::getDefaultScaleFromSharedObjectTemplate(JNIEnv *env, jobject self, jstring sharedObjectTemplateName)
{
	UNREF(env);
	UNREF(self);

	//-- Retrieve the server object template name.
	JavaStringParam sharedObjectTemplateNameParam(sharedObjectTemplateName);
	std::string     sharedObjectTemplateNameNarrow;
	
	JavaLibrary::convert(sharedObjectTemplateNameParam, sharedObjectTemplateNameNarrow);

	//-- Open the shared object template.
	ObjectTemplate const *const baseSharedObjectTemplate = ObjectTemplateList::fetch(sharedObjectTemplateNameNarrow);
	if (!baseSharedObjectTemplate)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "getDefaultScaleFromSharedObjectTemplate(): failed to open shared object template [%s].", sharedObjectTemplateNameNarrow.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		LOG("ScriptMethodsObjectInfo", (buffer));
		JavaLibrary::throwInternalScriptError(buffer);

		return static_cast<jfloat>(0.0f);
	}

	//-- Convert to a shared object template.
	SharedObjectTemplate const *const sharedObjectTemplate = baseSharedObjectTemplate->asSharedObjectTemplate();
	if (!sharedObjectTemplate)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "getDefaultScaleFromSharedObjectTemplate(): object template [%s] is not derived from SharedObjectTemplate.", sharedObjectTemplateNameNarrow.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		LOG("ScriptMethodsObjectInfo", (buffer));
		JavaLibrary::throwInternalScriptError(buffer);

		baseSharedObjectTemplate->releaseReference();
		return static_cast<jfloat>(0.0f);
	}

	//-- Get scale.
	jfloat const returnScale = static_cast<jfloat>(sharedObjectTemplate->getScale());

	baseSharedObjectTemplate->releaseReference();

	return returnScale;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::setOverrideMapColor(JNIEnv * env, jobject self, jlong object, jint r, jint g, jint b)
{
	TangibleObject * tangible = nullptr;
	if (!JavaLibrary::getObject(object, tangible))
		return JNI_FALSE;

	if(!tangible->setOverrideMapColor(static_cast<uint8>(r), static_cast<uint8>(g), static_cast<uint8>(b)))
	{
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::clearOverrideMapColor(JNIEnv * env, jobject self, jlong object)
{
	TangibleObject * tangible = nullptr;
	if (!JavaLibrary::getObject(object, tangible))
		return JNI_FALSE;

	if(!tangible->clearOverrideMapColor())
	{
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsObjectInfoNamespace::getOverrideMapColor(JNIEnv * env, jobject self, jlong object)
{
	TangibleObject * tangible = nullptr;
	if (!JavaLibrary::getObject(object, tangible))
		return nullptr;

	uint8 r, g, b;
	if(!tangible->getOverrideMapColor(r,g,b))
	{
		return nullptr;
	}

	return createColor(r, g, b, 255)->getReturnValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::setForceShowHam(JNIEnv * env, jobject self, jlong object, jboolean show)
{
	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(object, creature))
		return JNI_FALSE;

	if(!creature->setForceShowHam(show))
	{
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

jboolean JNICALL ScriptMethodsObjectInfoNamespace::isContainedByPlayerAppearanceInventory(JNIEnv *env, jobject self, jlong player, jlong item)
{
	UNREF(self);
	ServerObject *itemObj = nullptr;
	CreatureObject *playerObj = nullptr;

	if(!JavaLibrary::getObject(item, itemObj))
	{
		WARNING(true, ("isContainedByPlayerAppearanceInventory: Failed to get item object"));
	}
	if(!JavaLibrary::getObject(player, playerObj))
	{
		WARNING(true, ("isContainedByPlayerAppearanceInventory: Failed to get player object"));
	}

	if(!itemObj || !playerObj || !playerObj->isPlayerControlled())
		return JNI_FALSE;
	
	Object const * containedBy = ContainerInterface::getContainedByObject(*itemObj);
	if(!containedBy)
		return JNI_FALSE;
	 
	ServerObject const * playerAppearInv = playerObj->getAppearanceInventory();
	
	if(playerAppearInv && playerAppearInv == containedBy)
		return JNI_TRUE;
	
	return JNI_FALSE;
}

jlongArray JNICALL ScriptMethodsObjectInfoNamespace::getAllItemsFromAppearanceInventory(JNIEnv * env, jobject self, jlong player)
{
	UNREF(self);

	const CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return 0;

	std::vector<NetworkId> objectIds;

	// go through the player's appearance inventory
	const ServerObject * appearanceInventoryObject = playerCreature->getAppearanceInventory();
	if (appearanceInventoryObject != nullptr)
	{
		const SlottedContainer * appearanceInvContainer =
			ContainerInterface::getSlottedContainer(*appearanceInventoryObject);
		if (appearanceInvContainer != nullptr)
		{
			getGoodItemsFromContainer(*appearanceInvContainer, objectIds);
		}
		else
		{
			DEBUG_WARNING(true, ("JavaLibrary::getAllItemsFromAppearanceInventory creature %s "
				"appearance inventory object %s has no slotted container",
				playerCreature->getNetworkId().getValueString().c_str(),
				appearanceInventoryObject->getNetworkId().getValueString().c_str()));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("JavaLibrary::getAllItemsFromAppearanceInventory creature %s has "
			"no appearance inventory object",
			playerCreature->getNetworkId().getValueString().c_str()));
	}

	if (!objectIds.empty())
	{
		LocalLongArrayRefPtr returnedIds;
		if (ScriptConversion::convert(objectIds, returnedIds))
			return returnedIds->getReturnValue();
	}

	return 0;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::isAPlayerAppearanceInventoryContainer(JNIEnv *env, jobject self, jlong container)
{
	UNREF(self);

	const ServerObject * containerObj = nullptr;
	if (!JavaLibrary::getObject(container, containerObj))
		return JNI_FALSE;

	const CreatureObject * creatureParent = dynamic_cast<const CreatureObject*>(ContainerInterface::getContainedByObject(*containerObj));
	if(creatureParent && creatureParent->isPlayerControlled() && creatureParent->getAppearanceInventory() == containerObj)
		return JNI_TRUE;

	return JNI_FALSE;
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsObjectInfoNamespace::getAllWornItems(JNIEnv *env, jobject self, jlong player, jboolean ignoreAppearanceItems)
{
	UNREF(self);

	const CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return 0;

	std::vector<NetworkId> objectIds;
	std::map<SlotId, const ServerObject *> wornObjects;
	
	if(!ignoreAppearanceItems && playerCreature->getAppearanceInventory() )
	{
		const SlottedContainer * appearanceInventory = ContainerInterface::getSlottedContainer(*playerCreature->getAppearanceInventory());

		if(!appearanceInventory)
		{
			DEBUG_WARNING(true, ("JavaLibrary: getAllWornItems - Tried to get all the worn items(INCLUDING appearance items) from player [%s], but this player has no appearance inventory container.",
				playerCreature->getNetworkId().getValueString().c_str()));
			
			return nullptr;
		}

		for (ContainerConstIterator i(appearanceInventory->begin()); i != appearanceInventory->end(); ++i)
		{
			const CachedNetworkId & itemId = *i;
			const ServerObject * item = safe_cast<const ServerObject *>(
				itemId.getObject());
			if (item != nullptr && item->asTangibleObject() != nullptr &&
				item->asTangibleObject()->isVisible())
			{

				const SlottedContainmentProperty * slottedContainment = ContainerInterface::getSlottedContainmentProperty(*item); // Get the slot property of our item.

				if(slottedContainment && slottedContainment->getCurrentArrangement() >= 0)
				{
					SlottedContainmentProperty::SlotArrangement const slots = slottedContainment->getSlotArrangement(slottedContainment->getCurrentArrangement()); // Get the old arrangement
					for(unsigned int j = 0; j < slots.size(); ++j)
						wornObjects.insert(std::make_pair(slots[j], item));
				}
			}
		}	
	}

	const SlottedContainer * inventory = ContainerInterface::getSlottedContainer(*playerCreature);

	if(!inventory)
	{
		DEBUG_WARNING(true, ("JavaLibrary: getAllWornItems - Tried to get all the worn items from player [%s], but this player has no creature slot container.",
			playerCreature->getNetworkId().getValueString().c_str()));

		return nullptr;
	}

	for (ContainerConstIterator i(inventory->begin()); i != inventory->end(); ++i)
	{
		const CachedNetworkId & itemId = *i;
		const ServerObject * item = safe_cast<const ServerObject *>(
			itemId.getObject());
		if (item != nullptr && item->asTangibleObject() != nullptr &&
			item->asTangibleObject()->isVisible())
		{
			const SlottedContainmentProperty * slottedContainment = ContainerInterface::getSlottedContainmentProperty(*item); // Get the slot property of our item.

			if(slottedContainment && slottedContainment->getCurrentArrangement() >= 0)
			{
				SlottedContainmentProperty::SlotArrangement const slots = slottedContainment->getSlotArrangement(slottedContainment->getCurrentArrangement()); // Get the old arrangement
				bool slotOccupied = false;
				for(unsigned int j = 0; j < slots.size(); ++j)
				{
					if(wornObjects.find(slots[j]) != wornObjects.end())
					{
						slotOccupied = true;
						break;
					}
						
				}

				if(!slotOccupied)
				{
					for(unsigned int j = 0; j < slots.size(); ++j)
						wornObjects.insert(std::make_pair(slots[j], item));
				}
			}
		}
	}	

	std::map<SlotId, const ServerObject *>::iterator mapIter = wornObjects.begin();

	for(; mapIter != wornObjects.end(); ++mapIter)
		objectIds.push_back((*mapIter).second->getNetworkId());
	
	if (!objectIds.empty())
	{
		LocalLongArrayRefPtr returnedIds;
		if (ScriptConversion::convert(objectIds, returnedIds))
			return returnedIds->getReturnValue();
	}

	return nullptr;

}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectInfoNamespace::getAppearanceInventory(JNIEnv *env, jobject self, jlong player)
{
	UNREF(env);
	UNREF(self);

	const CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return 0;

	NetworkId appearInv = NetworkId::cms_invalid;
	if(playerCreature->getAppearanceInventory())
		appearInv = playerCreature->getAppearanceInventory()->getNetworkId();

	return appearInv == NetworkId::cms_invalid ? 0 : appearInv.getValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::setDecoyOrigin(JNIEnv *env, jobject self, jlong creature, jlong origin)
{
	UNREF(env);
	UNREF(self);

	CreatureObject * decoyCreature = nullptr;
	if (!JavaLibrary::getObject(creature, decoyCreature))
		return JNI_FALSE;
	
	const CreatureObject * originCreature = nullptr;
	if (!JavaLibrary::getObject(origin, originCreature))
		return JNI_FALSE;

	if(decoyCreature && originCreature)
	{
		decoyCreature->setDecoyOrigin(originCreature->getNetworkId());
		return JNI_TRUE;
	}

	return JNI_FALSE;

}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectInfoNamespace::getDecoyOrigin(JNIEnv *env, jobject self, jlong creature)
{
	UNREF(env);
	UNREF(self);

	CreatureObject * decoyCreature = nullptr;
	if (!JavaLibrary::getObject(creature, decoyCreature))
		return JNI_FALSE;

	if(decoyCreature)
		return decoyCreature->getDecoyOrigin().getValue();

	return 0;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectInfoNamespace::openRatingWindow(JNIEnv * env, jobject self, jlong player, jstring title, jstring description)
{
	UNREF(env);
	UNREF(self);

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
	{
		DEBUG_WARNING(true, ("OpenRatingWindow: Failed to get valid creature object with OID %d", player));
		return JNI_FALSE;
	}

	JavaStringParam localTitle(title);
	std::string narrowTitle;
	
	if(!JavaLibrary::convert(localTitle, narrowTitle))
	{
		DEBUG_WARNING(true, ("OpenRatingWindow: Failed to convert title string."));
		return JNI_FALSE;
	}

	JavaStringParam localDesc(description);
	std::string narrowDesc;

	if(!JavaLibrary::convert(localDesc, narrowDesc))
	{
		DEBUG_WARNING(true, ("OpenRatingWindow: Failed to convert description string."));
		return JNI_FALSE;
	}

	if (playerCreature->isPlayerControlled() && playerCreature->getController() != nullptr)
	{		
		playerCreature->getController()->appendMessage(
			static_cast<int>(CM_openRatingWindow),
			0,
			new MessageQueueGenericValueType<std::pair<std::string, std::string> >(std::make_pair(narrowTitle, narrowDesc)),
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);

		return JNI_TRUE;
	}

	return JNI_FALSE;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectInfoNamespace::openExamineWindow(JNIEnv * env, jobject self, jlong player, jlong item)
{
	UNREF(env);
	UNREF(self);

	CreatureObject const * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature) || !playerCreature || !playerCreature->getClient())
	{
		return;
	}

	ServerObject const * itemObject = nullptr;
	if (!JavaLibrary::getObject(item, itemObject) || !itemObject)
	{
		return;
	}

	if (!ObserveTracker::isObserving(*(playerCreature->getClient()), *itemObject))
	{
		return;
	}

	GenericValueTypeMessage<NetworkId> const openExamineWindowMsg("OpenExamineWindow", itemObject->getNetworkId());
	playerCreature->getClient()->send(openExamineWindowMsg, true);
}

// ======================================================================
