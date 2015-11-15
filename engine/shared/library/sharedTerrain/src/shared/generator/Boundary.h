//===================================================================
//
// Boundary.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_Boundary_H
#define INCLUDED_Boundary_H

//===================================================================

#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Transform2d.h"
#include "sharedMath/Vector2d.h"
#include "sharedTerrain/TerrainGenerator.h"

//===================================================================

class BoundaryCircle : public TerrainGenerator::Boundary
{
private:

	float centerX;
	float centerZ;
	float radius;
	float radiusSquared;

private:

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);
	void load_0002 (Iff& iff);

private:

	BoundaryCircle (const BoundaryCircle& rhs);
	BoundaryCircle& operator= (const BoundaryCircle& rhs);

public:

	BoundaryCircle ();
	virtual ~BoundaryCircle ();

	float getCenterX () const;
	float getCenterZ () const;
	float getRadius () const;
	void setCircle (float newCenterX, float newCenterZ, float newRadius);

	virtual void translate (const Vector2d& translation);
	virtual void scale (float scalar);
	virtual float isWithin (float worldX, float worldZ) const;
	virtual bool intersects(const Rectangle2d& other) const;
	virtual void scanConvertGT(float *o_data, const Rectangle2d &scanArea, int numberOfPoles) const;
	virtual void load (Iff& iff);
	virtual void save (Iff& iff) const;
	virtual void expand (Rectangle2d& extent) const;
	virtual const Vector2d getCenter () const;
	virtual void setCenter (const Vector2d& center);
};

//-------------------------------------------------------------------

inline float BoundaryCircle::getCenterX () const
{
	return centerX;
}

//-------------------------------------------------------------------

inline float BoundaryCircle::getCenterZ () const
{
	return centerZ;
}

//-------------------------------------------------------------------

inline float BoundaryCircle::getRadius () const
{
	return radius;
}

//===================================================================

class BoundaryRectangle : public TerrainGenerator::Boundary
{
private:

	typedef TerrainGenerator::Boundary BaseClass;

	Rectangle2d rectangle;
	Rectangle2d innerRectangle;

	bool m_useTransform;
	Transform2d m_transform;

	bool m_localWaterTable;
	bool m_localGlobalWaterTable;
	float m_localWaterTableHeight;
	float m_localWaterTableShaderSize;
	char * m_localWaterTableShaderTemplateName;
	TerrainGeneratorWaterType m_waterType;

private:

	void recalculate ();

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);
	void load_0002 (Iff& iff);
	void load_0003 (Iff& iff);
	void load_0004 (Iff& iff);

private:

	BoundaryRectangle (const BoundaryRectangle& rhs);
	BoundaryRectangle& operator= (const BoundaryRectangle& rhs);

public:

	BoundaryRectangle ();
	virtual ~BoundaryRectangle ();

	void setRectangle (const Rectangle2d& newRectangle);
	const Rectangle2d& getRectangle () const;

	virtual void setFeatherDistance (float newFeatherDistance);
	virtual void translate (const Vector2d& translation);
	virtual void scale (float scalar);
	virtual float isWithin (float worldX, float worldZ) const;
	virtual bool intersects(const Rectangle2d& other) const;
	virtual void scanConvertGT(float *o_data, const Rectangle2d &scanArea, int numberOfPoles) const;
	virtual void load (Iff& iff);
	virtual void save (Iff& iff) const;
	virtual void expand (Rectangle2d& extent) const;
	virtual const Vector2d getCenter () const;
	virtual void setCenter (const Vector2d& center);
	virtual void setRotation (float angle);

	bool isLocalWaterTable () const;
	void setLocalWaterTable (bool localWaterTable);
	bool isLocalGlobalWaterTable () const;
	void setLocalGlobalWaterTable (bool localGlobalWaterTable);
	float getLocalWaterTableHeight () const;
	void setLocalWaterTableHeight (float localWaterTableHeight);
	float getLocalWaterTableShaderSize () const;
	void setLocalWaterTableShaderSize (float localWaterTableShaderSize);
	char const * getLocalWaterTableShaderTemplateName () const;
	void setLocalWaterTableShaderTemplateName (char const * localWaterTableShaderTemplateName);
	TerrainGeneratorWaterType getWaterType () const;
	void setWaterType (TerrainGeneratorWaterType newWaterType);

};

//-------------------------------------------------------------------

inline const Rectangle2d& BoundaryRectangle::getRectangle () const
{
	return rectangle;
}

//-------------------------------------------------------------------

inline float BoundaryRectangle::getLocalWaterTableHeight () const
{
	return m_localWaterTableHeight;
}

//-------------------------------------------------------------------

inline float BoundaryRectangle::getLocalWaterTableShaderSize () const
{
	return m_localWaterTableShaderSize;
}

//-------------------------------------------------------------------

inline const char* BoundaryRectangle::getLocalWaterTableShaderTemplateName () const
{
	return m_localWaterTableShaderTemplateName;
}

