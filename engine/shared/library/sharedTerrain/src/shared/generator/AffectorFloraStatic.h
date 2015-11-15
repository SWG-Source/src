//===================================================================
//
// AffectorFloraStatic.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//===================================================================

#ifndef INCLUDED_AffectorFloraStatic_H
#define INCLUDED_AffectorFloraStatic_H

//===================================================================

#include "sharedTerrain/TerrainGenerator.h"

//===================================================================

class AffectorFloraStatic : public TerrainGenerator::Affector
{
protected:

	//-- not accessible
	mutable int               cachedFamilyId;
	mutable FloraGroup::Info  cachedFgi;
	mutable float             cachedDensity;

	//-- accessible
	int                       familyId;
	TerrainGeneratorOperation operation;
	bool                      removeAll;
	bool                      densityOverride;
	float                     densityOverrideDensity;

private:

	void                      load_0000 (Iff& iff);
	void                      load_0001 (Iff& iff);
	void                      load_0002 (Iff& iff);
	void                      load_0003 (Iff& iff);
	void                      load_0004 (Iff& iff);

private:

	AffectorFloraStatic (const AffectorFloraStatic& rhs);
	AffectorFloraStatic& operator= (const AffectorFloraStatic& rhs);

protected:

	virtual Array2d<FloraGroup::Info>* getFloraMap (const TerrainGenerator::GeneratorChunkData& generatorChunkData) const=0;

	void _legacyAffect(float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;

public:

	AffectorFloraStatic (Tag newTag, TerrainGeneratorAffectorType newType);
	virtual ~AffectorFloraStatic ();

	virtual void              prepare ();
	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;

	int          getFamilyId () const;
	void         setFamilyId (int newFamilyId);

	TerrainGeneratorOperation getOperation () const;
	void         setOperation (TerrainGeneratorOperation newOperation);

	bool         getRemoveAll () const;
	void         setRemoveAll (bool newRemoveAll);

	bool         getDensityOverride () const;
	void         setDensityOverride (bool newDensityOverride);
	float        getDensityOverrideDensity () const;
	void         setDensityOverrideDensity (float newDensityOverrideDensity);
};

//-------------------------------------------------------------------

inline int AffectorFloraStatic::getFamilyId () const
{
	return familyId;
}

//-------------------------------------------------------------------

inline TerrainGeneratorOperation AffectorFloraStatic::getOperation () const
{
	return operation;
}

//-------------------------------------------------------------------

inline bool AffectorFloraStatic::getRemoveAll () const
{
	return removeAll;
}

//-------------------------------------------------------------------

inline bool AffectorFloraStatic::getDensityOverride () const
{
	return densityOverride;
}

//-------------------------------------------------------------------

inline float AffectorFloraStatic::getDensityOverrideDensity () const
{
	return densityOverrideDensity;
}

//-------------------------------------------------------------------

class AffectorFloraStaticCollidableConstant : public AffectorFloraStatic
{
private:

	AffectorFloraStaticCollidableConstant (const AffectorFloraStaticCollidableConstant& rhs);
	AffectorFloraStaticCollidableConstant& operator= (const AffectorFloraStaticCollidableConstant& rhs);

private:

	virtual Array2d<FloraGroup::Info>* getFloraMap (const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;

public:

	AffectorFloraStaticCollidableConstant ();
	virtual ~AffectorFloraStaticCollidableConstant ();
	virtual unsigned          getAffectedMaps() const;
};

//-------------------------------------------------------------------

inline Array2d<FloraGroup::Info>* AffectorFloraStaticCollidableConstant::getFloraMap (const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	return generatorChunkData.floraStaticCollidableMap;
}

//-------------------------------------------------------------------

class AffectorFloraStaticNonCollidableConstant : public AffectorFloraStatic
{
private:

	AffectorFloraStaticNonCollidableConstant (const AffectorFloraStaticNonCollidableConstant& rhs);
	AffectorFloraStaticNonCollidableConstant& operator= (const AffectorFloraStaticNonCollidableConstant& rhs);

private:

	virtual Array2d<FloraGroup::Info>* getFloraMap (const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;

public:

	AffectorFloraStaticNonCollidableConstant ();
	virtual ~AffectorFloraStaticNonCollidableConstant ();
	virtual unsigned          getAffectedMaps() const;
};

//-------------------------------------------------------------------

inline Array2d<FloraGroup::Info>* AffectorFloraStaticNonCollidableConstant::getFloraMap (const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	return generatorChunkData.floraStaticNonCollidableMap;
}

//===================================================================

#endif
