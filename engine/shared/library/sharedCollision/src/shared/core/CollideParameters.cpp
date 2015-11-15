// ======================================================================
//
// CollideParameters.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CollideParameters.h"

// ======================================================================

CollideParameters const CollideParameters::cms_default;
CollideParameters const CollideParameters::cms_toolPickDefault(CollideParameters::Q_high, CollideParameters::T_collidable, true);

// ======================================================================

CollideParameters::CollideParameters() :
	m_quality(Q_high),
	m_type(T_collidable),
	m_isToolPickTest(false)
{
}

// ----------------------------------------------------------------------

CollideParameters::CollideParameters(CollideParameters::Quality q, CollideParameters::Type t, bool isToolPickTest) :
	m_quality(q),
	m_type(t),
	m_isToolPickTest(isToolPickTest)
{
}

// ----------------------------------------------------------------------

CollideParameters::~CollideParameters()
{
}

// ----------------------------------------------------------------------

CollideParameters::Quality CollideParameters::getQuality() const
{
	return m_quality;
}

// ----------------------------------------------------------------------

void CollideParameters::setQuality(Quality const quality)
{
	m_quality = quality;
}

// ----------------------------------------------------------------------

CollideParameters::Type CollideParameters::getType() const
{
	return m_type;
}

// ----------------------------------------------------------------------

void CollideParameters::setType(Type const type)
{
	m_type = type;
}

// ----------------------------------------------------------------------

bool CollideParameters::getToolPickTest() const
{
	return m_isToolPickTest;
}

// ----------------------------------------------------------------------

void CollideParameters::setToolPickTest(bool isToolPickTest)
{
	m_isToolPickTest = isToolPickTest;
}

// ----------------------------------------------------------------------

bool CollideParameters::typeShouldCollide(bool const collidable, bool const opaqueSolid) const
{
	if (m_type == T_collidable && collidable)
		return true;

	if (m_type == T_opaqueSolid && opaqueSolid)
		return true;

	return false;
}

// ======================================================================
