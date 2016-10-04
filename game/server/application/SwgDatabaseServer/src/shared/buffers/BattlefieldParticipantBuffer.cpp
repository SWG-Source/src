// ======================================================================
//
// BattlefieldParticipantBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/BattlefieldParticipantBuffer.h"

#include "SwgDatabaseServer/BattlefieldParticipantQuery.h"
#include "SwgDatabaseServer/Schema.h"
#include "serverGame/ServerBattlefieldMarkerObjectTemplate.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

BattlefieldParticipantBuffer::BattlefieldParticipantBuffer(DB::ModeQuery::Mode mode) :
		AbstractTableBuffer(),
		m_rows(),
		m_mode(mode)
{
}

// ----------------------------------------------------------------------

BattlefieldParticipantBuffer::~BattlefieldParticipantBuffer(void)
{
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------

void BattlefieldParticipantBuffer::setParticipantForRegion(const NetworkId &regionObjectId, const NetworkId &characterObjectId, uint32 factionId)
{
	DBSchema::BattlefieldParticipantRow *row=findRowByIndex(regionObjectId,characterObjectId);
	if (!row)
	{
		row=new DBSchema::BattlefieldParticipantRow;
		addRowToIndex(regionObjectId,characterObjectId,row);
	}
	row->region_object_id = regionObjectId;
	row->character_object_id = characterObjectId;
	row->faction_id = static_cast<int>(factionId);
}

// ----------------------------------------------------------------------

bool BattlefieldParticipantBuffer::getParticipantsForRegion(const NetworkId &objectId, std::vector<std::pair<NetworkId, uint32> > &values) const 
{
	DEBUG_FATAL(values.size()!=0,("Values vector should be empty on call to getBattlefieldParticipantForRegion()\n"));
	IndexType::const_iterator i ( m_rows.lower_bound(IndexKey(objectId,NetworkId::cms_invalid)) );
	if (i==m_rows.end())
		return false;
	for (; (i!=m_rows.end()) && (((*i).second)->region_object_id.getValue()==objectId); ++i)
	{
		NetworkId temp;
		(*i).second->character_object_id.getValue(temp);
		values.push_back(std::pair<NetworkId, uint32>(temp, static_cast<uint32>((*i).second->faction_id.getValue())));
	}
	return true;
}

// ----------------------------------------------------------------------

DBSchema::BattlefieldParticipantRow * BattlefieldParticipantBuffer::findRowByIndex(const NetworkId &regionObjectId, const NetworkId &characterObjectId)
{
	IndexType::iterator i=m_rows.find(IndexKey(regionObjectId,characterObjectId));
	if (i==m_rows.end())
		return 0;
	else
		return (*i).second;
}

// ----------------------------------------------------------------------

void BattlefieldParticipantBuffer::addRowToIndex (const NetworkId &regionObjectId, const NetworkId &characterObjectId, DBSchema::BattlefieldParticipantRow *row)
{
	m_rows[IndexKey(regionObjectId,characterObjectId)]=row;
}

// ----------------------------------------------------------------------

/**
 * Load the attributes for all the remembered objectId's.
 */
bool BattlefieldParticipantBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool)
{
	int rowsFetched = 0;
	if (tags.find(static_cast<Tag>(ServerBattlefieldMarkerObjectTemplate::ServerBattlefieldMarkerObjectTemplate_tag)) != tags.end())
	{
		DBQuery::GetAllBattlefieldParticipant qry(schema);

		if (! (session->exec(&qry)))
			return false;

		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t numRows = qry.getNumRowsFetched();
			size_t count = 0;
			const std::vector<DBSchema::BattlefieldParticipantRow> &data = qry.getData();

			for (std::vector<DBSchema::BattlefieldParticipantRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;

				DBSchema::BattlefieldParticipantRow *row=new DBSchema::BattlefieldParticipantRow(*i);
				NOT_NULL(row);
				NetworkId temp;
				row->character_object_id.getValue(temp);
				addRowToIndex(row->region_object_id.getValue(),temp,row);
			}
		}

		qry.done();
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

bool BattlefieldParticipantBuffer::save(DB::Session *session)
{
	LOG("SaveCounts",("BattlefieldParticipants:  %i saved to db",m_rows.size()));
	
	DBQuery::BattlefieldParticipantQuery qry;
	
	if (m_mode==DB::ModeQuery::mode_INSERT)
		qry.insertMode();
	else
		qry.updateMode();
	
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		qry.setData(*(i->second));
		if (! (session->exec(&qry)))
			return false;
	}
	qry.done();
	return true;
}

// ----------------------------------------------------------------------

void BattlefieldParticipantBuffer::removeObject(const NetworkId &object)
{
	IndexType::iterator i=m_rows.lower_bound(IndexKey(object,NetworkId::cms_invalid));
	while (i!=m_rows.end() && i->first.m_regionObjectId==object)
	{
		IndexType::iterator next=i;
		++next;
		delete i->second;
		i->second=nullptr;
		m_rows.erase(i);
		i=next;
	}
}

// ======================================================================
