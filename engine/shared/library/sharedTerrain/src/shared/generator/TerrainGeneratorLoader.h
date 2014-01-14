//===================================================================
//
// TerrainGeneratorLoader.h
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//--
//
// this class knows about all of the available terrain generator Layers, 
//	Actions, Boundaries, Filters and Affectors and how to initialize them
//
//===================================================================

#ifndef INCLUDED_TerrainGeneratorLoader_H
#define INCLUDED_TerrainGeneratorLoader_H

//===================================================================

#include "sharedTerrain/TerrainGenerator.h"

//===================================================================

class TerrainGeneratorLoader
{
private:

	TerrainGeneratorLoader ();
	~TerrainGeneratorLoader ();
	TerrainGeneratorLoader (const TerrainGeneratorLoader& rhs);
	TerrainGeneratorLoader& operator= (const TerrainGeneratorLoader& rhs);

public:

	//-- createLayer instantiates a layer
	static void loadLayerItem (Iff& iff, TerrainGenerator* terrainGenerator, TerrainGenerator::Layer* layer);
	
	//-- createBoundary instantiates a boundary
	static TerrainGenerator::Boundary* createBoundary (Iff& iff);

	//-- createFilter instantiates a filter
	static TerrainGenerator::Filter*   createFilter (Iff& iff, TerrainGenerator* terrainGenerator);

	//-- createAffector instantiates a affector
	static TerrainGenerator::Affector* createAffector (Iff& iff, TerrainGenerator* terrainGenerator);
};

//===================================================================

#endif
