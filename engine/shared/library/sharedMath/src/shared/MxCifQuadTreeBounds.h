// ======================================================================
//
// MxCifQuadTreeBounds.h
//
// Copyright 2002, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MxCifQuadTreeBounds_H
#define INCLUDED_MxCifQuadTreeBounds_H


//==============================================================================

/**
 * Base class used in MxCifQuadTree. Keeps track of the bounds of a 2-d geometric 
 * shape.
 */
class MxCifQuadTreeBounds
{
public:
	MxCifQuadTreeBounds(float minX, float minY, float maxX, float maxY, void * data = nullptr);
	virtual ~MxCifQuadTreeBounds(){};

	const float getMinX(void) const;
	const float getMinY(void) const;
	const float getMaxX(void) const;
	const float getMaxY(void) const;
	void *      getData(void) const;

	virtual bool isPointIn(float x, float y) const;

private:
	MxCifQuadTreeBounds();
	MxCifQuadTreeBounds(const MxCifQuadTreeBounds &);
	MxCifQuadTreeBounds & operator =(const MxCifQuadTreeBounds &);

private:
	const float m_minX, m_minY; // lower-left bound (--)
	const float m_maxX, m_maxY; // upper-right bound (++)
	void *      m_data;         // data associated with the area
};


//------------------------------------------------------------------------------

inline MxCifQuadTreeBounds::MxCifQuadTreeBounds(float minX, float minY, float maxX, 
	float maxY, void * data) :
	m_minX(minX),
	m_minY(minY),
	m_maxX(maxX),
	m_maxY(maxY),
	m_data(data)
{
}

inline const float MxCifQuadTreeBounds::getMinX(void) const 
{ 
	return m_minX; 
}

inline const float MxCifQuadTreeBounds::getMinY(void) const 
{ 
	return m_minY; 
}

inline const float MxCifQuadTreeBounds::getMaxX(void) const 
{ 
	return m_maxX; 
}

inline const float MxCifQuadTreeBounds::getMaxY(void) const 
{ 
	return m_maxY; 
}

inline void * MxCifQuadTreeBounds::getData(void) const
{
	return m_data;
}

//==============================================================================

/**
 * A circular area for use in a MxCifQuadTree.
 */
class MxCifQuadTreeCircleBounds : public MxCifQuadTreeBounds
{
public:
	MxCifQuadTreeCircleBounds(float centerX, float centerY, float radius, void * data = nullptr);

	float getCenterX() const;
	float getCenterY() const;
	float getRadius() const;

	virtual bool isPointIn(float x, float y) const;

private:
	MxCifQuadTreeCircleBounds();
	MxCifQuadTreeCircleBounds(const MxCifQuadTreeCircleBounds &);
	MxCifQuadTreeCircleBounds & operator =(const MxCifQuadTreeCircleBounds &);

private:
	const float m_centerX, m_centerY;
	const float m_radius;
	const float m_radiusSquared;
};


//------------------------------------------------------------------------------

inline MxCifQuadTreeCircleBounds::MxCifQuadTreeCircleBounds(float centerX, 
	float centerY, float radius, void * data) :
	MxCifQuadTreeBounds(centerX - radius, centerY - radius, centerX + radius, 
		centerY + radius, data),
	m_centerX(centerX),
	m_centerY(centerY),
	m_radius(radius),
	m_radiusSquared(radius * radius)
{
}

inline float MxCifQuadTreeCircleBounds::getCenterX() const
{
	return m_centerX;
}

inline float MxCifQuadTreeCircleBounds::getCenterY() const
{
	return m_centerY;
}

inline float MxCifQuadTreeCircleBounds::getRadius() const
{
	return m_radius;
}


//==============================================================================


#endif	// INCLUDED_MxCifQuadTreeBounds_H
