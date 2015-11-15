// ======================================================================
//
// PvpInternal.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PvpInternal.h"

#include "serverGame/BattlefieldMarkerObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GroupObject.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PvpFactions.h"
#include "serverGame/PvpUpdateObserver.h"
#include "serverGame/PvpRuleSetBase.h"
#include "serverGame/PvpRuleSetBattlefield.h"
#include "serverGame/PvpRuleSetNormal.h"
#include "serverGame/PvpRuleSetPet.h"
#include "serverGame/PvpRuleSetShip.h"
#include "serverGame/PvpRuleSetTruce.h"
#include "serverGame/RegionPvp.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverGame/TangibleObject.h"
#include "serverGame/TangibleController.h"
#include "serverNetworkMessages/MessageQueuePvpCommand.h"
#include "serverScript/GameScriptObject.h"
#include "serverUtility/PvpEnemy.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedObject/NetworkIdManager.h"
#include <vector>
#include <cstdio>

// ======================================================================

namespace PvpMessages
{
	ConstCharCrcString setEnemyFlag            ("setenemyflag");
	ConstCharCrcString setAlignedFaction       ("setalignedfaction");
	ConstCharCrcString setMercenaryFaction     ("setmercenaryfaction");
	ConstCharCrcString setType                 ("settype");
	ConstCharCrcString setFutureType           ("setfuturetype");
	ConstCharCrcString removeTempEnemyFlags    ("removetempenemyflags");
	ConstCharCrcString removeDuelEnemyFlags    ("removeduelenemyflags");
	ConstCharCrcString removePersonalEnemyFlags("removepersonalenemyflags");
}

// ======================================================================

namespace PvpRegionTypes
{
//	static const int Normal          = 0;
	static const int Truce           = 1;
	static const int Battlefield_Min = 2;
//	static const int Battlefield_Pvp = 2;
//	static const int Battlefield_Pve = 3;
	static const int Battlefield_Max = 3;
}

// ======================================================================

static PvpRuleSetBattlefield                   s_pvpRuleSetBattlefield;
static PvpRuleSetBase                          s_pvpRuleSetDisallowAll;
static PvpRuleSetNormal                        s_pvpRuleSetNormal;
static PvpRuleSetPet                           s_pvpRuleSetPet;
static PvpRuleSetShip                          s_pvpRuleSetShip;
static PvpRuleSetTruce                         s_pvpRuleSetTruce;
static std::set<NetworkId>                     s_objectsWithTempFlags;

// ======================================================================

PvpRuleSetBase &PvpInternal::getRuleSet(TangibleObject const &obj1, TangibleObject const &obj2)
{
	if (ConfigServerGame::getPvpDisableCombat())
		return s_pvpRuleSetDisallowAll;

	// if a ship is involved, use the ship ruleset
	if (obj1.asShipObject() || obj2.asShipObject())
		return s_pvpRuleSetShip;

	// if a pet is involved, use the pet ruleset, which will fall through to the appropriate other ruleset
	if (isPet(obj1) || isPet(obj2))
		return s_pvpRuleSetPet;

	Region const *region1 = obj1.getPvpRegion();
	Region const *region2 = obj2.getPvpRegion();
	if (region1 != region2)
		return s_pvpRuleSetDisallowAll;

	if (region1)
	{
		if (isTruceRegion(*region1))
			return s_pvpRuleSetTruce;
		if (isBattlefieldRegion(*region1))
			return s_pvpRuleSetBattlefield;
	}
	return s_pvpRuleSetNormal;
}

// ----------------------------------------------------------------------

Region const *PvpInternal::getPvpRegion(TangibleObject const &obj)
{
	// Run through the regions this object is in, and find a pvp region.
	// prefer truce to battlefield, battlefield to normal.  0 is returned
	// if there is no truce or battlefield region
	std::vector<Region const *> regions;
	RegionMaster::getRegionsAtPoint(ServerWorld::getSceneId(), obj.getPosition_w().x, obj.getPosition_w().z, regions);
	Region const *region = 0;
	for (std::vector<Region const *>::const_iterator i = regions.begin(); i != regions.end(); ++i)
	{
		if (isTruceRegion(**i))
			return *i;
		if (isBattlefieldRegion(**i))
			region = *i;
	}
	return region;
}

// ----------------------------------------------------------------------

bool PvpInternal::isTruceRegion(Region const &region)
{
	return region.getPvp() == PvpRegionTypes::Truce;
}

// ----------------------------------------------------------------------

bool PvpInternal::isBattlefieldRegion(Region const &region)
{
	int val = region.getPvp();
	return val >= PvpRegionTypes::Battlefield_Min && val <= PvpRegionTypes::Battlefield_Max;
}

// ----------------------------------------------------------------------

