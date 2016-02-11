//===================================================================
//
// LotManager.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/LotManager.h"

#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedObject/StructureFootprint.h"
#include "sharedObject/World.h"
#include "sharedTerrain/TerrainObject.h"

#include <algorithm>
#include <map>
#include <string>

//===================================================================
// LotManager::NoBuildEntry
//===================================================================

LotManager::NoBuildEntry::NoBuildEntry (Vector const & position_w, float radius) :
	m_position_w (position_w),
	m_radius (radius),
	m_spatialSubdivisionHandle (0)
{
}

// ----------------------------------------------------------------------

LotManager::NoBuildEntry::~NoBuildEntry ()
{
}

// ----------------------------------------------------------------------

Sphere const LotManager::NoBuildEntry::getSphere () const
{
	return Sphere (m_position_w, m_radius);
}

// ----------------------------------------------------------------------

SpatialSubdivisionHandle * LotManager::NoBuildEntry::getSpatialSubdivisionHandle ()
{
	return m_spatialSubdivisionHandle;
}

// ----------------------------------------------------------------------

void LotManager::NoBuildEntry::setSpatialSubdivisionHandle (SpatialSubdivisionHandle * const spatialSubdivisionHandle)
{
	m_spatialSubdivisionHandle = spatialSubdivisionHandle;
}

//===================================================================
// LotManagerNamespace
//===================================================================

namespace LotManagerNamespace
{
	bool ms_logEntries;
	bool ms_debugReport;

	void debugReport (void * context)
	{
		LotManager const * lotManager = reinterpret_cast<LotManager const *> (context);
		if (lotManager)
		{
			DEBUG_REPORT_PRINT (true, ("-- LotManager\n"));
			DEBUG_REPORT_PRINT (true, ("           noBuildEntries = %i\n", lotManager->getNumberOfNoBuildEntries ()));
			DEBUG_REPORT_PRINT (true, ("structureFootprintEntries = %i\n", lotManager->getNumberOfStructureFootprintEntries ()));
		}
	}
}

using namespace LotManagerNamespace;

//===================================================================
// PUBLIC LotManager
//===================================================================

LotManager::LotManager (const float mapWidthInMeters, const float chunkWidthInMeters) :
	m_mapWidthInMeters (mapWidthInMeters),
	m_chunkWidthInMeters (chunkWidthInMeters),
	m_width (static_cast<int> (mapWidthInMeters / chunkWidthInMeters)),
	m_width_2 (m_width / 2),
	m_noBuildEntryMap (new NoBuildEntryMap),
	m_structureFootprintEntryMap (new StructureFootprintEntryMap)
{
	DebugFlags::registerFlag (ms_logEntries, "SharedObject/LotManager", "logEntries");
	DebugFlags::registerFlag (ms_debugReport, "SharedObject/LotManager", "debugReport", debugReport, this);
}

//-------------------------------------------------------------------

LotManager::~LotManager ()
{
	DebugFlags::unregisterFlag (ms_logEntries);
	DebugFlags::unregisterFlag (ms_debugReport);

	DEBUG_WARNING (!m_noBuildEntryMap->empty (), ("LotManager::~LotManager: m_noBuildEntryMap is not empty"));
	std::for_each (m_noBuildEntryMap->begin (), m_noBuildEntryMap->end (), PointerDeleterPairSecond ());
	delete m_noBuildEntryMap;

	DEBUG_WARNING (!m_structureFootprintEntryMap->empty (), ("LotManager::~LotManager: m_structureFootprintEntryMap is not empty"));
	delete m_structureFootprintEntryMap;
}

//-------------------------------------------------------------------

