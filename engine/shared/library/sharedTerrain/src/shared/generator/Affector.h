//===================================================================
//
// Affector.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_Affector_H
#define INCLUDED_Affector_H

//===================================================================

#include "sharedMath/Vector2d.h"
#include "sharedTerrain/TerrainGenerator.h"
#include "sharedTerrain/HeightData.h"

//===================================================================

class AffectorExclude : public TerrainGenerator::Affector
{
public:

	AffectorExclude ();
	virtual ~AffectorExclude ();

	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;
	virtual unsigned          getAffectedMaps() const;

private:

	void load_0000 (Iff& iff);

private:

	AffectorExclude (const AffectorExclude& rhs);
	AffectorExclude& operator= (const AffectorExclude& rhs);
};

//===================================================================

class AffectorBoundaryPoly : public TerrainGenerator::Affector
{
public:

	AffectorBoundaryPoly (Tag newTag, TerrainGeneratorAffectorType newType);
	virtual ~AffectorBoundaryPoly ()=0;

	virtual float isWithin (float worldX, float worldZ) const;
	TerrainGeneratorFeatherFunction getFeatherFunction () const;
	void                setFeatherFunction (TerrainGeneratorFeatherFunction newFeatherFunction);
	float               getFeatherDistance () const;
	void                setFeatherDistance (float newFeatherDistance);

	void                rotate (float angle);
	void                rotate (float angle, const Vector2d& center);
	void                translate (const Vector2d& translation);
	void                scale (float scalar);
	void                expand (Rectangle2d& extent) const;
	const Vector2d      getCenter () const;

	const ArrayList<Vector2d>& getPointList () const;
	int                 getNumberOfPoints () const;
	const Vector2d&     getPoint (int index) const;
	void                addPoint (const Vector2d& point);
	void                replacePoint (int index, const Vector2d& point);
	void                removePoint (int index);
	void                clearPointList ();
	void                copyPointList (const ArrayList<Vector2d>& newPointList);

	void                setWidth (float newWidth);
	float               getWidth () const;

	const Rectangle2d&  getExtent () const;

	const HeightData&   getHeightData () const;
	void                clearHeightData ();
	void                addSegmentHeightData ();
	void                addPointHeightData (const Vector& point);
	virtual void        createHeightData ()=0;

	//--
	static bool         isEnabled ();
	static void         enable ();
	static void         disable ();

protected:

	struct FindData
	{
	public:

		float height;
		float distanceToCenter;
		float t;
		float length;

	public:

		void reset ();
	};

protected:

	virtual void recalculate ();

	//-- find the closest height along the list of points to position (returns t along entire list of segments)
	bool find (const Vector2d& point, const float width, FindData& result,bool ignoreHeight = false) const;

protected:

	TerrainGeneratorFeatherFunction m_featherFunction;
	float                           m_featherDistance;
	ArrayList<Vector2d>             m_pointList;
	ArrayList<float>                m_lengths;
	ArrayList<float>                m_lengthTotals;
	float                           m_width;
	Rectangle2d                     m_extent;
	HeightData                      m_heightData;

private:

	AffectorBoundaryPoly (const AffectorBoundaryPoly& rhs);
	AffectorBoundaryPoly& operator= (const AffectorBoundaryPoly& rhs);

private:

	static bool                     ms_enabled;
};

//===================================================================

inline const ArrayList<Vector2d>& AffectorBoundaryPoly::getPointList () const
{
	return m_pointList;
}

//-------------------------------------------------------------------

inline int AffectorBoundaryPoly::getNumberOfPoints () const
{
	return m_pointList.getNumberOfElements ();
}

//-------------------------------------------------------------------

inline const Vector2d& AffectorBoundaryPoly::getPoint (int index) const
{
	return m_pointList [index];
}

//-------------------------------------------------------------------

inline const Vector2d AffectorBoundaryPoly::getCenter () const
{
	return m_extent.getCenter ();
}

//-------------------------------------------------------------------

inline float AffectorBoundaryPoly::getWidth () const
{
	return m_width;
}

//-------------------------------------------------------------------

inline TerrainGeneratorFeatherFunction AffectorBoundaryPoly::getFeatherFunction () const
{
	return m_featherFunction;
}

//-------------------------------------------------------------------

inline float AffectorBoundaryPoly::getFeatherDistance () const
{
	return m_featherDistance;
}

//-------------------------------------------------------------------

inline const Rectangle2d& AffectorBoundaryPoly::getExtent () const
{
	return m_extent;
}

//-------------------------------------------------------------------

inline const HeightData& AffectorBoundaryPoly::getHeightData () const
{
	return m_heightData;
}

//===================================================================

#endif

