// ======================================================================
//
// WaypointBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/WaypointBuffer.h"

#include "SwgDatabaseServer/Schema.h"
#include "SwgDatabaseServer/WaypointQuery.h"
#include "serverGame/ServerMissionObjectTemplate.h"
#include "serverGame/ServerPlayerObjectTemplate.h"
#include "sharedDatabaseInterface/DbException.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedUtility/Location.h"
#include "sharedLog/Log.h"
#include <vector>


// ======================================================================

WaypointBuffer::WaypointBuffer(DB::ModeQuery::Mode mode) :
		AbstractTableBuffer(),
		m_mode(mode),
		m_data()
{
}

// ----------------------------------------------------------------------

WaypointBuffer::~WaypointBuffer(void)
{
}

// ----------------------------------------------------------------------

void WaypointBuffer::setWaypoint(const NetworkId &objectId, const NetworkId &waypointId, const PersistableWaypoint &value)
{
	m_data[IndexKey(objectId,waypointId)] = value;
}

// ----------------------------------------------------------------------

bool WaypointBuffer::getWaypointsForObject(const NetworkId &objectId, std::vector<PersistableWaypoint> &values) const
{
	DEBUG_FATAL(values.size()!=0,("Values vector should be empty on call to getWaypointForObject()\n"));
	IndexType::const_iterator i ( m_data.lower_bound(IndexKey(objectId,NetworkId::cms_invalid)) );
	if (i==m_data.end())
		return false;
	for (; (i!=m_data.end()) && (i->first.m_objectId==objectId); ++i)
	{
		std::string temp;
		values.push_back(i->second);
	}
	return true;
}

// ----------------------------------------------------------------------

bool WaypointBuffer::load(DB::Session *session,const DB::TagSet &tags, const std::string &schema, bool)
{
	int rowsFetched = 0;
	if (   tags.find(static_cast<Tag>(ServerPlayerObjectTemplate::ServerPlayerObjectTemplate_tag)) != tags.end()
	    || tags.find(static_cast<Tag>(ServerMissionObjectTemplate::ServerMissionObjectTemplate_tag)) != tags.end())
	{
		DBQuery::GetAllWaypoints qry(schema);

		if (! (session->exec(&qry)))
			return false;

		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t numRows = qry.getNumRowsFetched();
			size_t count = 0;
			const std::vector<DBSchema::WaypointRow> &data = qry.getData();

			for (std::vector<DBSchema::WaypointRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;

				const DBSchema::WaypointRow &row=*i;
				PersistableWaypoint temp;
				temp.m_appearanceNameCrc = row.appearance_name_crc.getValue();
				temp.m_location = Location(Vector(static_cast<float>(row.location_x.getValue()),
												  static_cast<float>(row.location_y.getValue()),
												  static_cast<float>(row.location_z.getValue())),
										   row.location_cell.getValue(),row.location_scene.getValue());
				temp.m_name = row.name.getValue();
				temp.m_networkId = row.waypoint_id.getValue();
				temp.m_color = static_cast<unsigned char>(row.color.getValue());
				temp.m_active = row.active.getValue();
				temp.m_detached = false;

				m_data[IndexKey(row.object_id.getValue(),temp.m_networkId)]=temp;
			}
		}

		qry.done();
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

bool WaypointBuffer::save(DB::Session *session)
{
	LOG("SaveCounts",("Waypoint:  %i saved to db",m_data.size()));
	
	DBQuery::WaypointQuery qry;
	
	if (m_mode==DB::ModeQuery::mode_INSERT)
		qry.insertMode();
	else
		qry.updateMode();
	
	for (IndexType::iterator i=m_data.begin(); i!=m_data.end(); ++i)
	{
		DBSchema::WaypointRow row;

		row.object_id = i->first.m_objectId;
		row.waypoint_id = i->first.m_waypointId;
		row.appearance_name_crc = i->second.m_appearanceNameCrc;
		row.location_x = i->second.m_location.getCoordinates().x;
		row.location_y = i->second.m_location.getCoordinates().y;
		row.location_z = i->second.m_location.getCoordinates().z;
		row.location_cell = i->second.m_location.getCell();
		row.location_scene = i->second.m_location.getSceneIdCrc();
		row.name = i->second.m_name;
		row.color = i->second.m_color;
		row.active = i->second.m_active;
		row.detached = i->second.m_detached;
		
		qry.setData(row);
		if (! (session->exec(&qry)))
			return false;
	}
	qry.done();
	return true;
}

// ----------------------------------------------------------------------

void WaypointBuffer::removeObject(const NetworkId &object)
{
	IndexType::iterator i=m_data.lower_bound(IndexKey(object,NetworkId::cms_invalid));
	while (i!=m_data.end() && i->first.m_objectId==object)
	{
		IndexType::iterator next=i;
		++next;
		m_data.erase(i);
		i=next;
	}
}

// ======================================================================
