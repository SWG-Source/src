// GrantSkill.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "GrantSkill.h"

//-----------------------------------------------------------------------

GrantSkill::GrantSkill(const std::string & n, const std::string & p, const std::string & c, const std::string & d, const std::vector<std::string> & cp) :
GameNetworkMessage("GrantSkill"),
category(c),
commandsProvided(),
description(d),
name(n),
discipline(p)
{
	commandsProvided.set(cp);
	addVariable(category);
	addVariable(commandsProvided);
	addVariable(description);
	addVariable(name);
	addVariable(discipline);
}

//-----------------------------------------------------------------------

GrantSkill::GrantSkill(Archive::ReadIterator & source) :
GameNetworkMessage("GrantSkill"),
category(),
commandsProvided(),
description(),
name(),
discipline()
{
	addVariable(category);
	addVariable(commandsProvided);
	addVariable(description);
	addVariable(name);
	addVariable(discipline);
	unpack(source);
}

//-----------------------------------------------------------------------

GrantSkill::~GrantSkill()
{
}


//-----------------------------------------------------------------------
