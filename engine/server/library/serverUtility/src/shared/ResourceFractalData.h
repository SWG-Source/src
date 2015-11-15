// ======================================================================
//
// ResourceFractalData.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ResourceFractalData_H
#define INCLUDED_ResourceFractalData_H

// ======================================================================

/**
 * Fractal parameters for resource pools
 */
struct ResourceFractalData
{
	float m_scaleX;
	float m_scaleY;
	float m_bias;
	float m_gain;
	int m_combinationRule;
	float m_frequency;
	float m_amplitude;
	int m_octaves;

	ResourceFractalData();
};

// ======================================================================

#endif
