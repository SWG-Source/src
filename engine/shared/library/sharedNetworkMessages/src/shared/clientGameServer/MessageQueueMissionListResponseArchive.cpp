//======================================================================
//
// MessageQueueMissionListResponseArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponseArchive.h"

#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponseData.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponseDataArchive.h"

//======================================================================

namespace Archive
{

	//----------------------------------------------------------------------

	void get (ReadIterator & source, MessageQueueMissionListResponse & target)
	{
		MessageQueueMissionListResponse::DataVector v;
		size_t s = 0;
		unsigned char sequenceId = 0;
		bool b = false;

		Archive::get(source, sequenceId);
		Archive::get(source, b);
		Archive::get(source, s);

		v.reserve (s);

		for(size_t i = 0; i < s; ++i)
		{
			MessageQueueMissionListResponse::DataElement r;
			Archive::get(source, r);
			v.push_back(r);		
		}
		
		target.set (v, sequenceId, b);
	}
	
	//----------------------------------------------------------------------

	void put (ByteStream & target, const MessageQueueMissionListResponse & source)
	{ 
		Archive::put(target, source.getSequenceId());
		Archive::put(target, source.getBountyTerminal());
		Archive::put(target, source.getResponse().size());
	
		typedef MessageQueueMissionListResponse::DataVector DataVector;
		const DataVector & v = source.getResponse();
		
		for(DataVector::const_iterator i = v.begin(); i != v.end(); ++i)
		{
			Archive::put(target, *i);
		}
	}
}

//======================================================================
