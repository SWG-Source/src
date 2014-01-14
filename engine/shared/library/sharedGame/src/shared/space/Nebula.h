//======================================================================
//
// Nebula.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_Nebula_H
#define INCLUDED_Nebula_H

//======================================================================

#include "sharedMath/Sphere.h"
#include "sharedMath/VectorArgb.h"

class SpatialSubdivisionHandle;
class Vector;

//----------------------------------------------------------------------

class Nebula
{
public:

	static int const ms_numStyleWeightings;

public:

	Nebula(int id, std::string const & name);
	~Nebula();

	void setColor(VectorArgb const & colorFacing, VectorArgb const & colorRampFacing, VectorArgb const & colorOriented, VectorArgb const & colorRampOriented);
	void setExtent(Vector const & pos_w, float radius);
	void setDensity(float density);
	void setShipEffects(float effectReactor, float effectEngine, float effectShields);
	void setLightningEffects(float lightningFrequency, float lightningDurationMax, std::string const & lightningAppearance, VectorArgb const & colorLightning, VectorArgb const & colorRampLightning, std::string const & lightningSound, std::string const & lightningSoundLoop, std::string const & lightningClientEffectHitClient, std::string const & lightningClientEffectHitServer);
	void setLightningDamage(float lightningDamageMin, float lightningDamageMax);
	void setEnvironmentalDamage(float environmentalDamageFrequency, float environmentalDamage, std::string const & environmentalDamageClientEffect);
	void setAmbientSound(std::string const & ambientSound, float ambientSoundVolume);
	void setCameraJitterAngle(float cameraJitterAngle);
	void setShaderInfo(int shaderIndex, float const * styleWeightings, float facingPercent);

	Sphere const & getSphere() const;
	float getDensity() const;
	VectorArgb const & getColor() const;
	VectorArgb const & getColorFacing() const;
	VectorArgb const & getColorOriented() const;
	VectorArgb const & getColorRampFacing() const;
	VectorArgb const & getColorRampOriented() const;
	float getFacingPercent() const;
	int getId() const;
	std::string const & getAmbientSound() const;
	float getAmbientSoundVolume() const;
	std::string const & getLightningAppearance() const;
	VectorArgb const & getLightningColor() const;
	VectorArgb const & getLightningColorRamp() const;
	std::string const & getLightningSound() const;
	std::string const & getLightningSoundLoop() const;
	std::string const & getLightningClientEffectHitClient() const;
	std::string const & getLightningClientEffectHitServer() const;
	float getLightningFrequency() const;
	float getLightningDamageMin() const;
	float getLightningDamageMax() const;
	float getLightningDurationMax() const;
	float getEffectEngine() const;
	float getEffectReactor() const;
	float getEffectShields() const;
	float getCameraJitterAngle() const;
	float getEnvironmentalDamageFrequency() const;
	float getEnvironmentalDamage() const;
	std::string const & getEnvironmentalDamageClientEffect() const;
	int getShaderIndex() const;
	std::string const & getName() const;

	SpatialSubdivisionHandle * getSpatialSubdivisionHandle() const;
	void setSpatialSubdivisionHandle(SpatialSubdivisionHandle * spatialSubdivisionHandle) const;

private:

	Nebula & operator=(Nebula const & rhs);
	Nebula(Nebula const & rhs);

private:

	int m_id;
	std::string m_name;
	Sphere m_sphere;
	float m_density;
	VectorArgb m_colorFacing;
	VectorArgb m_colorOriented;
	VectorArgb m_colorRampFacing;
	VectorArgb m_colorRampOriented;
	float m_facingPercent;
	std::string m_ambientSound;
	float m_ambientSoundVolume;

	float m_effectReactor;
	float m_effectEngine;
	float m_effectShields;

	float m_lightningFrequency;
	float m_lightningDamageMin;
	float m_lightningDamageMax;

	float m_lightningDurationMax;

	std::string m_lightningAppearance;

	VectorArgb m_lightningColor;
	VectorArgb m_lightningColorRamp;

	std::string m_lightningSound;
	std::string m_lightningSoundLoop;
	std::string m_lightningClientEffectHitClient;
	std::string m_lightningClientEffectHitServer;

	float m_environmentalDamageFrequency;
	float m_environmentalDamage;

