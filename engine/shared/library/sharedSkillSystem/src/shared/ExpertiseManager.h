//======================================================================
//
// ExpertiseManager.h
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ExpertiseManager_H
#define INCLUDED_ExpertiseManager_H

//======================================================================

class ExpertiseManager
{
public:

	static void install();
	static void remove();

	static std::string const & getExpertiseDatatableName();

	// Expertise Points
	static int getExpertisePointsForLevel(int level);

	// Expertise Trees
	typedef std::vector<int> TreeIdList;
	static void getExpertiseTrees(TreeIdList & treeIdList);
	static TreeIdList const & getExpertiseTreesForProfession(std::string const & skillTemplate);
	static std::string const & getExpertiseTreeNameFromId(int treeId);
	static std::string const & getExpertiseTreeUiBackgroundIdFromId(int treeId);

	// Individual Expertises
	static bool isExpertise(SkillObject const * skill);
	static int getExpertiseTree(std::string const & expertiseName);
	static int getExpertiseGrid(std::string const & expertiseName);
	static int getExpertiseTier(std::string const & expertiseName);
	static int getExpertiseRank(std::string const & expertiseName);
	static int getExpertiseRankMax(std::string const & expertiseName);

	// Expertise Grid

	static int const getNumExpertiseColumns();
	static int const getNumExpertiseTiers();

	static SkillObject const * getExpertiseSkillAt(int tree, int tier, int grid, int rank = 1);

	struct ExpertiseCoord
	{
		int tree;
		int tier;
		int grid;
		int rank;

		ExpertiseCoord();
		ExpertiseCoord(ExpertiseCoord const & rhs);
		ExpertiseCoord(int const treeValue, int const tierValue, int const gridValue);
		ExpertiseCoord(int const treeValue, int const tierValue, int const gridValue, int const rankValue);
		
		ExpertiseCoord const & operator=(ExpertiseCoord const & rhs);
		bool operator==(ExpertiseCoord const & rhs) const;
		bool operator!=(ExpertiseCoord const & rhs) const;
		bool operator<(ExpertiseCoord const & rhs) const;
	};

};

//======================================================================

#endif
