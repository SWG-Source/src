// ======================================================================
//
// ObjvarBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "ObjvarBuffer.h"

#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

namespace ObjvarBufferNamespace {
    const int ms_maxItemsPerExec = 10000;
}

using namespace ObjvarBufferNamespace;

// ======================================================================

ObjvarBuffer::ObjvarBuffer(DB::ModeQuery::Mode mode, ObjectTableBuffer *objectTableBuffer, bool useGoldNames)
        : AbstractTableBuffer(), m_mode(mode), m_data(), m_objectTableBuffer(objectTableBuffer),
          m_useGoldNames(useGoldNames) {
}

// ----------------------------------------------------------------------

ObjvarBuffer::~ObjvarBuffer() {
    m_data.clear();
}

bool
ObjvarBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase) {
    int rowsFetched;
    UNREF(tags); // all objects have objvars

    DBQuery::GetAllObjectVariables qry(schema);

    if (!(session->exec(&qry))) {
        return false;
    }

    while ((rowsFetched = qry.fetch()) > 0) {
        size_t numRows = qry.getNumRowsFetched();
        size_t count = 0;
        const std::vector <DBSchema::ObjectVariableRow> &data = qry.getData();

        for (std::vector<DBSchema::ObjectVariableRow>::const_iterator row = data.begin(); row != data.end(); ++row) {
            if (++count > numRows) {
                break;
            }

            IndexKey key(row->object_id.getValue(), row->name_id.getValue());
            ObjvarValue value;
            value.m_type = row->type.getValue();

            // The string is stored in the database as utf8, so a wide-to-narrow is appropriate
            value.m_value = Unicode::wideToNarrow(row->value.getValue());

            m_data.insert(std::make_pair(key, value));
        }
    }

    qry.done();
    if (rowsFetched < 0) {
        return false;
    }

    if (usingGoldDatabase) {
        // Check for local overrides to the gold data
        DBQuery::GetObjectVariableOverrides qry(DatabaseProcess::getInstance().getSchemaQualifier(), schema);
        if (!(session->exec(&qry))) {
            return false;
        }

        while ((rowsFetched = qry.fetch()) > 0) {
            size_t numRows = qry.getNumRowsFetched();
            size_t count = 0;
            const std::vector <DBSchema::ObjectVariableRow> &data = qry.getData();

            for (std::vector<DBSchema::ObjectVariableRow>::const_iterator row = data.begin();
                 row != data.end(); ++row) {
                if (++count > numRows) {
                    break;
                }

                IndexKey key(row->object_id.getValue(), row->name_id.getValue());
                ObjvarValue value;
                value.m_type = row->type.getValue();

                // The string is stored in the database as utf8, so a wide-to-narrow is appropriate
                value.m_value = Unicode::wideToNarrow(row->value.getValue());

                value.m_detached = false;

                m_data.insert(std::make_pair(key, value));
            }
        }

        qry.done();
    }
    return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

bool ObjvarBuffer::save(DB::Session *session) {
    LOG("SaveCounts", ("Objvars:  %i saved to db", m_data.size()));

    {
        DBQuery::AddObjectVariableQuery addQuery;
        if (!addQuery.setupData(session)) {
            return false;
        }
        for (DataType::iterator i = m_data.begin(); i != m_data.end(); ++i) {
            if (!i->second.m_detached && !i->second.m_inDatabase) {
                if (!addQuery.addData(i->first.m_objectId, i->first.m_nameId, i->second.m_type, i->second.m_value)) {
                    return false;
                }
            }
            if (addQuery.getNumItems() == ObjvarBufferNamespace::ms_maxItemsPerExec) {
                if (!(session->exec(&addQuery))) {
                    return false;
                }
                addQuery.clearData();
            }
        }
        if (addQuery.getNumItems() != 0) {
            if (!(session->exec(&addQuery))) {
                return false;
            }
        }
        addQuery.done();
        addQuery.freeData();
    }

    {
        DBQuery::UpdateObjectVariableQuery updateQuery;
        if (!updateQuery.setupData(session)) {
            return false;
        }
        for (DataType::iterator i = m_data.begin(); i != m_data.end(); ++i) {
            if (!i->second.m_detached && i->second.m_inDatabase) {
                if (!updateQuery.addData(i->first.m_objectId, i->first.m_nameId, i->second.m_type, i->second.m_value)) {
                    return false;
                }
            }
            if (updateQuery.getNumItems() == ObjvarBufferNamespace::ms_maxItemsPerExec) {
                if (!(session->exec(&updateQuery))) {
                    return false;
                }
                updateQuery.clearData();
            }
        }
        if (updateQuery.getNumItems() != 0) {
            if (!(session->exec(&updateQuery))) {
                return false;
            }
        }
        updateQuery.done();
        updateQuery.freeData();
    }

    {
        DBQuery::RemoveObjectVariableQuery removeQuery;
        if (!removeQuery.setupData(session)) {
            return false;
        }
        for (DataType::iterator i = m_data.begin(); i != m_data.end(); ++i) {
            if (i->second.m_detached && i->second.m_inDatabase) {
                if (!removeQuery.addData(i->first.m_objectId, i->first.m_nameId)) {
                    return false;
                }
            }
            if (removeQuery.getNumItems() == ObjvarBufferNamespace::ms_maxItemsPerExec) {
                if (!(session->exec(&removeQuery))) {
                    return false;
                }
                removeQuery.clearData();
            }
        }
        if (removeQuery.getNumItems() != 0) {
            if (!(session->exec(&removeQuery))) {
                return false;
            }
        }
        removeQuery.done();
        removeQuery.freeData();
    }
    return true;
}

// ----------------------------------------------------------------------

void
ObjvarBuffer::getObjvarsForObject(const NetworkId &objectId, std::vector <DynamicVariableList::MapType::Command> &commands) const {
    DynamicVariableList::MapType::Command c;

    // unpacked object variables

    {
        for (DataType::const_iterator i = m_data.lower_bound(IndexKey(objectId, 0));
             (i != m_data.end()) && (i->first.m_objectId == objectId); ++i) {
            std::string name;
            bool foundName = false;
            if (m_useGoldNames) {
                foundName = ObjvarNameManager::getGoldInstance().getName(i->first.m_nameId, name);
            } else {
                foundName = ObjvarNameManager::getInstance().getName(i->first.m_nameId, name);
            }

            if (foundName) {
                c.cmd = DynamicVariableList::MapType::Command::ADD;
                c.key = name;
                c.value.load(-1, i->second.m_type, Unicode::utf8ToWide(i->second.m_value));

                commands.push_back(c);
            } else {
                WARNING_STRICT_FATAL(true, ("Object %s has an objvar with name_id %i, which was not in the list of names.", objectId.getValueString().c_str(), i->first.m_nameId));
            }
        }
    }

    //packed object variables

    m_objectTableBuffer->getObjvarsForObject(objectId, commands);

    // Apply overrides, if any.
    // The last objvar sent to the game takes precedence, so we want to send the overrides after the values read in through the regular process

    {
        for (DataType::const_iterator i = m_data.lower_bound(IndexKey(objectId, 0));
             (i != m_data.end()) && (i->first.m_objectId == objectId); ++i) {
            std::string name;
            bool foundName = false;
            foundName = ObjvarNameManager::getInstance().getName(i->first.m_nameId, name); // overrides always use the live names, not the gold names.

            if (foundName) {
                c.cmd = DynamicVariableList::MapType::Command::ADD;
                c.key = name;
                c.value.load(-1, i->second.m_type, Unicode::utf8ToWide(i->second.m_value));

                commands.push_back(c);
            } else {
                WARNING_STRICT_FATAL(true, ("Object %s has an objvar with name_id %i, which was not in the list of names.", objectId.getValueString().c_str(), i->first.m_nameId));
            }
        }
    }
}

// ----------------------------------------------------------------------

void
ObjvarBuffer::updateObjvars(const NetworkId &objectId, const std::vector <DynamicVariableList::MapType::Command> &commands) {
    bool override = false; // flag that we're dealing with the gold data override case (storing an objvar change in the live database, on an object that came from the gold database)
    if (ConfigServerDatabase::getEnableGoldDatabase() && objectId < ConfigServerDatabase::getMaxGoldNetworkId()) {
        override = true;
    }

    for (std::vector<DynamicVariableList::MapType::Command>::const_iterator i = commands.begin();
         i != commands.end(); ++i) {
        switch (i->cmd) {
            case DynamicVariableList::MapType::Command::ADD :
            case DynamicVariableList::MapType::Command::SET : {
                std::string packedValue(Unicode::wideToUTF8(i->value.getPackedValueString()));

                if (i->value.getPosition() != -1 && ConfigServerDatabase::getEnableObjvarPacking() && !override) {
                    m_objectTableBuffer->handlePackedObjvar(objectId, i->value.getPosition(), i->key, i->value.getType(), packedValue);
                } else {
                    int nameId = ObjvarNameManager::getInstance().getOrAddNameId(i->key);

                    DataType::iterator row = m_data.find(IndexKey(objectId, nameId));
                    if (row == m_data.end()) {
                        row = m_data.insert(std::make_pair(IndexKey(objectId, nameId), ObjvarValue())).first;
                        if (i->cmd == DynamicVariableList::MapType::Command::ADD || override) {
                            row->second.m_inDatabase = false; // new variable
                        } else {
                            row->second.m_inDatabase = true;
                        } // update to existing variable
                    }
                    row->second.m_type = i->value.getType();
                    row->second.m_value = packedValue;
                    row->second.m_detached = false;
                }
                break;
            }

            case DynamicVariableList::MapType::Command::ERASE : {
                int nameId = ObjvarNameManager::getInstance().getNameId(i->key);
                if (nameId !=
                    0) // It's possible to get an ERASE for a packed objvar where the name was never used in the OBJECT_VARIABLES table.  We can safely ignore these.
                {
                    DataType::iterator row = m_data.find(IndexKey(objectId, nameId));
                    if (row == m_data.end()) {
                        if (i->value.getPosition() == -1 || override) {
                            row = m_data.insert(std::make_pair(IndexKey(objectId, nameId), ObjvarValue())).first;
                            row->second.m_inDatabase = true; // deleting existing variable
                        }
                        // else it was a packed objvar, and no update is necessary
                    }

                    row->second.m_type = i->value.getType();
                    row->second.m_detached = true; //why the fuck even store it at this point?
                }

                break;
            }

            default:
                WARNING_STRICT_FATAL(true, ("Unknown command %i unpacked in object variables for object %s.", i->cmd, objectId.getValueString().c_str()));
        }
    }

    DEBUG_WARNING(commands.size() >
                  100, ("Object %s has a suspicious number of objvar updates (%i updates).", objectId.getValueString().c_str(), commands.size()));
}

// ----------------------------------------------------------------------

void ObjvarBuffer::removeObject(const NetworkId &object) {
    DataType::iterator i = m_data.lower_bound(IndexKey(object, 0));
    while (i != m_data.end() && i->first.m_objectId==object) {
    	i = m_data.erase(i);
    }
}

// ======================================================================