void LotManager::addNoBuildEntry (Object const & object, float const noBuildRadius)
{
	NOT_NULL (m_noBuildEntryMap);

	Vector position_w = object.getPosition_w ();
	position_w.y = 0.f;

	NoBuildEntry * const noBuildEntry = new NoBuildEntry (position_w, noBuildRadius);
	noBuildEntry->setSpatialSubdivisionHandle (m_sphereTree.addObject (noBuildEntry));

	bool const result = m_noBuildEntryMap->insert (std::make_pair (&object, noBuildEntry)).second;
	UNREF (result);
	DEBUG_FATAL (!result, ("LotManager::addNoBuildEntry - entry already exists"));
	DEBUG_REPORT_LOG (ms_logEntries, ("LotManager: added no build entry for object %s [%s] at <%1.2f, %1.2f>\n", object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplateName () ? object.getObjectTemplateName () : "nullptr", position_w.x, position_w.z));
}

//-------------------------------------------------------------------

void LotManager::removeNoBuildEntry (const Object& object)
{
	NoBuildEntryMap::iterator iter = m_noBuildEntryMap->find (&object);
	NoBuildEntryMap::iterator end = m_noBuildEntryMap->end ();
	if (iter != end)
	{
		NoBuildEntry * const noBuildEntry = iter->second;

		m_sphereTree.removeObject (noBuildEntry->getSpatialSubdivisionHandle ());
		delete noBuildEntry;

		IGNORE_RETURN (m_noBuildEntryMap->erase (iter));
	}
	else
		DEBUG_FATAL (true, ("LotManager::removeNoBuildEntry - entry not found"));

	DEBUG_REPORT_LOG (ms_logEntries, ("LotManager: removed no build entry for object %s [%s]\n", object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplateName () ? object.getObjectTemplateName () : "nullptr"));
}

//-------------------------------------------------------------------

int LotManager::getNumberOfNoBuildEntries () const
{
	NOT_NULL (m_noBuildEntryMap);

	return static_cast<int> (m_noBuildEntryMap->size ());
}

//-------------------------------------------------------------------

void LotManager::addStructureFootprintEntry (const Object& object, const StructureFootprint& structureFootprint, const int x, const int z, const RotationType rotation)
{
	NOT_NULL (m_structureFootprintEntryMap);

	StructureFootprintEntry structureFootprintEntry;
	structureFootprintEntry.m_structureFootprint = &structureFootprint;
	structureFootprintEntry.m_x                  = x;
	structureFootprintEntry.m_z                  = z;
	structureFootprintEntry.m_rotation           = rotation;
	const bool result = m_structureFootprintEntryMap->insert (std::make_pair (&object, structureFootprintEntry)).second;
	UNREF (result);
	DEBUG_FATAL (!result, ("LotManager::addStructureFootprintEntry - entry already exists"));

	DEBUG_REPORT_LOG (ms_logEntries, ("LotManager: added structure footprint entry for object %s [%s] at <%1.2f, %1.2f>\n", object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplateName () ? object.getObjectTemplateName () : "nullptr", x * m_chunkWidthInMeters + m_chunkWidthInMeters * 0.5f, z * m_chunkWidthInMeters + m_chunkWidthInMeters * 0.5f));
}

//-------------------------------------------------------------------

void LotManager::removeStructureFootprintEntry (const Object& object)
{
	StructureFootprintEntryMap::iterator iter = m_structureFootprintEntryMap->find (&object);
	StructureFootprintEntryMap::iterator end = m_structureFootprintEntryMap->end ();
	if (iter != end)
		IGNORE_RETURN (m_structureFootprintEntryMap->erase (iter));
	else
		DEBUG_FATAL (true, ("LotManager::removeStructureFootprintEntry - entry not found"));

	DEBUG_REPORT_LOG (ms_logEntries, ("LotManager: removed structure footprint entry for object %s [%s]\n", object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplateName () ? object.getObjectTemplateName () : "nullptr"));
}

//-------------------------------------------------------------------

int LotManager::getNumberOfStructureFootprintEntries () const
{
	NOT_NULL (m_structureFootprintEntryMap);

	return static_cast<int> (m_structureFootprintEntryMap->size ());
}

