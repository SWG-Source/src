// ======================================================================
//
// Intersect3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Intersect3d.h"

#include "sharedCollision/Containment1d.h"
#include "sharedCollision/Intersect1d.h"
#include "sharedCollision/Intersect2d.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/Overlap3d.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedMath/Capsule.h"
#include "sharedMath/Ring.h"
#include "sharedMath/Triangle3d.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Ribbon3d.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/Quadratic.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/OrientedBox.h"
#include "sharedMath/OrientedCylinder.h"
#include "sharedMath/Plane.h"
#include "sharedMath/PolySolver.h"
#include "sharedMath/Torus.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/MultiShape.h"
#include "sharedMath/ShapeUtils.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Vector2d.h"

#include <algorithm>

namespace Intersect3dNamespace
{
	Vector const cs_boxPlaneNormals[6] =
	{
		Vector( 1.0f,  0.0f,  0.0f),
		Vector(-1.0f,  0.0f,  0.0f),
		Vector( 0.0f,  1.0f,  0.0f),
		Vector( 0.0f, -1.0f,  0.0f),
		Vector( 0.0f,  0.0f,  1.0f),
		Vector( 0.0f,  0.0f, -1.0f)
	};

	int const cs_edgeIndices[] =
	{
		0, 1,
		1, 3,
		3, 2,
		2, 0,
		4, 5,
		5, 7,
		7, 6,
		6, 4,
		0, 4,
		1, 5,
		3, 7,
		2, 6
	};
}

using namespace Intersect1d;
using namespace Intersect2d;
using namespace Intersect3dNamespace;

