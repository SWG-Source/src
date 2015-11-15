// ======================================================================
//
// CylinderExtent.h
// Austin Appleby
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CylinderExtent_H
#define INCLUDED_CylinderExtent_H

// ======================================================================

#include "sharedCollision/Extent.h"
#include "sharedMath/Cylinder.h"

class Iff;

// ======================================================================

// CylinderExtent MUST be derived from Extent and not BaseExtent because
// ExtentList requires it.

class CylinderExtent : public Extent
{
public:

	CylinderExtent();
	CylinderExtent( Cylinder const & cylinder );
	virtual ~CylinderExtent();

	static void         install     ( void );
	static void         remove      ( void );

	static Extent *     build       ( Iff & iff );

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

	Cylinder const &    getCylinder ( void ) const;
	void                setCylinder ( Cylinder const & newCylinder );

	// ----------

	typedef Cylinder ShapeType;

	ShapeType const & getShape ( void ) const;

    // ----------

protected:

	virtual bool realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const;

	void loadCylinder ( Iff & iff );
	void writeCylinder ( Iff & iff ) const;

	void load_0000 ( Iff & iff );

private:

	CylinderExtent(const CylinderExtent &);
	CylinderExtent &operator =(const CylinderExtent &);

	// ----------

	Cylinder m_cylinder;
};

// ----------------------------------------------------------------------

inline CylinderExtent::ShapeType const & CylinderExtent::getShape ( void ) const
{
	return getCylinder();
}

// ======================================================================

#endif
