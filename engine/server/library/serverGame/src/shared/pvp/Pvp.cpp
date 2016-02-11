// ======================================================================
//
// Pvp.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/Pvp.h"

#include "serverGame/CellObject.h"
#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/GuildObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PvpInternal.h"
#include "serverGame/PvpRuleSetBase.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/RegionPvp.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TangibleObject.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Scheduler.h"
#include "sharedGame/PvpData.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/UpdatePvpStatusMessage.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <limits>

// ======================================================================

namespace PvpNamespace
{
	bool s_installed;
	Scheduler *s_pvpScheduler;

	std::string const s_gcwRankTableName("datatables/gcw/gcw_rank.iff");
	std::string const s_gcwRankDecayExclusionTableName("datatables/gcw/gcw_rank_decay_exclusion.iff");

	Pvp::PvpRankInfo s_RankNone;
	std::map<int, Pvp::PvpRankInfo *> s_RankData;
	std::multimap<time_t, time_t> s_RankDecayExclusion;
	int s_minRatingForRank;
	int s_maxRatingForRank;
	std::string s_debugGcwRankTable;
	std::string s_debugGcwFormulasTable;
	std::string s_debugGcwRankDecayExclusion;

	void loadGcwRankTable();
	void loadGcwRankDecayExclusionTable();

	std::string const s_gcwScoreCategoryTableName("datatables/gcw/gcw_score_category.iff");
	std::map<std::string, Pvp::GcwScoreCategory const *> s_gcwScoreCategory;

	std::map<std::string, std::map<std::string, std::pair<std::pair<float, float>, float> > > s_gcwScoreCategoryRegions;

	std::map<std::string, std::map<std::string, int> > s_gcwScoreCategoryGroups;

	//
	// *****WARNING WARNING WARNING WARNING WARNING*****
	// this list must be kept in sync in
	// PvpNamespace::loadGcwScoreCategoryTable()
	// SwgCuiSpaceZoneMap::SwgCuiSpaceZoneMap()
	// SwgCuiHyperspaceMap::SwgCuiHyperspaceMap()
	// and gcw.scriptlib (validScenes and defaultRegions)
	// *****WARNING WARNING WARNING WARNING WARNING*****
	//
	std::map<std::string, Pvp::GcwScoreCategory const *> s_gcwScoreDefaultCategoryForPlanet;

	void loadGcwScoreCategoryTable();
}

// ======================================================================

using namespace PvpNamespace;

// ======================================================================

void Pvp::install()
{
	FATAL(s_installed, ("Pvp::install - already installed"));
	s_installed = true;
	s_pvpScheduler = new Scheduler;

	loadGcwRankTable();
	loadGcwRankDecayExclusionTable();
	loadGcwScoreCategoryTable();
}

// ----------------------------------------------------------------------

void Pvp::remove()
{
	FATAL(!s_installed, ("Pvp::remove - not installed"));

	for (std::map<int, Pvp::PvpRankInfo *>::const_iterator iter = s_RankData.begin(); iter != s_RankData.end(); ++iter)
		delete iter->second;

	s_RankData.clear();

	s_RankDecayExclusion.clear();

	for (std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter2 = s_gcwScoreCategory.begin(); iter2 != s_gcwScoreCategory.end(); ++iter2)
		delete iter2->second;

	s_gcwScoreCategory.clear();

	s_gcwScoreCategoryRegions.clear();

	s_gcwScoreCategoryGroups.clear();

	delete s_pvpScheduler;
	s_pvpScheduler = 0;
	s_installed = false;
}

// ----------------------------------------------------------------------

bool Pvp::canAttack(TangibleObject const &actor, TangibleObject const &target)
{
	if (!PvpInternal::getRuleSet(actor, target).canAttack(actor, target))
		return false;

	// At this point, we know that the client believes they are able to attack, but now we apply any rules which
	// do not affect the client status.

	// Prevent people from attacking if they are not allowed in their opponent's cell.  We cannot easily update
	// pvp status of clients on change of this condition, so we apply it outside of normal pvp rules.
	CreatureObject const * const creatureTarget = target.asCreatureObject();
	if (creatureTarget)
	{
		// if the target is not allowed in the actor's cell, don't allow the attack
		CellObject const * const actorCell = ContainerInterface::getContainingCellObject(actor);
		if (actorCell && !actorCell->isAllowed(*creatureTarget))
			return false;
	}
	CreatureObject const * const creatureActor = actor.asCreatureObject();
	if (creatureActor)
	{
		// if the actor is not allowed in the target's cell, don't allow the attack
		CellObject const * const targetCell = ContainerInterface::getContainingCellObject(target);
		if (targetCell && !targetCell->isAllowed(*creatureActor))
			return false;
	}
	return true;
}

// ----------------------------------------------------------------------

bool Pvp::canHelp(TangibleObject const &actor, TangibleObject const &target)
{
	return PvpInternal::getRuleSet(actor, target).canHelp(actor, target);
}

// ----------------------------------------------------------------------

void Pvp::attackPerformed(TangibleObject &actor, TangibleObject &target)
{
	std::vector<PvpEnemy> actorRepercussions;
	std::vector<PvpEnemy> targetRepercussions;
	PvpRuleSetBase &ruleSet = PvpInternal::getRuleSet(actor, target);
	ruleSet.getAttackRepercussions(actor, target, actorRepercussions, targetRepercussions);
	ruleSet.applyRepercussions(actor, target, actorRepercussions, targetRepercussions);
}

// ----------------------------------------------------------------------

void Pvp::helpPerformed(TangibleObject &actor, TangibleObject &target)
{
	std::vector<PvpEnemy> actorRepercussions;
	std::vector<PvpEnemy> targetRepercussions;
	PvpRuleSetBase &ruleSet = PvpInternal::getRuleSet(actor, target);
	ruleSet.getHelpRepercussions(actor, target, actorRepercussions, targetRepercussions);
	ruleSet.applyRepercussions(actor, target, actorRepercussions, targetRepercussions);
}

// ----------------------------------------------------------------------

bool Pvp::wouldAttackCauseAlignedEnemyFlag(TangibleObject const &actor, TangibleObject const &target)
{
	std::vector<PvpEnemy> actorRepercussions;
	std::vector<PvpEnemy> targetRepercussions;
	PvpInternal::getRuleSet(actor, target).getAttackRepercussions(actor, target, actorRepercussions, targetRepercussions);
	return PvpInternal::containsAlignedEnemyFlag(actorRepercussions);
}

// ----------------------------------------------------------------------

bool Pvp::wouldHelpCauseAlignedEnemyFlag(TangibleObject const &actor, TangibleObject const &target)
{
	std::vector<PvpEnemy> actorRepercussions;
	std::vector<PvpEnemy> targetRepercussions;
	PvpInternal::getRuleSet(actor, target).getHelpRepercussions(actor, target, actorRepercussions, targetRepercussions);
	return PvpInternal::containsAlignedEnemyFlag(actorRepercussions);
}

// ----------------------------------------------------------------------

bool Pvp::isEnemy(TangibleObject const &actor, TangibleObject const &target)
{
	return PvpInternal::getRuleSet(actor, target).isEnemy(actor, target);
}

// ----------------------------------------------------------------------

void Pvp::setAlignedFaction(TangibleObject &who, FactionId factionId)
{
	PvpInternal::setAlignedFaction(who, factionId);
}

// ----------------------------------------------------------------------

void Pvp::setNeutralMercenaryFaction(CreatureObject &who, FactionId factionId, PvpType pvpType)
{
	PvpInternal::setNeutralMercenaryFaction(who, factionId, pvpType);
}

