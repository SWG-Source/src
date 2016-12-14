// PlayerQuestObject.cpp
// Copyright 2009, Sony Online Entertainment Inc., all rights reserved.
// Author: Matt Boudreaux

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlayerQuestObject.h"

#include "UnicodeUtils.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/GameServer.h"
#include "serverGame/NameManager.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/ServerPlayerQuestObjectTemplate.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"

//-----------------------------------------------------------------------

const SharedObjectTemplate * PlayerQuestObject::m_defaultSharedTemplate = nullptr;

namespace PlayerQuestObjectNamespace
{
	const char * const s_playerQuestObjectTemplateName = "object/player_quest/base_player_quest_object.iff";
	const char * const s_tasksObjVar = "pgc.tasks";
	const char * const s_numTasksObjVar = "pgc.numTasks";
	const char * const s_taskCounterObjVar = "pgc.taskCounters";
	const char * const s_taskMaxCounterObjVar = "pgc.taskMaxCounters";
	const char * const s_taskStatusObjVar = "pgc.taskStatus";
	const char * const s_waypointsObjVar = "pgc.waypoints";
	const char * const s_recipeObjVar = "pgc.recipe";
	const char * const s_rewardObjVar = "pgc.rewards";

	const int s_taskCompleted = 2;
}

using namespace PlayerQuestObjectNamespace;
//-----------------------------------------------------------------------

PlayerQuestObject::PlayerQuestObject(const ServerPlayerQuestObjectTemplate * t) :
TangibleObject(t),
m_tasks(),
m_taskCounters(),
m_taskStatus(),
m_waypoints(),
m_rewards(),
m_creatorName(),
m_completed(),
m_recipe(false),
m_title(),
m_description(),
m_creator(),
m_totalTasks(),
m_difficulty(),
m_taskTitle1(),
m_taskDescription1(),
m_taskTitle2(),
m_taskDescription2(),
m_taskTitle3(),
m_taskDescription3(),
m_taskTitle4(),
m_taskDescription4(),
m_taskTitle5(),
m_taskDescription5(),
m_taskTitle6(),
m_taskDescription6(),
m_taskTitle7(),
m_taskDescription7(),
m_taskTitle8(),
m_taskDescription8(),
m_taskTitle9(),
m_taskDescription9(),
m_taskTitle10(),
m_taskDescription10(),
m_taskTitle11(),
m_taskDescription11(),
m_taskTitle12(),
m_taskDescription12(),
m_taskTitles(),
m_taskDescriptions()
{
	addMembersToPackages();
	ObjectTracker::addPlayerQuestObject();

	m_taskTitles.push_back(&m_taskTitle1);
	m_taskTitles.push_back(&m_taskTitle2);
	m_taskTitles.push_back(&m_taskTitle3);
	m_taskTitles.push_back(&m_taskTitle4);
	m_taskTitles.push_back(&m_taskTitle5);
	m_taskTitles.push_back(&m_taskTitle6);
	m_taskTitles.push_back(&m_taskTitle7);
	m_taskTitles.push_back(&m_taskTitle8);
	m_taskTitles.push_back(&m_taskTitle9);
	m_taskTitles.push_back(&m_taskTitle10);
	m_taskTitles.push_back(&m_taskTitle11);
	m_taskTitles.push_back(&m_taskTitle12);

	m_taskDescriptions.push_back(&m_taskDescription1);
	m_taskDescriptions.push_back(&m_taskDescription2);
	m_taskDescriptions.push_back(&m_taskDescription3);
	m_taskDescriptions.push_back(&m_taskDescription4);
	m_taskDescriptions.push_back(&m_taskDescription5);
	m_taskDescriptions.push_back(&m_taskDescription6);
	m_taskDescriptions.push_back(&m_taskDescription7);
	m_taskDescriptions.push_back(&m_taskDescription8);
	m_taskDescriptions.push_back(&m_taskDescription9);
	m_taskDescriptions.push_back(&m_taskDescription10);
	m_taskDescriptions.push_back(&m_taskDescription11);
	m_taskDescriptions.push_back(&m_taskDescription12);


}

//-----------------------------------------------------------------------

PlayerQuestObject::~PlayerQuestObject()
{
	ObjectTracker::removePlayerQuestObject();
}

//-----------------------------------------------------------------------

bool PlayerQuestObject::canTrade() const
{
	return ServerObject::canTrade();
}

//-----------------------------------------------------------------------

PlayerQuestObject * PlayerQuestObject::asPlayerQuestObject()
{
	return this;
}

//-----------------------------------------------------------------------

