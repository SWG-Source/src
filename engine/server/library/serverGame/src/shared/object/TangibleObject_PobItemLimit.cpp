// ======================================================================
//
// TangibleObject_PobItemLimit.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/TangibleObject.h"

#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedLog/Log.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

// ======================================================================

namespace TangibleObjectNamespace
{
	void remove();
	bool isCreatureObject(Object const &object);
	bool isPlayerPob(TangibleObject const &pob);
	bool countsToItemLimit(ServerObject const &item, TangibleObject const &pob, ServerObject const *firstParentOfDestination);
	void logContents(ServerObject const &item);
	std::string const &getItemLogPobType(TangibleObject const &pob); //lint !e1929 // ok to return a reference when it always refers to a static
	void permanentlyDestroyContents(ServerObject &item, int deleteReason);

	std::string const OBJVAR_POB_ITEM_LIMIT("itemLimit");
	std::string const OBJVAR_POB_ITEM_LIMIT_INCREASE ("structureChange.storageIncrease");
	std::string const s_itemLogPobTypeString_House("house");
	std::string const s_itemLogPobTypeString_Ship("ship");

	// for structures that uses up additional lots over the structure footprint,
	// keep a list of them here, so that we can include the additional lots when
	// calculating the base item limit for the structure
	std::map<uint32, int> s_mapPobLotIncreaseAdjustment;
}

using namespace TangibleObjectNamespace;

// ======================================================================

bool TangibleObjectNamespace::isCreatureObject(Object const &object)
{
	return object.getObjectTemplate()->getId() == ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag;
}

// ----------------------------------------------------------------------

bool TangibleObjectNamespace::isPlayerPob(TangibleObject const &pob)
{
	ShipObject const * const pobAsShip = pob.asShipObject();
	if (pobAsShip)
		return pobAsShip->isPlayerShip();
	BuildingObject const * const pobAsBuilding = pob.asBuildingObject();
	if (pobAsBuilding)
		return pobAsBuilding->isPlayerPlaced();
	return false;
}

// ----------------------------------------------------------------------

bool TangibleObjectNamespace::countsToItemLimit(ServerObject const &item, TangibleObject const &pob, ServerObject const *firstParentOfDestination)
{
	if (&item == firstParentOfDestination && item.getGameObjectType() == SharedObjectTemplate::GOT_misc_container_ship_loot)
		return true;

	return    item.isPersisted()
	       && firstParentOfDestination
	       && !isCreatureObject(item)
	       && !isCreatureObject(*firstParentOfDestination)
	       && isPlayerPob(pob)
	       && !firstParentOfDestination->isVendor();
}

// ----------------------------------------------------------------------

void TangibleObjectNamespace::logContents(ServerObject const &item)
{
	Container const * const container = ContainerInterface::getContainer(item);
	if (container)
	{
		std::string const &itemLog = item.getItemLog();
		for (ContainerConstIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject const * const content = safe_cast<ServerObject const *>((*i).getObject());
			if (content)
			{
				LOG("CustomerService", ("DropLog:    Item: %s in %s", content->getItemLog().c_str(), itemLog.c_str()));
				logContents(*content);
			}
		}
	}
}

// ----------------------------------------------------------------------

void TangibleObjectNamespace::remove()
{
}

// ----------------------------------------------------------------------

