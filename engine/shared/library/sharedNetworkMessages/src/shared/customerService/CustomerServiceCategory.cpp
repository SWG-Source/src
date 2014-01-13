// CustomerServiceCategory.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CustomerServiceCategory.h"

//-----------------------------------------------------------------------

CustomerServiceCategory::CustomerServiceCategory() :
m_categoryName(),
m_categoryId(0),
m_subCategories(),
m_isBugType(false),
m_isServiceType(false)
{
}

//-----------------------------------------------------------------------

CustomerServiceCategory::CustomerServiceCategory(Unicode::String const & categoryName, int const categoryId, bool const isBugType, bool const isServiceType) :
m_categoryName(categoryName),
m_categoryId(categoryId),
m_subCategories(),
m_isBugType(isBugType),
m_isServiceType(isServiceType)
{
}

//-----------------------------------------------------------------------

CustomerServiceCategory::~CustomerServiceCategory()
{
/*
	std::vector<CustomerServiceCategory *>::iterator categoriesIterator = 
		m_subCategories.begin();
	while (categoriesIterator != m_subCategories.end())
	{
		CustomerServiceCategory *category = (*categoriesIterator);
		delete category;
		++categoriesIterator;
	}
	*/
	m_subCategories.clear();
}

//-----------------------------------------------------------------------

void CustomerServiceCategory::addSubCategory(CustomerServiceCategory category)
{
	m_subCategories.push_back(category);
}

//-----------------------------------------------------------------------
