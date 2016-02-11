//======================================================================
//
// ExpertiseManager.cpp
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#include "sharedSkillSystem/FirstSharedSkillSystem.h"
#include "sharedSkillSystem/ExpertiseManager.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>

//======================================================================

namespace ExpertiseManagerNamespace
{
	bool s_installed = false;

	std::string const cs_expertiseSkillCategoryName("expertise");

	int const cs_numExpertiseColumns = 7;
	int const cs_numExpertiseTiers   = 5;

	std::string const cs_emptyString;
	ExpertiseManager::TreeIdList const cs_emptyTreeIdList;
	DataTable const cs_unusedDataTable;

	// for direct access to expertise datatable
	DataTable const * s_expertiseDatatable;

	// grid x,y,z -> expertise skill name
	typedef std::map<ExpertiseManager::ExpertiseCoord, std::string> ExpertiseGrid;
	ExpertiseGrid s_expertiseGrid;
	std::string const cs_expertiseDatatableName("datatables/expertise/expertise.iff");
	void loadExpertiseTable(DataTable const & datatable);

	// character level --> expertise points awarded
	typedef std::map<int, int> LevelToPointsMap;
	LevelToPointsMap s_expertisePointsForLevel;
	std::string const cs_expertisePointsDatatableName("datatables/player/player_level.iff");
	void loadExpertisePointsTable(DataTable const & datatable);

	// expertise tree id --> string id
	typedef std::map<int, std::string> TreeToStringIdMap;
	TreeToStringIdMap s_expertiseStringForTree;
	TreeToStringIdMap s_uiBackgroundIdForTree;
	std::string const cs_expertiseTreesDatatableName("datatables/expertise/expertise_trees.iff");
	void loadExpertiseTreesTable(DataTable const & datatable);

	// skill template --> expertise tree id list
	typedef std::map<std::string, ExpertiseManager::TreeIdList> ProfessionToTreeMap; 
	ProfessionToTreeMap s_expertiseTreesForProfession;
	std::string const cs_skillTemplateDatatableName("datatables/skill_template/skill_template.iff");
	void loadSkillTemplateTable(DataTable const & datatable);

	void splitString(char delim, std::string const & source, std::vector<std::string> & result);
};

using namespace ExpertiseManagerNamespace;

//======================================================================

void ExpertiseManagerNamespace::loadExpertiseTable(DataTable const & datatable)
{
	UNREF(datatable); // required for callback, but unused

	s_expertiseGrid.clear();

	s_expertiseDatatable = DataTableManager::getTable(cs_expertiseDatatableName, true);

	DEBUG_FATAL(!s_expertiseDatatable, ("ExpertiseManager: failed to load %s", cs_expertiseDatatableName.c_str()));

	int const numRows = s_expertiseDatatable->getNumRows();

	int const nameColumn = s_expertiseDatatable->findColumnNumber("NAME");
	int const treeColumn = s_expertiseDatatable->findColumnNumber("TREE");
	int const tierColumn = s_expertiseDatatable->findColumnNumber("TIER");
	int const gridColumn = s_expertiseDatatable->findColumnNumber("GRID");
	int const rankColumn = s_expertiseDatatable->findColumnNumber("RANK");

	for (int row = 0; row < numRows; ++row)
	{
		std::string const & name = s_expertiseDatatable->getStringValue(nameColumn, row);

		SkillObject const * skill = SkillManager::getInstance().getSkill(name);
		DEBUG_FATAL(!skill, ("ExpertiseManager: %s row %d: skill does not exist for %s", cs_expertiseDatatableName.c_str(), row, name.c_str()));

		int tree = s_expertiseDatatable->getIntValue(treeColumn, row);
		int tier = s_expertiseDatatable->getIntValue(tierColumn, row);
		int grid = s_expertiseDatatable->getIntValue(gridColumn, row);
		int rank = s_expertiseDatatable->getIntValue(rankColumn, row);

		DEBUG_WARNING(s_expertiseStringForTree.find(tree) == s_expertiseStringForTree.end(),
			("ExpertiseManager: %s row %d: expertise tree id %d for %s not defined in %s",
			cs_expertiseDatatableName.c_str(), row, tree, name.c_str(), cs_expertiseTreesDatatableName.c_str()));
		DEBUG_WARNING(tier > cs_numExpertiseTiers, ("ExpertiseManager: %s row %d: tier of %d for %s is greater than max of %d",
			cs_expertiseDatatableName.c_str(), row, tier, name.c_str(), cs_numExpertiseTiers));
		DEBUG_WARNING(grid > cs_numExpertiseColumns, ("ExpertiseManager: %s row %d: grid of %d for %s is greater than max of %d",
			cs_expertiseDatatableName.c_str(), row, grid, name.c_str(), cs_numExpertiseColumns));

		ExpertiseManager::ExpertiseCoord expertiseCoord(tree, tier, grid, rank);

		std::string const & skillName = skill->getSkillName();
		std::pair<ExpertiseGrid::iterator, bool> result = s_expertiseGrid.insert(std::make_pair(expertiseCoord, skillName));

		UNREF(result);
		//DEBUG_WARNING(!result.second, ("ExpertiseManager: %s row %d: duplicate expertise: %s", cs_expertiseDatatableName.c_str(), row, name.c_str()));
	}
}