//-------------------------------------------------------------------

bool LotManager::canPlace (const StructureFootprint* const structureFootprint, const int x, const int z, const RotationType rotation, float& height_w, bool const forceChunkCreation) const
{
	NOT_NULL (structureFootprint);

	BoxExtent boxExtent;
	boxExtent.setMin (Vector::maxXYZ);
	boxExtent.setMax (Vector::negativeMaxXYZ);

	switch (rotation)
	{
	default:
	case RT_0:
		{
			const int pivotX = structureFootprint->getPivotX ();
			const int pivotZ = structureFootprint->getPivotZ ();

			int sj = structureFootprint->getHeight () - 1;
			int j;
			for (j = 0; j < structureFootprint->getHeight (); ++j)
			{
				int si = 0;
				int i;
				for (i = 0; i < structureFootprint->getWidth (); ++i)
				{
					if (!canPlace (boxExtent, (x - pivotX) + i, (z - pivotZ) + j, structureFootprint, si, sj, forceChunkCreation))
						return false;

					++si;
				}

				--sj;
			}
		}
		break;

	case RT_90:
		{
			const int pivotX = structureFootprint->getPivotZ ();
			const int pivotZ = structureFootprint->getPivotX ();

			int sj = structureFootprint->getWidth () - 1;
			int j;
			for (j = 0; j < structureFootprint->getWidth (); ++j)
			{
				int si = structureFootprint->getHeight () - 1;
				int i;
				for (i = 0; i < structureFootprint->getHeight (); ++i)
				{
					if (!canPlace (boxExtent, (x - pivotX) + i, (z - (structureFootprint->getWidth () - 1 - pivotZ)) + j, structureFootprint, sj, si, forceChunkCreation))
						return false;

					--si;
				}

				--sj;
			}
		}
		break;

	case RT_180:
		{
			const int pivotX = structureFootprint->getPivotX ();
			const int pivotZ = (structureFootprint->getHeight () - 1) - structureFootprint->getPivotZ ();

			int sj = 0;
			int j;
			for (j = 0; j < structureFootprint->getHeight (); ++j)
			{
				int si = structureFootprint->getWidth () - 1;
				int i;
				for (i = 0; i < structureFootprint->getWidth (); ++i)
				{
					if (!canPlace (boxExtent, (x - (structureFootprint->getWidth () - 1 - pivotX)) + i, (z - pivotZ) + j, structureFootprint, si, sj, forceChunkCreation))
						return false;

					--si;
				}

				++sj;
			}
		}
		break;

	case RT_270:
		{
			const int pivotX = (structureFootprint->getHeight () - 1) - structureFootprint->getPivotZ ();
			const int pivotZ = structureFootprint->getPivotX ();

			int sj = 0;
			int j;
			for (j = 0; j < structureFootprint->getWidth (); ++j)
			{
				int si = 0;
				int i;
				for (i = 0; i < structureFootprint->getHeight (); ++i)
				{
					if (!canPlace (boxExtent, (x - pivotX) + i, (z - pivotZ) + j, structureFootprint, sj, si, forceChunkCreation))
						return false;

					++si;
				}

				++sj;
			}
		}
		break;
	}

	if (boxExtent.getHeight() < 0.0f)
	{
		WARNING(true, ("LotManager invalid boxExtent"));
		return false;
	}

	//-- if box extent is smaller than the tolerance, we're ok
	if (boxExtent.getHeight () > structureFootprint->getStructureReservationTolerance ())
		return false;

	height_w = boxExtent.getTop ();

	return true;
}

//-------------------------------------------------------------------

