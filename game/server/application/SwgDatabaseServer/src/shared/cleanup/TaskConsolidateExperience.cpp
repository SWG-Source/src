// ======================================================================
//
// TaskConsolidateExperience.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/TaskConsolidateExperience.h"

#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/NetworkId.h"
#include <map>
#include <string>
#include <vector>

// ======================================================================

TaskConsolidateExperience::TaskConsolidateExperience(float timeLimit) :
		m_timeLimit(timeLimit)
{
}

// ----------------------------------------------------------------------

TaskConsolidateExperience::~TaskConsolidateExperience()
{
}

// ----------------------------------------------------------------------

bool TaskConsolidateExperience::process(DB::Session *session)
{
	int rowsFetched = 0;
	NOT_NULL(session);

	session->setAutoCommitMode(false);

	PerformanceTimer timer;
	timer.start();
		
	std::vector<NetworkId> objectQueue;
	{
		// Make an ordered list of objects with lots of messages pending
		FindLotsOfMessagesQuery qry;
		if (! (session->exec(&qry)))
			return false;
		while ((rowsFetched = qry.fetch()) > 0)
		{
			objectQueue.push_back(qry.getNetworkId());
		}
	}
	if (rowsFetched < 0)
		return false;

	for (std::vector<NetworkId>::const_iterator objectToClean=objectQueue.begin(); objectToClean!=objectQueue.end() && timer.getSplitTime() < m_timeLimit; ++objectToClean)
	{
		bool bail=false;
		std::map<std::string, int> experiencePoints;
		{
			// Total up the experience grants for the object
			GetExperienceQuery qry(*objectToClean);
			if (! (session->exec(&qry)))
				return false;
			while ((rowsFetched = qry.fetch()) > 0 && !bail)
			{
				std::string type;
				int amount;
				if (splitExperience(qry.getData(), type, amount))
				{
					experiencePoints[type]+=amount;
				}
				else
				{
					WARNING(true,("Unrecognized experience string \"%s\" for object %s.  Bailing out of experience consolidation for this object.\n",
								  qry.getData().c_str(), objectToClean->getValueString().c_str()));
					bail=true;
				}
			}
		}
		if (rowsFetched < 0)
			return false;

		if (!bail)
		{
			{
				// delete all xp grants
				DeleteExperienceQuery qry(*objectToClean);
				if (! (session->exec(&qry)))
					return false;
			}

			{
				// replace with simplified grants
				GrantExperienceQuery qry;

				for (std::map<std::string, int>::const_iterator i=experiencePoints.begin(); i!=experiencePoints.end(); ++i)
				{
					qry.setData(*objectToClean, makeExperienceString(i->first, i->second));
					if (! (session->exec(&qry)))
						return false;
				}
			}

			session->commitTransaction();
		}
		bail=false;
	}
	return true;
}

// ----------------------------------------------------------------------

void TaskConsolidateExperience::onComplete()
{
}

// ----------------------------------------------------------------------

/**
 * Convert packed experience data into the type name and the amount.
 */
bool TaskConsolidateExperience::splitExperience(const std::string &packedData, std::string &type, int &amount) const
{
	size_t spacePos=packedData.find(' ');
	if (spacePos==std::string::npos) return false;
	type=std::string(packedData,0,spacePos);
	std::string amountString(packedData,spacePos+1,static_cast<unsigned int>(std::string::npos));
	amount=atoi(amountString.c_str());
	if (amount <= 0) return false; // string is probably badly-formed or parsed wrong
	return true;
}

// ----------------------------------------------------------------------

std::string TaskConsolidateExperience::makeExperienceString(const std::string &type, int amount)
{
	char buffer[1000];
	snprintf(buffer,1000,"%s %i",type.c_str(), amount);
	return std::string(buffer);
}

// ======================================================================

TaskConsolidateExperience::FindLotsOfMessagesQuery::FindLotsOfMessagesQuery()
{
}

// ----------------------------------------------------------------------
		
void TaskConsolidateExperience::FindLotsOfMessagesQuery::getSQL(std::string &sql)
{
	sql="begin :rc := "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.getObjectsWithExpMessages(); end;";
}

// ----------------------------------------------------------------------

bool TaskConsolidateExperience::FindLotsOfMessagesQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool TaskConsolidateExperience::FindLotsOfMessagesQuery::bindColumns()
{
	if (!bindCol(object_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskConsolidateExperience::FindLotsOfMessagesQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

NetworkId TaskConsolidateExperience::FindLotsOfMessagesQuery::getNetworkId() const
{
	return object_id.getValue();
}

// ======================================================================

TaskConsolidateExperience::GetExperienceQuery::GetExperienceQuery(const NetworkId &objectId) :
		object_id(objectId)
{
}

// ----------------------------------------------------------------------
		
void TaskConsolidateExperience::GetExperienceQuery::getSQL(std::string &sql)
{
	sql="begin :rc := "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.getExperienceGrants(:object_id); end;";
}

// ----------------------------------------------------------------------

bool TaskConsolidateExperience::GetExperienceQuery::bindParameters()
{
	if (!bindParameter(object_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskConsolidateExperience::GetExperienceQuery::bindColumns()
{
	if (!bindCol(grant_data)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskConsolidateExperience::GetExperienceQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

const std::string TaskConsolidateExperience::GetExperienceQuery::getData() const
{
	return grant_data.getValueASCII();
}

// ======================================================================

TaskConsolidateExperience::DeleteExperienceQuery::DeleteExperienceQuery(const NetworkId &objectId) :
		object_id(objectId)
{
}

// ----------------------------------------------------------------------
		
void TaskConsolidateExperience::DeleteExperienceQuery::getSQL(std::string &sql)
{
	sql="begin "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.delete_experience(:object_id); end;";
}

// ----------------------------------------------------------------------

bool TaskConsolidateExperience::DeleteExperienceQuery::bindParameters()
{
	if (!bindParameter(object_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskConsolidateExperience::DeleteExperienceQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskConsolidateExperience::DeleteExperienceQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================

TaskConsolidateExperience::GrantExperienceQuery::GrantExperienceQuery()
{
}

// ----------------------------------------------------------------------

void TaskConsolidateExperience::GrantExperienceQuery::getSQL(std::string &sql)
{
	sql="begin "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.grant_experience(:object_id, :experience_data); end;";
}

// ----------------------------------------------------------------------

bool TaskConsolidateExperience::GrantExperienceQuery::bindParameters()
{
	if (!bindParameter(object_id)) return false;
	if (!bindParameter(experience_data)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskConsolidateExperience::GrantExperienceQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskConsolidateExperience::GrantExperienceQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

void TaskConsolidateExperience::GrantExperienceQuery::setData(const NetworkId &objectId, const std::string &data)
{
	object_id=objectId;
	experience_data=data;
}

// ======================================================================
