// ======================================================================
//
// OrientedCylinderExtent.h
// Austin Appleby
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_OrientedCylinderExtent_H
#define INCLUDED_OrientedCylinderExtent_H

// ======================================================================

#include "sharedCollision/Extent.h"
#include "sharedMath/OrientedCylinder.h"

class Iff;

// ======================================================================

// OrientedCylinderExtent MUST be derived from Extent and not BaseExtent because
// ExtentList requires it.

class OrientedCylinderExtent : public Extent
{
public:

	OrientedCylinderExtent();
	OrientedCylinderExtent( OrientedCylinder const & cylinder );
	OrientedCylinderExtent( Cylinder const & cylinder, Transform const & tform );
	virtual ~OrientedCylinderExtent();

	static void         install     ( void );
	static void         remove      ( void );

	static Extent *     build       ( Iff & iff );

	// ----------
	// Virtual Extent interface
	
	virtual void            load                ( Iff & iff );
	virtual void            write               ( Iff & iff ) const;

	virtual bool            contains            ( Vector const & vector ) const;
	virtual bool            contains            ( Vector const & begin, Vector const & end ) const;

	virtual void            drawDebugShapes     ( DebugShapeRenderer * renderer ) const;

	virtual BaseExtent *    clone               ( void ) const;
	virtual void            copy                ( BaseExtent const * source );
	virtual void            transform           ( BaseExtent const * parent, Transform const & tform, float scale );
	virtual Sphere          getBoundingSphere   ( void ) const;

	// ----------

	OrientedCylinder const &    getCylinder ( void ) const;
	void                setCylinder ( OrientedCylinder const & newCylinder );

	// ----------

	typedef OrientedCylinder ShapeType;

	ShapeType const & getShape ( void ) const;

	// ----------

protected:

	void loadCylinder ( Iff & iff );
	void writeCylinder ( Iff & iff ) const;

	void load_0000 ( Iff & iff );

private:

	virtual bool realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const;

	OrientedCylinderExtent(const OrientedCylinderExtent &);
	OrientedCylinderExtent &operator =(const OrientedCylinderExtent &);

	// ----------

	OrientedCylinder m_cylinder;
};

// ----------------------------------------------------------------------

inline OrientedCylinderExtent::ShapeType const & OrientedCylinderExtent::getShape ( void ) const
{
	return getCylinder();
}

// ======================================================================

#endif
