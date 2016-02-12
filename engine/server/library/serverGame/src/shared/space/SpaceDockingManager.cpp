// ======================================================================
//
// SpaceDockingManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SpaceDockingManager.h"

#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedLog/Log.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Transform.h"
#include "sharedObject/Appearance.h"

#include <list>
#include <map>
#include <set>
#include <vector>

//-- TODO: Currently, hardpoints are not correctly oriented.  We will need to coordinate with art to fix them and remove this hack.
#define FIXUP_DOCKING_HARDPOINTS 1

// ======================================================================
//
// SpaceDockingManagerNamespace
//
// ======================================================================

namespace SpaceDockingManagerNamespace
{
	class DockPad;
	class DockableShip;

	typedef std::vector<DockPad> DockPadList;
	typedef std::map<CachedNetworkId, DockableShip> ShipsBeingDockedList;
	typedef std::set<NetworkId> OccupantList;

	bool getHardPoints(Object const & object, char const * const hardPointName, char const hardPointLetter, SpaceDockingManager::HardPointList & hardPointList);
	float getCollisionRadius(Object const & object);
	Transform getDockHardPoint(Object const & object);
	float getShortExtentAxisLength(Object const & object);
	void remove();

	class DockPad
	{
	public:

		DockPad()
		 : m_dockHardPoint()
		 , m_occupantList()
		 , m_radiusSquared(0.0f)
		 , m_approachHardPointList()
		 , m_exitHardPointList()
		 , m_dryDock(false)
		{
		}

		Transform m_dockHardPoint;
		OccupantList m_occupantList;
		float m_radiusSquared;
		SpaceDockingManager::HardPointList m_approachHardPointList;
		SpaceDockingManager::HardPointList m_exitHardPointList;
		bool m_dryDock;
	};

	class DockableShip
	{
	public:

		explicit DockableShip(Object const & object)
		 : m_dockPadList()
		{
			if (object.getAppearance() != nullptr)
			{
				// What dock pads do we have, and what are their radii

				bool done = false;
				char dockLetter = 97; // start with 'a'

				do
				{
					char text[256];
					snprintf(text, sizeof(text) - 1, "dock_%c", dockLetter);
					text[sizeof(text) - 1] = '\0';
					DockPad dockPad;
					if (object.getAppearance()->findHardpoint(TemporaryCrcString(text, false), dockPad.m_dockHardPoint))
					{
#if FIXUP_DOCKING_HARDPOINTS
						//-- TODO: Currently, hardpoints are not correctly oriented.  We will need to coordinate with art to fix them and remove this hack.
						dockPad.m_dockHardPoint.pitch_l(PI_OVER_2);
						dockPad.m_dockHardPoint.roll_l(PI_OVER_2);
#endif

						// Get the radius

						Transform dockRadiusHardPoint;
						snprintf(text, sizeof(text) - 1, "dockradius_%c", dockLetter);
						text[sizeof(text) - 1] = '\0';
						if (object.getAppearance()->findHardpoint(TemporaryCrcString(text, false), dockRadiusHardPoint))
						{
							dockPad.m_radiusSquared = dockPad.m_dockHardPoint.getPosition_p().magnitudeBetweenSquared(dockRadiusHardPoint.getPosition_p());
						}

						// Get the approach hard points

						if (!getHardPoints(object, "approach", dockLetter, dockPad.m_approachHardPointList))
						{
							DEBUG_WARNING(true, ("debug_ai: Missing approach hard points on object(%s)", object.getDebugInformation().c_str()));
						}

						// Get the exit hard points

						if (!getHardPoints(object, "exit", dockLetter, dockPad.m_exitHardPointList))
						{
							DEBUG_WARNING(true, ("debug_ai: Missing exit hard points on object(%s)", object.getDebugInformation().c_str()));
						}

						m_dockPadList.push_back(dockPad);
						++dockLetter;
					}
					else
					{
						done = true;
					}
				} while (!done);

				// Does this ship have a dry dock?

				Transform dryDockHardPoint;

				if (object.getAppearance()->findHardpoint(TemporaryCrcString("drydock_1", false), dryDockHardPoint))
				{
					DockPad dockPad;
					dockPad.m_dockHardPoint = dryDockHardPoint;
					dockPad.m_radiusSquared = -1.0f;
					dockPad.m_dryDock = true;
					m_dockPadList.push_back(dockPad);
				}
			}
		}

