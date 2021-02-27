// ======================================================================
//
// ConsoleCommandParserSpawner.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserSpawner.h"

#include "UnicodeUtils.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"

#include <cstdio>
#include <unordered_set>

// ======================================================================

namespace
{
	namespace CommandNames
	{
#define MAKE_COMMAND(a) const char * const a = #a
		MAKE_COMMAND(showQueue);
		MAKE_COMMAND(showCreatures);
#undef MAKE_COMMAND
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{CommandNames::showQueue,        0, "",                     "Lists players on the spawn queue."},
		{CommandNames::showCreatures,    0, "[pc|npc]",             "Lists all creatures on the server."},
		{"", 0, "", ""} // this must be last
	};

}

//-----------------------------------------------------------------

ConsoleCommandParserSpawner::ConsoleCommandParserSpawner (void) :
CommandParser ("Spawner", 0, "...", "Spawner related commands.", 0)
{
	createDelegateCommands (cmds);
}

//----------------------------------------------------------------------


bool ConsoleCommandParserSpawner::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & , String_t & result, const CommandParser *)
{			
	// ----------------------------------------------------------------

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }
	
	if (isCommand (argv[0], CommandNames::showQueue))
	{
		CreatureObject::showSpawnQueue(result);
		return true;
	}

	else if(isCommand (argv[0], CommandNames::showCreatures))
	{
		bool npcOnly = false;
		bool pcOnly = false;
		if(argv.size() >= 2)
		{
			std::string arg2 = Unicode::wideToNarrow(argv[1]);
			if(arg2 == "npc")
			{
				npcOnly = true;
			}
			else if(arg2 == "pc")
			{
				pcOnly = true;
			}
		}

		static const Unicode::String header(Unicode::narrowToWide(std::string("Creature Listing - ID: POS: Auth Server: Template Name\n")));
		result += header;
		const CreatureObject::AllCreaturesSet & creatureList = CreatureObject::getAllCreatures();
		int count = 0;
		for(auto i = creatureList.begin(); i != creatureList.end(); ++i)
		{
			const CreatureObject * creature = (*i);
			if(creature)
			{
				if(pcOnly && (!creature->isPlayerControlled()) )
					continue;
				if(npcOnly && creature->isPlayerControlled())
					continue;

				std::string creatureEntry = creature->getNetworkId().getValueString();
				creatureEntry += ": ";

				char locBuffer[32] = {"\0"};
				Vector loc = creature->getTransform_o2w().getPosition_p();
				snprintf(locBuffer, sizeof(locBuffer), "%.0f, %.0f: ", loc.x, loc.z);
				creatureEntry += locBuffer;

				snprintf(locBuffer, sizeof(locBuffer), "%lu: ", creature->getAuthServerProcessId());
				creatureEntry += locBuffer;
				creatureEntry += creature->getObjectTemplateName();

				creatureEntry += "\n";

				result += Unicode::narrowToWide(creatureEntry);
				count++;
			}
		}
		char countBuffer[32] = {"\0"};
		snprintf(countBuffer, sizeof(countBuffer), "%d creatures listed: ", count);
		result += Unicode::narrowToWide(std::string(countBuffer));
		return true;
	}

	
	else
	{
		result += getErrorMessage (argv[0], ERR_NO_HANDLER);
		return false;
	}
	
	return true;
}

// ======================================================================





