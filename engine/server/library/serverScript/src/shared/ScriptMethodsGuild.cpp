// ======================================================================
//
// ScriptMethodsGuild.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/GuildMemberInfo.h"
#include "serverGame/GuildObject.h"
#include "serverGame/ServerUniverse.h"
#include "sharedGame/GuildRankDataTable.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsGuildNamespace
// ======================================================================

namespace ScriptMethodsGuildNamespace
{
	bool install();

	jint         JNICALL getGuildId(JNIEnv *env, jobject self, jlong target);
	jintArray    JNICALL getAllGuildIds(JNIEnv *env, jobject self);
	jlong        JNICALL getMasterGuildObject(JNIEnv *env, jobject self);
	jintArray    JNICALL getGuildsAtWarWith(JNIEnv *env, jobject self, jint guildId);
	jobject      JNICALL getMasterGuildWarTableDictionary(JNIEnv * env, jobject self);
	jobject      JNICALL getInactiveGuildWarTableDictionary(JNIEnv * env, jobject self);
	jint         JNICALL findGuild(JNIEnv *env, jobject self, jstring name);
	jint         JNICALL createGuild(JNIEnv *env, jobject self, jstring name, jstring abbrev);
	void         JNICALL disbandGuild(JNIEnv *env, jobject self, jint guildId);
	jboolean     JNICALL guildExists(JNIEnv *env, jobject self, jint guildId);
	jstring      JNICALL guildGetName(JNIEnv *env, jobject self, jint guildId);
	jstring      JNICALL guildGetAbbrev(JNIEnv *env, jobject self, jint guildId);
	jlong        JNICALL guildGetLeader(JNIEnv *env, jobject self, jint guildId);
	jint         JNICALL guildGetElectionPreviousEndTime(JNIEnv *env, jobject self, jint guildId);
	jint         JNICALL guildGetElectionNextEndTime(JNIEnv *env, jobject self, jint guildId);
	jint         JNICALL guildGetCurrentFaction(JNIEnv *env, jobject self, jint guildId);
	jint         JNICALL guildGetPreviousFaction(JNIEnv *env, jobject self, jint guildId);
	jint         JNICALL guildGetTimeLeftPreviousFaction(JNIEnv *env, jobject self, jint guildId);
	jstring      JNICALL guildGetCurrentGcwDefenderRegion(JNIEnv *env, jobject self, jint guildId);
	jint         JNICALL guildGetTimeJoinedCurrentGcwDefenderRegion(JNIEnv *env, jobject self, jint guildId);
	jstring      JNICALL guildGetPreviousGcwDefenderRegion(JNIEnv *env, jobject self, jint guildId);
	jint         JNICALL guildGetTimeLeftPreviousGcwDefenderRegion(JNIEnv *env, jobject self, jint guildId);
	jlongArray   JNICALL guildGetMemberIds(JNIEnv *env, jobject self, jint guildId);
	jlongArray   JNICALL guildGetMemberIdsWithPermissions(JNIEnv *env, jobject self, jint guildId, jint permissions);
	jint         JNICALL guildGetCountMembersOnly(JNIEnv *env, jobject self, jint guildId);
	jint         JNICALL guildGetCountSponsoredOnly(JNIEnv *env, jobject self, jint guildId);
	jint         JNICALL guildGetCountMembersAndSponsored(JNIEnv *env, jobject self, jint guildId);
	jint         JNICALL guildGetCountMembersGuildWarPvPEnabled(JNIEnv *env, jobject self, jint guildId);
	jstring      JNICALL guildGetMemberName(JNIEnv *env, jobject self, jint guildId, jlong member);
	jstring      JNICALL guildGetMemberProfession(JNIEnv *env, jobject self, jint guildId, jlong member);
	jint         JNICALL guildGetMemberLevel(JNIEnv *env, jobject self, jint guildId, jlong member);
	jint         JNICALL guildGetMemberPermissions(JNIEnv *env, jobject self, jint guildId, jlong member);
	jstring      JNICALL guildGetMemberTitle(JNIEnv *env, jobject self, jint guildId, jlong member);
	jlong        JNICALL guildGetMemberAllegiance(JNIEnv *env, jobject self, jint guildId, jlong member);
	jintArray    JNICALL guildGetEnemies(JNIEnv *env, jobject self, jint guildId);
	void         JNICALL guildRemoveMember(JNIEnv *env, jobject self, jint guildId, jlong member);
	void         JNICALL guildAddCreatorMember(JNIEnv *env, jobject self, jint guildId, jlong member);
	void         JNICALL guildAddSponsorMember(JNIEnv *env, jobject self, jint guildId, jlong member);
	void         JNICALL guildSetMemberPermission(JNIEnv *env, jobject self, jint guildId, jlong member, jint permissions);
	void         JNICALL guildSetMemberTitle(JNIEnv *env, jobject self, jint guildId, jlong member, jstring title);
	void         JNICALL guildSetMemberAllegiance(JNIEnv *env, jobject self, jint guildId, jlong member, jlong allegiance);
	void         JNICALL guildSetMemberPermissionAndAllegiance(JNIEnv *env, jobject self, jint guildId, jlong member, jint permissions, jlong allegiance);
	jobjectArray JNICALL guildGetAllRanks(JNIEnv *env, jobject self);
	jobjectArray JNICALL guildGetTitleForRank(JNIEnv *env, jobject self, jstring rank);
	jobjectArray JNICALL guildGetMemberRank(JNIEnv *env, jobject self, jint guildId, jlong member);
	jboolean     JNICALL guildHasMemberRank(JNIEnv *env, jobject self, jint guildId, jlong member, jstring rank);
	void         JNICALL guildAddMemberRank(JNIEnv *env, jobject self, jint guildId, jlong member, jstring rank);
	void         JNICALL guildRemoveMemberRank(JNIEnv *env, jobject self, jint guildId, jlong member, jstring rank);
	void         JNICALL guildSetLeader(JNIEnv *env, jobject self, jint guildId, jlong leader);
	void         JNICALL guildSetElectionEndTime(JNIEnv *env, jobject self, jint guildId, jint electionPreviousEndTime, jint electionNextEndTime);
	void         JNICALL guildSetFaction(JNIEnv *env, jobject self, jint guildId, jint factionId);
	void         JNICALL guildSetGcwDefenderRegion(JNIEnv *env, jobject self, jint guildId, jstring gcwDefenderRegion);
	void         JNICALL guildRemoveEnemy(JNIEnv *env, jobject self, jint guildId, jint enemyId);
	void         JNICALL guildSetEnemy(JNIEnv *env, jobject self, jint guildId, jint enemyId);
	void         JNICALL guildUpdateGuildWarKillTracking(JNIEnv *env, jobject self, jlong killer, jlong victim);
	jint         JNICALL guildGetGuildWarKillCount(JNIEnv *env, jobject self, jint guildIdA, jint guildIdB);
	jint         JNICALL guildGetGuildWarKillCountUpdateTime(JNIEnv *env, jobject self, jint guildIdA, jint guildIdB);
	void         JNICALL guildSetName(JNIEnv *env, jobject self, jint guildId, jstring name);
	void         JNICALL guildSetAbbrev(JNIEnv *env, jobject self, jint guildId, jstring abbrev);
}

