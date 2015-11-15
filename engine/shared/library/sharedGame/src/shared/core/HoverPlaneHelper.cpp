//======================================================================
//
// HoverPlaneHelper.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/HoverPlaneHelper.h"

#include "sharedMath/Plane.h"
#include "sharedObject/Object.h"
#include "sharedObject/Appearance.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Footprint.h"
#include "sharedTerrain/TerrainObject.h"

#include <algorithm>

//----------------------------------------------------------------------

namespace
{
	/**
	* @param points an array of 10 Vectors, 5 for the main body and 5 for the lookAhead
	*/

	bool transformBoxPoints (const BoxExtent & box, const Transform & transform, Vector * points, const Vector & lookAhead)
	{
		const Vector & minV = box.getMin ();
		const Vector & maxV = box.getMax ();

		if (maxV.x == minV.x || maxV.z == minV.z)
			return false;
		
		//-- centerpoint
		points [0].x = (minV.x + maxV.x) * 0.5f;
		points [0].z = (minV.z + maxV.z) * 0.5f;
		
		//-- front right
		points [1].x = maxV.x;
		points [1].z = maxV.z;
		
		//-- front left
		points [2].x = minV.x;
		points [2].z = maxV.z;

		//-- back left
		points [3].x = minV.x;
		points [3].z = minV.z;

		//-- back right
		points [4].x = maxV.x;
		points [4].z = minV.z;
		
		for (int i = 0; i < 5; ++i)
		{
			Vector & v = points [i];
			v.y = minV.y;
			v = transform.rotateTranslate_l2p (v);

			points [i + 5] = v + lookAhead;
		}
		
		return true;
	}
	
	inline void computeRollPitch (const Vector & normal, float & roll, float & pitch)
	{		
		Vector normalX = normal;
		normalX.z = 0.0f;
		if (normalX.normalize ())
			roll = normalX.phi () + PI_OVER_2;
		else
			roll = 0.0f;
		
		if (normalX.x > 0.0f)
			roll = -roll;
		
		Vector normalZ = normal;
		normalZ.x = 0.0f;
		if (normalZ.normalize ())
			pitch = normalZ.phi () + PI_OVER_2;
		else
			pitch = 0.0f;
		
		if (normalZ.z < 0.0f)
			pitch = -pitch;
		
	}	
}

//----------------------------------------------------------------------

bool HoverPlaneHelper::findMinimumHoverPlane (const Object & obj, float & roll, float & pitch, const Vector & lookAhead)
{
	Object const * motor = obj.getParent ();

	if(motor)
	{
		CollisionProperty const * collision = motor->getCollisionProperty();

		if(collision)
		{
			Footprint const * foot = collision->getFootprint();

			if(foot && foot->isOnSolidFloor())
			{
				Vector const & groundNormal_w = foot->getGroundNormal_w();

				const Transform & transform_o2w = obj.getTransform_o2w ();

				computeRollPitch (transform_o2w.rotate_p2l (groundNormal_w), roll, pitch);

				return true;
			}
		}
	}

	// ----------

	const Appearance * const app = obj.getAppearance ();
	if (!app)
		return false;

	const BoxExtent * const box = dynamic_cast<const BoxExtent *>(app->getExtent ());
	if (!box)
		return false;
	
	const Transform & transform_o2w = obj.getTransform_o2w ();

	Vector points [10];
	if (!transformBoxPoints (*box, transform_o2w, points, lookAhead))
		return false;

	const TerrainObject * const terrainObject = TerrainObject::getInstance ();

	if (!terrainObject)
		return false;

	Vector normals [10];
	Vector avgNormal;

	Vector avgLookAhead;
	int lookAheadContribCount = 0;

	Sphere s = box->getSphere ();
	s.setCenter (transform_o2w.getPosition_p ());
	float avgDistanceAboveTerrain = 0.0f;
	float minDistanceAboveTerrain = 10000.0f;

	//- find terrain heights for points
	{
		const Vector oldCenterPoint = points [0];

		float y = 0.0f;
		for (int i = 0; i < 10; ++i)
		{
			if (terrainObject->getHeight (points [i], y, normals [i]))
			{
				float waterHeight = 0.0f;
				const bool isWater = terrainObject->getWaterHeight (points[i], waterHeight);
				if(isWater && waterHeight > y)
				{
					normals[i] = Vector::unitY;
					y = waterHeight;
				}

				if (i >= 5)
				{
					//-- lookahead point is not in the sphere, let it contribute to the lookahead normal
					if (!s.contains (points [i]))
					{
						points [i].y = y;
						avgLookAhead += points [i] - oldCenterPoint;
						++lookAheadContribCount;
					}
					points [i].y = y;
				}
				else
				{
					const float distanceAbove = points [i].y - y;
					avgDistanceAboveTerrain += distanceAbove;
					minDistanceAboveTerrain = std::max (0.0f, std::min (minDistanceAboveTerrain, distanceAbove));
					points [i].y = y;
					avgNormal += normals [i];
				}
			}
			else
				return false;
		}
	}

	avgNormal /= 5.0f;
	avgDistanceAboveTerrain /= 5.0f;

	computeRollPitch (transform_o2w.rotate_p2l (avgNormal), roll, pitch);
	
	//-- decrease the roll & pitch based on height over terrain
	//--

	const float extentRadius = s.getRadius ();

	avgDistanceAboveTerrain -= extentRadius;

	if (avgDistanceAboveTerrain > 0.0f)
	{
		if (extentRadius > 0.0f)
		{
			const float relativeDistance = 1.0f + (avgDistanceAboveTerrain / (extentRadius  * 0.5f));
			roll /= relativeDistance;
			pitch /= relativeDistance;
		}
	}

	if (lookAheadContribCount)
	{
		avgLookAhead  /= static_cast<float>(lookAheadContribCount);

		Vector vectorToLookAhead = transform_o2w.rotate_p2l (avgLookAhead);

		float pitchLookAhead = 0.0f;

		if (vectorToLookAhead.normalize ())
			pitchLookAhead = vectorToLookAhead.phi ();
		else
			pitchLookAhead = 0.0f;

		if (pitchLookAhead < pitch)
		{
			pitch = pitchLookAhead;
		}
	}

	return true;
}