void TangibleObject::install()
{
	// for structures that uses up additional lots over the structure footprint,
	// keep a list of them here, so that we can include the additional lots when
	// calculating the base item limit for the structure
	char const * const playerStructureDataTableName = "datatables/structure/player_structure.iff";
	DataTable * table = DataTableManager::getTable(playerStructureDataTableName, true);
	if (table)
	{
		int const columnStructure = table->findColumnNumber("STRUCTURE");
		int const columnLotReduction = table->findColumnNumber("LOT_REDUCTION");
		if ((columnStructure >= 0) && (columnLotReduction >= 0))
		{
			std::string structure;
			int lotReduction;

			for (int i = 0, numRows = table->getNumRows(); i < numRows; ++i)
			{
				structure = table->getStringValue(columnStructure, i);
				if (structure.empty())
					continue;

				lotReduction = table->getIntValue(columnLotReduction, i);
				if (lotReduction >= 0)
					continue;

				s_mapPobLotIncreaseAdjustment[CrcLowerString::calculateCrc(structure.c_str())] = -lotReduction;
			}
		}

		DataTableManager::close(playerStructureDataTableName);
	}

	ExitChain::add(TangibleObjectNamespace::remove, "TangibleObjectNamespace::remove");
}

// ----------------------------------------------------------------------

bool TangibleObject::onPobChildAboutToLoseItem(ServerObject *, ServerObject &, ServerObject *transferer)
{
	if (transferer && ContainerInterface::getTopmostContainer(*transferer) != this)
		return false;
	return true;
}

// ----------------------------------------------------------------------

bool TangibleObject::onPobChildAboutToGainItem(ServerObject &item, ServerObject *destination, ServerObject *transferer)
{
	// Only let people transfer stuff in a pob if they are in the pob
	if (transferer && ContainerInterface::getTopmostContainer(*transferer) != this)
		return false;

	ServerObject * const firstParent = destination ? safe_cast<ServerObject *>(ContainerInterface::getFirstParentInWorld(*destination)) : 0;
	if (countsToItemLimit(item, *this, firstParent))
	{
		Container const * const container = ContainerInterface::getContainer(*this);
		if (container)
		{
			// no dropping things that count toward item limits in demand loaded cells that are not loaded
			ServerObject const * const cell = safe_cast<ServerObject const *>((*container->begin()).getObject());
			if (cell && !cell->getLoadContents() && !cell->areContentsLoaded())
				return false;

			// check item limits
			if (getObjVars().hasItem(OBJVAR_POB_ITEM_LIMIT))
			{
				int volume = item.getVolume();
				if (item.getGameObjectType() == SharedObjectTemplate::GOT_misc_container_ship_loot)
					--volume;

				int baseItemLimit = 0;
				int additionalItems = 0;
				const int itemLimit = getPobTotalItemLimit(baseItemLimit, additionalItems);

				int const limit = getPobItemCount();

				DEBUG_REPORT_LOG(true,("*** \nnetworkId[%s]\ngetNumberOfLots()=%d\nbaseItemLimit=%d\nadditionalItems=%d\nlimit=%d\nvolume=%d\n\n\n",
					getNetworkId().getValueString().c_str(),
					getNumberOfLots(),
					baseItemLimit,
					additionalItems,
					limit,
					volume
					));

				if (limit + volume > itemLimit)
				{
					// if the item is currently immediately contained inside
					// a cell of this POB, then allow the transfer, because
					// we are only moving items within the POB, and not really
					// adding a new item; this is required for /restoreDecorationLayout
					// to work when the POB is already full, and all we are
					// doing is just moving stuff around the POB
					bool ignoreOverlimit = false;
					while (true)
					{
						ServerObject const * const itemContainingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(item));
						if (!itemContainingCell || !itemContainingCell->asCellObject())
							break;

						if (!destination || !destination->asCellObject())
							break;

						ServerObject const * const itemContainingPob = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*itemContainingCell));
						if (!itemContainingPob)
							break;

						if (const_cast<ServerObject *>(itemContainingPob) != this)
							break;

						if (const_cast<ServerObject *>(itemContainingPob) != ContainerInterface::getContainedByObject(*destination))
							break;

						ignoreOverlimit = true;
						break;
					}

					if (!ignoreOverlimit)
					{
						if (transferer && transferer->getClient())
							ContainerInterface::sendContainerMessageToClient(*transferer, Container::CEC_HouseItemLimit);
						return false;
					}
				}
			}
		}
	}
	return true;
}

// ----------------------------------------------------------------------

