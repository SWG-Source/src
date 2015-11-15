//==================================================================
//
// ServerProceduralTerrainAppearance_Cache.h
// asommers
//
// copyright 2002, sony online entertainment
//
//==================================================================

#ifndef INCLUDED_ServerProceduralTerrainAppearance_Cache_H
#define INCLUDED_ServerProceduralTerrainAppearance_Cache_H

//==================================================================

#include "sharedTerrain/ServerProceduralTerrainAppearance.h"

//==================================================================

class ServerProceduralTerrainAppearance::Cache
{
public:

	static void                        install ();
	static void                        remove ();

	static void                        warm (int vertexListSize, int planeListSize, int mapSize);

	static ArrayList<Vector>*          createVertexList (int size);
	static void                        destroyVertexList (ArrayList<Vector>* vertexList);

	static ArrayList<Plane>*           createPlaneList (int size);
	static void                        destroyPlaneList (ArrayList<Plane>* planeList);

	static Array2d<ShaderGroup::Info>* createShaderMap (int width, int height);
	static void                        destroyShaderMap (Array2d<ShaderGroup::Info>* map);

	static Array2d<FloraGroup::Info>*  createFloraMap (int width, int height);
	static void                        destroyFloraMap (Array2d<FloraGroup::Info>* map);

private:

	Cache ();
	~Cache ();
	Cache (const Cache&);
	Cache& operator= (const Cache&);
};

//==================================================================

#endif