TangibleObject const *PvpInternal::getPetMaster(TangibleObject const &who)
{
	CreatureObject const * const creature = who.asCreatureObject();
	if (creature && creature->getMasterId() != NetworkId::cms_invalid)
	{
		Object const * const o = NetworkIdManager::getObjectById(creature->getMasterId());
		if (o)
		{
			ServerObject const * const so = o->asServerObject();
			if (so)
			{
				TangibleObject const * const to = so->asTangibleObject();
				if (to)
					return to;
			}
		}
	}
	return &who;
}

// ----------------------------------------------------------------------

TangibleObject *PvpInternal::getPetMaster(TangibleObject &who)
{
	CreatureObject * const creature = who.asCreatureObject();
	if (creature && creature->getMasterId() != NetworkId::cms_invalid)
	{
		Object * const o = NetworkIdManager::getObjectById(creature->getMasterId());
		if (o)
		{
			ServerObject * const so = o->asServerObject();
			if (so)
			{
				TangibleObject * const to = so->asTangibleObject();
				if (to)
					return to;
			}
		}
	}
	return &who;
}

// ----------------------------------------------------------------------

bool PvpInternal::inSameGroup(TangibleObject const &obj1, TangibleObject const &obj2)
{
	CreatureObject const * const creature1 = obj1.asCreatureObject();
	if (creature1)
	{
		GroupObject const * const group1 = creature1->getGroup();
		if (group1)
		{
			CreatureObject const * const creature2 = obj2.asCreatureObject();
			if (creature2 && creature2->getGroup() == group1)
				return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------

void PvpInternal::getGroupMembers(TangibleObject const &src, std::vector<TangibleObject const *> &members)
{
	members.push_back(&src);
	CreatureObject const * const creature = src.asCreatureObject();
	if (creature)
	{
		GroupObject const * const group = creature->getGroup();
		if (group)
		{
			GroupObject::GroupMemberVector const &groupMembers = group->getGroupMembers();
			for (GroupObject::GroupMemberVector::const_iterator i = groupMembers.begin(); i != groupMembers.end(); ++i)
			{
				if ((*i).first != src.getNetworkId())
				{
					Object const * const o = NetworkIdManager::getObjectById((*i).first);
					if (o)
					{
						ServerObject const * const so = o->asServerObject();
						if (so)
						{
							CreatureObject const * const co = so->asCreatureObject();
							if (co)
								members.push_back(co);
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

Pvp::FactionId PvpInternal::getAlignedFaction(TangibleObject const &who)
{
	return who.getPvpFaction();
}

// ----------------------------------------------------------------------

Pvp::FactionId PvpInternal::getAlignedFaction(NetworkId const &who)
{
	Object const * const o = NetworkIdManager::getObjectById(who);
	if (o)
	{
		ServerObject const * const so = o->asServerObject();
		if (so)
		{
			TangibleObject const * const to = so->asTangibleObject();
			if (to)
				return to->getPvpFaction();
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

int PvpInternal::getExpireTimeMs()
{
	return ConfigServerGame::getPvpTempFlagExpireTimeMs();
}

// ----------------------------------------------------------------------

Pvp::FactionId PvpInternal::battlefieldGetFaction(TangibleObject const &who, RegionPvp const &region)
{
	return region.getBattlefieldFactionId(who.getNetworkId());
}

// ----------------------------------------------------------------------

bool PvpInternal::battlefieldIsParticipant(TangibleObject const &who, RegionPvp const &region)
{
	return battlefieldGetFaction(who, region) != 0;
}

// ----------------------------------------------------------------------

void PvpInternal::battlefieldSetParticipant(TangibleObject const &who, RegionPvp const &region, Pvp::FactionId factionId)
{
	region.setBattlefieldParticipant(who.getNetworkId(), factionId);
}

// ----------------------------------------------------------------------

bool PvpInternal::isPet(TangibleObject const &who)
{
	CreatureObject const * const creature = who.asCreatureObject();
	if (creature && creature->getMasterId() != NetworkId::cms_invalid)
		return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::isPlayer(TangibleObject const &who)
{
	return who.isPlayerControlled();
}

// ----------------------------------------------------------------------

bool PvpInternal::isNeutral(TangibleObject const &who)
{
	return who.getPvpType() == PvpType_Neutral;
}

// ----------------------------------------------------------------------

bool PvpInternal::isCovert(TangibleObject const &who)
{
	return who.getPvpType() == PvpType_Covert;
}

// ----------------------------------------------------------------------

bool PvpInternal::isDeclared(TangibleObject const &who)
{
	return who.getPvpType() == PvpType_Declared;
}

// ----------------------------------------------------------------------

bool PvpInternal::isPreparedToBeNeutral(TangibleObject const &who)
{
	return who.getPvpFutureType() == PvpType_Neutral;
}

// ----------------------------------------------------------------------

bool PvpInternal::isPreparedToBeCovert(TangibleObject const &who)
{
	return who.getPvpFutureType() == PvpType_Covert;
}

// ----------------------------------------------------------------------

bool PvpInternal::isPreparedToBeDeclared(TangibleObject const &who)
{
	return who.getPvpFutureType() == PvpType_Declared;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasAnyEnemyFlag(TangibleObject const &who)
{
	if (who.getPvpEnemies().size())
		return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasAnyTimedEnemyFlag(TangibleObject const &who)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).expireTime)
			return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasAnyAlignedTimedEnemyFlag(TangibleObject const &who)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).expireTime && isFactionAligned((*i).enemyFaction))
			return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasAnyAlignedEnemyFlag(TangibleObject const &who)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if (isFactionAligned((*i).enemyFaction))
			return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasAnyFactionEnemyFlag(TangibleObject const &who)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).enemyId == NetworkId::cms_invalid && isFactionAligned((*i).enemyFaction))
			return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasAnyBountyDuelEnemyFlag(TangibleObject const &who)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).enemyFaction == PvpFactions::getBountyDuelFactionId())
			return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasAnyGuildWarCoolDownPeriodEnemyFlag(TangibleObject const &who)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).enemyFaction == PvpFactions::getGuildWarCoolDownPeriodFactionId())
			return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasFactionEnemyFlag(TangibleObject const &who, Pvp::FactionId enemyAlign)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).enemyId == NetworkId::cms_invalid && (*i).enemyFaction == enemyAlign)
			return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasDuelEnemyFlag(TangibleObject const &who, NetworkId const &enemyId)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).enemyId == enemyId && (*i).enemyFaction == PvpFactions::getDuelFactionId())
			return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasPersonalEnemyFlag(TangibleObject const &who, NetworkId const &enemyId)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).enemyId == enemyId && (*i).enemyFaction != PvpFactions::getDuelFactionId())
			return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::hasBubbleCombatFlag(const TangibleObject &who)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).enemyFaction == PvpFactions::getBubbleCombatFactionId())
			return true;
	return false;
}

