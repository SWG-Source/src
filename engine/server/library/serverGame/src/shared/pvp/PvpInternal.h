// ======================================================================
//
// PvpInternal.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpInternal_H
#define _PvpInternal_H

// ======================================================================

#include "Pvp.h"

// ======================================================================

class PvpRuleSetBase;
class Region;
class RegionPvp;
class MessageQueuePvpCommand;
struct PvpEnemy;

// ======================================================================

class PvpInternal
{
public:
	static PvpRuleSetBase & getRuleSet          (TangibleObject const &obj1, TangibleObject const &obj2);
	static Region const *   getPvpRegion        (TangibleObject const &obj);
	static TangibleObject const *getPetMaster   (TangibleObject const &who);
	static TangibleObject *getPetMaster         (TangibleObject &who);
	static bool      inSameGroup                (TangibleObject const &obj1, TangibleObject const &obj2);
	static void      getGroupMembers            (TangibleObject const &src, std::vector<TangibleObject const *> &members);
	static Pvp::FactionId getAlignedFaction     (TangibleObject const &who);
	static Pvp::FactionId getAlignedFaction     (NetworkId const &who);
	static bool      hasAnyEnemyFlag            (TangibleObject const &who);
	static bool      hasAnyTimedEnemyFlag       (TangibleObject const &who);
	static bool      hasAnyAlignedTimedEnemyFlag(TangibleObject const &who);
	static bool      hasAnyAlignedEnemyFlag     (TangibleObject const &who);
	static bool      hasAnyFactionEnemyFlag     (TangibleObject const &who);
	static bool      hasAnyBountyDuelEnemyFlag  (TangibleObject const &who);
	static bool      hasAnyGuildWarCoolDownPeriodEnemyFlag(TangibleObject const &who);
	static bool      hasFactionEnemyFlag        (TangibleObject const &who, Pvp::FactionId enemyAlign);
	static bool      hasDuelEnemyFlag           (TangibleObject const &who, NetworkId const &enemyId);
	static bool      hasPersonalEnemyFlag       (TangibleObject const &who, NetworkId const &enemyId);
	static bool		 hasBubbleCombatFlag		(TangibleObject const &who);
	static bool		 hasBubbleCombatFlagAgainstTarget (TangibleObject const &who, NetworkId const &enemyId);
	static void      getPersonalEnemyIds        (TangibleObject const &who, std::vector<NetworkId> &enemyIds);
	static Pvp::FactionId battlefieldGetFaction (TangibleObject const &who, RegionPvp const &region);
	static bool      battlefieldIsParticipant   (TangibleObject const &who, RegionPvp const &region);
	static void      battlefieldSetParticipant  (TangibleObject const &who, RegionPvp const &region, Pvp::FactionId factionId);
	static bool      isPet                      (TangibleObject const &who);
	static bool      isPlayer                   (TangibleObject const &who);
	static bool      isNeutral                  (TangibleObject const &who);
	static bool      isCovert                   (TangibleObject const &who);
	static bool      isDeclared                 (TangibleObject const &who);
	static bool      isPreparedToBeNeutral      (TangibleObject const &who);
	static bool      isPreparedToBeCovert       (TangibleObject const &who);
	static bool      isPreparedToBeDeclared     (TangibleObject const &who);
	static void      buildPvpMessageSetFactionEnemyFlag(MessageQueuePvpCommand &messageQueuePvpCommand, Pvp::FactionId enemyAlign, int expireTimeMs = -1);
	static void      setFactionEnemyFlag        (TangibleObject &dest, Pvp::FactionId enemyAlign, int expireTimeMs = -1);
	static void      buildPvpMessageSetPersonalEnemyFlag(MessageQueuePvpCommand &messageQueuePvpCommand, NetworkId const &enemyId, Pvp::FactionId enemyAlign, int expireTimeMs = -1);
	static void      setPersonalEnemyFlag       (TangibleObject &dest, NetworkId const &enemyId, Pvp::FactionId enemyAlign, int expireTimeMs = -1);
	static void      setPersonalEnemyFlag       (TangibleObject &dest, TangibleObject const &src);
	static void      setPermanentPersonalEnemyFlag(TangibleObject &dest, NetworkId const &enemyId);
	static void      setDuelEnemyFlag           (TangibleObject &dest, TangibleObject const &src);
	static void      removeDuelEnemyFlags       (TangibleObject &dest, NetworkId const &enemyId);
	static void      removePersonalEnemyFlags   (TangibleObject &dest, NetworkId const &enemyId);
	static void      removeTempEnemyFlags       (TangibleObject &dest, NetworkId const &enemyId);
	static void      setAlignedFaction          (TangibleObject &dest, Pvp::FactionId align);
	static void      setNeutralMercenaryFaction (CreatureObject &dest, Pvp::FactionId align, Pvp::PvpType pvpType);
	static void      makeOnLeave                (TangibleObject &dest);
	static void      makeCovert                 (TangibleObject &dest);
	static void      makeDeclared               (TangibleObject &dest);
	static void      makeNeutral                (TangibleObject &dest);
	static void      prepareToBeCovert          (TangibleObject &dest);
	static void      prepareToBeDeclared        (TangibleObject &dest);
	static void      prepareToBeNeutral         (TangibleObject &dest);
	static int       getExpireTimeMs            ();
	static void      updateTimedFlags           (unsigned long updateTimeMs);
	static bool      areFactionsOpposed         (Pvp::FactionId align1, Pvp::FactionId align2);
	static void      handlePvpMessage           (TangibleObject &dest, MessageQueuePvpCommand const &pvpMessage);
	static void      handleAuthorityAcquire     (TangibleObject &dest);
	static void      handleCreatureDied         (TangibleObject &creature);
	static bool      containsAlignedEnemyFlag   (std::vector<PvpEnemy> const &flagList);
	static bool      isFactionAligned           (Pvp::FactionId faction);
	static void      forwardPvpMessage          (TangibleObject &dest, MessageQueuePvpCommand *messageQueuePvpCommand);

private:
	static bool      isTruceRegion              (Region const &regionObj);
	static bool      isBattlefieldRegion        (Region const &regionObj);
	static void      setPermFactionEnemyFlags   (TangibleObject &dest);
	static void      tempEnemyFlagAdded         (TangibleObject const &dest);
	static void      forwardPvpMessage          (TangibleObject &dest, uint32 command, NetworkId const &idParam, Pvp::FactionId factionParam, int intParam);
};

// ======================================================================

#endif // _PvpInternal_H

