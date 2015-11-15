//======================================================================
//
// InstallationResourceDataArchive.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_InstallationResourceDataArchive_H
#define INCLUDED_InstallationResourceDataArchive_H

//======================================================================

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedUtility/InstallationResourceData.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

namespace Archive
{
	//-----------------------------------------------------------------------
	
	inline void get(ReadIterator & source, InstallationResourceData & target)
	{
		get(source, target.m_id);
		get(source, target.m_name);
		get(source, target.m_parentName);
		get(source, target.m_efficiency);
	}
	
	//-----------------------------------------------------------------------
	
	inline void put(ByteStream & target, const InstallationResourceData & source)
	{
		put(target, source.m_id);
		put(target, source.m_name);
		put(target, source.m_parentName);
		put(target, source.m_efficiency);
	}
}

//======================================================================

#endif
