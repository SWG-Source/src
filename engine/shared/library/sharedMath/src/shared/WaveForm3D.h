// ============================================================================
//
// WaveForm3D.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_WaveForm3D_H
#define INCLUDED_WaveForm3D_H

//-------------------------------------------------------------------

#include "sharedMath/WaveForm.h"
#include "sharedMath/Vector.h"

//-------------------------------------------------------------------
//
// Simple wrapper class to the WaveForm class to manage points in 3D space
//
// Usage:
//
//    WaveForm3D waveForm;
//
//    // Initialize the data that the waveform can work with
//
//    waveForm.insert(0.0f, Vector(0.0f, 0.0f, 0.0f)); // time must be [0.0f .. 1.0f]
//    waveForm.insert(0.25f, Vector(0.0f, 0.0f, 5.0f));
//    waveForm.insert(0.5f, Vector(0.0f, 0.0f, 10.0f));
//    waveForm.insert(0.75f, Vector(0.0f, 0.0f, 15.0f));
//    waveForm.insert(1.0f, Vector(0.0f, 0.0f, 20.0f));
//
//    .
//    .
//    .
//
//    // at a later time extract the interpolated result
//
//    float const time = 0.2358f; //must be [0.0f .. 1.0f]
//    Vector pointAtTime;
//
//    waveForm.getValue(time, pointAtTime);
//
//-------------------------------------------------------------------

class WaveForm3D
{
public:

	WaveForm3D();
	~WaveForm3D();

	void insert(float const time, Vector const & controlPoint);
	void getValue(float const time, Vector & point, bool randomOrder = false);

private:
	WaveForm m_xWaveForm;
	WaveForm m_yWaveForm;
	WaveForm m_zWaveForm;

	WaveFormControlPointIter m_xWaveIterator;
	WaveFormControlPointIter m_yWaveIterator;
	WaveFormControlPointIter m_zWaveIterator;

	bool m_isDirty;
};

#endif
