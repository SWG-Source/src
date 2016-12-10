// ======================================================================
// 
// SpaceDockingManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_SpaceDockingManager_H
#define INCLUDED_SpaceDockingManager_H

class NetworkId;
class Object;
class Transform;

// ----------------------------------------------------------------------
class SpaceDockingManager
{
public:

	typedef std::list<Transform> HardPointList;

	static void install();

	static void fetchDockingProcedure(Object const & dockingUnit, Object const & dockTarget, Transform & dockHardPoint, HardPointList & approachHardPointList, HardPointList & exitHardPointList);
	static void releaseDockingProcedure(NetworkId const & dockingUnit, NetworkId const & dockTarget);

private:

	// Disable

	SpaceDockingManager();
	~SpaceDockingManager();
	SpaceDockingManager(SpaceDockingManager const &);
	SpaceDockingManager & operator =(SpaceDockingManager const &);
};

// ======================================================================

#endif // INCLUDED_SpaceDockingManager_H
