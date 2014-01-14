//
// AffectorRoad.cpp
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/AffectorRoad.h"

#include "sharedFile/Iff.h"
#include "sharedMath/Line2d.h"
#include "sharedMath/Vector2d.h"
#include "sharedTerrain/CoordinateHash.h"
#include "sharedRandom/FastRandomGenerator.h"
#include "sharedTerrain/Affector.h"
#include "sharedTerrain/Feather.h"

//-------------------------------------------------------------------
//
// AffectorRoad
//
AffectorRoad::AffectorRoad () :
	AffectorBoundaryPoly (TAG_AROA, TGAT_road),
	m_cachedFamilyId (-1),
	m_cachedSgi (),
	m_familyId (0),
	m_featherFunctionShader (TGFF_linear),
	m_featherDistanceShader (0.5f),
	m_hasFixedHeights(false),
	m_heightList ()
{ 
}

//-------------------------------------------------------------------

AffectorRoad::~AffectorRoad ()
{
}

//-------------------------------------------------------------------

void AffectorRoad::setFamilyId (const int newFamilyId)
{
	m_familyId = newFamilyId;
}

//-------------------------------------------------------------------

void AffectorRoad::setFeatherFunctionShader (TerrainGeneratorFeatherFunction newFeatherFunction)
{
	m_featherFunctionShader = newFeatherFunction;
}

//-------------------------------------------------------------------

void AffectorRoad::setFeatherDistanceShader (const float newFeatherDistance)
{
	m_featherDistanceShader = newFeatherDistance;
}

//-------------------------------------------------------------------

void AffectorRoad::setHasFixedHeights (const bool newVal)
{
	m_hasFixedHeights = newVal;
}

//-------------------------------------------------------------------

bool AffectorRoad::affectsHeight () const
{
	return true;
}

//-------------------------------------------------------------------

bool AffectorRoad::affectsShader () const
{
	return true;
}

//-------------------------------------------------------------------

void AffectorRoad::createInitialHeightList ()
{
	m_heightList.clear ();
	int size = m_pointList.getNumberOfElements ();
	for(int i = 0; i < size; i++)
	{
		m_heightList.add(0.0f);
	}
}

//-------------------------------------------------------------------

void AffectorRoad::copyHeightList (const ArrayList<float>& newHeightList)
{
	m_heightList = newHeightList;
}

//-------------------------------------------------------------------

void AffectorRoad::clearHeightList ()
{
	m_heightList.clear ();
}

//-------------------------------------------------------------------

unsigned AffectorRoad::getAffectedMaps() const
{
	return 
		  TGM_height 
		| TGM_shader 
		| TGM_floraStaticCollidable 
		| TGM_floraStaticNonCollidable 
		| TGM_floraDynamicNear 
		| TGM_floraDynamicFar
		;
}

//-------------------------------------------------------------------

void AffectorRoad::createHeightData ()
{
	m_heightData.createRoadData ();
}

//-------------------------------------------------------------------

void AffectorRoad::prepare ()
{
	m_cachedFamilyId = -1;
}

//-------------------------------------------------------------------

