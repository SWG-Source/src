// ======================================================================
//
// TaskGetStructuresForPurge.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/TaskGetStructuresForPurge.h"

#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/StructuresForPurgeMessage.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================


namespace TaskGetStructuresForPurgeNamespace
{
	class GetStructuresForPurgeQuery : public DB::Query
	{
	public:
		explicit GetStructuresForPurgeQuery(StationId stationId);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		NetworkId getStructureId() const;
		NetworkId getOwnerId() const;
		
	private:
		DB::BindableLong m_station_id;
		DB::BindableNetworkId m_structure_id;
		DB::BindableNetworkId m_owner_id;

	private: //disable
		GetStructuresForPurgeQuery(const GetStructuresForPurgeQuery&);
		GetStructuresForPurgeQuery& operator=(const GetStructuresForPurgeQuery&);
	};

	class GetVendorsForPurgeQuery : public DB::Query
	{
	public:
		explicit GetVendorsForPurgeQuery(StationId stationId);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		NetworkId getVendorId() const;
		NetworkId getOwnerId() const;
		Unicode::String getVendorName() const;
		
	private:
		DB::BindableLong m_station_id;
		DB::BindableNetworkId m_vendor_id;
		DB::BindableNetworkId m_owner_id;
		DB::BindableUnicode<128> m_vendor_name;

	private: //disable
		GetVendorsForPurgeQuery(const GetVendorsForPurgeQuery&);
		GetVendorsForPurgeQuery& operator=(const GetVendorsForPurgeQuery&);
	};
}
using namespace TaskGetStructuresForPurgeNamespace;


// ======================================================================

TaskGetStructuresForPurge::TaskGetStructuresForPurge(StationId account, bool warnOnly) :
		DB::TaskRequest(),
		m_account(account),
		m_warnOnly(warnOnly),
		m_structures(),
		m_vendors()
{
}

// ----------------------------------------------------------------------

TaskGetStructuresForPurge::~TaskGetStructuresForPurge()
{
}

// ----------------------------------------------------------------------

bool TaskGetStructuresForPurge::process(DB::Session *session)
{
	int rowsFetched;

	{
		GetStructuresForPurgeQuery qry(m_account);
		if (! (session->exec(&qry)))
			return false;

		do
		{
			rowsFetched = qry.fetch();
			if (rowsFetched > 0)
			{
				m_structures.push_back(std::make_pair(qry.getStructureId(),qry.getOwnerId()));
			}
		} while (rowsFetched > 0);
	
		qry.done();
		if (rowsFetched < 0)
			return false;
	}

	{
		GetVendorsForPurgeQuery qry(m_account);
		if (! (session->exec(&qry)))
			return false;

		do
		{
			rowsFetched = qry.fetch();
			if (rowsFetched > 0)
			{
				m_vendors.push_back(std::make_pair(qry.getVendorId(),std::make_pair(qry.getOwnerId(),qry.getVendorName())));
			}
		} while (rowsFetched > 0);
	
		qry.done();
		if (rowsFetched < 0)
			return false;
	}
	
	return true;
}

// ----------------------------------------------------------------------

void TaskGetStructuresForPurge::onComplete()
{
	StructuresForPurgeMessage msg(m_account, m_structures, m_vendors, m_warnOnly);
	DatabaseProcess::getInstance().sendToAnyGameServer(msg); // Any game server in the cluster can handle this.  It will send messages to other game servers as needed
}

// ======================================================================

GetStructuresForPurgeQuery::GetStructuresForPurgeQuery(StationId stationId) :
		DB::Query(),
		m_station_id(static_cast<long>(stationId))
{
}

// ----------------------------------------------------------------------

void GetStructuresForPurgeQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+DatabaseProcess::getInstance().getSchemaQualifier()+"datalookup.get_structures_for_purge(:station_id); end;";
}

// ----------------------------------------------------------------------

bool GetStructuresForPurgeQuery::bindParameters()
{
	if (!bindParameter(m_station_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool GetStructuresForPurgeQuery::bindColumns()
{
	if (!bindCol(m_structure_id)) return false;
	if (!bindCol(m_owner_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

GetStructuresForPurgeQuery::QueryMode GetStructuresForPurgeQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;   
}

// ----------------------------------------------------------------------

NetworkId GetStructuresForPurgeQuery::getStructureId() const
{
	return m_structure_id.getValue();
}

// ----------------------------------------------------------------------

NetworkId GetStructuresForPurgeQuery::getOwnerId() const
{
	return m_owner_id.getValue();
}

// ======================================================================

GetVendorsForPurgeQuery::GetVendorsForPurgeQuery(StationId stationId) :
		DB::Query(),
		m_station_id(stationId)
{
}

// ----------------------------------------------------------------------

void GetVendorsForPurgeQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+DatabaseProcess::getInstance().getSchemaQualifier()+"datalookup.get_vendors_for_purge(:station_id); end;";
}

// ----------------------------------------------------------------------

bool GetVendorsForPurgeQuery::bindParameters()
{
	if (!bindParameter(m_station_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool GetVendorsForPurgeQuery::bindColumns()
{
	if (!bindCol(m_vendor_id)) return false;
	if (!bindCol(m_owner_id)) return false;
	if (!bindCol(m_vendor_name)) return false;
	return true;
}

// ----------------------------------------------------------------------

GetVendorsForPurgeQuery::QueryMode GetVendorsForPurgeQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;   
}

// ----------------------------------------------------------------------

NetworkId GetVendorsForPurgeQuery::getVendorId() const
{
	return m_vendor_id.getValue();
}

// ----------------------------------------------------------------------

NetworkId GetVendorsForPurgeQuery::getOwnerId() const
{
	return m_owner_id.getValue();
}

// ----------------------------------------------------------------------

Unicode::String GetVendorsForPurgeQuery::getVendorName() const
{
	return m_vendor_name.getValue();
}

// ======================================================================
