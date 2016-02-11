//========================================================================
//
// ResourcePoolObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ResourcePoolObject.h"

#include "UnicodeUtils.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ServerUniverse.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFractal/MultiFractal.h"

#include <stdio.h>

// ----------------------------------------------------------------------

ResourcePoolObject::ResourcePoolObject(ResourceTypeObject const & resourceType, PlanetObject const & planetObject, uint32 fractalSeed) :
		m_resourceType(resourceType),
		m_planet(planetObject),
		m_fractalData(resourceType.getFractalData()),
		m_fractalSeed(fractalSeed),
		m_depletedTimestamp(resourceType.getDepletedTimestamp()),
		m_efficiencyMap(nullptr)
{
}

//-----------------------------------------------------------------------

ResourcePoolObject::~ResourcePoolObject()
{
	delete m_efficiencyMap;
	m_efficiencyMap=nullptr;
}

// ----------------------------------------------------------------------

/**
 * Called to harvest resources from the pool.
 * @param lastHarvestTick Tick count the last time this harvester
 * harvested from the pool.
 * @param installedEfficiency The value returned by placeHarvester() when
 * the harvester was placed.
 * @param harvesterId The objectID of the harvester collecting the resource.
 */
float ResourcePoolObject::harvest(float installedEfficiency, uint32 lastHarvestTime) const
{
	int elapsedTime=static_cast<int>(std::min(ServerClock::getInstance().getGameTimeSeconds(),m_depletedTimestamp) - lastHarvestTime);
	if (elapsedTime < 0)
		return 0;
	float elapsedTicks =static_cast<float>(elapsedTime) / static_cast<float>(ConfigServerGame::getSecondsPerResourceTick());
	float amountHarvested = elapsedTicks * installedEfficiency;

	return amountHarvested;
}

// ----------------------------------------------------------------------

/**
 * Get the value of the efficiency map at a specific location.
 */

float ResourcePoolObject::getEfficiencyAtLocation(float x, float z) const
{
	if (!m_efficiencyMap)
	{
		m_efficiencyMap = new MultiFractal;
		
		m_efficiencyMap->setSeed(m_fractalSeed);
		m_efficiencyMap->setScale(m_fractalData.m_scaleX, m_fractalData.m_scaleY);
		m_efficiencyMap->setBias(true,m_fractalData.m_bias);
		m_efficiencyMap->setGain(true,m_fractalData.m_gain);
		m_efficiencyMap->setCombinationRule(static_cast<MultiFractal::CombinationRule>(m_fractalData.m_combinationRule));
		m_efficiencyMap->setFrequency(m_fractalData.m_frequency);
		m_efficiencyMap->setAmplitude(m_fractalData.m_amplitude);
		m_efficiencyMap->setNumberOfOctaves(m_fractalData.m_octaves);
	}
	return m_efficiencyMap->getValue(x,z);
}

// ----------------------------------------------------------------------

void ResourcePoolObject::debugOutput(Unicode::String &output) const
{
	char buffer[256];

	sprintf(buffer,"Pool of %s:  ",m_resourceType.getNetworkId().getValueString().c_str());
	output+=Unicode::narrowToWide(buffer)+Unicode::narrowToWide(m_resourceType.getResourceName());

	if (isDepleted())
		output+=Unicode::narrowToWide("  DEPLETED");
	else
	{
		sprintf(buffer,"  %lu time units left\n",(m_depletedTimestamp - ServerClock::getInstance().getGameTimeSeconds())/ConfigServerGame::getResourceTimeScale());
		output+=Unicode::narrowToWide(buffer);
	}
}

// ----------------------------------------------------------------------

/**
 * Extract resource from the pool once only.  To avoid requiring a callback,
 * this function will allow you to harvest from a proxy object, possibly
 * giving you a little more than you could normally get if the pool is
 * almost depleted.
 * @param amount Maximum amount to harvest.
 * @location Point to use for the effeciency calculation.
 * @return Amount of resource you receive.
 */
int ResourcePoolObject::oneTimeHarvest(int amount, const Vector & location) const
{
	if (isDepleted())
		return 0;
	
	int result = static_cast<int>(static_cast<float>(amount) * getEfficiencyAtLocation(location.x, location.z));
	return result;
}

// ----------------------------------------------------------------------

bool ResourcePoolObject::isDepleted() const
{
	return m_resourceType.isDepleted();
}

// ======================================================================