// ----------------------------------------------------------------------

void Pvp::makeOnLeave(TangibleObject &who)
{
	PvpInternal::makeOnLeave(who);
}

// ----------------------------------------------------------------------

void Pvp::makeCovert(TangibleObject &who)
{
	PvpInternal::makeCovert(who);
}

// ----------------------------------------------------------------------

void Pvp::makeDeclared(TangibleObject &who)
{
	PvpInternal::makeDeclared(who);
}

// ----------------------------------------------------------------------

void Pvp::makeNeutral(TangibleObject &who)
{
	PvpInternal::makeNeutral(who);
}

// ----------------------------------------------------------------------

void Pvp::prepareToBeCovert(TangibleObject &who)
{
	PvpInternal::prepareToBeCovert(who);
}

// ----------------------------------------------------------------------

void Pvp::prepareToBeDeclared(TangibleObject &who)
{
	PvpInternal::prepareToBeDeclared(who);
}

// ----------------------------------------------------------------------

void Pvp::prepareToBeNeutral(TangibleObject &who)
{
	PvpInternal::prepareToBeNeutral(who);
}

// ----------------------------------------------------------------------

void Pvp::setFactionEnemyFlag(TangibleObject &who, FactionId enemyAlign, int expireTimeMs)
{
	PvpInternal::setFactionEnemyFlag(who, enemyAlign, expireTimeMs);
}

// ----------------------------------------------------------------------

void Pvp::setDuelEnemyFlag(TangibleObject &who, TangibleObject const &enemy)
{
	PvpInternal::setDuelEnemyFlag(who, enemy);
}

// ----------------------------------------------------------------------

void Pvp::setPersonalEnemyFlag(TangibleObject &who, TangibleObject const &enemy)
{
	PvpInternal::setPersonalEnemyFlag(who, enemy);
}

// ----------------------------------------------------------------------

void Pvp::setPermanentPersonalEnemyFlag(TangibleObject &who, NetworkId const &enemyId)
{
	PvpInternal::setPermanentPersonalEnemyFlag(who, enemyId);
}

// ----------------------------------------------------------------------

void Pvp::removeAllTempEnemyFlags(TangibleObject &dest)
{
	PvpInternal::removeTempEnemyFlags(dest, NetworkId::cms_invalid);
}

// ----------------------------------------------------------------------

void Pvp::removeDuelEnemyFlags(TangibleObject &dest, NetworkId const &enemyId)
{
	PvpInternal::removeDuelEnemyFlags(dest, enemyId);
}

// ----------------------------------------------------------------------

void Pvp::removePersonalEnemyFlags(TangibleObject &dest, NetworkId const &enemyId)
{
	PvpInternal::removePersonalEnemyFlags(dest, enemyId);
}

// ----------------------------------------------------------------------

void Pvp::removeTempEnemyFlags(TangibleObject &dest, NetworkId const &enemyId)
{
	PvpInternal::removeTempEnemyFlags(dest, enemyId);
}

// ----------------------------------------------------------------------

void Pvp::battlefieldSetParticipant(TangibleObject const &who, RegionPvp const &region, Pvp::FactionId factionId)
{
	PvpInternal::battlefieldSetParticipant(who, region, factionId);
}

// ----------------------------------------------------------------------

void Pvp::battlefieldClearParticipants(RegionPvp const &region)
{
	region.clearBattlefieldParticipants();
}

// ----------------------------------------------------------------------

void Pvp::handleCreatureDied(TangibleObject &creature)
{
	PvpInternal::handleCreatureDied(creature);
}

// ----------------------------------------------------------------------

bool Pvp::hasDuelEnemyFlag(TangibleObject const &who, NetworkId const &enemyId)
{
	return PvpInternal::hasDuelEnemyFlag(who, enemyId);
}

// ----------------------------------------------------------------------

bool Pvp::hasPersonalEnemyFlag(TangibleObject const &who, NetworkId const &enemyId)
{
	return PvpInternal::hasPersonalEnemyFlag(who, enemyId);
}

// ----------------------------------------------------------------------

bool Pvp::hasFactionEnemyFlag(TangibleObject const &who, FactionId factionId)
{
	return PvpInternal::hasFactionEnemyFlag(who, factionId);
}

// ----------------------------------------------------------------------

void Pvp::getPersonalEnemyIds(TangibleObject const &who, std::vector<NetworkId> &enemyIds)
{
	PvpInternal::getPersonalEnemyIds(who, enemyIds);
}

// ----------------------------------------------------------------------

bool Pvp::areFactionsOpposed(FactionId align1, FactionId align2)
{
	return PvpInternal::areFactionsOpposed(align1, align2);
}

// ----------------------------------------------------------------------

Pvp::FactionId Pvp::battlefieldGetFaction(TangibleObject const &who, RegionPvp const &region)
{
	return PvpInternal::battlefieldGetFaction(who, region);
}

// ----------------------------------------------------------------------

bool Pvp::battlefieldIsParticipant(TangibleObject const &who, RegionPvp const &region)
{
	return PvpInternal::battlefieldIsParticipant(who, region);
}

// ----------------------------------------------------------------------

bool Pvp::hasAnyTimedEnemyFlag(TangibleObject const &who)
{
	return PvpInternal::hasAnyTimedEnemyFlag(who);
}

// ----------------------------------------------------------------------

bool Pvp::hasAnyAlignedTimedEnemyFlag(TangibleObject const &who)
{
	return PvpInternal::hasAnyAlignedTimedEnemyFlag(who);
}

// ----------------------------------------------------------------------

bool Pvp::hasAnyBountyDuelEnemyFlag(TangibleObject const &who)
{
	return PvpInternal::hasAnyBountyDuelEnemyFlag(who);
}

// ----------------------------------------------------------------------

bool Pvp::hasAnyGuildWarCoolDownPeriodEnemyFlag(TangibleObject const &who)
{
	return PvpInternal::hasAnyGuildWarCoolDownPeriodEnemyFlag(who);
}

// ----------------------------------------------------------------------

Region const *Pvp::getPvpRegion(TangibleObject const &who)
{
	return PvpInternal::getPvpRegion(who);
}

// ----------------------------------------------------------------------

Pvp::FactionId Pvp::getApparantFaction(FactionId viewerFaction, TangibleObject const &target)
{
	FactionId const targetAlign = PvpInternal::getAlignedFaction(target);
	if (viewerFaction == targetAlign || PvpInternal::isDeclared(target))
		return targetAlign;
	return 0;
}

// ----------------------------------------------------------------------

bool Pvp::isDuelingAllowed(TangibleObject const &actor, TangibleObject const &target)
{
	return PvpInternal::getRuleSet(actor, target).isDuelingAllowed(actor, target);
}

// ----------------------------------------------------------------------

