//======================================================================
//
// CustomerServiceCommentArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CustomerServiceCommentArchive.h"

#include "sharedNetworkMessages/CustomerServiceComment.h"

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, CustomerServiceComment & target)
	{
		get (source, target.m_ticketId);	
		get (source, target.m_commentId);	
		get (source, target.m_fromCsr);	
		get (source, target.m_comment);	
		get (source, target.m_commentorName);	
	}

	void put (ByteStream & target, const CustomerServiceComment & source)
	{
		put (target, source.m_ticketId);
		put (target, source.m_commentId);
		put (target, source.m_fromCsr);
		put (target, source.m_comment);
		put (target, source.m_commentorName);
	}
}

//======================================================================
