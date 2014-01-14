//
// AffectorRibbon.cpp
//
// copyright 2005, Sony Online Entertainment
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/AffectorRibbon.h"

#include "sharedFile/Iff.h"
#include "sharedMath/Vector2d.h"
#include "sharedRandom/FastRandomGenerator.h"
#include "sharedTerrain/Affector.h"
#include "sharedTerrain/CoordinateHash.h"

//-------------------------------------------------------------------
//
// AffectorRibbon
//
AffectorRibbon::AffectorRibbon () :
	AffectorBoundaryPoly (TAG_ARIB, TGAT_ribbon),
	m_cachedTerrainShaderFamilyId (-1),
	m_cachedSgi (),
	m_waterShaderSize(64.0f),
	m_velocity(1.0f),
	m_capWidth(4.0f),
	m_terrainShaderFamilyId (0),
	m_featherFunctionTerrainShader (TGFF_linear),
	m_featherDistanceTerrainShader (0.5f),
	m_ribbonWaterShaderTemplateName (0),
	m_waterType(TGWT_water),
	m_heightList (),
	m_endCapPointList (),
	m_endCapExtent()
{
}

//-------------------------------------------------------------------

AffectorRibbon::~AffectorRibbon ()
{
	if (m_ribbonWaterShaderTemplateName)
	{
		delete [] m_ribbonWaterShaderTemplateName;
		m_ribbonWaterShaderTemplateName = 0;
	}
}

//-------------------------------------------------------------------

void AffectorRibbon::setWaterShaderSize (float val)
{
	m_waterShaderSize = val;
}

//-------------------------------------------------------------------

void AffectorRibbon::setVelocity (float val)
{
	m_velocity = val;
}

//-------------------------------------------------------------------

void AffectorRibbon::setCapWidth (float val)
{
	m_capWidth = val;
}

//-------------------------------------------------------------------
bool AffectorRibbon::affectsHeight () const
{
	return false;
}


//-------------------------------------------------------------------

void AffectorRibbon::setTerrainShaderFamilyId (const int newFamilyId)
{
	m_terrainShaderFamilyId = newFamilyId;
}

//-------------------------------------------------------------------

void AffectorRibbon::setFeatherFunctionTerrainShader (TerrainGeneratorFeatherFunction newFeatherFunction)
{
	m_featherFunctionTerrainShader = newFeatherFunction;
}

//-------------------------------------------------------------------

void AffectorRibbon::setFeatherDistanceTerrainShader (const float newFeatherDistance)
{
	m_featherDistanceTerrainShader = newFeatherDistance;
}

//-------------------------------------------------------------------

bool AffectorRibbon::affectsShader () const
{
	return true;
}

//-------------------------------------------------------------------

void AffectorRibbon::prepare ()
{
	m_cachedTerrainShaderFamilyId = -1;	
}

//-------------------------------------------------------------------

void AffectorRibbon::copyHeightList (const ArrayList<float>& newHeightList)
{
	m_heightList = newHeightList;
}

//-------------------------------------------------------------------

void AffectorRibbon::createInitialHeightList ()
{
	m_heightList.clear ();
	int size = m_pointList.getNumberOfElements ();
	for(int i = 0; i < size; i++)
	{
		m_heightList.add(0.0f);
	}
}

//-------------------------------------------------------------------

