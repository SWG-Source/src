//======================================================================
//
// NebulaManagerServer.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/NebulaManagerServer.h"

#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceVisibilityManager.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/SpatialDatabase.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/Nebula.h"
#include "sharedGame/NebulaManager.h"
#include "sharedMath/MultiShape.h"
#include "sharedMath/VectorArgb.h"
#include "sharedNetworkMessages/CreateNebulaLightningMessage.h"
#include "sharedNetworkMessages/EnvironmentalHitData.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/NebulaLightningData.h"
#include "sharedNetworkMessages/NebulaLightningHitData.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Controller.h"
#include "sharedRandom/Random.h"

#include <map>
#include <vector>

//======================================================================

namespace NebulaManagerServerNamespace
{

	typedef std::vector<NebulaLightningData> LightningDataVector;

	LightningDataVector s_lightningDataVector;

	typedef std::map<CachedNetworkId, uint32> NetworkIdTimeMap;
	NetworkIdTimeMap s_objectsRecentlyHitByLightning;

	static uint16 s_lastLightningId = 0;
}

using namespace NebulaManagerServerNamespace;

//----------------------------------------------------------------------

void NebulaManagerServer::loadScene(std::string const & sceneId)
{
	NebulaManager::setClearFunction(&NebulaManagerServer::clear);
	NebulaManager::loadScene(sceneId);

	s_objectsRecentlyHitByLightning.clear();
}

//----------------------------------------------------------------------

void NebulaManagerServer::clear()
{
	s_lightningDataVector.clear();
}

//----------------------------------------------------------------------

void NebulaManagerServer::update(float elapsedTime)
{
	UNREF(elapsedTime);
	
	handleEnqueuedLightningEvents();

	generateLightningEvents(elapsedTime);
}

//----------------------------------------------------------------------

void NebulaManagerServer::enqueueLightning(NebulaLightningData const & nebulaLightningData)
{
	Nebula const * const nebula = NebulaManager::getNebulaById(nebulaLightningData.nebulaId);
	if (nebula == nullptr)
	{
		WARNING(true, ("NebulaManagerServer::enqueueLightning invalid nebula [%d]", nebulaLightningData.nebulaId));
		return;
	}
	
	std::string const & lightningAppearance = nebula->getLightningAppearance();
	
	if (lightningAppearance.empty())
	{
		WARNING(true, ("NebulaManagerServer::enqueueLightning nebula [%d] has no lightning appearance", nebulaLightningData.nebulaId));
		return;
	}
	
	s_lightningDataVector.push_back(nebulaLightningData);

	//-- send enqueue messages to nearby clients
	// time is in the context of the connection server's sync stamp, so we need a unique message per connection server

	static float const s_defaultLightningObservationRange = 128.0f;

	typedef std::vector<NetworkId> NetworkIdVector;
	typedef std::map<ConnectionServerConnection *, NetworkIdVector> DistributionList;
	DistributionList distributionList;

	static std::set<Client *> observingClients;

	SpaceVisibilityManager::getClientsInRange(
		(nebulaLightningData.endpoint0+nebulaLightningData.endpoint1)*0.5f,
		nebulaLightningData.endpoint0.magnitudeBetween(nebulaLightningData.endpoint1)+s_defaultLightningObservationRange,
		observingClients);

	//-- Build distribution list of clients in range of the lightning
	for (std::set<Client *>::const_iterator i = observingClients.begin(); i != observingClients.end(); ++i)
		distributionList[(*i)->getConnection()].push_back((*i)->getCharacterObjectId());

	observingClients.clear();

	uint32 const clockTimeMs = Clock::timeMs();
	uint32 const deltaTimeStart = nebulaLightningData.syncStampStart - clockTimeMs;
	uint32 const deltaTimeEnd = nebulaLightningData.syncStampEnd - clockTimeMs;

	//-- Send CreateNebulaLightningMessage to distribution list
	{
		NebulaLightningData nebulaLightningDataToSend = nebulaLightningData;
		for (DistributionList::const_iterator iter = distributionList.begin(); iter != distributionList.end(); ++iter)
		{
			ConnectionServerConnection * const connectionServerConnection = (*iter).first;
			NetworkIdVector const & targetNetworkIds = (*iter).second;

			nebulaLightningDataToSend.syncStampStart = connectionServerConnection->getSyncStampLong() + deltaTimeStart;
			nebulaLightningDataToSend.syncStampEnd = connectionServerConnection->getSyncStampLong() + deltaTimeEnd;

			CreateNebulaLightningMessage const createNebulaLightningMessage(nebulaLightningDataToSend);
			GameClientMessage const gameClientMessage(targetNetworkIds, true, createNebulaLightningMessage);
			connectionServerConnection->send(gameClientMessage, true);
		}
	}
}

