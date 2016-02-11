// ======================================================================
//
// AiMovementArchive.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"

#include "serverGame/AiMovementFace.h"
#include "serverGame/AiMovementFlee.h"
#include "serverGame/AiMovementFollow.h"
#include "serverGame/AiLocation.h"
#include "serverGame/AiMovementLoiter.h"
#include "serverGame/AiMovementMove.h"
#include "serverGame/AiMovementSwarm.h"
#include "serverGame/AiMovementWander.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverNetworkMessages/AiMovementMessage.h"
#include "sharedObject/NetworkIdManager.h"

//-----------------------------------------------------------------------

namespace Archive
{
	// the archive functions for AiMovementMessage are here so we don't have to link 
	// serverGame with everything
	void get(ReadIterator & source, AiMovementMessage & target)
	{
		int movementType;
		get(source, target.m_objectId);
		get(source, movementType);

		AICreatureController * controller = nullptr;
		Object * object = NetworkIdManager::getObjectById(target.getObjectId());
		if (object != nullptr)
			controller = dynamic_cast<AICreatureController *>(object->getController());

		if (controller == nullptr)
		{
			WARNING(true, ("AiMovementMessage Archive::get, got message to object %s that doesn't have an AICreatureController", target.getObjectId().getValueString().c_str()));
			target.m_movement = AiMovementBaseNullPtr;
			return;
		}

		switch (static_cast<AiMovementType>(movementType))
		{
			case AMT_loiter:
				target.m_movement = AiMovementBasePtr(new AiMovementLoiter(controller, source));
				break;
			case AMT_wander:
				target.m_movement = AiMovementBasePtr(new AiMovementWander(controller, source));
				break;
			case AMT_follow:
				target.m_movement = AiMovementBasePtr(new AiMovementFollow(controller, source));
				break;
			case AMT_flee:
				target.m_movement = AiMovementBasePtr(new AiMovementFlee(controller, source));
				break;
			case AMT_move:
				target.m_movement = AiMovementBasePtr(new AiMovementMove(controller, source));
				break;
			case AMT_face:
				target.m_movement = AiMovementBasePtr(new AiMovementFace(controller, source));
				break;
			case AMT_swarm:
				target.m_movement = AiMovementBasePtr(new AiMovementSwarm(controller, source));
				break;
			case AMT_idle:
			case AMT_invalid:
			default:
				target.m_movement = AiMovementBaseNullPtr;
				break;
		}
	}

	void put(ByteStream & target, const AiMovementMessage & source)
	{
		put(target, source.getObjectId());
		put(target, source.getMovementType());

		switch (source.getMovementType())
		{
			case AMT_loiter:
			case AMT_wander:
			case AMT_follow:
			case AMT_flee:
			case AMT_move:
			case AMT_face:
			case AMT_swarm:
				if (source.getMovement() != AiMovementBaseNullPtr)
					source.getMovement()->pack(target);
				else
				{
					FATAL(true, ("AiMovementArchive::put AiMovementMessage: got type %d with no data", static_cast<int>(source.getMovementType())));
				}
				break;
			case AMT_idle:
			case AMT_invalid:
			default:
				break;
		}
	}

}

//=======================================================================

