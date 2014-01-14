// ======================================================================
//
// FloorContactShape.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FloorContactShape_H
#define INCLUDED_FloorContactShape_H

#include "sharedCollision/FloorLocator.h"
#include "sharedCollision/BaseClass.h"

class DebugShapeRenderer;

// ======================================================================

class FloorContactShape : public BaseClass
{
public:
	
	FloorLocator    m_contact;
	FloorLocator    m_result;

#ifdef _DEBUG
	FloorLocator    m_backupContact;
#endif

};

// ----------------------------------------------------------------------

#endif // #ifndef INCLUDED FloorContactShape_H

