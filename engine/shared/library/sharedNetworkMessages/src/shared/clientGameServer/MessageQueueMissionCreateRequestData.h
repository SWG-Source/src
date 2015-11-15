//======================================================================
//
// MessageQueueMissionCreateRequestData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionCreateRequestData_H
#define INCLUDED_MessageQueueMissionCreateRequestData_H

//======================================================================

#include "sharedFoundation/NetworkId.h"

//----------------------------------------------------------------------

class MessageQueueMissionCreateRequestData
{
public:
	MessageQueueMissionCreateRequestData ();

	unsigned char  type;
	NetworkId      idAssignee;
	NetworkId      idTerminal;
	NetworkId      idTarget;
	uint32         reward;
};

//======================================================================

#endif
