// ======================================================================
//
// PortalTriggerVolume.h
//
// Copyright 2000-04 Sony Online Entertainment
//
// ======================================================================

#ifndef _INCLUDED_PortalTriggerVolume_H
#define _INCLUDED_PortalTriggerVolume_H

// ======================================================================

#include "serverGame/TriggerVolume.h"

// ======================================================================

class PortalTriggerVolume : public TriggerVolume
{
public:
	PortalTriggerVolume(ServerObject &owner, real radius);
	virtual ~PortalTriggerVolume();

	virtual bool isPortalTriggerVolume() const;

	static char const *getName();
	
private:
	virtual void virtualOnEnter(ServerObject &object);
	virtual void virtualOnExit(ServerObject &object);

	PortalTriggerVolume(PortalTriggerVolume const &);
	PortalTriggerVolume &operator=(PortalTriggerVolume const &);
};

// ======================================================================

#endif // _INCLUDED_PortalTriggerVolume_H

