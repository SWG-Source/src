//===================================================================
//
// AffectorHeight.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_AffectorHeight_H
#define INCLUDED_AffectorHeight_H

//===================================================================

#include "sharedTerrain/TerrainGenerator.h"

//===================================================================

class AffectorHeightConstant : public TerrainGenerator::Affector
{
private:

	//-- accessible
	TerrainGeneratorOperation operation;
	float                     height;

private:

	void                      load_0000 (Iff& iff);

private:

	AffectorHeightConstant (const AffectorHeightConstant& rhs);
	AffectorHeightConstant& operator= (const AffectorHeightConstant& rhs);

public:

	AffectorHeightConstant ();
	virtual ~AffectorHeightConstant ();

	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual bool              affectsHeight () const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;
	virtual unsigned          getAffectedMaps() const;

	TerrainGeneratorOperation getOperation () const;
	void                      setOperation (TerrainGeneratorOperation newOperation);

	float                     getHeight () const;
	void                      setHeight (float newHeight);
};

//-------------------------------------------------------------------

inline TerrainGeneratorOperation AffectorHeightConstant::getOperation () const
{
	return operation;
}

//-------------------------------------------------------------------

inline float AffectorHeightConstant::getHeight () const
{
	return height;
}

//===================================================================

class AffectorHeightFractal : public TerrainGenerator::Affector
{
private:

	//-- not accessible
	mutable const MultiFractal* m_multiFractal;
	mutable int                 m_cachedFamilyId;

	//-- accessible
	int                         m_familyId;
	float                        m_scaleY;
	TerrainGeneratorOperation   m_operation;

private:

	void                      load_0000 (Iff& iff, FractalGroup& fractalGroup);
	void                      load_0001 (Iff& iff, FractalGroup& fractalGroup);
	void                      load_0002 (Iff& iff, FractalGroup& fractalGroup);
	void                      load_0003 (Iff& iff);

private:

	AffectorHeightFractal (const AffectorHeightFractal& rhs);
	AffectorHeightFractal& operator= (const AffectorHeightFractal& rhs);

public:

	AffectorHeightFractal ();
	virtual ~AffectorHeightFractal ();

	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual bool              affectsHeight () const;
	virtual void              load (Iff& iff, FractalGroup& fractalGroup);
	virtual void              save (Iff& iff) const;
	virtual unsigned          getAffectedMaps() const;

	TerrainGeneratorOperation getOperation () const;
	void          setOperation (TerrainGeneratorOperation newOperation);

	int           getFamilyId () const;
	void          setFamilyId (int id);

	float         getScaleY () const;
	void          setScaleY (float newScaleY);
};

//===================================================================

inline TerrainGeneratorOperation AffectorHeightFractal::getOperation () const
{
	return m_operation;
}

//-------------------------------------------------------------------

inline int AffectorHeightFractal::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------

inline float AffectorHeightFractal::getScaleY () const
{
	return m_scaleY;
}

//===================================================================

class AffectorHeightTerrace : public TerrainGenerator::Affector
{
private:

	float          height;
	float          fraction;

private:

	void          load_0000 (Iff& iff);
	void          load_0001 (Iff& iff);
	void          load_0002 (Iff& iff);
	void          load_0003 (Iff& iff);
	void          load_0004 (Iff& iff);

private:

	AffectorHeightTerrace (const AffectorHeightTerrace& rhs);
	AffectorHeightTerrace& operator= (const AffectorHeightTerrace& rhs);

public:

	AffectorHeightTerrace ();
	virtual ~AffectorHeightTerrace ();

	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual bool              affectsHeight () const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;
	virtual unsigned          getAffectedMaps() const;

	float         getHeight () const;
	void          setHeight (float newHeight);

	float         getFraction () const;
	void          setFraction (float newFraction);
};

//-------------------------------------------------------------------

inline float AffectorHeightTerrace::getHeight () const
{
	return height;
}

//-------------------------------------------------------------------

inline float AffectorHeightTerrace::getFraction () const
{
	return fraction;
}

//===================================================================

#endif
