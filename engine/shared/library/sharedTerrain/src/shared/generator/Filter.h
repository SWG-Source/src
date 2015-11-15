//===================================================================
//
// Filter.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_Filter_H
#define INCLUDED_Filter_H

//===================================================================

#include "sharedTerrain/TerrainGenerator.h"

//===================================================================
//
// FilterHeight
//
class FilterHeight : public TerrainGenerator::Filter
{
private:

	float lowHeight;
	float highHeight;

private:

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);
	void load_0002 (Iff& iff);

private:

	FilterHeight (const FilterHeight& rhs);
	FilterHeight& operator= (const FilterHeight& rhs);

public:

	FilterHeight ();
	virtual ~FilterHeight ();

	virtual float isWithin (float worldX, float worldZ, int x, int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void  load (Iff& iff);
	virtual void  save (Iff& iff) const;

	float         getLowHeight () const;
	void          setLowHeight (float newLowHeight);

	float         getHighHeight () const;
	void          setHighHeight (float newHighHeight);
};

//-------------------------------------------------------------------

inline float FilterHeight::getLowHeight () const
{
	return lowHeight;
}

//-------------------------------------------------------------------

inline float FilterHeight::getHighHeight () const
{
	return highHeight;
}

//-------------------------------------------------------------------
//
// FilterFractal
//
class FilterFractal : public TerrainGenerator::Filter
{
private:

	//-- not accessible
	mutable const MultiFractal* m_multiFractal;
	mutable int                 m_cachedFamilyId;

	//-- accessible
	int                         m_familyId;
	float                       m_scaleY;
	float                       m_lowFractalLimit;
	float                       m_highFractalLimit;

private:

	void load_0000 (Iff& iff, FractalGroup& fractalGroup);
	void load_0001 (Iff& iff, FractalGroup& fractalGroup);
	void load_0002 (Iff& iff, FractalGroup& fractalGroup);
	void load_0003 (Iff& iff, FractalGroup& fractalGroup);
	void load_0004 (Iff& iff, FractalGroup& fractalGroup);
	void load_0005 (Iff& iff);

private:

	FilterFractal (const FilterFractal& rhs);
	FilterFractal& operator= (const FilterFractal& rhs);

public:

	FilterFractal ();
	virtual ~FilterFractal ();

	virtual float isWithin (float worldX, float worldZ, int x, int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void  load (Iff& iff, FractalGroup& fractalGroup);
	virtual void  save (Iff& iff) const;

	int           getFamilyId () const;
	void          setFamilyId (int id);

	float         getScaleY () const;
	void          setScaleY (float newScaleY);

	float         getLowFractalLimit () const;
	void          setLowFractalLimit (float newLowFractalLimit);

	float         getHighFractalLimit () const;
	void          setHighFractalLimit (float newHighFractalLimit);
};

//-------------------------------------------------------------------

inline int FilterFractal::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------

inline float FilterFractal::getScaleY () const
{
	return m_scaleY;
}

//-------------------------------------------------------------------

inline float FilterFractal::getLowFractalLimit () const
{
	return m_lowFractalLimit;
}

//-------------------------------------------------------------------

inline float FilterFractal::getHighFractalLimit () const
{
	return m_highFractalLimit;
}

//-------------------------------------------------------------------
//
// FilterSlope
//
class FilterSlope : public TerrainGenerator::Filter
{
private:

	//-- accessible
	float maximumAngle;
	float minimumAngle;
    
	//-- not accessible
	float sinMaxAngle;
	float sinMinAngle;

private:

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);
	void load_0002 (Iff& iff);

private:

	FilterSlope (const FilterSlope& rhs);
	FilterSlope& operator= (const FilterSlope& rhs);

public:

	FilterSlope ();
	virtual ~FilterSlope ();

