// ======================================================================
//
// SetupServerGame.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SetupServerGame.h"

#include "serverGame/AiServerShipObjectInterface.h"
#include "serverGame/AttribModNameManager.h"
#include "serverGame/AuthTransferTracker.h"
#include "serverGame/BiographyManager.h"
#include "serverGame/BuffManager.h"
#include "serverGame/CombatTracker.h"
#include "serverGame/CommandCppFuncs.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/InstantDeleteList.h"
#include "serverGame/LogoutTracker.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverGame/NonCriticalTaskQueue.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlayerCreationManagerServer.h"
#include "serverGame/PositionUpdateTracker.h"
#include "serverGame/ServerBuffBuilderManager.h"
#include "serverGame/ServerBuildoutManager.h"
#include "serverGame/ServerImageDesignerManager.h"
#include "serverGame/ServerSecureTradeManager.h"
#include "serverGame/ServerUIManager.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipInternalDamageOverTimeManager.h"
#include "serverGame/SpacePathManager.h"
#include "serverGame/SpaceSquad.h"
#include "serverGame/SpaceSquadManager.h"
#include "serverGame/SurveySystem.h"
#include "serverScript/GameScriptObject.h"
#include "sharedSkillSystem/ExpertiseManager.h"
#include "sharedSkillSystem/LevelManager.h"
#include "serverUtility/AdminAccountManager.h"

// ======================================================================

void SetupServerGame::install()
{
	ConfigServerGame::install();

	// AdminAccountManager::install doesn't belong here it belongs
	// in ServerUtility but uses data from the ConfigServerGame
	AdminAccountManager::install(ConfigServerGame::getAdminAccountDataTable());

	BiographyManager::install();
	BuffManager::install();
	CommandCppFuncs::install();
	CombatTracker::install();
	AttribModNameManager::install();
	ServerWorld::install();
	IGNORE_RETURN(GameScriptObject::installScriptEngine());
	ConsoleMgr::install();
	ObserveTracker::install();
	PositionUpdateTracker::install();
	LogoutTracker::install();
	AuthTransferTracker::install();
	NonCriticalTaskQueue::install(static_cast<NonCriticalTaskQueue*>(nullptr));
	SurveySystem::install(static_cast<SurveySystem*>(nullptr));
	CreatureObject::install();
	NameManager::install();
	MessageToQueue::install();
	ServerUIManager::install();
	ServerImageDesignerManager::install();
	ServerBuffBuilderManager::install();
	InstantDeleteList::install();
	PlayerCreationManagerServer::install();
	DraftSchematicObject::install();
	ServerSecureTradeManager::install();
	ShipInternalDamageOverTimeManager::install();
	SpaceSquad::install();
	SpaceSquadManager::install();
	SpacePathManager::install();
	AiServerShipObjectInterface::install();
	ServerBuildoutManager::install();
	ExpertiseManager::install();
	LevelManager::install();
}

// ----------------------------------------------------------------------

void SetupServerGame::remove()
{
	ServerSecureTradeManager::remove();
	DraftSchematicObject::remove();
	PlayerCreationManagerServer::remove();
	ServerUIManager::remove();
	MessageToQueue::remove();
	PositionUpdateTracker::remove();
	ObserveTracker::remove();
	LogoutTracker::remove();
	AuthTransferTracker::remove();
	ConsoleMgr::remove();
	GameScriptObject::removeScriptEngine();
	ServerWorld::remove();
	CombatTracker::remove();
	CommandCppFuncs::remove();
	BuffManager::remove();
	BiographyManager::remove();
	GameServer::remove();

	// the following are all removed by ExitChain
	// InstantDeleteList
	// KnowledgeBaseManager
	// NameManager
	// CreatureObject
	// SurveySystem
	// NonCriticalTaskQueue
	// AttribModNameManager
	// ShipInternalDamageOverTimeManager
	// ServerBuildoutManager

}

// ======================================================================
