// Transceiver.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedMessageDispatch/FirstSharedMessageDispatch.h"
#include "sharedMessageDispatch/Transceiver.h"

#include <map>

namespace MessageDispatch {

//-----------------------------------------------------------------------

TransceiverBase::GlobalReceiverInfo::GlobalReceiverInfo() :
receivers(),
pendingAdds(),
pendingRemoves(),
locked(false)
{
}

//-----------------------------------------------------------------------

TransceiverBase::TransceiverBase() :
locked(false)
{
}

//-----------------------------------------------------------------------

TransceiverBase::~TransceiverBase()
{
}

//-----------------------------------------------------------------------

TransceiverBase::GlobalReceiverInfo & TransceiverBase::getGlobalReceiverInfo(const type_info & typeId)
{
	static std::map<const char * const, GlobalReceiverInfo> receiverSets;
	GlobalReceiverInfo & result = receiverSets[typeId.name()];
	return result;
}

//-----------------------------------------------------------------------

Callback::Callback() :
receivers()
{
}

//-----------------------------------------------------------------------

Callback::~Callback()
{
	std::vector<TransceiverBase *>::iterator i;
	TransceiverBase * t;
	for(i = receivers.begin(); i != receivers.end(); ++i)
	{
		t = (*i);
		delete t;
	}
}

//-----------------------------------------------------------------------

}//namespace MessageDispatch

