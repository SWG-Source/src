// ServerWorld.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerWorld.h"

#include "SwgGameServer/CombatEngine.h"
#include "serverGame/AiCombatPulseQueue.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/AuthTransferTracker.h"
#include "serverGame/CellObject.h"
#include "serverGame/CollisionCallbacks.h"
#include "serverGame/CombatTracker.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/ContainmentMessageManager.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DestroyMessageManager.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/FormManagerServer.h"
#include "serverGame/GameServer.h"
#include "serverGame/GameServerMessageArchive.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/LineOfSightCache.h"
#include "serverGame/LogoutTracker.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/MissileManager.h"
#include "serverGame/NamedObjectManager.h"
#include "serverGame/NebulaManagerServer.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PlayerSanityChecker.h"
#include "serverGame/PositionUpdateTracker.h"
#include "serverGame/ProjectileManager.h"
#include "serverGame/Pvp.h"
#include "serverGame/PvpUpdateObserver.h"
#include "serverGame/Region3dMaster.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ResourcePoolObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerArmorTemplate.h"
#include "serverGame/ServerAsteroidManager.h"
#include "serverGame/ServerBattlefieldMarkerObjectTemplate.h"
#include "serverGame/ServerBuildingObjectTemplate.h"
#include "serverGame/ServerCellObjectTemplate.h"
#include "serverGame/ServerCityObjectTemplate.h"
#include "serverGame/ServerConstructionContractObjectTemplate.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerDraftSchematicObjectTemplate.h"
#include "serverGame/ServerFactoryObjectTemplate.h"
#include "serverGame/ServerGroupObjectTemplate.h"
#include "serverGame/ServerGuildObjectTemplate.h"
#include "serverGame/ServerHarvesterInstallationObjectTemplate.h"
#include "serverGame/ServerInstallationObjectTemplate.h"
#include "serverGame/ServerIntangibleObjectTemplate.h"
#include "serverGame/ServerManufactureInstallationObjectTemplate.h"
#include "serverGame/ServerManufactureSchematicObjectTemplate.h"
#include "serverGame/ServerMissionObjectTemplate.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerObjectBaselinesManager.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerPlanetObjectTemplate.h"
#include "serverGame/ServerPlayerQuestObjectTemplate.h"
#include "serverGame/ServerPlayerObjectTemplate.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerResourceContainerObjectTemplate.h"
#include "serverGame/ServerShipObjectTemplate.h"
#include "serverGame/ServerStaticObjectTemplate.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerVehicleObjectTemplate.h"
#include "serverGame/ServerWeaponObjectTemplate.h"
#include "serverGame/ServerWorldIntangibleNotification.h"
#include "serverGame/ServerWorldTangibleNotification.h"
#include "serverGame/ServerWorldTerrainObjectNotification.h"
#include "serverGame/ServerWorldUniverseNotification.h"
#include "serverGame/ServerXpManagerObjectTemplate.h"
#include "serverGame/ShipAiEnemySearchManager.h"
#include "serverGame/ShipClientUpdateTracker.h"
#include "serverGame/ShipInternalDamageOverTimeManager.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpacePathManager.h"
#include "serverGame/SpaceSquadManager.h"
#include "serverGame/TeleportFixupHandler.h"
#include "serverGame/TriggerVolume.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/CreateNewObjectMessage.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/PlanetRemoveObject.h"
#include "serverNetworkMessages/UpdateObjectOnPlanetMessage.h"
#include "serverPathfinding/ServerPathfinding.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/SetupSharedCollision.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Scheduler.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/SharedBattlefieldMarkerObjectTemplate.h"
#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedCellObjectTemplate.h"
#include "sharedGame/SharedConstructionContractObjectTemplate.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedDraftSchematicObjectTemplate.h"
#include "sharedGame/SharedFactoryObjectTemplate.h"
#include "sharedGame/SharedGroupObjectTemplate.h"
#include "sharedGame/SharedGuildObjectTemplate.h"
#include "sharedGame/SharedInstallationObjectTemplate.h"
#include "sharedGame/SharedIntangibleObjectTemplate.h"
#include "sharedGame/SharedManufactureSchematicObjectTemplate.h"
#include "sharedGame/SharedMissionObjectTemplate.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedPlayerQuestObjectTemplate.h"
#include "sharedGame/SharedPlayerObjectTemplate.h"
#include "sharedGame/SharedResourceContainerObjectTemplate.h"
#include "sharedGame/SharedShipObjectTemplate.h"
#include "sharedGame/SharedStaticObjectTemplate.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedGame/SharedTerrainSurfaceObjectTemplate.h"
#include "sharedGame/SharedUniverseObjectTemplate.h"
#include "sharedGame/SharedVehicleObjectTemplate.h"
#include "sharedGame/SharedWeaponObjectTemplate.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/LotManager.h"
#include "sharedObject/NetworkController.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortallizedSphereTree.h"
#include "sharedObject/SphereGrid.h"
#include "sharedObject/PortalProperty.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/Location.h"
#include "sharedUtility/SynchronizedWeatherGenerator.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedLog/Log.h"

Object const *getContainingPobForObjectInWorld(Object const &object)
{
	Object const * const containingObject = ContainerInterface::getContainedByObject(object);
	if (containingObject)
	{
		Object const * const portallizedObject = ContainerInterface::getTopmostContainer(*containingObject);
		FATAL(!portallizedObject, ("no topmost container of container of object in world?"));
		// Note: in the case of object being a creature riding a mount,
		// portallizedObject is not actually portallized, but returning 0
		// for the world cell is correct in that case so the result is
		// still correct.
		if (portallizedObject->getPortalProperty())
			return portallizedObject;
	}
	return 0;
}

void compare_results_int(std::set<TriggerVolume *> &results, const std::set<TriggerVolume *> &results2, ServerObject* object)
{
	if (results.empty() && results2.empty())    // nothing to say........
		return;

	Vector v = object->getPosition_w();
	const Object* p = getContainingPobForObjectInWorld(*object);

	if (results == results2)
	{
		return;
	}

	LOG("SphereGrid", ("---- Results differ for object at (%f %f %f)  %p --------", v.x, v.y, v.z, p));

	std::set<TriggerVolume*>::iterator iter;

	LOG("SphereGrid", ("=============== Tree Results %d =================", results.size()));
	for (iter = results.begin(); iter != results.end(); ++iter)
	{
		TriggerVolume* volume = *iter;
		ServerObject const &volumeOwner = volume->getOwner();
		const Object* pob = getContainingPobForObjectInWorld(volume->getOwner());
		Sphere const &localSphere = volumeOwner.getLocalSphere();
		Sphere world(volumeOwner.getTransform_o2w().rotateTranslate_l2p(localSphere.getCenter()), volume->getRadius());
		Vector c = world.getCenter();
		LOG("SphereGrid", (" (%f %f %f) %f POB = %p  DIST=%f", c.x, c.y, c.z, world.getRadius(), pob, c.magnitudeBetween(v)));
	}
	LOG("SphereGrid", ("------------------ Grid Results %d ------------------", results2.size()));
	for (iter = results2.begin(); iter != results2.end(); ++iter)
	{
		TriggerVolume* volume = *iter;
		ServerObject const &volumeOwner = volume->getOwner();
		const Object* pob = getContainingPobForObjectInWorld(volume->getOwner());
		Sphere const &localSphere = volumeOwner.getLocalSphere();
		Sphere world(volumeOwner.getTransform_o2w().rotateTranslate_l2p(localSphere.getCenter()), volume->getRadius());   // o2p or o2w
		Vector c = world.getCenter();
		LOG("SphereGrid", (" (%f %f %f) %f POB = %p  DIST=%f", c.x, c.y, c.z, world.getRadius(), pob, c.magnitudeBetween(v)));
	}
	DEBUG_FATAL(true, ("FATAL: SphereGrid failed to match SphereTree result set."));
}

void compare_results(Capsule const &test, std::vector<TriggerVolume *> &results_in, const std::set<TriggerVolume *> &results2, ServerObject* object)
{
	size_t i;
	std::set<TriggerVolume*> results;
	for (i = 0; i < results_in.size(); ++i)
	{
		TriggerVolume* volume = results_in[i];
		ServerObject const &volumeOwner = volume->getOwner();
		Sphere const &localSphere = volumeOwner.getLocalSphere();
		Sphere world(volumeOwner.getTransform_o2w().rotateTranslate_l2p(localSphere.getCenter()), volume->getRadius());   // o2p or o2w

		if (test.intersectsSphere(world))
		{
			results.insert(results_in[i]);
		}
	}
	compare_results_int(results, results2, object);
}

void compare_results(Vector const &center_w, float radius, std::vector<TriggerVolume *> &results_in, const std::set<TriggerVolume *> &results2, ServerObject* object)
{
	size_t i;
	std::set<TriggerVolume*> results;
	for (i = 0; i < results_in.size(); ++i)
	{
		TriggerVolume* volume = results_in[i];
		ServerObject const &volumeOwner = volume->getOwner();
		Sphere const &localSphere = volumeOwner.getLocalSphere();
		Sphere world(volumeOwner.getTransform_o2w().rotateTranslate_l2p(localSphere.getCenter()), volume->getRadius());   // o2p or o2w

		Sphere test(center_w, radius);
		if (test.intersectsSphere(world))
		{
			results.insert(results_in[i]);
		}
	}
	compare_results_int(results, results2, object);
}

// ======================================================================

namespace ServerWorldNamespace
{
	std::vector<const TangibleObject *> s_loadBeaconEntries;
	bool ms_logTriggerStats = false;

	std::vector<Object *>	  gs_pendingConcludeVector;
	std::vector<std::pair<int, ServerObject *> >    gs_pendingConcludeOpsVector;
	bool			   gs_pendingConcludeLock = false;

	typedef std::pair<Vector, Vector> MovePositionPair;
	typedef std::map<ServerObject*, MovePositionPair> MoveObjectMap;
	typedef std::vector<MoveObjectMap *> MoveObjectList;

	MoveObjectList s_moveObjectList;
	bool	   s_moveObjectListLock = false;
	bool	   s_moveObjectListValid = false;
	bool	   s_preloadComplete = false;
	std::vector<Watcher<ServerObject> > s_preloadCompleteTriggerObjects;

	int	    s_numMoveLists = 0;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool isPlayerHouseHook(Object const * object);
	void issueCollisionNearWarpWarning(Object const &object, Vector const &oldPosition_w, Vector const &newPosition_w, int segmentCount);
	void issueCollisionFarWarpWarning(Object const &object, Vector const &oldPosition_w, Vector const &newPosition_w, int segmentCount);
	void doPreloadCompleteTrigger(ServerObject &obj);
	void updatePlanetServer();
	void checkSpaceBattlefieldZone();

	bool s_checkedForSpaceScene = false; // s_spaceScene can't be trusted unless this is true
	bool s_spaceScene = false;
	bool s_checkedSpaceBattlefieldZone = false;
	bool s_isSpaceBattlefieldZone = false;
	bool s_isSpaceBattlefieldZoneWithPvp = false;
	char const * const s_spaceBattlefieldDataTableFilename = "datatables/space_zones/battlefield_zones.iff";
	int s_planetServerUpdateIndex = 0;
}

using namespace ServerWorldNamespace;

// ======================================================================
// namespace ServerWorldNamespace
// ======================================================================

bool ServerWorldNamespace::isPlayerHouseHook(Object const *object)
{
	if (object && object->getPortalProperty())
	{
		ServerObject const * const serverObject = object->asServerObject();
		if (serverObject
			&& (serverObject->asShipObject()
				|| serverObject->getObjVars().hasItem("player_structure")))
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------

void ServerWorldNamespace::issueCollisionNearWarpWarning(Object const &object, Vector const &oldPosition_w, Vector const &newPosition_w, int segmentCount)
{
	ServerObject const *const serverObject = object.asServerObject();

	//-- Only issue these for authoritative server objects.  Proxy server objects will hit this condition after an intra-planet teleport.
	// @todo allow proxies to know about a teleport and inform CollisionWorld so that we can always report these.
	DEBUG_WARNING(!serverObject || serverObject->isAuthoritative(),
		("CollisionWorld::update() had %d segments for object id=[%s], template=[%s], authority=[%s], game sever id=[%d], start position=[%.2f,%.2f,%.2f], end position=[%.2f,%.2f,%.2f], object probably should have warped but collision system is not warping it.",
			segmentCount,
			object.getNetworkId().getValueString().c_str(),
			object.getObjectTemplateName(),
			serverObject ? (serverObject->isAuthoritative() ? "authoritative" : "proxy") : "<not ServerObject-derived>",
			static_cast<int>(GameServer::getInstance().getProcessId()),
			oldPosition_w.x,
			oldPosition_w.y,
			oldPosition_w.z,
			newPosition_w.x,
			newPosition_w.y,
			newPosition_w.z
			));
}

// ----------------------------------------------------------------------

void ServerWorldNamespace::issueCollisionFarWarpWarning(Object const &object, Vector const &oldPosition_w, Vector const &newPosition_w, int segmentCount)
{
	ServerObject const *const serverObject = object.asServerObject();

	//-- Only issue these for authoritative server objects.  Proxy server objects will hit this condition after an intra-planet teleport.
	// @todo allow proxies to know about a teleport and inform CollisionWorld so that we can always report these.
	DEBUG_WARNING(!serverObject || serverObject->isAuthoritative(),
		("CollisionWorld::update() had %d segments for object id=[%s], template=[%s], authority=[%s], game sever id=[%d], start position=[%.2f,%.2f,%.2f], end position=[%.2f,%.2f,%.2f], collision system will consider this a warp and adjust accordingly.",
			segmentCount,
			object.getNetworkId().getValueString().c_str(),
			object.getObjectTemplateName(),
			serverObject ? (serverObject->isAuthoritative() ? "authoritative" : "proxy") : "<not ServerObject-derived>",
			static_cast<int>(GameServer::getInstance().getProcessId()),
			oldPosition_w.x,
			oldPosition_w.y,
			oldPosition_w.z,
			newPosition_w.x,
			newPosition_w.y,
			newPosition_w.z
			));
}

// ----------------------------------------------------------------------

bool isRelevantToTriggerVolumes(Object const &object)
{
	return object.getObjectTemplate()->getId() == ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag;
}

// ----------------------------------------------------------------------

class PlayerShipFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	PlayerShipFilter(ServerObject const *excludeObject) :
		m_excludeObject(excludeObject)
	{
	}

	bool operator() (ServerObject * const &object) const
	{
		return object != m_excludeObject && object->asShipObject() && object->asShipObject()->isPlayerShip();
	}

private:
	ServerObject const *m_excludeObject;
};

class CreatureFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	CreatureFilter() {}
	bool operator() (ServerObject * const &object) const {
		return (object->getObjectTemplate()->getId() == ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag);
	}
};

class AuthoritativeNonPlayerCreatureFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	AuthoritativeNonPlayerCreatureFilter() {}
	bool operator() (ServerObject * const &object) const {
		return (object->isAuthoritative() && object->asCreatureObject() != nullptr && !object->isPlayerControlled());
	}
};

class TriggerVolumeFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	TriggerVolumeFilter() {}
	bool operator() (ServerObject * const &object) const
	{
		return isRelevantToTriggerVolumes(*object);
	}
};

class TriggerVolumeFilterWithIgnoredObject : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	TriggerVolumeFilterWithIgnoredObject(ServerObject const &ignoredObject) :
		m_ignoredObject(ignoredObject)
	{
	}

	bool operator()(ServerObject *const &object) const
	{
		return object != &m_ignoredObject && isRelevantToTriggerVolumes(*object);
	}

private:
	TriggerVolumeFilterWithIgnoredObject(TriggerVolumeFilterWithIgnoredObject const &);
	TriggerVolumeFilterWithIgnoredObject &operator=(TriggerVolumeFilterWithIgnoredObject const &);

private:
	ServerObject const &m_ignoredObject;
};

class ServerObjectSphereExtentAccessor : public PortallizedSphereTreeAccessor<ServerObject *, ServerObjectSphereExtentAccessor>
{
public:

	static Sphere const getExtent(ServerObject const *object)
	{
		NOT_NULL(object);
		Sphere const &localSphere = object->getLocalSphere();
		return Sphere(object->getTransform_o2p().rotateTranslate_l2p(localSphere.getCenter()), localSphere.getRadius());
	}

	static Object const *getCurrentPob(ServerObject const *object)
	{
		return getContainingPobForObjectInWorld(*NON_NULL(object));
	}
};

//-----------------------------------------------------------------------

class TriggerVolumeSphereExtentAccessor : public PortallizedSphereTreeAccessor<TriggerVolume *, TriggerVolumeSphereExtentAccessor>
{
public:

	static Sphere const getExtent(TriggerVolume const *volume)
	{
		NOT_NULL(volume);
		ServerObject const &volumeOwner = volume->getOwner();
		Sphere const &localSphere = volumeOwner.getLocalSphere();
		return Sphere(volumeOwner.getTransform_o2p().rotateTranslate_l2p(localSphere.getCenter()), volume->getRadius());
	}

	static Object const *getCurrentPob(TriggerVolume const *t)
	{
		NOT_NULL(t);
		return getContainingPobForObjectInWorld(t->getOwner());
	}
};

//-----------------------------------------------------------------------

class TriggerVolumeSphereExtentAccessor_Grid : public PortallizedSphereTreeAccessor<TriggerVolume *, TriggerVolumeSphereExtentAccessor_Grid>
{
public:

	static Sphere const getExtent(TriggerVolume const *volume)
	{
		NOT_NULL(volume);
		ServerObject const &volumeOwner = volume->getOwner();
		Sphere const &localSphere = volumeOwner.getLocalSphere();
		return Sphere(volumeOwner.getTransform_o2w().rotateTranslate_l2p(localSphere.getCenter()), volume->getRadius());
	}

	static Object const *getCurrentPob(TriggerVolume const *t)
	{
		NOT_NULL(t);
		return getContainingPobForObjectInWorld(t->getOwner());
	}
};

//----------------------------------------------------------------------

bool			   ServerWorld::m_installed = false;
Timer *			ServerWorld::m_idleTimer = 0;
std::string *		  ServerWorld::m_sceneId = 0;
SynchronizedWeatherGenerator * ServerWorld::m_weatherGenerator = 0;

//-----------------------------------------------------------------------
// sphere tree instance for the server world static class
//
// g_objectSphereTree has a one-to-one relationship between objects and their
// sphere extents.
//
// g_triggerSphereTree has a many to one relationship between trigger volumes
// and their owner objects.
PortallizedSphereTree<ServerObject *, ServerObjectSphereExtentAccessor> *g_objectSphereTree = 0;

PortallizedSphereTree<TriggerVolume *, TriggerVolumeSphereExtentAccessor> *g_triggerSphereTree = 0;
DoubleSphereGrid<TriggerVolume *, TriggerVolumeSphereExtentAccessor_Grid> *g_triggerSphereGrid = 0;

// ----------------------------------------------------------------------

void ServerWorld::addIntangibleObject(Object * object)
{
	World::addObject(object, static_cast<int>(WOL_Intangible));
}

// ----------------------------------------------------------------------

void ServerWorld::removeIntangibleObject(Object * object)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::removeIntangibleObject");
	IGNORE_RETURN(World::removeObject(object, static_cast<int>(WOL_Intangible)));
}

//-----------------------------------------------------------------------

void ServerWorld::addObjectToConcludeList(ServerObject * object)
{
	if (!gs_pendingConcludeLock)
	{
		if (object)
		{
			gs_pendingConcludeVector.push_back(object);
		}
	}
	else
	{
		gs_pendingConcludeOpsVector.push_back(std::make_pair(0, object));
	}
}

//-----------------------------------------------------------------------

void ServerWorld::addTangibleObject(ServerObject * object)
{
	if (object)
	{
		DEBUG_FATAL(object->isBeingDestroyed(), ("Destroyed objects should no longer be added back to the world, they should have Object::scheduleForAlter() called on them."));

		DEBUG_FATAL(!object->getObjectType(), ("Attempt to add unknown object to world, aborting add, continuing to run (FIXME!)"));
#ifdef _DEBUG
		if (object->getPosition_w() == Vector::zero && (!dynamic_cast<UniverseObject*>(object)))
		{
			DEBUG_WARNING(true, ("Adding object to origin, probably an error.  server id=[%d], object id=[%s], object template name=[%s]",
				static_cast<int>(GameServer::getInstance().getProcessId()),
				object->getNetworkId().getValueString().c_str(),
				object->getObjectTemplateName()));
		}
#endif
		if (object->getObjectType())
		{
			World::addObject(object, static_cast<int>(WOL_Tangible));

			// place the object in the sphere tree
			if (!object->asCellObject())
			{
				Sphere s = object->getSphereExtent();
				float r = s.getRadius();
				WARNING_STRICT_FATAL(r != r, ("Someone is adding object %s:%s to the world that has a sphere extent radius that is Not a Number!", object->getObjectTemplateName(), object->getNetworkId().getValueString().c_str()));

				//what the fuck?
				if ((r == r) && (s.getCenter() == s.getCenter())) // check for NaN in radius
				{
					//First put in object sphere tree
					g_objectSphereTree->onObjectAdded(object);

					//Now notify triggers.
					if (isRelevantToTriggerVolumes(*object))
					{
						std::vector<TriggerVolume *> results;
						std::set<TriggerVolume *> results2;

						int system = ConfigServerGame::getTriggerVolumeSystem();  // 0 = old, 1 = compare, 2 = new

						if (system <= 1)
						{
							PROFILER_AUTO_BLOCK_DEFINE("sphere_tree_find");
							g_triggerSphereTree->findInRange(s.getCenter(), s.getRadius(), results);
						}

						if (system >= 1)
						{
							PROFILER_AUTO_BLOCK_DEFINE("sphere_grid_find");
							g_triggerSphereGrid->findInRange(s.getCenter(), s.getRadius(), results2);
						}

						if (system == 1)
						{
							compare_results(s.getCenter(), s.getRadius(), results, results2, object);
						}

						// potential add to trigger volumes
						if (system != 2)
						{
							for (std::vector<TriggerVolume *>::const_iterator i = results.begin(); i != results.end(); ++i)
								(*i)->addObject(*object);
						}
						else if (system == 2)
						{
							for (std::set<TriggerVolume *>::const_iterator i = results2.begin(); i != results2.end(); ++i)
								(*i)->addObject(*object);
						}
					}
				}
				else
				{
					WARNING(r != r, ("Radius is Not a Number!"));
					WARNING(s.getCenter() != s.getCenter(), ("Sphere center is Not a Number!"));
					DEBUG_FATAL(true, ("Bad sphere data when adding object to the world!"));
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void ServerWorld::addUniverseObject(UniverseObject * object)
{
	NOT_NULL(object);
	World::addObject(object, static_cast<int>(WOL_Intangible));
	ServerUniverse::getInstance().addUniverseObject(*object);
	object->onAddedToWorld();
}

// ----------------------------------------------------------------------

void ServerWorld::removeUniverseObject(UniverseObject * object)
{
	IGNORE_RETURN(World::removeObject(object, static_cast<int>(WOL_Intangible)));
}

//-----------------------------------------------------------------------

void ServerWorld::addObjectTriggerVolume(TriggerVolume * triggerVolume)
{
	int system = ConfigServerGame::getTriggerVolumeSystem();  // 0 = old, 1 = compare, 2 = new
	if (system <= 1)
	{
		PROFILER_AUTO_BLOCK_DEFINE("sphere_tree_add");
		g_triggerSphereTree->onObjectAdded(triggerVolume);
	}

	if (system >= 1)
	{
		PROFILER_AUTO_BLOCK_DEFINE("sphere_grid_add");
		g_triggerSphereGrid->onObjectAdded(triggerVolume);
	}

	// query the object sphere tree for objects that will be added to
	// the trigger volume now that it has been added
	std::vector<ServerObject *> results;
	// This is filtered as appropriate for things which should be in trigger volumes
	g_objectSphereTree->findInRange(triggerVolume->getOwner().getPosition_w(), triggerVolume->getRadius(), TriggerVolumeFilter(), results);
	for (std::vector<ServerObject *>::iterator i = results.begin(); i != results.end(); ++i)
		triggerVolume->addObject(**i);
}

//-----------------------------------------------------------------------

/**
 * Creates a new authoritative object. The object will be added to the world
 * when we have received a signal that it has been persisted.
 * NOTE: createNewObjectEnd must be called to complete the creation process.
 *
 * @param templateName    template to create the object from
 * @param transform       where in the world the object should be placed
 * @param cell	    cell the new object should be in
 * @param persisted       flag to persist the object on creation
 * @param hyperspace      whether the object should appear to hyperspace in on clients
 *
 * @return the new object
 */
ServerObject *ServerWorld::createNewObject(std::string const &templateName, Transform const &transform, ServerObject *cell, bool persisted, bool hyperspace)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObject1crc");

	if (templateName.find('\\') != std::string::npos)
	{
		DEBUG_REPORT_LOG(true, ("Tried to pass a backslash into createNewObject for template name %s\n", templateName.c_str()));
		return 0;
	}

	return createNewObject(Crc::calculate(templateName.c_str()), transform, cell, persisted, hyperspace);
}

//-----------------------------------------------------------------------

/**
 * Creates a new authoritative object. The object will be added to the world
 * when we have received a signal that it has been persisted.
 * NOTE: createNewObjectEnd must be called to complete the creation process.
 *
 * @param templateCrc     template to create the object from
 * @param position	where in the world the object should be placed
 * @param cell	    cell the new object should be in
 * @param persisted       flag to persist the object on creation
 * @param hyperspace      whether the object should appear to hyperspace in on clients
 *
 * @return the new object
 */
ServerObject *ServerWorld::createNewObject(uint32 templateCrc, Transform const &transform, ServerObject *cell, bool persisted, bool hyperspace)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObject1");

	ServerObject * const newObject = createObjectFromTemplate(templateCrc, NetworkId::cms_invalid);
	if (newObject)
	{
		newObject->setHyperspaceOnCreate(hyperspace);
		return createNewObjectIntermediate(newObject, transform, cell, persisted);
	}
	return 0;
}

//-----------------------------------------------------------------------

/**
 * Creates a new authoritative object. The object will be added to the world
 * when we have received a signal that it has been persisted.
 * NOTE: createNewObjectEnd must be called to complete the creation process.
 *
 * @param objectTemplate  template to create the object from
 * @param position	where in the world the object should be placed
 * @param cell	    cell the new object should be in
 * @param persisted       flag to persist the object on creation
 * @param hyperspace      whether the object should appear to hyperspace in on clients
 *
 * @return the new object
 */
ServerObject *ServerWorld::createNewObject(ServerObjectTemplate const &objectTemplate, Transform const &transform, ServerObject *cell, bool persisted, bool hyperspace)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObject2");

	ServerObject * const newObject = createObjectFromTemplate(objectTemplate, NetworkId::cms_invalid);
	if (newObject)
	{
		newObject->setHyperspaceOnCreate(hyperspace);
		return createNewObjectIntermediate(newObject, transform, cell, persisted);
	}
	return 0;
}

//-----------------------------------------------------------------------

/**
 * Creates a new authoritative object in a container.
 * NOTE: createNewObjectEnd must be called to complete the creation process.
 *
 * @param templateName    template to create the object from
 * @param container       container to place the new object in
 * @param persisted       flag to persist the object on creation
 *
 * @return the new object
 */
ServerObject *ServerWorld::createNewObject(std::string const &templateName, ServerObject &container, bool persisted, bool allowOverloaded)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObject3");

	if (templateName.find('\\') != std::string::npos)
	{
		DEBUG_REPORT_LOG(true, ("Tried to pass a backslash into createNewObject for template name %s\n", templateName.c_str()));
		return 0;
	}
	return createNewObject(Crc::calculate(templateName.c_str()), container, persisted, allowOverloaded);
}

//-----------------------------------------------------------------------

/**
 * Creates a new authoritative object in a container.
 * NOTE: createNewObjectEnd must be called to complete the creation process.
 *
 * @param templateCrc     template to create the object from
 * @param container       container to place the new object in
 * @param persisted       flag to persist the object on creation
 *
 * @return the new object
 */
ServerObject *ServerWorld::createNewObject(uint32 templateCrc, ServerObject &container, bool persisted, bool allowOverloaded)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObject3crc");

	ServerObject * const newObject = createObjectFromTemplate(templateCrc, NetworkId::cms_invalid);
	if (!newObject)
		return 0;

	return createNewObjectIntermediate(newObject, container, persisted, allowOverloaded);
}

