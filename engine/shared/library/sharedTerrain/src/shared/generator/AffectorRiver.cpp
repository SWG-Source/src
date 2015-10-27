//===================================================================
//
// AffectorRiver.cpp
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/AffectorRiver.h"

#include "sharedTerrain/Affector.h"
#include "sharedTerrain/CoordinateHash.h"
#include "sharedRandom/FastRandomGenerator.h"
#include "sharedFile/Iff.h"

#include <algorithm>

//===================================================================

AffectorRiver::AffectorRiver () :
	AffectorBoundaryPoly (TAG_ARIV, TGAT_river),
	m_cachedBankFamilyId (-1),
	m_cachedBankSgi (),
	m_cachedBottomFamilyId (-1),
	m_cachedBottomSgi (),
	m_multiFractal (),
	m_bankFamilyId (0), 
	m_bottomFamilyId (0), 
	m_trenchDepth (0.f),
	m_hasLocalWaterTable (false),
	m_localWaterTableDepth (2.f),
	m_localWaterTableWidth (4.f),
	m_localWaterTableShaderSize (2.f),
	m_localWaterTableShaderTemplateName (0),
	m_velocity (0.f),
	m_waterType(TGWT_water)

{ 
	m_multiFractal.setScale (0.01f);
	m_multiFractal.setGain (true, 0.9f);
	m_multiFractal.setNumberOfOctaves (3);
}

//-------------------------------------------------------------------

AffectorRiver::~AffectorRiver ()
{
	if (m_localWaterTableShaderTemplateName)
	{
		delete [] m_localWaterTableShaderTemplateName;
		m_localWaterTableShaderTemplateName = 0;
	}
}

//-------------------------------------------------------------------

void AffectorRiver::setTrenchDepth (const float newTrenchDepth)
{
	m_trenchDepth = newTrenchDepth;
}

//-------------------------------------------------------------------

void AffectorRiver::setVelocity (const float newVelocity)
{
	m_velocity = newVelocity;
}

//-------------------------------------------------------------------

void AffectorRiver::setBankFamilyId (const int newBankFamilyId)
{
	m_bankFamilyId = newBankFamilyId;
}

//-------------------------------------------------------------------

void AffectorRiver::setBottomFamilyId (const int newBottomFamilyId)
{
	m_bottomFamilyId = newBottomFamilyId;
}

//-------------------------------------------------------------------

void AffectorRiver::setWaterType (TerrainGeneratorWaterType newWaterType)
{
	m_waterType = newWaterType;
}

//-------------------------------------------------------------------

bool AffectorRiver::affectsHeight () const
{
	return true;
}

//-------------------------------------------------------------------

bool AffectorRiver::affectsShader () const
{
	return true;
}

//-------------------------------------------------------------------

unsigned AffectorRiver::getAffectedMaps() const
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

void AffectorRiver::createHeightData ()
{
	m_heightData.createRiverData ();
}

//-------------------------------------------------------------------

void AffectorRiver::prepare ()
{
	m_cachedBankFamilyId   = -1;
	m_cachedBottomFamilyId = -1;
}

//-------------------------------------------------------------------

