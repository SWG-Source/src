// ======================================================================
//
// LocationBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "LocationBuffer.h"

#include "SwgDatabaseServer/LocationQuery.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "sharedLog/Log.h"

// ======================================================================

LocationBuffer::LocationBuffer(DB::ModeQuery::Mode mode) :
		AbstractTableBuffer()
{
	UNREF(mode);
}

// ----------------------------------------------------------------------

LocationBuffer::~LocationBuffer()
{
}

// ----------------------------------------------------------------------

bool LocationBuffer::save(DB::Session *session)
{
	LOG("SaveCounts",("Locations:  %i saved to db",m_saveData.size()));
	
	DBQuery::SaveLocationQuery qry;
	for (SaveDataType::const_iterator i=m_saveData.begin(); i!=m_saveData.end(); ++i)
	{
		NetworkId networkId=i->first;
		const std::vector<BufferEntry> &data=i->second;
		for (std::vector<BufferEntry>::const_iterator j=data.begin(); j!=data.end(); ++j)
		{
			qry.setData(networkId, j->m_listId, j->m_action, j->m_index, j->m_data);
			if (! (session->exec(&qry)))
				return false;
		}
	}
	
	qry.done();
	return true;
}

// ----------------------------------------------------------------------

bool LocationBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool)
{
	int rowsFetched = 0;
	if (tags.find(static_cast<Tag>(ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag)) != tags.end())
	{
		DBQuery::LoadLocationQuery qry(schema);
		LocationData temp;

		if (! (session->exec(&qry)))
			return false;
		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t numRows = qry.getNumRowsFetched();
			size_t count = 0;
			const std::vector<DBQuery::LoadLocationQuery::LocationRow> &data = qry.getData();

			for (std::vector<DBQuery::LoadLocationQuery::LocationRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;

				i->name.getValue(temp.name);
				i->scene.getValue(temp.scene);
				temp.location.setCenter(static_cast<float>(i->x.getValue()),static_cast<float>(i->y.getValue()),static_cast<float>(i->z.getValue()));
				temp.location.setRadius(static_cast<float>(i->radius.getValue()));
		
				m_loadData[IndexKey(i->object_id.getValue(),i->list_id.getValue(),i->index.getValue())]=temp;
			}
		}
		qry.done();
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void LocationBuffer::removeObject(const NetworkId &object)
{
	m_saveData.erase(object);
}

// ----------------------------------------------------------------------

void LocationBuffer::addAction(BufferEntry::Action action, const NetworkId &objectId, size_t listId, size_t sequenceNumber, const LocationData &value)
{
	m_saveData[objectId].push_back(BufferEntry(listId, action, sequenceNumber, value));
}

// ----------------------------------------------------------------------

void LocationBuffer::getLocationList(const NetworkId &objectId, size_t listId, std::vector<LocationData> &values) const
{
#ifdef _DEBUG
	size_t expectedIndex = 0;
#endif
	
	for (LoadDataType::const_iterator i=m_loadData.find(IndexKey(objectId, listId, 0)); (i!=m_loadData.end()) && (i->first.m_objectId == objectId) && (i->first.m_listId == listId); ++i)
	{
#ifdef _DEBUG
		DEBUG_FATAL(i->first.m_index != expectedIndex++,("Programmer bug:  location list was not sorted or had gaps in the sequence.  Object id %s, list id %i, sequence %i\n",objectId.getValueString().c_str(), listId, i->first.m_index));
#endif

		values.push_back(i->second);
	}
}

// ======================================================================

LocationBuffer::BufferEntry::BufferEntry(size_t listId, Action action, size_t index, const LocationData &data) :
		m_listId(listId),
		m_action(action),
		m_index(index),
		m_data(data)
{
}

// ======================================================================

LocationBuffer::IndexKey::IndexKey()
{
}

// ----------------------------------------------------------------------

LocationBuffer::IndexKey::IndexKey(const NetworkId &objectId, size_t listId, size_t index) :
		m_objectId(objectId),
		m_listId(listId),
		m_index(index)
{
}

// ----------------------------------------------------------------------

LocationBuffer::IndexKey::IndexKey(const IndexKey &rhs) :
		m_objectId(rhs.m_objectId),
		m_listId(rhs.m_listId),
		m_index(rhs.m_index)
{
}

// ----------------------------------------------------------------------

bool LocationBuffer::IndexKey::operator<(const IndexKey &rhs) const
{
	if (m_objectId == rhs.m_objectId)
		if (m_listId == rhs.m_listId)
			return (m_index < rhs.m_index);
		else
			return (m_listId < rhs.m_listId);
	else
		return (m_objectId < rhs.m_objectId);
}

// ======================================================================