		int getOccupantCount() const
		{
			int result = 0;
			DockPadList::const_iterator iterDockPadList = m_dockPadList.begin();

			for (; iterDockPadList != m_dockPadList.end(); ++iterDockPadList)
			{
				result += static_cast<int>(iterDockPadList->m_occupantList.size());
			}

			return result;
		}

		DockPadList m_dockPadList;
	};

	ShipsBeingDockedList s_shipsBeingDockedList;
	float const s_dockingApproachRadiusGain = 3.0f;
	float const s_dockingExitRadiusGain = 5.0f;
}

using namespace SpaceDockingManagerNamespace;

// ----------------------------------------------------------------------
float SpaceDockingManagerNamespace::getCollisionRadius(Object const & object)
{
	CollisionProperty const * const collisionProperty = object.getCollisionProperty();
	float radius = 0.0f;

	if (collisionProperty != nullptr)
	{
		radius = collisionProperty->getBoundingSphere_l().getRadius();
	}

	return radius;
}

// ----------------------------------------------------------------------
float SpaceDockingManagerNamespace::getShortExtentAxisLength(Object const & object)
{
	float result;
	CollisionProperty const * const collisionProperty = NON_NULL(object.getCollisionProperty());
	AxialBox const axialBox = collisionProperty->getExtent_l()->getBoundingBox();

	if (   (axialBox.getWidth() < axialBox.getHeight())
		&& (axialBox.getWidth() < axialBox.getDepth()))
	{
		// Width is the short axis

		result = axialBox.getMax().x;
	}
	else if (axialBox.getHeight() < axialBox.getDepth())
	{
		// Height is the short axis

		result = axialBox.getMax().y;
	}
	else
	{
		// Depth is the short axis

		result = axialBox.getMax().z;
	}

	return result;
}

// ----------------------------------------------------------------------
Transform SpaceDockingManagerNamespace::getDockHardPoint(Object const & object)
{
	Transform result;
	CollisionProperty const * const collisionProperty = NON_NULL(object.getCollisionProperty());
	AxialBox const axialBox = collisionProperty->getExtent_l()->getBoundingBox();

	// Find the short axis of the object's extent

	if (   (axialBox.getWidth() < axialBox.getHeight())
		&& (axialBox.getWidth() < axialBox.getDepth()))
	{
		// Width is the short axis

		result.yaw_l(-PI_OVER_2);
		result.move_l(Vector(0.0f, 0.0f, -axialBox.getMax().x));
	}
	else if (axialBox.getHeight() < axialBox.getDepth())
	{
		// Height is the short axis

		result.pitch_l(PI_OVER_2);
		result.move_l(Vector(0.0f, 0.0f, -axialBox.getMax().y));
	}
	else
	{
		// Depth is the short axis

		result.yaw_l(PI);
		result.move_l(Vector(0.0f, 0.0f, -axialBox.getMax().z));
	}

	return result;
}

// ----------------------------------------------------------------------
bool SpaceDockingManagerNamespace::getHardPoints(Object const & object, char const * const hardPointName, char const hardPointLetter, SpaceDockingManager::HardPointList & hardPointList)
{
	NOT_NULL(hardPointName);
	NOT_NULL(hardPointLetter);

	hardPointList.clear();

	if (object.getAppearance() != nullptr)
	{
		int hardPointIndex = 1;
		bool done = false;

		while (!done)
		{
			char text[256];
			snprintf(text, sizeof(text) - 1, "%s_%c_%d", hardPointName, hardPointLetter, hardPointIndex);
			text[sizeof(text) - 1] = '\0';
			Transform transform;
			if (object.getAppearance()->findHardpoint(TemporaryCrcString(text, false), transform))
			{
#if FIXUP_DOCKING_HARDPOINTS
				//-- TODO: Currently, hardpoints are not correctly oriented.  We will need to coordinate with art to fix them and remove this hack.
				transform.pitch_l(PI_OVER_2);
				transform.roll_l(PI_OVER_2);
#endif

				hardPointList.push_back(transform);
				++hardPointIndex;
			}
			else
			{
				done = true;
			}
		}
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("SpaceDockingManagerNamespace::getHardPoints() Why does this object(%s) have no appearance?", object.getDebugInformation().c_str()));
	}

	return !hardPointList.empty();
}