void AffectorRibbon::generateEndCapPointList ()
{
	// tapered square - 8 sides - not a perfect octagon
	m_endCapPointList.clear();

	if(m_capWidth > 0 && m_pointList.getNumberOfElements() > 1)
	{
		Vector2d up_vect = m_pointList[0] - m_pointList[1];
		up_vect.normalize();
		Vector2d down_vect = -up_vect;
		Vector2d right_vect = Vector2d(up_vect.y,-up_vect.x);
		Vector2d left_vect = -right_vect;

		// 0
		Vector2d currentPoint = m_pointList[0] + (left_vect * m_capWidth/2.0f);
		m_endCapPointList.add(currentPoint);

		up_vect *= m_capWidth;
		down_vect *= m_capWidth;
		right_vect *= m_capWidth;
		left_vect *= m_capWidth;

		// 1
		currentPoint = m_endCapPointList[0] + up_vect + left_vect;
		m_endCapPointList.add(currentPoint);

		// 2
		currentPoint = m_endCapPointList[1] + up_vect;
		m_endCapPointList.add(currentPoint);

		// 3
		currentPoint = m_endCapPointList[2] + right_vect + up_vect;
		m_endCapPointList.add(currentPoint);

		// 4
		currentPoint = m_endCapPointList[3] + right_vect;
		m_endCapPointList.add(currentPoint);

		// 5
		currentPoint = m_endCapPointList[4] + down_vect + right_vect;
		m_endCapPointList.add(currentPoint);

		// 6
		currentPoint = m_endCapPointList[5] + down_vect;
		m_endCapPointList.add(currentPoint);

		// 7
		currentPoint = m_endCapPointList[6] + left_vect + down_vect;
		m_endCapPointList.add(currentPoint);

		
	}
}

//-------------------------------------------------------------------

unsigned AffectorRibbon::getAffectedMaps() const
{
	return unsigned(TGM_ALL);
	// TODO - make this correctly report the maps set by 'affect'
}

//-------------------------------------------------------------------

void AffectorRibbon::affect (const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (!isEnabled ())
		return;

	if (m_terrainShaderFamilyId == 0)
		return;

	
	if (amount > 0.f)
	{
		if (m_extent.isWithin (worldX, worldZ))
		{
			const float  width_2        = m_width * 0.5f;
			bool found = false;
			FindData result;
			if (find (Vector2d (worldX, worldZ), width_2, result,true)) // check the strip
			{
				const float distanceToCenter = fabsf (result.distanceToCenter);
				if (WithinRangeInclusiveInclusive (0.f, distanceToCenter, width_2 * (1.f - getFeatherDistanceTerrainShader ())))
				{
					found = true;
				}
			}
			if(!found && m_endCapExtent.isWithin(worldX, worldZ)) // check the endcap
			{
				Vector2d centerOfEndCap = m_endCapExtent.getCenter();
				const float distanceToCenter = centerOfEndCap.magnitudeBetween(Vector2d(worldX,worldZ));
				if(WithinRangeInclusiveInclusive (0.f, distanceToCenter, (width_2 * 3.0f) * (1.f - getFeatherDistanceTerrainShader())))
				{
					found = true;
				}
			}
			if(found)
			{
				//-- set the shader
				if (m_cachedTerrainShaderFamilyId != m_terrainShaderFamilyId)
				{
					m_cachedTerrainShaderFamilyId = m_terrainShaderFamilyId;
					m_cachedSgi      = generatorChunkData.shaderGroup->chooseShader (m_terrainShaderFamilyId);
				}

				FastRandomGenerator randomGenerator(CoordinateHash::hashTuple(worldX, worldZ));
				ShaderGroup::Info sgi = m_cachedSgi;
				sgi.setChildChoice (randomGenerator.randomFloat());
				generatorChunkData.shaderMap->setData (x, z, sgi);
			}
		}
	}
}

//-------------------------------------------------------------------

void AffectorRibbon::setRibbonWaterShaderTemplateName (const char* newRibbonWaterShaderTemplateName)
{
	if (m_ribbonWaterShaderTemplateName)
	{
		delete [] m_ribbonWaterShaderTemplateName;
		m_ribbonWaterShaderTemplateName = 0;
	}

	if (newRibbonWaterShaderTemplateName)
		m_ribbonWaterShaderTemplateName = DuplicateString (newRibbonWaterShaderTemplateName);
}

//-------------------------------------------------------------------

