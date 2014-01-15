// MissionTerminalController.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MissionTerminalController_H
#define	_INCLUDED_MissionTerminalController_H

//-----------------------------------------------------------------------

#include "serverGame/TangibleController.h"

//-----------------------------------------------------------------------

class MissionTerminalController : public TangibleController
{
public:
	MissionTerminalController(TangibleObject * owner);
	~MissionTerminalController();

private:
	MissionTerminalController & operator = (const MissionTerminalController & rhs);
	MissionTerminalController(const MissionTerminalController & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MissionTerminalController_H