bool PvpInternal::hasBubbleCombatFlagAgainstTarget(const TangibleObject &who, const NetworkId &enemyId)
{
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).enemyId == enemyId && (*i).enemyFaction == PvpFactions::getBubbleCombatFactionId())
			return true;
	return false;
}

// ----------------------------------------------------------------------

void PvpInternal::getPersonalEnemyIds(TangibleObject const &who, std::vector<NetworkId> &enemyIds)
{
	enemyIds.clear();
	TangibleObject::PvpEnemies const &enemies = who.getPvpEnemies();
	for (TangibleObject::PvpEnemies::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
		if ((*i).enemyId != NetworkId::cms_invalid)
			enemyIds.push_back((*i).enemyId);
}

// ----------------------------------------------------------------------

void PvpInternal::setPermFactionEnemyFlags(TangibleObject &dest)
{
	FATAL(!dest.isAuthoritative(), ("setPermFactionEnemyFlags on non-authoritative object"));

	// run through the current enemies list and remove any permanent factional enemies
	TangibleObject::PvpEnemies &enemies = dest.getPvpEnemies();
	unsigned int pos = 0;
	while (pos < enemies.size())
	{
		if (!(enemies.get(pos).expireTime) && (enemies.get(pos).enemyId == NetworkId::cms_invalid) && isFactionAligned(enemies.get(pos).enemyFaction))
			enemies.erase(pos);
		else
			++pos;
	}
	// add any permanent factional enemy flags
	Pvp::FactionId align = getAlignedFaction(dest);
	Pvp::PvpType pvpType = dest.getPvpType();

	// if player character is neutral, see if he's a mercenary
	if (PvpData::isNeutralFactionId(align) && isPlayer(dest))
	{
		align = dest.getPvpMercenaryFaction();
		pvpType = dest.getPvpMercenaryType();
	}

	if ((pvpType == PvpType_Declared) && align)
	{
		std::vector<Pvp::FactionId> const &opposingFactions = PvpFactions::getOpposingFactions(align);
		for (std::vector<Pvp::FactionId>::const_iterator j = opposingFactions.begin(); j != opposingFactions.end(); ++j)
			enemies.push_back(PvpEnemy(NetworkId::cms_invalid, *j, 0));
	}
}

// ----------------------------------------------------------------------

void PvpInternal::buildPvpMessageSetFactionEnemyFlag(MessageQueuePvpCommand & messageQueuePvpCommand, Pvp::FactionId enemyAlign, int expireTimeMs)
{
	messageQueuePvpCommand.addPvpCommand(PvpMessages::setEnemyFlag.getCrc(), NetworkId::cms_invalid, static_cast<int>(enemyAlign), expireTimeMs);
}

// ----------------------------------------------------------------------

void PvpInternal::setFactionEnemyFlag(TangibleObject &dest, Pvp::FactionId enemyAlign, int expireTimeMs)
{
	if (!dest.isAuthoritative())
	{
		MessageQueuePvpCommand * messageQueuePvpCommand = new MessageQueuePvpCommand();
		buildPvpMessageSetFactionEnemyFlag(*messageQueuePvpCommand, enemyAlign, expireTimeMs);

		// forwardPvpMessage() will own messageQueuePvpCommand
		// and will be responsible for deallocation
		forwardPvpMessage(dest, messageQueuePvpCommand);

		return;
	}

	//XXX You cannot be flagged against your own faction, so check to make sure that we aren't trying to do that
	Pvp::FactionId destFaction = PvpInternal::getAlignedFaction(dest);
	if (destFaction == enemyAlign)
		return;
	//XXX end

	if (expireTimeMs == -1)
		expireTimeMs = getExpireTimeMs();

	PvpUpdateObserver o(&dest, Archive::ADOO_generic);

	TangibleObject::PvpEnemies &enemies = dest.getPvpEnemies();

	// if the enemy is already set, update and refresh it
	for (unsigned int pos = 0; pos < enemies.size(); ++pos)
	{
		PvpEnemy const &enemy = enemies.get(pos);
		if (enemy.enemyId == NetworkId::cms_invalid && enemy.enemyFaction == enemyAlign)
		{
			// if they have a permanent flag for this faction, or the time difference isn't significant, don't bother to update it,
			// but do update if the flag is going from temporary to permanent
			if (enemy.expireTime && ((expireTimeMs == 0) || (expireTimeMs-enemy.expireTime > 30000)))
				enemies.set(pos, PvpEnemy(NetworkId::cms_invalid, enemyAlign, expireTimeMs));
			return;
		}
	}
	// they didn't already have a flag for this faction, so add one
	enemies.push_back(PvpEnemy(NetworkId::cms_invalid, enemyAlign, expireTimeMs));
	tempEnemyFlagAdded(dest);
}

// ----------------------------------------------------------------------

void PvpInternal::buildPvpMessageSetPersonalEnemyFlag(MessageQueuePvpCommand &messageQueuePvpCommand, NetworkId const &enemyId, Pvp::FactionId enemyAlign, int expireTimeMs)
{
	messageQueuePvpCommand.addPvpCommand(PvpMessages::setEnemyFlag.getCrc(), enemyId, static_cast<int>(enemyAlign), expireTimeMs);
}

// ----------------------------------------------------------------------

void PvpInternal::setPersonalEnemyFlag(TangibleObject &dest, NetworkId const &enemyId, Pvp::FactionId enemyAlign, int expireTimeMs)
{
	if (!dest.isAuthoritative())
	{
		MessageQueuePvpCommand * messageQueuePvpCommand = new MessageQueuePvpCommand();
		buildPvpMessageSetPersonalEnemyFlag(*messageQueuePvpCommand, enemyId, enemyAlign, expireTimeMs);

		// forwardPvpMessage() will own messageQueuePvpCommand
		// and will be responsible for deallocation
		forwardPvpMessage(dest, messageQueuePvpCommand);

		return;
	}

	if (expireTimeMs == -1)
		expireTimeMs = getExpireTimeMs();

	PvpUpdateObserver o(&dest, Archive::ADOO_generic);

	TangibleObject::PvpEnemies &enemies = dest.getPvpEnemies();

	// if the enemy is already set, update and refresh it
	for (unsigned int pos = 0; pos < enemies.size(); ++pos)
	{
		PvpEnemy const &enemy = enemies.get(pos);
		if (enemy.enemyId == enemyId)
		{
			// only update it if the time difference is significant (30 sec)
			// or the flag is going from temporary to permanent
			// or the alignment has changed
			if (((expireTimeMs == 0) || (expireTimeMs-enemy.expireTime > 30000)) && (enemy.expireTime != 0))
				enemies.set(pos, PvpEnemy(enemyId, enemyAlign, expireTimeMs));
			else if (enemy.enemyFaction != enemyAlign)
				enemies.set(pos, PvpEnemy(enemyId, enemyAlign, ((enemy.expireTime == 0) ? 0 : expireTimeMs)));
			return;
		}
	}
	// they didn't already have a flag for this faction, so add one
	enemies.push_back(PvpEnemy(enemyId, enemyAlign, expireTimeMs));
	tempEnemyFlagAdded(dest);
}

// ----------------------------------------------------------------------

void PvpInternal::setPersonalEnemyFlag(TangibleObject &dest, TangibleObject const &src)
{
	setPersonalEnemyFlag(dest, src.getNetworkId(), getAlignedFaction(src));
}

// ----------------------------------------------------------------------

void PvpInternal::setDuelEnemyFlag(TangibleObject &dest, TangibleObject const &src)
{
	setPersonalEnemyFlag(dest, src.getNetworkId(), PvpFactions::getDuelFactionId(), 0);
}

// ----------------------------------------------------------------------

void PvpInternal::setPermanentPersonalEnemyFlag(TangibleObject &dest, NetworkId const &enemyId)
{
	setPersonalEnemyFlag(dest, enemyId, getAlignedFaction(enemyId), 0); // 0 expireTime = permanent
}

// ----------------------------------------------------------------------

bool PvpInternal::areFactionsOpposed(Pvp::FactionId align1, Pvp::FactionId align2)
{
	std::vector<Pvp::FactionId> const &opposingFactions = PvpFactions::getOpposingFactions(align1);
	return std::find(opposingFactions.begin(), opposingFactions.end(), align2) != opposingFactions.end();
}

// ----------------------------------------------------------------------

void PvpInternal::setAlignedFaction(TangibleObject &dest, Pvp::FactionId align)
{
	if (getAlignedFaction(dest) != align)
	{
		if (!dest.isAuthoritative())
		{
			forwardPvpMessage(dest, PvpMessages::setAlignedFaction.getCrc(), NetworkId::cms_invalid, align, 0);
			return;
		}

		PvpUpdateObserver o(&dest, Archive::ADOO_generic);

		dest.setPvpFaction(align);
		if (isDeclared(dest))
			setPermFactionEnemyFlags(dest);
	}
}

// ----------------------------------------------------------------------

void PvpInternal::setNeutralMercenaryFaction(CreatureObject &dest, Pvp::FactionId align, Pvp::PvpType pvpType)
{
	// this only applies to neutral
	if (!PvpData::isNeutralFactionId(getAlignedFaction(dest)) && !PvpData::isNeutralFactionId(align))
		return;

	if ((dest.getPvpMercenaryFaction() != align) || (dest.getPvpMercenaryType() != pvpType))
	{
		if (!dest.isAuthoritative())
		{
			forwardPvpMessage(dest, PvpMessages::setMercenaryFaction.getCrc(), NetworkId::cms_invalid, align, pvpType);
			return;
		}

		bool const wasDeclared = (!PvpData::isNeutralFactionId(dest.getPvpMercenaryFaction()) && (dest.getPvpMercenaryType() == PvpType_Declared));

		PvpUpdateObserver o(&dest, Archive::ADOO_generic);

		dest.setPvpMercenaryFaction(align, pvpType);

		bool const isDeclared = (!PvpData::isNeutralFactionId(dest.getPvpMercenaryFaction()) && (dest.getPvpMercenaryType() == PvpType_Declared));

		if (wasDeclared != isDeclared)
			setPermFactionEnemyFlags(dest);
	}
}

// ----------------------------------------------------------------------

void PvpInternal::makeOnLeave(TangibleObject &dest)
{
	// don't need to do anything if the character
	// is not factional or already "on leave";
	// "on leave" means character belongs to
	// a faction and is PvpType_Neutral
	if (dest.getPvpFaction() == 0)
		return;

	if (dest.getPvpType() == PvpType_Neutral)
		return;

	if (!dest.isAuthoritative())
	{
		// 782497134 (just an arbitrary random value) is a special value to indicate "on leave"
		forwardPvpMessage(dest, PvpMessages::setType.getCrc(), NetworkId::cms_invalid, 0, 782497134);
		return;
	}

	PvpUpdateObserver o(&dest, Archive::ADOO_generic);

	bool wasDeclared = isDeclared(dest);
	dest.setPvpType(PvpType_Neutral);
	if (wasDeclared)
		setPermFactionEnemyFlags(dest);
}

// ----------------------------------------------------------------------

void PvpInternal::makeCovert(TangibleObject &dest)
{
	if (!isCovert(dest))
	{
		if (!dest.isAuthoritative())
		{
			forwardPvpMessage(dest, PvpMessages::setType.getCrc(), NetworkId::cms_invalid, 0, 1);
			return;
		}

		PvpUpdateObserver o(&dest, Archive::ADOO_generic);

		bool wasDeclared = isDeclared(dest);
		dest.setPvpType(PvpType_Covert);
		if (wasDeclared)
			setPermFactionEnemyFlags(dest);
	}
}

// ----------------------------------------------------------------------

void PvpInternal::makeDeclared(TangibleObject &dest)
{
	if (!isDeclared(dest))
	{
		if (!dest.isAuthoritative())
		{
			forwardPvpMessage(dest, PvpMessages::setType.getCrc(), NetworkId::cms_invalid, 0, 2);
			return;
		}

		PvpUpdateObserver o(&dest, Archive::ADOO_generic);

		dest.setPvpType(PvpType_Declared);
		setPermFactionEnemyFlags(dest);
	}
}

// ----------------------------------------------------------------------

void PvpInternal::makeNeutral(TangibleObject &dest)
{
	// don't need to do anything if
	// the character is already neutral
	if ((dest.getPvpFaction() == 0) && (dest.getPvpType() == PvpType_Neutral))
		return;

	if (!dest.isAuthoritative())
	{
		forwardPvpMessage(dest, PvpMessages::setType.getCrc(), NetworkId::cms_invalid, 0, 0);
		return;
	}

	PvpUpdateObserver o(&dest, Archive::ADOO_generic);

	bool wasDeclared = isDeclared(dest);

	if (dest.getPvpType() != PvpType_Neutral)
		dest.setPvpType(PvpType_Neutral);

	if (wasDeclared)
		setPermFactionEnemyFlags(dest);

	// lose faction when made neutral
	if (dest.getPvpFaction() != 0)
		setAlignedFaction(dest, 0);
}

// ----------------------------------------------------------------------

void PvpInternal::prepareToBeCovert(TangibleObject &dest)
{
	if (!isPreparedToBeCovert(dest))
	{
		if (!dest.isAuthoritative())
		{
			forwardPvpMessage(dest, PvpMessages::setFutureType.getCrc(), NetworkId::cms_invalid, 0, 1);
			return;
		}

		PvpUpdateObserver o(&dest, Archive::ADOO_generic);
		dest.setPvpFutureType(PvpType_Covert);
	}
}

// ----------------------------------------------------------------------

void PvpInternal::prepareToBeDeclared(TangibleObject &dest)
{
	if (!isPreparedToBeDeclared(dest))
	{
		if (!dest.isAuthoritative())
		{
			forwardPvpMessage(dest, PvpMessages::setFutureType.getCrc(), NetworkId::cms_invalid, 0, 2);
			return;
		}

		PvpUpdateObserver o(&dest, Archive::ADOO_generic);
		dest.setPvpFutureType(PvpType_Declared);
	}
}

// ----------------------------------------------------------------------

void PvpInternal::prepareToBeNeutral(TangibleObject &dest)
{
	if (!isPreparedToBeNeutral(dest))
	{
		if (!dest.isAuthoritative())
		{
			forwardPvpMessage(dest, PvpMessages::setFutureType.getCrc(), NetworkId::cms_invalid, 0, 0);
			return;
		}

		PvpUpdateObserver o(&dest, Archive::ADOO_generic);
		dest.setPvpFutureType(PvpType_Neutral);
	}
}

// ----------------------------------------------------------------------

void PvpInternal::tempEnemyFlagAdded(TangibleObject const &dest)
{
	IGNORE_RETURN(s_objectsWithTempFlags.insert(dest.getNetworkId()));
}

// ----------------------------------------------------------------------

void PvpInternal::removeDuelEnemyFlags(TangibleObject &dest, NetworkId const &enemyId)
{
	if (!dest.isAuthoritative())
	{
		forwardPvpMessage(dest, PvpMessages::removeDuelEnemyFlags.getCrc(), enemyId, 0, 0);
		return;
	}

	PvpUpdateObserver o(&dest, Archive::ADOO_generic);
	TangibleObject::PvpEnemies &enemies = dest.getPvpEnemies();
	unsigned int pos = 0;
	while (pos < enemies.size())
	{
		PvpEnemy const &enemy = enemies.get(pos);
		bool removed = false;
		if (enemy.enemyFaction == PvpFactions::getDuelFactionId())
		{
			if (enemyId == NetworkId::cms_invalid)
			{
				// if removing all duel flags from someone, remove the opposite flag from those involved
				Object * const o = NetworkIdManager::getObjectById(enemy.enemyId);
				if (o)
				{
					ServerObject * const so = o->asServerObject();
					if (so)
					{
						TangibleObject * const to = so->asTangibleObject();
						if (to)
							removeDuelEnemyFlags(*to, dest.getNetworkId());
					}
				}
				removed = true;
				enemies.erase(pos);
			}
			else if (enemy.enemyId == enemyId)
			{
				removed = true;
				enemies.erase(pos);
			}
		}
		if (!removed)
			++pos;
	}
}

// ----------------------------------------------------------------------

void PvpInternal::removePersonalEnemyFlags(TangibleObject &dest, NetworkId const &enemyId)
{
	if (!dest.isAuthoritative())
	{
		forwardPvpMessage(dest, PvpMessages::removePersonalEnemyFlags.getCrc(), enemyId, 0, 0);
		return;
	}

	PvpUpdateObserver o(&dest, Archive::ADOO_generic);
	TangibleObject::PvpEnemies &enemies = dest.getPvpEnemies();
	unsigned int pos = 0;
	while (pos < enemies.size())
	{
		PvpEnemy const &enemy = enemies.get(pos);
		if (enemy.enemyId == enemyId && enemy.enemyFaction != PvpFactions::getDuelFactionId())
			enemies.erase(pos);
		else
			++pos;
	}
}

// ----------------------------------------------------------------------

void PvpInternal::removeTempEnemyFlags(TangibleObject &dest, NetworkId const &enemyId)
{
	if (!dest.isAuthoritative())
	{
		forwardPvpMessage(dest, PvpMessages::removeTempEnemyFlags.getCrc(), enemyId, 0, 0);
		return;
	}

	PvpUpdateObserver o(&dest, Archive::ADOO_generic);
	TangibleObject::PvpEnemies &enemies = dest.getPvpEnemies();
	unsigned int pos = 0;
	while (pos < enemies.size())
	{
		PvpEnemy const &enemy = enemies.get(pos);
		if (enemy.expireTime && (enemyId == NetworkId::cms_invalid || enemy.enemyId == enemyId))
			enemies.erase(pos);
		else
			++pos;
	}
}

// ----------------------------------------------------------------------

void PvpInternal::updateTimedFlags(unsigned long updateTimeMs)
{
	// Run through adjusting the remaining time on any temp flags.  This may cause pvp status changes, so
	// we use a PvpUpdateObserver to catch anything significant.
	std::set<NetworkId>::iterator i = s_objectsWithTempFlags.begin();
	while (i != s_objectsWithTempFlags.end())
	{
		bool removeFlag = true;
		Object *o = NetworkIdManager::getObjectById(*i);
		if (o)
		{
			ServerObject *so = o->asServerObject();
			if (so && so->isAuthoritative())
			{
				TangibleObject *to = so->asTangibleObject();
				if (to)
				{
					PvpUpdateObserver updateObserver(to, Archive::ADOO_generic);
					TangibleObject::PvpEnemies &enemies = to->getPvpEnemies();
					unsigned int pos = 0;
					while (pos < enemies.size())
					{
						PvpEnemy const &enemy = enemies.get(pos);
						int expireTime = enemy.expireTime;

						if (expireTime)
						{
							expireTime -= static_cast<int>(updateTimeMs);
							if (expireTime <= 0)
								enemies.erase(pos);
							else
							{
								enemies.set(pos, PvpEnemy(enemy.enemyId, enemy.enemyFaction, expireTime));
								removeFlag = false;
								++pos;
							}
						}
						else
							++pos;
					}
				}
			}
		}
		if (removeFlag)
			s_objectsWithTempFlags.erase(i++);
		else
			++i;
	}
}

// ----------------------------------------------------------------------

void PvpInternal::handleAuthorityAcquire(TangibleObject &dest)
{
	setPermFactionEnemyFlags(dest);

	if (hasAnyTimedEnemyFlag(dest))
		IGNORE_RETURN(s_objectsWithTempFlags.insert(dest.getNetworkId()));
}

// ----------------------------------------------------------------------

/**
 * Clean up TEFs to a creature that has died.
 * @todo: this is really slow, speed it up!
 */
void PvpInternal::handleCreatureDied(TangibleObject &creature)
{
	// clear out any duel flags on the player, and reciprocated flags for any we can find
	if (creature.isAuthoritative() && creature.isPlayerControlled())
		removeDuelEnemyFlags(creature, NetworkId::cms_invalid);

	// Run through removing pef's toward this creature from auth players
	std::set<NetworkId>::iterator i = s_objectsWithTempFlags.begin();
	while (i != s_objectsWithTempFlags.end())
	{
		bool removeFlag = true;
		Object *o = NetworkIdManager::getObjectById(*i);
		if (o)
		{
			ServerObject *so = o->asServerObject();
			if (so && so->isAuthoritative())
			{
				TangibleObject *to = so->asTangibleObject();
				if (to)
				{
		 			if (!to->isPlayerControlled())
						removeFlag = false;
					else
					{
						PvpUpdateObserver updateObserver(to, Archive::ADOO_generic);
						TangibleObject::PvpEnemies &enemies = to->getPvpEnemies();
						unsigned int pos = 0;
						while (pos < enemies.size())
						{
							PvpEnemy const &enemy = enemies.get(pos);
							if (enemy.expireTime && enemy.enemyId == creature.getNetworkId())
								enemies.erase(pos);
							else
							{
								removeFlag = false;
								++pos;
							}
						}
					}
				}
			}
		}
		if (removeFlag)
			s_objectsWithTempFlags.erase(i++);
		else
			++i;
	}
}

// ----------------------------------------------------------------------

// messageQueuePvpCommand must be dynamically allocated; function owns
// messageQueuePvpCommand and will be responsible for deallocation
void PvpInternal::forwardPvpMessage(TangibleObject &dest, MessageQueuePvpCommand *messageQueuePvpCommand)
{
	if (!messageQueuePvpCommand)
		return;

	Controller *controller = dest.getController();
	FATAL(!controller, ("forwardPvpMessage with no object controller"));
	safe_cast<TangibleController*>(controller)->appendMessage(
		static_cast<int>(CM_pvpMessage),
		0.0f,
		messageQueuePvpCommand,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_SERVER);
}

// ----------------------------------------------------------------------

void PvpInternal::forwardPvpMessage(TangibleObject &dest, uint32 command, NetworkId const &idParam, Pvp::FactionId factionParam, int intParam)
{
	// forwardPvpMessage() will own messageQueuePvpCommand
	// and will be responsible for deallocation
	forwardPvpMessage(dest, new MessageQueuePvpCommand(command, idParam, static_cast<int>(factionParam), intParam));
}

// ----------------------------------------------------------------------

void PvpInternal::handlePvpMessage(TangibleObject &dest, MessageQueuePvpCommand const &pvpMessage)
{
	MessageQueuePvpCommand::PvpCommands const & pvpCommands = pvpMessage.getPvpCommands();

	if (pvpCommands.empty())
		return;

	PvpUpdateObserver o(&dest, Archive::ADOO_generic);

	for (MessageQueuePvpCommand::PvpCommands::const_iterator i = pvpCommands.begin(); i != pvpCommands.end(); ++i)
	{
		if (i->m_command == PvpMessages::setEnemyFlag.getCrc())
		{
			if (i->m_idParam == NetworkId::cms_invalid)
				setFactionEnemyFlag(dest, static_cast<Pvp::FactionId>(i->m_intParam1), i->m_intParam2);
			else
				setPersonalEnemyFlag(dest, i->m_idParam, static_cast<Pvp::FactionId>(i->m_intParam1), i->m_intParam2);
		}
		else if (i->m_command == PvpMessages::setAlignedFaction.getCrc())
		{
			setAlignedFaction(dest, static_cast<Pvp::FactionId>(i->m_intParam1));
		}
		else if (i->m_command == PvpMessages::setMercenaryFaction.getCrc())
		{
			CreatureObject * const co = dest.asCreatureObject();
			if (co)
				setNeutralMercenaryFaction(*co, static_cast<Pvp::FactionId>(i->m_intParam1), static_cast<Pvp::PvpType>(i->m_intParam2));
		}
		else if (i->m_command == PvpMessages::setType.getCrc())
		{
			switch (i->m_intParam2)
			{
			case PvpType_Neutral:
				makeNeutral(dest);
				break;
			case PvpType_Covert:
				makeCovert(dest);
				break;
			case PvpType_Declared:
				makeDeclared(dest);
				break;
			// 782497134 (just an arbitrary random value) is a special value to indicate "on leave"
			case 782497134:
				makeOnLeave(dest);
				break;
			default:
				FATAL(true, ("bad pvp type specified"));
				break;
			}
		}
		else if (i->m_command == PvpMessages::setFutureType.getCrc())
		{
			switch (i->m_intParam2)
			{
			case PvpType_Neutral:
				prepareToBeNeutral(dest);
				break;
			case PvpType_Covert:
				prepareToBeCovert(dest);
				break;
			case PvpType_Declared:
				prepareToBeDeclared(dest);
				break;
			default:
				FATAL(true, ("bad future pvp type specified"));
				break;
			}
		}
		else if (i->m_command == PvpMessages::removeTempEnemyFlags.getCrc())
		{
			removeTempEnemyFlags(dest, i->m_idParam);
		}
		else if (i->m_command == PvpMessages::removeDuelEnemyFlags.getCrc())
		{
			removeDuelEnemyFlags(dest, i->m_idParam);
		}
		else if (i->m_command == PvpMessages::removePersonalEnemyFlags.getCrc())
		{
			removePersonalEnemyFlags(dest, i->m_idParam);
		}
	}
}

// ----------------------------------------------------------------------

bool PvpInternal::containsAlignedEnemyFlag(std::vector<PvpEnemy> const &flagList)
{
	for (std::vector<PvpEnemy>::const_iterator i = flagList.begin(); i != flagList.end(); ++i)
		if (isFactionAligned((*i).enemyFaction))
			return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpInternal::isFactionAligned(Pvp::FactionId faction)
{
	if (   faction
	    && faction != PvpFactions::getDuelFactionId()
	    && faction != PvpFactions::getBountyDuelFactionId()
	    && faction != PvpFactions::getGuildWarCoolDownPeriodFactionId()
	    && faction != PvpFactions::getBattlefieldFactionId()
	    && !PvpFactions::isNonaggressiveFaction(faction)
	    && !PvpFactions::isBountyTargetFaction(faction))
		return true;
	return false;
}

// ======================================================================