void Pvp::getClientVisibleStatus(Client const &viewerClient, TangibleObject const &target, uint32 &flags, FactionId &factionId)
{
	PROFILER_AUTO_BLOCK_DEFINE("Pvp::getClientVisibleStatus");

	TangibleObject const &viewer = target.getPvpViewer(viewerClient);
	PvpRuleSetBase &ruleSet = PvpInternal::getRuleSet(viewer, target);

	flags = 0;
	factionId = 0;
	if (PvpInternal::isPlayer(target))
		flags |= PvpStatusFlags::IsPlayer;
	if (ruleSet.canAttack(viewer, target))
		flags |= PvpStatusFlags::YouCanAttack;
	if (ruleSet.canAttack(target, viewer))
		flags |= PvpStatusFlags::CanAttackYou;
	if (ruleSet.isEnemy(target, viewer))
		flags |= PvpStatusFlags::IsEnemy;
	if (ruleSet.canHelp(viewer, target))
		flags |= PvpStatusFlags::YouCanHelp;

	TangibleObject const * realTarget = &target;

	// when dealing with pets, pvp type and faction is based on the master
	if (PvpInternal::isPet(*realTarget))
		realTarget = PvpInternal::getPetMaster(*realTarget);

	if (realTarget)
	{
		// only get to know about enemy flags if you're the one watching
		if (&viewer == realTarget && PvpInternal::hasAnyAlignedTimedEnemyFlag(viewer))
			flags |= PvpStatusFlags::HasEnemies;
		FactionId const targetAlign = PvpInternal::getAlignedFaction(*realTarget);
		// only get to know about alignment if the target is declared
  	// or has same alignment as viewer
		if (PvpInternal::isDeclared(*realTarget))
		{
			flags |= PvpStatusFlags::Declared;
			factionId = targetAlign;
		}
		else if (PvpInternal::getAlignedFaction(viewer) == targetAlign)
			factionId = targetAlign;

		if (PvpInternal::isPreparedToBeDeclared(*realTarget))
		{
			flags |= PvpStatusFlags::WillBeDeclared;
		}

		//if (PvpInternal::isPreparedToBeNeutral(*realTarget))
		//{
		//	flags |= PvpStatusFlags::WasDeclared;
		//}
	
		if (PvpInternal::isPreparedToBeCovert(*realTarget))
		{
			flags |= PvpStatusFlags::WasDeclared;
		}
	}
}

// ----------------------------------------------------------------------

