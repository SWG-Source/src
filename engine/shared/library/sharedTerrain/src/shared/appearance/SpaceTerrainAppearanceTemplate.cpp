// ======================================================================
//
// SpaceTerrainAppearanceTemplate.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/SpaceTerrainAppearanceTemplate.h"

#include "sharedFile/Iff.h"

#include <string>
#include <vector>

// ======================================================================

namespace SpaceTerrainAppearanceTemplateNamespace
{
	Tag const TAG_AMBI = TAG(A,M,B,I);
	Tag const TAG_CELE = TAG(C,E,L,E);
	Tag const TAG_CLEA = TAG(C,L,E,A);
	Tag const TAG_DIST = TAG(D,I,S,T);
	Tag const TAG_DUST = TAG(D,U,S,T);
	Tag const TAG_ENVI = TAG(E,N,V,I);
	Tag const TAG_FOG = TAG3(F,O,G);
	Tag const TAG_PARA = TAG(P,A,R,A);
	Tag const TAG_PLAN = TAG(P,L,A,N);
	Tag const TAG_STAR = TAG(S,T,A,R);
	Tag const TAG_SKYB = TAG(S,K,Y,B);
}

using namespace SpaceTerrainAppearanceTemplateNamespace;

// ======================================================================
// PUBLIC SpaceTerrainAppearanceTemplate
// ======================================================================

SpaceTerrainAppearanceTemplate::SpaceTerrainAppearanceTemplate(char const * const filename, Iff * const iff) :
	AppearanceTemplate(filename),
	m_clearColor(),
	m_ambientColor(),
	m_lightDataList(new LightDataList),
	m_environmentTextureName(),
	m_fogEnabled(false),
	m_fogColor(),
	m_fogDensity(0.f),
	m_numberOfStars(0),
	m_starColorRampName(),
	m_numberOfDust(0),
	m_dustRadius(0.f),
	m_skyBoxCubeMap(false),
	m_skyBoxTextureNameMask(),
	m_celestialDataList(new CelestialDataList),
	m_distantAppearanceDataList(new DistantAppearanceDataList),
	m_mapWidthInMeters(16384.0f)
{
	NOT_NULL(iff);
	load(*iff);
}

// ----------------------------------------------------------------------

SpaceTerrainAppearanceTemplate::~SpaceTerrainAppearanceTemplate()
{
	delete m_lightDataList;
	delete m_celestialDataList;
	delete m_distantAppearanceDataList;
}

// ----------------------------------------------------------------------

PackedRgb const & SpaceTerrainAppearanceTemplate::getClearColor() const
{
	return m_clearColor;
}

// ----------------------------------------------------------------------

VectorArgb const & SpaceTerrainAppearanceTemplate::getAmbientColor() const
{
	return m_ambientColor;
}

// ----------------------------------------------------------------------

int SpaceTerrainAppearanceTemplate::getNumberOfParallelLights() const
{
	NOT_NULL(m_lightDataList);
	return static_cast<int>(m_lightDataList->size());
}

// ----------------------------------------------------------------------

VectorArgb const & SpaceTerrainAppearanceTemplate::getParallelLightDiffuseColor(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfParallelLights());
	return (*m_lightDataList)[static_cast<size_t>(index)].m_diffuseColor;
}

// ----------------------------------------------------------------------

VectorArgb const & SpaceTerrainAppearanceTemplate::getParallelLightSpecularColor(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfParallelLights());
	return (*m_lightDataList)[static_cast<size_t>(index)].m_specularColor;
}

// ----------------------------------------------------------------------

Vector const & SpaceTerrainAppearanceTemplate::getParallelLightDirection_w(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfParallelLights());
	return (*m_lightDataList)[static_cast<size_t>(index)].m_direction_w;
}

// ----------------------------------------------------------------------

CrcString const & SpaceTerrainAppearanceTemplate::getEnvironmentTextureName() const
{
	return m_environmentTextureName;
}

// ----------------------------------------------------------------------

bool SpaceTerrainAppearanceTemplate::getFogEnabled() const
{
	return m_fogEnabled;
}

// ----------------------------------------------------------------------

PackedArgb const & SpaceTerrainAppearanceTemplate::getFogColor() const
{
	return m_fogColor;
}

