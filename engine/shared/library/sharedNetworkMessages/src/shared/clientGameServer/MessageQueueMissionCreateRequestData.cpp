//======================================================================
//
// MessageQueueMissionCreateRequestData.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionCreateRequestData.h"

//======================================================================

MessageQueueMissionCreateRequestData::MessageQueueMissionCreateRequestData () :
type (),
idAssignee (NetworkId::cms_invalid),
idTerminal (NetworkId::cms_invalid),
idTarget (NetworkId::cms_invalid),
reward (0)
{
}

//======================================================================
