// ======================================================================
// 
// Formation.h
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_Formation_H
#define INCLUDED_Formation_H

#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

class Squad;
class Object;

// ----------------------------------------------------------------------
class Formation
{
public:

	enum Shape
	{
		S_invalid = 0,
		S_claw,
		S_wall,
		S_sphere,
		S_delta,
		S_broad,
		S_x,
		S_count
	};

	static Vector const getPosition_w(Transform const & transform_l2w, Vector const & position_l);

#ifdef _DEBUG
	static char const * const getShapeString(Shape const shape);
#endif // _DEBUG

public:

	Formation();

	void markDirty();
	bool isDirty() const;

	void setShape(Shape const shape);
	Shape getShape() const;

	void setSpacing(float const spacing);
	float getSpacing() const;

	void setTransform(Transform const & transform);
	Transform const & getTransform() const;

	void build(Squad & squad);

private:

	Vector getPosition_l(Squad const & squad, int const slotIndex) const;

	Shape m_shape;
	float m_spacing;
	Transform m_transform;
	bool m_dirty;

	// Disabled

	Formation(Formation const &);
	Formation & operator =(Formation const &);
};

// ======================================================================

#endif // INCLUDED_Formation_H
