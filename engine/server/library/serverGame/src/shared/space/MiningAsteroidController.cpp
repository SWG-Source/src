// ======================================================================
//
// MiningAsteroidController.cpp
// jwatson
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/MiningAsteroidController.h"

#include "serverGame/ShipObject.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/ShipDynamicsModel.h"
#include "sharedObject/AlterResult.h"

//----------------------------------------------------------------------

namespace MiningAsteroidControllerNamespace
{
	static const float s_dynamicLifeSpanSecs = 60.0f;
}

using namespace MiningAsteroidControllerNamespace;

//----------------------------------------------------------------------

MiningAsteroidController::MiningAsteroidController(ShipObject * const owner) :
ShipController(owner),
m_lifeTimeRemainingSecs(s_dynamicLifeSpanSecs),
m_lastNumberOfHits(0)
{
	
}

//----------------------------------------------------------------------

MiningAsteroidController::~MiningAsteroidController()
{
}

//----------------------------------------------------------------------

float MiningAsteroidController::realAlter(float elapsedTime)
{
	PROFILER_AUTO_BLOCK_DEFINE("MiningAsteroidController::realAlter");

	ShipObject * const shipOwner = NON_NULL(safe_cast<ShipObject *>(getOwner()));
	int const got = shipOwner->getGameObjectType();

	bool killIt = false;

	if (got == SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic)
	{		
		//-- we've been shot lately, reset our timer
		if (m_lastNumberOfHits != shipOwner->getNumberOfHits())
		{
			m_lifeTimeRemainingSecs = s_dynamicLifeSpanSecs;
			m_lastNumberOfHits = shipOwner->getNumberOfHits();
		}
		else
			m_lifeTimeRemainingSecs -= elapsedTime;

		Vector const & pos_w = shipOwner->getPosition_p();

		//-- timeout
		if (m_lifeTimeRemainingSecs <= 0.0f)
			killIt = true;
		//-- nobody is observing it
		else if (shipOwner->getObservers().empty())
			killIt = true;
		else
		{
			m_shipDynamicsModel->setRollRate(1.0f);
			
			Vector const & velocity_w = m_shipDynamicsModel->getVelocity();
			shipOwner->lookAt_p(pos_w + velocity_w);
			shipOwner->move_p(velocity_w * elapsedTime);
			shipOwner->roll_o(elapsedTime);
			
			m_shipDynamicsModel->setTransform(shipOwner->getTransform_o2p());
		}
	}

	float alterResult = ShipController::realAlter(elapsedTime);
	AlterResult::incorporateAlterResult(alterResult, killIt ? AlterResult::cms_kill : AlterResult::cms_alterNextFrame);
	return alterResult;
}

//----------------------------------------------------------------------

bool MiningAsteroidController::shouldCheckForEnemies() const
{
	return false;
}

//----------------------------------------------------------------------

void MiningAsteroidController::experiencedCollision()
{
}

//----------------------------------------------------------------------

void MiningAsteroidController::setVelocity_w(Vector const & velocity_w)
{
	m_shipDynamicsModel->setVelocity(velocity_w);
}

//----------------------------------------------------------------------

Vector const & MiningAsteroidController::getVelocity_w() const
{
	return m_shipDynamicsModel->getVelocity();
}

//----------------------------------------------------------------------



