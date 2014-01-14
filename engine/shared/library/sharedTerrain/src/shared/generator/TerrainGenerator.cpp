//
// TerrainGenerator.cpp
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/TerrainGenerator.h"

#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/Iff.h"
#include "sharedMath/Vector2d.h"
#include "sharedTerrain/Feather.h"
#include "sharedTerrain/TerrainGeneratorLoader.h"
#include "sharedTerrain/Filter.h"

#include <algorithm>
#include <malloc.h>

#if defined(PLATFORM_LINUX)
#include <alloca.h>
#define _alloca alloca
#endif

//===================================================================
// TerrainGeneratorNamespace
//===================================================================

namespace TerrainGeneratorNamespace
{
	//-------------------------------------------------------------------

	// FuzzyOr (1, x) = 1
	template<class T>
	inline const T FuzzyOr (const T& a, const T& b)
	{
		return std::max (a, b);
	}

	//-------------------------------------------------------------------

	// FuzzyAnd (0, x) = 0
	template<class T>
	inline const T FuzzyAnd (const T& a, const T& b)
	{
		return std::min (a, b);
	}

	//-------------------------------------------------------------------
}

using namespace TerrainGeneratorNamespace;

//===================================================================
// STATIC PUBLIC TerrainGenerator
//===================================================================

//-------------------------------------------------------------------
//
// TerrainGenerator::CreateChunkBuffer
//

TerrainGenerator::CreateChunkBuffer::CreateChunkBuffer () :
	heightMap (),
	colorMap (),
	shaderMap (),
	floraStaticCollidableMap (),
	floraStaticNonCollidableMap (),
	floraDynamicNearMap (),
	floraDynamicFarMap (),
	environmentMap (),
	vertexPositionMap (),
	vertexNormalMap (),
	excludeMap (),
	passableMap ()
{
}

//-------------------------------------------------------------------

TerrainGenerator::CreateChunkBuffer::~CreateChunkBuffer ()
{
}

//-------------------------------------------------------------------

void TerrainGenerator::CreateChunkBuffer::allocate (const int poleCount)
{
	heightMap.allocate              (poleCount, poleCount);
	colorMap.allocate               (poleCount, poleCount);
	shaderMap.allocate              (poleCount, poleCount);
	floraStaticCollidableMap.allocate (poleCount, poleCount);
	floraStaticNonCollidableMap.allocate (poleCount, poleCount);
	floraDynamicNearMap.allocate    (poleCount, poleCount);
	floraDynamicFarMap.allocate      (poleCount, poleCount);
	environmentMap.allocate          (poleCount, poleCount);
	vertexPositionMap.allocate       (poleCount, poleCount);
	vertexNormalMap.allocate         (poleCount, poleCount);
	excludeMap.allocate              (poleCount, poleCount);
	passableMap.allocate             (poleCount, poleCount);
}

//-------------------------------------------------------------------

void TerrainGenerator::CreateChunkBuffer::validate () const
{
	DEBUG_FATAL (heightMap.isEmpty (),               ("heightMap has not been allocated"));
	DEBUG_FATAL (colorMap.isEmpty (),                ("colorMap has not been allocated"));
	DEBUG_FATAL (shaderMap.isEmpty (),               ("shaderMap has not been allocated"));
	DEBUG_FATAL (floraStaticCollidableMap.isEmpty (), ("floraStaticCollidableMap has not been allocated"));
	DEBUG_FATAL (floraStaticNonCollidableMap.isEmpty (), ("floraStaticNonCollidableMap has not been allocated"));
	DEBUG_FATAL (floraDynamicNearMap.isEmpty (),     ("floraDynamicNearMap has not been allocated"));
	DEBUG_FATAL (floraDynamicFarMap.isEmpty (),      ("floraDynamicFarMap has not been allocated"));
	DEBUG_FATAL (environmentMap.isEmpty (),          ("environmentMap has not been allocated"));
	DEBUG_FATAL (vertexPositionMap.isEmpty (),       ("vertexPositionMap has not been allocated"));
	DEBUG_FATAL (vertexNormalMap.isEmpty (),         ("vertexNormalMap has not been allocated"));
	DEBUG_FATAL (excludeMap.isEmpty (),              ("excludeMap has not been allocated"));
	DEBUG_FATAL (passableMap.isEmpty (),             ("passableMap has not been allocated"));
}

//-------------------------------------------------------------------
//
// TerrainGenerator::LayerItem
//
TerrainGenerator::LayerItem::LayerItem (const Tag tag) :
	m_tag (tag),
	m_active (true),
	m_pruned(false),
	m_name (0)
{
}

//-------------------------------------------------------------------

TerrainGenerator::LayerItem::~LayerItem ()
{
	delete [] m_name;
	m_name = 0;
}

//-------------------------------------------------------------------

void TerrainGenerator::LayerItem::setActive (const bool active)
{
	m_active = active;
}

//===================================================================

void TerrainGenerator::LayerItem::setName (const char* const name)
{
	delete [] m_name;
	m_name = 0;
	
	if (name)
		m_name = DuplicateString (name);
}

//-------------------------------------------------------------------

void TerrainGenerator::LayerItem::prepare ()
{
}

//-------------------------------------------------------------------

void TerrainGenerator::LayerItem::load (Iff& iff)
{
	iff.enterForm (TAG_IHDR);

	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	case TAG_0001:
		load_0001 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}

	iff.exitForm (TAG_IHDR);
}

//-------------------------------------------------------------------

void TerrainGenerator::LayerItem::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		iff.enterChunk (TAG_DATA);

			setActive (iff.read_int32 () != 0);
			
			char nameBuffer [100];
			iff.read_string (nameBuffer, 100);

			setName (nameBuffer);
	
			PackedRgb toolColorNowUnused;
			toolColorNowUnused.r = iff.read_uint8 ();
			toolColorNowUnused.g = iff.read_uint8 ();
			toolColorNowUnused.b = iff.read_uint8 ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void TerrainGenerator::LayerItem::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		iff.enterChunk (TAG_DATA);

			setActive (iff.read_int32 () != 0);

			char nameBuffer [100];
			iff.read_string (nameBuffer, 100);

			setName (nameBuffer);
	
		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void TerrainGenerator::LayerItem::save (Iff& iff) const
{
	iff.insertForm (TAG_IHDR);

		iff.insertForm (TAG_0001);

			iff.insertChunk (TAG_DATA);

				iff.insertChunkData (isActive () ? static_cast<int32> (1) : static_cast<int32> (0));
				iff.insertChunkString (getName () ? getName () : "");

			iff.exitChunk (TAG_DATA);
	
		iff.exitForm (TAG_0001);

	iff.exitForm (TAG_IHDR);
}

//-------------------------------------------------------------------
//
// TerrainGenerator::GeneratorChunkData
//
TerrainGenerator::GeneratorChunkData::GeneratorChunkData (bool legacyMode) :
	originOffset(0),
	numberOfPoles(0),
	upperPad(0),
	distanceBetweenPoles (0),
	start (),
	heightMap (0),
	colorMap (0),
	shaderMap (0),
	floraStaticCollidableMap (0),
	floraStaticNonCollidableMap (0),
	floraDynamicNearMap (0),
	floraDynamicFarMap (0),
	environmentMap (0),
	vertexPositionMap (0),
	vertexNormalMap (0),
	excludeMap (0),
	passableMap (0),
	shaderGroup (0),
	floraGroup (0),
	radialGroup (0),
	environmentGroup (0),
	fractalGroup (0),
	bitmapGroup (0),
	m_legacyRandomGenerator(legacyMode ? new RandomGenerator : (RandomGenerator *)0),
	normalsDirtyIUO (false),
	shadersDirtyIUO (false),
	chunkExtentIUO ()
{
}

