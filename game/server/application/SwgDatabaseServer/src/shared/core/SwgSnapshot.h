// ======================================================================
//
// SwgSnapshot.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgSnapshot_H
#define INCLUDED_SwgSnapshot_H

// ======================================================================

#include "Archive/ByteStream.h"
#include "serverDatabase/Snapshot.h"
#include "serverDatabase/TableBuffer.h"
#include "sharedFoundation/Tag.h"
#include "SwgDatabaseServer/BattlefieldParticipantBuffer.h"
#include "SwgDatabaseServer/BountyHunterTargetBuffer.h"
#include "SwgDatabaseServer/CreatureObjectBuffer.h"
#include "SwgDatabaseServer/ExperienceBuffer.h"
#include "SwgDatabaseServer/IndexedNetworkTableBuffer.h"
#include "SwgDatabaseServer/LocationBuffer.h"
#include "SwgDatabaseServer/ManufactureSchematicAttributeBuffer.h"
#include "SwgDatabaseServer/MessageBuffer.h"
#include "SwgDatabaseServer/ObjectQueries.h"
#include "SwgDatabaseServer/ObjectTableBuffer.h"
#include "SwgDatabaseServer/ObjvarBuffer.h"
#include "SwgDatabaseServer/PropertyListBuffer.h"
#include "SwgDatabaseServer/ResourceTypeBuffer.h"
#include "SwgDatabaseServer/Schema.h"
#include "SwgDatabaseServer/ScriptBuffer.h"
#include "SwgDatabaseServer/WaypointBuffer.h"
namespace DB
{
	class Session;
}

class BatchBaselinesMessageData;
class BaselinesMessage;
class DeltasMessage;
class GameServerConnection;
class ImmediateDeleteCustomPersistStep;
class NetworkTableBuffer;
class OfflineMoneyCustomPersistStep;
class UpdateObjectPositionMessage;

