// ======================================================================
//
// AiShipBehaviorAttackFighter_Maneuver_Path.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorAttackFighter_Maneuver_Path.h"

#include "sharedMath/Range.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Transform.h"
#include "sharedRandom/Random.h"
#include "sharedFoundation/Misc.h"

#include <algorithm>
#include <deque>

// ======================================================================

namespace AiShipBehaviorAttackFighter_Maneuver_Path
{
	// You must pass in an initialized transform.

	void constructLookAtTransformation(Transform & objectTransform, Vector const & lookAtPosition)
	{
		Vector const upVector(objectTransform.getLocalFrameJ_p());
		Vector const objectPosition(objectTransform.getPosition_p());

		Vector k = lookAtPosition - objectPosition;
		if (!k.normalize())
			return;

		Vector i = upVector.cross(k);
		if (!i.normalize())
			return;

		k = i.cross(upVector);
		if (!k.normalize())
			return;

		objectTransform.setLocalFrameIJK_p(i, upVector, k);
	}
}

using namespace AiShipBehaviorAttackFighter_Maneuver_Path;

// ======================================================================

AiShipBehaviorAttackFighter::Maneuver::Path::Path() :
m_nodeList(new NodeList)
{

}
	
//-----------------------------------------------------------------------
	
AiShipBehaviorAttackFighter::Maneuver::Path::~Path()
{
	delete m_nodeList;
}

//-----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::Maneuver::Path::getNextNode(Index & currentIndex, Node & desiredPosition, Transform const & currentLocation, float const distanceTolerance) const
{
	bool hasNextNode = getNode(currentIndex, desiredPosition);
	
	if (hasNextNode)
	{
		if ((desiredPosition - currentLocation.getPosition_p()).magnitudeSquared() < sqr(distanceTolerance))
		{
			currentIndex++;
		}
	}

	return hasNextNode;
}


//-----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::Maneuver::Path::addNode(Node const & node)
{
	m_nodeList->push_back(node);
}

//-----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::Maneuver::Path::getNode(Index const & index, Node & node) const
{
	bool found = false;

	if (!m_nodeList->empty() && index < static_cast<int>(m_nodeList->size()))
	{
		node = (*m_nodeList)[static_cast<const size_t>(index)];
		found = true;
	}
	
	return found;
}

//-----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver::Path * AiShipBehaviorAttackFighter::Maneuver::Path::createPath()
{
	return new Path;
}

//-----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver::Path * AiShipBehaviorAttackFighter::Maneuver::Path::createPath(
	PathTypes const pathType,
	float const complexity,
	float const tension,
	float const objectRadius,
	float const turnRadius,
	Transform const & startTransform,
	Vector const & endPosition)
{
	float const pathLength = startTransform.getPosition_p().magnitudeBetween(endPosition);
	
	// Ensure the path length is > 1.
	DEBUG_WARNING(pathLength < 1.0f, ("Path length must be >= 1.0"));

	Transform transformToEnd(startTransform);
	constructLookAtTransformation(transformToEnd, endPosition);

	DEBUG_WARNING(objectRadius < 1.0f, ("objectRadius must be >= 1.0"));

	// Create a path length.
	float const totalPathLengthDesired = std::max(pathLength, 1.0f) + turnRadius + objectRadius;

	// Ensure totalPathLength is > 1.
	DEBUG_WARNING(totalPathLengthDesired < 1.0f, ("Total path length desired must be >= 1.0"));
	float const totalPathLength = std::max(totalPathLengthDesired, 1.0f);

	float const turnRadiusPlusObjectRadius = (turnRadius + objectRadius) * 0.5f;
	DEBUG_WARNING(turnRadiusPlusObjectRadius < 1.0f, ("turnRadiusPlusObjectRadius must be >= 1.0"));

	// Generate the inner and outer radii
	float const pathRadiusMultiplier = (1.0f + tension - complexity) * 0.5f;
	float const radius1 = totalPathLength * Random::randomReal(0.0f, pathRadiusMultiplier);
	float const radius2 = totalPathLength * Random::randomReal(0.0f, pathRadiusMultiplier);
	float const minTurns = std::min(radius1, radius2) / std::max(objectRadius, 1.0f);
	float const maxTurns = std::max(radius1, radius2) / std::max(objectRadius, 1.0f);
	float const turns = Random::randomReal(minTurns, maxTurns);

	// 0 = straight, 1 = parabolic1, 2 = parabolic2
	int const mode = Random::random(0, static_cast<int>(complexity));
	
	float const amp = Random::randomReal(0.0f, tension);
	float const tms = Random::randomReal(0.0f, (tension + complexity) * 0.5f); // K
	float const angle = Random::randomReal(PI_OVER_4, clamp(PI_OVER_4, PI_TIMES_2 * complexity, PI_TIMES_2));

	Path * const path = NON_NULL(new Path);
	Node node;

	unsigned const int ncvs = static_cast<unsigned int>(totalPathLength / std::max(turnRadiusPlusObjectRadius, 1.0f)); // Number of CVs

	float const ooHeight = 1.0f / totalPathLength;
	float const sqrHeight = sqr(totalPathLength);
	float const ooSqrHeight = 1.0f / sqrHeight;
	
	for (unsigned i = 0; i < ncvs; i++)
	{   
		node.z = static_cast<float>(i) * static_cast<float>(turnRadiusPlusObjectRadius);
		float const tInterpolation = node.z * ooHeight;
		
		float t0 = amp * sinf((tms * tInterpolation) + angle);
		float t1 = 0.0f;

		switch(mode) 
		{
			case 0: 
				t1 = node.z * ooHeight; 
				break;

			case 1: 
				t1 = sqr(node.z) * ooSqrHeight; 
				break;
			default:
			case 2: 
				t1 = sqrtf(node.z * ooHeight); 
				break;
		}

		float const t0t1Interpolant = t0 * (radius1 - (radius1 - radius2) * t1);
		float const offset = (node.z * turns * 2.0f * PI) * ooHeight;

		node.x = t0t1Interpolant * cosf(offset);
		node.y = t0t1Interpolant * sinf(offset);

		if (PT_loop == pathType)
		{
			Node const & loopNode = cubicInterpolate(transformToEnd.rotateTranslate_l2p(node), startTransform.getPosition_p(), tInterpolation);
			path->addNode(loopNode);
		}
		else
		{
			path->addNode(transformToEnd.rotateTranslate_l2p(node));
		}
	}


	return path;
}

//-----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::Maneuver::Path::deletePath(AiShipBehaviorAttackFighter::Maneuver::Path * path)
{
	delete path;
}

//-----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::Maneuver::Path::clearNodes()
{
	m_nodeList->clear();
}

//-----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::Maneuver::Path::popFrontNode()
{
	m_nodeList->pop_front();
}

//-----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::Maneuver::Path::isEmpty() const
{
	return m_nodeList->empty();
}

//-----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::Maneuver::Path::getFrontNode(Node & node) const
{
	bool success = !m_nodeList->empty();
	
	if (success)
	{
		node = m_nodeList->front();
	}

	return success;
}

//-----------------------------------------------------------------------

int AiShipBehaviorAttackFighter::Maneuver::Path::getLength() const
{
	return static_cast<int>(m_nodeList->size());
}

// ======================================================================