//----------------------------------------------------------------------

bool HoverPlaneHelper::findMinimumHoverHeight (const Object & obj, float & height, const Vector & lookAhead)
{
	Object const * motor = obj.getParent ();

	const float obj_y_w = obj.getPosition_w ().y;

	float minDeltaY = 10000.0f;

	if(motor)
	{
		CollisionProperty const * collision = motor->getCollisionProperty();

		if(collision)
		{
			Footprint const * foot = collision->getFootprint();

			if(foot && foot->isOnSolidFloor())
			{
				float groundHeight = -REAL_MAX;

				if(foot->getGroundHeight(groundHeight))
				{
					minDeltaY = obj_y_w - groundHeight;
				}
			}
		}
	}

	// ----------
	
	const Appearance * const app = obj.getAppearance ();
	if (!app)
		return false;

	const BoxExtent * const box = dynamic_cast<const BoxExtent *>(app->getExtent ());
	if (!box)
		return false;
	
	const Transform & transform_o2w = obj.getTransform_o2w ();

	Vector points [10];
	if (!transformBoxPoints (*box, transform_o2w, points, lookAhead))
		return false;

	const TerrainObject * const terrainObject = TerrainObject::getInstance ();

	if (!terrainObject)
		return false;

	float minDeltaYLookAhead = 10000.0f;

	bool foundLookAhead = false;

	//- find terrain heights for points
	{
		bool foundStrict    = false;

		for (int i = 0; i < 10; ++i)
		{
			const Vector & p = points [i];

			float pointHeight = 0.0f;

			if (terrainObject->getHeight (p, pointHeight))
			{
				float deltaPointHeight = p.y - pointHeight;

				//-- lookahead box, non-strict
				if (i >= 5)
				{
					foundLookAhead = true;
					minDeltaYLookAhead = std::min (minDeltaYLookAhead, deltaPointHeight);
				}
				else
				{
					foundStrict = true;

					float waterHeight = 0.0f;
					if (terrainObject->getWaterHeight (p, waterHeight))
					{
						pointHeight = std::max (pointHeight, waterHeight);
						deltaPointHeight = p.y - pointHeight;
					}
					
					minDeltaY = std::min (minDeltaY, deltaPointHeight);
				}
			}
		}

		if (!foundStrict)
			return false;
	}

	height = obj_y_w - minDeltaY;

	if (foundLookAhead)
	{
		float heightLookAhead = obj_y_w - minDeltaYLookAhead;
		if (heightLookAhead > height)
			height = (height + heightLookAhead) * 0.5f;
	}

	return true;
}

//----------------------------------------------------------------------