//-----------------------------------------------------------------------

/**
 * Creates a new authoritative object in a conatiner.
 * NOTE: createNewObjectEnd must be called to complete the creation process.
 *
 * @param objectTemplate  template to create the object from
 * @param container       container to place the new object in
 * @param persisted       flag to persist the object on creation
 *
 * @return the new object
 */
ServerObject *ServerWorld::createNewObject(ServerObjectTemplate const &objectTemplate, ServerObject &container, bool persisted, bool allowOverload)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObject4");

	ServerObject * const newObject = createObjectFromTemplate(objectTemplate, NetworkId::cms_invalid);
	if (!newObject)
		return 0;

	return createNewObjectIntermediate(newObject, container, persisted, allowOverload);
}

//-----------------------------------------------------------------------

ServerObject *ServerWorld::createNewObject(ServerObjectTemplate const &objectTemplate, ServerObject &container, SlotId const &slotId, bool persisted)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObject5");

	ServerObject * const newObject = createObjectFromTemplate(objectTemplate, NetworkId::cms_invalid);
	if (!newObject)
		return 0;

	return createNewObjectIntermediate(newObject, container, slotId, persisted);
}

//-----------------------------------------------------------------------

ServerObject *ServerWorld::createNewObject(std::string const &templateName, ServerObject &container, SlotId const &slotId, bool persisted)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObject6");

	if (templateName.find('\\') != std::string::npos)
	{
		DEBUG_REPORT_LOG(true, ("Tried to pass a backslash into createNewObject for template name %s\n", templateName.c_str()));
		return 0;
	}
	return createNewObject(Crc::calculate(templateName.c_str()), container, slotId, persisted);
}

//-----------------------------------------------------------------------

ServerObject *ServerWorld::createNewObject(uint32 templateCrc, ServerObject &container, SlotId const &slotId, bool persisted)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObject6crc");

	ServerObject * const newObject = createObjectFromTemplate(templateCrc, NetworkId::cms_invalid);
	if (!newObject)
		return 0;

	return createNewObjectIntermediate(newObject, container, slotId, persisted);
}

//-----------------------------------------------------------------------

/**
 * Creates a new temporary manufacturing schematic for use in crafting. It would
 * be nice if we could pull this out of ServerWorld, but there's too much important
 * functionality in createNewObjectIntermediate to duplicate that function.
 *
 * @param creator		the player creating the schematic
 * @param container		container to place the new object in
 * @param slotId		slot to place the new object in
 * @param persisted		flag to persist the schematic on creation
 *
 * @return the newly created schematic
 */
ManufactureSchematicObject* ServerWorld::createNewManufacturingSchematic(
	const CachedNetworkId & creator, ServerObject & container, const SlotId & slotId,
	bool persisted)
{
	CreatureObject * const creature = dynamic_cast<CreatureObject *>(creator.getObject());
	if (creature == nullptr)
		return nullptr;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
		return nullptr;

	const DraftSchematicObject * const draftSchematic = player->getCurrentDraftSchematic();
	if (draftSchematic == nullptr)
		return nullptr;

	ManufactureSchematicObject * const manfSchematic = draftSchematic->createManufactureSchematic(creator);
	if (manfSchematic == nullptr)
		return nullptr;

	if (createNewObjectIntermediate(manfSchematic, container, slotId,
		persisted) == nullptr)
	{
		delete manfSchematic;
		return nullptr;
	}

	return manfSchematic;
}	// ServerWorld::createNewManufacturingSchematic

//-----------------------------------------------------------------------

/**
 * Creates a new temporary manufacturing schematic for use in crafting. It would
 * be nice if we could pull this out of ServerWorld, but there's too much important
 * functionality in createNewObjectIntermediate to duplicate that function.
 *
 * @param source		the draft schematic the manf schematic is created from
 * @param position		where to create the schematic
 * @param persisted		flag to persist the schematic on creation
 *
 * @return the newly created schematic
 */
ManufactureSchematicObject* ServerWorld::createNewManufacturingSchematic(
	const DraftSchematicObject & source, const Vector & position, bool persisted)
{
	ManufactureSchematicObject * const manfSchematic =
		source.createManufactureSchematic(CachedNetworkId::cms_cachedInvalid);
	if (manfSchematic == nullptr)
		return nullptr;

	Transform transform;
	transform.setPosition_p(position);
	if (createNewObjectIntermediate(manfSchematic, transform, 0, persisted) == nullptr)
	{
		delete manfSchematic;
		return nullptr;
	}

	return manfSchematic;
}	// ServerWorld::createNewManufacturingSchematic

//-----------------------------------------------------------------------

/**
 * Creates a new temporary manufacturing schematic for use in crafting. It would
 * be nice if we could pull this out of ServerWorld, but there's too much important
 * functionality in createNewObjectIntermediate to duplicate that function.
 *
 * @param source		the draft schematic the manf schematic is created from
 * @param container		container to create the schematic in
 * @param persisted		flag to persist the schematic on creation
 *
 * @return the newly created schematic
 */
ManufactureSchematicObject* ServerWorld::createNewManufacturingSchematic(
	const DraftSchematicObject & source, ServerObject & container, bool persisted)
{
	ManufactureSchematicObject * const manfSchematic =
		source.createManufactureSchematic(CachedNetworkId::cms_cachedInvalid);
	if (manfSchematic == nullptr)
		return nullptr;

	if (createNewObjectIntermediate(manfSchematic, container, persisted, false) == nullptr)
		return nullptr;
	return manfSchematic;
}	// ServerWorld::createNewManufacturingSchematic

//-----------------------------------------------------------------------

/**
 * Called by the two above createNewObjectStart functions.  Done to provide a
 * function for common code.
 *
 * @param newObject			the new object being created
 * @param position			where in the world the object should be placed
 * @param cell				cell the new object should be in
 * @param persisted			flag to persist the object on creation
 *
 * @return the new object
 */
ServerObject* ServerWorld::createNewObjectIntermediate(ServerObject *newObject, Transform const &transform, ServerObject *cell, bool persisted)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObjectIntermediate1");

	if (newObject->getNetworkId() == NetworkId::cms_invalid)
	{
		WARNING_STRICT_FATAL(true, ("Tried to create an object with network id 0."));
		return nullptr;
	}

	NetworkController *objectController = dynamic_cast<NetworkController *>(newObject->getController());
	if (objectController == nullptr)
		objectController = dynamic_cast<NetworkController *>(newObject->createDefaultController());
	NOT_NULL(objectController);

	// initialize the object
	newObject->setAuthority();

	if (!newObject->serverObjectInitializeFirstTimeObject(cell, transform))
	{
		delete newObject;
		return nullptr;
	}

	// prevent initial object data from being re-sent as deltas
	newObject->clearDeltas();

	const ServerObjectTemplate * objectTemplate = safe_cast<const ServerObjectTemplate *>(newObject->getObjectTemplate());

	if (persisted || objectTemplate->getPersistByDefault())
	{
		newObject->persist();
	}

	return newObject;
}

//-----------------------------------------------------------------------

/**
 * Called by the two above createNewObjectStart in container functions. Done to
 * provide a function for common code.
 *
 * @param newObject			the new object being created
 * @param container			container to place the new object in
 * @param persisted			flag to persist the object on creation
 *
 * @return the new object
 */
ServerObject* ServerWorld::createNewObjectIntermediate(ServerObject* newObject, ServerObject & container, bool persisted, bool allowOverload)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObjectIntermediate2");

	if (newObject->getNetworkId() == NetworkId::cms_invalid)
	{
		WARNING_STRICT_FATAL(true, ("Tried to create an object with network id 0."));
		return nullptr;
	}

	NetworkController *objectController = dynamic_cast<NetworkController *>(newObject->getController());
	if (objectController == nullptr)
		objectController = dynamic_cast<NetworkController *>(newObject->createDefaultController());
	NOT_NULL(objectController);

	// initialize the object
	newObject->setAuthority();

	IGNORE_RETURN(newObject->serverObjectInitializeFirstTimeObject(0, Transform::identity));

	// prevent initial object data from being re-sent as deltas
	newObject->clearDeltas();
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	if (!ContainerInterface::transferItemToGeneralContainer(container, *newObject, nullptr, tmp, allowOverload))
	{
		IGNORE_RETURN(newObject->permanentlyDestroy(DeleteReasons::BadContainerTransfer));
		newObject = nullptr;
	}
	else
	{
		const ServerObjectTemplate * objectTemplate = safe_cast<
			const ServerObjectTemplate *>(newObject->getObjectTemplate());

		if (persisted || objectTemplate->getPersistByDefault())
		{
			newObject->persist();
		}
	}

	return newObject;
}

//-----------------------------------------------------------------------

ServerObject* ServerWorld::createNewObjectIntermediate(ServerObject* newObject, ServerObject & container, const SlotId & slotId, bool persisted)
{
	PROFILER_AUTO_BLOCK_DEFINE("createNewObjectIntermediate3");

	if (newObject->getNetworkId() == NetworkId::cms_invalid)
	{
		WARNING_STRICT_FATAL(true, ("Tried to create an object with network id 0."));
		return nullptr;
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("createNewObjectIntermedate,getController");
		NetworkController *objectController = dynamic_cast<NetworkController *>(newObject->getController());
		if (objectController == nullptr)
			objectController = dynamic_cast<NetworkController *>(newObject->createDefaultController());
		NOT_NULL(objectController);
	}
	{
		PROFILER_AUTO_BLOCK_DEFINE("createNewObjectIntermediate.setAuthority");
		newObject->setAuthority();
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("initializeFirstTimeObject");
		IGNORE_RETURN(newObject->serverObjectInitializeFirstTimeObject(0, Transform::identity));
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("clearDeltas");
		// prevent initial object data from being re-sent as deltas
		newObject->clearDeltas();
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("transferItem");
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		if (!ContainerInterface::transferItemToSlottedContainer(container, *newObject, slotId, nullptr, tmp))
		{
			PROFILER_AUTO_BLOCK_DEFINE("failedTransfer");
			IGNORE_RETURN(newObject->permanentlyDestroy(DeleteReasons::BadContainerTransfer));
			newObject = nullptr;
		}
		else
		{
			PROFILER_AUTO_BLOCK_DEFINE("successTransfer");
			const ServerObjectTemplate * objectTemplate = safe_cast<
				const ServerObjectTemplate *>(newObject->getObjectTemplate());

			if (persisted || objectTemplate->getPersistByDefault())
			{
				newObject->persist();
			}
		}
	}
	return newObject;
}

//-----------------------------------------------------------------------

/**
 * Creates a proxy object. The object will be added to the world when we have
 * received it's baselines.
 *
 * @param templateName		template to create the object from
 * @param id				the proxy's id
 *
 * @return the new object
 */
ServerObject* ServerWorld::createProxyObject(const std::string &templateName, const NetworkId &id, bool createAuthoritative)
{
	PROFILER_AUTO_BLOCK_DEFINE("createProxyObject");

	return createProxyObject(Crc::calculate(templateName.c_str()), id, createAuthoritative);
}	// ServerWorld::createProxyObject

//-----------------------------------------------------------------------

/**
 * Creates a proxy object. The object will be added to the world when we have
 * received it's baselines.
 *
 * @param templateName		template to create the object from
 * @param id				the proxy's id
 *
 * @return the new object
 */
ServerObject* ServerWorld::createProxyObject(uint32 templateCrc, const NetworkId &id, bool createAuthoritative)
{
	PROFILER_AUTO_BLOCK_DEFINE("createProxyObjectCrc");

	ServerObject *newObject = createObjectFromTemplate(templateCrc, id);
	DEBUG_WARNING(newObject == 0, ("Failed to load object from template [%s]", ObjectTemplateList::lookUp(templateCrc).getString()));
	if (newObject)
	{
		ServerController *objectController = dynamic_cast<ServerController *>(newObject->getController());
		if (objectController == nullptr)
			objectController = dynamic_cast<ServerController *>(newObject->createDefaultController());
		NOT_NULL(objectController);

		// initialize the object
		if (createAuthoritative)
			newObject->setAuthority();
		else
			objectController->setAuthoritative(false);
	}
	return newObject;
}	// ServerWorld::createProxyObject

//-----------------------------------------------------------------------

void ServerWorld::debugDump()
{
	World::debugReport();
}

//-----------------------------------------------------------------------

void ServerWorld::dumpObjectSphereTree(std::vector<std::pair<ServerObject *, Sphere> > & results)
{
	g_objectSphereTree->dumpSphereTree(results);
}

//-----------------------------------------------------------------------

void ServerWorld::dumpTriggerSphereTree(std::vector<std::pair<TriggerVolume *, Sphere> > & results)
{
	g_triggerSphereTree->dumpSphereTree(results);
}

//-----------------------------------------------------------------------

ServerObject * ServerWorld::findObjectByNetworkId(const NetworkId& id, bool searchQueuedList)
{
	UNREF(searchQueuedList);
	if (id.getValue() == 0)
		return 0;

	ServerObject * object = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(id));
	if (object && (!object->isInitialized() || object->isBeingDestroyed()))
		return 0;
	return object;
}

//-----------------------------------------------------------------------

ServerObject * ServerWorld::findUninitializedObjectByNetworkId(const NetworkId& id)
{
	if (!id.isValid())
	{
		DEBUG_REPORT_LOG(true, ("ERROR - Tried to invoke ServerWorld::findUninitializedObjectByNetworkId with id %s\n", id.getValueString().c_str()));
		return 0;
	}

	ServerObject * object = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(id));
	if (object && object->isInitialized())
	{
		DEBUG_REPORT_LOG(true, ("Attempting to get an initialized object with uninitialized accessor %s\n", id.getValueString().c_str()));
		return 0;
	}
	return object;
}

//-----------------------------------------------------------------------

void ServerWorld::findObjectsInRange(const Vector & location, const float distance, std::vector<ServerObject *> & results)
{
	g_objectSphereTree->findInRange(location, distance, results);
}

