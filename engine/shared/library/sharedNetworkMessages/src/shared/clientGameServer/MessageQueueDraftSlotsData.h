
//======================================================================
//
// MessageQueueDraftSlotsData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueDraftSlotsData_H
#define INCLUDED_MessageQueueDraftSlotsData_H

//======================================================================

#include "sharedNetworkMessages/MessageQueueDraftSlotsDataOption.h"
#include "StringId.h"
#include <vector>

//----------------------------------------------------------------------

class MessageQueueDraftSlotsData
{
public:
	typedef MessageQueueDraftSlotsDataOption Option;
	typedef stdvector<Option>::fwd           OptionVector;
	
	StringId                   name;
	bool                       optional;
	OptionVector               options;
	std::string                hardpoint;

	MessageQueueDraftSlotsData ();
};

//======================================================================

#endif
