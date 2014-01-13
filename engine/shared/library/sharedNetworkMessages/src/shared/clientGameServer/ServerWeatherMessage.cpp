// ==================================================================
//
// ServerWeatherMessage.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ServerWeatherMessage.h"

#include "sharedMathArchive/VectorArchive.h"

// ==================================================================
// STATIC PUBLIC ServerWeatherMessage
// ==================================================================

const char* const ServerWeatherMessage::cms_name = "ServerWeatherMessage";

// ==================================================================
// PUBLIC ServerWeatherMessage
// ==================================================================

ServerWeatherMessage::ServerWeatherMessage (const int index, const Vector& windVelocity_w) :
	GameNetworkMessage (ServerWeatherMessage::cms_name),
	m_index (index),
	m_windVelocity_w (windVelocity_w)
{
	addVariable (m_index);
	addVariable (m_windVelocity_w);
}

// ------------------------------------------------------------------

ServerWeatherMessage::ServerWeatherMessage (Archive::ReadIterator& source) :
	GameNetworkMessage (ServerWeatherMessage::cms_name),
	m_index (),
	m_windVelocity_w ()
{
	addVariable (m_index);
	addVariable (m_windVelocity_w);
	unpack (source);
}

// ------------------------------------------------------------------

ServerWeatherMessage::~ServerWeatherMessage ()
{
}

// ------------------------------------------------------------------

int ServerWeatherMessage::getIndex () const
{
	return m_index.get ();
}

// ----------------------------------------------------------------------

const Vector& ServerWeatherMessage::getWindVelocity_w () const
{
	return m_windVelocity_w.get ();
}

// ==================================================================
