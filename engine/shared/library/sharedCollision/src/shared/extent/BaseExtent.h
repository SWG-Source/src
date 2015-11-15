// ======================================================================
//
// BaseExtent.h
// Austin Appleby
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BaseExtent_H
#define INCLUDED_BaseExtent_H

#include "sharedCollision/CollisionEnums.h"

class Iff;
class Vector;
class DebugShapeRenderer;
class Transform;
class Sphere;
class Line3d;
class Ray3d;
class Segment3d;
class Range;
class AxialBox;

// ======================================================================

class BaseExtent
{
public:

	BaseExtent(void);
	BaseExtent( ExtentType extentType );
	virtual ~BaseExtent(void);

	// ----------
	// Virtual Extent interface

	virtual void            load                ( Iff & iff ) = 0;
	virtual void            write               ( Iff & iff ) const = 0;

	virtual bool            contains            ( Vector const & vector ) const = 0;
	virtual bool            contains            ( Vector const & begin, Vector const & end ) const = 0;

	bool intersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const;
	bool intersect(Vector const & begin, Vector const & end, real * time) const;
	bool intersect(Vector const & begin, Vector const & end) const;

	virtual Range rangedIntersect(Line3d const & line) const = 0;
	virtual Range rangedIntersect(Ray3d const & ray) const = 0;
	virtual Range rangedIntersect(Segment3d const & seg) const = 0;

	virtual void            drawDebugShapes     ( DebugShapeRenderer * renderer ) const = 0;

	virtual BaseExtent *    clone               ( void ) const = 0;
	virtual void            copy                ( BaseExtent const * source ) = 0;
	virtual void            transform           ( BaseExtent const * parent, Transform const & tform, float scale ) = 0;
	virtual AxialBox        getBoundingBox      ( void ) const = 0;
	virtual Sphere          getBoundingSphere   ( void ) const = 0;
	virtual Vector          getCenter           ( void ) const;
	virtual float           getRadius           ( void ) const; // not necessarily the same as the bounding sphere radius

	virtual bool            validate            ( void ) const;

	// ----------
	// Concrete Extent interface

	ExtentType const &      getType         ( void ) const;
protected:
	virtual bool realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const = 0;
private:

   	ExtentType  m_extentType;

    // ----------

	BaseExtent(const BaseExtent &);
	BaseExtent &operator =(const BaseExtent &);
};

// ----------------------------------------------------------------------

inline ExtentType const & BaseExtent::getType ( void ) const
{
    return m_extentType;
}

// ======================================================================

#endif // #ifndef INCLUDED_BaseExtent_H
