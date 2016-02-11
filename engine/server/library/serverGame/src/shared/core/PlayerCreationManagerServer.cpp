//======================================================================
//
// PlayerCreationManagerServer.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlayerCreationManagerServer.h"

#include "UnicodeUtils.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/NameManager.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedUtility/DataTable.h"

//======================================================================

void PlayerCreationManagerServer::install ()
{
	if (!PlayerCreationManager::isInstalled ())
		PlayerCreationManager::install (true);
}

//----------------------------------------------------------------------

void PlayerCreationManagerServer::remove ()
{
	if (PlayerCreationManager::isInstalled ())
		PlayerCreationManager::remove ();
}

//----------------------------------------------------------------------

bool PlayerCreationManagerServer::setupPlayer(CreatureObject & obj, const std::string & profession, StationId account, bool isJedi)
{
	// NOTE: the isJedi flag doesn't actually create a Jedi character, but we need it
	// so that the database/login server will keep track of a player's extra character
	// slots correctly
	UNREF(isJedi);

	AttribVector attribs;
	const SkillVector * skills = 0;
	const EqVector * eq        = 0;

	const char * const sharedObjectTemplateNameStr = obj.getSharedTemplateName ();

	if (!sharedObjectTemplateNameStr)
		return false;

	const std::string sharedObjectTemplateName (sharedObjectTemplateNameStr);

	if (!getDefaults(sharedObjectTemplateName, profession, attribs, skills, eq))
	{
		WARNING (true, ("PlayerCreationManagerServer error loading for %s, %s", sharedObjectTemplateName.c_str (), profession.c_str ()));
		return false;
	}

	//----------------------------------------------------------------------
	//-- setup skills
	
	if (skills)
	{
		for (SkillVector::const_iterator it = skills->begin (); it != skills->end (); ++it)
		{
			const std::string & skillName = *it;

			const SkillObject * skill = SkillManager::getInstance ().getSkill (*it);
			if (skill)
			{
				//-- newb tutorial will grant the skill
				obj.setObjVarItem ("newbie.hasSkill", skillName);
				break;
			}
			else
				WARNING (true, ("PlayerCreationManagerServer Bad skill [%s] for profesion [%s]", skillName.c_str (), profession.c_str ()));
		}
	}
	
	//----------------------------------------------------------------------
	//-- setup attribs
	if (!attribs.empty())
	{
		WARNING(attribs.size() != static_cast<size_t>(Attributes::NumberOfAttributes), ("Bad number of attribs %d when creating avatar", attribs.size()));
		int i = 0;
		for (AttribVector::const_iterator it = attribs.begin (); it != attribs.end (); ++it)
		{
			obj.initializeAttribute (i++, static_cast<Attributes::Value>(*it));
		}
	}

	//----------------------------------------------------------------------
	//-- setup equipment

	if (eq)
	{
		for (EqVector::const_iterator it = eq->begin (); it != eq->end (); ++it)
		{
			const EqInfo & eqi = *it;
			
			const int32 arrangement = eqi.arrangement;
			UNREF(arrangement);
			//Ignore arrangement for now.  Use the first available.  If we need to change this
			//instead of specifiying the arrangement, we'll specify which valid arrangement to use.
			
			ServerObject * const item = ServerWorld::createNewObject(
				eqi.serverTemplateName, obj, false);
			
			if (!item)
				WARNING (true, ("Invalid equipment template: '%s'", eqi.serverTemplateName.c_str ()));

			else
			{
				//Attach insurance variables here:
				if (item->asTangibleObject() != nullptr)
					item->asTangibleObject()->setUninsurable(true);
			}
		}
	}

	// ----------------------------------------------------------------------

	// **************** BEGIN SPECIAL TESTCENTER CODE **********************

	// if we are on TC2 and [GameServer] loginAsBountyHunter is set, 
	// make the player a master bounty hunter
	if (ConfigServerGame::getLoginAsBountyHunter())
	{
		// check that our first name ends with "MBH"
		std::string firstName(Unicode::wideToNarrow(obj.getAssignedObjectFirstName()));
		// check that we are on TC2
		std::string cluster(ConfigFile::getKeyString("CentralServer", "clusterName", ""));
		if ((firstName.size() > 3 && firstName.substr(firstName.size() - 3) == "MBH" ) && 
			(cluster == "TestCenter-Bria" || cluster == "sjakab"))
		{
			static const std::string bounty_skills[] = {
				"combat_marksman_novice",
				"combat_marksman_rifle_01",
				"combat_marksman_rifle_02",
				"combat_marksman_rifle_03",
				"combat_marksman_rifle_04",
				"combat_marksman_pistol_01",
				"combat_marksman_pistol_02",
				"combat_marksman_pistol_03",
				"combat_marksman_pistol_04",
				"combat_marksman_carbine_01",
				"combat_marksman_carbine_02",
				"combat_marksman_carbine_03",
				"combat_marksman_carbine_04",
				"combat_marksman_support_01",
				"combat_marksman_support_02",
				"combat_marksman_support_03",
				"combat_marksman_support_04",
				"combat_marksman_master",
				"outdoors_scout_novice",
				"outdoors_scout_movement_01",
				"outdoors_scout_movement_02",
				"outdoors_scout_movement_03",
				"outdoors_scout_movement_04",
				"outdoors_scout_tools_01",
				"outdoors_scout_tools_02",
				"outdoors_scout_tools_03",
				"outdoors_scout_tools_04",
				"outdoors_scout_harvest_01",
				"outdoors_scout_harvest_02",
				"outdoors_scout_harvest_03",
				"outdoors_scout_harvest_04",
				"outdoors_scout_camp_01",
				"outdoors_scout_camp_02",
				"outdoors_scout_camp_03",
				"outdoors_scout_camp_04",
				"outdoors_scout_master",
				"combat_bountyhunter_novice",
				"combat_bountyhunter_investigation_01",
				"combat_bountyhunter_investigation_02",
				"combat_bountyhunter_investigation_03",
				"combat_bountyhunter_investigation_04",
				"combat_bountyhunter_droidcontrol_01",
				"combat_bountyhunter_droidcontrol_02",
				"combat_bountyhunter_droidcontrol_03",
				"combat_bountyhunter_droidcontrol_04",
				"combat_bountyhunter_droidresponse_01",
				"combat_bountyhunter_droidresponse_02",
				"combat_bountyhunter_droidresponse_03",
				"combat_bountyhunter_droidresponse_04",
				"combat_bountyhunter_support_01",
				"combat_bountyhunter_support_02",
				"combat_bountyhunter_support_03",
				"combat_bountyhunter_support_04",
				"combat_bountyhunter_master"
				};
			static const size_t numSkills = sizeof(bounty_skills) / sizeof(std::string);

			for (size_t i = 0; i < numSkills; ++i)
			{
				const SkillObject * skill = SkillManager::getInstance().getSkill(bounty_skills[i]);
				if (skill != nullptr)
					obj.grantSkill(*skill);
			}
		}
	}

	// **************** END SPECIAL TESTCENTER CODE ************************

	return true;
}

// ======================================================================

void PlayerCreationManagerServer::renamePlayer(int8 renameCharacterMessageSource, uint32 stationId, const NetworkId & oid, const Unicode::String & newName, const Unicode::String & oldName, const NetworkId &requestedBy)
{
	if (!NameManager::getInstance().isPlayer(oid))
		return;

	RenameCharacterMessageEx const msg(static_cast<RenameCharacterMessageEx::RenameCharacterMessageSource>(renameCharacterMessageSource),stationId,oid,newName,oldName,requestedBy);
	GameServer::getInstance().sendToDatabaseServer(msg);
}

//======================================================================
