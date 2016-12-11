// ======================================================================
//
// ContainerInterface.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ContainerInterface.h"

#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverGame/VeteranRewardManager.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/SlotIdArchive.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedObject/VolumeContainmentProperty.h"

// ======================================================================

namespace ContainerInterfaceNamespace
{

	// ----------------------------------------------------------------------

	bool findContainingBankPlayer(ServerObject const &item, CreatureObject const *&player)
	{
		ServerObject const *lastContainer = &item;

		while (lastContainer)
		{
			//-- are we contained?
			Object const * const o = ContainerInterface::getContainedByObject(*lastContainer);
			ServerObject const * const containedBy = o ? o->asServerObject() : 0;
			if (!containedBy)
				return false;

			//-- are we contained by a creature?
			CreatureObject const * const creature = containedBy->asCreatureObject();

			if (creature)
			{
				if (!creature->isPlayerControlled())
					return false;

				if (creature->getBank() == lastContainer)
				{
					//-- if player is passed in non-nullptr, the found creature must match it
					if (player && player != creature)
						return false;

					player = creature;
					return true;
				}
			}

			lastContainer = containedBy;
		}

		return false;
	}

	// ----------------------------------------------------------------------

	void logBankSubContentsMoved(CreatureObject const &player, ServerObject &item, bool added)
	{
		Container const * const container = ContainerInterface::getContainer(item);
		if (!container)
			return;

		for (ContainerConstIterator it = container->begin(); it != container->end(); ++it)
		{
			CachedNetworkId const &id = *it;
			ServerObject * const obj = safe_cast<ServerObject *>(id.getObject());
			if (obj)
			{
				if (added)
				{
					LOG("CustomerService", ("Bank:%s has added %s to their bank, contained in [%s]",
						PlayerObject::getAccountDescription(&player).c_str(),
						ServerObject::getLogDescription(obj).c_str(),
						item.getNetworkId().getValueString().c_str()));
				}
				else
				{
					LOG("CustomerService", ("Bank:%s has removed %s from their bank, contained in [%s]",
						PlayerObject::getAccountDescription(&player).c_str(),
						ServerObject::getLogDescription(obj).c_str(),
						item.getNetworkId().getValueString().c_str()));
				}

				logBankSubContentsMoved(player, *obj, added);
			}
		}
	}

	// ----------------------------------------------------------------------

	bool checkSlottedContainerRemove(ServerObject &item, ServerObject const *transferer, Container::ContainerErrorCode &error)
	{
		error = Container::CEC_Success;

		if (transferer)
		{
			// Check each slot to make sure it can be manipulated.
			SlottedContainmentProperty * const slottedProp = ContainerInterface::getSlottedContainmentProperty(item);
			if (!slottedProp)
			{
				WARNING_STRICT_FATAL(true, ("Invaid item (no slottedContainmentProperty) in transferItemToSlottedContainer"));
				error = Container::CEC_WrongType;
				return false;
			}

			if (!slottedProp->canManipulateArrangement(slottedProp->getCurrentArrangement()))
			{
				error = Container::CEC_WrongType;
				return false;
			}
		}
		return true;
	}

	// ----------------------------------------------------------------------

	bool checkTransferScripts(ServerObject *destination, ServerObject &item, ServerObject *transferer, Container::ContainerErrorCode &error)
	{
		PROFILER_AUTO_BLOCK_DEFINE("checkTransferScripts");

		error = Container::CEC_Success;

		NetworkId const &transfererId = transferer ? transferer->getNetworkId() : NetworkId::cms_invalid;
		ServerObject * const source = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(item));
		NetworkId const &sourceId = source ? source->getNetworkId() : NetworkId::cms_invalid;
		NetworkId const &destId = destination ? destination->getNetworkId() : NetworkId::cms_invalid;

		if (source)
		{
			if (source->getScriptObject())
			{
				ScriptParams params;
				params.addParam(destId);
				params.addParam(transfererId);
				params.addParam(item.getNetworkId());

				//-- Handle source container's scripts.
				if (source->getScriptObject())
				{
					if (source->getScriptObject()->trigAllScripts(Scripting::TRIG_ABOUT_TO_LOSE_ITEM, params) == SCRIPT_OVERRIDE)
					{
						error = Container::CEC_BlockedByScript;
						LOG("ScriptInvestigation", ("Source tried to prevent container transfer"));
						return false;
					}
				}
			}

			if (!source->onContainerAboutToLoseItem(destination, item, transferer))
			{
				//-- Source container prevented the container from losing the item.
				error = Container::CEC_BlockedByScript;
				return false;
			}
			else
			{
				if (ContainerInterface::getSlottedContainer(*source))
				{
					if (!checkSlottedContainerRemove(item, transferer, error))
					{
						error = Container::CEC_BlockedBySourceContainer;
						return false;
					}
				}
			}
		}

