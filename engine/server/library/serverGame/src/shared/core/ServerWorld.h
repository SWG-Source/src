// ServerWorld.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_SERVER_WORLD_H
#define	_SERVER_WORLD_H

//-----------------------------------------------------------------------

#include "sharedObject/World.h"

//-----------------------------------------------------------------------

class CachedNetworkId;
class CellObject;
class CellProperty;
class Client;
class DraftSchematicObject;
class Location;
class ManufactureSchematicObject;
class Object;
class ObjectList;
class ObjectNotification;
class PlayerObject;
class ResourceClassObject;
class ServerObject;
class ServerObjectTemplate;
class ServerWorldIntangibleNotification;
class ServerWorldTangibleNotification;
class ServerWorldTerrainObjectNotification;
class ServerWorldUniverseNotification;
class SlotId;
class Sphere;
class SynchronizedWeatherGenerator;
class TangibleObject;
class Timer;
class Transform;
class TriggerVolume;
class UniverseObject;
class WeatherData;
class Vector;

//-----------------------------------------------------------------------

class ServerWorld : public World
{
public:
	friend class ServerWorldTangibleNotification;
	friend class ServerWorldIntangibleNotification;
	friend class ServerWorldUniverseNotification;
	friend class ServerWorldTerrainObjectNotification;

	typedef ServerWorldTangibleNotification      TangibleNotification;
	typedef ServerWorldIntangibleNotification    IntangibleNotification;
	typedef ServerWorldUniverseNotification      UniverseNotification;
	typedef ServerWorldTerrainObjectNotification TerrainObjectNotification;

public:

	static const ObjectNotification  &getTangibleNotification      ();
	static const ObjectNotification  &getTerrainObjectNotification ();
	static const ObjectNotification  &getIntangibleNotification    ();
	static const ObjectNotification  &getUniverseNotification      ();

	static ServerObject *       findObjectByNetworkId          (const NetworkId& id, bool searchQueuedList=false);
	static ServerObject *       findUninitializedObjectByNetworkId(const NetworkId& id);
	static void                 findObjectsInRange             (const Vector & location, const float distance, std::vector<ServerObject *> & results);
	static void                 findStaticCollidableObjectsInRange (const Vector & location, const float distance, std::vector<ServerObject *> & results);
	static void                 findCreaturesInRange           (const Vector & location, const float distance, std::vector<ServerObject *> & results);
	static void                 findAuthoritativeNonPlayerCreaturesInRange(const Vector & location, const float distance, std::vector<ServerObject *> & results);
	static void                 findCreaturesOfNicheInRange    (const Vector & location, const float distance, int niche, int mask, std::vector<ServerObject *> & results);
	static void                 findCreaturesOfSpeciesInRange  (const Vector & location, const float distance, int species, std::vector<ServerObject *> & results);
	static void                 findCreaturesOfRaceInRange     (const Vector & location, const float distance, int species, int race, std::vector<ServerObject *> & results);
	static void                 findNonCreaturesInRange        (const Vector & location, const float distance, std::vector<ServerObject *> & results);
	static void                 findNPCsInRange                (const Vector & location, const float distance, std::vector<ServerObject *> & results);
	static void                 findPlayerCreaturesInRange     (const Vector & location, const float distance, std::vector<ServerObject *> & results);

