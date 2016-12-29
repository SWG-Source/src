// ======================================================================
//
// SwgSnapshot.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/SwgSnapshot.h"

#include "Archive/AutoDeltaPackedMap.h"
#include "Archive/AutoDeltaSet.h"
#include "Archive/AutoDeltaVector.h"
#include "SwgDatabaseServer/ObjectTableBuffer.h"
#include "SwgDatabaseServer/OfflineMoneyCustomPersistStep.h"
#include "SwgDatabaseServer/PersistableWaypoint.h"
#include "localizationArchive/StringIdArchive.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/ImmediateDeleteCustomPersistStep.h"
#include "serverDatabase/MessageToManager.h"
#include "serverDatabase/ObjectLocator.h"
#include "serverNetworkMessages/CreateObjectMessage.h"
#include "serverNetworkMessages/EndBaselinesMessage.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "serverNetworkMessages/SetAuthoritativeMessage.h"
#include "serverNetworkMessages/UpdateObjectPositionMessage.h"
#include "serverUtility/MissionLocation.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/Buff.h"
#include "sharedGame/MatchMakingId.h"
#include "sharedGame/PlayerQuestData.h"
#include "sharedLog/Log.h"
#include "sharedMath/Quaternion.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedUtility/Location.h"
#include "swgSharedUtility/Attributes.def"

// ======================================================================

/**
 * Construct a snapshot.
 *
 * This and the header file should be the only places that the entire
 * list of buffers is hard-coded.  This will reduce the number
 * of places the code needs to be changed if new buffers are added.
 * (Referring to a _specific_ buffer when you know you need that buffer
 * is kosher.)
 */
SwgSnapshot::SwgSnapshot(DB::ModeQuery::Mode mode, bool useGoldDatabase)
        : Snapshot(mode, useGoldDatabase), m_battlefieldMarkerObjectBuffer(mode), m_battlefieldParticipantBuffer(mode),
          m_bountyHunterTargetBuffer(), m_buildingObjectBuffer(mode), m_cellObjectBuffer(mode),
          m_cityObjectBuffer(mode), m_creatureObjectBuffer(mode), m_experienceBuffer(mode), m_factoryObjectBuffer(mode),
          m_guildObjectBuffer(mode), m_harvesterInstallationObjectBuffer(mode), m_installationObjectBuffer(mode),
          m_intangibleObjectBuffer(mode), m_locationBuffer(mode), m_manufactureInstallationObjectBuffer(mode),
          m_manufactureSchematicAttributeBuffer(mode), m_manufactureSchematicObjectBuffer(mode), m_messageBuffer(),
          m_missionObjectBuffer(mode), m_objectTableBuffer(mode),
          m_objvarBuffer(mode, &m_objectTableBuffer, useGoldDatabase), m_planetObjectBuffer(mode),
          m_playerObjectBuffer(mode), m_playerQuestObjectBuffer(mode), m_propertyListBuffer(mode),
          m_resourceContainerObjectBuffer(mode), m_resourceTypeBuffer(), m_scriptBuffer(), m_shipObjectBuffer(mode),
          m_staticObjectBuffer(mode), m_tangibleObjectBuffer(mode), m_universeObjectBuffer(mode),
          m_vehicleObjectBuffer(mode), m_waypointBuffer(mode), m_weaponObjectBuffer(mode),
          m_immediateDeleteStep(nullptr), m_offlineMoneyCustomPersistStep(nullptr) {}

// ----------------------------------------------------------------------

SwgSnapshot::~SwgSnapshot() {}

// ----------------------------------------------------------------------

/**
 * Called by a worker thread.  Saves the snapshot to the database
 * @todo:  would it be better to have SwgSnapshot add a CustomStep for
 * the objectTableBuffer and not override this function?
 */

bool SwgSnapshot::saveToDB(DB::Session *session) {
    // artificial delay can be added here to help debug timing issues
    if (ConfigServerDatabase::getWriteDelay() > 0) {
        LOG("SwgSnapshot::saveToDB", ("Delaying writing of snapshot for %i seconds", ConfigServerDatabase::getWriteDelay()));
        Os::sleep(ConfigServerDatabase::getWriteDelay() * 1000);
    }

    session->setAutoCommitMode(false);

    for (auto step = m_customStepList.begin(); step !=m_customStepList.end(); ++step)
    {
            if (!(*step)->beforePersist(session)) {
                    return false;
            }
    }

    // save all the buffers
    if (!(m_objectTableBuffer.save(session))) { return false; }
    if (!(m_battlefieldMarkerObjectBuffer.save(session))) { return false; }
    if (!(m_battlefieldParticipantBuffer.save(session))) { return false; }
    if (!(m_buildingObjectBuffer.save(session))) { return false; }
    if (!(m_bountyHunterTargetBuffer.save(session))) { return false; }
    if (!(m_cellObjectBuffer.save(session))) { return false; }
    if (!(m_cityObjectBuffer.save(session))) { return false; }
    if (!(m_creatureObjectBuffer.save(session))) { return false; }
    if (!(m_experienceBuffer.save(session))) { return false; }
    if (!(m_factoryObjectBuffer.save(session))) { return false; }
    if (!(m_guildObjectBuffer.save(session))) { return false; }
    if (!(m_harvesterInstallationObjectBuffer.save(session))) { return false; }
    if (!(m_installationObjectBuffer.save(session))) { return false; }
    if (!(m_intangibleObjectBuffer.save(session))) { return false; }
    if (!(m_locationBuffer.save(session))) { return false; }
    if (!(m_manufactureInstallationObjectBuffer.save(session))) { return false; }
    if (!(m_manufactureSchematicAttributeBuffer.save(session))) { return false; }
    if (!(m_manufactureSchematicObjectBuffer.save(session))) { return false; }
    if (!(m_messageBuffer.save(session))) { return false; }
    if (!(m_missionObjectBuffer.save(session))) { return false; }
    if (!(m_objvarBuffer.save(session))) { return false; }
    if (!(m_planetObjectBuffer.save(session))) { return false; }
    if (!(m_playerObjectBuffer.save(session))) { return false; }
    if (!(m_playerQuestObjectBuffer.save(session))) { return false; }
    if (!(m_propertyListBuffer.save(session))) { return false; }
    if (!(m_resourceContainerObjectBuffer.save(session))) { return false; }
    if (!(m_resourceTypeBuffer.save(session))) { return false; }
    if (!(m_scriptBuffer.save(session))) { return false; }
    if (!(m_shipObjectBuffer.save(session))) { return false; }
    if (!(m_staticObjectBuffer.save(session))) { return false; }
    if (!(m_tangibleObjectBuffer.save(session))) { return false; }
    if (!(m_universeObjectBuffer.save(session))) { return false; }
    if (!(m_vehicleObjectBuffer.save(session))) { return false; }
    if (!(m_waypointBuffer.save(session))) { return false; }
    if (!(m_weaponObjectBuffer.save(session))) { return false; }

    for (auto step = m_customStepList.begin(); step !=m_customStepList.end(); ++step)
    {
            if (!(*step)->afterPersist(session)){
                    return false;
            }
    }

    // save the parent class
    if (!(Snapshot::saveToDB(session))) { return false; }

    // commit/flush to the db
    session->commitTransaction();

    return true;
}

