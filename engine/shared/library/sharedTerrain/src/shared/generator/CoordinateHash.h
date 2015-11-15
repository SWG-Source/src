//===================================================================
//
// CoordinateHash.h
// bearhart 6-30-2005
//
// copyright 2005, Sony Online Entertainment
//
//--
//
// Class to hash terrain coordinate pairs into good random seeds.
//
//===================================================================

#ifndef INCLUDED_CoordinateHash_H
#define INCLUDED_CoordinateHash_H

//===================================================================

class CoordinateHash
{
public:
	static unsigned long hashTuple(float x, float z);
	static float makeFloat(unsigned long hash);
};

//===================================================================

#endif

