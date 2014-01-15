//======================================================================
//
// GroupMemberParam.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_GroupMemberParam_H
#define INCLUDED_GroupMemberParam_H

//======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedGame/LfgCharacterData.h"

class GroupMemberParam
{
public:
	GroupMemberParam();
	GroupMemberParam(NetworkId const & memberId, std::string const & memberName, int memberDifficulty, LfgCharacterData::Profession memberProfession, bool memberIsPC);
	GroupMemberParam(NetworkId const & memberId, std::string const & memberName, int memberDifficulty, LfgCharacterData::Profession memberProfession, bool memberIsPC, NetworkId const & memberShipId, bool const memberShipIsPOB, bool const memberOwnsPOB);

	NetworkId m_memberId;
	std::string m_memberName;
	int m_memberDifficulty;
	LfgCharacterData::Profession m_memberProfession;
	bool m_memberIsPC;
	NetworkId m_memberShipId;
	bool m_memberShipIsPOB;
	bool m_memberOwnsPOB;

};

//======================================================================

#endif