//----------------------------------------------------------------------

void ExpertiseManagerNamespace::loadExpertisePointsTable(DataTable const & datatable)
{
	UNREF(datatable); // required for callback, but unused

	s_expertisePointsForLevel.clear();

	DataTable const * s_expertisePointsDatatable = DataTableManager::getTable(cs_expertisePointsDatatableName, true);

	DEBUG_FATAL(!s_expertisePointsDatatable, ("ExpertiseManager: failed to load %s", cs_expertisePointsDatatableName.c_str()));

	int const numRows = s_expertisePointsDatatable->getNumRows();

	int const levelColumn  = s_expertisePointsDatatable->findColumnNumber("level");
	int const pointsColumn = s_expertisePointsDatatable->findColumnNumber("expertise_points");

	for (int row = 0; row < numRows; ++row)
	{
		int level  = s_expertisePointsDatatable->getIntValue(levelColumn, row);
		int points = s_expertisePointsDatatable->getIntValue(pointsColumn, row);
		s_expertisePointsForLevel[level] = points;
	}
}

//----------------------------------------------------------------------

void ExpertiseManagerNamespace::loadExpertiseTreesTable(DataTable const & datatable)
{
	UNREF(datatable); // required for callback, but unused

	s_expertiseStringForTree.clear();

	DataTable const * s_expertiseTreesDatatable = DataTableManager::getTable(cs_expertiseTreesDatatableName, true);

	DEBUG_FATAL(!s_expertiseTreesDatatable, ("ExpertiseManager: failed to load %s", cs_expertiseTreesDatatableName.c_str()));

	int const numRows = s_expertiseTreesDatatable->getNumRows();

	int const treeIdColumn   = s_expertiseTreesDatatable->findColumnNumber("expertise_tree_id");
	int const stringIdColumn = s_expertiseTreesDatatable->findColumnNumber("expertise_tree_string_id");
	int const uiBackgroundIdColumn = s_expertiseTreesDatatable->findColumnNumber("ui_background_id");

	for (int row = 0; row < numRows; ++row)
	{
		int treeId                   = s_expertiseTreesDatatable->getIntValue(treeIdColumn, row);
		std::string const & stringId = s_expertiseTreesDatatable->getStringValue(stringIdColumn, row);
		s_expertiseStringForTree[treeId] = stringId;
		std::string const & uiStringId = s_expertiseTreesDatatable->getStringValue(uiBackgroundIdColumn, row);
		s_uiBackgroundIdForTree[treeId] = uiStringId;
	}
}

//----------------------------------------------------------------------

