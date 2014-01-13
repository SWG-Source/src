// RevokeSkill.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "RevokeSkill.h"

//-----------------------------------------------------------------------

RevokeSkill::RevokeSkill(const std::string & s) : 
GameNetworkMessage("RevokeSkill"),
skillName(s)
{
	addVariable(skillName);
}

//-----------------------------------------------------------------------

RevokeSkill::RevokeSkill(Archive::ReadIterator & source) :
GameNetworkMessage("RevokeSkill"),
skillName()
{
	addVariable(skillName);
	unpack(source);
}

//-----------------------------------------------------------------------

RevokeSkill::~RevokeSkill()
{
}

//-----------------------------------------------------------------------