void AffectorRoad::affect (const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (generatorChunkData.m_legacyRandomGenerator)
	{
		_legacyAffect(worldX, worldZ, x, z, amount, generatorChunkData);
		return;
	}
	if (!isEnabled ())
		return;

	if (amount > 0.f)
	{
		if (m_extent.isWithin (worldX, worldZ))
		{
			const float  width_2        = m_width * 0.5f;
			const float originalHeight = generatorChunkData.heightMap->getData (x, z);

			FindData result;
			if (find (Vector2d (worldX, worldZ), width_2, result))
			{
				const float distanceToCenter = fabsf (result.distanceToCenter);
				const float desiredHeight    = result.height;

				if (WithinRangeInclusiveInclusive (0.f, distanceToCenter, width_2 * (1.f - getFeatherDistance ())))
				{

					if(!m_hasFixedHeights)
					{
						generatorChunkData.heightMap->setData (x, z, desiredHeight);
					}
					else
					{
						// ramped road affector using fixed heights
						generatorChunkData.heightMap->setData (x,z, getRampedHeight(worldX,worldZ,originalHeight));
					}

					//-- clear the flora
					generatorChunkData.floraStaticCollidableMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
					generatorChunkData.floraStaticNonCollidableMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
					generatorChunkData.floraDynamicNearMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
					generatorChunkData.floraDynamicFarMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
				}
				else
				{
					//-- height
					const float t = distanceToCenter / width_2;
					DEBUG_FATAL (t < 0.f || t > 1.f, ("t is out of range"));

					if(!m_hasFixedHeights)
					{
						generatorChunkData.heightMap->setData (x, z, linearInterpolate (desiredHeight, originalHeight, t));
					}
					else
					{
						// ramped road affector using fixed heights
						generatorChunkData.heightMap->setData (x, z, getRampedHeight(worldX,worldZ,originalHeight));
					}

				}

				if (WithinRangeInclusiveInclusive (0.f, distanceToCenter, width_2 * (1.f - getFeatherDistanceShader ())))
				{
					//-- set the shader
					if (m_cachedFamilyId != m_familyId)
					{
						m_cachedFamilyId = m_familyId;
						m_cachedSgi      = generatorChunkData.shaderGroup->chooseShader (m_familyId);
					}

					FastRandomGenerator randomGenerator(CoordinateHash::hashTuple(worldX, worldZ));

					ShaderGroup::Info sgi = m_cachedSgi;
					sgi.setChildChoice (randomGenerator.randomFloat());
					generatorChunkData.shaderMap->setData (x, z, sgi);
				}
			}
		}
	}
}

//-------------------------------------------------------------------

float AffectorRoad::getRampedHeight (const float worldX, const float worldZ,const float terrainHeight) const
{
	if (!m_extent.isWithin (worldX, worldZ))
		return -100.f;

	const float widthSquared = sqr (m_width/2.0f);

	float distanceSquared = widthSquared;
	int foundIndex = -1;

	//-- first, scan how far we are from the points
	{
		const int n = m_pointList.getNumberOfElements ();
		int i;
		for (i = 0; i < n; ++i)
		{
			const float x = m_pointList [i].x;
			const float y = m_pointList [i].y;
			const float thisDistanceSquared = sqr (worldX - x) + sqr (worldZ - y);
			if (thisDistanceSquared < distanceSquared)
			{
				distanceSquared = thisDistanceSquared;
				
				foundIndex = i;
			}
		}
	}

	
	//-- next, scan each line
	{
		const int n = m_pointList.getNumberOfElements () - 1;
		int i;
		for (i = 0; i < n; ++i)
		{
			const float x1 = m_pointList [i].x;
			const float y1 = m_pointList [i].y;  //lint !e578  //-- hides y1 (double)
			const float x2 = m_pointList [i + 1].x;
			const float y2 = m_pointList [i + 1].y;

			const float u = ((worldX - x1) * (x2 - x1) + (worldZ - y1) * (y2 - y1)) / (sqr (x2 - x1) + sqr (y2 - y1));
			if (u >= 0 && u <= 1)
			{
				const float x = x1 + u * (x2 - x1);
				const float y = y1 + u * (y2 - y1);
				const float thisDistanceSquared = sqr (worldX - x) + sqr (worldZ - y);
				if (thisDistanceSquared < distanceSquared)
				{
					distanceSquared = thisDistanceSquared;
					foundIndex = i;
				}
			}
		}
	}

	if (distanceSquared < widthSquared)
	{

		DEBUG_FATAL(foundIndex == -1,("AffectorRoad::getRampedHeight - foundIndex is -1"));
		
		float height = 0.0f;

		// start feathering
		float featherMultiplier = 1.0f;

		const float newFeatherDistance        = (m_width/2.0f) * (1.f - getFeatherDistance ());

		const float newFeatherDistanceSquared = sqr (newFeatherDistance);

		if (distanceSquared >= newFeatherDistanceSquared)
		{
			featherMultiplier = 1.f - (sqrt (distanceSquared) - newFeatherDistance) / (m_width/2.0f - newFeatherDistance);
			const Feather feather(getFeatherFunction());
			featherMultiplier=feather.feather(0.f, 1.f, featherMultiplier);
		}
		// end feathering

		const float startHeight = m_heightList[foundIndex];
		if(foundIndex != m_pointList.getNumberOfElements() - 1)
		{
			// project the world point onto the line between the control points and interp for the height
			const Vector2d position(worldX,worldZ);
			const float endHeight = m_heightList[foundIndex + 1];
			const Vector2d p1(m_pointList [foundIndex].x,m_pointList [foundIndex].y);
			const Vector2d p2(m_pointList [foundIndex+1].x,m_pointList [foundIndex+1].y);
			const float totalDistance = p1.magnitudeBetween(p2);

			const Line2d line (p1,p2);
			const float distToLine = line.computeDistanceTo(position);
			const float realDistToPoint1 = p1.magnitudeBetween(position);
			const float realDistToPoint2 = p2.magnitudeBetween(position);
			const float projectedDistToPoint1 = sqrt(sqr(realDistToPoint1) - sqr(distToLine));
			const float projectedDistToPoint2 = sqrt(sqr(realDistToPoint2) - sqr(distToLine));

			if(totalDistance != 0.0f && projectedDistToPoint1 <= totalDistance && projectedDistToPoint2 <= totalDistance)
			{
				height = startHeight + (projectedDistToPoint1/totalDistance) * (endHeight - startHeight);
			}
			else
			{
				height = startHeight;
			}
		}
		else
		{
			height = startHeight;
		}

		// don't build canyons or ridges due to feathering to 0 height
		const float delta = height - terrainHeight;
		height = terrainHeight + (delta * featherMultiplier);

		return height;
	}

	return -100.f;
}


