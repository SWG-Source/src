// ======================================================================
//
// ScriptMethodsPvp.cpp - implements script methods dealing with Pvp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/BattlefieldMarkerObject.h"
#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/Pvp.h"
#include "serverGame/PvpFactions.h"
#include "serverGame/RegionPvp.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TangibleObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/FormattedString.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsPvpNamespace
// ======================================================================

namespace ScriptMethodsPvpNamespace
{
	bool install();

	// ----------------------------------------------------------------------

	const char * makeCopyOfString(const char * rhs)
	{
		char * lhs = nullptr;
		if (rhs)
		{
			lhs = new char[strlen(rhs) + 1];
			strcpy(lhs, rhs);
		}
		else
		{
			lhs = new char[1];
			lhs[0] = '\0';
		}

		return lhs;
	}

	// ----------------------------------------------------------------------

	void         JNICALL pvpSetAttackableOverride(JNIEnv *env, jobject self, jlong dest, jboolean value);
	jboolean     JNICALL pvpCanAttack(JNIEnv *env, jobject self, jlong actor, jlong target);
	jboolean     JNICALL pvpCanHelp(JNIEnv *env, jobject self, jlong actor, jlong target);
	void         JNICALL pvpAttackPerformed(JNIEnv *env, jobject self, jlong actor, jlong target);
	void         JNICALL pvpHelpPerformed(JNIEnv *env, jobject self, jlong actor, jlong target);
	void         JNICALL pvpNeutralSetMercenaryFaction(JNIEnv *env, jobject self, jlong dest, jint factionId, jboolean declared);
	jint         JNICALL pvpNeutralGetMercenaryFaction(JNIEnv *env, jobject self, jlong who);
	jboolean     JNICALL pvpNeutralIsMercenaryDeclared(JNIEnv *env, jobject self, jlong who);
	void         JNICALL pvpSetAlignedFaction(JNIEnv *env, jobject self, jlong dest, jint factionId);
	void         JNICALL pvpMakeOnLeave(JNIEnv *env, jobject self, jlong dest);
	void         JNICALL pvpMakeCovert(JNIEnv *env, jobject self, jlong dest);
	void         JNICALL pvpMakeDeclared(JNIEnv *env, jobject self, jlong dest);
	void         JNICALL pvpMakeNeutral(JNIEnv *env, jobject self, jlong dest);
	void         JNICALL pvpPrepareToBeCovert(JNIEnv *env, jobject self, jlong dest);
	void         JNICALL pvpPrepareToBeDeclared(JNIEnv *env, jobject self, jlong dest);
	void         JNICALL pvpPrepareToBeNeutral(JNIEnv *env, jobject self, jlong dest);
	void         JNICALL pvpSetPersonalEnemyFlag(JNIEnv *env, jobject self, jlong dest, jlong enemy);
	void         JNICALL pvpSetPermanentPersonalEnemyFlag(JNIEnv *env, jobject self, jlong dest, jlong enemy);
	void         JNICALL pvpSetFactionEnemyFlag(JNIEnv *env, jobject self, jlong dest, jint factionId);
	void         JNICALL pvpSetGuildWarCoolDownPeriodEnemyFlag(JNIEnv *env, jobject self, jlong dest);
	jint         JNICALL pvpGetType(JNIEnv *env, jobject self, jlong who);
	jint         JNICALL pvpGetAlignedFaction(JNIEnv *env, jobject self, jlong who);
	jobjectArray JNICALL pvpGetEnemyFlags(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL pvpIsEnemy(JNIEnv *env, jobject self, jlong actor, jlong target);
	jboolean     JNICALL pvpIsDueling(JNIEnv *env, jobject self, jlong actor, jlong target);
	jboolean     JNICALL pvpHasBattlefieldEnemyFlag(JNIEnv *env, jobject self, jlong target);
	jlongArray   JNICALL pvpGetEnemiesInRange(JNIEnv *env, jobject self, jlong actor, jlong location, jfloat range);
	jlongArray   JNICALL pvpGetEnemiesInCone(JNIEnv *env, jobject self, jlong actor, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle);
	jlongArray   JNICALL pvpGetEnemiesInConeLocation(JNIEnv *env, jobject self, jlong actor, jlong coneCenterObjectId, jobject coneDirectionLocation, jfloat range, jfloat angle);
	jlongArray   JNICALL pvpGetTargetsInRange(JNIEnv *env, jobject self, jlong actor, jlong location, jfloat range);
	jlongArray   JNICALL pvpGetTargetsInCone(JNIEnv *env, jobject self, jlong actor, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle);
	jlongArray   JNICALL pvpGetTargetsInConeLocation(JNIEnv *env, jobject self, jlong actor, jlong coneCenterObjectId, jobject coneDirectionLocation, jfloat range, jfloat angle);
	jboolean     JNICALL pvpAreFactionsOpposed(JNIEnv *env, jobject self, jint faction1, jint faction2);
	jint         JNICALL pvpBattlefieldGetFaction(JNIEnv *env, jobject self, jlong target, jobject region);
	jboolean     JNICALL pvpBattlefieldIsParticipant(JNIEnv *env, jobject self, jlong target, jobject region);
	void         JNICALL pvpBattlefieldSetParticipant(JNIEnv *env, jobject self, jlong target, jobject region, jint faction);
	jlongArray   JNICALL pvpBattlefieldGetParticipantsForFaction(JNIEnv *env, jobject self, jobject region, jint faction);
	void         JNICALL pvpBattlefieldClearParticipants(JNIEnv *env, jobject self, jobject region);
	void         JNICALL pvpRemoveAllTempEnemyFlags(JNIEnv *env, jobject self, jlong target);
	jlong        JNICALL getBattlefieldRegionMasterObject(JNIEnv *env, jobject self, jobject battlefieldRegion);
	void         JNICALL setBattlefieldMarkerRegionName(JNIEnv *env, jobject self, jlong marker, jstring regionName);
	jboolean     JNICALL pvpHasAnyTempEnemyFlags(JNIEnv *env, jobject self, jlong target);
	jlongArray   JNICALL pvpGetPersonalEnemyIds(JNIEnv *env, jobject self, jlong target);
	void         JNICALL pvpRemoveTempEnemyFlags(JNIEnv *env, jobject self, jlong target, jlong enemyId);
	jboolean     JNICALL pvpWouldAttackCauseAlignedEnemyFlag(JNIEnv *env, jobject self, jlong actor, jlong target);
	jboolean     JNICALL pvpWouldHelpCauseAlignedEnemyFlag(JNIEnv *env, jobject self, jlong actor, jlong target);
	jboolean     JNICALL pvpHasPersonalEnemyFlag(JNIEnv *env, jobject self, jlong actor, jlong target);
	void         JNICALL pvpRemovePersonalEnemyFlags(JNIEnv *env, jobject self, jlong target, jlong enemyId);
	void         JNICALL pvpModifyCurrentGcwPoints(JNIEnv *env, jobject self, jlong target, jint adjustment);
	void         JNICALL pvpModifyCurrentPvpKills(JNIEnv *env, jobject self, jlong target, jint adjustment);
	jint         JNICALL pvpGetCurrentGcwRank(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL pvpGetMaxGcwImperialRank(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL pvpGetMaxGcwRebelRank(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL pvpGetCurrentGcwPoints(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL pvpGetCurrentGcwRating(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL pvpGetCurrentPvpKills(JNIEnv *env, jobject self, jlong target);
	jlong        JNICALL pvpGetLifetimeGcwPoints(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL pvpGetMaxGcwImperialRating(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL pvpGetMaxGcwRebelRating(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL pvpGetLifetimePvpKills(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL pvpGetNextGcwRatingCalcTime(JNIEnv *env, jobject self, jlong target);
	void         JNICALL ctsUseOnlySetGcwInfo(JNIEnv *env, jobject self, jlong target, jint currentGcwPoints, jint currentGcwRating, jint currentPvpKills, jlong lifetimeGcwPoints, jint maxGcwImperialRating, jint maxGcwRebelRating, jint lifetimePvpKills, jint nextGcwRatingCalcTime);
	void         JNICALL adjustGcwImperialScore(JNIEnv *env, jobject self, jstring source, jlong sourceOid, jstring gcwCategory, jint adjustment);
	void         JNICALL adjustGcwRebelScore(JNIEnv *env, jobject self, jstring source, jlong sourceOid, jstring gcwCategory, jint adjustment);
	jint         JNICALL getGcwImperialScorePercentile(JNIEnv *env, jobject self, jstring gcwCategory);
	jint         JNICALL getGcwGroupImperialScorePercentile(JNIEnv *env, jobject self, jstring gcwGroup);
	jobject      JNICALL getGcwFactionalPresenceTableDictionary(JNIEnv * env, jobject self);
	jobject      JNICALL getGcwContributionTrackingTableDictionary(JNIEnv * env, jobject self, jlong player);
	jobjectArray JNICALL getGcwDefenderRegions(JNIEnv *env, jobject self);
	jobjectArray JNICALL getGcwDefenderRegionsCitiesImperial(JNIEnv *env, jobject self);
	jobjectArray JNICALL getGcwDefenderRegionsCitiesRebel(JNIEnv *env, jobject self);
	jint         JNICALL getGcwDefenderRegionsCitiesVersion(JNIEnv *env, jobject self);
	jobjectArray JNICALL getGcwDefenderRegionsGuildsImperial(JNIEnv *env, jobject self);
	jobjectArray JNICALL getGcwDefenderRegionsGuildsRebel(JNIEnv *env, jobject self);
	jint         JNICALL getGcwDefenderRegionsGuildsVersion(JNIEnv *env, jobject self);
	jintArray    JNICALL getGcwDefenderRegionCitiesImperial(JNIEnv *env, jobject self, jstring gcwCategory);
	jintArray    JNICALL getGcwDefenderRegionCitiesRebel(JNIEnv *env, jobject self, jstring gcwCategory);
	jintArray    JNICALL getGcwDefenderRegionGuildsImperial(JNIEnv *env, jobject self, jstring gcwCategory);
	jintArray    JNICALL getGcwDefenderRegionGuildsRebel(JNIEnv *env, jobject self, jstring gcwCategory);
	jfloat       JNICALL getGcwDefenderRegionImperialBonus(JNIEnv *env, jobject self, jstring gcwCategory);
	jfloat       JNICALL getGcwDefenderRegionRebelBonus(JNIEnv *env, jobject self, jstring gcwCategory);
}

//========================================================================
// install
//========================================================================

bool ScriptMethodsPvpNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsPvpNamespace::c)}
	JF("_pvpSetAttackableOverride", "(JZ)V", pvpSetAttackableOverride),
	JF("_pvpCanAttack", "(JJ)Z", pvpCanAttack),
	JF("_pvpCanHelp", "(JJ)Z", pvpCanHelp),
	JF("_pvpAttackPerformed", "(JJ)V", pvpAttackPerformed),
	JF("_pvpHelpPerformed", "(JJ)V", pvpHelpPerformed),
	JF("_pvpNeutralSetMercenaryFaction", "(JIZ)V", pvpNeutralSetMercenaryFaction),
	JF("_pvpNeutralGetMercenaryFaction", "(J)I", pvpNeutralGetMercenaryFaction),
	JF("_pvpNeutralIsMercenaryDeclared", "(J)Z", pvpNeutralIsMercenaryDeclared),
	JF("_pvpSetAlignedFaction", "(JI)V", pvpSetAlignedFaction),
	JF("_pvpMakeOnLeave", "(J)V", pvpMakeOnLeave),
	JF("_pvpMakeCovert", "(J)V", pvpMakeCovert),
	JF("_pvpMakeDeclared", "(J)V", pvpMakeDeclared),
	JF("_pvpMakeNeutral", "(J)V", pvpMakeNeutral),
	JF("_pvpPrepareToBeCovert", "(J)V", pvpPrepareToBeCovert),
	JF("_pvpPrepareToBeDeclared", "(J)V", pvpPrepareToBeDeclared),
	JF("_pvpPrepareToBeNeutral", "(J)V", pvpPrepareToBeNeutral),
	JF("_pvpSetPersonalEnemyFlag", "(JJ)V", pvpSetPersonalEnemyFlag),
	JF("_pvpSetPermanentPersonalEnemyFlag", "(JJ)V", pvpSetPermanentPersonalEnemyFlag),
	JF("_pvpSetFactionEnemyFlag", "(JI)V", pvpSetFactionEnemyFlag),
	JF("_pvpSetGuildWarCoolDownPeriodEnemyFlag", "(J)V", pvpSetGuildWarCoolDownPeriodEnemyFlag),
	JF("_pvpGetType", "(J)I", pvpGetType),
	JF("_pvpGetAlignedFaction", "(J)I", pvpGetAlignedFaction),
	JF("_pvpGetEnemyFlags", "(J)[Ljava/lang/String;", pvpGetEnemyFlags),
	JF("_pvpIsEnemy", "(JJ)Z", pvpIsEnemy),
	JF("_pvpIsDueling", "(JJ)Z", pvpIsDueling),
	JF("_pvpHasBattlefieldEnemyFlag", "(J)Z", pvpHasBattlefieldEnemyFlag),
	JF("_pvpGetEnemiesInRange", "(JJF)[J", pvpGetEnemiesInRange),
	JF("_pvpGetEnemiesInCone", "(JJJFF)[J", pvpGetEnemiesInCone),
	JF("_pvpGetEnemiesInCone", "(JJLscript/location;FF)[J", pvpGetEnemiesInConeLocation),
	JF("_pvpGetTargetsInRange", "(JJF)[J", pvpGetTargetsInRange),
	JF("_pvpGetTargetsInCone", "(JJJFF)[J", pvpGetTargetsInCone),
	JF("_pvpGetTargetsInCone", "(JJLscript/location;FF)[J", pvpGetTargetsInConeLocation),
	JF("pvpAreFactionsOpposed", "(II)Z", pvpAreFactionsOpposed),
	JF("_pvpBattlefieldGetFaction", "(JLscript/region;)I", pvpBattlefieldGetFaction),
	JF("_pvpBattlefieldIsParticipant", "(JLscript/region;)Z", pvpBattlefieldIsParticipant),
	JF("_pvpBattlefieldSetParticipant", "(JLscript/region;I)V", pvpBattlefieldSetParticipant),
	JF("_pvpBattlefieldGetParticipantsForFaction", "(Lscript/region;I)[J", pvpBattlefieldGetParticipantsForFaction),
	JF("pvpBattlefieldClearParticipants", "(Lscript/region;)V", pvpBattlefieldClearParticipants),
	JF("_pvpRemoveAllTempEnemyFlags", "(J)V", pvpRemoveAllTempEnemyFlags),
	JF("_getBattlefieldRegionMasterObject", "(Lscript/region;)J", getBattlefieldRegionMasterObject),
	JF("_setBattlefieldMarkerRegionName", "(JLjava/lang/String;)V", setBattlefieldMarkerRegionName),
	JF("_pvpHasAnyTempEnemyFlags", "(J)Z", pvpHasAnyTempEnemyFlags),
	JF("_pvpGetPersonalEnemyIds", "(J)[J", pvpGetPersonalEnemyIds),
	JF("_pvpRemoveTempEnemyFlags", "(JJ)V", pvpRemoveTempEnemyFlags),
	JF("_pvpWouldAttackCauseAlignedEnemyFlag", "(JJ)Z", pvpWouldAttackCauseAlignedEnemyFlag),
	JF("_pvpWouldHelpCauseAlignedEnemyFlag", "(JJ)Z", pvpWouldHelpCauseAlignedEnemyFlag),
	JF("_pvpHasPersonalEnemyFlag", "(JJ)Z", pvpHasPersonalEnemyFlag),
	JF("_pvpRemovePersonalEnemyFlags", "(JJ)V", pvpRemovePersonalEnemyFlags),
	JF("_pvpModifyCurrentGcwPoints", "(JI)V", pvpModifyCurrentGcwPoints),
	JF("_pvpModifyCurrentPvpKills", "(JI)V", pvpModifyCurrentPvpKills),
	JF("_pvpGetCurrentGcwRank", "(J)I", pvpGetCurrentGcwRank),
	JF("_pvpGetMaxGcwImperialRank", "(J)I", pvpGetMaxGcwImperialRank),
	JF("_pvpGetMaxGcwRebelRank", "(J)I", pvpGetMaxGcwRebelRank),
	JF("_pvpGetCurrentGcwPoints", "(J)I", pvpGetCurrentGcwPoints),
	JF("_pvpGetCurrentGcwRating", "(J)I", pvpGetCurrentGcwRating),
	JF("_pvpGetCurrentPvpKills", "(J)I", pvpGetCurrentPvpKills),
	JF("_pvpGetLifetimeGcwPoints", "(J)J", pvpGetLifetimeGcwPoints),
	JF("_pvpGetMaxGcwImperialRating", "(J)I", pvpGetMaxGcwImperialRating),
	JF("_pvpGetMaxGcwRebelRating", "(J)I", pvpGetMaxGcwRebelRating),
	JF("_pvpGetLifetimePvpKills", "(J)I", pvpGetLifetimePvpKills),
	JF("_pvpGetNextGcwRatingCalcTime", "(J)I", pvpGetNextGcwRatingCalcTime),
	JF("_ctsUseOnlySetGcwInfo", "(JIIIJIIII)V", ctsUseOnlySetGcwInfo),
	JF("_adjustGcwImperialScore", "(Ljava/lang/String;JLjava/lang/String;I)V", adjustGcwImperialScore),
	JF("_adjustGcwRebelScore", "(Ljava/lang/String;JLjava/lang/String;I)V", adjustGcwRebelScore),
	JF("getGcwImperialScorePercentile", "(Ljava/lang/String;)I", getGcwImperialScorePercentile),
	JF("getGcwGroupImperialScorePercentile", "(Ljava/lang/String;)I", getGcwGroupImperialScorePercentile),
	JF("getGcwFactionalPresenceTableDictionary", "()Lscript/dictionary;", getGcwFactionalPresenceTableDictionary),
	JF("_getGcwContributionTrackingTableDictionary", "(J)Lscript/dictionary;", getGcwContributionTrackingTableDictionary),
	JF("_getGcwDefenderRegions", "()[Ljava/lang/String;", getGcwDefenderRegions),
	JF("_getGcwDefenderRegionsCitiesImperial", "()[Ljava/lang/String;", getGcwDefenderRegionsCitiesImperial),
	JF("_getGcwDefenderRegionsCitiesRebel", "()[Ljava/lang/String;", getGcwDefenderRegionsCitiesRebel),
	JF("getGcwDefenderRegionsCitiesVersion", "()I", getGcwDefenderRegionsCitiesVersion),
	JF("_getGcwDefenderRegionsGuildsImperial", "()[Ljava/lang/String;", getGcwDefenderRegionsGuildsImperial),
	JF("_getGcwDefenderRegionsGuildsRebel", "()[Ljava/lang/String;", getGcwDefenderRegionsGuildsRebel),
	JF("getGcwDefenderRegionsGuildsVersion", "()I", getGcwDefenderRegionsGuildsVersion),
	JF("getGcwDefenderRegionCitiesImperial", "(Ljava/lang/String;)[I", getGcwDefenderRegionCitiesImperial),
	JF("getGcwDefenderRegionCitiesRebel", "(Ljava/lang/String;)[I", getGcwDefenderRegionCitiesRebel),
	JF("getGcwDefenderRegionGuildsImperial", "(Ljava/lang/String;)[I", getGcwDefenderRegionGuildsImperial),
	JF("getGcwDefenderRegionGuildsRebel", "(Ljava/lang/String;)[I", getGcwDefenderRegionGuildsRebel),
	JF("getGcwDefenderRegionImperialBonus", "(Ljava/lang/String;)F", getGcwDefenderRegionImperialBonus),
	JF("getGcwDefenderRegionRebelBonus", "(Ljava/lang/String;)F", getGcwDefenderRegionRebelBonus),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}

// ======================================================================
// class JavaLibrary JNI command table methods
// ======================================================================

//------------------------------------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::pvpSetAttackableOverride(JNIEnv *env, jobject self, jlong dest, jboolean value)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
		return;
	if (destObj)
		destObj->setAttackableOverride(value);
}

/**
 * Check whether an object is allowed to attack another.
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param actor    actor for the aggressive action
 * @param target   target for the aggressive action
 *
 * @return whether allowed
 */
jboolean JNICALL ScriptMethodsPvpNamespace::pvpCanAttack(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	TangibleObject *actorObj = 0;
	if (!JavaLibrary::getObject(actor, actorObj))
		return JNI_FALSE;
	TangibleObject *targetObj = 0;
	if (!JavaLibrary::getObject(target, targetObj))
		return JNI_FALSE;

	if (!actorObj || !targetObj)
		return JNI_FALSE;

	if (Pvp::canAttack(*actorObj, *targetObj))
		return JNI_TRUE;
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

/**
 * Check whether an object is allowed to help another.
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param actor    actor for the helping action
 * @param target   target for the helping action
 *
 * @return whether allowed
 */
jboolean JNICALL ScriptMethodsPvpNamespace::pvpCanHelp(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	TangibleObject *actorObj = 0;
	if (!JavaLibrary::getObject(actor, actorObj))
		return JNI_FALSE;
	TangibleObject *targetObj = 0;
	if (!JavaLibrary::getObject(target, targetObj))
		return JNI_FALSE;

	if (!actorObj || !targetObj)
		return JNI_FALSE;

	if (Pvp::canHelp(*actorObj, *targetObj))
		return JNI_TRUE;
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

/**
 * Handle an aggressive action having been done.
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param actor    actor for the aggressive action
 * @param target   target for the aggressive action
 */
void JNICALL ScriptMethodsPvpNamespace::pvpAttackPerformed(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	TangibleObject *actorObj = 0, *targetObj = 0;
	if (!JavaLibrary::getObject(actor, actorObj))
		return;
	if (!JavaLibrary::getObject(target, targetObj))
		return;

	if (!actorObj || !targetObj)
		return;

	if (ConfigServerGame::getDebugPvp() && !Pvp::canAttack(*actorObj, *targetObj))
		JavaLibrary::throwInternalScriptError("Tried to call pvpAttackPerformed when pvpCanAttack is false");
	else
		Pvp::attackPerformed(*actorObj, *targetObj);
}

// ----------------------------------------------------------------------

/**
 * Handle a helping action having been done.
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param actor    actor for the helping action
 * @param target   target for the helping action
 */
void JNICALL ScriptMethodsPvpNamespace::pvpHelpPerformed(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	TangibleObject *actorObj = 0, *targetObj = 0;
	if (!JavaLibrary::getObject(actor, actorObj))
		return;
	if (!JavaLibrary::getObject(target, targetObj))
		return;

	if (!actorObj || !targetObj)
		return;

	if (ConfigServerGame::getDebugPvp() && !Pvp::canHelp(*actorObj, *targetObj))
		JavaLibrary::throwInternalScriptError("Tried to call pvpHelpPerformed when pvpCanHelp is false");
	else
		Pvp::helpPerformed(*actorObj, *targetObj);
}

// ----------------------------------------------------------------------

/**
* Set mercenary faction (Imperial or Rebel or 0) for a neutral player character
*
* @param env       Java Environment
* @param self      class calling this function
* @param dest      object to set on
* @param factionId id for the faction
* @param declared  declared (or covert)
*/
void JNICALL ScriptMethodsPvpNamespace::pvpNeutralSetMercenaryFaction(JNIEnv *env, jobject self, jlong dest, jint factionId, jboolean declared)
{
	CreatureObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
		return;

	// only applies to player character
	if (!PlayerCreatureController::getPlayerObject(destObj))
		return;

	if (PvpData::isNeutralFactionId(static_cast<Pvp::FactionId>(factionId)))
		Pvp::setNeutralMercenaryFaction(*destObj, static_cast<Pvp::FactionId>(factionId), PvpType_Neutral);
	else if (PvpData::isImperialFactionId(static_cast<Pvp::FactionId>(factionId)) || PvpData::isRebelFactionId(static_cast<Pvp::FactionId>(factionId)))
		Pvp::setNeutralMercenaryFaction(*destObj, static_cast<Pvp::FactionId>(factionId), ((declared == JNI_FALSE) ? PvpType_Covert : PvpType_Declared));
}

// ----------------------------------------------------------------------

/**
* Get mercenary faction (Imperial or Rebel or 0) for a neutral player character
*
* @param env       Java Environment
* @param self      class calling this function
* @param who       object to get mercenary faction from
* @return  hash value for the faction
*/
jint JNICALL ScriptMethodsPvpNamespace::pvpNeutralGetMercenaryFaction(JNIEnv *env, jobject self, jlong who)
{
	CreatureObject *obj = 0;
	if (!JavaLibrary::getObject(who, obj))
		return 0;

	return static_cast<jint>(obj->getPvpMercenaryFaction());
}

// ----------------------------------------------------------------------

/**
* Get mercenary declared (vs covert) state for a neutral player character
*
* @param env       Java Environment
* @param self      class calling this function
* @param who       object to get mercenary faction from
* @return boolean
*/
jboolean JNICALL ScriptMethodsPvpNamespace::pvpNeutralIsMercenaryDeclared(JNIEnv *env, jobject self, jlong who)
{
	CreatureObject *obj = 0;
	if (!JavaLibrary::getObject(who, obj))
		return JNI_FALSE;

	return ((obj->getPvpMercenaryType() == PvpType_Declared) ? JNI_TRUE : JNI_FALSE);
}

// ----------------------------------------------------------------------

/**
 * Set aligned faction for an object
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param dest      object to set on
 * @param factionId id for the faction
 */
void JNICALL ScriptMethodsPvpNamespace::pvpSetAlignedFaction(JNIEnv *env, jobject self, jlong dest, jint factionId)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
		return;
	if (destObj)
		Pvp::setAlignedFaction(*destObj, static_cast<Pvp::FactionId>(factionId));
}

// ----------------------------------------------------------------------

/**
* Set an object to on leave
*
* @param env       Java Environment
* @param self      class calling this function
* @param dest      object to set on
*/
void JNICALL ScriptMethodsPvpNamespace::pvpMakeOnLeave(JNIEnv *env, jobject self, jlong dest)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
		return;
	if (destObj)
		Pvp::makeOnLeave(*destObj);
}

// ----------------------------------------------------------------------

/**
 * Set an object to covert
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param dest      object to set on
 */
void JNICALL ScriptMethodsPvpNamespace::pvpMakeCovert(JNIEnv *env, jobject self, jlong dest)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
		return;
	if (destObj)
		Pvp::makeCovert(*destObj);
}

// ----------------------------------------------------------------------

/**
 * Set an object to declared
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param dest      object to set on
 */
void JNICALL ScriptMethodsPvpNamespace::pvpMakeDeclared(JNIEnv *env, jobject self, jlong dest)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
		return;
	if (destObj)
		Pvp::makeDeclared(*destObj);
}

// ----------------------------------------------------------------------

/**
 * Set an object to neutral
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param dest      object to set on
 */
void JNICALL ScriptMethodsPvpNamespace::pvpMakeNeutral(JNIEnv *env, jobject self, jlong dest)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
		return;
	if (destObj)
		Pvp::makeNeutral(*destObj);
}

// ----------------------------------------------------------------------

/**
 * Set a personal enemy flag on an object
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param dest      object to set on
 * @param enemy     enemy object
 */
void JNICALL ScriptMethodsPvpNamespace::pvpSetPersonalEnemyFlag(JNIEnv *env, jobject self, jlong dest, jlong enemy)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
		return;
	TangibleObject *enemyObj = 0;
	if (!JavaLibrary::getObject(enemy, enemyObj))
		return;
	if (destObj && enemyObj)
		Pvp::setPersonalEnemyFlag(*destObj, *enemyObj);
}

// ----------------------------------------------------------------------

/**
 * Set a permanent personal enemy flag on an object
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param dest      object to set on
 * @param enemy     enemy object
 */
void JNICALL ScriptMethodsPvpNamespace::pvpSetPermanentPersonalEnemyFlag(JNIEnv *env, jobject self, jlong dest, jlong enemy)
{
	TangibleObject *destObj = 0;
	if (JavaLibrary::getObject(dest, destObj))
	{
		NetworkId enemyId(enemy);
		if (enemyId != NetworkId::cms_invalid)
			Pvp::setPermanentPersonalEnemyFlag(*destObj, enemyId);
	}
}

// ----------------------------------------------------------------------

/**
 * Set a faction enemy flag on an object
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param dest      object to set on
 * @param factionId enemy faction
 */
void JNICALL ScriptMethodsPvpNamespace::pvpSetFactionEnemyFlag(JNIEnv *env, jobject self, jlong dest, jint factionId)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
		return;
	if (destObj)
		Pvp::setFactionEnemyFlag(*destObj, static_cast<Pvp::FactionId>(factionId));
}

/**
 * Set the "guild war cool down period" enemy flag on an object
 * to prevent it from participating in any guild war related
 * pvp for a period of time
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param dest      object to set on
 */
void JNICALL ScriptMethodsPvpNamespace::pvpSetGuildWarCoolDownPeriodEnemyFlag(JNIEnv *env, jobject self, jlong dest)
{
	CreatureObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
		return;
	if (destObj && destObj->getGuildWarEnabled() && (ConfigServerGame::getPvpGuildWarCoolDownPeriodTimeMs() > 0))
		Pvp::setFactionEnemyFlag(*destObj, PvpFactions::getGuildWarCoolDownPeriodFactionId(), ConfigServerGame::getPvpGuildWarCoolDownPeriodTimeMs());
}

// ----------------------------------------------------------------------

/**
 * Get the pvp type for an object (0=neutral, 1=covert, 2=declared)
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param who       object to get pvp type from
 * @return  pvp type for the object
 */
jint JNICALL ScriptMethodsPvpNamespace::pvpGetType(JNIEnv *env, jobject self, jlong who)
{
	TangibleObject *obj = 0;
	if (!JavaLibrary::getObject(who, obj))
		return 0;
	return obj->getPvpType();
}

// ----------------------------------------------------------------------

/**
 * Get the aligned faction for an object (hash string of the faction name)
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param who       object to get aligned faction from
 * @return  hash value for the faction
 */
jint JNICALL ScriptMethodsPvpNamespace::pvpGetAlignedFaction(JNIEnv *env, jobject self, jlong who)
{
	TangibleObject *obj = 0;
	if (!JavaLibrary::getObject(who, obj))
		return 0;
	return obj->getPvpFaction();
}

// ----------------------------------------------------------------------

/**
 * Get the enemy flags on an object (string array, "enemyId enemyAlign expireTime")
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param who       object to get enemy flags from
 * @return  array of enemy flag specifiers
 */
jobjectArray JNICALL ScriptMethodsPvpNamespace::pvpGetEnemyFlags(JNIEnv *env, jobject self, jlong who)
{
	LocalObjectArrayRefPtr strArray;
	TangibleObject *obj = 0;
	if (JavaLibrary::getObject(who, obj))
	{
		std::vector<PvpEnemy> const &enemies = obj->getPvpEnemies().get();
		std::vector<std::string> enemyStrings;
		for (std::vector<PvpEnemy>::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		{
			// faction id is stored in C++ as an unsigned crc, but in script/Java,
			// there is no unsigned, so we must return the faction id as a signed
			// crc, so script/Java can parse the string to arrive at the corresponding
			// signed crc
			char buffer[64];
			sprintf(buffer, "%s %d %d",
				(*i).enemyId.getValueString().c_str(),
				static_cast<int>((*i).enemyFaction),
				(*i).expireTime);
			enemyStrings.push_back(std::string(buffer));
		}
		if (ScriptConversion::convert(enemyStrings, strArray))
			return strArray->getReturnValue();
	}
	return nullptr;
}

// ----------------------------------------------------------------------

/**
 * Check whether the target is either an explicit or faction enemy of the actor
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param actor     person checking enemy status
 * @param target    target to check enemy status on
 * @return  true if enemy, false if not or on error
 */
jboolean JNICALL ScriptMethodsPvpNamespace::pvpIsEnemy(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	TangibleObject *actorObj = 0;
	TangibleObject *targetObj = 0;
	if (JavaLibrary::getObject(actor, actorObj) &&
		JavaLibrary::getObject(target, targetObj) &&
		Pvp::isEnemy(*actorObj, *targetObj))
	{
		return JNI_TRUE;
	}
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

/**
 * Check whether the actor and target are currently dueling
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param actor     actor
 * @param target    target
 * @return true if dueling, false if not or on error
 */
jboolean JNICALL ScriptMethodsPvpNamespace::pvpIsDueling(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	TangibleObject *actorObj = 0;
	TangibleObject *targetObj = 0;
	if (JavaLibrary::getObject(actor, actorObj) &&
		JavaLibrary::getObject(target, targetObj) &&
		Pvp::hasDuelEnemyFlag(*actorObj, targetObj->getNetworkId()) &&
		Pvp::hasDuelEnemyFlag(*targetObj, actorObj->getNetworkId()))
	{
		return JNI_TRUE;
	}
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

/**
 * Check whether the target has a battlefield enemy flag
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param target    target to look for a battlefield enemy flag on
 * @return  true if enemy, false if not or on error
 */
jboolean JNICALL ScriptMethodsPvpNamespace::pvpHasBattlefieldEnemyFlag(JNIEnv *env, jobject self, jlong target)
{
	// a battlefield enemy flag is an FEF toward the battlefield faction
	TangibleObject *targetObj = 0;
	if (!JavaLibrary::getObject(target, targetObj))
		return JNI_FALSE;
	return Pvp::hasFactionEnemyFlag(*targetObj, PvpFactions::getBattlefieldFactionId());
}

// ----------------------------------------------------------------------

/**
 * Get an array of enemy objects which are in range of a location.
 *
 * @param env   Java Environment
 * @param self  class calling this function
 * @param actor The actor for the enemy check
 * @param from  The center of the range
 * @param range The distance to query.
 * @return nullptr if there is an error, or an array of enemy objects which are in range.
 */
jlongArray JNICALL ScriptMethodsPvpNamespace::pvpGetEnemiesInRange(JNIEnv *env, jobject self, jlong actor, jlong location, jfloat range)
{
	UNREF(self);

	TangibleObject *actorObject = 0;
	if (!JavaLibrary::getObject(actor, actorObject) || !actorObject)
		return 0;

	if (!location)
		return 0;
	Vector target;
	if (!ScriptConversion::convertWorld(location, target))
		return 0;

	std::vector<ServerObject*> results;
	ServerWorld::findObjectsInRange(target, range, results);

	std::vector<ServerObject*> filteredResults(results.size());
	for (std::vector<ServerObject*>::iterator i = results.begin(); i != results.end(); ++i)
	{
		TangibleObject *t = (*i)->asTangibleObject();
		if (t && Pvp::isEnemy(*actorObject, *t))
			filteredResults.push_back(t);
	}

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(filteredResults, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 * Get an array of enemy objects which are in a cone of a location.
 *
 * @param env                  Java Environment
 * @param self                 class calling this function
 * @param actor                The actor for the enemy check
 * @param coneCenterObject     The object at the center (base, pointy part) of the cone.
 * @param coneDirectionObject  An object to orient the axis of the cone.  The axis goes from
 *                             the coneCenterObject position to this object's position.
 * @param range                The distance to query from the center of the cone.  Note the cone
 *                             extends along the cone axis this distance, and does not necessarily
 *                             extend as far as the cone direction object's distance from the cone center.
 * @param angle                The cone angle, in degrees, that the cone sweeps out.  The
 *                             total cone angle is twice this angle; therefore, this angle
 *                             represents the angle swept out to each side of the cone axis.
 * @return nullptr if there is an error, or an array of enemy objects which are in range.
 */
jlongArray JNICALL ScriptMethodsPvpNamespace::pvpGetEnemiesInCone(JNIEnv *env, jobject self, jlong actor, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle)
{
	UNREF(self);

	if (!coneCenterObjectId || !coneDirectionObjectId)
		return 0;

	TangibleObject *actorObject = 0;
	Object *coneCenterObject = 0;
	Object *coneDirectionObject = 0;

	if (!JavaLibrary::getObject(actor, actorObject) || !actorObject)
		return 0;
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject) || !coneCenterObject)
		return 0;
	if (!JavaLibrary::getObject(coneDirectionObjectId, coneDirectionObject) || !coneDirectionObject)
		return 0;

	std::vector<ServerObject*> results;
	ServerWorld::findObjectsInCone(*coneCenterObject, *coneDirectionObject, range, convertDegreesToRadians(angle), results);

	std::vector<ServerObject*> filteredResults(results.size());
	for (std::vector<ServerObject*>::iterator i = results.begin(); i != results.end(); ++i)
	{
		TangibleObject *t = (*i)->asTangibleObject();
		if (t && Pvp::isEnemy(*actorObject, *t))
			filteredResults.push_back(t);
	}

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(filteredResults, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 * Get an array of enemy objects which are in a cone of a location.
 *
 * @param env                   Java Environment
 * @param self                  class calling this function
 * @param actor                 The actor for the enemy check
 * @param coneCenterObjectId    The object at the center (base, pointy part) of the cone.
 * @param coneDirectionLocation The position to orient the axis of the cone.  The axis goes from
 *                              the coneCenterObject position to this position (scene value of the location ignored).
 * @param range                 The distance to query from the center of the cone.  Note the cone
 *                              extends along the cone axis this distance, and does not necessarily
 *                              extend as far as the cone direction object's distance from the cone center.
 * @param angle                 The cone angle, in degrees, that the cone sweeps out.  The
 *                              total cone angle is twice this angle; therefore, this angle
 *                              represents the angle swept out to each side of the cone axis.
 * @return nullptr if there is an error, or an array of enemy objects which are in range.
 */
jlongArray JNICALL ScriptMethodsPvpNamespace::pvpGetEnemiesInConeLocation(JNIEnv *env, jobject self, jlong actor, jlong coneCenterObjectId, jobject coneDirectionLocation, jfloat range, jfloat angle)
{
	UNREF(self);

	if (!coneCenterObjectId || !coneDirectionLocation)
		return 0;

	TangibleObject *actorObject = 0;
	Object *coneCenterObject = 0;
	Location coneDirection;

	if (!JavaLibrary::getObject(actor, actorObject) || !actorObject)
		return 0;
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject) || !coneCenterObject)
		return 0;
	if (!ScriptConversion::convert(LocalRefParam(coneDirectionLocation), coneDirection))
		return 0;

	std::vector<ServerObject*> results;
	ServerWorld::findObjectsInCone(*coneCenterObject, coneDirection, range, convertDegreesToRadians(angle), results);

	std::vector<ServerObject*> filteredResults(results.size());
	for (std::vector<ServerObject*>::iterator i = results.begin(); i != results.end(); ++i)
	{
		TangibleObject *t = (*i)->asTangibleObject();
		if (t && Pvp::isEnemy(*actorObject, *t))
			filteredResults.push_back(t);
	}

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(filteredResults, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 * Get an array of attackable objects which are in range of a location.
 *
 * @param env   Java Environment
 * @param self  class calling this function
 * @param actor The actor for the canAttack check
 * @param from  The center of the range
 * @param range The distance to query.
 * @return nullptr if there is an error, or an array of attackable objects which are in range.
 */
jlongArray JNICALL ScriptMethodsPvpNamespace::pvpGetTargetsInRange(JNIEnv *env, jobject self, jlong actor, jlong location, jfloat range)
{
	UNREF(self);

	TangibleObject *actorObject = 0;
	if (!JavaLibrary::getObject(actor, actorObject) || !actorObject)
		return 0;

	if (!location)
		return 0;

	Vector target;
	if (!ScriptConversion::convertWorld(location, target))
		return 0;

	std::vector<ServerObject*> results;
	ServerWorld::findObjectsInRange(target, range, results);

	std::vector<ServerObject*> filteredResults(results.size());
	for (std::vector<ServerObject*>::iterator i = results.begin(); i != results.end(); ++i)
	{
		TangibleObject *t = (*i)->asTangibleObject();
		if (t && Pvp::canAttack(*actorObject, *t))
			filteredResults.push_back(t);
	}

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(filteredResults, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 * Get an array of attackable objects which are in a cone of a location.
 *
 * @param env                  Java Environment
 * @param self                 class calling this function
 * @param actor                The actor for the canAttack check
 * @param coneCenterObject     The object at the center (base, pointy part) of the cone.
 * @param coneDirectionObject  An object to orient the axis of the cone.  The axis goes from
 *                             the coneCenterObject position to this object's position.
 * @param range                The distance to query from the center of the cone.  Note the cone
 *                             extends along the cone axis this distance, and does not necessarily
 *                             extend as far as the cone direction object's distance from the cone center.
 * @param angle                The cone angle, in degrees, that the cone sweeps out.  The
 *                             total cone angle is twice this angle; therefore, this angle
 *                             represents the angle swept out to each side of the cone axis.
 * @return nullptr if there is an error, or an array of attackable objects which are in range.
 */
jlongArray JNICALL ScriptMethodsPvpNamespace::pvpGetTargetsInCone(JNIEnv *env, jobject self, jlong actor, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle)
{
	UNREF(self);

	if (!coneCenterObjectId || !coneDirectionObjectId)
		return 0;

	TangibleObject *actorObject = 0;
	Object *coneCenterObject = 0;
	Object *coneDirectionObject = 0;

	if (!JavaLibrary::getObject(actor, actorObject) || !actorObject)
		return 0;
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject) || !coneCenterObject)
		return 0;
	if (!JavaLibrary::getObject(coneDirectionObjectId, coneDirectionObject) || !coneDirectionObject)
		return 0;

	std::vector<ServerObject*> results;
	ServerWorld::findObjectsInCone(*coneCenterObject, *coneDirectionObject, range, convertDegreesToRadians(angle), results);

	std::vector<ServerObject*> filteredResults(results.size());
	for (std::vector<ServerObject*>::iterator i = results.begin(); i != results.end(); ++i)
	{
		TangibleObject *t = (*i)->asTangibleObject();
		if (t && Pvp::canAttack(*actorObject, *t))
			filteredResults.push_back(t);
	}

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(filteredResults, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 * Get an array of attackable objects which are in a cone of a location.
 *
 * @param env                   Java Environment
 * @param self                  class calling this function
 * @param actor                 The actor for the canAttack check
 * @param coneCenterObject      The object at the center (base, pointy part) of the cone.
 * @param coneDirectionLocation The position to orient the axis of the cone.  The axis goes from
 *                              the coneCenterObject position to this position (scene value of the location ignored).
 * @param range                 The distance to query from the center of the cone.  Note the cone
 *                              extends along the cone axis this distance, and does not necessarily
 *                              extend as far as the cone direction object's distance from the cone center.
 * @param angle                 The cone angle, in degrees, that the cone sweeps out.  The
 *                              total cone angle is twice this angle; therefore, this angle
 *                              represents the angle swept out to each side of the cone axis.
 * @return nullptr if there is an error, or an array of attackable objects which are in range.
 */
jlongArray JNICALL ScriptMethodsPvpNamespace::pvpGetTargetsInConeLocation(JNIEnv *env, jobject self, jlong actor, jlong coneCenterObjectId, jobject coneDirectionLocation, jfloat range, jfloat angle)
{
	UNREF(self);

	if (!coneCenterObjectId || !coneDirectionLocation)
		return 0;

	TangibleObject *actorObject = 0;
	Object *coneCenterObject = 0;
	Location coneDirection;

	if (!JavaLibrary::getObject(actor, actorObject) || !actorObject)
		return 0;
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject) || !coneCenterObject)
		return 0;
	if (!ScriptConversion::convert(LocalRefParam(coneDirectionLocation), coneDirection))
		return 0;

	std::vector<ServerObject*> results;
	ServerWorld::findObjectsInCone(*coneCenterObject, coneDirection, range, convertDegreesToRadians(angle), results);

	std::vector<ServerObject*> filteredResults(results.size());
	for (std::vector<ServerObject*>::iterator i = results.begin(); i != results.end(); ++i)
	{
		TangibleObject *t = (*i)->asTangibleObject();
		if (t && Pvp::canAttack(*actorObject, *t))
			filteredResults.push_back(t);
	}

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(filteredResults, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 * Check whether a faction has another faction as an opponent
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param faction1  id for faction to check opponents of
 * @param faction2  id for potential opponent
 * @return whether factions are opposed
 */
jboolean JNICALL ScriptMethodsPvpNamespace::pvpAreFactionsOpposed(JNIEnv *env, jobject self, jint faction1, jint faction2)
{
	return Pvp::areFactionsOpposed(faction1, faction2);
}

// ----------------------------------------------------------------------

/**
 * Get a person's faction on a particular battlefield
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param target    the person to get the faction for
 * @param region    the battlefield region
 * @return the faction id or 0
 */
jint JNICALL ScriptMethodsPvpNamespace::pvpBattlefieldGetFaction(JNIEnv *env, jobject self, jlong target, jobject region)
{
	TangibleObject *targetObject = 0;
	if (!JavaLibrary::getObject(target, targetObject))
		return 0;

	Region const *regionObject = 0;
	if (!ScriptConversion::convert(region, regionObject))
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldGetFaction object passed in is not a region"));
		return 0;
	}

	RegionPvp const *pvpRegion = regionObject->asRegionPvp();
	if (!pvpRegion)
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldGetFaction object passed in is not a pvp region"));
		return 0;
	}

	return Pvp::battlefieldGetFaction(*targetObject, *pvpRegion);
}

// ----------------------------------------------------------------------

/**
 * Check whether a person is a participant on a battlefield
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param target    the person to check
 * @param region    the battlefield region
 * @return  whether the person is involved in the battlefield
 */
jboolean JNICALL ScriptMethodsPvpNamespace::pvpBattlefieldIsParticipant(JNIEnv *env, jobject self, jlong target, jobject region)
{
	TangibleObject *targetObject = 0;
	if (!JavaLibrary::getObject(target, targetObject))
		return JNI_FALSE;

	Region const *regionObject = 0;
	if (!ScriptConversion::convert(region, regionObject))
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldIsParticipant object passed in is not a region"));
		return JNI_FALSE;
	}

	RegionPvp const *pvpRegion = regionObject->asRegionPvp();
	if (!pvpRegion)
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldIsParticipant object passed in is not a pvp region"));
		return JNI_FALSE;
	}

	if (Pvp::battlefieldIsParticipant(*targetObject, *pvpRegion))
		return JNI_TRUE;
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

/**
 * Set the faction a person is fighting for on a battlefield
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param target    the person
 * @param region    the battlefield region
 * @param faction   the faction
 */
void JNICALL ScriptMethodsPvpNamespace::pvpBattlefieldSetParticipant(JNIEnv *env, jobject self, jlong target, jobject region, jint faction)
{
	TangibleObject *targetObject = 0;
	if (!JavaLibrary::getObject(target, targetObject))
		return;

	Region const *regionObject = 0;
	if (!ScriptConversion::convert(region, regionObject))
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldSetParticipant object passed in is not a region"));
		return;
	}

	RegionPvp const *pvpRegion = regionObject->asRegionPvp();
	if (!pvpRegion)
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldSetParticipant object passed in is not a pvp region"));
		return;
	}

	Pvp::battlefieldSetParticipant(*targetObject, *pvpRegion, faction);
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsPvpNamespace::pvpBattlefieldGetParticipantsForFaction(JNIEnv *env, jobject self, jobject region, jint faction)
{
	Region const *regionObject = 0;
	if (!ScriptConversion::convert(region, regionObject))
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldGetParticipantsForFaction object passed in is not a region"));
		return 0;
	}

	RegionPvp const *pvpRegion = regionObject->asRegionPvp();
	if (!pvpRegion)
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldGetParticipantsForFaction object passed in is not a pvp region"));
		return 0;
	}

	BattlefieldMarkerObject const *marker = pvpRegion->getBattlefieldMarker();
	if (!marker)
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldGetParticipantsForFaction could not find battlefield marker for region"));
		return 0;
	}

	std::map<NetworkId, uint32> const &participantMap = marker->getBattlefieldParticipants();

	int count = 0;

	{
		for (std::map<NetworkId, uint32>::const_iterator i = participantMap.begin(); i != participantMap.end(); ++i)
			if (faction == 0 || (*i).second == static_cast<uint32>(faction))
				++count;
	}

	LocalLongArrayRefPtr items = createNewLongArray(count);
	if (items == LocalLongArrayRef::cms_nullPtr)
		return 0;

	int index = 0;
	jlong jlongTmp;
	{
		for (std::map<NetworkId, uint32>::const_iterator i = participantMap.begin(); i != participantMap.end(); ++i)
		{
			if (faction == 0 || (*i).second == static_cast<uint32>(faction))
			{
				jlongTmp = ((*i).first).getValue();
				setLongArrayRegion(*items, index, 1, &jlongTmp);
				++index;
			}
		}
	}

	return items->getReturnValue();
}

// ----------------------------------------------------------------------

/**
 * Clear the participant info for a battlefield region.
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param region    the battlefield region
 */
void JNICALL ScriptMethodsPvpNamespace::pvpBattlefieldClearParticipants(JNIEnv *env, jobject self, jobject region)
{
	Region const *regionObject = 0;
	if (!ScriptConversion::convert(region, regionObject))
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldClearParticipants object passed in is not a region"));
		return;
	}

