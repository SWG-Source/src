// ======================================================================
//
// PropertyListBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "PropertyListBuffer.h"

#include "SwgDatabaseServer/PropertyListQuery.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

using namespace PropertyListBufferNamespace;

// ======================================================================

PropertyListBuffer::PropertyListBuffer(DB::ModeQuery::Mode mode) :
		TableBuffer<DBSchema::PropertyListQueryData, DBQuery::PropertyListQuery>(mode),
		m_loadData()
{
}

// ----------------------------------------------------------------------

PropertyListBuffer::~PropertyListBuffer(void)
{
}

// ----------------------------------------------------------------------

void PropertyListBuffer::insertProperty(const NetworkId &objectId, int listId, const std::string &value)
{
	PropertyListEntry entry(objectId, listId, value);
	m_adds.insert(entry);
	m_deletes.erase(entry); // if this add cancels out a previous delete, don't save the delete
}

// ----------------------------------------------------------------------

void PropertyListBuffer::deleteProperty(const NetworkId &objectId, int listId, const std::string &value)
{
	PropertyListEntry entry(objectId, listId, value);
	m_deletes.insert(entry);
	m_adds.erase(entry); // if this delete cancels out a previous add, don't save the add
}

// ----------------------------------------------------------------------

void PropertyListBuffer::clearProperties(const NetworkId &objectId, int listId)
{
	static const std::string emptyString;
	PropertyListEntry entry(objectId, listId, emptyString);

	// erase any changes we saved for this object & list previously.  For each set, find the range
	// of entries corresponding to this object & list, then erase them
	PropertyListSet::iterator startOfRange = m_deletes.lower_bound(entry);
	PropertyListSet::iterator endOfRange;
	for (endOfRange = startOfRange;
		 endOfRange!=m_deletes.end() && endOfRange->m_objectId == objectId && endOfRange->m_listId == listId;
		 ++endOfRange)
		;
	m_deletes.erase(startOfRange, endOfRange);
	
	startOfRange = m_adds.lower_bound(entry);
	for (endOfRange = startOfRange;
		 endOfRange!=m_adds.end() && endOfRange->m_objectId == objectId && endOfRange->m_listId == listId;
		 ++endOfRange)
		;
	m_adds.erase(startOfRange, endOfRange);

	m_clears.insert(entry);
}

// ----------------------------------------------------------------------

/**
 * Get all the commands for a particular object and value type.
 */

bool PropertyListBuffer::getPropertyListForObject(const NetworkId &objectId, int listId, std::set<std::string> &values) const
{
	static const std::string emptyString;
	
	DEBUG_FATAL(!values.empty(),("Values vector should be empty on call to getPropertyListsForObject()\n"));

	// The data is ordered by object id then by list id, so start with the first entry matching the object id and list id.
	// Return all the entries in sequence until we move to the next list or the next object
	for (PropertyListSet::const_iterator i = m_loadData.lower_bound(PropertyListEntry(objectId, listId, emptyString));
		 i!=m_loadData.end() && i->m_objectId == objectId && i->m_listId == listId;
		 ++i)
	{
		values.insert(i->m_value);
	}

	return true;
}

// ----------------------------------------------------------------------

