//
// Boundary.cpp
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/Boundary.h"
#include "sharedTerrain/Feather.h"

#include "sharedFile/Iff.h"
#include "sharedMath/Vector2d.h"

#include <algorithm>

//-------------------------------------------------------------------
//
// BoundaryCircle
//
BoundaryCircle::BoundaryCircle () :
	TerrainGenerator::Boundary (TAG_BCIR, TGBT_circle),
	centerX (0),
	centerZ (0),
	radius (0),
	radiusSquared (0)
{
}

//-------------------------------------------------------------------

BoundaryCircle::~BoundaryCircle ()
{
}

//-------------------------------------------------------------------

const Vector2d BoundaryCircle::getCenter () const
{
	return Vector2d (centerX, centerZ);
}

//-------------------------------------------------------------------

void BoundaryCircle::setCircle (const float newCenterX, const float newCenterZ, const float newRadius)
{
	centerX       = newCenterX;
	centerZ       = newCenterZ;
	radius        = static_cast<float> (fabs (newRadius));
	radiusSquared = sqr (radius);
}

//-------------------------------------------------------------------

void BoundaryCircle::translate (const Vector2d& translation)
{
	centerX += translation.x;
	centerZ += translation.y;
}

//-------------------------------------------------------------------

void BoundaryCircle::scale (const float scalar)
{
	radius *= scalar;
	radiusSquared = sqr (radius);
}

//-------------------------------------------------------------------

bool BoundaryCircle::intersects(const Rectangle2d& other) const
{
	Rectangle2d rect(centerX-radius, centerZ-radius, centerX+radius, centerZ+radius);
	return rect.intersects(other);
}

//-------------------------------------------------------------------

float BoundaryCircle::isWithin (const float worldX, const float worldZ) const
{
	const float distanceSquared = sqr (centerX - worldX) + sqr (centerZ - worldZ);

	if (distanceSquared > radiusSquared)
		return 0.f;

	//-- calculate feathering
	const float innerRadiusSquared = sqr (radius * (1 - getFeatherDistance ()));

  	if (distanceSquared <= innerRadiusSquared)
  		return 1.f;

	return 1.f - ((distanceSquared - innerRadiusSquared) / (radiusSquared - innerRadiusSquared));
}

//-------------------------------------------------------------------

void BoundaryCircle::scanConvertGT(float *o_data, const Rectangle2d &scanArea, int numberOfPoles) const
{
	if (!intersects(scanArea))
	{
		return;
	}

	const float real2Sample = float(numberOfPoles-1) / (scanArea.y1 - scanArea.y0);

	// transform circle relative to scanArea.
	float cx = centerX - scanArea.x0;
	float cz = centerZ - scanArea.y0;

	// scale circle to sample area
	cx *= real2Sample;
	cz *= real2Sample;
	const float r  = radius * real2Sample;

	const int zmin = std::max(0, int(ceil(cz-r)));
	const int zmax = std::min(numberOfPoles-1, int(floor(cz+r)));
	if (zmax<zmin)
	{
		return;
	}

	const float rSquared=sqr(r);
	const float innerRSquared = sqr(r * (1-getFeatherDistance()));
	for (int z=zmin;z<=zmax;z++)
	{
		const float zDistanceSquared = sqr(float(z) - cz);

		// evaluate circle intersection with span
		const float xrad = sqrt(r*r - zDistanceSquared);
		const int xmin = std::max(0, int(ceil(cx-xrad)));
		const int xmax = std::min(numberOfPoles-1, int(floor(cx+xrad)));

		float *destRow = o_data + z*numberOfPoles;
		for (int x = xmin;x<=xmax;x++)
		{
			const float distanceSquared = sqr(float(x) - cx) + zDistanceSquared;

			//-- calculate feathering
  			if (distanceSquared <= innerRSquared)
			{
				destRow[x]=1.0f;
			}
			else
			{
				float amount = 1.f - ((distanceSquared - innerRSquared) / (rSquared - innerRSquared));
				const Feather feather(getFeatherFunction());
				amount = feather.feather(0.f, 1.f, amount);
				if (amount>destRow[x])
				{
					destRow[x]=amount;
				}
			}
		}
	}
}

//-------------------------------------------------------------------

void BoundaryCircle::expand (Rectangle2d& extent) const
{
	extent.expand (centerX - radius, centerZ - radius);
	extent.expand (centerX + radius, centerZ + radius);
}

//-------------------------------------------------------------------

void BoundaryCircle::setCenter (const Vector2d& center)
{
	translate (center - getCenter ());
}

//-------------------------------------------------------------------