void ExpertiseManagerNamespace::loadSkillTemplateTable(DataTable const & datatable)
{
	UNREF(datatable); // required for callback, but unused

	s_expertiseTreesForProfession.clear();

	DataTable const * s_skillTemplateDatatable = DataTableManager::getTable(cs_skillTemplateDatatableName, true);

	DEBUG_FATAL(!s_skillTemplateDatatable, ("ExpertiseManager: failed to load %s", cs_skillTemplateDatatableName.c_str()));

	int const numRows = s_skillTemplateDatatable->getNumRows();

	int const templateNameColumn   = s_skillTemplateDatatable->findColumnNumber("templateName");
	int const expertiseTreesColumn = s_skillTemplateDatatable->findColumnNumber("expertiseTrees");

	for (int row = 0; row < numRows; ++row)
	{
		std::string const & templateName   = s_skillTemplateDatatable->getStringValue(templateNameColumn, row);
		std::string const & expertiseTrees = s_skillTemplateDatatable->getStringValue(expertiseTreesColumn, row);

		if (expertiseTrees.empty())
		{
			continue;
		}

		ExpertiseManager::TreeIdList treeIdList;
		std::vector<std::string> stringList;
		splitString(',', expertiseTrees, stringList);

		for (std::vector<std::string>::const_iterator i = stringList.begin(); i != stringList.end(); ++i)
		{
			int treeId = atoi((*i).c_str());
			DEBUG_FATAL(treeId < 1, ("ExpertiseManager: %s row %d: invalid tree id %d", cs_skillTemplateDatatableName.c_str(), row, treeId));
			treeIdList.push_back(treeId);
		}

		s_expertiseTreesForProfession.insert(std::make_pair(templateName, treeIdList));
	}
}

//----------------------------------------------------------------------

void ExpertiseManagerNamespace::splitString(char delim, std::string const & source, std::vector<std::string> & result)
{
	std::string::size_type curPos = 0;
	while(curPos != std::string::npos)
	{
		std::string::size_type nextPos = source.find(delim, curPos);
		std::string element = source.substr(curPos, nextPos - curPos);
		result.push_back(element);
		curPos = nextPos;
		if(curPos != std::string::npos)
			curPos++;
	}
}

//======================================================================

void ExpertiseManager::install()
{
	DEBUG_FATAL(s_installed, ("ExpertiseManager already installed"));
	s_installed = true;

	loadSkillTemplateTable(cs_unusedDataTable);
	DataTableManager::addReloadCallback(cs_skillTemplateDatatableName, &loadSkillTemplateTable);

	loadExpertisePointsTable(cs_unusedDataTable);
	DataTableManager::addReloadCallback(cs_expertisePointsDatatableName, &loadExpertisePointsTable);

	loadExpertiseTreesTable(cs_unusedDataTable);
	DataTableManager::addReloadCallback(cs_expertiseTreesDatatableName, &loadExpertiseTreesTable);

	loadExpertiseTable(cs_unusedDataTable);
	DataTableManager::addReloadCallback(cs_expertiseDatatableName, &loadExpertiseTable);

	ExitChain::add(ExpertiseManager::remove, "ExpertiseManager");
}

//----------------------------------------------------------------------

void ExpertiseManager::remove()
{
	DEBUG_FATAL(!s_installed, ("ExpertiseManager not installed"));
	s_installed = false;
}

//----------------------------------------------------------------------

/**
 * @return the iff path of the expertise datatable
 */
std::string const & ExpertiseManager::getExpertiseDatatableName()
{
	return cs_expertiseDatatableName;
}

//----------------------------------------------------------------------

/**
 * @return int - the number of expertise columns in the grid
 *         (i.e. max x coordinate)
 */
int const ExpertiseManager::getNumExpertiseColumns()
{
	return cs_numExpertiseColumns;
}

//----------------------------------------------------------------------

/**
 * @return int - the number of expertise tiers in the grid
 *         (i.e. max y coordinate)
 */
int const ExpertiseManager::getNumExpertiseTiers()
{
	return cs_numExpertiseTiers;
}

//----------------------------------------------------------------------

/**
 * @param tree - tree id number
 * @param tier - tier number (aka "y coordinate")
 * @param grid - grid number (aka "x coordinate")
 * @param rank - rank number (aka "z coordinate"). a rank
 *             of 1 is assumed when a particular rank is
 *             not needed, since rank 1 is guaranteed to
 *             exist.
 * 
 * @return - skill object for expertise at grid location.
 *         returns nullptr if none found
 */
SkillObject const * ExpertiseManager::getExpertiseSkillAt(int tree, int tier, int grid, int rank)
{
	SkillObject const * skill = 0;

	ExpertiseCoord expertiseCoord(tree, tier, grid, rank);

	ExpertiseGrid::const_iterator i = s_expertiseGrid.find(expertiseCoord);
	if (i != s_expertiseGrid.end())
	{
		std::string const & skillName = (*i).second;
		skill = SkillManager::getInstance().getSkill(skillName);
	}

	return skill;
}

