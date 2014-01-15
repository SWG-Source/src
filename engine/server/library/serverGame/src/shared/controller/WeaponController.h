//WeaponController.h

#ifndef	_WEAPON_CONTROLLER_H
#define	_WEAPON_CONTROLLER_H

//-----------------------------------------------------------------------

#include "serverGame/TangibleController.h"

//----------------------------------------------------------------------

class WeaponObject;

//-----------------------------------------------------------------------

class WeaponController : public TangibleController
{
public:
    explicit WeaponController         (WeaponObject * newOwner);
    ~WeaponController                 (void);

private:
    WeaponController				(void);
    WeaponController				(const WeaponController & other);
    WeaponController	operator=	(const WeaponController & other);
};

//-----------------------------------------------------------------------

#endif
