//======================================================================
//
// MoveSimManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/MoveSimManager.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/MoveSimController.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/UnloadObjectMessage.h"
#include "sharedFoundation/FloatMath.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedTerrain/TerrainObject.h"


#include <vector>

//----------------------------------------------------------------------

namespace MoveSimManagerNamespace
{
	typedef std::vector<NetworkId> NetworkIdVector;
	NetworkIdVector s_moveSimCreatures;

	const char * const templateName_npc = "object/mobile/kaadu_hue.iff";
	const char * const templateName_pc  = "object/creature/player/bothan_female.iff";

	namespace ObjVars
	{
		const std::string moveSim        = "moveSim";
		const std::string moveSimPlayer  = "moveSimPlayer";
		const std::string moveSimPosX    = "moveSimPosX";
		const std::string moveSimPosZ    = "moveSimPosZ";
		const std::string moveSimRadius  = "moveSimRadius";
	}

	void setCreatureObjVars (CreatureObject & creature, const Vector & pos_w, float radius)
	{
		const bool isPlayer = creature.isPlayerControlled ();

		creature.setObjVarItem (ObjVars::moveSim,       int(1));
		creature.setObjVarItem (ObjVars::moveSimPlayer, int(isPlayer ? 1 : 0));
		creature.setObjVarItem (ObjVars::moveSimPosX,   pos_w.x);
		creature.setObjVarItem (ObjVars::moveSimPosZ,   pos_w.z);
		creature.setObjVarItem (ObjVars::moveSimRadius, radius);
	}
}

using namespace MoveSimManagerNamespace;

//----------------------------------------------------------------------

/**
*
* @param centerPoint the point to wander around
* @param roamRadius the radius around the point to wander around, limited by the edges of the map
*
*/

void MoveSimManager::start (const NetworkId & userId, int numNpcToSpawn, int numPcToSpawn, float radius, float moveSpeed)
{
	if (!ConfigServerGame::getMoveSimEnabled ())
		return;

	const ServerObject * const player = safe_cast<ServerObject *>(NetworkIdManager::getObjectById (userId));

	if (!player)
		return;

	float halfMapWidth = 1000.0f;
	TerrainObject * const terrainObject = TerrainObject::getInstance ();	
	if (terrainObject)
	{
		//- subtract a 512 meter invalid band around the edge of the map
		halfMapWidth = (terrainObject->getMapWidthInMeters () * 0.5f) - 512.0f;
	}

	const Vector & player_pos_w = player->getPosition_w ();

	const int numToSpawn = numPcToSpawn + numNpcToSpawn;

	const float randomRadiusArea = std::min (halfMapWidth * 2.0f, radius * 0.7f);
	for (int i = 0; i < numToSpawn; ++i)
	{
		Vector pos = player_pos_w;
		pos.x += Random::randomReal (-randomRadiusArea, randomRadiusArea);
		pos.z += Random::randomReal (-randomRadiusArea, randomRadiusArea);

		pos.x = std::min (halfMapWidth, std::max (-halfMapWidth, pos.x));
		pos.z = std::min (halfMapWidth, std::max (-halfMapWidth, pos.z));

		float angle = Random::randomReal (0.0f, PI * 2.0f);

		Transform tr;
		tr.setPosition_p(pos);
		tr.yaw_l(angle);

		CreatureObject * creature = 0;
		
		if (numPcToSpawn-- > 0)
		{
			creature = dynamic_cast<CreatureObject *>(ServerWorld::createNewObject(templateName_pc, tr, 0, false));
			creature->setPlayerControlled (true);
			ServerWorld::createNewObject (ConfigServerGame::getPlayerObjectTemplate (), *creature, false);
		}
		else
		{
			creature = dynamic_cast<CreatureObject *>(ServerWorld::createNewObject(templateName_npc, tr, 0, false));
		}

		if (!creature)
			return;

		creature->addToWorld ();

		const float naturalRunSpeed = safe_cast<const SharedCreatureObjectTemplate *>(creature->getSharedTemplate())->getSpeed(SharedCreatureObjectTemplate::MT_run);
		const float moveScale = moveSpeed / naturalRunSpeed;

		creature->setMovementScale (moveScale);

		MoveSimController * const msc = new MoveSimController (creature, player_pos_w, radius);
		creature->setController (msc);

		creature->scheduleForAlter ();

		setCreatureObjVars (*creature, player_pos_w, radius);

		s_moveSimCreatures.push_back (creature->getNetworkId ());
	}
}

//----------------------------------------------------------------------

void MoveSimManager::clear ()
{
	for (NetworkIdVector::const_iterator it = s_moveSimCreatures.begin (); it != s_moveSimCreatures.end (); ++it)
	{
		const NetworkId & id = *it;
		ServerObject * const obj = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById (id));
		if (obj && obj->isAuthoritative ())
			obj->unload ();
		else
		{
			ServerMessageForwarding::beginBroadcast();

			UnloadObjectMessage const unloadMessage(id);
			ServerMessageForwarding::send(unloadMessage);

			ServerMessageForwarding::end();
		}
	}

	s_moveSimCreatures.clear ();
}

//----------------------------------------------------------------------

void MoveSimManager::checkApplySimulation (CreatureObject & creature)
{
	if (!ConfigServerGame::getMoveSimEnabled ())
		return;

	int isMoveSim = 0;
	if (creature.getObjVars ().getItem (ObjVars::moveSim, isMoveSim) && isMoveSim != 0)
	{
		const NetworkId & id = creature.getNetworkId ();
		int isPlayer = 0;
		if (creature.getObjVars ().getItem (ObjVars::moveSimPlayer, isPlayer))
			creature.setPlayerControlled (isPlayer != 0);
		
		Vector pos_w;
		if (!creature.getObjVars ().getItem (ObjVars::moveSimPosX, pos_w.x))
			WARNING (true, ("MoveSimManager no x from objvar for [%s]", id.getValueString ().c_str ()));
		
		if (!creature.getObjVars ().getItem (ObjVars::moveSimPosZ, pos_w.z))
			WARNING (true, ("MoveSimManager no z from objvar for [%s]", id.getValueString ().c_str ()));
		
		float radius = 0.0f;
		if (!creature.getObjVars ().getItem (ObjVars::moveSimRadius, radius))
			WARNING (true, ("MoveSimManager no radius from objvar for [%s]", id.getValueString ().c_str ()));
		
		MoveSimController * const msc = new MoveSimController (&creature, pos_w, radius);
		creature.setController (msc);
		creature.scheduleForAlter ();
		
		if (std::find (s_moveSimCreatures.begin (), s_moveSimCreatures.end (), id) != s_moveSimCreatures.end())
			s_moveSimCreatures.push_back (id);

		REPORT_LOG_PRINT (true, ("MoveSimManager [%ld] applied simulation to [%s]\n", GameServer::getInstance ().getProcessId (), id.getValueString ().c_str ()));
	}
}

//----------------------------------------------------------------------
