//======================================================================
//
// GroupMemberParamArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GroupMemberParamArchive.h"

#include "Archive/Archive.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "serverNetworkMessages/GroupMemberParam.h"
#include "unicodeArchive/UnicodeArchive.h"

//======================================================================

namespace Archive
{
	//----------------------------------------------------------------------

	void get(Archive::ReadIterator & source, GroupMemberParam & target)
	{
		get(source, target.m_memberId);
		get(source, target.m_memberName);
		get(source, target.m_memberDifficulty);

		uint8 uChar = 0;
		get(source, uChar);
		target.m_memberProfession = static_cast<LfgCharacterData::Profession>(uChar);

		get(source, target.m_memberIsPC);
		get(source, target.m_memberShipId);
		get(source, target.m_memberShipIsPOB);
		get(source, target.m_memberOwnsPOB);
	}

	//----------------------------------------------------------------------

	void put(ByteStream & target, const GroupMemberParam & source)
	{
		put(target, source.m_memberId);
		put(target, source.m_memberName);
		put(target, source.m_memberDifficulty);
		put(target, static_cast<uint8>(source.m_memberProfession));
		put(target, source.m_memberIsPC);
		put(target, source.m_memberShipId);
		put(target, source.m_memberShipIsPOB);
		put(target, source.m_memberOwnsPOB);
	}
}

//======================================================================
