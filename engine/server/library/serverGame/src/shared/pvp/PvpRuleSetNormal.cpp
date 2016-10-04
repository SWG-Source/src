// ======================================================================
//
// PvpRuleSetNormal.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PvpRuleSetNormal.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PvpFactions.h"
#include "serverGame/PvpInternal.h"
#include "serverGame/TangibleObject.h"
#include "serverScript/GameScriptObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

bool PvpRuleSetNormal::canAttack(TangibleObject const &actor, TangibleObject const &target) const
{
	// people can never hurt themselves
	if (&actor == &target)
		return false;

	// non-pvpable and invulnerable objects cannot be attacked
	if (target.isNonPvpObject() || target.isInvulnerable())
		return false;

	bool const actorIsPlayer = PvpInternal::isPlayer(actor);
	bool const targetIsPlayer = PvpInternal::isPlayer(target);
	Pvp::FactionId targetFaction = PvpInternal::getAlignedFaction(target);
	Pvp::FactionId actorFaction = PvpInternal::getAlignedFaction(actor);
	Pvp::PvpType actorPvpType = actor.getPvpType();
	Pvp::PvpType targetPvpType = target.getPvpType();

	CreatureObject const *creatureActor = actor.asCreatureObject();
	CreatureObject const *creatureTarget = target.asCreatureObject();

	// if player character is neutral, see if he's a mercenary
	if (PvpData::isNeutralFactionId(targetFaction) && targetIsPlayer)
	{
		targetFaction = target.getPvpMercenaryFaction();
		targetPvpType = target.getPvpMercenaryType();
	}

	// if player character is neutral, see if he's a mercenary
	if (PvpData::isNeutralFactionId(actorFaction) && actorIsPlayer)
	{
		actorFaction = actor.getPvpMercenaryFaction();
		actorPvpType = actor.getPvpMercenaryType();
	}

	// only creatures can attack without having the pvpCanAttack objvar
	if (!creatureActor)
	{
		int pvpCanAttack = 0;
		if (!actor.getObjVars().getItem("pvpCanAttack", pvpCanAttack) || pvpCanAttack != 1)
			return false;
	}
	else if (creatureActor->getInvulnerabilityTimer() > 0.f)
		return false; // players cannot attack while their invulnerability timer is active

	// unattackable creatures may not be attacked
	if (PvpFactions::isUnattackableFaction(targetFaction))
		return false;
	if (PvpFactions::isUnattackableFaction(actorFaction))
		return false;

	// special forces NPC can only attack opposing special forces NPC or player
	// special forces NPC can only be attacked by opposing special forces NPC or player
	bool const actorIsSpecialForcesNPC = (!actorIsPlayer && actor.getObjVars().hasItem("huysMagicSpecialForcesOnlyObjvar"));
	bool const targetIsSpecialForcesNPC = (!targetIsPlayer && target.getObjVars().hasItem("huysMagicSpecialForcesOnlyObjvar"));
	if (actorIsSpecialForcesNPC || targetIsSpecialForcesNPC)
	{
		if (actorIsSpecialForcesNPC)
		{
			if (actorFaction != targetFaction)
			{
				// special forces NPC can attack opposing special forces NPC
				if (targetIsSpecialForcesNPC)
					return true;

				// special forces NPC can attack opposing special forces player
				if (targetIsPlayer && (targetPvpType == PvpType_Declared))
					return true;
			}

			return false;
		}

		if (actorFaction != targetFaction)
		{
			// special forces player can attack opposing special forces NPC
			if (actorIsPlayer && (actorPvpType == PvpType_Declared))
				return true;
		}

		return false;
	}

	// anyone may attack someone they are mutually dueling with
	if (PvpInternal::hasDuelEnemyFlag(target, actor.getNetworkId())
		&& PvpInternal::hasDuelEnemyFlag(actor, target.getNetworkId()))
		return true;

	// a bounty hunter can attack their target
	if (actorIsPlayer && targetIsPlayer && creatureActor && creatureTarget && creatureActor->hasBounty(*creatureTarget) && creatureActor->hasBountyMissionForTarget(creatureTarget->getNetworkId()))
		return true;

	if (targetIsPlayer && actorIsPlayer)
	{
		// noone may attack someone in their own group
		if (PvpInternal::inSameGroup(actor, target))
			return false;

		if (actorFaction != targetFaction)
		{
			//IF I'M DECLARED AND THEY ARE AS WELL, WE CAN ATTACK.
			if ((actorPvpType == PvpType_Declared) && (targetPvpType == PvpType_Declared))
			{
				return true;
			}
		}
	}

	// anyone may attack someone with a PEF against them (unless they are unattackable)
	if (PvpInternal::hasPersonalEnemyFlag(target, actor.getNetworkId()))
		return true;

	// Bubble NPCs may have first attack against people they have a PEF against.
	if (!actorIsPlayer && PvpFactions::isBubbleFaction(actorFaction))
	{
		if (PvpInternal::hasPersonalEnemyFlag(actor, target.getNetworkId()))
			return true;
	}

	// If we are in bubble combat mode, only the bubble participants are allowed to attack.
	// Bubble participants have a TEF so they would of hit the logic above us.
	if (PvpFactions::isBubbleFaction(targetFaction))
		return false;

	// noone may attack a bounty target unless they had a PEF against them,
	// which is why this check ***MUST*** be placed after the above
	// PvpInternal::hasPersonalEnemyFlag() check so that the actor with
	// the PEF against the bounty target can attack the bounty target
	if (PvpFactions::isBountyTargetFaction(targetFaction))
	{
		// defensive check just in case an NPC bounty target
		// didn't have a PEF for the bounty hunter set on it
		if (target.getScriptObject() && target.getScriptObject()->hasScript("systems.missions.dynamic.mission_bounty_target"))
		{
			NetworkId bhId;
			if (target.getObjVars().getItem("objHunter", bhId) && (bhId == actor.getNetworkId()))
			{
				/*
								static int sentinel = 0;

								// PvpInternal::setPermanentPersonalEnemyFlag() can cause PvpRuleSetNormal::canAttack()
								// to get called again; to prevent infinite recursion, don't call
								// PvpInternal::setPermanentPersonalEnemyFlag() again
								if (sentinel == 0)
								{
									sentinel = 1;

									// need to const cast because of updating target to set the PEF
									PvpInternal::setPermanentPersonalEnemyFlag(const_cast<TangibleObject &>(target), actor.getNetworkId());

									sentinel = 0;
								}
				*/

				return true;
			}
		}

		return false;
	}

	// guild and militia rules only apply to creature vs. creature
	if (creatureActor && creatureTarget)
	{
		// guild members may attack members of other guilds which have declared war on their guild,
		// but only if neither guild members is in a guild war "cool down" period
		if (GuildInterface::hasDeclaredWarAgainst(creatureTarget->getGuildId(), creatureActor->getGuildId())
			&& GuildInterface::hasDeclaredWarAgainst(creatureActor->getGuildId(), creatureTarget->getGuildId())
			&& creatureActor->getGuildWarEnabled()
			&& creatureTarget->getGuildWarEnabled()
			&& !PvpInternal::hasAnyGuildWarCoolDownPeriodEnemyFlag(*creatureActor)
			&& !PvpInternal::hasAnyGuildWarCoolDownPeriodEnemyFlag(*creatureTarget))
			return true;
	}

	if ((actorIsPlayer) && (!targetIsPlayer))
	{
		// PC attacker rules
		// anyone may attack npc targets with no faction opponents
		if (PvpFactions::getOpposingFactions(targetFaction).empty())
			return true;

		if (targetFaction != 0)
		{
			// so they're factional
			if (actorFaction != targetFaction)
			{
				//they have a differnent faction than me
				//am I neutral?
				if (actorPvpType == PvpType_Neutral)
				{
					return false;
				}
				return true;
			}
			return false;
		}
		else
		{
			return true;
		}
	}

	if (!actorIsPlayer)
	{
		// NPC attacker rules

		if (!actorFaction)
		{
			// A neutral NPC can attack anything
			return true;
		}
		else if ((!targetFaction) && (targetIsPlayer))
		{
			// The target is neutral and the target is a player, NPC can't attack
			return false;
		}
		else if ((!targetFaction) && (!targetIsPlayer))
		{
			// The target is neutral and the target is not a player, NPC can attack
			return true;
		}
		else if (targetFaction == actorFaction)
		{
			// NPCs can’t attack a target of the same faction
			return false;
		}
		else if (PvpFactions::isNonaggressiveFaction(actorFaction))
		{
			// Non-Agg creatures may not attack anything
			return false;
		}
		else if (targetPvpType == PvpType_Neutral)
		{
			// The player target is On Leave!!
			return false;
		}
		else
		{
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------

bool PvpRuleSetNormal::canHelp(TangibleObject const &actor, TangibleObject const &target) const
{
	// people can always help themselves
	if (&actor == &target)
		return true;

	bool const actorIsPlayer = PvpInternal::isPlayer(actor);
	bool const targetIsPlayer = PvpInternal::isPlayer(target);
	Pvp::FactionId targetFaction = PvpInternal::getAlignedFaction(target);
	Pvp::FactionId actorFaction = PvpInternal::getAlignedFaction(actor);
	Pvp::PvpType actorPvpType = actor.getPvpType();
	Pvp::PvpType targetPvpType = target.getPvpType();

	// if player character is neutral, see if he's a mercenary
	if (PvpData::isNeutralFactionId(targetFaction) && targetIsPlayer)
	{
		targetFaction = target.getPvpMercenaryFaction();
		targetPvpType = target.getPvpMercenaryType();
	}

	// if player character is neutral, see if he's a mercenary
	if (PvpData::isNeutralFactionId(actorFaction) && actorIsPlayer)
	{
		actorFaction = actor.getPvpMercenaryFaction();
		actorPvpType = actor.getPvpMercenaryType();
	}

	// npcs are allowed to help anyone
	if (!actorIsPlayer)
		return true;

	// help target not player and not pet
	if (!targetIsPlayer && !PvpInternal::isPet(target))
		return false;

	// Cannot help Jedi or bounty hunters that are currently engaged in combat with each other
	if (PvpInternal::hasAnyBountyDuelEnemyFlag(target))
		return false;
	if (PvpInternal::hasBubbleCombatFlag(target))
		return false;

	// Neutrals may only help neutrals
	if (actorPvpType == PvpType_Neutral)
	{
		if (targetPvpType != PvpType_Neutral)
			return false;
		else
			return true;
	}

	// Coverts may help neturals and other coverts of the same faction (but not overts)
	if (actorPvpType == PvpType_Covert)
	{
		if (targetPvpType == PvpType_Neutral)
			return true;
		if ((targetPvpType == PvpType_Covert) && (actorFaction == targetFaction))
			return true;
		return false;
	}

	// Overts may help neutrals and anyone of their faction
	if (actorPvpType == PvpType_Declared)
	{
		if (targetPvpType == PvpType_Neutral)
			return true;
		if (actorFaction == targetFaction)
			return true;
		return false;
	}

	return false;
}

// ----------------------------------------------------------------------

void PvpRuleSetNormal::getAttackRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const
{
	actorRepercussions.clear();
	targetRepercussions.clear();

	if (&actor == &target)
		return;

	// attacks between dueling characters only refresh duel flags
	// attacks between dueling characters have no repercussions
	if (PvpInternal::hasDuelEnemyFlag(target, actor.getNetworkId())
		&& PvpInternal::hasDuelEnemyFlag(actor, target.getNetworkId()))
		return;

	bool const actorIsPlayer = PvpInternal::isPlayer(actor);
	bool const targetIsPlayer = PvpInternal::isPlayer(target);
	CreatureObject const *creatureActor = actor.asCreatureObject();
	CreatureObject const *creatureTarget = target.asCreatureObject();

	if (actorIsPlayer && targetIsPlayer && creatureActor && creatureTarget)
	{
		// attacks between the bounty hunter and their target
		// takes precedence over any other factional considerations
		// and guild wars
		if ((creatureActor->hasBounty(*creatureTarget) && creatureActor->hasBountyMissionForTarget(creatureTarget->getNetworkId())) || (creatureTarget->hasBounty(*creatureActor) && creatureTarget->hasBountyMissionForTarget(creatureActor->getNetworkId())))
		{
			// bounty PEF are applied to both parties as soon as one party attacks
			actorRepercussions.push_back(PvpEnemy(target.getNetworkId(), PvpFactions::getBountyDuelFactionId(), -1));
			targetRepercussions.push_back(PvpEnemy(actor.getNetworkId(), PvpFactions::getBountyDuelFactionId(), -1));
			return;
		}

		// attacks between guild war members have no repercussions, but only if
		// both the guild war members are not in the guild war "cool down" period
		if (GuildInterface::hasDeclaredWarAgainst(creatureActor->getGuildId(), creatureTarget->getGuildId())
			&& GuildInterface::hasDeclaredWarAgainst(creatureTarget->getGuildId(), creatureActor->getGuildId())
			&& creatureActor->getGuildWarEnabled()
			&& creatureTarget->getGuildWarEnabled()
			&& !PvpInternal::hasAnyGuildWarCoolDownPeriodEnemyFlag(*creatureActor)
			&& !PvpInternal::hasAnyGuildWarCoolDownPeriodEnemyFlag(*creatureTarget))
			return;
	}

	Pvp::FactionId targetFaction = PvpInternal::getAlignedFaction(target);
	Pvp::FactionId actorFaction = PvpInternal::getAlignedFaction(actor);
	Pvp::PvpType actorPvpType = actor.getPvpType();
	Pvp::PvpType targetPvpType = target.getPvpType();

	// if player character is neutral, see if he's a mercenary
	if (PvpData::isNeutralFactionId(targetFaction) && targetIsPlayer)
	{
		targetFaction = target.getPvpMercenaryFaction();
		targetPvpType = target.getPvpMercenaryType();
	}

	// if player character is neutral, see if he's a mercenary
	if (PvpData::isNeutralFactionId(actorFaction) && actorIsPlayer)
	{
		actorFaction = actor.getPvpMercenaryFaction();
		actorPvpType = actor.getPvpMercenaryType();
	}

	// Only use factions for npcs and non-neutral players
	if (actorIsPlayer && (actorPvpType == PvpType_Neutral))
	{
		actorFaction = 0;
	}

	if (targetIsPlayer && (targetPvpType == PvpType_Neutral))
	{
		targetFaction = 0;
	}

	if (PvpFactions::isBubbleFaction(actorFaction) || PvpFactions::isBubbleFaction(targetFaction))
	{
		if (actorIsPlayer)
			actorRepercussions.push_back(PvpEnemy(target.getNetworkId(), PvpFactions::getBubbleCombatFactionId(), -1));
		else if (targetIsPlayer)
			targetRepercussions.push_back(PvpEnemy(actor.getNetworkId(), PvpFactions::getBubbleCombatFactionId(), -1));

		return;
	}

	// attacking an aligned gives you a timed FEF for their faction,
	// unless they are in a guild that has declared war on your guild and
	// both you and the aligned are not in the guild war "cool down" period
	if (targetFaction && targetFaction != actorFaction) // XXX
	{
		// if we get this far, we know that both you and the aligned
		// are not in a guild war and neither of you are in the
		// guild war "cool down" period
		actorRepercussions.push_back(PvpEnemy(NetworkId::cms_invalid, targetFaction, -1));
	}

	// attacking gives you a timed PEF against the target and any non-neutral members of their group
	// not aligned with your faction
	{
		// if a player is being attacked by an NPC while the player
		// is incapped (i.e. a deathblow performed while the player
		// is incapped), don't set any timed PEF on the NPC attacker,
		// as the player will be dead, so we don't want any timed PEF
		// between the player and the attacker NPC after the attacker
		// NPC kills the player
		//
		// if !(targetIsPlayer && !actorIsPlayer && creatureTarget && creatureTarget->isIncapacitated())
		if (!targetIsPlayer || actorIsPlayer || !creatureTarget || !creatureTarget->isIncapacitated())
			actorRepercussions.push_back(PvpEnemy(target.getNetworkId(), targetFaction, -1));

		// opposing factional PEF are applied to both parties as soon as one party attacks
		if (targetFaction && actorFaction && (targetFaction != actorFaction) && PvpInternal::isFactionAligned(targetFaction) && PvpInternal::isFactionAligned(actorFaction) && PvpInternal::areFactionsOpposed(actorFaction, targetFaction))
			targetRepercussions.push_back(PvpEnemy(actor.getNetworkId(), actorFaction, -1));
	}
}

// ----------------------------------------------------------------------

void PvpRuleSetNormal::getHelpRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const
{
	actorRepercussions.clear();
	targetRepercussions.clear();
}

// ----------------------------------------------------------------------

bool PvpRuleSetNormal::isEnemy(TangibleObject const &actor, TangibleObject const &target) const
{
	if (PvpInternal::hasPersonalEnemyFlag(target, actor.getNetworkId()))
		return true;

	// until they actually start fighting, a bounty hunter's target should
	// appear red to the bounty hunter, but not vice versa
	bool const actorIsPlayer = PvpInternal::isPlayer(actor);
	bool const targetIsPlayer = PvpInternal::isPlayer(target);
	CreatureObject const *creatureActor = actor.asCreatureObject();
	CreatureObject const *creatureTarget = target.asCreatureObject();
	if (actorIsPlayer && targetIsPlayer && creatureActor && creatureTarget && creatureTarget->hasBounty(*creatureActor) && creatureTarget->hasBountyMissionForTarget(creatureActor->getNetworkId()))
		return true;

	Pvp::FactionId targetFaction = PvpInternal::getAlignedFaction(target);
	Pvp::FactionId actorFaction = PvpInternal::getAlignedFaction(actor);
	Pvp::PvpType actorPvpType = actor.getPvpType();
	Pvp::PvpType targetPvpType = target.getPvpType();

	// if player character is neutral, see if he's a mercenary
	if (PvpData::isNeutralFactionId(targetFaction) && targetIsPlayer)
	{
		targetFaction = target.getPvpMercenaryFaction();
		targetPvpType = target.getPvpMercenaryType();
	}

	// if player character is neutral, see if he's a mercenary
	if (PvpData::isNeutralFactionId(actorFaction) && actorIsPlayer)
	{
		actorFaction = actor.getPvpMercenaryFaction();
		actorPvpType = actor.getPvpMercenaryType();
	}

	if (PvpFactions::isUnattackableFaction(actorFaction))
		return false;

	if (PvpFactions::isUnattackableFaction(targetFaction))
		return false;

	// Only use factions for npcs and non-neutral players
	if (actorIsPlayer && (actorPvpType == PvpType_Neutral))
	{
		actorFaction = 0;
	}

	// special forces NPC can only attack opposing special forces NPC or player
	// special forces NPC can only be attacked by opposing special forces NPC or player
	bool const actorIsSpecialForcesNPC = (!actorIsPlayer && actor.getObjVars().hasItem("huysMagicSpecialForcesOnlyObjvar"));
	bool const targetIsSpecialForcesNPC = (!targetIsPlayer && target.getObjVars().hasItem("huysMagicSpecialForcesOnlyObjvar"));
	if (actorIsSpecialForcesNPC || targetIsSpecialForcesNPC)
	{
		if (actorIsSpecialForcesNPC)
		{
			if (actorFaction != targetFaction)
			{
				// special forces NPC can attack opposing special forces NPC
				if (targetIsSpecialForcesNPC)
					return true;

				// special forces NPC can attack opposing special forces player
				if (targetIsPlayer && (targetPvpType == PvpType_Declared))
					return true;
			}

			return false;
		}

		if (actorFaction != targetFaction)
		{
			// special forces player can attack opposing special forces NPC
			if (actorIsPlayer && (actorPvpType == PvpType_Declared))
				return true;
		}

		return false;
	}

	// do duel and guild war checks before attackable player faction checks
	//   this is so covert opposite faction players who duel or guild war can still death blow correctly
	if (PvpInternal::hasDuelEnemyFlag(target, actor.getNetworkId())
		&& PvpInternal::hasDuelEnemyFlag(actor, target.getNetworkId()))
		return true;

	// guild and militia rules only apply to creature vs. creature
	if (creatureActor && creatureTarget)
	{
		// guild members are enemies of other guilds which have declared war on their guild,
		// but only if neither guild members is in a guild war "cool down" period
		// being an enemy sets the client's "default action" to death blow for incapacitated players in a guild war
		if (GuildInterface::hasDeclaredWarAgainst(creatureTarget->getGuildId(), creatureActor->getGuildId())
			&& GuildInterface::hasDeclaredWarAgainst(creatureActor->getGuildId(), creatureTarget->getGuildId())
			&& creatureActor->getGuildWarEnabled()
			&& creatureTarget->getGuildWarEnabled()
			&& !PvpInternal::hasAnyGuildWarCoolDownPeriodEnemyFlag(*creatureActor)
			&& !PvpInternal::hasAnyGuildWarCoolDownPeriodEnemyFlag(*creatureTarget))
			return true;
	}

	if (actorFaction)
	{
		//i'm not neutral
		if (targetFaction)
		{
			if (!targetIsPlayer)
			{
				return true;
			}
			else
			{
				if ((actorPvpType != PvpType_Neutral) && (targetPvpType != PvpType_Neutral) && (targetFaction != actorFaction))
				{
					if (actorIsPlayer)
					{
						if ((actorPvpType == PvpType_Declared) && (targetPvpType == PvpType_Declared))
						{
							return true;
						}
						return false;
					}
					else
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool PvpRuleSetNormal::isDuelingAllowed(TangibleObject const &actor, TangibleObject const &target) const
{
	bool const actorIsPlayer = PvpInternal::isPlayer(actor);
	bool const targetIsPlayer = PvpInternal::isPlayer(target);
	CreatureObject const * actorCreature = actor.asCreatureObject();
	CreatureObject const * targetCreature = target.asCreatureObject();

	return (actorCreature && actorIsPlayer && actorCreature->isInWorld() && !actorCreature->isInTutorial()
		&& targetCreature && targetIsPlayer && targetCreature->isInWorld() && !targetCreature->isInTutorial());
}

// ======================================================================