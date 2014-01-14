// ======================================================================
//
// Extent.h
// copyright 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef EXTENT_H
#define EXTENT_H

// ======================================================================

class VectorArgb;
class Iff;
class DebugShapeRenderer;

#include "sharedMath/Sphere.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/CollisionEnums.h"

// ======================================================================

class Extent : public BaseExtent
{
public:

	Extent(void);
	
	explicit Extent( ExtentType const & type );
	explicit Extent( Sphere const & newSphere );
	
	Extent( ExtentType const & type, Sphere const & sphere );
	Extent( Vector const & newSphereCenter, real newSphereRadius );

	virtual ~Extent(void);

	static Extent *create(Iff & iff);
	static void   install();
	static void   remove();

	// ----------

	int getReferenceCount ( void ) const;
	int incrementReference ( void ) const;
	int decrementReference ( void ) const;

	// ----------
	
   	Sphere const &          getSphere       ( void ) const;
	void                    setSphere       (const Sphere &sphere);

	bool                    testSphereOnly  (Vector const & vector) const;
	bool                    testSphereOnly  (Vector const & begin, Vector const & end) const;
	bool                    testSphereOnly  (Vector const & begin, Vector const & end, real *time) const;
	bool                    testSphereOnly  (Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time) const;

	// ----------

	typedef Sphere ShapeType;

	ShapeType  const &      getShape        ( void ) const;
	ExtentType const &      getType         ( void ) const;

	// ----------
	// Virtual Extent interface
	
	virtual void            load                ( Iff & iff );
	virtual void            write               ( Iff & iff ) const;

	virtual bool            contains            ( Vector const & vector ) const;
	virtual bool            contains            ( Vector const & begin, Vector const & end ) const;

	virtual Range rangedIntersect(Line3d const & line) const;
	virtual Range rangedIntersect(Ray3d const & ray) const;
	virtual Range rangedIntersect(Segment3d const & seg) const;

	virtual void            drawDebugShapes     ( DebugShapeRenderer * renderer ) const;

	virtual BaseExtent *    clone               ( void ) const;
	virtual void            copy                ( BaseExtent const * source );
	virtual void            transform           ( BaseExtent const * parent, Transform const & tform, float scale );
	virtual AxialBox        getBoundingBox      ( void ) const;
	virtual Sphere          getBoundingSphere   ( void ) const;

	// ----------

protected:

	virtual bool realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const;

	void loadSphere     ( Iff & iff );
	void loadSphere_old ( Iff & iff );

	void load_0000      ( Iff & iff );
	void load_0001      ( Iff & iff );

	void writeSphere    ( Iff & iff ) const;

	// ----------

	Sphere      m_sphere;

private:

	Extent(const Extent &);
	Extent &operator =(const Extent &);

	mutable int m_referenceCount;
};

// ======================================================================
/**
 * Set the enclosing sphere of this extent.
 */

inline void Extent::setSphere(const Sphere &newSphere)
{
	m_sphere = newSphere;
}

// ----------------------------------------------------------------------
/**
 * Get the enclosing sphere of this extent.
 * 
 * Allowing the sphere to be centered around a point other that the origin
 * may allow the radius to be significantly smaller.
 */

inline const Sphere &Extent::getSphere(void) const
{
	return m_sphere;
}

// ----------------------------------------------------------------------
/**
 * Test a point against the Extent's bounding sphere.
 * 
 * @param vector  Point to test inside the sphere
 * @return True if the point is inside the line segment, otherwise false
 */

inline bool Extent::testSphereOnly(Vector const & vector) const
{
	return m_sphere.contains(vector);
}

// ----------------------------------------------------------------------

inline bool Extent::testSphereOnly(Vector const & begin, Vector const & end) const
{
	real time;
	Vector * normal = 0;
	return testSphereOnly(begin, end, normal, &time);
}

// ----------------------------------------------------------------------

inline bool Extent::testSphereOnly(Vector const & begin, Vector const & end, real *time) const
{
	Vector * normal = 0;
	return testSphereOnly(begin, end, normal, time);
}

// ----------------------------------------------------------------------

inline Extent::ShapeType const & Extent::getShape ( void ) const 
{
	return getSphere();
}

// ======================================================================

#endif