	virtual float isWithin (float worldX, float worldZ, int x, int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void  load (Iff& iff);
	virtual void  save (Iff& iff) const;
	virtual bool  needsNormals () const;

	float         getMinimumAngle () const;
	void          setMinimumAngle (float newMinimumAngle);

	float         getMaximumAngle () const;
	void          setMaximumAngle (float newMaximumAngle);
};

//-------------------------------------------------------------------

inline float FilterSlope::getMinimumAngle () const
{
	return minimumAngle;
}

//-------------------------------------------------------------------

inline float FilterSlope::getMaximumAngle () const
{
	return maximumAngle;
}

//-------------------------------------------------------------------
//
// FilterDirection
//
class FilterDirection : public TerrainGenerator::Filter
{
private:

	//-- accessible
	float maximumAngle;
	float minimumAngle;

    //-- inaccessible
	float maximumFeatherAngle;
	float minimumFeatherAngle;
    
private:

	void load_0000 (Iff& iff);

private:

	FilterDirection (const FilterDirection& rhs);
	FilterDirection& operator= (const FilterDirection& rhs);

public:

	FilterDirection ();
	virtual ~FilterDirection ();

	virtual float isWithin (float worldX, float worldZ, int x, int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void  load (Iff& iff);
	virtual void  save (Iff& iff) const;
	virtual bool  needsNormals () const;

	float         getMinimumAngle () const;
	void          setMinimumAngle (float newMinimumAngle);

	float         getMaximumAngle () const;
	void          setMaximumAngle (float newMaximumAngle);
};

//-------------------------------------------------------------------

inline float FilterDirection::getMinimumAngle () const
{
	return minimumAngle;
}

//-------------------------------------------------------------------

inline float FilterDirection::getMaximumAngle () const
{
	return maximumAngle;
}

//-------------------------------------------------------------------
//
// FilterShader
//
class FilterShader : public TerrainGenerator::Filter
{
private:

	int  familyId;

private:

	void load_0000 (Iff& iff);

private:

	FilterShader (const FilterShader& rhs);
	FilterShader& operator= (const FilterShader& rhs);

public:

	FilterShader ();
	virtual ~FilterShader ();

	virtual float isWithin (float worldX, float worldZ, int x, int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void  load (Iff& iff);
	virtual void  save (Iff& iff) const;
	virtual bool  needsShaders () const;

	int           getFamilyId () const;
	void          setFamilyId (int id);
};

//-------------------------------------------------------------------

inline int FilterShader::getFamilyId () const
{
	return familyId;
}







//-------------------------------------------------------------------
//
// FilterBitmap
//
class BitmapGroup;
class FilterBitmap : public TerrainGenerator::Filter
{
private:

	int m_familyId;
	float m_lowBitmapLimit;
	float m_highBitmapLimit;
	Rectangle2d m_extent;

	float m_gain;

private:

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);

private:

	FilterBitmap (const FilterBitmap& rhs);
	FilterBitmap& operator= (const FilterBitmap& rhs);

public:

	FilterBitmap ();
	virtual ~FilterBitmap ();

	virtual float isWithin (float worldX, float worldZ, int x, int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void  load (Iff& iff/*, BitmapGroup& bitmapGroup*/);
	virtual void  save (Iff& iff) const;

	int           getFamilyId () const;
	void          setFamilyId (int id);

	float         getLowBitmapLimit () const;
	void          setLowBitmapLimit (float newLowBitmapLimit);

	float         getHighBitmapLimit () const;
	void          setHighBitmapLimit (float newHighBitmapLimit);

	float         getGain () const;
	void          setGain (float newValue);

	void          setExtent(const Rectangle2d& rect);

};

//-------------------------------------------------------------------

inline int FilterBitmap::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------

inline float FilterBitmap::getLowBitmapLimit () const
{
	return m_lowBitmapLimit;
}

//-------------------------------------------------------------------

inline float FilterBitmap::getHighBitmapLimit () const
{
	return m_highBitmapLimit;
}

//-------------------------------------------------------------------

inline float FilterBitmap::getGain () const
{
	return m_gain;
}

//===================================================================

#endif