void AffectorRiver::affect (const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
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
			const float width_2        = m_width * 0.5f;
			const float originalHeight = generatorChunkData.heightMap->getData (x, z);

			FindData result;
			if (find (Vector2d (worldX, worldZ), width_2, result))
			{
				//-- set the height
				const float distanceToCenter = fabsf (result.distanceToCenter);
				const float desiredHeight    = result.height - m_trenchDepth;
				const float subWidth         = width_2 * m_multiFractal.getValue (result.t);
				const float feather          = subWidth * (1.f - getFeatherDistance ());

				FastRandomGenerator randomGenerator(CoordinateHash::hashTuple(worldX, worldZ));

				if (distanceToCenter <= feather)
				{
					generatorChunkData.heightMap->setData (x, z, desiredHeight);

					//-- set the shader
					if (m_cachedBottomFamilyId != m_bottomFamilyId)
					{
						m_cachedBottomFamilyId = m_bottomFamilyId;
						m_cachedBottomSgi      = generatorChunkData.shaderGroup->chooseShader (m_bottomFamilyId);
					}

					ShaderGroup::Info sgi = m_cachedBottomSgi;
					sgi.setChildChoice (randomGenerator.randomFloat());
					generatorChunkData.shaderMap->setData (x, z, sgi);

					//-- clear the flora
					generatorChunkData.floraStaticCollidableMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
					generatorChunkData.floraStaticNonCollidableMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
					generatorChunkData.floraDynamicNearMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
					generatorChunkData.floraDynamicFarMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
				}
				else
				{
					if (distanceToCenter <= subWidth)
					{
						//-- height
						const float t = distanceToCenter / subWidth;
						DEBUG_FATAL (t < 0.f || t > 1.f, ("t is out of range"));

						generatorChunkData.heightMap->setData (x, z, linearInterpolate (desiredHeight, originalHeight, sqr (t)));

						if (m_cachedBankFamilyId != m_bankFamilyId)
						{
							m_cachedBankFamilyId = m_bankFamilyId;
							m_cachedBankSgi      = generatorChunkData.shaderGroup->chooseShader (m_bankFamilyId);
						}

						ShaderGroup::Info sgi = m_cachedBankSgi;
						sgi.setChildChoice(randomGenerator.randomFloat());
						generatorChunkData.shaderMap->setData (x, z, sgi);
					}
				}
			}
		}
	}
}

