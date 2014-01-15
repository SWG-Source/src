// ======================================================================
//
// Region3dMaster.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/Region3dMaster.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/Region3d.h"
#include "serverGame/RegionBox.h"
#include "serverGame/RegionSphere.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedMath/SphereTree.h"
#include "sharedUtility/DataTable.h"
#include "UnicodeUtils.h"

// ======================================================================

namespace Region3dMasterNamespace
{

	// ----------------------------------------------------------------------

	class RegionSphereExtentAccessor: public BaseSphereTreeAccessor<Region3d const *, RegionSphereExtentAccessor>
	{
	public:
		static Sphere const getExtent(Region3d const *region)
		{
			NOT_NULL(region);
			return region->getContainingSphere();
		}
		static char const *getDebugName(Region3d const *region)
		{
			return region->getName().c_str();
		}
	};

	class RegionFilter: public SpatialSubdivisionFilter<Region3d const *>
	{
	public:
		explicit RegionFilter(Vector const &searchPoint) :
			m_searchPoint(searchPoint)
		{
		}

		bool operator()(Region3d const * const &region) const
		{
			return region->containsPoint(m_searchPoint);
		}

	private:
		RegionFilter(); // disable

	private:
		Vector m_searchPoint;
	};

	// ----------------------------------------------------------------------

	bool s_installed;
	// s_nameRegionMap's key field is a pointer to the region name contained
	// within the Region3d.  Region3d objects are added and removed from the
	// map during their constructor / destructor, so we guarantee that the
	// key is always pointing to valid data while in the map.
	typedef std::map<std::string const *, Region3d *, LessPointerComparator> RegionNameMap;
	RegionNameMap s_nameRegionMap;
	SphereTree<Region3d const *, RegionSphereExtentAccessor> s_regionSphereTree;

	// ----------------------------------------------------------------------

	void clearRegions()
	{
		while (!s_nameRegionMap.empty())
			delete (*s_nameRegionMap.begin()).second;
	}

	// ----------------------------------------------------------------------

	std::string getRegionTableNameForScene(std::string const &sceneId)
	{
		static std::string prefix("datatables/region3d/");
		static std::string suffix(".iff");
		return prefix + sceneId + suffix;
	}

	// ----------------------------------------------------------------------

	void remove()
	{
		if (s_installed)
		{
			clearRegions();
			s_installed = false;
		}
	}

	// ----------------------------------------------------------------------

	void requireColumn(DataTable const &t, char const *columnName, char const *tableName)
	{
		FATAL(t.findColumnNumber(columnName) < 0, ("Region3dMaster: require column '%s' missing from datatable '%s'", columnName, tableName));
	}

	// ----------------------------------------------------------------------
}
using namespace Region3dMasterNamespace;

// ======================================================================

void Region3dMaster::install() // static
{
	if (!s_installed)
	{
		ExitChain::add(remove, "Region3dMaster");

		reloadRegionTable();
		s_installed = true;
	}
}

// ----------------------------------------------------------------------

