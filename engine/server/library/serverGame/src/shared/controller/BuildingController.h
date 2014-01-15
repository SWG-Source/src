//BuildingController.h

#ifndef	_BUILDING_CONTROLLER_H
#define	_BUILDING_CONTROLLER_H

//-----------------------------------------------------------------------

#include "serverGame/TangibleController.h"

//----------------------------------------------------------------------

class BuildingObject;

//-----------------------------------------------------------------------

class BuildingController : public TangibleController
{
public:
    explicit BuildingController         (BuildingObject * newOwner);
    ~BuildingController                 (void);

protected:
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);
	
private:
    BuildingController				(void);
    BuildingController				(const BuildingController & other);
    BuildingController&	operator=	(const BuildingController & other);
};

//-----------------------------------------------------------------------

#endif
