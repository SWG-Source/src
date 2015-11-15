// ======================================================================
//
// RequestCategoriesResponseMessage.h
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDE_RequestCategoriesResponseMessage_H
#define	INCLUDE_RequestCategoriesResponseMessage_H

#include "sharedNetworkMessages/CustomerServiceCategory.h"
#include "sharedNetworkMessages/CustomerServiceCategoryArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------
class RequestCategoriesResponseMessage : public GameNetworkMessage
{
public:

	RequestCategoriesResponseMessage(int32 const result, std::vector<CustomerServiceCategory> const & categories);
	explicit RequestCategoriesResponseMessage(Archive::ReadIterator & source);
	~RequestCategoriesResponseMessage();

	int32 getResult() const;
	std::vector<CustomerServiceCategory> const & getCategories() const;

private: 

	Archive::AutoVariable<int32> m_result;
	Archive::AutoArray<CustomerServiceCategory> m_categories;
};

// ======================================================================

#endif // INCLUDE_RequestCategoriesResponseMessage_H
