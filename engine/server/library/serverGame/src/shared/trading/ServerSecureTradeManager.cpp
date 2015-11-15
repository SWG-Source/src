// ======================================================================
//
// ServerSecureTradeManager.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================


#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerSecureTradeManager.h"

#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerSecureTrade.h"
#include "serverGame/ServerWorld.h"

#include "sharedFoundation/GameControllerMessage.h"

#include "sharedNetworkMessages/MessageQueueSecureTrade.h"
#include "sharedNetworkMessages/SecureTradeMessages.h"


// ------------------------------------------------------------

void ServerSecureTradeManager::install()
{
}

// ------------------------------------------------------------

void ServerSecureTradeManager::remove()
{
}

// ------------------------------------------------------------

/** Normally request is an asynchronous request.  But it can fail immediately (and return false) if
* The initiator is already trading
*/

bool ServerSecureTradeManager::requestTradeWith (CreatureObject & initiator, CreatureObject & recipient)
{
	if (PlayerObject::isIgnoring(recipient.getNetworkId(), initiator.getNetworkId()))
	{
		DEBUG_REPORT_LOG(true, ("Trade request stopped due to recipient ignoring initiator\n"));
		return false;
	}

	//Send message to recipient.
	return recipient.getCreatureController()->tradeRequested(initiator);
}

// ------------------------------------------------------------

void ServerSecureTradeManager::acceptTradeRequest (CreatureObject & initiator,CreatureObject & recipient)
{
	//@todo.  This function will need to see if the two objects are on the same server. 
	//If not, then request that they be of the planet server and queue up a request
	//If so proceed with pretrade.  For now, always proceed.

	beginPreTrade(initiator, recipient);

}

// ------------------------------------------------------------

/** 
	This is issued if the player refuses the trade, or if the recip is already trading.
*/
void ServerSecureTradeManager::refuseTrade(CreatureObject & initiator, CreatureObject & recipient)
{
	MessageQueueSecureTrade * m = new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_DeniedTrade, initiator.getNetworkId(), recipient.getNetworkId());
	initiator.getCreatureController()->appendMessage(static_cast<int>(CM_secureTrade), 0.0f, m,
					GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);

}

// ------------------------------------------------------------

/** When a trade is aborted or exitted, this call will clean things up.
*/
void ServerSecureTradeManager::clearTrade (CreatureObject & initiator, CreatureObject & recipient)
{
	ServerSecureTrade * secureTrade = initiator.getCreatureController()->getSecureTrade();
	DEBUG_FATAL( secureTrade != recipient.getCreatureController()->getSecureTrade(), ("Parties not involved in the same trade!\n"));

	initiator.getCreatureController()->setSecureTrade(0);
	recipient.getCreatureController()->setSecureTrade(0);

	delete secureTrade;

	//@todo unlock players from server migration
}
	
// ------------------------------------------------------------

void ServerSecureTradeManager::beginPreTrade(CreatureObject & initiator, CreatureObject & recipient)
{
	if (initiator.getCreatureController()->getSecureTrade() || recipient.getCreatureController()->getSecureTrade())
	{
		DEBUG_REPORT_LOG(true, ("Objects already trading\n"));
		return;
	}

	//@todo Lock the players from server migration.
	if (!initiator.isAuthoritative() || !recipient.isAuthoritative())
	{
		WARNING_STRICT_FATAL(true, ("Cannot execute trade between objects not on the same server\n"));
		return;
	}

	ServerSecureTrade * secureTrade = new ServerSecureTrade(initiator, recipient);

	initiator.getCreatureController()->setSecureTrade(secureTrade);
	recipient.getCreatureController()->setSecureTrade(secureTrade);


	secureTrade->beginTrading();
}

// ------------------------------------------------------------