void AffectorRibbon::setWaterType (TerrainGeneratorWaterType newWaterType)
{
	m_waterType = newWaterType;
}

//-------------------------------------------------------------------

void AffectorRibbon::load (Iff& iff)
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

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("invalid AffectorRibbon version %s/%s", buffer, tagBuffer));
		}
		break;
	}

	generateEndCapPointList();
	recalculate ();
}

//-------------------------------------------------------------------

void AffectorRibbon::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			m_heightData.load (iff);
			m_heightData.clear(); // we don't use m_heightdata anymore

			iff.enterChunk (TAG_DATA);

				const int n = iff.read_int32 ();
				int i;
				for (i = 0; i < n; ++i)
				{
					Vector2d point;
					point.x = iff.read_float ();
					point.y = iff.read_float ();
					addPoint (point);
					m_heightList.add(0.0f);
				}

				setWidth (iff.read_float ());
				IGNORE_RETURN(iff.read_float ()); // old depth
				setWaterShaderSize (64.0f);
				setVelocity (1.0f);
				setCapWidth(getWidth()); // default cap size of old ribbons to width of ribbon 
				IGNORE_RETURN(iff.read_int32 ()); // old forward direction

				char* templateName = iff.read_string ();
				setRibbonWaterShaderTemplateName (templateName);
				delete [] templateName;

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorRibbon::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			m_heightData.load (iff);
			m_heightData.clear(); // we don't use m_heightdata anymore

			iff.enterChunk (TAG_DATA);

				const int n = iff.read_int32 ();
				int i;
				for (i = 0; i < n; ++i)
				{
					Vector2d point;
					point.x = iff.read_float ();
					point.y = iff.read_float ();
					addPoint (point);
					m_heightList.add(0.0f);
				}

				setWidth (iff.read_float ());
				IGNORE_RETURN(iff.read_float ()); // old depth
				setWaterShaderSize (iff.read_float ());
				setVelocity (iff.read_float ());
				setCapWidth(getWidth()); // default cap size of old ribbons to width of ribbon 
				IGNORE_RETURN(iff.read_int32 ()); // old forward direction

				char* templateName = iff.read_string ();
				setRibbonWaterShaderTemplateName (templateName);
				delete [] templateName;

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void AffectorRibbon::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			m_heightData.load (iff);
			m_heightData.clear(); // we don't use m_heightdata anymore

			iff.enterChunk (TAG_DATA);

				const int n = iff.read_int32 ();
				int i;
				for (i = 0; i < n; ++i)
				{
					Vector2d point;
					point.x = iff.read_float ();
					point.y = iff.read_float ();
					addPoint (point);
					m_heightList.add(0.0f);
				}

				setWidth (iff.read_float ());
				IGNORE_RETURN(iff.read_float ()); // old depth
				setWaterShaderSize (iff.read_float ());
				setVelocity (iff.read_float ());
				setCapWidth(getWidth()); // default cap size of old ribbons to width of ribbon 
				IGNORE_RETURN(iff.read_int32 ()); // old forward direction

				char* templateName = iff.read_string ();
				setRibbonWaterShaderTemplateName (templateName);
				delete [] templateName;

				setWaterType(static_cast<TerrainGeneratorWaterType> (iff.read_int32 ()));

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void AffectorRibbon::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			//m_heightData.load (iff);

			iff.enterChunk (TAG_DATA);

				const int n = iff.read_int32 ();
				int i;
				for (i = 0; i < n; ++i)
				{
					Vector2d point;
					point.x = iff.read_float ();
					point.y = iff.read_float ();
					addPoint (point);
					m_heightList.add(iff.read_float());
				}

				setWidth (iff.read_float ());
				setWaterShaderSize (iff.read_float ());
				setVelocity (iff.read_float ());
				setCapWidth(getWidth()); // default cap size of old ribbons to width of ribbon 
				IGNORE_RETURN(iff.read_int32 ()); // old forward direction

				char* templateName = iff.read_string ();
				setRibbonWaterShaderTemplateName (templateName);
				delete [] templateName;

				setWaterType(static_cast<TerrainGeneratorWaterType> (iff.read_int32 ()));

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

