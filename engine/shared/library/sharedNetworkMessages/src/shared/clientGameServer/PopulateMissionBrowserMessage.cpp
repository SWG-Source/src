// ======================================================================
//
// PopulateMissionBrowserMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/PopulateMissionBrowserMessage.h"

// ======================================================================

PopulateMissionBrowserMessage::PopulateMissionBrowserMessage(std::vector<NetworkId> const &missions) :
	GameNetworkMessage("PopulateMissionBrowserMessage"),
	m_missions()
{
	m_missions.set(missions);
	AutoByteStream::addVariable(m_missions);
}

// ----------------------------------------------------------------------

PopulateMissionBrowserMessage::PopulateMissionBrowserMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("PopulateMissionBrowserMessage"),
	m_missions()
{
	AutoByteStream::addVariable(m_missions);
	unpack(source);
}

// ----------------------------------------------------------------------

PopulateMissionBrowserMessage::~PopulateMissionBrowserMessage()
{
}

// ======================================================================