typedef IndexedNetworkTableBuffer<DBSchema::BattlefieldMarkerObjectBufferRow, DBSchema::BattlefieldMarkerObjectRow, DBQuery::BattlefieldMarkerObjectQuery, DBQuery::BattlefieldMarkerObjectQuerySelect> BattlefieldMarkerObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::BuildingObjectBufferRow, DBSchema::BuildingObjectRow, DBQuery::BuildingObjectQuery, DBQuery::BuildingObjectQuerySelect> BuildingObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::CellObjectBufferRow, DBSchema::CellObjectRow, DBQuery::CellObjectQuery, DBQuery::CellObjectQuerySelect> CellObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::CityObjectBufferRow, DBSchema::CityObjectRow, DBQuery::CityObjectQuery, DBQuery::CityObjectQuerySelect> CityObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::FactoryObjectBufferRow, DBSchema::FactoryObjectRow, DBQuery::FactoryObjectQuery, DBQuery::FactoryObjectQuerySelect> FactoryObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::GuildObjectBufferRow, DBSchema::GuildObjectRow, DBQuery::GuildObjectQuery, DBQuery::GuildObjectQuerySelect> GuildObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::HarvesterInstallationObjectBufferRow, DBSchema::HarvesterInstallationObjectRow, DBQuery::HarvesterInstallationObjectQuery, DBQuery::HarvesterInstallationObjectQuerySelect> HarvesterInstallationObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::InstallationObjectBufferRow, DBSchema::InstallationObjectRow, DBQuery::InstallationObjectQuery, DBQuery::InstallationObjectQuerySelect> InstallationObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::IntangibleObjectBufferRow, DBSchema::IntangibleObjectRow, DBQuery::IntangibleObjectQuery, DBQuery::IntangibleObjectQuerySelect> IntangibleObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::ManufactureInstallationObjectBufferRow, DBSchema::ManufactureInstallationObjectRow, DBQuery::ManufactureInstallationObjectQuery, DBQuery::ManufactureInstallationObjectQuerySelect> ManufactureInstallationObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::ManufactureSchematicObjectBufferRow, DBSchema::ManufactureSchematicObjectRow, DBQuery::ManufactureSchematicObjectQuery, DBQuery::ManufactureSchematicObjectQuerySelect> ManufactureSchematicObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::MissionObjectBufferRow, DBSchema::MissionObjectRow, DBQuery::MissionObjectQuery, DBQuery::MissionObjectQuerySelect> MissionObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::PlanetObjectBufferRow, DBSchema::PlanetObjectRow, DBQuery::PlanetObjectQuery, DBQuery::PlanetObjectQuerySelect> PlanetObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::PlayerObjectBufferRow, DBSchema::PlayerObjectRow, DBQuery::PlayerObjectQuery, DBQuery::PlayerObjectQuerySelect> PlayerObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::ResourceContainerObjectBufferRow, DBSchema::ResourceContainerObjectRow, DBQuery::ResourceContainerObjectQuery, DBQuery::ResourceContainerObjectQuerySelect> ResourceContainerObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::ShipObjectBufferRow, DBSchema::ShipObjectRow, DBQuery::ShipObjectQuery, DBQuery::ShipObjectQuerySelect> ShipObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::StaticObjectBufferRow, DBSchema::StaticObjectRow, DBQuery::StaticObjectQuery, DBQuery::StaticObjectQuerySelect> StaticObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::TangibleObjectBufferRow, DBSchema::TangibleObjectRow, DBQuery::TangibleObjectQuery, DBQuery::TangibleObjectQuerySelect> TangibleObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::UniverseObjectBufferRow, DBSchema::UniverseObjectRow, DBQuery::UniverseObjectQuery, DBQuery::UniverseObjectQuerySelect> UniverseObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::VehicleObjectBufferRow, DBSchema::VehicleObjectRow, DBQuery::VehicleObjectQuery, DBQuery::VehicleObjectQuerySelect> VehicleObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::WeaponObjectBufferRow, DBSchema::WeaponObjectRow, DBQuery::WeaponObjectQuery, DBQuery::WeaponObjectQuerySelect> WeaponObjectBuffer;
typedef IndexedNetworkTableBuffer<DBSchema::PlayerQuestObjectBufferRow, DBSchema::PlayerQuestObjectRow, DBQuery::PlayerQuestObjectQuery, DBQuery::PlayerQuestObjectQuerySelect> PlayerQuestObjectBuffer;
// ======================================================================

/**
 * A Snapshot customized to Star Wars Galaxies.
 *
 * @todo:  there are a lot of public functions that could be made private
 */
class SwgSnapshot : public Snapshot
{
public:
	SwgSnapshot(DB::ModeQuery::Mode mode, bool useGoldDatabase);
	~SwgSnapshot();

	// Network Functions:
	virtual void handleUpdateObjectPosition(const UpdateObjectPositionMessage &message);
	virtual void handleDeleteMessage   (const NetworkId &objectID, int reasonCode, bool immediate, bool demandLoadedContainer, bool cascadeReason);
	virtual void handleMessageTo       (const MessageToPayload &data);
	virtual void handleMessageToAck    (const MessageToId &messageId);
	virtual void handleAddResourceTypeMessage (AddResourceTypeMessage const & message);
	virtual void handleBountyHunterTargetMessage (const BountyHunterTargetMessage &message);
	virtual bool send                  (GameServerConnection *connection) const;

	// Misc game-specific persistence
	void getMoneyFromOfflineObject     (uint32 replyServer, NetworkId const & sourceObject, int amount, NetworkId const & replyTo, std::string const & successCallback, std::string const & failCallback, std::vector<int8> const & packedDictionary);

	// Saving and Loading:
	virtual bool saveToDB(DB::Session *session);
	virtual bool load    (DB::Session *session);
	virtual void saveCompleted         ();
	virtual void getWorldContainers    (std::vector<NetworkId> &containers) const;
	
	virtual void handleCommoditiesDataMessage(const MessageDispatch::MessageBase & message);
	virtual void startLoadAfterSaveComplete();

	// Functions for object creation:
	virtual void newObject(NetworkId const & objectId, int templateId, Tag typeId);
	