void AffectorRibbon::load_0004 (Iff& iff)
{
	iff.enterForm (TAG_0004);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			//m_heightData.load (iff);

			iff.enterChunk (TAG_DATA);

				const int n = iff.read_int32 ();
				int i;
				for (i = 0; i < n; ++i)
				{
					Vector2d point;
					point.x = iff.read_float ();
					point.y = iff.read_float ();
					addPoint (point);
					m_heightList.add(iff.read_float());
				}

				setWidth (iff.read_float ());
				setWaterShaderSize (iff.read_float ());
				setVelocity (iff.read_float ());
				setCapWidth(iff.read_float ()); 

				char* templateName = iff.read_string ();
				setRibbonWaterShaderTemplateName (templateName);
				delete [] templateName;

				setWaterType(static_cast<TerrainGeneratorWaterType> (iff.read_int32 ()));

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------

void AffectorRibbon::load_0005 (Iff& iff)
{
	iff.enterForm (TAG_0005);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			//m_heightData.load (iff);

			iff.enterChunk (TAG_DATA);

				const int n = iff.read_int32 ();
				int i;
				for (i = 0; i < n; ++i)
				{
					Vector2d point;
					point.x = iff.read_float ();
					point.y = iff.read_float ();
					addPoint (point);
					m_heightList.add(iff.read_float());
				}

				setWidth (iff.read_float ());
				setWaterShaderSize (iff.read_float ());
				setVelocity (iff.read_float ());
				setCapWidth(iff.read_float ()); 
				setTerrainShaderFamilyId(iff.read_int32());
				setFeatherFunctionTerrainShader (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				const float newFeatherDistanceTerrainShader = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistanceTerrainShader (newFeatherDistanceTerrainShader);

				char* templateName = iff.read_string ();
				setRibbonWaterShaderTemplateName (templateName);
				delete [] templateName;

				setWaterType(static_cast<TerrainGeneratorWaterType> (iff.read_int32 ()));

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0005);
}

//-------------------------------------------------------------------

void AffectorRibbon::save (Iff& iff) const
{
	iff.insertForm (TAG_0005);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertForm (TAG_DATA);

			//m_heightData.save (iff);

			iff.insertChunk (TAG_DATA);

				iff.insertChunkData (m_pointList.getNumberOfElements ());
				int i;
				for (i = 0; i < m_pointList.getNumberOfElements (); ++i)
				{
					iff.insertChunkData (getPoint (i).x);
					iff.insertChunkData (getPoint (i).y);
					iff.insertChunkData (m_heightList[i]);
				}

				iff.insertChunkData (getWidth ());
				iff.insertChunkData (getWaterShaderSize ());
				iff.insertChunkData (getVelocity ());
				iff.insertChunkData (getCapWidth ());
				iff.insertChunkData (getTerrainShaderFamilyId ());
				iff.insertChunkData (static_cast<int32> (getFeatherFunctionTerrainShader ()));
				iff.insertChunkData (getFeatherDistanceTerrainShader ());
				iff.insertChunkString (getRibbonWaterShaderTemplateName () ? getRibbonWaterShaderTemplateName () : "");
				iff.insertChunkData (static_cast<int32> (getWaterType()));

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0005);
}

//-------------------------------------------------------------------

struct Segment
{
	Vector topControlPoint;
	Vector bottomControlPoint;
	Vector direction;
	Vector topNormal;
	Vector bottomNormal;
	
};