//----------------------------------------------------------------------

/*
* handle enqueued lightning events
*/

void NebulaManagerServer::handleEnqueuedLightningEvents()
{
	uint32 const clockTimeMs = Clock::timeMs();
	
	for (LightningDataVector::iterator it = s_lightningDataVector.begin(); it != s_lightningDataVector.end();)
	{
		NebulaLightningData const & nebulaLightningData = (*it);
		
		Nebula const * const nebula = NebulaManager::getNebulaById(nebulaLightningData.nebulaId);
		if (nebula == nullptr)
		{
			it = s_lightningDataVector.erase(it);
			continue;
		}
				
		//-- lightning occurs now or in the past
		if (nebulaLightningData.syncStampStart <= clockTimeMs)
		{
			//-- lightning is finished
			if (nebulaLightningData.syncStampEnd < clockTimeMs)
			{
				it = s_lightningDataVector.erase(it);
				continue;
			}
			
			Vector startPosition_w = nebulaLightningData.endpoint0;
			Vector endPosition_w = nebulaLightningData.endpoint1;			
						
			Vector unitLightningRay = (endPosition_w - startPosition_w);
			if (!unitLightningRay.normalize())
			{
				++it;
				continue;
			}

			typedef std::vector<Object *> ObjectVector;
			
			static ObjectVector collidingObjects;
			collidingObjects.clear();
			
			if (CollisionWorld::getDatabase()->queryObjects(CellProperty::getWorldCellProperty(), MultiShape(Sphere(startPosition_w, 0.1f)), endPosition_w - startPosition_w, &collidingObjects, &collidingObjects))
			{
				for (ObjectVector::const_iterator cit = collidingObjects.begin(); cit != collidingObjects.end(); ++cit)
				{
					Object * const hitObject = NON_NULL(*cit);
					ServerObject const * const serverHitObject = hitObject->asServerObject();
					ShipObject const * const shipHitObject = serverHitObject->asShipObject();
					
					if (shipHitObject != nullptr)
					{
						NetworkIdTimeMap::const_iterator const oit = s_objectsRecentlyHitByLightning.find(CachedNetworkId(*hitObject));
						if (oit == s_objectsRecentlyHitByLightning.end() || (*oit).second < clockTimeMs)
						{
							Vector const & closestPoint_w = hitObject->getPosition_w().findClosestPointOnLine(startPosition_w, endPosition_w);
							Vector const & closestPoint_o = hitObject->rotateTranslate_w2o(closestPoint_w);
							float const dotProduct = Vector::unitZ.dot(closestPoint_o);
							int const side = dotProduct > 0.0f ? 0 : 1;
							float const damage = Random::randomReal(nebula->getLightningDamageMin(), nebula->getLightningDamageMax());
							
							GameScriptObject * const gso = const_cast<GameScriptObject *>(serverHitObject->getScriptObject());
							ScriptParams params;
							params.addParam(side);
							params.addParam(damage);
							
							IGNORE_RETURN(gso->trigAllScripts(Scripting::TRIG_SHIP_HIT_BY_LIGHTNING, params));
							
							MessageQueue::Data * const data = new MessageQueueGenericValueType<NebulaLightningHitData>(NebulaLightningHitData(nebulaLightningData.lightningId, static_cast<int8>(side), damage));
							hitObject->getController()->appendMessage(CM_lightningHitShip, 0.0f, data, 	
								GameControllerMessageFlags::SEND | 
								GameControllerMessageFlags::RELIABLE | 
								GameControllerMessageFlags::DEST_ALL_CLIENT);
							
							//-- allow at most 1 hit per object per 2 seconds
							s_objectsRecentlyHitByLightning[CachedNetworkId(*hitObject)] = clockTimeMs + 2000;
						}
					}
				}
			}
		}
		
		++it;
	}
}

