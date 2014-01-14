//===================================================================
//
// AffectorRibbon.h
//
// copyright 2005, Sony Online Entertainment
//

//===================================================================

#ifndef INCLUDED_AffectorRibbon_H
#define INCLUDED_AffectorRibbon_H

//===================================================================

#include "sharedTerrain/Affector.h"

//===================================================================

class AffectorRibbon : public AffectorBoundaryPoly
{
public:
	struct Quad
	{
		Vector   points [4];
	};

public:

	AffectorRibbon ();
	virtual ~AffectorRibbon ();

	virtual void      prepare ();
	virtual void      affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void      load (Iff& iff);
	virtual void      save (Iff& iff) const;
	virtual bool      affectsHeight () const;
	virtual bool      affectsShader () const;
	virtual void      createHeightData () {};
	virtual unsigned  getAffectedMaps() const;

	void                setWaterShaderSize(float val);
	float               getWaterShaderSize() const;
	void                setVelocity(float val);
	float               getVelocity() const;
	void				setCapWidth (float newCapWidth);
	float				getCapWidth () const;

	void createQuadList (ArrayList<Quad>& quadList) const;

	const char*       getRibbonWaterShaderTemplateName () const;
	void setRibbonWaterShaderTemplateName (const char* newRibbonShaderTemplateName);
	TerrainGeneratorWaterType getWaterType () const;
	void                setWaterType (TerrainGeneratorWaterType newWaterType);
	const ArrayList<float>& getHeightList () const;
	void copyHeightList (const ArrayList<float>& newHeightList);
	void createInitialHeightList ();
	void generateEndCapPointList ();
	const ArrayList<Vector2d>& getEndCapPointList () const;

	int               getTerrainShaderFamilyId () const;
	void              setTerrainShaderFamilyId (int newFamilyId);
	TerrainGeneratorFeatherFunction getFeatherFunctionTerrainShader () const;
	void                setFeatherFunctionTerrainShader (TerrainGeneratorFeatherFunction newFeatherFunction);
	float               getFeatherDistanceTerrainShader () const;
	void                setFeatherDistanceTerrainShader (float newFeatherDistance);
	const Rectangle2d& getEndCapExtent() const;


private:

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);
	void load_0002 (Iff& iff);
	void load_0003 (Iff& iff);
	void load_0004 (Iff& iff);
	void load_0005 (Iff& iff);
	virtual void recalculate ();

private:

	AffectorRibbon (const AffectorRibbon& rhs);
	AffectorRibbon& operator= (const AffectorRibbon& rhs);

private:

	mutable int                     m_cachedTerrainShaderFamilyId;
	mutable ShaderGroup::Info       m_cachedSgi;

	float							m_waterShaderSize;
	float							m_velocity;
	float							m_capWidth;
	int                             m_terrainShaderFamilyId;
	TerrainGeneratorFeatherFunction m_featherFunctionTerrainShader;
	float                           m_featherDistanceTerrainShader;
	char*                           m_ribbonWaterShaderTemplateName;
	TerrainGeneratorWaterType m_waterType;
	ArrayList<float> m_heightList;
	ArrayList<Vector2d> m_endCapPointList;
	Rectangle2d m_endCapExtent;

};

//-------------------------------------------------------------------

inline float AffectorRibbon::getWaterShaderSize () const
{
	return m_waterShaderSize;
}

//-------------------------------------------------------------------

inline float AffectorRibbon::getVelocity () const
{
	return m_velocity;
}

//-------------------------------------------------------------------

inline const char* AffectorRibbon::getRibbonWaterShaderTemplateName () const
{
	return m_ribbonWaterShaderTemplateName;
}

//-------------------------------------------------------------------

inline TerrainGeneratorWaterType AffectorRibbon::getWaterType () const
{
	return m_waterType;
}

//-------------------------------------------------------------------

inline const ArrayList<float>& AffectorRibbon::getHeightList () const
{
	return m_heightList;
}

//-------------------------------------------------------------------

inline float AffectorRibbon::getCapWidth () const
{
	return m_capWidth;
}

//-------------------------------------------------------------------

inline const ArrayList<Vector2d>& AffectorRibbon::getEndCapPointList () const
{
	return m_endCapPointList;
}

//-------------------------------------------------------------------

inline int AffectorRibbon::getTerrainShaderFamilyId () const
{
	return m_terrainShaderFamilyId;
}

//-------------------------------------------------------------------

inline TerrainGeneratorFeatherFunction AffectorRibbon::getFeatherFunctionTerrainShader () const
{
	return m_featherFunctionTerrainShader;
}

//-------------------------------------------------------------------

inline float AffectorRibbon::getFeatherDistanceTerrainShader () const
{
	return m_featherDistanceTerrainShader;
}

//-------------------------------------------------------------------

inline const Rectangle2d& AffectorRibbon::getEndCapExtent() const
{
	return m_endCapExtent;
}

//===================================================================

#endif
