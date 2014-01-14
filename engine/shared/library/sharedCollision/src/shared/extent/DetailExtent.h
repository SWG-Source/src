// ======================================================================
//
// DetailExtent.h
// Austin Appleby
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DetailExtent_H
#define INCLUDED_DetailExtent_H

// ======================================================================

#include "sharedCollision/CompositeExtent.h"

// ======================================================================

class DetailExtent : public CompositeExtent
{
public:

	DetailExtent();
	virtual ~DetailExtent();

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

	virtual bool            validate            ( void ) const;

protected:

	virtual bool realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const;

	void updateBounds ( void );

	void load_0000 ( Iff & iff );

private:

	DetailExtent(const DetailExtent &);
	DetailExtent &operator =(const DetailExtent &);
};

// ======================================================================

#endif
