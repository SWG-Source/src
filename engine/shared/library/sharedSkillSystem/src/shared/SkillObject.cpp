// SkillObject.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedSkillSystem/FirstSharedSkillSystem.h"
#include "sharedSkillSystem/SkillObject.h"

#include "sharedSkillSystem/SkillManager.h"
#include "sharedUtility/DataTable.h"

#include "UnicodeUtils.h"

#include <algorithm>

const std::string SkillObject::ms_skillLabel                        = "NAME";
const std::string SkillObject::ms_prerequisiteSkillsLabel           = "SKILLS_REQUIRED";
const std::string SkillObject::ms_prerequisiteExperienceTypeLabel   = "XP_TYPE";
const std::string SkillObject::ms_prerequisiteExperienceAmountLabel = "XP_COST";
const std::string SkillObject::ms_prerequisiteExperienceLimitLabel  = "XP_CAP";
const std::string SkillObject::ms_prerequisiteSpeciesLabel          = "SPECIES_REQUIRED";
const std::string SkillObject::ms_commandsLabel                     = "COMMANDS";
const std::string SkillObject::ms_statisticsModifiersLabel          = "SKILL_MODS";
const std::string SkillObject::ms_parentLabel                       = "PARENT";
const std::string SkillObject::ms_schematicsGrantedLabel            = "SCHEMATICS_GRANTED";
const std::string SkillObject::ms_isTitleLabel                      = "IS_TITLE";
const std::string SkillObject::ms_isProfessionLabel                 = "IS_PROFESSION";
const std::string SkillObject::ms_isSearchableLabel                 = "SEARCHABLE";

//-----------------------------------------------------------------

namespace
{
	int       s_recursionCountDepends    = 0;
	const int s_recursionCountDependsMax = 7;

	//----------------------------------------------------------------------

	typedef std::vector<std::string> StringVector;
	inline void tokenizeList (const std::string & str, StringVector & sv, int row, const std::string & columnName)
	{
		UNREF (row);
		UNREF (columnName);

		size_t endpos = 0;
		std::string token;
		static const char * const whitespace = ",";

		DEBUG_WARNING (str.find (" \n\r\t") != std::string::npos, ("SkillObject Cell string [%s] row %d column [%s] contains spaces or other invalid characters", row, columnName.c_str ()));

		while (Unicode::getFirstToken (str, endpos, endpos, token, whitespace))
		{
			sv.push_back (token);
			if (endpos == std::string::npos)
				break;

			++endpos;
		}
	}
}

//---------------------------------------------------------------------

SkillObject::SkillData::SkillData() :
prerequisiteSkills             (),
prerequisiteExperience         (),
prerequisiteSpecies            (),
prerequisiteFactionStanding    (),
skillName                      ("UNINITIALIZED SKILL"),
nextSkillBoxes                 (),
prevSkill                      (0),
commandsProvided               (),
schematicsGranted              (),
statisticModifiers             (),
isProfession                   (false),
isTitle                        (false),
isSearchable                   (false)
{
}

//---------------------------------------------------------------------

SkillObject::SkillData::SkillData(const SkillData & source) :
prerequisiteSkills            (source.prerequisiteSkills),
prerequisiteExperience        (source.prerequisiteExperience),
prerequisiteSpecies           (source.prerequisiteSpecies),
prerequisiteFactionStanding   (source.prerequisiteFactionStanding),
skillName                     (source.skillName),
nextSkillBoxes                (source.nextSkillBoxes),
prevSkill                     (0),
commandsProvided              (source.commandsProvided),
schematicsGranted             (source.schematicsGranted),
statisticModifiers            (source.statisticModifiers),
isProfession                  (source.isProfession),
isTitle                       (source.isTitle),
isSearchable                  (source.isSearchable)
{
}

//---------------------------------------------------------------------

SkillObject::SkillData::~SkillData()
{
}

//---------------------------------------------------------------------