		if (destination)
		{
			if (transferer)
			{
				// if the item contains any no trade items, the item
				// and destination must be owned by the transferer
				if (item.markedNoTradeRecursive(false, true))
				{
					// the item and destination must be owned by the transferer
					if (   destination->getOwnerId() != transferer->getNetworkId()
						&& (   !transferer->getClient()
						|| !transferer->getClient()->isGod()))
					{
						error = Container::CEC_NoPermission;
						return false;
					}

					if (   item.getOwnerId() != transferer->getNetworkId()
						&& (   !transferer->getClient()
						|| !transferer->getClient()->isGod()))
					{
						error = Container::CEC_NoPermission;
						return false;
					}
				}
				// item doesn't contain any no trade items, but the item itself is no trade
				else if (!item.isPlayerControlled() && item.markedNoTrade())
				{
					// item is no trade, the destination must be owned by the transferer
					if (   destination->getOwnerId() != transferer->getNetworkId()
						&& (   !transferer->getClient()
						|| !transferer->getClient()->isGod()))
					{
						error = Container::CEC_NoPermission;
						return false;
					}

					if (item.markedNoTradeShared(true))
					{
						// if the no trade item is a "shared" no trade item and is immediately
						// contained in a structure cell, and is being transferred to the
						// transferer's inventory (i.e. picked up), also allow the transfer if
						// the transferer is on the same account as the current owner of the item
						bool blockedByNoTrade = false;
						if (item.getOwnerId() != transferer->getNetworkId())
						{
							blockedByNoTrade = true;

							ServerObject const * const containedBy = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(item));
							if (containedBy && containedBy->asCellObject())
							{
								CreatureObject const * const transfererCreatureObject = transferer->asCreatureObject();
								if (transfererCreatureObject && (transfererCreatureObject->getInventory() == destination))
								{
									PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(transfererCreatureObject);
									if (playerObject && (playerObject->getStationId() == NameManager::getInstance().getPlayerStationId(item.getOwnerId())))
										blockedByNoTrade = false;
								}
							}
						}

						if (   blockedByNoTrade
							&& (   !transferer->getClient()
							|| !transferer->getClient()->isGod()))
						{
							error = Container::CEC_NoPermission;
							return false;
						}
					}
					else
					{
						// just regular no trade item, so the item must be owned by the transferer
						if (   item.getOwnerId() != transferer->getNetworkId()
							&& (   !transferer->getClient()
							|| !transferer->getClient()->isGod()))
						{
							error = Container::CEC_NoPermission;
							return false;
						}
					}
				}
			}

			if (destination->getScriptObject())
			{
				ScriptParams params;
				params.addParam(sourceId);
				params.addParam(transfererId);
				params.addParam(item.getNetworkId());

				//-- Call scripts on destination container owner.
				if (destination->getScriptObject())
				{
					if (destination->getScriptObject()->trigAllScripts(Scripting::TRIG_ABOUT_TO_RECEIVE_ITEM, params) == SCRIPT_OVERRIDE)
					{
						LOG("ScriptInvestigation", ("Destination tried to prevent container transfer"));
						error = Container::CEC_BlockedByScript;
						return false;
					}
				}
			}