//-----------------------------------------------------------------------

class StaticCollidableObjectFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	StaticCollidableObjectFilter() {}
	bool operator() (ServerObject * const &object) const
	{
		CollisionProperty const * collision = object->getCollisionProperty();
		if (!collision)
			return false;
		bool mobile = collision->isMobile();
		return !mobile;
	}
};

void ServerWorld::findStaticCollidableObjectsInRange(const Vector & location, const float distance, std::vector<ServerObject *> & results)
{
	g_objectSphereTree->findInRange(location, distance, StaticCollidableObjectFilter(), results);
}

//-----------------------------------------------------------------------

void ServerWorld::findCreaturesInRange(const Vector & location, const float distance, std::vector<ServerObject *> & results)
{
	g_objectSphereTree->findInRange(location, distance, CreatureFilter(), results);
}

//-----------------------------------------------------------------------

void ServerWorld::findAuthoritativeNonPlayerCreaturesInRange(const Vector & location, const float distance, std::vector<ServerObject *> & results)
{
	g_objectSphereTree->findInRange(location, distance, AuthoritativeNonPlayerCreatureFilter(), results);
}

//-----------------------------------------------------------------------

class CreatureNicheFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	explicit CreatureNicheFilter(int i_type, int i_mask) : type(i_type), mask(i_mask) { type &= mask; }
	bool operator() (ServerObject * const &object) const
	{
		CreatureObject * creature = dynamic_cast<CreatureObject *>(object);
		if (!creature)
			return false;
		return (creature->getNiche() & mask) == type;
	}

private:
	CreatureNicheFilter();

private:
	int type, mask;
};

void ServerWorld::findCreaturesOfNicheInRange(const Vector & location, const float distance, int type, int mask, std::vector<ServerObject *> & results)
{
	g_objectSphereTree->findInRange(location, distance, CreatureNicheFilter(type, mask), results);
}

//-----------------------------------------------------------------------

class CreatureSpeciesFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	explicit CreatureSpeciesFilter(int i_species) : species(i_species) {}
	bool operator() (ServerObject * const &object) const
	{
		CreatureObject * creature = object->asCreatureObject();
		if (!creature)
			return false;
		return creature->getSpecies() == species;
	}

private:
	CreatureSpeciesFilter();

private:
	int species;
};

void ServerWorld::findCreaturesOfSpeciesInRange(const Vector & location, const float distance, int species, std::vector<ServerObject *> & results)
{
	g_objectSphereTree->findInRange(location, distance, CreatureSpeciesFilter(species), results);
}

//-----------------------------------------------------------------------

class CreatureRaceFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	explicit CreatureRaceFilter(int i_species, int i_race) : species(i_species), race(i_race) {}
	bool operator() (ServerObject * const &object) const
	{
		CreatureObject * creature = dynamic_cast<CreatureObject *>(object);
		if (!creature)
			return false;
		return creature->getSpecies() == species && creature->getRace() == race;
	}

private:
	CreatureRaceFilter();

private:
	int species, race;
};

void ServerWorld::findCreaturesOfRaceInRange(const Vector & location, const float distance, int species, int race, std::vector<ServerObject *> & results)
{
	g_objectSphereTree->findInRange(location, distance, CreatureRaceFilter(species, race), results);
}

//-----------------------------------------------------------------------

class NPCFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	bool operator() (ServerObject * const &object) const
	{
		CreatureObject *c = dynamic_cast<CreatureObject *>(object);
		if (c)
			return !(c->isPlayerControlled());
		else
			return false;
	}
};

/**
 * Find all creatures that are not player-controlled within a given radius.
 */
void ServerWorld::findNPCsInRange(const Vector & location, const float distance, std::vector<ServerObject *> & results)
{
	g_objectSphereTree->findInRange(location, distance, NPCFilter(), results);
}

//-----------------------------------------------------------------------

class PlayerFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	bool operator() (ServerObject * const &object) const
	{
		CreatureObject *c = dynamic_cast<CreatureObject *>(object);
		if (c)
			return (c->isPlayerControlled());
		else
			return false;
	}
};

/**
 * Find all creatures that are player-controlled within a given radius.  (Won't find vehicles, for example.)
 */
void ServerWorld::findPlayerCreaturesInRange(const Vector & location, const float distance, std::vector<ServerObject *> & results)
{
	g_objectSphereTree->findInRange(location, distance, PlayerFilter(), results);
}

//-----------------------------------------------------------------------

/**
 * Internal cone-test function for the getXInCone functions. Does not do distance
 * testing because we already know the object is in range.
 *
 * This function does precalculations that can be done once when iterating through objects to test.
 *
 */
static void _isObjectInConeLoopSetup(const Object & coneCenterObject, const Object & coneDirectionObject, float angle, Vector & coneAxisVector, float & cosAngle)
{
	// NOTE: all calculations are made within the object space of coneCenterObject.

	//-- Compute cone axis vector.
	coneAxisVector = coneCenterObject.rotateTranslate_w2o(coneDirectionObject.getPosition_w());
	IGNORE_RETURN(coneAxisVector.normalize());

	cosAngle = cos(angle);
}

/**
 * Internal cone-test function for the getXInCone functions. Does not do distance
 * testing because we already know the object is in range.
 *
 * This function does precalculations that can be done once when iterating through objects to test.
 *
 */
static void _isObjectInConeLoopSetup(const Object & coneCenterObject, const Location & coneDirection, float angle, Vector & coneAxisVector, float & cosAngle)
{
	// NOTE: all calculations are made within the object space of coneCenterObject.

	//-- Compute cone axis vector.
	const ServerObject * cell = nullptr;
	if (coneDirection.getCell() != NetworkId::cms_invalid)
		cell = safe_cast<const ServerObject *>(NetworkIdManager::getObjectById(coneDirection.getCell()));
	if (cell == nullptr)
		coneAxisVector = coneCenterObject.rotateTranslate_w2o(coneDirection.getCoordinates());
	else
	{
		// convert the location to world coordinates
		Vector worldDirection(cell->getTransform_o2w().rotateTranslate_l2p(coneDirection.getCoordinates()));
		coneAxisVector = coneCenterObject.rotateTranslate_w2o(worldDirection);
	}
	IGNORE_RETURN(coneAxisVector.normalize());

	cosAngle = cos(angle);
}

/**
 * Internal cone-test function for the getXInCone functions. Does not do distance
 * testing because we already know the object is in range.
 *
 * This function is used when iterating through objects and takes parameters computed in _isObjectInConeLoopSetup.
 *
 * @return true if the object is in the cone
 */
static bool _isObjectInConeLoop(const Object & coneCenterObject, const Object & testObject, const Vector & coneAxisVector, const float cosAngle)
{
	// NOTE: all calculations are made within the object space of coneCenterObject.

	//-- Get test object orientation in cone's center object's space.
	Vector testOrientation = coneCenterObject.rotateTranslate_w2o(testObject.getPosition_w());
	IGNORE_RETURN(testOrientation.normalize());

	//-- return true if angle between forward and position_o is smaller than the cone angle

	const float dotProduct = coneAxisVector.dot(testOrientation);
	const bool  withinCone = (dotProduct >= cosAngle);

	return withinCone;
}

// ----------------------------------------------------------------------
/**
 * Find all objects within a given cone.
 *
 * @param coneCenterObject		location of cone point
 * @param coneDirectionObject	position to orient the axis of the cone
 * @param distance				radius of cone
 * @param angle					cone angle in radians
 * @param results				vector to store the found objects in
 */