void Region3dMaster::reloadRegionTable() // static
{
	clearRegions();

	std::string const &region3dTableName = getRegionTableNameForScene(ConfigServerGame::getSceneID());

	Iff iff;
	if (iff.open(region3dTableName.c_str(), true))
	{
		DataTable region3dTable;
		region3dTable.load(iff);

		REPORT_LOG(true, ("Region3dMaster: loading 3d region table '%s'\n", region3dTableName.c_str()));

		requireColumn(region3dTable, "regionName", region3dTableName.c_str());
		requireColumn(region3dTable, "x1", region3dTableName.c_str());
		requireColumn(region3dTable, "y1", region3dTableName.c_str());
		requireColumn(region3dTable, "z1", region3dTableName.c_str());
		requireColumn(region3dTable, "x2", region3dTableName.c_str());
		requireColumn(region3dTable, "y2", region3dTableName.c_str());
		requireColumn(region3dTable, "z2", region3dTableName.c_str());
		requireColumn(region3dTable, "radius", region3dTableName.c_str());

		for (int row = 0; row < region3dTable.getNumRows(); ++row)
		{
			// regionName  geometry x1 y1 z1 x2 y2 z2 radius
			// where regionName is a string id specifier, geometry is 0 for sphere, 1 for box,
			// boxes use extents x1 y1 z1 x2 y2 z2, with x1 < x2, y1 < y2, z1 < z2,
			// and spheres use x1 y1 z1 as the center, with radius radius
			std::string regionName(Unicode::getTrim(region3dTable.getStringValue("regionName", row)));
			int geometry = region3dTable.getIntValue("geometry", row);
			float x1 = region3dTable.getFloatValue("x1", row);
			float y1 = region3dTable.getFloatValue("y1", row); //lint !e578
			float z1 = region3dTable.getFloatValue("z1", row);

			// Build script params to include everything in the data table row.
			// The newed region takes ownership of the regionScriptData and is
			// responsible for deleting it.
			ScriptParams *regionScriptData = new ScriptParams;
			for (int column = 0; column < region3dTable.getNumColumns(); ++column)
			{
				switch (region3dTable.getDataTypeForColumn(column).getType())
				{
				case DataTableColumnType::DT_Int:
				case DataTableColumnType::DT_HashString:
				case DataTableColumnType::DT_Enum:
				case DataTableColumnType::DT_BitVector:
					regionScriptData->addParam(region3dTable.getIntValue(column, row), region3dTable.getColumnName(column), true);
					break;
				case DataTableColumnType::DT_Bool:
					regionScriptData->addParam(region3dTable.getIntValue(column, row) ? true : false, region3dTable.getColumnName(column), true);
					break;
				case DataTableColumnType::DT_Float:
					regionScriptData->addParam(region3dTable.getFloatValue(column, row), region3dTable.getColumnName(column), true);
					break;
				case DataTableColumnType::DT_String:
				case DataTableColumnType::DT_PackedObjVars:
					{
						std::string const &value = region3dTable.getStringValue(column, row);
						char *s = new char[value.size()+1];
						strcpy(s, value.c_str());
						regionScriptData->addParam(s, region3dTable.getColumnName(column), true);
					} //lint !e429 // regionScriptData takes ownership of s
					break;
				case DataTableColumnType::DT_Comment:
				case DataTableColumnType::DT_Unknown:
					break;
				}
			}

			switch (geometry)
			{
			case 0: // sphere
				{
					float radius = region3dTable.getFloatValue("radius", row);
					FATAL(radius <= 0.0f, ("Region3dMaster: Region sphere with bad radius '%s'", regionName.c_str()));
					IGNORE_RETURN(new RegionSphere(regionName, Vector(x1, y1, z1), radius, regionScriptData));
				}
				break;
			case 1: // box
				{
					float x2 = region3dTable.getFloatValue("x2", row);
					float y2 = region3dTable.getFloatValue("y2", row);
					float z2 = region3dTable.getFloatValue("z2", row);
					FATAL(x2 <= x1 || y2 <= y1 || z2 <= z1, ("Region3dMaster: Region box with inverted coordinates '%s'", regionName.c_str()));
					IGNORE_RETURN(new RegionBox(regionName, Vector(x1, y1, z1), Vector(x2, y2, z2), regionScriptData));
				}
				break;
			default:
				FATAL(true, ("Region3dMaster: Unknown geometry type %d for region '%s'", geometry, regionName.c_str()));
				break;
			}
		} //lint !e429 // either the new region takes ownership of regionScriptData or we've fataled
	}
}

	// ----------------------------------------------------------------------

Region3d const *Region3dMaster::getRegionByName(std::string const &regionName) // static
{
	RegionNameMap::const_iterator i = s_nameRegionMap.find(&regionName);
	if (i != s_nameRegionMap.end())
		return (*i).second;
	return 0;
}

// ----------------------------------------------------------------------

void Region3dMaster::getRegionsAtPoint(Vector const &point, std::vector<Region3d const *> &results) // static
{
	s_regionSphereTree.findInRange(point, 0.0f, RegionFilter(point), results);
}

// ----------------------------------------------------------------------

Region3d const *Region3dMaster::getSmallestRegionAtPoint(Vector const &point) // static
{
	std::vector<Region3d const *> results;
	getRegionsAtPoint(point, results);

	Region3d const *best = 0;
	for (std::vector<Region3d const *>::iterator i = results.begin(); i != results.end(); ++i)
		if (!best || (*i)->getVolume() < best->getVolume())
			best = *i;

	return best;
}

// ----------------------------------------------------------------------

void Region3dMaster::addRegion(Region3d &region) // static
{
	FATAL(s_nameRegionMap.find(&region.getName()) != s_nameRegionMap.end(), ("Region3dMaster: tried to add a region with a duplicate name '%s'", region.getName().c_str()));

	// add to the name map
	IGNORE_RETURN(s_nameRegionMap.insert(std::make_pair(&region.getName(), &region)));
	// add to the sphere tree
	region.setSpatialSubdivisionHandle(s_regionSphereTree.addObject(&region));
}

// ----------------------------------------------------------------------

void Region3dMaster::removeRegion(Region3d &region) // static
{
	// remove from the name map
	IGNORE_RETURN(s_nameRegionMap.erase(&region.getName()));

	// remove from the sphere tree
	s_regionSphereTree.removeObject(region.getSpatialSubdivisionHandle());
}

// ======================================================================