void Pvp::forceStatusUpdate(TangibleObject &who)
{
	// send status of this object to all observing it (including self)
	if (!who.isNonPvpObject())
	{
		std::set<Client *> const &clients = who.getObservers();
		for (std::set<Client *>::const_iterator i = clients.begin(); i != clients.end(); ++i)
		{
			uint32 flags, factionId;
			Pvp::getClientVisibleStatus(**i, who, flags, factionId);
			if (ConfigServerGame::getLogPvpUpdates())
			{
				LOG("PvpUpdate", ("sending UpdatePvpStatus for %s to client %s, flags=%d, factionId=%d (forced)",
					who.getNetworkId().getValueString().c_str(),
					(*i)->getCharacterObjectId().getValueString().c_str(),
					flags,
					factionId));
			}
			UpdatePvpStatusMessage const statusMessage(who.getNetworkId(), flags, factionId);
			(*i)->send(statusMessage, true);
			PvpUpdateObserver::updatePvpStatusCache(*i, who, flags, factionId);
		}
	}
	// send status of all objects this object is observing (except self)
	{
		Client * const client = who.getClient();
		if (client)
		{
			Client::ObservingList const &objs = client->getObserving();
			for (Client::ObservingList::const_iterator i = objs.begin(); i != objs.end(); ++i)
			{
				TangibleObject * const obj = (*i)->asTangibleObject();
				if (obj && obj != &who && !obj->isNonPvpObject())
				{
					uint32 flags, factionId;
					Pvp::getClientVisibleStatus(*client, *obj, flags, factionId);
					if (ConfigServerGame::getLogPvpUpdates())
					{
						LOG("PvpUpdate", ("sending UpdatePvpStatus for %s to client %s, flags=%d, factionId=%d (forced)",
							obj->getNetworkId().getValueString().c_str(),
							client->getCharacterObjectId().getValueString().c_str(),
							flags,
							factionId));
					}
					UpdatePvpStatusMessage const statusMessage(obj->getNetworkId(), flags, factionId);
					client->send(statusMessage, true);
					PvpUpdateObserver::updatePvpStatusCache(client, *obj, flags, factionId);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void Pvp::checkForRegionChange(TangibleObject &who)
{
	FATAL(!who.isAuthoritative(), ("checkForRegionChange on non-authoritative object"));
	if (!who.isNonPvpObject())
	{
		// pvp regions
		Region const * const pvpRegion = getPvpRegion(who);
		if (pvpRegion != who.getPvpRegion())
			who.setPvpRegion(pvpRegion ? pvpRegion->asRegionPvp() : 0);
		// cities
		CreatureObject * const creature = who.asCreatureObject();
		if (creature && creature->isPlayerControlled())
		{
			Vector const v(creature->getPosition_w());
			int const newCityId = CityInterface::getCityAtLocation(ServerWorld::getSceneId(), static_cast<int>(v.x), static_cast<int>(v.z), 0);
			if (newCityId != creature->getLocatedInCityId())
				creature->setLocatedInCityId(newCityId);
		}
	}
}

// ----------------------------------------------------------------------

void Pvp::handlePvpMessage(TangibleObject &dest, MessageQueuePvpCommand const &pvpMessage)
{
	PvpInternal::handlePvpMessage(dest, pvpMessage);
}

// ----------------------------------------------------------------------

void Pvp::handleAuthorityAcquire(TangibleObject &dest)
{
	PvpInternal::handleAuthorityAcquire(dest);
}

// ----------------------------------------------------------------------

void Pvp::updateTimedFlags(const void *context)
{
	UNREF(context);

	unsigned long const updateTimeMs = static_cast<unsigned long>(ConfigServerGame::getPvpUpdateTimeMs());
	PvpInternal::updateTimedFlags(updateTimeMs);
	getScheduler().setCallback(Pvp::updateTimedFlags, nullptr, updateTimeMs);
}

// ----------------------------------------------------------------------

Scheduler &Pvp::getScheduler()
{
	FATAL(!s_pvpScheduler, ("No Pvp scheduler found"));
	return *s_pvpScheduler;
}

// ----------------------------------------------------------------------

Pvp::PvpRankInfo const & Pvp::getRankInfo(int rating)
{
	if (rating < s_minRatingForRank)
		return s_RankNone;

	std::map<int, Pvp::PvpRankInfo *>::const_iterator iterFind = s_RankData.lower_bound(rating);
	if (iterFind == s_RankData.end())
		return s_RankNone;

	return *(iterFind->second);
}

// ----------------------------------------------------------------------

int Pvp::getMinRatingForRank()
{
	return s_minRatingForRank;
}

// ----------------------------------------------------------------------

int Pvp::getMaxRatingForRank()
{
	return s_maxRatingForRank;
}

// ----------------------------------------------------------------------

std::string const & Pvp::debugGetGcwRankTable()
{
	return s_debugGcwRankTable;
}

// ----------------------------------------------------------------------

std::string const & Pvp::debugGetGcwFormulasTable()
{
	return s_debugGcwFormulasTable;
}

// ----------------------------------------------------------------------

std::string const & Pvp::debugGetGcwRankDecayExclusion()
{
	return s_debugGcwRankDecayExclusion;
}

// ----------------------------------------------------------------------

int Pvp::calculateRatingAdjustment(int const gcwPoints, int const currentRating, int &totalEarnedRating, int &totalEarnedRatingAfterDecay, int &cappedRatingAdjustment)
{
	totalEarnedRating = 0;
	totalEarnedRatingAfterDecay = 0;
	cappedRatingAdjustment = 0;

	// get current rank
	Pvp::PvpRankInfo const & rankInfo = getRankInfo(currentRating);
	if (rankInfo.rank <= 0)
		return 0;

	// nothing to do if no points to convert and current rank doesn't decay
	if ((gcwPoints <= 0) && (rankInfo.ratingDecayBalance <= 0))
		return 0;

	// convert points to rating
	if (gcwPoints > 0)
	{
		// round any fractional rating up so that no hard earned GCW points are "lost"
		int64 numerator = static_cast<int64>(gcwPoints) * static_cast<int64>(rankInfo.ratingEarningCap);
		int64 denominator = static_cast<int64>(gcwPoints) + static_cast<int64>(rankInfo.ratingEarningCap);
		totalEarnedRating = static_cast<int>(numerator / denominator);
		if (numerator % denominator)
			++totalEarnedRating;

		// if the rank is a decayable rank, and the GCW points is more than
		// what is required to offset rank decay, use a different formula
		// to calculate the additional rank progress
		if ((rankInfo.ratingDecayBalance > 0) && (totalEarnedRating >= rankInfo.ratingDecayBalance) && (rankInfo.pointToOffsetDecay > 0) && (gcwPoints > rankInfo.pointToOffsetDecay))
		{
			numerator = static_cast<int64>(gcwPoints - rankInfo.pointToOffsetDecay) * static_cast<int64>(rankInfo.ratingEarningCap - rankInfo.ratingDecayBalance + 1000);
			denominator = static_cast<int64>(gcwPoints - rankInfo.pointToOffsetDecay) + static_cast<int64>(rankInfo.ratingEarningCap - rankInfo.ratingDecayBalance + 1000);
			totalEarnedRating = static_cast<int>(numerator / denominator);
			if (numerator % denominator)
				++totalEarnedRating;

			totalEarnedRating += rankInfo.ratingDecayBalance;
		}
	}
	else
	{
		totalEarnedRating = 0;
	}

	// apply rating decay, if any
	totalEarnedRatingAfterDecay = totalEarnedRating - rankInfo.ratingDecayBalance;

	// if rating loss from decay, cap loss
	cappedRatingAdjustment = std::max(-rankInfo.maxRatingDecay, totalEarnedRatingAfterDecay);

	// if there's a decay floor, make sure the rating decay doesn't cause the rating to fall below the floor
	int finalRatingAdjustment = cappedRatingAdjustment;
	if ((rankInfo.ratingDecayFloor > 0) && (finalRatingAdjustment < 0) && ((currentRating + finalRatingAdjustment) < rankInfo.ratingDecayFloor))
		finalRatingAdjustment = rankInfo.ratingDecayFloor - currentRating;

	return finalRatingAdjustment;
}

// ----------------------------------------------------------------------

bool Pvp::isInGcwRankDecayExclusionInterval(time_t t)
{
	for (std::multimap<time_t, time_t>::const_iterator iter = s_RankDecayExclusion.lower_bound(t); iter != s_RankDecayExclusion.end(); ++iter)
	{
		if (iter->second <= t)
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------

Pvp::GcwScoreCategory const * Pvp::getGcwScoreCategory(std::string const & scoreCategory)
{
	std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator const iterFind = s_gcwScoreCategory.find(scoreCategory);
	if (iterFind != s_gcwScoreCategory.end())
		return iterFind->second;

	return nullptr;
}

// ----------------------------------------------------------------------

Pvp::GcwScoreCategory const * Pvp::getGcwScoreDefaultCategoryForPlanet(std::string const & planetName)
{
	std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator const iterFind = s_gcwScoreDefaultCategoryForPlanet.find(planetName);
	if (iterFind != s_gcwScoreDefaultCategoryForPlanet.end())
		return iterFind->second;

	return nullptr;
}

// ----------------------------------------------------------------------

std::map<std::string, Pvp::GcwScoreCategory const *> const & Pvp::getAllGcwScoreCategory()
{
	return s_gcwScoreCategory;
}

// ----------------------------------------------------------------------

std::map<std::string, std::map<std::string, std::pair<std::pair<float, float>, float> > > const & Pvp::getGcwScoreCategoryRegions()
{
	return s_gcwScoreCategoryRegions;
}

// ----------------------------------------------------------------------

std::string const & Pvp::getGcwScoreCategoryRegion(std::string const & planetName, Vector const & position_w)
{
	std::map<std::string, std::map<std::string, std::pair<std::pair<float, float>, float> > >::const_iterator const iterPlanet = s_gcwScoreCategoryRegions.find(planetName);
	if (iterPlanet != s_gcwScoreCategoryRegions.end())
	{
		static std::vector<const Region *> regions;
		regions.clear();

		RegionMaster::getRegionsAtPoint(planetName, position_w.x, position_w.z, regions);

		std::map<std::string, std::pair<std::pair<float, float>, float> >::const_iterator iterFind;
		for (std::vector<const Region *>::const_iterator iter = regions.begin(); iter != regions.end(); ++iter)
		{
			iterFind = iterPlanet->second.find(Unicode::wideToNarrow((*iter)->getName()));
			if (iterFind != iterPlanet->second.end())
				return iterFind->first;
		}
	}

	static std::string const empty;
	return empty;
}

// ----------------------------------------------------------------------

std::map<std::string, std::map<std::string, int> > const & Pvp::getGcwScoreCategoryGroups()
{
	return s_gcwScoreCategoryGroups;
}

// ----------------------------------------------------------------------

bool Pvp::getGcwScoreCategoryGroupTotalPoints(std::string const & group, std::string const & category, int & totalPoints)
{
	std::map<std::string, std::map<std::string, int> >::const_iterator iterFindGroup = s_gcwScoreCategoryGroups.find(group);
	if (iterFindGroup != s_gcwScoreCategoryGroups.end())
	{
		std::map<std::string, int>::const_iterator iterFindCategory = iterFindGroup->second.find(category);
		if (iterFindCategory != iterFindGroup->second.end())
		{
			totalPoints = iterFindCategory->second;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

int Pvp::calculateGcwImperialScorePercentile(std::string const & gcwCategory, std::map<std::string, std::pair<int64, int64> > const & gcwImperialScore, std::map<std::string, std::pair<int64, int64> > const & gcwRebelScore)
{
	if (!getGcwScoreCategory(gcwCategory))
		return 50; // tie

	uint64 totalImperial = 0ull;
	{
		std::map<std::string, std::pair<int64, int64> >::const_iterator const iterImperial = gcwImperialScore.find(gcwCategory);
		if (iterImperial != gcwImperialScore.end())
			totalImperial = static_cast<uint64>(std::max(0ll, iterImperial->second.first)) + static_cast<uint64>(std::max(0ll, iterImperial->second.second));
	}

	uint64 total = totalImperial;
	{
		std::map<std::string, std::pair<int64, int64> >::const_iterator const iterRebel = gcwRebelScore.find(gcwCategory);
		if (iterRebel != gcwRebelScore.end())
			total += (static_cast<uint64>(std::max(0ll, iterRebel->second.first)) + static_cast<uint64>(std::max(0ll, iterRebel->second.second)));
	}

	// if the score is less than 1, treat it as a tie; note that score is scaled by 1000
	if (total < static_cast<uint64>(c_gcwScoreScaleFactor))
		return 50; // tie

	return static_cast<int>(totalImperial * 100ull / total);
}

// ----------------------------------------------------------------------

int Pvp::calculateGcwImperialScorePercentile(std::string const & gcwCategory, std::map<std::string, std::pair<uint64, uint64> > const & gcwImperialScore, std::map<std::string, std::pair<uint64, uint64> > const & gcwRebelScore)
{
	if (!getGcwScoreCategory(gcwCategory))
		return 50; // tie

	uint64 totalImperial = 0ull;
	{
		std::map<std::string, std::pair<uint64, uint64> >::const_iterator const iterImperial = gcwImperialScore.find(gcwCategory);
		if (iterImperial != gcwImperialScore.end())
			totalImperial = iterImperial->second.first + iterImperial->second.second;
	}

	uint64 total = totalImperial;
	{
		std::map<std::string, std::pair<uint64, uint64> >::const_iterator const iterRebel = gcwRebelScore.find(gcwCategory);
		if (iterRebel != gcwRebelScore.end())
			total += (iterRebel->second.first + iterRebel->second.second);
	}

	// if the score is less than 1, treat it as a tie; note that score is scaled by 1000
	if (total < static_cast<uint64>(c_gcwScoreScaleFactor))
		return 50; // tie

	return static_cast<int>(totalImperial * 100ull / total);
}

// ----------------------------------------------------------------------

float Pvp::getGcwDefenderRegionImperialBonus(std::string const & scoreCategory)
{
	GuildObject const * const go = ServerUniverse::getInstance().getMasterGuildObject();
	if (go)
		return go->getGcwDefenderRegionImperialBonus(scoreCategory);

	return 0.0f;
}

// ----------------------------------------------------------------------

float Pvp::getGcwDefenderRegionRebelBonus(std::string const & scoreCategory)
{
	GuildObject const * const go = ServerUniverse::getInstance().getMasterGuildObject();
	if (go)
		return go->getGcwDefenderRegionRebelBonus(scoreCategory);

	return 0.0f;
}

// ----------------------------------------------------------------------

bool Pvp::getGcwDefenderRegionBonus(CreatureObject const & creature, PlayerObject const & player, float & bonus)
{
	std::string const & playerCurrentGcwRegion = player.getCurrentGcwRegion();
	if (playerCurrentGcwRegion.empty())
		return false;

	{
		std::pair<std::string, std::pair<bool, bool> > const & cityGcwDefenderRegionInfo = player.getCityGcwDefenderRegionInfo();
		if (!cityGcwDefenderRegionInfo.first.empty() && cityGcwDefenderRegionInfo.second.first && (playerCurrentGcwRegion == cityGcwDefenderRegionInfo.first))
		{
			uint32 const faction = creature.getPvpFaction();
			if (PvpData::isImperialFactionId(faction))
			{
				bonus = Pvp::getGcwDefenderRegionImperialBonus(playerCurrentGcwRegion);
				return true;
			}
			else if (PvpData::isRebelFactionId(faction))
			{
				bonus = Pvp::getGcwDefenderRegionRebelBonus(playerCurrentGcwRegion);
				return true;
			}
		}
	}

	{
		std::pair<std::string, std::pair<bool, bool> > const & guildGcwDefenderRegionInfo = player.getGuildGcwDefenderRegionInfo();
		if (!guildGcwDefenderRegionInfo.first.empty() && guildGcwDefenderRegionInfo.second.first && (playerCurrentGcwRegion == guildGcwDefenderRegionInfo.first))
		{
			uint32 const faction = creature.getPvpFaction();
			if (PvpData::isImperialFactionId(faction))
			{
				bonus = Pvp::getGcwDefenderRegionImperialBonus(playerCurrentGcwRegion);
				return true;
			}
			else if (PvpData::isRebelFactionId(faction))
			{
				bonus = Pvp::getGcwDefenderRegionRebelBonus(playerCurrentGcwRegion);
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

void PvpNamespace::loadGcwRankTable()
{
	s_RankNone.dataTableRow = -1;
	s_RankNone.rank = 0;
	s_RankNone.minRating = -1;
	s_RankNone.maxRating = -1;
	s_RankNone.imperialTitle = std::string("imperial_none");
	s_RankNone.rebelTitle = std::string("rebel_none");
	s_RankNone.ratingEarningCap = 0;
	s_RankNone.ratingDecayBalance = 0;
	s_RankNone.maxRatingDecay = 0;
	s_RankNone.ratingDecayFloor = 0;
	s_RankNone.pointToOffsetDecay = 0;

	for (std::map<int, Pvp::PvpRankInfo *>::const_iterator iter = s_RankData.begin(); iter != s_RankData.end(); ++iter)
		delete iter->second;

	s_RankData.clear();

	{
		DataTable * data = NON_NULL(DataTableManager::getTable(s_gcwRankTableName, true));
		int numRows = data->getNumRows();
		Pvp::PvpRankInfo * pvpRankInfo;
		for (int row=0; row<numRows; ++row)
		{
			pvpRankInfo = new Pvp::PvpRankInfo();
			pvpRankInfo->dataTableRow = row;
			pvpRankInfo->rank = data->getIntValue("Rank", row);
			pvpRankInfo->minRating = data->getIntValue("MinRating", row);
			pvpRankInfo->maxRating = data->getIntValue("MaxRating", row);
			pvpRankInfo->imperialTitle = data->getStringValue("ImperialTitle", row);
			pvpRankInfo->rebelTitle = data->getStringValue("RebelTitle", row);
			pvpRankInfo->ratingEarningCap = data->getIntValue("RatingEarningCap", row);
			pvpRankInfo->ratingDecayBalance = data->getIntValue("RatingDecayBalance", row);
			pvpRankInfo->maxRatingDecay = data->getIntValue("MaxRatingDecay", row);
			pvpRankInfo->ratingDecayFloor = data->getIntValue("RatingDecayFloor", row);

			// begin per-row data consistency check
			FATAL((pvpRankInfo->rank < 1), ("%s, row (%d), Rank (%d) must be >= 1", s_gcwRankTableName.c_str(), row, pvpRankInfo->rank));
			FATAL((pvpRankInfo->minRating < 0), ("%s, row (%d), MinRating (%d) must be >= 0", s_gcwRankTableName.c_str(), row, pvpRankInfo->minRating));
			FATAL((pvpRankInfo->maxRating < 0), ("%s, row (%d), MaxRating (%d) must be >= 0", s_gcwRankTableName.c_str(), row, pvpRankInfo->maxRating));
			FATAL((pvpRankInfo->minRating >= pvpRankInfo->maxRating), ("%s, row (%d), MinRating (%d) must be less than MaxRating (%d)", s_gcwRankTableName.c_str(), row, pvpRankInfo->minRating, pvpRankInfo->maxRating));
			FATAL((pvpRankInfo->ratingEarningCap < 0), ("%s, row (%d), RatingEarningCap (%d) must be >= 0", s_gcwRankTableName.c_str(), row, pvpRankInfo->ratingEarningCap));
			FATAL((pvpRankInfo->ratingDecayBalance < 0), ("%s, row (%d), RatingDecayBalance (%d) must be >= 0", s_gcwRankTableName.c_str(), row, pvpRankInfo->ratingDecayBalance));
			FATAL((pvpRankInfo->maxRatingDecay < 0), ("%s, row (%d), MaxRatingDecay (%d) must be >= 0", s_gcwRankTableName.c_str(), row, pvpRankInfo->maxRatingDecay));
			FATAL((pvpRankInfo->ratingDecayFloor < 0), ("%s, row (%d), RatingDecayFloor (%d) must be >= 0", s_gcwRankTableName.c_str(), row, pvpRankInfo->ratingDecayFloor));
			// end per-row data consistency check

			s_RankData[pvpRankInfo->maxRating] = pvpRankInfo;
		}

		DataTableManager::close(s_gcwRankTableName);
	}

	// start inter-row data consistency check
	s_minRatingForRank = 0;
	s_maxRatingForRank = 0;
	int previousRank = 0;

	std::map<int, Pvp::PvpRankInfo *>::iterator iter;
	for (iter = s_RankData.begin(); iter != s_RankData.end(); ++iter)
	{
		// first rank must have Rank = 1 and MinRating = 0
		if (iter == s_RankData.begin())
		{
			FATAL((iter->second->rank != 1), ("%s, row (%d), first Rank (%d) must be 1", s_gcwRankTableName.c_str(), iter->second->dataTableRow, iter->second->rank));
			FATAL((iter->second->minRating != 0), ("%s, row (%d), first rank's MinRating (%d) must be 0", s_gcwRankTableName.c_str(), iter->second->dataTableRow, iter->second->minRating));

			s_minRatingForRank = iter->second->minRating;
		}
		// subsequent rank must have Rank = <previous rank> + 1
		// and MinRating = <previous rank MaxRating> + 1
		else
		{
			FATAL(((previousRank + 1) != iter->second->rank), ("%s, row (%d), Rank (%d) must be 1 more than previous Rank (%d)", s_gcwRankTableName.c_str(), iter->second->dataTableRow, iter->second->rank, previousRank));
			FATAL(((s_maxRatingForRank + 1) != iter->second->minRating), ("%s, row (%d), MinRating (%d) must be 1 more than previous MaxRating (%d)", s_gcwRankTableName.c_str(), iter->second->dataTableRow, iter->second->minRating, s_maxRatingForRank));
		}

		s_maxRatingForRank = iter->second->maxRating;
		previousRank = iter->second->rank;

		// calculate pointToOffsetDecay for each decayable rank
		if (iter->second->ratingDecayBalance > 0)
			iter->second->pointToOffsetDecay = static_cast<int>((static_cast<int64>(iter->second->ratingEarningCap) * static_cast<int64>(iter->second->ratingDecayBalance - 1)) / static_cast<int64>(iter->second->ratingEarningCap - iter->second->ratingDecayBalance + 1)) + 1;
		else
			iter->second->pointToOffsetDecay = 0;
	}
	// end inter-row data consistency check

	char buffer[2048];
	s_debugGcwRankTable  = "Rank    Required Rating    Imperial Title / Rebel Title\n";
	s_debugGcwRankTable += "====    ===============    ============================\n";

	s_debugGcwFormulasTable  = "Rank RatingEarningCap RatingDecayBalance MaxRatingDecay RatingDecayFloor PointToOffsetDecay\n";
	s_debugGcwFormulasTable += "==== ================ ================== ============== ================ ==================\n";

	for (iter = s_RankData.begin(); iter != s_RankData.end(); ++iter)
	{
		snprintf(buffer, sizeof(buffer)-1, "%2d      %6d - %6d    %s / %s\n", iter->second->rank, iter->second->minRating, iter->second->maxRating, iter->second->imperialTitle.c_str(), iter->second->rebelTitle.c_str());
		buffer[sizeof(buffer)-1] = '\0';
		s_debugGcwRankTable += buffer;

		snprintf(buffer, sizeof(buffer)-1, "%2d   %16d %18d %14d %16d %18d\n", iter->second->rank, iter->second->ratingEarningCap, iter->second->ratingDecayBalance, iter->second->maxRatingDecay, iter->second->ratingDecayFloor, iter->second->pointToOffsetDecay);
		buffer[sizeof(buffer)-1] = '\0';
		s_debugGcwFormulasTable += buffer;
	}

	snprintf(buffer, sizeof(buffer)-1, "minimum rating for rank: %d\n", s_minRatingForRank);
	buffer[sizeof(buffer)-1] = '\0';
	s_debugGcwRankTable += buffer;

	snprintf(buffer, sizeof(buffer)-1, "maximum rating for rank: %d\n", s_maxRatingForRank);
	buffer[sizeof(buffer)-1] = '\0';
	s_debugGcwRankTable += buffer;
}

// ----------------------------------------------------------------------

void PvpNamespace::loadGcwRankDecayExclusionTable()
{
	s_RankDecayExclusion.clear();

	int intervalBegin, intervalEnd;
	{
		DataTable * data = NON_NULL(DataTableManager::getTable(s_gcwRankDecayExclusionTableName, true));
		int numRows = data->getNumRows();
		for (int row=0; row<numRows; ++row)
		{
			intervalBegin = data->getIntValue("IntervalBegin", row);
			intervalEnd = data->getIntValue("IntervalEnd", row);

			FATAL((intervalBegin <= 0), ("%s, row (%d), IntervalBegin (%d) must be >= 1", s_gcwRankDecayExclusionTableName.c_str(), row, intervalBegin));
			FATAL((intervalEnd <= 0), ("%s, row (%d), IntervalEnd (%d) must be >= 1", s_gcwRankDecayExclusionTableName.c_str(), row, intervalEnd));
			FATAL((intervalBegin >= intervalEnd), ("%s, row (%d), IntervalBegin (%d) must be less than IntervalEnd (%d)", s_gcwRankDecayExclusionTableName.c_str(), row, intervalBegin, intervalEnd));

			IGNORE_RETURN(s_RankDecayExclusion.insert(std::make_pair(static_cast<time_t>(intervalEnd), static_cast<time_t>(intervalBegin))));
		}

		DataTableManager::close(s_gcwRankDecayExclusionTableName);
	}

	// read exclusion intervals from config file
	static int const min = std::numeric_limits<int>::min();
	int index = 0;
	while (true)
	{
		intervalBegin = ConfigFile::getKeyInt("GameServer", "gcwRankDecayExclusionIntervalBegin", index, min);
		intervalEnd = ConfigFile::getKeyInt("GameServer", "gcwRankDecayExclusionIntervalEnd", index++, min);

		if ((intervalBegin == min) || (intervalEnd == min))
			break;

		FATAL((intervalBegin <= 0), ("[GameServer] config option gcwRankDecayExclusionIntervalBegin (%d) must be >= 1", intervalBegin));
		FATAL((intervalEnd <= 0), ("[GameServer] config option gcwRankDecayExclusionIntervalEnd (%d) must be >= 1", intervalEnd));
		FATAL((intervalBegin >= intervalEnd), ("[GameServer] config option gcwRankDecayExclusionIntervalBegin (%d) must be less than gcwRankDecayExclusionIntervalEnd (%d)", intervalBegin, intervalEnd));

		IGNORE_RETURN(s_RankDecayExclusion.insert(std::make_pair(static_cast<time_t>(intervalEnd), static_cast<time_t>(intervalBegin))));
	}

	char buffer[2048];
	s_debugGcwRankDecayExclusion  = "Begin                                                                      End\n";
	s_debugGcwRankDecayExclusion += "=====                                                                      ===\n";

	for (std::multimap<time_t, time_t>::const_iterator iter = s_RankDecayExclusion.begin(); iter != s_RankDecayExclusion.end(); ++iter)
	{
		snprintf(buffer, sizeof(buffer)-1, "%10ld (%s) (%s) %10ld (%s) (%s)\n", iter->second, CalendarTime::convertEpochToTimeStringGMT(iter->second).c_str(), CalendarTime::convertEpochToTimeStringLocal(iter->second).c_str(), iter->first, CalendarTime::convertEpochToTimeStringGMT(iter->first).c_str(), CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str());
		buffer[sizeof(buffer)-1] = '\0';
		s_debugGcwRankDecayExclusion += buffer;
	}
}

// ----------------------------------------------------------------------

void PvpNamespace::loadGcwScoreCategoryTable()
{
	if (!s_gcwScoreCategory.empty())
		return;

	DataTable * data = NON_NULL(DataTableManager::getTable(s_gcwScoreCategoryTableName, true));

	int const columnCategoryName = data->findColumnNumber("CategoryName");
	FATAL((columnCategoryName < 0), ("column \"CategoryName\" not found in %s", s_gcwScoreCategoryTableName.c_str()));

	int const columnCategoryScoreWeight = data->findColumnNumber("CategoryScoreWeight");
	FATAL((columnCategoryScoreWeight < 0), ("column \"CategoryScoreWeight\" not found in %s", s_gcwScoreCategoryTableName.c_str()));

	int const columnRegionPlanet = data->findColumnNumber("RegionPlanet");
	FATAL((columnRegionPlanet < 0), ("column \"RegionPlanet\" not found in %s", s_gcwScoreCategoryTableName.c_str()));

	int const columnGcwRegionDefender = data->findColumnNumber("GcwRegionDefender");
	FATAL((columnGcwRegionDefender < 0), ("column \"GcwRegionDefender\" not found in %s", s_gcwScoreCategoryTableName.c_str()));

	int const columnNotifyOnPercentileChange = data->findColumnNumber("NotifyOnPercentileChange");
	FATAL((columnNotifyOnPercentileChange < 0), ("column \"NotifyOnPercentileChange\" not found in %s", s_gcwScoreCategoryTableName.c_str()));

	// the can be a variable number of "GcwGroup" columns, as long as the
	// columns are named GcwGroup1, GcwGroup2, GcwGroup3, GcwGroup4,
	// GcwGroup5, and so on
	std::vector<int> columnGcwGroup;
	char buffer[128];
	int columnNumber;
	for (int i = 1; i <= 1000000000; ++i)
	{
		snprintf(buffer, sizeof(buffer)-1, "GcwGroup%d", i);
		buffer[sizeof(buffer)-1] = '\0';

		columnNumber = data->findColumnNumber(buffer);
		if (columnNumber < 0)
			break;

		columnGcwGroup.push_back(columnNumber);
	}

	int const numRows = data->getNumRows();
	Pvp::GcwScoreCategory * gcwScoreCategory;
	int const cMaxCategoryScoreWeight = 1000000;
	int categoryScoreWeightBase, categoryScoreWeight;
	std::string::size_type posCategoryScoreWeightOverride;
	std::string regionPlanet;
	Region const * region;
	RegionCircle const * regionCircle;
	float regionCenterX, regionCenterZ, regionRadius;
	std::string gcwGroup;
	std::map<std::string, int> gcwGroupTotalCategoryScoreWeight;
	std::map<std::string, int>::iterator iterFindGcwGroupTotalWeight;
	for (int row=0; row<numRows; ++row)
	{
		gcwScoreCategory = new Pvp::GcwScoreCategory();

		gcwScoreCategory->categoryName = data->getStringValue(columnCategoryName, row);
		if (gcwScoreCategory->categoryName.empty())
		{
			delete gcwScoreCategory;
			continue;
		}

		categoryScoreWeightBase = data->getIntValue(columnCategoryScoreWeight, row);

		regionPlanet = data->getStringValue(columnRegionPlanet, row);
		if (!regionPlanet.empty())
		{
			region = RegionMaster::getRegionByName(regionPlanet, Unicode::narrowToWide(gcwScoreCategory->categoryName));
			if (region)
			{
				regionCircle = dynamic_cast<RegionCircle const *>(region);
				if (regionCircle)
				{
					regionCircle->getExtent(regionCenterX, regionCenterZ, regionRadius);
					(s_gcwScoreCategoryRegions[regionPlanet])[gcwScoreCategory->categoryName] = std::make_pair(std::make_pair(regionCenterX, regionCenterZ), regionRadius);						
				}
				else
				{
					FATAL(true, ("%s, row (%d), GCW score category (%s) must be a circle region", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwScoreCategory->categoryName.c_str()));
				}

				FATAL((region->getGeography() != RegionNamespace::RG_fictional), ("%s, row (%d), GCW score category (%s) must be a fictional region", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwScoreCategory->categoryName.c_str()));
				FATAL(region->getMunicipal(), ("%s, row (%d), GCW score category (%s) must not be a municipal region", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwScoreCategory->categoryName.c_str()));
				FATAL(region->isVisible(), ("%s, row (%d), GCW score category (%s) must not be a visible region", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwScoreCategory->categoryName.c_str()));
				FATAL(region->getNotify(), ("%s, row (%d), GCW score category (%s) must not be a notify region", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwScoreCategory->categoryName.c_str()));
			}
			else
			{
				FATAL(true, ("%s, row (%d), GCW score category (%s) has no region defined in the corresponding planet region file", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwScoreCategory->categoryName.c_str()));
			}
		}

		gcwScoreCategory->gcwRegionDefender = (data->getIntValue(columnGcwRegionDefender, row) != 0);
		gcwScoreCategory->notifyOnPercentileChange = (data->getIntValue(columnNotifyOnPercentileChange, row) != 0);

		// read all GCW group for GCW score category
		gcwScoreCategory->gcwScoreCategoryGroups.clear();
		for (std::vector<int>::const_iterator iterColumnGcwGroup = columnGcwGroup.begin(); iterColumnGcwGroup != columnGcwGroup.end(); ++iterColumnGcwGroup)
		{
			gcwGroup = data->getStringValue(*iterColumnGcwGroup, row);

			if (gcwGroup.empty())
				continue;

			categoryScoreWeight = categoryScoreWeightBase;

			// see if there's a category score weight override and use the override value
			posCategoryScoreWeightOverride = gcwGroup.find(':');
			if (posCategoryScoreWeightOverride != std::string::npos)
			{
				categoryScoreWeight = ::atoi(gcwGroup.substr(posCategoryScoreWeightOverride + 1).c_str());
				gcwGroup = gcwGroup.substr(0, posCategoryScoreWeightOverride);

				FATAL(((categoryScoreWeight < 1) || (categoryScoreWeight > cMaxCategoryScoreWeight)), ("%s, row (%d), GCW score category (%s) override category score weight for GCW group (%s) must be 1 - %d inclusive", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwScoreCategory->categoryName.c_str(), gcwGroup.c_str(), cMaxCategoryScoreWeight));
			}

			if (gcwGroup.empty())
				continue;

			FATAL((gcwScoreCategory->gcwScoreCategoryGroups.count(gcwGroup) > 0), ("%s, row (%d), duplicate GCW group (%s) defined for GCW score category (%s)", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwGroup.c_str(), gcwScoreCategory->categoryName.c_str()));

			IGNORE_RETURN(gcwScoreCategory->gcwScoreCategoryGroups.insert(gcwGroup));
			IGNORE_RETURN(s_gcwScoreCategoryGroups[gcwGroup].insert(std::make_pair(gcwScoreCategory->categoryName, categoryScoreWeight)));

			iterFindGcwGroupTotalWeight = gcwGroupTotalCategoryScoreWeight.find(gcwGroup);
			if (iterFindGcwGroupTotalWeight != gcwGroupTotalCategoryScoreWeight.end())
			{
				iterFindGcwGroupTotalWeight->second += categoryScoreWeight;
			}
			else
			{
				gcwGroupTotalCategoryScoreWeight.insert(std::make_pair(gcwGroup, categoryScoreWeight));
			}
		}

		FATAL((!gcwScoreCategory->gcwScoreCategoryGroups.empty() && ((categoryScoreWeightBase < 1) || (categoryScoreWeightBase > cMaxCategoryScoreWeight))), ("%s, row (%d), GCW score category (%s) that is in a GCW group must have CategoryScoreWeight value of 1 - %d inclusive", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwScoreCategory->categoryName.c_str(), cMaxCategoryScoreWeight));
		FATAL((gcwScoreCategory->gcwScoreCategoryGroups.empty() && (categoryScoreWeightBase != 0)), ("%s, row (%d), GCW score category (%s) that is NOT in any GCW group must have CategoryScoreWeight value of 0", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwScoreCategory->categoryName.c_str()));

		FATAL((s_gcwScoreCategory.count(gcwScoreCategory->categoryName) > 0), ("%s, row (%d), duplicate GCW score category (%s)", s_gcwScoreCategoryTableName.c_str(), (row+3), gcwScoreCategory->categoryName.c_str()));

		IGNORE_RETURN(s_gcwScoreCategory.insert(std::make_pair(gcwScoreCategory->categoryName, gcwScoreCategory)));
	}

	DataTableManager::close(s_gcwScoreCategoryTableName);

	// for all the GCW groups, calculate the amount that each GCW category in the group
	// contribute to the GCW group score; the total available score for a GCW group is
	// 1,000,000,000, which will be divided up among the GCW category in the group based
	// on the GCW category's categoryScoreWeight value
	std::map<std::string, std::map<std::string, int> >::iterator iterFindGcwGroup;
	int const cGcwGroupScoreTotal = 1000000000;
	int gcwGroupScoreRemaining;
	for (std::map<std::string, int>::const_iterator iterGcwGroupTotalWeight = gcwGroupTotalCategoryScoreWeight.begin(); iterGcwGroupTotalWeight != gcwGroupTotalCategoryScoreWeight.end(); ++iterGcwGroupTotalWeight)
	{
		iterFindGcwGroup = s_gcwScoreCategoryGroups.find(iterGcwGroupTotalWeight->first);

		FATAL((iterFindGcwGroup == s_gcwScoreCategoryGroups.end()), ("GCW group (%s) not found in s_gcwScoreCategoryGroups", iterGcwGroupTotalWeight->first.c_str()));
		FATAL((iterFindGcwGroup->second.empty()), ("GCW group (%s) has no associated GCW score categories", iterGcwGroupTotalWeight->first.c_str()));

		gcwGroupScoreRemaining = cGcwGroupScoreTotal;
		for (std::map<std::string, int>::iterator iterGcwGroupWeight = iterFindGcwGroup->second.begin(); iterGcwGroupWeight != iterFindGcwGroup->second.end(); ++iterGcwGroupWeight)
		{
			iterGcwGroupWeight->second = static_cast<int>(static_cast<int64>(iterGcwGroupWeight->second) * static_cast<int64>(cGcwGroupScoreTotal) / static_cast<int64>(iterGcwGroupTotalWeight->second));
			gcwGroupScoreRemaining -= iterGcwGroupWeight->second;
		}

		FATAL((gcwGroupScoreRemaining < 0), ("GCW group (%s) used up more than its allotted %d points (%d points over the limit)", iterGcwGroupTotalWeight->first.c_str(), cGcwGroupScoreTotal, -gcwGroupScoreRemaining));

		// if there's any points left (from rounding), distribute it evenly amongst the GCW score categories
		while (gcwGroupScoreRemaining > 0)
		{
			for (std::map<std::string, int>::iterator iterGcwGroupWeight = iterFindGcwGroup->second.begin(); iterGcwGroupWeight != iterFindGcwGroup->second.end(); ++iterGcwGroupWeight)
			{
				++(iterGcwGroupWeight->second);
				--gcwGroupScoreRemaining;

				if (gcwGroupScoreRemaining <= 0)
					break;
			}
		}
	}

#ifdef _DEBUG
	// sanity check to make sure all GCW groups have allocated all
	// its 1,000,000,000 points amongst all its GCW score categories
	{
		for (std::map<std::string, std::map<std::string, int> >::const_iterator iterGroup = s_gcwScoreCategoryGroups.begin(); iterGroup != s_gcwScoreCategoryGroups.end(); ++iterGroup)
		{
			gcwGroupScoreRemaining = cGcwGroupScoreTotal;
			for (std::map<std::string, int>::const_iterator iterCategory = iterGroup->second.begin(); iterCategory != iterGroup->second.end(); ++iterCategory)
			{
				gcwGroupScoreRemaining -= iterCategory->second;
			}

			FATAL((gcwGroupScoreRemaining > 0), ("GCW group (%s) did not use up all its allotted %d points (%d points remaining)", iterGroup->first.c_str(), cGcwGroupScoreTotal, gcwGroupScoreRemaining));
			FATAL((gcwGroupScoreRemaining < 0), ("GCW group (%s) used up more than its allotted %d points (%d points over the limit)", iterGroup->first.c_str(), cGcwGroupScoreTotal, -gcwGroupScoreRemaining));
		}
	}
#endif

	// build list of default categories for the ground planets
	//
	// *****WARNING WARNING WARNING WARNING WARNING*****
	// this list must be kept in sync in
	// PvpNamespace::loadGcwScoreCategoryTable()
	// SwgCuiSpaceZoneMap::SwgCuiSpaceZoneMap()
	// SwgCuiHyperspaceMap::SwgCuiHyperspaceMap()
	// and gcw.scriptlib (validScenes and defaultRegions)
	// *****WARNING WARNING WARNING WARNING WARNING*****
	//
	s_gcwScoreDefaultCategoryForPlanet["tatooine"] = Pvp::getGcwScoreCategory("gcw_region_tatooine_13");
	s_gcwScoreDefaultCategoryForPlanet["corellia"] = Pvp::getGcwScoreCategory("gcw_region_corellia_13");
	s_gcwScoreDefaultCategoryForPlanet["dantooine"] = Pvp::getGcwScoreCategory("gcw_region_dantooine_16");
	s_gcwScoreDefaultCategoryForPlanet["dathomir"] = Pvp::getGcwScoreCategory("gcw_region_dathomir_13");
	s_gcwScoreDefaultCategoryForPlanet["endor"] = Pvp::getGcwScoreCategory("gcw_region_endor_15");
	s_gcwScoreDefaultCategoryForPlanet["lok"] = Pvp::getGcwScoreCategory("gcw_region_lok_13");
	s_gcwScoreDefaultCategoryForPlanet["naboo"] = Pvp::getGcwScoreCategory("gcw_region_naboo_13");
	s_gcwScoreDefaultCategoryForPlanet["rori"] = Pvp::getGcwScoreCategory("gcw_region_rori_13");
	s_gcwScoreDefaultCategoryForPlanet["talus"] = Pvp::getGcwScoreCategory("gcw_region_talus_16");
	s_gcwScoreDefaultCategoryForPlanet["yavin4"] = Pvp::getGcwScoreCategory("gcw_region_yavin4_17");
	s_gcwScoreDefaultCategoryForPlanet["space_tatooine"] = Pvp::getGcwScoreCategory("gcw_region_tatooine_12");
	s_gcwScoreDefaultCategoryForPlanet["space_corellia"] = Pvp::getGcwScoreCategory("gcw_region_corellia_14");
	s_gcwScoreDefaultCategoryForPlanet["space_dantooine"] = Pvp::getGcwScoreCategory("gcw_region_dantooine_17");
	s_gcwScoreDefaultCategoryForPlanet["space_dathomir"] = Pvp::getGcwScoreCategory("gcw_region_dathomir_12");
	s_gcwScoreDefaultCategoryForPlanet["space_endor"] = Pvp::getGcwScoreCategory("gcw_region_endor_16");
	s_gcwScoreDefaultCategoryForPlanet["space_lok"] = Pvp::getGcwScoreCategory("gcw_region_lok_14");
	s_gcwScoreDefaultCategoryForPlanet["space_naboo"] = Pvp::getGcwScoreCategory("gcw_region_naboo_14");
	s_gcwScoreDefaultCategoryForPlanet["space_yavin4"] = Pvp::getGcwScoreCategory("gcw_region_yavin4_18");
}

// ======================================================================