//----------------------------------------------------------------------

/**
 * @param level - a character's combat level
 * 
 * @return int - max Expertise Points available at that level
 */
int ExpertiseManager::getExpertisePointsForLevel(int level)
{
	int totalPoints = 0;

	for (LevelToPointsMap::const_iterator i = s_expertisePointsForLevel.begin(); i != s_expertisePointsForLevel.end(); ++i)
	{
		int const levelForRow  = (*i).first;
		int const pointsForRow = (*i).second;
		if (levelForRow <= level)
		{
			totalPoints += pointsForRow;
		}
	}

	return totalPoints;
}

//----------------------------------------------------------------------

/**
 * @param empty list to populate with all existing tree ids
 */
void ExpertiseManager::getExpertiseTrees(ExpertiseManager::TreeIdList & treeIdList)
{
	treeIdList.clear();
	for (TreeToStringIdMap::const_iterator i = s_expertiseStringForTree.begin(); i != s_expertiseStringForTree.end(); ++i)
	{
		treeIdList.push_back((*i).first);
	}
}

//----------------------------------------------------------------------

/**
 * @param skillTemplate - a profession template name from skill_template table
 * 
 * @return list of tree id ints for profession
 */
ExpertiseManager::TreeIdList const & ExpertiseManager::getExpertiseTreesForProfession(std::string const & skillTemplate)
{
	ProfessionToTreeMap::const_iterator i = s_expertiseTreesForProfession.find(skillTemplate);
	if (i != s_expertiseTreesForProfession.end())
	{
		return (*i).second;
	}

	return cs_emptyTreeIdList;
}

//----------------------------------------------------------------------

/**
 * @param treeId - int id of expertise tree
 * 
 * @return string name of expertise tree, suitable for use in
 *         creating a StringId and localizing
 */
std::string const & ExpertiseManager::getExpertiseTreeNameFromId(int treeId)
{
	TreeToStringIdMap::const_iterator i = s_expertiseStringForTree.find(treeId);
	if (i != s_expertiseStringForTree.end())
	{
		return (*i).second;
	}

	return cs_emptyString;
}

//----------------------------------------------------------------------

std::string const & ExpertiseManager::getExpertiseTreeUiBackgroundIdFromId(int treeId)
{
	TreeToStringIdMap::const_iterator i =  s_uiBackgroundIdForTree.find(treeId);
	if (i != s_uiBackgroundIdForTree.end())
	{
		return (*i).second;
	}

	return cs_emptyString;
}

//----------------------------------------------------------------------

/**
 * @param skill - pointer to a skill
 * 
 * @return bool - true if skill is an Expertise skill
 */
bool ExpertiseManager::isExpertise(SkillObject const * skill)
{
	bool result = false;

	if (skill)
	{
		SkillObject const * category = skill->findCategory();
		if (category)
		{
			std::string const & categoryName = category->getSkillName();
			if (categoryName == cs_expertiseSkillCategoryName)
			{
				result = true;
			}
		}
	}

	return result;
}

//----------------------------------------------------------------------

/**
 * @param expertiseName - name of the expertise
 * 
 * @return int - the id number of the expertise
 */
int ExpertiseManager::getExpertiseTree(std::string const & expertiseName)
{
	int result = 0;

	if (s_expertiseDatatable)
	{
		int rowNum = s_expertiseDatatable->searchColumnString(0, expertiseName);
		if(rowNum == -1)
		{
			DEBUG_WARNING(true, ("while looking for tree that expertise %s belongs to, could not find expertise", expertiseName.c_str()));
			return -1;
		}
		result = s_expertiseDatatable->getIntValue("TREE", rowNum);
	}

	return result;
}

//----------------------------------------------------------------------

/**
 * @param expertiseName - name of the expertise
 * 
 * @return int - the grid (aka "x coordinate") of the expertise
 */
int ExpertiseManager::getExpertiseGrid(std::string const & expertiseName)
{
	int result = 0;

	if (s_expertiseDatatable)
	{
		int rowNum = s_expertiseDatatable->searchColumnString(0, expertiseName);
		result = s_expertiseDatatable->getIntValue("GRID", rowNum);
	}

	return result;
}

