// ======================================================================
//
// SpaceTerrainAppearanceTemplate.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SpaceTerrainAppearanceTemplate_H
#define INCLUDED_SpaceTerrainAppearanceTemplate_H

// ======================================================================

#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/Tag.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"

class Iff;

// ======================================================================

class SpaceTerrainAppearanceTemplate : public AppearanceTemplate
{
public:

	SpaceTerrainAppearanceTemplate(char const * filename, Iff * iff);
	virtual ~SpaceTerrainAppearanceTemplate() = 0;

	PackedRgb const & getClearColor() const;

	VectorArgb const & getAmbientColor() const;

	int getNumberOfParallelLights() const;
	VectorArgb const & getParallelLightDiffuseColor(int index) const;
	VectorArgb const & getParallelLightSpecularColor(int index) const;
	Vector const & getParallelLightDirection_w(int index) const;

	CrcString const & getEnvironmentTextureName() const;

	bool getFogEnabled() const;
	PackedArgb const & getFogColor() const;
	float getFogDensity() const;

	int getNumberOfStars() const;
	CrcString const & getStarColorRampName() const;

	int getNumberOfDust() const;
	float getDustRadius() const;

	bool getSkyBoxCubeMap() const;
	CrcString const & getSkyBoxTextureNameMask() const;

	int getNumberOfDistantAppearances() const;
	CrcString const & getDistantAppearanceTemplateName(int index) const;
	Vector const & getDistantAppearanceDirection_w(int index) const;
	Vector const & getDistantAppearanceOrientation_w(int index) const;
	float getPlanetHaloRoll(int index) const;
	float getPlanetHaloScale(int index) const;
	bool isDistantAppearanceInfiniteDistance(int index) const;

	int getNumberOfCelestials() const;
	CrcString const & getCelestialBackShaderTemplateName(int index) const;
	float getCelestialBackSize(int index) const;
	CrcString const & getCelestialFrontShaderTemplateName(int index) const;
	float getCelestialFrontSize(int index) const;
	Vector const & getCelestialDirection_w(int index) const;

	float getMapWidthInMeters() const;

protected:

	static Tag const cms_spaceTerrainAppearanceTemplateTag;

private:

	struct LightData
	{
		bool m_dot3;
		VectorArgb m_diffuseColor;
		VectorArgb m_specularColor;
		Vector m_direction_w;
	};

	struct DistantAppearanceData
	{
		PersistentCrcString m_appearanceTemplateName;
		Vector m_direction_w;
		Vector m_orientation_w;
		float m_haloRoll;
		float m_haloScale;
		bool m_infiniteDistance;

	//private:

	//	DistantAppearanceData operator=(DistantAppearanceData const & rhs);
	};

	struct CelestialData
	{
		PersistentCrcString m_backShaderTemplateName;
		float m_backSize;
		PersistentCrcString m_frontShaderTemplateName;
		float m_frontSize;
		Vector m_direction_w;

	//private:

	//	CelestialData operator=(CelestialData const & rhs);
	};

private:

	SpaceTerrainAppearanceTemplate();
	SpaceTerrainAppearanceTemplate(SpaceTerrainAppearanceTemplate const &);
	SpaceTerrainAppearanceTemplate & operator=(SpaceTerrainAppearanceTemplate const &);

	void load(Iff & iff);
	void load_0000(Iff & iff);

private:

	PackedRgb m_clearColor;
	
	VectorArgb m_ambientColor;

	typedef std::vector<LightData> LightDataList;
	LightDataList * const m_lightDataList;

	PersistentCrcString m_environmentTextureName;

	bool m_fogEnabled;
	PackedArgb m_fogColor;
	float m_fogDensity;

	int m_numberOfStars;
	PersistentCrcString m_starColorRampName;

	int m_numberOfDust;
	float m_dustRadius;

	bool m_skyBoxCubeMap;
	PersistentCrcString m_skyBoxTextureNameMask;

	typedef std::vector<CelestialData> CelestialDataList;
	CelestialDataList * const m_celestialDataList;

	typedef std::vector<DistantAppearanceData> DistantAppearanceDataList;
	DistantAppearanceDataList * const m_distantAppearanceDataList;

	float m_mapWidthInMeters;
}; 

// ======================================================================

#endif