void TangibleObject::onPobChildLostItem(ServerObject *, ServerObject &item, ServerObject *source, ServerObject *transferer)
{
	ServerObject * const firstParent = source ? safe_cast<ServerObject *>(ContainerInterface::getFirstParentInWorld(*source)) : 0;
	if (countsToItemLimit(item, *this, firstParent))
	{
		LOG(
			"CustomerService",
			(
				"DropLog: Player %s has removed object %s from %s %s",
				transferer ? PlayerObject::getAccountDescription(transferer->asCreatureObject()).c_str() : "The Game System",
				item.getItemLog().c_str(),
				getItemLogPobType(*this).c_str(),
				getNetworkId().getValueString().c_str()));
		logContents(item);

		fixUpPobItemLimit();
	}
}

// ----------------------------------------------------------------------

void TangibleObject::onPobChildGainItem(ServerObject &item, ServerObject *, ServerObject *transferer)
{
	ServerObject * const firstParent = safe_cast<ServerObject *>(ContainerInterface::getFirstParentInWorld(item));
	if (countsToItemLimit(item, *this, firstParent))
	{
		LOG(
			"CustomerService",
			(
				"DropLog: Player %s has put object %s in %s %s",
				transferer ? PlayerObject::getAccountDescription(transferer->asCreatureObject()).c_str() : "The Game System",
				item.getItemLog().c_str(),
				getItemLogPobType(*this).c_str(),
				getNetworkId().getValueString().c_str()));
		logContents(item);

		fixUpPobItemLimit();
	}
}

// ----------------------------------------------------------------------

// returns the number of items (that count against the item limit) currently in this building
int TangibleObject::getPobItemCount() const
{
	int limit = 0;
	if (getObjVars().getItem(OBJVAR_POB_ITEM_LIMIT, limit))
		return limit;

	return 0;
}

// ----------------------------------------------------------------------

// returns the base number of items this pob can hold
int TangibleObject::getPobBaseItemLimit() const
{
	int limitBasedOnLots = 0;
	if (!getObjVars().getItem("structure.capacity_override", limitBasedOnLots))
	{
		int numberOfLots = (getNumberOfLots() / 4);

		// the higher of the lots used by the footprint vs lots actually
		// consumed will be used to calculate the base limit based on lots
		std::map<uint32, int>::const_iterator const iterFind = s_mapPobLotIncreaseAdjustment.find(getTemplateCrc());
		if (iterFind != s_mapPobLotIncreaseAdjustment.end())
			numberOfLots += iterFind->second;

		limitBasedOnLots = numberOfLots * ConfigServerGame::getHouseItemLimitMultiplier();
	}

	const int limitBasedOnConfig = ConfigServerGame::getMaxHouseItemLimit();
	const int baseItemLimit = std::min(limitBasedOnLots,limitBasedOnConfig);

	DEBUG_REPORT_LOG(true,("*** \nnetworkId[%s]\ngetNumberOfLots()=%d\nlimitBasedOnLots=%d\nlimitBasedOnConfig=%d\nbaseItemLimit=%d\n\n\n",
		getNetworkId().getValueString().c_str(),
		getNumberOfLots(),
		limitBasedOnLots,
		limitBasedOnConfig,
		baseItemLimit
	));

	return baseItemLimit;
}

// ----------------------------------------------------------------------

// returns the total number of items this building can hold (base + storage increase)
int TangibleObject::getPobTotalItemLimit(int & base, int & additional) const
{
	base = getPobBaseItemLimit();

	if (!getObjVars().getItem(OBJVAR_POB_ITEM_LIMIT_INCREASE, additional))
		additional = 0;

	return (base + additional);
}

// ----------------------------------------------------------------------