namespace Intersect3d
{

// ----------------------------------------------------------------------

bool	IntersectLinePlaneUnsided	( Line3d const & L,
									  Plane3d const & P,
									  Vector & outPoint,
									  real & outParam )
{
	real dot = L.getNormal().dot(P.getNormal());

	if(dot == 0) return false;

	real t = ((P.getPoint()-L.getPoint()).dot(P.getNormal())) / (L.getNormal().dot(P.getNormal()));

	outPoint = L.getPoint() + L.getNormal()*t;
	outParam = t;

	return true;
}

// ----------------------------------------------------------------------

bool	IntersectLineTriSided	( Line3d const & L,
								  Triangle3d const & T,
								  Vector & outPoint,
								  real & outParam )
{
	bool intersects = Overlap3d::TestLineTriSided( L, T );

	if(intersects)
	{
		return IntersectLinePlaneUnsided( L, T.getPlane(), outPoint, outParam );
	}
	else
	{
		return false;
	}
}

// ----------

bool	IntersectLineTriUnsided	( Line3d const & L,
								  Triangle3d const & T,
								  Vector & outPoint,
								  real & outParam )
{
	bool intersects = Overlap3d::TestLineTriUnsided( L, T );

	if(intersects)
	{
		return IntersectLinePlaneUnsided( L, T.getPlane(), outPoint, outParam );
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool	IntersectRayTriSided	( Ray3d const & R,
								  Triangle3d const & T,
								  Vector & outPoint,
								  real & outParam )
{
	if(Overlap3d::TestRayTriSided(R,T) == HR_Miss) return false;

	return IntersectLinePlaneUnsided( R.getLine(), T.getPlane(), outPoint, outParam);
}

// ----------

bool	IntersectRayTriUnsided  ( Ray3d const & R,
								  Triangle3d const & T,
								  Vector & outPoint,
								  real & outParam )
{
	if(Overlap3d::TestRayTriUnsided(R,T) == HR_Miss) return false;

	return IntersectLinePlaneUnsided( R.getLine(), T.getPlane(), outPoint, outParam);
}

// ----------------------------------------------------------------------

bool	IntersectSegPlaneUnsided( Segment3d const & S,
								  Plane3d const & P,
								  Vector & outPoint,
								  real & outParam )
{
	if(!Overlap3d::TestSegPlaneUnsided(S,P)) return false;

	return IntersectLinePlaneUnsided( S.getLine(), P, outPoint, outParam );
}

// ----------------------------------------------------------------------

bool	IntersectSegTriSided	( Segment3d const & S,
								  Triangle3d const & T,
								  Vector & outPoint,
								  real & outParam )
{
	if(!Overlap3d::TestSegTriSided(S,T)) return false;

	return IntersectSegPlaneUnsided(S,T.getPlane(),outPoint,outParam);
}

// ----------

bool	IntersectSegTriUnsided	( Segment3d const & S,
								  Triangle3d const & T,
								  Vector & outPoint,
								  real & outParam )
{
	if(!Overlap3d::TestSegTriUnsided(S,T)) return false;

	return IntersectSegPlaneUnsided(S,T.getPlane(),outPoint,outParam);
}

// ----------------------------------------------------------------------

bool IntersectSegRibbonSided ( Segment3d const & S, Ribbon3d const & R, float & outT, float & outU, float & outV )
{
	if(Overlap3d::TestSegRibbonSided(S,R))
	{
		Range temp = IntersectSegPlane(S,R.getPlane());

		outT = temp.getMin();

		Vector hitPoint = S.atParam(outT);

		// Compute the ribbon's parametric point of intersection

		Vector d = hitPoint - R.getPointA();

		Vector ribbonNormal = R.getDir().cross(R.getDelta());

		IGNORE_RETURN(ribbonNormal.normalize());

		Matrix3 matrix = Matrix3::fromColumns(R.getDir(),R.getDelta(),ribbonNormal);

		Matrix3 inverse;

		IGNORE_RETURN( matrix.invert(inverse) );

		outU = d.dot( inverse.row0() );
		outV = d.dot( inverse.row1() );

		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------
// Parametric linear-vs-planar

Range IntersectLinePlane ( Line3d const & line, Plane3d const & plane )
{
	return IntersectPointPlane( line.getPoint(), line.getNormal(), plane );
}

Range IntersectRayPlane ( Ray3d const & ray, Plane3d const & plane )
{
	return IntersectRanges ( IntersectLinePlane( ray.getLine(), plane ), Range::plusInf );
}

Range IntersectSegPlane ( Segment3d const & seg, Plane3d const & plane )
{
	return IntersectRanges( IntersectLinePlane(seg.getLine(),plane), Range::unit );
}

// ----------------------------------------------------------------------
// Parametric linear-vs-volume intersection tests

Range IntersectLineSphere ( Line3d const & L, Sphere const & S )
{
	Quadratic Q = Distance3d::DistancePointPoint(L.getPoint(), L.getNormal(), S.getCenter());

	float min;
	float max;

	float rad2 = S.getRadius() * S.getRadius();

	if(Q.solveFor(rad2,min,max))
	{
		return Range(min,max);
	}
	else
	{
		return Range::empty;
	}
}

// ----------

Range IntersectRaySphere ( Ray3d const & R, Sphere const & S )
{
	return IntersectRanges( IntersectLineSphere(R.getLine(),S), Range::plusInf );
}

// ----------

Range IntersectSegSphere ( Segment3d const & seg, Sphere const & S )
{
	return IntersectRanges( IntersectLineSphere(seg.getLine(),S), Range::unit );
}

// ----------------------------------------------------------------------

Range IntersectLineABox ( Line3d const & L, AxialBox const & box )
{
	return IntersectPointABox( L.getPoint(), L.getNormal(), box );
}

// ----------

Range IntersectRayABox ( Ray3d const & R, AxialBox const & box )
{
	Range hitRange = IntersectLineABox(R.getLine(),box);

	return IntersectRanges(hitRange,Range::plusInf);
}

// ----------

Range IntersectSegABox ( Segment3d const & S, AxialBox const & box )
{
	Range hitRange = IntersectLineABox(S.getLine(),box);

	return IntersectRanges(hitRange,Range::unit);
}

// ----------------------------------------------------------------------
// Line-torus intersection from Graphics Gems 2

int	inttor	(Vector const & Base, Vector const & Dcos, double radius,double rplane, double rnorm, int * nhits, double rhits[4])
{
	double	rho, a0, b0;		/* Related constants		*/
	double	f, l, t, g, q, m, u;	/* Ray dependent terms		*/
	double	C[5];			/* Quartic coefficients		*/

	*nhits  = 0;

/*	Compute constants related to the torus.				*/

	rho = rplane*rplane / (rnorm*rnorm);
	a0  = 4. * radius*radius;
	b0  = radius*radius - rplane*rplane;

/*	Compute ray dependent terms.					*/

	f = 1. - Dcos.y*Dcos.y;
	l = 2. * (Base.x*Dcos.x + Base.z*Dcos.z);
	t = Base.x*Base.x + Base.z*Base.z;
	g = f + rho * Dcos.y*Dcos.y;
	q = a0 / (g*g);
	m = (l + 2.*rho*Dcos.y*Base.y) / g;
	u = (t +    rho*Base.y*Base.y + b0) / g;

/*	Compute the coefficients of the quartic.			*/

	C[4] = 1.0;
	C[3] = 2. * m;
	C[2] = m*m + 2.*u - q*f;
	C[1] = 2.*m*u - q*l;
	C[0] = u*u - q*t;

/*	Use quartic root solver found in "Graphics Gems" by Jochen	*/
/*	Schwarze.							*/

	*nhits = PolySolver::solveQuartic (C,rhits);

/*	SolveQuartic returns root pairs in reversed order.		*/
	m = rhits[0]; u = rhits[1]; rhits[0] = u; rhits[1] = m;
	m = rhits[2]; u = rhits[3]; rhits[2] = u; rhits[3] = m;

	return (*nhits != 0);
}

Range IntersectLineTorus_local(Line3d const & L, Torus const & T)
{
	Vector P = L.getPoint();
	Vector N = L.getNormal();

	double nmag = N.magnitude();

	IGNORE_RETURN( N.normalize() );

	double radius = T.getMajorRadius();
	double rminor = T.getMinorRadius();

	int hitCount;
	double hits[4];

	IGNORE_RETURN( inttor(P,N,radius,rminor,rminor,&hitCount,hits) );

	if(hitCount == 0)
	{
		return Range::empty;
	}

	// ----------
	// Put a range around the roots

	double dmin = REAL_MAX;
	double dmax = -REAL_MAX;

	for(int i = 0; i < hitCount; i++)
	{
		if(hits[i] != hits[i]) continue;

		double hit = hits[i] / nmag;

		dmin = std::min(dmin, hit);
		dmax = std::max(dmax, hit);
	}

	return Range( static_cast<float>(dmin), static_cast<float>(dmax) );
}

Range IntersectLineTorus( Line3d const & L, Torus const & T )
{
	Vector P = T.transformToLocal(L.getPoint());
	Vector N = T.rotateToLocal(L.getNormal());

	Line3d localLine(P,N);

    Torus localTorus( Vector::zero, T.getMajorRadius(), T.getMinorRadius() );

	return IntersectLineTorus_local( localLine, localTorus );
}

// ----------------------------------------------------------------------

Range IntersectLineCylinder ( Line3d const & L, Cylinder const & C )
{
	Range slabRange = IntersectFloatRange( L.getPoint().y, L.getNormal().y, C.getRangeY() );

	Range tubeRange = IntersectPointCircle( L.getPoint(), L.getNormal(), C.getBaseCircle() );

	return IntersectRanges(slabRange,tubeRange);
}

Range IntersectRayCylinder ( Ray3d const & R, Cylinder const & C )
{
	Range lineRange = IntersectLineCylinder( R.getLine(), C );

	return IntersectRanges(lineRange,Range::plusInf);
}

Range IntersectSegCylinder ( Segment3d const & S, Cylinder const & C )
{
	Range lineRange = IntersectLineCylinder( S.getLine(), C );

	return IntersectRanges(lineRange,Range::unit);
}

// ----------------------------------------------------------------------

Range IntersectLineShape( Line3d const & L, MultiShape const & S )
{
	Line3d localLine = ShapeUtils::transform_p2l( L, S.getTransform_l2p() );
	MultiShape::BaseType type = S.getBaseType();

	if     (type == MultiShape::MSBT_Sphere)    { return Intersect( localLine, S.getLocalSphere() ); }
	else if(type == MultiShape::MSBT_Cylinder)  { return Intersect( localLine, S.getLocalCylinder() ); }
	else if(type == MultiShape::MSBT_Box)       { return Intersect( localLine, S.getLocalAxialBox() ); }
	else                                        { return Range::empty; }
}

Range IntersectRayShape ( Ray3d const & ray, MultiShape const & shape )
{
	Range lineRange = IntersectLineShape( ray.getLine(), shape );

	return IntersectRanges( lineRange, Range::plusInf );
}

Range IntersectSegShape ( Segment3d const & seg, MultiShape const & shape )
{
	Range lineRange = IntersectLineShape( seg.getLine(), shape );

	return IntersectRanges( lineRange, Range::unit );
}


Intersect3d::ResultData::ResultData()
: m_getSurfaceNormal(false)
, m_surfaceNormal()
, m_length(0.0f)
{}

Intersect3d::ResultData::ResultData(bool const getSurfaceNormal)
: m_getSurfaceNormal(getSurfaceNormal)
, m_surfaceNormal()
, m_length(0.0f)
{}

// From Real Time Rendering
bool intersectRaySphereWithData(Ray3d const & ray, Sphere const & sphere, ResultData * const data)
{
	Vector const l(sphere.getCenter() - ray.getPoint());
	float const lSquared = l.magnitudeSquared();
	float const rSquared = sqr(sphere.getRadius());

	if (lSquared <= rSquared)
	{
		if (data)
		{
			if (data->m_getSurfaceNormal)
			{
				Vector normal(ray.getPoint() - sphere.getCenter());
				if (!normal.normalize())
					normal = -ray.getNormal();

				data->m_surfaceNormal = normal;
			}

			data->m_length = 0.f;
		}

		return true;
	}

	// project l onto d
	float const d = l.dot(ray.getNormal());

	// ray behind the sphere
	if (d < 0.0f)
	{
		return(false);
	}

	// pythagorean: Squared distance from the center to the projection
	float const mSquared = lSquared - sqr(d);

	// check if missed completely
	if (mSquared > rSquared)
	{
		return(false);
	}

	float const q = sqrt(rSquared - mSquared);
	float const length = (lSquared < rSquared) ? (d + q) : (d - q); // inside or outside

	Vector const hitPoint(ray.getPoint() + (ray.getNormal() * length));

	if (data)
	{
		if (data->m_getSurfaceNormal)
		{
			Vector normal(hitPoint - sphere.getCenter());
			normal.normalize();
			data->m_surfaceNormal = normal;
		}

		data->m_length = length;
	}

	return(true);
}

// this is the Woo method that is found in Graphic Gems and realtime rendering
bool intersectRayABoxWithData(Ray3d const & ray, AxialBox const & box, ResultData * const data)
{
	Vector const max(box.getMax());
	Vector const min(box.getMin());

	float const rayOrigin[3] = { ray.getPoint().x, ray.getPoint().y, ray.getPoint().z };
	float const rayDirection[3] = { ray.getNormal().x, ray.getNormal().y, ray.getNormal().z };
	float const minBounds[3] = { min.x, min.y, min.z };
	float const maxBounds[3] = { max.x, max.y, max.z };
	bool middleQuadrant[3] = { false, false, false };
	float candidatePlane[3] = { 0.0f, 0.0f, 0.0f };
	bool inside = true;
	int i;

	// find candidate planes
	for (i = 0; i < 3; ++i)
	{
		if (rayOrigin[i] < minBounds[i])
		{
			// left quadrant
			candidatePlane[i] = minBounds[i];
			inside = false;
		}
		else if (rayOrigin[i] > maxBounds[i])
		{
			// right quadrant
			candidatePlane[i] = maxBounds[i];
			inside = false;
		}
		else
		{
			middleQuadrant[i] = true;
		}
	}

	// ray origin inside bounding box
	if (inside)
	{
		if (data != 0)
		{
			// find the normal from inside if we need to
			if (data->m_getSurfaceNormal)
			{
				float const planeD[6] =
							{
								Plane::computeD(cs_boxPlaneNormals[0], max),
								Plane::computeD(cs_boxPlaneNormals[1], min),
								Plane::computeD(cs_boxPlaneNormals[2], max),
								Plane::computeD(cs_boxPlaneNormals[3], min),
								Plane::computeD(cs_boxPlaneNormals[4], max),
								Plane::computeD(cs_boxPlaneNormals[5], min)
							};

				float minLength = REAL_MAX;
				int whichPlane = 0;
				// find the closest plane intersection that is facing away from us

				for(i = 0; i < 6; ++i)
				{
					float const dotRayPlaneNormal = ray.getNormal().dot(cs_boxPlaneNormals[i]);

					// only test the planes facing away from us
					if (dotRayPlaneNormal > 0.0f)
					{
						float const length = ((cs_boxPlaneNormals[i] * -1.0f).dot(ray.getPoint()) - planeD[i]) / dotRayPlaneNormal;

						if ((length > 0.0f) && (length < minLength))
						{
							whichPlane = i;
							minLength = length;
						}
					}
				}
				data->m_surfaceNormal = -cs_boxPlaneNormals[whichPlane];
			}

			data->m_length = 0.0f;
		}

		return true;
	}

	float maxT[3];

	// calculate distances to candidate planes
	for (i = 0; i < 3; i++)
	{
		if ((!middleQuadrant[i]) && (rayDirection[i] != 0.0f))
			maxT[i] = (candidatePlane[i] - rayOrigin[i]) / rayDirection[i];
		else
			maxT[i] = -1.0f;
	}

	// get largest of the maxT's for final choice of intersection
	int whichPlane = 0;
	for (i = 1; i < 3; i++)
	{
		if (maxT[whichPlane] < maxT[i])
		{
			whichPlane = i;
		}
	}

	// check final candidate actually inside box
	if (maxT[whichPlane] < 0.0f)
	{
		return false;
	}

	float hitPoint[3] = { 0.0f, 0.0f, 0.0f };

	for (i = 0; i < 3; i++)
	{
		if (whichPlane != i)
		{
			hitPoint[i] = rayOrigin[i] + maxT[whichPlane] * rayDirection[i];
			if (hitPoint[i] < minBounds[i] || hitPoint[i] > maxBounds[i])
			{
				return false;
			}
		}
		else
		{
			hitPoint[i] = candidatePlane[i];
		}
	}

	if (data != 0)
	{
		if (data->m_getSurfaceNormal)
		{
			float hitNormal[3] = { 0.0f, 0.0f, 0.0f };
			hitNormal[whichPlane] = 1.0f;

			Vector normal(hitNormal[0], hitNormal[1], hitNormal[2]);
			float const dotRayNormal = ray.getNormal().dot(normal);

			data->m_surfaceNormal = (dotRayNormal < 0.0f) ? normal : -normal;
		}

		data->m_length = (inside) ? 0.0f : maxT[whichPlane];
	}
	return true;
}


bool intersectRayOBoxWithData(Ray3d const & ray, OrientedBox const & box, ResultData * const data)
{
	Vector const localRayOrigin = box.transformToLocal(ray.getPoint());
	Vector const localRayNormal = box.rotateToLocal(ray.getNormal());
	Ray3d const localRay(localRayOrigin, localRayNormal);

	if (intersectRayABoxWithData(localRay, box.getLocalShape(), data))
	{
		if ((data) && (data->m_getSurfaceNormal))
		{
			data->m_surfaceNormal = box.rotateToWorld(data->m_surfaceNormal);
		}
		return(true);
	}
	return(false);
}

bool intersectRayYawedBoxWithData(Ray3d const & ray, YawedBox const & box, ResultData * const data)
{
	Vector const localRayOrigin = box.transformToLocal(ray.getPoint());
	Vector const localRayNormal = box.rotateToLocal(ray.getNormal());
	Ray3d const localRay(localRayOrigin, localRayNormal);

	if (intersectRayABoxWithData(localRay, box.getLocalShape(), data))
	{
		if ((data) && (data->m_getSurfaceNormal))
		{
			data->m_surfaceNormal = box.rotateToWorld(data->m_surfaceNormal);
		}

		return true;
	}

	return false;
}

bool intersectRayCylinderWithData(Ray3d const & ray, Cylinder const & cylinder, ResultData * const data)
{
	Range const capRange(Intersect1d::IntersectFloatRange(ray.getPoint().y, ray.getNormal().y, cylinder.getRangeY()));
	Range const tubeRange(Intersect2d::IntersectPointCircle(ray.getPoint(), ray.getNormal(), cylinder.getBaseCircle()));
	Range finalRange(Intersect1d::IntersectRanges(capRange, tubeRange));

	// clamp min because we are not concerned with intersections
	// that occur behind us
	finalRange.setMin(clamp(0.f, finalRange.getMin(), REAL_MAX));

	if (!finalRange.isEmpty())
	{
		if (data != 0)
		{
			float const length = finalRange.getMin();

			if (data->m_getSurfaceNormal)
			{
				Vector const hitPoint(ray.atParam(length));

				Vector const baseCenter(cylinder.getBase());
				Vector const topCenter(baseCenter.x, baseCenter.y + cylinder.getHeight(), baseCenter.z);

				Plane const baseCapPlane(-Vector::unitY, baseCenter);
				Plane const topCapPlane(Vector::unitY, topCenter);

				float const distanceToBase = baseCapPlane.computeDistanceTo(hitPoint);
				float const distanceToTop = topCapPlane.computeDistanceTo(hitPoint);

				if (WithinEpsilonInclusive(distanceToBase, 0.0f, 0.0001f))
				{
					data->m_surfaceNormal = -Vector::unitY;
				}

				else if (WithinEpsilonInclusive(distanceToTop, 0.0f, 0.0001f))
				{
					data->m_surfaceNormal = Vector::unitY;
				}

				else
				{
					Vector normal(hitPoint - cylinder.getBase());
					normal.y = 0.0f;
					normal.normalize();

					data->m_surfaceNormal = normal;
				}
			}

			data->m_length = length;
		}
		return true;
	}

	return false;
}

bool intersectRayOCylinderWithData(Ray3d const & ray, OrientedCylinder const & cylinder, ResultData * const data)
{
	Vector const localRayOrigin = cylinder.transformToLocal(ray.getPoint());
	Vector const localRayNormal = cylinder.rotateToLocal(ray.getNormal());
	Ray3d const localRay(localRayOrigin, localRayNormal);

	if (intersectRayCylinderWithData(localRay, cylinder.getLocalShape(), data))
	{
		if ((data) && (data->m_getSurfaceNormal))
		{
			data->m_surfaceNormal = cylinder.rotateToWorld(data->m_surfaceNormal);
		}
		return(true);
	}
	return(false);
}

bool intersectRayShapeWithData(Ray3d const & ray, MultiShape const & shape, ResultData * const data)
{
	MultiShape::ShapeType type = shape.getShapeType();

	switch (type)
	{
		case MultiShape::MST_Sphere:
			return intersectRaySphereWithData(ray, shape.getSphere(), data);

		case MultiShape::MST_AxialBox:
			return intersectRayABoxWithData(ray, shape.getAxialBox(), data);

		case MultiShape::MST_OrientedBox:
			return intersectRayOBoxWithData(ray, shape.getOrientedBox(), data);

		case MultiShape::MST_YawedBox:
			return intersectRayYawedBoxWithData(ray, shape.getYawedBox(), data);

		case MultiShape::MST_Cylinder:
			return intersectRayCylinderWithData(ray, shape.getCylinder(), data);

		case MultiShape::MST_OrientedCylinder:
			return intersectRayOCylinderWithData(ray, shape.getOrientedCylinder(), data);

		default:
			DEBUG_WARNING(true, ("Intersect3d::intersectRayShapeWithData: intersection with shape %i is not implemented", static_cast<int>(type)));
			break;
	}

	return false;
}

bool intersectTriangleBox(AxialBox const & box, Vector const & point1, Vector const & point2, Vector const & point3)
{
	//-- If a point is inside then return true
	if (box.contains(point1))
		return true;

	if (box.contains(point2))
		return true;

	if (box.contains(point3))
		return true;

	//-- Try to early out by testing sphere/plane
	//-- generate the normal first so we don't fatal if we hit 0 area triangles here
	Vector normal = (point1-point3).cross(point2-point1);
	if (!normal.normalize())
		return false;

	Plane const trianglePlane(normal, point1);
	Vector intersection;
	if (sqr(trianglePlane.computeDistanceTo(box.getCenter())) <= box.getRadiusSquared())
	{
		//-- Since the bounding sphere intersects the plane, now test each edge of the axial box with the triangle
		{
			for (int i = 0; i < 12; ++i)
			{
				int const edgeIndex0 = cs_edgeIndices[i * 2 + 0];
				int const edgeIndex1 = cs_edgeIndices[i * 2 + 1];
				if (trianglePlane.findIntersection(box.getCorner(edgeIndex0), box.getCorner(edgeIndex1), intersection))
				{
					if (intersection.inPolygon(point1, point2, point3))
						return true;
				}
			}
		}

		// we now need to test if the triangle edges intersect the box planes
		// and if the intersection point is then in the box
		{
			Vector const & max = box.getMax();
			Vector const & min = box.getMin();
			float const planeD[6] =
						{
							Plane::computeD(cs_boxPlaneNormals[0], max),
							Plane::computeD(cs_boxPlaneNormals[1], min),
							Plane::computeD(cs_boxPlaneNormals[2], max),
							Plane::computeD(cs_boxPlaneNormals[3], min),
							Plane::computeD(cs_boxPlaneNormals[4], max),
							Plane::computeD(cs_boxPlaneNormals[5], min)
						};

			for (int i = 0; i < 6; ++i)
			{
				Plane const facePlane(cs_boxPlaneNormals[i], planeD[i]);

				if (facePlane.findIntersection(point1, point2, intersection))
				{
					if (box.contains(intersection))
						return true;
				}

				if (facePlane.findIntersection(point2, point3, intersection))
				{
					if (box.contains(intersection))
						return true;
				}

				if (facePlane.findIntersection(point3, point1, intersection))
				{
					if (box.contains(intersection))
						return true;
				}
			}
		}
	}

	return false;
}


// ----------------------------------------------------------------------
// Motion tests

// Moving point-vs-planar

Range IntersectPointPlane( Vector const & A, Vector const & V, Plane3d const & P )
{
	float dist = (A - P.getPoint()).dot(P.getNormal());

	float velocity = V.dot(P.getNormal());

	if(velocity == 0.0f)
	{
		ContainmentResult result = Containment3d::TestPointPlane(A,P);

		if(result == CR_Boundary)
		{
			// Point is on the plane and is moving in the plane

			return Range::inf;
		}
		else
		{
			// Point is not on the plane and is moving parallel to the plane

			return Range::empty;
		}
	}
	else
	{
		float t = -dist / velocity;

		return Range(t,t);
	}
}


// ----------------------------------------------------------------------
// Moving point-vs-volume

Range IntersectPointABox ( Vector const & P, Vector const & V, AxialBox const & B )
{
	Range timeX = IntersectFloatRange( P.x, V.x, B.getRangeX() );
	Range timeY = IntersectFloatRange( P.y, V.y, B.getRangeY() );
	Range timeZ = IntersectFloatRange( P.z, V.z, B.getRangeZ() );

	return IntersectRanges(timeX,timeY,timeZ);
}

// ----------

Range IntersectPointSphere ( Vector const & P, Vector const & V, Sphere const & S )
{
	Quadratic Q = Distance3d::DistancePointPoint(P,V,S.getCenter());

	real rad2 = S.getRadius() * S.getRadius();

	float newMin;
    float newMax;

	if(Q.solveFor( rad2, newMin, newMax ))
	{
		return Range(newMin,newMax);
	}
	else
	{
		return Range::empty;
	}
}

// ----------------------------------------------------------------------
// Moving linear-vs-linear

Range IntersectLineLine ( Line3d const & A, Vector const & V, Line3d const & B )
{
	Vector D = B.getPoint() - A.getPoint();

	Vector N = A.getNormal().cross(B.getNormal());

	IGNORE_RETURN( N.normalize() );

	float dist = D.dot(N);

	float velocity = V.dot(N);

	if(velocity == 0)
	{
		return Range::empty;
	}

	float hitTime = dist / velocity;

	return Range(hitTime,hitTime);
}


Range IntersectLineSeg ( Line3d const & A, Vector const & V, Segment3d const & B )
{
	Range hitTime = IntersectLineLine(A,V,B.getLine());

	if(!hitTime.isEmpty())
	{
		float u;
		float v;

		Line3d A2(A.getPoint() + V * hitTime.getMin(), A.getNormal());

		Distance3d::ClosestPairLineLine( A2, B.getLine(), u, v );

		if(v < 0.0f) return Range::empty;
		if(v > 1.0f) return Range::empty;
	}

	return hitTime;
}

Range IntersectRaySeg ( Ray3d const & A, Vector const & V, Segment3d const & B )
{
	Range hitTime = IntersectLineLine(A.getLine(),V,B.getLine());

	if(!hitTime.isEmpty())
	{
		float u;
		float v;

		Line3d A2(A.getPoint() + V * hitTime.getMin(), A.getNormal());

		Distance3d::ClosestPairLineLine( A2, B.getLine(), u, v );

		if(u < 0.0f) return Range::empty;
		if(v < 0.0f) return Range::empty;
		if(v > 1.0f) return Range::empty;
	}

	return hitTime;
}


// ----------------------------------------------------------------------
// Moving linear-vs-planar

Range IntersectRayPlane ( Ray3d const & A, Vector const & V, Plane3d const & P )
{
	Range hitPointPlane = IntersectPointPlane( A.getPoint(), V, P );

	float hitTime = hitPointPlane.getMin();

	// velocity along the plane's normal
	float v = A.getNormal().dot(V);

	// relative ray direction with respect to the plane's normal
	float d = A.getNormal().dot(P.getNormal());

	if(v == 0)
	{
		// Ray origin isn't moving, or is moving parallel to the plane

		ContainmentResult result = Containment3d::TestPointPlane(A.getPoint(),P);

		if(d == 0)
		{
			// and ray is parallel to the plane

			if(result == CR_Outside)       return Range::empty;
			else if(result == CR_Boundary) return Range::inf;
			else                           return Range::empty;
		}
		else if(d < 0)
		{
			// ray is pointing towards the plane

			if(result == CR_Outside)       return Range::inf;
			else if(result == CR_Boundary) return Range::inf;
			else                           return Range::empty;
		}
		else
		{
			// ray is pointing away from the plane

			if(result == CR_Outside)       return Range::empty;
			else if(result == CR_Boundary) return Range::inf;
			else                           return Range::inf;
		}
	}
	else if(v < 0)
	{
		// Ray is moving towards the plane

		if(d == 0)      return hitPointPlane;
		else if( d < 0) return Range(-REAL_MAX,hitTime);
		else            return Range(hitTime,REAL_MAX);
	}
	else
	{
		// Ray is moving away from the plane

		if(d == 0)      return hitPointPlane;
		else if (d < 0) return Range(hitTime,REAL_MAX);
		else            return Range(-REAL_MAX,hitTime);
	}
}

// ----------

Range IntersectSegPlane ( Segment3d const & S, Vector const & V, Plane3d const & P )
{
	Line3d axis(P.getPoint(),P.getNormal());

	float vel = Collision2d::ComponentAlong(V,P.getNormal());

	Range R = Distance3d::ProjectAxis(axis,S);

	return IntersectFloatRange( 0.0f, -vel, R );
}

// ----------

Range IntersectLineTri ( Line3d const & A, Vector const & V, Triangle3d const & T )
{
	Range hit0 = IntersectLineSeg(A,V,T.getEdgeSegment0());
	Range hit1 = IntersectLineSeg(A,V,T.getEdgeSegment1());
	Range hit2 = IntersectLineSeg(A,V,T.getEdgeSegment2());

	return Containment1d::EncloseRanges(hit0,hit1,hit2);
}

// ----------

Range IntersectRayTri ( Ray3d const & A, Vector const & V, Triangle3d const & T )
{
	Range hitRayPlane = IntersectRayPlane(A,V,T.getPlane());

	Range hitLineTri = IntersectLineTri(A.getLine(),V,T);

	return IntersectRanges(hitRayPlane,hitLineTri);
}

// ----------

Range IntersectSegTri ( Segment3d const & A, Vector const & V, Triangle3d const & T )
{
	Range hitSegPlane = IntersectSegPlane(A,V,T.getPlane());

	Range hitLineTri = IntersectLineTri(A.getLine(),V,T);

	return IntersectRanges(hitSegPlane,hitLineTri);
}

// ----------------------------------------------------------------------
// Moving volume-vs-point

Range IntersectSpherePoint ( Sphere const & S, Vector const & V, Vector const & P )
{
	return IntersectPointSphere(P,-V,S);
}

// ----------------------------------------------------------------------
// Moving volume-vs-linear

Range IntersectSphereXLine ( Sphere const & S, Vector const & V, Vector const & B )
{
	Circle newCircle( Vector(S.getCenter().y,0.0f,S.getCenter().z), S.getRadius() );

	Vector newV(V.y,0.0f,V.z);

	Vector newB(B.y,0.0f,B.z);

	return Intersect2d::IntersectCirclePoint( newCircle, newV, newB );
}


Range IntersectSphereYLine ( Sphere const & S, Vector const & V, Vector const & B )
{
	return Intersect2d::IntersectCirclePoint( S.getCircle(), V, B);
}

Range IntersectSphereZLine ( Sphere const & S, Vector const & V, Vector const & B )
{
	Circle newCircle( Vector(S.getCenter().x,0.0f,S.getCenter().y), S.getRadius() );

	Vector newV(V.x,0.0f,V.y);

	Vector newB(B.x,0.0f,B.y);

	return Intersect2d::IntersectCirclePoint( newCircle, newV, newB );
}

// ----------------------------------------------------------------------
// Moving volume-vs-planar

Range IntersectSpherePlane( Sphere const & S, Vector const & V, Plane3d const & P )
{
	Line3d axis(P.getPoint(),P.getNormal());

	float vel = Collision2d::ComponentAlong(V,P.getNormal());

	Range R = Distance3d::ProjectAxis(axis,S);

	return IntersectRangeFloat( R, vel, 0.0f );
}

Range IntersectSphereRing ( Sphere const & S, Vector const & V, Ring const & R )
{
	Line3d L( S.getCenter(), V );

	Torus T( R.getCenter(), R.getRadius(), S.getRadius() );

	return IntersectLineTorus(L,T);
}


// ----------------------------------------------------------------------
// Moving volume-vs-volume

Range IntersectSphereSphere ( Sphere const & A, Vector const & velocity, Sphere const & B )
{
	Quadratic Q = Distance3d::DistancePointPoint(A.getCenter(),velocity,B.getCenter());

	real sumRadius = A.getRadius() + B.getRadius();

	real sumRad2 = sumRadius * sumRadius;

	float newMin;
    float newMax;

	if(Q.solveFor( sumRad2, newMin, newMax ))
	{
		return Range(newMin,newMax);
	}
	else
	{
		Capsule const capsule(A, velocity);
		if (capsule.intersectsSphere(B))
		{
			return Range::plusInf;
		}
	}

	return Range::empty;
}

// ----------
// Accessory methods for IntersectSphereABox

Range IntersectBoxEdgesX ( Sphere const & S, Vector const & V, AxialBox const & B )
{
	Vector C = B.getCenter();
	Vector D = B.getDelta();

	Range pp = IntersectSphereXLine( S, V, C + Vector(0.0f,  D.y,  D.z) );
	Range pn = IntersectSphereXLine( S, V, C + Vector(0.0f,  D.y, -D.z) );
	Range np = IntersectSphereXLine( S, V, C + Vector(0.0f, -D.y,  D.z) );
	Range nn = IntersectSphereXLine( S, V, C + Vector(0.0f, -D.y, -D.z) );

	return Containment1d::EncloseRanges(pp,pn,np,nn);
}

Range IntersectBoxEdgesY ( Sphere const & S, Vector const & V, AxialBox const & B )
{
	Vector C = B.getCenter();
	Vector D = B.getDelta();

	Range pp = IntersectSphereYLine( S, V, C + Vector( D.x, 0.0f,  D.z) );
	Range pn = IntersectSphereYLine( S, V, C + Vector( D.x, 0.0f, -D.z) );
	Range np = IntersectSphereYLine( S, V, C + Vector(-D.x, 0.0f,  D.z) );
	Range nn = IntersectSphereYLine( S, V, C + Vector(-D.x, 0.0f, -D.z) );

	return Containment1d::EncloseRanges(pp,pn,np,nn);
}

Range IntersectBoxEdgesZ ( Sphere const & S, Vector const & V, AxialBox const & B )
{
	Vector C = B.getCenter();
	Vector D = B.getDelta();

	Range pp = IntersectSphereZLine( S, V, C + Vector( D.x,  D.y, 0.0f) );
	Range pn = IntersectSphereZLine( S, V, C + Vector( D.x, -D.y, 0.0f) );
	Range np = IntersectSphereZLine( S, V, C + Vector(-D.x,  D.y, 0.0f) );
	Range nn = IntersectSphereZLine( S, V, C + Vector(-D.x, -D.y, 0.0f) );

	return Containment1d::EncloseRanges(pp,pn,np,nn);
}

// ----------

Range IntersectSphereABox ( Sphere const & S, Vector const & V, AxialBox const & B )
{
	if (V == Vector::zero)
	{
		Vector const & max = B.getMax();
		Vector const & min = B.getMin();
		float const planeD[6] =
					{
						Plane::computeD(cs_boxPlaneNormals[0], max),
						Plane::computeD(cs_boxPlaneNormals[1], min),
						Plane::computeD(cs_boxPlaneNormals[2], max),
						Plane::computeD(cs_boxPlaneNormals[3], min),
						Plane::computeD(cs_boxPlaneNormals[4], max),
						Plane::computeD(cs_boxPlaneNormals[5], min)
					};

		Vector const & sphereCenter = S.getCenter();
		float const sphereRadius = S.getRadius();

		for (int i = 0; i < 6; ++i)
		{
			Plane const plane(cs_boxPlaneNormals[i], planeD[i]);
			if (plane.computeDistanceTo(sphereCenter) > sphereRadius)
			{
				return Range::empty;
			}
		}

		return Range::plusInf;
	}

	Range result = Range::empty;

	// Intersect collision ranges for the box slabs

	Range slabX = B.getRangeX();
	Range slabY = B.getRangeY();
	Range slabZ = B.getRangeZ();

	Range slabHitX = Intersect1d::IntersectRangeRange(S.getRangeX(),V.x,slabX);
	Range slabHitY = Intersect1d::IntersectRangeRange(S.getRangeY(),V.y,slabY);
	Range slabHitZ = Intersect1d::IntersectRangeRange(S.getRangeZ(),V.z,slabZ);

	Range allSlabsHit = Intersect1d::IntersectRanges(slabHitX,slabHitY,slabHitZ);

	if(allSlabsHit.isEmpty())
	{
		return Range::empty;
	}

	// ----------
	// Compute the time ranges when the center of the sphere is inside the box slabs

	Vector C = S.getCenter();

	Range centerInSlabX = Intersect1d::IntersectFloatRange(C.x,V.x,slabX);
	Range centerInSlabY = Intersect1d::IntersectFloatRange(C.y,V.y,slabY);
	Range centerInSlabZ = Intersect1d::IntersectFloatRange(C.z,V.z,slabZ);

	// ----------
	// Face tests

	// If the sphere's center is inside two slabs when the sphere's bounding box
	// hits the test box, then that hit time is a contact time

	bool foundMin = false;
	bool foundMax = false;

	int minCounter = 0;
	int maxCounter = 0;

	if(centerInSlabX.contains(allSlabsHit.getMin())) minCounter++;
	if(centerInSlabY.contains(allSlabsHit.getMin())) minCounter++;
	if(centerInSlabZ.contains(allSlabsHit.getMin())) minCounter++;

	if(centerInSlabX.contains(allSlabsHit.getMax())) maxCounter++;
	if(centerInSlabY.contains(allSlabsHit.getMax())) maxCounter++;
	if(centerInSlabZ.contains(allSlabsHit.getMax())) maxCounter++;

	if((minCounter == 3) || (maxCounter == 3))
	{
		// The sphere and box will always and forevermore intersect
		// (which can happen if the sphere isn't moving)

		return Range::inf;
	}

	if(minCounter == 2)
	{
		result.setMin(allSlabsHit.getMin());
		foundMin = true;
	}

	if(maxCounter == 2)
	{
		result.setMax(allSlabsHit.getMax());
		foundMax = true;
	}

	if(foundMin && foundMax) return result;

	// ----------
	// Edge tests

	// The first edge hit that occurs while the center is inside the
	// corresponding slab is a contact time

	float minEdge = REAL_MAX;
	float maxEdge = -REAL_MAX;

	Range edgeRangeX = IntersectBoxEdgesX(S,V,B);
	Range edgeRangeY = IntersectBoxEdgesY(S,V,B);
	Range edgeRangeZ = IntersectBoxEdgesZ(S,V,B);

	if(!foundMin)
	{
		if(centerInSlabX.contains(edgeRangeX.getMin()))
		{
			if(edgeRangeX.getMin() < minEdge) minEdge = edgeRangeX.getMin();
		}

		if(centerInSlabY.contains(edgeRangeY.getMin()))
		{
			if(edgeRangeY.getMin() < minEdge) minEdge = edgeRangeY.getMin();
		}

		if(centerInSlabZ.contains(edgeRangeZ.getMin()))
		{
			if(edgeRangeZ.getMin() < minEdge) minEdge = edgeRangeZ.getMin();
		}
	}

	if(!foundMax)
	{
		if(centerInSlabX.contains(edgeRangeX.getMax()))
		{
			if(edgeRangeX.getMax() > maxEdge) maxEdge = edgeRangeX.getMax();
		}

		if(centerInSlabY.contains(edgeRangeY.getMax()))
		{
			if(edgeRangeY.getMax() > maxEdge) maxEdge = edgeRangeY.getMax();
		}

		if(centerInSlabZ.contains(edgeRangeZ.getMax()))
		{
			if(edgeRangeZ.getMax() > maxEdge) maxEdge = edgeRangeZ.getMax();
		}
	}

	if(minEdge != REAL_MAX)
	{
		result.setMin(minEdge);
		foundMin = true;
	}

	if(maxEdge != -REAL_MAX)
	{
		result.setMax(maxEdge);
		foundMax = true;
	}

	if(foundMin && foundMax) return result;

	// ----------
	// Corner tests

	Range cornerRange = Range::empty;

	for(int i = 0; i < 8; i++)
	{
		Range temp = IntersectSpherePoint(S,V,B.getCorner(i));

		cornerRange = Containment1d::EncloseRanges(cornerRange,temp);
	}

	if(!cornerRange.isEmpty())
	{
		if(!foundMin)
		{
			result.setMin(cornerRange.getMin());
			foundMin = true;
		}

		if(!foundMax)
		{
			result.setMax(cornerRange.getMax());
			foundMax = true;
		}
	}

	// ----------
	// Done

	if(foundMin ^ foundMax)
	{
		return Range::empty;
	}

	return result;
}

// ----------

Range IntersectSphereYBox ( Sphere const & S, Vector const & V, YawedBox const & B )
{
	Vector localCenter = B.transformToLocal(S.getCenter());

	Sphere localSphere(localCenter,S.getRadius());

	AxialBox localBox = B.getLocalShape();

	Vector localVelocity = B.rotateToLocal(V);

	return IntersectSphereABox( localSphere, localVelocity, localBox );
}

// ----------

Range IntersectSphereOBox ( Sphere const & S, Vector const & V, OrientedBox const & B )
{
	Vector localCenter = B.transformToLocal(S.getCenter());

	Sphere localSphere(localCenter,S.getRadius());

	AxialBox localBox = B.getLocalShape();

	Vector localVelocity = B.rotateToLocal(V);

	return IntersectSphereABox( localSphere, localVelocity, localBox );
}

// ----------
// IntersectSphereTorus is implemented as line-vs-expanded-torus

Range IntersectSphereTorus    ( Sphere const & S, Vector const & V, Torus const & T )
{
	Line3d L( S.getCenter(), V);

	Torus T2( T.getCenter(), T.getAxis(), T.getMajorRadius(), T.getMinorRadius() + S.getRadius() );

	return IntersectLineTorus( L, T2 );
}

// ----------

Range IntersectSphereCylinder ( Sphere const & S, Vector const & V, Cylinder const & C )
{
	if (C.getHeight() < 0.0f || C.getRadius() < 0.0f)
	{
		WARNING(true, ("Intersect3d::IntersectSphereCylinder invalid cylinder height or radius"));
		return Range::empty;
	}

	Range tubeRange = IntersectCircleCircle(S.getCircle(),V,C.getBaseCircle());
	Range slabRange = IntersectRangeRange(S.getRangeY(),V.y,C.getRangeY());

	Range volumeRange = IntersectRanges(tubeRange,slabRange);

	if(volumeRange.isEmpty())
	{
		return Range::empty;
	}

	Range ringRangeA = Intersect3d::IntersectSphereRing(S,V,C.getTopRing());
	Range ringRangeB = Intersect3d::IntersectSphereRing(S,V,C.getBaseRing());

	ringRangeA = Intersect3d::IntersectSphereRing(S,V,C.getTopRing());
	ringRangeB = Intersect3d::IntersectSphereRing(S,V,C.getBaseRing());

	Range boundaryRange = Containment1d::EncloseRanges(ringRangeA,ringRangeB);

	{
		Sphere temp( S.getCenter() + V * volumeRange.getMin(), S.getRadius() );

		if(Distance3d::DistanceSphereCylinder(temp,C) > 0.01f)
		{
			if(boundaryRange.isEmpty()) return Range::empty;

			volumeRange.setMin( boundaryRange.getMin() );
		}
	}

	{
		Sphere temp( S.getCenter() + V * volumeRange.getMax(), S.getRadius() );

		if(Distance3d::DistanceSphereCylinder(temp,C) > 0.01f)
		{
			if(boundaryRange.isEmpty()) return Range::empty;

			volumeRange.setMax( boundaryRange.getMax() );
		}
	}

	return volumeRange;
}

// ----------------------------------------------------------------------

Range IntersectSphereOCylinder(Sphere const & sphere, Vector const & V, OrientedCylinder const & cylinder)
{
	Vector const localCenter = cylinder.transformToLocal(sphere.getCenter());

	Sphere const localSphere(localCenter,sphere.getRadius());

	Cylinder const localCylinder = cylinder.getLocalShape();

	Vector const localVelocity = cylinder.rotateToLocal(V);

	return IntersectSphereCylinder(localSphere, localVelocity, localCylinder);
}

// ----------------------------------------------------------------------

Range IntersectABoxABox ( AxialBox const & A, Vector const & V, AxialBox const & B )
{
	Range timeX = IntersectRangeRange( A.getRangeX(), V.x, B.getRangeX() );
	Range timeY = IntersectRangeRange( A.getRangeY(), V.x, B.getRangeY() );
	Range timeZ = IntersectRangeRange( A.getRangeZ(), V.x, B.getRangeZ() );

	return IntersectRanges(timeX,timeY,timeZ);
}

// ----------------------------------------------------------------------

Range IntersectMultiShapes ( MultiShape const & A, Vector const & V, MultiShape const & B )
{
	MultiShape::ShapeType typeA = A.getShapeType();
	MultiShape::ShapeType typeB = B.getShapeType();

	// ----------

	if(typeA == MultiShape::MST_Sphere)
	{
		Sphere sphereA = A.getSphere();

		if     (typeB == MultiShape::MST_Sphere)           { return Intersect( sphereA, V, B.getSphere()   ); }
		else if(typeB == MultiShape::MST_Cylinder)         { return Intersect( sphereA, V, B.getCylinder() ); }
		else if(typeB == MultiShape::MST_OrientedCylinder) { return Intersect( sphereA, V, B.getOrientedCylinder() ); }
		else if(typeB == MultiShape::MST_AxialBox)         { return Intersect( sphereA, V, B.getAxialBox() ); }
		else if(typeB == MultiShape::MST_YawedBox)         { return Intersect( sphereA, V, B.getYawedBox() ); }
		else if(typeB == MultiShape::MST_OrientedBox)      { return Intersect( sphereA, V, B.getOrientedBox() ); }
		else                                               { return Range::empty; }
	}
	else if(typeB == MultiShape::MST_Sphere)
	{
		Sphere sphereB = B.getSphere();

		if     (typeB == MultiShape::MST_Sphere)           { return Intersect( sphereB, -V, A.getSphere() ); }
		else if(typeB == MultiShape::MST_Cylinder)         { return Intersect( sphereB, -V, A.getCylinder() ); }
		else if(typeB == MultiShape::MST_OrientedCylinder) { return Intersect( sphereB, -V, A.getOrientedCylinder() ); }
		else if(typeB == MultiShape::MST_AxialBox)         { return Intersect( sphereB, -V, A.getAxialBox() ); }
		else if(typeB == MultiShape::MST_YawedBox)         { return Intersect( sphereB, -V, A.getYawedBox() ); }
		else if(typeB == MultiShape::MST_OrientedBox)      { return Intersect( sphereB, -V, A.getOrientedBox() ); }
		else                                               { return Range::empty; }
	}
	else
	{
		return Range::empty;
	}
}

// ----------------------------------------------------------------------

Range IntersectCircleSeg_Below ( Circle const & circle, Vector const & inV, Segment3d const & S )
{
	DEBUG_FATAL(inV.y != 0.0f,("IntersectCircleSeg_Below - can't handle Y movement yet\n"));

	Vector V(inV.x,0.0f,inV.z);

	Vector A = S.getBegin();
	Vector B = S.getEnd();
	Vector const & C = circle.getCenter();

	if(A.y > B.y) std::swap(A,B);

	if(C.y > B.y)
	{
		return Range::empty;
	}
	else if(C.y < A.y)
	{
		return Intersect2d::IntersectCircleSeg(circle,V,S);
	}
	else
	{
		if(A.y == B.y)
		{
			return Intersect2d::IntersectCircleSeg(circle,V,S);
		}
		else
		{
			float param = (C.y - A.y) / (B.y - A.y);

			Vector D = A + (B - A) * param;

			return Intersect2d::IntersectCircleSeg(circle,V,Segment3d(D,B));
		}
	}
}

// ----------------------------------------------------------------------

}	// namespace Intersect3d

