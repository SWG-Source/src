// GrantSkill.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_GrantSkill_H
#define	_INCLUDED_GrantSkill_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class GrantSkill : public GameNetworkMessage
{
public:
	GrantSkill(const std::string & name, const std::string & discipline, const std::string & category, const std::string & description, const std::vector<std::string> & commandsProvided);
	GrantSkill(Archive::ReadIterator & source);
	~GrantSkill();

	const std::string &               getCategory          () const;
	const std::vector<std::string> &  getCommandsProvided  () const;
	const std::string &               getDescription       () const;
	const std::string &               getName              () const;
	const std::string &               getDiscipline        () const;

private:
	Archive::AutoVariable<std::string> category;
	Archive::AutoArray<std::string>    commandsProvided;
	Archive::AutoVariable<std::string> description;
	Archive::AutoVariable<std::string> name;
	Archive::AutoVariable<std::string> discipline;	
};

//-----------------------------------------------------------------------

inline const std::string & GrantSkill::getCategory() const
{
	return category.get();
}

//-----------------------------------------------------------------------

inline const std::vector<std::string> & GrantSkill::getCommandsProvided() const
{
	return commandsProvided.get();
}

//-----------------------------------------------------------------------

inline const std::string & GrantSkill::getDescription() const
{
	return description.get();
}

//-----------------------------------------------------------------------

inline const std::string & GrantSkill::getName() const
{
	return name.get();
}

//-----------------------------------------------------------------------

inline const std::string & GrantSkill::getDiscipline() const
{
	return discipline.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GrantSkill_H