void ServerWorld::findObjectsInCone(const Object & coneCenterObject,
	const Object & coneDirectionObject, float distance,
	float angle, std::vector<ServerObject *> & results)
{
	std::vector<ServerObject *> rangeResults;
	findObjectsInRange(coneCenterObject.getPosition_w(), distance, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirectionObject, angle, coneAxisVector, cosAngle);

	// Check each object within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

// ----------------------------------------------------------------------

/**
 * Find all objects within a given cone.
 *
 * @param coneCenterObject	location of cone point
 * @param coneDirection     position to orient the axis of the cone
 * @param distance			radius of cone
 * @param angle				cone angle in radians
 * @param results			vector to store the found objects in
 */
void ServerWorld::findObjectsInCone(const Object & coneCenterObject,
	const Location & coneDirection, float distance,
	float angle, std::vector<ServerObject *> & results)
{
	std::vector<ServerObject *> rangeResults;
	findObjectsInRange(coneCenterObject.getPosition_w(), distance, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirection, angle, coneAxisVector, cosAngle);

	// Check each object within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

// ----------------------------------------------------------------------
/**
 * Find all creatures within a given cone.
 *
 * @param coneCenterObject		location of cone point
 * @param coneDirectionObject	position to orient the axis of the cone
 * @param distance				radius of cone
 * @param angle					cone angle in radians
 * @param results				vector to store the found objects in
 */
void ServerWorld::findCreaturesInCone(const Object & coneCenterObject,
	const Object & coneDirectionObject, float distance,
	float angle, std::vector<ServerObject *> & results)
{
	// Query for the creatures within the given range.
	std::vector<ServerObject *> rangeResults;
	findCreaturesInRange(coneCenterObject.getPosition_w(), distance, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirectionObject, angle, coneAxisVector, cosAngle);

	// Check each creature within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

// ----------------------------------------------------------------------
/**
 * Find all creatures within a given cone.
 *
 * @param coneCenterObject	location of cone point
 * @param coneDirection		position to orient the axis of the cone
 * @param distance			radius of cone
 * @param angle				cone angle in radians
 * @param results			vector to store the found objects in
 */
void ServerWorld::findCreaturesInCone(const Object & coneCenterObject,
	const Location & coneDirection, float distance,
	float angle, std::vector<ServerObject *> & results)
{
	// Query for the creatures within the given range.
	std::vector<ServerObject *> rangeResults;
	findCreaturesInRange(coneCenterObject.getPosition_w(), distance, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirection, angle, coneAxisVector, cosAngle);

	// Check each creature within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

// ----------------------------------------------------------------------
/**
 * Find all authoritative non-player creatures within a given cone.
 *
 * @param coneSourceObject	location of cone point, along with cone orientation along +z axis.
 * @param distance		radius of cone
 * @param angle			cone angle in radians
 * @param results		vector to store the found objects in
 */
void ServerWorld::findAuthoritativeNonPlayerCreaturesInCone(const Object & coneCenterObject,
	const Object & coneDirectionObject, float distance,
	float angle, std::vector<ServerObject *> & results)
{
	// Query for the creatures within the given range.
	std::vector<ServerObject *> rangeResults;
	findAuthoritativeNonPlayerCreaturesInRange(coneCenterObject.getPosition_w(), distance, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirectionObject, angle, coneAxisVector, cosAngle);

	// Check each creature within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

// ----------------------------------------------------------------------
/**
 * Find all creatures of a given niche within a given cone.
 *
 * @param location		location of cone point
 * @param distance		radius of cone
 * @param angle			cone angle in radians
 * @param results		vector to store the found objects in
 */
void ServerWorld::findCreaturesOfNicheInCone(const Object & coneCenterObject,
	const Object & coneDirectionObject, float distance, float angle, int niche,
	int mask, std::vector<ServerObject *> & results)
{
	std::vector<ServerObject *> rangeResults;
	findCreaturesOfNicheInRange(coneCenterObject.getPosition_w(), distance, niche, mask, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirectionObject, angle, coneAxisVector, cosAngle);

	// Check each creature within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

// ----------------------------------------------------------------------
/**
 * Find all creatures of a given species within a given cone.
 *
 * @param location		location of cone point
 * @param distance		radius of cone
 * @param angle			cone angle in radians
 * @param results		vector to store the found objects in
 */
void ServerWorld::findCreaturesOfSpeciesInCone(const Object & coneCenterObject,
	const Object & coneDirectionObject, float distance,
	float angle, int species, std::vector<ServerObject *> & results)
{
	std::vector<ServerObject *> rangeResults;
	findCreaturesOfSpeciesInRange(coneCenterObject.getPosition_w(), distance, species, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirectionObject, angle, coneAxisVector, cosAngle);

	// Check each creature within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

// ----------------------------------------------------------------------
/**
 * Find all creatures of a given race within a given cone.
 *
 * @param location		location of cone point
 * @param distance		radius of cone
 * @param angle			cone angle in radians
 * @param results		vector to store the found objects in
 */
void ServerWorld::findCreaturesOfRaceInCone(const Object & coneCenterObject,
	const Object & coneDirectionObject, float distance,
	float angle, int species, int race, std::vector<ServerObject *> & results)
{
	std::vector<ServerObject *> rangeResults;
	findCreaturesOfRaceInRange(coneCenterObject.getPosition_w(), distance, species, race, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirectionObject, angle, coneAxisVector, cosAngle);

	// Check each creature within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

// ----------------------------------------------------------------------
/**
 * Find all objects that aren't creatures within a given cone.
 *
 * @param location		location of cone point
 * @param distance		radius of cone
 * @param angle			cone angle in radians
 * @param results		vector to store the found objects in
 */
void ServerWorld::findNonCreaturesInCone(const Object & coneCenterObject,
	const Object & coneDirectionObject, float distance, float angle,
	std::vector<ServerObject *> & results)
{
	std::vector<ServerObject *> rangeResults;
	findNonCreaturesInRange(coneCenterObject.getPosition_w(), distance, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirectionObject, angle, coneAxisVector, cosAngle);

	// Check each creature within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

// ----------------------------------------------------------------------
/**
 * Find all creatures that are not player-controlled within a given cone.
 *
 * @param location		location of cone point
 * @param distance		radius of cone
 * @param angle			cone angle in radians
 * @param results		vector to store the found objects in
 */
void ServerWorld::findNPCsInCone(const Object & coneCenterObject,
	const Object & coneDirectionObject, float distance,
	float angle, std::vector<ServerObject *> & results)
{
	std::vector<ServerObject *> rangeResults;
	findNPCsInRange(coneCenterObject.getPosition_w(), distance, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirectionObject, angle, coneAxisVector, cosAngle);

	// Check each creature within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

// ----------------------------------------------------------------------
/**
 * Find all creatures that are player-controlled within a given cone.  (Won't find vehicles, for example.)
 *
 * @param location		location of cone point
 * @param distance		radius of cone
 * @param angle			cone angle in radians
 * @param results		vector to store the found objects in
 */
void ServerWorld::findPlayerCreaturesInCone(const Object & coneCenterObject,
	const Object & coneDirectionObject, float distance,
	float angle, std::vector<ServerObject *> & results)
{
	std::vector<ServerObject *> rangeResults;
	findPlayerCreaturesInRange(coneCenterObject.getPosition_w(), distance, rangeResults);

	Vector coneAxisVector;
	float cosAngle = 0;
	_isObjectInConeLoopSetup(coneCenterObject, coneDirectionObject, angle, coneAxisVector, cosAngle);

	// Check each creature within range to see if they fall within the arc of the cone.
	const std::vector<ServerObject *>::const_iterator endIter = rangeResults.end();
	for (std::vector<ServerObject *>::const_iterator iter = rangeResults.begin(); iter != endIter; ++iter)
	{
		// Get the object that will be tested against the cone.  Ignore it
		// if it is the cone center object.
		const ServerObject *const testObject = *iter;
		if (!testObject || (testObject == &coneCenterObject))
			continue;

		// Test against the cone, add to list if successful.
		if (_isObjectInConeLoop(coneCenterObject, *testObject, coneAxisVector, cosAngle))
			results.push_back(*iter);
	}
}

//-----------------------------------------------------------------------

/**
 * Find closest NPC.
 * @param distance the maximum distance to look, which must be less than the object interaction limit
 */
ServerObject *ServerWorld::findClosestNPC(const Vector & location, float distance)
{
	if (distance > 128)
		distance = 128; // TODO:  #define somewhere ?

	std::vector<ServerObject*> candidates;
	findNPCsInRange(location, distance, candidates);
	return findClosestObjectInList(location, candidates);
}

// ----------------------------------------------------------------------

/**
 * Find closest Player.
 * @param distance the maximum distance to look, which must be less than the object interaction limit
 */
ServerObject *ServerWorld::findClosestPlayer(const Vector & location, float distance)
{
	if (distance > 128)
		distance = 128; // TODO:  #define somewhere ?

	std::vector<ServerObject*> candidates;
	findPlayerCreaturesInRange(location, distance, candidates);
	return findClosestObjectInList(location, candidates);
}

class PobFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	PobFilter() {}
	bool operator() (ServerObject * const &object) const
	{
		PortalProperty const * portal = object->getPortalProperty();
		if (!portal)
			return false;
		return true;
	}
};

ServerObject *ServerWorld::findPobAtLocation(const Vector & location_w)
{
	std::vector<ServerObject*> candidates;
	g_objectSphereTree->findInRange(location_w, 0.1f, PobFilter(), candidates);

	return findClosestObjectInList(location_w, candidates);
}

CellProperty const * ServerWorld::findCellAtLocation(const Vector & location_w)
{
	CellProperty const * cell = CellProperty::getWorldCellProperty();

	ServerObject const * const pob = findPobAtLocation(location_w);
	if (pob)
	{
		PortalProperty const * const portal = pob->getPortalProperty();
		if (portal)
		{
			Vector location_l = pob->rotateTranslate_w2o(location_w);
			cell = portal->findContainingCell(location_l);
		}
	}

	return cell;
}

// ----------------------------------------------------------------------

/**
 * Helper function for findClosest.
 * Given a list of objects, determines which one is closest to a point
 */
ServerObject *ServerWorld::findClosestObjectInList(const Vector &location, const std::vector<ServerObject*> &candidates)
{
	if (candidates.empty()) return nullptr;

	typedef std::vector<ServerObject *> CandidatesType;

	CandidatesType::const_iterator closest = candidates.begin();
	float minDistance = (*closest)->getPosition_w().magnitudeBetweenSquared(location);

	for (CandidatesType::const_iterator i = closest + 1; i != candidates.end(); ++i)
	{
		float distance = (*i)->getPosition_w().magnitudeBetweenSquared(location);
		if (distance < minDistance)
		{
			minDistance = distance;
			closest = i;
		}
	}

	return *closest;
}

//-----------------------------------------------------------------------

class NonCreatureFilter : public SpatialSubdivisionFilter<ServerObject *>
{
public:
	bool operator() (ServerObject * const &object) const { return dynamic_cast<CreatureObject *>(object) == 0; }
};

void ServerWorld::findNonCreaturesInRange(const Vector & location, const float distance, std::vector<ServerObject *> & results)
{
	g_objectSphereTree->findInRange(location, distance, NonCreatureFilter(), results);
}

//-----------------------------------------------------------------------

int ServerWorld::getNumObjects()
{
	return World::getNumberOfObjects(static_cast<int>(WOL_Tangible));
}

//-----------------------------------------------------------------------

ServerObject * ServerWorld::getObject(int index)
{
	//-- Justin Randall [10/23/2000 9:13:32 AM] --
	//	only NetworkObjects may be added to this list (see addObject() above)
	//	The cast here is safe. Todo: make a NetworkObjectList class that keeps
	//	objects in a hash for location by network id.
	return static_cast<ServerObject *>(World::getObject(static_cast<int>(WOL_Tangible), index));
}

//-----------------------------------------------------------------------

static Object *beginCreateServerCellObject(int index)
{
	ServerObject *newObject = ServerWorld::createNewObject("object/cell/cell.iff", Transform::identity, 0, false);
	NOT_NULL(newObject);
	safe_cast<CellObject *>(newObject)->setCell(index);

	return newObject;
}

//-----------------------------------------------------------------------

static void endCreateServerCellObject(Object *newObject)
{
	ServerObject* cellObject = safe_cast<ServerObject*>(newObject);
	if (!cellObject)
	{
		DEBUG_FATAL(true, ("Um...cell is not a server object?"));
		return;
	}
	ServerObject* portalObject = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*cellObject));
	if (!portalObject)
	{
		DEBUG_FATAL(true, ("This cell is free floating with no parent"));
		return;
	}

	cellObject->setLoadWith(portalObject->getNetworkId());

	if (portalObject->isPersisted())
		cellObject->persist();
}

//-----------------------------------------------------------------------

void ServerWorld::install()
{
	DEBUG_FATAL(m_installed, ("Trying to reinstall ServerWorld"));

	DebugFlags::registerFlag(ms_logTriggerStats, "ServerGame", "logTriggerStats");

	m_idleTimer = new Timer;
	g_objectSphereTree = new PortallizedSphereTree<ServerObject *, ServerObjectSphereExtentAccessor>;

	g_triggerSphereTree = new PortallizedSphereTree<TriggerVolume *, TriggerVolumeSphereExtentAccessor>;
	g_triggerSphereGrid = new DoubleSphereGrid<TriggerVolume *, TriggerVolumeSphereExtentAccessor_Grid>;

	s_numMoveLists = ConfigServerGame::getNumberOfMoveObjectLists();
	for (int i = 0; i < s_numMoveLists; ++i)
	{
		s_moveObjectList.push_back(new MoveObjectMap);
	}
	s_moveObjectListValid = true;

	PortalProperty::install(beginCreateServerCellObject, endCreateServerCellObject);

	{
		SetupSharedCollision::Data data;

		data.installExtents = true;
		data.installCollisionWorld = true;
		data.playEffect = nullptr;
		data.isPlayerHouse = &isPlayerHouseHook;
		data.serverSide = true;

		SetupSharedCollision::install(data);

		CollisionCallbacks::install();

		// setup callback handlers to display more server-specific information
		// when the collision detection thinks a warp situation has occurred.
		CollisionWorld::setNearWarpWarningCallback(issueCollisionNearWarpWarning);
		CollisionWorld::setFarWarpWarningCallback(issueCollisionFarWarpWarning);
	}

	Region3dMaster::install();
	RegionMaster::install();
	Pvp::install(); // must be done after RegionMaster::install()
	GameServerMessageArchive::install();
	AiCombatPulseQueue::install();

	// install the object templates
	ServerArmorTemplate::install(true);
	ServerBattlefieldMarkerObjectTemplate::install(true);
	ServerBuildingObjectTemplate::install(true);
	ServerCellObjectTemplate::install();
	ServerCityObjectTemplate::install(true);
	ServerConstructionContractObjectTemplate::install(true);
	ServerCreatureObjectTemplate::install(true);
	ServerDraftSchematicObjectTemplate::install(true);
	ServerFactoryObjectTemplate::install(true);
	ServerGroupObjectTemplate::install(true);
	ServerGuildObjectTemplate::install(true);
	ServerHarvesterInstallationObjectTemplate::install(true);
	ServerInstallationObjectTemplate::install(true);
	ServerIntangibleObjectTemplate::install(true);
	ServerManufactureInstallationObjectTemplate::install(true);
	ServerManufactureSchematicObjectTemplate::install(true);
	ServerMissionObjectTemplate::install(true);
	ServerObjectTemplate::install(true);
	ServerPlanetObjectTemplate::install(true);
	ServerPlayerQuestObjectTemplate::install(true);
	ServerPlayerObjectTemplate::install(true);
	ServerResourceContainerObjectTemplate::install(true);
	ServerShipObjectTemplate::install(true);
	ServerStaticObjectTemplate::install(true);
	ServerTangibleObjectTemplate::install(true);
	ServerUniverseObjectTemplate::install(true);
	ServerVehicleObjectTemplate::install(true);
	ServerWeaponObjectTemplate::install(true);
	ServerXpManagerObjectTemplate::install(true);

	SharedBattlefieldMarkerObjectTemplate::install(true);
	SharedBuildingObjectTemplate::install(true);
	SharedCellObjectTemplate::install(true);
	SharedConstructionContractObjectTemplate::install(true);
	SharedCreatureObjectTemplate::install(true);
	SharedDraftSchematicObjectTemplate::install(true);
	SharedFactoryObjectTemplate::install(true);
	SharedGroupObjectTemplate::install(true);
	SharedGuildObjectTemplate::install(true);
	SharedInstallationObjectTemplate::install(true);
	SharedIntangibleObjectTemplate::install(true);
	SharedManufactureSchematicObjectTemplate::install(true);
	SharedMissionObjectTemplate::install(true);
	SharedObjectTemplate::install(true);
	SharedPlayerQuestObjectTemplate::install(true);
	SharedPlayerObjectTemplate::install(true);
	SharedResourceContainerObjectTemplate::install(true);
	SharedShipObjectTemplate::install(true);
	SharedStaticObjectTemplate::install(true);
	SharedTangibleObjectTemplate::install(true);
	SharedTerrainSurfaceObjectTemplate::install(true);
	SharedUniverseObjectTemplate::install(true);
	SharedVehicleObjectTemplate::install(true);
	SharedWeaponObjectTemplate::install(true);

	ShipComponentDescriptor::fetchSharedObjectTemplates();

	m_weatherGenerator = new SynchronizedWeatherGenerator(ConfigServerGame::getWeatherUpdateSeconds());

	FormManagerServer::install();

	// install the world
	World::install();

	m_idleTimer->setExpireTime(1.0f);
	m_installed = true;

	m_sceneId = new std::string(ConfigServerGame::getSceneID());

	Pvp::getScheduler().setCallback(Pvp::updateTimedFlags, nullptr, ConfigServerGame::getPvpUpdateTimeMs());

	NebulaManagerServer::loadScene(*m_sceneId);

	CollisionWorld::handleSceneChange(*m_sceneId);
}

//-----------------------------------------------------------------------

void ServerWorld::updateObjectDatabase(ServerObject & movingObject)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::updateObjectDatabase");
	g_objectSphereTree->onObjectMoved(&movingObject);
}

// ----------

void ServerWorld::updateTriggerDatabase(ServerObject & movingObject)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::updateTriggerDatabase");

	// move the trigger volumes
	ServerObject::TriggerVolumeMap const &volumes = movingObject.getTriggerVolumeMap();
	for (ServerObject::TriggerVolumeMap::const_iterator v = volumes.begin(); v != volumes.end(); ++v)
	{
		int system = ConfigServerGame::getTriggerVolumeSystem();  // 0 = old, 1 = compare, 2 = new
		if (system <= 1)
		{
			PROFILER_AUTO_BLOCK_DEFINE("sphere_tree_move");
			g_triggerSphereTree->onObjectMoved((*v).second);
		}

		if (system >= 1)
		{
			PROFILER_AUTO_BLOCK_DEFINE("sphere_grid_move");
			g_triggerSphereGrid->onObjectMoved((*v).second);
		}
	}
}

// ----------
// handle triggers caused by a moving object hitting a trigger

void ServerWorld::triggerMovingObjects(ServerObject &movingObject, Vector const &start, Vector const &end)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::triggerMovingObjects");

	if (!isRelevantToTriggerVolumes(movingObject))
		return;

	if (isSpaceScene())
	{
		// In space, we only care about objects moving within a pob, and start and
		// end aren't real useful, since we need position change relative to the
		// pob, which we currently don't store.  With this implementation,
		// an object completely crossing a trigger volume in one move will not
		// trigger it.
		Object const * const pob = getContainingPobForObjectInWorld(movingObject);
		if (pob)
		{
			static std::vector<TriggerVolume *> results;
			static std::set<TriggerVolume *> results2;

			int system = ConfigServerGame::getTriggerVolumeSystem();  // 0 = old, 1 = compare, 2 = new

			if (system <= 1)
				g_triggerSphereTree->findInRange(pob, Vector::zero, 16384.f, results);
			if (system >= 1)
				g_triggerSphereGrid->findInRange(pob, Vector::zero, 16384.f, results2);

			if (system == 1)
			{
				DEBUG_FATAL(results.size() != results2.size(), ("SphereGrid failed (%d vs. %d).", results.size(), results2.size()));
			}

			if (system != 2)
			{
				for (std::vector<TriggerVolume *>::const_iterator i = results.begin(); i != results.end(); ++i)
					if ((*i) && &(*i)->getOwner() != &movingObject)
						(*i)->objectMoved(movingObject);
			}
			else if (system == 2)
			{
				for (std::set<TriggerVolume *>::const_iterator i = results2.begin(); i != results2.end(); ++i)
					if ((*i) && &(*i)->getOwner() != &movingObject)
						(*i)->objectMoved(movingObject);
			}

			results.clear();
			results2.clear();
		}
	}
	else
	{
		float const distanceTravelledSquared = end.magnitudeBetweenSquared(start);
		float const extentSphereRadius = movingObject.getSphereExtent().getRadius();

		static std::vector<TriggerVolume *> results;
		static std::set<TriggerVolume *> results2;

		int system = ConfigServerGame::getTriggerVolumeSystem();  // 0 = old, 1 = compare, 2 = new

		if (distanceTravelledSquared < sqr(ConfigServerGame::getTriggerVolumeWarpDistance()))
		{
			Capsule const queryCapsule(start, end, extentSphereRadius);
			{
				if (system <= 1)
				{
					PROFILER_AUTO_BLOCK_DEFINE("sphere_tree_find");
					g_triggerSphereTree->findInRange(queryCapsule, results);
				}

				if (system >= 1)
				{
					PROFILER_AUTO_BLOCK_DEFINE("sphere_grid_find");
					g_triggerSphereGrid->findInRange(queryCapsule, results2);
				}

				if (system == 1)
				{
					compare_results(queryCapsule, results, results2, &movingObject);
				}
			}
		}
		else
		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::triggerMovingObject::findInRange (teleport)");

			// We moved really far, so consider this a warp
			if (system <= 1)
			{
				PROFILER_AUTO_BLOCK_DEFINE("sphere_tree_find");
				g_triggerSphereTree->findInRange(end, extentSphereRadius, results);
			}

			if (system >= 1)
			{
				PROFILER_AUTO_BLOCK_DEFINE("sphere_grid_find");
				g_triggerSphereGrid->findInRange(end, extentSphereRadius, results2);
			}

			if (system == 1)
			{
				compare_results(end, extentSphereRadius, results, results2, &movingObject);
			}
		}

		if (system != 2)
		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::triggerMovingObject::loop");
			for (std::vector<TriggerVolume *>::const_iterator i = results.begin(); i != results.end(); ++i)
				if ((*i) && &(*i)->getOwner() != &movingObject)
					(*i)->moveObject(movingObject, start, end);
		}
		else if (system == 2)
		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::triggerMovingObject::loop");
			for (std::set<TriggerVolume *>::const_iterator i = results2.begin(); i != results2.end(); ++i)
				if ((*i) && &(*i)->getOwner() != &movingObject)
					(*i)->moveObject(movingObject, start, end);
		}

		results.clear();
		results2.clear();
	}
}

// ----------
// handle triggers caused by a moving trigger hitting an object

void ServerWorld::triggerMovingTriggers(ServerObject &movingObject, Vector const &start, Vector const &end)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::triggerMovingTriggers");

	if (isSpaceScene())
	{
		// In space, we only care about triggers moving within a pob, and start
		// and end aren't real useful, since we need position change relative to
		// the pob, which we currently don't store.  With this implementation,
		// a trigger volume completely crossing an object in one move will not
		// trigger it.
		Object const * const pob = getContainingPobForObjectInWorld(movingObject);
		if (pob)
		{
			TriggerVolumeFilterWithIgnoredObject const filter(movingObject);

			ServerObject::TriggerVolumeMap &volumes = movingObject.getTriggerVolumeMap();

			for (ServerObject::TriggerVolumeMap::iterator v = volumes.begin(); v != volumes.end(); ++v)
			{
				TriggerVolume * const t = (*v).second;
				static std::vector<ServerObject *> results;
				g_objectSphereTree->findInRange(pob, t->getOwner().getPosition_p(), t->getRadius(), filter, results);
				std::sort(results.begin(), results.end());
				t->contentsChanged(results);
				results.clear();
			}
		}
	}
	else
	{
		ServerObject::TriggerVolumeMap &volumes = movingObject.getTriggerVolumeMap();

		DEBUG_REPORT_LOG(ms_logTriggerStats, ("**** Outloop %d\n", volumes.size()));
		for (ServerObject::TriggerVolumeMap::iterator v = volumes.begin(); v != volumes.end(); ++v)
		{
			TriggerVolume * const t = (*v).second;

			static std::vector<ServerObject *> results;

			Capsule const queryCapsule(start, end, t->getRadius());
			{
				PROFILER_AUTO_BLOCK_DEFINE("SphereTree::findInRange");
				g_objectSphereTree->findInRange(queryCapsule, TriggerVolumeFilter(), results);
			}

			DEBUG_REPORT_LOG(ms_logTriggerStats, ("     **** Innerloop %d\n", results.size()));
			{
				PROFILER_AUTO_BLOCK_DEFINE("MoveTriggerVolumes");
				int clcount = 0;
				int crcount = 0;
				for (std::vector<ServerObject *>::iterator i = results.begin(); i != results.end(); ++i)
				{
					ServerObject * const object = *i;
					if (ms_logTriggerStats)
					{
						crcount++;
						if (object->getClient())
							clcount++;
					}

					if (object != nullptr)
						t->moveTriggerVolume(*object, start, end);
				}
				DEBUG_REPORT_LOG(ms_logTriggerStats, ("       Creature count: %d Client count: %d\n", crcount, clcount));
			}

			results.clear();
		}
	}
}