//----------------------------------------------------------------------

/**
 * @param expertiseName - name of the expertise
 * 
 * @return int - the tier (aka "y coordinate") of the expertise
 */
int ExpertiseManager::getExpertiseTier(std::string const & expertiseName)
{
	int result = 0;

	if (s_expertiseDatatable)
	{
		int rowNum = s_expertiseDatatable->searchColumnString(0, expertiseName);
		result = s_expertiseDatatable->getIntValue("TIER", rowNum);
	}

	return result;
}

//----------------------------------------------------------------------

/**
 * @param expertiseName - name of the expertise
 * 
 * @return int - the rank (aka "z coordinate") of the expertise
 */
int ExpertiseManager::getExpertiseRank(std::string const & expertiseName)
{
	int result = 0;

	if (s_expertiseDatatable)
	{
		int rowNum = s_expertiseDatatable->searchColumnString(0, expertiseName);
		result = s_expertiseDatatable->getIntValue("RANK", rowNum);
	}

	return result;
}

//----------------------------------------------------------------------

/**
 * @param expertiseName - name of the expertise
 * 
 * @return int - the highest existing rank (aka "z coordinate") of the expertise
 */
int ExpertiseManager::getExpertiseRankMax(std::string const & expertiseName)
{
	int result = 0;

	int tree = getExpertiseTree(expertiseName);
	int tier = getExpertiseTier(expertiseName);
	int grid = getExpertiseGrid(expertiseName);
	int rank = getExpertiseRank(expertiseName);

	while (getExpertiseSkillAt(tree, tier, grid, rank) != 0)
	{
		result = rank;
		++rank;
	}

	return result;
}

//======================================================================

ExpertiseManager::ExpertiseCoord::ExpertiseCoord() :
tree(1),
tier(1),
grid(1),
rank(1)
{
}

// ----------------------------------------------------------------------

ExpertiseManager::ExpertiseCoord::ExpertiseCoord(ExpertiseCoord const & rhs) :
tree(rhs.tree),
tier(rhs.tier),
grid(rhs.grid),
rank(rhs.rank)
{
}

// ----------------------------------------------------------------------

ExpertiseManager::ExpertiseCoord::ExpertiseCoord(int const treeValue, int const tierValue, int const gridValue) :
tree(treeValue),
tier(tierValue),
grid(gridValue),
rank(1)
{
}

// ----------------------------------------------------------------------

ExpertiseManager::ExpertiseCoord::ExpertiseCoord(int const treeValue, int const tierValue, int const gridValue, int const rankValue) :
tree(treeValue),
tier(tierValue),
grid(gridValue),
rank(rankValue)
{
}

// ----------------------------------------------------------------------

ExpertiseManager::ExpertiseCoord const & ExpertiseManager::ExpertiseCoord::operator=(ExpertiseManager::ExpertiseCoord const & rhs)
{
	tree = rhs.tree;
	tier = rhs.tier;
	grid = rhs.grid;
	rank = rhs.rank;
	return *this;
}

// ----------------------------------------------------------------------

bool ExpertiseManager::ExpertiseCoord::operator==(ExpertiseManager::ExpertiseCoord const & rhs) const
{
	return (tree == rhs.tree) && (tier == rhs.tier) && (grid == rhs.grid) && (rank == rhs.rank);
}

// ----------------------------------------------------------------------

bool ExpertiseManager::ExpertiseCoord::operator!=(ExpertiseManager::ExpertiseCoord const & rhs) const
{
	return !((tree == rhs.tree) && (tier == rhs.tier) && (grid == rhs.grid) && (rank == rhs.rank));
}

// ----------------------------------------------------------------------

bool ExpertiseManager::ExpertiseCoord::operator<(ExpertiseManager::ExpertiseCoord const & rhs) const
{
	if (tree < rhs.tree)
	{
		return true;
	}
	if (tree == rhs.tree)
	{
		if (tier < rhs.tier)
		{
			return true;
		}
		if (tier == rhs.tier)
		{
			if (grid < rhs.grid)
			{
				return true;
			}
			if (grid == rhs.grid)
			{
				return (rank < rhs.rank);
			}
		}
	}
	return false;
}

//======================================================================

