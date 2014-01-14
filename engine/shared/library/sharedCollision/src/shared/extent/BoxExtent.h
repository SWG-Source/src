// ======================================================================
//
// BoxExtent.h
// jeff grills
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#ifndef BOX_EXTENT_H
#define BOX_EXTENT_H

// ======================================================================

#include "sharedCollision/Extent.h"
#include "sharedMath/AxialBox.h"

// ======================================================================

class BoxExtent : public Extent
{
public:

	BoxExtent(void);
    BoxExtent( AxialBox const & newBox );
	BoxExtent(const Vector &newMin, const Vector &newMax, const Sphere & s);

	virtual ~BoxExtent(void);

	static Extent *create(Iff &iff);  //lint !e1511 // hides non-virtual base member

	static void    install();
	static void    remove();

	static void   *operator new(size_t size);
	static void    operator delete(void *pointer);

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

	void           setMin (const Vector &vector);
	void           setMax (const Vector &vector);
	void           set (const Vector &newMin, const Vector &newMax, const Vector &newCenter, real newRadius);
	void           setNegative ();
	void           updateMinAndMax (const Vector &vector);
	void           calculateCenterAndRadius (void);

	Vector const & getMin (void) const;
	Vector const & getMax (void) const;

	real           getWidth  (void) const;  // x
	real           getHeight (void) const;  // y
	real           getLength (void) const;  // z

	void           grow (const BoxExtent & other);
	void           grow (const Vector & v, bool recompute = true);
	void           getCornerVectors (Vector * vs) const;

	real           getRight  ( void ) const;
	real           getTop    ( void ) const;
	real           getFront  ( void ) const;

	real           getLeft   ( void ) const;
	real           getBottom ( void ) const;
	real           getBack   ( void ) const;

	// ----------

	AxialBox const &  getBox      ( void ) const;
	void          setBox      ( AxialBox const & box );

	// ----------

	typedef AxialBox ShapeType;

	ShapeType const & getShape ( void ) const;

	// ----------
	
protected:

	virtual bool realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const;

	// ----------
	// Save/Load methods

	void          loadBox   ( Iff & iff );

	void          load_0000 ( Iff & iff );
	void          load_0001 ( Iff & iff );

	void          writeBox  ( Iff & iff ) const;

private:

	AxialBox m_box;

	BoxExtent(const BoxExtent &);
	BoxExtent &operator =(const BoxExtent &);
};

// ======================================================================

inline Vector const & BoxExtent::getMin (void) const
{
	return m_box.getMin();
}

inline Vector const & BoxExtent::getMax (void) const
{
	return m_box.getMax();
}

inline real BoxExtent::getWidth  (void) const
{
	return m_box.getWidth();
}

inline real BoxExtent::getHeight (void) const
{
	return m_box.getHeight();
}

inline real BoxExtent::getLength (void) const
{
	return m_box.getDepth();
}

// ----------------------------------------------------------------------
/**
 * Set the minimum extents of the box extent.
 */
inline void BoxExtent::setMin (const Vector &vector)
{
	m_box.setMin(vector);
}

// ----------------------------------------------------------------------
/**
 * Set the minimum extents of the box extent.
 */
inline void BoxExtent::setMax (const Vector &vector)
{
	m_box.setMax(vector);
}

// ----------------------------------------------------------------------
/**
 * Set all fields of the box extent.
 */
inline void BoxExtent::set (const Vector &newMin, const Vector &newMax, const Vector &newCenter, real newRadius)
{
	setMin(newMin);
	setMax(newMax);
	setSphere(Sphere(newCenter, newRadius));
}

/** ----------------------------------------------------------------------
* 
* Obtain an array of Vectors representing the 8 corners of the BoxExtent.
* Vector 0 is the min vector, Vector 7 is the max vector.
*
* @param vs a pointer to an array of 8 Vectors.  Vector 0 is the min vector, Vector 7 is the max vector.
*/

inline void BoxExtent::getCornerVectors (Vector *vs) const
{
	Vector const & min = m_box.getMin();
	Vector const & max = m_box.getMax();

	vs[0].x = min.x;
	vs[0].y = min.y;
	vs[0].z = min.z;

	vs[1].x = max.x;
	vs[1].y = min.y;
	vs[1].z = min.z;

	vs[2].x = max.x;
	vs[2].y = max.y;
	vs[2].z = min.z;

	vs[3].x = min.x;
	vs[3].y = max.y;
	vs[3].z = min.z;

	vs[4].x = max.x;
	vs[4].y = min.y;
	vs[4].z = max.z;

	vs[5].x = min.x;
	vs[5].y = min.y;
	vs[5].z = max.z;

	vs[6].x = min.x;
	vs[6].y = max.y;
	vs[6].z = max.z;

	vs[7].x = max.x;
	vs[7].y = max.y;
	vs[7].z = max.z;

}

// ----------------------------------------------------------------------

inline AxialBox const & BoxExtent::getBox ( void ) const
{
	return m_box;
}

// ----------------------------------------------------------------------

inline BoxExtent::ShapeType const & BoxExtent::getShape ( void ) const
{
	return getBox();
}

// ======================================================================
#endif