// ----------------------------------------------------------------------

void ServerWorld::moveObject(ServerObject & movingObject, const Vector & start, const Vector & end)
{
	DEBUG_FATAL(!s_moveObjectListValid, ("Cannot operate on uninitialized object list"));
	//If there are no move lists, execute the move right away.  Otherwise queue it up
	if (s_numMoveLists <= 0 || !s_moveObjectListValid)
	{
		internalMoveObject(movingObject, start, end);
	}
	else
	{
		FATAL(s_moveObjectListLock, ("Trying to move an object while the move list is locked"));
		unsigned int whichList = movingObject.getNetworkId().getHashValue() % s_numMoveLists;
		MoveObjectMap * moveMap = s_moveObjectList[whichList];
		MoveObjectMap::iterator i = moveMap->find(&movingObject);
		if (i != moveMap->end())
		{
			//If the object is already scheduled to be moved, update the end position.
			i->second.second = end;
		}
		else
		{
			//If the object is not already scheduled to be moved, add it to the map.
			(*moveMap)[&movingObject] = std::pair<Vector, Vector>(start, end);
		}
	}
}

//-----------------------------------------------------------------------

void ServerWorld::updateMoveObjectList()
{
	DEBUG_FATAL(!s_moveObjectListValid, ("Cannot operate on uninitialized object list"));

	if (s_numMoveLists <= 0 || !s_moveObjectListValid)
		return;

	static int frameCounter = 0;

	s_moveObjectListLock = true;
	if (++frameCounter >= s_numMoveLists)
		frameCounter = 0;

	MoveObjectMap * moveMap = s_moveObjectList[frameCounter];
	MoveObjectMap::iterator i = moveMap->begin();
	for (; i != moveMap->end(); ++i)
	{
		NOT_NULL(i->first);
		internalMoveObject(*(i->first), i->second.first, i->second.second);
	}
	moveMap->clear();
	s_moveObjectListLock = false;
}

//-----------------------------------------------------------------------

void ServerWorld::internalMoveObject(ServerObject & movingObject, const Vector & start, const Vector & end)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::moveObject");

	// check for NaN
	if (start == start && end == end && movingObject.getSphereExtent().getRadius() == movingObject.getSphereExtent().getRadius())
	{
		updateObjectDatabase(movingObject);
		updateTriggerDatabase(movingObject);
		triggerMovingObjects(movingObject, start, end);
		triggerMovingTriggers(movingObject, start, end);
	}
	else
	{
		DEBUG_FATAL(start != start, ("Object %s:%s is moving from an invalid start position (NaN)", movingObject.getObjectTemplateName(), movingObject.getNetworkId().getValueString().c_str()));
		DEBUG_FATAL(end != end, ("Object %s:%s is moving to an invalid end position (NaN)", movingObject.getObjectTemplateName(), movingObject.getNetworkId().getValueString().c_str()));
		DEBUG_FATAL(start != start, ("Object %s:%s is moving from an invalid start position (NaN)", movingObject.getObjectTemplateName(), movingObject.getNetworkId().getValueString().c_str()));
		DEBUG_FATAL(movingObject.getSphereExtent().getRadius() != movingObject.getSphereExtent().getRadius(), ("Object %s:%s has an invalid sphere extent radius %f", movingObject.getNetworkId().getValueString().c_str(), movingObject.getSphereExtent().getRadius()));
	}
}

//------------------------------------------------------------------------------------------

void ServerWorld::remove()
{
	DEBUG_FATAL(!m_installed, ("Trying to reremove ServerWorld"));

	DebugFlags::unregisterFlag(ms_logTriggerStats);

	World::remove();

	delete m_sceneId;
	m_sceneId = 0;

	delete g_objectSphereTree;
	g_objectSphereTree = 0;
	delete g_triggerSphereTree;
	g_triggerSphereTree = 0;

	delete g_triggerSphereGrid;
	g_triggerSphereGrid = 0;

	delete m_idleTimer;
	m_idleTimer = 0;

	delete m_weatherGenerator;
	m_weatherGenerator = 0;

	for (MoveObjectList::iterator moveIter = s_moveObjectList.begin(); moveIter != s_moveObjectList.end(); ++moveIter)
	{
		delete *moveIter;
	}
	s_moveObjectList.clear();
	s_moveObjectListValid = false;

	gs_pendingConcludeVector.clear();

	CollisionWorld::setNearWarpWarningCallback(nullptr);
	CollisionWorld::setFarWarpWarningCallback(nullptr);

	Pvp::remove();

	m_installed = false;
}

//-----------------------------------------------------------------------

void ServerWorld::removeObjectTriggerVolume(TriggerVolume * triggerVolume)
{
	int system = ConfigServerGame::getTriggerVolumeSystem();  // 0 = old, 1 = compare, 2 = new
	if (system <= 1)
	{
		PROFILER_AUTO_BLOCK_DEFINE("sphere_tree_remove");
		g_triggerSphereTree->onObjectRemoved(triggerVolume);
	}

	if (system >= 1)
	{
		PROFILER_AUTO_BLOCK_DEFINE("sphere_grid_remove");
		g_triggerSphereGrid->onObjectRemoved(triggerVolume);
	}
}

//-----------------------------------------------------------------------
/** Clean up the various lists associated with objects being destroyed.
 */
void ServerWorld::removeObjectFromGame(const ServerObject& object)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::removeObjectFromGame");
	FATAL(gs_pendingConcludeLock, ("Attempt to delete an object DURING CONCLUDE"));
	std::vector<Object *>::iterator vf = std::find(gs_pendingConcludeVector.begin(), gs_pendingConcludeVector.end(), const_cast<ServerObject *>(&object));
	if (vf != gs_pendingConcludeVector.end())
	{
		*vf = gs_pendingConcludeVector.back();
		gs_pendingConcludeVector.pop_back();
	}
	else
	{
		std::vector<std::pair<int, ServerObject *> >::iterator po = gs_pendingConcludeOpsVector.begin();
		for (; po != gs_pendingConcludeOpsVector.end(); ++po)
		{
			if ((*po).second == &object)
			{
				IGNORE_RETURN(gs_pendingConcludeOpsVector.erase(po));
				break;
			}
		}
	}

	if (s_numMoveLists > 0 && s_moveObjectListValid)
	{
		FATAL(s_moveObjectListLock, ("Trying to delete an object while the move lists are locked"));
		unsigned int whichList = object.getNetworkId().getHashValue() % s_numMoveLists;
		MoveObjectMap * moveMap = s_moveObjectList[whichList];
		MoveObjectMap::iterator i = moveMap->find(const_cast<ServerObject*>(&object));
		if (i != moveMap->end())
		{
			moveMap->erase(i);
		}
	}
}

//-----------------------------------------------------------------------

void ServerWorld::removeTangibleObject(ServerObject *object)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::removeTangibleObject");
	DEBUG_WARNING(!object, ("removeTangibleObject() was called with a nullptr object parameter, this is probably not what the program intended to do"));

	if (!object)
		return;

	if (!object->isInWorld())
	{
		WARNING_STRICT_FATAL(!object->isInWorld(), ("Tried to remove an object that was not in the world!"));
		return;
	}

	// if the object is being destroyed, the trigger has already gone off
	// player creatures aren't destroyed before being removed, so it's safe to invoke
	// the trigger here
	if (object->isAuthoritative() && (object->getScriptObject() != nullptr) && !object->isBeingDestroyed())
	{
		ScriptParams params;
		IGNORE_RETURN(object->getScriptObject()->trigAllScripts(Scripting::TRIG_REMOVING_FROM_WORLD, params));
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("removeTangibleObject - World::removeObject");
		if (object->isBeingDestroyed() && World::removeObject(object, static_cast<int>(WOL_Inactive)))
		{
			DEBUG_REPORT_LOG(true, ("Removing deleted object %s from world\n", object->getNetworkId().getValueString().c_str()));
			return;
		}
	}

	DEBUG_REPORT_LOG(ConfigServerGame::getLogObjectLoading(), ("Removing object %s from world\n", object->getNetworkId().getValueString().c_str()));

	{
		PROFILER_AUTO_BLOCK_DEFINE("removeTangibleObject - World::onRemovingFromWorld");
		object->onRemovingFromWorld();
	}

	if (object->isAuthoritative())
	{
		// advise planet server that the object is gone
		PlanetRemoveObject const removeMessage(object->getNetworkId());
		GameServer::getInstance().sendToPlanetServer(removeMessage);
	}

	if (!object->asCellObject())
	{
		PROFILER_AUTO_BLOCK_DEFINE("removeTangibleObject - Sphere tree and trigger volume cleanup");

		// tell all interested trigger volume about the removal
		if (object->getTriggerVolumeEntered())
		{
			std::set<TriggerVolume *> const triggerVolumeEntered = *(object->getTriggerVolumeEntered());
			for (std::set<TriggerVolume *>::const_iterator i = triggerVolumeEntered.begin(); i != triggerVolumeEntered.end(); ++i)
				(*i)->removeObject(*object);
		}

		// Now remove the objects in our trigger volume
		ServerObject::TriggerVolumeMap &volumes = object->getTriggerVolumeMap();
		for (ServerObject::TriggerVolumeMap::iterator v = volumes.begin(); v != volumes.end(); ++v)
		{
			TriggerVolume * const t = (*v).second;
			TriggerVolume::ContentsSet const contents = object->getTriggerVolume(t->getName())->getContents();
			for (TriggerVolume::ContentsSet::const_iterator objectIterator = contents.begin(); objectIterator != contents.end(); ++objectIterator)
				t->removeObject(**objectIterator);
			// Now remove the actual trigger volume
			removeObjectTriggerVolume(t);
		}
		// remove the object sphere
		g_objectSphereTree->onObjectRemoved(object);
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("removeTangibleObject - World::removeObject 2");
		IGNORE_RETURN(World::removeObject(object, static_cast<int>(WOL_Tangible)));
	}

	//If the object is a ref obejct, remove it.
	{
		PROFILER_AUTO_BLOCK_DEFINE("removeTangibleObject - removeReferenceObject");
		TerrainObject * const terrain = TerrainObject::getInstance();
		if (terrain && terrain->isReferenceObject(object))
			terrain->removeReferenceObject(object);
	}
}

