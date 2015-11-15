//===================================================================
//
// TerrainModificationHelper.h
// copyright 2001, sony online entertainment
//
//
//===================================================================

#ifndef INCLUDED_TerrainModificationHelper_H
#define INCLUDED_TerrainModificationHelper_H

//===================================================================

#include "TerrainGenerator.h"

//===================================================================

class TerrainModificationHelper
{
public:

	static void setPosition (TerrainGenerator::Layer* layer, const Vector2d& position);
	static void setRotation (TerrainGenerator::Layer* layer, float angle);
	static void setHeight (TerrainGenerator::Layer* layer, float height);
	static TerrainGenerator::Layer* importLayer (const char* filename);

private:

	TerrainModificationHelper ();
	~TerrainModificationHelper ();
	TerrainModificationHelper (const TerrainModificationHelper& rhs);
	TerrainModificationHelper& operator= (const TerrainModificationHelper& rhs);
};

//===================================================================

#endif
