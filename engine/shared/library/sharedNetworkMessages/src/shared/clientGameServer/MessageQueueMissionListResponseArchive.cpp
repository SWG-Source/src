//======================================================================
//
// MessageQueueMissionListResponseArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponseArchive.h"
#include "Archive/ArchiveCount.h"

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
		// The element count is 32 bits on the wire (as the 32-bit client and
		// 32-bit servers have always sent it), never size_t.
		uint32_t s = 0;
		unsigned char sequenceId = 0;
		bool b = false;

		Archive::get(source, sequenceId);
		Archive::get(source, b);
		Archive::get(source, s);

		v.reserve (s);

		for(uint32_t i = 0; i < s; ++i)
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
		uint32_t const count = ArchiveCount::fromSize<uint32_t>(source.getResponse().size());
		Archive::put(target, source.getSequenceId());
		Archive::put(target, source.getBountyTerminal());
		Archive::put(target, count);
	
		typedef MessageQueueMissionListResponse::DataVector DataVector;
		const DataVector & v = source.getResponse();
		
		for(DataVector::const_iterator i = v.begin(); i != v.end(); ++i)
		{
			Archive::put(target, *i);
		}
	}
}

//======================================================================
