// ======================================================================
//
// Distance3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Distance3d.h"

#include "sharedCollision/Collision2d.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/Containment1d.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Intersect2d.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/Overlap3d.h"

#include "sharedMath/AxialBox.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/MultiShape.h"
#include "sharedMath/OrientedBox.h"
#include "sharedMath/OrientedCircle.h"
#include "sharedMath/OrientedCylinder.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Quadratic.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Triangle3d.h"
#include "sharedMath/Vector.h"
#include "sharedMath/YawedBox.h"

#include <vector>
#include <algorithm>

using namespace Intersect3d;

namespace Distance3d
{

// ----------

Vector selectCloser ( Vector const & P, Vector const & A, Vector const & B )
{
	real dist1 = (P-A).magnitudeSquared();
	real dist2 = (P-B).magnitudeSquared();

	if(dist1 < dist2)
	{
		return A;
	}
	else
	{
		return B;
	}
}

// ======================================================================
// Closest distance computation

real DistancePointPoint ( Vector const & A, Vector const & B )
{
	return (B-A).magnitude();
}

// ----------------------------------------------------------------------

real DistancePointLine ( Vector const & V, Line3d const & L )
{
	Vector closestPoint = ClosestPointLine(V,L);

	real dist = (closestPoint - V).magnitude();

	return dist;
}

// ----------

real DistancePointSeg ( Vector const & V, Segment3d const & S )
{
    Vector closestPoint = ClosestPointSeg(V,S);

    real dist = (closestPoint - V).magnitude();

    return dist;
}

// ----------------------------------------------------------------------

real DistancePointPlane ( Vector const & V, Plane3d const & P )
{
	return (V - P.getPoint()).dot(P.getNormal());
}

// ----------

real    DistancePointTri ( Vector const & V, Triangle3d const & T )
{
	Vector P = ClosestPointTri(V,T);

	return DistancePointPoint(P,V);
}

// ----------

real DistancePointPoly ( Vector const & V, VertexList const & vertices )
{
	Vector P = ClosestPointPoly(V,vertices);

	return DistancePointPoint(P,V);
}

// ----------------------------------------------------------------------

float DistancePointSphere ( Vector const & V, Sphere const & S )
{
	float dist = DistancePointPoint(V,S.getCenter()) - S.getRadius();

	if(dist < 0.0f) dist = 0.0f;

	return dist;
}

// ----------

real DistancePointCylinder ( Vector const & V, Cylinder const & C )
{
	Vector close = ClosestPointCylinder(V,C);

	return DistancePointPoint(V,close);
}

// ----------

real DistancePointABox ( Vector const & V, AxialBox const & B )
{
	Vector close = ClosestPointABox(V,B);

	return DistancePointPoint(V,close);
}

// ----------

real DistancePointYBox ( Vector const & V, YawedBox const & B )
{
	Vector close = ClosestPointYBox(V,B);

	return DistancePointPoint(V,close);
}

// ----------

real DistancePointOBox ( Vector const & V, OrientedBox const & B )
{
	Vector close = ClosestPointOBox(V,B);

	return DistancePointPoint(V,close);
}

// ----------------------------------------------------------------------

float DistancePointShape ( Vector const & V, MultiShape const & S )
{
	MultiShape::ShapeType type = S.getShapeType();

	// ----------

	if     (type == MultiShape::MST_Sphere)           { return DistanceBetween( V, S.getSphere() ); }
	else if(type == MultiShape::MST_Cylinder)         { return DistanceBetween( V, S.getCylinder() ); }
	else if(type == MultiShape::MST_AxialBox)         { return DistanceBetween( V, S.getAxialBox() ); }
	else if(type == MultiShape::MST_YawedBox)         { return DistanceBetween( V, S.getYawedBox() ); }
	else if(type == MultiShape::MST_OrientedBox)      { return DistanceBetween( V, S.getOrientedBox() ); }
	else                                              { return REAL_MAX; }
}
// ----------------------------------------------------------------------

real    DistanceLineLine ( Line3d const & A, Line3d const & B )
{
	Vector D = B.getPoint() - A.getPoint();

	Vector N = A.getNormal().cross(B.getNormal());

	IGNORE_RETURN( N.normalize() );

	float dist = static_cast<float>(fabs( D.dot(N) ));

	return dist;
}

// ----------------------------------------------------------------------

real DistanceSphereCylinder ( Sphere const & S, Cylinder const & C )
{
	float dist = DistancePointCylinder(S.getCenter(),C);

	dist -= S.getRadius();

	return dist;
}

// ======================================================================

real Distance2PointPoint ( Vector const & A, Vector const & B )
{
	return (B-A).magnitudeSquared();
}

// ----------

real Distance2PointSeg ( Vector const & V, Segment3d const & S )
{
    Vector closestPoint = ClosestPointSeg(V,S);

    real dist2 = (closestPoint - V).magnitudeSquared();

    return dist2;
}

// ----------

real Distance2PointPoly ( Vector const & V, VertexList const & vertices )
{
	Vector P = ClosestPointPoly(V,vertices);

	return Distance2PointPoint(P,V);
}

// ----------

real Distance2PointABox ( Vector const & V, AxialBox const & B )
{
	Vector close = ClosestPointABox(V,B);

	return Distance2PointPoint(V,close);
}

// ======================================================================
// returns the parametric coordinates for the closest pair on A and B

// The derivation for this is a bit long - express the distance function 
// F in terms of s and t, find the min by solving for where the partial
// derivatives of F are both 0. The math collapses down a lot.

bool ClosestPairLineLine ( Line3d const & A, Line3d const & B, float & outA, float & outB )
{
	Vector D = B.getPoint() - A.getPoint();

	Vector AN = A.getNormal();
	Vector BN = B.getNormal();

	float aa = AN.dot(AN);
	float ab = AN.dot(BN);
	float bb = BN.dot(BN);

	float ad = AN.dot(D);
	float bd = BN.dot(D);

	Matrix2 M( aa, -ab, -ab, bb );

	Vector2d O(0.0f,0.0f);
	
	if(M.solveFor(Vector2d(ad,-bd),O))
	{
		outA = O.x;
		outB = O.y;

		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

Vector ClosestPointLine ( Vector const & V, Line3d const & L )
{
	Vector B = L.getPoint();
	Vector N = L.getNormal();

	float n2 = N.magnitudeSquared();

	Vector C = V - B;

	real t = C.dot(N) / n2;

	return B + N * t;
}

// ----------

Vector ClosestPointRay ( Vector const & V, Ray3d const & R )
{
	Vector B = R.getPoint();
	Vector N = R.getNormal();

	float n2 = N.magnitudeSquared();

	Vector C = V - B;

	float t = std::max( 0.0f, C.dot(N) / n2 );

	return B + N * t;
}

// ----------

Vector ClosestPointSeg ( Vector const & V, Segment3d const & S )
{
	Vector B = S.getBegin();
	Vector N = S.getDelta();

	float n2 = N.magnitudeSquared();

	Vector C = V - B;

	float t = clamp( 0.0f, C.dot(N) / n2, 1.0f );

	return B + N * t;
}

// ----------

Vector ClosestPointPlane ( Vector const & V, Plane3d const & P )
{
	Vector point;
	real param;

	Line3d line(V,P.getNormal());

	bool hit = IntersectLinePlaneUnsided( line, P, point, param );

	if(!hit)
	{
		DEBUG_FATAL(true,("ClosestPointPlane - Can't intersect line with plane"));
		
		return P.getPoint();
	}

	return point;
}

// ----------

Vector ClosestPointTri ( Vector const & V, Triangle3d const & T )
{
	{
		Line3d L(V,T.getNormal());

		Vector point;
		float param;

		if(IntersectLineTriUnsided(L,T,point,param))
		{
			return point;
		}
	}

	Vector edgeA = ClosestPointSeg(V,T.getEdgeSegment0());
	Vector edgeB = ClosestPointSeg(V,T.getEdgeSegment1());
	Vector edgeC = ClosestPointSeg(V,T.getEdgeSegment2());

	Vector out = edgeA;
	
	out = selectCloser(V,out,edgeB);
	out = selectCloser(V,out,edgeC);

	return out;
}

// ----------

Vector ClosestPointSphere ( Vector const & V, Sphere const & S )
{
	Vector delta = V - S.getCenter();

	IGNORE_RETURN( delta.normalize() );

	delta *= S.getRadius();

	Vector point = S.getCenter() + delta;

	return point;
}

// ----------

Vector ClosestPointCylinder ( Vector const & V, Cylinder const & C )
{
	Vector delta = V - C.getBase();

	delta.y = 0.0f;

	real dist = delta.magnitude();

	Vector point = V;

	// clamp the x-z coords of the point so they're inside the tube

	IGNORE_RETURN( delta.normalize() );
	delta *= std::min( C.getRadius(), dist );
	
	point = C.getBase() + delta;

	// and clamp the y coord so it's inside also

	real min = C.getBase().y;
	real max = min + C.getHeight();

	point.y = clamp( min, V.y, max );

	return point;
}

// ----------

Vector ClosestPointABox ( Vector const & V, AxialBox const & B )
{
	Vector temp;

	Vector min = B.getMin();
	Vector max = B.getMax();

	temp.x = clamp( min.x, V.x, max.x );
	temp.y = clamp( min.y, V.y, max.y );
	temp.z = clamp( min.z, V.z, max.z );

	return temp;
}

Vector ClosestPointYBox ( Vector const & V, YawedBox const & B )
{
	Vector lV = B.transformToLocal(V);

	AxialBox lB = B.getLocalShape();

	Vector lC = ClosestPointABox(lV,lB);

	Vector out = B.transformToWorld(lC);

	return out;
}

Vector ClosestPointOBox ( Vector const & V, OrientedBox const & B )
{
	Vector lV = B.transformToLocal(V);

	AxialBox lB = B.getLocalShape();

	Vector lC = ClosestPointABox(lV,lB);

	Vector out = B.transformToWorld(lC);

	return out;
}

// ----------------------------------------------------------------------

Vector ClosestPointYLine ( Segment3d const & S, Vector const & V )
{
	Line3d A = S.getLine();
	Line3d B(V,Vector::unitY);

	float paramA;
	float paramB;

	ClosestPairLineLine(A,B,paramA,paramB);

	if(paramA < 0.0f) paramA = 0.0f;
	if(paramA > 1.0f) paramA = 1.0f;

	return S.atParam(paramA);
}

// ----------------------------------------------------------------------

// This is a really crappy way to find the closest point on a polygon, but it works.

// Find the closest point on a polygon to a given point by tesselating the polygon
// into triangles and finding the closest point on any triangle.

// This will fail for non-convex polygons.

Vector ClosestPointPoly ( Vector const & V, VertexList const & vertices )
{
	if(vertices.size() == 0) return V;

	if(vertices.size() == 1) return vertices[0];

	if(vertices.size() == 2) return ClosestPointSeg( V, Segment3d(vertices[0],vertices[1]) );

	// ----------

	Vector closest = ClosestPointTri( V, Triangle3d(vertices[0],vertices[1],vertices[2]) );

	int nTris = vertices.size() - 2;

	for(int i = 1; i < nTris; i++)
	{
		Triangle3d tri( vertices[0], vertices[i+1], vertices[i+2] );

		Vector temp = ClosestPointTri(V,tri);

		closest = selectCloser(V,closest,temp);
	}

	return closest;
}

// ----------------------------------------------------------------------
// Separating plane calculation

// Given the results of two object-plane containment tests, determine if
// the objects are separated by the plane

bool TestSeparated ( ContainmentResult A, ContainmentResult B )
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE( CR_Outside, A, CR_Invalid);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE( CR_Outside, A, CR_Invalid);

	// If either object overlaps the plane, the plane is not a separating plane.

	if((A == CR_Overlap) || (B == CR_Overlap)) return false;

	// If either object lies in the plane, the plane is a separating plane
	// unless both objects lie in the plane.

	if((A == CR_Boundary) || (B == CR_Boundary))
	{
		return !((A == CR_Boundary) && (B == CR_Boundary));
	}

	// Otherwise, if the objects are not on the same side of the plane,
	// the plane is a separating plane.

	bool aInside = ((A == CR_Inside) || (A == CR_TouchingInside));
	bool bInside = ((B == CR_Inside) || (B == CR_TouchingInside));

	return aInside != bInside; //lint !e731
}

bool TestSeparated        ( Triangle3d const & A, 
						    Triangle3d const & B, 
							Plane3d const & P,
							real epsilon )
{
	ContainmentResult testA = Containment3d::TestTriPlane(A,P,epsilon);
	ContainmentResult testB = Containment3d::TestTriPlane(B,P,epsilon);

	return TestSeparated(testA,testB);
}

bool TestCoplanar( Triangle3d const & A, Triangle3d const & B, real epsilon )
{
	int testA = Containment3d::TestTriPlane(A,B.getPlane(),epsilon);
	int testB = Containment3d::TestTriPlane(B,A.getPlane(),epsilon);

	return (testA == CR_Boundary) && (testB == CR_Boundary);
}

// ----------------------------------------------------------------------
// Compute a separating plane for two coplanar triangles by testing
// planes perpendicular to the triangle edges

bool SeparateTriTri_coplanar ( Triangle3d const & A, Triangle3d const & B, Plane3d & out, real epsilon )
{
	if(!TestCoplanar(A,B,epsilon))
	{
		return false;
	}

	// ----------
	// Test if triangle B is outside any of A's edge-planes

	{
		for(int i = 0; i < 3; i++)
		{
			Plane3d P = Plane3d( A.getCorner(i), A.getEdgeDir(i).cross( A.getNormal() ) );

			ContainmentResult test = Containment3d::TestTriPlane( B, P, epsilon );

			if( (test == CR_Outside) || (test == CR_TouchingOutside) || (test == CR_Boundary) ) 
			{
				out = P;
				return true;
			}
		}
	}
	
	// ----------
	// Test if triangle A is outside any of B's edge-planes

	{
		for(int i = 0; i < 3; i++)
		{
			Plane3d P = Plane3d( B.getCorner(i), B.getEdgeDir(i).cross( B.getNormal() ) );

			ContainmentResult test = Containment3d::TestTriPlane( A, P, epsilon );

			if( (test == CR_Outside) || (test == CR_TouchingOutside) || (test == CR_Boundary) ) 
			{
				out = P;
				return true;
			}
		}
	}

	// ----------

	return false;
}

// ----------------------------------------------------------------------

bool Separate ( Triangle3d const & A, Triangle3d const & B, Plane3d & out, real epsilon )
{
	// If the two triangles are intersecting, no separating plane exists

	//@todo - Is it more efficient to use SeparateTriTri to compute Overlap3d::TestTriTri via
	// determining that no separating planes exist?

	if(Overlap3d::TestTriTri(A,B)) 
	{
		return false;
	}

	// If the triangles are coplanar we have to generate the separating plane
	// differently

	if(TestCoplanar(A,B,epsilon)) 
	{
		return SeparateTriTri_coplanar(A,B,out,epsilon);
	}

	// ----------
	// Check to see if either triangle is a separating plane

	if(TestSeparated(A,B,A.getPlane(),epsilon))
	{
		out = A.getPlane();
		return true;
	}

	if(TestSeparated(A,B,B.getPlane(),epsilon))
	{
		out = B.getPlane();
		return true;
	}

	// ----------
	// Neither triangle is itself a separating plane, so try all edge-vert pairs

	{
		// A's edges, B's vertices

		for(int i = 0; i < 3; i++)
		{
			Segment3d S = A.getEdgeSegment(i);

			for(int j = 0; j < 3; j++)
			{
				Vector V = B.getCorner(j);

				Plane3d P(V,S.getBegin(),S.getEnd());

				if(TestSeparated(A,B,P,epsilon))
				{
					out = P;
					return true;
				}
			}
		}
	}

	{
		// B's edges, A's vertices

		for(int i = 0; i < 3; i++)
		{
			Segment3d S = B.getEdgeSegment(i);

			for(int j = 0; j < 3; j++)
			{
				Vector V = A.getCorner(j);

				Plane3d P(V,S.getBegin(),S.getEnd());

				if(TestSeparated(A,B,P,epsilon))
				{
					out = P;
					return true;
				}
			}
		}
	}

	// ----------
	// No separating plane found!

	return false;
}

// ----------------------------------------------------------------------
// Project a point onto an axis

float ProjectAxis ( Line3d const & L, Vector const & V )
{
	return Collision3d::ComponentAlong( V - L.getPoint(), L.getNormal() );
}

// ----------

Range ProjectAxis ( Line3d const & L, AxialBox const & B )
{
	Vector const & N = L.getNormal();
	Vector extent = B.getDelta();

    real x = std::abs( Collision3d::ComponentAlong( Vector::unitX, N ) ) * extent.x;
    real y = std::abs( Collision3d::ComponentAlong( Vector::unitY, N ) ) * extent.y;
    real z = std::abs( Collision3d::ComponentAlong( Vector::unitZ, N ) ) * extent.z;

	real d = x + y + z;

	real c = ProjectAxis ( L, B.getCenter() );

	return Range( c - d, c + d );
}

// ----------

Range ProjectAxis ( Line3d const & L, OrientedBox const & B )
{
	Vector const & N = L.getNormal();

	real x = std::abs( Collision3d::ComponentAlong( B.getAxisX(), N ) ) * B.getExtentX();
	real y = std::abs( Collision3d::ComponentAlong( B.getAxisY(), N ) ) * B.getExtentY();
	real z = std::abs( Collision3d::ComponentAlong( B.getAxisZ(), N ) ) * B.getExtentZ();

	real d = x + y + z;

	real c = ProjectAxis( L, B.getCenter() );

	return Range( c - d, c + d );
}

// ----------

Range ProjectAxis ( Line3d const & L, Sphere const & S )
{
	real d = S.getRadius() / L.getNormal().magnitude();

	real c = ProjectAxis( L, S.getCenter() );

	return Range( c - d, c + d );
}

// ----------

Range ProjectAxis ( Line3d const & L, OrientedCircle const & S )
{
	Vector const & N = L.getNormal();

	Vector cross = S.getAxis().cross(N);

	float sine = cross.magnitude() / N.magnitude();

	real d = sine * S.getRadius();

	real c = ProjectAxis( L, S.getCenter() );

	return Range( c - d, c + d );
}

// ----------

Range ProjectAxis ( Line3d const & L, Segment3d const & C )
{
	real a = ProjectAxis(L,C.getBegin());
	real b = ProjectAxis(L,C.getEnd());

	if(a < b) std::swap(a,b);

	return Range(a,b);
}

// ----------

Range ProjectAxis ( Line3d const & L, OrientedCylinder const & C )
{
	Vector const & N = L.getNormal();

	Vector cross = C.getAxis().cross(N);

	float sine = cross.magnitude() / N.magnitude();

	real d = sine * C.getRadius();

	Range c = ProjectAxis( L, C.getAxisSegment() );
	
	return Range( c.getMin() - d, c.getMax() + d );
}

// ----------------------------------------------------------------------

Quadratic DistancePointPoint ( Vector const & A, Vector const & V, Vector const & B )
{
	Vector D = (B-A);

	real a = V.magnitudeSquared();
	real b = -2.0f * D.dot(V);
	real c = D.magnitudeSquared();

	return Quadratic(a,b,c);
}

// ----------------------------------------------------------------------

}	// namespace Collision3d
