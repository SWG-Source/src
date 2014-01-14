//===================================================================
//
// AffectorRiver.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_AffectorRiver_H
#define INCLUDED_AffectorRiver_H

//===================================================================

#include "sharedFractal/MultiFractal.h"
#include "sharedTerrain/Affector.h"
#include "sharedMath/Vector2d.h"

//===================================================================

class AffectorRiver : public AffectorBoundaryPoly
{
public:

	struct WaterTable
	{
		Vector   points [4];
		Vector2d direction;
	};

public:

	AffectorRiver ();
	virtual ~AffectorRiver ();

	virtual void      prepare ();
	virtual void      affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void      load (Iff& iff);
	virtual void      save (Iff& iff) const;
	virtual bool      affectsHeight () const;
	virtual bool      affectsShader () const;
	virtual unsigned  getAffectedMaps() const;
	virtual void      createHeightData ();

	void              setTrenchDepth (float newTrenchDepth);
	void              setVelocity (float newVelocity);

	float             getTrenchDepth () const;
	float             getVelocity () const;

	int               getBankFamilyId () const;
	void              setBankFamilyId (int newBankFamilyId);
	int               getBottomFamilyId () const;
	void              setBottomFamilyId (int newBottomFamilyId);

	void              createWaterTables (ArrayList<WaterTable>& waterTableList) const;

	bool              getHasLocalWaterTable () const;
	void              setHasLocalWaterTable (bool newHasWater);
	float             getLocalWaterTableShaderSize () const;
	void              setLocalWaterTableShaderSize (float newLocalWaterTableShaderSize);
	const char*       getLocalWaterTableShaderTemplateName () const;
	void              setLocalWaterTableShaderTemplateName (const char* newLocalWaterTableShaderTemplateName);
	float             getLocalWaterTableDepth () const;
	void              setLocalWaterTableDepth (float newLocalWaterTableDepth);
	float             getLocalWaterTableWidth () const;
	void              setLocalWaterTableWidth (float newLocalWaterTableWidth);
	TerrainGeneratorWaterType getWaterType () const;
	void                setWaterType (TerrainGeneratorWaterType newWaterType);


private:

	void _legacyAffect(const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);
	void load_0002 (Iff& iff);
	void load_0003 (Iff& iff);
	void load_0004 (Iff& iff);
	void load_0005 (Iff& iff);
	void load_0006 (Iff& iff);

private:

	AffectorRiver (const AffectorRiver& rhs);
	AffectorRiver& operator= (const AffectorRiver& rhs);

private:

	mutable int                     m_cachedBankFamilyId;
	mutable ShaderGroup::Info       m_cachedBankSgi;
	mutable int                     m_cachedBottomFamilyId;
	mutable ShaderGroup::Info       m_cachedBottomSgi;

	MultiFractal                    m_multiFractal;

	//-- accessible
	int                             m_bankFamilyId;
	int                             m_bottomFamilyId;
	float                           m_trenchDepth;
	bool                            m_hasLocalWaterTable;
	float                           m_localWaterTableDepth;
	float                           m_localWaterTableWidth;
	float                           m_localWaterTableShaderSize;
	char*                           m_localWaterTableShaderTemplateName;
	float                           m_velocity;
	TerrainGeneratorWaterType m_waterType;

};

//-------------------------------------------------------------------

inline int AffectorRiver::getBankFamilyId () const
{
	return m_bankFamilyId;
}

//-------------------------------------------------------------------

inline int AffectorRiver::getBottomFamilyId () const
{
	return m_bottomFamilyId;
}

//-------------------------------------------------------------------

inline float AffectorRiver::getTrenchDepth () const
{
	return m_trenchDepth;
}

//-------------------------------------------------------------------

inline float AffectorRiver::getVelocity () const
{
	return m_velocity;
}

//-------------------------------------------------------------------

inline float AffectorRiver::getLocalWaterTableShaderSize () const
{
	return m_localWaterTableShaderSize;
}

//-------------------------------------------------------------------

inline const char* AffectorRiver::getLocalWaterTableShaderTemplateName () const
{
	return m_localWaterTableShaderTemplateName;
}

//-------------------------------------------------------------------

inline bool AffectorRiver::getHasLocalWaterTable () const
{
	return m_hasLocalWaterTable;
}

//-------------------------------------------------------------------

inline float AffectorRiver::getLocalWaterTableDepth () const
{
	return m_localWaterTableDepth;
}

//-------------------------------------------------------------------

inline float AffectorRiver::getLocalWaterTableWidth () const
{
	return m_localWaterTableWidth;
}

//-------------------------------------------------------------------

inline TerrainGeneratorWaterType AffectorRiver::getWaterType () const
{
	return m_waterType;
}


//===================================================================

#endif