//-------------------------------------------------------------------

TerrainGenerator::GeneratorChunkData::~GeneratorChunkData ()
{
	heightMap = 0;
	colorMap = 0;
	shaderMap = 0;
	floraStaticCollidableMap = 0;
	floraStaticNonCollidableMap = 0;
	floraDynamicNearMap = 0;
	floraDynamicFarMap = 0;
	environmentMap = 0;
	vertexPositionMap = 0;
	vertexNormalMap = 0;
	excludeMap = 0;
	passableMap = 0;
	shaderGroup = 0;
	floraGroup = 0;
	radialGroup = 0;
	environmentGroup = 0;
	fractalGroup = 0;
	bitmapGroup = 0;
	delete m_legacyRandomGenerator;
}

//-------------------------------------------------------------------

void TerrainGenerator::GeneratorChunkData::validate () const
{
	DEBUG_FATAL (numberOfPoles == 0, ("TerrainGenerator::GeneratorChunkData::validate - numberOfPoles == 0"));
	DEBUG_FATAL (distanceBetweenPoles == 0, ("TerrainGenerator::GeneratorChunkData::validate - distanceBetweenPoles == 0"));
	NOT_NULL (heightMap);
	NOT_NULL (colorMap);
	NOT_NULL (shaderMap);
	NOT_NULL (floraStaticCollidableMap);
	NOT_NULL (floraStaticNonCollidableMap);
	NOT_NULL (floraDynamicNearMap);
	NOT_NULL (floraDynamicFarMap);
	NOT_NULL (environmentMap);
	NOT_NULL (vertexPositionMap);
	NOT_NULL (vertexNormalMap);
	NOT_NULL (excludeMap);
	NOT_NULL (passableMap);
	NOT_NULL (shaderGroup);
	NOT_NULL (floraGroup);
	NOT_NULL (radialGroup);
	NOT_NULL (environmentGroup);
	NOT_NULL (fractalGroup);
	NOT_NULL (bitmapGroup);
}

//-------------------------------------------------------------------
//
// TerrainGenerator::Boundary
//
TerrainGenerator::Boundary::Boundary (const Tag tag, const TerrainGeneratorBoundaryType type) :
	TerrainGenerator::LayerItem (tag),
	m_type (type),
	m_featherFunction (TGFF_linear),
	m_featherDistance (0.f)
{
}

//-------------------------------------------------------------------

TerrainGenerator::Boundary::~Boundary ()
{
}

//-------------------------------------------------------------------

void TerrainGenerator::Boundary::rotate (const float /*angle*/)
{
}

//-------------------------------------------------------------------

void TerrainGenerator::Boundary::rotate (const float /*angle*/, const Vector2d& /*center*/)
{
}

//-------------------------------------------------------------------

void TerrainGenerator::Boundary::translate (const Vector2d& /*translation*/)
{
}

//-------------------------------------------------------------------

void TerrainGenerator::Boundary::scale (const float /*scalar*/)
{
}

//-------------------------------------------------------------------

const Vector2d TerrainGenerator::Boundary::getCenter () const
{
	return Vector2d ();
}

//-------------------------------------------------------------------

void TerrainGenerator::Boundary::setCenter (const Vector2d& /*center*/)
{
}

//-------------------------------------------------------------------

void TerrainGenerator::Boundary::setRotation (const float /*angle*/)
{
}

//-------------------------------------------------------------------

void TerrainGenerator::Boundary::scanConvertGT(float *o_data, const Rectangle2d &scanArea, int numberOfPoles) const
{
	if (!intersects(scanArea))
	{
		return;
	}

	const int sampleWidth = numberOfPoles-1;
	if (sampleWidth==0)
	{
		float amount = isWithin(scanArea.x0, scanArea.y0);
		const Feather feather(getFeatherFunction());
		o_data[0]=std::max(o_data[0], feather.feather(0.f, 1.f, amount));
		return;
	}

	const float scale = (scanArea.x1 - scanArea.x0) / float(sampleWidth);
	for (int z=0;z<numberOfPoles;z++)
	{
		float *const destRow = o_data + z*numberOfPoles;
		const float worldZ = scanArea.y0 + float(z)*scale;
		for (int x=0;x<numberOfPoles;x++)
		{
			const float worldX = scanArea.x0 + float(x)*scale;

			float amount = isWithin(worldX, worldZ);

			const Feather feather(getFeatherFunction());
			amount = feather.feather(0.f, 1.f, amount);
			if (amount>destRow[x])
			{
				destRow[x]=amount;
			}
		}
	}
}

//-------------------------------------------------------------------
//
// TerrainGenerator::Filter
//
TerrainGenerator::Filter::Filter (const Tag tag, const TerrainGeneratorFilterType type) :
	TerrainGenerator::LayerItem (tag),
	m_type (type),
	m_featherFunction (TGFF_linear),
	m_featherDistance (0.f)
{
}

//-------------------------------------------------------------------

TerrainGenerator::Filter::~Filter ()
{
}

//-------------------------------------------------------------------

bool TerrainGenerator::Filter::needsNormals () const
{
	return false;
}

//-------------------------------------------------------------------

bool TerrainGenerator::Filter::needsShaders () const
{
	return false;
}

//-------------------------------------------------------------------
//
// TerrainGenerator::Affector
//
TerrainGenerator::Affector::Affector (const Tag tag, const TerrainGeneratorAffectorType type) :
	TerrainGenerator::LayerItem (tag),
	m_type (type)
{
}

//-------------------------------------------------------------------

TerrainGenerator::Affector::~Affector ()
{
}

//-------------------------------------------------------------------

bool TerrainGenerator::Affector::affectsHeight () const
{
	return false;
}

//-------------------------------------------------------------------

bool TerrainGenerator::Affector::affectsShader () const
{
	return false;
}

float TerrainGenerator::Affector::isWithin (float worldX, float worldZ) const
{
	UNREF(worldX);
	UNREF(worldZ);
	return 0.0f;
}

//-------------------------------------------------------------------
//
// TerrainGenerator::Layer::ProfileData
//

TerrainGenerator::Layer::ProfileData::ProfileData () :
	timeInOverhead (0),
	timeInBoundaries (0),
	timeInFilters (0),
	timeInAffectors (0),
	timeInSubLayers (0)
{
}

//-------------------------------------------------------------------
	
TerrainGenerator::Layer::ProfileData::~ProfileData ()
{
}

//-------------------------------------------------------------------

float TerrainGenerator::Layer::ProfileData::getLayerTime () const
{
	return 
		timeInOverhead   +
		timeInBoundaries +
		timeInFilters    +
		timeInAffectors;
}

//-------------------------------------------------------------------

