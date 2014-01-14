// ============================================================================
//
// WaveForm3D.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/WaveForm3D.h"

//-----------------------------------------------------------------------------

WaveForm3D::WaveForm3D()
: m_xWaveForm(WaveForm())
, m_yWaveForm(WaveForm())
, m_zWaveForm(WaveForm())
, m_xWaveIterator(WaveFormControlPointIter())
, m_yWaveIterator(WaveFormControlPointIter())
, m_zWaveIterator(WaveFormControlPointIter())
, m_isDirty(true)
{
}

//-----------------------------------------------------------------------------

WaveForm3D::~WaveForm3D()
{
}

//-----------------------------------------------------------------------------

void WaveForm3D::insert(float const time, Vector const & controlPoint)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0.0f, time, 1.0f);

	m_xWaveForm.insert(WaveFormControlPoint(time, controlPoint.x));
	m_yWaveForm.insert(WaveFormControlPoint(time, controlPoint.y));
	m_zWaveForm.insert(WaveFormControlPoint(time, controlPoint.z));

	m_isDirty = true;
}

//-----------------------------------------------------------------------------

/**
 * Get the value at a paricular point in time
 * @param time the time
 * @param point (output) the point
 * @param randomOrder Set this to true if this function will be called with 
 * values for "time" that are not in increasing order
 */
void WaveForm3D::getValue(float const time, Vector & point, bool randomOrder)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0.0f, time, 1.0f);

	if (m_isDirty || randomOrder)
	{
		m_xWaveIterator.reset(m_xWaveForm.getIteratorBegin());
		m_yWaveIterator.reset(m_yWaveForm.getIteratorBegin());
		m_zWaveIterator.reset(m_zWaveForm.getIteratorBegin());
		m_isDirty = false;
	}

	float const x = m_xWaveForm.getValue(m_xWaveIterator, time);
	float const y = m_yWaveForm.getValue(m_yWaveIterator, time);
	float const z = m_zWaveForm.getValue(m_zWaveIterator, time);

	point.set(x, y, z);
}

// ============================================================================
