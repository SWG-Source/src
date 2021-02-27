// ======================================================================
//
// ConsoleCommandParserSkill.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserSkill.h"

#include "UnicodeUtils.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedLog/Log.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"

#include <cstdio>

// ======================================================================

namespace
{
	namespace CommandNames
	{
#define MAKE_COMMAND(a) const char * const a = #a
		MAKE_COMMAND(getCommandList);
		MAKE_COMMAND(getSkillMods);
		MAKE_COMMAND(getSkillList);
		MAKE_COMMAND(grantExperience);
		MAKE_COMMAND(grantSkill);
		MAKE_COMMAND(grantSkillMod);
		MAKE_COMMAND(grantCommand);
		MAKE_COMMAND(revokeSkill);
		MAKE_COMMAND(revokeSkillMod);
		MAKE_COMMAND(getSchematics);
		MAKE_COMMAND(grantSchematicGroup);
		MAKE_COMMAND(revokeSchematicGroup);
		MAKE_COMMAND(grantSchematic);
		MAKE_COMMAND(revokeSchematic);
		MAKE_COMMAND(getLevel);
#undef MAKE_COMMAND
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{CommandNames::getCommandList,        0, "[creature OID]",                     "Lists commands on creature."},
		{CommandNames::getSkillMods,          0, "[creature OID]",                     "List all mods on a creature."},
		{CommandNames::getSkillList,          0, "[creature OID]",                     "Lists commands on creature."},
		{CommandNames::grantExperience,       3, "[creature OID] [experience type] [amount]", "Grants/remove experience to/from creature"},
		{CommandNames::grantSkill,            1, "<skill name> [creature OID]",        "Grants skill to creature"},
		{CommandNames::grantSkillMod,         2, "<skill mod name> <mod value> [creature OID]",        "Grants skill mod to creature"},
		{CommandNames::grantCommand,          1, "<command name> [creature OID]",      "Grants command to creature"},
		{CommandNames::revokeSkill,           1, "<skill name> [creature OID]",        "Remove skill from creature"},
		{CommandNames::revokeSkillMod,        1, "<skill mod name> [creature OID]",    "Remove skill mod from creature"},
		{CommandNames::getSchematics,         0, "[creature OID]",                     "Lists draft schematics owned by a creature"},
		{CommandNames::grantSchematicGroup,   1, "<group name> [creature OID]",        "Grants a schematic group to a creature"},
		{CommandNames::revokeSchematicGroup,  1, "<group name> [creature OID]",        "Removes a schematic group from a creature"},
		{CommandNames::grantSchematic,        1, "<schematic name> [creature OID]",    "Grants a draftt schematic to a creature"},
		{CommandNames::revokeSchematic,       1, "<schematic name> [creature OID]",    "Removes a draftt schematic from a creature"},
		{CommandNames::getLevel,              0, "[creature OID]",                     "List creature's current level."},
		{"", 0, "", ""} // this must be last
	};

	//----------------------------------------------------------------------

	CreatureObject * findCreature (const NetworkId & userId, const CommandParser::StringVector_t & argv, int index)
	{
		if (index < 0)
			return 0;

		NetworkId oid = userId;

		if (argv.size () > static_cast<size_t>(index))
			oid = NetworkId (Unicode::wideToNarrow (argv [index]));

		return dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(oid));
	}
}

//-----------------------------------------------------------------

ConsoleCommandParserSkill::ConsoleCommandParserSkill (void) :
CommandParser ("skill", 0, "...", "Skill related commands.", 0)
{
	createDelegateCommands (cmds);
}

//----------------------------------------------------------------------


