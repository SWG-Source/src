//======================================================================
//
// Nebula.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/Nebula.h"

#include "sharedMath/Vector.h"

//======================================================================

int const Nebula::ms_numStyleWeightings = 16;

//----------------------------------------------------------------------

Nebula::Nebula(int id, std::string const & name) :
m_id(id),
m_name(name),
m_sphere(),
m_density(1.0f),
m_colorFacing(),
m_colorOriented(),
m_colorRampFacing(),
m_colorRampOriented(),
m_facingPercent(0.0f),
m_ambientSound(),
m_ambientSoundVolume(1.0f),
m_effectReactor(0.0f),
m_effectEngine(0.0f),
m_effectShields(0.0f),
m_lightningFrequency(0.0f),
m_lightningDamageMin(0.0f),
m_lightningDamageMax(0.0f),
m_lightningDurationMax(0.0f),
m_lightningAppearance(),
m_lightningColor(),
m_lightningColorRamp(),
m_lightningSound(),
m_lightningSoundLoop(),
m_lightningClientEffectHitClient(),
m_lightningClientEffectHitServer(),
m_environmentalDamageFrequency(0.0f),
m_environmentalDamage(0.0f),
m_environmentalDamageClientEffect(),
m_cameraJitterAngle(0.0f),
m_shaderIndex(0),
m_spatialSubdivisionHandle(0)
{
}

//----------------------------------------------------------------------

Nebula::~Nebula()
{
}

//----------------------------------------------------------------------

void Nebula::setColor(VectorArgb const & colorFacing, VectorArgb const & colorRampFacing, VectorArgb const & colorOriented, VectorArgb const & colorRampOriented)
{
	m_colorFacing = colorFacing;
	m_colorOriented = colorOriented;
	m_colorRampFacing = colorRampFacing;
	m_colorRampOriented = colorRampOriented;
}

//----------------------------------------------------------------------

void Nebula::setExtent(Vector const & pos_w, float const radius)
{
	m_sphere.set(pos_w, radius);
}

//----------------------------------------------------------------------

void Nebula::setShipEffects(float const effectReactor, float const effectEngine, float const effectShields)
{
	m_effectReactor = effectReactor;
	m_effectEngine = effectEngine;
	m_effectShields = effectShields;
}

//----------------------------------------------------------------------

void Nebula::setLightningEffects(float const lightningFrequency, float const lightningDurationMax, std::string const & lightningAppearance, VectorArgb const & lightningColor, VectorArgb const & lightningColorRamp, std::string const & lightningSound, std::string const & lightningSoundLoop, std::string const & lightningClientEffectHitClient, std::string const & lightningClientEffectHitServer)
{
	m_lightningFrequency = lightningFrequency;
	m_lightningDurationMax = lightningDurationMax;
	m_lightningAppearance = lightningAppearance;

	m_lightningColor = lightningColor;
	m_lightningColorRamp = lightningColorRamp;

	m_lightningSound = lightningSound;
	m_lightningSoundLoop = lightningSoundLoop;
	m_lightningClientEffectHitClient = lightningClientEffectHitClient;
	m_lightningClientEffectHitServer = lightningClientEffectHitServer;
}

//----------------------------------------------------------------------

void Nebula::setEnvironmentalDamage(float const environmentalDamageFrequency, float const environmentalDamage, std::string const & environmentalDamageClientEffect)
{
	m_environmentalDamageFrequency = environmentalDamageFrequency;
	m_environmentalDamage = environmentalDamage;
	m_environmentalDamageClientEffect = environmentalDamageClientEffect;
}

//----------------------------------------------------------------------

void Nebula::setDensity(float const density)
{
	m_density = density;
}

//----------------------------------------------------------------------

void Nebula::setAmbientSound(std::string const & ambientSound, float ambientSoundVolume)
{
	m_ambientSound = ambientSound;
	m_ambientSoundVolume = ambientSoundVolume;
}

//----------------------------------------------------------------------

void Nebula::setCameraJitterAngle(float const cameraJitterAngle)
{
	m_cameraJitterAngle = cameraJitterAngle;
}

//----------------------------------------------------------------------

void Nebula::setLightningDamage(float const lightningDamageMin, float const lightningDamageMax)
{
	m_lightningDamageMin = lightningDamageMin;
	m_lightningDamageMax = lightningDamageMax;
}

//----------------------------------------------------------------------

int Nebula::getShaderIndex() const
{
	return m_shaderIndex;
}

//----------------------------------------------------------------------

void Nebula::setShaderInfo(int shaderIndex, float const * styleWeightings, float facingPercent)
{
	m_shaderIndex = shaderIndex;
	for (int i = 0; i < ms_numStyleWeightings; ++i)
	{
		m_styleWeightings[i] = styleWeightings[i];
	}

	m_facingPercent = facingPercent;

}

//----------------------------------------------------------------------

std::string const & Nebula::getName() const
{
	return m_name;
}

//----------------------------------------------------------------------

SpatialSubdivisionHandle * Nebula::getSpatialSubdivisionHandle() const
{
	return m_spatialSubdivisionHandle;
} 

//----------------------------------------------------------------------

void Nebula::setSpatialSubdivisionHandle (SpatialSubdivisionHandle * const spatialSubdivisionHandle) const
{
	m_spatialSubdivisionHandle = spatialSubdivisionHandle;
}

//======================================================================