//========================================================================
// install
//========================================================================

bool ScriptMethodsGuildNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsGuildNamespace::c)}
	JF("_getGuildId", "(J)I", getGuildId),
	JF("getAllGuildIds", "()[I", getAllGuildIds),
	JF("_getMasterGuildObject", "()J", getMasterGuildObject),
	JF("findGuild", "(Ljava/lang/String;)I", findGuild),
	JF("createGuild", "(Ljava/lang/String;Ljava/lang/String;)I", createGuild),
	JF("disbandGuild", "(I)V", disbandGuild),
	JF("guildExists", "(I)Z", guildExists),
	JF("guildGetName", "(I)Ljava/lang/String;", guildGetName),
	JF("guildGetAbbrev", "(I)Ljava/lang/String;", guildGetAbbrev),
	JF("_guildGetLeader", "(I)J", guildGetLeader),
	JF("guildGetElectionPreviousEndTime", "(I)I", guildGetElectionPreviousEndTime),
	JF("guildGetElectionNextEndTime", "(I)I", guildGetElectionNextEndTime),
	JF("guildGetCurrentFaction", "(I)I", guildGetCurrentFaction),
	JF("guildGetPreviousFaction", "(I)I", guildGetPreviousFaction),
	JF("guildGetTimeLeftPreviousFaction", "(I)I", guildGetTimeLeftPreviousFaction),
	JF("guildGetCurrentGcwDefenderRegion", "(I)Ljava/lang/String;", guildGetCurrentGcwDefenderRegion),
	JF("guildGetTimeJoinedCurrentGcwDefenderRegion", "(I)I", guildGetTimeJoinedCurrentGcwDefenderRegion),
	JF("guildGetPreviousGcwDefenderRegion", "(I)Ljava/lang/String;", guildGetPreviousGcwDefenderRegion),
	JF("guildGetTimeLeftPreviousGcwDefenderRegion", "(I)I", guildGetTimeLeftPreviousGcwDefenderRegion),
	JF("_guildGetMemberIds", "(I)[J", guildGetMemberIds),
	JF("_guildGetMemberIdsWithPermissions", "(II)[J", guildGetMemberIdsWithPermissions),
	JF("guildGetCountMembersOnly", "(I)I", guildGetCountMembersOnly),
	JF("guildGetCountSponsoredOnly", "(I)I", guildGetCountSponsoredOnly),
	JF("guildGetCountMembersAndSponsored", "(I)I", guildGetCountMembersAndSponsored),
	JF("guildGetCountMembersGuildWarPvPEnabled", "(I)I", guildGetCountMembersGuildWarPvPEnabled),
	JF("_guildGetMemberName", "(IJ)Ljava/lang/String;", guildGetMemberName),
	JF("_guildGetMemberProfession", "(IJ)Ljava/lang/String;", guildGetMemberProfession),
	JF("_guildGetMemberLevel", "(IJ)I", guildGetMemberLevel),
	JF("_guildGetMemberPermissions", "(IJ)I", guildGetMemberPermissions),
	JF("_guildGetMemberTitle", "(IJ)Ljava/lang/String;", guildGetMemberTitle),
	JF("_guildGetMemberAllegiance", "(IJ)J", guildGetMemberAllegiance),
	JF("guildGetEnemies", "(I)[I", guildGetEnemies),
	JF("getGuildsAtWarWith", "(I)[I", getGuildsAtWarWith),
	JF("getMasterGuildWarTableDictionary", "()Lscript/dictionary;", getMasterGuildWarTableDictionary),
	JF("getInactiveGuildWarTableDictionary", "()Lscript/dictionary;", getInactiveGuildWarTableDictionary),
	JF("_guildRemoveMember", "(IJ)V", guildRemoveMember),
	JF("_guildAddCreatorMember", "(IJ)V", guildAddCreatorMember),
	JF("_guildAddSponsorMember", "(IJ)V", guildAddSponsorMember),
	JF("_guildSetMemberPermission", "(IJI)V", guildSetMemberPermission),
	JF("_guildSetMemberTitle", "(IJLjava/lang/String;)V", guildSetMemberTitle),
	JF("_guildSetMemberAllegiance", "(IJJ)V", guildSetMemberAllegiance),
	JF("_guildSetMemberPermissionAndAllegiance", "(IJIJ)V", guildSetMemberPermissionAndAllegiance),
	JF("guildGetAllRanks", "()[Ljava/lang/String;", guildGetAllRanks),
	JF("guildGetTitleForRank", "(Ljava/lang/String;)[Ljava/lang/String;", guildGetTitleForRank),
	JF("_guildGetMemberRank", "(IJ)[Ljava/lang/String;", guildGetMemberRank),
	JF("_guildHasMemberRank", "(IJLjava/lang/String;)Z", guildHasMemberRank),
	JF("_guildAddMemberRank", "(IJLjava/lang/String;)V", guildAddMemberRank),
	JF("_guildRemoveMemberRank", "(IJLjava/lang/String;)V", guildRemoveMemberRank),
	JF("_guildSetLeader", "(IJ)V", guildSetLeader),
	JF("guildSetElectionEndTime", "(III)V", guildSetElectionEndTime),
	JF("guildSetFaction", "(II)V", guildSetFaction),
	JF("guildSetGcwDefenderRegion", "(ILjava/lang/String;)V", guildSetGcwDefenderRegion),
	JF("guildRemoveEnemy", "(II)V", guildRemoveEnemy),
	JF("guildSetEnemy", "(II)V", guildSetEnemy),
	JF("_guildUpdateGuildWarKillTracking", "(JJ)V", guildUpdateGuildWarKillTracking),
	JF("guildGetGuildWarKillCount", "(II)I", guildGetGuildWarKillCount),
	JF("guildGetGuildWarKillCountUpdateTime", "(II)I", guildGetGuildWarKillCountUpdateTime),
	JF("guildSetName", "(ILjava/lang/String;)V", guildSetName),
	JF("guildSetAbbrev", "(ILjava/lang/String;)V", guildSetAbbrev),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jint JNICALL ScriptMethodsGuildNamespace::getGuildId(JNIEnv *env, jobject self, jlong target)
{
	CreatureObject *targetObj = 0;
	if (!JavaLibrary::getObject(target, targetObj))
	{
		DEBUG_WARNING(true, ("JavaLibrary::getGuildId: called with invalid target"));
		return 0;
	}
	return targetObj->getGuildId();
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsGuildNamespace::getAllGuildIds(JNIEnv *env, jobject self)
{
	std::vector<int> guildIds;
	GuildInterface::getAllGuildIds(guildIds);
	LocalIntArrayRefPtr ret = createNewIntArray(guildIds.size());
	if (!guildIds.empty())
		setIntArrayRegion(*ret, 0, guildIds.size(), reinterpret_cast<jint *>(&guildIds[0]));
	return ret->getReturnValue();
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsGuildNamespace::getMasterGuildObject(JNIEnv *env, jobject self)
{
	GuildObject *masterGuildObject = ServerUniverse::getInstance().getMasterGuildObject();
	return masterGuildObject ? (masterGuildObject->getNetworkId()).getValue() : (NetworkId::cms_invalid).getValue();
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsGuildNamespace::getGuildsAtWarWith(JNIEnv *env, jobject self, jint guildId)
{
	std::vector<int> const &enemies = GuildInterface::getGuildsAtWarWith(guildId);
	LocalIntArrayRefPtr ret = createNewIntArray(enemies.size());
	if (!enemies.empty())
		setIntArrayRegion(*ret, 0, enemies.size(), const_cast<jint *>(reinterpret_cast<jint const *>(&enemies[0])));
	return ret->getReturnValue();
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsGuildNamespace::getMasterGuildWarTableDictionary(JNIEnv * env, jobject self)
{
	ScriptParams const * scriptParams = GuildInterface::getMasterGuildWarTableDictionary();
	if (!scriptParams)
		return 0;

	JavaDictionaryPtr dictionary;
	JavaLibrary::instance()->convert(*scriptParams, dictionary);
	return dictionary->getReturnValue();
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsGuildNamespace::getInactiveGuildWarTableDictionary(JNIEnv * env, jobject self)
{
	ScriptParams const * scriptParams = GuildInterface::getInactiveGuildWarTableDictionary();
	if (!scriptParams)
		return 0;

	JavaDictionaryPtr dictionary;
	JavaLibrary::instance()->convert(*scriptParams, dictionary);
	return dictionary->getReturnValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::findGuild(JNIEnv *env, jobject self, jstring name)
{
	JavaStringParam localNameStr(name);
	std::string nameStr;
	if (!JavaLibrary::convert(localNameStr, nameStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::findGuild: invalid name"));
		return 0;
	}
	return GuildInterface::findGuild(nameStr);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::createGuild(JNIEnv *env, jobject self, jstring name, jstring abbrev)
{
	JavaStringParam localNameStr(name);
	std::string nameStr;
	if (!JavaLibrary::convert(localNameStr, nameStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::createGuild: invalid name"));
		return 0;
	}

	JavaStringParam localAbbrevStr(abbrev);
	std::string abbrevStr;
	if (!JavaLibrary::convert(localAbbrevStr, abbrevStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::createGuild: invalid abbrev"));
		return 0;
	}

	return GuildInterface::createGuild(nameStr, abbrevStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::disbandGuild(JNIEnv *env, jobject self, jint guildId)
{
	GuildInterface::disbandGuild(guildId);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsGuildNamespace::guildExists(JNIEnv *env, jobject self, jint guildId)
{
	return GuildInterface::guildExists(guildId);
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsGuildNamespace::guildGetName(JNIEnv *env, jobject self, jint guildId)
{
	JavaString str(GuildInterface::getGuildName(guildId).c_str());
	return str.getReturnValue();
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsGuildNamespace::guildGetAbbrev(JNIEnv *env, jobject self, jint guildId)
{
	JavaString str(GuildInterface::getGuildAbbrev(guildId).c_str());
	return str.getReturnValue();
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsGuildNamespace::guildGetLeader(JNIEnv *env, jobject self, jint guildId)
{
	return (GuildInterface::getGuildLeaderId(guildId)).getValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetElectionPreviousEndTime(JNIEnv *env, jobject self, jint guildId)
{
	return GuildInterface::getGuildElectionPreviousEndTime(guildId);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetElectionNextEndTime(JNIEnv *env, jobject self, jint guildId)
{
	return GuildInterface::getGuildElectionNextEndTime(guildId);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetCurrentFaction(JNIEnv *env, jobject self, jint guildId)
{
	return static_cast<jint>(GuildInterface::getGuildCurrentFaction(guildId));
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetPreviousFaction(JNIEnv *env, jobject self, jint guildId)
{
	return static_cast<jint>(GuildInterface::getGuildPreviousFaction(guildId));
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetTimeLeftPreviousFaction(JNIEnv *env, jobject self, jint guildId)
{
	return GuildInterface::getTimeLeftGuildPreviousFaction(guildId);
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsGuildNamespace::guildGetCurrentGcwDefenderRegion(JNIEnv *env, jobject self, jint guildId)
{
	JavaString str(GuildInterface::getGuildCurrentGcwDefenderRegion(guildId).c_str());
	return str.getReturnValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetTimeJoinedCurrentGcwDefenderRegion(JNIEnv *env, jobject self, jint guildId)
{
	return GuildInterface::getTimeJoinedGuildCurrentGcwDefenderRegion(guildId);
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsGuildNamespace::guildGetPreviousGcwDefenderRegion(JNIEnv *env, jobject self, jint guildId)
{
	JavaString str(GuildInterface::getGuildPreviousGcwDefenderRegion(guildId).c_str());
	return str.getReturnValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetTimeLeftPreviousGcwDefenderRegion(JNIEnv *env, jobject self, jint guildId)
{
	return GuildInterface::getTimeLeftGuildPreviousGcwDefenderRegion(guildId);
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsGuildNamespace::guildGetMemberIds(JNIEnv *env, jobject self, jint guildId)
{
	std::vector<NetworkId> results;
	GuildInterface::getGuildMemberIds(guildId, results);
	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsGuildNamespace::guildGetMemberIdsWithPermissions(JNIEnv *env, jobject self, jint guildId, jint permissions)
{
	std::vector<NetworkId> results;
	GuildInterface::getGuildMemberIdsWithPermissions(guildId, permissions, results);
	if (results.empty())
		return 0;

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();

	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetCountMembersOnly(JNIEnv *env, jobject self, jint guildId)
{
	return GuildInterface::getGuildCountMembersOnly(guildId);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetCountSponsoredOnly(JNIEnv *env, jobject self, jint guildId)
{
	return GuildInterface::getGuildCountSponsoredOnly(guildId);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetCountMembersAndSponsored(JNIEnv *env, jobject self, jint guildId)
{
	return GuildInterface::getGuildCountMembersAndSponsored(guildId);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetCountMembersGuildWarPvPEnabled(JNIEnv *env, jobject self, jint guildId)
{
	return GuildInterface::getGuildCountMembersGuildWarPvPEnabled(guildId);
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsGuildNamespace::guildGetMemberName(JNIEnv *env, jobject self, jint guildId, jlong member)
{
	NetworkId memberId(member);
	GuildMemberInfo const *memberInfo = GuildInterface::getGuildMemberInfo(guildId, memberId);
	if (memberInfo)
	{
		JavaString str(memberInfo->m_name.c_str());
		return str.getReturnValue();
	}
	DEBUG_WARNING(true, ("JavaLibrary::guildGetMemberName called with invalid info (guildId=%d memberId=%s)", guildId, memberId.getValueString().c_str()));
	return 0;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsGuildNamespace::guildGetMemberProfession(JNIEnv *env, jobject self, jint guildId, jlong member)
{
	NetworkId memberId(member);
	GuildMemberInfo const *memberInfo = GuildInterface::getGuildMemberInfo(guildId, memberId);
	if (memberInfo)
	{
		JavaString str(memberInfo->m_professionSkillTemplate.c_str());
		return str.getReturnValue();
	}
	DEBUG_WARNING(true, ("JavaLibrary::guildGetMemberProfession called with invalid info (guildId=%d memberId=%s)", guildId, memberId.getValueString().c_str()));
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetMemberLevel(JNIEnv *env, jobject self, jint guildId, jlong member)
{
	NetworkId memberId(member);
	GuildMemberInfo const *memberInfo = GuildInterface::getGuildMemberInfo(guildId, memberId);
	if (memberInfo)
		return memberInfo->m_level;
	DEBUG_WARNING(true, ("JavaLibrary::guildGetMemberLevel called with invalid info (guildId=%d memberId=%s)", guildId, memberId.getValueString().c_str()));
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetMemberPermissions(JNIEnv *env, jobject self, jint guildId, jlong member)
{
	NetworkId memberId(member);
	GuildMemberInfo const *memberInfo = GuildInterface::getGuildMemberInfo(guildId, memberId);
	if (memberInfo)
		return memberInfo->m_permissions;
	DEBUG_WARNING(true, ("JavaLibrary::guildGetMemberPermissions called with invalid info (guildId=%d memberId=%s)", guildId, memberId.getValueString().c_str()));
	return 0;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsGuildNamespace::guildGetMemberTitle(JNIEnv *env, jobject self, jint guildId, jlong member)
{
	NetworkId memberId(member);
	GuildMemberInfo const *memberInfo = GuildInterface::getGuildMemberInfo(guildId, memberId);
	if (memberInfo)
	{
		JavaString str(memberInfo->m_title.c_str());
		return str.getReturnValue();
	}
	DEBUG_WARNING(true, ("JavaLibrary::guildGetMemberTitle called with invalid info (guildId=%d memberId=%s)", guildId, memberId.getValueString().c_str()));
	return 0;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsGuildNamespace::guildGetMemberAllegiance(JNIEnv *env, jobject self, jint guildId, jlong member)
{
	NetworkId memberId(member);
	GuildMemberInfo const *memberInfo = GuildInterface::getGuildMemberInfo(guildId, memberId);
	if (memberInfo)
		return (memberInfo->m_allegiance).getValue();
	DEBUG_WARNING(true, ("JavaLibrary::guildGetMemberAllegiance called with invalid info (guildId=%d memberId=%s)", guildId, memberId.getValueString().c_str()));
	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildRemoveMember(JNIEnv *env, jobject self, jint guildId, jlong member)
{
	NetworkId memberId(member);
	GuildInterface::removeGuildMember(guildId, memberId);
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsGuildNamespace::guildGetEnemies(JNIEnv *env, jobject self, jint guildId)
{
	std::vector<int> const &enemies = GuildInterface::getGuildEnemies(guildId);
	LocalIntArrayRefPtr ret = createNewIntArray(enemies.size());
	if (!enemies.empty())
		setIntArrayRegion(*ret, 0, enemies.size(), const_cast<jint *>(reinterpret_cast<jint const *>(&enemies[0])));
	return ret->getReturnValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetLeader(JNIEnv *env, jobject self, jint guildId, jlong leader)
{
	NetworkId leaderId(leader);
	GuildInterface::setGuildLeader(guildId, leaderId);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetElectionEndTime(JNIEnv *env, jobject self, jint guildId, jint electionPreviousEndTime, jint electionNextEndTime)
{
	GuildInterface::setGuildElectionEndTime(guildId, electionPreviousEndTime, electionNextEndTime);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetFaction(JNIEnv *env, jobject self, jint guildId, jint factionId)
{
	GuildInterface::setGuildFaction(guildId, static_cast<uint32>(factionId));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetGcwDefenderRegion(JNIEnv *env, jobject self, jint guildId, jstring gcwDefenderRegion)
{
	JavaStringParam localGcwDefenderRegionStr(gcwDefenderRegion);
	std::string gcwDefenderRegionStr;
	if (JavaLibrary::convert(localGcwDefenderRegionStr, gcwDefenderRegionStr))
	{
		if (!gcwDefenderRegionStr.empty())
		{
			Pvp::GcwScoreCategory const * const gcwCategory = Pvp::getGcwScoreCategory(gcwDefenderRegionStr);
			if (!gcwCategory)
			{
				DEBUG_WARNING(true, ("JavaLibrary::guildSetGcwDefenderRegion: invalid GCW score category (%s)", gcwDefenderRegionStr.c_str()));
				return;
			}

			if (!gcwCategory->gcwRegionDefender)
			{
				DEBUG_WARNING(true, ("JavaLibrary::guildSetGcwDefenderRegion: GCW score category (%s) is not flagged as GcwRegionDefender", gcwDefenderRegionStr.c_str()));
				return;
			}
		}
	}
	else
	{
		gcwDefenderRegionStr.clear();
	}

	GuildInterface::setGuildGcwDefenderRegion(guildId, gcwDefenderRegionStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildAddCreatorMember(JNIEnv *env, jobject self, jint guildId, jlong member)
{
	NetworkId memberId(member);
	GuildInterface::addGuildCreatorMember(guildId, memberId);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildAddSponsorMember(JNIEnv *env, jobject self, jint guildId, jlong member)
{
	NetworkId memberId(member);
	GuildInterface::addGuildSponsorMember(guildId, memberId);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetMemberPermission(JNIEnv *env, jobject self, jint guildId, jlong member, jint permissions)
{
	NetworkId memberId(member);
	GuildInterface::setGuildMemberPermission(guildId, memberId, permissions);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetMemberTitle(JNIEnv *env, jobject self, jint guildId, jlong member, jstring title)
{
	NetworkId memberId(member);

	JavaStringParam localTitleStr(title);
	std::string titleStr;
	if (!JavaLibrary::convert(localTitleStr, titleStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::guildSetMemberTitle: invalid title"));
		return;
	}

	GuildInterface::setGuildMemberTitle(guildId, memberId, titleStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetMemberAllegiance(JNIEnv *env, jobject self, jint guildId, jlong member, jlong allegiance)
{
	NetworkId memberId(member);
	NetworkId allegianceId(allegiance);
	GuildInterface::setGuildMemberAllegiance(guildId, memberId, allegianceId);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetMemberPermissionAndAllegiance(JNIEnv *env, jobject self, jint guildId, jlong member, jint permissions, jlong allegiance)
{
	NetworkId memberId(member);
	NetworkId allegianceId(allegiance);
	GuildInterface::setGuildMemberPermissionAndAllegiance(guildId, memberId, permissions, allegianceId);
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsGuildNamespace::guildGetAllRanks(JNIEnv *env, jobject self)
{
	std::vector<GuildRankDataTable::GuildRank const *> const & ranks = GuildRankDataTable::getAllRanks();
	if (ranks.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(ranks.size(), JavaLibrary::getClsString());
	for (std::vector<GuildRankDataTable::GuildRank const *>::const_iterator iter = ranks.begin(); iter != ranks.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsGuildNamespace::guildGetTitleForRank(JNIEnv *env, jobject self, jstring rank)
{
	JavaStringParam localRankStr(rank);
	std::string rankStr;
	if (!JavaLibrary::convert(localRankStr, rankStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::guildGetTitleForRank: invalid rank"));
		return 0;
	}

	GuildRankDataTable::GuildRank const * rankInfo = GuildRankDataTable::getRank(rankStr);
	if (!rankInfo || rankInfo->titles.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(rankInfo->titles.size(), JavaLibrary::getClsString());
	for (std::vector<std::string>::const_iterator iter = rankInfo->titles.begin(); iter != rankInfo->titles.end(); ++iter)
	{
		JavaString jval(*iter);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsGuildNamespace::guildGetMemberRank(JNIEnv *env, jobject self, jint guildId, jlong member)
{
	std::vector<std::string> ranks;
	GuildInterface::getGuildMemberRank(guildId, NetworkId(static_cast<NetworkId::NetworkIdType>(member)), ranks);
	if (ranks.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(ranks.size(), JavaLibrary::getClsString());
	for (std::vector<std::string>::const_iterator iter = ranks.begin(); iter != ranks.end(); ++iter)
	{
		JavaString jval(*iter);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsGuildNamespace::guildHasMemberRank(JNIEnv *env, jobject self, jint guildId, jlong member, jstring rank)
{
	NetworkId memberId(member);

	JavaStringParam localRankStr(rank);
	std::string rankStr;
	if (!JavaLibrary::convert(localRankStr, rankStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::guildHasMemberRank: invalid rank"));
		return JNI_FALSE;
	}

	return GuildInterface::hasGuildMemberRank(guildId, memberId, rankStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildAddMemberRank(JNIEnv *env, jobject self, jint guildId, jlong member, jstring rank)
{
	NetworkId memberId(member);

	JavaStringParam localRankStr(rank);
	std::string rankStr;
	if (!JavaLibrary::convert(localRankStr, rankStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::guildAddMemberRank: invalid rank"));
		return;
	}

	GuildInterface::addGuildMemberRank(guildId, memberId, rankStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildRemoveMemberRank(JNIEnv *env, jobject self, jint guildId, jlong member, jstring rank)
{
	NetworkId memberId(member);

	JavaStringParam localRankStr(rank);
	std::string rankStr;
	if (!JavaLibrary::convert(localRankStr, rankStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::guildRemoveMemberRank: invalid rank"));
		return;
	}

	GuildInterface::removeGuildMemberRank(guildId, memberId, rankStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildRemoveEnemy(JNIEnv *env, jobject self, jint guildId, jint enemyId)
{
	GuildInterface::removeGuildEnemy(guildId, enemyId);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetEnemy(JNIEnv *env, jobject self, jint guildId, jint enemyId)
{
	GuildInterface::setGuildEnemy(guildId, enemyId);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildUpdateGuildWarKillTracking(JNIEnv *env, jobject self, jlong killer, jlong victim)
{
	CreatureObject const *killerObj = 0;
	if (!JavaLibrary::getObject(killer, killerObj))
		return;

	CreatureObject const *victimObj = 0;
	if (!JavaLibrary::getObject(victim, victimObj))
		return;

	if (killerObj && victimObj)
		GuildInterface::updateGuildWarKillTracking(*killerObj, *victimObj);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetGuildWarKillCount(JNIEnv *env, jobject self, jint guildIdA, jint guildIdB)
{
	return GuildInterface::getGuildEnemyKillInfo(guildIdA, guildIdB).first;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsGuildNamespace::guildGetGuildWarKillCountUpdateTime(JNIEnv *env, jobject self, jint guildIdA, jint guildIdB)
{
	return GuildInterface::getGuildEnemyKillInfo(guildIdA, guildIdB).second;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetName(JNIEnv *env, jobject self, jint guildId, jstring name)
{
	JavaStringParam localNewNameStr(name);
	std::string newNameStr;
	if (!JavaLibrary::convert(localNewNameStr, newNameStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::guildSetName: invalid name"));
		return;
	}
	GuildInterface::setGuildName(guildId, newNameStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsGuildNamespace::guildSetAbbrev(JNIEnv *env, jobject self, jint guildId, jstring abbrev)
{
	JavaStringParam localNewAbbrevStr(abbrev);
	std::string newAbbrevStr;
	if (!JavaLibrary::convert(localNewAbbrevStr, newAbbrevStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::guildSetAbbrev: invalid abbrev"));
		return;
	}
	GuildInterface::setGuildAbbrev(guildId, newAbbrevStr);
}

// ======================================================================