	void newBattlefieldMarkerObject       (NetworkId const & objectId);
	void newBuildingObject                (NetworkId const & objectId);
	void newCellObject                    (NetworkId const & objectId);
	void newCityObject                    (NetworkId const & objectId);
	void newCountingObject                (NetworkId const & objectId);
	void newCreatureObject                (NetworkId const & objectId);
	void newFactoryObject                 (NetworkId const & objectId);
	void newGuildObject                   (NetworkId const & objectId);
	void newHarvesterInstallationObject   (NetworkId const & objectId);
	void newInstallationObject            (NetworkId const & objectId);
	void newIntangibleObject              (NetworkId const & objectId);
	void newManufactureInstallationObject (NetworkId const & objectId);
	void newManufactureSchematicObject    (NetworkId const & objectId);
	void newMissionObject                 (NetworkId const & objectId);
	void newPlanetObject                  (NetworkId const & objectId);
	void newPlayerObject                  (NetworkId const & objectId);
	void newPlayerQuestObject	      (NetworkId const & objectId);
	void newRegionCircleObject            (NetworkId const & objectId);
	void newRegionObject                  (NetworkId const & objectId);
	void newRegionRectangleObject         (NetworkId const & objectId);
	void newResourceContainerObject       (NetworkId const & objectId);
	void newShipObject                    (NetworkId const & objectId);
	void newStaticObject                  (NetworkId const & objectId);
	void newTangibleObject                (NetworkId const & objectId);
	void newUniverseObject                (NetworkId const & objectId);
	void newVehicleObject                 (NetworkId const & objectId);
	void newWeaponObject                  (NetworkId const & objectId);

	// Functions for object loading:
//	virtual void addObjectIdForLoad(NetworkId const & objectId);

