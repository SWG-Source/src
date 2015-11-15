//======================================================================
//
// MiningAsteroidController.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MiningAsteroidController_H
#define INCLUDED_MiningAsteroidController_H

//======================================================================

#include "serverGame/ShipController.h"

//----------------------------------------------------------------------

class MiningAsteroidController : public ShipController
{
public:

	explicit MiningAsteroidController(ShipObject * const owner);
	virtual ~MiningAsteroidController();
	virtual bool shouldCheckForEnemies() const;

	void setVelocity_w(Vector const & velocity_w);
	Vector const & getVelocity_w() const;
	
private:
	                         MiningAsteroidController ();
	                         MiningAsteroidController (const MiningAsteroidController & rhs);
	MiningAsteroidController & operator= (const MiningAsteroidController & rhs);

protected:

	virtual float realAlter(float elapsedTime);
	virtual void experiencedCollision();

private:

	float m_lifeTimeRemainingSecs;
	int m_lastNumberOfHits;
};

//======================================================================

#endif
