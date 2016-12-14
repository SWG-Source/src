// ======================================================================
//
// AiShipBehaviorAttackFighter_Maneuver_Path.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AiShipBehaviorAttackFighter_Maneuver_Path_H
#define INCLUDED_AiShipBehaviorAttackFighter_Maneuver_Path_H

#include "serverGame/AiShipBehaviorAttackFighter_Maneuver.h"

class Transform;

// ======================================================================

class AiShipBehaviorAttackFighter::Maneuver::Path
{
public:
	typedef Vector Node;
	typedef int Index;

	enum PathTypes
	{
		PT_empty,
		PT_first,
		PT_helix = PT_first,
		PT_loop, // guarantees final point of the generated "createPath" path is the endPosition
		PT_last = PT_loop,
		PT_count = PT_last
	};

	bool getNextNode(Index & currentIndex, Node & desiredPosition, Transform const & currentLocation, float const distanceTolerance) const;
	void addNode(Node const & node);
	bool getNode(Index const & index, Node & node) const;
	bool getFrontNode(Node & node) const;
	bool isEmpty() const;
	void popFrontNode();
	void clearNodes();
	int getLength() const;

	static Path * createPath(PathTypes const pathType,				// Selects the path type.
								float const complexity, 			// Complexity of the path. (skill)
								float const tension,				// How tight the path follows control points. (aggresiveness)
								float const objectRadius,			// The size of the thing moving through the paths.
								float const turnRadius,
								Transform const & startXForm,		// Starting position of the path.
								Vector const & endPosition);

	static Path * createPath();

	static void deletePath(Path * path);

private:
	Path();
	virtual ~Path();

	typedef std::deque<Node> NodeList;
	NodeList * const m_nodeList;

private: // Disabled.
	Path(Path const &);
	Path &operator=(Path const &);
};

#endif
