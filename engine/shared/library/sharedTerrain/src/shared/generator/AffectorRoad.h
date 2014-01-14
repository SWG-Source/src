//===================================================================
//
// AffectorRoad.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//===================================================================

#ifndef INCLUDED_AffectorRoad_H
#define INCLUDED_AffectorRoad_H

//===================================================================

#include "sharedTerrain/Affector.h"

//===================================================================

class AffectorRoad : public AffectorBoundaryPoly
{
public:

	AffectorRoad ();
	virtual ~AffectorRoad ();

	virtual void      prepare ();
	virtual void      affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void      load (Iff& iff);
	virtual void      save (Iff& iff) const;
	virtual bool      affectsHeight () const;
	virtual bool      affectsShader () const;
	virtual unsigned  getAffectedMaps() const;
	virtual void      createHeightData ();

	int               getFamilyId () const;
	void              setFamilyId (int newFamilyId);

	TerrainGeneratorFeatherFunction getFeatherFunctionShader () const;
	void                setFeatherFunctionShader (TerrainGeneratorFeatherFunction newFeatherFunction);
	float               getFeatherDistanceShader () const;
	void                setFeatherDistanceShader (float newFeatherDistance);

	float getRampedHeight (const float worldX, const float worldZ,const float terrainHeight) const;

	bool getHasFixedHeights() const;
	void setHasFixedHeights(const bool newVal);
	void createInitialHeightList ();
	void clearHeightList ();
	const ArrayList<float>& getHeightList () const;
	void copyHeightList (const ArrayList<float>& newHeightList);

private:

	void _legacyAffect (const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);
	void load_0002 (Iff& iff);
	void load_0003 (Iff& iff);
	void load_0004 (Iff& iff);
	void load_0005 (Iff& iff);
	void load_0006 (Iff& iff);

private:

	AffectorRoad (const AffectorRoad& rhs);
	AffectorRoad& operator= (const AffectorRoad& rhs);

private:

	mutable int                     m_cachedFamilyId;
	mutable ShaderGroup::Info       m_cachedSgi;

	//-- accessible
	int                             m_familyId;
	TerrainGeneratorFeatherFunction m_featherFunctionShader;
	float                           m_featherDistanceShader;
	bool m_hasFixedHeights;
	ArrayList<float> m_heightList;
};

//-------------------------------------------------------------------

inline int AffectorRoad::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------

inline TerrainGeneratorFeatherFunction AffectorRoad::getFeatherFunctionShader () const
{
	return m_featherFunctionShader;
}

//-------------------------------------------------------------------

inline float AffectorRoad::getFeatherDistanceShader () const
{
	return m_featherDistanceShader;
}

//-------------------------------------------------------------------

inline bool AffectorRoad::getHasFixedHeights () const
{
	return m_hasFixedHeights;
}

//-------------------------------------------------------------------

inline const ArrayList<float>& AffectorRoad::getHeightList () const
{
	return m_heightList;
}

//===================================================================

#endif