float TerrainGenerator::Layer::ProfileData::getTotalTime () const
{
	return getLayerTime () + timeInSubLayers;
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::ProfileData::reset ()
{
	timeInOverhead   = 0;
	timeInBoundaries = 0;
	timeInFilters    = 0;
	timeInAffectors  = 0;
	timeInSubLayers  = 0;
}

//-------------------------------------------------------------------

bool TerrainGenerator::Layer::ProfileData::isWorthCounting () const
{
	const float minimumTime = 0.001f;

	return 
		timeInOverhead   > minimumTime ||
		timeInBoundaries > minimumTime ||
		timeInFilters    > minimumTime ||
		timeInAffectors  > minimumTime ||
		timeInSubLayers  > minimumTime;
}


//-------------------------------------------------------------------
//
// TerrainGenerator::Layer
//

TerrainGenerator::Layer::Layer () :
	LayerItem (TAG_LAYR),
	m_boundaryList (),
	m_filterList (),
	m_affectorList (),
	m_subLayerList(),
	m_hasActiveBoundaries (false),
	m_hasActiveFilters (false),
	m_hasActiveAffectors (false),
	m_hasUnprunedAffectors(false),
	m_hasActiveLayers (false),
	m_hasUnprunedLayers(false),
	m_invertBoundaries (false),
	m_invertFilters (false),
	m_useExtent (false),
	m_extent (),
	m_modificationHeight (0.f),
	m_expanded (false),
	m_notes (0),
	m_profileData ()
{
}

//-------------------------------------------------------------------

TerrainGenerator::Layer::~Layer ()
{
	if (m_notes)
	{
		delete [] m_notes;
		m_notes = 0;
	}

	int i;
	for (i = 0; i < m_boundaryList.getNumberOfElements (); i++)
	{
		delete m_boundaryList [i];
		m_boundaryList [i] = 0;
	}

	m_boundaryList.clear ();

	for (i = 0; i < m_filterList.getNumberOfElements (); i++)
	{
		delete m_filterList [i];
		m_filterList [i] = 0;
	}

	m_filterList.clear ();

	for (i = 0; i < m_affectorList.getNumberOfElements (); i++)
	{
		delete m_affectorList [i];
		m_affectorList [i] = 0;
	}

	m_affectorList.clear ();

	for (i = 0; i < m_subLayerList.getNumberOfElements (); i++)
	{
		delete m_subLayerList[i];
		m_subLayerList[i] = 0;
	}
	m_subLayerList.clear ();
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::setNotes (const char* const notes)
{
	if (m_notes)
	{
		delete [] m_notes;
		m_notes = 0;
	}

	if (notes)
		m_notes = DuplicateString (notes);
}

//----------------------------------------------------------------------

bool TerrainGenerator::Layer::computeHasPassableAffectors() const
{
	
	{
		for (int i = 0; i < m_affectorList.getNumberOfElements (); i++)
		{
			Affector const * const affector = m_affectorList [i];

			if (affector->getType() == TGAT_passable)
			{
				return true;
			}
		}
	}

	{
		for (int i = 0; i < m_subLayerList.getNumberOfElements (); i++)
		{
			if (m_subLayerList[i]->computeHasPassableAffectors())
				return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::addBoundary (Boundary* const boundary)
{
	m_boundaryList.add (boundary);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::removeBoundary (const int index)
{
	delete m_boundaryList [index];

	m_boundaryList.removeIndexAndCompactList (index);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::removeBoundary (const Boundary* const boundary, const bool doDelete)
{
	//-- find boundary in list
	int i;
	for (i = 0; i < m_boundaryList.getNumberOfElements (); i++)
		if (m_boundaryList [i] == boundary)
		{
			if (doDelete)
				delete m_boundaryList [i];

			m_boundaryList [i] = 0;

			break;
		}

	DEBUG_FATAL (i == m_boundaryList.getNumberOfElements (), ("boundary not found in boundary list"));
	m_boundaryList.removeIndexAndCompactList (i);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::addFilter (Filter* const filter)
{
	m_filterList.add (filter);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::removeFilter (const int index)
{
	delete m_filterList [index];

	m_filterList.removeIndexAndCompactList (index);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::removeFilter (const Filter* const filter, const bool doDelete)
{
	//-- find filter in list
	int i;
	for (i = 0; i < m_filterList.getNumberOfElements (); i++)
		if (m_filterList [i] == filter)
		{
			if (doDelete)
				delete m_filterList [i];

			m_filterList [i] = 0;

			break;
		}

	DEBUG_FATAL (i == m_filterList.getNumberOfElements (), ("filter not found in filter list"));
	m_filterList.removeIndexAndCompactList (i);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::addAffector (Affector* const affector)
{
	m_affectorList.add (affector);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::removeAffector (const int index)
{
	delete m_affectorList [index];

	m_affectorList.removeIndexAndCompactList (index);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::removeAffector (const Affector* const affector, const bool doDelete)
{
	//-- find affector in list
	int i;
	for (i = 0; i < m_affectorList.getNumberOfElements (); i++)
		if (m_affectorList [i] == affector)
		{
			if (doDelete)
				delete m_affectorList [i];

			m_affectorList [i] = 0;

			break;
		}

	DEBUG_FATAL (i == m_affectorList.getNumberOfElements (), ("affector not found in affector list"));
	m_affectorList.removeIndexAndCompactList (i);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::prepare ()
{
	m_hasActiveBoundaries = false;
	{
		int i;
		for (i = 0; i < m_boundaryList.getNumberOfElements (); ++i)
		{
			if (m_boundaryList[i]->isActive ())
			{
				m_hasActiveBoundaries=true;

				m_boundaryList [i]->prepare ();
			}
		}
	}

	m_hasActiveFilters = false;
	{
		int i;
		for (i = 0; i < m_filterList.getNumberOfElements (); ++i)
		{
			if (m_filterList [i]->isActive ())
			{
				m_hasActiveFilters=true;

				m_filterList [i]->prepare ();
			}
		}
	}

	m_hasActiveAffectors = false;
	{
		int i;
		for (i = 0; i < m_affectorList.getNumberOfElements (); ++i)
		{
			if (m_affectorList [i]->isActive ())
			{
				m_hasActiveAffectors=true;

				m_affectorList [i]->prepare ();
			}
		}
	}

	m_hasActiveLayers = false;
	{
		int i;
		for (i = 0; i < m_subLayerList.getNumberOfElements (); ++i)
		{
			if (m_subLayerList[i]->isActive())
			{
				m_hasActiveLayers=true;

				m_subLayerList[i]->prepare ();
			}
		}
	}
}

//-------------------------------------------------------------------
void TerrainGenerator::Layer::_oldBoundaryTest(float &fuzzyTest, float worldX, float worldZ) const
{
	fuzzyTest = 0.f;

	//-- see if it is within any of the boundaries (if any)
	bool hasActiveBoundaries = false;

	if (m_hasActiveBoundaries)
	{
		int i;
		for (i = 0; i < m_boundaryList.getNumberOfElements (); i++)
		{
			if (m_boundaryList [i]->isActive ())
			{
				hasActiveBoundaries = true;

				const Feather feather (m_boundaryList [i]->getFeatherFunction ());

				const float amount = m_boundaryList [i]->isWithin (worldX, worldZ);
				DEBUG_FATAL (amount < 0.f || amount > 1.f, ("amount out of range [0-1] %1.2f", amount));

				fuzzyTest = FuzzyOr (fuzzyTest, feather.feather (0.f, 1.f, amount));
				if (fuzzyTest == 1.f) 
					break;
			}
		}

	}

	if (!hasActiveBoundaries)
	{
		fuzzyTest = 1.f;
	}
}

//-------------------------------------------------------------------

bool TerrainGenerator::Layer::prune(unsigned &mapMask, const Rectangle2d &chunkExtentIUO)
{
	m_hasUnprunedLayers=false;
	m_hasUnprunedAffectors=false;

	// ------------------------------------------------------------------------
	//-- if there are no affectors and no layers, don't do anything
	if (!m_hasActiveAffectors && !m_hasActiveLayers)
	{
		setPruned(true);
		return true;
	}
	// ------------------------------------------------------------------------

	// ------------------------------------------------------------------------
	//-- if the chunk is nowhere near the layer, don't do anything
	if (m_useExtent && !m_extent.intersects(chunkExtentIUO))
	{
		setPruned(true);
		return true;
	}
	// ------------------------------------------------------------------------

	// ------------------------------------------------------------------------
	if (m_hasActiveLayers)
	{
		for (int i = m_subLayerList.getNumberOfElements()-1; i >=0 ; i--)
		{
			Layer * layer = m_subLayerList[i];
			if (!layer->prune(mapMask, chunkExtentIUO))
			{
				m_hasUnprunedLayers=true;
			}
		}
	}
	// ------------------------------------------------------------------------

	// ------------------------------------------------------------------------
	if (m_hasActiveAffectors)
	{
		for (int i = m_affectorList.getNumberOfElements()-1; i>=0 ; i--)
		{
			Affector *a = m_affectorList[i];

			bool isPruned = !a->isActive();
			if (!isPruned)
			{
				unsigned maps = a->getAffectedMaps();
				isPruned = (maps&mapMask)==0;
			}
			/*
			if (isPruned)
			{
				REPORT_LOG_PRINT(true, ("Affector pruned!.\n"));
			}
			*/
			a->setPruned(isPruned);
			if (!a->isPruned())
			{
				m_hasUnprunedAffectors=true;
			}
		}
	}
	// ------------------------------------------------------------------------

	const bool newPruned = !m_hasUnprunedAffectors && !m_hasUnprunedLayers;

	// ------------------------------------------------------------------------
	// update the map mask for any filter needs.
	if (!newPruned && m_hasActiveFilters)
	{
		for (int i = 0; i < m_filterList.getNumberOfElements(); i++)
		{
			Filter *f = m_filterList[i];
			if (!f->isActive())
			{
				continue;
			}
			if (f->needsNormals())
			{
				mapMask |= (TGM_height | TGM_vertexNormal);
				break;
			}
		}
		if (m_hasActiveAffectors)
		{
			for (int i = 0; i < m_filterList.getNumberOfElements(); i++)
			{
				Filter *f = m_filterList[i];
				if (!f->isActive())
				{
					continue;
				}
				if (f->needsShaders())
				{
					mapMask |= TGM_shader;
					break;
				}
			}
		}
	}
	// ------------------------------------------------------------------------

	// ------------------------------------------------------------------------
	setPruned(newPruned);
	return newPruned;
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::affect (const float * previousAmountMap, const GeneratorChunkData& generatorChunkData) const
{
	//-----------------------------------------------------------------------
	//-- scan filters to see if we need to generate plane and vertex normals
	if (m_hasActiveFilters)
	{
		for (int i = 0; i < m_filterList.getNumberOfElements (); i++)
		{
			if (m_filterList [i]->isActive () && m_filterList [i]->needsNormals ())
			{
				if (generatorChunkData.normalsDirtyIUO)
				{
					TerrainGenerator::generatePlaneAndVertexNormals (generatorChunkData);

					generatorChunkData.normalsDirtyIUO = false;
				}

				break;
			}
		}

		//-----------------------------------------------------------------------
		//-- scan filters to see if we need to stamp shader families
		if (m_hasActiveAffectors)
		{
			//-- synchronize shader children
			for (int i = 0; i < m_filterList.getNumberOfElements (); i++)
			{
				if (m_filterList [i]->isActive () && m_filterList [i]->needsShaders ())
				{
					if (generatorChunkData.shadersDirtyIUO)
					{
						synchronizeShaders (generatorChunkData);

						generatorChunkData.shadersDirtyIUO = false;
					}

					break;
				}
			}
		}
		//-----------------------------------------------------------------------
	}
	//-----------------------------------------------------------------------

	const bool onlyHasSubLayers = !m_hasActiveBoundaries && !m_hasActiveFilters && !m_hasActiveAffectors;
	const int numberOfPoles = generatorChunkData.numberOfPoles;

	//-----------------------------------------------------------------------
	//-- only allocate the amount map if we have sublayers. the amount map is used for feathering of sublayers
	float *amountMap=0;
	if (m_hasActiveLayers && !onlyHasSubLayers)
	{
		amountMap=(float *)_alloca(numberOfPoles*numberOfPoles*sizeof(*amountMap));
	}
	//-----------------------------------------------------------------------

	bool shouldAffectSubLayers = onlyHasSubLayers;
	if (!onlyHasSubLayers)
	{
		//---------------------------------------------------------------------------------------------
		float *boundaryMap=0;
		if (m_hasActiveBoundaries)
		{
			for (int i = 0; i < m_boundaryList.getNumberOfElements(); i++)
			{
				Boundary *b = m_boundaryList[i];
				if (!b->isActive())
				{
					continue;
				}

				if (!boundaryMap)
				{
					const int boundaryMapSize=numberOfPoles*numberOfPoles*sizeof(*boundaryMap);
					boundaryMap = (float *)_alloca(boundaryMapSize);
					memset(boundaryMap, 0, boundaryMapSize);
				}

				b->scanConvertGT(boundaryMap, generatorChunkData.chunkExtentIUO, numberOfPoles);
			}
		}
		//---------------------------------------------------------------------------------------------

		const bool invertBoundaries=m_invertBoundaries;
		const float distanceBetweenPoles = generatorChunkData.distanceBetweenPoles;
		for (int z = 0; z < numberOfPoles; z++)
		{
			const int rowIndex = z * numberOfPoles;

			const float worldZ = generatorChunkData.start.z + static_cast<float>(z)*distanceBetweenPoles;
			const float *previousAmountRow = previousAmountMap + rowIndex;

			for (int x = 0; x < numberOfPoles; x++)
			{
				const float worldX         = generatorChunkData.start.x + static_cast<float>(x)*distanceBetweenPoles;
				const float previousAmount = previousAmountRow[x];

				float fuzzyTest;
				//-------------------------------------------------------------------------------------
				if (boundaryMap)
				{
					fuzzyTest=boundaryMap[rowIndex + x];
				}
				else
				{
					fuzzyTest=1.0;
				}
				//-------------------------------------------------------------------------------------
				//float _fuzzyTest;
				//_oldBoundaryTest(_fuzzyTest, worldX, worldZ);
				//DEBUG_FATAL(fabs(_fuzzyTest-fuzzyTest)>1e-6, (""));
				//-------------------------------------------------------------------------------------

				if (invertBoundaries)
				{
					fuzzyTest = 1.f - fuzzyTest;
				}

				DEBUG_FATAL (fuzzyTest < 0.f || fuzzyTest > 1.f, ("Boundary tests returned invalid value: %1.3f", fuzzyTest));

				if (fuzzyTest > 0.f)
				{
					//-- see if it passes all filters (if any)
					if (m_hasActiveFilters)
					{
						int i;
						for (i = 0; i < m_filterList.getNumberOfElements (); i++)
						{
							if (m_filterList [i]->isActive ())
							{

								if(m_filterList[i]->getType() == TGFT_bitmap) // special case the bitmap filter because of boundaries
								{
									FilterBitmap *filterBitmap = safe_cast<FilterBitmap *>(m_filterList[i]);
									filterBitmap->setExtent(m_extent);
								}
								
								const Feather feather (m_filterList [i]->getFeatherFunction ());

								const float amount = m_filterList [i]->isWithin (worldX, worldZ, x, z, generatorChunkData);
								
								DEBUG_FATAL (amount < 0.f || amount > 1.f, ("amount out of range [0-1] %1.2f", amount));

								fuzzyTest = FuzzyAnd (fuzzyTest, feather.feather (0.f, 1.f, amount));

								if (fuzzyTest == 0.f) 
									break;
								
							}
						}

					}

					DEBUG_FATAL (fuzzyTest < 0.f || fuzzyTest > 1.f, ("Filter tests returned invalid value: %1.3f", fuzzyTest));

					if (m_invertFilters)
					{
						fuzzyTest = 1.f - fuzzyTest;
					}
					
					if (fuzzyTest > 0.f)
					{
						//-- there was at least one fuzzy test valid here, so we should affect sublayers
						shouldAffectSubLayers = true;

						//-- run all affectors
						if (m_hasUnprunedAffectors)
						{
							for (int i = 0; i < m_affectorList.getNumberOfElements (); i++)
							{
								Affector *a = m_affectorList[i];
								if (!a->isPruned())
								{
									a->affect (worldX, worldZ, x, z, fuzzyTest * previousAmount, generatorChunkData);

									if (a->affectsHeight())
									{
										generatorChunkData.normalsDirtyIUO = true;
									}

									if (a->affectsShader())
									{
										generatorChunkData.shadersDirtyIUO = true;
									}
								}
							}
						}
					}
				}

				if (amountMap)
				{
					amountMap[rowIndex + x]=fuzzyTest * previousAmount;
				}
			}
		}
	}

	//-- now affect the layers
	if (shouldAffectSubLayers && m_hasActiveLayers)
	{
		for (int i = 0; i < m_subLayerList.getNumberOfElements (); i++)
		{
			const Layer *l = m_subLayerList[i];
			if (!l->isPruned())
			{
				l->affect(onlyHasSubLayers ? previousAmountMap : amountMap, generatorChunkData);
			}
		}
	}
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::load_ACTN (Iff& iff, TerrainGenerator* const terrainGenerator)
{
	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_ACTN_0000 (iff, terrainGenerator);
		break;

	case TAG_0001:
		load_ACTN_0001 (iff, terrainGenerator);
		break;

	case TAG_0002:
		load_ACTN_0002 (iff, terrainGenerator);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::load_ACTN_0000 (Iff& iff, TerrainGenerator* const terrainGenerator)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			//-- load boundaries, filters, and/or affectors
			TerrainGeneratorLoader::loadLayerItem (iff, terrainGenerator, this);
		}

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::load_ACTN_0001 (Iff& iff, TerrainGenerator* const terrainGenerator)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load the layer data
		iff.enterChunk (TAG_ADTA);

			setInvertBoundaries (iff.read_int32 () != 0);

		iff.exitChunk (TAG_ADTA);

		//-- load specific data
		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			//-- load boundaries, filters, and/or affectors
			TerrainGeneratorLoader::loadLayerItem (iff, terrainGenerator, this);
		}

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::load_ACTN_0002 (Iff& iff, TerrainGenerator* const terrainGenerator)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load the layer data
		iff.enterChunk (TAG_ADTA);

			setInvertBoundaries (iff.read_int32 () != 0);
			setInvertFilters (iff.read_int32 () != 0);

		iff.exitChunk (TAG_ADTA);

		//-- load specific data
		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			//-- load boundaries, filters, and/or affectors
			TerrainGeneratorLoader::loadLayerItem (iff, terrainGenerator, this);
		}

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::addLayer (Layer* const layer)
{
	m_subLayerList.add (layer);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::removeLayer (const int index)
{
	delete m_subLayerList [index];

	m_subLayerList.removeIndexAndCompactList (index);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::removeLayer (const Layer* const layer, const bool doDelete)
{
	//-- find layer in list
	int i;
	for (i = 0; i < m_subLayerList.getNumberOfElements (); i++)
		if (m_subLayerList[i] == layer)
		{
			if (doDelete)
				delete m_subLayerList[i];

			m_subLayerList[i] = 0;

			break;
		}

	DEBUG_FATAL (i == m_subLayerList.getNumberOfElements (), ("layer not found in layer list"));
	m_subLayerList.removeIndexAndCompactList (i);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::promoteLayer (const Layer* const layer)
{
	//-- find layer in layer list
	int i;
	for (i = 0; i < m_subLayerList.getNumberOfElements (); i++)
		if (m_subLayerList[i] == layer)
			break;

	//-- if found and promote-able
	if (i < m_subLayerList.getNumberOfElements ())
		if (i < m_subLayerList.getNumberOfElements () - 1)
			std::swap (m_subLayerList[i], m_subLayerList[i + 1]);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::demoteLayer (const Layer* const layer)
{
	//-- find layer in layer list
	int i;
	for (i = 0; i < m_subLayerList.getNumberOfElements (); i++)
		if (m_subLayerList[i] == layer)
			break;

	//-- if found and demote-able
	if (i < m_subLayerList.getNumberOfElements ())
		if (i > 0)
			std::swap (m_subLayerList[i], m_subLayerList[i - 1]);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::calculateExtent ()
{
	m_useExtent = false;

	if (!m_invertBoundaries)
	{
		//-- set extent to extreme inverse
		m_extent.x0 =  FLT_MAX;
		m_extent.y0 =  FLT_MAX;
		m_extent.x1 = -FLT_MAX;
		m_extent.y1 = -FLT_MAX;

		//-- check boundaries
		int i;
		for (i = 0; i < getNumberOfBoundaries (); i++)
		{
			Boundary* boundary = getBoundary (i);

			if (boundary->isActive ())
			{
				m_useExtent = true;

				boundary->expand (m_extent);
			}
		}

		//-- run through all layers
		for (i = 0; i < getNumberOfLayers (); i++)
			if (getLayer (i)->isActive ())
				getLayer (i)->calculateExtent ();
	}
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::setModificationHeight (float const modificationHeight)
{
	m_modificationHeight = modificationHeight;
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::load (Iff& iff, TerrainGenerator* const terrainGenerator)
{
	iff.enterForm (TAG_LAYR);

		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff, terrainGenerator);
			break;

		case TAG_0001:
			load_0001 (iff, terrainGenerator);
			break;

		case TAG_0002:
			load_0002 (iff, terrainGenerator);
			break;

		case TAG_0003:
			load_0003 (iff, terrainGenerator);
			break;

		case TAG_0004:
			load_0004 (iff, terrainGenerator);
			break;

		default:
			{
				char tagBuffer [5];
				ConvertTagToString (iff.getCurrentName (), tagBuffer);

				char buffer [128];
				iff.formatLocation (buffer, sizeof (buffer));
				DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
			}
			break;
		}

	iff.exitForm (TAG_LAYR);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::load_0000 (Iff& iff, TerrainGenerator* const terrainGenerator)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterForm (TAG_ACTN);

				Layer* layer = new Layer ();
				layer->load_ACTN (iff, terrainGenerator);
				m_subLayerList.add(layer);

			iff.exitForm (TAG_ACTN);
		} //lint !e429  //-- layer has not been freed or returned

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::load_0001 (Iff& iff, TerrainGenerator* const terrainGenerator)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load the layer data
		iff.enterChunk (TAG_ADTA);

			setInvertBoundaries (iff.read_int32 () != 0);
			setInvertFilters (iff.read_int32 () != 0);

		iff.exitChunk (TAG_ADTA);

		//-- load specific data
		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			//-- load boundaries, filters, and/or affectors
			TerrainGeneratorLoader::loadLayerItem (iff, terrainGenerator, this);
		}

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::load_0002 (Iff& iff, TerrainGenerator* const terrainGenerator)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load the layer data
		iff.enterChunk (TAG_ADTA);

			setInvertBoundaries (iff.read_int32 () != 0);
			setInvertFilters (iff.read_int32 () != 0);
			setExpanded     (iff.read_int32 () != 0);

		iff.exitChunk (TAG_ADTA);

		//-- load specific data
		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			//-- load boundaries, filters, and/or affectors
			TerrainGeneratorLoader::loadLayerItem (iff, terrainGenerator, this);
		}

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::load_0003 (Iff& iff, TerrainGenerator* const terrainGenerator)
{
	iff.enterForm (TAG_0003);

		//-- load the base data
		LayerItem::load (iff);

		//-- load the layer data
		iff.enterChunk (TAG_ADTA);

			setInvertBoundaries (iff.read_int32 () != 0);
			setInvertFilters (iff.read_int32 () != 0);
			setExpanded     (iff.read_int32 () != 0);
			
			char* notes = iff.read_string ();
			setNotes (notes);
			delete [] notes;
			notes = 0;

		iff.exitChunk (TAG_ADTA);

		//-- load specific data
		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			//-- load boundaries, filters, and/or affectors
			TerrainGeneratorLoader::loadLayerItem (iff, terrainGenerator, this);
		}

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::load_0004 (Iff& iff, TerrainGenerator* const terrainGenerator)
{
	iff.enterForm (TAG_0004);

		//-- load the base data
		LayerItem::load (iff);

		//-- load the layer data
		iff.enterChunk (TAG_ADTA);
			setInvertBoundaries (iff.read_int32 () != 0);
			setInvertFilters (iff.read_int32 () != 0);
			
			{
				const int unused = iff.read_int32 ();
				UNREF (unused);
			}

			setExpanded (iff.read_int32 () != 0);
			
			char* notes = iff.read_string ();
			setNotes (notes);
			delete [] notes;
			notes = 0;

		iff.exitChunk (TAG_ADTA);

		//-- load specific data
		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			//-- load boundaries, filters, and/or affectors
			TerrainGeneratorLoader::loadLayerItem (iff, terrainGenerator, this);
		}

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::save (Iff& iff) const
{
	iff.insertForm (TAG_LAYR);

		//-- save layer
		iff.insertForm (TAG_0003);  //-- write 0005 next

			LayerItem::save (iff);

			//-- save the layer data
			iff.insertChunk (TAG_ADTA);

				iff.insertChunkData (getInvertBoundaries () ? static_cast<int32> (1) : static_cast<int32> (0));
				iff.insertChunkData (getInvertFilters () ? static_cast<int32> (1) : static_cast<int32> (0));
				iff.insertChunkData (getExpanded () ? static_cast<int32> (1) : static_cast<int32> (0));
				iff.insertChunkString (getNotes () ? getNotes () : "");

			iff.exitChunk (TAG_ADTA);

			//-- save boundaries, filters, and affectors
			int n = m_boundaryList.getNumberOfElements ();
			int i;
			for (i = 0; i < n; i++)
			{
				iff.insertForm (m_boundaryList [i]->getTag ());

					m_boundaryList [i]->save (iff);

				iff.exitForm (m_boundaryList [i]->getTag ());
			}

			n = m_filterList.getNumberOfElements ();
			for (i = 0; i < n; i++)
			{
				iff.insertForm (m_filterList [i]->getTag ());

					m_filterList [i]->save (iff);

				iff.exitForm (m_filterList [i]->getTag ());
			}

			n = m_affectorList.getNumberOfElements ();
			for (i = 0; i < n; i++)
			{
				iff.insertForm (m_affectorList [i]->getTag ());

					m_affectorList [i]->save (iff);

				iff.exitForm (m_affectorList [i]->getTag ());
			}

			n = m_subLayerList.getNumberOfElements ();
			for (i = 0; i < n; i++)
			{
				m_subLayerList[i]->save (iff);
			}

		iff.exitForm (TAG_0003);

	iff.exitForm (TAG_LAYR);
}

//-------------------------------------------------------------------
//
// TerrainGenerator
//
TerrainGenerator::TerrainGenerator () :
	m_shaderGroup (),
	m_floraGroup (),
	m_radialGroup (),
	m_environmentGroup (),
	m_fractalGroup (),
	m_bitmapGroup (),
	m_layerList (),
	m_sampleMaps(unsigned(TGM_ALL)),
	m_hasPassableAffectors(false),
	m_groupsPrepared (false)
{
}

//-------------------------------------------------------------------

TerrainGenerator::~TerrainGenerator ()
{
	reset ();
}

//-------------------------------------------------------------------

void TerrainGenerator::reset ()
{
	//-- delete shader group
	m_shaderGroup.removeAllFamilies ();

	//-- delete flora group
	m_floraGroup.removeAllFamilies ();

	//-- delete radial group
	m_radialGroup.removeAllFamilies ();

	//-- delete environment group
	m_environmentGroup.removeAllFamilies ();

	//-- delete fractal group
	m_fractalGroup.reset ();

	//-- delete bitmap group
	m_bitmapGroup.reset ();

	//-- delete layerList
	int i;
	for (i = 0; i < m_layerList.getNumberOfElements (); i++)
	{
		delete m_layerList [i];
		m_layerList [i] = 0;
	}

	m_layerList.clear ();
}

//-------------------------------------------------------------------

void TerrainGenerator::setMapsToSample(unsigned TGM_flags)
{
	m_sampleMaps = TGM_flags;
}

//-------------------------------------------------------------------

void TerrainGenerator::addLayer (Layer* const layer)
{
	m_layerList.add (layer);
}

//-------------------------------------------------------------------

void TerrainGenerator::removeLayer (const int index)
{
	delete m_layerList [index];

	m_layerList.removeIndexAndCompactList (index);
}

//-------------------------------------------------------------------

void TerrainGenerator::removeLayer (Layer* const layer, const bool doDelete)
{
	//-- find layer in list
	int i;
	for (i = 0; i < m_layerList.getNumberOfElements (); i++)
		if (m_layerList [i] == layer)
		{
			if (doDelete)
				delete layer;

			m_layerList [i] = 0;

			break;
		}

	DEBUG_FATAL (i == m_layerList.getNumberOfElements (), ("layer not found in layer list"));
	m_layerList.removeIndexAndCompactList (i);
}

//-------------------------------------------------------------------

void TerrainGenerator::affect (const GeneratorChunkData& generatorChunkData) const
{
	int i;

	float *const amountMap = (float *)_alloca(generatorChunkData.numberOfPoles*generatorChunkData.numberOfPoles*sizeof(*amountMap));
	const int totalPoles=generatorChunkData.numberOfPoles*generatorChunkData.numberOfPoles;
	for (i=0;i<totalPoles;i++)
	{
		amountMap[i]=1.0f;
	}

	generatorChunkData.normalsDirtyIUO = true;
	generatorChunkData.shadersDirtyIUO = true;

	//-- calculate chunk extents
	generatorChunkData.chunkExtentIUO.x0 = generatorChunkData.start.x;
	generatorChunkData.chunkExtentIUO.y0 = generatorChunkData.start.z;
	generatorChunkData.chunkExtentIUO.x1 = generatorChunkData.chunkExtentIUO.x0 + (static_cast<float> (generatorChunkData.numberOfPoles - 1) * generatorChunkData.distanceBetweenPoles);
	generatorChunkData.chunkExtentIUO.y1 = generatorChunkData.chunkExtentIUO.y0 + (static_cast<float> (generatorChunkData.numberOfPoles - 1) * generatorChunkData.distanceBetweenPoles);

	// ------------------------------------------------------------------

	unsigned sampleMaps = m_sampleMaps;

	if (generatorChunkData.isLegacyMode())
	{
		sampleMaps |= 
			( TGM_shader 
			| TGM_floraStaticCollidable
			| TGM_floraStaticNonCollidable
			| TGM_floraDynamicNear
			| TGM_floraDynamicFar
			);
	}

	if (sampleMaps & (TGM_vertexPosition | TGM_vertexNormal))
	{
		sampleMaps|=TGM_height;
	}

	for (i = m_layerList.getNumberOfElements()-1; i>=0 ; i--)
	{
		Layer *l = m_layerList[i];
		l->prune(sampleMaps, generatorChunkData.chunkExtentIUO);
	}

	// ------------------------------------------------------------------

	for (i = 0; i < m_layerList.getNumberOfElements (); i++)
	{
		Layer *l = m_layerList[i];
		if (!l->isPruned())
		{
			l->affect(amountMap, generatorChunkData);
		}
	}

	// ------------------------------------------------------------------

	if (  (sampleMaps & TGM_vertexNormal)!=0 
		&& generatorChunkData.normalsDirtyIUO
		)
	{
		TerrainGenerator::generatePlaneAndVertexNormals (generatorChunkData);

		generatorChunkData.normalsDirtyIUO = false;
	}

	if ( (sampleMaps & TGM_vertexPosition) != 0)
	{
		_generateVertexPositions(generatorChunkData);
	}

	if (  (sampleMaps & TGM_shader)!=0
		&& generatorChunkData.shadersDirtyIUO
		)
	{
		TerrainGenerator::synchronizeShaders (generatorChunkData);

		generatorChunkData.shadersDirtyIUO = false;
	}
}

//-------------------------------------------------------------------

void TerrainGenerator::prepare ()
{
	m_groupsPrepared = false;

	//-- calculate extent information
	calculateExtent ();

	int i;
	for (i = 0; i < m_layerList.getNumberOfElements (); ++i)
		if (m_layerList [i]->isActive ())
			m_layerList [i]->prepare ();
}

//-------------------------------------------------------------------

void TerrainGenerator::generateChunk (const GeneratorChunkData& generatorChunkData) const
{
	//--
	if (!m_groupsPrepared)
	{
		m_groupsPrepared = true;
		const_cast<TerrainGenerator*> (this)->m_fractalGroup.prepare (generatorChunkData.numberOfPoles, generatorChunkData.numberOfPoles);
	}

	generatorChunkData.validate ();

	//-- clear all maps
	generatorChunkData.heightMap->makeZero ();
	generatorChunkData.colorMap->makeValue (PackedRgb::solidWhite);
	generatorChunkData.shaderMap->makeValue (m_shaderGroup.getDefaultShader ());
	generatorChunkData.floraStaticCollidableMap->makeValue (m_floraGroup.getDefaultFlora ());
	generatorChunkData.floraStaticNonCollidableMap->makeValue (m_floraGroup.getDefaultFlora ());
	generatorChunkData.floraDynamicNearMap->makeValue (m_radialGroup.getDefaultRadial ());
	generatorChunkData.floraDynamicFarMap->makeValue (m_radialGroup.getDefaultRadial ());
	generatorChunkData.environmentMap->makeValue (m_environmentGroup.getDefaultEnvironment ());
	generatorChunkData.excludeMap->makeZero ();
	generatorChunkData.passableMap->makeValue(true);

	if (generatorChunkData.m_legacyRandomGenerator)
	{
		//-- use an arbitrary hashing function of x,z
		const uint32 seed = static_cast<uint32> 
			 ((sqr (generatorChunkData.start.x) - (0.5f * generatorChunkData.start.x)) + 
			 (sqr (generatorChunkData.start.z) - (1.5f * generatorChunkData.start.z)));

		generatorChunkData.m_legacyRandomGenerator->setSeed (seed);
	}

	//-- run the affectors
	affect (generatorChunkData);
}

//----------------------------------------------------------------------

bool TerrainGenerator::hasPassableAffectors() const
{
	return m_hasPassableAffectors;
}

//-------------------------------------------------------------------

void TerrainGenerator::_generateVertexPositions(const GeneratorChunkData& generatorChunkData) const
{
	//
	//-- cache all variables
	//
	const Array2d<float>* const heightMap                 = generatorChunkData.heightMap;
	Array2d<Vector>* const      vertexPositionMap         = generatorChunkData.vertexPositionMap;
	const int                   numberOfPoles             = generatorChunkData.numberOfPoles;
	const float                 distanceBetweenPoles      = generatorChunkData.distanceBetweenPoles;
	const Vector                start                     = generatorChunkData.start;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- build vertex position buffer
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	for (int z = 0; z < numberOfPoles; z++)
	{
		const float posZ = start.z + (z * distanceBetweenPoles);
		const float *heightRow = &heightMap->getData(0, z);
		Vector *positionRow = &vertexPositionMap->getData(0, z);

		float fx = start.x;
		for (int x = 0; x < numberOfPoles; x++, fx+=distanceBetweenPoles)
		{
			Vector &v=positionRow[x];

			v.x=fx;
			v.y=start.y + heightRow[x];
			v.z=posZ;
		}
	}
}

//-------------------------------------------------------------------

void TerrainGenerator::generatePlaneAndVertexNormals (const GeneratorChunkData& generatorChunkData)
{
	//
	//-- cache all variables
	//
	const Array2d<float>* const heightMap                 = generatorChunkData.heightMap;
	Array2d<Vector>* const      vertexNormalMap           = generatorChunkData.vertexNormalMap;
	const int                   numberOfPoles             = generatorChunkData.numberOfPoles;
	const float                 distanceBetweenPoles      = generatorChunkData.distanceBetweenPoles;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- build planeArray is used to build the vertex normal buffer and collision
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	{
		Vector normal_ur;
		Vector normal_ll;

		//-- zero out the plane data
		vertexNormalMap->makeZero ();

		for (int z = 0; z < numberOfPoles - 1; z++)
		{
			const float *heightRow0 = &heightMap->getData(0, z);
			const float *heightRow1 = &heightMap->getData(0, z+1);

			Vector *normalRow0 = &vertexNormalMap->getData(0, z);
			Vector *normalRow1 = &vertexNormalMap->getData(0, z+1);

			for (int x = 0; x < numberOfPoles - 1; x++)
			{
				//-- get the float vectors from the height buffer
				const Vector v20(-distanceBetweenPoles, heightRow1[x]-heightRow0[x+1], distanceBetweenPoles);
				const Vector v01( distanceBetweenPoles, heightRow1[x+1]-heightRow1[x], 0);
				const Vector v32( distanceBetweenPoles, heightRow0[x+1]-heightRow0[x], 0);

				//-- grab normals
				//-- FIXME_ALS - this doesn't match the tile data organization
				normal_ur = v20.cross(v01);
				normal_ll = v20.cross(v32);

				//-- add normals to vertex normal buffer at ur (0, 1, 2)
				normalRow1[x]   += normal_ur; //vertexNormalMap->getData (x,     z + 1) += normal_ur;
				normalRow1[x+1] += normal_ur; //vertexNormalMap->getData (x + 1, z + 1) += normal_ur;
				normalRow0[x+1] += normal_ur; //vertexNormalMap->getData (x + 1, z)     += normal_ur;

				//-- add normals to vertex normal buffer at ll (2, 3, 0)
				normalRow0[x+1] += normal_ll; //vertexNormalMap->getData (x + 1, z)     += normal_ll;
				normalRow0[x]   += normal_ll; //vertexNormalMap->getData (x,     z)     += normal_ll;
				normalRow1[x]   += normal_ll; //vertexNormalMap->getData (x,     z + 1) += normal_ll;
			}
		}
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- make pass on vertexNormalMap to normalize all vertex normals
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	{
		Vector *normals = &vertexNormalMap->getData(0,0);

		const int totalPoles = numberOfPoles*numberOfPoles;
		const Vector *const normalsStop = normals + totalPoles;

		while (normals!=normalsStop)
		{
			const float mag = normals->magnitude();
			*normals *= 1.0f / mag;
			normals++;
		}
	}
}

//-------------------------------------------------------------------

void TerrainGenerator::synchronizeShaders (const GeneratorChunkData& generatorChunkData) 
{
	const int originOffset = generatorChunkData.originOffset;

	const int preOffset = 2 - originOffset;
	const int postOffset = originOffset;

	Array2d<ShaderGroup::Info> *const shaderMap = generatorChunkData.shaderMap;

	int upperPad = generatorChunkData.upperPad - 1;
	if (upperPad<0)
	{
		upperPad=0;
	}

	const int maxHeight = ((shaderMap->getHeight()-postOffset-1)|3)-preOffset+1;
	int height = shaderMap->getHeight() - upperPad;
	if (height>maxHeight)
	{
		height=maxHeight;
	}

	const int maxWidth = ((shaderMap->getWidth()-postOffset-1)|3)-preOffset+1;
	int width = shaderMap->getWidth() - upperPad;
	if (width>maxWidth)
	{
		width=maxWidth;
	}

	for (int z = 0; z < height; ++z)
	{
		int preOffsetZ = z + preOffset;
		if (preOffsetZ<0)
		{
			preOffsetZ=0;
		}
		const int newZ = (preOffsetZ & ~3) + postOffset;

		const ShaderGroup::Info *srcRow = &shaderMap->getData(0, newZ);
		ShaderGroup::Info *destRow = &shaderMap->getData(0, z);

		for (int x = 0; x < width; ++x)
		{
			int preOffsetX = x + preOffset;
			if (preOffsetX<0)
			{
				preOffsetX=0;
			}
			const int newX = (preOffsetX & ~3) + postOffset;

			const ShaderGroup::Info old_sgi = destRow[x]; //shaderMap->getData (x, z);

			ShaderGroup::Info new_sgi = srcRow[newX]; //shaderMap->getData (newX, newZ);

			new_sgi.setChildChoice (old_sgi.getChildChoice ());

			destRow[x]=new_sgi; //shaderMap->setData (x, z, new_sgi);
		}
	}
}

//-------------------------------------------------------------------

void TerrainGenerator::load (Iff& iff)
{
	iff.enterForm (TAG (T,G,E,N));

	//-- specific load
	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	default:
		{
			char buffer[128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("bad version number at %s", buffer));
		}
		break;
	}

	iff.exitForm (TAG (T,G,E,N));

	//-- prepare at least once after load (in case we're not running the tool)
	prepare ();

	//-- check for passable affectors

	{
		int n = m_layerList.getNumberOfElements ();
		for (int i = 0; i < n && !m_hasPassableAffectors; i++)
		{
			m_hasPassableAffectors = m_layerList [i]->computeHasPassableAffectors();
		}
	}
}

//-------------------------------------------------------------------

void TerrainGenerator::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

	//-- load shader group (this allows creation of TerrainGenerator outside of terrain system)
	m_shaderGroup.load (iff);

	//-- load flora group
	m_floraGroup.load (iff);

	//-- load flora group
	m_radialGroup.load (iff);

	//-- load environment group
	m_environmentGroup.load (iff);

	//-- load fractal group
	m_fractalGroup.load (iff);

	//-- load bitmap group
	m_bitmapGroup.load(iff);

	//-- load layers
	if (iff.enterForm (TAG (L,Y,R,S), true))
	{
		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			Layer* layer = new Layer ();
			layer->load (iff, this);
			addLayer (layer);
		} //lint !e429  //-- layer has not been freed or returned
	
		iff.exitForm (TAG (L,Y,R,S));
	}

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void TerrainGenerator::saveLayers (Iff& iff) const
{
	iff.insertForm (TAG (L,Y,R,S));

		int n = m_layerList.getNumberOfElements ();
		int i;
		for (i = 0; i < n; i++)
			m_layerList [i]->save (iff);

	iff.exitForm (TAG (L,Y,R,S));
}

//-------------------------------------------------------------------

void TerrainGenerator::save (Iff& iff) const
{
	iff.insertForm (TAG (T,G,E,N));

		iff.insertForm (TAG_0000);

			//-- write shader group
			m_shaderGroup.save (iff);

			//-- write flora group
			m_floraGroup.save (iff);

			//-- write flora group
			m_radialGroup.save (iff);

			//-- write environment group
			m_environmentGroup.save (iff);

			//-- write fractal group
			m_fractalGroup.save (iff);

			//-- write bitmap group
			m_bitmapGroup.save(iff);

			//-- save layers
			saveLayers (iff);

		iff.exitForm (TAG_0000);

	iff.exitForm (TAG (T,G,E,N));
}

//-------------------------------------------------------------------

void TerrainGenerator::promoteLayer (const Layer* const layer)
{
	//-- find layer in layer list
	int i;
	for (i = 0; i < m_layerList.getNumberOfElements (); i++)
		if (m_layerList [i] == layer)
			break;

	//-- if found and promote-able
	if (i < m_layerList.getNumberOfElements ())
		if (i < m_layerList.getNumberOfElements () - 1)
			std::swap (m_layerList [i], m_layerList [i + 1]);
}

//-------------------------------------------------------------------

void TerrainGenerator::demoteLayer (const Layer* const layer)
{
	//-- find layer in layer list
	int i;
	for (i = 0; i < m_layerList.getNumberOfElements (); i++)
		if (m_layerList [i] == layer)
			break;

	//-- if found and demote-able
	if (i < m_layerList.getNumberOfElements ())
		if (i > 0)
			std::swap (m_layerList [i], m_layerList [i - 1]);
}

//-------------------------------------------------------------------

void TerrainGenerator::calculateExtent ()
{
	int i;
	for (i = 0; i < m_layerList.getNumberOfElements (); i++)
		if (m_layerList [i]->isActive ())
			m_layerList [i]->calculateExtent ();
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::resetProfileData ()
{
	m_profileData.reset ();

	int i;
	for (i = 0; i < m_subLayerList.getNumberOfElements(); i++)
		m_subLayerList[i]->resetProfileData ();
}

//-------------------------------------------------------------------

void TerrainGenerator::resetProfileData ()
{
	int i;
	for (i = 0; i < m_layerList.getNumberOfElements (); i++)
		m_layerList [i]->resetProfileData ();
}

//-------------------------------------------------------------------

void TerrainGenerator::Boundary::setFeatherFunction (const TerrainGeneratorFeatherFunction featherFunction)
{
	m_featherFunction = featherFunction;
}

//-------------------------------------------------------------------

void TerrainGenerator::Boundary::setFeatherDistance (const float featherDistance)
{
	m_featherDistance = featherDistance;
}

//-------------------------------------------------------------------

void TerrainGenerator::Filter::setFeatherFunction (const TerrainGeneratorFeatherFunction featherFunction)
{
	m_featherFunction = featherFunction;
}

//-------------------------------------------------------------------

void TerrainGenerator::Filter::setFeatherDistance (const float featherDistance)
{
	m_featherDistance = featherDistance;
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::setInvertBoundaries (const bool invertBoundaries)
{
	m_invertBoundaries = invertBoundaries;
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::setInvertFilters (const bool invertFilters)
{
	m_invertFilters = invertFilters;
}

//-------------------------------------------------------------------

void TerrainGenerator::Layer::setExpanded (const bool expanded)
{
	m_expanded = expanded;
}

//-------------------------------------------------------------------

const TerrainGenerator::Layer::ProfileData& TerrainGenerator::Layer::getProfileData () const
{
	return m_profileData;
}

//===================================================================