	static void                 findObjectsInCone              (const Object & coneCenterObject, const Object & coneDirectionObject, const float distance, float angle, std::vector<ServerObject *> & results);
	static void                 findObjectsInCone              (const Object & coneCenterObject, const Location & coneDirection, const float distance, float angle, std::vector<ServerObject *> & results);
	static void                 findCreaturesInCone            (const Object & coneCenterObject, const Object & coneDirectionObject, const float distance, float angle, std::vector<ServerObject *> & results);
	static void                 findCreaturesInCone            (const Object & coneCenterObject, const Location & coneDirection, const float distance, float angle, std::vector<ServerObject *> & results);
	static void                 findAuthoritativeNonPlayerCreaturesInCone(const Object & coneCenterObject, const Object & coneDirectionObject, const float distance, float angle, std::vector<ServerObject *> & results);
	static void                 findCreaturesOfNicheInCone     (const Object & coneCenterObject, const Object & coneDirectionObject, const float distance, float angle, int niche, int mask, std::vector<ServerObject *> & results);
	static void                 findCreaturesOfSpeciesInCone   (const Object & coneCenterObject, const Object & coneDirectionObject, const float distance, float angle, int species, std::vector<ServerObject *> & results);
	static void                 findCreaturesOfRaceInCone      (const Object & coneCenterObject, const Object & coneDirectionObject, const float distance, float angle, int species, int race, std::vector<ServerObject *> & results);
	static void                 findNonCreaturesInCone         (const Object & coneCenterObject, const Object & coneDirectionObject, const float distance, float angle, std::vector<ServerObject *> & results);
	static void                 findNPCsInCone                 (const Object & coneCenterObject, const Object & coneDirectionObject, const float distance, float angle, std::vector<ServerObject *> & results);
	static void                 findPlayerCreaturesInCone      (const Object & coneCenterObject, const Object & coneDirectionObject, const float distance, float angle, std::vector<ServerObject *> & results);

	static ServerObject *       findClosestNPC                 (const Vector & location, float distance);
	static ServerObject *       findClosestPlayer              (const Vector & location, float distance);
	
	static ServerObject *       findPobAtLocation              (const Vector & location_w);
	static CellProperty const * findCellAtLocation             (const Vector & location_w);

	static int                  getNumObjects                  (void);
	static ServerObject *       getObject                      (int index); //lint !e1511 //ok to hide base function
	static void                 install                        (void); //lint !e1511 //ok to hide base function

	static void                 moveObject                     (ServerObject & movingObject, const Vector & start, const Vector & end);
	static void                 remove                         (void); //lint !e1511 //ok to hide base function
	static void                 removeObject                   (ServerObject * object);
	static void                 removeObjectFromGame           (const ServerObject& object);
	static void                 update                         (real time); //lint !e1511 //ok to hide base function

	// object creation functions
	static ServerObject*        createNewObject                 (std::string const &templateName, Transform const &transform, ServerObject *cell, bool persisted, bool hyperspace = false);
	static ServerObject*        createNewObject                 (uint32 templateCrc, Transform const &transform, ServerObject *cell, bool persisted, bool hyperspace = false);
	static ServerObject*        createNewObject                 (ServerObjectTemplate const &objectTemplate, Transform const &transform, ServerObject *cell, bool persisted, bool hyperspace = false);
	static ServerObject*        createNewObject                 (std::string const &templateName, ServerObject &container, bool persisted, bool allowOverload = false);
	static ServerObject*        createNewObject                 (uint32 templateCrc, ServerObject &container, bool persisted, bool allowOverload = false);
	static ServerObject*        createNewObject                 (ServerObjectTemplate const &objectTemplate, ServerObject &container, bool persisted, bool allowOverloaded = false);
	static ServerObject*        createNewObject                 (std::string const &templateName, ServerObject &container, SlotId const &slotId, bool persisted);
	static ServerObject*        createNewObject                 (uint32 templateCrc, ServerObject &container, SlotId const &slotId, bool persisted);
	static ServerObject*        createNewObject                 (ServerObjectTemplate const &objectTemplate, ServerObject &container, SlotId const &slotId, bool persisted);
	static ManufactureSchematicObject * createNewManufacturingSchematic(const CachedNetworkId & creator, ServerObject & container, const SlotId & slotId, bool persisted);
	static ManufactureSchematicObject * createNewManufacturingSchematic(const DraftSchematicObject & source, const Vector & position, bool persisted);
	static ManufactureSchematicObject * createNewManufacturingSchematic(const DraftSchematicObject & source, ServerObject & container, bool persisted);
	static ServerObject*        createProxyObject               (std::string const &templateName, NetworkId const &newId, bool createAuthoritative);
	static ServerObject*        createProxyObject               (uint32 templateCrc, NetworkId const &newId, bool createAuthoritative);

