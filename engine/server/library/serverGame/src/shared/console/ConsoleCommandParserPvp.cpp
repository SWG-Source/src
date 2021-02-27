// ======================================================================
//
// ConsoleCommandParserPvp.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserPvp.h"

#include "UnicodeUtils.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/Pvp.h"
#include "serverGame/PvpInternal.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverScript/GameScriptObject.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/PvpData.h"

// ======================================================================
 
static const CommandParser::CmdInfo cmds[] =
{
	{"showGcwRankTable",                0, "",                           "Display the GCW rating/rank/rank title translation table"},
	{"showGcwFormulasTable",            0, "",                           "Display the GCW formulas table for calculating rating and rating decay"},
	{"showGcwRankDecayExclusionTimeTable", 0, "",                        "Display information about when GCW rank loss due to rank decay is disabled"},
	{"getGcwInfo",                      1, "<oid>",                      "Get the GCW info for the specified character"},
	{"adjustCurrentGcwPoint",           2, "<oid> <adjustment>",         "Adjust the current GCW point for the specified character (only factioned character has GCW point; doing this on a non-factioned character will set the character's GCW points to 0)"},
	{"adjustCurrentGcwRating",          2, "<oid> <adjustment>",         "Adjust the current GCW rating for the specified character (only factioned character has rating; doing this on a non-factioned character will set the character's rating to -1)"},
	{"adjustCurrentPvpKill",            2, "<oid> <adjustment>",         "Adjust the current PvP kill for the specified character"},
	{"adjustLifetimeGcwPoint",          2, "<oid> <adjustment>",         "Adjust the lifetime GCW point for the specified character"},
	{"adjustMaxGcwImperialRating",      2, "<oid> <adjustment>",         "Adjust the maximum attained GCW Imperial rating for the specified character"},
	{"adjustMaxGcwRebelRating",         2, "<oid> <adjustment>",         "Adjust the maximum attained GCW Rebel rating for the specified character"},
	{"adjustLifetimePvpKill",           2, "<oid> <adjustment>",         "Adjust the lifetime PvP kill for the specified character"},
	{"adjustNextGcwRatingCalcTime",     2, "<oid> <number of weeks>",    "If the character has current GCW points or is at a decayable rank, adjust the time that the next GCW rating calculation will occur for the specified character (the adjustment value is entered in number of weeks in the range of -1000 weeks to +1000 weeks, with 0 to set the next calculation time to now)"},
	{"testGcwPointToRatingConversion",  2, "<gcw points> <current rating>", "Specify the number of GCW points and the current rating value and see what would happen to the rating when the GCW points are converted to rating"},
	{"getCombatSpamFilter",             1, "<oid>",                      "Get the combat spam filter value for the specified character"},
	{"makeCovertImperialMercenary",     1, "<oid>",                      "Make the specified neutral player character a covert imperial mercenary"},
	{"makeOvertImperialMercenary",      1, "<oid>",                      "Make the specified neutral player character an overt imperial mercenary"},
	{"makeCovertRebelMercenary",        1, "<oid>",                      "Make the specified neutral player character a covert rebel mercenary"},
	{"makeOvertRebelMercenary",         1, "<oid>",                      "Make the specified neutral player character an overt rebel mercenary"},
	{"makeMercenaryNone",               1, "<oid>",                      "Removes factional mercenary status from the specified neutral player character"},
	{"getMaster",                       1, "<oid>",                      "Gets the master of a pet"},
	{"pvpCanAttack",                    2, "<actor> <target>",           "Performs a pvpCanAttack test on the specified objects"},
	{"pvpAttackPerformed",              2, "<actor> <target>",           "Performs a pvpAttackPerformed operation on the specified objects, as if the actor had attacked the target"},
	{"spaceMakeOvert",                  1, "<oid>",                      "Make the ship the specified character is in overt"},
	{"spaceClearOvert",                 1, "<oid>",                      "Make the ship the specified character is in non-overt"},
	{"",                                0, "", ""} // this must be last
};

//-----------------------------------------------------------------