SkillObject::SkillData & SkillObject::SkillData::operator = (const SkillData & rhs)
{
	if(&rhs != this)
	{
		prerequisiteSkills =             rhs.prerequisiteSkills;
		prerequisiteExperience =         rhs.prerequisiteExperience;
		prerequisiteSpecies =            rhs.prerequisiteSpecies;
		prerequisiteFactionStanding =    rhs.prerequisiteFactionStanding;
		skillName =                      rhs.skillName;
		nextSkillBoxes =                 rhs.nextSkillBoxes;
		commandsProvided =               rhs.commandsProvided;
		schematicsGranted =              rhs.schematicsGranted;
		statisticModifiers =             rhs.statisticModifiers;
		isProfession =                   rhs.isProfession;
		isTitle =                        rhs.isTitle;
		isSearchable =                   rhs.isSearchable;
	}
	return *this;
}

//---------------------------------------------------------------------

SkillObject::SkillObject() :
skillData()
{
}

//---------------------------------------------------------------------

SkillObject::~SkillObject()
{
}

//---------------------------------------------------------------------

const SkillObject::SkillVector & SkillObject::getNextSkillBoxes() const
{
	return skillData.nextSkillBoxes;
}

//-----------------------------------------------------------------------

const SkillObject * SkillObject::getPrevSkill() const
{
	return skillData.prevSkill;
}

//---------------------------------------------------------------------

const SkillObject::SkillData & SkillObject::getSkillData() const
{
	return skillData;
}

//---------------------------------------------------------------------

const SkillObject::ExperienceVector & SkillObject::getPrerequisiteExperienceVector () const
{
	return skillData.prerequisiteExperience;
}

//-----------------------------------------------------------------

const SkillObject::ExperiencePair * SkillObject::getPrerequisiteExperience       () const
{
	if (skillData.prerequisiteExperience.empty ())
		return 0;
	else
		return &skillData.prerequisiteExperience.front ();
}

//---------------------------------------------------------------------

const SkillObject::SpeciesFlagVector & SkillObject::getPrerequisiteSpecies() const
{
	return skillData.prerequisiteSpecies;
}

//---------------------------------------------------------------------

const SkillObject::GenericModVector & SkillObject::getPrerequisiteFactionStanding() const
{
	return skillData.prerequisiteFactionStanding;
}

//---------------------------------------------------------------------

const SkillObject::SkillVector & SkillObject::getPrerequisiteSkills() const
{
	return skillData.prerequisiteSkills;
}

//---------------------------------------------------------------------

const std::string & SkillObject::getSkillName() const
{
	return skillData.skillName;
}

//---------------------------------------------------------------------

const SkillObject::StringVector & SkillObject::getCommandsProvided() const
{
	return skillData.commandsProvided;
}

//---------------------------------------------------------------------

const SkillObject::StringVector & SkillObject::getSchematicsGranted() const
{
	return skillData.schematicsGranted;
}

//---------------------------------------------------------------------

const std::vector<std::pair<std::string, int> > & SkillObject::getStatisticModifiers() const
{
	return skillData.statisticModifiers;
}

//-----------------------------------------------------------------------

SkillObject::SkillData & SkillObject::getSkillData() 
{
	return skillData;
}

//-----------------------------------------------------------------------

const bool SkillObject::hasCommand(const std::string & commandName) const
{
	return std::binary_search (skillData.commandsProvided.begin (), skillData.commandsProvided.end (), commandName);
}

//-----------------------------------------------------------------------
/** get primary category for this skill */

