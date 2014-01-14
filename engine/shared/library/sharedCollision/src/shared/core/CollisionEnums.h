// ======================================================================
//
// CollisionEnums.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CollisionEnums_H
#define INCLUDED_CollisionEnums_H

// ======================================================================

enum CanMoveResult
{
	CMR_MoveOK,
	CMR_HitFloorEdge,
	CMR_HitObstacle,
	CMR_MissedGoal,
	CMR_Error,

	CMR_Invalid
};

enum EdgeIntersectResult
{
	EIR_MissedEdge,
	EIR_HitEdge,
	EIR_PassesEdge,
	EIR_GrazesEdge,

	EIR_Invalid
};

enum ContainmentResult
{
	CR_Outside,         // Test point/shape is totally outside the boundary
	CR_Inside,          // Test point/shape is totally contained by the boundary
	CR_Boundary,        // Test point/shape is entirely within epsilon of the boundary of the containing shape, 
	CR_Overlap,         // Test shape is on both sides of the boundary
	CR_TouchingInside,  // Test shape has points inside the boundary and within epsilon of the boundary
	CR_TouchingOutside, // Test shape has points outside the boundary and within epsilon of the boundary

	CR_Invalid,
};

namespace Containment
{
	// True if the object has no points outside of the boundary

	inline bool isContainment( ContainmentResult C )
	{
		return (C == CR_Inside) || (C == CR_TouchingInside) || (C == CR_Boundary);
	}

	// True if the object has no points inside the boundary

	inline bool isNonContainment( ContainmentResult C )
	{
		return (C == CR_Outside) || (C == CR_TouchingOutside) || (C == CR_Boundary);
	}

	// True if the object has points on the boundary

	inline bool isContact( ContainmentResult C ) 
	{
		return (C == CR_Boundary) || (C == CR_Overlap) || (C == CR_TouchingInside) || (C == CR_TouchingOutside);
	}

	ContainmentResult    ComposeAxisTests ( ContainmentResult A, ContainmentResult B );
	ContainmentResult    ComposeAxisTests ( ContainmentResult A, ContainmentResult B, ContainmentResult C );
}

// ----------

enum ResolutionResult
{
	RR_NoCollision,     // The objects don't collide
	RR_DontKnowHow,     // We don't know how to resolve collisions between those types of objects
	RR_Past,            // The collision happened in the past, we can't resolve it.
	RR_Future,          // The collision happens too far in the future, we can't resolve it
	RR_Resolved,        // The objects collided, and the collision has been resolved
	RR_Failed,          // The objects collided, but we were unable to resolve the collision

	RR_Invalid
};

// ----------

enum FloorEdgeType
{
	FET_Uncrossable = 0,
	FET_Crossable = 1,
	FET_WallBase = 2,
	FET_WallTop = 3,

	FET_Invalid,
};

// ----------

enum HitResult
{
	HR_Miss,                // Primitive does not hit the test shape
	HR_Hit,                 // Primitive hits the test shape

	HR_HitFront,            // Primitive hits the front side of the test shape
	HR_HitBack,             // Primtiive hits the back side of the test shape
	HR_TouchFront,          // Segment's begin point is in front of the shape and its end point is on the shape
	HR_TouchBack,           // Segment's begin point is in front of the shape and its end point is on the shape
	HR_LeaveFront,          // Segment/ray begins on the shape and goes in front of the shape
	HR_LeaveBack,           // Segment/ray begins on the shape and goes behind the shape
	HR_Coplanar,            // Primitive is coplanar with the test shape

	HR_Invalid,
};

// ----------

enum CollinearResult
{
	CLR_LeftSide,           // (2D) Point lies on the left (counterclockwise) side of AB
	CLR_RightSide,          // (2D) Point lies on the right (clockwise) side of AB

	CLR_CoincidentA,        // Point is coincident with A
	CLR_CoincidentB,        // Point is coincident with B

	CLR_OverlapLeft,		// Point is collinear with AB, forms line V-A-B
	CLR_OverlapCenter,      // Point is collinear with AB, forms line A-V-B
	CLR_OverlapRight,       // Point is collinear with AB, forms line A-B-V

	// ----------

	CLR_Disjoint,           // (3D) Point does not lie on the line
	CLR_Overlap,            // OverlapLeft || OverlapRight || OverlapCenter || CoincidentA || CoincidentB
};

