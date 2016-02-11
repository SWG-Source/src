// ======================================================================
//
// SurveySystem.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SurveySystem.h"

#include "UnicodeUtils.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ResourcePoolObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/ExitChain.h"
#include "swgSharedNetworkMessages/ResourceListForSurveyMessage.h"
#include "swgSharedNetworkMessages/SurveyMessage.h"

// ======================================================================

SurveySystem::SurveySystem() :
		Singleton2<SurveySystem>()
{
	ExitChain::add(&remove, "SurveySystem::remove");
}

// ----------------------------------------------------------------------

SurveySystem::~SurveySystem()
{
}

// ----------------------------------------------------------------------

void SurveySystem::requestResourceListForSurvey(const NetworkId &playerId, const NetworkId &surveyTool, const std::string &parentResourceClassName) const
{
	NonCriticalTaskQueue::getInstance().addTask(new TaskGetResourceList(playerId, surveyTool, parentResourceClassName));
}

// ----------------------------------------------------------------------

void SurveySystem::requestSurvey(const NetworkId &playerId, const std::string &parentResourceClassName, const std::string &resourceTypeName, const Vector &location, int surveyRange, int numPoints) const
{
	NonCriticalTaskQueue::getInstance().addTask(new TaskSurvey(playerId, parentResourceClassName, resourceTypeName, location, surveyRange, numPoints));
}

// ======================================================================

SurveySystem::TaskGetResourceList::TaskGetResourceList(const NetworkId &playerId, const NetworkId& surveyTool, const std::string &parentResourceClassName) :
		m_playerId(playerId),
		m_surveyTool(surveyTool),
		m_parentResourceClassName(new std::string(parentResourceClassName))
{
}

// ----------------------------------------------------------------------

SurveySystem::TaskGetResourceList::~TaskGetResourceList()
{
	delete m_parentResourceClassName;
	m_parentResourceClassName = 0;
}

// ----------------------------------------------------------------------

bool SurveySystem::TaskGetResourceList::run()
{
	Client *client = GameServer::getInstance().getClient(m_playerId);
	const ResourceClassObject *masterClass = ServerUniverse::getInstance().getResourceClassByName(*m_parentResourceClassName);
	if (client && masterClass)
	{
		std::vector<ResourceTypeObject const *> results;
		std::vector<ResourceListForSurveyMessage::DataItem> sendableResults;
		ServerUniverse::getInstance().getCurrentPlanet()->getAvailableResourceList(results,*masterClass);
		for (std::vector<ResourceTypeObject const *>::const_iterator i=results.begin(); i!=results.end(); ++i)
		{
			ResourceTypeObject const * const rto = *i;
			NOT_NULL(rto);
			if (rto)
			{
				ResourceListForSurveyMessage::DataItem d;
				d.resourceName    = rto->getResourceName();
				d.resourceId      = rto->getNetworkId();
				d.parentClassName = rto->getParentClass().getResourceClassName();
				
				sendableResults.push_back(d);
			}
		}
		ResourceListForSurveyMessage msg(sendableResults, masterClass->getResourceClassName(), m_surveyTool);
		client->send(msg,true);
	}
	else
	{
		// TODO:  request isn't valid -- what do we do?
	}

	return true;
}

// ======================================================================

SurveySystem::TaskSurvey::TaskSurvey(const NetworkId &playerId, const std::string &parentResourceClassName, const std::string &resourceTypeName, const Vector &location, int surveyRange, int numPoints) :
		m_playerId                (playerId),
		m_parentResourceClassName (new std::string(parentResourceClassName)),
		m_resourceTypeName        (new std::string(resourceTypeName)),
		m_location                (location),
		m_surveyRange             (surveyRange),
		m_numPoints               (numPoints)
{
	DEBUG_WARNING((m_numPoints/2)*2 == m_numPoints,("The number of points on a side should be an odd number when taking a survey, to guarantee that the point the player is standing on is included in the survey.\n"));
}

// ----------------------------------------------------------------------

SurveySystem::TaskSurvey::~TaskSurvey()
{
	delete m_parentResourceClassName; //lint !e605 // deleting const pointer
	delete m_resourceTypeName;        //lint !e605 // deleting const pointer
	m_parentResourceClassName = 0;
	m_resourceTypeName = 0;
}

// ----------------------------------------------------------------------

bool SurveySystem::TaskSurvey::run()
{
	Client const *              client            = GameServer::getInstance().getClient(m_playerId);
	ResourceTypeObject const *  typeObj           = ServerUniverse::getInstance().getResourceTypeByName(*m_resourceTypeName);
	ResourceClassObject const * parentClass       = ServerUniverse::getInstance().getResourceClassByName(*m_parentResourceClassName);
	ResourcePoolObject const *  pool              = typeObj ? typeObj->getPoolForCurrentPlanet() : nullptr;
	int                         distBetweenPoints = m_surveyRange / (m_numPoints - 1); // -1 is so that we get points at both ends
	int                         radius            = m_surveyRange / 2;
	
	if (client && typeObj && parentClass && pool && (typeObj->isDerivedFrom(*parentClass)))
	{
		std::vector<SurveyMessage::DataItem> surveyData;
		SurveyMessage::DataItem item;

		//vectors to store the data that goes to the script trigger
		std::vector<float>   xVals;
		std::vector<float>   zVals;
		std::vector<float>   efficiencyVals;
		
		for (item.m_location.x=m_location.x-radius; item.m_location.x<=m_location.x+radius; item.m_location.x+=distBetweenPoints)
			for (item.m_location.z=m_location.z-radius; item.m_location.z<=m_location.z+radius; item.m_location.z+=distBetweenPoints)
			{
				item.m_efficiency = pool->getEfficiencyAtLocation(item.m_location.x, item.m_location.z);
				surveyData.push_back(item);
				DEBUG_REPORT_LOG(true,("Adding data item (%f,%f,%f) -- %f\n",item.m_location.x, item.m_location.y, item.m_location.z, item.m_efficiency));
				xVals.push_back(item.m_location.x);
				zVals.push_back(item.m_location.z);
				efficiencyVals.push_back(item.m_efficiency);
			}

		//send the client the response data
		SurveyMessage msg(surveyData);
		client->send(msg,true);

		//send the data to script trigger
		ScriptParams params;
		params.addParam(xVals);
		params.addParam(zVals);
		params.addParam(efficiencyVals);
		ServerObject* so = ServerWorld::findObjectByNetworkId(m_playerId);
		if (so)
			IGNORE_RETURN(so->getScriptObject()->trigAllScripts(Scripting::TRIG_SURVEY_DATA_RECEIVED, params));
	}
	else
	{
		// TODO: Survey is not valid -- what to do?
	}
	return true;
}

// ======================================================================
