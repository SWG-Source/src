// ======================================================================
//
// ResourceHarvesterActivatePageMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ResourceHarvesterActivatePageMessage.h"

//-----------------------------------------------------------------------

const char * const ResourceHarvesterActivatePageMessage::MESSAGE_TYPE = "ResourceHarvesterActivatePageMessage::MESSAGE_TYPE";

//-----------------------------------------------------------------

ResourceHarvesterActivatePageMessage::ResourceHarvesterActivatePageMessage (const NetworkId & harvesterId) :
GameNetworkMessage (MESSAGE_TYPE),
m_harvesterId (harvesterId)
{
	addVariable (m_harvesterId);
}

//-----------------------------------------------------------------------

ResourceHarvesterActivatePageMessage::ResourceHarvesterActivatePageMessage (Archive::ReadIterator & source) :
GameNetworkMessage (MESSAGE_TYPE),
m_harvesterId ()
{
	addVariable (m_harvesterId);
	unpack (source);
}

//----------------------------------------------------------------------

ResourceHarvesterActivatePageMessage::~ResourceHarvesterActivatePageMessage ()
{
}

//-----------------------------------------------------------------------
