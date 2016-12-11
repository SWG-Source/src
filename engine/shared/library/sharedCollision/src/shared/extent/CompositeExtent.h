// ======================================================================
//
// CompositeExtent.h
// Austin Appleby
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CompositeExtent_H
#define INCLUDED_CompositeExtent_H

// ======================================================================

#include "sharedCollision/Extent.h"

#include "sharedMath/AxialBox.h"

class Iff;

// ======================================================================

class CompositeExtent : public Extent
{
public:

	CompositeExtent( ExtentType const & type );
	virtual ~CompositeExtent();

	// ----------
	// Virtual Extent interface

	virtual void            load                ( Iff & iff );
	virtual void            write               ( Iff & iff ) const;

	virtual void            drawDebugShapes     ( DebugShapeRenderer * renderer ) const;

	virtual void            copy                ( BaseExtent const * source );
	virtual void            transform           ( BaseExtent const * parent, Transform const & tform, float scale );
	virtual AxialBox        getBoundingBox      ( void ) const;
	virtual Sphere          getBoundingSphere   ( void ) const;

	// ----------
	// CompositeExtent interface

	virtual int                 getExtentCount  ( void ) const;
	virtual BaseExtent *        getExtent       ( int whichExtent );
	virtual BaseExtent const *  getExtent       ( int whichExtent ) const;

	virtual void                attachExtent    ( BaseExtent * newExtent );

protected:

	void updateBounds ( void );
	void cloneChildren ( CompositeExtent * dest ) const;
	void load_0000 ( Iff & iff );

	// ----------

	typedef std::vector< BaseExtent * > ExtentVec;

	ExtentVec * m_extents;

	AxialBox m_box;

private:

	CompositeExtent(const CompositeExtent &);
	CompositeExtent &operator =(const CompositeExtent &);
};

// ======================================================================

#endif
