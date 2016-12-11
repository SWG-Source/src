// ======================================================================
//
// PathSearch.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_PathSearch_H
#define	INCLUDED_PathSearch_H

class PathGraph;
class PathNode;
class PathSearch;
class PathSearchNode;
class PathSearchQueue;


struct PathNodeHasher
{
	size_t operator() ( PathNode const * node ) const;
};

typedef std::vector<int> IndexList;
typedef std::vector<PathNode const *> NodeList;

// ======================================================================

class PathSearch
{
public:

	PathSearch();
	~PathSearch();

	static void       install       ( void );

	// ----------

	bool              search        ( PathGraph const * graph, int startIndex, int goalIndex );

	bool              search        ( PathGraph const * graph, int startIndex, IndexList const & goalIndices );

	IndexList const & getPath       ( void ) const;

protected:

	PathSearchNode *  search        ( void );

	float             costBetween   ( PathNode const * A, PathNode const * B ) const;
	
	float             calcHeuristic ( PathNode const * A ) const;
	float             calcHeuristic ( PathNode const * A, PathNode const * goal ) const;

	void              cleanup       ( void );

	bool              buildPath     ( PathSearchNode * endNode );

	bool              atGoal        ( PathSearchNode * endNode ) const;

	// ----------

	friend class PathSearchNode;

	PathGraph const * m_graph;
	PathNode const *  m_start;

	PathNode const *  m_goal;

	bool              m_multiGoal;
	NodeList *        m_goals;

	PathSearchQueue * m_queue;

	IndexList *       m_path;

	NodeList *        m_visitedNodes;
};

// ======================================================================

#endif // INCLUDED_PathSearch_H

