// CustomerServiceSearchResult.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CustomerServiceSearchResult.h"

//-----------------------------------------------------------------------

CustomerServiceSearchResult::CustomerServiceSearchResult(
	const Unicode::String &title, 
	const std::string &id, 
	short matchPercent
) :
m_title(title),
m_id(id),
m_matchPercent(matchPercent)
{
}

//-----------------------------------------------------------------------

CustomerServiceSearchResult::CustomerServiceSearchResult() :
m_title(),
m_id(),
m_matchPercent(0)
{
}

//-----------------------------------------------------------------------

CustomerServiceSearchResult::~CustomerServiceSearchResult()
{
}

//-----------------------------------------------------------------------
