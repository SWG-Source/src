//TurretController.h

#ifndef	_TURRET_CONTROLLER_H
#define	_TURRET_CONTROLLER_H

//-----------------------------------------------------------------------

#include "serverGame/WeaponController.h"

//----------------------------------------------------------------------

class WeaponObject;

//-----------------------------------------------------------------------

class TurretController : public WeaponController
{
public:
    explicit TurretController         (WeaponObject * newOwner);
    ~TurretController                 (void);

private:
    TurretController				(void);
    TurretController				(const TurretController & other);
    TurretController	operator=	(const TurretController & other);
};

//-----------------------------------------------------------------------

#endif
