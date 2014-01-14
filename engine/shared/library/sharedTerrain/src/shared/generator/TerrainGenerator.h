//===================================================================
//
// TerrainGenerator.h
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//--
//
// figure out:
// 1) if a layer is inactive, it shouldn't be loaded instead of tested 
//		before use
//
//===================================================================

#ifndef INCLUDED_TerrainGenerator_H
#define INCLUDED_TerrainGenerator_H

//===================================================================

#include "sharedFoundation/ArrayList.h"
#include "sharedFoundation/Tag.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Vector.h"
#include "sharedRandom/RandomGenerator.h"
#include "sharedTerrain/Array2d.h"
#include "sharedTerrain/BitmapGroup.h"
#include "sharedTerrain/EnvironmentGroup.h"
#include "sharedTerrain/FloraGroup.h"
#include "sharedTerrain/FractalGroup.h"
#include "sharedTerrain/RadialGroup.h"
#include "sharedTerrain/ShaderGroup.h"
#include "sharedTerrain/TerrainGeneratorType.h"

class Shader;

//===================================================================

class TerrainGenerator
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// GeneratorChunkData is used to request a chunk from the generator
	//
	struct GeneratorChunkData
	{
	public:

		//-- number of poles also represents the number of valid indices in the chunk maps
		int                              originOffset;
		int                              numberOfPoles;
		int                              upperPad;
		float                            distanceBetweenPoles;

		//-- offset into world space for left-rear corner of chunk to build (including the buffer zone)
		Vector                           start;

		//-- numberOfPoles x numberOfPoles
		Array2d<float>*                  heightMap;

		//-- numberOfPoles x numberOfPoles
		Array2d<PackedRgb>*              colorMap;

		//-- since a tile is two poles, every other entry is skipped
		Array2d<ShaderGroup::Info>*      shaderMap;

		//-- map of flora per tile
		Array2d<FloraGroup::Info>*       floraStaticCollidableMap;
		
		//-- map of flora per tile
		Array2d<FloraGroup::Info>*       floraStaticNonCollidableMap;
		
		//-- map of radial flora per tile
		Array2d<RadialGroup::Info>*      floraDynamicNearMap;
		
		//-- map of radial flora per tile
		Array2d<RadialGroup::Info>*      floraDynamicFarMap;
		
		//-- map of environment block data per tile
		Array2d<EnvironmentGroup::Info>* environmentMap;
		
		//-- used to calculate the plane data and vertex normals
		Array2d<Vector>*                 vertexPositionMap;
		
		//-- the normal at the vertex
		Array2d<Vector>*                 vertexNormalMap;

		//-- allows tiles to be excluded
		Array2d<bool>*                   excludeMap;

		//-- allows tiles to be passable/not passable
		Array2d<bool>*                   passableMap;

		//-- groups
		const ShaderGroup*               shaderGroup;
		const FloraGroup*                floraGroup;
		const RadialGroup*               radialGroup;
		const EnvironmentGroup*          environmentGroup;
		const FractalGroup*              fractalGroup;	
		const BitmapGroup*               bitmapGroup;

		//-- provides random numbers for choosers and affectors
		RandomGenerator                 *m_legacyRandomGenerator;

		//-- internal use only
		mutable bool                     normalsDirtyIUO;
		mutable bool                     shadersDirtyIUO;
		mutable Rectangle2d              chunkExtentIUO;

	private:

		GeneratorChunkData (const GeneratorChunkData& rhs);
		GeneratorChunkData& operator= (const GeneratorChunkData& rhs);

	public:

		GeneratorChunkData(bool legacyMode);
		~GeneratorChunkData ();

		void validate () const;

		bool isLegacyMode() const { return m_legacyRandomGenerator!=0; }
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// CreateChunkBuffer is a scratchpad for generating terrain chunks
	//
	struct CreateChunkBuffer
	{
	public:

		//-- actual maps
		Array2d<float>              heightMap;
		Array2d<PackedRgb>         colorMap;
		Array2d<ShaderGroup::Info> shaderMap;
		Array2d<FloraGroup::Info>  floraStaticCollidableMap;
		Array2d<FloraGroup::Info>  floraStaticNonCollidableMap;
		Array2d<RadialGroup::Info> floraDynamicNearMap;
		Array2d<RadialGroup::Info> floraDynamicFarMap;
		Array2d<EnvironmentGroup::Info> environmentMap;
		Array2d<Vector>            vertexPositionMap;
		Array2d<Vector>            vertexNormalMap;
		Array2d<bool>              excludeMap;
		Array2d<bool>              passableMap;

	private:

		CreateChunkBuffer (const CreateChunkBuffer& rhs);
		CreateChunkBuffer& operator= (const CreateChunkBuffer& rhs);

	public:

		CreateChunkBuffer ();
		~CreateChunkBuffer ();

		void allocate (int poleCount);

		void validate () const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// LayerItem holds tool specific data for all Boundaries, Filters, Affectors, and Layers
	//
	class LayerItem
	{
	private:

		const Tag        m_tag;
		bool             m_active;
		bool             m_pruned;
		char*            m_name;

	private:

		void             load_0000 (Iff& iff);
		void             load_0001 (Iff& iff);

	private:

		LayerItem ();
		LayerItem (const LayerItem& rhs);
		LayerItem& operator= (const LayerItem& rhs);

	public:

		explicit LayerItem (Tag tag);
		virtual ~LayerItem ()=0;

		Tag              getTag () const;

		void             setActive (bool active);
		bool             isActive () const;

		void             setPruned(bool pruned) { m_pruned=pruned; }
		bool             isPruned() const       { return m_pruned; }

		void             setName (const char* name);
		const char*      getName () const;

		virtual void     prepare ();
		virtual void     load (Iff& iff);
		virtual void     save (Iff& iff) const=0;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// Boundary is the interface for all boundary types. This could potentially 
	//	be optimized by having the circle as the base type, and only checking 
	//	isWithin for derived types if isWithin for the circle test passes first 
	//	since the circle would be the fastest check.
	//
	class Boundary : public LayerItem
	{
	private:

		const TerrainGeneratorBoundaryType m_type;

		TerrainGeneratorFeatherFunction    m_featherFunction;
		float                              m_featherDistance;

	private:

		Boundary ();
		Boundary (const Boundary& rhs);
		Boundary& operator= (const Boundary& rhs);

	public:

		Boundary (Tag tag, TerrainGeneratorBoundaryType type);
		virtual ~Boundary ()=0;

		TerrainGeneratorBoundaryType getType () const;

		TerrainGeneratorFeatherFunction getFeatherFunction () const;
		void          setFeatherFunction (TerrainGeneratorFeatherFunction featherFunction);

		float         getFeatherDistance () const;
		virtual void  setFeatherDistance (float featherDistance);

		virtual void  rotate (float angle);
		virtual void  rotate (float angle, const Vector2d& center);
		virtual void  translate (const Vector2d& translation);
		virtual void  scale (float scalar);
		virtual float isWithin (float worldX, float worldZ) const=0;
		virtual void  expand (Rectangle2d& extent) const=0;
		virtual const Vector2d getCenter () const=0;

		virtual bool intersects(const Rectangle2d& other) const=0;
		virtual void scanConvertGT(float *o_data, const Rectangle2d &scanArea, int numberOfPoles) const;

		//-- run-time rule interface
		virtual void  setCenter (const Vector2d& center);
		virtual void  setRotation (float angle);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Filter : public LayerItem
	{
	private:

		const TerrainGeneratorFilterType m_type;

		TerrainGeneratorFeatherFunction  m_featherFunction;
		float                            m_featherDistance;

	private:

		Filter ();
		Filter (const Filter& rhs);
		Filter& operator= (const Filter& rhs);

	public:

		Filter (Tag tag, TerrainGeneratorFilterType type);
		virtual ~Filter ()=0;

		TerrainGeneratorFilterType getType () const;

		TerrainGeneratorFeatherFunction getFeatherFunction () const;
		void          setFeatherFunction (TerrainGeneratorFeatherFunction featherFunction);

		float         getFeatherDistance () const;
		virtual void  setFeatherDistance (float featherDistance);

		virtual float isWithin (float worldX, float worldZ, int x, int z, const GeneratorChunkData& generatorChunkData) const=0;

		virtual bool  needsNormals () const;
		virtual bool  needsShaders () const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Affector : public LayerItem
	{
	private:

		const TerrainGeneratorAffectorType m_type;

	private:

		Affector ();
		Affector (const Affector& rhs);
		Affector& operator= (const Affector& rhs);

	public:

		Affector (Tag tag, TerrainGeneratorAffectorType type);
		virtual ~Affector ()=0;

		TerrainGeneratorAffectorType getType () const;

		virtual void affect (float worldX, float worldZ, int x, int z, float amount, const GeneratorChunkData& generatorChunkData) const=0;
		virtual bool affectsHeight () const;
		virtual bool affectsShader () const;
		virtual unsigned getAffectedMaps() const=0;
		virtual float isWithin (float worldX, float worldZ) const;

	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Layer : public LayerItem
	{
	public:

		struct ProfileData
		{
		public:

			float timeInOverhead;
			float timeInBoundaries;
			float timeInFilters;
			float timeInAffectors;
			float timeInSubLayers;

		public:

			ProfileData ();
			~ProfileData ();

			bool isWorthCounting () const;
			float getLayerTime () const;
			float getTotalTime () const;

			void reset ();
		};

	private:

		ArrayList<Boundary*>   m_boundaryList;
		ArrayList<Filter*>     m_filterList;
		ArrayList<Affector*>   m_affectorList;
		ArrayList<Layer*>      m_subLayerList;

		bool                   m_hasActiveBoundaries;
		bool                   m_hasActiveFilters;
		bool                   m_hasActiveAffectors;
		bool                   m_hasUnprunedAffectors;
		bool                   m_hasActiveLayers;
		bool                   m_hasUnprunedLayers;

		bool                   m_invertBoundaries;
		bool                   m_invertFilters;

		bool                   m_useExtent;
		Rectangle2d            m_extent;

		float                  m_modificationHeight;

		bool                   m_expanded;

		char*                  m_notes;

	private:

		mutable ProfileData    m_profileData;

	private:

		void            load_0000 (Iff& iff, TerrainGenerator* terrainGenerator); //lint !e1511 // member hides non-virt
		void            load_0001 (Iff& iff, TerrainGenerator* terrainGenerator); //lint !e1511 // member hides non-virt
		void            load_0002 (Iff& iff, TerrainGenerator* terrainGenerator); //lint !e1511 // member hides non-virt
		void            load_0003 (Iff& iff, TerrainGenerator* terrainGenerator); //lint !e1511 // member hides non-virt
		void            load_0004 (Iff& iff, TerrainGenerator* terrainGenerator); //lint !e1511 // member hides non-virt

		void            load_ACTN (Iff& iff, TerrainGenerator* terrainGenerator);
		void            load_ACTN_0000 (Iff& iff, TerrainGenerator* terrainGenerator);
		void            load_ACTN_0001 (Iff& iff, TerrainGenerator* terrainGenerator);
		void            load_ACTN_0002 (Iff& iff, TerrainGenerator* terrainGenerator);

	private:

		Layer (const Layer& rhs);
		Layer& operator= (const Layer& rhs);

	public:

		Layer ();
		~Layer ();

		void _oldBoundaryTest(float &fuzzyTest, float worldX, float worldZ) const;

		void              affect (const float *previousAmountMap, const GeneratorChunkData& generatorChunkData) const;
		virtual void      prepare ();
		virtual void      load (Iff& iff, TerrainGenerator* terrainGenerator);
		virtual void      save (Iff& iff) const;

		virtual bool      prune(unsigned &mapMask, const Rectangle2d &chunkExtentIUO);

		bool              getInvertBoundaries () const;
		void              setInvertBoundaries (bool invertBoundaries);

		bool              getInvertFilters () const;
		void              setInvertFilters (bool invertFilters);

		int               getNumberOfBoundaries () const;
		const Boundary*   getBoundary (int index) const;
		Boundary*         getBoundary (int index);
		void              addBoundary (Boundary* boundary);
		void              removeBoundary (int index);
		void              removeBoundary (const Boundary* boundary, bool doDelete);
		void              promoteBoundary (const Boundary* boundary);
		void              demoteBoundary (const Boundary* boundary);

		int               getNumberOfFilters () const;
		const Filter*     getFilter (int index) const;
		Filter*           getFilter (int index);
		void              addFilter (Filter* filter);
		void              removeFilter (int index);
		void              removeFilter (const Filter* filter, bool doDelete);
		void              promoteFilter (const Filter* filter);
		void              demoteFilter (const Filter* filter);

		int               getNumberOfAffectors () const;
		const Affector*   getAffector (int index) const;
		Affector*         getAffector (int index);
		void              addAffector (Affector* affector);
		void              removeAffector (int index);
		void              removeAffector (const Affector* affector, bool doDelete);

		int               getNumberOfLayers () const;
		const Layer*      getLayer (int index) const;
		Layer*            getLayer (int index);
		void              addLayer (Layer* layer);
		void              removeLayer (int index);
		void              removeLayer (const Layer* layer, bool doDelete);
		void              promoteLayer (const Layer* layer);
		void              demoteLayer (const Layer* layer);

		void              calculateExtent ();
		bool              getUseExtent () const;
		const Rectangle2d& getExtent () const;

		float getModificationHeight () const;
		void  setModificationHeight (float modificationHeight);

		//-- tool use only
		void               setExpanded (bool expanded);
		bool               getExpanded () const;

		void               resetProfileData ();
		const ProfileData& getProfileData () const;

		void               setNotes (const char* notes);
		const char*        getNotes () const;

		bool computeHasPassableAffectors() const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	ShaderGroup            m_shaderGroup;
	FloraGroup             m_floraGroup;
	RadialGroup            m_radialGroup;
	EnvironmentGroup       m_environmentGroup;
	FractalGroup           m_fractalGroup;
	BitmapGroup            m_bitmapGroup;

	ArrayList<Layer*>      m_layerList;

	unsigned               m_sampleMaps;
	bool                   m_hasPassableAffectors;

private:

	mutable bool           m_groupsPrepared;

private:

	void _generateVertexPositions(const GeneratorChunkData& generatorChunkData) const;
	void affect (const GeneratorChunkData& generatorChunkData) const;

	void load_0000 (Iff& iff);

	void saveLayerList (Iff& iff, const ArrayList<Layer*>& layerList) const;
	void saveLayers (Iff& iff) const;

	void calculateExtent ();

private:

	TerrainGenerator (const TerrainGenerator& rhs);
	TerrainGenerator& operator= (const TerrainGenerator& rhs);

public:

	TerrainGenerator ();
	~TerrainGenerator ();

	// set which maps you are interested in.
	// see the TerrainGeneratorMap bit flags
	void setMapsToSample(unsigned TGM_flags);

	const ShaderGroup& getShaderGroup () const;
	ShaderGroup&       getShaderGroup ();

	const FloraGroup&  getFloraGroup () const;
	FloraGroup&        getFloraGroup ();

	const RadialGroup& getRadialGroup () const;
	RadialGroup&       getRadialGroup ();

	const EnvironmentGroup& getEnvironmentGroup () const;
	EnvironmentGroup&       getEnvironmentGroup ();

	const FractalGroup& getFractalGroup () const;
	FractalGroup&       getFractalGroup ();

	const BitmapGroup& getBitmapGroup() const;
	BitmapGroup&       getBitmapGroup();

	//-- layer interface
	int                getNumberOfLayers () const;
	const Layer*       getLayer (int index) const;
	Layer*             getLayer (int index);
	void               addLayer (Layer* layer);
	void               removeLayer (int index);
	void               removeLayer (Layer* layer, bool doDelete);
	void               promoteLayer (const Layer* layer);
	void               demoteLayer (const Layer* layer);

	void               resetProfileData ();

	//-- reset the generator (clears shader groups and flora groups and removes all layers)
	void               reset ();

	//-- load the generator
	void               load (Iff& iff);

	//-- save the generator
	void               save (Iff& iff) const;

	//-- prepare should only be called from the tool and is used to validate the data before generation
	void               prepare ();

	//-- fills out data specific to a chunk
	void               generateChunk (const GeneratorChunkData& generatorChunkData) const;

	bool hasPassableAffectors() const;

public:

	//-- used internally
	static void        generatePlaneAndVertexNormals (const GeneratorChunkData& generatorChunkData);
	static void        synchronizeShaders (const GeneratorChunkData& generatorChunkData);
};  

//===================================================================

inline Tag TerrainGenerator::LayerItem::getTag () const
{
	return m_tag;
}

//-------------------------------------------------------------------

inline const char* TerrainGenerator::LayerItem::getName () const
{
	return m_name;
}

//===================================================================

inline TerrainGeneratorBoundaryType TerrainGenerator::Boundary::getType () const
{
	return m_type;
}

//-------------------------------------------------------------------

inline TerrainGeneratorFeatherFunction TerrainGenerator::Boundary::getFeatherFunction () const
{
	return m_featherFunction;
}

//-------------------------------------------------------------------

inline float TerrainGenerator::Boundary::getFeatherDistance () const
{
	return m_featherDistance;
}

//===================================================================

inline TerrainGeneratorFilterType TerrainGenerator::Filter::getType () const
{
	return m_type;
}

//-------------------------------------------------------------------

inline TerrainGeneratorFeatherFunction TerrainGenerator::Filter::getFeatherFunction () const
{
	return m_featherFunction;
}

//-------------------------------------------------------------------

inline float TerrainGenerator::Filter::getFeatherDistance () const
{
	return m_featherDistance;
}

//===================================================================

inline TerrainGeneratorAffectorType TerrainGenerator::Affector::getType () const
{
	return m_type;
}

//===================================================================

inline bool TerrainGenerator::Layer::getInvertBoundaries () const
{
	return m_invertBoundaries;
}

//-------------------------------------------------------------------

inline bool TerrainGenerator::Layer::getInvertFilters () const
{
	return m_invertFilters;
}

//-------------------------------------------------------------------

inline bool TerrainGenerator::Layer::getExpanded () const
{
	return m_expanded;
}

//-------------------------------------------------------------------

inline const char* TerrainGenerator::Layer::getNotes () const
{
	return m_notes;
}

//-------------------------------------------------------------------

inline bool TerrainGenerator::Layer::getUseExtent () const
{
	return m_useExtent;
}

//-------------------------------------------------------------------

inline const Rectangle2d& TerrainGenerator::Layer::getExtent () const
{
	return m_extent;
}

//-------------------------------------------------------------------

inline float TerrainGenerator::Layer::getModificationHeight () const
{
	return m_modificationHeight;
}

//-------------------------------------------------------------------

inline int TerrainGenerator::Layer::getNumberOfBoundaries () const
{
	return m_boundaryList.getNumberOfElements ();
}

//-------------------------------------------------------------------

inline const TerrainGenerator::Boundary* TerrainGenerator::Layer::getBoundary (int index) const
{
	return m_boundaryList [index];
}

//-------------------------------------------------------------------

inline TerrainGenerator::Boundary* TerrainGenerator::Layer::getBoundary (int index)
{
	return m_boundaryList [index];
}

//-------------------------------------------------------------------

inline int TerrainGenerator::Layer::getNumberOfFilters () const
{
	return m_filterList.getNumberOfElements ();
}

//-------------------------------------------------------------------

inline const TerrainGenerator::Filter* TerrainGenerator::Layer::getFilter (int index) const
{
	return m_filterList [index];
}

//-------------------------------------------------------------------

inline TerrainGenerator::Filter* TerrainGenerator::Layer::getFilter (int index)
{
	return m_filterList [index];
}

//-------------------------------------------------------------------

inline int TerrainGenerator::Layer::getNumberOfAffectors () const
{
	return m_affectorList.getNumberOfElements ();
}

//-------------------------------------------------------------------

inline const TerrainGenerator::Affector* TerrainGenerator::Layer::getAffector (int index) const
{
	return m_affectorList [index];
}

//-------------------------------------------------------------------

inline TerrainGenerator::Affector* TerrainGenerator::Layer::getAffector (int index)
{
	return m_affectorList [index];
}

//-------------------------------------------------------------------

inline int TerrainGenerator::Layer::getNumberOfLayers () const
{
	return m_subLayerList.getNumberOfElements ();
}

//-------------------------------------------------------------------

inline const TerrainGenerator::Layer* TerrainGenerator::Layer::getLayer (int index) const
{
	return m_subLayerList[index];
}

//-------------------------------------------------------------------

inline TerrainGenerator::Layer* TerrainGenerator::Layer::getLayer (int index)
{
	return m_subLayerList[index];
}

//===================================================================

inline int TerrainGenerator::getNumberOfLayers () const
{
	return m_layerList.getNumberOfElements ();
}

//-------------------------------------------------------------------

inline const TerrainGenerator::Layer* TerrainGenerator::getLayer (int index) const
{
	return m_layerList [index];
}

//-------------------------------------------------------------------

inline TerrainGenerator::Layer* TerrainGenerator::getLayer (int index)
{
	return m_layerList [index];
}

//-------------------------------------------------------------------

inline const ShaderGroup& TerrainGenerator::getShaderGroup () const
{
	return m_shaderGroup;
}

//-------------------------------------------------------------------

inline ShaderGroup& TerrainGenerator::getShaderGroup ()
{
	return m_shaderGroup; //lint !e1536 // exposing low access member
}

//-------------------------------------------------------------------

inline const FloraGroup& TerrainGenerator::getFloraGroup () const
{
	return m_floraGroup;
}

//-------------------------------------------------------------------

inline FloraGroup& TerrainGenerator::getFloraGroup ()
{
	return m_floraGroup; //lint !e1536 // exposing low access member
}

//-------------------------------------------------------------------

inline const RadialGroup& TerrainGenerator::getRadialGroup () const
{
	return m_radialGroup;
}

//-------------------------------------------------------------------

inline RadialGroup& TerrainGenerator::getRadialGroup ()
{
	return m_radialGroup; //lint !e1536 // exposing low access member
}

//-------------------------------------------------------------------

inline const EnvironmentGroup& TerrainGenerator::getEnvironmentGroup () const
{
	return m_environmentGroup;
}

//-------------------------------------------------------------------

inline EnvironmentGroup& TerrainGenerator::getEnvironmentGroup ()
{
	return m_environmentGroup; //lint !e1536 // exposing low access member
}

//-------------------------------------------------------------------

inline const FractalGroup& TerrainGenerator::getFractalGroup () const
{
	return m_fractalGroup;
}

//-------------------------------------------------------------------

inline FractalGroup& TerrainGenerator::getFractalGroup ()
{
	return m_fractalGroup; //lint !e1536 // exposing low access member
}

//-------------------------------------------------------------------

inline const BitmapGroup& TerrainGenerator::getBitmapGroup () const
{
	return m_bitmapGroup;
}

//-------------------------------------------------------------------

inline BitmapGroup& TerrainGenerator::getBitmapGroup ()
{
	return m_bitmapGroup; //lint !e1536 // exposing low access member
}

//-------------------------------------------------------------------

inline bool TerrainGenerator::LayerItem::isActive () const
{
	return m_active;
}


//===================================================================

#endif