void TangibleObject::fixUpPobItemLimit()
{
	DEBUG_FATAL(!getPortalProperty(), ("fixUpPobItemLimit called on non-pob"));

	Container const * const container = ContainerInterface::getContainer(*this);
	if (container)
	{
		int totalVolume = 0;
		for (ContainerConstIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject const * const cell = safe_cast<ServerObject const *>((*i).getObject());
			if (cell)
			{
				Container const * const cellContainer = ContainerInterface::getContainer(*cell);
				if (cellContainer)
				{
					for (ContainerConstIterator j = cellContainer->begin(); j != cellContainer->end(); ++j)
					{
						ServerObject const * const cellContent = safe_cast<ServerObject const *>((*j).getObject());
						if (cellContent && countsToItemLimit(*cellContent, *this, cellContent))
						{
							totalVolume += cellContent->getVolume();
							if (cellContent->getGameObjectType() == SharedObjectTemplate::GOT_misc_container_ship_loot)
							{
								--totalVolume;
							}
							else if (cellContent->asTangibleObject() && cellContent->asTangibleObject()->isCraftingStation())
							{
								// for crafting station, the contents of the hopper count against the item limit
								static const ConstCharCrcLowerString INGREDIENT_HOPPER_SLOT_NAME = "ingredient_hopper";
								static const SlotId hopperSlotId(SlotIdManager::findSlotId(INGREDIENT_HOPPER_SLOT_NAME));

								Container::ContainerErrorCode tmp = Container::CEC_Success;
								SlottedContainer const * const stationContainer = ContainerInterface::getSlottedContainer(*cellContent);
								if (stationContainer)
								{
									Container::ContainedItem const hopperId = stationContainer->getObjectInSlot(hopperSlotId, tmp);

									if (hopperId.isValid())
									{
										ServerObject const * const hopper = safe_cast<ServerObject const *>(hopperId.getObject());

										if (hopper)
										{
											VolumeContainer const * const hopperContainer = ContainerInterface::getVolumeContainer(*hopper);

											if (hopperContainer)
												totalVolume += hopperContainer->getCurrentVolume();
										}
									}
								}
							}
						}
					}
				}
			}
		}
		IGNORE_RETURN(setObjVarItem(OBJVAR_POB_ITEM_LIMIT, totalVolume));
	}
}

// ----------------------------------------------------------------------