PlayerQuestObject const * PlayerQuestObject::asPlayerQuestObject() const
{
	return this;
}

//-----------------------------------------------------------------------

void PlayerQuestObject::removeDefaultTemplate()
{
	if(m_defaultSharedTemplate)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}

//-----------------------------------------------------------------------

void PlayerQuestObject::endBaselines()
{
	ServerObject::endBaselines();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	TangibleObject::getAttributes(data);

	// Push on any extra data here.
}

//-----------------------------------------------------------------------

bool PlayerQuestObject::onContainerAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer)
{
	return ServerObject::onContainerAboutToLoseItem(destination, item, transferer);
}

//-----------------------------------------------------------------------

bool PlayerQuestObject::onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer)
{
	return ServerObject::onContainerAboutToTransfer(destination, transferer);
}

//-----------------------------------------------------------------------

int PlayerQuestObject::onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer)
{
	return ServerObject::onContainerAboutToGainItem(item, transferer);
}

//-----------------------------------------------------------------------

void PlayerQuestObject::onAddedToWorld()
{
	TangibleObject::onAddedToWorld();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::initializeFirstTimeObject()
{
	TangibleObject::initializeFirstTimeObject();
	saveDataToObjVars();
}

//-----------------------------------------------------------------------

SharedObjectTemplate const * PlayerQuestObject::getDefaultSharedTemplate() const
{
	return m_defaultSharedTemplate;
}

//-----------------------------------------------------------------------

PlayerQuestObject * PlayerQuestObject::createPlayerQuestObjectInCreatureDatapad(CreatureObject * creatureWithDatapad)
{
	PlayerQuestObject * result = 0;
	ServerObject * dataPad = creatureWithDatapad->getDatapad();
	if(dataPad)
		result = safe_cast<PlayerQuestObject *>(ServerWorld::createNewObject(s_playerQuestObjectTemplateName, *dataPad, false));

	return result;
}


//-----------------------------------------------------------------------

PlayerQuestObject * PlayerQuestObject::createPlayerQuestObjectInCreatureInventory(CreatureObject * creatureWithInventory)
{
	PlayerQuestObject * result = 0;
	ServerObject * inventory = creatureWithInventory->getInventory();
	if(inventory)
		result = safe_cast<PlayerQuestObject *>(ServerWorld::createNewObject(s_playerQuestObjectTemplateName, *inventory, false));

	return result;
}


//-----------------------------------------------------------------------

void PlayerQuestObject::getTasks( std::vector<Unicode::String> &taskVector)
{
	taskVector.clear();
	for(std::vector<Unicode::String>::size_type i = 0; i < m_tasks.get().size(); ++i)
		taskVector.push_back(m_tasks.get()[i]);
}

//-----------------------------------------------------------------------

void PlayerQuestObject::getTaskStatus(std::vector<int> &taskStatus)
{
	taskStatus.clear();
	for(std::vector<int>::size_type i = 0; i < m_taskStatus.get().size(); ++i)
		taskStatus.push_back(m_taskStatus.get()[i]);
}

//-----------------------------------------------------------------------

void PlayerQuestObject::getTaskCounters(std::vector<int> & counterVector)
{
	counterVector.clear();
	for(std::vector<int>::size_type i = 0; i < m_taskCounters.get().size(); ++i)
		counterVector.push_back(m_taskCounters.get()[i].first);

}

//-----------------------------------------------------------------------

void PlayerQuestObject::addNewTask(std::string title, std::string description, int counterMax, std::string waypoint)
{

	m_taskStatus.push_back(0);
	m_taskCounters.push_back(std::make_pair(0, counterMax));

	m_waypoints.push_back(waypoint);

	m_totalTasks = m_totalTasks.get() + 1;

	setTaskTitle(m_totalTasks.get() - 1, title);
	setTaskDescription(m_totalTasks.get() - 1, description);

	saveDataToObjVars();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::readInObjVarData()
{
	// Total Tasks
	int numTasks = 0;
	if(getObjVars().getItem(s_numTasksObjVar, numTasks))
		m_totalTasks = numTasks;

	// Task Information
	std::vector<Unicode::String> tasks;
	if(getObjVars().getItem(s_tasksObjVar, tasks))
	{
		for(std::vector<Unicode::String>::size_type i = 0; i < tasks.size(); ++i)
			m_tasks.push_back(tasks[i]);
	}

	// Task Counters
	std::vector<int> counters;
	if(getObjVars().getItem(s_taskCounterObjVar, counters))
	{
		std::vector<int> maxValues;
		if(getObjVars().getItem(s_taskMaxCounterObjVar, maxValues))
			for(std::vector<int>::size_type i = 0; i < counters.size(); ++i)
				m_taskCounters.push_back(std::make_pair(counters[i], maxValues[i]));
	}

	// Task Status
	std::vector<int> status;
	if(getObjVars().getItem(s_taskStatusObjVar, status))
	{
		for(std::vector<int>::size_type i = 0; i < status.size(); ++i)
			m_taskStatus.push_back(status[i]);
	}

	// Waypoints.
	std::vector<Unicode::String> waypoints;
	if(getObjVars().getItem(s_waypointsObjVar, waypoints))
	{
		for(std::vector<Unicode::String>::size_type i = 0; i < waypoints.size(); ++i)
			m_waypoints.push_back(Unicode::wideToNarrow(waypoints[i]));
	}

	// Recipe info
	int recipe;
	if(getObjVars().getItem(s_recipeObjVar, recipe))
	{
		m_recipe = (recipe == 1 ? true: false);
	}

	if(m_creator.get() != NetworkId::cms_invalid)
	{
		m_creatorName = NameManager::getInstance().getPlayerFullName(m_creator.get());
	}

	// Rewards
	std::string rewards;
	if(getObjVars().getItem(s_rewardObjVar, rewards))
	{
		m_rewards = rewards;
	}

	checkForCompleted();

	fixObjVarDelimiter();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::saveDataToObjVars()
{
	setObjVarItem(s_tasksObjVar, m_tasks.get());
	setObjVarItem(s_numTasksObjVar, m_totalTasks.get());
	setObjVarItem(s_taskStatusObjVar, m_taskStatus.get());

	// Save counter obj vars.
	std::vector<int> currentValues;
	std::vector<int> maxValues;
	std::vector< std::pair<int,int> >::const_iterator iter = m_taskCounters.begin();
	for(; iter != m_taskCounters.end(); ++iter)
	{
		currentValues.push_back((*iter).first);
		maxValues.push_back((*iter).second);
	}

	setObjVarItem(s_taskCounterObjVar, currentValues);
	setObjVarItem(s_taskMaxCounterObjVar, maxValues);

	//Waypoints
	std::vector<Unicode::String> convertedWaypoints;
	for(std::vector<std::string>::size_type i = 0; i < m_waypoints.size(); ++i)
	{
		convertedWaypoints.push_back(Unicode::narrowToWide(m_waypoints.get()[i]));
	}

	setObjVarItem(s_waypointsObjVar, convertedWaypoints);

	setObjVarItem(s_recipeObjVar, m_recipe.get() ? 1 : 0);

	setObjVarItem(s_rewardObjVar, m_rewards.get());
}

//-----------------------------------------------------------------------

void PlayerQuestObject::onLoadedFromDatabase()
{
	readInObjVarData();
}

//-----------------------------------------------------------------------

int PlayerQuestObject::getTaskStatus(int taskIndex)
{
	if(taskIndex < 0 || static_cast<unsigned int>(taskIndex) > m_taskStatus.size())
		return -1;

	return m_taskStatus.get()[taskIndex];
}

//-----------------------------------------------------------------------

void PlayerQuestObject::setTaskStatus(int taskIndex, int status)
{
	if(taskIndex < 0 || static_cast<unsigned int>(taskIndex) > m_taskStatus.size())
		return;

	m_taskStatus.set(taskIndex, status);

	saveDataToObjVars();

	checkForCompleted();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::setTaskTitle(int index, std::string title)
{
	if(index < 0 || index >= m_totalTasks.get())
		return;

	*(m_taskTitles[index]) = title;
}

//-----------------------------------------------------------------------

std::string const & PlayerQuestObject::getTaskTitle(int index)
{
	return m_taskTitles[index]->get();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::setTaskDescription(int index, std::string description)
{
	if(index < 0 || index >= m_totalTasks.get())
		return;

	*(m_taskDescriptions[index]) = description;
}

//-----------------------------------------------------------------------

std::string const & PlayerQuestObject::getTaskDescription(int index)
{
	return m_taskDescriptions[index]->get();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::setTaskCounter(int index, int value)
{
	int maxValue = m_taskCounters.get()[index].second;
	m_taskCounters.set(static_cast<unsigned int>(index), std::make_pair(value, maxValue));

	saveDataToObjVars();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::checkForCompleted()
{
	std::vector<int>::size_type i = 0;
	for(; i < m_taskStatus.size(); ++i)
	{
		if(m_taskStatus.get()[i] != s_taskCompleted)
		{
			m_completed = false;
			return;
		}
	}

	m_completed = true;
}

//-----------------------------------------------------------------------

void PlayerQuestObject::setRecipe(bool recipe)
{
	m_recipe = recipe;
	saveDataToObjVars();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::addRecipeTaskData(std::string data)
{
	m_tasks.push_back(Unicode::narrowToWide(data));

	saveDataToObjVars();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::setRecipeTaskData(int index, std::string data)
{
	m_tasks.set(static_cast<unsigned int>(index), Unicode::narrowToWide(data));

	saveDataToObjVars();
}

//-----------------------------------------------------------------------


std::string const PlayerQuestObject::getRecipeTaskData(int index)
{
	return Unicode::wideToNarrow(m_tasks.get()[index]);
}

//-----------------------------------------------------------------------

void PlayerQuestObject::setRewards(std::string data)
{
	m_rewards = data;
	saveDataToObjVars();
}

//-----------------------------------------------------------------------

std::string const & PlayerQuestObject::getRewards()
{
	return m_rewards.get();
}

//-----------------------------------------------------------------------

void PlayerQuestObject::setCreator(NetworkId const & creator)
{
	m_creator = creator;

	if(m_creator.get() != NetworkId::cms_invalid)
	{
		m_creatorName = NameManager::getInstance().getPlayerFullName(m_creator.get());
	}
}

void PlayerQuestObject::resetAllData()
{
	for(unsigned int i = 0; i < m_taskTitles.size(); ++i)
	{
		*(m_taskTitles[i]) = "";
	}

	for(unsigned int i = 0; i < m_taskDescriptions.size(); ++i)
	{
		*(m_taskDescriptions[i]) = "";
	}

	m_title = "";
	m_description = "";

	m_tasks.clear();
	m_totalTasks = 0;
	m_difficulty = 0;
	m_taskStatus.clear();
	m_taskCounters.clear();
	m_waypoints.clear();
	m_recipe = 0;
	m_rewards = "";
	m_completed = false;
	m_creator = NetworkId::cms_invalid;
	m_creatorName = "";

	saveDataToObjVars();
}

void PlayerQuestObject::getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const
{
	if(name == "PGCPacked")
	{
		// title
		m_title.pack(target);

		// description
		m_description.pack(target);

		// creator
		m_creator.pack(target);

		// total tasks
		m_totalTasks.pack(target);

		// difficulty
		m_difficulty.pack(target);

		// All our task titles/descriptions
		for(int i = 0; i < m_totalTasks.get(); ++i)
		{
			m_taskTitles[i]->pack(target);
			m_taskDescriptions[i]->pack(target);
		}

		// Everything else should be stored as objvars.
	}
	else
	{
		TangibleObject::getByteStreamFromAutoVariable(name, target);
	}
}

void PlayerQuestObject::setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source)
{
	Archive::ReadIterator ri(source);
	if(name == "PGCPacked")
	{
		// title
		m_title.unpackDelta(ri);

		// description
		m_description.unpackDelta(ri);

		// creator
		m_creator.unpackDelta(ri);

		// total tasks
		m_totalTasks.unpackDelta(ri);

		// difficulty
		m_difficulty.unpackDelta(ri);

		// All our task titles/descriptions
		for(int i = 0; i < m_totalTasks.get(); ++i)
		{
			m_taskTitles[i]->unpackDelta(ri);
			m_taskDescriptions[i]->unpackDelta(ri);
		}

		// Make sure our other auto-deltas are up to date.
		readInObjVarData();
	}
	else
	{
		TangibleObject::setAutoVariableFromByteStream(name, source);
	}

	addObjectToConcludeList();
}

void PlayerQuestObject::fixObjVarDelimiter()
{
	// This should only affect Recipe/Task Data and Rewards string
	bool saveNeeded = false;
	for(unsigned int i = 0; i < m_tasks.size(); ++i)
	{
		std::string narrowString = Unicode::wideToNarrow(m_tasks[i]);

		std::string::size_type iter = narrowString.find('|');
		while(iter != std::string::npos)
		{
			narrowString.replace(iter, iter, 1, '~');
			saveNeeded = true;
			iter = narrowString.find('|', iter + 1);
		}

		if(saveNeeded)
			m_tasks.set(i,Unicode::narrowToWide(narrowString));
	}

	std::string rewards = m_rewards.get();
	std::string::size_type iter = rewards.find('|');
	while(iter != std::string::npos)
	{
		rewards.replace(iter, iter, 1, '~');
		saveNeeded = true;
		iter = rewards.find('|', iter + 1);
	}

	if(saveNeeded)
	{
		m_rewards = rewards;
		saveDataToObjVars();
	}
}
