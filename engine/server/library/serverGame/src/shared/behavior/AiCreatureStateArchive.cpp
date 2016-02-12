// ======================================================================
//
// AiCreatureStateArchive.cpp
// copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiCreatureStateArchive.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverNetworkMessages/AiCreatureStateMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/NetworkIdManager.h"

//-----------------------------------------------------------------------

namespace Archive
{
	// The archive functions for AiCreatureStateMessage are here so we don't have to link serverGame with everything

	void get(ReadIterator & source, AiCreatureStateMessage & target)
	{
		get(source, target.m_networkId);
		get(source, target.m_movement);

#ifdef _DEBUG
		Object * const object = NetworkIdManager::getObjectById(target.m_networkId);

		if (object == nullptr)
		{
			WARNING(true, ("AiCreatureStateArchive::get() Unable to resolve networkId(%s) to an Object", target.m_networkId.getValueString().c_str()));
			return;
		}

		AICreatureController * const controller = (object != nullptr) ? AICreatureController::asAiCreatureController(object->getController()) : nullptr;

		if (controller == nullptr)
		{
			WARNING(true, ("AiCreatureStateArchive::get() Message to object(%s) that does not have an AICreatureController", object->getDebugInformation().c_str()));
			return;
		}
#endif // _DEBUG
	}

	void put(ByteStream & target, AiCreatureStateMessage const & source)
	{
		put(target, source.m_networkId);
		put(target, source.m_movement);
	}
}

//=======================================================================