void TangibleObject::deletePobPersistedContents(CreatureObject const *player, int deleteReason)
{
	DEBUG_FATAL(!getPortalProperty(), ("deletePobPersistedContents called on non-pob"));

	if (player && player->getNetworkId() != getOwnerId())
		return; // only the owner of the pob can do this

	if (isPlayerPob(*this))
	{
		Container * const container = ContainerInterface::getContainer(*this);
		if (container)
		{
			for (ContainerIterator i = container->begin(); i != container->end(); ++i)
			{
				ServerObject * const cell = safe_cast<ServerObject *>((*i).getObject());
				if (cell)
				{
					Container * const cellContainer = ContainerInterface::getContainer(*cell);
					if (cellContainer)
					{
						for (ContainerIterator j = cellContainer->begin(); j != cellContainer->end(); ++j)
						{
							ServerObject * const cellContent = safe_cast<ServerObject *>((*j).getObject());
							if (cellContent && countsToItemLimit(*cellContent, *this, cellContent))
							{
								if (cellContent->getGameObjectType() == SharedObjectTemplate::GOT_misc_container_ship_loot)
									permanentlyDestroyContents(*cellContent, deleteReason);
								else
									IGNORE_RETURN(cellContent->permanentlyDestroy(deleteReason));
							}
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

NetworkId const & TangibleObject::movePobItemToPlayer(CreatureObject const &player, int index, bool overrideOwnership)
{
	DEBUG_FATAL(!getPortalProperty(), ("movePobItemToPlayer called on non-pob"));

	if (!overrideOwnership && player.getNetworkId() != getOwnerId())
		return NetworkId::cms_invalid;// only the owner of the pob can do this

	if (ContainerInterface::getTopmostContainer(player) != this)
		return NetworkId::cms_invalid; // only works if player is standing in the pob

	CellObject const * const playerCell = ContainerInterface::getContainingCellObject(player);
	if (playerCell && isPlayerPob(*this))
	{
		Container * const container = ContainerInterface::getContainer(*this);
		if (container)
		{
			for (ContainerIterator i = container->begin(); i != container->end(); ++i)
			{
				ServerObject * const cell = safe_cast<ServerObject *>((*i).getObject());
				if (cell)
				{
					Container *cellContainer = ContainerInterface::getContainer(*cell);
					if (cellContainer)
					{
						for (ContainerIterator j = cellContainer->begin(); j != cellContainer->end(); ++j)
						{
							ServerObject * const cellContent = safe_cast<ServerObject *>((*j).getObject());
							if (   cellContent
							    && countsToItemLimit(*cellContent, *this, cellContent)
							    && cellContent->getGameObjectType() != SharedObjectTemplate::GOT_misc_container_ship_loot)
							{
								if (index <= 0)
								{
									cellContent->teleportObject(player.getPosition_w(), playerCell->getNetworkId(), "", player.getPosition_p(), "");
									return cellContent->getNetworkId();
								}
								--index;
							}
						}
					}
				}
			}
		}
	}

	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

NetworkId const & TangibleObject::movePobItemToPlayer(CreatureObject const &player, NetworkId const &item, bool overrideOwnership)
{
	DEBUG_FATAL(!getPortalProperty(), ("movePobItemToPlayer called on non-pob"));

	if (!overrideOwnership && player.getNetworkId() != getOwnerId())
		return NetworkId::cms_invalid;// only the owner of the pob can do this

	if (ContainerInterface::getTopmostContainer(player) != this)
		return NetworkId::cms_invalid; // only works if player is standing in the pob

	ServerObject * const itemObject = ServerWorld::findObjectByNetworkId(item);
	if (!itemObject)
		return NetworkId::cms_invalid; // item not found

	ServerObject const * const itemContainer = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*itemObject));
	if (!itemContainer)
		return NetworkId::cms_invalid; // item must be the topmost item in the cell

	if (!itemContainer->asCellObject())
		return NetworkId::cms_invalid; // item must be the topmost item in the cell

	if (ContainerInterface::getTopmostContainer(*itemContainer) != this)
		return NetworkId::cms_invalid; // item is not contained inside this pob

	CellObject const * const playerCell = ContainerInterface::getContainingCellObject(player);
	if (playerCell && isPlayerPob(*this) && countsToItemLimit(*itemObject, *this, itemObject) && (itemObject->getGameObjectType() != SharedObjectTemplate::GOT_misc_container_ship_loot))
	{
		itemObject->teleportObject(player.getPosition_w(), playerCell->getNetworkId(), "", player.getPosition_p(), "");
		return itemObject->getNetworkId();
	}

	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

std::string const &TangibleObjectNamespace::getItemLogPobType(TangibleObject const &container)
{ //lint !e1929 // ok to return a reference when it always refers to a static
	if (container.asShipObject())
		return s_itemLogPobTypeString_Ship;
	return s_itemLogPobTypeString_House;
}

// ----------------------------------------------------------------------

void TangibleObjectNamespace::permanentlyDestroyContents(ServerObject &item, int deleteReason)
{
	std::vector<Watcher<ServerObject> > contents;

	Container * const container = ContainerInterface::getContainer(item);
	if (container)
	{
		for (ContainerIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject * const content = safe_cast<ServerObject *>((*i).getObject());
			if (content)
				contents.push_back(Watcher<ServerObject>(content));
		}
	}

	for (std::vector<Watcher<ServerObject> >::iterator i = contents.begin(); i != contents.end(); ++i)
	{
		ServerObject * const content = (*i).getPointer();
		if (content)
			IGNORE_RETURN(content->permanentlyDestroy(deleteReason));
	}
}

// ======================================================================

