// ======================================================================
//
// SynchronizedWeatherGenerator.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/SynchronizedWeatherGenerator.h"

#include "sharedMath/Transform.h"
#include "sharedRandom/Random.h"

// ======================================================================

/** 
 * Generate random weather.
 */

void SynchronizedWeatherGenerator::generateWeather(unsigned long serverTime) const
{
	unsigned long roundedTime = serverTime - (serverTime % m_updateInterval);
	m_nextUpdate = roundedTime + m_updateInterval;
	DEBUG_REPORT_LOG(true,("Generating weather as of %lu\n",roundedTime));

	NOT_NULL(m_randomGenerator);
	m_randomGenerator->setSeed(roundedTime);
		
	Transform t;
	t.yaw_l(m_randomGenerator->randomReal(PI_TIMES_2));
	m_weather.m_wind = t.rotate_l2p(Vector::unitZ * static_cast<float>(m_minWind + m_randomGenerator->random(m_maxWind - m_minWind)));
	m_weather.m_weatherIntensity = m_randomGenerator->random(3); //TODO:  make higher intensities more rare
	m_weather.m_temperature = m_minTemp + m_randomGenerator->random(m_maxTemp - m_minTemp); //TODO:  make temperature change more gradual, base it on previous temperature ?
}

// ----------------------------------------------------------------------

SynchronizedWeatherGenerator::SynchronizedWeatherGenerator(int updateInterval) :
		m_updateInterval(updateInterval),
		m_weather(),
		m_nextUpdate(0),
		m_minTemp(0),
		m_maxTemp(0),
		m_minWind(0),
		m_maxWind(0),
		m_randomGenerator(new RandomGenerator)
{
	// Until we can integrate with environment blocks:
	// the weather is standardized to a mild climate, with
	// a wind speed of 0-10 m/s (roughly equal to 0-20 miles per hour), temperature 0-35 degrees
	// (32 - 95 degrees Fahrenheit.)
	// TODO:  Integrate with environment blocks
	setEnvironment(0,35,0,10);
}

// ----------------------------------------------------------------------

void SynchronizedWeatherGenerator::setEnvironment(int minTemp, int maxTemp, int minWind, int maxWind)
{
	m_minTemp = minTemp;
	m_maxTemp = maxTemp;
	m_minWind = minWind;
	m_maxWind = maxWind;
}

// ----------------------------------------------------------------------

SynchronizedWeatherGenerator::~SynchronizedWeatherGenerator()
{
	delete m_randomGenerator;
	m_randomGenerator = 0;
}

// ======================================================================