// ----------------------------------------------------------------------

float SpaceTerrainAppearanceTemplate::getFogDensity() const
{
	return m_fogDensity;
}

// ----------------------------------------------------------------------

int SpaceTerrainAppearanceTemplate::getNumberOfStars() const
{
	return m_numberOfStars;
}

// ----------------------------------------------------------------------

CrcString const & SpaceTerrainAppearanceTemplate::getStarColorRampName() const
{
	return m_starColorRampName;
}

// ----------------------------------------------------------------------

int SpaceTerrainAppearanceTemplate::getNumberOfDust() const
{
	return m_numberOfDust;
}

// ----------------------------------------------------------------------

float SpaceTerrainAppearanceTemplate::getDustRadius() const
{
	return m_dustRadius;
}

// ----------------------------------------------------------------------

bool SpaceTerrainAppearanceTemplate::getSkyBoxCubeMap() const
{
	return m_skyBoxCubeMap;
}

// ----------------------------------------------------------------------

CrcString const & SpaceTerrainAppearanceTemplate::getSkyBoxTextureNameMask() const
{
	return m_skyBoxTextureNameMask;
}

// ----------------------------------------------------------------------

int SpaceTerrainAppearanceTemplate::getNumberOfDistantAppearances() const
{
	NOT_NULL(m_distantAppearanceDataList);
	return static_cast<int>(m_distantAppearanceDataList->size());
}

// ----------------------------------------------------------------------

CrcString const & SpaceTerrainAppearanceTemplate::getDistantAppearanceTemplateName(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfDistantAppearances());
	return (*m_distantAppearanceDataList)[static_cast<size_t>(index)].m_appearanceTemplateName;
}

// ----------------------------------------------------------------------

Vector const & SpaceTerrainAppearanceTemplate::getDistantAppearanceDirection_w(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfDistantAppearances());
	return (*m_distantAppearanceDataList)[static_cast<size_t>(index)].m_direction_w;
}

// ----------------------------------------------------------------------

Vector const & SpaceTerrainAppearanceTemplate::getDistantAppearanceOrientation_w(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfDistantAppearances());
	return (*m_distantAppearanceDataList)[static_cast<size_t>(index)].m_orientation_w;
}

// ----------------------------------------------------------------------

float SpaceTerrainAppearanceTemplate::getPlanetHaloRoll(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfDistantAppearances());
	return (*m_distantAppearanceDataList)[static_cast<size_t>(index)].m_haloRoll;
}

// ----------------------------------------------------------------------

float SpaceTerrainAppearanceTemplate::getPlanetHaloScale(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfDistantAppearances());
	return (*m_distantAppearanceDataList)[static_cast<size_t>(index)].m_haloScale;
}

//----------------------------------------------------------------------

bool SpaceTerrainAppearanceTemplate::isDistantAppearanceInfiniteDistance(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfDistantAppearances());
	return (*m_distantAppearanceDataList)[static_cast<size_t>(index)].m_infiniteDistance;
}

// ----------------------------------------------------------------------

int SpaceTerrainAppearanceTemplate::getNumberOfCelestials() const
{
	NOT_NULL(m_celestialDataList);
	return static_cast<int>(m_celestialDataList->size());
}

// ----------------------------------------------------------------------

CrcString const & SpaceTerrainAppearanceTemplate::getCelestialBackShaderTemplateName(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfCelestials());
	return (*m_celestialDataList)[static_cast<size_t>(index)].m_backShaderTemplateName;
}

// ----------------------------------------------------------------------

float SpaceTerrainAppearanceTemplate::getCelestialBackSize(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfCelestials());
	return (*m_celestialDataList)[static_cast<size_t>(index)].m_backSize;
}

// ----------------------------------------------------------------------

CrcString const & SpaceTerrainAppearanceTemplate::getCelestialFrontShaderTemplateName(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfCelestials());
	return (*m_celestialDataList)[static_cast<size_t>(index)].m_frontShaderTemplateName;
}

// ----------------------------------------------------------------------

float SpaceTerrainAppearanceTemplate::getCelestialFrontSize(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfCelestials());
	return (*m_celestialDataList)[static_cast<size_t>(index)].m_frontSize;
}

