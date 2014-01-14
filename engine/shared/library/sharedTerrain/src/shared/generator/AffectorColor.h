//===================================================================
//
// AffectorColor.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_AffectorColor_H
#define INCLUDED_AffectorColor_H

//===================================================================

#include "sharedTerrain/TerrainGenerator.h"

class Image;

//===================================================================

class AffectorColorConstant : public TerrainGenerator::Affector
{
private:

	//-- accessible
	TerrainGeneratorOperation operation;
	PackedRgb                 color;

private:

	void load_0000 (Iff& iff);

private:

	AffectorColorConstant (const AffectorColorConstant& rhs);
	AffectorColorConstant& operator= (const AffectorColorConstant& rhs);

public:

	AffectorColorConstant ();
	virtual ~AffectorColorConstant ();

	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;
	virtual unsigned          getAffectedMaps() const;

	TerrainGeneratorOperation getOperation () const;
	void                      setOperation (TerrainGeneratorOperation newOperation);

	const PackedRgb&          getColor () const;
	void                      setColor (const PackedRgb& newColor);
};

//-------------------------------------------------------------------

inline TerrainGeneratorOperation AffectorColorConstant::getOperation () const
{
	return operation;
}

//-------------------------------------------------------------------

inline const PackedRgb& AffectorColorConstant::getColor () const
{
	return color;
}

//-------------------------------------------------------------------

class AffectorColorRampHeight : public TerrainGenerator::Affector
{
private:

	//-- not accessible
	Image*                    image;

	//-- accessible
	TerrainGeneratorOperation operation;
	float                     lowHeight;
	float                     highHeight;
	std::string*              imageName;

private:

	void load_0000 (Iff& iff);

private:

	AffectorColorRampHeight (const AffectorColorRampHeight& rhs);
	AffectorColorRampHeight& operator= (const AffectorColorRampHeight& rhs);

public:

	AffectorColorRampHeight ();
	virtual ~AffectorColorRampHeight ();

	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;
	virtual unsigned          getAffectedMaps() const;

	TerrainGeneratorOperation getOperation () const;
	void                      setOperation (TerrainGeneratorOperation newOperation);

	float                     getLowHeight () const;
	void                      setLowHeight (float newLowHeight);

	float                     getHighHeight () const;
	void                      setHighHeight (float newHighHeight);

	const std::string&        getImageName () const;
	void                      setImage (const std::string& newImageName);
};

//-------------------------------------------------------------------

inline TerrainGeneratorOperation AffectorColorRampHeight::getOperation () const
{
	return operation;
}

//-------------------------------------------------------------------

inline float AffectorColorRampHeight::getLowHeight () const
{
	return lowHeight;
}

//-------------------------------------------------------------------

inline float AffectorColorRampHeight::getHighHeight () const
{
	return highHeight;
}

//-------------------------------------------------------------------

inline const std::string& AffectorColorRampHeight::getImageName () const
{
	return *imageName;
}

//-------------------------------------------------------------------

class AffectorColorRampFractal : public TerrainGenerator::Affector
{
private:

	//-- not accessible
	mutable const MultiFractal* m_multiFractal;
	mutable int                 m_cachedFamilyId;
	Image*                      image;

	//-- accessible
	int                         m_familyId;
	TerrainGeneratorOperation   operation;
	std::string*                imageName;

private:

	void                        load_0000 (Iff& iff, FractalGroup& fractalGroup);
	void                        load_0001 (Iff& iff);

private:

	AffectorColorRampFractal (const AffectorColorRampFractal& rhs);
	AffectorColorRampFractal& operator= (const AffectorColorRampFractal& rhs);

public:

	AffectorColorRampFractal ();
	virtual ~AffectorColorRampFractal ();

	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void              load (Iff& iff, FractalGroup& fractalGroup);
	virtual void              save (Iff& iff) const;
	virtual unsigned          getAffectedMaps() const;

	TerrainGeneratorOperation getOperation () const;
	void         setOperation (TerrainGeneratorOperation newOperation);

	const std::string& getImageName () const;
	void         setImage (const std::string& newImageName);

	int          getFamilyId () const;
	void         setFamilyId (int id);
};

//-------------------------------------------------------------------

inline int AffectorColorRampFractal::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------

inline TerrainGeneratorOperation AffectorColorRampFractal::getOperation () const
{
	return operation;
}

//-------------------------------------------------------------------

inline const std::string& AffectorColorRampFractal::getImageName () const
{
	return *imageName;
}

//===================================================================

#endif
