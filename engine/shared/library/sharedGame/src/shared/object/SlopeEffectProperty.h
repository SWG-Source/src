// ======================================================================
//
// SlopeEffectProperty.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SlopeEffectProperty_H
#define INCLUDED_SlopeEffectProperty_H

// ======================================================================

#include "sharedObject/Property.h"
#include "sharedMath/Vector.h"
#include <vector>

// ======================================================================

/**
 * This property is used to keep track of any induced slope effects that a creature has.
 * A slope effect acts as an artificial "hill" that the creature is climbing.
 */
class SlopeEffectProperty : public Property
{
public:

	static PropertyId getClassPropertyId();

public:
	SlopeEffectProperty(Object& thisObject);
	~SlopeEffectProperty();

	const std::vector<Vector> & getNormals() const;
	void addNormal(const Vector & normal);

private:
	std::vector<Vector> m_normals;

private:
	SlopeEffectProperty();
	SlopeEffectProperty(const SlopeEffectProperty&);
	SlopeEffectProperty& operator= (const SlopeEffectProperty&);
};


// ======================================================================

inline const std::vector<Vector> & SlopeEffectProperty::getNormals() const
{
	return m_normals;
}


// ======================================================================


#endif

