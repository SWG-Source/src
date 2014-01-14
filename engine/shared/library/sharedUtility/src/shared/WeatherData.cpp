// ======================================================================
//
// WeatherData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/WeatherData.h"

#include <string>

// ======================================================================

void WeatherData::debugPrint(std::string &output) const
{
	char buffer [256];
	snprintf(buffer,256,"Wind (%f %f %f), Temperature %i, Weather %i",m_wind.x,m_wind.y,m_wind.z,m_temperature,m_weatherIntensity);
	output=buffer;
}

// ======================================================================
