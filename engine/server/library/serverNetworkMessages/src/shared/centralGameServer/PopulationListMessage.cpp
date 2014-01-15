// ======================================================================
//
// PopulationListMessage.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/PopulationListMessage.h"

// ======================================================================

PopulationListMessage::PopulationListMessage(const PopulationList &list) :
		GameNetworkMessage("PopulationListMessage"),
		m_list(list)
{
	addVariable(m_list);
}

// ----------------------------------------------------------------------

PopulationListMessage::PopulationListMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("PopulationListMessage"),
		m_list()
{
	addVariable(m_list);
	unpack(source);
}

// ----------------------------------------------------------------------

PopulationListMessage::~PopulationListMessage()
{
}
   
// ======================================================================