			Container::ContainerErrorCode const destinationResult = static_cast<Container::ContainerErrorCode>(destination->onContainerAboutToGainItem(item, transferer));
			if (destinationResult != Container::CEC_Success)
			{
				error = destinationResult;
				return false;
			}
		}

		if (item.getScriptObject())
		{
			ScriptParams params;
			params.addParam(destId);
			params.addParam(transfererId);

			if (item.getScriptObject()->trigAllScripts(Scripting::TRIG_ABOUT_TO_BE_XFERRED, params) == SCRIPT_OVERRIDE)
			{
				LOG("ScriptInvestigation", ("Item tried to prevent container transfer"));
				error = Container::CEC_BlockedByScript;
				return false;
			}
		}

		if (!item.onContainerAboutToTransfer(destination, transferer))
		{
			error = Container::CEC_BlockedByItemBeingTransferred;
			return false;
		}

		return true;
	}

	// ----------------------------------------------------------------------

	bool sharedTransferBegin(ServerObject &item, ServerObject *&sourceObject, Container *&sourceContainer, Container::ContainerErrorCode &error)
	{
		PROFILER_AUTO_BLOCK_DEFINE("sharedTransferBegin");
		error = Container::CEC_Success;
		ContainedByProperty * const containedBy = ContainerInterface::getContainedByProperty(item);
		if (!containedBy || !containedBy->getContainedBy())
		{
			// This item is not contained by anything!
			// This means it is in the world!
			// Return nullptr for the source container, but succeed.
			sourceContainer = nullptr;
			return true;
		}

		ServerObject * const source = containedBy->getContainedBy()->asServerObject();
		sourceObject = source;
		if (!source)
		{
			WARNING_STRICT_FATAL(true, ("This item's source was not found!"));
			error = Container::CEC_NotFound;
			return false;
		}

		sourceContainer = ContainerInterface::getContainer(*source);
		if (!sourceContainer)
		{
			error = Container::CEC_NoContainer;
			WARNING_STRICT_FATAL(true, ("This source contains stuff, but has no property!!"));
			return false;
		}

		return true;
	}

	// -----------------------------------------------------------------------

	bool handleTransferSource(Container *source, ServerObject &item, Container::ContainerErrorCode &error)
	{
		PROFILER_AUTO_BLOCK_DEFINE("handleTransferSource");
		error = Container::CEC_Success;
		return !source || source->remove(item, error);
	}

	// -----------------------------------------------------------------------

	void handleTransferScripts(ServerObject &item, ServerObject *sourceObject, ServerObject *destination, ServerObject *transferer, Container::ContainerErrorCode &error)
	{
		error = Container::CEC_Success;
		NetworkId const transfererId = transferer ? transferer->getNetworkId() : NetworkId::cms_invalid;
		NetworkId const sourceId = sourceObject ? sourceObject->getNetworkId() : NetworkId::cms_invalid;
		NetworkId const destinationId = destination ? destination->getNetworkId() : NetworkId::cms_invalid;

		if (sourceObject)
		{
			if (sourceObject->getScriptObject())
			{
				ScriptParams params;
				params.addParam(destinationId);
				params.addParam(transfererId);
				params.addParam(item.getNetworkId());

				//-- Run scripts on source's container.
				bool const sourceContinue = (!sourceObject->getScriptObject() || sourceObject->getScriptObject()->trigAllScripts(Scripting::TRIG_LOST_ITEM, params) == SCRIPT_CONTINUE);
				if (sourceContinue)
					sourceObject->onContainerLostItem(destination, item, transferer);
			}

			//-- player is transfering the object out of the bank or any of its nested containers
			CreatureObject const *player = 0;
			if (findContainingBankPlayer(*sourceObject, player))
			{
				if (!destination || !findContainingBankPlayer(*destination, player))
				{
					if (sourceObject == player->getBank())
					{
						LOG("CustomerService", ("Bank:%s has removed %s from their bank",
							PlayerObject::getAccountDescription(player).c_str(),
							ServerObject::getLogDescription(&item).c_str()));
					}
					else
					{
						LOG("CustomerService", ("Bank:%s has removed %s from their bank subcontainer %s",
							PlayerObject::getAccountDescription(player).c_str(),
							ServerObject::getLogDescription(&item).c_str(),
							ServerObject::getLogDescription(sourceObject).c_str()));
					}

					logBankSubContentsMoved(*player, item, false);
				}
			}
		}

		if (destination)
		{
			if (destination->getScriptObject())
			{
				ScriptParams params;
				params.addParam(sourceId);
				params.addParam(transfererId);
				params.addParam(item.getNetworkId());

				//-- Run scripts on destination's container.
				bool const destinationContinue = (!destination->getScriptObject() || destination->getScriptObject()->trigAllScripts(Scripting::TRIG_RECEIVED_ITEM, params) == SCRIPT_CONTINUE);

				if (destinationContinue)
					destination->onContainerGainItem(item, sourceObject, transferer);
			}

			//-- player is transfering the object into the bank or any of its nested containers
			CreatureObject const *player = 0;
			if (findContainingBankPlayer(*destination, player))
			{
				if (!sourceObject || !findContainingBankPlayer(*sourceObject, player))
				{
					if (destination == player->getBank())
					{
						LOG("CustomerService", ("Bank:%s has added %s to their bank",
							PlayerObject::getAccountDescription(player).c_str(),
							ServerObject::getLogDescription(&item).c_str()));
					}
					else
					{
						LOG("CustomerService", ("Bank:%s has added %s to their bank subcontainer %s",
							PlayerObject::getAccountDescription(player).c_str(),
							ServerObject::getLogDescription(&item).c_str(),
							ServerObject::getLogDescription(destination).c_str()));
					}

					logBankSubContentsMoved(*player, item, true);
				}
			}
		}

		if (item.getScriptObject())
		{
			ScriptParams params;
			params.addParam(sourceId);
			params.addParam(destinationId);
			params.addParam(transfererId);

			if (item.getScriptObject()->trigAllScripts(Scripting::TRIG_XFERRED, params) == SCRIPT_CONTINUE)
				item.onContainerTransfer(destination, transferer);
		}
	}

	// ----------------------------------------------------------------------

}

using namespace ContainerInterfaceNamespace;

// ======================================================================

//@todo consider hiding all container functions and friend this class to containers.