void AffectorRoad::_legacyAffect (const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (!isEnabled ())
		return;

	if (amount > 0.f)
	{
		if (m_extent.isWithin (worldX, worldZ))
		{
			const float  width_2        = m_width * 0.5f;
			const float originalHeight = generatorChunkData.heightMap->getData (x, z);

			FindData result;
			if (find (Vector2d (worldX, worldZ), width_2, result))
			{
				const float distanceToCenter = fabsf (result.distanceToCenter);
				const float desiredHeight    = result.height;

				if (WithinRangeInclusiveInclusive (0.f, distanceToCenter, width_2 * (1.f - getFeatherDistance ())))
				{
					generatorChunkData.heightMap->setData (x, z, desiredHeight);

					//-- clear the flora
					generatorChunkData.floraStaticCollidableMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
					generatorChunkData.floraStaticNonCollidableMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
					generatorChunkData.floraDynamicNearMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
					generatorChunkData.floraDynamicFarMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
				}
				else
				{
					//-- height
					const float t = distanceToCenter / width_2;
					DEBUG_FATAL (t < 0.f || t > 1.f, ("t is out of range"));

					generatorChunkData.heightMap->setData (x, z, linearInterpolate (desiredHeight, originalHeight, t));
				}

				if (WithinRangeInclusiveInclusive (0.f, distanceToCenter, width_2 * (1.f - getFeatherDistanceShader ())))
				{
					//-- set the shader
					if (m_cachedFamilyId != m_familyId)
					{
						m_cachedFamilyId = m_familyId;
						m_cachedSgi      = generatorChunkData.shaderGroup->chooseShader (m_familyId);
					}

					ShaderGroup::Info sgi = m_cachedSgi;
					sgi.setChildChoice (generatorChunkData.m_legacyRandomGenerator->randomReal (0.0f, 1.0f));
					generatorChunkData.shaderMap->setData (x, z, sgi);
				}
			}
		}
	}
}

//-------------------------------------------------------------------

void AffectorRoad::load (Iff& iff)
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

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("invalid AffectorRoad version %s/%s", buffer, tagBuffer));
		}
		break;
	}

	recalculate ();
}

//-------------------------------------------------------------------

