// ======================================================================
//
// CreatureObject_Mounts.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CreatureObject.h"

#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "sharedGame/SharedSaddleManager.h"
#include "sharedCollision/CollisionProperty.h" 
#include "sharedCollision/CollisionWorld.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedGame/MountValidScaleRangeTable.h"
#include "sharedGame/SharedSaddleManager.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedCollision/Footprint.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgSharedUtility/Postures.h"
#include "UnicodeUtils.h"

#include <string>

// ======================================================================

namespace CreatureObjectNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string const  cs_petControlDeviceObjVarName("pet.controlDevice");
	char const *const  cs_riderSlotName = "rider";
	
	char const *const  cs_mountErrorChannelName = "mounts-error";
	char const *const  cs_mountInfoChannelName = "mounts-info";
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int const cs_totalNumberOfRiders = 8;

	SlotId s_riderSlotId[cs_totalNumberOfRiders] =
	{
		SlotId(SlotId::invalid),
		SlotId(SlotId::invalid),
		SlotId(SlotId::invalid),
		SlotId(SlotId::invalid),
		SlotId(SlotId::invalid),
		SlotId(SlotId::invalid),
		SlotId(SlotId::invalid),
		SlotId(SlotId::invalid),
	};

	CreatureObject * realGetMountingRider(CreatureObject const * mount, SlotId const & slot);
	CrcString const * getLogicalSaddleNameForMount(Object const * mount);
	int getSaddleSeatingCapacity(CreatureObject const * mount);
	bool realDetachRider(CreatureObject * mount, SlotId const & slot);
	void LogMovement(CreatureObject const * mount);
}

// ======================================================================

CreatureObject * CreatureObjectNamespace::realGetMountingRider(CreatureObject const * const mount, SlotId const & slot)
{
	//-- Ensure mounts are enabled.
	if (!ConfigServerGame::getMountsEnabled())
		return nullptr;

	//-- Get the rider element from the slotted container.	
	SlottedContainer const * const slottedContainer = ContainerInterface::getSlottedContainer(*mount);
	if (!slottedContainer)
		return nullptr;

	//-- Check for a rider.
	Container::ContainerErrorCode errorCode = Container::CEC_Success;
	CachedNetworkId riderId = slottedContainer->getObjectInSlot(slot, errorCode);
	if (errorCode != Container::CEC_Success)
		return nullptr;

	Object * const riderObject = riderId.getObject();

	ServerObject * const riderServerObject = (riderObject != 0) ? riderObject->asServerObject() : 0;

	CreatureObject * const riderCreatureObject = (riderServerObject != 0) ? riderServerObject->asCreatureObject() : 0;

	return riderCreatureObject;
}

// ----------------------------------------------------------------------

