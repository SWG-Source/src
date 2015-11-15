// ======================================================================
//
// ContainerInterface.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ContainerInterface_H
#define	INCLUDED_ContainerInterface_H


/** This will be moved to a game dependant place so that we can add game restrictions on these function.
*/

#include "sharedObject/Container.h"
#include "sharedObject/SlotId.h"

// ======================================================================

class CellObject;
class CellProperty;
class ContainedByProperty;
class NetworkId;
class Object;
class ServerObject;
class SlottedContainer;
class SlottedContainmentProperty;
class Transform;
class VolumeContainer;
class VolumeContainmentProperty;

// ======================================================================

/**
* The ContainerInterface is intended to provide an interface to the game (network, scripts, UI, etc) for manipulating
* containers.  The game shouldn't interface with the containers directly, since this will take care of scripts (on the server side)
* and will clean up old containers, and keep everything synch'd.
*/

class ContainerInterface
{
public:
	static bool canTransferTo(ServerObject *destination, ServerObject &item, ServerObject *transferer, Container::ContainerErrorCode &error);
	static bool canTransferToSlot(ServerObject &destination, ServerObject &item, SlotId const &slotId, ServerObject *transferer, Container::ContainerErrorCode &error);
	static bool transferItemToGeneralContainer(ServerObject &destination, ServerObject &item, ServerObject *transferer, Container::ContainerErrorCode &error, bool allowOverloaded = false);
	static bool transferItemToSlottedContainer(ServerObject &destination, ServerObject &item, int arrangementIndex, ServerObject *transferer, Container::ContainerErrorCode &error);
	static bool transferItemToSlottedContainer(ServerObject &destination, ServerObject &item, SlotId const &slot, ServerObject *transferer, Container::ContainerErrorCode &error);
	static bool transferItemToUnknownContainer(ServerObject &destination, ServerObject &item, int arrangementIndex, Transform const &transform, ServerObject *transferer, Container::ContainerErrorCode &error);
	static bool transferItemToVolumeContainer(ServerObject &destination, ServerObject &item, ServerObject *transferer, Container::ContainerErrorCode &error, bool allowOverloaded = false);
	static bool transferItemToCell(ServerObject &destination, ServerObject &item, ServerObject *transferer, Container::ContainerErrorCode &error);
	static bool transferItemToCell(ServerObject &destination, ServerObject &item, Transform const &t, ServerObject *transferer, Container::ContainerErrorCode &error);
	static bool transferItemToWorld(ServerObject &item, Transform const &position, ServerObject *transferer, Container::ContainerErrorCode &error);
	static bool canPlayerManipulateSlot(SlotId const &slot);
	
	static bool      onObjectDestroy(ServerObject& item);

	// Property manipulator helper functions
	static Container *              getContainer(Object &obj);
	static Container const *        getContainer(Object const &obj);
	static SlottedContainer *       getSlottedContainer(Object &obj);
	static SlottedContainer const * getSlottedContainer(Object const &obj);
	static VolumeContainer *        getVolumeContainer(Object &obj);
	static VolumeContainer const *  getVolumeContainer(Object const &obj);
	static CellProperty *           getCell(Object &obj);
	static CellProperty const *     getCell(Object const &obj);

	static Object *                           getContainedByObject(Object &obj);
	static Object const *                     getContainedByObject(Object const &obj);
	static ContainedByProperty *              getContainedByProperty(Object &obj);
	static ContainedByProperty const *        getContainedByProperty(Object const &obj);
	static SlottedContainmentProperty *       getSlottedContainmentProperty(Object &obj);
	static SlottedContainmentProperty const * getSlottedContainmentProperty(Object const &obj);
	static VolumeContainmentProperty *        getVolumeContainmentProperty(Object &obj);
	static VolumeContainmentProperty const *  getVolumeContainmentProperty(Object const &obj);

	static Object *           getTopmostContainer(Object &obj);
	static Object const *     getTopmostContainer(Object const &obj);
	static Object *           getFirstParentInWorld(Object &obj);
	static Object const *     getFirstParentInWorld(Object const &obj);
	static CellObject *       getContainingCellObject(Object &obj);
	static CellObject const * getContainingCellObject(Object const &obj);

	static bool               isNestedWithin(Object const &obj, NetworkId const &containerId);

	static void sendContainerMessageToClient(ServerObject const &player, Container::ContainerErrorCode code, ServerObject const *target = 0);
	static NetworkId const getLoadWithContainerId(ServerObject const &obj);

private:
	ContainerInterface();
	ContainerInterface(ContainerInterface const &);
	ContainerInterface &operator=(ContainerInterface const &);
	~ContainerInterface();
};

// ======================================================================

#endif // INCLUDED_ContainerInterface_H

