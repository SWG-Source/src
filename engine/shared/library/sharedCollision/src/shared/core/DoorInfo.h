// ======================================================================
//
// DoorInfo.h
// Copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DoorInfo_H
#define INCLUDED_DoorInfo_H

#include "sharedMath/Vector.h"

class IndexedTriangleList;

// ----------------------------------------------------------------------

class DoorInfo
{
public:

	const char *         m_frameAppearance;
	const char *         m_doorAppearance;
	const char *         m_doorAppearance2;

	bool                 m_doorFlip2;
	Vector               m_delta;
	float                m_openTime;
	float                m_closeTime;
	float                m_spring;
	float                m_smoothness;   
	float                m_triggerRadius;
	bool                 m_forceField;

	const char *         m_openBeginEffect;
	const char *         m_openEndEffect;
	const char *         m_closeBeginEffect;
	const char *         m_closeEndEffect;

	IndexedTriangleList const * m_portalGeometry;

	bool                 m_alwaysOpen;
};

// ======================================================================

#endif