void AffectorRiver::_legacyAffect(const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (!isEnabled ())
		return;

	if (amount > 0.f)
	{
		if (m_extent.isWithin (worldX, worldZ))
		{
			const float width_2        = m_width * 0.5f;
			const float originalHeight = generatorChunkData.heightMap->getData (x, z);

			FindData result;
			if (find (Vector2d (worldX, worldZ), width_2, result))
			{
				//-- set the height
				const float distanceToCenter = fabsf (result.distanceToCenter);
				const float desiredHeight    = result.height - m_trenchDepth;
				const float subWidth         = width_2 * m_multiFractal.getValue (result.t);
				const float feather          = subWidth * (1.f - getFeatherDistance ());

				if (distanceToCenter <= feather)
				{
					generatorChunkData.heightMap->setData (x, z, desiredHeight);

					//-- set the shader
					if (m_cachedBottomFamilyId != m_bottomFamilyId)
					{
						m_cachedBottomFamilyId = m_bottomFamilyId;
						m_cachedBottomSgi      = generatorChunkData.shaderGroup->chooseShader (m_bottomFamilyId);
					}

					ShaderGroup::Info sgi = m_cachedBottomSgi;
					sgi.setChildChoice (generatorChunkData.m_legacyRandomGenerator->randomReal (0.0f, 1.0f));
					generatorChunkData.shaderMap->setData (x, z, sgi);

					//-- clear the flora
					generatorChunkData.floraStaticCollidableMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
					generatorChunkData.floraStaticNonCollidableMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
					generatorChunkData.floraDynamicNearMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
					generatorChunkData.floraDynamicFarMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
				}
				else
				{
					if (distanceToCenter <= subWidth)
					{
						//-- height
						const float t = distanceToCenter / subWidth;
						DEBUG_FATAL (t < 0.f || t > 1.f, ("t is out of range"));

						generatorChunkData.heightMap->setData (x, z, linearInterpolate (desiredHeight, originalHeight, sqr (t)));

						if (m_cachedBankFamilyId != m_bankFamilyId)
						{
							m_cachedBankFamilyId = m_bankFamilyId;
							m_cachedBankSgi      = generatorChunkData.shaderGroup->chooseShader (m_bankFamilyId);
						}

						ShaderGroup::Info sgi = m_cachedBankSgi;
						sgi.setChildChoice (generatorChunkData.m_legacyRandomGenerator->randomReal (0.0f, 1.0f));
						generatorChunkData.shaderMap->setData (x, z, sgi);
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------------

void AffectorRiver::load (Iff& iff)
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
			DEBUG_FATAL (true, ("invalid AffectorRiver version %s/%s", buffer, tagBuffer));
		}
		break;
	}

	recalculate ();
}

//-------------------------------------------------------------------

void AffectorRiver::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

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
				setBankFamilyId (iff.read_int32 ());
				setBottomFamilyId (getBankFamilyId ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorRiver::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

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
				setBankFamilyId (iff.read_int32 ());
				setBottomFamilyId (getBankFamilyId ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);

				setTrenchDepth (iff.read_float ());
				setVelocity (iff.read_float ());
				setLocalWaterTableShaderSize (iff.read_float ());

				char* templateName = iff.read_string ();
				setLocalWaterTableShaderTemplateName (templateName);
				delete [] templateName;

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void AffectorRiver::load_0002 (Iff& iff)
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
				setBankFamilyId (iff.read_int32 ());
				setBottomFamilyId (getBankFamilyId ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);

				setTrenchDepth (iff.read_float ());
				setVelocity (iff.read_float ());

				m_hasLocalWaterTable = iff.read_int32 () != 0;
				setLocalWaterTableDepth (iff.read_float ());
				setLocalWaterTableShaderSize (iff.read_float ());
				setLocalWaterTableWidth (getWidth ());

				char* templateName = iff.read_string ();
				setLocalWaterTableShaderTemplateName (templateName);
				delete [] templateName;

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void AffectorRiver::load_0003 (Iff& iff)
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
				setBankFamilyId (iff.read_int32 ());
				setBottomFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);

				setTrenchDepth (iff.read_float ());
				setVelocity (iff.read_float ());

				m_hasLocalWaterTable = iff.read_int32 () != 0;
				setLocalWaterTableDepth (iff.read_float ());
				setLocalWaterTableShaderSize (iff.read_float ());
				setLocalWaterTableWidth (getWidth ());

				char* templateName = iff.read_string ();
				setLocalWaterTableShaderTemplateName (templateName);
				delete [] templateName;

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

void AffectorRiver::load_0004 (Iff& iff)
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
				setBankFamilyId (iff.read_int32 ());
				setBottomFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);

				setTrenchDepth (iff.read_float ());
				setVelocity (iff.read_float ());

				m_hasLocalWaterTable = iff.read_int32 () != 0;
				setLocalWaterTableDepth (iff.read_float ());
				setLocalWaterTableShaderSize (iff.read_float ());
				setLocalWaterTableWidth (getWidth ());

				char* templateName = iff.read_string ();
				setLocalWaterTableShaderTemplateName (templateName);
				delete [] templateName;

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------

void AffectorRiver::load_0005 (Iff& iff)
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
				setBankFamilyId (iff.read_int32 ());
				setBottomFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);

				setTrenchDepth (iff.read_float ());
				setVelocity (iff.read_float ());

				m_hasLocalWaterTable = iff.read_int32 () != 0;
				setLocalWaterTableDepth (iff.read_float ());
				setLocalWaterTableWidth (iff.read_float ());
				setLocalWaterTableShaderSize (iff.read_float ());

				char* templateName = iff.read_string ();
				setLocalWaterTableShaderTemplateName (templateName);
				delete [] templateName;

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0005);
}

//-------------------------------------------------------------------

void AffectorRiver::load_0006 (Iff& iff)
{
	iff.enterForm (TAG_0006);

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
				setBankFamilyId (iff.read_int32 ());
				setBottomFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

				const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
				setFeatherDistance (newFeatherDistance);

				setTrenchDepth (iff.read_float ());
				setVelocity (iff.read_float ());

				m_hasLocalWaterTable = iff.read_int32 () != 0;
				setLocalWaterTableDepth (iff.read_float ());
				setLocalWaterTableWidth (iff.read_float ());
				setLocalWaterTableShaderSize (iff.read_float ());

				char* templateName = iff.read_string ();
				setLocalWaterTableShaderTemplateName (templateName);
				delete [] templateName;

				setWaterType(static_cast<TerrainGeneratorWaterType> (iff.read_int32 ()));


			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0006);
}

