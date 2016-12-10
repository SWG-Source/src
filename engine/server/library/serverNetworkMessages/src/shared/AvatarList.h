// ======================================================================
//
// AvatarList.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AvatarList_H
#define INCLUDED_AvatarList_H

// ======================================================================

#include "Unicode.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

/**
 * Data structures for passing around the list of avatars for an account.
 */
struct AvatarRecord
{
	Unicode::String m_name;
	int m_objectTemplateId;
	NetworkId m_networkId;
	uint32 m_clusterId;
	int m_characterType;
};

// ======================================================================

typedef std::vector<AvatarRecord> AvatarList;

// ======================================================================

#endif