// ----------------------------------------------------------------------
void SpaceDockingManagerNamespace::remove()
{
	if (!s_shipsBeingDockedList.empty())
	{
		WARNING(true, ("debug_ai: The s_shipsBeingDockedList is not empty(%u), this is a sign of a reference counting problem.", s_shipsBeingDockedList.size()));
	}
}

// ======================================================================
//
// SpaceDockingManager
//
// ======================================================================

// ----------------------------------------------------------------------
void SpaceDockingManager::install()
{
	ExitChain::add(&remove, "SpaceDockingManagerNamespace::remove");
}

// ----------------------------------------------------------------------
void SpaceDockingManager::fetchDockingProcedure(Object const & dockingUnit, Object const & dockTarget, Transform & dockHardPoint, HardPointList & approachHardPointList, HardPointList & exitHardPointList)
{
	approachHardPointList.clear();
	exitHardPointList.clear();

	// Find the correct dock pad to set down on

	ShipsBeingDockedList::iterator iterShipsBeingDockedList = s_shipsBeingDockedList.find(CachedNetworkId(dockTarget));

	if (iterShipsBeingDockedList == s_shipsBeingDockedList.end())
	{
		// The unit is not currently being docked, setup the data for storing the docking occupants

		std::pair<ShipsBeingDockedList::iterator, bool> result = s_shipsBeingDockedList.insert(std::make_pair(CachedNetworkId(dockTarget), DockableShip(dockTarget)));

		iterShipsBeingDockedList = result.first;
	}

	// Find an open docking pad that the docking unit can fit onto

	DockableShip & dockableShip = iterShipsBeingDockedList->second;
	typedef std::multimap<unsigned int /* occupantCount */, int /* index */> DockPadPriorityList;
	DockPadPriorityList dockPadPriorityList;

	{
		float const dockingUnitRadiusSquared = sqr(getCollisionRadius(dockingUnit));
		int dockPadIndex = 0;
		DockPadList::const_iterator iterDockPadList = dockableShip.m_dockPadList.begin();

		for (; iterDockPadList != dockableShip.m_dockPadList.end(); ++iterDockPadList)
		{
			DockPad const & dockPad = *iterDockPadList;

			if (   (dockingUnitRadiusSquared <= dockPad.m_radiusSquared)
				|| (dockPad.m_radiusSquared < 0.0f))
			{
				// This is a possible pad to dock on

				if (dockPad.m_radiusSquared < 0.0f)
				{
					// Dry dock should be the last resort to dock on, so set its occupant count high

					IGNORE_RETURN(dockPadPriorityList.insert(std::make_pair(9999, dockPadIndex)));
				}
				else
				{
					IGNORE_RETURN(dockPadPriorityList.insert(std::make_pair(dockPad.m_occupantList.size(), dockPadIndex)));
				}
			}
			++dockPadIndex;
		}
	}

	// If there is any results in the dockPriorityList, then we will set down on the least occupied pad

	if (!dockPadPriorityList.empty())
	{
		DockPad & dockPad = dockableShip.m_dockPadList[static_cast<unsigned int>(dockPadPriorityList.begin()->second)];

		// Add an occupant to this dock pad

		IGNORE_RETURN(dockPad.m_occupantList.insert(dockingUnit.getNetworkId()));

		if (dockPad.m_dryDock)
		{
			// Since this is a dry dock, we must take into account the radius of the ship docking with the target

			float const dockingUnitRadius = getCollisionRadius(dockingUnit);
			float const dockingUnitShortExtentAxisLength = getShortExtentAxisLength(dockingUnit);
		
			Transform dryDockHardPoint(dockPad.m_dockHardPoint);
			dryDockHardPoint.move_l(Vector(0.0f, 0.0f, -dockingUnitShortExtentAxisLength));
			dockHardPoint = dryDockHardPoint;

			Transform approachHardPoint(dryDockHardPoint);
			approachHardPoint.move_l(Vector(0.0f, 0.0f, -dockingUnitRadius * s_dockingApproachRadiusGain));
			approachHardPointList.push_back(approachHardPoint);

			Transform exitHardPoint(dryDockHardPoint);
			exitHardPoint.move_l(Vector(0.0f, 0.0f, -dockingUnitRadius * s_dockingExitRadiusGain));
			exitHardPointList.push_back(exitHardPoint);
		}
		else
		{
			float const dockingUnitShortExtentAxisLength = getShortExtentAxisLength(dockingUnit);
			dockHardPoint = dockPad.m_dockHardPoint;
			dockHardPoint.move_l(Vector(0.0f, 0.0f, -dockingUnitShortExtentAxisLength));

			approachHardPointList = dockPad.m_approachHardPointList;
			exitHardPointList = dockPad.m_exitHardPointList;
		}
	}
	else
	{
		// No dry dock, we will dock beside the unit against the long local axis
		
		Transform const dockTargetHardPoint(getDockHardPoint(dockTarget));
		float const dockingUnitRadius = getCollisionRadius(dockingUnit);
		float const dockingUnitShortExtentAxisLength = getShortExtentAxisLength(dockingUnit);

		dockHardPoint = dockTargetHardPoint;
		dockHardPoint.move_l(Vector(0.0f, 0.0f, -dockingUnitShortExtentAxisLength));

		Transform approachHardPoint(dockTargetHardPoint);
		approachHardPoint.move_l(Vector(0.0f, 0.0f, -dockingUnitRadius * s_dockingApproachRadiusGain));
		approachHardPointList.push_back(approachHardPoint);

		Transform exitHardPoint(dockTargetHardPoint);
		exitHardPoint.move_l(Vector(0.0f, 0.0f, -dockingUnitRadius * s_dockingExitRadiusGain));
		exitHardPointList.push_back(exitHardPoint);
	}
}

