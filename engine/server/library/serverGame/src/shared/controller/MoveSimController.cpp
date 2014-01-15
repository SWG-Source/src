//======================================================================
//
// MoveSimController.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/MoveSimController.h"

#include "serverGame/CreatureObject.h"
#include "sharedObject/AlterResult.h"
#include "sharedTerrain/TerrainObject.h"

//-----------------------------------------------------------------------

/**
*
* @param centerPoint the point to wander around
* @param roamRadius the radius around the point to wander around, limited by the edges of the map
*/

MoveSimController::MoveSimController (CreatureObject * newOwner, const Vector & centerPoint, float roamRadius) : 
CreatureController (newOwner),
m_centerPoint (centerPoint),
m_roamRadius  (roamRadius),
m_lastPoint   (),
m_lastUnstuckTimer (0.0f)
{
	m_lastPoint = newOwner->getPosition_w ();
}

//-----------------------------------------------------------------------

/**
*
* Reverse the object's direction and bias it toward the center 
*/

void MoveSimController::reverse (float towardsCenterFactor)
{
	const CreatureObject * const creature = safe_cast<CreatureObject *>(getOwner ());
	
	NOT_NULL (creature);

	const Vector & pos_w = getOwner ()->getPosition_w ();	
	Vector heading = getOwner ()->getObjectFrameK_w ();
	
	Vector vectorToCenter = m_centerPoint - pos_w;
	vectorToCenter.normalize ();
	heading = - heading;
	heading += vectorToCenter * towardsCenterFactor;
	heading.normalize ();
	const float theta = heading.theta ();

	getOwner ()->resetRotate_o2p ();
	getOwner ()->yaw_o (theta);
}

//-----------------------------------------------------------------------

/**
*
* Keep the object moving my moving its goal ahead of it.
* If we are outside the desired area, reverse
*/

float MoveSimController::realAlter(float time)
{
	const float retval = CreatureController::realAlter (time);

	const CreatureObject * const creature = safe_cast<CreatureObject *>(getOwner ());

	if (!creature || !creature->isAuthoritative ())
		return retval;

	const float runspeed = creature->getRunSpeed ();
	const float frameDistance = runspeed * time;
	
	const Vector & pos_w = getOwner ()->getPosition_w ();
	const float magSquaredCenter = pos_w.magnitudeBetweenSquared (m_centerPoint);
	
	Vector heading = getOwner ()->getObjectFrameK_w ();
	
	float halfMapWidth = 32.0f;
		
	TerrainObject * const terrainObject = TerrainObject::getInstance ();	
	if (terrainObject)
	{
		//- subtract a 512 meter invalid band around the edge of the map
		halfMapWidth = (terrainObject->getMapWidthInMeters () * 0.5f) - 512.0f;
	}
	
	Vector vectorToCenter = m_centerPoint - pos_w;	
	vectorToCenter.normalize ();
	const float heading_dot = vectorToCenter.dot (heading);
	
	//-- if we've moved less than 1% of our frame distance last frame, then consider ourselves
	//-- stuck, so turn around
	const float stuckThresholdSquared = sqr (frameDistance * 0.01f);	
	if (m_lastUnstuckTimer < 0.0f && m_lastPoint.magnitudeBetweenSquared (pos_w) < stuckThresholdSquared)
	{
		reverse (2.0f);
		//-- don't try to unstick for another 2 seconds
		m_lastUnstuckTimer = 2.0f;
	}
	
	//-- if we're out of range and moving away from the center, turn around
	else if (
		magSquaredCenter > sqr (m_roamRadius) ||
		pos_w.x < -halfMapWidth ||
		pos_w.x > halfMapWidth ||
		pos_w.z < -halfMapWidth ||
		pos_w.z > halfMapWidth)
	{		
		if (heading_dot < 0.0f)
			reverse (2.0f);
		else if (heading_dot < 0.5)
		{
			const float theta = vectorToCenter.theta ();

			getOwner ()->resetRotate_o2p ();
			getOwner ()->yaw_o (theta);
		}
	}
	
	m_lastPoint = pos_w;
	
	Transform t = getOwner ()->getTransform_o2w ();
	//-multiply frame distance by 110% fudge factor
	t.move_l (Vector::unitZ * frameDistance * 1.1f);
	setGoal (t, 0);
	
	m_lastUnstuckTimer -= time;

	return AlterResult::cms_alterNextFrame;
}

//-----------------------------------------------------------------------