void BoundaryCircle::load (Iff& iff)
{
	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	case TAG_0001:
		load_0001 (iff);
		break;

	case TAG_0002:
		load_0002 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void BoundaryCircle::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			centerX       = iff.read_float ();
			centerZ       = iff.read_float ();
			radius        = iff.read_float ();
			radiusSquared = sqr (radius);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void BoundaryCircle::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			float unused   = iff.read_float ();
			UNREF (unused);

			centerX       = iff.read_float ();
			centerZ       = iff.read_float ();
			radius        = iff.read_float ();
			radiusSquared = sqr (radius);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void BoundaryCircle::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			centerX       = iff.read_float ();
			centerZ       = iff.read_float ();
			radius        = iff.read_float ();
			radiusSquared = sqr (radius);

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
			setFeatherDistance (newFeatherDistance);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void BoundaryCircle::save (Iff& iff) const
{
	iff.insertForm (TAG_0002);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (centerX);
			iff.insertChunkData (centerZ);
			iff.insertChunkData (radius);

			iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
			iff.insertChunkData (getFeatherDistance ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------
//
// BoundaryRectangle
//
BoundaryRectangle::BoundaryRectangle () :
	TerrainGenerator::Boundary (TAG_BREC, TGBT_rectangle),
	rectangle (),
	innerRectangle (),
	m_useTransform (false),
	m_transform (),
	m_localWaterTable (false),
	m_localGlobalWaterTable (false),
	m_localWaterTableHeight (0),
	m_localWaterTableShaderSize (2.f),
	m_localWaterTableShaderTemplateName (0),
	m_waterType(TGWT_water)
{
}

//-------------------------------------------------------------------

BoundaryRectangle::~BoundaryRectangle ()
{
	if (m_localWaterTableShaderTemplateName)
	{
		delete [] m_localWaterTableShaderTemplateName;
		m_localWaterTableShaderTemplateName = 0;
	}
}

//-------------------------------------------------------------------

const Vector2d BoundaryRectangle::getCenter () const
{
	return rectangle.getCenter ();
}

//-------------------------------------------------------------------

void BoundaryRectangle::setRectangle (const Rectangle2d& newRectangle)
{
	rectangle = newRectangle;

	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryRectangle::setFeatherDistance (const float newFeatherDistance)
{
	Boundary::setFeatherDistance (newFeatherDistance);

	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryRectangle::setWaterType (TerrainGeneratorWaterType newWaterType)
{
	m_waterType = newWaterType;
}

//-------------------------------------------------------------------

void BoundaryRectangle::recalculate ()
{
	//-- fixup coordinates
	if (rectangle.x0 > rectangle.x1)
		std::swap (rectangle.x0, rectangle.x1);

	if (rectangle.y0 > rectangle.y1)
		std::swap (rectangle.y0, rectangle.y1);

	//-- find out which is smaller 
	const float width   = rectangle.getWidth ();
	const float height  = rectangle.getHeight ();
	const float feather = 0.5f * std::min (width, height) * getFeatherDistance ();

	//-- compute inner rectangle
	innerRectangle = rectangle;
	innerRectangle.x0 += feather;
	innerRectangle.y0 += feather;
	innerRectangle.x1 -= feather;
	innerRectangle.y1 -= feather;
}

//-------------------------------------------------------------------

void BoundaryRectangle::translate (const Vector2d& translation)
{
	rectangle.translate (translation.x, translation.y);

	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryRectangle::scale (const float scalar)
{
	rectangle.scale (scalar);

	recalculate ();
}

//-------------------------------------------------------------------

float BoundaryRectangle::isWithin (float worldX, float worldZ) const
{
	if (m_useTransform)
	{
		const Vector2d transformed = m_transform.rotateTranslate_p2l (Vector2d (worldX, worldZ));

		worldX = transformed.x;
		worldZ = transformed.y;
	}

	//-- test outside
	if (!rectangle.isWithin (worldX, worldZ))
		return 0.f;

	//-- do we need to feather?
	if (getFeatherDistance () == 0.f)
		return 1.f;

	//-- test inside
	if (innerRectangle.isWithin (worldX, worldZ))
		return 1.f;

	const float left   = worldX - rectangle.x0;
	const float right  = rectangle.x1 - worldX;
	const float top    = worldZ - rectangle.y0;
	const float bottom = rectangle.y1 - worldZ;

	//-- find closest edge
	const float width   = rectangle.getWidth ();
	const float height  = rectangle.getHeight ();
	const float feather = 0.5f * std::min (width, height) * getFeatherDistance ();

	float distance = feather;

	if (left < distance)
		distance = left;

	if (right < distance)
		distance = right;

	if (top < distance)
		distance = top;

	if (bottom < distance)
		distance = bottom;

	return distance / feather;
}

//-------------------------------------------------------------------

bool BoundaryRectangle::intersects(const Rectangle2d& other) const
{
	if (m_useTransform)
	{
		Rectangle2d localRect;
		m_transform.rotateTranslate_p2l(localRect, other);
		return localRect.intersects(rectangle);
	}
	else
	{
		return other.intersects(rectangle);
	}
}

//-------------------------------------------------------------------

void BoundaryRectangle::scanConvertGT(float *o_data, const Rectangle2d &scanArea, int numberOfPoles) const
{
	if (m_useTransform)
	{
		this->BaseClass::scanConvertGT(o_data, scanArea, numberOfPoles);
		return;
	}

	if (!rectangle.intersects(scanArea))
	{
		return;
	}

	const float real2Sample = float(numberOfPoles-1) / (scanArea.y1 - scanArea.y0);

	const Rectangle2d rect(
		(rectangle.x0 - scanArea.x0)*real2Sample,
		(rectangle.y0 - scanArea.y0)*real2Sample,
		(rectangle.x1 - scanArea.x0)*real2Sample,
		(rectangle.y1 - scanArea.y0)*real2Sample
	);

	const int zmin = std::max(0, int(ceil(rect.y0)));
	const int zmax = std::min(numberOfPoles-1, int(floor(rect.y1)));
	if (zmax<zmin)
	{
		return;
	}

	const int xmin = std::max(0, int(ceil(rect.x0)));
	const int xmax = std::min(numberOfPoles-1, int(floor(rect.x1)));
	if (xmax<xmin)
	{
		return;
	}

	const float featherDistance=getFeatherDistance();
	int x, z;
	if (featherDistance==0)
	{
		for (z=zmin;z<=zmax;z++)
		{
			float *destRow = o_data + z*numberOfPoles;
			for (x=xmin;x<=xmax;x++)
			{
				destRow[x]=1.0f;
			}
		}
	}
	else
	{
		const float width   = rect.getWidth();
		const float height  = rect.getHeight();
		const float maxFeather = 0.5f * std::min(width, height) * featherDistance;

		int zchange;

		z=zmin;
		zchange=int(floor(rect.y0+maxFeather));
		if (zchange>=zmin)
		{
			if (zchange>zmax)
			{
				zchange=zmax;
			}
			for (;z<=zchange;z++)
			{
				const float zfeather = float(z) - rect.y0;
				float *destRow = o_data + z*numberOfPoles;
				for (x=xmin;x<=xmax;x++)
				{
					const float xfeather0 = float(x) - rect.x0;
					const float xfeather1 = rect.x1 - float(x);
					const float xfeather = std::min(xfeather0, xfeather1);
					const float feather = std::min(xfeather, zfeather);

					float amount = feather / maxFeather; 
					amount = Feather(getFeatherFunction()).feather(0.f, 1.f, amount);
					if (amount>destRow[x])
					{
						destRow[x]=amount;
					}
				}
			}
		}

		zchange=int(floor(rect.y1-maxFeather));
		if (zchange>zmax)
		{
			zchange=zmax;
		}
		for (;z<=zchange;z++)
		{
			float *destRow = o_data + z*numberOfPoles;

			for (x=xmin;x<=xmax;x++)
			{
				const float xfeather0 = float(x) - rect.x0;
				const float xfeather1 = rect.x1 - float(x);
				const float xfeather = std::min(xfeather0, xfeather1);
				float amount = xfeather / maxFeather; 
				if (amount>1.0f)
				{
					destRow[x]=1.0f;
				}
				else
				{
					amount = Feather(getFeatherFunction()).feather(0.f, 1.f, amount);
					if (amount>destRow[x])
					{
						destRow[x]=amount;
					}
				}
			}
		}

		for (;z<=zmax;z++)
		{
			const float zfeather = rect.y1 - float(z);
			float *destRow = o_data + z*numberOfPoles;
			for (x=xmin;x<=xmax;x++)
			{
				const float xfeather0 = float(x) - rect.x0;
				const float xfeather1 = rect.x1 - float(x);
				const float xfeather = std::min(xfeather0, xfeather1);
				const float feather = std::min(xfeather, zfeather);

				float amount = feather / maxFeather; 
				amount = Feather(getFeatherFunction()).feather(0.f, 1.f, amount);
				if (amount>destRow[x])
				{
					destRow[x]=amount;
				}
			}
		}
	}
}

//-------------------------------------------------------------------

void BoundaryRectangle::expand (Rectangle2d& extent) const
{
	if (m_useTransform)
	{
		const Vector2d center      = rectangle.getCenter ();
		const Vector2d p0          = Vector2d (rectangle.x0, rectangle.y0) - center;
		const Vector2d p1          = Vector2d (rectangle.x1, rectangle.y1) - center;
		const float    magnitude   = std::max (p0.magnitude (), p1.magnitude ());
		const Vector2d position    = m_transform.getPosition_p ();

		extent.expand (position.x - magnitude, position.y - magnitude);
		extent.expand (position.x + magnitude, position.y + magnitude);
	}
	else
	{
		extent.expand (rectangle.x0, rectangle.y0);
		extent.expand (rectangle.x1, rectangle.y1);
	}
}

//-------------------------------------------------------------------

void BoundaryRectangle::setCenter (const Vector2d& center)
{
	m_useTransform = true;
	m_transform.setPosition_p (center);
}

//-------------------------------------------------------------------

void BoundaryRectangle::setRotation (const float angle)
{
	m_useTransform = true;
	m_transform.resetRotate_l2p ();
	m_transform.yaw_l (angle);
}

//-------------------------------------------------------------------

bool BoundaryRectangle::isLocalWaterTable () const
{
	return m_localWaterTable;
}

//-------------------------------------------------------------------

void BoundaryRectangle::setLocalWaterTable (bool const localWaterTable)
{
	m_localWaterTable = localWaterTable;
}

//-------------------------------------------------------------------

bool BoundaryRectangle::isLocalGlobalWaterTable () const
{
	return m_localGlobalWaterTable;
}

//-------------------------------------------------------------------

void BoundaryRectangle::setLocalGlobalWaterTable (bool const localGlobalWaterTable)
{
	m_localGlobalWaterTable = localGlobalWaterTable;
}

//-------------------------------------------------------------------

void BoundaryRectangle::setLocalWaterTableHeight (float const localWaterTableHeight)
{
	m_localWaterTableHeight = localWaterTableHeight;
}

//-------------------------------------------------------------------

void BoundaryRectangle::setLocalWaterTableShaderSize (float const localWaterTableShaderSize)
{
	m_localWaterTableShaderSize = localWaterTableShaderSize;
}

//-------------------------------------------------------------------

void BoundaryRectangle::setLocalWaterTableShaderTemplateName (char const * const localWaterTableShaderTemplateName)
{
	if (m_localWaterTableShaderTemplateName)
	{
		delete [] m_localWaterTableShaderTemplateName;
		m_localWaterTableShaderTemplateName = 0;
	}

	if (localWaterTableShaderTemplateName)
		m_localWaterTableShaderTemplateName = DuplicateString (localWaterTableShaderTemplateName);
}

//-------------------------------------------------------------------

void BoundaryRectangle::load (Iff& iff)
{
	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	case TAG_0001:
		load_0001 (iff);
		break;

	case TAG_0002:
		load_0002 (iff);
		break;

	case TAG_0003:
		load_0003 (iff);
		break;

	case TAG_0004:
		load_0004 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}

	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryRectangle::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			rectangle.x0 = iff.read_float ();
			rectangle.y0 = iff.read_float ();
			rectangle.x1 = iff.read_float ();
			rectangle.y1 = iff.read_float ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void BoundaryRectangle::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			float unused   = iff.read_float ();
			UNREF (unused);

			rectangle.x0  = iff.read_float ();
			rectangle.y0  = iff.read_float ();
			rectangle.x1  = iff.read_float ();
			rectangle.y1  = iff.read_float ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void BoundaryRectangle::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			rectangle.x0 = iff.read_float ();
			rectangle.y0 = iff.read_float ();
			rectangle.x1 = iff.read_float ();
			rectangle.y1 = iff.read_float ();

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

			const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
			setFeatherDistance (newFeatherDistance);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void BoundaryRectangle::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			rectangle.x0 = iff.read_float ();
			rectangle.y0 = iff.read_float ();
			rectangle.x1 = iff.read_float ();
			rectangle.y1 = iff.read_float ();

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

			const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
			setFeatherDistance (newFeatherDistance);

			m_localWaterTable = iff.read_int32 () != 0;
			m_localGlobalWaterTable = iff.read_int32 () != 0;
			m_localWaterTableHeight = iff.read_float ();
			m_localWaterTableShaderSize = iff.read_float ();
			m_localWaterTableShaderTemplateName = iff.read_string ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

void BoundaryRectangle::load_0004 (Iff& iff)
{
	iff.enterForm (TAG_0004);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			rectangle.x0 = iff.read_float ();
			rectangle.y0 = iff.read_float ();
			rectangle.x1 = iff.read_float ();
			rectangle.y1 = iff.read_float ();

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

			const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
			setFeatherDistance (newFeatherDistance);

			m_localWaterTable = iff.read_int32 () != 0;
			m_localGlobalWaterTable = iff.read_int32 () != 0;
			m_localWaterTableHeight = iff.read_float ();
			m_localWaterTableShaderSize = iff.read_float ();
			m_localWaterTableShaderTemplateName = iff.read_string ();
			setWaterType(static_cast<TerrainGeneratorWaterType> (iff.read_int32 ()));

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------

void BoundaryRectangle::save (Iff& iff) const
{
	iff.insertForm (TAG_0004);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (rectangle.x0);
			iff.insertChunkData (rectangle.y0);
			iff.insertChunkData (rectangle.x1);
			iff.insertChunkData (rectangle.y1);

			iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
			iff.insertChunkData (getFeatherDistance ());

			iff.insertChunkData (m_localWaterTable ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkData (m_localGlobalWaterTable ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkData (m_localWaterTableHeight);
			iff.insertChunkData (m_localWaterTableShaderSize);
			iff.insertChunkString (m_localWaterTableShaderTemplateName ? m_localWaterTableShaderTemplateName : "");
			iff.insertChunkData (static_cast<int32> (getWaterType()));

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------
//
// BoundaryPoly
//
BoundaryPoly::BoundaryPoly (Tag newTag, TerrainGeneratorBoundaryType newType) :
	TerrainGenerator::Boundary (newTag, newType),
	pointList (),
	extent ()
{
}

//-------------------------------------------------------------------

BoundaryPoly::~BoundaryPoly ()
{
}

//-------------------------------------------------------------------

const Vector2d BoundaryPoly::getCenter () const
{
	return extent.getCenter ();
}

//-------------------------------------------------------------------

void BoundaryPoly::addPoint (const Vector2d& point)
{
	pointList.add (point);
}

//-------------------------------------------------------------------

void BoundaryPoly::replacePoint (const int index, const Vector2d& newPoint)
{
	pointList [index] = newPoint;

	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryPoly::removePoint (const int index)
{
	pointList.removeIndexAndCompactList (index);

	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryPoly::clearPointList ()
{
	pointList.clear ();
}

//-------------------------------------------------------------------

void BoundaryPoly::copyPointList (const ArrayList<Vector2d>& newPointList)
{
	pointList = newPointList;

	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryPoly::recalculate ()
{
	extent.x0 = FLT_MAX;
	extent.y0 = FLT_MAX;
	extent.x1 = -FLT_MAX;
	extent.y1 = -FLT_MAX;

	int i;
	for (i = 0; i < pointList.getNumberOfElements (); i++)
		extent.expand (pointList [i].x, pointList [i].y);
}

//-------------------------------------------------------------------

void BoundaryPoly::rotate (const float angle)
{
	if (pointList.getNumberOfElements () == 0)
		return;

	Vector2d center;

	int i;
	for (i = 0; i < pointList.getNumberOfElements (); i++)
		center += pointList [i];

	center.x /= static_cast<float> (pointList.getNumberOfElements ());
	center.y /= static_cast<float> (pointList.getNumberOfElements ());

	rotate (angle, center);
}

//-------------------------------------------------------------------

void BoundaryPoly::rotate (const float angle, const Vector2d& center)
{
	if (pointList.getNumberOfElements () == 0)
		return;

	const float cosAngle = cos (angle);
	const float sinAngle = sin (angle);

	int i;
	for (i = 0; i < pointList.getNumberOfElements (); i++)
	{
		Vector2d point;
		point.x = pointList [i].x - center.x;
		point.y = pointList [i].y - center.y;

		Vector2d newPoint;
		newPoint.x = center.x + point.x * cosAngle - point.y * sinAngle;
		newPoint.y = center.y + point.x * sinAngle + point.y * cosAngle;

		pointList [i] = newPoint;
	}
	
	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryPoly::translate (const Vector2d& translation)
{
	int i;
	for (i = 0; i < pointList.getNumberOfElements (); i++)
		pointList [i] += translation;
	
	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryPoly::scale (const float scalar)
{
	if (pointList.getNumberOfElements () == 0)
		return;

	Vector2d center;

	int i;
	for (i = 0; i < pointList.getNumberOfElements (); i++)
		center += pointList [i];

	center.x /= static_cast<float> (pointList.getNumberOfElements ());
	center.y /= static_cast<float> (pointList.getNumberOfElements ());

	for (i = 0; i < pointList.getNumberOfElements (); i++)
	{
		pointList [i].x = (pointList [i].x - center.x) * scalar + center.x;
		pointList [i].y = (pointList [i].y - center.y) * scalar + center.y;
	}
	
	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryPoly::expand (Rectangle2d& parentExtent) const
{
	parentExtent.expand (extent.x0, extent.y0);
	parentExtent.expand (extent.x1, extent.y1);
}

//-------------------------------------------------------------------

bool BoundaryPoly::intersects(const Rectangle2d& other) const
{
	return other.intersects(extent);
}

//-------------------------------------------------------------------
//
// BoundaryPolygon
//
BoundaryPolygon::BoundaryPolygon () :
	BoundaryPoly (TAG_BPOL, TGBT_polygon),
	localWaterTable (false),
	localWaterTableHeight (0),
	localWaterTableShaderSize (2.f),
	localWaterTableShaderTemplateName (0),
	m_waterType(TGWT_water)
{
}

//-------------------------------------------------------------------

BoundaryPolygon::~BoundaryPolygon ()
{
	if (localWaterTableShaderTemplateName)
	{
		delete [] localWaterTableShaderTemplateName;
		localWaterTableShaderTemplateName = 0;
	}
}

//-------------------------------------------------------------------

bool BoundaryPolygon::isLocalWaterTable () const
{
	return localWaterTable;
}

//-------------------------------------------------------------------

void BoundaryPolygon::setLocalWaterTable (const bool newLocalWaterTable)
{
	localWaterTable = newLocalWaterTable;
}

//-------------------------------------------------------------------

void BoundaryPolygon::setLocalWaterTableHeight (const float newLocalWaterTableHeight)
{
	localWaterTableHeight = newLocalWaterTableHeight;
}

//-------------------------------------------------------------------

void BoundaryPolygon::setLocalWaterTableShaderSize (const float newLocalWaterTableShaderSize)
{
	localWaterTableShaderSize = newLocalWaterTableShaderSize;
}

//-------------------------------------------------------------------

void BoundaryPolygon::setLocalWaterTableShaderTemplateName (const char* newLocalWaterTableShaderTemplateName)
{
	if (localWaterTableShaderTemplateName)
	{
		delete [] localWaterTableShaderTemplateName;
		localWaterTableShaderTemplateName = 0;
	}

	if (newLocalWaterTableShaderTemplateName)
		localWaterTableShaderTemplateName = DuplicateString (newLocalWaterTableShaderTemplateName);
}

//-------------------------------------------------------------------

void BoundaryPolygon::setWaterType (TerrainGeneratorWaterType newWaterType)
{
	m_waterType = newWaterType;
}

//-------------------------------------------------------------------

float BoundaryPolygon::isWithin (const float worldX, const float worldZ) const
{
	if (!extent.isWithin (worldX, worldZ))
		return 0.f;

	bool in = false;

	const int n = pointList.getNumberOfElements ();

	//-- check to make sure the point is inside the shape
	int i;
	int j;
	for (i = 0, j = n - 1; i < n; j = i++) 
		if ((((pointList [i].y <= worldZ) && (worldZ < pointList [j].y)) || ((pointList [j].y <= worldZ) && (worldZ < pointList [i].y))) && (worldX < (pointList [j].x - pointList [i].x) * (worldZ - pointList [i].y) / (pointList [j].y - pointList [i].y) + pointList [i].x))
			in = !in;

	if (!in)
		return 0.f;
	
	if (getFeatherDistance () == 0.f)
		return 1.f;

	const float featherDistanceSquared = sqr (getFeatherDistance ());

	float distanceSquared = featherDistanceSquared;

	for (i = 0; i < n; ++i)
	{
		const float x = pointList [i].x;
		const float y = pointList [i].y;
		const float thisDistanceSquared = sqr (worldX - x) + sqr (worldZ - y);
		if (thisDistanceSquared < distanceSquared)
			distanceSquared = thisDistanceSquared;
	}
	
	for (i = 0, j = n - 1; i < n; j = i++)
	{
		//-- compute u on parametric line
		const float x1 = pointList [j].x;
		const float y1 = pointList [j].y;  //lint !e578  //-- hides y1 (double)
		const float x2 = pointList [i].x;
		const float y2 = pointList [i].y;

		const float u = ((worldX - x1) * (x2 - x1) + (worldZ - y1) * (y2 - y1)) / (sqr (x1 - x2) + sqr (y1 - y2));
		if (u >= 0 && u <= 1)
		{
			const float x = x1 + u * (x2 - x1);
			const float y = y1 + u * (y2 - y1);
			const float thisDistanceSquared = sqr (worldX - x) + sqr (worldZ - y);
			if (thisDistanceSquared < distanceSquared)
				distanceSquared = thisDistanceSquared;
		}
	}

	if (!WithinEpsilonInclusive (featherDistanceSquared, distanceSquared, 0.0001f))
		return sqrt (distanceSquared) / getFeatherDistance ();

	return 1.f;
}

//-------------------------------------------------------------------

void BoundaryPolygon::load (Iff& iff)
{
	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	case TAG_0001:
		load_0001 (iff);
		break;

	case TAG_0002:
		load_0002 (iff);
		break;

	case TAG_0003:
		load_0003 (iff);
		break;

	case TAG_0004:
		load_0004 (iff);
		break;

	case TAG_0005:
		load_0005 (iff);
		break;

	case TAG_0006:
		load_0006 (iff);
		break;

	case TAG_0007:
		load_0007 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}

	//-- make sure we update our bounding box
	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryPolygon::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			Vector2d point;

			int n = iff.getChunkLengthLeft (sizeof (Vector2d));
			int i;
			for (i = 0; i < n; i++)
			{
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void BoundaryPolygon::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			float unused   = iff.read_float ();
			UNREF (unused);

			Vector2d point;

			int n = iff.getChunkLengthLeft (sizeof (Vector2d));
			int i;
			for (i = 0; i < n; i++)
			{
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void BoundaryPolygon::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			setFeatherDistance (iff.read_float ());

			int n = iff.getChunkLengthLeft (sizeof (Vector2d));
			int i;
			for (i = 0; i < n; i++)
			{
				Vector2d point;
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void BoundaryPolygon::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			setFeatherDistance (iff.read_float ());
			
			localWaterTable       = iff.read_int32 () != 0;
			localWaterTableHeight = iff.read_float ();
			localWaterTableShaderTemplateName = iff.read_string ();

			int n = iff.getChunkLengthLeft (sizeof (Vector2d));
			int i;
			for (i = 0; i < n; i++)
			{
				Vector2d point;
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

void BoundaryPolygon::load_0004 (Iff& iff)
{
	iff.enterForm (TAG_0004);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			setFeatherDistance (iff.read_float ());
			
			localWaterTable       = iff.read_int32 () != 0;
			localWaterTableHeight = iff.read_float ();
			localWaterTableShaderSize = iff.read_float ();
			localWaterTableShaderTemplateName = iff.read_string ();

			int n = iff.getChunkLengthLeft (sizeof (Vector2d));
			int i;
			for (i = 0; i < n; i++)
			{
				Vector2d point;
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------

void BoundaryPolygon::load_0005 (Iff& iff)
{
	iff.enterForm (TAG_0005);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			const int n = iff.read_int32 ();
			int i;
			for (i = 0; i < n; i++)
			{
				Vector2d point;
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}
			
			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			setFeatherDistance (iff.read_float ());
			
			localWaterTable       = iff.read_int32 () != 0;
			localWaterTableHeight = iff.read_float ();
			localWaterTableShaderSize = iff.read_float ();
			localWaterTableShaderTemplateName = iff.read_string ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0005);
}

//-------------------------------------------------------------------

void BoundaryPolygon::load_0006 (Iff& iff)
{
	iff.enterForm (TAG_0006);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			const int n = iff.read_int32 ();
			int i;
			for (i = 0; i < n; i++)
			{
				Vector2d point;
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}
			
			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			setFeatherDistance (iff.read_float ());
			
			localWaterTable       = iff.read_int32 () != 0;
			localWaterTableHeight = iff.read_float ();
			localWaterTableShaderSize = iff.read_float ();
			IGNORE_RETURN (iff.read_int32 ());
			localWaterTableShaderTemplateName = iff.read_string ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0006);
}

//-------------------------------------------------------------------

void BoundaryPolygon::load_0007 (Iff& iff)
{
	iff.enterForm (TAG_0007);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			const int n = iff.read_int32 ();
			int i;
			for (i = 0; i < n; i++)
			{
				Vector2d point;
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}
			
			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			setFeatherDistance (iff.read_float ());
			
			localWaterTable       = iff.read_int32 () != 0;
			localWaterTableHeight = iff.read_float ();
			localWaterTableShaderSize = iff.read_float ();
			setWaterType(static_cast<TerrainGeneratorWaterType> (iff.read_int32 ()));
			//IGNORE_RETURN (iff.read_int32 ());
			localWaterTableShaderTemplateName = iff.read_string (); 
			

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0007);
}

//-------------------------------------------------------------------

void BoundaryPolygon::save (Iff& iff) const
{
	iff.insertForm (TAG_0007);   

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (pointList.getNumberOfElements ());

			int i;
			for (i = 0; i < pointList.getNumberOfElements (); i++)
			{
				iff.insertChunkData (pointList [i].x);
				iff.insertChunkData (pointList [i].y);
			}

			iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
			iff.insertChunkData (getFeatherDistance ());
			iff.insertChunkData (localWaterTable ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkData (localWaterTableHeight);
			iff.insertChunkData (localWaterTableShaderSize);
			iff.insertChunkData (static_cast<int32> (getWaterType()));
			iff.insertChunkString (localWaterTableShaderTemplateName ? localWaterTableShaderTemplateName : "");
			

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0007);
}

//-------------------------------------------------------------------
//
// BoundaryPolyline
//
BoundaryPolyline::BoundaryPolyline () :
	BoundaryPoly (TAG_BPLN, TGBT_polyline),
	m_width (2.f)
{
}

//-------------------------------------------------------------------

BoundaryPolyline::~BoundaryPolyline ()
{
}

//-------------------------------------------------------------------

void BoundaryPolyline::setWidth (const float newWidth)
{
	m_width = newWidth;

	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryPolyline::recalculate ()
{
	BoundaryPoly::recalculate ();

	extent.x0 -= m_width;
	extent.x1 += m_width;
	extent.y0 -= m_width;
	extent.y1 += m_width;
}

//-------------------------------------------------------------------

float BoundaryPolyline::isWithin (const float worldX, const float worldZ) const
{
	if (!extent.isWithin (worldX, worldZ))
		return 0.f;

	const float widthSquared = sqr (m_width);

	float distanceSquared = widthSquared;

	//-- first, scan how far we are from the points
	{
		const int n = pointList.getNumberOfElements ();
		int i;
		for (i = 0; i < n; ++i)
		{
			const float x = pointList [i].x;
			const float y = pointList [i].y;
			const float thisDistanceSquared = sqr (worldX - x) + sqr (worldZ - y);
			if (thisDistanceSquared < distanceSquared)
				distanceSquared = thisDistanceSquared;
		}
	}

	//-- next, scan each line
	{
		const int n = pointList.getNumberOfElements () - 1;
		int i;
		for (i = 0; i < n; ++i)
		{
			const float x1 = pointList [i].x;
			const float y1 = pointList [i].y;  //lint !e578  //-- hides y1 (double)
			const float x2 = pointList [i + 1].x;
			const float y2 = pointList [i + 1].y;

			const float u = ((worldX - x1) * (x2 - x1) + (worldZ - y1) * (y2 - y1)) / (sqr (x2 - x1) + sqr (y2 - y1));
			if (u >= 0 && u <= 1)
			{
				const float x = x1 + u * (x2 - x1);
				const float y = y1 + u * (y2 - y1);
				const float thisDistanceSquared = sqr (worldX - x) + sqr (worldZ - y);
				if (thisDistanceSquared < distanceSquared)
					distanceSquared = thisDistanceSquared;
			}
		}
	}

	if (distanceSquared < widthSquared)
	{
		const float newFeatherDistance        = m_width * (1.f - getFeatherDistance ());
		const float newFeatherDistanceSquared = sqr (newFeatherDistance);

		if (distanceSquared < newFeatherDistanceSquared)
			return 1.f;

		return 1.f - (sqrt (distanceSquared) - newFeatherDistance) / (m_width - newFeatherDistance);
	}

	return 0.f;
}

//-------------------------------------------------------------------

void BoundaryPolyline::load (Iff& iff)
{
	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	case TAG_0001:
		load_0001 (iff);
		break;

	case TAG_0002:
		load_0002 (iff);
		break;

	case TAG_0003:
		load_0003 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}

	//-- make sure we update our bounding box
	recalculate ();
}

//-------------------------------------------------------------------

void BoundaryPolyline::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			setFeatherDistance (iff.read_float ());
			setWidth (iff.read_float ());

			int n = iff.getChunkLengthLeft (sizeof (Vector2d));
			int i;
			for (i = 0; i < n; i++)
			{
				Vector2d point;
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void BoundaryPolyline::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			const int n = iff.read_int32 ();
			int i;
			for (i = 0; i < n; i++)
			{
				Vector2d point;
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			setFeatherDistance (iff.read_float ());
			setWidth (iff.read_float ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void BoundaryPolyline::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			int n = iff.read_int32 ();
			int i;
			for (i = 0; i < n; i++)
			{
				Vector2d point;
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}

			n = iff.read_int32 (); // old height list
			for(i = 0; i < n; i++)
			{
				IGNORE_RETURN(iff.read_float());
			}

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			setFeatherDistance (iff.read_float ());
			setWidth (iff.read_float ());
			IGNORE_RETURN(iff.read_int32()); // old has widths

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void BoundaryPolyline::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			int n = iff.read_int32 ();
			int i;
			for (i = 0; i < n; i++)
			{
				Vector2d point;
				point.x = iff.read_float ();
				point.y = iff.read_float ();

				pointList.add (point);
			}

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
			setFeatherDistance (iff.read_float ());
			setWidth (iff.read_float ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------



void BoundaryPolyline::save (Iff& iff) const
{
	iff.insertForm (TAG_0003);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (pointList.getNumberOfElements ());

			int i;
			for (i = 0; i < pointList.getNumberOfElements (); i++)
			{
				iff.insertChunkData (pointList [i].x);
				iff.insertChunkData (pointList [i].y);
			}

			iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
			iff.insertChunkData (getFeatherDistance ());
			iff.insertChunkData (m_width);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

