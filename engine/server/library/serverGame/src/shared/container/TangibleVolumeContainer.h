// ===============================================================
// TangibleVolumeContainer.h
// copyright 2002 SonyOnline Interactive
// All rights reserved
// ===============================================================

#ifndef	INCLUDED_TangibleVolumeContainer_H
#define	INCLUDED_TangibleVolumeContainer_H

//-----------------------------------------------------------------------

#include "sharedObject/VolumeContainer.h"


class TangibleVolumeContainer : public VolumeContainer
{

public:
	TangibleVolumeContainer(Object& owner, int totalVolume);
	~TangibleVolumeContainer();

	virtual bool           mayAdd (const Object& item, ContainerErrorCode& error) const;

private:
	TangibleVolumeContainer();
	TangibleVolumeContainer(const TangibleVolumeContainer&);
	TangibleVolumeContainer & operator= (const TangibleVolumeContainer &);

};


#endif