bool ConsoleCommandParserSkill::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & , String_t & result, const CommandParser *)
{

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

    // ----------------------------------------------------------------
	
	if (isCommand (argv[0], CommandNames::grantSkill))
	{
		CreatureObject * const creature = findCreature (userId, argv, 2);

		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			const std::string skillName = Unicode::wideToNarrow (argv[1]);
			const SkillObject * const skill = SkillManager::getInstance ().getSkill (skillName);
			if (skill == nullptr)
			{
				result += Unicode::narrowToWide ("unknown skill");
			}
			else
			{
				creature->grantSkill (*skill);
				result += getErrorMessage (argv[0], ERR_SUCCESS);
			}
		}
		return true;
	}
	
	// ----------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::grantSkillMod))
	{
		CreatureObject * const creature = findCreature (userId, argv, 3);

		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			const std::string skillModName = Unicode::wideToNarrow (argv[1]);
			int value = atoi (Unicode::wideToNarrow (argv [2]).c_str ());
			creature->setModValue(skillModName, value);
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
		return true;
	}

	// ----------------------------------------------------------------
	
	else if (isCommand (argv[0], CommandNames::grantCommand))
	{
		CreatureObject * const creature = findCreature (userId, argv, 2);

		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			const std::string commandName = Unicode::wideToNarrow (argv[1]);
			if (creature->grantCommand (commandName, false))
				result += getErrorMessage (argv[0], ERR_SUCCESS);
			else
				result += getErrorMessage (argv[0], ERR_FAIL);
		}
		return true;
	}
	
	//-----------------------------------------------------------------------
	
	else if (isCommand (argv[0], CommandNames::getSkillMods))
	{
		const CreatureObject * const creature = findCreature (userId, argv, 2);
		if(creature)
		{
			std::map<std::string, std::pair<int, int> >::const_iterator i;
			const std::map<std::string, std::pair<int, int> > & modmap = creature->getModMap();

			char valbuf[32] = {"\0"};
			int count = 0;
			for(i = modmap.begin(); i != modmap.end(); ++i, ++count)
			{
				snprintf(valbuf, sizeof(valbuf), "%i/%i", (*i).second.first, (*i).second.second);
				result += Unicode::narrowToWide((*i).first);
				result += Unicode::narrowToWide(std::string(" "));
				result += Unicode::narrowToWide(std::string(valbuf));
				result += Unicode::narrowToWide(std::string("\n"));
			}
			snprintf(valbuf, sizeof(valbuf), "%i", count);
			result += Unicode::narrowToWide(std::string(valbuf));
			result += Unicode::narrowToWide(std::string(" mods returned\n"));
			return true;
		}
		else
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
	}

	// ----------------------------------------------------------------
	
	else if (isCommand (argv[0], CommandNames::grantExperience))
	{
		CreatureObject * const creature = findCreature (userId, argv, 1);

		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			const std::string expType = Unicode::wideToNarrow (argv[2]);
			
			if (argv.size () > 3)
			{
				const int amount = atoi (Unicode::wideToNarrow (argv [3]).c_str ());
				creature->grantExperiencePoints (expType, amount);
				result += Unicode::narrowToWide ("granted");
			}
			else
			{
				const int amount = creature->getExperiencePoints (expType);
				result += Unicode::narrowToWide (expType);
				result.append (1, ' ');
				result.append (1, ':');
				result.append (1, ' ');
				
				char buf[64];
				result += Unicode::narrowToWide (_itoa (amount, buf, 10));
			}
		}
		return true;
	}
	
	//-----------------------------------------------------------------------
	
	else if (isCommand (argv[0], CommandNames::getSkillList))
	{
		CreatureObject * const creature = findCreature (userId, argv, 1);
		
		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			char buf[1024];
			sprintf (buf, "skill list object %s, num %d\n", creature->getNetworkId ().getValueString ().c_str (), creature->getSkillList ().size ());
			result += Unicode::narrowToWide (buf);
			for (CreatureObject::SkillList::const_iterator i = creature->getSkillList ().begin (); i != creature->getSkillList ().end (); ++i)
			{
				if (*i)
				{
					result += Unicode::narrowToWide ( (*i)->getSkillName ());
					result += Unicode::narrowToWide ("\n");
				}
				else
					result += Unicode::narrowToWide ("[invalid]\n");
			}
		}
		return true;
	}
	
	//-----------------------------------------------------------------------
	
	else if (isCommand (argv[0], CommandNames::getCommandList))
	{
		CreatureObject * const creature = findCreature (userId, argv, 1);
		
		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			char buf[1024];
			sprintf (buf, "command list object %s, num %d\n", creature->getNetworkId ().getValueString ().c_str (), creature->getCommandList ().size ());
			result += Unicode::narrowToWide (buf);
			std::map<std::string, int>::const_iterator i;
			for (i = creature->getCommandList ().begin (); i != creature->getCommandList ().end (); ++i)
			{
				result += Unicode::narrowToWide ((*i).first);
				result += Unicode::narrowToWide ("\n");
			}
		}

		return true;
	}
	
	// ----------------------------------------------------------------
	
	else if (isCommand (argv[0], CommandNames::revokeSkill))
	{
		CreatureObject * const creature = findCreature (userId, argv, 2);

		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			std::string skillName = Unicode::wideToNarrow (argv[1]);
			const SkillObject * skill = SkillManager::getInstance ().getSkill (skillName);
			if (skill == nullptr)
			{
				result += Unicode::narrowToWide ("unknown skill");
			}
			else
			{
				LOG("CustomerService", ("Skill: A GOD has requested the revocation of skill %s from character %s.", 
					skillName.c_str(), creature->getNetworkId().getValueString().c_str()));
				creature->revokeSkill (*skill);
				result += getErrorMessage (argv[0], ERR_SUCCESS);
			}
		}
		return true;
	}

	// ----------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::revokeSkillMod))
	{
		CreatureObject * const creature = findCreature (userId, argv, 2);

		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			const std::string skillModName = Unicode::wideToNarrow (argv[1]);
			creature->setModValue(skillModName, 0);
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::getSchematics))
	{
		CreatureObject * const creature = findCreature (userId, argv, 1);
		
		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			const std::map<std::pair<uint32, uint32>,int> & schematics = creature->getDraftSchematics();
			if (!schematics.empty())
			{
				std::map<std::pair<uint32, uint32>,int>::const_iterator iter;
				for (iter = schematics.begin(); iter != schematics.end(); ++iter)
				{
					const ConstCharCrcString & crcString = ObjectTemplateList::lookUp((*iter).first.first);
					if (!crcString.isEmpty())
					{
						result += Unicode::narrowToWide(crcString.getString());
						result += Unicode::narrowToWide("\n");
					}
				}
			}
			else
				result += Unicode::narrowToWide("No schematics available");
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::grantSchematicGroup))
	{
		CreatureObject * const creature = findCreature (userId, argv, 2);
		
		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			creature->grantSchematicGroup("+" + Unicode::wideToNarrow(argv[1]), false);
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::revokeSchematicGroup))
	{
		CreatureObject * const creature = findCreature (userId, argv, 2);
		
		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			creature->grantSchematicGroup("-" + Unicode::wideToNarrow(argv[1]), false);
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::grantSchematic))
	{
		CreatureObject * const creature = findCreature (userId, argv, 2);
		
		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			creature->grantSchematic(Unicode::wideToNarrow(argv[1]), false);
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::revokeSchematic))
	{
		CreatureObject * const creature = findCreature (userId, argv, 2);
		
		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			creature->revokeSchematic(Unicode::wideToNarrow(argv[1]), false);
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::getLevel))
	{
		CreatureObject * const creature = findCreature (userId, argv, 1);
		
		if (creature == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			char valbuf[32] = {"\0"};
			result += Unicode::narrowToWide(std::string("Level "));
			snprintf(valbuf, sizeof(valbuf), "%i", creature->getLevel());
			result += Unicode::narrowToWide(std::string(valbuf));

			result += Unicode::narrowToWide(std::string(" TotalLevelXp "));
			snprintf(valbuf, sizeof(valbuf), "%i", creature->getLevelXp());
			result += Unicode::narrowToWide(std::string(valbuf));
		}
	}

	//-----------------------------------------------------------------

	else
	{
		result += getErrorMessage (argv[0], ERR_NO_HANDLER);
	}
	
	return true;
}

// ======================================================================