	std::string m_environmentalDamageClientEffect;

	float m_cameraJitterAngle;
	int m_shaderIndex;

	float m_styleWeightings[16];

	mutable SpatialSubdivisionHandle * m_spatialSubdivisionHandle;

	// light
	// fog params
	// flashes
	// shifting clouds
	// etc...
};

//----------------------------------------------------------------------

inline int Nebula::getId() const
{
	return m_id;
}

//----------------------------------------------------------------------

inline Sphere const & Nebula::getSphere() const
{
	return m_sphere;
}

//----------------------------------------------------------------------

inline VectorArgb const & Nebula::getColor() const
{
	return m_colorFacing;
}

//----------------------------------------------------------------------

inline std::string const & Nebula::getAmbientSound() const
{
	return m_ambientSound;
}

//----------------------------------------------------------------------

inline float Nebula::getAmbientSoundVolume() const
{
	return m_ambientSoundVolume;
}

//----------------------------------------------------------------------

inline std::string const & Nebula::getLightningAppearance() const
{
	return m_lightningAppearance;
}

//----------------------------------------------------------------------

inline float Nebula::getLightningFrequency() const
{
	return m_lightningFrequency;
}

//----------------------------------------------------------------------

inline std::string const & Nebula::getLightningSound() const
{
	return m_lightningSound;
}

//----------------------------------------------------------------------

inline std::string const & Nebula::getLightningSoundLoop() const
{
	return m_lightningSoundLoop;
}

//----------------------------------------------------------------------

inline std::string const & Nebula::getLightningClientEffectHitClient() const
{
	return m_lightningClientEffectHitClient;
}

//----------------------------------------------------------------------

inline std::string const & Nebula::getLightningClientEffectHitServer() const
{
	return m_lightningClientEffectHitServer;
}

//----------------------------------------------------------------------

inline float Nebula::getEffectEngine() const
{
	return m_effectEngine;
}

//----------------------------------------------------------------------

inline float Nebula::getEffectReactor() const
{
	return m_effectReactor;
}

//----------------------------------------------------------------------

inline float Nebula::getEffectShields() const
{
	return m_effectShields;
}

//----------------------------------------------------------------------

inline float Nebula::getCameraJitterAngle() const
{
	return m_cameraJitterAngle;
}

//----------------------------------------------------------------------

inline float Nebula::getLightningDamageMin() const
{
	return m_lightningDamageMin;
}

//----------------------------------------------------------------------

inline float Nebula::getLightningDamageMax() const
{
	return m_lightningDamageMax;
}

//----------------------------------------------------------------------

inline float Nebula::getLightningDurationMax() const
{
	return m_lightningDurationMax;
}

//----------------------------------------------------------------------

inline float Nebula::getEnvironmentalDamageFrequency() const
{
	return m_environmentalDamageFrequency;
}

//----------------------------------------------------------------------

inline float Nebula::getEnvironmentalDamage() const
{
	return m_environmentalDamage;
}

//----------------------------------------------------------------------

inline std::string const & Nebula::getEnvironmentalDamageClientEffect() const
{
	return m_environmentalDamageClientEffect;
}

//----------------------------------------------------------------------

inline float Nebula::getDensity() const
{
	return m_density;
}

//----------------------------------------------------------------------

inline VectorArgb const & Nebula::getColorFacing() const
{
	return m_colorFacing;
}

//----------------------------------------------------------------------

inline VectorArgb const & Nebula::getColorOriented() const
{
	return m_colorOriented;
}

//----------------------------------------------------------------------

inline VectorArgb const & Nebula::getColorRampFacing() const
{
	return m_colorRampFacing;
}

//----------------------------------------------------------------------

inline VectorArgb const & Nebula::getColorRampOriented() const
{
	return m_colorRampOriented;
}

//----------------------------------------------------------------------

inline float Nebula::getFacingPercent() const
{
	return m_facingPercent;
}

//----------------------------------------------------------------------

inline VectorArgb const & Nebula::getLightningColor() const
{
	return m_lightningColor;
}

//----------------------------------------------------------------------

inline VectorArgb const & Nebula::getLightningColorRamp() const
{
	return m_lightningColorRamp;
}

//======================================================================

#endif
