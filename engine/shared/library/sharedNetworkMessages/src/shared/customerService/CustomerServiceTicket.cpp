// CustomerServiceTicket.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CustomerServiceTicket.h"

//-----------------------------------------------------------------------

CustomerServiceTicket::CustomerServiceTicket(
	unsigned int categoryId, 
	unsigned int subCategoryId, 
	const std::string &characterName, 
	const Unicode::String &details, 
	const std::string &language,
	unsigned int ticketId,
	int64 modifiedDate,
	bool read,
	bool closed
) :
m_categoryId(categoryId),
m_subCategoryId(subCategoryId),
m_characterName(characterName),
m_details(details),
m_language(language),
m_ticketId(ticketId),
m_modifiedDate(modifiedDate),
m_read(read),
m_closed(closed)
{
}

//-----------------------------------------------------------------------

CustomerServiceTicket::CustomerServiceTicket() :
m_categoryId(0),
m_subCategoryId(0),
m_characterName(),
m_details(),
m_language(),
m_ticketId(0),
m_modifiedDate(0),
m_read(false),
m_closed(false)
{
}

//-----------------------------------------------------------------------

CustomerServiceTicket::~CustomerServiceTicket()
{
}

//-----------------------------------------------------------------------
