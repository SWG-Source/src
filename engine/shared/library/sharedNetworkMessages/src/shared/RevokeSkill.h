// RevokeSkill.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_RevokeSkill_H
#define	_INCLUDED_RevokeSkill_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class RevokeSkill : public GameNetworkMessage
{
public:
	RevokeSkill(const std::string & skillName);
	RevokeSkill(Archive::ReadIterator & source);
	~RevokeSkill();

	const std::string & getSkillName() const;

private:
	RevokeSkill & operator = (const RevokeSkill & rhs);
	RevokeSkill(const RevokeSkill & source);

private:
	Archive::AutoVariable<std::string>  skillName;
};

//-----------------------------------------------------------------------

inline const std::string & RevokeSkill::getSkillName() const
{
	return skillName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_RevokeSkill_H