void AffectorRoad::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorRoad::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			Vector2d newStart;
			newStart.x = iff.read_float ();
			newStart.y = iff.read_float ();
			addPoint (newStart);

			Vector2d newEnd;
			newEnd.x = iff.read_float ();
			newEnd.y = iff.read_float ();
			addPoint (newEnd);

			setWidth (iff.read_float ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void AffectorRoad::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			m_heightData.load (iff);

			iff.enterChunk (TAG_DATA);

				Vector2d newStart;
				newStart.x = iff.read_float ();
				newStart.y = iff.read_float ();
				addPoint (newStart);

				Vector2d newEnd;
				newEnd.x = iff.read_float ();
				newEnd.y = iff.read_float ();
				addPoint (newEnd);

				setWidth (iff.read_float ());

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void AffectorRoad::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			m_heightData.load (iff);

			iff.enterChunk (TAG_DATA);

				Vector2d newStart;
				newStart.x = iff.read_float ();
				newStart.y = iff.read_float ();
				addPoint (newStart);

				Vector2d newEnd;
				newEnd.x = iff.read_float ();
				newEnd.y = iff.read_float ();
				addPoint (newEnd);

				setWidth (iff.read_float ());
				setFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);
				setFeatherFunctionShader (getFeatherFunction ());
				setFeatherDistanceShader (getFeatherDistance ());

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

void AffectorRoad::load_0004 (Iff& iff)
{
	iff.enterForm (TAG_0004);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			m_heightData.load (iff);

			iff.enterChunk (TAG_DATA);

				const int n = iff.read_int32 ();
				int i;
				for (i = 0; i < n; ++i)
				{
					Vector2d point;
					point.x = iff.read_float ();
					point.y = iff.read_float ();
					addPoint (point);
				}

				setWidth (iff.read_float ());
				setFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);
				setFeatherFunctionShader (getFeatherFunction ());
				setFeatherDistanceShader (getFeatherDistance ());

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------

void AffectorRoad::load_0005 (Iff& iff)
{
	iff.enterForm (TAG_0005);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			m_heightData.load (iff);

			iff.enterChunk (TAG_DATA);

				const int n = iff.read_int32 ();
				int i;
				for (i = 0; i < n; ++i)
				{
					Vector2d point;
					point.x = iff.read_float ();
					point.y = iff.read_float ();
					addPoint (point);
				}

				setWidth (iff.read_float ());
				setFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);
				setFeatherFunctionShader (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				const float newFeatherDistanceShader = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistanceShader (newFeatherDistanceShader);

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0005);
}

//-------------------------------------------------------------------

void AffectorRoad::load_0006 (Iff& iff)
{
	iff.enterForm (TAG_0006);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			m_heightData.load (iff);

			iff.enterChunk (TAG_DATA);

				int n = iff.read_int32 ();
				int i;
				for (i = 0; i < n; ++i)
				{
					Vector2d point;
					point.x = iff.read_float ();
					point.y = iff.read_float ();
					addPoint (point);
				}

				n = iff.read_int32 ();
				for(i = 0; i < n; ++i)
				{
					m_heightList.add(iff.read_float());
				}

				setWidth (iff.read_float ());
				setFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);
				setFeatherFunctionShader (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				const float newFeatherDistanceShader = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistanceShader (newFeatherDistanceShader);
				setHasFixedHeights(iff.read_int32() != 0);

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0006);
}

//-------------------------------------------------------------------

void AffectorRoad::save (Iff& iff) const
{
	iff.insertForm (TAG_0006);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertForm (TAG_DATA);

			m_heightData.save (iff);

			iff.insertChunk (TAG_DATA);

				iff.insertChunkData (m_pointList.getNumberOfElements ());
				int i;
				for (i = 0; i < m_pointList.getNumberOfElements (); ++i)
				{
					iff.insertChunkData (getPoint (i).x);
					iff.insertChunkData (getPoint (i).y);
				}

				iff.insertChunkData (m_heightList.getNumberOfElements ());
				for (i = 0; i < m_heightList.getNumberOfElements (); ++i)
				{
					iff.insertChunkData (m_heightList[i]);
				}

				iff.insertChunkData (getWidth ());
				iff.insertChunkData (getFamilyId ());
				iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
				iff.insertChunkData (getFeatherDistance ());
				iff.insertChunkData (static_cast<int32> (getFeatherFunctionShader ()));
				iff.insertChunkData (getFeatherDistanceShader ());
				iff.insertChunkData (m_hasFixedHeights ? static_cast<int32> (1) : static_cast<int32> (0));

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0006);
}

//-------------------------------------------------------------------

