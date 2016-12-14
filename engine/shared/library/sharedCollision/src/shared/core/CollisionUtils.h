// ======================================================================
//
// CollisionUtils.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_CollisionUtils_H
#define INCLUDED_CollisionUtils_H

#include "sharedMath/Vector.h"
#include "sharedMath/Transform.h"

class Vector;
class ABox;
class Vector;
class Triangle3d;
class Line3d;
class Segment3d;
class Ribbon3d;
class Circle;
class Sphere;
class Capsule;
class Transform;
class CellProperty;
class Object;
class CollisionProperty;
class MultiShape;
class BaseExtent;

typedef std::vector<Vector> VectorVector;
typedef std::vector<Vector> VertexList;
typedef std::vector<BaseExtent const *> ExtentVec;

// ----------------------------------------------------------------------

#ifdef _MSC_VER

inline bool isNan ( float const & x )
{
	if(_isnan(x)) return true;

	if(!_finite(x)) return true;

	return false;
}

inline bool isNan ( Vector const & v )
{
	if(isNan(v.x)) return true;
	if(isNan(v.y)) return true;
	if(isNan(v.z)) return true;

	return false;
}

inline bool isNan ( Transform const & t )
{
	return t.isNaN();
}

#define NAN_CHECK(x) DEBUG_FATAL(isNan(x),("Floating-point value is not a number"))

#else

#define NAN_CHECK(x) 

#endif

// ----------------------------------------------------------------------

struct Matrix3
{
	static Matrix3 fromRows( Vector const & row0, Vector const & row1, Vector const & row2);
	static Matrix3 fromColumns ( Vector const & col0, Vector const & col1, Vector const & col2 );

	// ----------

	Vector row0 ( void ) const;
	Vector row1 ( void ) const;
	Vector row2 ( void ) const;

	Vector col0 ( void ) const;
	Vector col1 ( void ) const;
	Vector col2 ( void ) const;

	bool invert ( Matrix3 & out ) const;

	Matrix3 multiply ( Matrix3 const & A ) const;

	Vector operator * ( Vector const & V ) const;

	// ----------

	float m00, m01, m02;
	float m10, m11, m12;
	float m20, m21, m22;
};

// ----------------------------------------------------------------------

namespace Collision
{
	extern const real gs_equalityEpsilon;
	extern const real gs_planarEpsilon;
	extern const real gs_spatialEpsilon;
	extern const real gs_twistEpsilon;
};

// ----------------------------------------------------------------------
// 2d operations

namespace Collision2d
{

float   ComponentAlong          ( Vector const & V, Vector const & N );

}; // namespace Collision2d

// ----------------------------------------------------------------------
// 3d operations

namespace Collision3d
{

// ----------------------------------------------------------------------
// Side/twist tests

int     TestLineLineTwist       ( Line3d const & A, Line3d const & B );

int     TestLineSegTwist        ( Line3d const & A, Segment3d const & B );
int     TestSegLineTwist        ( Segment3d const & A, Line3d const & B );

int     TestSegSegTwist         ( Segment3d const & A, Segment3d const & B);

// ----------------------------------------------------------------------
// Miscellaneous

// The idea of an exit test for a segment exiting a triangle in 3d is a 
// little wonky. This does effectively the same thing as projecting the 
// segment down onto the triangle and figuring out which edge the projected
// segment exits.

int     TestExitTri             ( Ribbon3d const & R, Triangle3d const & tri );
int     TestEntranceTri         ( Ribbon3d const & R, Triangle3d const & tri );

Vector  ClipPath                ( Vector const & begin, 
                                  Vector const & delta, 
                                  Triangle3d const & tri, 
                                  int whichEdge, 
                                  Vector const & up );

float   ComponentAlong          ( Vector const & V, Vector const & N );
Vector  SlideAlong              ( Vector const & V, Vector const & N, float t );

Vector  SingleSlide             ( Vector V, Vector A );
Vector  DoubleSlide             ( Vector V, Vector A, Vector B );

bool    IsEdgeConcave           ( Triangle3d const & triA, int edgeA,
                                  Triangle3d const & triB );

Vector  MoveIntoTriangle        ( Vector const & point, 
                                  Triangle3d const & tri, 
                                  real desiredDist );

void    MovePolyOnto            ( VertexList & verts, Vector V );

bool    FindTangentPoint        ( Vector A, Circle C, bool right, Vector & outTangent );
bool    FindAvoidancePoint      ( Vector A, Vector B, Circle C, Vector & outPoint );

bool    FindAvoidanceThetas     ( Vector A, MultiShape const & shape, float & outThetaA, float & outDistA, float & outThetaB, float & outDistB );

bool    CalcAvoidancePoint      ( Object const * mob, Vector const & delta, Object const * obstacle, Vector & out );

bool    CalcAvoidancePoints3d   ( Sphere const & mob, Vector const & delta, Sphere const & obstacle, Vector & avoidA, Vector & avoidB );

// ----------------------------------------------------------------------

};  // namespace Collision3d

// ----------------------------------------------------------------------

namespace CollisionUtils
{

Vector    transformToWorld   ( CellProperty const * cellA, Vector const & point_A );
Sphere    transformToWorld   ( CellProperty const * cellA, Sphere const & sphere_A );
Capsule   transformToWorld   ( CellProperty const * cellA, Capsule const & capsule_A );
Transform transformToWorld   ( CellProperty const * cellA, Transform const & transform_A );

Vector    transformFromWorld ( Vector const & point_w,        CellProperty const * cellB );
Sphere    transformFromWorld ( Sphere const & sphere_w,       CellProperty const * cellB );
Capsule   transformFromWorld ( Capsule const & capsule_w,     CellProperty const * cellB );
Transform transformFromWorld ( Transform const & transform_w, CellProperty const * cellB );

// ----------

Vector    transformToCell    ( CellProperty const * cellA, Vector const & point,        CellProperty const * cellB );
Transform transformToCell    ( CellProperty const * cellA, Transform const & transform, CellProperty const * cellB );
Sphere    transformToCell    ( CellProperty const * cellA, Sphere const & sphere,       CellProperty const * cellB );
Capsule   transformToCell    ( CellProperty const * cellA, Capsule const & capsule,     CellProperty const * cellB );
MultiShape transformToCell   ( CellProperty const * cellA, MultiShape const & shape,    CellProperty const * cellB );

Vector    rotateToCell       ( CellProperty const * cellA, Vector const & dir,          CellProperty const * cellB );

// ----------

bool      testPortalVis      ( CellProperty const * cellA, Vector const & pointA, CellProperty const * cellB, Vector const & pointB );

// ----------

bool      epsilonEqual       ( Sphere const & A, Sphere const & B, float epsilon );


};



// ----------------------------------------------------------------------

#endif
