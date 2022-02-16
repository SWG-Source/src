// ======================================================================
//
// OfflineMoneyCustomPersistStep.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/OfflineMoneyCustomPersistStep.h"

#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/GetMoneyFromOfflineObjectMessage.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/NetworkId.h"
#include <string>
#include <vector>

// ======================================================================

namespace OfflineMoneyCustomPersistStepNamespace
{
	struct GetMoneyData
	{
		uint32 m_replyServer;
		NetworkId m_sourceObject;
		int m_amount;
		NetworkId m_replyTo;
		std::string m_successCallback;
		std::string m_failCallback;
		std::vector<int8> m_packedDictionary;
		bool m_success;
	};

	class MoneyGrabQuery: public DB::Query
	{
	public:
		MoneyGrabQuery();

		void setSource(NetworkId const & sourceObject, int amount);
		bool getResult() const;

		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
		QueryMode getExecutionMode() const;

	private:
		DB::BindableNetworkId m_sourceObject;
		DB::BindableLong m_amount;
		DB::BindableBool m_result;

	private:
		MoneyGrabQuery(const MoneyGrabQuery&); // disable
		MoneyGrabQuery& operator=(const MoneyGrabQuery&); // disable
	};
}

using namespace OfflineMoneyCustomPersistStepNamespace;

// ======================================================================

OfflineMoneyCustomPersistStep::OfflineMoneyCustomPersistStep() :
		m_getMoneyRequests(new std::vector<OfflineMoneyCustomPersistStepNamespace::GetMoneyData*>)
{
}

// ----------------------------------------------------------------------

OfflineMoneyCustomPersistStep::~OfflineMoneyCustomPersistStep()
{
	for (GetMoneyRequestsType::const_iterator i=m_getMoneyRequests->begin(); i!=m_getMoneyRequests->end(); ++i)
		delete *i;

	delete m_getMoneyRequests;
}

// ----------------------------------------------------------------------

void OfflineMoneyCustomPersistStep::getMoneyFromOfflineObject(uint32 replyServer, NetworkId const & sourceObject, int amount, NetworkId const & replyTo, std::string const & successCallback, std::string const & failCallback, std::vector<int8> const & packedDictionary)
{
	GetMoneyData * const data=new GetMoneyData;
	data->m_replyServer=replyServer;
	data->m_sourceObject=sourceObject;
	data->m_amount=amount;
	data->m_replyTo=replyTo;
	data->m_successCallback=successCallback;
	data->m_failCallback=failCallback;
	data->m_packedDictionary=packedDictionary;
	data->m_success=false;

	m_getMoneyRequests->push_back(data);
}

// ----------------------------------------------------------------------

bool OfflineMoneyCustomPersistStep::beforePersist (DB::Session * /*session*/)
{
	return true;
}

// ----------------------------------------------------------------------

bool OfflineMoneyCustomPersistStep::afterPersist  (DB::Session *session)
{
	if (m_getMoneyRequests->empty())
		return true;

	MoneyGrabQuery qry;
	for (GetMoneyRequestsType::const_iterator i=m_getMoneyRequests->begin(); i!=m_getMoneyRequests->end(); ++i)
	{
		qry.setSource((*i)->m_sourceObject,(*i)->m_amount);
		if (!session->exec(&qry))
		{
			return false;
		}
		(*i)->m_success=qry.getResult();
	}
	qry.done();

	return true;
}

// ----------------------------------------------------------------------

void OfflineMoneyCustomPersistStep::onComplete()
{
	for (GetMoneyRequestsType::const_iterator i=m_getMoneyRequests->begin(); i!=m_getMoneyRequests->end(); ++i)
	{
		GetMoneyFromOfflineObjectMessage replyMessage((*i)->m_sourceObject, (*i)->m_amount, (*i)->m_replyTo, (*i)->m_successCallback, (*i)->m_failCallback, (*i)->m_packedDictionary,(*i)->m_success);
		DatabaseProcess::getInstance().sendToGameServer((*i)->m_replyServer, replyMessage);
	}
}

// ======================================================================

MoneyGrabQuery::MoneyGrabQuery() :
		DB::Query(),
		m_sourceObject(),
		m_amount(),
		m_result()
{
}

// ----------------------------------------------------------------------

void MoneyGrabQuery::setSource(NetworkId const & sourceObject, int amount)
{
	m_sourceObject=sourceObject;
	m_amount=amount;
}

// ----------------------------------------------------------------------

bool MoneyGrabQuery::getResult() const
{
	return m_result.getValue();
}

// ----------------------------------------------------------------------

bool MoneyGrabQuery::bindParameters()
{
	if (!bindParameter(m_result)) return false;
	if (!bindParameter(m_sourceObject)) return false;
	if (!bindParameter(m_amount)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool MoneyGrabQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void MoneyGrabQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.subtract_money_from_object(:object_id, :amount); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode MoneyGrabQuery::getExecutionMode() const
{
	return (MODE_PROCEXEC);
}

// ======================================================================
