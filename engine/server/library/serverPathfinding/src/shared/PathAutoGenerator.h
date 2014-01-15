//======================================================================
//
// PathAutoGenerator.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PathAutoGenerator_H
#define INCLUDED_PathAutoGenerator_H

//======================================================================

class Region;
class Vector;

//----------------------------------------------------------------------

class PathAutoGenerator
{
public:
	static Region const * findPathRegion(Vector const & pos_w);
	static void pathAutoGenerate(Vector const & pos_w, float nodeDistance, float obstacleDistance, Unicode::String & result);
	static void pathAutoCleanup(Vector const & pos_w, Unicode::String & result);
};

//======================================================================

#endif
