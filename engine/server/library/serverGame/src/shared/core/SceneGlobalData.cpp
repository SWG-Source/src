// ======================================================================
//
// SceneGlobalData.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SceneGlobalData.h"

#include "serverGame/ConfigServerGame.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/Log.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <algorithm>

// ======================================================================

namespace SceneGlobalDataNamespace
{
	std::string const ms_SceneGlobalDataTableName("datatables/planet_server/sceneglobaldata.iff");

	bool ms_installed = false;

	class SceneData 
	{
	public:
		SceneData(DataTable const & dataTable, int row);

		std::string const & getId() const;
		float getUpdateRadius() const;
		
	private:
		std::string const m_id;
		float const m_updateradius;

	private:  //disable:
		SceneData();  
		SceneData(SceneData const &);
		SceneData & operator=(SceneData const &);
	};

	typedef std::map<std::string, SceneData *> SceneDataType;
	
	SceneDataType ms_SceneDataItems;

	SceneData  * getSceneDataByName(std::string const & sceneName);
	
	std::vector<std::string> buildVectorFromString(std::string packedString);
}

using namespace SceneGlobalDataNamespace;

// ======================================================================

void SceneGlobalData::install()
{
	DEBUG_FATAL(ms_installed,("Programmer bug:  SceneGlobalData::install() called twice"));
	ms_installed = true;
	ExitChain::add(SceneGlobalData::remove, "SceneGlobalData::remove");

	if (ConfigServerGame::getEnableSceneGlobalData())
	{
		DataTable * data = NON_NULL(DataTableManager::getTable(ms_SceneGlobalDataTableName, true));
		int numRows = data->getNumRows();
		{
			for (int row=0; row<numRows; ++row)
			{
				SceneData * newScene = new SceneData(*data,row);		
				std::pair<SceneDataType::iterator, bool> result = ms_SceneDataItems.insert(std::make_pair(newScene->getId(), newScene));
				WARNING_DEBUG_FATAL(!result.second,("Data bug:  Scene %s was specified more than once in data table %s",newScene->getId().c_str(), ms_SceneGlobalDataTableName.c_str()));		
			}
		}

	}
}

// ----------------------------------------------------------------------

void SceneGlobalData::remove()
{
	for (SceneDataType::iterator i=ms_SceneDataItems.begin(); i!=ms_SceneDataItems.end(); ++i)
		delete i->second;

	ms_SceneDataItems.clear();
	
	DEBUG_FATAL(!ms_installed,("Programmer bug:  SceneGlobalData was not installed"));
	ms_installed = false;
}

// ----------------------------------------------------------------------

float SceneGlobalData::getUpdateRadius(std::string const & sceneName)
{
	SceneData * Scene = SceneGlobalDataNamespace::getSceneDataByName(sceneName);

	if (Scene) 
	{
		return Scene->getUpdateRadius();
	}
	else
	{
		return 0.0f;
	}	
}

// ----------------------------------------------------------------------

SceneData * SceneGlobalDataNamespace::getSceneDataByName(std::string const & sceneName)
{
	SceneDataType::iterator i=ms_SceneDataItems.find(sceneName);
	if (i!=ms_SceneDataItems.end())
		return i->second;
	else
		return nullptr;
}

// ======================================================================

SceneData::SceneData(DataTable const & dataTable, int row) :
		m_id(dataTable.getStringValue("ID",row)),
		m_updateradius(dataTable.getFloatValue("UpdateRadius",row))
{
	DEBUG_REPORT_LOG(true,("[SceneGlobalData] : Initialized global scene info with: %s, %f\n ",m_id.c_str(), m_updateradius));
}

// ----------------------------------------------------------------------

std::string const & SceneData::getId() const
{
	return m_id;
}

// ----------------------------------------------------------------------

float SceneData::getUpdateRadius() const
{
	return m_updateradius;
}

// ----------------------------------------------------------------------

// ======================================================================
