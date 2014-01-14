// ======================================================================
//
// SimpleExtent.h
// Austin Appleby
//
// copyright 2002 Sony Online Entertainment
//
// SimpleExtent is a very simple extent class that's nothing but a single
// MultiShape.
//
// ======================================================================

#ifndef INCLUDED_SimpleExtent_H
#define INCLUDED_SimpleExtent_H

#include "sharedCollision/BaseExtent.h"
#include "sharedMath/MultiShape.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"

class Iff;
class DebugShapeRenderer;

// ======================================================================

class SimpleExtent : public BaseExtent
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	SimpleExtent(void);
	SimpleExtent( MultiShape const & newShape );
	virtual ~SimpleExtent(void);

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
	virtual Vector          getCenter           ( void ) const;
	virtual float           getRadius           ( void ) const;

	// ----------

	MultiShape const &      getShape        ( void ) const;
	void                    setShape        ( MultiShape const & newShape );

protected:

	virtual bool realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const;

	void    load_0000   ( Iff & iff );
	void    loadShape   ( Iff & iff );
	void    writeShape  ( Iff & iff ) const;

	// ----------

	MultiShape m_shape;

private:

	SimpleExtent(const SimpleExtent &);
	SimpleExtent &operator =(const SimpleExtent &);
};

// ----------------------------------------------------------------------

inline MultiShape const & SimpleExtent::getShape ( void ) const
{
	return m_shape;
}

inline void SimpleExtent::setShape ( MultiShape const & newShape )
{
	m_shape = newShape;
}

// ======================================================================

#endif // #ifndef INCLUDED_SimpleExtent_H
