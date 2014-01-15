// ======================================================================
//
// ResourceFractalData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/ResourceFractalData.h"

#include "Archive/AutoByteStream.h"

// ======================================================================

ResourceFractalData::ResourceFractalData() :
		m_scaleX(0.0f),
		m_scaleY(0.0f),
		m_bias(0.0f),
		m_gain(0.0f),
		m_combinationRule(0),
		m_frequency(0.0f),
		m_amplitude(0.0f),
		m_octaves(0)
{
}

// ======================================================================