//-------------------------------------------------------------------

inline TerrainGeneratorWaterType BoundaryRectangle::getWaterType () const
{
	return m_waterType;
}

//===================================================================

class BoundaryPoly : public TerrainGenerator::Boundary
{
protected:

	ArrayList<Vector2d> pointList;
	Rectangle2d extent;

protected:

	virtual void recalculate ();

private:

	BoundaryPoly (const BoundaryPoly& rhs);
	BoundaryPoly& operator= (const BoundaryPoly& rhs);

public:

	BoundaryPoly (Tag newTag, TerrainGeneratorBoundaryType newType);
	virtual ~BoundaryPoly ()=0;

	virtual void rotate (float angle);
	virtual void rotate (float angle, const Vector2d& center);
	virtual void translate (const Vector2d& translation);
	virtual void scale (float scalar);
	virtual void expand (Rectangle2d& extent) const;
	virtual const Vector2d getCenter () const;

	virtual bool intersects(const Rectangle2d& other) const;

	const ArrayList<Vector2d>& getPointList () const;
	int getNumberOfPoints () const;
	const Vector2d& getPoint (int index) const;
	void addPoint (const Vector2d& point);
	void replacePoint (int index, const Vector2d& point);
	void removePoint (int index);
	void clearPointList ();
	void copyPointList (const ArrayList<Vector2d>& newPointList);
};

//-------------------------------------------------------------------

inline const ArrayList<Vector2d>& BoundaryPoly::getPointList () const
{
	return pointList;
}

//-------------------------------------------------------------------

inline int BoundaryPoly::getNumberOfPoints () const
{
	return pointList.getNumberOfElements ();
}

//-------------------------------------------------------------------

inline const Vector2d& BoundaryPoly::getPoint (int index) const
{
	return pointList [index];
}

//===================================================================

class BoundaryPolygon : public BoundaryPoly
{
private:

	bool localWaterTable;
	float localWaterTableHeight;
	float localWaterTableShaderSize;
	char* localWaterTableShaderTemplateName;
	TerrainGeneratorWaterType m_waterType;


private:

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);
	void load_0002 (Iff& iff);
	void load_0003 (Iff& iff);
	void load_0004 (Iff& iff);
	void load_0005 (Iff& iff);
	void load_0006 (Iff& iff);
	void load_0007 (Iff& iff);

private:

	BoundaryPolygon (const BoundaryPolygon& rhs);
	BoundaryPolygon& operator= (const BoundaryPolygon& rhs);

public:

	BoundaryPolygon ();
	virtual ~BoundaryPolygon ();

	virtual float isWithin (float worldX, float worldZ) const;
	virtual void load (Iff& iff);
	virtual void save (Iff& iff) const;

	bool isLocalWaterTable () const;
	void setLocalWaterTable (bool newLocalWaterTable);
	float getLocalWaterTableHeight () const;
	void setLocalWaterTableHeight (float newLocalWaterTableHeight);
	float getLocalWaterTableShaderSize () const;
	void setLocalWaterTableShaderSize (float newLocalWaterTableShaderSize);
	const char* getLocalWaterTableShaderTemplateName () const;
	void setLocalWaterTableShaderTemplateName (const char* newLocalWaterTableShaderTemplateName);
	TerrainGeneratorWaterType getWaterType () const;
	void setWaterType (TerrainGeneratorWaterType newWaterType);

};

//-------------------------------------------------------------------

inline float BoundaryPolygon::getLocalWaterTableHeight () const
{
	return localWaterTableHeight;
}

//-------------------------------------------------------------------

inline float BoundaryPolygon::getLocalWaterTableShaderSize () const
{
	return localWaterTableShaderSize;
}

//-------------------------------------------------------------------

inline const char* BoundaryPolygon::getLocalWaterTableShaderTemplateName () const
{
	return localWaterTableShaderTemplateName;
}

//-------------------------------------------------------------------

inline TerrainGeneratorWaterType BoundaryPolygon::getWaterType () const
{
	return m_waterType;
}

//===================================================================

class BoundaryPolyline : public BoundaryPoly
{
private:

	float m_width;

private:

	virtual void recalculate ();

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);
	void load_0002 (Iff& iff);
	void load_0003 (Iff& iff);

private:

	BoundaryPolyline (const BoundaryPolyline& rhs);
	BoundaryPolyline& operator= (const BoundaryPolyline& rhs);

public:

	BoundaryPolyline ();
	virtual ~BoundaryPolyline ();

	virtual float isWithin (float worldX, float worldZ) const;
	virtual void load (Iff& iff);
	virtual void save (Iff& iff) const;

	void setWidth (float newWidth);
	float getWidth () const;
};

//-------------------------------------------------------------------

inline float BoundaryPolyline::getWidth () const
{
	return m_width;
}

//===================================================================

#endif
