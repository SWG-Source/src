

// ======================================================================
//
// ObjectMenuRequestDataArchive.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectMenuRequestDataArchive_H
#define INCLUDED_ObjectMenuRequestDataArchive_H

//-----------------------------------------------------------------------

#include "unicodeArchive/UnicodeArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/ObjectMenuRequestData.h"
#include "Archive/Archive.h"

//-----------------------------------------------------------------------

namespace Archive
{
	
	//-----------------------------------------------------------------------
	
	inline void get(ReadIterator & source, ObjectMenuRequestData & target)
	{
		get(source, target.m_id);
		get(source, target.m_parent);
		get(source, target.m_menuItemType);
		get(source, target.m_flags);
		get(source, target.m_label);
	}
	
	//-----------------------------------------------------------------------
	
	inline void put(ByteStream & target, const ObjectMenuRequestData & source)
	{
		put(target, source.m_id);
		put(target, source.m_parent);
		put(target, source.m_menuItemType);
		put(target, source.m_flags);
		put(target, source.m_label);
	}
}

//----------------------------------------------------------------------

#endif