bool ContainerInterface::canTransferTo(ServerObject *destination, ServerObject &item, ServerObject *transferer, Container::ContainerErrorCode &error)
{
	PROFILER_AUTO_BLOCK_DEFINE("canTransferTo");

	error = Container::CEC_Success;

	// can't transfer unless both item and destination are authoritative
	if (!item.isAuthoritative() || (destination && !destination->isAuthoritative()))
	{
		// move item and destination to the same server, if possible
		if (destination && (item.isAuthoritative() != destination->isAuthoritative()))
		{
			GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
				"RequestSameServer",
				std::make_pair(
					ContainerInterface::getTopmostContainer(item)->getNetworkId(),
					ContainerInterface::getTopmostContainer(*destination)->getNetworkId()));
			GameServer::getInstance().sendToPlanetServer(rssMessage);
		}
		else
		{
			// we are either trying to move a proxy of the item into the world, or we
			// are trying to move a proxy of the item into a proxy of the destination,
			// which will not be allowed as that will leave things inconsistent on
			// this server and on the server where the item and destination is authoritative
			WARNING(true, ("canTransferTo() not allowing move of proxy of item (%s) into proxy of destination (%s)",
				item.getDebugInformation().c_str(),
				(destination ? destination->getDebugInformation().c_str() : "world")));
		}

		error = Container::CEC_TryAgain;
		return false;
	}

	// Cannot move a tcg card that is in the middle of the redemption process
	if (!VeteranRewardManager::checkForTcgRedemptionInProgress(item))
	{
		error = Container::CEC_BlockedByItemBeingTransferred;
		return false;
	}

	// Cannot move a reward item that is in the middle of the trade in process
	if (!VeteranRewardManager::checkForTradeInInProgress(item))
	{
		error = Container::CEC_BlockedByItemBeingTransferred;
		return false;
	}

	// if the character is currently over the lot limit, and is trying to drop
	// an item into a cell of a structure (*NOT* a POB ship) or the ingredient
	// hopper of a factory, block the drop unless it is being dropped into a
	// cell of the structure that caused the lot limit to be exceeded
	if (transferer && destination)
	{
		CreatureObject const * const transfererCreatureObject = transferer->asCreatureObject();
		if (transfererCreatureObject)
		{
			PlayerObject const * const transfererPlayerObject = PlayerCreatureController::getPlayerObject(transfererCreatureObject);
			if (transfererPlayerObject && (transfererPlayerObject->getAccountNumLots() > transfererCreatureObject->getMaxNumberOfLots()) && transfererCreatureObject->getObjVars().hasItem("lotOverlimit.structure_id"))
			{
				NetworkId lotOverlimitStructure;
				if (transfererCreatureObject->getObjVars().getItem("lotOverlimit.structure_id", lotOverlimitStructure) && lotOverlimitStructure.isValid())
				{
					// determine the destination type
					ServerObject const * topmostDestinationParent = nullptr;
					ServerObject const * destinationParent = destination;
					while (destinationParent)
					{
						// if item is being put into another creature object, allow it
						if (destinationParent->asCreatureObject())
							break;

						// if item is being put into a vendor, allow it
						if (destinationParent->isVendor() || destinationParent->isBazaarTerminal())
							break;

						// if item is being put into a gold or buildout object, allow it
						if (ConfigServerGame::getMaxGoldNetworkId().isValid() && (destinationParent->getNetworkId() < ConfigServerGame::getMaxGoldNetworkId()))
							break;

						if (destinationParent->getNetworkId() < NetworkId::cms_invalid)
							break;

						topmostDestinationParent = destinationParent;
						destinationParent = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*destinationParent));
					}

					// we went all the way to the topmost destination container
					// so see if it is a building or a factory, and if so, restrict
					// the transfer unless it is a building and is the building that
					// caused the lot limit to be exceeded
					if (!destinationParent && topmostDestinationParent)
					{
						if ((topmostDestinationParent->getGameObjectType() == SharedObjectTemplate::GOT_installation_factory) ||
							(GameObjectTypes::isTypeOf(topmostDestinationParent->getGameObjectType(), SharedObjectTemplate::GOT_building) && (topmostDestinationParent->getNetworkId() != lotOverlimitStructure)))
						{
							if (transfererCreatureObject->getClient())
							{
								std::string lotOverlimitStructureName;
								std::string lotOverlimitStructureLocation;

								IGNORE_RETURN(transfererCreatureObject->getObjVars().getItem("lotOverlimit.structure_name", lotOverlimitStructureName));
								IGNORE_RETURN(transfererCreatureObject->getObjVars().getItem("lotOverlimit.structure_location", lotOverlimitStructureLocation));

								Chat::sendSystemMessage(*transfererCreatureObject, Unicode::narrowToWide(FormattedString<1024>().sprintf("You are currently over the lot limit, and cannot drop any item into (or move around items within) any factory or structure, other than the %s structure located at %s that caused you to exceed the lot limit.", lotOverlimitStructureName.c_str(), lotOverlimitStructureLocation.c_str())), Unicode::emptyString);
							}

							error = Container::CEC_BlockedByDestinationContainer;
							return false;
						}
					}
				}
			}
		}
	}

	// we can only get to this point if both the item and
	// destination (if specified) are authoritative
	if (!checkTransferScripts (destination, item, transferer, error))
		return false;

	if (destination)
	{
		Container const * const container = getContainer(*destination);
		if (container)
		{
			bool const retval = container->mayAdd(item, error);
			DEBUG_REPORT_LOG(!retval, ("Container prevented transfer\n"));
			return retval;
		}
		else
		{
			DEBUG_REPORT_LOG(true, ("Destination %s is not a container\n", destination->getNetworkId().getValueString().c_str()));
			error = Container::CEC_NoContainer;
			return false;
		}
	}

	return true;
}

// -----------------------------------------------------------------------

bool ContainerInterface::canTransferToSlot(ServerObject &destination, ServerObject &item, SlotId const &slotId, ServerObject *transferer, Container::ContainerErrorCode &error)
{
	error = Container::CEC_Success;

	SlottedContainer const * const container = destination.getSlottedContainerProperty();
	if (!container)
	{
		error = Container::CEC_NoContainer;
		return false;
	}

	if (!container->mayAdd(item, slotId, error))
	{
		DEBUG_REPORT_LOG(true, ("Container %s prevented transfer for item %s\n", destination.getDebugInformation().c_str(), item.getDebugInformation().c_str()));
		return false;
	}

	// Cannot move a tcg card that is in the middle of the redemption process
	if (!VeteranRewardManager::checkForTcgRedemptionInProgress(item))
	{
		error = Container::CEC_BlockedByItemBeingTransferred;
		return false;
	}

	// Cannot move a reward item that is in the middle of the trade in process
	if (!VeteranRewardManager::checkForTradeInInProgress(item))
	{
		error = Container::CEC_BlockedByItemBeingTransferred;
		return false;
	}

	if (!checkTransferScripts (&destination, item, transferer, error))
	{
		DEBUG_REPORT_LOG(true, ("Scripts prevented container %s transfer for item %s\n", destination.getDebugInformation().c_str(), item.getDebugInformation().c_str()));
		return false;
	}

	return true;
}

// -----------------------------------------------------------------------

/**
 *  Transfer the item to the container specified by destination, however it can.
 *  This means for spatial containers it will put the object at the origin of the parent coord space
 *  and for a slotted container, it will put the object in the first available slot.
 */
