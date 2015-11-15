// ======================================================================
//
// ProjectileManager.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ProjectileManager_H
#define INCLUDED_ProjectileManager_H

// ======================================================================

class Client;
class ShipObject;
class Transform;

// ======================================================================

class ProjectileManager
{
public:

	static void create(Client const *gunnerClient, ShipObject &owner, int weaponIndex, int projectileIndex, int targetedComponent, float startDeltaTime, Transform const & transform_p, float length, float speed, float duration, bool const fromAutoTurret);
	static void stopBeam(ShipObject & shipOwner, int weaponIndex);
	static void update(float timePassed);
};

// ======================================================================

#endif // INCLUDED_ProjectileManager_H

