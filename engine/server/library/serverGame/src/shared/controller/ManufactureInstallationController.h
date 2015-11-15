// ManufactureInstallationController.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ManufactureInstallationController_H
#define	_INCLUDED_ManufactureInstallationController_H

//-----------------------------------------------------------------------

#include "serverGame/InstallationController.h"

class ManufactureInstallationObject;

//-----------------------------------------------------------------------

class ManufactureInstallationController : public InstallationController
{
public:
	explicit ManufactureInstallationController(ManufactureInstallationObject * owner);
	~ManufactureInstallationController();

protected:
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);

private:
	ManufactureInstallationController & operator = (const ManufactureInstallationController & rhs);
	ManufactureInstallationController(const ManufactureInstallationController & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ManufactureInstallationController_H