bool PropertyListBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool)
{
	int rowsFetched;
	UNREF(tags);
	
	DBQuery::GetAllPropertyLists qry(schema);

	if (! (session->exec(&qry)))
		return false;

	while ((rowsFetched = qry.fetch()) > 0)
	{
		size_t numRows = qry.getNumRowsFetched();
		size_t count = 0;
		const std::vector<DBSchema::PropertyListQueryData> &data = qry.getData();

		for (std::vector<DBSchema::PropertyListQueryData>::const_iterator i=data.begin(); i!=data.end(); ++i)
		{
			if (++count > numRows)
				break;

			const DBSchema::PropertyListQueryData &row=*i;
			m_loadData.insert(PropertyListEntry(row.object_id.getValue(), row.list_id.getValue(), row.value.getValueASCII()));
		}
	}

	qry.done();
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

bool PropertyListBuffer::save(DB::Session *session)
{
	DBQuery::PropertyListQuery qry_ot_CLEAR;
	DBQuery::PropertyListQuery qry_ot_DELETE;
	DBQuery::PropertyListQuery qry_ot_INSERT;
	int actualSaves=0;

 	if (!qry_ot_DELETE.setupData(session)) return false;
	if (!qry_ot_INSERT.setupData(session)) return false;
	if (!qry_ot_CLEAR.setupData(session)) return false;

	// Clears must be done first, so that we don't clear something we were trying to add
	{
		for (PropertyListSet::const_iterator i=m_clears.begin(); i!=m_clears.end(); ++i)
		{
			if (m_cancelledObjects.find(i->m_objectId) == m_cancelledObjects.end())
			{
				++actualSaves;
				if (!qry_ot_CLEAR.addData(i->m_objectId, i->m_listId, i->m_value, DBSchema::PropertyListQueryData::ot_CLEAR))
					return false;
				if (qry_ot_CLEAR.getNumItems() == PropertyListBufferNamespace::ms_maxItemsPerExec)
				{	
					if (! (session->exec(&qry_ot_CLEAR)))
						return false;
					qry_ot_CLEAR.clearData();
				}
			}
		}
		if (qry_ot_CLEAR.getNumItems() != 0)
		{
			if (! (session->exec(&qry_ot_CLEAR)))
				return false;
		}
		qry_ot_CLEAR.done();
		qry_ot_CLEAR.freeData();
	}

	{ 
		for (PropertyListSet::const_iterator i=m_adds.begin(); i!=m_adds.end(); ++i)
		{
			if (m_cancelledObjects.find(i->m_objectId) == m_cancelledObjects.end())
			{
				++actualSaves;
				if (!qry_ot_INSERT.addData(i->m_objectId, i->m_listId, i->m_value, DBSchema::PropertyListQueryData::ot_INSERT))
					return false;
				if (qry_ot_INSERT.getNumItems() == PropertyListBufferNamespace::ms_maxItemsPerExec)
				{	
					if (! (session->exec(&qry_ot_INSERT)))
						return false;
					qry_ot_INSERT.clearData();
				}
			}
		}
		if (qry_ot_INSERT.getNumItems() != 0)
		{
			if (! (session->exec(&qry_ot_INSERT)))
				return false;
		}
		qry_ot_INSERT.done();
		qry_ot_INSERT.freeData();
	}

	{ 
		for (PropertyListSet::const_iterator i=m_deletes.begin(); i!=m_deletes.end(); ++i)
		{
			if (m_cancelledObjects.find(i->m_objectId) == m_cancelledObjects.end())
			{
				++actualSaves;
				if (!qry_ot_DELETE.addData(i->m_objectId, i->m_listId, i->m_value, DBSchema::PropertyListQueryData::ot_DELETE))
					return false;
				if (qry_ot_DELETE.getNumItems() == PropertyListBufferNamespace::ms_maxItemsPerExec)
				{	
					if (! (session->exec(&qry_ot_DELETE)))
						return false;
					qry_ot_DELETE.clearData();
				}
			}
		}
		if (qry_ot_DELETE.getNumItems() != 0)
		{
			if (! (session->exec(&qry_ot_DELETE)))
				return false;
		}
		qry_ot_DELETE.done();
		qry_ot_DELETE.freeData();
	}
	
	LOG("SaveCounts",("PropertyList updates:  %i (out of %i) saved to db",actualSaves, m_rows.size()));
	return true;
}

// ----------------------------------------------------------------------

void PropertyListBuffer::removeObject(const NetworkId &object)
{
	m_cancelledObjects.insert(object);
}

// ======================================================================

PropertyListBuffer::PropertyListEntry::PropertyListEntry() :
	m_objectId(),
	m_listId(0),
	m_value()
{
}

// ----------------------------------------------------------------------

PropertyListBuffer::PropertyListEntry::PropertyListEntry(const PropertyListBuffer::PropertyListEntry & rhs) :
	m_objectId(rhs.m_objectId),
	m_listId(rhs.m_listId),
	m_value(rhs.m_value)
{
}

// ----------------------------------------------------------------------

PropertyListBuffer::PropertyListEntry & PropertyListBuffer::PropertyListEntry::operator=(const PropertyListBuffer::PropertyListEntry & rhs)
{
	m_objectId=rhs.m_objectId;
	m_listId=rhs.m_listId;
	m_value=rhs.m_value;

	return *this;
}

// ======================================================================