bool LotManager::canPlace (Rectangle2d const & rectangle2d) const
{
	const int x0 = TerrainObject::getConstInstance ()->calculateChunkX (Vector (rectangle2d.x0, 0.f, rectangle2d.y0));
	const int z0 = TerrainObject::getConstInstance ()->calculateChunkZ (Vector (rectangle2d.x0, 0.f, rectangle2d.y0));
	const int x1 = TerrainObject::getConstInstance ()->calculateChunkX (Vector (rectangle2d.x1, 0.f, rectangle2d.y1));
	const int z1 = TerrainObject::getConstInstance ()->calculateChunkZ (Vector (rectangle2d.x1, 0.f, rectangle2d.y1));

	int x;
	int z;
	for (z = z0; z <= z1; ++z)
		for (x = x0; x <= x1; ++x)
			if (getLotType (x, z) == LT_illegal)
				return false;

	return true;
}

//-------------------------------------------------------------------

int LotManager::getWidth () const
{
	return m_width;
}

//-------------------------------------------------------------------

LotType LotManager::getLotType (const int x, const int z) const
{
	//-- all we want to know is if the lot is occupied
	if (x < -m_width_2 || x >= m_width_2 || z < -m_width_2 || z >= m_width_2)
		return LT_illegal;

	//-- go through the no build entries
	{
		const Vector position_w (x * m_chunkWidthInMeters + m_chunkWidthInMeters * 0.5f, 0.f, z * m_chunkWidthInMeters + m_chunkWidthInMeters * 0.5f);

		if (m_sphereTree.isWithin (position_w))
			return LT_illegal;
	}

	//-- go through the structure footprint entries
	{
		StructureFootprintEntryMap::iterator iter = m_structureFootprintEntryMap->begin ();
		StructureFootprintEntryMap::iterator end = m_structureFootprintEntryMap->end ();
		for (; iter != end; ++iter)
		{
			const StructureFootprintEntry& structureFootprintEntry = iter->second;

			switch (structureFootprintEntry.m_rotation)
			{
			default:
			case RT_0:
				{
					const int pivotX = structureFootprintEntry.m_structureFootprint->getPivotX ();
					const int pivotZ = structureFootprintEntry.m_structureFootprint->getPivotZ ();

					const int minX = structureFootprintEntry.m_x - pivotX;
					const int maxX = minX + structureFootprintEntry.m_structureFootprint->getWidth ();
					const int minZ = structureFootprintEntry.m_z - pivotZ;
					const int maxZ = minZ + structureFootprintEntry.m_structureFootprint->getHeight ();

					if (x >= minX && x < maxX && z >= minZ && z < maxZ)
					{
						const int sx = x - minX;
						const int sz = (structureFootprintEntry.m_structureFootprint->getHeight () - 1) - (z - minZ);

						const LotType lotType = structureFootprintEntry.m_structureFootprint->getLotType (sx, sz);

						if (lotType == LT_hard || lotType == LT_structure)
							return LT_illegal;
					}
				}
				break;

			case RT_90:
				{
					const int pivotX = structureFootprintEntry.m_structureFootprint->getPivotZ ();
					const int pivotZ = structureFootprintEntry.m_structureFootprint->getPivotX ();

					const int minX = structureFootprintEntry.m_x - pivotX;
					const int maxX = minX + structureFootprintEntry.m_structureFootprint->getHeight ();
					const int minZ = structureFootprintEntry.m_z - (structureFootprintEntry.m_structureFootprint->getWidth () - 1 - pivotZ);
					const int maxZ = minZ + structureFootprintEntry.m_structureFootprint->getWidth ();

					if (x >= minX && x < maxX && z >= minZ && z < maxZ)
					{
						const int sx = (structureFootprintEntry.m_structureFootprint->getHeight () - 1) - (x - minX);
						const int sz = (structureFootprintEntry.m_structureFootprint->getWidth () - 1) - (z - minZ);

						const LotType lotType = structureFootprintEntry.m_structureFootprint->getLotType (sz, sx);

						if (lotType == LT_hard || lotType == LT_structure)
							return LT_illegal;
					}
				}
				break;

			case RT_180:
				{
					const int pivotX = structureFootprintEntry.m_structureFootprint->getPivotX ();
					const int pivotZ = (structureFootprintEntry.m_structureFootprint->getHeight () - 1) - structureFootprintEntry.m_structureFootprint->getPivotZ ();

					const int minX = structureFootprintEntry.m_x - (structureFootprintEntry.m_structureFootprint->getWidth () - 1 - pivotX);
					const int maxX = minX + structureFootprintEntry.m_structureFootprint->getWidth ();
					const int minZ = structureFootprintEntry.m_z - pivotZ;
					const int maxZ = minZ + structureFootprintEntry.m_structureFootprint->getHeight ();

					if (x >= minX && x < maxX && z >= minZ && z < maxZ)
					{
						const int sx = (structureFootprintEntry.m_structureFootprint->getWidth () - 1) - (x - minX);
						const int sz = z - minZ;

						const LotType lotType = structureFootprintEntry.m_structureFootprint->getLotType (sx, sz);

						if (lotType == LT_hard || lotType == LT_structure)
							return LT_illegal;
					}
				}
				break;

			case RT_270:
				{
					const int pivotX = (structureFootprintEntry.m_structureFootprint->getHeight () - 1) - structureFootprintEntry.m_structureFootprint->getPivotZ ();
					const int pivotZ = structureFootprintEntry.m_structureFootprint->getPivotX ();

					const int minX = structureFootprintEntry.m_x - pivotX;
					const int maxX = minX + structureFootprintEntry.m_structureFootprint->getHeight ();
					const int minZ = structureFootprintEntry.m_z - pivotZ;
					const int maxZ = minZ + structureFootprintEntry.m_structureFootprint->getWidth ();

					if (x >= minX && x < maxX && z >= minZ && z < maxZ)
					{
						const int sx = x - minX;
						const int sz = z - minZ;

						const LotType lotType = structureFootprintEntry.m_structureFootprint->getLotType (sz, sx);

						if (lotType == LT_hard || lotType == LT_structure)
							return LT_illegal;
					}
				}
				break;
			}
		}
	}

	return LT_nothing;
}

