//========================================================================
//
// PreloadFinishedMessage.cpp
//
// copyright 2002 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/PreloadFinishedMessage.h"

#include <vector>

#include "serverNetworkMessages/CentralGameServerMessages.h"

//-----------------------------------------------------------------------

PreloadFinishedMessage::PreloadFinishedMessage(bool finished) :
		GameNetworkMessage("PreloadFinishedMessage"),
		m_finished(finished)
{
	addVariable(m_finished);
}

//-----------------------------------------------------------------------

PreloadFinishedMessage::PreloadFinishedMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("PreloadFinishedMessage"),
		m_finished(false)
{
	addVariable(m_finished);
	unpack(source);
}

//-----------------------------------------------------------------------

PreloadFinishedMessage::~PreloadFinishedMessage()
{
}

// ======================================================================
