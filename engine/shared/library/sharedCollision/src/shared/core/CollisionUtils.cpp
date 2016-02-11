// ======================================================================
//
// CollisionUtils.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CollisionUtils.h"

#include "sharedCollision/Overlap2d.h"
#include "sharedCollision/Overlap3d.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Intersect2d.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/Containment2d.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/SimpleExtent.h"
#include "sharedCollision/ComponentExtent.h"
#include "sharedCollision/DetailExtent.h"
#include "sharedCollision/CollisionEnums.h"
#include "sharedCollision/CollisionDetect.h"

#include "sharedObject/CellProperty.h"

#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/OrientedBox.h"
#include "sharedMath/Triangle3d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Ribbon3d.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Circle.h"
#include "sharedMath/RangeLoop.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/MultiShape.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Ray3d.h"

#include <vector>
#include <algorithm>

using namespace Intersect3d;
using namespace Overlap3d;

// ----------------------------------------------------------------------

const real  Collision::gs_planarEpsilon     = 0.000000001f;
const real  Collision::gs_equalityEpsilon   = 0.000000001f;
const real  Collision::gs_spatialEpsilon    = 0.000000001f;
const real  Collision::gs_twistEpsilon      = 0.000000001f;

// ----------------------------------------------------------------------

Matrix3 Matrix3::fromRows ( Vector const & row0, Vector const & row1, Vector const & row2 )
{
	Matrix3 temp;

	temp.m00 = row0.x; temp.m01 = row0.y; temp.m02 = row0.z;
	temp.m10 = row1.x; temp.m11 = row1.y; temp.m12 = row1.z;
	temp.m20 = row2.x; temp.m21 = row2.y; temp.m22 = row2.z;

	return temp;
}

// ----------

Matrix3 Matrix3::fromColumns ( Vector const & col0, Vector const & col1, Vector const & col2 )
{
	Matrix3 temp;

	temp.m00 = col0.x; temp.m01 = col1.x; temp.m02 = col2.x;
	temp.m10 = col0.y; temp.m11 = col1.y; temp.m12 = col2.y;
	temp.m20 = col0.z; temp.m21 = col1.z; temp.m22 = col2.z;

	return temp;
}

// ----------

Vector Matrix3::row0 ( void ) const { return Vector(m00,m01,m02); }
Vector Matrix3::row1 ( void ) const { return Vector(m10,m11,m12); }
Vector Matrix3::row2 ( void ) const { return Vector(m20,m21,m22); }

Vector Matrix3::col0 ( void ) const { return Vector(m00,m10,m20); }
Vector Matrix3::col1 ( void ) const { return Vector(m01,m11,m21); }
Vector Matrix3::col2 ( void ) const { return Vector(m02,m12,m22); }

// ----------

bool Matrix3::invert ( Matrix3 & out ) const
{
	Matrix3 t;

	t.m00 =   ( m11*m22 - m12*m21 );
	t.m01 = - ( m10*m22 - m12*m20 );
	t.m02 =   ( m10*m21 - m11*m20 );

	t.m10 = - ( m01*m22 - m02*m21 );
	t.m11 =   ( m00*m22 - m02*m20 );
	t.m12 = - ( m00*m21 - m01*m20 );

	t.m20 =   ( m01*m12 - m02*m11 );
	t.m21 = - ( m00*m12 - m02*m10 );
	t.m22 =   ( m00*m11 - m01*m10 );

	float det = m00 * t.m00 + m01 * t.m01 + m02 * t.m02;

	if ( fabs(det) <= 0.00000001f ) return false;

	float inv = 1.0f/det;

	out.m00 = t.m00 * inv; 
	out.m01 = t.m10 * inv; 
	out.m02 = t.m20 * inv;

	out.m10 = t.m01 * inv; 
	out.m11 = t.m11 * inv; 
	out.m12 = t.m21 * inv;

	out.m20 = t.m02 * inv; 
	out.m21 = t.m12 * inv; 
	out.m22 = t.m22 * inv;

	return true;
}

// ----------

Matrix3 Matrix3::multiply ( Matrix3 const & A ) const
{
	Matrix3 T;

	T.m00 = m00 * A.m00 + m01 * A.m10 + m02 * A.m20;
	T.m01 = m00 * A.m01 + m01 * A.m11 + m02 * A.m21;
	T.m02 = m00 * A.m02 + m01 * A.m12 + m02 * A.m22;

	T.m10 = m10 * A.m00 + m11 * A.m10 + m12 * A.m20;
	T.m11 = m10 * A.m01 + m11 * A.m11 + m12 * A.m21;
	T.m12 = m10 * A.m02 + m11 * A.m12 + m12 * A.m22;

	T.m20 = m20 * A.m00 + m21 * A.m10 + m22 * A.m20;
	T.m21 = m20 * A.m01 + m21 * A.m11 + m22 * A.m21;
	T.m22 = m20 * A.m02 + m21 * A.m12 + m22 * A.m22;

	return T;
}

// ----------

Vector Matrix3::operator * ( Vector const & V ) const
{
	float x = m00 * V.x + m01 * V.y + m02 * V.z;
	float y = m10 * V.x + m11 * V.y + m12 * V.z;
	float z = m20 * V.x + m21 * V.y + m22 * V.z;

	return Vector(x,y,z);
}

// ----------------------------------------------------------------------

namespace Collision2d
{

// ----------

float ComponentAlong( Vector const & V2, Vector const & N2 )
{
	Vector V(V2.x,0.0f,V2.z);
	Vector N(N2.x,0.0f,N2.z);

	float nmag2 = N.magnitudeSquared();

	if(nmag2 > 0.0f)
	{
		return V.dot(N) / nmag2;
	}
	else
	{
		return 0.0f;
	}
}

// ----------

}	// namespace Collision2d

// ----------------------------------------------------------------------

