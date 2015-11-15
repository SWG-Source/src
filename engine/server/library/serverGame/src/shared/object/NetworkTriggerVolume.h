// NetworkTriggerVolume.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.


#ifndef	_INCLUDED_NetworkTriggerVolume_H
#define	_INCLUDED_NetworkTriggerVolume_H

//-----------------------------------------------------------------------

#include "serverGame/TriggerVolume.h"

//-----------------------------------------------------------------------

namespace NetworkTriggerVolumeNamespace
{
	const char NetworkTriggerVolumeName[] = "m_networkUpdateFar";
}

//-----------------------------------------------------------------------

class NetworkTriggerVolume : public TriggerVolume
{
public:
	NetworkTriggerVolume(ServerObject & owner, real radius);
	virtual ~NetworkTriggerVolume();

	virtual bool isNetworkTriggerVolume() const;
	
private:
	virtual void virtualOnEnter(ServerObject& object);
	virtual void virtualOnExit(ServerObject& object);

	NetworkTriggerVolume(const NetworkTriggerVolume&);
	NetworkTriggerVolume& operator= (const NetworkTriggerVolume& );
};

#endif