// ----------------------------------------------------------------------

Vector const & SpaceTerrainAppearanceTemplate::getCelestialDirection_w(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfCelestials());
	return (*m_celestialDataList)[static_cast<size_t>(index)].m_direction_w;
}

// ======================================================================
// PROTECTED SpaceTerrainAppearanceTemplate
// ======================================================================

Tag const SpaceTerrainAppearanceTemplate::cms_spaceTerrainAppearanceTemplateTag = TAG(S,T,A,T);

// ======================================================================
// PRIVATE SpaceTerrainAppearanceTemplate
// ======================================================================

void SpaceTerrainAppearanceTemplate::load(Iff & iff)
{
	iff.enterForm(cms_spaceTerrainAppearanceTemplateTag);

		switch(iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

		default:
			{
				char tagBuffer[5];
				ConvertTagToString(iff.getCurrentName(), tagBuffer);

				char buffer[128];
				iff.formatLocation(buffer, sizeof(buffer));
				
				FATAL(true, ("SpaceTerrainAppearanceTemplate::load: unsupported version tag %s/%s", buffer, tagBuffer));
			}
			break;
		}

	iff.exitForm(cms_spaceTerrainAppearanceTemplateTag);
}

// ----------------------------------------------------------------------

void SpaceTerrainAppearanceTemplate::load_0000(Iff & iff)
{
	iff.enterForm(TAG_0000);

		while (iff.getNumberOfBlocksLeft())
		{
			switch (iff.getCurrentName())
			{
			case TAG_INFO:
				{
					iff.enterForm(TAG_INFO);
					{
						iff.enterChunk(TAG_0000);
						{
							m_mapWidthInMeters = iff.read_float();
						}
						iff.exitChunk(TAG_0000);
					}
					iff.exitForm(TAG_INFO);
				}
				break;

			case TAG_CLEA:
				{
					iff.enterForm(TAG_CLEA);
						iff.enterChunk(TAG_0000);

							m_clearColor.r = static_cast<uint8>(clamp(0.f, iff.read_float(), 1.f) * 255.f);
							m_clearColor.g = static_cast<uint8>(clamp(0.f, iff.read_float(), 1.f) * 255.f);
							m_clearColor.b = static_cast<uint8>(clamp(0.f, iff.read_float(), 1.f) * 255.f);

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_CLEA);
				}
				break;

			case TAG_AMBI:
				{
					iff.enterForm(TAG_AMBI);
						iff.enterChunk(TAG_0000);

							m_ambientColor = iff.read_floatVectorArgb();

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_AMBI);
				}
				break;

			case TAG_PARA:
				{
					iff.enterForm(TAG_PARA);
						iff.enterChunk(TAG_0000);

							LightData lightData;
							lightData.m_dot3 = iff.read_bool8();
							lightData.m_diffuseColor = iff.read_floatVectorArgb();
							lightData.m_specularColor = iff.read_floatVectorArgb();
							lightData.m_direction_w = iff.read_floatVector();
							m_lightDataList->push_back(lightData);

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_PARA);
				}
				break;

			case TAG_ENVI:
				{
					iff.enterForm(TAG_ENVI);
						iff.enterChunk(TAG_0000);
						
							std::string environmentTextureName;
							iff.read_string(environmentTextureName);
							m_environmentTextureName.set(environmentTextureName.c_str(), true);

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_ENVI);
				}
				break;

			case TAG_FOG:
				{
					iff.enterForm(TAG_FOG);
						iff.enterChunk(TAG_0000);

							m_fogEnabled = true;
							VectorArgb const fogColor = iff.read_floatVectorArgb();
							m_fogColor.setArgb(fogColor);
							m_fogDensity = fogColor.a;

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_FOG);
				}
				break;

			case TAG_STAR:
				{
					iff.enterForm(TAG_STAR);
						iff.enterChunk(TAG_0000);

							std::string starColorRampName;
							iff.read_string(starColorRampName);
							m_starColorRampName.set(starColorRampName.c_str(), true);

							m_numberOfStars = iff.read_int32();

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_STAR);
				}
				break;

			case TAG_DUST:
				{
					iff.enterForm(TAG_DUST);
						iff.enterChunk(TAG_0000);

							m_numberOfDust = iff.read_int32();
							m_dustRadius = iff.read_float();

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_DUST);
				}
				break;

			case TAG_SKYB:
				{
					iff.enterForm(TAG_SKYB);
						iff.enterChunk(TAG_0000);

							m_skyBoxCubeMap = iff.read_bool8();

							std::string textureNameMask;
							iff.read_string(textureNameMask);
							m_skyBoxTextureNameMask.set(textureNameMask.c_str(), true);

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_SKYB);
				}
				break;

			case TAG_DIST:
				{
					iff.enterForm(TAG_DIST);
						iff.enterChunk(TAG_0000);

							DistantAppearanceData distantAppearanceData;
							std::string appearanceTemplateName;
							iff.read_string(appearanceTemplateName);
							distantAppearanceData.m_appearanceTemplateName.set(appearanceTemplateName.c_str(), true);
							distantAppearanceData.m_direction_w = iff.read_floatVector();
							distantAppearanceData.m_orientation_w.x = convertDegreesToRadians(iff.read_float());
							distantAppearanceData.m_orientation_w.y = convertDegreesToRadians(iff.read_float());
							distantAppearanceData.m_orientation_w.z = convertDegreesToRadians(iff.read_float());
							distantAppearanceData.m_haloRoll = 0.f;
							distantAppearanceData.m_haloScale = 1.f;
							distantAppearanceData.m_infiniteDistance = (iff.read_int8() != 0);
							m_distantAppearanceDataList->push_back(distantAppearanceData);

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_DIST);
				}
				break;

			case TAG_PLAN:
				{
					iff.enterForm(TAG_PLAN);
						iff.enterChunk(TAG_0000);

							DistantAppearanceData distantAppearanceData;
							std::string appearanceTemplateName;
							iff.read_string(appearanceTemplateName);
							distantAppearanceData.m_appearanceTemplateName.set(appearanceTemplateName.c_str(), true);
							distantAppearanceData.m_direction_w = iff.read_floatVector();
							distantAppearanceData.m_orientation_w.x = convertDegreesToRadians(iff.read_float());
							distantAppearanceData.m_orientation_w.y = convertDegreesToRadians(iff.read_float());
							distantAppearanceData.m_orientation_w.z = convertDegreesToRadians(iff.read_float());
							distantAppearanceData.m_haloRoll = convertDegreesToRadians(iff.read_float());
							distantAppearanceData.m_haloScale = iff.read_float();
							distantAppearanceData.m_infiniteDistance = (iff.read_int8() != 0);
							m_distantAppearanceDataList->push_back(distantAppearanceData);

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_PLAN);
				}
				break;

			case TAG_CELE:
				{
					iff.enterForm(TAG_CELE);
						iff.enterChunk(TAG_0000);

							CelestialData celestialData;
							std::string shaderTemplateName;
							iff.read_string(shaderTemplateName);
							celestialData.m_backShaderTemplateName.set(shaderTemplateName.c_str(), true);
							celestialData.m_backSize = iff.read_float();
							iff.read_string(shaderTemplateName);
							celestialData.m_frontShaderTemplateName.set(shaderTemplateName.c_str(), true);
							celestialData.m_frontSize = iff.read_float();
							celestialData.m_direction_w = iff.read_floatVector();
							m_celestialDataList->push_back(celestialData);

						iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_CELE);
				}
				break;

			default:
				{
#ifdef _DEBUG
					char tagBuffer[5];
					ConvertTagToString(iff.getCurrentName(), tagBuffer);

					char buffer[128];
					iff.formatLocation(buffer, sizeof(buffer));
					
					DEBUG_WARNING(true, ("SpaceTerrainAppearanceTemplate::load: skipping unknown chunk type %s/%s", buffer, tagBuffer));
#endif

					IGNORE_RETURN(iff.goForward());
				}
				break;
			}
		}

	iff.exitForm(TAG_0000);
}

//----------------------------------------------------------------------

float SpaceTerrainAppearanceTemplate::getMapWidthInMeters() const
{
	return m_mapWidthInMeters;
}

// ======================================================================
