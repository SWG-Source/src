//==================================================================
//
// ServerProceduralTerrainAppearance_Cache.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//==================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/ServerProceduralTerrainAppearance_Cache.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Plane.h"
#include "sharedTerrain/ConfigSharedTerrain.h"

#include <algorithm>
#include <vector>

//==================================================================

namespace ServerProceduralTerrainAppearanceCacheNamespace
{
	typedef std::vector<ArrayList<Vector>*>           VertexListList;
	typedef std::vector<ArrayList<Plane>*>            PlaneListList;
	typedef std::vector<Array2d<ShaderGroup::Info>*>  ShaderGroupList;
	typedef std::vector<Array2d<FloraGroup::Info>*>   FloraGroupList;

	bool               ms_installed;
	uint               ms_preloadSize = 2048;
	VertexListList*    ms_vertexListList;
	PlaneListList*     ms_planeListList;
	ShaderGroupList*   ms_shaderMapList;
	FloraGroupList*    ms_floraMapList;

#ifdef _DEBUG
	bool               ms_debugReport;
#endif

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void debugDump ();

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ArrayList<Vector>* localCreateVertexList (const int size)
	{
		ArrayList<Vector>* const vertexList = new ArrayList<Vector>;
		vertexList->preallocate (size, true);

		return vertexList;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ArrayList<Plane>* localCreatePlaneList (const int size)
	{
		ArrayList<Plane>* const planeList = new ArrayList<Plane>;
		planeList->preallocate (size, true);

		return planeList;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Array2d<ShaderGroup::Info>* localCreateShaderMap (const int width, const int height)
	{
		Array2d<ShaderGroup::Info>* const map = new Array2d<ShaderGroup::Info>;
		map->allocate (width, height);

		return map;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Array2d<FloraGroup::Info>* localCreateFloraMap (const int width, const int height)
	{
		Array2d<FloraGroup::Info>* const map = new Array2d<FloraGroup::Info>;
		map->allocate (width, height);

		return map;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ServerProceduralTerrainAppearanceCacheNamespace;

//==================================================================

void ServerProceduralTerrainAppearance::Cache::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ServerProceduralTerrainAppearance::Cache::install\n"));

	DEBUG_FATAL (ms_installed, ("already installed"));
	ms_installed = true;

	ms_preloadSize = static_cast<uint> (ConfigSharedTerrain::getMaximumNumberOfChunksAllowed ());

	//-- can't precreate maps because we don't know what size they will be
	ms_vertexListList = new VertexListList;
	ms_planeListList  = new PlaneListList;
	ms_shaderMapList  = new ShaderGroupList;
	ms_floraMapList   = new FloraGroupList;

#ifdef _DEBUG
	DebugFlags::registerFlag (ServerProceduralTerrainAppearanceCacheNamespace::ms_debugReport, "SharedTerrain", "reportServerProceduralTerrainAppearanceCache", ServerProceduralTerrainAppearanceCacheNamespace::debugDump);
#endif
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::Cache::remove ()
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	ms_installed = false;

	//-- delete non-array lists
#define DELETE_SPECIFIC_LIST(a) { \
		DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportLogPrint (), ("Terrain Cache %sList = %i\n", #a, ms_ ## a ## List->size ())); \
		std::for_each (ms_ ## a ## List->begin (), ms_ ## a ## List->end (), PointerDeleter ()); \
		delete ms_ ## a ## List; \
		ms_ ## a ## List = 0; \
	}

	DELETE_SPECIFIC_LIST (vertexList);
	DELETE_SPECIFIC_LIST (planeList);
	DELETE_SPECIFIC_LIST (shaderMap);
	DELETE_SPECIFIC_LIST (floraMap);

#ifdef _DEBUG
	DebugFlags::unregisterFlag (ServerProceduralTerrainAppearanceCacheNamespace::ms_debugReport);
#endif
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::Cache::warm (const int vertexListSize, const int planeListSize, const int mapSize)
{
	NOT_NULL (ms_vertexListList);
	ms_vertexListList->reserve (ms_preloadSize);

	NOT_NULL (ms_planeListList);
	ms_planeListList->reserve (ms_preloadSize);

	NOT_NULL (ms_shaderMapList);
	ms_shaderMapList->reserve (ms_preloadSize);

	NOT_NULL (ms_floraMapList);
	ms_floraMapList->reserve (ms_preloadSize);

	uint i;
	for (i = 0; i < ms_preloadSize; ++i)
	{
		ms_vertexListList->push_back (localCreateVertexList (vertexListSize));
		ms_planeListList->push_back (localCreatePlaneList (planeListSize));
		ms_shaderMapList->push_back (localCreateShaderMap (mapSize, mapSize));
		ms_floraMapList->push_back (localCreateFloraMap (mapSize, mapSize));
	}
}

//-------------------------------------------------------------------

ArrayList<Vector>* ServerProceduralTerrainAppearance::Cache::createVertexList (const int size)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (ms_vertexListList);

	ArrayList<Vector>* vertexList = 0;

	//-- just take one off the list
	if (!ms_vertexListList->empty ())
	{
		vertexList = ms_vertexListList->back ();
		ms_vertexListList->pop_back ();
	}
	else
	{
		//-- create one
		vertexList = localCreateVertexList (size);
	}
	
	NOT_NULL (vertexList);

	return vertexList;
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::Cache::destroyVertexList (ArrayList<Vector>* const vertexList)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (ms_vertexListList);

	if (vertexList)
		ms_vertexListList->push_back (vertexList);
}

//-------------------------------------------------------------------

ArrayList<Plane>* ServerProceduralTerrainAppearance::Cache::createPlaneList (const int size)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (ms_planeListList);

	ArrayList<Plane>* planeList = 0;

	//-- just take one off the list
	if (!ms_planeListList->empty ())
	{
		planeList = ms_planeListList->back ();
		ms_planeListList->pop_back ();
	}
	else
	{
		//-- create one
		planeList = localCreatePlaneList (size);
	}
	
	NOT_NULL (planeList);

	return planeList;
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::Cache::destroyPlaneList (ArrayList<Plane>* const planeList)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (ms_planeListList);

	if (planeList)
		ms_planeListList->push_back (planeList);
}

//-------------------------------------------------------------------

Array2d<ShaderGroup::Info>* ServerProceduralTerrainAppearance::Cache::createShaderMap (int width, int height)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (ms_shaderMapList);

	Array2d<ShaderGroup::Info>* map = 0;

	//-- just take one off the list
	if (!ms_shaderMapList->empty ())
	{
		map = ms_shaderMapList->back ();
		ms_shaderMapList->pop_back ();
	}
	else
	{
		//-- create one
		map = localCreateShaderMap (width, height);
	}
	
	NOT_NULL (map);
	DEBUG_FATAL (width != map->getWidth () && height != map->getHeight (), ("ServerProceduralTerrainAppearance::Cache - cached map of wrong size"));

	return map;
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::Cache::destroyShaderMap (Array2d<ShaderGroup::Info>* map)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (ms_shaderMapList);

	if (map)
		ms_shaderMapList->push_back (map);
}

//-------------------------------------------------------------------

Array2d<FloraGroup::Info>* ServerProceduralTerrainAppearance::Cache::createFloraMap (int width, int height)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (ms_floraMapList);

	Array2d<FloraGroup::Info>* map = 0;

	//-- just take one off the list
	if (!ms_floraMapList->empty ())
	{
		map = ms_floraMapList->back ();
		ms_floraMapList->pop_back ();
	}
	else
	{
		//-- create one
		map = localCreateFloraMap (width, height);
	}
	
	NOT_NULL (map);
	DEBUG_FATAL (width != map->getWidth () && height != map->getHeight (), ("ServerProceduralTerrainAppearance::Cache - cached map of wrong size"));

	return map;
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::Cache::destroyFloraMap (Array2d<FloraGroup::Info>* map)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (ms_floraMapList);

	if (map)
		ms_floraMapList->push_back (map);
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearanceCacheNamespace::debugDump ()
{
#ifdef _DEBUG

	DEBUG_FATAL (!ms_installed, ("not installed"));

	NOT_NULL (ms_vertexListList);
	NOT_NULL (ms_planeListList);
	NOT_NULL (ms_shaderMapList);
	NOT_NULL (ms_floraMapList);

	DEBUG_REPORT_PRINT (ms_debugReport, ("vertexListList size  = %i\n", ms_vertexListList->size ()));
	DEBUG_REPORT_PRINT (ms_debugReport, ("planeListList size  = %i\n",  ms_planeListList->size ()));
	DEBUG_REPORT_PRINT (ms_debugReport, ("shaderMapList size = %i\n",   ms_shaderMapList->size ()));
	DEBUG_REPORT_PRINT (ms_debugReport, ("floraMapList size = %i\n",    ms_floraMapList->size ()));

#endif
}

//==================================================================