	void loadAttributes                    (DB::Session *session, NetworkId const & objectId);
	void loadBattlefieldMarkerObject       (DB::Session *session, NetworkId const & objectId);
	void loadBuildingObject                (DB::Session *session, NetworkId const & objectId);
	void loadCellObject                    (DB::Session *session, NetworkId const & objectId);
	void loadCityObject                    (DB::Session *session, NetworkId const & objectId);
	void loadCommands                      (DB::Session *session, NetworkId const & objectId);
	void loadCountingObject                (DB::Session *session, NetworkId const & objectId);
	void loadCreatureObject                (DB::Session *session, NetworkId const & objectId);
	void loadFactoryObject                 (DB::Session *session, NetworkId const & objectId);
	void loadGuildObject                   (DB::Session *session, NetworkId const & objectId);
	void loadHarvesterInstallationObject   (DB::Session *session, NetworkId const & objectId);
	void loadInstallationObject            (DB::Session *session, NetworkId const & objectId);
	void loadIntangibleObject              (DB::Session *session, NetworkId const & objectId);
	void loadManufactureInstallationObject (DB::Session *session, NetworkId const & objectId);
	void loadManufactureSchematicObject    (DB::Session *session, NetworkId const & objectId);
	void loadMissionObject                 (DB::Session *session, NetworkId const & objectId);
	void loadPlanetObject                  (DB::Session *session, NetworkId const & objectId);
	void loadPlayerObject                  (DB::Session *session, NetworkId const & objectId);
	void loadPlayerQuestObject			   (DB::Session *session, NetworkId const & objectId);
	void loadRegionCircleObject            (DB::Session *session, NetworkId const & objectId);
	void loadRegionObject                  (DB::Session *session, NetworkId const & objectId);
	void loadRegionRectangleObject         (DB::Session *session, NetworkId const & objectId);
	void loadResourceContainerObject       (DB::Session *session, NetworkId const & objectId);
	void loadSkills                        (DB::Session *session, NetworkId const & objectId);
	void loadShipObject                    (DB::Session *session, NetworkId const & objectId);
	void loadStaticObject                  (DB::Session *session, NetworkId const & objectId);
	void loadTangibleObject                (DB::Session *session, NetworkId const & objectId);
	void loadUniverseObject                (DB::Session *session, NetworkId const & objectId);
	void loadVehicleObject                 (DB::Session *session, NetworkId const & objectId);
	void loadWeaponObject                  (DB::Session *session, NetworkId const & objectId);

	
protected:
	BattlefieldMarkerObjectBuffer m_battlefieldMarkerObjectBuffer;
	BattlefieldParticipantBuffer m_battlefieldParticipantBuffer;
	BountyHunterTargetBuffer m_bountyHunterTargetBuffer;
	BuildingObjectBuffer m_buildingObjectBuffer;
	CellObjectBuffer m_cellObjectBuffer;
	CityObjectBuffer m_cityObjectBuffer;
	CreatureObjectBuffer m_creatureObjectBuffer;
	ExperienceBuffer m_experienceBuffer;
	FactoryObjectBuffer m_factoryObjectBuffer;
	GuildObjectBuffer m_guildObjectBuffer;
	HarvesterInstallationObjectBuffer m_harvesterInstallationObjectBuffer;
	InstallationObjectBuffer m_installationObjectBuffer;
	IntangibleObjectBuffer m_intangibleObjectBuffer;
	LocationBuffer m_locationBuffer;
	ManufactureInstallationObjectBuffer m_manufactureInstallationObjectBuffer;
	ManufactureSchematicAttributeBuffer m_manufactureSchematicAttributeBuffer;
	ManufactureSchematicObjectBuffer m_manufactureSchematicObjectBuffer;
	MessageBuffer m_messageBuffer;
	MissionObjectBuffer m_missionObjectBuffer;
	ObjectTableBuffer m_objectTableBuffer;
	ObjvarBuffer m_objvarBuffer;
	PlanetObjectBuffer m_planetObjectBuffer;
	PlayerObjectBuffer m_playerObjectBuffer;
	PlayerQuestObjectBuffer m_playerQuestObjectBuffer;
	PropertyListBuffer m_propertyListBuffer;
	ResourceContainerObjectBuffer m_resourceContainerObjectBuffer;
	ResourceTypeBuffer m_resourceTypeBuffer;
	ScriptBuffer m_scriptBuffer;
	ShipObjectBuffer m_shipObjectBuffer;
	StaticObjectBuffer m_staticObjectBuffer;
	TangibleObjectBuffer m_tangibleObjectBuffer;
	UniverseObjectBuffer m_universeObjectBuffer;
	VehicleObjectBuffer m_vehicleObjectBuffer;
	WaypointBuffer m_waypointBuffer;
	WeaponObjectBuffer m_weaponObjectBuffer;

	typedef std::vector<NetworkId> OIDListType;
//	OIDListType m_oidList;

	ImmediateDeleteCustomPersistStep *m_immediateDeleteStep;
	OfflineMoneyCustomPersistStep *m_offlineMoneyCustomPersistStep;
	
  private:
	void registerTags();
	
	void loadObject(DB::Session *session,const Tag &typeId, const NetworkId &objectId);
	
