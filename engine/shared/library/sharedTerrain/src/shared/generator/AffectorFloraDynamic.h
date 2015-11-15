//===================================================================
//
// AffectorFloraDynamic.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_AffectorFloraDynamic_H
#define INCLUDED_AffectorFloraDynamic_H

//===================================================================

#include "sharedTerrain/TerrainGenerator.h"

//===================================================================

class AffectorFloraDynamic : public TerrainGenerator::Affector
{
private:

	//-- not accessible
	mutable int               cachedFamilyId;
	mutable RadialGroup::Info cachedRgi;
	mutable float             cachedDensity;

	//-- accessible
	int                       familyId;
	TerrainGeneratorOperation operation;
	bool                      removeAll;
	bool                      densityOverride;
	float                     densityOverrideDensity;

private:

	void               load_0000 (Iff& iff);
	void               load_0001 (Iff& iff);
	void               load_0002 (Iff& iff);

private:

	AffectorFloraDynamic (const AffectorFloraDynamic& rhs);
	AffectorFloraDynamic& operator= (const AffectorFloraDynamic& rhs);

protected:

	virtual Array2d<RadialGroup::Info>* getFloraMap (const TerrainGenerator::GeneratorChunkData& generatorChunkData) const=0;

	void _legacyAffect(float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;

public:

	AffectorFloraDynamic (Tag newTag, TerrainGeneratorAffectorType newType);
	virtual ~AffectorFloraDynamic ();

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

inline int AffectorFloraDynamic::getFamilyId () const
{
	return familyId;
}

//-------------------------------------------------------------------

inline TerrainGeneratorOperation AffectorFloraDynamic::getOperation () const
{
	return operation;
}

//-------------------------------------------------------------------

inline bool AffectorFloraDynamic::getRemoveAll () const
{
	return removeAll;
}

//-------------------------------------------------------------------

inline bool AffectorFloraDynamic::getDensityOverride () const
{
	return densityOverride;
}

//-------------------------------------------------------------------

inline float AffectorFloraDynamic::getDensityOverrideDensity () const
{
	return densityOverrideDensity;
}

//===================================================================

class AffectorFloraDynamicNearConstant : public AffectorFloraDynamic
{
private:

	AffectorFloraDynamicNearConstant (const AffectorFloraDynamicNearConstant& rhs);
	AffectorFloraDynamicNearConstant& operator= (const AffectorFloraDynamicNearConstant& rhs);

private:

	virtual Array2d<RadialGroup::Info>* getFloraMap (const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;

public:

	AffectorFloraDynamicNearConstant ();
	virtual ~AffectorFloraDynamicNearConstant ();

	virtual unsigned          getAffectedMaps() const;
};

//-------------------------------------------------------------------

inline Array2d<RadialGroup::Info>* AffectorFloraDynamicNearConstant::getFloraMap (const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	return generatorChunkData.floraDynamicNearMap;
}

//===================================================================

class AffectorFloraDynamicFarConstant : public AffectorFloraDynamic
{
private:

	AffectorFloraDynamicFarConstant (const AffectorFloraDynamicFarConstant& rhs);
	AffectorFloraDynamicFarConstant& operator= (const AffectorFloraDynamicFarConstant& rhs);

private:

	virtual Array2d<RadialGroup::Info>* getFloraMap (const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;

public:

	AffectorFloraDynamicFarConstant ();
	virtual ~AffectorFloraDynamicFarConstant ();

	virtual unsigned          getAffectedMaps() const;
};

//-------------------------------------------------------------------

inline Array2d<RadialGroup::Info>* AffectorFloraDynamicFarConstant::getFloraMap (const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	return generatorChunkData.floraDynamicFarMap;
}

//===================================================================

#endif