//-----------------------------------------------------------------------

void ServerWorld::update(real time)
{
	DEBUG_FATAL(!m_installed, ("ServerWorld is NOT installed!"));

	//-- update network
	{
		PROFILER_AUTO_BLOCK_DEFINE("NetworkHandler::dispatch()");
		NetworkHandler::dispatch();
	}

	// scheduler code
	World::beginFrame();
	{
		{
			PROFILER_AUTO_BLOCK_DEFINE("Alter");
			AlterScheduler::alter(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::runSpawnQueue");
			CreatureObject::runSpawnQueue();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::updateMissionRequestQueue");
			CreatureObject::updateMissionRequestQueue();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("CreatureObject::runMissionCreationQueue");
			CreatureObject::runMissionCreationQueue();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("TriggerVolume::runScriptTriggers");
			TriggerVolume::runScriptTriggers();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("Schedulers");
			{
				PROFILER_AUTO_BLOCK_DEFINE("Scheduler:Script");
				GameScriptObject::getScriptScheduler().update(Clock::timeMs());
			}

			{
				PROFILER_AUTO_BLOCK_DEFINE("Scheduler:Observe");
				ObserveTracker::update();
			}

			{
				PROFILER_AUTO_BLOCK_DEFINE("Scheduler:PositionUpdate");
				PositionUpdateTracker::getScheduler().update(Clock::timeMs());
			}

			{
				PROFILER_AUTO_BLOCK_DEFINE("Scheduler:Logout");
				LogoutTracker::getScheduler().update(Clock::timeMs());
			}

			{
				PROFILER_AUTO_BLOCK_DEFINE("Scheduler:Pvp");
				Pvp::getScheduler().update(Clock::timeMs());
			}
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("TeleportFixupHandler::update");
			TeleportFixupHandler::update(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("CombatTracker::update");
			CombatTracker::update();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("AuthTransferTracker::update");
			AuthTransferTracker::update();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerPathfinding::update");
			ServerPathfinding::update(15.0f / 1000.0f);  // 15 msec for pathfinding max
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::update");
			CollisionWorld::update(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ProjectileManager::update");
			ProjectileManager::update(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("BaselinesManager::update");
			ServerObjectBaselinesManager::update();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ContainmentMessageManager::update");
			ContainmentMessageManager::update();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("DestroyMessageManager::update");
			DestroyMessageManager::update();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("PlayerSanityChecker::update");
			PlayerSanityChecker::update(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("GuildInterface::update");
			GuildInterface::update();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("MissileManager::update");
			MissileManager::getInstance().update();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerAsteroidManager::update");
			ServerAsteroidManager::update(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ShipClientUpdateTracker::update");
			ShipClientUpdateTracker::update(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ShipAiEnemySearchManager::update");
			ShipAiEnemySearchManager::update();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("NebulaManagerServer::update");
			NebulaManagerServer::update(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ShipInternalDamageOverTimeManager::update");
			ShipInternalDamageOverTimeManager::update(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("SpaceSquadManager::alter");
			SpaceSquadManager::alter(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("SpacePathManager::alter");
			SpacePathManager::alter(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("AiCombatPulseQueue::alter");
			AiCombatPulseQueue::alter(time);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::conclude");

			ServerObject::concludeScriptVars();
			static std::vector<ServerObject *> deferredConcludes; // for objects not initialized
			deferredConcludes.clear();

			if (!gs_pendingConcludeVector.empty())
			{
				{
					PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::emptyPending");
					std::vector<Object *>::iterator concludeIter;
					gs_pendingConcludeLock = true;
					for (concludeIter = gs_pendingConcludeVector.begin(); concludeIter != gs_pendingConcludeVector.end(); ++concludeIter)
					{
						ServerObject *o = (*concludeIter)->asServerObject();
						WARNING_STRICT_FATAL(!o, ("nullptr object in conclude list!"));
						if (o)
						{
							if (o->isInitialized())
								o->conclude();
							else
								deferredConcludes.push_back(o);
						}
					}
					gs_pendingConcludeVector.clear();
					gs_pendingConcludeLock = false;
				}
				{
					PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::pendingConcludeOps");
					std::vector<std::pair<int, ServerObject *> >::iterator po;
					for (po = gs_pendingConcludeOpsVector.begin(); po != gs_pendingConcludeOpsVector.end(); ++po)
					{
						switch ((*po).first)
						{
						case 0:
							addObjectToConcludeList((*po).second);
							break;
						case 1:
							removeObjectFromGame(*(*po).second);
							break;
						default:
							break;
						}
					}
					gs_pendingConcludeOpsVector.clear();
				}
				{
					PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::addDeferredConclude");
					std::vector<ServerObject *>::iterator i;
					for (i = deferredConcludes.begin(); i != deferredConcludes.end(); ++i)
					{
						addObjectToConcludeList(*i);
					}
				}
			}
		}

		if (s_numMoveLists > 0)
		{
			PROFILER_AUTO_BLOCK_DEFINE("updateMoveObjectList");
			updateMoveObjectList();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("PvpUpdateObserver_update");
			PvpUpdateObserver::update();
		}

#ifdef DEBUG_WEATHER
		static int weatherFrameCount = 0;
		if (ServerClock::getInstance().isSet() && (--weatherFrameCount <= 0))
		{
			weatherFrameCount = 10;
			std::string buffer;
			getWeather().debugPrint(buffer);
			DEBUG_REPORT_LOG(true, ("Weather data is %s\n", buffer.c_str()));
		}
#endif
	}

	updatePlanetServer();

	LineOfSightCache::update();

	World::endFrame();
}

//-----------------------------------------------------------------------

const std::string &ServerWorld::getSceneId()
{
	NOT_NULL(m_sceneId);
	return *m_sceneId;
}

//-----------------------------------------------------------------------

const ObjectNotification &ServerWorld::getTangibleNotification()
{
	return ServerWorldTangibleNotification::getInstance();
}

//-----------------------------------------------------------------------

const ObjectNotification &ServerWorld::getTerrainObjectNotification()
{
	return ServerWorldTerrainObjectNotification::getInstance();
}

//-----------------------------------------------------------------------

const ObjectNotification &ServerWorld::getIntangibleNotification()
{
	return ServerWorldIntangibleNotification::getInstance();
}

//-----------------------------------------------------------------------

const ObjectNotification &ServerWorld::getUniverseNotification()
{
	return ServerWorldUniverseNotification::getInstance();
}

//-------------------------------------------------------------------

const WeatherData & ServerWorld::getWeather()
{
	NOT_NULL(m_weatherGenerator);
	return m_weatherGenerator->getWeather(ServerClock::getInstance().getGameTimeSeconds());
}

//-----------------------------------------------------------------------

void ServerWorld::addLoadBeacon(const TangibleObject * loadBeacon)
{
	ServerWorldNamespace::s_loadBeaconEntries.push_back(loadBeacon);
}

//-----------------------------------------------------------------------

void ServerWorld::removeLoadBeacon(const TangibleObject * loadBeacon)
{
	std::vector<const TangibleObject *>::iterator f = std::find(ServerWorldNamespace::s_loadBeaconEntries.begin(), ServerWorldNamespace::s_loadBeaconEntries.end(), loadBeacon);
	if (f != ServerWorldNamespace::s_loadBeaconEntries.end())
		IGNORE_RETURN(ServerWorldNamespace::s_loadBeaconEntries.erase(f));
}

//-----------------------------------------------------------------------

bool ServerWorld::isInLoadBeaconRange(const Vector & worldPosition)
{
	std::vector<const TangibleObject *>::const_iterator i;
	for (i = ServerWorldNamespace::s_loadBeaconEntries.begin(); i != ServerWorldNamespace::s_loadBeaconEntries.end(); ++i)
	{
		float radius = static_cast<float>((*i)->getInterestRadius()) + 300.0f;
		radius = radius * radius * 2;
		Vector beaconPosition = (*i)->getTransform_o2w().getPosition_p();
		if (beaconPosition.magnitudeBetweenSquared(worldPosition) < radius)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------

/**
 * Finds a "good" location in the world where we can place an object. Good is
 * defined as a resonably flat area, not in water, where the object being placed
 * won't intersect other objects. The caller may disable the slope or water check
 * if they want.
 *
 * @param areaSizeX		       the x size that should be clear
 * @param areaSizeZ		       the z size that should be clear
 * @param searchRectLowerLeftLocation     ll position of the area to look for a good location
 * @param searchRectUpperRightLocation    ur position of the area to look for a good location
 * @param dontCheckWater		  flag to skip the underwater check
 * @param dontCheckSlope		  flag to skip the slope check
 *
 * @return a position in the world where the object can be placed, or 0 0 0 if
 * there is no valid position
 */
Vector ServerWorld::getGoodLocation(float areaSizeX, float areaSizeZ,
	const Vector & searchRectLowerLeftLocation,
	const Vector & searchRectUpperRightLocation,
	bool dontCheckWater, bool dontCheckSlope)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::getGoodLocation");

	Vector goodLocation;

	TerrainObject const * const terrainObject = TerrainObject::getInstance();
	if (!terrainObject)
	{
		DEBUG_WARNING(true, ("getGoodLocation (): PB there is no terrain system"));
		return goodLocation;
	}

	LotManager const * const lotManager = ServerWorld::getConstLotManager();
	if (!lotManager)
	{
		DEBUG_WARNING(true, ("getGoodLocation (): PB there is no lot system"));
		return goodLocation;
	}

	//build searching rectangles from the given locations
	const float minimum = -8192.f + 512.f;
	const float maximum = 8192.f - 512.f;

	Rectangle2d searchRect;
	searchRect.x0 = searchRectLowerLeftLocation.x;
	searchRect.x0 = clamp(minimum, searchRectLowerLeftLocation.x, maximum);
	searchRect.y0 = searchRectLowerLeftLocation.z;
	searchRect.y0 = clamp(minimum, searchRectLowerLeftLocation.z, maximum);
	searchRect.x1 = searchRectUpperRightLocation.x;
	searchRect.x1 = clamp(minimum, searchRectUpperRightLocation.x, maximum);
	searchRect.y1 = searchRectUpperRightLocation.z;
	searchRect.y1 = clamp(minimum, searchRectUpperRightLocation.z, maximum);

	//validate scripter-input rectangle areas > 0
	if (searchRect.x0 > searchRect.x1)
	{
		DEBUG_WARNING(true, ("getGoodLocation (): DB searchRect.x0 (%1.2f) > searchRect.x1 (%1.2f)", searchRect.x0, searchRect.x1));
		return goodLocation;
	}

	if (searchRect.y0 > searchRect.y1)
	{
		DEBUG_WARNING(true, ("getGoodLocation (): DB searchRect.z0 (%1.2f) > searchRect.z1 (%1.2f)", searchRect.y0, searchRect.y1));
		return goodLocation;
	}

	//validate that the area we want to find is smaller than our search area
	if (areaSizeX > searchRect.getWidth())
	{
		DEBUG_WARNING(true, ("getGoodLocation (): DB goal rectangle x (%1.2f) is larger than our search rectangle width (%1.2f)", areaSizeX, searchRect.getWidth()));
		return goodLocation;
	}

	if (areaSizeZ > searchRect.getHeight())
	{
		DEBUG_WARNING(true, ("getGoodLocation (): DB goal rectangle y (%1.2f) is larger than our search rectangle height (%1.2f)", areaSizeZ, searchRect.getHeight()));
		return goodLocation;
	}

	Rectangle2d startingRect;
	startingRect.x0 = searchRect.x0;
	startingRect.y0 = searchRect.y0;
	startingRect.x1 = startingRect.x0 + areaSizeX;
	startingRect.y1 = startingRect.y0 + areaSizeZ;

	//generate vectors with a random raster range
	int numRows = static_cast<int>(floor(searchRect.getWidth() / areaSizeX));
	int numCols = static_cast<int>(floor(searchRect.getHeight() / areaSizeZ));
	static std::vector<int> xRange;
	static std::vector<int> yRange;
	xRange.clear();
	yRange.clear();
	for (int i = 0; i < numRows; ++i)
		xRange.push_back(i);
	for (int j = 0; j < numCols; ++j)
		yRange.push_back(j);

	std::random_shuffle(xRange.begin(), xRange.end());
	std::random_shuffle(yRange.begin(), yRange.end());

	Rectangle2d successRect;
	bool success = false;
	Rectangle2d currentRect;
	for (std::vector<int>::iterator it_x = xRange.begin(); it_x != xRange.end() && !success; ++it_x)
	{
		for (std::vector<int>::iterator it_y = yRange.begin(); it_y != yRange.end() && !success; ++it_y)
		{
			currentRect = startingRect;

			//jump to the random raster area
			currentRect.translate(*it_x * areaSizeX, *it_y * areaSizeZ);

			//the rectangle is a "good location" if it has neither water nor a steep slope
			if (!dontCheckWater && terrainObject->getWater(currentRect))
				continue;

			if (!dontCheckSlope && terrainObject->getSlope(currentRect))
				continue;

			if (!lotManager->canPlace(currentRect))
				continue;

			success = true;
			successRect = currentRect;
		}
	}

	if (!success)
		return goodLocation;

	//validate that our success point lies within the search area (debugging tests)
	if (successRect.x0 < searchRect.x0 || successRect.x1 > searchRect.x1 || successRect.y0 < searchRect.y0 || successRect.y1 > searchRect.y1)
	{
		DEBUG_WARNING(true, ("getGoodLocation (): PB result does not fit within the search location"));
		return goodLocation;
	}

	//get the center as a 2d point
	Vector2d successLoc2d = successRect.getCenter();

	//conver that point into a location-friendly 3d point
	goodLocation = Vector(successLoc2d.x, 0.f, successLoc2d.y);

	IGNORE_RETURN(terrainObject->getHeightForceChunkCreation(goodLocation, goodLocation.y));

	return goodLocation;
}	// ServerWorld::getGoodLocation

//-----------------------------------------------------------------------

/**
* Finds a "good" location in the world where we can place an object. Good is
* defined as a resonably flat area, not in water, where the object being placed
* won't intersect other objects. The caller may disable the slope or water check
* if they want. This function checks for nearby collidable static objects as well.
*
* @param areaSizeX		       the x size that should be clear
* @param areaSizeZ		       the z size that should be clear
* @param searchRectLowerLeftLocation     ll position of the area to look for a good location
* @param searchRectUpperRightLocation    ur position of the area to look for a good location
* @param dontCheckWater		  flag to skip the underwater check
* @param dontCheckSlope		  flag to skip the slope check
* @param minStaticObjDistance the minimum distance a static object must be away from a "good" location
*
* @return a position in the world where the object can be placed, or 0 0 0 if
* there is no valid position
*/
Vector ServerWorld::getGoodLocationAvoidCollidables(float areaSizeX, float areaSizeZ,
	const Vector & searchRectLowerLeftLocation,
	const Vector & searchRectUpperRightLocation,
	bool dontCheckWater, bool dontCheckSlope, float minStaticObjDistance)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::getGoodLocation");

	Vector goodLocation;

	TerrainObject const * const terrainObject = TerrainObject::getInstance();
	if (!terrainObject)
	{
		DEBUG_WARNING(true, ("getGoodLocation (): PB there is no terrain system"));
		return goodLocation;
	}

	LotManager const * const lotManager = ServerWorld::getConstLotManager();
	if (!lotManager)
	{
		DEBUG_WARNING(true, ("getGoodLocation (): PB there is no lot system"));
		return goodLocation;
	}

	//build searching rectangles from the given locations
	const float minimum = -8192.f + 512.f;
	const float maximum = 8192.f - 512.f;

	Rectangle2d searchRect;
	searchRect.x0 = searchRectLowerLeftLocation.x;
	searchRect.x0 = clamp(minimum, searchRectLowerLeftLocation.x, maximum);
	searchRect.y0 = searchRectLowerLeftLocation.z;
	searchRect.y0 = clamp(minimum, searchRectLowerLeftLocation.z, maximum);
	searchRect.x1 = searchRectUpperRightLocation.x;
	searchRect.x1 = clamp(minimum, searchRectUpperRightLocation.x, maximum);
	searchRect.y1 = searchRectUpperRightLocation.z;
	searchRect.y1 = clamp(minimum, searchRectUpperRightLocation.z, maximum);

	//validate scripter-input rectangle areas > 0
	if (searchRect.x0 > searchRect.x1)
	{
		DEBUG_WARNING(true, ("getGoodLocation (): DB searchRect.x0 (%1.2f) > searchRect.x1 (%1.2f)", searchRect.x0, searchRect.x1));
		return goodLocation;
	}

	if (searchRect.y0 > searchRect.y1)
	{
		DEBUG_WARNING(true, ("getGoodLocation (): DB searchRect.z0 (%1.2f) > searchRect.z1 (%1.2f)", searchRect.y0, searchRect.y1));
		return goodLocation;
	}

	//validate that the area we want to find is smaller than our search area
	if (areaSizeX > searchRect.getWidth())
	{
		DEBUG_WARNING(true, ("getGoodLocation (): DB goal rectangle x (%1.2f) is larger than our search rectangle width (%1.2f)", areaSizeX, searchRect.getWidth()));
		return goodLocation;
	}

	if (areaSizeZ > searchRect.getHeight())
	{
		DEBUG_WARNING(true, ("getGoodLocation (): DB goal rectangle y (%1.2f) is larger than our search rectangle height (%1.2f)", areaSizeZ, searchRect.getHeight()));
		return goodLocation;
	}

	Rectangle2d startingRect;
	startingRect.x0 = searchRect.x0;
	startingRect.y0 = searchRect.y0;
	startingRect.x1 = startingRect.x0 + areaSizeX;
	startingRect.y1 = startingRect.y0 + areaSizeZ;

	//generate vectors with a random raster range
	int numRows = static_cast<int>(floor(searchRect.getWidth() / areaSizeX));
	int numCols = static_cast<int>(floor(searchRect.getHeight() / areaSizeZ));
	static std::vector<int> xRange;
	static std::vector<int> yRange;
	xRange.clear();
	yRange.clear();
	for (int i = 0; i < numRows; ++i)
		xRange.push_back(i);
	for (int j = 0; j < numCols; ++j)
		yRange.push_back(j);

	std::random_shuffle(xRange.begin(), xRange.end());
	std::random_shuffle(yRange.begin(), yRange.end());

	Rectangle2d successRect;
	bool success = false;
	Rectangle2d currentRect;
	for (std::vector<int>::iterator it_x = xRange.begin(); it_x != xRange.end() && !success; ++it_x)
	{
		for (std::vector<int>::iterator it_y = yRange.begin(); it_y != yRange.end() && !success; ++it_y)
		{
			currentRect = startingRect;

			//jump to the random raster area
			currentRect.translate(*it_x * areaSizeX, *it_y * areaSizeZ);

			//the rectangle is a "good location" if it has neither water nor a steep slope
			if (!dontCheckWater && terrainObject->getWater(currentRect))
				continue;

			if (!dontCheckSlope && terrainObject->getSlope(currentRect))
				continue;

			if (!lotManager->canPlace(currentRect))
				continue;

			// Look for nearby static collidable objects.
			std::vector<ServerObject*> collidables;
			Vector2d Loc2d = currentRect.getCenter();
			Vector checkLoc = Vector(Loc2d.x, 0.f, Loc2d.y);

			ServerWorld::findStaticCollidableObjectsInRange(checkLoc, minStaticObjDistance, collidables);

			if (!collidables.empty())
				continue;

			success = true;
			successRect = currentRect;
		}
	}

	if (!success)
		return goodLocation;

	//validate that our success point lies within the search area (debugging tests)
	if (successRect.x0 < searchRect.x0 || successRect.x1 > searchRect.x1 || successRect.y0 < searchRect.y0 || successRect.y1 > searchRect.y1)
	{
		DEBUG_WARNING(true, ("getGoodLocation (): PB result does not fit within the search location"));
		return goodLocation;
	}

	//get the center as a 2d point
	Vector2d successLoc2d = successRect.getCenter();

	//convert that point into a location-friendly 3d point
	goodLocation = Vector(successLoc2d.x, 0.f, successLoc2d.y);

	IGNORE_RETURN(terrainObject->getHeightForceChunkCreation(goodLocation, goodLocation.y));

	return goodLocation;
}	// ServerWorld::getGoodLocationAvoidCollidables

// ----------------------------------------------------------------------

void ServerWorld::requestPreloadCompleteTrigger(ServerObject &obj)
{
	if (s_preloadComplete)
		doPreloadCompleteTrigger(obj);
	else
		s_preloadCompleteTriggerObjects.push_back(Watcher<ServerObject>(&obj));
}

// ----------------------------------------------------------------------

void ServerWorld::onPreloadComplete()
{
	if (!s_preloadComplete)
	{
		NamedObjectManager::createZoneObjects();

		s_preloadComplete = true;
		for (std::vector<Watcher<ServerObject> >::const_iterator i = s_preloadCompleteTriggerObjects.begin(); i != s_preloadCompleteTriggerObjects.end(); ++i)
			if ((*i).getPointer())
				doPreloadCompleteTrigger(*((*i).getPointer()));
		s_preloadCompleteTriggerObjects.clear();
	}

	if (isSpaceScene())
		ServerAsteroidManager::setupStaticFields(getSceneId());
}

// ----------------------------------------------------------------------

bool ServerWorld::isPreloadComplete()
{
	return s_preloadComplete;
}

// ----------------------------------------------------------------------

bool ServerWorld::isSpaceScene()
{
	if (!s_checkedForSpaceScene)
	{
		s_spaceScene = (strncmp("space_", getSceneId().c_str(), 6) == 0);
		s_checkedForSpaceScene = true;
	}

	return s_spaceScene;
}

// ----------------------------------------------------------------------

void ServerWorldNamespace::checkSpaceBattlefieldZone()
{
	Iff iff;
	if (iff.open(s_spaceBattlefieldDataTableFilename, true))
	{
		DataTable dataTable;
		dataTable.load(iff);

		char const * const zoneName = ConfigServerGame::getSceneID();

		int const numberOfRows = dataTable.getNumRows();
		for (int row = 0; row < numberOfRows; ++row)
		{
			if (!_stricmp(zoneName, dataTable.getStringValue("zoneName", row)))
			{
				s_isSpaceBattlefieldZone = true;
				s_isSpaceBattlefieldZoneWithPvp = (dataTable.getIntValue("pvp", row) != 0);
				break;
			}
		}
	}
	s_checkedSpaceBattlefieldZone = true;
}

// ----------------------------------------------------------------------

bool ServerWorld::isSpaceBattlefieldZone()
{
	if (!s_checkedSpaceBattlefieldZone)
		checkSpaceBattlefieldZone();

	return s_isSpaceBattlefieldZone;
}

// ----------------------------------------------------------------------

bool ServerWorld::isSpaceBattlefieldZoneWithPvp()
{
	if (!s_checkedSpaceBattlefieldZone)
		checkSpaceBattlefieldZone();

	return s_isSpaceBattlefieldZoneWithPvp;
}

// ----------------------------------------------------------------------

void ServerWorld::getSpatialChatListeners(ServerObject &source, float distance, std::vector<ServerObject *> &results)
{
	if (ServerWorld::isSpaceScene())
	{
		ShipObject * const ship = ShipObject::getContainingShipObject(&source);
		if (ship)
		{
			// All creatures in the ship where the speech is taking place should hear it
			ship->findAllPassengers(*reinterpret_cast<std::vector<CreatureObject *> *>(&results), false);
			// If the speaker is a player in a ship station, then all player ships in range should hear it
			if (source.isPlayerControlled() && !source.isInWorld())
			{
				PlayerShipFilter const filter(ship);
				g_objectSphereTree->findInRange(0, ship->getPosition_p(), distance, filter, results);
			}
		}
	}
	else
		findObjectsInRange(source.findPosition_w(), distance, results);
}

// ----------------------------------------------------------------------

int ServerWorld::getNumMoveLists(std::vector<int> & moveListSize)
{
	moveListSize.clear();

	for (MoveObjectList::const_iterator moveIter = s_moveObjectList.begin(); moveIter != s_moveObjectList.end(); ++moveIter)
	{
		moveListSize.push_back(static_cast<int>((*moveIter)->size()));
	}

	return s_numMoveLists;
}

// ----------------------------------------------------------------------

void ServerWorld::setNumMoveLists(int numMoveLists)
{
	if (!s_moveObjectListValid)
		return;

	// this method can only be used to change the number of move lists;
	// it cannot be used to enable or disable the use of move lists
	if ((s_numMoveLists <= 0) || (numMoveLists <= 0))
		return;

	if (s_numMoveLists == numMoveLists)
		return;

	// save off the original move lists
	MoveObjectList orgMoveObjectList(s_moveObjectList);

	// create new move lists
	s_numMoveLists = numMoveLists;
	s_moveObjectList.clear();

	for (int i = 0; i < s_numMoveLists; ++i)
	{
		s_moveObjectList.push_back(new MoveObjectMap);
	}

	// rebuild new move lists from original move lists and delete original move lists
	for (MoveObjectList::iterator moveIter = orgMoveObjectList.begin(); moveIter != orgMoveObjectList.end(); ++moveIter)
	{
		for (MoveObjectMap::const_iterator i = (*moveIter)->begin(); i != (*moveIter)->end(); ++i)
		{
			moveObject(*(i->first), i->second.first, i->second.second);
		}

		delete *moveIter;
	}
}

// ----------------------------------------------------------------------

void ServerWorldNamespace::doPreloadCompleteTrigger(ServerObject &obj)
{
	if (obj.isAuthoritative() && obj.getScriptObject())
	{
		ScriptParams params;
		IGNORE_RETURN(obj.getScriptObject()->trigAllScripts(Scripting::TRIG_PRELOAD_COMPLETE, params));
	}
}

// ----------------------------------------------------------------------

void ServerWorldNamespace::updatePlanetServer()
{
	int const objectCount = World::getNumberOfObjects(WOL_Tangible);
	int updateCount = ConfigServerGame::getSpacePlanetServerUpdatesPerFrame();

	if (updateCount && objectCount && ServerWorld::isSpaceScene())
	{
		while (updateCount)
		{
			++s_planetServerUpdateIndex;
			if (s_planetServerUpdateIndex >= objectCount)
			{
				s_planetServerUpdateIndex = 0;
				break;
			}
			Object const * const o = World::getConstObject(WOL_Tangible, s_planetServerUpdateIndex);
			if (o)
			{
				ServerObject const * const so = o->asServerObject();
				if (so)
				{
					ShipObject const * const ship = so->asShipObject();
					if (ship)
					{
						UpdateObjectOnPlanetMessage const msg(
							ship->getNetworkId(),
							ship->getNetworkId(),
							static_cast<int>(ship->getPosition_p().x),
							static_cast<int>(ship->getPosition_p().y),
							static_cast<int>(ship->getPosition_p().z),
							ship->isPlayerShip() ? 1 : 0,
							0,
							false,
							false,
							static_cast<int>(ship->getObjectType()),
							0,
							false,
							ship->getTemplateCrc(),
							-1,
							-1);
						GameServer::getInstance().sendToPlanetServer(msg);
						--updateCount;
					}
				}
			}
		}
	}
}

// ======================================================================