//-------------------------------------------------------------------

void AffectorRiver::save (Iff& iff) const
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

				iff.insertChunkData (getWidth ());
				iff.insertChunkData (getBankFamilyId ());
				iff.insertChunkData (getBottomFamilyId ());
				iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
				iff.insertChunkData (getFeatherDistance ());
				iff.insertChunkData (getTrenchDepth ());
				iff.insertChunkData (getVelocity ());
				iff.insertChunkData (getHasLocalWaterTable () ? static_cast<int32> (1) : static_cast<int32> (0));
				iff.insertChunkData (getLocalWaterTableDepth ());
				iff.insertChunkData (getLocalWaterTableWidth ());
				iff.insertChunkData (getLocalWaterTableShaderSize ());
				iff.insertChunkString (getLocalWaterTableShaderTemplateName () ? getLocalWaterTableShaderTemplateName () : "");
				iff.insertChunkData (static_cast<int32> (getWaterType()));

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0006);
}

//-------------------------------------------------------------------

void AffectorRiver::createWaterTables (ArrayList<AffectorRiver::WaterTable>& waterTableList) const
{
	Vector      previousLeftStrip;
	Vector      previousRightStrip;
	const float waterWidth  = m_localWaterTableWidth * 0.5f;

	int i;
	for (i = 0; i < m_pointList.size () - 1; ++i)
	{
		//
		// setup initial states
		//
		const bool      atStart               = i == 0;
		const bool      atEndNext             = i == (m_pointList.size () - 2);

		const Vector2d& currentStart          = m_pointList [i];
		const Vector2d& currentEnd            = m_pointList [i + 1];
		const Vector2d  currentSegment        = currentEnd - currentStart;
		const Vector2d  currentDirection      = Vector2d::normalized (currentSegment);
		const Vector2d  currentNormal         = Vector2d::normal (currentDirection, false);
		const float     currentSegmentLength  = currentSegment.magnitude ();
		const Vector2d  currentStartLeft      = currentStart - currentNormal * waterWidth;
		const Vector2d  currentStartRight     = currentStart + currentNormal * waterWidth;
		const Vector2d  currentEndLeft        = currentEnd   - currentNormal * waterWidth;
		const Vector2d  currentEndRight       = currentEnd   + currentNormal * waterWidth;

		const Vector2d& previousStart         = !atStart ? m_pointList [i - 1] : m_pointList [i];
		const Vector2d& previousEnd           = m_pointList [i];
		const Vector2d  previousSegment       = previousEnd - previousStart;
		const Vector2d  previousDirection     = !atStart ? Vector2d::normalized (previousSegment) : currentDirection;
		const Vector2d  previousNormal        = !atStart ? Vector2d::normal (previousDirection, false) : currentNormal;
		const float     previousSegmentLength = previousSegment.magnitude ();
		const Vector2d  previousStartLeft     = previousStart - previousNormal * waterWidth;
		const Vector2d  previousStartRight    = previousStart + previousNormal * waterWidth;
		const Vector2d  previousEndLeft       = previousEnd   - previousNormal * waterWidth;
		const Vector2d  previousEndRight      = previousEnd   + previousNormal * waterWidth;

		const Vector2d& nextStart             = m_pointList [i + 1];
		const Vector2d& nextEnd               = !atEndNext ? m_pointList [i + 2] : m_pointList [i + 1];
		const Vector2d  nextSegment           = nextEnd - nextStart;
		const Vector2d  nextDirection         = !atEndNext ? Vector2d::normalized (nextSegment) : currentDirection;
		const Vector2d  nextNormal            = !atEndNext ? Vector2d::normal (nextDirection, false) : currentNormal;
		const float     nextSegmentLength     = nextSegment.magnitude ();
		const Vector2d  nextStartLeft         = nextStart - nextNormal * waterWidth;
		const Vector2d  nextStartRight        = nextStart + nextNormal * waterWidth;
		const Vector2d  nextEndLeft           = nextEnd   - nextNormal * waterWidth;
		const Vector2d  nextEndRight          = nextEnd   + nextNormal * waterWidth;

		const Vector2d  left [6] =
		{
			!atStart ? Vector2d::linearInterpolate (previousStartLeft, previousEndLeft, ((previousSegmentLength - waterWidth) / previousSegmentLength)) : currentStartLeft,
			!atStart ? currentStart - (Vector2d::normalized (currentStart - (previousEndLeft + currentStartLeft) * 0.5f) * waterWidth) : currentStartLeft,
			Vector2d::linearInterpolate (currentStartLeft,  currentEndLeft, waterWidth / currentSegmentLength),
			Vector2d::linearInterpolate (currentStartLeft,  currentEndLeft, ((currentSegmentLength - waterWidth) / currentSegmentLength)),
			!atEndNext ? currentEnd - (Vector2d::normalized (currentEnd   - (currentEndLeft + nextStartLeft) * 0.5f) * waterWidth) : currentEndLeft,
			!atEndNext ? Vector2d::linearInterpolate (nextStartLeft, nextEndLeft, waterWidth / nextSegmentLength) : nextStartLeft
		};

		const Vector2d  right [6] = 
		{
			!atStart ? Vector2d::linearInterpolate (previousStartRight, previousEndRight, ((previousSegmentLength - waterWidth) / previousSegmentLength)) : currentStartRight,
			!atStart ? currentStart - (Vector2d::normalized (currentStart - (previousEndRight + currentStartRight) * 0.5f) * waterWidth) : currentStartRight,
			Vector2d::linearInterpolate (currentStartRight,  currentEndRight, waterWidth / currentSegmentLength),
			Vector2d::linearInterpolate (currentStartRight,  currentEndRight, ((currentSegmentLength - waterWidth) / currentSegmentLength)),
			!atEndNext ? currentEnd - (Vector2d::normalized (currentEnd   - (currentEndRight + nextStartRight) * 0.5f) * waterWidth) : currentEndRight,
			!atEndNext ? Vector2d::linearInterpolate (nextStartRight, nextEndRight, waterWidth / nextSegmentLength) : nextStartRight
		};

		if (!atStart)
		{
			if (left [0].magnitudeBetweenSquared (left [2]) < right [0].magnitudeBetweenSquared (right [2]))
				const_cast<Vector2d*> (left) [1] = (left [0] + left [2]) * 0.5f;
			else
				const_cast<Vector2d*> (right) [1] = (right [0] + right [2]) * 0.5f;
		}

		if (!atEndNext)
		{
			if (left [3].magnitudeBetweenSquared (left [5]) < right [3].magnitudeBetweenSquared (right [5]))
				const_cast<Vector2d*> (left) [4] = (left [3] + left [5]) * 0.5f;
			else
				const_cast<Vector2d*> (right) [4] = (right [3] + right [5]) * 0.5f;
		}

		//
		// walk the current segment
		//
		{
			int j;
			for (j = 0; j < m_heightData.getNumberOfPoints (i); ++j)
			{
				const Vector2d point (m_heightData.getPoint (i, j).x, m_heightData.getPoint (i, j).z);

				Vector2d leftStrip  = point - currentNormal * waterWidth;
				Vector2d rightStrip = point + currentNormal * waterWidth;

				const float segmentLength = currentStart.magnitudeBetween (point);

				if (segmentLength < waterWidth)
				{
					const float t = segmentLength / waterWidth;
					leftStrip     = Vector2d::linearInterpolate (left [1], left [2], t);
					rightStrip    = Vector2d::linearInterpolate (right [1], right [2], t);
				}
				else
					if (segmentLength > currentSegmentLength - waterWidth)
					{
						const float t = (segmentLength + waterWidth - currentSegmentLength) / waterWidth;
						leftStrip     = Vector2d::linearInterpolate (left [3], left [4], t);
						rightStrip    = Vector2d::linearInterpolate (right [3], right [4], t);
					}

				if (j == 0)
				{
					if (atStart)
					{
						const Vector2d initialLeft  = currentStartLeft - (currentDirection * waterWidth);
						const Vector2d initialRight = currentStartRight - (currentDirection * waterWidth);

						AffectorRiver::WaterTable table;
						table.points [0].set (initialRight.x, m_heightData.getPoint (i, j).y - m_localWaterTableDepth, initialRight.y);
						table.points [1].set (initialLeft.x, m_heightData.getPoint (i, j).y - m_localWaterTableDepth, initialLeft.y);
						table.points [2].set (leftStrip.x, m_heightData.getPoint (i, j).y - m_localWaterTableDepth, leftStrip.y);
						table.points [3].set (rightStrip.x, m_heightData.getPoint (i, j).y - m_localWaterTableDepth, rightStrip.y);
						table.direction = currentDirection;
						waterTableList.add (table);
					}
				}
				else
				{
					AffectorRiver::WaterTable table;
					table.points [0] = previousRightStrip;
					table.points [1] = previousLeftStrip;
					table.points [2].set (leftStrip.x, m_heightData.getPoint (i, j).y - m_localWaterTableDepth, leftStrip.y);
					table.points [3].set (rightStrip.x, m_heightData.getPoint (i, j).y - m_localWaterTableDepth, rightStrip.y);
					table.direction = currentDirection;
					waterTableList.add (table);
				}

				previousLeftStrip.set (leftStrip.x, m_heightData.getPoint (i, j).y - m_localWaterTableDepth, leftStrip.y);
				previousRightStrip.set (rightStrip.x, m_heightData.getPoint (i, j).y - m_localWaterTableDepth, rightStrip.y);

				if (j == m_heightData.getNumberOfPoints (i) - 1)
				{
					if (atEndNext)
					{
						const Vector2d finalLeft  = currentEndLeft + (currentDirection * waterWidth);
						const Vector2d finalRight = currentEndRight + (currentDirection * waterWidth);

						AffectorRiver::WaterTable table;
						table.points [0] = previousRightStrip;
						table.points [1] = previousLeftStrip;
						table.points [2].set (finalLeft.x, m_heightData.getPoint (i, j).y - m_localWaterTableDepth, finalLeft.y);
						table.points [3].set (finalRight.x, m_heightData.getPoint (i, j).y - m_localWaterTableDepth, finalRight.y);
						table.direction = currentDirection;
						waterTableList.add (table);
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------------

void AffectorRiver::setLocalWaterTableShaderSize (const float newLocalWaterTableShaderSize)
{
	m_localWaterTableShaderSize = newLocalWaterTableShaderSize;
}

//-------------------------------------------------------------------

void AffectorRiver::setLocalWaterTableShaderTemplateName (const char* newLocalWaterTableShaderTemplateName)
{
	if (m_localWaterTableShaderTemplateName)
	{
		delete [] m_localWaterTableShaderTemplateName;
		m_localWaterTableShaderTemplateName = 0;
	}

	if (newLocalWaterTableShaderTemplateName)
		m_localWaterTableShaderTemplateName = DuplicateString (newLocalWaterTableShaderTemplateName);
}

//-------------------------------------------------------------------

void AffectorRiver::setHasLocalWaterTable (const bool newHasLocalWaterTable)
{
	m_hasLocalWaterTable = newHasLocalWaterTable;
}

//-------------------------------------------------------------------

void AffectorRiver::setLocalWaterTableDepth (const float newLocalWaterTableDepth)
{
	m_localWaterTableDepth = newLocalWaterTableDepth;
}

//-------------------------------------------------------------------

void AffectorRiver::setLocalWaterTableWidth (const float newLocalWaterTableWidth)
{
	m_localWaterTableWidth = newLocalWaterTableWidth;
}

//===================================================================

