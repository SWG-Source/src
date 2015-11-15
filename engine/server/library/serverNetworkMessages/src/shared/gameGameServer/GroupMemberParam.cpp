//======================================================================
//
// GroupMemberParam.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GroupMemberParam.h"

// ======================================================================

GroupMemberParam::GroupMemberParam()
: m_memberId()
, m_memberName()
, m_memberDifficulty()
, m_memberProfession(LfgCharacterData::Prof_Unknown)
, m_memberIsPC(true)
, m_memberShipId()
, m_memberShipIsPOB(false)
, m_memberOwnsPOB(false)
{
}

GroupMemberParam::GroupMemberParam(NetworkId const & memberId, std::string const & memberName, int memberDifficulty, LfgCharacterData::Profession memberProfession, bool memberIsPC)
: m_memberId(memberId)
, m_memberName(memberName)
, m_memberDifficulty(memberDifficulty)
, m_memberProfession(memberProfession)
, m_memberIsPC(memberIsPC)
, m_memberShipId()
, m_memberShipIsPOB(false)
, m_memberOwnsPOB(false)
{
}

// ----------------------------------------------------------------------

GroupMemberParam::GroupMemberParam(NetworkId const & memberId, std::string const & memberName, int memberDifficulty, LfgCharacterData::Profession memberProfession, bool memberIsPC, NetworkId const & memberShipId, bool const memberShipIsPOB, bool const memberOwnsPOB)
: m_memberId(memberId)
, m_memberName(memberName)
, m_memberDifficulty(memberDifficulty)
, m_memberProfession(memberProfession)
, m_memberIsPC(memberIsPC)
, m_memberShipId(memberShipId)
, m_memberShipIsPOB(memberShipIsPOB)
, m_memberOwnsPOB(memberOwnsPOB)
{
}


//======================================================================