	virtual void decodeServerData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline);
	virtual void decodeSharedData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline);
	virtual void decodeClientData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline);
	virtual void decodeParentClientData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline);
	
	virtual bool encodeParentClientData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const;
	virtual bool encodeClientData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const;
	virtual bool encodeServerData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const;
	virtual bool encodeSharedData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const;

	void decodeClientBattlefieldMarkerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientBuildingObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientCellObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientCityObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientCreatureObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientGuildObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientHarvesterInstallationObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientInstallationObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientIntangibleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientManufactureInstallationObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientMissionObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientPlanetObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientPlayerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientPlayerQuestObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientRegionCircleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientRegionObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientRegionRectangleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientResourceContainerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientServerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientShipObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientStaticObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientTangibleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientUniverseObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientVehicleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientWeaponObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);

	void decodeServerBattlefieldMarkerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerBuildingObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerCellObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerCityObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerCreatureObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerGuildObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerHarvesterInstallationObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerInstallationObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerIntangibleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerManufactureInstallationObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerMissionObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerPlanetObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerPlayerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerPlayerQuestObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerRegionCircleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerRegionObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerRegionRectangleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerResourceContainerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerServerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerShipObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerStaticObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerTangibleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerUniverseObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerVehicleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerWeaponObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);

	void decodeSharedBattlefieldMarkerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedBuildingObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedCellObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);	
	void decodeSharedCityObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);	
	void decodeSharedCreatureObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedGuildObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedHarvesterInstallationObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedInstallationObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedIntangibleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedManufactureInstallationObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedMissionObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedPlanetObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedPlayerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedPlayerQuestObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedRegionCircleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedRegionObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedRegionRectangleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedResourceContainerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedServerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedShipObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedStaticObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedTangibleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedUniverseObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedVehicleObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedWeaponObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);

	void decodeServerManufactureSchematicObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientManufactureSchematicObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeSharedManufactureSchematicObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	bool encodeServerManufactureSchematicObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientManufactureSchematicObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedManufactureSchematicObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	void decodeSharedFactoryObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerFactoryObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientFactoryObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	bool encodeServerFactoryObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedFactoryObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientFactoryObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	
	void decodeSharedCountingObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeServerCountingObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	void decodeClientCountingObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	
	bool encodeSharedCountingObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerCountingObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientCountingObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	
	bool encodeClientBattlefieldMarkerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientBuildingObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientCellObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientCityObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientCreatureObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientGuildObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientHarvesterInstallationObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientInstallationObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientIntangibleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientManufactureInstallationObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientMissionObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientPlanetObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientPlayerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientPlayerQuestObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientRegionCircleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientRegionObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientRegionRectangleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientResourceContainerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientServerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientShipObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientStaticObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientTangibleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientUniverseObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientVehicleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeClientWeaponObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;

	bool encodeServerBattlefieldMarkerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerBuildingObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerCellObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerCityObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerCreatureObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerGuildObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerHarvesterInstallationObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerInstallationObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerIntangibleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerManufactureInstallationObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerMissionObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerPlanetObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerPlayerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerPlayerQuestObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerRegionCircleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerRegionObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerRegionRectangleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerResourceContainerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerServerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerShipObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerStaticObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerTangibleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerUniverseObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerVehicleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeServerWeaponObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;

	bool encodeSharedBattlefieldMarkerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedBuildingObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedCellObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedCityObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedCreatureObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedGuildObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedHarvesterInstallationObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedInstallationObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedIntangibleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedManufactureInstallationObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedMissionObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedPlanetObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedPlayerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedPlayerQuestObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedRegionCircleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedRegionObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedRegionRectangleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedResourceContainerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedServerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedShipObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedStaticObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedTangibleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedUniverseObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedVehicleObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;
	bool encodeSharedWeaponObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;

	void decodeParentClientPlayerObject(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline);
	bool encodeParentClientPlayerObject(NetworkId const & objectId, Archive::ByteStream &data, bool addCount=true) const;

  private:
	void decodeAttributes       (const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline, int offset);
	void decodeBattlefieldParticipants(const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline);
	void decodePropertyList     (const NetworkId &objectId, int listId, Archive::ReadIterator &data, bool isBaseline);
	void decodeExperience       (const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline);
	void decodeLocationDataList (const NetworkId &objectId, size_t listId, Archive::ReadIterator &data, bool isBaseline);
	void decodeManufactureSchematicAttributes(const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline);
	void decodeMissionLocation  (Archive::ReadIterator &data, DB::BindableDouble &x,DB::BindableDouble &y, DB::BindableDouble &z, DB::BindableString<500> &planet, DB::BindableString<500> &region_name,DB::BindableNetworkId &cell) const;
	void decodeObjVars          (NetworkId const & objectId, Archive::ReadIterator &data, bool isBaseline);
	void decodeObjVarFreeFlags  (const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline);
	void decodeScriptObject     (NetworkId const & objectId, Archive::ReadIterator &data, bool isBaseline);
	void decodeStringId         (Archive::ReadIterator &data,DB::BufferString &table,DB::BufferString &text) const;
	void decodeVector           (Archive::ReadIterator &data, DB::BindableDouble &x,DB::BindableDouble &y, DB::BindableDouble &z) const;
	void decodeMatchMakingId    (Archive::ReadIterator &data, DB::BufferString &column) const;
	void decodeComponents       (NetworkId const & objectId, Archive::ReadIterator &data, bool isBaseline);
	void decodeLocation         (Archive::ReadIterator &data, DB::BindableDouble &x,DB::BindableDouble &y, DB::BindableDouble &z, DB::BindableNetworkId &cell, DB::BindableLong &planet) const;
	void decodeWaypoints        (const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline);
	void decodeSingleWaypoint   (const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline);
	void decodeQuests           (NetworkId const & networkId, Archive::ReadIterator &data, DB::BufferString &part1, DB::BufferString &part2, DB::BufferString &part3, DB::BufferString &part4) const;
	void decodePersistedBuffs   (Archive::ReadIterator &data, DB::BufferString &dbString) const;

	void encodeAttributes       (const NetworkId & objectId, Archive::ByteStream &data, int offset) const;
	void encodeBattlefieldParticipants(const NetworkId &objectId, Archive::ByteStream &data) const;
	void encodePropertyList     (const NetworkId & objectId, int listId, Archive::ByteStream &data) const;
	void encodeExperience       (const NetworkId & objectId, Archive::ByteStream &data) const;
	void encodeLocationDataList (const NetworkId & objectId, size_t listId, Archive::ByteStream &data) const;
	void encodeManufactureSchematicAttributes(const NetworkId & objectId, Archive::ByteStream &data) const;
	void encodeMissionLocation  (Archive::ByteStream &data, const DB::BindableDouble &x,const DB::BindableDouble &y, const DB::BindableDouble &z, const DB::BindableString<500> &planet, const DB::BindableString<500> &region_name, const DB::BindableNetworkId &cell) const;
	void encodeObjVars          (const NetworkId & objectId, Archive::ByteStream &data) const;
	void encodeObjVarFreeFlags  (const NetworkId & objectId, Archive::ByteStream &data) const;
	void encodeScriptObject     (const NetworkId & objectId, Archive::ByteStream &data) const;
	void encodeStringId         (Archive::ByteStream &data, const DB::BufferString &table, const DB::BufferString &text) const;
	void encodeVector           (Archive::ByteStream &data, const DB::BindableDouble &x, const DB::BindableDouble &y, const DB::BindableDouble &z) const;
	void encodeMatchMakingId    (Archive::ByteStream &data, const DB::BufferString &column) const;
	void encodeComponents       (const NetworkId &objectId, Archive::ByteStream &data) const;
	void encodeLocation         (Archive::ByteStream &data, const DB::BindableDouble &x,const DB::BindableDouble &y, const DB::BindableDouble &z, const DB::BindableNetworkId &cell, const DB::BindableLong &planet) const;
	void encodeWaypoints        (const NetworkId & objectId, Archive::ByteStream &data) const;
	void encodeSingleWaypoint   (const NetworkId & objectId, Archive::ByteStream &data) const;
	void encodeQuests           (Archive::ByteStream &data, DB::BufferString const &part1, DB::BufferString const &part2, DB::BufferString const &part3, DB::BufferString const &part4) const;
	void encodePersistedBuffs   (Archive::ByteStream &data, DB::BufferString const& dbString) const;

	void ignorePersistedFlag    (const NetworkId &objectId, Archive::ReadIterator &data) const;

	OfflineMoneyCustomPersistStep * getOfflineMoneyCustomPersistStep();
	
  private:
	SwgSnapshot(const SwgSnapshot&); //disable
	SwgSnapshot &operator=(const SwgSnapshot&); //disable
};

// ======================================================================
#endif