namespace Collision3d
{

// ----------------------------------------------------------------------
// Return the magnitude of the component of V in the direction of N

// if t = componentAlong(V,N),

// V - (N * t) is perpendicular to N.

float ComponentAlong( Vector const & V, Vector const & N )
{
	float nmag2 = N.magnitudeSquared();

	if(nmag2 > 0.0f)
	{
		return V.dot(N) / nmag2;
	}
	else
	{
		return 0.0f;
	}
}

// ----------------------------------------------------------------------
// Given a movement velocity, a collision normal, and a 

Vector SlideAlong ( Vector const & V, Vector const & N, float t )
{
	float nmag2 = N.magnitudeSquared();

	if(nmag2 > 0.0f)
	{
		real d = V.dot(N);

		if(d < 0.0f)
		{
			real mag = (-d * (1-t)) / nmag2;
	
			return V + N*mag;
		}
		else
		{
			return V;
		}
	}
	else
	{
		return V;
	}
}

// ----------------------------------------------------------------------

Vector SingleSlide ( Vector V, Vector N )
{
	if(V.dot(N) > 0.0f) return V;

	// build normal-binormal-tangent vectors

	Vector up(0.0f,1.0f,0.0f);

	static float angleEpsilon = 0.99996192306417128873735516482698f; // cos(0.5 degrees)

	float cosTheta = N.y;

	if(cosTheta > angleEpsilon)
	{
		// Surface is almost exactly horizontal

		V.y = 0.0f;

		return V;
	}

	Vector B = N.cross(up);
	Vector T = B.cross(N);

	IGNORE_RETURN(N.normalize());
	IGNORE_RETURN(B.normalize());
	IGNORE_RETURN(T.normalize());

	float x = V.dot(B);
	float y = V.dot(T);
	float z = V.dot(N);

	if(z < 0.0f) z = 0.0f;

	if(N.y < 0.0f)
	{
		// Surface is facing down. 
		
		if(N.y < -0.86602540378443864676372317075294f) // -cos(30 degrees)
		{
			// Surface is tilted down a lot. Since we don't know what the terrain is doing
			// at this point, we can't allow any movement because we might end up putting
			// the character under the terrain.

			return Vector::zero;
		}
	}
	else
	{
		// Surface is facing up. Do not allow upward Y movement on slopes or players will
		// be able to climb up rocks and such.

		// Very shallow slopes are OK, otherwise players can get stuck on top of extents.
		// Very steep slops are OK, otherwise players can slide down under the terrain

		static float angleTolerance1 = 0.9659258262890682867497431997289f; // cos(15 degrees)
		static float angleTolerance2 = 0.08715574274765817355806427083747f; // cos(85 degrees)

		if((cosTheta < angleTolerance1) && (cosTheta > angleTolerance2))
		{
			if(y > 0.0f) y = 0.0f;
		}
	}

	Vector out = (B * x) + (T * y) + (N * z);

	return out;
}

Vector DoubleSlide ( Vector V, Vector A, Vector B )
{
	Vector C = A.cross(B);

	IGNORE_RETURN(A.normalize());
	IGNORE_RETURN(B.normalize());
	IGNORE_RETURN(C.normalize());

	float a = A.dot(V);
	float b = B.dot(V);

	Vector V2;

	if((a > 0.0f) && (b > 0.0f))
	{
		// Velocity is moving away from both contacts
		V2 = V;
	}
	else
	{
		float c = 0.00174532836589830883577820272085f; // 1/10 of 1 degree

		if( A.dot(B) < c)
		{
			// The corner is acute, we can't resolve this as a slide along one wall

			V2 = Vector::zero;
		}
		else
		{
			// The corner is obtuse, this could be resolved as either no motion
			// or a slide along one of the walls

			Vector A2 = C.cross(A);
			Vector B2 = B.cross(C);

			IGNORE_RETURN(A2.normalize());
			IGNORE_RETURN(B2.normalize());

			float a2 = A2.dot(V);
			float b2 = B2.dot(V);

			if((a2 < 0.0f) && (b2 < 0.0f))
			{
				return Vector::zero;
			}
			else if (a2 > 0.0f)
			{
				V2 = V - A * a;
			}
			else if (b2 > 0.0f)
			{
				V2 = V - B * b;
			}
			else
			{
				// should never get here

				V2 = V;
			}
		}
	}

	return V2;
}

// ----------------------------------------------------------------------
// Returns true if the lines twist in a clockwise (left hand)
// fashion

// This little function has given me no end of grief.

// Logically, twist(A,B) == twist(B,A) == -twist(-A,B), but due to
// numerical inaccuracies this isn't always the case. 

// In order to enforce these identities, we have to impose an arbitrary
// ordering on the arguments so that the return values are always consistent,
// even if their values are iffy due to precision issues.

inline bool vectorLess ( Vector const & A, Vector const & B )
{
	if(A.x != B.x) return (A.x < B.x);

	if(A.y != B.y) return (A.y < B.y);

	if(A.z != B.z) return (A.z < B.z);

	return false;
}


// This is the original version of the test. It's short, simple, and wrong.
// It doesn't handle degenerate cases and can return inconsistent results if 
// the lines are swapped or the line directions are negated.

/*
bool TestLineLineTwist ( Line3d const & A, Line3d const & B )
{
	Vector D = B.getPoint() - A.getPoint();

	return D.cross(A.getNormal()).dot(B.getNormal()) < 0.0f;
}
*/

#ifdef _MSC_VER

inline void faster_normalize( Vector & V )
{
	float t = (V.x * V.x) + (V.y * V.y) + (V.z * V.z);

	__asm fld t;
	__asm fsqrt;
	__asm fstp t;

	t = 1.0f / t;

	V.x *= t;
	V.y *= t;
	V.z *= t;
}

int     TestLineTwist4	( Line3d const & A, Line3d const & B )
{
	float epsilon = 0.00005f;

	Vector AN = A.getNormal();
	Vector BN = B.getNormal();

	faster_normalize(AN);
	faster_normalize(BN);

	Vector N = AN.cross(BN);

	Vector D = B.getPoint() - A.getPoint();

	real V = D.dot(N);

	if      ( V >  epsilon )  return 1;
	else if ( V < -epsilon )  return -1;
	else                      return 0;
}

#else

int     TestLineTwist4	( Line3d const & A, Line3d const & B )
{
	float epsilon = 0.00005f;

	Vector AN = A.getNormal();
	Vector BN = B.getNormal();

	IGNORE_RETURN(AN.normalize());
	IGNORE_RETURN(BN.normalize());

	Vector N = AN.cross(BN);

	Vector D = B.getPoint() - A.getPoint();

	real V = D.dot(N);

	if      ( V >  epsilon )  return 1;
	else if ( V < -epsilon )  return -1;
	else                      return 0;
}

#endif

int TestLineLineTwist ( Line3d const & inA, Line3d const & inB )
{
	Line3d A = inA;
	Line3d B = inB;

	bool flipped = false;

	if(vectorLess(B.getPoint(),A.getPoint()))
	{
		std::swap(A,B);
	}

	if(vectorLess(A.getNormal(),Vector::zero))
	{
		A.flip();

		flipped = !flipped;
	}

	if(vectorLess(B.getNormal(),Vector::zero))
	{
		B.flip();

		flipped = !flipped;
	}

	int result = TestLineTwist4(A,B);

	return flipped ? -result : result;
}


// ----------

int TestSegLineTwist ( Segment3d const & A, Line3d const & B )
{
	if(vectorLess(A.getEnd(),A.getBegin()))
	{
		return -TestLineLineTwist(A.getReverseLine(),B);
	}
	else
	{
		return TestLineLineTwist(A.getLine(),B);
	}
}

int TestLineSegTwist ( Line3d const & A, Segment3d const & B )
{
	if(vectorLess(B.getEnd(),B.getBegin()))
	{
		return -TestLineLineTwist(A,B.getReverseLine());
	}
	else
	{
		return TestLineLineTwist(A,B.getLine());
	}
}

int TestSegSegTwist ( Segment3d const & A, Segment3d const & B )
{
	if(vectorLess(A.getEnd(),A.getBegin()))
	{
		return -TestLineSegTwist(A.getReverseLine(),B);
	}
	else
	{
		return TestLineSegTwist(A.getLine(),B);
	}
}

//----------------------------------------------------------------------------

CollinearResult testCollinear ( Vector const & V, Vector const & A, Vector const & B )
{
    Vector dA = A - V;
    Vector dB = B - V;

    float a2 = dA.dot(dA);
    float b2 = dB.dot(dB);

    float d = dB.x*dA.y - dB.y*dA.x;

	float d2 = d*d;

	float test = 1.0e-6f * b2 * a2;

    if ( d2 > test )
    {
        if ( d > 0.0f )
        {
            return CLR_LeftSide;
        }
        else if ( d < 0.0f )
        {
            return CLR_RightSide;
        }
    }

    float t = dA.dot(dB);

    if ( t < 0.0f )
    {
		// V - A - B

        return CLR_OverlapLeft;
    }
	else if (t <= b2)
	{
		// A - V - B

	    return CLR_OverlapCenter;
	}
	else
	{
		// A - B - V

        return CLR_OverlapRight;
	}
}

// ----------------------------------------------------------------------
// Returns true if the line passing through V and poly[index] is left-
// tangent (touches on the counterclockwise side) to the polygon

bool tangentPointL ( Vector const & V, VertexList const & poly, int index )
{
	int size = poly.size();

	int iA = (index > 0) ? index - 1 : size - 1;
	int iB = index;
	int iC = (index < size - 1) ? index + 1 : 0;

	Vector const & A = poly[iA];
	Vector const & B = poly[iB];
	Vector const & C = poly[iC];

	if(testCollinear(V,B,C) == CLR_RightSide)
	{
		if(testCollinear(V,A,B) != CLR_LeftSide)
		{
			return true;
		}
	}

	return false;
}

// ----------
// Returns true if the line passing through V and poly[index] is right-
// tangent (touches on the clockwise side) to the polygon

bool tangentPointR ( Vector const & V, VertexList const & poly, int index )
{
	int size = poly.size();

	int iA = (index > 0) ? index - 1 : size - 1;
	int iB = index;
	int iC = (index < size - 1) ? index + 1 : 0;

	Vector const & A = poly[iA];
	Vector const & B = poly[iB];
	Vector const & C = poly[iC];

	if(testCollinear(V,A,B) == CLR_LeftSide)
	{
		if(testCollinear(V,B,C) != CLR_RightSide)
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

void rightTangent ( Vector const & point, VertexList const & poly, int & index )
{
    int size = poly.size();

    for(int i = 0; i < size; i++)
    {
		if(tangentPointR(point,poly,i))
		{
			index = i;
			return;
		}
    }

    FATAL( true, ("CollisionUtils::rightTangent(point,poly) - couldn't find a valid tangent\n"));
}

void leftTangent ( Vector const & point, VertexList const & poly, int & index )
{
    int size = poly.size();

    for(int i = 0; i < size; i++)
    {
		if(tangentPointL(point,poly,i))
		{
			index = i;
			return;
		}
    }

    FATAL( true, ("CollisionUtils::leftTangent(point,poly) - couldn't find a valid tangent\n"));
}

// ----------------------------------------------------------------------
// Find the pair of vertices A, B such that A is the left tangent for
// polyA from B, and B is the right tangent for poly B from A.

// This is a variation on the code from Magic Software, but this one
// works for any pair of starting indices

void rightTangent ( VertexList const & polyA, VertexList const & polyB, int & indexA, int & indexB )
{
    int sizeA = polyA.size();
	int sizeB = polyB.size();

    int i(0);

	indexA = 0;
	indexB = 0;

    for (i = 0; i < sizeA+sizeB; i++)
    {
		bool stepped = false;

		// step around poly A until we find the tangent point for B

		Vector const & B = polyB[indexB];

		while(!tangentPointR( B, polyA, indexA))
		{
			indexA = (indexA < sizeA - 1) ? indexA + 1 : 0;
			stepped = true;
		}

		// step around poly B until we find the tangent point for A

		Vector const & A = polyA[indexA];

		while(!tangentPointL( A, polyB, indexB))
		{
			indexB = (indexB > 0) ? indexB - 1 : sizeB - 1;
			stepped = true;
		}

		// if we didn't go anywhere then both points are valid tangents and we're done.

        if(stepped == false) break;
    }

    FATAL( i >= sizeA+sizeB, ("CollisionUtils::rightTangent(poly,poly) - iterated too far without finding a valid tangent\n"));
}

//----------------------------------------------------------------------------

void appendPoly ( VertexList const & poly, int min, int max, VertexList & outPoly )
{
	int cursor = min;
	int size = poly.size();

	do
	{
		outPoly.push_back( poly[cursor] );

		cursor++;
		if(cursor == size) cursor = 0;
	}
	while (cursor != max);

	outPoly.push_back(poly[max]);
}

// ----------

void joinPolyPoly ( VertexList const & polyA, int minA, int maxA, 
				    VertexList const & polyB, int minB, int maxB, 
					VertexList & outPoly )
{
	appendPoly(polyA,minA,maxA,outPoly);
	appendPoly(polyB,minB,maxB,outPoly);
}

// ----------

void joinPointPoly ( Vector const & point,
					 VertexList const & poly, int min, int max,
					 VertexList & outPoly )
{
	outPoly.push_back(point);
	appendPoly(poly,min,max,outPoly);
}

// ----------

void joinPointPoint ( Vector const & pointA,
					  Vector const & pointB,
					  VertexList & outPoly )
{
	outPoly.push_back(pointA);
	outPoly.push_back(pointB);
}

// ----------------------------------------------------------------------

void MergePointPoint ( Vector const & pointA, Vector const & pointB, VertexList & outPoly )
{
	outPoly.push_back(pointA);
	outPoly.push_back(pointB);
}

// ----------

void MergePointLine ( Vector const & pointA, VertexList const & lineB, VertexList & outPoly )
{
	CollinearResult result = testCollinear( pointA, lineB[0], lineB[1] );

    switch ( result )
    {
    case CLR_RightSide:
		outPoly.push_back(lineB[1]);
		outPoly.push_back(lineB[0]);
		outPoly.push_back(pointA);
        break;

    case CLR_LeftSide:
		outPoly.push_back(lineB[0]);
		outPoly.push_back(lineB[1]);
		outPoly.push_back(pointA);
        break;

    case CLR_OverlapLeft:
		outPoly.push_back(pointA);
		outPoly.push_back(lineB[1]);
        break;

    case CLR_OverlapRight:
		outPoly.push_back(lineB[0]);
		outPoly.push_back(pointA);
        break;

	case CLR_OverlapCenter:
		outPoly.push_back(lineB[0]);
		outPoly.push_back(lineB[1]);
		break;

	default:
		break;
    }
}

// ----------------------------------------------------------------------

void MergePointPoly ( Vector const & pointA, VertexList const & polyB, VertexList & outPoly )
{
	int sizeB = polyB.size();

	if(sizeB == 0) { outPoly.push_back(pointA); return; }
	if(sizeB == 1) { MergePointPoint(pointA,polyB[0],outPoly); return; }
	if(sizeB == 2) { MergePointLine(pointA,polyB,outPoly); return; }

	int minB(0);
	int maxB(0);

	rightTangent(pointA,polyB,minB);
	leftTangent(pointA,polyB,maxB);

	joinPointPoly(pointA,polyB,minB,maxB,outPoly);
}

// ----------------------------------------------------------------------

void MergePolyPoly ( VertexList & polyA, VertexList & polyB, VertexList & outPoly )
{
	int sizeA = polyA.size();
	int sizeB = polyB.size();

	if(sizeA == 0) { outPoly = polyB; return; }
    if(sizeA == 1) { MergePointPoly(polyA[0],polyB,outPoly); return; }

	if(sizeB == 0) { outPoly = polyA; return; }
	if(sizeB == 1) { MergePointPoly(polyB[0],polyA,outPoly); return; }

	// ----------

	int minA, maxA, minB, maxB;

	rightTangent(polyA,polyB,maxA,minB);
	rightTangent(polyB,polyA,maxB,minA);

	joinPolyPoly(polyA,minA,maxA,polyB,minB,maxB,outPoly);
}

// ----------------------------------------------------------------------

void BuildConvexHull ( Vector const * sortedVerts, int vertCount, VertexList & outPoly )
{
	if(sortedVerts == nullptr) return;

	if(vertCount <= 0) return;
	if(vertCount == 1) { outPoly.push_back(sortedVerts[0]); return; }
	if(vertCount == 2) { outPoly.push_back(sortedVerts[0]); outPoly.push_back(sortedVerts[1]); return; }

	VertexList polyA;
	VertexList polyB;

	int center = vertCount / 2;

	BuildConvexHull(sortedVerts,center,polyA);
	BuildConvexHull(&sortedVerts[center],vertCount - center, polyB);

	MergePolyPoly(polyA,polyB,outPoly);
}

// ----------

bool VertexSorterX ( Vector const & A, Vector const & B )
{
	return A.x < B.x;
}

void BuildConvexHull ( VertexList const & verts, VertexList & outPoly )
{
	VertexList sortedVerts = verts;

	std::sort(sortedVerts.begin(),sortedVerts.end(),VertexSorterX);

	int count = verts.size();

	BuildConvexHull( &sortedVerts.front(), count, outPoly );
}

// ----------------------------------------------------------------------
// 3-d exit test

// This is kinda freaky to try and visualize...

// This test does the same basic thing as projecting the segment onto the triangle
// and determining which edge the segment exited.

// It does it in a slightly different way than normal though - instead of doing a
// projection and then doing segment intersection, it creates a ribbon from the
// given segment and the "down" vector, and determines which edge of the triangle
// goes through that ribbon from front to back. 

int		TestExitTri		( Ribbon3d const & R, Triangle3d const & tri )
{
	if(TestSegRibbon(Segment3d(tri.getCornerA(),tri.getCornerB()),R) == HR_HitBack) return 0;
	if(TestSegRibbon(Segment3d(tri.getCornerB(),tri.getCornerC()),R) == HR_HitBack) return 1;
	if(TestSegRibbon(Segment3d(tri.getCornerC(),tri.getCornerA()),R) == HR_HitBack) return 2;

	return -1;
}

int     TestEntranceTri ( Ribbon3d const & R, Triangle3d const & tri )
{
	if(TestSegRibbon(Segment3d(tri.getCornerA(),tri.getCornerB()),R) == HR_HitFront) return 0;
	if(TestSegRibbon(Segment3d(tri.getCornerB(),tri.getCornerC()),R) == HR_HitFront) return 1;
	if(TestSegRibbon(Segment3d(tri.getCornerC(),tri.getCornerA()),R) == HR_HitFront) return 2;

	return -1;
}

// ----------

Vector	MoveIntoTriangle	( Vector const & inPoint, Triangle3d const & tri, real desiredDist )
{
	Vector outPoint = inPoint;

	for(int i = 0; i < 3; i++)
	{
		Vector planePoint = tri.getCorner(i);

		Vector edge = tri.getCorner(i+1) - tri.getCorner(i);
		Vector normal = tri.getNormal();
		Vector planeNormal = normal.cross(edge);

		IGNORE_RETURN(planeNormal.normalize());

		real dist = Distance3d::DistancePointPlane(outPoint,Plane3d(planePoint,planeNormal));

		if(dist < desiredDist)
		{
			outPoint += planeNormal * (desiredDist - dist);
		}
	}

	return outPoint;
}

// ----------------------------------------------------------------------
// Assumes that the two triangles are neighbors and that they share 
// the edges given by edgeA and edgeB

bool	IsEdgeConcave	( Triangle3d const & triA, int edgeA,
						  Triangle3d const & triB )
{
	return Containment3d::TestPointTri( triA.getCorner(edgeA+2), triB ) == CR_Outside;
}

// ----------------------------------------------------------------------
// Translate the polygon the minimum amount to make it coincident with
// the given vertex

void MovePolyOnto ( VertexList & verts, Vector V )
{
	Vector close = Distance3d::ClosestPointPoly(V,verts);

	for(uint i = 0; i < verts.size(); i++)
	{
		verts[i] = verts[i] - close;
	}
}

// ----------------------------------------------------------------------
// Avoidance theta calculation

RangeLoop CalcAvoidanceThetas_Point ( Vector const & A, Vector const & B )
{
	Vector D = B - A;
	D.y = 0.0f;

	if(D.magnitudeSquared() < 0.000001f) return RangeLoop::empty;

	float theta = atan2(D.z,D.x) / PI_TIMES_2;

	return RangeLoop(theta,theta);
}

// ----------

RangeLoop CalcAvoidanceThetas ( Vector const & A, Sphere const & S )
{
	Vector D = S.getCenter() - A;
	D.y = 0.0f;

	float dist = D.magnitude();
	float radius = S.getRadius();

	if(dist < 0.000001f) return RangeLoop::empty;

	float thetaA = atan2(D.z,D.x) / PI_TIMES_2;

	float thetaB;

	if(dist < radius) 
	{
		thetaB = 0.25f;
	}
	else
	{
		thetaB = asin( radius/dist ) / PI_TIMES_2;
	}

	return RangeLoop( thetaA - thetaB, thetaA + thetaB );
}

// ----------

RangeLoop CalcAvoidanceThetas_Point ( Sphere const & S, Vector const & V )
{
	return CalcAvoidanceThetas ( S.getCenter(), Sphere(V,S.getRadius()) );
}

// ----------

template< class RoundThing >
RangeLoop CalcAvoidanceThetas_Round ( Sphere const & A, RoundThing const & B )
{
	return CalcAvoidanceThetas ( A.getCenter(), Sphere(B.getCenter(), A.getRadius() + B.getRadius()) );
}

// ----------

template< class RoundThing >
RangeLoop CalcAvoidanceThetas_Round ( Vector const & A, RoundThing const & B )
{
	return CalcAvoidanceThetas ( A, Sphere(B.getCenter(), B.getRadius()) );
}

// ----------

template<class Thing, class Box>
RangeLoop CalcAvoidanceThetas_Box ( Thing const & S, Box const & box )
{
	RangeLoop range = RangeLoop::empty;

	for(int i = 0; i < 8; i++)
	{
		RangeLoop temp = CalcAvoidanceThetas_Point(S,box.getCorner(i));

		range = RangeLoop::enclose(range,temp);
	}

	return range;
}

// ----------

template<class Thing>
RangeLoop CalcAvoidanceThetas ( Thing const & T, MultiShape const & S )
{
	MultiShape::ShapeType type = S.getShapeType();

	// ----------

	if     (type == MultiShape::MST_AxialBox)         { return CalcAvoidanceThetas_Box( T, S.getAxialBox() ); }
	else if(type == MultiShape::MST_YawedBox)         { return CalcAvoidanceThetas_Box( T, S.getYawedBox() ); }
	else if(type == MultiShape::MST_OrientedBox)      { return CalcAvoidanceThetas_Box( T, S.getOrientedBox() ); }
	else if(type == MultiShape::MST_Cylinder)         { return CalcAvoidanceThetas_Round( T, S.getCylinder() ); }
	else if(type == MultiShape::MST_Sphere)           { return CalcAvoidanceThetas_Round( T, S.getSphere() ); }
	else                                              { return RangeLoop::empty; }
}

// ----------

RangeLoop CalcAvoidanceThetas ( Sphere const & S, BaseExtent const * extent );

RangeLoop CalcAvoidanceThetas ( Sphere const & S, SimpleExtent const * extent )
{
	if(extent == nullptr) return RangeLoop::empty;

	return CalcAvoidanceThetas(S,extent->getShape());
}

// ----------

RangeLoop CalcAvoidanceThetas ( Sphere const & S, ComponentExtent const * extent )
{
	if(extent == nullptr) return RangeLoop::empty;

	RangeLoop range = RangeLoop::empty;

	int count = extent->getExtentCount();

	for(int i = 0; i < count; i++)
	{
		BaseExtent const * child = extent->getExtent(i);

		if(child == nullptr) continue;

		RangeLoop temp = CalcAvoidanceThetas(S,child);

		range = RangeLoop::enclose(range,temp);
	}

	return range;
}

// ----------

RangeLoop CalcAvoidanceThetas ( Sphere const & S, DetailExtent const * extent )
{
	if(extent == nullptr) return RangeLoop::empty;

	int count = extent->getExtentCount();

	BaseExtent const * child = extent->getExtent(count - 1);

	if(child == nullptr) return RangeLoop::empty;

	return CalcAvoidanceThetas(S,child);
}

// ----------

RangeLoop CalcAvoidanceThetas ( Sphere const & S, BaseExtent const * extent )
{
	if(extent == nullptr) return RangeLoop::empty;

	ExtentType type = extent->getType();

	// ----------

	if     (type == ET_Simple)    return CalcAvoidanceThetas(S,safe_cast<SimpleExtent const *>(extent));
	else if(type == ET_Component) return CalcAvoidanceThetas(S,safe_cast<ComponentExtent const *>(extent));
	else if(type == ET_Detail)    return CalcAvoidanceThetas(S,safe_cast<DetailExtent const *>(extent));
	else                          return RangeLoop::empty;
}

// ----------

RangeLoop CalcAvoidanceThetas ( Sphere const & S, ExtentVec const & extents )
{
	RangeLoop range = RangeLoop::empty;

	int count = extents.size();

	for(int i = 0; i < count; i++)
	{
		BaseExtent const * child = extents[i];

		if(child == nullptr) continue;

		RangeLoop temp = CalcAvoidanceThetas(S,child);

		range = RangeLoop::enclose(range,temp);
	}

	return range;
}

// ----------------------------------------------------------------------

bool intersectRays ( Vector const & A, float thetaA, Vector const & B, float thetaB, Vector & out )
{
	thetaA *= PI_TIMES_2;
	thetaB *= PI_TIMES_2;

	Vector nA( cos(thetaA), 0.0f, sin(thetaA) );
	Vector nB( cos(thetaB), 0.0f, sin(thetaB) );

	return Intersect2d::IntersectRayRay( Ray3d(A,nA), Ray3d(B,nB), out );
}

// ----------

bool CalcAvoidancePoint ( Vector const & A, Vector const & B, RangeLoop const & rangeA, RangeLoop const & rangeB, Vector & out )
{
	Vector delta = B - A;
	
	delta.y = 0.0f;

	float thetaA = atan2(delta.z,delta.x) / PI_TIMES_2;
	float thetaB = thetaA + 0.5f;

	// if the ranges are empty, then we have no collision - return the destination
	if(rangeA.isEmpty() || rangeB.isEmpty())
	{
		out = B;
		return true;
	}

	if(rangeB.getSize() > 0.6f) return false; // goal is inside the thing we're trying to avoid

	// if either range contains exclusively the respective theta, we can't find a good path
	if(!rangeA.containsExclusive(thetaA) || !rangeB.containsExclusive(thetaB)) return false;

	float minA = rangeA.getMin();
	float maxA = rangeA.getMax();
	float minB = rangeB.getMin();
	float maxB = rangeB.getMax();

	Vector C = Vector::zero;
	Vector D = Vector::zero;

	bool foundC = intersectRays( A, minA, B, maxB, C );
	bool foundD = intersectRays( A, maxA, B, minB, D );

	float distAC = A.magnitudeBetween(C);
	float distBC = B.magnitudeBetween(C);

	float distAD = A.magnitudeBetween(D);
	float distBD = B.magnitudeBetween(D);

	bool gotoC = true;

	if(foundC && foundD)
	{
		gotoC = (distAC + distBC) < (distAD + distBD);
	}
	else if(foundC)
	{
		gotoC = true;
	}
	else if(foundD)
	{
		gotoC = false;
	}
	else
	{
		// both start and end are partially inside the obstacle.
		// pick a silhouette theta and walk towards it

		float diffThetaA = RangeLoop(minA,maxB).getSize();
		float diffThetaB = RangeLoop(minB,maxA).getSize();

		if(diffThetaA > diffThetaB)
		{
			float t = minA * PI_TIMES_2;

			Vector offset( cos(t), 0.0f, sin(t) );

			out = A + offset;
		}
		else
		{
			float t = maxA * PI_TIMES_2;

			Vector offset( cos(t), 0.0f, sin(t) );

			out = A + offset;
		}

		return true;
	}

	// Explanation - In some cases obstacle avoidance can produce an avoidance point that's
	// really really really close to the start point. Since moving there doesn't get the mob
	// to the goal effectively, in those cases we scoot the avoidance point out a bit
	// to keep the mob from getting stuck.

	const float cornerScoot = 0.3f;

	if(gotoC)
	{
		if(distAC >= cornerScoot)
		{
			out = C;
		}
		else
		{
			// Avoidance point is too close to starting point - move it away from the obstacle
			float bisectTheta = RangeLoop(maxB,minA).atParam(0.5f) * PI_TIMES_2;

			Vector offset( cos(bisectTheta), 0.0f, sin(bisectTheta) );

			offset *= cornerScoot - distAC;

			out = C + offset;
		}
	}
	else
	{
		if(distAD >= cornerScoot)
		{
			out = D;
		}
		else
		{
			float bisectTheta = RangeLoop(maxA,minB).atParam(0.5f) * PI_TIMES_2;

			Vector offset( cos(bisectTheta), 0.0f, sin(bisectTheta) );

			offset *= cornerScoot - distAD;

			out = D + offset;
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool CalcAvoidancePoint ( Sphere const & A, Vector const & delta, BaseExtent const * E, Vector & out )
{
	Sphere B(A.getCenter() + delta,A.getRadius());

	RangeLoop rangeA = CalcAvoidanceThetas(A,E);
	RangeLoop rangeB = CalcAvoidanceThetas(B,E);

	return CalcAvoidancePoint(A.getCenter(),B.getCenter(),rangeA,rangeB,out);
}

bool CalcAvoidancePoint ( Sphere const & A, Vector const & delta, ExtentVec const & extents, Vector & out )
{
	Sphere B(A.getCenter() + delta,A.getRadius());

	RangeLoop rangeA = CalcAvoidanceThetas(A,extents);
	RangeLoop rangeB = CalcAvoidanceThetas(B,extents);

	return CalcAvoidancePoint(A.getCenter(),B.getCenter(),rangeA,rangeB,out);
}

// ----------------------------------------------------------------------
// Obstacle exploding - break down an obstacle list into a set of simple
// extents that need to be avoided

void explodeObstacle ( Sphere const & sphere, Vector const & delta, BaseExtent const * extent, ExtentVec & outList );

void explodeObstacle ( Sphere const & sphere, Vector const & delta, SimpleExtent const * extent, ExtentVec & outList )
{
	DetectResult result = CollisionDetect::testSphereExtent(sphere,delta,extent);

	if(result.collided)
	{
		outList.push_back(extent);
	}
}

void explodeObstacle ( Sphere const & sphere, Vector const & delta, DetailExtent const * extent, ExtentVec & outList )
{
	if(extent == nullptr) return;

	int count = extent->getExtentCount();

	explodeObstacle(sphere,delta,extent->getExtent(count-1),outList);
}

void explodeObstacle ( Sphere const & sphere, Vector const & delta, ComponentExtent const * extent, ExtentVec & outList )
{
	if(extent == nullptr) return;

	int count = extent->getExtentCount();

	for(int i = 0; i < count; i++)
	{
		explodeObstacle(sphere,delta,extent->getExtent(i),outList);
	}
}

void explodeObstacle ( Sphere const & sphere, Vector const & delta, BaseExtent const * extent, ExtentVec & outList )
{
	if(extent == nullptr) return;

	ExtentType type = extent->getType();

	if     (type == ET_Simple)    { explodeObstacle(sphere,delta,safe_cast<SimpleExtent const *>(extent),outList); }
	else if(type == ET_Component) { explodeObstacle(sphere,delta,safe_cast<ComponentExtent const *>(extent),outList); }
	else if(type == ET_Detail)    { explodeObstacle(sphere,delta,safe_cast<DetailExtent const *>(extent),outList); }
}

void explodeObstacle ( Sphere const & sphere, Vector const & delta, ExtentVec const & extentVec, ExtentVec & outList )
{
	int count = extentVec.size();

	for(int i = 0; i < count; i++)
	{
		explodeObstacle(sphere,delta,extentVec[i],outList);
	}
}

// ----------------------------------------------------------------------

bool CalcAvoidancePoint ( Sphere const & sphere, Transform const & sphereTransform_p2w, Vector const & delta, CollisionProperty const * obstacle, Vector & out )
{
	if(obstacle == nullptr) return false;

	static ExtentVec tempExtents;

	BaseExtent const * obstacleExtent = obstacle->getExtent_p();
	if(obstacleExtent)
	{
		// Use the normal obstacle extents.
		explodeObstacle(sphere,delta,obstacleExtent,tempExtents);
	}
	else
	{
		// The object doesn't have a set of extents.  See if the object has a floor.
		Floor const *const floor = obstacle->getFloor();
		if (!floor)
			return false;

		// Build a SimpleExtent out of the floor surrounding sphere.
		Sphere const floorSphere_w = floor->getBoundingSphere_w();

		// Build an obstacle sphere in the same parent space as the sphere (mob) parameter.
		Vector const floorSphereCenter_mobP = sphereTransform_p2w.rotateTranslate_p2l(floorSphere_w.getCenter());
		Sphere const floorSphere_mobP(floorSphereCenter_mobP, floorSphere_w.getRadius());

		// Build a SimpleExtent.  This needs to be static since tempExtents stores a pointer to it.
		static SimpleExtent  floorExtent;
		floorExtent.setShape(MultiShape(floorSphere_mobP));

		// Add shape to temp extents.
		explodeObstacle(sphere, delta, &floorExtent, tempExtents);
	}


	bool result = CalcAvoidancePoint(sphere,delta,tempExtents,out);

	tempExtents.clear();

	return result;
}

// ----------

bool CalcAvoidancePoint ( CollisionProperty const * mob, Vector const & delta, CollisionProperty const * obstacle, Vector & out )
{
	if(mob == nullptr) return false;
	if(obstacle == nullptr) return false;

	BaseExtent const * mobExtent = mob->getExtent_p();

	if(mobExtent == nullptr) return false;

	Sphere mobSphere = mobExtent->getBoundingSphere();

	return CalcAvoidancePoint(mobSphere,mob->getOwner().getTransform_p2w(),delta,obstacle,out);
}

// ----------

bool CalcAvoidancePoint ( Object const * mob, Vector const & delta, Object const * obstacle, Vector & out )
{
	if(mob == nullptr) return false;
	if(obstacle == nullptr) return false;

	CollisionProperty const * mobCollision = mob->getCollisionProperty();
	CollisionProperty const * obstacleCollision = obstacle->getCollisionProperty();

	return CalcAvoidancePoint(mobCollision,delta,obstacleCollision,out);
}

// ----------------------------------------------------------------------
// normalized obstacle space - ship at the origin moving along positive Z

bool CalcAvoidancePoints3d ( Sphere const & obstacle, Vector & avoidA, Vector & avoidB )
{
	Vector center = obstacle.getCenter();
	float radius = obstacle.getRadius();

	Vector delta = -center;

	delta.z = 0.0f;

	delta.normalize();

	delta *= radius;

	avoidA = Vector( center.x + delta.x, center.y + delta.y, center.z - radius );
	avoidB = Vector( center.x + delta.x, center.y + delta.y, center.z + radius );

	return true;
}

// ----------------------------------------------------------------------

bool CalcAvoidancePoints3d ( Sphere const & mob, Vector const & delta, Sphere const & obstacle, Vector & avoidA, Vector & avoidB )
{
	Vector K = delta;

	K.normalize();

	Vector I,J;

	if(std::abs(K.y) > 0.95)
	{
		// line A-B is almost vertical, build orthonormal basis using X axis

		J = K.cross(Vector::unitX);
		J.normalize();

		I = J.cross(K);
		I.normalize();
	}
	else
	{
		I = Vector::unitY.cross(K);
		I.normalize();

		J = K.cross(I);
		J.normalize();
	}

	Transform T;

	T.setLocalFrameIJK_p(I,J,K);
	T.setPosition_p(mob.getCenter());

	Sphere normalizedObstacle(T.rotateTranslate_p2l(obstacle.getCenter()),obstacle.getRadius() + mob.getRadius());
	
	bool calcOk = CalcAvoidancePoints3d ( normalizedObstacle, avoidA, avoidB );

	if(!calcOk) return false;

	avoidA = T.rotateTranslate_l2p(avoidA);
	avoidB = T.rotateTranslate_l2p(avoidB);

	return true;
}

// ----------------------------------------------------------------------

}	// namespace Collision3d

// ----------------------------------------------------------------------

namespace CollisionUtils
{

Vector transformToWorld ( CellProperty const * cellA, Vector const & point_A )
{
	return transformToCell( cellA, point_A, CellProperty::getWorldCellProperty() );
}

Sphere transformToWorld ( CellProperty const * cellA, Sphere const & sphere_A )
{
	return transformToCell( cellA, sphere_A, CellProperty::getWorldCellProperty() );
}

Capsule transformToWorld ( CellProperty const * cellA, Capsule const & capsule_A )
{
	return transformToCell( cellA, capsule_A, CellProperty::getWorldCellProperty() );
}

Transform transformToWorld ( CellProperty const * cellA, Transform const & transform_A )
{
	return transformToCell( cellA, transform_A, CellProperty::getWorldCellProperty() );
}

Vector transformFromWorld ( Vector const & point_w, CellProperty const * cellB )
{
	return transformToCell( CellProperty::getWorldCellProperty(), point_w, cellB );
}

Sphere transformFromWorld ( Sphere const & sphere_w, CellProperty const * cellB )
{
	return transformToCell ( CellProperty::getWorldCellProperty(), sphere_w, cellB );
}

Capsule transformFromWorld ( Capsule const & capsule_w, CellProperty const * cellB )
{
	return transformToCell ( CellProperty::getWorldCellProperty(), capsule_w, cellB );
}

Transform transformFromWorld ( Transform const & transform_w, CellProperty const * cellB )
{
	return transformToCell( CellProperty::getWorldCellProperty(), transform_w, cellB );
}

Vector transformToCell ( CellProperty const * cellA, Vector const & point_A, CellProperty const * cellB )
{
	CellProperty const * worldCell = CellProperty::getWorldCellProperty();

	if(cellA == nullptr) cellA = worldCell;
	if(cellB == nullptr) cellB = worldCell;

	if(cellA == cellB) return point_A;

	if(cellA == CellProperty::getWorldCellProperty())
	{
		return cellB->getOwner().rotateTranslate_w2o(point_A);
	}
	else if(cellB == CellProperty::getWorldCellProperty())
	{
		return cellA->getOwner().rotateTranslate_o2w(point_A);
	}
	else
	{
		Vector point_w = cellA->getOwner().rotateTranslate_o2w(point_A);
		Vector point_B = cellB->getOwner().rotateTranslate_w2o(point_w);

		return point_B;
	}
}

Vector rotateToCell ( CellProperty const * cellA, Vector const & point_A, CellProperty const * cellB )
{
	CellProperty const * worldCell = CellProperty::getWorldCellProperty();

	if(cellA == nullptr) cellA = worldCell;
	if(cellB == nullptr) cellB = worldCell;

	if(cellA == cellB) return point_A;

	if(cellA == CellProperty::getWorldCellProperty())
	{
		return cellB->getOwner().rotate_w2o(point_A);
	}
	else if(cellB == CellProperty::getWorldCellProperty())
	{
		return cellA->getOwner().rotate_o2w(point_A);
	}
	else
	{
		Vector point_w = cellA->getOwner().rotate_o2w(point_A);
		Vector point_B = cellB->getOwner().rotate_w2o(point_w);

		return point_B;
	}
}

// ----------------------------------------------------------------------

Transform transformToCell ( CellProperty const * cellA, Transform const & transform_A, CellProperty const * cellB )
{
	CellProperty const * worldCell = CellProperty::getWorldCellProperty();

	if(cellA == nullptr) cellA = worldCell;
	if(cellB == nullptr) cellB = worldCell;

	if(cellA == cellB) return transform_A;

	if(cellA == worldCell)
	{
		Transform const & objectToWorld = transform_A;

		Transform cellAToWorld = cellB->getOwner().getTransform_o2w();

		Transform worldToCellA;
		worldToCellA.invert(cellAToWorld);

		Transform objectToCellA;
		objectToCellA.multiply(worldToCellA, objectToWorld);

		return objectToCellA;
	}
	else if (cellB == worldCell)
	{
		Transform const & objectToCellA = transform_A;

		Transform cellAToWorld = cellA->getOwner().getTransform_o2w();

		Transform objectToWorld;
		objectToWorld.multiply(cellAToWorld, objectToCellA);

		return objectToWorld;
	}
	else
	{
		Transform const & objectToCellA = transform_A;

		Transform cellAToWorld = cellA->getOwner().getTransform_o2w();

		Transform objectToWorld;
		objectToWorld.multiply(cellAToWorld,objectToCellA);

		Transform cellBToWorld = cellB->getOwner().getTransform_o2w();

		Transform worldToCellB;
		worldToCellB.invert(cellBToWorld);

		Transform objectToCellB;
		objectToCellB.multiply(worldToCellB,objectToWorld);

		return objectToCellB;
	}
}

// ----------------------------------------------------------------------

Sphere transformToCell ( CellProperty const * cellA, Sphere const & sphere_A, CellProperty const * cellB)
{
	Vector center_B = transformToCell(cellA,sphere_A.getCenter(),cellB);

	return Sphere(center_B,sphere_A.getRadius());
}

// ----------------------------------------------------------------------

Capsule transformToCell ( CellProperty const * cellA, Capsule const & capsule_A, CellProperty const * cellB)
{
	Vector pointA_B = transformToCell(cellA,capsule_A.getPointA(),cellB);
	Vector pointB_B = transformToCell(cellA,capsule_A.getPointB(),cellB);

	return Capsule(pointA_B,pointB_B,capsule_A.getRadius());
}
// ----------------------------------------------------------------------

MultiShape transformToCell ( CellProperty const * cellA, MultiShape const & shape, CellProperty const * cellB )
{
	Vector center = transformToCell(cellA,shape.getCenter(),cellB);

	Vector axisX =  rotateToCell(cellA,shape.getAxisX(),cellB);
	Vector axisY =  rotateToCell(cellA,shape.getAxisY(),cellB);
	Vector axisZ =  rotateToCell(cellA,shape.getAxisZ(),cellB);

	return MultiShape ( shape.getBaseType(),
	                    center, axisX, axisY, axisZ,
	                    shape.getExtentX(), shape.getExtentY(), shape.getExtentZ() );
}

// ----------------------------------------------------------------------

bool testPortalVis ( CellProperty const * cellA, Vector const & pointA, CellProperty const * cellB, Vector const & pointB )
{
	if(cellA == nullptr) return false;
	if(cellB == nullptr) return false;

	float hitTime = 0.0f;

	// ----------
	// If the two points are in the same cell, they're visible if the line between them doesn't hit any portals in the cell

	if(cellA == cellB)
	{
		return cellA->getDestinationCell(pointA,pointB,hitTime) == nullptr;
	}

	// ----------
	// The two points are in different cells

	// Find point B's location in this cell, and determine if the line between point A
	// and the local point B hits a portal

	Vector localB = transformToCell(cellB,pointB,cellA);
	CellProperty *newCellA = cellA->getDestinationCell(pointA,localB,hitTime);
	if(newCellA)
	{
		// If it does, re-run the query starting in the cell on the other side of the 
		// portal, starting from the intersection of the segment and the portal

		Vector hitPoint = pointA + (localB - pointA) * hitTime;
		Vector newPointA = transformToCell(cellA,hitPoint,newCellA);
		return testPortalVis( newCellA, newPointA, cellB, pointB );
	}

	// The segment never leaves this cell, so it can't possibly get to point B
	return false;
}
// ----------------------------------------------------------------------

bool epsilonEqual ( Sphere const & A, Sphere const & B, float epsilon )
{
	if(std::abs(A.getRadius() - B.getRadius()) > epsilon) return false;

	Vector cA = A.getCenter();
	Vector cB = B.getCenter();

	if(std::abs(cA.x - cB.x) > epsilon) return false;
	if(std::abs(cA.y - cB.y) > epsilon) return false;
	if(std::abs(cA.z - cB.z) > epsilon) return false;

	return true;
}

// ----------------------------------------------------------------------

} // namespace CollisionUtils

// ======================================================================