//----------------------------------------------------------------------

void NebulaManagerServer::generateLightningEvents(float elapsedTime)
{
	uint32 const clockTimeMs = Clock::timeMs();

	NebulaManager::NebulaVector const & nebulaVector = NebulaManager::getNebulaVector();
	for (NebulaManager::NebulaVector::const_iterator it = nebulaVector.begin(); it != nebulaVector.end(); ++it)
	{
		Nebula const * const nebula = NON_NULL(*it);
		
		std::string const & lightningAppearance = nebula->getLightningAppearance();
		
		if (lightningAppearance.empty())
			continue;

		float const lightningFrequency = nebula->getLightningFrequency();
			
		if (lightningFrequency <= 0.0f)
			continue;
		
		float const density = nebula->getDensity();
		if (density <= 0.0f)
			continue;

		float const averageLightningCountThisFrame = std::min(0.5f, elapsedTime * lightningFrequency);
		
		if (Random::randomReal(0.0f, 1.0f) < averageLightningCountThisFrame)
		{			
			float const radius = nebula->getSphere().getRadius() * 0.3f;
			Vector const & center = nebula->getSphere().getCenter();
			
			NebulaLightningData nebulaLightningData;
			
			//-- zero is the invalid lightning id
			if (++s_lastLightningId == 0)
				++s_lastLightningId;
			
			float const lightningDurationMax = nebula->getLightningDurationMax();
			int const lightningDurationMaxMs = static_cast<int>(lightningDurationMax * 1000.0f);

			nebulaLightningData.lightningId = s_lastLightningId;
			nebulaLightningData.syncStampStart = clockTimeMs + 1000;
			nebulaLightningData.syncStampEnd = nebulaLightningData.syncStampStart + static_cast<uint32>(Random::random(lightningDurationMaxMs / 2, lightningDurationMaxMs));

			nebulaLightningData.nebulaId = nebula->getId();
			
			nebulaLightningData.endpoint0.x = Random::randomReal(center.x - radius, center.x + radius);
			nebulaLightningData.endpoint0.y = Random::randomReal(center.y - radius, center.y + radius);
			nebulaLightningData.endpoint0.z = Random::randomReal(center.z - radius, center.z + radius);
			
			nebulaLightningData.endpoint1.x = Random::randomReal(center.x - radius, center.x + radius);
			nebulaLightningData.endpoint1.y = Random::randomReal(center.y - radius, center.y + radius);
			nebulaLightningData.endpoint1.z = Random::randomReal(center.z - radius, center.z + radius);

			NebulaManagerServer::enqueueLightning(nebulaLightningData);
		}
	}
}

//----------------------------------------------------------------------

void NebulaManagerServer::handleEnvironmentalDamage(ServerObject & victim, int nebulaId)
{
	Nebula const * const nebula = NebulaManager::getNebulaById(nebulaId);
	if (nebula == nullptr)
		return;
	
	float const environmentalDamageFrequency = nebula->getEnvironmentalDamageFrequency();
	
	if (environmentalDamageFrequency <= 0.0f)
		return;
	
	const float lastFrameTime = Clock::frameTime();
	float const averageRandomDamageCountThisFrame = std::min(0.5f, lastFrameTime * environmentalDamageFrequency);
	
	if (Random::randomReal(0.0f, 1.0f) < averageRandomDamageCountThisFrame)
	{			
		float const damage = nebula->getEnvironmentalDamage();
		int const side = Random::random(0,1);
		
		GameScriptObject * const gso = const_cast<GameScriptObject *>(victim.getScriptObject());
		ScriptParams params;
		params.addParam(side);
		params.addParam(damage);
		
		IGNORE_RETURN(gso->trigAllScripts(Scripting::TRIG_SHIP_HIT_BY_ENVIRONMENT, params));
		
		MessageQueue::Data * const data = new MessageQueueGenericValueType<EnvironmentalHitData>(EnvironmentalHitData(nebula->getId(), static_cast<int8>(side), damage));
		victim.getController()->appendMessage(CM_environmentHitShip, 0.0f, data, 	
			GameControllerMessageFlags::SEND | 
			GameControllerMessageFlags::RELIABLE | 
			GameControllerMessageFlags::DEST_ALL_CLIENT);
	}
}

//======================================================================