bool ContainerInterface::transferItemToGeneralContainer(ServerObject &destination, ServerObject &item, ServerObject *transferer, Container::ContainerErrorCode &error, bool allowOverloaded)
{
	error = Container::CEC_Success;

	VolumeContainer * const destContainerV = getVolumeContainer(destination);
	if (destContainerV)
	{
		return transferItemToVolumeContainer(destination, item, transferer, error, allowOverloaded);
	}

	SlottedContainer * const destContainerL = getSlottedContainer(destination);
	if (destContainerL)
	{
		int tmp = -1;
		if (destContainerL->getFirstUnoccupiedArrangement(item, tmp, error))
			return transferItemToSlottedContainer(destination, item, tmp, transferer, error);
	}

	return false;
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToSlottedContainer(ServerObject &destination, ServerObject &item, int arrangementIndex, ServerObject *transferer, Container::ContainerErrorCode &error)
{
	PROFILER_AUTO_BLOCK_DEFINE("transferItemToSlottedContainer");

	error = Container::CEC_Success;

	if (arrangementIndex < 0)
	{
		DEBUG_WARNING(true, ("Invaid arrangement index in transferItemToSlottedContainer"));
		error = Container::CEC_InvalidArrangement;
		return false;
	}

	if (transferer) //@todo add GM/permissions check
	{
		//Check each slot to make sure it can be manipulated.
		SlottedContainmentProperty * const slottedProp = getSlottedContainmentProperty(item);
		if (!slottedProp)
		{
			WARNING_STRICT_FATAL(true, ("Invaid item (no slottedContainmentProperty) in transferItemToSlottedContainer"));
			error = Container::CEC_WrongType;
			return false;
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("transferItemToSlottedContainer.canManipulateArrangement");
			//@todo better player feedback
			if (!slottedProp->canManipulateArrangement(arrangementIndex))
			{
				error = Container::CEC_NoPermission;
				return false;
			}
		}
	}

	ServerObject *sourceObject = nullptr;
	Container *sourceContainer = nullptr;

	if (!sharedTransferBegin(item, sourceObject, sourceContainer, error))
		return false;

	SlottedContainer * const destContainer = getSlottedContainer(destination);
	if (!destContainer)
	{
		WARNING_STRICT_FATAL(true, ("This destination is not a slot container!!"));
		if (error == Container::CEC_Success)
			error = Container::CEC_NoContainer;
		return false;
	}

	if (!canTransferTo(&destination, item, transferer, error))
		return false;

	//@todo currently canTransferTo calls mayAdd, and we call it again.  Optimize.
	if (!destContainer->mayAdd(item, arrangementIndex, error))
		return false;

	//check all scripts

	//check source
	//then check item
	//then check destiation

	if (!handleTransferSource(sourceContainer, item, error))
		return false;

	if (!destContainer->add(item, arrangementIndex, error))
	{
		//@todo re-examine use of fatal here.
		WARNING_STRICT_FATAL(true, ("Checks to add an item succeeded, but the add failed! Panic"));
		return false;
	}

	item.onContainerTransferComplete(sourceObject, &destination);
	handleTransferScripts(item, sourceObject, &destination, transferer, error);
	return true;
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToSlottedContainer(ServerObject &destination, ServerObject &item, SlotId const &slotId, ServerObject *transferer, Container::ContainerErrorCode &error)
{
	error = Container::CEC_Success;

	SlottedContainmentProperty * const slottedProp = getSlottedContainmentProperty(item);
	if (!slottedProp)
	{
		WARNING_STRICT_FATAL(true, ("Invaid item (no slottedContainmentProperty) in transferItemToSlottedContainer"));
		error = Container::CEC_WrongType;
		return false;
	}
	return transferItemToSlottedContainer(destination, item, slottedProp->getBestArrangementForSlot(slotId), transferer, error);
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToUnknownContainer(ServerObject &destination, ServerObject &item, int arrangementIndex, Transform const &transform, ServerObject *transferer, Container::ContainerErrorCode &error)
{
	error = Container::CEC_Success;

	VolumeContainer * const destContainerV = getVolumeContainer(destination);
	if (destContainerV)
	{
		return transferItemToVolumeContainer(destination, item, transferer, error);
	}

	SlottedContainer * const destContainerL = getSlottedContainer(destination);
	if (destContainerL)
	{
		return transferItemToSlottedContainer(destination, item, arrangementIndex, transferer, error);
	}

	CellProperty * const destContainerC = getCell(destination);
	if (destContainerC)
	{
		bool retval = transferItemToCell(destination, item, transform, transferer, error);
		return retval;
	}

	error = Container::CEC_NoContainer;

	return false;
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToVolumeContainer(ServerObject &destination, ServerObject &item, ServerObject *transferer, Container::ContainerErrorCode &error, bool allowOverloaded)
{
	error = Container::CEC_Success;

	ServerObject *sourceObject = nullptr;
	Container *sourceContainer = nullptr;

	if (!sharedTransferBegin(item, sourceObject, sourceContainer, error))
		return false;

	VolumeContainer * const destContainer = getVolumeContainer(destination);
	if (!destContainer)
	{
		WARNING_STRICT_FATAL(true, ("This destination is not a volume container!!"));
		error = Container::CEC_WrongType;
		return false;
	}

	//@todo temp code to catch players going into inventories
	if (item.isPlayerControlled())
	{
		DEBUG_FATAL(true, ("Trying to put a player inside a volume container!"));
		error = Container::CEC_BlockedByItemBeingTransferred;
		return false;
	}

	if (!canTransferTo(&destination, item, transferer, error))
	{
		// If container message is anthing but full false
		// If it is full and overloaded is false: false
		if (allowOverloaded)
		{
			if (! (error == Container::CEC_Full || error == Container::CEC_TooLarge))
				return false;
		}
		else
			return false;
	}

	if (!handleTransferSource(sourceContainer, item, error))
		return false;

	// add() with allowOverloaded doesn't work, so we use the following workaround.
	// See JavaLibrary::createNewObjectInInventoryOverloadedCrc() for where this
	// workaround is also used.
	int oldCapacity = 0;
	if (allowOverloaded)
		oldCapacity = destContainer->debugDoNotUseSetCapacity(-1);

	bool const success = destContainer->add(item, error, allowOverloaded);

	if (allowOverloaded)
	{
		IGNORE_RETURN(destContainer->debugDoNotUseSetCapacity(oldCapacity));
		IGNORE_RETURN(destContainer->recalculateVolume());
	}

	WARNING_DEBUG_FATAL(!success, ("Checks to add an item succeeded, but the add failed! Panic"));

	if (!success)
		return false;

	item.onContainerTransferComplete(sourceObject, &destination);
	handleTransferScripts(item, sourceObject, &destination, transferer, error);
	return true;
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToCell(ServerObject &destination, ServerObject &item, Transform const &t, ServerObject *transferer, Container::ContainerErrorCode &error)
{
	error = Container::CEC_Success;

	ServerObject *sourceObject = nullptr;
	Container *sourceContainer = nullptr;

	//check source & item
	if (!sharedTransferBegin(item, sourceObject, sourceContainer, error))
	{
		DEBUG_REPORT_LOG(true, ("Could not transfer from source to cell\n"));
		return false;
	}

	CellProperty * const destContainer = getCell(destination);
	if (!destContainer)
	{
		WARNING_STRICT_FATAL(true, ("This destination is not a cell!!"));
		error = Container::CEC_NoContainer;
		return false;
	}

	if (!item.canDropInWorld())
	{
		error = Container::CEC_WrongType;
		return false;
	}

	//check all scripts & destination
	if (!canTransferTo(&destination, item, transferer, error))
	{
		DEBUG_REPORT_LOG(true, ("Could not transfer to dest cell\n"));
		return false;
	}

	if (!handleTransferSource(sourceContainer, item, error))
	{
		DEBUG_REPORT_LOG(true, ("Could not transfer from source 2\n"));
		return false;
	}

	destContainer->addObject_w(item);
	item.setTransform_o2p(t);

	item.onContainerTransferComplete(sourceObject, &destination);
	handleTransferScripts(item, sourceObject, &destination, transferer, error);
	return true;
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToCell(ServerObject &destinationCell, ServerObject &item, ServerObject *transferer, Container::ContainerErrorCode &error)
{
#ifdef DEBUG
	Transform const sanityCheck = item.getTransform_o2w();
#endif

	Transform const t = destinationCell.getTransform_o2w().rotateTranslate_l2p(item.getTransform_o2p());
	bool const retval = transferItemToCell(destinationCell, item, t, transferer, error);

#ifdef DEBUG
	DEBUG_FATAL(sanityCheck != item.getTransform_o2w());
#endif
	return retval;
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToWorld(ServerObject &item, Transform const &pos, ServerObject *transferer, Container::ContainerErrorCode &error)
{
	error = Container::CEC_Success;

	if (transferer && transferer->getClient() && !transferer->getClient()->isGod())
	{
		// Only GMs are allowed to drop things into the world.
		DEBUG_REPORT_LOG(true, ("Player tried to drop something in world, but they are not a god.\n"));
		error = Container::CEC_NoPermission;
		return false;
	}

	if (!item.canDropInWorld())
	{
		error = Container::CEC_WrongType;
		return false;
	}

	if (!canTransferTo(nullptr, item, transferer, error))
	{
		return false;
	}

	ServerObject *sourceObject = nullptr;
	Container *sourceContainer = nullptr;

	if (!sharedTransferBegin(item, sourceObject, sourceContainer, error))
	{
		DEBUG_REPORT_LOG(true, ("Could not transfer to world because source disallowed.\n"));
		return false;
	}
	if (!sourceContainer)
	{
		// already in the world
		DEBUG_REPORT_LOG(true, ("Failed transfer to world because it was already there\n"));
		error = Container::CEC_Unknown;
		return false;
	}

	if (!handleTransferSource(sourceContainer, item, error))
	{
		DEBUG_REPORT_LOG(true, ("Could not transfer to world because source disallowed 2.\n"));
		return false;
	}

	item.setTransform_o2p(pos);

	item.onContainerTransferComplete(sourceObject, nullptr);
	handleTransferScripts(item, sourceObject, nullptr, transferer, error);
	return true;
}

// -----------------------------------------------------------------------

bool ContainerInterface::canPlayerManipulateSlot(SlotId const &slot)
{
	return SlotIdManager::isSlotPlayerModifiable(slot);
}

// -----------------------------------------------------------------------

Container *ContainerInterface::getContainer(Object &obj)
{
	return obj.getContainerProperty();
}

// -----------------------------------------------------------------------

Container const *ContainerInterface::getContainer(Object const &obj)
{
	return obj.getContainerProperty();
}

// ----------------------------------------------------------------------

SlottedContainer *ContainerInterface::getSlottedContainer(Object &obj)
{
	return obj.getSlottedContainerProperty();
}

// ----------------------------------------------------------------------

SlottedContainer const *ContainerInterface::getSlottedContainer(Object const &obj)
{
	return obj.getSlottedContainerProperty();
}

// -----------------------------------------------------------------------

VolumeContainer *ContainerInterface::getVolumeContainer(Object &obj)
{
	return obj.getVolumeContainerProperty();
}

// -----------------------------------------------------------------------

VolumeContainer const *ContainerInterface::getVolumeContainer(Object const &obj)
{
	return obj.getVolumeContainerProperty();
}

// -----------------------------------------------------------------------

CellProperty *ContainerInterface::getCell(Object &obj)
{
	return obj.getCellProperty();
}

// -----------------------------------------------------------------------

CellProperty const *ContainerInterface::getCell(Object const &obj)
{
	return obj.getCellProperty();
}

// -----------------------------------------------------------------------

Object *ContainerInterface::getContainedByObject(Object &obj)
{
	ContainedByProperty * const containedBy = getContainedByProperty(obj);
	if (containedBy)
		return containedBy->getContainedBy();
	return nullptr;
}

// ----------------------------------------------------------------------

Object const *ContainerInterface::getContainedByObject(Object const &obj)
{
	ContainedByProperty const * const containedBy = getContainedByProperty(obj);
	if (containedBy)
		return containedBy->getContainedBy();
	return nullptr;
}

// -----------------------------------------------------------------------

ContainedByProperty *ContainerInterface::getContainedByProperty(Object &obj)
{
	return obj.getContainedByProperty();
}

// -----------------------------------------------------------------------

ContainedByProperty const *ContainerInterface::getContainedByProperty(Object const &obj)
{
	return obj.getContainedByProperty();
}

// -----------------------------------------------------------------------

SlottedContainmentProperty *ContainerInterface::getSlottedContainmentProperty(Object &obj)
{
	return safe_cast<SlottedContainmentProperty *>(obj.getProperty(SlottedContainmentProperty::getClassPropertyId()));
}

// -----------------------------------------------------------------------

SlottedContainmentProperty const *ContainerInterface::getSlottedContainmentProperty(Object const &obj)
{
	return safe_cast<SlottedContainmentProperty const *>(obj.getProperty(SlottedContainmentProperty::getClassPropertyId()));
}

// -----------------------------------------------------------------------

VolumeContainmentProperty *ContainerInterface::getVolumeContainmentProperty(Object &obj)
{
	return safe_cast<VolumeContainmentProperty *>(obj.getProperty(VolumeContainmentProperty::getClassPropertyId()));
}

// -----------------------------------------------------------------------

VolumeContainmentProperty const *ContainerInterface::getVolumeContainmentProperty(Object const & obj)
{
	return safe_cast<VolumeContainmentProperty const *>(obj.getProperty(VolumeContainmentProperty::getClassPropertyId()));
}

// -----------------------------------------------------------------------

Object *ContainerInterface::getTopmostContainer(Object &obj)
{
	// Do I have a containedBy property?  If not I am topmost.
	ContainedByProperty *containedBy = getContainedByProperty(obj);
	if (!containedBy)
		return &obj;

	// Is my containedBy property empty?  If so I am topmost.
	Object *currentContainer = containedBy->getContainedBy();
	if (!currentContainer)
		return &obj;

	// Does my parent have a containedBy property?  If not he is topmost.
	containedBy = getContainedByProperty(*currentContainer);
	if (!containedBy)
		return currentContainer;

	// Iterate from here.
	Object *nextContainer = containedBy->getContainedBy();
	while (nextContainer)
	{
		currentContainer = nextContainer;
		containedBy = getContainedByProperty(*currentContainer);
		if (!containedBy)
			return currentContainer;
		nextContainer = containedBy->getContainedBy();
	}
	return currentContainer;
}

// -----------------------------------------------------------------------

Object const *ContainerInterface::getTopmostContainer(Object const &obj)
{
	return getTopmostContainer(const_cast<Object &>(obj));
}

// -----------------------------------------------------------------------
// Returns the object if it is in the world, or the first parent of the object
// that is in the world.  This returns nullptr, if the none of the parents of the object are in the world.

Object *ContainerInterface::getFirstParentInWorld(Object &obj)
{
	// Am I in the world? If so, I am topmost.
	if (obj.isInWorld())
		return &obj;

	// Do I have a containedBy property?  If not I am topmost, but am not in the world
	ContainedByProperty const *containedBy = getContainedByProperty(obj);
	if (!containedBy)
	{
		WARNING_STRICT_FATAL(true, ("All objects should have a containedby property"));
		return nullptr;
	}

	// Is my containedBy property empty?  If so I am topmost but am not in the world
	Object *currentContainer = containedBy->getContainedBy();
	if (!currentContainer)
	{
		return nullptr;
	}

	// Does my parent expose contents?  If it does, then return nullptr since I have been removed from the world.
	if (!getContainer(*currentContainer) || getContainer(*currentContainer)->isContentItemExposedWith(*currentContainer))
	{
		return nullptr;
	}

	// Is my parent in the world?  If so, he is topmost
	if (currentContainer->isInWorld())
		return currentContainer;

	// Does my parent have a containedBy property?  If not he is topmost.
	containedBy = getContainedByProperty(*currentContainer);
	if (!containedBy)
	{
		WARNING_STRICT_FATAL(true, ("All objects should have a containedby property"));
		return (currentContainer->isInWorld()) ? currentContainer : nullptr;
	}

	// Iterate from here.
	Object *nextContainer = containedBy->getContainedBy();
	while (nextContainer && getContainer(*nextContainer) && !getContainer(*nextContainer)->isContentItemExposedWith(*nextContainer))
	{
		currentContainer = nextContainer;

		// If I am in the world, I am topmost.
		if (currentContainer->isInWorld())
			return currentContainer;

		//If I am not contained, I am topmost, but not in world
		containedBy = getContainedByProperty(*currentContainer);
		if (!containedBy)
		{
			WARNING_STRICT_FATAL(true, ("All objects should have a containedby property"));
			return nullptr;
		}

		nextContainer = containedBy->getContainedBy();
	}
	return currentContainer->isInWorld() ? currentContainer : nullptr;
}

// -----------------------------------------------------------------------

Object const *ContainerInterface::getFirstParentInWorld(Object const &obj)
{
	return getFirstParentInWorld(const_cast<Object &>(obj));
}

// -----------------------------------------------------------------------

CellObject *ContainerInterface::getContainingCellObject(Object &obj)
{
	CellObject *cellObj = 0;
	Object *o = getContainedByObject(obj);
	while (o)
	{
		ServerObject * const so = o->asServerObject();
		if (so)
		{
			cellObj = so->asCellObject();
			if (cellObj)
				break;
		}
		o = getContainedByObject(*o);
	}

	return cellObj;
}

// -----------------------------------------------------------------------

CellObject const *ContainerInterface::getContainingCellObject(Object const &obj)
{
	CellObject const *cellObj = 0;
	Object const *o = getContainedByObject(obj);
	while (o)
	{
		ServerObject const * const so = o->asServerObject();
		if (so)
		{
			cellObj = so->asCellObject();
			if (cellObj)
				break;
		}
		o = getContainedByObject(*o);
	}

	return cellObj;
}

//----------------------------------------------------------------------

bool ContainerInterface::isNestedWithin(Object const &obj, NetworkId const &containerId)
{
	const Object* parent = getContainedByObject(obj);
	while (parent)
	{
		if (parent->getNetworkId() == containerId)
			return true;

		parent = getContainedByObject(*parent);
	}

	return false;
}

// ----------------------------------------------------------------------

void ContainerInterface::sendContainerMessageToClient(ServerObject const &player, Container::ContainerErrorCode errorCode, ServerObject const *target)
{
	//-- Check if error code is one that we skip sending messages to client for.
	//   In these cases the scripter should have already generated a message.
	bool const sendMessage = (errorCode != Container::CEC_BlockedByScript && errorCode != Container::CEC_SilentError);

	if (sendMessage || (player.getClient() && player.getClient()->isGod()))
	{
		char tmp[64];
		if (target && (!target->getAssignedObjectName().empty() || target->getObjectNameStringId().isValid()))
			snprintf(tmp, sizeof(tmp)-1, "container%02d_prose", static_cast<int>(errorCode));
		else
			snprintf(tmp, sizeof(tmp)-1, "container%02d", static_cast<int>(errorCode));
		tmp[sizeof(tmp)-1] = '\0';

		StringId const code("container_error_message", tmp);

		Chat::sendSystemMessageSimple(player, code, target);
	}
}

// -----------------------------------------------------------------------

// ----------------------------------------------------------------------
//
// The load_with container for an object is defined as follows:
//
// If the object is a player controlled creature, the load_with is the object
//
// Otherwise, iterate through containing objects until encountering:
// 1) A player controlled creature (the load_with is the creature)
// 2) A demand loaded container (the load_with is the container)
// 3) The topmost container (the load_with is the topmost container, unless it is a ship with an owner, in which case it is the owner)

NetworkId const ContainerInterface::getLoadWithContainerId(ServerObject const &obj)
{
	if (obj.isPlayerControlled())
		return obj.getNetworkId();

	ServerObject const *o = &obj;
	ServerObject const *nextObj = safe_cast<ServerObject const *>(NON_NULL(getContainedByProperty(*o))->getContainedBy());
	while (nextObj)
	{
		o = nextObj;
		if (o->isPlayerControlled() || !o->getLoadContents())
			return o->getNetworkId();
		nextObj = safe_cast<ServerObject const *>(NON_NULL(getContainedByProperty(*o))->getContainedBy());
	}

	ShipObject const * const ship = o->asShipObject();
	if (ship)
	{
		NetworkId const &owner = ship->getOwnerId();
		if (owner != NetworkId::cms_invalid)
			return owner;
	}
	return o->getNetworkId();
}

// -----------------------------------------------------------------------

bool ContainerInterface::onObjectDestroy(ServerObject& item) // currently only called by ServerObject::onPermanentlyDestroyed()
{
	ServerObject* parentObject = safe_cast<ServerObject*>(getContainedByObject(item));
	if (parentObject)
	{
		Container *const container = getContainer(*parentObject);
		if (container)
		{
			Container::ContainerErrorCode error = Container::CEC_Success;
			if (!container->remove(item, error))
			{	
				// TODO: DDM change to FATAL ... this should never happen but the impact of it is too scary right now
				WARNING_DEBUG_FATAL(true, ("ContainerInterface::onObjectDestroy: object %s could not be removed from container %s for reason %d?\n", item.getDebugInformation().c_str(), parentObject->getDebugInformation().c_str(), static_cast<int>(error)));
				
				return false;
			}
			handleTransferScripts(item, parentObject, nullptr, nullptr, error);
		}
	}
	return true;
}

// ======================================================================