void CreatureObjectNamespace::LogMovement(CreatureObject const * const mount)
{
	NOT_NULL(mount);

	typedef std::vector<CreatureObject const *> Riders;
	Riders riders;

	if (mount != 0)
	{
		mount->getMountingRiders(riders);

		if (!riders.empty())
		{
			{
				int const process = static_cast<int>(GameServer::getInstance().getProcessId());
				char const * const id = mount->getNetworkId().getValueString().c_str();
				Vector const location_w(mount->getPosition_w());
				LOG("mounts-movement", ("server id=[%d], Vehicle id=[%s] is located at %f %f %f", process, id, location_w.x, location_w.y, location_w.z));
			}

			Riders::const_iterator ii = riders.begin();
			Riders::const_iterator iiEnd = riders.end();

			for (; ii != iiEnd; ++ii)
			{
				CreatureObject const *const rider = *ii;

				if (rider != 0)
				{
					int const process = static_cast<int>(GameServer::getInstance().getProcessId());
					char const * const id = rider->getNetworkId().getValueString().c_str();
					Vector const location_w(rider->getPosition_w());
					LOG("mounts-movement", ("server id=[%d], passenger id=[%s] is located at %f %f %f", process, id, location_w.x, location_w.y, location_w.z));
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

CrcString const * CreatureObjectNamespace::getLogicalSaddleNameForMount(Object const * const mount)
{
	NOT_NULL(mount);

	ServerObject const * const serverObject = mount->asServerObject();
	SharedObjectTemplate const * const sharedObjectTemplate = serverObject->getSharedTemplate();

	std::string const appearanceFilename = (sharedObjectTemplate != 0) ? sharedObjectTemplate->getAppearanceFilename() : "None";

	ConstCharCrcString const appearanceFilenameCrc(appearanceFilename.c_str());

	CrcString const * const logicalSaddleName = SharedSaddleManager::getLogicalSaddleNameForMountAppearance(appearanceFilenameCrc);

	return logicalSaddleName;
}

// ----------------------------------------------------------------------

int CreatureObjectNamespace::getSaddleSeatingCapacity(CreatureObject const * const mount)
{
	NOT_NULL(mount);

	// default to 1 seat...
	int saddleSeatingCapacity = 1;

	/*{
		ServerObject const * const serverObject = mount->asServerObject();
		SharedObjectTemplate const * const sharedObjectTemplate = serverObject->getSharedTemplate();

		std::string const appearanceFilename = (sharedObjectTemplate != 0) ? sharedObjectTemplate->getAppearanceFilename() : "None";
		DEBUG_REPORT_LOG(true, ("sharedObjectTemplate appearanceFilename = '%s'\n", appearanceFilename.c_str()));
	}*/

	CrcString const * const logicalSaddleName = getLogicalSaddleNameForMount(mount);

	if (logicalSaddleName != 0)
	{
		saddleSeatingCapacity = SharedSaddleManager::getSaddleSeatingCapacityForLogicalSaddleName(*logicalSaddleName);
	}

	return saddleSeatingCapacity;	
}

// ----------------------------------------------------------------------

bool CreatureObjectNamespace::realDetachRider(CreatureObject * const mount, SlotId const & slot)
{
	NOT_NULL(mount);
	
	bool detachedRider = false;

	//-- If this object is a mount and has an object in its rider slot,
	//   remove that object from the container now.  It will be a player and must
	//   be removed prior to the mount hitting ServerObject::permanentlyDestroyObject().
	SlottedContainer * const slottedContainer = ContainerInterface::getSlottedContainer(*mount);
	if (slottedContainer)
	{
		//-- Check for a rider.
		Container::ContainerErrorCode errorCode = Container::CEC_Success;
		CachedNetworkId riderId = slottedContainer->getObjectInSlot(slot, errorCode);
		if (errorCode == Container::CEC_Success)
		{
			ServerObject *const rider = safe_cast<ServerObject*>(riderId.getObject());
			if (rider)
			{
				// Turn off mounted state on rider.
				CreatureObject *const creatureRider = rider->asCreatureObject();
				if (creatureRider)
					creatureRider->setState(States::RidingMount, false);

				// Allow dismounted rider to participate in CollisionWorld.
				CollisionProperty *const collisionProperty = rider->getCollisionProperty();
				if (collisionProperty)
				{
					collisionProperty->setDisableCollisionWorldAddRemove(false);
					if (mount->isInWorld())
						CollisionWorld::addObject(rider);
				}

				// Rider should be authoritative if on the mount and the mount is authoritative.
				if (!rider->isAuthoritative())
				{
					// Something really screwy happened here --- the mount was authoritative but the
					// rider, contained by the mount, was not authoritative.  Sounds like a container
					// authority bug.
					WARNING(true, ("CreatureObject::realDetachRider(): server id=[%d], mount id=[%s], rider id=[%s], not ejecting rider because rider is not authoritative.",
						static_cast<int>(GameServer::getInstance().getProcessId()),
						mount->getNetworkId().getValueString().c_str(),
						rider->getNetworkId().getValueString().c_str()));
				}
				else
				{
					// Transfer rider to world.
					// @todo: -TRF- transfer to mount's cell if we allow mounts inside cells.
					IGNORE_RETURN(ContainerInterface::transferItemToWorld(*rider, mount->getTransform_o2w(), nullptr, errorCode));
					if (errorCode == Container::CEC_Success)
					{
						detachedRider = true;

						// Add rider to CollisionWorld.  We removed the rider from CollisionWorld
						// when the rider mounted the creature.
						CollisionWorld::addObject(rider);
					}
					else
					{
						DEBUG_WARNING(errorCode != Container::CEC_Success, ("mount id=[%s] failed to force unmount the rider when removed from the world, container error code [%d].", mount->getNetworkId().getValueString().c_str(), static_cast<int>(errorCode)));
					}
					rider->updatePositionOnPlanetServer(true); // Force an update to the Planet Server, in case we delete the mount next

					if (detachedRider && !ServerWorld::isSpaceScene() && mount->isInWorldCell())
					{
						/*
						 * Force the player to the position of the mount. This is intended to prevent
						 * the player glitching through a wall. We perform some correction on the
						 * Y/height position to ensure that they can still move when they dismount over water.
						 */
						Vector mountPosition = mount->getPosition_w();
						Footprint * riderFootprint = collisionProperty ? collisionProperty->getFootprint() : nullptr;

						float groundPosition;
						if (!riderFootprint || riderFootprint->isOnSolidFloor() || !TerrainObject::getConstInstance()->getHeightForceChunkCreation(mountPosition, groundPosition))
						{
							// If the vehicle is on a solid floor (i.e. a building) or we fail to get the position
							// from the terrain instance, we fall back to using the vehicle's Y position.
							groundPosition = mountPosition.y;
						}

						rider->teleportObject(Vector(mountPosition.x, groundPosition, mountPosition.z), NetworkId::cms_invalid, "", Vector(), "");
					}
				}
			}
		}
	}
	return detachedRider;
}

// ======================================================================

using namespace CreatureObjectNamespace;

// ======================================================================
// CreatureObject: PRIVATE STATIC
// ======================================================================
/**
 * Install the mounts code.
 *
 * This function should be called only from CreatureObject::install().
 */

void CreatureObject::installForMounts()
{
	s_riderSlotId[0] = SlotIdManager::findSlotId(ConstCharCrcLowerString(cs_riderSlotName));

	char slotName[256];

	for (int i = 1; i < cs_totalNumberOfRiders; ++i)
	{
		snprintf(slotName, 255, "%s%d", cs_riderSlotName, i);
		s_riderSlotId[i] = SlotIdManager::findSlotId(ConstCharCrcLowerString(slotName));
	}
}	

// ======================================================================
// CreatureObject: PUBLIC
// ======================================================================

/**
 * If this is the driver of a vehicle/mount, transfer the rider
 * position to the mount.
 *
 * Note: this function must be called prior to the rider's
 * m_commandQueue->update() function.  This is because the
 * command queue might get a message that has to look at the
 * mounted vehicle's position (e.g. during a dismount), which
 * will not yet be updated until this function is called.
 */

void CreatureObject::transferRiderPositionToMount()
{
	//-- This function only applies to the driving rider of a mount.
	if (!getState(States::RidingMount))
		return;

	//-- Ensure we can get to this rider's vehicle/mount.
	CreatureObject *const mountObject = getMountedCreature();
	if (!mountObject)
	{
		LOG("mounts-bug", ("CreatureObject::transferRiderPositionToMount(): server id=[%d],object id=[%s] has state RidingMount but getMountedCreature() returns nullptr.", static_cast<int>(GameServer::getInstance().getProcessId()), getNetworkId().getValueString().c_str()));
		emergencyDismountForRider();
		return;
	}

	if (mountObject->getPrimaryMountingRider() != this)
	{
		return;
	}

	//-- Set mount to the same cell and o2p transform as the rider.
	//   Note: this code assumes there is no parent-child relationship
	//   between the rider and the mount on the server.
	CellProperty *const riderCellProperty = getParentCell();
	bool const cellChanged = (mountObject->getParentCell() != riderCellProperty);

	if (cellChanged)
		mountObject->setParentCell(riderCellProperty);

	// Only assign new o2p transform to mount if it is different from the rider's; otherwise,
	// identical information gets flooded to all client proxys of the mount due to other code
	// assuming the transform actually changed.  Note: we might want to put the change detection
	// logic at a lower level so code like this doesn't have to duplicate this functionality
	// everywhere.
	Transform const &riderTransform_o2p = getTransform_o2p();
	Transform const &mountTransform_o2p = mountObject->getTransform_o2p();

	bool const transformChanged = (mountTransform_o2p != riderTransform_o2p);
	if (transformChanged)
		mountObject->setTransform_o2p(riderTransform_o2p);

	if (cellChanged || transformChanged)
	{
		// Update the vehicle's collision property if either the cell or the transform changed.
		// This is effectively warping the vehicle so that the server-side collision doesn't
		// get stuck on something that the rider's client didn't get stuck on.  I do not use
		// the warp command on CollisionProperty because it has several other side effects that
		// are heavier than necessary for updating the previous position.  If this code ever has
		// problems, consider using the CollisionProperty warp but watch for performance issues.
		CollisionProperty *const mountCollisionProperty = mountObject->getCollisionProperty();
		if (mountCollisionProperty)
			mountCollisionProperty->setLastPos(riderCellProperty, riderTransform_o2p);
	}
}

// ----------------------------------------------------------------------
/** 
 * Handle alter for mounts functionality, specifically for authoritative
 * CreatureObjects.
 */

void CreatureObject::alterAuthoritativeForMounts()
{
	DEBUG_FATAL(!isAuthoritative(), ("This should only be called from CreatureObject::alter() when isAuthoratative() is true."));

	//-- Handle rider or mount incapacitation case.
	Postures::Enumerator const currentPosture = getPosture();
	bool const shouldBeDismounted = (currentPosture != Postures::Upright);

	if (shouldBeDismounted)
	{
		//-- Check for a rider on a mount.
		if (getState(States::RidingMount))
		{
			DEBUG_LOG(cs_mountInfoChannelName, ("rider id=[%s] is being forced to dismount because rider is in posture [%s]", getNetworkId().getValueString().c_str(), Postures::getPostureName(currentPosture)));
			
			// Get the mount, request a dismount.
			CreatureObject *const mountObject = getMountedCreature();
			if (mountObject)
			{
				bool const detachSuccess = mountObject->detachRider(getNetworkId());
				if (!detachSuccess)
					LOG(cs_mountErrorChannelName, ("mount id=[%s] failed to successfully detach the rider during posture-initiated dismount.", mountObject->getNetworkId().getValueString().c_str()));
				else
				{
					DEBUG_LOG(cs_mountInfoChannelName, ("mount id=[%s] successfully detached the rider during posture-initiated dismount.", mountObject->getNetworkId().getValueString().c_str()));
				}
			}
			else
				LOG(cs_mountErrorChannelName, ("unexpected: rider object id=[%s],template=[%s] is reporting state States::RidingMount turned on but getMountedCreature() returned nullptr.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		}
		else if (getState(States::MountedCreature))
		{
			DEBUG_LOG(cs_mountInfoChannelName, ("mount id=[%s] is being forced to dismount the rider because the mount is in posture [%s]", getNetworkId().getValueString().c_str(), Postures::getPostureName(currentPosture)));

			// This creature is the mount.  Dismount its rider.
			bool const detachSuccess = detachAllRiders();
			if (!detachSuccess)
				LOG(cs_mountErrorChannelName, ("mount id=[%s] failed to successfully detach the rider during posture-initiated dismount.", getNetworkId().getValueString().c_str()));
			else
			{
				DEBUG_LOG(cs_mountInfoChannelName, ("mount id=[%s] successfully detached the rider during posture-initiated dismount.", getNetworkId().getValueString().c_str()));
			}
		}
	}
}

// ----------------------------------------------------------------------

static void BroadcastSystemMessageToClusterWithLimiter(char const *message)
{
	if (ConfigServerGame::getMountsSanityCheckerBroadcastEnabled())
	{
		static int requestCount = 0;
	
		++requestCount;
		if ((requestCount > 0) && (requestCount <= ConfigServerGame::getMountsSanityCheckerBroadcastLimit()))
			Chat::broadcastSystemMessageToCluster(Unicode::narrowToWide(message), Unicode::emptyString);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::alterAnyForMounts()
{
	//LogMovement(this);

	if (ConfigServerGame::getMountsSanityCheckerEnabled())
	{
		// NOTE: it is important to remember that we need to ignore the
		//       States::MountedCreature and States::RidingMount states
		//       when dealing with proxy objects.
		//
		//       Proxy objects for the mount and rider can and usually do
		//       get the state changes on a different frame than the
		//       container change.

		//-- Handle sanity checking from a mount's point of view.
		
		// Check if the mount has riders.

		typedef std::vector<CreatureObject const *> Riders;
		Riders riders;

		getMountingRiders(riders);

		Riders::const_iterator ii = riders.begin();
		Riders::const_iterator iiEnd = riders.end();

		for (; ii != iiEnd; ++ii)
		{
			CreatureObject const *const rider = *ii;

			if (rider)
			{
				bool triggeredWarning = false;
				
				// Ensure the rider points back to this mount.
				if (rider->getMountedCreature() != this)
				{
					WARNING(true,
									("mounts sanity check failure: game server id=[%d], mount id=[%s,%s], rider id=[%s,%s], rider's mount address=[%p]: mount has rider but rider says it is mounted by something other than this mount.",
									 static_cast<int>(GameServer::getInstance().getProcessId()),
									 getNetworkId().getValueString().c_str(),
									 isAuthoritative() ? "authoritative" : "proxy",
									 rider->getNetworkId().getValueString().c_str(),
									 rider->isAuthoritative() ? "authoritative" : "proxy",
									 rider->getMountedCreature()
									 ));
					triggeredWarning = true;
				}

				// Ensure this mount has the MountedCreature state (only do check on authoritative mount).
				if (isAuthoritative() && !getState(States::MountedCreature))
				{
					WARNING(true,
									("mounts sanity check failure: game server id=[%d], mount id=[%s,%s], rider id=[%s,%s]: authoritative mount does not have MountedCreature state set but getMountingRider returned a non-nullptr value.",
									 static_cast<int>(GameServer::getInstance().getProcessId()),
									 getNetworkId().getValueString().c_str(),
									 isAuthoritative() ? "authoritative" : "proxy",
									 rider->getNetworkId().getValueString().c_str(),
									 rider->isAuthoritative() ? "authoritative" : "proxy"
									 ));
					triggeredWarning = true;
				}

				// Broadcast system message to the cluster.
				if (triggeredWarning)
				{
					char buffer[1024];
					IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "Attention player %s: please stop whatever you're doing and contact Todd Fiala at x382 (Austin) to address an issue we need to track down related to mount %s.  If this player just logged out, please tell them to contact me.",
									 Unicode::wideToNarrow(rider->getObjectName()).c_str(),
									 Unicode::wideToNarrow(getObjectName()).c_str()
									 ));
					buffer[sizeof(buffer) - 1] = '\0';
					BroadcastSystemMessageToClusterWithLimiter(buffer);
				}
			}
			else
			{
				// Ensure we don't have the mounted creature state set (only do check on authoritative mount).
				
				WARNING(isAuthoritative() && getState(States::MountedCreature),
								("mounts sanity check failure: game server id=[%d], mount id=[%s,%s]: authoritative mount does have MountedCreature state set but getMountingRider() returned a nullptr value.",
								 static_cast<int>(GameServer::getInstance().getProcessId()),
								 getNetworkId().getValueString().c_str(),
								 isAuthoritative() ? "authoritative" : "proxy"
								 ));
			}
		}

		//-- Handle sanity checking from a rider's point of view.

		/*
		// Check if the rider has a mount.
		CreatureObject const *const mount = getMountedCreature();
		if (mount)
		{
			bool triggeredWarning = false;
			
			// Ensure the mount has this rider as one of its riders.
			// This needs to change once we support multiple riders per mount.

			not just the primary!!!
			if (mount->getPrimaryMountingRider() != this)
			{
				WARNING(true,
								("mounts sanity check failure: game server id=[%d], rider id=[%s,%s], mount id=[%s,%s], mount's rider address=[%p]: rider has mount but mount says its rider is something other than this rider.",
								 static_cast<int>(GameServer::getInstance().getProcessId()),
								 getNetworkId().getValueString().c_str(),
								 isAuthoritative() ? "authoritative" : "proxy",
								 mount->getNetworkId().getValueString().c_str(),
								 mount->isAuthoritative() ? "authoritative" : "proxy",
								 mount->getMountingRider()
								 ));
				triggeredWarning = true;
			}
			
			// Ensure this rider has the RidingMount state (only do check on authoritative rider).
			if (isAuthoritative() && !getState(States::RidingMount))
			{
				WARNING(true,
								("mounts sanity check failure: game server id=[%d], rider id=[%s,%s], mount id=[%s,%s]: authoritative rider does not have RidingMount state set but getMountedCreature() returned a non-nullptr value.",
								 static_cast<int>(GameServer::getInstance().getProcessId()),
								 getNetworkId().getValueString().c_str(),
								 isAuthoritative() ? "authoritative" : "proxy",
								 mount->getNetworkId().getValueString().c_str(),
								 mount->isAuthoritative() ? "authoritative" : "proxy"
								 ));
				triggeredWarning = true;
			}

			// Broadcast system message to the cluster.
			if (triggeredWarning)
			{
				char buffer[1024];
				IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "Attention player %s: please stop whatever you're doing and contact Todd Fiala at x382 (Austin) to address an issue we need to track down related to mount %s.  If this player just logged out, please tell them to contact me.",
								 Unicode::wideToNarrow(getObjectName()).c_str(),
								 Unicode::wideToNarrow(mount->getObjectName()).c_str()
								 ));
				buffer[sizeof(buffer) - 1] = '\0';
				BroadcastSystemMessageToClusterWithLimiter(buffer);
			}
		}
		else
		{
			// Ensure we don't have the RidingMount state set (only do check on authoritative rider).
			WARNING(isAuthoritative() && getState(States::RidingMount),
							("mounts sanity check failure: game server id=[%d], rider id=[%s,%s]: authoritative rider does have RidingMount state set but getMountedCreature() returned a nullptr value.",
							 static_cast<int>(GameServer::getInstance().getProcessId()),
							 getNetworkId().getValueString().c_str(),
							 isAuthoritative() ? "authoritative" : "proxy"
							 ));
		}
		*/
	}
}

// ----------------------------------------------------------------------

bool CreatureObject::onContainerAboutToTransferForMounts(ServerObject const * destination, ServerObject* /* transferer */)
{
	//-- Do not let a mounted rider get detached from a container.
	bool const isRidingMount = getState(States::RidingMount);
	if (!isRidingMount)
		return true;

	//-- We only allow container transfers of a rider while the rider is not yet mounted
	//   (i.e. the container creature has not yet been set.)  We must clear the RidingMount
	//   state on a rider prior to intentionally dismounting the rider (see and use detachRider()
	//   on the mount object.
	bool const canChangeContainerNow = (getMountedCreature() == nullptr);
	if (!canChangeContainerNow)
	{
		LOG("mounts-bug", ("CO::onContainerAboutToTransferForMounts():server id=[%d],rider id=[%s] erroneously tried to transfer the player into object id=[%s].",
											 static_cast<int>(GameServer::getInstance().getProcessId()),
											 getNetworkId().getValueString().c_str(),
											 (destination == nullptr) ? "<nullptr (world cell)>" : destination->getNetworkId().getValueString().c_str()));
	}
	return canChangeContainerNow;
}

// ----------------------------------------------------------------------
/**
 * Determine if this creature can support becoming a mount.
 *
 * This function only checks the potential for the CreatureObject to
 * become a mount.  To check if this creature is a mount, use
 * isMountable().
 */

int CreatureObject::getMountabilityStatus() const
{
	//-- Ensure mounts are enabled.
	if (!ConfigServerGame::getMountsEnabled())
	{
		LOG(cs_mountInfoChannelName, ("CreatureObject::canBeMadeMountable() called on object id=[%s] when mounts are disabled, ignoring call.", getNetworkId().getValueString().c_str()));
		return static_cast<int>(MountValidScaleRangeTable::MS_speciesUnmountable);
	}

	//-- Indicate we're mountable if we've already been made mountable.
	if (isMountable())
		return static_cast<int>(MountValidScaleRangeTable::MS_creatureMountable);

	SharedObjectTemplate const *const sharedObjectTemplate = getSharedTemplate();

	// Note: multi-seater mount support will need to change saddle capacity.
	int   const saddleCapacity  = 1;
	float const appearanceScale = getScaleFactor();

	MountValidScaleRangeTable::MountabilityStatus const result = MountValidScaleRangeTable::doesCreatureSupportScaleAndSaddleCapacity(TemporaryCrcString(sharedObjectTemplate->getAppearanceFilename().c_str(), true), appearanceScale, saddleCapacity);
	if (result != MountValidScaleRangeTable::MS_creatureMountable)
		return static_cast<int>(result);

	//-- Get the slotted container.
	SlottedContainer const * const slottedContainer = ContainerInterface::getSlottedContainer(*this);

	int const maxSlots = getSaddleSeatingCapacity(this);
	bool hasRequiredSlot = false;

	for (int i = 0; i < maxSlots; ++i)
	{
		hasRequiredSlot = slottedContainer && slottedContainer->hasSlot(s_riderSlotId[i]);

		if (hasRequiredSlot)
		{
			break;
		}
	}

	return (hasRequiredSlot ? static_cast<int>(MountValidScaleRangeTable::MS_creatureMountable) : static_cast<int>(MountValidScaleRangeTable::MS_speciesMountableMissingRiderSlot));
}

// ----------------------------------------------------------------------
/**
 * Convert a pet into a mountable-pet.
 *
 * This function will DEBUG_FATAL if canBeMadeMountable() returns false.
 * The caller should have already checked canBeMadeMountable().
 *
 * This function flips the TangibleObject condition flag for C_mount.
 */

void CreatureObject::makePetMountable()
{
	//-- Ensure mounts are enabled.
	if (!ConfigServerGame::getMountsEnabled())
	{
		LOG(cs_mountInfoChannelName, ("CreatureObject::makePetMountable() called on object id=[%s] when mounts are disabled, ignoring call.", getNetworkId().getValueString().c_str()));
		return;
	}
	
	//-- Validate preconditions.
	DEBUG_FATAL(!isAuthoritative(), ("makePetMountable(): called on non-authoritative creature id=[%s],template=[%s].", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	DEBUG_FATAL(getMountabilityStatus() != static_cast<int>(MountValidScaleRangeTable::MS_creatureMountable), ("makePetMountable(): creature id=[%s],template=[%s] this creature cannot be made mountable. Caller should have checked getMountabilityStatus() first.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	DEBUG_FATAL(isMountable(), ("makePetMountable(): creature id=[%s],template=[%s] already is mountable.  Caller doesn't need to call makePetMountable().", getNetworkId().getValueString().c_str(), getObjectTemplateName()));

	//-- Get the pet control device network id.
	NetworkId const pcdNetworkId = getPetControlDeviceId();
	if (!pcdNetworkId.isValid())
	{
		DEBUG_FATAL(true, ("makePetMountable(): creature id=[%s],template=[%s] does not have a valid pet control device id on it. Expecting script to place this objvar on the pet.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		return;
	}

	//-- Set the Condition bit to indicate we're a mount.
	setCondition(getCondition() | static_cast<int>(ServerTangibleObjectTemplate::C_mount));

	//-- Set the Creature as static so that it doesn't count against the spawn limit.
	setIsStatic(true);
}

// ----------------------------------------------------------------------
/**
 * Determine if this creature is a mount.
 */

bool CreatureObject::isMountable() const
{
	if (ConfigServerGame::getMountsEnabled())
	{
		return (getCondition() & static_cast<int>(ServerTangibleObjectTemplate::C_mount)) != 0;
	}

	return false;
}

// ----------------------------------------------------------------------

bool CreatureObject::isMountableAndHasRoomForAnother() const
{
	bool hasEmptySlot = false;

	if (ConfigServerGame::getMountsEnabled())
	{
		if (isMountable())
		{
			//-- Get the slotted container.
			SlottedContainer const * const slottedContainer = ContainerInterface::getSlottedContainer(*this);

			int const maxSlots = getSaddleSeatingCapacity(this);

			for (int i = 0; i < maxSlots; ++i)
			{
				if (slottedContainer && slottedContainer->hasSlot(s_riderSlotId[i]))
				{
					Container::ContainerErrorCode error = Container::CEC_Success;

					if (slottedContainer->isSlotEmpty(s_riderSlotId[i], error) && error == Container::CEC_Success)
					{
						hasEmptySlot = true;
						break;
					}
				}
			}
		}
	}

	return hasEmptySlot;
}

//----------------------------------------------------------------------
/**
 * Assuming this CreatureObject is a pet or mount, get the network id
 * of the associated pet control device's id.
 *
 * This function gets the network id from the obj_id pet.controlDevice
 * objvar placed on all pets by the pet script-based system.
 */

NetworkId CreatureObject::getPetControlDeviceId() const
{
	//-- Retrieve the list of objvars on the creature.
	DynamicVariableList const &variableList = getObjVars();

	//-- Get pet control device's network id.
	NetworkId petControlDeviceId;
	
	if (variableList.getItem(cs_petControlDeviceObjVarName, petControlDeviceId))
		return petControlDeviceId;
	else
		return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

bool CreatureObject::detachRider(NetworkId const riderId)
{
	//-- Ensure mounts are enabled.
	if (!ConfigServerGame::getMountsEnabled())
		return false;

	//-- Only do this on the authoritative mount.
	if (!isAuthoritative())
	{
		typedef MessageQueueGenericValueType<NetworkId> Message;
		Message * const msg = new Message(riderId);
		sendControllerMessageToAuthServer(CM_detachRiderForMount, msg);
		return true;
	}

	//-- Validate preconditions.
	DEBUG_FATAL(!isMountable(), ("CreatureObject::detachRider(): called on an object id=[%s],template=[%s] that is not mountable, makes no sense.", getNetworkId().getValueString().c_str()));

	int const maxSlots = getSaddleSeatingCapacity(this);
	bool forceDetachAll = false; // incase riderId is the first person
	bool detachedRider = false;

	for (int i = 0; i < maxSlots; ++i)
	{
		CreatureObject const * const rider = realGetMountingRider(this, s_riderSlotId[i]);

		bool const detach = ((rider != 0) && (rider->getNetworkId() == riderId)) || forceDetachAll;

		if (detach)
		{
			detachedRider = realDetachRider(this, s_riderSlotId[i]);
			if (i == 0)
			{
				forceDetachAll = true;
			}
		}
	}

	if (forceDetachAll)
	{
		//-- Turn off MountedCreature state on mount.
		setState(States::MountedCreature, false);
	}

	//-- Set mount's AlterScheduler schedule phase back to zero so that
	//   the mount gets altered along with everything else.
	AlterScheduler::setObjectSchedulePhase(*this, 0);

	updatePositionOnPlanetServer(true);

	return detachedRider;
}

// ----------------------------------------------------------------------

bool CreatureObject::detachAllRiders()
{
	//-- Ensure mounts are enabled.
	if (!ConfigServerGame::getMountsEnabled())
		return false;

	//-- Only do this on the authoritative mount.
	if (!isAuthoritative())
	{
		// add a plural message
		sendControllerMessageToAuthServer(CM_detachAllRidersForMount, nullptr);
		return true;
	}

	//-- Validate preconditions.
	DEBUG_FATAL(!isMountable(), ("CreatureObject::detachRider(): called on an object id=[%s],template=[%s] that is not mountable, makes no sense.", getNetworkId().getValueString().c_str()));

	int const maxSlots = getSaddleSeatingCapacity(this);
	int numberOfSuccesses = 0;

	for (int i = 0; i < maxSlots; ++i)
	{
		if (realDetachRider(this, s_riderSlotId[i]))
		{
			++numberOfSuccesses;
		}
	}

	bool const detachedRider = numberOfSuccesses == maxSlots;

	//-- Turn off MountedCreature state on mount.
	setState(States::MountedCreature, false);

	//-- Set mount's AlterScheduler schedule phase back to zero so that
	//   the mount gets altered along with everything else.
	AlterScheduler::setObjectSchedulePhase(*this, 0);

	return detachedRider;
}

// ----------------------------------------------------------------------

CreatureObject const *CreatureObject::getPrimaryMountingRider() const
{
	return const_cast<CreatureObject*>(this)->getPrimaryMountingRider();
}

// ----------------------------------------------------------------------

CreatureObject * CreatureObject::getPrimaryMountingRider()
{
	return realGetMountingRider(this, s_riderSlotId[0]);
}

// ----------------------------------------------------------------------

void CreatureObject::getMountingRiders(std::vector<const CreatureObject *> & riders) const
{
	int const maxSlots = getSaddleSeatingCapacity(this);

	for (int i = 0; i < maxSlots; ++i)
	{
		CreatureObject const * const rider = realGetMountingRider(this, s_riderSlotId[i]);

		if (rider != 0)
		{
			riders.push_back(rider);
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::getMountingRiders(std::vector<CreatureObject *> & riders)
{
	int const maxSlots = getSaddleSeatingCapacity(this);

	for (int i = 0; i < maxSlots; ++i)
	{
		CreatureObject * const rider = realGetMountingRider(this, s_riderSlotId[i]);

		if (rider != 0)
		{
			riders.push_back(rider);
		}
	}
}


// ----------------------------------------------------------------------
/**
 * Instruct this rider to ride on the specified mount.
 *
 * @param mountObject  the CreatureObject that will be ridden by this
 *                     instance.
 *
 * @return  true if this rider was able to mount the creature; false otherwise.
 */

bool CreatureObject::mountCreature(CreatureObject &mountObject)
{
	//-- Ensure mounts are enabled.
	if (!ConfigServerGame::getMountsEnabled())
	{
		LOG(cs_mountInfoChannelName, ("CreatureObject::mountCreature() called on rider object id=[%s] when mounts are disabled, ignoring call.", getNetworkId().getValueString().c_str()));
		return false;
	}

	//-- Ensure we (the rider) are not already mounted.
	if (getState(States::RidingMount))
	{
		DEBUG_FATAL(true, ("mountCreature(): rider id=[%s] template=[%s] is already riding a mount.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		return false;
	}

	//-- Ensure the mount is mountable.
	if (!mountObject.isMountable())
	{
		DEBUG_FATAL(true, ("mountCreature(): specified mountObject id=[%s] template=[%s] is not mountable.", mountObject.getNetworkId().getValueString().c_str(), mountObject.getObjectTemplateName()));
		return false;
	}

	//-- Ensure both the mount and the rider are authoritative on this server.
	//   (They should be by design of how this function gets called.)
	if (!isAuthoritative() || !mountObject.isAuthoritative())
	{
		DEBUG_WARNING(true, ("mountCreature(): rider id=[%s] or mount id=[%s] was not authoritative on this server, mounting aborted.", getNetworkId().getValueString().c_str(), mountObject.getNetworkId().getValueString().c_str()));
		return false;
	}

	//-- Reject if the player is not in the world cell
	if (!isInWorldCell())
	{
		DEBUG_WARNING(true, ("mountCreature(): rider id=[%s] is not in the world cell, mounting aborted.", getNetworkId().getValueString().c_str()));
		return false;
	}

	//-- Reject if the mount is not in the world cell
	if (!mountObject.isInWorldCell())
	{
		DEBUG_WARNING(true, ("mountCreature(): mount id=[%s] is not in the world cell, mounting aborted.", mountObject.getNetworkId().getValueString().c_str()));
		return false;
	}

	// ... set the new mount-related states on the rider and mount.  This is necessary
	//     so that I can efficiently handle CreatureObject::onContainerTransferComplete,
	//     which is called before transferItemToSlottedContainer() completes.
	setState(States::RidingMount, true);
	mountObject.setState(States::MountedCreature, true);
	
	Container::ContainerErrorCode errorCode = Container::CEC_Success;
	bool transferSuccess = false;

	// transfer into the first available slot

	int const maxSlots = getSaddleSeatingCapacity(&mountObject);

	for (int i = 0; i < maxSlots; ++i)
	{
		if (ContainerInterface::canTransferToSlot(mountObject, *this, s_riderSlotId[i], nullptr, errorCode))
		{
			transferSuccess = ContainerInterface::transferItemToSlottedContainer(mountObject, *this, s_riderSlotId[i], nullptr, errorCode);

			if ((transferSuccess) && (errorCode == Container::CEC_Success))
			{
				break;
			}
		}
	}
	
	DEBUG_FATAL(transferSuccess && (errorCode != Container::CEC_Success), ("mountCreature(): transferItemToSlottedContainer() returned success but container error code returned error %d.", static_cast<int>(errorCode)));

	if (!transferSuccess)
	{
		// Clear these states so that the objects are not in a funky bad state.
		setState(States::RidingMount, false);

		if (mountObject.getPrimaryMountingRider() == 0)
		{
			// clear state if empty mount
			mountObject.setState(States::MountedCreature, false);
		}

		return transferSuccess;
	}

	//-- Take the rider out of CollisionWorld.
	// @todo: add all object non-auto-delta change info to another function
	//        that can be called via controller message to proxies, end baselines
	//        on proxies and from here.
	if (isInWorld())
		CollisionWorld::removeObject(this);

	CollisionProperty *const collisionProperty = getCollisionProperty();
	if (collisionProperty)
		collisionProperty->setDisableCollisionWorldAddRemove(true);

	//-- Set the mount's AlterScheduler phase to 1 so that it gets an alter after the rider.
	//   The rider will count on this so that the rider can update the mount's server position
	//   prior to the mount getting altered that frame.
	AlterScheduler::setObjectSchedulePhase(mountObject, 1);

	//-- Tell rider to update movement info so it pulls walk/run speed from the mount.
	updateMovementInfo();
	
	//-- Indicate transfer success.
	return transferSuccess;
}

//----------------------------------------------------------------------

CreatureObject const *CreatureObject::getMountedCreature() const
{
	return const_cast<CreatureObject*>(this)->getMountedCreature();
}

//----------------------------------------------------------------------

CreatureObject *CreatureObject::getMountedCreature()
{
	//-- Ensure mounts are enabled.
	if (!ConfigServerGame::getMountsEnabled())
		return nullptr;

	//-- Check this creature's container object.
	ServerObject *const container = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*this));
	if (!container)
		return nullptr;

	//-- Check if the container is a creature.
	CreatureObject *const creatureContainer = container->asCreatureObject();
	if (!creatureContainer)
		return nullptr;

	//-- Ignore non-mountable creature objects.
	CreatureObject *const mount = creatureContainer->isMountable() ? creatureContainer : nullptr;
	return mount;
}

// ----------------------------------------------------------------------
/**
 * Cleanup the mount-related state on this rider such that they no longer
 * act as if mounted.
 *
 * DO NOT CALL THIS UNLESS YOU KNOW THAT THE INTERNAL MOUNTS STATE IS SOMEHOW
 * OUT OF SYNC.
 *
 * The purpose for this emergency function is to handle the case where internal
 * mount-related state is no longer consistent.  This function is intended to be
 * called when a rider appears to be mounted (via the RidingMount state) but
 * is not contained by its mount.
 */

void CreatureObject::emergencyDismountForRider()
{
	LOG("mounts-bug", ("CreatureObject::emergencyDismountRider() called, server id=[%d], rider id=[%s], rider is [%s]", static_cast<int>(GameServer::getInstance().getProcessId()), getNetworkId().getValueString().c_str(), isAuthoritative() ? "authoritative" : "proxy"));

	//-- Ensure we have the RidingMount state.
	if (!getState(States::RidingMount))
	{
		WARNING(true, ("CreatureObject::emergencyDismountRider() called when RidingMount state is not set.  Ignoring call."));
		return;
	}

	//-- Pass request along to authoritative server if we're not it.
	if (!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_emergencyDismountForRider, nullptr);
		return;
	}
	
	//-- Try to get the mount.
	CreatureObject *const mount = getMountedCreature();
	if (mount)
	{
		IGNORE_RETURN(mount->detachRider(getNetworkId()));
		return;
	}

	// NOTE: if we've made it this far, the rider appears to be mounted (by states) but the
	// containment relationship has been broken erroneously.  Try to fixup the player from here.

	//-- Set state to be not riding.
	setState(States::RidingMount, false);

	//-- Transfer the rider to the world cell.
	Container::ContainerErrorCode  errorCode = Container::CEC_Success;
	bool const transferSucceeded = ContainerInterface::transferItemToWorld (*this, this->getTransform_o2w(), nullptr, errorCode);
	WARNING(!transferSucceeded || (errorCode != Container::CEC_Success), ("Transfer to world failed, return value=[%s], error code=[%d].", transferSucceeded ? "true" : "false", static_cast<int>(errorCode)));
}

// ======================================================================
