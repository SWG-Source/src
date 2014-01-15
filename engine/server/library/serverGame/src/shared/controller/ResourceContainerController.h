//ResourceContainerController.h

#ifndef	_WEAPON_CONTROLLER_H
#define	_WEAPON_CONTROLLER_H

//-----------------------------------------------------------------------

#include "serverGame/TangibleController.h"

//----------------------------------------------------------------------

class ResourceContainerObject;

//-----------------------------------------------------------------------

class ResourceContainerController : public TangibleController
{
public:
    explicit ResourceContainerController         (ResourceContainerObject * newOwner);
    ~ResourceContainerController                 (void);

private:
    ResourceContainerController				(void);
    ResourceContainerController				(const ResourceContainerController & other);
    ResourceContainerController	operator=	(const ResourceContainerController & other);
};

//-----------------------------------------------------------------------

#endif
