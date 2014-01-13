// CustomerServiceCategory.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

#ifndef INCLUDED_CustomerServiceCategory_H
#define INCLUDED_CustomerServiceCategory_H

#include <string>
#include <vector>

//--------------------------------------------------------

class CustomerServiceCategory
{
public:

	CustomerServiceCategory();
	CustomerServiceCategory(Unicode::String const & categoryName, int const categoryId, bool const isBugType, bool const isServiceType);
	~CustomerServiceCategory();

	void addSubCategory(CustomerServiceCategory category);

	Unicode::String const & getCategoryName() const { return m_categoryName; }
	int getCategoryId() const { return m_categoryId; }
	std::vector<CustomerServiceCategory> const & getSubCategories() const { return m_subCategories; }

	Unicode::String m_categoryName;
	int m_categoryId;
	std::vector<CustomerServiceCategory> m_subCategories;
	bool m_isBugType;
	bool m_isServiceType;
};

#endif //INCLUDED_CustomerServiceCategory_H