// ----------------------------------------------------------------------
void SpaceDockingManager::releaseDockingProcedure(NetworkId const & dockingUnit, NetworkId const & dockTarget)
{
	ShipsBeingDockedList::iterator iterShipsBeingDockedList = s_shipsBeingDockedList.find(CachedNetworkId(dockTarget));

	if (iterShipsBeingDockedList != s_shipsBeingDockedList.end())
	{
		// Remove the docking unit from the pad

		DockableShip & dockableShip = iterShipsBeingDockedList->second;
		DockPadList::iterator iterDockPadList = dockableShip.m_dockPadList.begin();
		bool foundUnit = false;

		for (; iterDockPadList != dockableShip.m_dockPadList.end(); ++iterDockPadList)
		{
			OccupantList::iterator iterOccupantList = iterDockPadList->m_occupantList.find(dockingUnit);

			if (iterOccupantList != iterDockPadList->m_occupantList.end())
			{
				LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("SpaceDockingManager::releaseDockingProcedure() dockingUnit(%s) dockTarget(%s) Removing unit from docking pad.", dockingUnit.getValueString().c_str(), dockTarget.getValueString().c_str()));

				iterDockPadList->m_occupantList.erase(iterOccupantList);
				foundUnit = true;
				break;
			}
		}

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled() && !foundUnit, "debug_ai", ("SpaceDockingManager::releaseDockingProcedure() dockingUnit(%s) dockTarget(%s) Unable to find the dockingUnit on a dock pad.", dockingUnit.getValueString().c_str(), dockTarget.getValueString().c_str()));

		if (iterShipsBeingDockedList->second.getOccupantCount() <= 0)
		{
			// No more ships are using this ship to dock, stop tracking it

			s_shipsBeingDockedList.erase(iterShipsBeingDockedList);

			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("SpaceDockingManager::releaseDockingProcedure() REMOVING dockTarget(%s) s_shipsBeingDockedList.size(%u)", dockTarget.getValueString().c_str(), s_shipsBeingDockedList.size()));
		}
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("SpaceDockingManager::releaseDockingProcedure() Why is this dockTarget(%s) not in the ShipsBeingDockedList?", dockTarget.getValueString().c_str()));
	}
}

// ======================================================================
