// ======================================================================
//
// ParametersMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ParametersMessage.h"

// ======================================================================

ParametersMessage::ParametersMessage(int weatherUpdateInterval) :
		GameNetworkMessage("ParametersMessage"),
		m_weatherUpdateInterval(weatherUpdateInterval)
{
	addVariable(m_weatherUpdateInterval);
}

//-----------------------------------------------------------------------

ParametersMessage::ParametersMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ParametersMessage"),
		m_weatherUpdateInterval()
{
	addVariable(m_weatherUpdateInterval);

	unpack(source);
}

// ----------------------------------------------------------------------

ParametersMessage::~ParametersMessage()
{
}


// ======================================================================

