// ======================================================================
//
// WeatherData.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_WeatherData_H
#define INCLUDED_WeatherData_H

// ======================================================================

#include "sharedMath/Vector.h"

// ======================================================================

/** 
 * A data structure to hold weather information.
 */
class WeatherData
{
  public:
	Vector m_wind;
	int m_weatherIntensity;
	int m_temperature;

	void debugPrint(std::string &output) const;
};

// ======================================================================

#endif
