// ======================================================================
//
// ObjvarBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjvarBuffer_H
#define INCLUDED_ObjvarBuffer_H

// ======================================================================

#include "SwgDatabaseServer/ObjectTableBuffer.h"
#include "SwgDatabaseServer/ObjectVariableQueries.h"
#include "serverDatabase/AbstractTableBuffer.h"
#include "sharedFoundation/DynamicVariableList.h"
#include <map>
#include <string>

// ======================================================================

class ObjectTableBuffer;
namespace DBSchema {
    struct ObjvarRow;
}

// ======================================================================

/** 
 * ObjvarBuffer is a table buffer customized to the job of loading and
 * saving object variables.
 *
 * Differences from standard table buffers:
 *
 * 1)  Objvars are keyed by object_id and name
 *
 * 2)  When loading, only the object_id is specified, and all objvars
 *     are loaded for that object.
 */

class ObjvarBuffer : public AbstractTableBuffer {
public:
    explicit ObjvarBuffer(DB::ModeQuery::Mode mode, ObjectTableBuffer *objectTableBuffer, bool useGoldNames);

    virtual ~ObjvarBuffer(void);

    virtual bool load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);

    virtual bool save(DB::Session *session);

    virtual void removeObject(const NetworkId &object);

    DBSchema::ObjectVariableRow *findRowByIndex(const NetworkId &objectId, const std::string &name);

    void updateObjvars(const NetworkId &objectId, const std::vector <DynamicVariableList::MapType::Command> &commands);

    void
    getObjvarsForObject(const NetworkId &objectId, std::vector <DynamicVariableList::MapType::Command> &commands) const;

private:
    struct IndexKey {
        const NetworkId m_objectId;
        int m_nameId;

        IndexKey(const NetworkId &objectId, int nameId);

        bool operator==(const IndexKey &rhs) const;

        bool operator<(const IndexKey &rhs) const;

        bool operator>(const IndexKey &rhs) const;
    };

    struct ObjvarValue {
        int m_type;
        std::string m_value;
        bool m_detached;
        bool m_inDatabase;
    };

    typedef std::map <IndexKey, ObjvarValue> DataType;

    DB::ModeQuery::Mode m_mode;
    DataType m_data;
    ObjectTableBuffer *m_objectTableBuffer;
    bool m_useGoldNames;

//	typedef std::map<NetworkId,std::vector<DBSchema::ObjectVariableRow*> > LoadIndexType;
//	LoadIndexType m_loadIndex;

private:
    ObjvarBuffer(); //disable
    ObjvarBuffer(const ObjvarBuffer &); //disable
    ObjvarBuffer &operator=(const ObjvarBuffer &); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

inline ObjvarBuffer::IndexKey::IndexKey(const NetworkId &objectId, int nameId)
        : m_objectId(objectId), m_nameId(nameId) {
}

// ----------------------------------------------------------------------

inline bool ObjvarBuffer::IndexKey::operator==(const IndexKey &rhs) const {
    return ((m_objectId == rhs.m_objectId) && (m_nameId == rhs.m_nameId));
}

// ----------------------------------------------------------------------

inline bool ObjvarBuffer::IndexKey::operator<(const IndexKey &rhs) const {
    if (m_objectId < rhs.m_objectId) {
        return true;
    }

    if (m_objectId == rhs.m_objectId) {
        return (m_nameId < rhs.m_nameId);
    }

    return false;
}

inline bool ObjvarBuffer::IndexKey::operator>(const IndexKey &rhs) const {
    if (m_objectId > rhs.m_objectId) {
        return true;
    }

    if (m_objectId == rhs.m_objectId) {
        return (m_nameId > rhs.m_nameId);
    }

    return false;
}

// ======================================================================

#endif