void AffectorRibbon::createQuadList (ArrayList<AffectorRibbon::Quad>& quadList) const
{
	ArrayList<Segment> segmentList;

	const int pointListSize = m_pointList.size();
	
	if(pointListSize < 2)
	{
		DEBUG_FATAL(true,("void AffectorRibbon::createQuadList - less than 2 control points for ribbon"));
		return;
	}

	const float width = m_width/2.0f;

	// get control points, calc direction, and initial normals
	int i;
	for (i = 0; i < pointListSize; ++i) 
	{
		
		Segment segment;
		if(i == 0) // handle first segment
		{
			Vector nextSegmentTopControlPoint = Vector(m_pointList[i+1].x,m_heightList[i+1],m_pointList[i+1].y);

			segment.bottomControlPoint = Vector(m_pointList[i].x,m_heightList[i],m_pointList[i].y);
			segment.direction = nextSegmentTopControlPoint - segment.bottomControlPoint;
			segment.direction.y = 0.0f;
			segment.direction.normalize();
			segment.topControlPoint = segment.bottomControlPoint - (segment.direction * width);
			segment.topNormal = segment.bottomNormal = Vector(segment.direction.z,0.0f,-segment.direction.x);
			segmentList.add(segment);
		}
		
		if(i == pointListSize - 1) // handle last segment
		{
			Segment lastSegment = segmentList[segmentList.size() - 1];
			segment.direction = lastSegment.direction;
			segment.topControlPoint = lastSegment.bottomControlPoint;
			segment.bottomControlPoint = segment.topControlPoint + (segment.direction * width);
			segment.bottomControlPoint.y = segment.topControlPoint.y;
			segment.topNormal = segment.bottomNormal = Vector(segment.direction.z,0.0f,-segment.direction.x);
			segmentList.add(segment);
		}
		else // handle middle segments
		{
			segment.topControlPoint = Vector(m_pointList[i].x,m_heightList[i],m_pointList[i].y);
			segment.bottomControlPoint = Vector(m_pointList[i+1].x,m_heightList[i+1],m_pointList[i+1].y);
			segment.direction = segment.bottomControlPoint - segment.topControlPoint;
			segment.direction.y = 0.0f;
			segment.direction.normalize();
			segment.topNormal = segment.bottomNormal = Vector(segment.direction.z,0.0f,-segment.direction.x);
			segmentList.add(segment);
		}
	}

	// walk through the segment list and fix the shared normals
	const int segmentListSize = segmentList.size();
	for(i = 1; i < segmentListSize; i++) // skip the first segment - it's bottom normal gets fixed by the next segment
	{
		segmentList[i].topNormal = (segmentList[i].topNormal + segmentList[i - 1].bottomNormal) / 2.0f;
		segmentList[i - 1].bottomNormal = segmentList[i].topNormal;
	}

	// walk the segments and build up the points for the quad
	for(i = 0; i < segmentListSize; i++)
	{
		AffectorRibbon::Quad quad;

		quad.points[0] = (segmentList[i].topNormal * width) + segmentList[i].topControlPoint;
		quad.points[1] = (-segmentList[i].topNormal * width) + segmentList[i].topControlPoint;
		quad.points[2] = (-segmentList[i].bottomNormal * width) + segmentList[i].bottomControlPoint;
		quad.points[3] = (segmentList[i].bottomNormal * width) + segmentList[i].bottomControlPoint;	
		quadList.add (quad);
	}
}

//-------------------------------------------------------------------

void AffectorRibbon::recalculate ()
{
	AffectorBoundaryPoly::recalculate();

	m_endCapExtent.x0 = FLT_MAX;
	m_endCapExtent.y0 = FLT_MAX;
	m_endCapExtent.x1 = -FLT_MAX;
	m_endCapExtent.y1 = -FLT_MAX;

	int i;
	for (i = 0; i < m_endCapPointList.getNumberOfElements (); ++i)
	{
		m_extent.expand (m_endCapPointList [i].x, m_endCapPointList [i].y);
		m_endCapExtent.expand (m_endCapPointList [i].x, m_endCapPointList [i].y);
	}	
}







