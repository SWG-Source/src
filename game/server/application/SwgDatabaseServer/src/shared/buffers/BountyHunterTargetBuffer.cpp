// ======================================================================
//
// BountyHunterTargetBuffer.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/BountyHunterTargetBuffer.h"

#include "SwgDatabaseServer/BountyHunterTargetQuery.h"
#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverGame/ServerPlanetObjectTemplate.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

BountyHunterTargetBuffer::BountyHunterTargetBuffer() :
		AbstractTableBuffer(),
		m_data()
{
}

// ----------------------------------------------------------------------

BountyHunterTargetBuffer::~BountyHunterTargetBuffer()
{
	for (DataType::iterator i=m_data.begin(); i!=m_data.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------

void BountyHunterTargetBuffer::setBountyHunterTarget(const NetworkId &objectId, const NetworkId &targetId)
{
	DBSchema::BountyHunterTargetRow *row=findRowByIndex(objectId);
	if (!row)
	{
		row=new DBSchema::BountyHunterTargetRow;
		addRowToIndex(objectId, row);
	}
	row->object_id = objectId;
	row->target_id = targetId;
}

// ----------------------------------------------------------------------

void BountyHunterTargetBuffer::sendBountyHunterTargetMessage(GameServerConnection & conn) const
{
	if (m_data.empty())
		return;
	
	std::vector< std::pair< NetworkId, NetworkId > > targetList;
	
	for (DataType::const_iterator row = m_data.begin(); row != m_data.end(); ++row)
	{
		NOT_NULL(row->second);
		
		NetworkId objectId;
		NetworkId targetId;
		row->second->object_id.getValue(objectId);
		row->second->target_id.getValue(targetId);
		targetList.push_back(std::make_pair(objectId, targetId));
	}

	DEBUG_REPORT_LOG(true,("Sending BountyHunterTargetsList\n"));
	BountyHunterTargetListMessage msg(targetList);
	conn.send(msg,true);
}

// ----------------------------------------------------------------------

DBSchema::BountyHunterTargetRow * BountyHunterTargetBuffer::findRowByIndex(const NetworkId &objectId)
{
	DataType::iterator i=m_data.find(objectId);
	if (i==m_data.end())
		return 0;
	else
		return (*i).second;
}

// ----------------------------------------------------------------------

void BountyHunterTargetBuffer::addRowToIndex (const NetworkId &objectId, DBSchema::BountyHunterTargetRow *row)
{
	m_data[objectId]=row;
}

// ----------------------------------------------------------------------

bool BountyHunterTargetBuffer::save(DB::Session *session)
{
	bool rval = true;
	if (m_data.empty())
		return true;
	
	int actualSaves=0;
	DBQuery::BountyHunterTargetQuery qry;
	if (!qry.setupData(session))
		return false;

	for (DataType::const_iterator i=m_data.begin(); i!=m_data.end(); ++i)
	{
		++actualSaves;
		NOT_NULL(i->second);
		if (!qry.addData(*(i->second)))
			return false;

		if (qry.getNumItems() == ConfigServerDatabase::getDefaultMessageBulkBindSize())
		{	
			if (! (session->exec(&qry)))
				return false;
			qry.clearData();
		}
	}
	
	if (qry.getNumItems() != 0)
		rval = session->exec(&qry);

	qry.done();
	qry.freeData();

	LOG("SaveCounts",("BountyHunterTargets:  %i saved in db",actualSaves));
	return rval;
}

// ----------------------------------------------------------------------

bool BountyHunterTargetBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase)
{
	int rowsFetched = 0;
	// Only load Bounty Hunter Target data if the planets are being loaded
	if (tags.find(ServerPlanetObjectTemplate::ServerPlanetObjectTemplate_tag)!=tags.end())
	{
		DEBUG_REPORT_LOG(true,("Loading BountyHunterTargets\n"));
		DBQuery::BountyHunterTargetQuerySelect qry(schema);

		if (! (session->exec(&qry)))
			return false;

		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t const numRows = qry.getNumRowsFetched();
			size_t count = 0;
			std::vector<DBSchema::BountyHunterTargetRow> const & data = qry.getData();

			for (std::vector<DBSchema::BountyHunterTargetRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;

				DBSchema::BountyHunterTargetRow *row=new DBSchema::BountyHunterTargetRow(*i);
				NOT_NULL(row);
				NetworkId temp;
				row->object_id.getValue(temp);
				m_data.insert(std::make_pair(row->object_id.getValue(), row));
			}
		}

		qry.done();
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void BountyHunterTargetBuffer::removeObject(const NetworkId &object)
{
	DBSchema::BountyHunterTargetRow *row=findRowByIndex(object);
	if (row)
	{
		const NetworkId zeroNetworkId(static_cast<NetworkId::NetworkIdType>(0));
		row->object_id = object;
		row->target_id = zeroNetworkId;
	}
}

// ======================================================================
