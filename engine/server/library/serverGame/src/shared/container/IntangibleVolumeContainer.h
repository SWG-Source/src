// ===============================================================
// IntangibleVolumeContainer.h
// copyright 2002 SonyOnline Interactive
// All rights reserved
// ===============================================================

#ifndef	INCLUDED_IntangibleVolumeContainer_H
#define	INCLUDED_IntangibleVolumeContainer_H

//-----------------------------------------------------------------------

#include "sharedObject/VolumeContainer.h"


class IntangibleVolumeContainer : public VolumeContainer
{

public:
	IntangibleVolumeContainer(Object& owner, int totalVolume);
	~IntangibleVolumeContainer();

	virtual bool           mayAdd (const Object& item, ContainerErrorCode& error) const;

private:
	IntangibleVolumeContainer();
	IntangibleVolumeContainer(const IntangibleVolumeContainer&);
	IntangibleVolumeContainer & operator= (const IntangibleVolumeContainer &);

};


#endif
