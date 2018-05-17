// ======================================================================
//
// CharacterCreationTracker.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstCentralServer.h"
#include "CharacterCreationTracker.h"

#include "ConfigCentralServer.h"
#include "UnicodeUtils.h"
#include "serverNetworkMessages/CentralConnectionServerMessages.h"
#include "serverNetworkMessages/LoginCreateCharacterAckMessage.h"
#include "serverNetworkMessages/LoginCreateCharacterMessage.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/NameErrors.h"
#include "sharedNetworkMessages/NetworkStringIds.h"
#include "sharedUtility/StartingLocationData.h"
#include "sharedUtility/StartingLocationManager.h"

// ======================================================================

CharacterCreationTracker * CharacterCreationTracker::ms_instance = nullptr;

// ======================================================================

void CharacterCreationTracker::install()
{
	DEBUG_FATAL(ms_instance != nullptr,("Called install() twice.\n"));
	ms_instance = new CharacterCreationTracker;
	ExitChain::add(CharacterCreationTracker::remove,"CharacterCreationTracker::remove");
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::remove()
{
	DEBUG_FATAL(!ms_instance,("Not installed.\n"));
	delete ms_instance;
	ms_instance = 0;
}

// ----------------------------------------------------------------------

CharacterCreationTracker::CharacterCreationTracker() :
	m_creations()
{
}

// ----------------------------------------------------------------------

CharacterCreationTracker::~CharacterCreationTracker()
{
	for (CreationsType::iterator i = m_creations.begin(); i != m_creations.end(); ++i)
		delete i->second;
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::handleCreateNewCharacter(const ConnectionCreateCharacter &msg)
{
	// - Check whether a creation request is already active for this account
	CreationsType::iterator creationRecord = m_creations.find(msg.getStationId());
	if (creationRecord != m_creations.end())
	{
		if (ServerClock::getInstance().getGameTimeSeconds() > (creationRecord->second->m_creationTime + ConfigCentralServer::getCharacterCreationTimeout()))
		{
			LOG("TraceCharacterCreation", ("%d allowing character creation because previous one timed out", msg.getStationId()));
			DEBUG_REPORT_LOG(true,("Allowing character creation for account %li because previous one timed out.\n",msg.getStationId()));
			unlockAccount(msg.getStationId());
			creationRecord = m_creations.end();
		}
		else
		{
			LOG("TraceCharacterCreation", ("%d refusing character creation because one is already in progress", msg.getStationId()));
			DEBUG_REPORT_LOG(true,("Refusing character creation for account %li because one was already in progress.\n",msg.getStationId()));
			ConnectionCreateCharacterFailed f(msg.getStationId(), msg.getCharacterName(), NameErrors::nameDeclinedRetry, FormattedString<2048>().sprintf("%lu refusing character creation because one is already in progress", msg.getStationId())); //lint !e40 // undeclared identifier nameDeclinedEntry
			CentralServer::getInstance().sendToConnectionServerForAccount(msg.getStationId(), f, true);
			return;
		}
	}

	// - Check whether they are creating characters too rapidly
	// Note that this locks only when creation succeeds.  Failing to create a lot of characters in a row does not lock the account.
	FastCreationLockType::iterator fcl = m_fastCreationLock.find(msg.getStationId());
	if (fcl != m_fastCreationLock.end())
	{
		if (!msg.getNoRateLimit() && ((Clock::timeSeconds() - fcl->second) < (msg.getIsForCharacterTransfer() ? static_cast<uint32>(ConfigCentralServer::getCharacterCtsCreationRateLimitSeconds()) : static_cast<uint32>(ConfigCentralServer::getCharacterCreationRateLimitSeconds()))))
		{
			LOG("TraceCharacterCreation", ("%d refusing character creation because not enough time has passed since the previous one", msg.getStationId()));
			DEBUG_REPORT_LOG(true,("Refusing character creation for account %li because not enough time has passed since the previous one\n",msg.getStationId()));
			ConnectionCreateCharacterFailed f(msg.getStationId(), msg.getCharacterName(), NameErrors::nameDeclinedTooFast, FormattedString<2048>().sprintf("%lu refusing character creation because not enough time has passed since the previous one", msg.getStationId()));
			CentralServer::getInstance().sendToConnectionServerForAccount(msg.getStationId(), f, true);
			return;
		}
	}
	
	if (creationRecord == m_creations.end())
		creationRecord = m_creations.insert(std::pair<StationId,CreationRecord*>(msg.getStationId(),new CreationRecord)).first;

	// - determine starting location
	static std::string tutorialPlanetName("tutorial");
	std::string planetName;
	Vector coordinates(0.0f,0.0f,0.0f);
	NetworkId cellId(NetworkId::cms_invalid);
	bool useNewbieTutorial = (ConfigCentralServer::getNewbieTutorialEnabled() && msg.getUseNewbieTutorial() && ! msg.getIsForCharacterTransfer());
	if (useNewbieTutorial)
		planetName = tutorialPlanetName;
	else
	{
		if (!getStartLocation(msg.getStartingLocation(), planetName, coordinates, cellId))
		{
			// bad starting location
			LOG("TraceCharacterCreation", ("%d bad starting location (%s)", msg.getStationId(), msg.getStartingLocation().c_str()));
			ConnectionCreateCharacterFailed cccf(msg.getStationId(), msg.getCharacterName(), SharedStringIds::character_create_failed_bad_location, FormattedString<2048>().sprintf("%lu bad starting location (%s)", msg.getStationId(), msg.getStartingLocation().c_str()));
			CentralServer::getInstance().sendToConnectionServerForAccount(msg.getStationId(), cccf, true);
			unlockAccount(msg.getStationId());
			return;
		}
	}

	// - send request to game server
	creationRecord->second->m_gameCreationRequest = new CentralCreateCharacter(
		msg.getStationId(),
		msg.getCharacterName(),
		msg.getTemplateName(),
		msg.getScaleFactor(),
		planetName,
		coordinates,
		cellId,
		msg.getAppearanceData(),
		msg.getHairTemplateName(),
		msg.getHairAppearanceData(),
		msg.getProfession(),
		msg.getBiography(),
		useNewbieTutorial,
		msg.getSkillTemplate(),
		msg.getWorkingSkill(),
		msg.getJedi(),
		msg.getGameFeatures());
	uint32 gameServerId = CentralServer::getInstance().sendToRandomGameServer(*(creationRecord->second->m_gameCreationRequest));
	if (gameServerId == 0)
	{
		DEBUG_REPORT_LOG(true, ("Could not find a game server for character creation, starting a tutorial server\n"));
		LOG("TraceCharacterCreation", ("%d waiting for game server", msg.getStationId()));
		CentralServer::getInstance().startPlanetServer(CentralServer::getInstance().getHostForScene(tutorialPlanetName), tutorialPlanetName, 0);
		creationRecord->second->m_stage = CreationRecord::S_queuedForGameServer;
		return;
	}

	LOG("TraceCharacterCreation", ("%d sending CentralCreateCharacter(%s) to game server %lu", msg.getStationId(), Unicode::wideToNarrow(msg.getCharacterName()).c_str(), gameServerId));
	creationRecord->second->m_stage = CreationRecord::S_sentToGameServer;
	creationRecord->second->m_gameServerId = gameServerId;
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::unlockAccount(StationId account)
{
	delete m_creations[account];
	IGNORE_RETURN(m_creations.erase(account));
	LOG("TraceCharacterCreation", ("%d removing character creation lock", account));
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::retryGameServerCreates()
{
	for (CreationsType::iterator i=m_creations.begin(); i!=m_creations.end(); ++i)
	{
		if (i->second->m_stage == CreationRecord::S_queuedForGameServer)
		{
			uint32 gameServerId = CentralServer::getInstance().sendToRandomGameServer(*i->second->m_gameCreationRequest);
			if (gameServerId != 0)
			{
				i->second->m_stage = CreationRecord::S_sentToGameServer;
				i->second->m_gameServerId = gameServerId;
				LOG("TraceCharacterCreation", ("%d sending CentralCreateCharacter to game server %lu", i->first,gameServerId));
			}
			else
			{
				i->second->m_gameServerId = 0;
			}
		}
	}
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::retryLoginServerCreates()
{
	for (CreationsType::iterator i=m_creations.begin(); i!=m_creations.end(); ++i)
	{
		if (i->second->m_stage == CreationRecord::S_queuedForLoginServer)
		{
			uint32 loginServerId = CentralServer::getInstance().sendToArbitraryLoginServer(*i->second->m_loginCreationRequest, false);
			if (loginServerId != 0 )
			{
				i->second->m_loginServerId = loginServerId;
				i->second->m_stage = CreationRecord::S_sentToLoginServer;
				LOG("TraceCharacterCreation", ("%d sending LoginCreateCharacterMessage", i->first));
			}
			else
			{
				i->second->m_loginServerId = 0;
			}
		}
	}
}

// ----------------------------------------------------------------------

const bool CharacterCreationTracker::getStartLocation(const std::string & name, std::string & planetName, Vector & coordinates, NetworkId & cellId) const
{
	const StartingLocationData * const sld = StartingLocationManager::findLocationByName (name);
	if (sld)
	{
		planetName    = sld->planet;
		coordinates.x = sld->x;
		coordinates.z = sld->z;
		cellId        = NetworkId (sld->cellId);
		return true;
	}

	REPORT_LOG(true, ("The start location \"%s\" could not be found in StartingLocationManager", name.c_str()));
	return false;
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::onGameServerDisconnect(uint32 serverId)
{
	for (CreationsType::iterator i=m_creations.begin(); i!=m_creations.end(); ++i)
	{
		if (i->second->m_stage == CreationRecord::S_sentToGameServer && i->second->m_gameServerId==serverId)
		{
			i->second->m_stage = CreationRecord::S_queuedForGameServer;
			i->second->m_gameServerId = 0;
			LOG("TraceCharacterCreation", ("%d requeueing because game server disconnected", i->first));
		}
	}
	retryGameServerCreates();
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::onLoginServerDisconnect(uint32 loginServerId)
{
	for (CreationsType::iterator i=m_creations.begin(); i!=m_creations.end(); ++i)
	{
		if (i->second->m_stage == CreationRecord::S_sentToLoginServer && i->second->m_loginServerId==loginServerId)
		{
			i->second->m_stage = CreationRecord::S_queuedForLoginServer;
			i->second->m_loginServerId = 0;
			LOG("TraceCharacterCreation", ("%d requeueing because login server disconnected", i->first));
		}
	}
	retryLoginServerCreates();
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::handleDatabaseCreateCharacterSuccess(StationId account, const Unicode::String &characterName, const NetworkId &characterObjectId, int templateId, bool jedi)
{
	// - Check we are at the right stage
	LOG("TraceCharacterCreation", ("%d DatabaseCreateCharacterSuccess(%s)", account, characterObjectId.getValueString().c_str()));
	CreationsType::iterator creationRecord=m_creations.find(account);
	if (creationRecord==m_creations.end() || creationRecord->second->m_stage != CreationRecord::S_sentToGameServer)
	{
		LOG("TraceCharacterCreation", ("%d DatabaseCreateCharacterSuccess was unexpected - exiting", account));
		DEBUG_WARNING(true,("Programmer bug:  got GameCreateCharacter message for accout %d, which we weren't expecting.\n",account));
		return;
	}

	// - Tell the login server to add the character
	LoginCreateCharacterMessage *msg = new LoginCreateCharacterMessage(account,characterName,characterObjectId,templateId,jedi);
	creationRecord->second->m_loginCreationRequest = msg;
	creationRecord->second->m_characterId = characterObjectId;
	uint32 loginServerId = CentralServer::getInstance().sendToArbitraryLoginServer(*msg, false);
	if (loginServerId != 0)
	{
		creationRecord->second->m_loginServerId = loginServerId;
		creationRecord->second->m_stage = CreationRecord::S_sentToLoginServer;
		LOG("TraceCharacterCreation", ("%d sending LoginCreateCharacterMessage", account));
	}
	else
	{
		creationRecord->second->m_loginServerId = 0;
		creationRecord->second->m_stage = CreationRecord::S_queuedForLoginServer;
		LOG("TraceCharacterCreation", ("%d waiting for login server", account));
	}
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::handleLoginCreateCharacterAck(StationId account)
{
	// - Check we are at the right stage
	LOG("TraceCharacterCreation", ("%d LoginCreateCharacterAckMessage", account));
	CreationsType::iterator creationRecord=m_creations.find(account);
	if (creationRecord==m_creations.end() || creationRecord->second->m_stage != CreationRecord::S_sentToLoginServer)
	{
		LOG("TraceCharacterCreation", ("%d LoginCreateCharacterAckMessage was unexpected - exiting", account));
		DEBUG_WARNING(true,("Programmer bug:  got LoginCreateCharacterAckMessage message for account %d, which we weren't expecting.\n",account));
		return;
	}

	// - Tell the client the character has been created
	LOG("TraceCharacterCreation", ("%d acknowledgeCharacterCreate(%s)", account, creationRecord->second->m_characterId.getValueString().c_str()));
	LOG("CustomerService", ("Player:created character %s for stationId %u", creationRecord->second->m_characterId.getValueString().c_str(), account));
	const ConnectionCreateCharacterSuccess c(account, creationRecord->second->m_characterId);
	CentralServer::getInstance().sendToConnectionServerForAccount(account, c, true);
	unlockAccount(account);

	// let all connection servers know that a new character has been created for the station account
	GenericValueTypeMessage<StationId> const ncc("NewCharacterCreated", account);
	CentralServer::getInstance().sendToAllConnectionServers(ncc, true);

	setFastCreationLock(account);
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::handleGameCreateCharacterFailed(StationId account, Unicode::String const &characterName, StringId const &errorMessage, std::string const &optionalDetailedErrorMessage)
{
	CreationsType::iterator creationRecord=m_creations.find(account);
	if (creationRecord==m_creations.end() || creationRecord->second->m_stage != CreationRecord::S_sentToGameServer)
	{
		LOG("TraceCharacterCreation", ("%d got handleGameCreateCharacterFailed, but we weren't in the sentToGameServer stage - ignoring", account));
		DEBUG_WARNING(true,("Programmer bug:  got handleGameCreateCharacterFailed message for account %d, which we weren't expecting.\n",account));
		return;
	}

	LOG("TraceCharacterCreation", ("%d received GameCreateCharacterFailed(%s)", account, Unicode::wideToNarrow(characterName).c_str()));
	ConnectionCreateCharacterFailed cccf(account, characterName, errorMessage, optionalDetailedErrorMessage);
	CentralServer::getInstance().sendToConnectionServerForAccount(account, cccf, true);
	unlockAccount(account);
}

// ----------------------------------------------------------------------

void CharacterCreationTracker::setFastCreationLock(StationId account)
{
	m_fastCreationLock[account]=Clock::timeSeconds();
}

// ======================================================================

CharacterCreationTracker::CreationRecord::CreationRecord() :
		m_stage(S_queuedForGameServer),
		m_gameCreationRequest(nullptr),
		m_loginCreationRequest(nullptr),
		m_creationTime(ServerClock::getInstance().getGameTimeSeconds()),
		m_gameServerId(0),
		m_loginServerId(0),
		m_characterId(NetworkId::cms_invalid)
{
}

// ----------------------------------------------------------------------

CharacterCreationTracker::CreationRecord::~CreationRecord()
{
	delete m_gameCreationRequest;
	delete m_loginCreationRequest;
	m_gameCreationRequest = 0;
	m_loginCreationRequest = 0;
}
