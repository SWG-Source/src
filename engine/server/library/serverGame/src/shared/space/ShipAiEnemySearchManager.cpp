// ======================================================================
//
// ShipAiEnemySearchManager.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipAiEnemySearchManager.h"

#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceAttackSquad.h"
#include "serverGame/SpaceVisibilityManager.h"
#include "sharedLog/Log.h"
#include <queue>

// ======================================================================

namespace ShipAiEnemySearchManagerNamespace
{
	std::queue<Watcher<ShipObject> > s_enemySearchQueue;

	void checkForEnemies(ShipObject &ship);

	int const s_maxNumberOfAttackingShips = 10;
}

using namespace ShipAiEnemySearchManagerNamespace;

// ======================================================================

void ShipAiEnemySearchManager::update()
{
	int count = std::min(static_cast<int>(s_enemySearchQueue.size()), ConfigServerGame::getSpaceAiEnemySearchesPerFrame());

	for (int i = 0; i < count; ++i)
	{
		ShipObject * const ship = s_enemySearchQueue.front().getPointer();
		if (ship)
			checkForEnemies(*ship);
		s_enemySearchQueue.pop();
	}
}

// ----------------------------------------------------------------------

void ShipAiEnemySearchManager::add(ShipObject &ship)
{
	s_enemySearchQueue.push(Watcher<ShipObject>(&ship));
}

// ----------------------------------------------------------------------

void ShipAiEnemySearchManagerNamespace::checkForEnemies(ShipObject &ship)
{
	ShipController * const shipController = NON_NULL(ship.getController()->asShipController());

	if (shipController)
	{
		AiShipController * const aiShipController = (shipController != nullptr) ? shipController->asAiShipController() : nullptr;
		float const aggroRadiusSquared = sqr((aiShipController != nullptr) ? aiShipController->getAggroRadius() : 512.0f); // 512.0f is the distance at which turrets on player ships start getting targets
		Vector const shipPosition_w = ship.getPosition_w();
		
		static std::vector<ServerObject *> s_visibilityList;
		static std::vector<ShipObject *> s_enemyList;

		// Get visible objects
		SpaceVisibilityManager::getObjectsVisibleFromLocation(ship.getPosition_w(), s_visibilityList);

		{
			// Filter visible objects for enemies
			for (std::vector<ServerObject *>::const_iterator i = s_visibilityList.begin(); i != s_visibilityList.end(); ++i)
			{
				ServerObject * const visibleServerObject = (*i);
				ShipObject * const visibleShipObject = visibleServerObject->asShipObject();

				if (visibleShipObject)
				{
					if (   (visibleShipObject->getPosition_w().magnitudeBetweenSquared(shipPosition_w) < aggroRadiusSquared)
						&& !visibleShipObject->isAutoAggroImmune())
					{
						if (Pvp::isEnemy(ship, *visibleShipObject))
						{
							if (aiShipController->isValidTarget(*visibleShipObject))
							{
								s_enemyList.push_back(visibleShipObject);
							}
						}
					}
				}
			}
		}
		s_visibilityList.clear();

		// Randomly pick an enemy and attack
		if (!s_enemyList.empty())
		{
			if (ship.isCapitalShip())
			{
				// Capital ship -- put all enemies into the list (because it can attack multiple enemies using turrets)
				float const damage = 1.0f;
				for (std::vector<ShipObject *>::const_iterator i=s_enemyList.begin(); i!=s_enemyList.end(); ++i)
				{
					IGNORE_RETURN(shipController->addDamageTaken((*i)->getNetworkId(), damage, false));

					LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipAiEnemySearchManagerNamespace::checkForEnemies() ENEMY DETECTED: enemy(%s) totalEnemies(%u)", (*i)->getNetworkId().getValueString().c_str(), s_enemyList.size()));
				}
			}	
			else
			{
				// Fighter -- pick a random enemy
				unsigned int const randomIndex = static_cast<unsigned int>(rand() % static_cast<int>(s_enemyList.size()));
				float const damage = 1.0f;
				ShipObject * const enemy = s_enemyList[randomIndex];
				ShipController * const enemyShipController = enemy->getController()->asShipController();

				if (enemyShipController != nullptr)
				{
					// Limit the number of ships that can attack a single enemy

					int const attackingCount = enemyShipController->getNumberOfAiUnitsAttackingMe();

					if (attackingCount < s_maxNumberOfAttackingShips)
					{
						IGNORE_RETURN(aiShipController->getAttackSquad().addDamageTaken(enemy->getNetworkId(), damage));

						LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipAiEnemySearchManagerNamespace::checkForEnemies() ENEMY DETECTED: enemy(%s) totalEnemies(%u) numberAttackTheEnemy(%d)", enemy->getNetworkId().getValueString().c_str(), s_enemyList.size(), attackingCount));
					}
				}
			}
		}
		s_enemyList.clear();

		shipController->clearEnemyCheckQueuedFlag();
	}
}

// ======================================================================

