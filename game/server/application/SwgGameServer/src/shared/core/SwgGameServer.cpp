// ======================================================================
//
// SwgGameServer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgGameServer.h"
#include "SwgGameServer/SwgGameServer.h"
#include "serverGame/CommoditiesMarket.h"
#include "serverGame/PreloadManager.h"
#include "serverNetworkMessages/BountyHunterTargetListMessage.h"
#include "serverNetworkMessages/GameServerCSRequestMessage.h"
#include "serverNetworkMessages/GameServerCSResponseMessage.h"
#include "sharedDebug/Profiler.h"
#include "sharedGame/AppearanceManager.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/MessageManager.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "SwgGameServer/CombatEngine.h"
#include "SwgGameServer/JediManagerObject.h"
#include "SwgGameServer/ServerJediManagerObjectTemplate.h"
#include "SwgGameServer/SwgServerCreatureObjectTemplate.h"
#include "SwgGameServer/SwgServerPlayerObjectTemplate.h"
#include "SwgGameServer/SwgServerUniverse.h"
#include "SwgGameServer/CSHandler.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//#undef PROFILE_INDIVIDUAL_MESSAGES
#define PROFILE_INDIVIDUAL_MESSAGES 1

#ifdef PROFILE_INDIVIDUAL_MESSAGES
	#define MESSAGE_PROFILER_BLOCK(a) PROFILER_AUTO_BLOCK_DEFINE(a)
#else
	#define MESSAGE_PROFILER_BLOCK(a) NOP
#endif


//-----------------------------------------------------------------------

SwgGameServer::SwgGameServer() :
	GameServer()
{
	connectToMessage("BountyHunterTargetListMessage");
	connectToMessage("DeleteCharacterNotificationMessage");
}

//-----------------------------------------------------------------------

SwgGameServer::~SwgGameServer()
{
}

//-----------------------------------------------------------------------

void SwgGameServer::install()
{
	DEBUG_FATAL (ms_instance != nullptr, ("already installed"));
	ms_instance = new SwgGameServer;

	SwgServerUniverse::install();
	CombatEngine::install();
	ServerJediManagerObjectTemplate::install();
	SwgServerCreatureObjectTemplate::install();
	SwgServerPlayerObjectTemplate::install();
	// the preload manager MUST be installed after the SwgServer... templates
	PreloadManager::install();
	CommoditiesMarket::install();
	
	CSHandler::install();
}

//-----------------------------------------------------------------------

void SwgGameServer::initialize()
{
	GameServer::initialize();
}

//-----------------------------------------------------------------------

void SwgGameServer::shutdown()
{
	GameServer::shutdown();
	CommoditiesMarket::remove();
	CSHandler::remove();
}

void SwgGameServer::handleCSRequest( GameServerCSRequestMessage & request )
{
	CSHandler::getInstance().handle( request );
}

//-----------------------------------------------------------------------

void SwgGameServer::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	static Archive::ReadIterator ri;
	static Archive::ByteStream bs;

	PROFILER_AUTO_BLOCK_DEFINE("SwgGameServer::receiveMessage");

	const uint32 msgType = message.getType();
	
	if (msgType == constcrc("BountyHunterTargetListMessage"))
	{
		DEBUG_REPORT_LOG(true, ("SwgGameServer: got BountyHunterTargetListMessage\n"));
		MESSAGE_PROFILER_BLOCK("BountyHunterTargetListMessage");
		ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const BountyHunterTargetListMessage * msg = new BountyHunterTargetListMessage(ri);

		JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();

		if (jediManager != nullptr && jediManager->isAuthoritative())
		{
			jediManager->addJediBounties(*msg);
			delete msg;
		}
		else
		{
			LOG("BountyHunterTargetList", ("Queuing BountyHunterTargetList from DB because JediManagerObject is not yet ready"));
			JediManagerObject::queueBountyHunterTargetListFromDB(msg);
		}
	}
	else if (msgType == constcrc("DeleteCharacterNotificationMessage"))
	{
		ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const GenericValueTypeMessage<NetworkId> msg(ri);

		JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();

		if (jediManager != nullptr)
		{
			jediManager->characterBeingDeleted(msg.getValue());
		}
	}

	// GameServer::receiveMessage does some bookkeeping stuff, so always call it
	GameServer::receiveMessage(source, message);
}

