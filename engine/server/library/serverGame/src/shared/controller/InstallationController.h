//========================================================================
//
// InstallationController.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_InstallationController_H
#define	_INCLUDED_InstallationController_H

//-----------------------------------------------------------------------

#include "serverGame/TangibleController.h"

//----------------------------------------------------------------------

class InstallationObject;

//-----------------------------------------------------------------------

class InstallationController : public TangibleController
{
public:
    explicit InstallationController     (InstallationObject * newOwner);
    virtual ~InstallationController     (void);

protected:
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);
	
private:
    InstallationController				(void);
    InstallationController				(const InstallationController & other);
    InstallationController&	operator=	(const InstallationController & other);
};

//-----------------------------------------------------------------------

#endif