	RegionPvp const *pvpRegion = regionObject->asRegionPvp();
	if (!pvpRegion)
	{
		WARNING(true, ("JavaLibrary::pvpBattlefieldClearParticipants object passed in is not a pvp region"));
		return;
	}

	Pvp::battlefieldClearParticipants(*pvpRegion);
}

// ----------------------------------------------------------------------

/**
 * Remove all temp enemy flags from an object (generally used for death)
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param target    the object to strip temp enemy flags from
 */
void JNICALL ScriptMethodsPvpNamespace::pvpRemoveAllTempEnemyFlags(JNIEnv *env, jobject self, jlong target)
{
	TangibleObject *targetObject = 0;
	if (JavaLibrary::getObject(target, targetObject))
		Pvp::removeAllTempEnemyFlags(*targetObject);
}

// ----------------------------------------------------------------------

/**
 * Returns the battlefield marker object associated with a battlefield region.
 *
 * @param env       Java Environment
 * @param self      class calling this function
 * @param region    the battlefield region
 *
 * @return the marker object
 */
jlong JNICALL ScriptMethodsPvpNamespace::getBattlefieldRegionMasterObject(JNIEnv *env, jobject self, jobject battlefieldRegion)
{
	Region const *regionObject = 0;
	if (!ScriptConversion::convert(battlefieldRegion, regionObject))
	{
		WARNING(true, ("JavaLibrary::getBattlefieldRegionMasterObject object passed in is not a region"));
		return 0;
	}

	RegionPvp const *pvpRegion = regionObject->asRegionPvp();
	if (!pvpRegion)
	{
		WARNING(true, ("JavaLibrary::getBattlefieldRegionMasterObject object passed in is not a pvp region"));
		return 0;
	}

	BattlefieldMarkerObject const *markerObject = pvpRegion->getBattlefieldMarker();
	NetworkId marker = markerObject ? markerObject->getNetworkId() : NetworkId::cms_invalid;
	return marker.getValue();
}	// JavaLibrary::getBattlefieldRegionMasterObject

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::setBattlefieldMarkerRegionName(JNIEnv *env, jobject self, jlong marker, jstring regionName)
{
	BattlefieldMarkerObject *markerObject = 0;
	if (JavaLibrary::getObject(marker, markerObject))
	{
		JavaStringParam localRegionName(regionName);
		std::string regionNameString;
		JavaLibrary::convert(localRegionName, regionNameString);
		markerObject->setRegionName(regionNameString);
	}
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPvpNamespace::pvpHasAnyTempEnemyFlags(JNIEnv *env, jobject self, jlong target)
{
	TangibleObject *targetObject = 0;
	if (JavaLibrary::getObject(target, targetObject))
		return static_cast<bool>(Pvp::hasAnyTimedEnemyFlag(*targetObject) ? JNI_TRUE : JNI_FALSE);
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsPvpNamespace::pvpGetPersonalEnemyIds(JNIEnv *env, jobject self, jlong target)
{
	TangibleObject *targetObject = 0;
	if (!JavaLibrary::getObject(target, targetObject))
		return 0;

	std::vector<NetworkId> enemyIds;
	Pvp::getPersonalEnemyIds(*targetObject, enemyIds);

	LocalLongArrayRefPtr items = createNewLongArray(enemyIds.size());
	if (items == LocalLongArrayRef::cms_nullPtr)
		return 0;

	jlong jlongTmp;
	for (unsigned int i = 0; i < enemyIds.size(); ++i)
	{
		jlongTmp = enemyIds[i].getValue();
		setLongArrayRegion(*items, i, 1, &jlongTmp);
	}

	return items->getReturnValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::pvpRemoveTempEnemyFlags(JNIEnv *env, jobject self, jlong target, jlong enemyId)
{
	TangibleObject *targetObject = 0;
	if (!JavaLibrary::getObject(target, targetObject))
		return;

	NetworkId enemyNetworkId(enemyId);
	Pvp::removeTempEnemyFlags(*targetObject, enemyNetworkId);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPvpNamespace::pvpWouldAttackCauseAlignedEnemyFlag(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	TangibleObject *actorObject = 0;
	TangibleObject *targetObject = 0;
	if (!JavaLibrary::getObject(actor, actorObject) ||
	    !JavaLibrary::getObject(target, targetObject))
	{
		return JNI_FALSE;
	}
	return Pvp::wouldAttackCauseAlignedEnemyFlag(*actorObject, *targetObject);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPvpNamespace::pvpWouldHelpCauseAlignedEnemyFlag(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	TangibleObject *actorObject = 0;
	TangibleObject *targetObject = 0;
	if (!JavaLibrary::getObject(actor, actorObject) ||
	    !JavaLibrary::getObject(target, targetObject))
	{
		return JNI_FALSE;
	}
	return Pvp::wouldHelpCauseAlignedEnemyFlag(*actorObject, *targetObject);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPvpNamespace::pvpHasPersonalEnemyFlag(JNIEnv *env, jobject self, jlong actor, jlong target)
{
	TangibleObject *actorObj = 0;
	if (!JavaLibrary::getObject(actor, actorObj))
		return JNI_FALSE;

	if (!actorObj)
		return JNI_FALSE;

	NetworkId targetNetworkId(target);
	if (Pvp::hasPersonalEnemyFlag(*actorObj, targetNetworkId))
		return JNI_TRUE;

	return JNI_FALSE;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::pvpRemovePersonalEnemyFlags(JNIEnv *env, jobject self, jlong target, jlong enemyId)
{
	TangibleObject *targetObject = 0;
	if (!JavaLibrary::getObject(target, targetObject))
		return;

	if (!targetObject)
		return;

	NetworkId enemyNetworkId(enemyId);
	Pvp::removePersonalEnemyFlags(*targetObject, enemyNetworkId);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::pvpPrepareToBeCovert(JNIEnv * env, jobject /*self*/, jlong dest)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
	{
		return;
	}

	if (destObj != 0)
	{
		Pvp::prepareToBeCovert(*destObj);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::pvpPrepareToBeDeclared(JNIEnv * env, jobject /*self*/, jlong dest)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
	{
		return;
	}

	if (destObj != 0)
	{
		Pvp::prepareToBeDeclared(*destObj);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::pvpPrepareToBeNeutral(JNIEnv * env, jobject /*self*/, jlong dest)
{
	TangibleObject *destObj = 0;
	if (!JavaLibrary::getObject(dest, destObj))
	{
		return;
	}

	if (destObj != 0)
	{
		Pvp::prepareToBeNeutral(*destObj);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::pvpModifyCurrentGcwPoints(JNIEnv *env, jobject self, jlong target, jint adjustment)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return;

	PlayerObject * const player = PlayerCreatureController::getPlayerObject(creature);
	if (player)
	{
		// grant GCW Region Defender bonus
		float bonus = 0.0f;
		if ((adjustment > 0) && Pvp::getGcwDefenderRegionBonus(*creature, *player, bonus) && (bonus > 0.0f))
			adjustment += std::max(1, static_cast<int>(static_cast<double>(bonus) * static_cast<double>(adjustment) / static_cast<double>(100)));

		player->modifyCurrentGcwPoints(adjustment, true);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::pvpModifyCurrentPvpKills(JNIEnv *env, jobject self, jlong target, jint adjustment)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player)
		player->modifyCurrentPvpKills(adjustment, true);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::pvpGetCurrentGcwRank(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return 0;

	return player->getCurrentGcwRank();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::pvpGetMaxGcwImperialRank(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return 0;

	return player->getMaxGcwImperialRank();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::pvpGetMaxGcwRebelRank(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return 0;

	return player->getMaxGcwRebelRank();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::pvpGetCurrentGcwPoints(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return 0;

	return static_cast<jint>(player->getCurrentGcwPoints());
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::pvpGetCurrentGcwRating(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return -1;

	return static_cast<jint>(player->getCurrentGcwRating());
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::pvpGetCurrentPvpKills(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return 0;

	return static_cast<jint>(player->getCurrentPvpKills());
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsPvpNamespace::pvpGetLifetimeGcwPoints(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return 0;

	return static_cast<jlong>(player->getLifetimeGcwPoints());
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::pvpGetMaxGcwImperialRating(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return -1;

	return static_cast<jint>(player->getMaxGcwImperialRating());
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::pvpGetMaxGcwRebelRating(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return -1;

	return static_cast<jint>(player->getMaxGcwRebelRating());
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::pvpGetLifetimePvpKills(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return 0;

	return static_cast<jint>(player->getLifetimePvpKills());
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::pvpGetNextGcwRatingCalcTime(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return 0;

	return static_cast<jint>(player->getNextGcwRatingCalcTime());
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::ctsUseOnlySetGcwInfo(JNIEnv *env, jobject self, jlong target, jint currentGcwPoints, jint currentGcwRating, jint currentPvpKills, jlong lifetimeGcwPoints, jint maxGcwImperialRating, jint maxGcwRebelRating, jint lifetimePvpKills, jint nextGcwRatingCalcTime)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (!player)
		return;

	player->ctsUseOnlySetGcwInfo(static_cast<int32>(currentGcwPoints), static_cast<int32>(currentGcwRating), static_cast<int32>(currentPvpKills), static_cast<int64>(lifetimeGcwPoints), static_cast<int32>(maxGcwImperialRating), static_cast<int32>(maxGcwRebelRating), static_cast<int32>(lifetimePvpKills), static_cast<int32>(nextGcwRatingCalcTime));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::adjustGcwImperialScore(JNIEnv *env, jobject self, jstring source, jlong sourceOid, jstring gcwCategory, jint adjustment)
{
	JavaStringParam localSource(source);
	std::string sourceString;
	JavaLibrary::convert(localSource, sourceString);

	JavaStringParam localGcwCategory(gcwCategory);
	std::string gcwCategoryString;
	JavaLibrary::convert(localGcwCategory, gcwCategoryString);

	CreatureObject * creature = 0;
	JavaLibrary::getObject(sourceOid, creature);

	ServerUniverse::getInstance().adjustGcwImperialScore(sourceString, creature, gcwCategoryString, adjustment);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPvpNamespace::adjustGcwRebelScore(JNIEnv *env, jobject self, jstring source, jlong sourceOid, jstring gcwCategory, jint adjustment)
{
	JavaStringParam localSource(source);
	std::string sourceString;
	JavaLibrary::convert(localSource, sourceString);

	JavaStringParam localGcwCategory(gcwCategory);
	std::string gcwCategoryString;
	JavaLibrary::convert(localGcwCategory, gcwCategoryString);

	CreatureObject * creature = 0;
	JavaLibrary::getObject(sourceOid, creature);

	ServerUniverse::getInstance().adjustGcwRebelScore(sourceString, creature, gcwCategoryString, adjustment);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::getGcwImperialScorePercentile(JNIEnv *env, jobject self, jstring gcwCategory)
{
	JavaStringParam localGcwCategory(gcwCategory);
	std::string gcwCategoryString;
	JavaLibrary::convert(localGcwCategory, gcwCategoryString);

	return ServerUniverse::getInstance().getGcwImperialScorePercentile(gcwCategoryString);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::getGcwGroupImperialScorePercentile(JNIEnv *env, jobject self, jstring gcwGroup)
{
	JavaStringParam localGcwGroup(gcwGroup);
	std::string gcwGroupString;
	JavaLibrary::convert(localGcwGroup, gcwGroupString);

	return ServerUniverse::getInstance().getGcwGroupImperialScorePercentile(gcwGroupString);
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsPvpNamespace::getGcwFactionalPresenceTableDictionary(JNIEnv * env, jobject self)
{
	ScriptParams const * scriptParams = PlanetObject::getConnectedCharacterLfgDataFactionalPresenceTableDictionary();
	if (!scriptParams)
		return 0;

	JavaDictionaryPtr dictionary;
	JavaLibrary::instance()->convert(*scriptParams, dictionary);
	return dictionary->getReturnValue();
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsPvpNamespace::getGcwContributionTrackingTableDictionary(JNIEnv * env, jobject self, jlong player)
{
	UNREF(self);

	CreatureObject const * creatureObj = 0;
	if (!JavaLibrary::getObject(player, creatureObj))
		return 0;

	PlayerObject const * const playerObj = PlayerCreatureController::getPlayerObject(creatureObj);
	if (!playerObj)
		return 0;

	DynamicVariableList::NestedList const gcwContribution(playerObj->getObjVars(), "gcwContributionTracking");
	int const size = gcwContribution.getCount();
	if (size <= 0)
		return 0;

	std::vector<const char *> * scriptParamsRegion = new std::vector<const char *>;
	std::vector<const char *> * scriptParamsTime = new std::vector<const char *>;
	scriptParamsRegion->reserve(size);
	scriptParamsTime->reserve(size);

	char buffer[128];
	int timeLastContributed = 0;
	for (DynamicVariableList::NestedList::const_iterator i = gcwContribution.begin(); i != gcwContribution.end(); ++i)
	{
		if (Pvp::getGcwScoreCategory(i.getName()) && i.getValue(timeLastContributed))
		{
			snprintf(buffer, sizeof(buffer)-1, "@gcw_regions:%s", i.getName().c_str());
			buffer[sizeof(buffer)-1] = '\0';

			scriptParamsRegion->push_back(makeCopyOfString(buffer));
			scriptParamsTime->push_back(makeCopyOfString(CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(static_cast<time_t>(timeLastContributed)).c_str()));
		}
	}

	// column header
	static const char * s_scriptParamsColumnHeadersText[2] = 
	{
		"GCW Region/Category",
		"Time Of Last Contribution"
	};
	static std::vector<const char *> s_scriptParamsColumnHeaders(s_scriptParamsColumnHeadersText, s_scriptParamsColumnHeadersText + (sizeof(s_scriptParamsColumnHeadersText) / sizeof(const char *)));

	// column type
	static const char * s_scriptParamsColumnTypeText[2] = 
	{
		"text",
		"text"
	};
	static std::vector<const char *> s_scriptParamsColumnType(s_scriptParamsColumnTypeText, s_scriptParamsColumnTypeText + (sizeof(s_scriptParamsColumnTypeText) / sizeof(const char *)));

	ScriptParams sp;
	sp.addParam(s_scriptParamsColumnHeaders, "column", false);
	sp.addParam(s_scriptParamsColumnType, "columnType", false);
	sp.addParam(*scriptParamsRegion, "column0", true);
	sp.addParam(*scriptParamsTime, "column1", true);

	JavaDictionaryPtr dictionary;
	JavaLibrary::instance()->convert(sp, dictionary);
	return dictionary->getReturnValue();
}

//-----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegions(JNIEnv * /*env*/, jobject /*self*/)
{
	// walk the list twice, this method shouldn't get called
	// very often as the result is cached in script
	std::map<std::string, Pvp::GcwScoreCategory const *> const & allGcwScoreCategory = Pvp::getAllGcwScoreCategory(); 
	std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter;
	int count = 0;
	for (iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
	{
		if (iter->second->gcwRegionDefender)
			++count;
	}

	if (count <= 0)
		return 0;

	LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsString());
	count = 0;
	for (iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
	{
		if (iter->second->gcwRegionDefender)
		{
			JavaString jval(std::string("@gcw_regions:") + iter->second->categoryName);
			setObjectArrayElement(*valueArray, count++, jval);
		}
	}

	return valueArray->getReturnValue();
}

//-----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionsCitiesImperial(JNIEnv *env, jobject self)
{
	// walk the list twice, this method shouldn't get called
	// very often as the result is cached in script
	std::map<std::string, Pvp::GcwScoreCategory const *> const & allGcwScoreCategory = Pvp::getAllGcwScoreCategory(); 
	std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter;
	int count = 0;
	for (iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
	{
		if (iter->second->gcwRegionDefender)
			++count;
	}

	if (count <= 0)
		return 0;

	LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsString());
	std::map<std::pair<std::string, int>, uint32> const & gcwRegionDefenderCities = CityInterface::getGcwRegionDefenderCities();
	count = 0;
	for (iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
	{
		if (iter->second->gcwRegionDefender)
		{
			std::string cities;
			int numberOfCities = 0;

			for (std::map<std::pair<std::string, int>, uint32>::const_iterator iterCity = gcwRegionDefenderCities.lower_bound(std::make_pair(iter->first, 0)); ((iterCity != gcwRegionDefenderCities.end()) && (iterCity->first.first == iter->first)); ++iterCity)
			{
				if (PvpData::isImperialFactionId(iterCity->second))
				{
					if (!cities.empty())
						cities += ", ";

					cities += CityInterface::getCityInfo(iterCity->first.second).getCityName();
					++numberOfCities;
				}
			}

			if ((numberOfCities > 0) && !cities.empty())
				cities = std::string(FormattedString<32>().sprintf("%02d - ", numberOfCities)) + cities;

			JavaString jval(cities);
			setObjectArrayElement(*valueArray, count++, jval);
		}
	}

	return valueArray->getReturnValue();
}

//-----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionsCitiesRebel(JNIEnv *env, jobject self)
{
	// walk the list twice, this method shouldn't get called
	// very often as the result is cached in script
	std::map<std::string, Pvp::GcwScoreCategory const *> const & allGcwScoreCategory = Pvp::getAllGcwScoreCategory(); 
	std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter;
	int count = 0;
	for (iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
	{
		if (iter->second->gcwRegionDefender)
			++count;
	}

	if (count <= 0)
		return 0;

	LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsString());
	std::map<std::pair<std::string, int>, uint32> const & gcwRegionDefenderCities = CityInterface::getGcwRegionDefenderCities();
	count = 0;
	for (iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
	{
		if (iter->second->gcwRegionDefender)
		{
			std::string cities;
			int numberOfCities = 0;

			for (std::map<std::pair<std::string, int>, uint32>::const_iterator iterCity = gcwRegionDefenderCities.lower_bound(std::make_pair(iter->first, 0)); ((iterCity != gcwRegionDefenderCities.end()) && (iterCity->first.first == iter->first)); ++iterCity)
			{
				if (PvpData::isRebelFactionId(iterCity->second))
				{
					if (!cities.empty())
						cities += ", ";

					cities += CityInterface::getCityInfo(iterCity->first.second).getCityName();
					++numberOfCities;
				}
			}

			if ((numberOfCities > 0) && !cities.empty())
				cities = std::string(FormattedString<32>().sprintf("%02d - ", numberOfCities)) + cities;

			JavaString jval(cities);
			setObjectArrayElement(*valueArray, count++, jval);
		}
	}

	return valueArray->getReturnValue();
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionsCitiesVersion(JNIEnv *env, jobject self)
{
	return CityInterface::getGcwRegionDefenderCitiesVersion();
}

//-----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionsGuildsImperial(JNIEnv *env, jobject self)
{
	// walk the list twice, this method shouldn't get called
	// very often as the result is cached in script
	std::map<std::string, Pvp::GcwScoreCategory const *> const & allGcwScoreCategory = Pvp::getAllGcwScoreCategory(); 
	std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter;
	int count = 0;
	for (iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
	{
		if (iter->second->gcwRegionDefender)
			++count;
	}

	if (count <= 0)
		return 0;

	LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsString());
	std::map<std::pair<std::string, int>, uint32> const & gcwRegionDefenderGuilds = GuildInterface::getGcwRegionDefenderGuilds();
	count = 0;
	for (iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
	{
		if (iter->second->gcwRegionDefender)
		{
			std::string guilds;
			int numberOfGuilds = 0;

			for (std::map<std::pair<std::string, int>, uint32>::const_iterator iterGuild = gcwRegionDefenderGuilds.lower_bound(std::make_pair(iter->first, 0)); ((iterGuild != gcwRegionDefenderGuilds.end()) && (iterGuild->first.first == iter->first)); ++iterGuild)
			{
				if (PvpData::isImperialFactionId(iterGuild->second))
				{
					if (!guilds.empty())
						guilds += ", ";

					guilds += GuildInterface::getGuildName(iterGuild->first.second);
					guilds += " (";
					guilds += GuildInterface::getGuildAbbrev(iterGuild->first.second);
					guilds += ")";
					++numberOfGuilds;
				}
			}

			if ((numberOfGuilds > 0) && !guilds.empty())
				guilds = std::string(FormattedString<32>().sprintf("%02d - ", numberOfGuilds)) + guilds;

			JavaString jval(guilds);
			setObjectArrayElement(*valueArray, count++, jval);
		}
	}

	return valueArray->getReturnValue();
}

//-----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionsGuildsRebel(JNIEnv *env, jobject self)
{
	// walk the list twice, this method shouldn't get called
	// very often as the result is cached in script
	std::map<std::string, Pvp::GcwScoreCategory const *> const & allGcwScoreCategory = Pvp::getAllGcwScoreCategory(); 
	std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter;
	int count = 0;
	for (iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
	{
		if (iter->second->gcwRegionDefender)
			++count;
	}

	if (count <= 0)
		return 0;

	LocalObjectArrayRefPtr valueArray = createNewObjectArray(count, JavaLibrary::getClsString());
	std::map<std::pair<std::string, int>, uint32> const & gcwRegionDefenderGuilds = GuildInterface::getGcwRegionDefenderGuilds();
	count = 0;
	for (iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
	{
		if (iter->second->gcwRegionDefender)
		{
			std::string guilds;
			int numberOfGuilds = 0;

			for (std::map<std::pair<std::string, int>, uint32>::const_iterator iterGuild = gcwRegionDefenderGuilds.lower_bound(std::make_pair(iter->first, 0)); ((iterGuild != gcwRegionDefenderGuilds.end()) && (iterGuild->first.first == iter->first)); ++iterGuild)
			{
				if (PvpData::isRebelFactionId(iterGuild->second))
				{
					if (!guilds.empty())
						guilds += ", ";

					guilds += GuildInterface::getGuildName(iterGuild->first.second);
					guilds += " (";
					guilds += GuildInterface::getGuildAbbrev(iterGuild->first.second);
					guilds += ")";
					++numberOfGuilds;
				}
			}

			if ((numberOfGuilds > 0) && !guilds.empty())
				guilds = std::string(FormattedString<32>().sprintf("%02d - ", numberOfGuilds)) + guilds;

			JavaString jval(guilds);
			setObjectArrayElement(*valueArray, count++, jval);
		}
	}

	return valueArray->getReturnValue();
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionsGuildsVersion(JNIEnv *env, jobject self)
{
	return GuildInterface::getGcwRegionDefenderGuildsVersion();
}

//-----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionCitiesImperial(JNIEnv *env, jobject self, jstring gcwCategory)
{
	JavaStringParam localGcwCategory(gcwCategory);
	std::string gcwCategoryString;
	if (!JavaLibrary::convert(localGcwCategory, gcwCategoryString))
		return 0;

	std::vector<int> cityIds;
	std::map<std::pair<std::string, int>, uint32> const & gcwRegionDefenderCities = CityInterface::getGcwRegionDefenderCities();
	for (std::map<std::pair<std::string, int>, uint32>::const_iterator iterCity = gcwRegionDefenderCities.lower_bound(std::make_pair(gcwCategoryString, 0)); ((iterCity != gcwRegionDefenderCities.end()) && (iterCity->first.first == gcwCategoryString)); ++iterCity)
	{
		if (PvpData::isImperialFactionId(iterCity->second))
			cityIds.push_back(iterCity->first.second);
	}

	if (cityIds.empty())
		return 0;

	LocalIntArrayRefPtr ret = createNewIntArray(cityIds.size());
	setIntArrayRegion(*ret, 0, cityIds.size(), reinterpret_cast<jint *>(&cityIds[0]));
	return ret->getReturnValue();
}

//-----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionCitiesRebel(JNIEnv *env, jobject self, jstring gcwCategory)
{
	JavaStringParam localGcwCategory(gcwCategory);
	std::string gcwCategoryString;
	if (!JavaLibrary::convert(localGcwCategory, gcwCategoryString))
		return 0;

	std::vector<int> cityIds;
	std::map<std::pair<std::string, int>, uint32> const & gcwRegionDefenderCities = CityInterface::getGcwRegionDefenderCities();
	for (std::map<std::pair<std::string, int>, uint32>::const_iterator iterCity = gcwRegionDefenderCities.lower_bound(std::make_pair(gcwCategoryString, 0)); ((iterCity != gcwRegionDefenderCities.end()) && (iterCity->first.first == gcwCategoryString)); ++iterCity)
	{
		if (PvpData::isRebelFactionId(iterCity->second))
			cityIds.push_back(iterCity->first.second);
	}

	if (cityIds.empty())
		return 0;

	LocalIntArrayRefPtr ret = createNewIntArray(cityIds.size());
	setIntArrayRegion(*ret, 0, cityIds.size(), reinterpret_cast<jint *>(&cityIds[0]));
	return ret->getReturnValue();
}

//-----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionGuildsImperial(JNIEnv *env, jobject self, jstring gcwCategory)
{
	JavaStringParam localGcwCategory(gcwCategory);
	std::string gcwCategoryString;
	if (!JavaLibrary::convert(localGcwCategory, gcwCategoryString))
		return 0;

	std::vector<int> guildIds;
	std::map<std::pair<std::string, int>, uint32> const & gcwRegionDefenderGuilds = GuildInterface::getGcwRegionDefenderGuilds();
	for (std::map<std::pair<std::string, int>, uint32>::const_iterator iterGuild = gcwRegionDefenderGuilds.lower_bound(std::make_pair(gcwCategoryString, 0)); ((iterGuild != gcwRegionDefenderGuilds.end()) && (iterGuild->first.first == gcwCategoryString)); ++iterGuild)
	{
		if (PvpData::isImperialFactionId(iterGuild->second))
			guildIds.push_back(iterGuild->first.second);
	}

	if (guildIds.empty())
		return 0;

	LocalIntArrayRefPtr ret = createNewIntArray(guildIds.size());
	setIntArrayRegion(*ret, 0, guildIds.size(), reinterpret_cast<jint *>(&guildIds[0]));
	return ret->getReturnValue();
}

//-----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionGuildsRebel(JNIEnv *env, jobject self, jstring gcwCategory)
{
	JavaStringParam localGcwCategory(gcwCategory);
	std::string gcwCategoryString;
	if (!JavaLibrary::convert(localGcwCategory, gcwCategoryString))
		return 0;

	std::vector<int> guildIds;
	std::map<std::pair<std::string, int>, uint32> const & gcwRegionDefenderGuilds = GuildInterface::getGcwRegionDefenderGuilds();
	for (std::map<std::pair<std::string, int>, uint32>::const_iterator iterGuild = gcwRegionDefenderGuilds.lower_bound(std::make_pair(gcwCategoryString, 0)); ((iterGuild != gcwRegionDefenderGuilds.end()) && (iterGuild->first.first == gcwCategoryString)); ++iterGuild)
	{
		if (PvpData::isRebelFactionId(iterGuild->second))
			guildIds.push_back(iterGuild->first.second);
	}

	if (guildIds.empty())
		return 0;

	LocalIntArrayRefPtr ret = createNewIntArray(guildIds.size());
	setIntArrayRegion(*ret, 0, guildIds.size(), reinterpret_cast<jint *>(&guildIds[0]));
	return ret->getReturnValue();
}

//-----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionImperialBonus(JNIEnv *env, jobject self, jstring gcwCategory)
{
	JavaStringParam localGcwCategory(gcwCategory);
	std::string gcwCategoryString;
	if (!JavaLibrary::convert(localGcwCategory, gcwCategoryString))
		return 0.0f;

	return Pvp::getGcwDefenderRegionImperialBonus(gcwCategoryString);
}

//-----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsPvpNamespace::getGcwDefenderRegionRebelBonus(JNIEnv *env, jobject self, jstring gcwCategory)
{
	JavaStringParam localGcwCategory(gcwCategory);
	std::string gcwCategoryString;
	if (!JavaLibrary::convert(localGcwCategory, gcwCategoryString))
		return 0.0f;

	return Pvp::getGcwDefenderRegionRebelBonus(gcwCategoryString);
}

// ======================================================================