ConsoleCommandParserPvp::ConsoleCommandParserPvp (void) :
CommandParser ("pvp", 0, "...", "Pvp related commands.", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------

bool ConsoleCommandParserPvp::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL (node);
	UNREF (userId);

	UNREF(originalCommand);

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	// ----------------------------------------------------------------------

	if (isCommand (argv [0], "showGcwRankTable"))
	{
		result += Unicode::narrowToWide(Pvp::debugGetGcwRankTable());
		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "showGcwFormulasTable"))
	{
		result += Unicode::narrowToWide(Pvp::debugGetGcwFormulasTable());
		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "showGcwRankDecayExclusionTimeTable"))
	{
		result += Unicode::narrowToWide(Pvp::debugGetGcwRankDecayExclusion());
		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "getGcwInfo"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject const* o = dynamic_cast<ServerObject const*>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject const* c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}
		else
		{
			PlayerObject const* p = PlayerCreatureController::getPlayerObject(c);
			if (p == nullptr)
			{
				result += Unicode::narrowToWide("specified object is not a character object\n");
				return true;
			}
			else
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) GCW info\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));

				if (PvpData::isImperialFactionId(c->getPvpFaction()))
					result += Unicode::narrowToWide("current faction:                        Imperial\n");
				else if (PvpData::isRebelFactionId(c->getPvpFaction()))
					result += Unicode::narrowToWide("current faction:                        Rebel\n");
				else
					result += Unicode::narrowToWide("current faction:                        None\n");

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("current GCW points:                     %ld\n",p->getCurrentGcwPoints()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("current GCW rating:                     %ld\n",p->getCurrentGcwRating()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("current GCW rank:                       %d",p->getCurrentGcwRank()));
				if (p->getCurrentGcwRank() > 0)
				{
					Pvp::PvpRankInfo const & pvpRankInfo = Pvp::getRankInfo(p->getCurrentGcwRating());

					if (PvpData::isImperialFactionId(c->getPvpFaction()))
						result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (%s: %d - %d)\n",pvpRankInfo.imperialTitle.c_str(), pvpRankInfo.minRating, pvpRankInfo.maxRating));
					else if (PvpData::isRebelFactionId(c->getPvpFaction()))
						result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (%s: %d - %d)\n",pvpRankInfo.rebelTitle.c_str(), pvpRankInfo.minRating, pvpRankInfo.maxRating));
					else
						result += Unicode::narrowToWide("\n");
				}
				else
				{
					result += Unicode::narrowToWide("\n");
				}

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("current GCW rank progress:              %0.2f%%\n",p->getCurrentGcwRankProgress()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("current PvP kills:                      %ld\n",p->getCurrentPvpKills()));

				result += Unicode::narrowToWide("lifetime GCW points:                    ");
				result += Unicode::narrowToWide(FormattedString<512>().sprintf(INT64_FORMAT_SPECIFIER,p->getLifetimeGcwPoints()));
				result += Unicode::narrowToWide("\n");

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("max GCW imperial rating:                %ld\n",p->getMaxGcwImperialRating()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("max GCW imperial rank:                  %d",p->getMaxGcwImperialRank()));
				if (p->getMaxGcwImperialRank() > 0)
				{
					Pvp::PvpRankInfo const & pvpRankInfo = Pvp::getRankInfo(p->getMaxGcwImperialRating());
					result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (%s: %d - %d)\n",pvpRankInfo.imperialTitle.c_str(), pvpRankInfo.minRating, pvpRankInfo.maxRating));
				}
				else
				{
					result += Unicode::narrowToWide("\n");
				}

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("max GCW rebel rating:                   %ld\n",p->getMaxGcwRebelRating()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("max GCW rebel rank:                     %d",p->getMaxGcwRebelRank()));
				if (p->getMaxGcwRebelRank() > 0)
				{
					Pvp::PvpRankInfo const & pvpRankInfo = Pvp::getRankInfo(p->getMaxGcwRebelRating());
					result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (%s: %d - %d)\n",pvpRankInfo.rebelTitle.c_str(), pvpRankInfo.minRating, pvpRankInfo.maxRating));
				}
				else
				{
					result += Unicode::narrowToWide("\n");
				}

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("lifetime PvP kills:                     %ld\n",p->getLifetimePvpKills()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("next GCW rating calculation time:       %ld",p->getNextGcwRatingCalcTime()));
				int32 const now = static_cast<int32>(::time(nullptr));
				if (p->getNextGcwRatingCalcTime() > 0)
				{
					if (p->getNextGcwRatingCalcTime() >= now)
						result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (%s)",CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(p->getNextGcwRatingCalcTime() - now)).c_str()));
					else
						result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (-%s)",CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(now - p->getNextGcwRatingCalcTime())).c_str()));

					result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (%s) (%s)\n",CalendarTime::convertEpochToTimeStringGMT(static_cast<time_t>(p->getNextGcwRatingCalcTime())).c_str(), CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(p->getNextGcwRatingCalcTime())).c_str()));
				}
				else
				{
					result += Unicode::narrowToWide("\n");
				}

				if (p->getGcwRatingActualCalcTime() > 0)
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("actual GCW rating calculation time:     %ld",p->getGcwRatingActualCalcTime()));

					if (p->getGcwRatingActualCalcTime() >= now)
						result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (%s)",CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(p->getGcwRatingActualCalcTime() - now)).c_str()));
					else
						result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (-%s)",CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(now - p->getGcwRatingActualCalcTime())).c_str()));

					result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (%s) (%s)\n",CalendarTime::convertEpochToTimeStringGMT(static_cast<time_t>(p->getGcwRatingActualCalcTime())).c_str(), CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(p->getGcwRatingActualCalcTime())).c_str()));
				}
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "adjustCurrentGcwPoint"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject * c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}
		else
		{
			PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
			if (p == nullptr)
			{
				result += Unicode::narrowToWide("specified object is not a character object\n");
				return true;
			}
			else
			{
				int const adjustment = atoi(Unicode::wideToNarrow(argv[2]).c_str());
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) adjusting current GCW point (%ld) by (%d)\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getCurrentGcwPoints(), adjustment));
				p->modifyCurrentGcwPoints(adjustment, false);
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "adjustCurrentGcwRating"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject * c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}
		else
		{
			PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
			if (p == nullptr)
			{
				result += Unicode::narrowToWide("specified object is not a character object\n");
				return true;
			}
			else
			{
				int const adjustment = atoi(Unicode::wideToNarrow(argv[2]).c_str());
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) adjusting current GCW rating (%ld) by (%d)\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getCurrentGcwRating(), adjustment));
				p->modifyCurrentGcwRating(adjustment, false);
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "adjustCurrentPvpKill"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject * c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}
		else
		{
			PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
			if (p == nullptr)
			{
				result += Unicode::narrowToWide("specified object is not a character object\n");
				return true;
			}
			else
			{
				int const adjustment = atoi(Unicode::wideToNarrow(argv[2]).c_str());
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) adjusting current PvP kill (%ld) by (%d)\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getCurrentPvpKills(), adjustment));
				p->modifyCurrentPvpKills(adjustment, false);
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "adjustLifetimeGcwPoint"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject * c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}
		else
		{
			PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
			if (p == nullptr)
			{
				result += Unicode::narrowToWide("specified object is not a character object\n");
				return true;
			}
			else
			{
				int const adjustment = atoi(Unicode::wideToNarrow(argv[2]).c_str());
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) adjusting lifetime GCW point (", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf(INT64_FORMAT_SPECIFIER,p->getLifetimeGcwPoints()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf(") by (%d)\n", adjustment));
				p->modifyLifetimeGcwPoints(adjustment);
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "adjustMaxGcwImperialRating"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject * c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}
		else
		{
			PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
			if (p == nullptr)
			{
				result += Unicode::narrowToWide("specified object is not a character object\n");
				return true;
			}
			else
			{
				int const adjustment = atoi(Unicode::wideToNarrow(argv[2]).c_str());
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) adjusting maximum attained Imperial rating (%ld) by (%d)\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getMaxGcwImperialRating(), adjustment));
				p->modifyMaxGcwImperialRating(adjustment);
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "adjustMaxGcwRebelRating"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject * c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}
		else
		{
			PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
			if (p == nullptr)
			{
				result += Unicode::narrowToWide("specified object is not a character object\n");
				return true;
			}
			else
			{
				int const adjustment = atoi(Unicode::wideToNarrow(argv[2]).c_str());
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) adjusting maximum attained Rebel rating (%ld) by (%d)\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getMaxGcwRebelRating(), adjustment));
				p->modifyMaxGcwRebelRating(adjustment);
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "adjustLifetimePvpKill"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject * c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}
		else
		{
			PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
			if (p == nullptr)
			{
				result += Unicode::narrowToWide("specified object is not a character object\n");
				return true;
			}
			else
			{
				int const adjustment = atoi(Unicode::wideToNarrow(argv[2]).c_str());
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) adjusting lifetime PvP kill (%ld) by (%d)\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getLifetimePvpKills(), adjustment));
				p->modifyLifetimePvpKills(adjustment);
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "adjustNextGcwRatingCalcTime"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		// ignore command if the player doesn't need rating recalculated
		if (!p->needsGcwRatingRecalculated())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("request rejected because character object %s (%s) doesn't require rating calculation (i.e. has no current GCW points and has no current PvP kills and is not at a decayable rank)\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));
			return true;
		}

		int const adjustment = atoi(Unicode::wideToNarrow(argv[2]).c_str());

		if ((p->getNextGcwRatingCalcTime() <= 0) && (adjustment != 0))
		{
			result += Unicode::narrowToWide("there is currently no next GCW rating calculation time for the character object.  Set the next GCW rating calculation time to \"now\" first (adjustNextGcwRatingCalcTime 0) before adjusting it\n");
			return true;
		}

		if (adjustment != 0)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) adjusting next GCW rating calculation time (%ld) by (%d) weeks\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getNextGcwRatingCalcTime(), adjustment));
		else
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) adjusting next GCW rating calculation time (%ld) to now\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getNextGcwRatingCalcTime()));

		p->modifyNextGcwRatingCalcTime(adjustment);

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "testGcwPointToRatingConversion"))
	{
		int const gcwPoints = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		int const currentRating = atoi(Unicode::wideToNarrow(argv[2]).c_str());

		int totalEarnedRating, totalEarnedRatingAfterDecay, cappedRatingAdjustment;
		int const ratingAdjustment = Pvp::calculateRatingAdjustment(gcwPoints, currentRating, totalEarnedRating, totalEarnedRatingAfterDecay, cappedRatingAdjustment);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("GCW points:                      %d\n", gcwPoints));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("current GCW rating:              %d\n", currentRating));

		Pvp::PvpRankInfo const & pvpCurrentRankInfo = Pvp::getRankInfo(currentRating);
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("current GCW rank:                %d", pvpCurrentRankInfo.rank));
		if (pvpCurrentRankInfo.rank > 0)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (%d - %d) (%s / %s)\n", pvpCurrentRankInfo.minRating, pvpCurrentRankInfo.maxRating, pvpCurrentRankInfo.imperialTitle.c_str(), pvpCurrentRankInfo.rebelTitle.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("current GCW rank progress:       %0.2f%%\n", static_cast<float>(currentRating - pvpCurrentRankInfo.minRating) * 100.0f / static_cast<float>(pvpCurrentRankInfo.maxRating - pvpCurrentRankInfo.minRating + 1)));
		}
		else
		{
			result += Unicode::narrowToWide("\n");
		}

		result += Unicode::narrowToWide("\n");

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("total earned rating:             %d\n", totalEarnedRating));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("total earned rating after decay: %d\n", totalEarnedRatingAfterDecay));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("capped rating adjustment:        %d\n", cappedRatingAdjustment));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("final rating adjustment:         %d\n", ratingAdjustment));

		result += Unicode::narrowToWide("\n");

		int newRating = currentRating + ratingAdjustment;

		// make sure updated rating is in range of valid rating
		newRating = std::max(newRating, Pvp::getMinRatingForRank());
		newRating = std::min(newRating, Pvp::getMaxRatingForRank());

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("new GCW rating:                  %d\n", newRating));

		Pvp::PvpRankInfo const & pvpNewRankInfo = Pvp::getRankInfo(newRating);
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("new GCW rank:                    %d", pvpNewRankInfo.rank));
		if (pvpNewRankInfo.rank > 0)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf(" (%d - %d) (%s / %s)\n", pvpNewRankInfo.minRating, pvpNewRankInfo.maxRating, pvpNewRankInfo.imperialTitle.c_str(), pvpNewRankInfo.rebelTitle.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("new GCW rank progress:           %0.2f%%\n", static_cast<float>(newRating - pvpNewRankInfo.minRating) * 100.0f / (pvpNewRankInfo.maxRating - pvpNewRankInfo.minRating + 1)));
		}
		else
		{
			result += Unicode::narrowToWide("\n");
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "getCombatSpamFilter"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else
		{
			Client * c = o->getClient();
			if (c)
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("combat spam filter value for character object %s (%s) is (%d, %.2f)\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), static_cast<int>(c->getCombatSpamFilter()), sqrt(static_cast<float>(c->getCombatSpamRangeSquaredFilter()))));
			}
			else
			{
				result += Unicode::narrowToWide("specified object has no Client object\n");
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "makeCovertImperialMercenary"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * const c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * const p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		if (!PvpData::isNeutralFactionId(static_cast<uint32>(c->getPvpFaction())))
		{
			result += Unicode::narrowToWide("specified object is not neutral faction\n");
			return true;
		}

		if (!ConfigServerGame::getEnableCovertImperialMercenary())
		{
			result += Unicode::narrowToWide("covert imperial mercenary functionality is currently disabled\n");
			return true;
		}

		Pvp::setNeutralMercenaryFaction(*c, PvpData::getImperialFactionId(), PvpType_Covert);

		if (p->isAuthoritative() && p->hasCompletedCollectionSlot("covert_imperial_mercenary"))
		{
			p->setTitle("covert_imperial_mercenary");
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Making (%s) covert imperial mercenary\n", oid.getValueString().c_str()));
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "makeOvertImperialMercenary"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * const c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * const p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		if (!PvpData::isNeutralFactionId(static_cast<uint32>(c->getPvpFaction())))
		{
			result += Unicode::narrowToWide("specified object is not neutral faction\n");
			return true;
		}

		if (!ConfigServerGame::getEnableOvertImperialMercenary())
		{
			result += Unicode::narrowToWide("overt imperial mercenary functionality is currently disabled\n");
			return true;
		}

		Pvp::setNeutralMercenaryFaction(*c, PvpData::getImperialFactionId(), PvpType_Declared);

		if (p->isAuthoritative() && p->hasCompletedCollectionSlot("overt_imperial_mercenary"))
		{
			p->setTitle("overt_imperial_mercenary");
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Making (%s) overt imperial mercenary\n", oid.getValueString().c_str()));
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "makeCovertRebelMercenary"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * const c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * const p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		if (!PvpData::isNeutralFactionId(static_cast<uint32>(c->getPvpFaction())))
		{
			result += Unicode::narrowToWide("specified object is not neutral faction\n");
			return true;
		}

		if (!ConfigServerGame::getEnableCovertRebelMercenary())
		{
			result += Unicode::narrowToWide("covert rebel mercenary functionality is currently disabled\n");
			return true;
		}

		Pvp::setNeutralMercenaryFaction(*c, PvpData::getRebelFactionId(), PvpType_Covert);

		if (p->isAuthoritative() && p->hasCompletedCollectionSlot("covert_rebel_mercenary"))
		{
			p->setTitle("covert_rebel_mercenary");
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Making (%s) covert rebel mercenary\n", oid.getValueString().c_str()));
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "makeOvertRebelMercenary"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * const c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * const p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		if (!PvpData::isNeutralFactionId(static_cast<uint32>(c->getPvpFaction())))
		{
			result += Unicode::narrowToWide("specified object is not neutral faction\n");
			return true;
		}

		if (!ConfigServerGame::getEnableOvertRebelMercenary())
		{
			result += Unicode::narrowToWide("overt rebel mercenary functionality is currently disabled\n");
			return true;
		}

		Pvp::setNeutralMercenaryFaction(*c, PvpData::getRebelFactionId(), PvpType_Declared);

		if (p->isAuthoritative() && p->hasCompletedCollectionSlot("overt_rebel_mercenary"))
		{
			p->setTitle("overt_rebel_mercenary");
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Making (%s) overt rebel mercenary\n", oid.getValueString().c_str()));
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "makeMercenaryNone"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * const c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * const p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		if (!PvpData::isNeutralFactionId(static_cast<uint32>(c->getPvpFaction())))
		{
			result += Unicode::narrowToWide("specified object is not neutral faction\n");
			return true;
		}

		Pvp::setNeutralMercenaryFaction(*c, PvpData::getNeutralFactionId(), PvpType_Neutral);

		if (p->isAuthoritative())
		{
			std::string const & currentTitle = p->getTitle();
			if ((currentTitle == "covert_imperial_mercenary") || (currentTitle == "overt_imperial_mercenary") || (currentTitle == "covert_rebel_mercenary") || (currentTitle == "overt_rebel_mercenary") ||
			    (currentTitle == "covert_imperial_sympathizer") || (currentTitle == "overt_imperial_sympathizer") || (currentTitle == "covert_rebel_sympathizer") || (currentTitle == "overt_rebel_sympathizer") ||
			    (currentTitle == "covert_imperial_loyalist") || (currentTitle == "overt_imperial_loyalist") || (currentTitle == "covert_rebel_insurgent") || (currentTitle == "overt_rebel_insurgent"))
			{
				p->setTitle(std::string());
			}
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Removing all mercenary status for (%s)\n", oid.getValueString().c_str()));
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "getMaster"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
		ServerObject const * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject const * const c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("getMasterId() for (%s) returns (%s)\n", c->getNetworkId().getValueString().c_str(), c->getMasterId().getValueString().c_str()));
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "pvpCanAttack"))
	{
		NetworkId const actorOid(Unicode::wideToNarrow(argv[1]));
		ServerObject const * const actorSo = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(actorOid));
		if (actorSo == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		TangibleObject const * const actorTo = actorSo->asTangibleObject();
		if (actorTo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("(%s) is not a TangibleObject\n", actorOid.getValueString().c_str()));
			return true;
		}

		NetworkId const targetOid(Unicode::wideToNarrow(argv[2]));
		ServerObject const * const targetSo = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(targetOid));
		if (targetSo == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		TangibleObject const * const targetTo = targetSo->asTangibleObject();
		if (targetTo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("(%s) is not a TangibleObject\n", targetOid.getValueString().c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Pvp::canAttack() for (actor=%s) (target=%s) (%s)\n", actorTo->getNetworkId().getValueString().c_str(), targetTo->getNetworkId().getValueString().c_str(), (Pvp::canAttack(*actorTo, *targetTo) ? "true" : "false")));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Pvp::canAttack() for (actor=%s) (target=%s) (%s)\n\n", targetTo->getNetworkId().getValueString().c_str(), actorTo->getNetworkId().getValueString().c_str(), (Pvp::canAttack(*targetTo, *actorTo) ? "true" : "false")));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Pvp::canHelp() for (actor=%s) (target=%s) (%s)\n", actorTo->getNetworkId().getValueString().c_str(), targetTo->getNetworkId().getValueString().c_str(), (Pvp::canHelp(*actorTo, *targetTo) ? "true" : "false")));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Pvp::canHelp() for (actor=%s) (target=%s) (%s)\n\n", targetTo->getNetworkId().getValueString().c_str(), actorTo->getNetworkId().getValueString().c_str(), (Pvp::canHelp(*targetTo, *actorTo) ? "true" : "false")));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Pvp::wouldAttackCauseAlignedEnemyFlag() for (actor=%s) (target=%s) (%s)\n", actorTo->getNetworkId().getValueString().c_str(), targetTo->getNetworkId().getValueString().c_str(), (Pvp::wouldAttackCauseAlignedEnemyFlag(*actorTo, *targetTo) ? "true" : "false")));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Pvp::wouldAttackCauseAlignedEnemyFlag() for (actor=%s) (target=%s) (%s)\n\n", targetTo->getNetworkId().getValueString().c_str(), actorTo->getNetworkId().getValueString().c_str(), (Pvp::wouldAttackCauseAlignedEnemyFlag(*targetTo, *actorTo) ? "true" : "false")));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Pvp::wouldHelpCauseAlignedEnemyFlag() for (actor=%s) (target=%s) (%s)\n", actorTo->getNetworkId().getValueString().c_str(), targetTo->getNetworkId().getValueString().c_str(), (Pvp::wouldHelpCauseAlignedEnemyFlag(*actorTo, *targetTo) ? "true" : "false")));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Pvp::wouldHelpCauseAlignedEnemyFlag() for (actor=%s) (target=%s) (%s)\n\n", targetTo->getNetworkId().getValueString().c_str(), actorTo->getNetworkId().getValueString().c_str(), (Pvp::wouldHelpCauseAlignedEnemyFlag(*targetTo, *actorTo) ? "true" : "false")));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Pvp::isEnemy() for (actor=%s) (target=%s) (%s)\n", actorTo->getNetworkId().getValueString().c_str(), targetTo->getNetworkId().getValueString().c_str(), (Pvp::isEnemy(*actorTo, *targetTo) ? "true" : "false")));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Pvp::isEnemy() for (actor=%s) (target=%s) (%s)\n", targetTo->getNetworkId().getValueString().c_str(), actorTo->getNetworkId().getValueString().c_str(), (Pvp::isEnemy(*targetTo, *actorTo) ? "true" : "false")));
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "pvpAttackPerformed"))
	{
		NetworkId const actorOid(Unicode::wideToNarrow(argv[1]));
		ServerObject * const actorSo = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(actorOid));
		if (actorSo == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		TangibleObject * const actorTo = actorSo->asTangibleObject();
		if (actorTo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("(%s) is not a TangibleObject\n", actorOid.getValueString().c_str()));
			return true;
		}

		NetworkId const targetOid(Unicode::wideToNarrow(argv[2]));
		ServerObject * const targetSo = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(targetOid));
		if (targetSo == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		TangibleObject * const targetTo = targetSo->asTangibleObject();
		if (targetTo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("(%s) is not a TangibleObject\n", targetOid.getValueString().c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("calling pvpAttackPerformed() for (actor=%s) (target=%s)\n", actorTo->getNetworkId().getValueString().c_str(), targetTo->getNetworkId().getValueString().c_str()));
		Pvp::attackPerformed(*actorTo, *targetTo);
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "spaceMakeOvert"))
	{
		if (!ServerWorld::isSpaceScene())
		{
			result += Unicode::narrowToWide("command can only be used in a space zone\n");
			return true;
		}

		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
		ServerObject const * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject const * const c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject const * const p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		ShipObject const * const ship = ShipObject::getContainingShipObject(c);
		if (ship == nullptr)
		{
			result += Unicode::narrowToWide("specified character object is not in a ship\n");
			return true;
		}

		if (!ship->isPlayerShip())
		{
			result += Unicode::narrowToWide("ship is not a player ship\n");
			return true;
		}

		if (ship->isInvulnerable())
		{
			result += Unicode::narrowToWide("ship is invulnerable\n");
			return true;
		}

		uint32 const shipFaction = ship->getSpaceFaction();
		if (!PvpData::isImperialFactionId(shipFaction) && !PvpData::isRebelFactionId(shipFaction))
		{
			result += Unicode::narrowToWide("ship faction is not Imperial or Rebel\n");
			return true;
		}

		if (PvpInternal::isDeclared(*ship))
		{
			result += Unicode::narrowToWide("ship is already overt\n");
			return true;
		}

		if (ship->timeUntilMessageTo("checkSpacePVPStatus") >= 0)
		{
			result += Unicode::narrowToWide("ship currently has a pending overt status change request\n");
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Requesting to make ship (%s) containing (%s) overt.  Please wait for confirmation.\n", ship->getNetworkId().getValueString().c_str(), c->getNetworkId().getValueString().c_str()));
		GameScriptObject::callSpaceMakeOvert(c->getNetworkId());
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "spaceClearOvert"))
	{
		if (!ServerWorld::isSpaceScene())
		{
			result += Unicode::narrowToWide("command can only be used in a space zone\n");
			return true;
		}

		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
		ServerObject const * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject const * const c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject const * const p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		ShipObject const * const ship = ShipObject::getContainingShipObject(c);
		if (ship == nullptr)
		{
			result += Unicode::narrowToWide("specified character object is not in a ship\n");
			return true;
		}

		if (!ship->isPlayerShip())
		{
			result += Unicode::narrowToWide("ship is not a player ship\n");
			return true;
		}

		if (ship->isInvulnerable())
		{
			result += Unicode::narrowToWide("ship is invulnerable\n");
			return true;
		}

		uint32 const shipFaction = ship->getSpaceFaction();
		if (!PvpData::isImperialFactionId(shipFaction) && !PvpData::isRebelFactionId(shipFaction))
		{
			result += Unicode::narrowToWide("ship faction is not Imperial or Rebel\n");
			return true;
		}

		if (!PvpInternal::isDeclared(*ship))
		{
			result += Unicode::narrowToWide("ship is not overt\n");
			return true;
		}

		if (ship->timeUntilMessageTo("checkSpacePVPStatus") >= 0)
		{
			result += Unicode::narrowToWide("ship currently has a pending overt status change request\n");
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Requesting to make ship (%s) containing (%s) non-overt.  Please wait for confirmation.\n", ship->getNetworkId().getValueString().c_str(), c->getNetworkId().getValueString().c_str()));
		GameScriptObject::callSpaceClearOvert(ship->getNetworkId());
	}

	// ----------------------------------------------------------------------

	else
	{
		result += getErrorMessage(argv[0], ERR_NO_HANDLER);
	}

	return true;

}	// ConsoleCommandParserPvp::performParsing

// ======================================================================
