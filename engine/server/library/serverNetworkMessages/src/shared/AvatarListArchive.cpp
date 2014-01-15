// ======================================================================
//
// AvatarList.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "Archive/Archive.h"
#include "serverNetworkMessages/AvatarList.h"

// ======================================================================

namespace Archive
{
	void get (ReadIterator & source, AvatarRecord & target)
	{
		get(source, target.m_name);
		get(source, target.m_objectTemplateId);
		get(source, target.m_networkId);
		get(source, target.m_clusterId);
		get(source, target.m_characterType);
	}
	
	void put (ByteStream & target, const AvatarRecord & source)
	{
		put(target, source.m_name);
		put(target, source.m_objectTemplateId);
		put(target, source.m_networkId);
		put(target, source.m_clusterId);
		put(target, source.m_characterType);
	}
}

// ======================================================================
