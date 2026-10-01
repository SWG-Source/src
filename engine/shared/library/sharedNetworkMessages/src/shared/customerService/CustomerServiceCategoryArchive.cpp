//======================================================================
//
// CustomerServiceCategoryArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CustomerServiceCategoryArchive.h"
#include "Archive/ArchiveCount.h"

#include "sharedNetworkMessages/CustomerServiceCategory.h"

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, CustomerServiceCategory & target)
	{
		get (source, target.m_categoryName);	
		get (source, target.m_categoryId);	
		int size = 0;
		get (source, size);

		target.m_subCategories.clear();
		for (int i = 0; i < size; ++i)
		{
			CustomerServiceCategory category;	
			get (source, category);
			target.m_subCategories.push_back(category);
		}
		get (source, target.m_isBugType);	
		get (source, target.m_isServiceType);	
	}

	void put (ByteStream & target, const CustomerServiceCategory & source)
	{
		int const size = ArchiveCount::fromSize<int>(source.m_subCategories.size());

		put (target, source.m_categoryName);
		put (target, source.m_categoryId);
		put (target, size);

		std::vector<CustomerServiceCategory>::const_iterator i = source.m_subCategories.begin();
		while (i != source.m_subCategories.end())
		{
			put (target, (*i));
			++i;
		}

		put (target, source.m_isBugType);	
		put (target, source.m_isServiceType);	
	}
}

//======================================================================
