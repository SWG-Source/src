//======================================================================
//
// CustomerServiceTicketArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CustomerServiceTicketArchive.h"

#include "sharedNetworkMessages/CustomerServiceTicket.h"

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, CustomerServiceTicket & target)
	{
		get (source, target.m_categoryId);	
		get (source, target.m_subCategoryId);	
		get (source, target.m_characterName);	
		get (source, target.m_details);	
		get (source, target.m_language);	
		get (source, target.m_ticketId);	
		get (source, target.m_modifiedDate);	
		get (source, target.m_read);	
		get (source, target.m_closed);	
	}

	void put (ByteStream & target, const CustomerServiceTicket & source)
	{
		put (target, source.m_categoryId);
		put (target, source.m_subCategoryId);
		put (target, source.m_characterName);
		put (target, source.m_details);
		put (target, source.m_language);
		put (target, source.m_ticketId);
		put (target, source.m_modifiedDate);
		put (target, source.m_read);
		put (target, source.m_closed);
	}
}

//======================================================================
