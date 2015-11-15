//========================================================================
//
// HarvesterInstallationController.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_HarvesterInstallationController_H
#define	_INCLUDED_HarvesterInstallationController_H

//-----------------------------------------------------------------------

#include "serverGame/InstallationController.h"

//----------------------------------------------------------------------

class HarvesterInstallationObject;

//-----------------------------------------------------------------------

class HarvesterInstallationController : public InstallationController
{
public:
    explicit HarvesterInstallationController (HarvesterInstallationObject * newOwner);
    virtual ~HarvesterInstallationController (void);

protected:
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);
	
private:
    HarvesterInstallationController				(void);
    HarvesterInstallationController				(const HarvesterInstallationController & other);
    HarvesterInstallationController&	operator=	(const HarvesterInstallationController & other);
};

//-----------------------------------------------------------------------

#endif