//===================================================================
// PRIVATE LotManager
//===================================================================

bool LotManager::canPlace (BoxExtent& boxExtent, const int x, const int z, const StructureFootprint* const structureFootprint, const int structureX, const int structureZ, bool const forceChunkCreation) const
{
	const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING (true, ("LotManager::canPlace - no terrain"));
		return false;
	}

	//-- verify not in water
	if (terrainObject->getWater (x, z))
		return false;

	//-- check lot types and grow extent
	const LotType sourceLotType      = structureFootprint->getLotType (structureX, structureZ);
	const LotType destinationLotType = getLotType (x, z);
	const Vector position_w (x * m_chunkWidthInMeters + m_chunkWidthInMeters * 0.495f, 0.f, z * m_chunkWidthInMeters + m_chunkWidthInMeters * 0.495f);
	const BoxExtent* const chunkExtent = forceChunkCreation ? terrainObject->getChunkExtentForceChunkCreation (position_w) : terrainObject->getChunkExtent (position_w);
	if (!chunkExtent)
	{
		DEBUG_WARNING (true, ("LotManager::canPlace - could not find extent for <%1.2f, %1.2f>\n", position_w.x, position_w.z));
		return false;
	}

	bool ok = false;
	bool growExtent = false;

	if (sourceLotType == LT_nothing)
		ok = true;

	if (sourceLotType == LT_hard && destinationLotType == LT_nothing)
	{
		ok = true;
		growExtent = true;
	}

	if (sourceLotType == LT_structure && destinationLotType == LT_nothing)
	{
		ok = true;
		growExtent = true;
	}

	bool const updateBoxExtent = structureFootprint->getBoxTestRect().isWithin(static_cast<float>(structureX), static_cast<float>(structureZ));
	if (updateBoxExtent && growExtent)
		boxExtent.grow (*chunkExtent);

	return ok;
}

//===================================================================