// ----------

/*
enum OverlapResult
{
	OR_Identical,
	OR_AContainsB,
	OR_BContainsA,
	OR_Overlap,
	OR_Disjoint,

	OR_Invalid
};

*/

// ----------
// The values for these two enums are taken from SharedObjectTemplate.h
// If they change, this file needs to change too

// Why did I duplicate them? Well, since their definition is nested inside
// SharedObjectTemplate, there's no way to get to them without #including
// SharedObjectTemplate.h, and including that file creates a bunch of dependencies
// that tend to break builds since there are two different files named
// SharedObjectTemplate.h, one in sharedTemplate and one in sharedGame.

enum InteractionType
{
	IT_See = 0x0001,
	IT_Target = 0x0002,
	IT_CombatRanged = 0x0004,
	IT_CombatMelee = 0x0008,
	IT_Manipulate = 0x0010,
	IT_Talk = 0x0020,		//     // Sound doesn't travel in straight lines, but...
	IT_RadialDamage = 0x0040,		//     // Explosions cause radial damage
	IT_Camera = 0x0080,		//     // This is a bit different from the other actions - if an object blocks CAT_Camera it means the object blocks the camera's view of the target.

	IT_All = IT_See | IT_Target | IT_CombatRanged | IT_CombatMelee | IT_Manipulate | IT_Talk | IT_RadialDamage | IT_Camera,

	IT_Any = 0xFFFFFFFF,
};

enum MaterialType
{
	MT_Solid = 0x0001,		//     // Generic solid 'stuff'
	MT_Meat = 0x0002,		//     // Living things are made of meat
	MT_Metal = 0x0004,		//     // Droids and machines are made of metal
	MT_Energy = 0x0008,		//     // Ghosts are made of energy
	MT_Brush = 0x0010,		//     // Bushes are made of brush
	MT_Door = 0x0020,		//     // Doors are made of door
	MT_Monster = 0x0040,		//     // Monsters are made of monster
	MT_Player = 0x0080,		//     // Players are made of player
	MT_NPC = 0x0100,		//     // NPCs are made of NPC

	MT_Any = 0xFFFFFFFF,
};

// ----------

// Update the database name table in CollisionWorld if this changes

enum SpatialDatabaseType
{
	SDT_Floors,
	SDT_Statics,
	SDT_Flora,

	SDT_Invalid,
};

// ----------

enum ExtentType
{
	ET_Sphere,
	ET_Cylinder,
	ET_Box,
	ET_Detail,
	ET_Component,
	ET_Mesh,
	ET_Terrain,
	ET_OrientedBox,
	ET_OrientedCylinder,
    ET_Simple,
    ET_Null,                // Dummy extent - doesn't collide with anything
	ET_Floor,

	ET_ExtentTypeCount,
};

// ----------

enum PathWalkResult
{
	PWR_MissedStartTri,     // The walk failed because the path didn't enter the mesh at the starting triangle
	PWR_CantEnter,          // The walk failed because the path couldn't enter the starting triangle
	PWR_DoesntEnter,        // The walk didn't enter the floor
	PWR_HitBeforeEnter,     // The footprint circle hit an edge of the floor before the center entered the floor
	PWR_WalkOk,             // The walk succeeded
	PWR_WalkFailed,         // The walk failed (probably an error)
	PWR_HitEdge,            // The walk hit an edge of the mesh
	PWR_HitPortalEdge,      // The walk hit a portal-adjacent edge of the mesh
	PWR_HitPast,            // The walk hit an edge some time in the past
	PWR_InContact,          // The walk was unable to move because the point/circle was already in contact with something
	PWR_ExitedMesh,         // The walk exited the mesh
	PWR_CenterHitEdge,      // The center of the circle hit an edge
	PWR_CenterInContact,    // The center of the circle is in contact with an edge
	PWR_StartLocInvalid,    // bad starting point for the walk (embedded in a wall or something)

	PWR_Invalid,
};

// ----------

enum QueryInteractionResult
{
	QIR_None,	
	QIR_HitTerrain,
	QIR_HitObjectExtent,
	QIR_HitObjectFloor,
	QIR_HitCellExtent,
	QIR_HitCellFloor,
	QIR_MissedTarget,

	QIR_Invalid,
};

// ======================================================================

#endif //#ifndef INCLUDED_CollisionEnums_H