	static void                 addObjectTriggerVolume          (TriggerVolume * triggerVolume);
	static void                 addObjectToConcludeList         (ServerObject * object);
	static void                 removeObjectFromConcludeList    (ServerObject * object);
	static void                 debugDump                       (void); //lint !e1511 //ok to hide base function
	static const std::string &  getSceneId                      (void);
	static bool                 isSpaceScene                    ();
	static bool                 isSpaceBattlefieldZone          ();
	static bool                 isSpaceBattlefieldZoneWithPvp   ();
	static void                 removeObjectTriggerVolume       (TriggerVolume * triggerVolume);

	static void                 dumpObjectSphereTree            (std::vector<std::pair<ServerObject *, Sphere> > & results);
	static void                 dumpTriggerSphereTree           (std::vector<std::pair<TriggerVolume *, Sphere> > & results);

	static const WeatherData &  getWeather                      ();

	static void                 addLoadBeacon                   (const TangibleObject * loadBeacon);
	static void                 removeLoadBeacon                (const TangibleObject * loadBeacon);
	static bool                 isInLoadBeaconRange             (const Vector & worldPosition);

	static Vector               getGoodLocation                 (float areaSizeX, float areaSizeY, const Vector & searchRectLowerLeftLocation, const Vector & searchRectUpperRightLocation, bool dontCheckWater, bool dontCheckSlope);
	static Vector               getGoodLocationAvoidCollidables (float areaSizeX, float areaSizeY, const Vector & searchRectLowerLeftLocation, const Vector & searchRectUpperRightLocation, bool dontCheckWater, bool dontCheckSlope, float minStaticObjDistance);
	
	static void                 requestPreloadCompleteTrigger   (ServerObject &obj);
	static void                 onPreloadComplete               ();
	static bool                 isPreloadComplete               ();

	static void                 getSpatialChatListeners(ServerObject &source, float distance, std::vector<ServerObject *> &results);

	static int                  getNumMoveLists(std::vector<int> & moveListSize);
	static void                 setNumMoveLists(int numMoveLists);

protected:
	friend class ServerObjectLint;
	static ServerObject *       createObjectFromTemplate(const std::string &templateName, const NetworkId & newId);
	static ServerObject *       createObjectFromTemplate(uint32 templateCrc, const NetworkId & newId);
	static ServerObject *       createObjectFromTemplate(const ServerObjectTemplate &objectTemplate, const NetworkId & newId);

private:
	ServerWorld();
	ServerWorld(const ServerWorld & other);
	ServerWorld& operator=(const ServerWorld & other);
	~ServerWorld();

	static ServerObject* createNewObjectIntermediate(ServerObject* newObject, const Transform & transform, ServerObject *cell, bool persisted);  ///< Called by CreateNewObject(*) functions.  This is provided to share code.
	static ServerObject* createNewObjectIntermediate(ServerObject* newObject, ServerObject & container, bool persisted, bool);  ///< Called by CreateNewObject(*) functions.  This is provided to share code.
	static ServerObject* createNewObjectIntermediate(ServerObject* newObject, ServerObject & container, const SlotId & slotId, bool persisted);

	static ServerObject *findClosestObjectInList(const Vector &location, const std::vector<ServerObject*> &candidates);

	static void    addTangibleObject(ServerObject * object);
	static void    removeTangibleObject(ServerObject * object);

	static void    addIntangibleObject(Object * object);
	static void    removeIntangibleObject(Object * object);

	static void    addUniverseObject(UniverseObject * object);
	static void    removeUniverseObject(UniverseObject * object);


	static void    internalMoveObject(ServerObject & movingObject, const Vector & start, const Vector & end);
	static void    updateMoveObjectList();
	static void    updateObjectDatabase            (ServerObject & movingObject);
	static void    updateTriggerDatabase           (ServerObject & movingObject);
	static void    triggerMovingObjects            (ServerObject & movingObject, const Vector & start, const Vector & end);
	static void    triggerMovingTriggers           (ServerObject & movingObject, const Vector & start, const Vector & end);

	static bool	                           m_installed;
    static Timer *                         m_idleTimer;
	static std::string *                   m_sceneId;

	static SynchronizedWeatherGenerator *  m_weatherGenerator;
};

//-----------------------------------------------------------------------

#endif	// _SERVER_WORLD_H

