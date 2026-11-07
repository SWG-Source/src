// ======================================================================
//
// ParametersMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ParametersMessage.h"

// ======================================================================

ParametersMessage::ParametersMessage(int weatherUpdateInterval, int entertainerCaptchaPercent) :
		GameNetworkMessage("ParametersMessage"),
		m_weatherUpdateInterval(weatherUpdateInterval),
		m_entertainerCaptchaPercent(entertainerCaptchaPercent)
{
	addVariable(m_weatherUpdateInterval);
	addVariable(m_entertainerCaptchaPercent);
}

//-----------------------------------------------------------------------

ParametersMessage::ParametersMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ParametersMessage"),
		m_weatherUpdateInterval(),
		m_entertainerCaptchaPercent()
{
	addVariable(m_weatherUpdateInterval);
	addVariable(m_entertainerCaptchaPercent);

	unpack(source);
}

// ----------------------------------------------------------------------

ParametersMessage::~ParametersMessage()
{
}


// ======================================================================

