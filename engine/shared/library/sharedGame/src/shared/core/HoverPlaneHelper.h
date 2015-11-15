//======================================================================
//
// HoverPlaneHelper.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_HoverPlaneHelper_H
#define INCLUDED_HoverPlaneHelper_H

//======================================================================

class NetworkId;
class Plane;
class Object;
class Vector;

//----------------------------------------------------------------------

class HoverPlaneHelper
{
public:

	static bool findMinimumHoverPlane  (const Object & obj, float & roll, float & pitch, const Vector & lookAhead);
	static bool findMinimumHoverHeight (const Object & obj, float & height, const Vector & lookAhead);

};

//======================================================================

#endif
