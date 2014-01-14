// ======================================================================
//
// SynchronizedWeatherGenerator.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SynchronizedWeatherGenerator_H
#define INCLUDED_SynchronizedWeatherGenerator_H

// ======================================================================

#include "sharedUtility/WeatherData.h"

class RandomGenerator;

// ======================================================================

/** 
 * A class that generates random weather based on the clock.
 * As long as their clocks are synchronized, multiple servers or clients
 * can use this class and get the same weather
 */
class SynchronizedWeatherGenerator
{
  public:
	SynchronizedWeatherGenerator(int updateInterval);
	~SynchronizedWeatherGenerator();

	void setEnvironment(int minTemp, int maxTemp, int minWind, int maxWind);
	const WeatherData &getWeather(unsigned long serverTime) const;

  private:
	int m_updateInterval;
	mutable WeatherData m_weather;
	mutable unsigned long m_nextUpdate; // "unsigned long" for consistency with time functions
		
	int m_minTemp;
	int m_maxTemp;
	int m_minWind;
	int m_maxWind;

	RandomGenerator *m_randomGenerator;

  private:
	void generateWeather(unsigned long serverTime) const;
};

// ----------------------------------------------------------------------

/**
 * Quickly check whether our cached weather data is still usable.  Return
 * it if it is, regenerate it if it isn't.
 * @param serverTime The time according to the game server (see ServerClock.cpp)
 */
inline const WeatherData &SynchronizedWeatherGenerator::getWeather(unsigned long serverTime) const
{
	if (serverTime >= m_nextUpdate)
		generateWeather(serverTime);
	return m_weather;
}

// ======================================================================

#endif
