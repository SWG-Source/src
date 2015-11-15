// ======================================================================
//
// ComponentExtent.h
// Austin Appleby
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ComponentExtent_H
#define INCLUDED_ComponentExtent_H

// ======================================================================

#include "sharedCollision/CompositeExtent.h"

class Iff;

// ======================================================================

class ComponentExtent : public CompositeExtent
{
public:

	ComponentExtent();
	virtual ~ComponentExtent();

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

	virtual BaseExtent *    clone               ( void ) const;

protected:

	virtual bool realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const;

	void load_0000 ( Iff & iff );

private:

	ComponentExtent(const ComponentExtent &);
	ComponentExtent &operator =(const ComponentExtent &);
};

// ======================================================================

#endif