// ----------------------------------------------------------------------

/**
 * Called when a message is received to update an object's transform and container.
 *
 * Identifies what the message changes, and puts it in the appropriate
 * table buffer.
 */

void SwgSnapshot::handleUpdateObjectPosition(const UpdateObjectPositionMessage &message) {
    if (message.isCreatureObject()) //&& ( message.getPlayerControlled() ))
    {
        m_creatureObjectBuffer.handleUpdateObjectPosition(message);
    }

    if (m_objectTableBuffer.handleUpdateObjectPosition(message)) return;

    FATAL(true, ("UpdateObjectPosition was not handled by any of the object buffers."));
}

// ----------------------------------------------------------------------

/**
 * Called when a message is received to delete an object.
 *
 * Tells the objectTableBuffer to set the deleted column to true.
 */

void
SwgSnapshot::handleDeleteMessage(const NetworkId &objectId, int reasonCode, bool immediate, bool demandLoadedContainer, bool cascadeReason) {
    m_objectTableBuffer.handleDeleteMessage(objectId, reasonCode, demandLoadedContainer, cascadeReason);

    m_battlefieldMarkerObjectBuffer.removeObject(objectId);
    m_battlefieldParticipantBuffer.removeObject(objectId);
    m_buildingObjectBuffer.removeObject(objectId);
    m_bountyHunterTargetBuffer.removeObject(objectId);
    m_cellObjectBuffer.removeObject(objectId);
    m_cityObjectBuffer.removeObject(objectId);
    m_creatureObjectBuffer.removeObject(objectId);
    m_experienceBuffer.removeObject(objectId);
    m_factoryObjectBuffer.removeObject(objectId);
    m_guildObjectBuffer.removeObject(objectId);
    m_harvesterInstallationObjectBuffer.removeObject(objectId);
    m_installationObjectBuffer.removeObject(objectId);
    m_intangibleObjectBuffer.removeObject(objectId);
    m_locationBuffer.removeObject(objectId);
    m_manufactureInstallationObjectBuffer.removeObject(objectId);
    m_manufactureSchematicAttributeBuffer.removeObject(objectId);
    m_manufactureSchematicObjectBuffer.removeObject(objectId);
    m_messageBuffer.removeObject(objectId);
    m_missionObjectBuffer.removeObject(objectId);
    m_objvarBuffer.removeObject(objectId);
    m_planetObjectBuffer.removeObject(objectId);
    m_playerObjectBuffer.removeObject(objectId);
    m_playerQuestObjectBuffer.removeObject(objectId);
    m_propertyListBuffer.removeObject(objectId);
    m_resourceContainerObjectBuffer.removeObject(objectId);
    m_resourceTypeBuffer.removeObject(objectId);
    m_scriptBuffer.removeObject(objectId);
    m_shipObjectBuffer.removeObject(objectId);
    m_staticObjectBuffer.removeObject(objectId);
    m_tangibleObjectBuffer.removeObject(objectId);
    m_universeObjectBuffer.removeObject(objectId);
    m_vehicleObjectBuffer.removeObject(objectId);
    m_waypointBuffer.removeObject(objectId);
    m_weaponObjectBuffer.removeObject(objectId);

    if (immediate && (getMode() != DB::ModeQuery::mode_INSERT)) {
        if (!m_immediateDeleteStep) {
            m_immediateDeleteStep = new ImmediateDeleteCustomPersistStep;
            addCustomPersistStep(m_immediateDeleteStep);
        }

        m_immediateDeleteStep->addObject(objectId);
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::handleMessageTo(const MessageToPayload &data) {
    m_messageBuffer.handleMessageTo(data);
}

// ----------------------------------------------------------------------

void SwgSnapshot::handleMessageToAck(const MessageToId &messageId) {
    m_messageBuffer.handleMessageToAck(messageId);
}

// ----------------------------------------------------------------------

/**
 * Decodes the Objvars data for an object from the network package.
 */
void SwgSnapshot::decodeObjVars(NetworkId const &objectId, Archive::ReadIterator &data, bool isBaseline) {
    std::vector <DynamicVariableList::MapType::Command> commands;
    if (isBaseline) {
        DynamicVariableList::MapType::unpack(data, commands);
    } else {
        DynamicVariableList::MapType::unpackDelta(data, commands);
    }
    m_objvarBuffer.updateObjvars(objectId, commands);
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeObjVars(const NetworkId &objectId, Archive::ByteStream &data) const {
    PROFILER_AUTO_BLOCK_DEFINE("encodeObjVars");
    static std::vector <DynamicVariableList::MapType::Command> commands;
    commands.clear();

    m_objvarBuffer.getObjvarsForObject(objectId, commands);
    DynamicVariableList::MapType::pack(data, commands);
}

// ----------------------------------------------------------------------

void SwgSnapshot::decodeObjVarFreeFlags(const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline) {
    UNREF(isBaseline);
    int freePositions;
    Archive::get(data, freePositions);
    m_objectTableBuffer.decodeObjVarFreeFlags(objectId, freePositions);
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeObjVarFreeFlags(const NetworkId &objectId, Archive::ByteStream &data) const {
    PROFILER_AUTO_BLOCK_DEFINE("encodeObjVarFreeFlags");
    int freePositions = m_objectTableBuffer.encodeObjVarFreeFlags(objectId);
    Archive::put(data, freePositions);
}

// ----------------------------------------------------------------------

void SwgSnapshot::decodeScriptObject(NetworkId const &objectId, Archive::ReadIterator &data, bool /* isBaseline */) {
    std::string packedScriptList;
    Archive::get(data, packedScriptList);

    if (packedScriptList.length() == 0) {
        packedScriptList = ' ';
    } // avoid confusing an empty list with nullptr

    DBSchema::ObjectBufferRow *row = m_objectTableBuffer.findRowByIndex(objectId);
    if (row == nullptr) {
        row = m_objectTableBuffer.addEmptyRow(objectId);
    }

    row->script_list = packedScriptList;
    m_scriptBuffer.clearScripts(objectId);
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeScriptObject(const NetworkId &objectId, Archive::ByteStream &data) const {
    PROFILER_AUTO_BLOCK_DEFINE("encodeScriptObject");
    const DBSchema::ObjectBufferRow *row = m_objectTableBuffer.findConstRowByIndex(objectId);
    if (row) {
        std::string temp;
        row->script_list.getValue(temp);
        if (temp.empty()) {
            // Object might have a non-converted script list
            std::vector <std::string> scripts;
            m_scriptBuffer.getScriptsForObject(objectId, scripts);

            for (std::vector<std::string>::const_iterator i = scripts.begin(); i != scripts.end(); ++i) {
                temp += (*i) + ':';
            }
        }
        Archive::put(data, temp);
    }
}

// ----------------------------------------------------------------------

void
SwgSnapshot::decodeAttributes(const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline, int offset) {
    if (isBaseline) {
        size_t size, baselineCommandCount;
        Archive::get(data, size);
        Archive::get(data, baselineCommandCount);

        Attributes::Value temp;
        for (size_t i = 0; i < size; ++i) {
            Archive::get(data, temp);
            m_creatureObjectBuffer.setAttribute(objectId, i + offset, temp);
        }
    } else {
        typedef Archive::AutoDeltaVector <Attributes::Value> VectorType;
        size_t numCommands, baselineCommandCount;
        Archive::get(data, numCommands);
        Archive::get(data, baselineCommandCount);
        for (size_t count = 0; count < numCommands; ++count) {
            unsigned char cmd;// datatype?
            uint16 index;

            Archive::get(data, cmd);

            switch (cmd) {
                case VectorType::Command::ERASE:
                    FATAL(true, ("AutoDeltaVector::Command::ERASE is not supported for attributes.\n"));

                case VectorType::Command::INSERT:
                    FATAL(true, ("AutoDeltaVector::Command::INSERT is not supported for attributes.\n"));

                case VectorType::Command::SET: {
                    Archive::get(data, index);
                    Attributes::Value value;
                    Archive::get(data, value);
//					DEBUG_REPORT_LOG(true,("Got Attribute SET with object id %s, index %i, value %i\n",objectId.getValueString().c_str(),index,value));
                    m_creatureObjectBuffer.setAttribute(objectId, index + offset, value);
                    break;
                }

                case VectorType::Command::SETALL:
                    FATAL(true, ("AutoDeltaVector::Command::SETALL is not supported for attributes.\n"));

                case VectorType::Command::CLEAR:
                    FATAL(true, ("AutoDeltaVector::Command::CLEAR is not supported for attributes.\n"));

                default:
                    FATAL(true, ("Unknown AutoDeltaVector command.\n"));
            }
        }
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeAttributes(const NetworkId &objectId, Archive::ByteStream &data, int offset) const {
    std::vector <Attributes::Value> values;
    m_creatureObjectBuffer.getAttributesForObject(objectId, values, offset, Attributes::NumberOfAttributes);

    Archive::put(data, Attributes::NumberOfAttributes);
    Archive::put(data, static_cast<size_t>(0)); // baselineCommandCount
    if (values.size() < static_cast<size_t>(Attributes::NumberOfAttributes)) {
        DEBUG_REPORT_LOG(true, ("Object %s did not have valid attribute data in the database.  Missing attributes will be set to 100.\n", objectId.getValueString().c_str()));
        values.resize(Attributes::NumberOfAttributes, 100);
    }
    for (int i = 0; i < Attributes::NumberOfAttributes; ++i) {
        Archive::put(data, values[i]);
    }
}

// ----------------------------------------------------------------------

void
SwgSnapshot::decodePropertyList(const NetworkId &objectId, int listId, Archive::ReadIterator &data, bool isBaseline) {
    typedef Archive::AutoDeltaSet <std::string> SetType;
    typedef std::vector <SetType::Command> CommandList;
    CommandList commands;
    if (isBaseline) {
        SetType::unpack(data, commands);
    } else {
        SetType::unpackDelta(data, commands);
    }
    for (CommandList::const_iterator i = commands.begin(); i != commands.end(); ++i) {
        switch (i->cmd) {
            case SetType::Command::ERASE:
                m_propertyListBuffer.deleteProperty(objectId, listId, i->value);
                break;
            case SetType::Command::INSERT:
                m_propertyListBuffer.insertProperty(objectId, listId, i->value);
                break;
            case SetType::Command::CLEAR:
                m_propertyListBuffer.clearProperties(objectId, listId);
                break;
            default:
                FATAL(true, ("Unknown AutoDeltaSet command.\n"));
        }
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodePropertyList(const NetworkId &objectId, int listId, Archive::ByteStream &data) const {
    typedef Archive::AutoDeltaSet <std::string> SetType;
    typedef std::set <std::string> ValuesType;
    ValuesType values;

    m_propertyListBuffer.getPropertyListForObject(objectId, listId, values);
    SetType::pack(data, values);
}

// ======================================================================
// Loading functions

/**
 * Load the objects.
 *
 * This function will load all the objects listed in m_oidList.
 */
bool SwgSnapshot::load(DB::Session *session) {
    DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(), ("Load begin.\n"));

    std::string schema;
    if (getUseGoldDatabase()) {
        schema = DatabaseProcess::getInstance().getGoldSchemaQualifier();
    } else {
        schema = DatabaseProcess::getInstance().getSchemaQualifier();
    }

    session->setAutoCommitMode(false);

    int objectCount = 0;
    int objectsLocated;

    for (LocatorListType::iterator loc = m_locatorList.begin(); loc != m_locatorList.end(); ++loc) {
        if (!((*loc)->locateObjects(session, schema, objectsLocated))) {
            return false;
        }
        objectCount += objectsLocated;
    }
    LOG("LoadTimes", ("Snapshot has %i objects", objectCount));
    DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(), ("Objects Located.\n"));

    if (objectCount > 0) {
        registerTags();

        DB::TagSet tags;

        if (!(m_objectTableBuffer.load(session, tags, schema, getUseGoldDatabase()))) {
            return false;
        }
        m_objectTableBuffer.getTags(tags);

        // the buffer list was fucked so we'll do this the old fashioned way
        if (!((m_battlefieldMarkerObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_battlefieldParticipantBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_buildingObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_bountyHunterTargetBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_cellObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_cityObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_creatureObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_experienceBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_factoryObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_guildObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_harvesterInstallationObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_installationObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_intangibleObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_locationBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_manufactureInstallationObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_manufactureSchematicAttributeBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_manufactureSchematicObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_messageBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_missionObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_objvarBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_planetObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_playerObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_playerQuestObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_propertyListBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_resourceContainerObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_resourceTypeBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_scriptBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_shipObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_staticObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_tangibleObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_universeObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_vehicleObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_waypointBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }
        if (!((m_weaponObjectBuffer).load(session, tags, schema, getUseGoldDatabase()))) { return false; }

        DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(), ("Load end.\n"));
    } else {
        DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(), ("Load skipped because there were no objects.\n"));
    }

    session->commitTransaction(); // will cause temp tables to be erased
    return true;
}

// ----------------------------------------------------------------------

/**
 * Send the snapshot to the appropriate server.
 *
 * This function will send messages to the appropriate gameServer to
 * create a proxy for the objects in the snapshot, then it will send a
 * message to central to relinquish authority for the objects.
 *
 * @return true if the objects were sent, false if they were not.
 */
bool SwgSnapshot::send(GameServerConnection *connection) const {
    PROFILER_AUTO_BLOCK_DEFINE("SwgSnapshot::send");

    if (connection == 0) {
        return false;
    }

    DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(), ("Sending Snapshot\n"));

    PROFILER_BLOCK_DEFINE(prebaselinesBlock, "sendPreBaselinesCustomData");
    PROFILER_BLOCK_ENTER(prebaselinesBlock);
    for (LocatorListType::const_iterator loc = m_locatorList.begin(); loc != m_locatorList.end(); ++loc) {
        (*loc)->sendPreBaselinesCustomData(*connection);
    }
    PROFILER_BLOCK_LEAVE(prebaselinesBlock);

    OIDListType oidList;
    m_objectTableBuffer.getObjectList(oidList);

    PROFILER_BLOCK_DEFINE(sendObjectData, "send object data");
    PROFILER_BLOCK_ENTER(sendObjectData);

    static std::vector <BatchBaselinesMessageData> baselines;
    baselines.clear();

    for (OIDListType::iterator i = oidList.begin(); i != oidList.end(); ++i) {
        PROFILER_BLOCK_DEFINE(createBlock, "object create and position");
        PROFILER_BLOCK_ENTER(createBlock);

        const DBSchema::ObjectBufferRow *baseData = m_objectTableBuffer.findConstRowByIndex((*i));
        NOT_NULL(baseData);

        NetworkId networkId = (*i);
        DEBUG_FATAL(networkId != baseData->object_id.getValue(), ("Object ID and row value didn't match"));

        uint32 crc = baseData->object_template_id.getValue();
        CreateObjectByCrcMessage com(networkId, crc, static_cast<unsigned short>(baseData->type_id.getValue()), true, NetworkId(baseData->contained_by.getValue()));
//		connection->send(com,true);
        DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(), ("\tSent CreateObjectMessage for object %s\n", networkId.getValueString().c_str()));

        Transform t;
        Quaternion q(static_cast<real>(baseData->quaternion_w.getValue()), static_cast<real>(baseData->quaternion_x.getValue()), static_cast<real>(baseData->quaternion_y.getValue()), static_cast<real>(baseData->quaternion_z.getValue()));
        q.getTransform(&t); // reorients the transform according to the quaternion

        t.validate();

        t.setPosition_p(Vector(static_cast<real>(baseData->x.getValue()), static_cast<real>(baseData->y.getValue()), static_cast<real>(baseData->z.getValue())));

        UpdateObjectPositionMessage uopm(NetworkId(baseData->object_id.getValue()), t, t, NetworkId(baseData->contained_by.getValue()), baseData->slot_arrangement.getValue(), NetworkId(baseData->load_with.getValue()), baseData->player_controlled.getValue(), false);

//		connection->send(uopm,true);

        PROFILER_BLOCK_LEAVE(createBlock);

        PROFILER_BLOCK_DEFINE(encodeData, "encode data");
        PROFILER_BLOCK_ENTER(encodeData);

        bool okToSend = encodeParentClientData(networkId, baseData->type_id.getValue(), baselines);
        okToSend = okToSend && encodeClientData(networkId, baseData->type_id.getValue(), baselines);
        okToSend = okToSend && encodeServerData(networkId, baseData->type_id.getValue(), baselines);
        okToSend = okToSend && encodeSharedData(networkId, baseData->type_id.getValue(), baselines);

        PROFILER_BLOCK_LEAVE(encodeData);

        if (okToSend) {
            PROFILER_AUTO_BLOCK_DEFINE("connection->send (baselines)");

            connection->send(com, true);
            connection->send(uopm, true);
        } else {
            (*i) = NetworkId::cms_invalid;
        }
    }

    BatchBaselinesMessage bbm(baselines);
    connection->send(bbm, true);

    PROFILER_BLOCK_LEAVE(sendObjectData);

    PROFILER_BLOCK_DEFINE(sendEndBaselines, "send EndBaselines");
    PROFILER_BLOCK_ENTER(sendEndBaselines);

    // Send EndBaselines in reverse order (container & portal system requires this)
    for (OIDListType::reverse_iterator r = oidList.rbegin(); r != oidList.rend(); ++r) {
        if (*r != NetworkId::cms_invalid) {
            EndBaselinesMessage const ebm(*r);
            connection->send(ebm, true);
        }
//		DEBUG_REPORT_LOG(true, ("\tSent EndBaselinesMessage for object %i\n",(*r).getValue()));
    }
    PROFILER_BLOCK_LEAVE(sendEndBaselines);

    // Send resource data (if any) after all objects but before the post-baselines custom data
    m_resourceTypeBuffer.sendResourceTypeObjects(*connection);
    m_bountyHunterTargetBuffer.sendBountyHunterTargetMessage(*connection);

    // Send post-baselines custom data, such as "UniverseComplete" messages, etc.
    PROFILER_BLOCK_DEFINE(sendPostBaselinesCustomData, "sendPostBaselinesCustomData");
    PROFILER_BLOCK_ENTER(sendPostBaselinesCustomData);

    for (LocatorListType::const_iterator loc2 = m_locatorList.begin(); loc2 != m_locatorList.end(); ++loc2) {
        (*loc2)->sendPostBaselinesCustomData(*connection);
    }
    PROFILER_BLOCK_LEAVE(sendPostBaselinesCustomData);

    // Send any MessageTos for these objects
    // This must happen after EndBaselinesMessage is sent for all of the objects
    {
        PROFILER_AUTO_BLOCK_DEFINE("send MessageTos");

        m_messageBuffer.sendMessages(*connection);
        for (OIDListType::const_iterator i = oidList.begin(); i != oidList.end(); ++i) {
            if (*i != NetworkId::cms_invalid) {
                MessageToManager::getInstance().sendMessagesForObject(*i, *connection);
            }
        }
    }

    DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(), ("Done sending Snapshot\n"));
    return true;
}

// ----------------------------------------------------------------------

void SwgSnapshot::loadAttributes(DB::Session *session, NetworkId const &objectId) {
    UNREF(session);
}

// ----------------------------------------------------------------------

void SwgSnapshot::loadCommands(DB::Session *session, NetworkId const &objectId) {
    UNREF(session);
    UNREF(objectId);
}

// ----------------------------------------------------------------------

void SwgSnapshot::loadSkills(DB::Session *session, NetworkId const &objectId) {
    UNREF(session);
    UNREF(objectId);
}

// ----------------------------------------------------------------------

/**
 * Helper function to avoid having to put too much logic into generated code.
 */
void
SwgSnapshot::decodeMissionLocation(Archive::ReadIterator &data, DB::BindableDouble &x, DB::BindableDouble &y, DB::BindableDouble &z, DB::BindableString<500> &planet, DB::BindableString<500> &region_name, DB::BindableNetworkId &cell) const {
    MissionLocation temp;
    Archive::get(data, temp);
    temp.packToDatabase(x, y, z, planet, region_name, cell);
}

// ----------------------------------------------------------------------

/**
 * Helper function to avoid having to put too much logic into generated code.
 */
void
SwgSnapshot::encodeMissionLocation(Archive::ByteStream &data, const DB::BindableDouble &x, const DB::BindableDouble &y, const DB::BindableDouble &z, const DB::BindableString<500> &planet, const DB::BindableString<500> &region_name, const DB::BindableNetworkId &cell) const {
    MissionLocation temp;
    temp.unpackFromDatabase(x, y, z, planet, region_name, cell);
    Archive::put(data, temp);
}

// ----------------------------------------------------------------------

void
SwgSnapshot::decodeVector(Archive::ReadIterator &data, DB::BindableDouble &x, DB::BindableDouble &y, DB::BindableDouble &z) const {
    Vector temp;
    Archive::get(data, temp);
    x.setValue(temp.x);
    y.setValue(temp.y);
    z.setValue(temp.z);
}

// ----------------------------------------------------------------------

void
SwgSnapshot::encodeVector(Archive::ByteStream &data, const DB::BindableDouble &x, const DB::BindableDouble &y, const DB::BindableDouble &z) const {
    Vector temp;
    x.getValue(temp.x);
    y.getValue(temp.y);
    z.getValue(temp.z);
    Archive::put(data, temp);
}

// ----------------------------------------------------------------------

void SwgSnapshot::decodeStringId(Archive::ReadIterator &data, DB::BufferString &table, DB::BufferString &text) const {
    StringId temp;
    Archive::get(data, temp);
    temp.packToDatabase(table, text);
}

// ----------------------------------------------------------------------

void
SwgSnapshot::encodeStringId(Archive::ByteStream &data, const DB::BufferString &table, const DB::BufferString &text) const {
    StringId temp;
    temp.unpackFromDatabase(table, text);
    Archive::put(data, temp);
}

// ----------------------------------------------------------------------

void SwgSnapshot::ignorePersistedFlag(const NetworkId &objectId, Archive::ReadIterator &data) const {
    bool persisted;
    Archive::get(data, persisted);

    UNREF(objectId);
}

// ----------------------------------------------------------------------

void
SwgSnapshot::decodeLocationDataList(const NetworkId &objectId, size_t listId, Archive::ReadIterator &data, bool isBaseline) {
    if (isBaseline) {
        size_t size, baselineCommandCount;
        Archive::get(data, size);
        Archive::get(data, baselineCommandCount);

        LocationData temp;
        for (size_t i = 0; i < size; ++i) {
            Archive::get(data, temp);
            m_locationBuffer.set(objectId, listId, i, temp);
        }
    } else {
        typedef Archive::AutoDeltaVector <LocationData> VectorType;
        size_t numCommands, baselineCommandCount;
        Archive::get(data, numCommands);
        Archive::get(data, baselineCommandCount);
        for (size_t count = 0; count < numCommands; ++count) {
            unsigned char cmd;
            uint16 index;

            Archive::get(data, cmd);
            Archive::get(data, index);

            switch (cmd) {
                case VectorType::Command::ERASE:
                    m_locationBuffer.erase(objectId, listId, index);
                    break;

                case VectorType::Command::INSERT: {
                    LocationData value;
                    Archive::get(data, value);
                    m_locationBuffer.insert(objectId, listId, index, value);
                    break;
                }

                case VectorType::Command::SET: {
                    LocationData value;
                    Archive::get(data, value);
                    m_locationBuffer.set(objectId, listId, index, value);
                    break;
                }

                case VectorType::Command::SETALL:
                    FATAL(true, ("AutoDeltaVector::Command::SETALL is not supported for location data lists.\n"));

                case VectorType::Command::CLEAR:
                    FATAL(true, ("AutoDeltaVector::Command::CLEAR is not supported for location data lists.\n"));

                default:
                    FATAL(true, ("Unknown AutoDeltaVector command.\n"));
            }
        }
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeLocationDataList(const NetworkId &objectId, size_t listId, Archive::ByteStream &data) const {
    std::vector <LocationData> values;
    m_locationBuffer.getLocationList(objectId, listId, values);

    Archive::put(data, values.size());
    Archive::put(data, static_cast<size_t>(0));
    for (std::vector<LocationData>::iterator i = values.begin(); i != values.end(); ++i) {
        Archive::put(data, *i);
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::decodeExperience(const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline) {
    typedef Archive::AutoDeltaMap<std::string, int> MapType;
    typedef std::vector <MapType::Command> CommandList;
    CommandList commands;
    if (isBaseline) {
        MapType::unpack(data, commands);
    } else {
        MapType::unpackDelta(data, commands);
    }
    for (CommandList::const_iterator i = commands.begin(); i != commands.end(); ++i) {
        switch ((*i).cmd) {
            case MapType::Command::ERASE:
                m_experienceBuffer.removeExperience(objectId, (*i).key);
                break;

            case MapType::Command::ADD:
            case MapType::Command::SET:
                m_experienceBuffer.setExperience(objectId, (*i).key, (*i).value);
                break;

            default:
                FATAL(true, ("Unknown AutoDeltaMap command.\n"));
        }
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeExperience(const NetworkId &objectId, Archive::ByteStream &data) const {
    typedef std::vector <std::pair<std::string, int>> ValuesType;
    ValuesType values;
    m_experienceBuffer.getExperienceForObject(objectId, values);

    Archive::put(data, values.size());
    Archive::put(data, static_cast<size_t>(0)); // baselineCommandCount
    for (ValuesType::const_iterator i = values.begin(); i != values.end(); ++i) {
        const unsigned char command = 0; //ADD
        Archive::put(data, command);
        Archive::put(data, (*i).first);
        Archive::put(data, (*i).second);
    }
}

// ----------------------------------------------------------------------

void
SwgSnapshot::decodeBattlefieldParticipants(const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline) {
    typedef Archive::AutoDeltaMap <NetworkId, uint32> MapType;
    typedef std::vector <MapType::Command> CommandList;
    CommandList commands;
    if (isBaseline) {
        MapType::unpack(data, commands);
    } else {
        MapType::unpackDelta(data, commands);
    }
    for (CommandList::const_iterator i = commands.begin(); i != commands.end(); ++i) {
        switch ((*i).cmd) {
            case MapType::Command::ERASE:
                m_battlefieldParticipantBuffer.removeParticipantForRegion(objectId, (*i).key);
                break;

            case MapType::Command::ADD:
            case MapType::Command::SET:
                m_battlefieldParticipantBuffer.setParticipantForRegion(objectId, (*i).key, (*i).value);
                break;

            default:
                FATAL(true, ("Unknown AutoDeltaMap command.\n"));
        }
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeBattlefieldParticipants(const NetworkId &objectId, Archive::ByteStream &data) const {
    typedef std::vector <std::pair<NetworkId, uint32>> ValuesType;
    ValuesType values;
    m_battlefieldParticipantBuffer.getParticipantsForRegion(objectId, values);

    Archive::put(data, values.size());
    Archive::put(data, static_cast<size_t>(0)); // baselineCommandCount
    for (ValuesType::const_iterator i = values.begin(); i != values.end(); ++i) {
        const unsigned char command = 0; //ADD
        Archive::put(data, command);
        Archive::put(data, (*i).first);
        Archive::put(data, (*i).second);
    }
}

// ----------------------------------------------------------------------

void
SwgSnapshot::decodeManufactureSchematicAttributes(const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline) {
    typedef Archive::AutoDeltaMap<StringId, float> MapType;
    typedef std::vector <MapType::Command> CommandList;
    CommandList commands;
    if (isBaseline) {
        MapType::unpack(data, commands);
    } else {
        MapType::unpackDelta(data, commands);
    }
    for (CommandList::const_iterator i = commands.begin(); i != commands.end(); ++i) {
        switch ((*i).cmd) {
            case MapType::Command::ERASE:
                m_manufactureSchematicAttributeBuffer.removeManufactureSchematicAttribute(objectId, (*i).key.getCanonicalRepresentation());
                break;

            case MapType::Command::ADD:
            case MapType::Command::SET:
                m_manufactureSchematicAttributeBuffer.setManufactureSchematicAttribute(objectId, (*i).key.getCanonicalRepresentation(), (*i).value);
                break;

            default:
                FATAL(true, ("Unknown AutoDeltaMap command.\n"));
        }
    }
}

void SwgSnapshot::encodeManufactureSchematicAttributes(const NetworkId &objectId, Archive::ByteStream &data) const {
    typedef std::vector <std::pair<std::string, float>> ValuesType;
    ValuesType values;
    m_manufactureSchematicAttributeBuffer.getDataForObject(objectId, values);

    Archive::put(data, values.size());
    Archive::put(data, static_cast<size_t>(0)); // baselineCommandCount
    for (ValuesType::const_iterator i = values.begin(); i != values.end(); ++i) {
        const unsigned char command = 0; //ADD
        Archive::put(data, command);
        StringId temp((*i).first);
        Archive::put(data, temp);
        Archive::put(data, (*i).second);
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::decodeMatchMakingId(Archive::ReadIterator &data, DB::BufferString &column) const {
    MatchMakingId mmid;
    std::string temp;
    Archive::get(data, mmid);
    mmid.packBitString(temp);
    column.setValue(temp);
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeMatchMakingId(Archive::ByteStream &data, const DB::BufferString &column) const {
    MatchMakingId mmid;
    std::string temp;
    column.getValue(temp);
    mmid.unPackBitString(temp);
    Archive::put(data, mmid);
}
// ----------------------------------------------------------------------

void SwgSnapshot::decodeComponents(NetworkId const &objectId, Archive::ReadIterator &data, bool isBaseline) {
    typedef Archive::AutoDeltaSet<int> SetType;
    typedef std::vector <SetType::Command> CommandList;
    CommandList commands;
    if (isBaseline) {
        SetType::unpack(data, commands);
    } else {
        SetType::unpackDelta(data, commands);
    }
    for (CommandList::const_iterator i = commands.begin(); i != commands.end(); ++i) {
        switch (i->cmd) {
            case SetType::Command::ERASE: {
                char buffer[50];
                _itoa(i->value, buffer, 10);
                m_propertyListBuffer.deleteProperty(objectId, 11, buffer);
                break;
            }

            case SetType::Command::INSERT: {
                char buffer[50];
                _itoa(i->value, buffer, 10);
                m_propertyListBuffer.insertProperty(objectId, 11, buffer);
                break;
            }

            case SetType::Command::CLEAR:
                m_propertyListBuffer.clearProperties(objectId, 11);
                break;

            default:
                FATAL(true, ("Unknown AutoDeltaSet command.\n"));
        }
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeComponents(const NetworkId &objectId, Archive::ByteStream &data) const {
    typedef Archive::AutoDeltaSet<int> SetType;
    typedef std::set <std::string> PackedValuesType;
    typedef std::set<int> ValuesType;

    PackedValuesType packedValues;
    m_propertyListBuffer.getPropertyListForObject(objectId, 11, packedValues);

    ValuesType values;
    for (PackedValuesType::const_iterator i = packedValues.begin(); i != packedValues.end(); ++i) {
        values.insert(atoi((*i).c_str()));
    }

    SetType::pack(data, values);
}

// ----------------------------------------------------------------------

void
SwgSnapshot::decodeLocation(Archive::ReadIterator &data, DB::BindableDouble &x, DB::BindableDouble &y, DB::BindableDouble &z, DB::BindableNetworkId &cell, DB::BindableLong &planet) const {
    Location temp;
    Archive::get(data, temp);
    Vector vec = temp.getCoordinates();
    x = vec.x;
    y = vec.y;
    z = vec.z;
    cell = temp.getCell();
    planet = temp.getSceneIdCrc();
}

// ----------------------------------------------------------------------

void
SwgSnapshot::encodeLocation(Archive::ByteStream &data, const DB::BindableDouble &x, const DB::BindableDouble &y, const DB::BindableDouble &z, const DB::BindableNetworkId &cell, const DB::BindableLong &planet) const {
    Location temp(Vector(static_cast<float>(x.getValue()), static_cast<float>(y.getValue()), static_cast<float>(z.getValue())), cell.getValue(), planet.getValue());
    Archive::put(data, temp);
}

// ----------------------------------------------------------------------

void SwgSnapshot::decodeWaypoints(const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline) {
    typedef Archive::AutoDeltaMap <NetworkId, PersistableWaypoint> MapType;
    typedef std::vector <MapType::Command> CommandList;
    CommandList commands;
    if (isBaseline) {
        MapType::unpack(data, commands);
    } else {
        MapType::unpackDelta(data, commands);
    }
    for (CommandList::const_iterator i = commands.begin(); i != commands.end(); ++i) {
        switch ((*i).cmd) {
            case MapType::Command::ERASE:
                m_waypointBuffer.removeWaypoint(objectId, (*i).key);
                break;

            case MapType::Command::ADD:
            case MapType::Command::SET:
                m_waypointBuffer.setWaypoint(objectId, (*i).key, (*i).value);
                break;

            default:
                FATAL(true, ("Unknown AutoDeltaMap command.\n"));
        }
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeWaypoints(const NetworkId &objectId, Archive::ByteStream &data) const {
    typedef std::vector <PersistableWaypoint> ValuesType;
    ValuesType values;
    m_waypointBuffer.getWaypointsForObject(objectId, values);

    Archive::put(data, values.size());
    Archive::put(data, static_cast<size_t>(0)); // baselineCommandCount
    for (ValuesType::const_iterator i = values.begin(); i != values.end(); ++i) {
        const unsigned char command = 0; //ADD
        Archive::put(data, command);
        Archive::put(data, i->m_networkId);
        Archive::put(data, *i);
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::decodeSingleWaypoint(const NetworkId &objectId, Archive::ReadIterator &data, bool isBaseline) {
    UNREF(isBaseline);  // parameter is there for consistency.

    PersistableWaypoint value;
    Archive::get(data, value);

    m_waypointBuffer.setWaypoint(objectId, value.m_networkId, value);
}

// ----------------------------------------------------------------------

void SwgSnapshot::encodeSingleWaypoint(const NetworkId &objectId, Archive::ByteStream &data) const {
    typedef std::vector <PersistableWaypoint> ValuesType;
    ValuesType values;
    m_waypointBuffer.getWaypointsForObject(objectId, values);

    DEBUG_FATAL(values.size() !=
                1, ("Object %s should have exactly 1 waypoint, but it has %i", objectId.getValueString().c_str(), values.size()));

    ValuesType::const_iterator i = values.begin();
    if (i != values.end()) {
        Archive::put(data, *i);
    } else {
        Archive::put(data, PersistableWaypoint());
    }
}

// ----------------------------------------------------------------------

void SwgSnapshot::saveCompleted() {
    m_messageBuffer.removeSavedMessagesFromMemory();
    Snapshot::saveCompleted();
}

// ----------------------------------------------------------------------

void SwgSnapshot::handleCommoditiesDataMessage(const MessageDispatch::MessageBase &message) {
    UNREF(message);
    DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(), ("Received Commodities Data in non-commodities snapshot!\n"));
}

// ----------------------------------------------------------------------

void SwgSnapshot::startLoadAfterSaveComplete() {
    DEBUG_WARNING(true, ("Called startLoadAfterSaveComplete in non-commodities snapshot!\n"));
}

// ----------------------------------------------------------------------
/**
 * For gold mode, return a list of all the objects in the world that may be
 * ordinary (non-demand-load) containers.  The live database will be checked
 * for objects inside these objects.
 */
void SwgSnapshot::getWorldContainers(std::vector <NetworkId> &containers) const {
    m_objectTableBuffer.getWorldContainers(containers);
}

// ----------------------------------------------------------------------

void SwgSnapshot::handleAddResourceTypeMessage(AddResourceTypeMessage const &message) {
    m_resourceTypeBuffer.handleAddResourceTypeMessage(message);
}

// ----------------------------------------------------------------------

void SwgSnapshot::handleBountyHunterTargetMessage(const BountyHunterTargetMessage &message) {
    m_bountyHunterTargetBuffer.setBountyHunterTarget(message.GetObjectId(), message.GetTargetId());
}

// ----------------------------------------------------------------------

void
SwgSnapshot::getMoneyFromOfflineObject(uint32 replyServer, NetworkId const &sourceObject, int amount, NetworkId const &replyTo, std::string const &successCallback, std::string const &failCallback, std::vector <int8> const &packedDictionary) {
    // Object might be "offline" but still buffered.  Try to handle it in memory first:
    if (!m_objectTableBuffer.getMoneyFromOfflineObject(replyServer, sourceObject, amount, replyTo, successCallback, failCallback, packedDictionary)) {
        // Object is offline.  Handle it in the DB
        getOfflineMoneyCustomPersistStep()->getMoneyFromOfflineObject(replyServer, sourceObject, amount, replyTo, successCallback, failCallback, packedDictionary);
    }
}

// ----------------------------------------------------------------------

OfflineMoneyCustomPersistStep *SwgSnapshot::getOfflineMoneyCustomPersistStep() {
    if (!m_offlineMoneyCustomPersistStep) {
        m_offlineMoneyCustomPersistStep = new OfflineMoneyCustomPersistStep;
        addCustomPersistStep(m_offlineMoneyCustomPersistStep);
    }
    return m_offlineMoneyCustomPersistStep;
}

// ----------------------------------------------------------------------

void
SwgSnapshot::decodeQuests(NetworkId const &networkId, Archive::ReadIterator &data, DB::BufferString &part1, DB::BufferString &part2, DB::BufferString &part3, DB::BufferString &part4) const {

    const int ki_num_parts = 4;
    const int ki_chunk_size = DBSchema::PlayerObjectRow::QUEST_DATA_SIZE;
    static const std::string space(" ");

    std::string packedValue;
    Archive::AutoDeltaPackedMap<uint32, PlayerQuestData>::unpack(data, packedValue);

    if (packedValue.length() > ki_chunk_size * ki_num_parts) {
        // Remove completed quests to save space.  This is a last-ditch effort to save a crash, not a good way to
        // handle it.

        WARNING(true, ("QuestData:  Object %s had too much quest data to be stored in the database.  Truncating.", networkId.getValueString().c_str()));
        int charsToRemove = packedValue.length() - ki_chunk_size * ki_num_parts;
        char const *sourcePos = packedValue.c_str();
        std::string newPackedValue;
        bool activeQuest = false;
        char entry[100];
        char *entryPos = entry;
        while (charsToRemove > 0 && *sourcePos != '\0') {
            *(entryPos++) = *sourcePos;

            if (*sourcePos == ' ') {
                activeQuest = true;
            }
            if (*(sourcePos++) == ':') {
                if (activeQuest) {
                    // copy this active quest
                    *entryPos = '\0';
                    newPackedValue += entry;
                    entryPos = entry;
                    activeQuest = false;
                } else {
                    // don't copy this completed quest
                    charsToRemove -= entryPos - entry;
                    entryPos = entry;
                }
            }
        }
        packedValue = newPackedValue + sourcePos;
    }


    part1 = std::string(packedValue, 0, ki_chunk_size);

    // part2
    if (packedValue.length() > ki_chunk_size * 1) {
        part2 = std::string(packedValue, ki_chunk_size * 1, ki_chunk_size);
    } else {
        part2 = space;
    }

    // part3
    if (packedValue.length() > ki_chunk_size * 2) {
        part3 = std::string(packedValue, ki_chunk_size * 2, ki_chunk_size);
    } else {
        part3 = space;
    }

    // part4
    if (packedValue.length() > ki_chunk_size * 3) {
        part4 = std::string(packedValue, ki_chunk_size * 3, ki_chunk_size);
    } else {
        part4 = space;
    }
}

// ----------------------------------------------------------------------

void
SwgSnapshot::encodeQuests(Archive::ByteStream &data, DB::BufferString const &part1, DB::BufferString const &part2, DB::BufferString const &part3, DB::BufferString const &part4) const {
    std::string packedValue;
    std::string packedValue2;
    std::string packedValue3;
    std::string packedValue4;
    part1.getValue(packedValue);
    part2.getValue(packedValue2);
    part3.getValue(packedValue3);
    part4.getValue(packedValue4);
    if (packedValue2 != " ") {
        packedValue += packedValue2;
    }
    if (packedValue3 != " ") {
        packedValue += packedValue3;
    }
    if (packedValue4 != " ") {
        packedValue += packedValue4;
    }
    Archive::AutoDeltaPackedMap<uint32, PlayerQuestData>::pack(data, packedValue);
}

// ----------------------------------------------------------------------

#ifdef WIN32
static const char* oldschoolBuffFormat = "%lu %I64u";
static const char* versionTwoFormat = "%u %u %f %u %I64i %u:";
#else
static const char *oldschoolBuffFormat = "%lu %llu";
static const char *versionTwoFormat = "%u %u %f %u %lli %u:";
#endif

static const char *versionOneFormat = "%u %u %f %u:";
static const char *versionFormat = "%uv";

void SwgSnapshot::decodePersistedBuffs(Archive::ReadIterator &data, DB::BufferString &dbString) const {
    const unsigned version = 2;

    std::string packedValue;

    Archive::AutoDeltaMap <uint32, Buff::PackedBuff> temp;
    Archive::get(data, temp);
    std::map <uint32, Buff::PackedBuff> const &rawMap = temp.getMap();

    FormattedString<200> formater;

    if (rawMap.empty()) {
        packedValue = " ";
    } else {
        packedValue += formater.sprintf(versionFormat, version);

        for (std::map<uint32, Buff::PackedBuff>::const_iterator i = rawMap.begin(); i != rawMap.end(); ++i) {
            uint32 tempCrc = (*i).first;
            Buff::PackedBuff const &tempPacked = (*i).second;
            packedValue += formater.sprintf(versionTwoFormat, tempCrc, tempPacked.endtime, tempPacked.value, tempPacked.duration, tempPacked.caster.getValue(), tempPacked.stackCount);
        }
    }

    dbString.setValue(packedValue);
}

void SwgSnapshot::encodePersistedBuffs(Archive::ByteStream &data, DB::BufferString const &dbString) const {
    // some persisted buffs in the database were written as a AutoDeltaPackedMap of Crc's and uint64
    // the new type ones are marked with a version number followed by a 'v' then the data

    std::string stringRepresentation;
    dbString.getValue(stringRepresentation);
    size_t numchars = stringRepresentation.size();

    int version = -1;

    //format: [<versionNumber>v]<packedData>

    //extract the version
    if (numchars > 0) {
        size_t vpos = stringRepresentation.find('v');
        if (vpos == stringRepresentation.npos) {
            version = 0;
        } else {
            std::string versionToken = stringRepresentation.substr(0, vpos);
            sscanf(versionToken.c_str(), versionFormat, &version);

            stringRepresentation = stringRepresentation.substr(vpos + 1);
        }
    }

    Archive::AutoDeltaMap<uint32, Buff::PackedBuff>::Command tempCommand;
    std::vector <Archive::AutoDeltaMap<uint32, Buff::PackedBuff>::Command> commands;
    char temp[200];

    if (version == 0) {
        //string is the old AutoDeltaPackedMap written as [<crc> <packed64>:]*
        uint64 tempPacked64;
        uint32 tempCrc;

        int numbuffs = Archive::countCharacter(stringRepresentation, ':');
        commands.reserve(numbuffs);

        int tempPos = 0;
        for (std::string::const_iterator i = stringRepresentation.begin(); i != stringRepresentation.end(); ++i) {
            if (*i == ':') {
                temp[tempPos] = '\0';
                sscanf(temp, oldschoolBuffFormat, &tempCrc, &tempPacked64);

                tempCommand.cmd = Archive::AutoDeltaMap<uint32, Buff::PackedBuff>::Command::ADD;
                tempCommand.key = tempCrc;
                tempCommand.value = Buff::makePackedBuff(tempPacked64);

                commands.push_back(tempCommand);

                tempPos = 0;
            } else {
                temp[tempPos++] = *i;
            }
        }
    } else if (version == 1) {
        //version 1 written as [<crc> <endtime> <value> <duration>:]*
        uint32 tempCrc = 0, tempEndtime = 0, tempDuration = 0;
        float tempValue = 0.0f;

        int numbuffs = Archive::countCharacter(stringRepresentation, ':');
        commands.reserve(numbuffs);

        int tempPos = 0;
        for (std::string::const_iterator i = stringRepresentation.begin(); i != stringRepresentation.end(); ++i) {
            if (*i == ':') {
                temp[tempPos] = '\0';
                sscanf(temp, versionOneFormat, &tempCrc, &tempEndtime, &tempValue, &tempDuration);

                tempCommand.cmd = Archive::AutoDeltaMap<uint32, Buff::PackedBuff>::Command::ADD;
                tempCommand.key = tempCrc;
                tempCommand.value = Buff(tempCrc, tempEndtime, tempValue, tempDuration).getPackedBuffValue();

                commands.push_back(tempCommand);

                tempPos = 0;
            } else {
                temp[tempPos++] = *i;
            }
        }
    } else if (version == 2) {
        //version 2 written as [<crc> <endtime> <value> <duration> <caster> <stackCount>:]*
        uint32 tempCrc = 0, tempEndtime = 0, tempDuration = 0, tempStackCount = 0;
        float tempValue = 0.0f;
        int64 tempCaster = 0;

        int numbuffs = Archive::countCharacter(stringRepresentation, ':');
        commands.reserve(numbuffs);

        int tempPos = 0;
        for (std::string::const_iterator i = stringRepresentation.begin(); i != stringRepresentation.end(); ++i) {
            if (*i == ':') {
                temp[tempPos] = '\0';
                sscanf(temp, versionTwoFormat, &tempCrc, &tempEndtime, &tempValue, &tempDuration, &tempCaster, &tempStackCount);

                tempCommand.cmd = Archive::AutoDeltaMap<uint32, Buff::PackedBuff>::Command::ADD;
                tempCommand.key = tempCrc;
                tempCommand.value = Buff(tempCrc, tempEndtime, tempValue, tempDuration, NetworkId(tempCaster), tempStackCount).getPackedBuffValue();

                commands.push_back(tempCommand);

                tempPos = 0;
            } else {
                temp[tempPos++] = *i;
            }
        }
    } else {
        //we don't know how to convert this string to buffs, treat it as though there were no buffs there at all
        WARNING(true, ("encodePersistedBuffs: Cannot convert db string with version %d to buffs", version));
    }

    Archive::AutoDeltaMap<uint32, Buff::PackedBuff>::pack(data, commands);

}

// ======================================================================