const SkillObject * SkillObject::findCategory() const
{
	const SkillObject * p = this;

	for (;;)
	{
		if (p)
		{
			//-- the categories are at the 2nd level of the heirarchy

			if (p->skillData.prevSkill && p->skillData.prevSkill->skillData.prevSkill == 0)
			{
				return p;
			}

			p = p->skillData.prevSkill;
		}
		else
		{
			break;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------

const bool SkillObject::isProfession() const
{
	return skillData.isProfession;
}

//-----------------------------------------------------------------------

const bool SkillObject::isTitle() const
{
	return skillData.isTitle;
}

//-----------------------------------------------------------------------

const bool SkillObject::isSearchable() const
{
	return skillData.isSearchable;
}

//---------------------------------------------------------------------

void SkillObject::loadPrerequisiteSkills(DataTable &dataTable, int skillRow)
{
	const std::string & cellString = dataTable.getStringValue(SkillObject::ms_prerequisiteSkillsLabel, skillRow);
	StringVector sv;
	sv.clear ();
	tokenizeList (cellString, sv, skillRow, SkillObject::ms_prerequisiteSkillsLabel);
	skillData.prerequisiteSkills.reserve (sv.size ());

	SkillManager & manager = SkillManager::getInstance ();

	for (StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
	{
		const std::string & skillName = *it;
		const SkillObject * const skill = manager.getSkill (skillName);
		if (skill)
			skillData.prerequisiteSkills.push_back(skill);
		else
			WARNING (true, ("prerequisiteSkills skill [%s] does not exist", skillName.c_str ()));
	}

	std::sort (skillData.prerequisiteSkills.begin (), skillData.prerequisiteSkills.end ());
}

//----------------------------------------------------------------------

void SkillObject::loadPrerequisiteExperience(DataTable &dataTable, int skillRow)
{
	const int xpAmount = dataTable.getIntValue(SkillObject::ms_prerequisiteExperienceAmountLabel, skillRow);
	const int xpLimit = dataTable.getIntValue(SkillObject::ms_prerequisiteExperienceLimitLabel, skillRow);
	if (xpAmount > 0 || xpLimit > 0)
	{
		const std::string & experienceType = dataTable.getStringValue(SkillObject::ms_prerequisiteExperienceTypeLabel, skillRow); 
		if (!experienceType.empty ())
			skillData.prerequisiteExperience.push_back(std::make_pair(experienceType, std::make_pair(xpAmount, xpLimit)));
		else
			WARNING (true, ("Nonzero experience specified but empty experience name on row %d", skillRow));
	}
}

//---------------------------------------------------------------------

void SkillObject::loadPrerequisiteSpecies(DataTable &dataTable, int skillRow)
{
	const std::string & cellString = dataTable.getStringValue(SkillObject::ms_prerequisiteSpeciesLabel, skillRow);
	static StringVector sv;
	sv.clear ();
	tokenizeList (cellString, sv, skillRow, SkillObject::ms_prerequisiteSpeciesLabel);
	skillData.prerequisiteSpecies.reserve (sv.size ());

	for (StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
		skillData.prerequisiteSpecies.push_back (std::make_pair (*it, true));
}

//---------------------------------------------------------------------

void SkillObject::loadCommands(DataTable &dataTable, int skillRow)
{
	const std::string & cellString = dataTable.getStringValue(SkillObject::ms_commandsLabel, skillRow);
	static StringVector sv;
	sv.clear ();
	tokenizeList (cellString, sv, skillRow, SkillObject::ms_commandsLabel);
	skillData.commandsProvided.reserve (sv.size ());

	for (StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
		skillData.commandsProvided.push_back (*it);

	std::sort (skillData.commandsProvided.begin (), skillData.commandsProvided.end ());
}

//---------------------------------------------------------------------

void SkillObject::loadSchematicsGranted(DataTable &dataTable, int skillRow) 
{
	const std::string & cellString = dataTable.getStringValue(SkillObject::ms_schematicsGrantedLabel, skillRow);
	static StringVector sv;
	sv.clear ();
	tokenizeList (cellString, sv, skillRow, SkillObject::ms_schematicsGrantedLabel);
	skillData.schematicsGranted.reserve (sv.size ());

	for (StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
		skillData.schematicsGranted.push_back (*it);

	std::sort (skillData.schematicsGranted.begin (), skillData.schematicsGranted.end ());
}

//---------------------------------------------------------------------

void SkillObject::loadStatisticsModifiers(DataTable &dataTable, int skillRow)
{
	const std::string & cellString = dataTable.getStringValue(SkillObject::ms_statisticsModifiersLabel, skillRow);
	static StringVector sv;
	sv.clear ();
	tokenizeList (cellString, sv, skillRow, SkillObject::ms_statisticsModifiersLabel);
	skillData.statisticModifiers.reserve (sv.size ());

	for (StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
	{
		const std::string & statEntry = *it;
		const size_t splitPos = statEntry.find ('=');

		if (splitPos == std::string::npos)
			WARNING (true, ("Statistic modifier without an = [%s], skill=%s", statEntry.c_str(), skillData.skillName.c_str()));
		else
		{
			const char * const statValueString = statEntry.c_str () + splitPos + 1;
			const std::string & statValueName   = statEntry.substr (0, splitPos);
			const int32 modifier = atoi (statValueString);
			skillData.statisticModifiers.push_back(std::make_pair(statValueName, modifier));
		}
	}
}

//---------------------------------------------------------------------

void SkillObject::connectLinks(DataTable &dataTable, const std::string & parentName)
{
	if (!parentName.empty ())
	{
		const SkillObject * const parent = SkillManager::getInstance().getSkill (parentName);
		if (parent)
		{
			skillData.prevSkill = parent;
		}
	}

	std::string nextSkillSearchName = skillData.skillName;
	if (nextSkillSearchName == "skill_system_root")
	{
		nextSkillSearchName.clear ();
	}

	int rows = dataTable.getNumRows();
	for (int i = 0; i < rows; i++)
	{
		const std::string & tmpParent = dataTable.getStringValue(SkillObject::ms_parentLabel, i);

		if (tmpParent == nextSkillSearchName)
		{
			const std::string & nextSkillName = dataTable.getStringValue(SkillObject::ms_skillLabel, i);
			const SkillObject * const nextSkill = SkillManager::getInstance().getSkill(nextSkillName);
			if (nextSkill)
			{
				skillData.nextSkillBoxes.push_back(nextSkill);
			}
		}
	}

}

//---------------------------------------------------------------------

bool SkillObject::load(DataTable & dataTable, const std::string & skillName)
{
	if (skillName == "skill_system_root")
	{
		skillData = SkillData();
		skillData.skillName = skillName;
		connectLinks (dataTable, "");
		return true;
	}
	
	const int skillColumn = dataTable.findColumnNumber (SkillObject::ms_skillLabel);
	if (skillColumn == -1)
	{
		WARNING (true, ("SkillObject::load Could not find column %s in DataTable", SkillObject::ms_skillLabel.c_str()));
		return false;
	}

	const int skillRow = dataTable.searchColumnString (skillColumn, skillName);
	if (skillRow == -1)
	{
		skillData = SkillData();		
		return false;
	}
	
	skillData = SkillData();
	
	skillData.skillName = skillName;
	
	loadPrerequisiteSkills      (dataTable, skillRow);
	loadPrerequisiteExperience  (dataTable, skillRow);
	loadPrerequisiteSpecies     (dataTable, skillRow);

	std::string parent          = dataTable.getStringValue(SkillObject::ms_parentLabel, skillRow);
	if (parent.empty ())
	{
		parent = "skill_system_root";
	}
	connectLinks   (dataTable, parent);

	loadCommands            (dataTable, skillRow);
	loadSchematicsGranted   (dataTable, skillRow); 
	loadStatisticsModifiers (dataTable, skillRow);

	skillData.isTitle                 = dataTable.getIntValue(SkillObject::ms_isTitleLabel, skillRow) != 0;
	skillData.isProfession            = dataTable.getIntValue(SkillObject::ms_isProfessionLabel, skillRow) != 0;
	skillData.isSearchable            = dataTable.getIntValue(SkillObject::ms_isSearchableLabel, skillRow) != 0;

	return true;
}

//----------------------------------------------------------------------

const SkillObject * SkillObject::findProfessionForSkill          () const
{
	const SkillObject * s = this;

	while (s)
	{
		if (s->isProfession ())
			return s;

		s = s->getPrevSkill ();
	}

	return 0;
}

//-----------------------------------------------------------------

bool SkillObject::dependsUponSkill (const SkillObject & skill, bool immediatePrereqsOnly) const
{
	bool result = false;

	++s_recursionCountDepends;

	if (s_recursionCountDepends >= s_recursionCountDependsMax)
	{
		WARNING (true, ("recursion max hit in skill dependency check"));
	}
	else if (this == &skill)
	{
		result = true;
	}
	else
	{		
		const SkillVector & preReqs = getPrerequisiteSkills();
		for (SkillVector::const_iterator it = preReqs.begin (); it != preReqs.end (); ++it)
		{
			const SkillObject * const prereq = NON_NULL (*it);
			
			if(immediatePrereqsOnly)
			{
				if(prereq == &skill)
					result = true;
			}
			else
			{			
				if (prereq->dependsUponSkill (skill))
				{
					result = true;
				}
			}
		}
	}
	
	--s_recursionCountDepends;
	
	return result;
}

//---------------------------------------------------------------------
