// ======================================================================
//
// RequestCategoriesResponseMessage.cpp
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/RequestCategoriesResponseMessage.h"

// ----------------------------------------------------------------------
RequestCategoriesResponseMessage::RequestCategoriesResponseMessage(int32 const result, std::vector<CustomerServiceCategory> const & categories)
 : GameNetworkMessage("RequestCategoriesResponseMessage")
 , m_result(result)
 , m_categories()
{
	m_categories.set(categories);
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_categories);
}

// ----------------------------------------------------------------------
RequestCategoriesResponseMessage::RequestCategoriesResponseMessage(Archive::ReadIterator & source)
 : GameNetworkMessage("RequestCategoriesResponseMessage")
 , m_result()
 , m_categories()
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_categories);
	unpack(source);
}

// ----------------------------------------------------------------------
RequestCategoriesResponseMessage::~RequestCategoriesResponseMessage()
{
}

// ----------------------------------------------------------------------
int32 RequestCategoriesResponseMessage::getResult() const
{
	return m_result.get();
}

// ----------------------------------------------------------------------
std::vector<CustomerServiceCategory> const & RequestCategoriesResponseMessage::getCategories() const
{
	return m_categories.get();
}

// ======================================================================
