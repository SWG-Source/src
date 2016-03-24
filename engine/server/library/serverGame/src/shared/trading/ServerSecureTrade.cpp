// ======================================================================
//
// ServerSecureTrade.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================


#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerSecureTrade.h"

#include "serverGame/Chat.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/Pvp.h"
#include "serverGame/ServerSecureTradeManager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/SecureTradeMessages.h"
#include "sharedObject/VolumeContainer.h"

#include <vector>
// ----------------------------------------------------------------------

namespace ServerSecureTradeNamespace
{
	bool isContainedByPlayerButNotEquipped(const CreatureObject & player, const ServerObject& item)
	{
		//@todo TFiala - this function will need to change with mounts
		const Object* topmost = ContainerInterface::getFirstParentInWorld(item);
		const Object* parent = ContainerInterface::getContainedByObject(item);

		//This function should return true if the topmost player is equal to the player passed into the function and
		//  if it is not immediatly contained by the player (equipped)
		
		if (topmost == &player && parent != &player)
			return true;

		return false;
	}
};

using namespace ServerSecureTradeNamespace;

//------------------------------------------------------------------------------------------

ServerSecureTrade::ServerSecureTrade(CreatureObject & initiator, CreatureObject & recipient) :
m_id(0),
m_initiator(&initiator),
m_initiatorContents(new std::vector<Watcher<ServerObject> >),
m_initiatorMoney(0),
m_initiatorOk(false),
m_initiatorVerify(false),
m_recipient(&recipient),
m_recipientContents(new std::vector<Watcher<ServerObject> >),
m_recipientMoney (0),
m_recipientOk(false),
m_recipientVerify(false),
m_tradeState(ServerSecureTrade::TS_Initializing)
{
	//todo get id from the manager, if we need an id.
}

// ----------------------------------------------------------------------

ServerSecureTrade::~ServerSecureTrade()
{
	delete m_initiatorContents;
	delete m_recipientContents;
}

// ----------------------------------------------------------------------

void ServerSecureTrade::acceptOffer(const CreatureObject & trader)
{
	if (m_tradeState != TS_Trading)
	{
		DEBUG_REPORT_LOG(true, ("Secure Trade: accepting an offer in wrong state\n"));
		return;
	}

	if (&trader == m_initiator)
	{
		m_initiatorOk = true;

		Client * const recipientClient = m_recipient->getClient();
		if (recipientClient)
		{
			AcceptTransactionMessage const m;
			recipientClient->send(m, true);
		}
	}
	else if (&trader == m_recipient)
	{
		m_recipientOk = true;

		Client * const initiatorClient = m_initiator->getClient();
		if (initiatorClient)
		{
			AcceptTransactionMessage const m;
			initiatorClient->send(m, true);
		}
	}
	else
		DEBUG_REPORT_LOG(true, ("Secure Trade: received an accept from non participant\n"));

	if (m_initiatorOk && m_recipientOk)
		beginVerification();
}

// ----------------------------------------------------------------------

void ServerSecureTrade::addItem(const CreatureObject & trader, const ServerObject & item)
{

	if (m_tradeState != TS_Trading)
	{
		DEBUG_REPORT_LOG(true, ("Secure Trade: received an item in non-trade state\n"));
		beginTrading();
	}

	if (!item.canTradeRecursive(false) && (!(m_initiator->getClient() && m_initiator->getClient()->isGod())) && (!(m_recipient->getClient() && m_recipient->getClient()->isGod())))
	{
		DEBUG_REPORT_LOG(true, ("Secure Trade: cannot trade item marked with objvar noTrade\n"));
		Client * const traderClient = trader.getClient();
		if (traderClient)
		{
			AddItemFailedMessage const m(item.getNetworkId());
			traderClient->send(m, true);
		}
		return;
	}

	//Only allow trade of things in inventory (but not equipped)
	Container::ContainerErrorCode code =  Container::CEC_Success;
	if (!isContainedByPlayerButNotEquipped(trader, item)) //m_recipient->canManipulateObject(item, true, true, true, 0, code))
	{
		Client * const traderClient = trader.getClient();
		if (traderClient)
		{
			AddItemFailedMessage const m(item.getNetworkId());
			traderClient->send(m, true);
		}

		code = Container::CEC_TradeEquipped;
		ContainerInterface::sendContainerMessageToClient(trader, code);
		return;
	}
	
	if (&trader == m_initiator)
	{
		if (std::find(m_initiatorContents->begin(), m_initiatorContents->end(), &item) == m_initiatorContents->end())
		{
			m_initiatorContents->push_back(Watcher<ServerObject>(const_cast<ServerObject *> (&item)));
			
			Client * const recipientClient = m_recipient->getClient();
			if (recipientClient)
			{
				item.sendCreateAndBaselinesToClient(*recipientClient);

				AddItemMessage const m(item.getNetworkId());
				recipientClient->send(m, true);
			}
		}
		else
		{
			Client * const traderClient = trader.getClient();
			if (traderClient)
			{
				AddItemFailedMessage const m(item.getNetworkId());
				traderClient->send(m, true);
			}
			//@todo need an error message
		}
	}
	else if (& trader == m_recipient)
	{
		if (std::find(m_recipientContents->begin(), m_recipientContents->end(), &item) == m_recipientContents->end())
		{
			m_recipientContents->push_back(Watcher<ServerObject>(const_cast<ServerObject *> (&item)));
			
			Client * const initiatorClient = m_initiator->getClient();
			if (initiatorClient)
			{
				item.sendCreateAndBaselinesToClient(*initiatorClient);

				AddItemMessage const m(item.getNetworkId());
				initiatorClient->send(m, true);
			}
		}
		else
		{
			Client * const traderClient = trader.getClient();
			if (traderClient)
			{
				AddItemFailedMessage const m(item.getNetworkId());
				traderClient->send(m, true);
			}
			//@todo need an error message about duplicate item.
		}

	}
	else
	{
		WARNING_STRICT_FATAL(true, ("Secure Trade: received an add Item from non participant\n"));
		return;
	}
	unacceptOffer(*m_initiator);
	unacceptOffer(*m_recipient);

}

// ----------------------------------------------------------------------

void ServerSecureTrade::beginTrading()
{
	m_tradeState = TS_Trading;
	m_initiatorOk = false;
	m_recipientOk = false;

	Client * const initiatorClient = m_initiator->getClient();
	if (initiatorClient)
	{
		BeginTradeMessage const toInitiator(m_recipient->getNetworkId());
		initiatorClient->send(toInitiator, true);
	}

	Client * const recipientClient = m_recipient->getClient();
	if (recipientClient)
	{
		BeginTradeMessage const toRecipient(m_initiator->getNetworkId());
		recipientClient->send(toRecipient, true);
	}
}

// ----------------------------------------------------------------------

void ServerSecureTrade::beginVerification()
{
	WARNING_STRICT_FATAL(m_tradeState != TS_Trading, ("Illegal state transition to verify state\n"));
	m_tradeState = TS_Verifying;
	m_initiatorVerify = false;
	m_recipientVerify = false;

	BeginVerificationMessage const m;

	Client * const initiatorClient = m_initiator->getClient();
	if (initiatorClient)
		initiatorClient->send(m, true);

	Client * const recipientClient = m_recipient->getClient();
	if (recipientClient)
		recipientClient->send(m, true);
}

// ----------------------------------------------------------------------

void ServerSecureTrade::cancelTrade(const CreatureObject & trader)
{
	if (&trader != m_initiator && &trader != m_recipient)
	{
		WARNING_STRICT_FATAL(true, ("Secure Trade: received a cancel from non participant\n"));
		return;
	}

	AbortTradeMessage const m;

	Client * const initiatorClient = m_initiator->getClient();
	if (initiatorClient)
		initiatorClient->send(m, true);

	Client * const recipientClient = m_recipient->getClient();
	if (recipientClient)
		recipientClient->send(m, true);

	//clean up and exit
	exitTrade();
}

// ----------------------------------------------------------------------

void ServerSecureTrade::completeTrade()
{
	//@todo we need better feedback messages
	if (m_tradeState != TS_Verifying)
	{
		WARNING_STRICT_FATAL(true, ("Illegal state transition to complete state\n"));
		return;
	}
	if (!m_initiator || !m_recipient)
	{
		WARNING_STRICT_FATAL(true, ("Null trade participant\n"));
		return;
	}

	m_tradeState = TS_Complete;

	//Do a range check
	float distance = m_initiator->findPosition_w().magnitudeBetween(m_recipient->findPosition_w());
	distance -= m_initiator->getRadius();
	distance -= m_recipient->getRadius();
	// Check range on the server with a 1m fudge factor in it.
	if (distance > 6.0f + 1.0f)
	{
		Client * const initiatorClient = m_initiator->getClient();
		if (initiatorClient)
		{
			ProsePackage prosePackage;
			prosePackage.stringId = StringId("ui_trade", "out_of_range_prose");
			prosePackage.target.str = m_recipient->getAssignedObjectName();
			Chat::sendSystemMessage(*m_initiator, prosePackage);
		}

		Client * const recipientClient = m_recipient->getClient();
		if (recipientClient)
		{
			ProsePackage prosePackage;
			prosePackage.stringId = StringId("ui_trade", "out_of_range_prose");
			prosePackage.target.str = m_initiator->getAssignedObjectName();
			Chat::sendSystemMessage(*m_recipient, prosePackage);
		}

		cancelTrade(*m_initiator);
		return;
	}
	
//Do the trade

	//@todo We need to verify the cash can be transferred!  This may require adding a seperate stage (verifying) that waits for  a DB response.

	const long initatorTotalMoney = m_recipientMoney - m_initiatorMoney;
	if (initatorTotalMoney > 0)
	{
		if (m_recipient->getCashBalance() < initatorTotalMoney)
		{
			cancelTrade(*m_recipient);
			return;
		}
	}
	else if (m_initiatorMoney < 0)
	{
		if (m_initiator->getCashBalance() < -initatorTotalMoney)
		{
			cancelTrade(*m_initiator);
			return;
		}
	}

	ServerObject* initiatorInventory = m_initiator->getInventory();
	ServerObject* recipientInventory = m_recipient->getInventory();
	ServerObject* initiatorDatapad = m_initiator->getDatapad();
	ServerObject* recipientDatapad = m_recipient->getDatapad();
	Container::ContainerErrorCode errorCode = Container::CEC_Success;

	if (!initiatorInventory || !recipientInventory || !initiatorDatapad || !recipientDatapad)
	{
		cancelTrade(*m_initiator);
		errorCode = Container::CEC_WrongType;
		ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
		ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
		return;
	}

	int totalInitiatorTangibleVolume = 0;
	int totalInitiatorInTangibleVolume = 0;
	int totalRecipientTangibleVolume = 0;
	int totalRecipientInTangibleVolume = 0;
	
	std::vector<Watcher<ServerObject> >::iterator i = m_initiatorContents->begin();
	//check scripts on all I's items
	for (; i != m_initiatorContents->end(); ++i)
	{
		ServerObject* item = *i;
		if (!item)
		{
			//Item was deleted during trade.  Better cancel it.
			errorCode = Container::CEC_NotFound;
			ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
			ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
			cancelTrade(*m_initiator);
			return;
		}
		if (dynamic_cast<TangibleObject*>(item))
		{
			if (!ContainerInterface::canTransferTo(recipientInventory, *item, nullptr, errorCode))
			{
				ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
				ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
				cancelTrade(*m_initiator);
				return;
			}
			totalInitiatorTangibleVolume += item->getVolume();
					
		}
		else
		{
			if (!ContainerInterface::canTransferTo(recipientDatapad, *item, nullptr, errorCode))
			{
				ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
				ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
				cancelTrade(*m_initiator);
				return;
			}
			totalInitiatorInTangibleVolume += item->getVolume();
		}
	}

	//check scripts on all R's items
	for (i = m_recipientContents->begin(); i != m_recipientContents->end(); ++i)
	{
		ServerObject* item = *i;
		if (!item)
		{
			//Item was deleted during trade.  Better cancel it.
			errorCode = Container::CEC_NotFound;
			ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
			ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
			cancelTrade(*m_initiator);
			return;
		}
		if (dynamic_cast<TangibleObject*>(item))
		{
			if (!ContainerInterface::canTransferTo(initiatorInventory, *item, nullptr, errorCode))
			{
				ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
				ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
				cancelTrade(*m_initiator);
				return;
			}
			totalRecipientTangibleVolume += item->getVolume();
		}
		else
		{
			if (!ContainerInterface::canTransferTo(initiatorDatapad, *item, nullptr, errorCode))
			{
				ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
				ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
				cancelTrade(*m_initiator);
				return;
			}
			totalRecipientInTangibleVolume += item->getVolume();
		}

	}

	//Check total volumes:
	VolumeContainer * volCont = ContainerInterface::getVolumeContainer(*initiatorInventory);
	if (!volCont || !volCont->checkVolume(totalRecipientTangibleVolume))
	{
		errorCode = Container::CEC_InventoryFull;
		ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
		ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
		cancelTrade(*m_recipient);
		return;
	}
	volCont = ContainerInterface::getVolumeContainer(*initiatorDatapad);
	if (!volCont || !volCont->checkVolume(totalRecipientInTangibleVolume))
	{
		errorCode = Container::CEC_InventoryFull;
		ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
		ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
		cancelTrade(*m_recipient);
		return;
	}
	volCont = ContainerInterface::getVolumeContainer(*recipientInventory);
	if (!volCont || !volCont->checkVolume(totalInitiatorTangibleVolume))
	{
		errorCode = Container::CEC_InventoryFull;
		ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
		ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
		cancelTrade(*m_recipient);
		return;
	}
	volCont = ContainerInterface::getVolumeContainer(*recipientDatapad);
	if (!volCont || !volCont->checkVolume(totalInitiatorInTangibleVolume))
	{
		errorCode = Container::CEC_InventoryFull;
		ContainerInterface::sendContainerMessageToClient(*m_initiator, errorCode);
		ContainerInterface::sendContainerMessageToClient(*m_recipient, errorCode);
		cancelTrade(*m_recipient);
		return;
	}
	
	
	//Do transfers.  All transfers SHOULD succeed at this point.  @todo check this by walking through source.
	//First money
	if (initatorTotalMoney > 0)
	{
		CachedNetworkId id(*m_initiator);
		if (!m_recipient->transferCashTo(id, initatorTotalMoney))
		{
			cancelTrade(*m_recipient);
			return;
		}
		LOG("CustomerService", ("Trade:%s received %d money from %s", PlayerObject::getAccountDescription(m_initiator->getNetworkId()).c_str(), initatorTotalMoney, PlayerObject::getAccountDescription(m_recipient->getNetworkId()).c_str()));
	}
	else if (initatorTotalMoney < 0)
	{
		CachedNetworkId id(*m_recipient);
		if (!m_initiator->transferCashTo(id, -initatorTotalMoney))
		{
			cancelTrade(*m_initiator);
			return;
		}
		LOG("CustomerService", ("Trade:%s received %d money from %s", PlayerObject::getAccountDescription(m_recipient->getNetworkId()).c_str(), -initatorTotalMoney, PlayerObject::getAccountDescription(m_initiator->getNetworkId()).c_str()));
	}

	//Then contents
	
	for (i = m_initiatorContents->begin(); i != m_initiatorContents->end(); ++i)
	{
		ServerObject* item = *i;
		NOT_NULL(item);
		if (dynamic_cast<TangibleObject*>(item))
		{
			if (!ContainerInterface::transferItemToVolumeContainer(*recipientInventory, *item, nullptr, errorCode))
			{
				DEBUG_FATAL(true, ("Secure trade transfer failure in initiator transfer!!\n"));
				LOG("CustomerService", ("Trade:%s FAILED TO receive %s from %s", PlayerObject::getAccountDescription(m_recipient->getNetworkId()).c_str(), ServerObject::getLogDescription(item).c_str(), PlayerObject::getAccountDescription(m_initiator->getNetworkId()).c_str()));
			}
			else
			{
				LOG("CustomerService", ("Trade:%s received %s from %s", PlayerObject::getAccountDescription(m_recipient->getNetworkId()).c_str(), ServerObject::getLogDescription(item).c_str(), PlayerObject::getAccountDescription(m_initiator->getNetworkId()).c_str()));
				logTradeitemContents(*m_recipient, *item, *m_initiator);
			}
		}
		else
		{
			if (!ContainerInterface::transferItemToVolumeContainer(*recipientDatapad, *item, nullptr, errorCode))
			{
				DEBUG_FATAL(true, ("Secure trade transfer failure in initiator transfer!!\n"));
				LOG("CustomerService", ("Trade:%s FAILED TO receive %s from %s", PlayerObject::getAccountDescription(m_recipient->getNetworkId()).c_str(), ServerObject::getLogDescription(item).c_str(), PlayerObject::getAccountDescription(m_initiator->getNetworkId()).c_str()));
			}
			else
			{
				LOG("CustomerService", ("Trade:%s received %s from %s", PlayerObject::getAccountDescription(m_recipient->getNetworkId()).c_str(), ServerObject::getLogDescription(item).c_str(), PlayerObject::getAccountDescription(m_initiator->getNetworkId()).c_str()));
				logTradeitemContents(*m_recipient, *item, *m_initiator);
			}

		}
		
	}

	for (i = m_recipientContents->begin(); i != m_recipientContents->end(); ++i)
	{
		ServerObject* item = *i;
		NOT_NULL(item);
		if (dynamic_cast<TangibleObject*>(item))
		{
			if (!ContainerInterface::transferItemToVolumeContainer(*initiatorInventory, *item, nullptr, errorCode))
			{
				DEBUG_FATAL(true, ("Secure trade transfer failure in recipient transfer!!\n"));
				LOG("CustomerService", ("Trade:%s FAILED TO receive %s from %s", PlayerObject::getAccountDescription(m_initiator->getNetworkId()).c_str(), ServerObject::getLogDescription(item).c_str(), PlayerObject::getAccountDescription(m_recipient->getNetworkId()).c_str()));
			}
			else
			{
				LOG("CustomerService", ("Trade:%s received %s from %s", PlayerObject::getAccountDescription(m_initiator->getNetworkId()).c_str(), ServerObject::getLogDescription(item).c_str(), PlayerObject::getAccountDescription(m_recipient->getNetworkId()).c_str()));
				logTradeitemContents(*m_initiator, *item, *m_recipient);
			}

		}
		else
		{
			if (!ContainerInterface::transferItemToVolumeContainer(*initiatorDatapad, *item, nullptr, errorCode))
			{
				DEBUG_FATAL(true, ("Secure trade transfer failure in recipient transfer!!\n"));
				LOG("CustomerService", ("Trade:%s FAILED TO receive %s from %s", PlayerObject::getAccountDescription(m_initiator->getNetworkId()).c_str(), ServerObject::getLogDescription(item).c_str(), PlayerObject::getAccountDescription(m_recipient->getNetworkId()).c_str()));
			}
			else
			{
				LOG("CustomerService", ("Trade:%s received %s from %s", PlayerObject::getAccountDescription(m_initiator->getNetworkId()).c_str(), ServerObject::getLogDescription(item).c_str(), PlayerObject::getAccountDescription(m_recipient->getNetworkId()).c_str()));
				logTradeitemContents(*m_initiator, *item, *m_recipient);
			}
		}
	}

	//If successful send message to clients.
	TradeCompleteMessage const m;

	Client * const initiatorClient = m_initiator->getClient();
	if (initiatorClient)
		initiatorClient->send(m, true);

	Client * const recipientClient = m_recipient->getClient();
	if (recipientClient)
		recipientClient->send(m, true);

	Pvp::helpPerformed(*m_initiator, *m_recipient);
	Pvp::helpPerformed(*m_recipient, *m_initiator);
	
	
	exitTrade();

}

// ----------------------------------------------------------------------

void ServerSecureTrade::logTradeitemContents(const CreatureObject & to, 
	const ServerObject & item, const CreatureObject & from) const
{
	const Container * container = ContainerInterface::getContainer(item);
	if (container != nullptr)
	{
		for (ContainerConstIterator i = container->begin(); i != container->end(); ++i)
		{
			const Object * o = (*i).getObject();
			if (o != nullptr)
			{
				const ServerObject * content = safe_cast<const ServerObject *>(o);
				LOG("CustomerService", ("Trade:%s received %s contained in %s from %s", 
					PlayerObject::getAccountDescription(to.getNetworkId()).c_str(), 
					ServerObject::getLogDescription(content).c_str(), 
					ServerObject::getLogDescription(&item).c_str(),
					PlayerObject::getAccountDescription(from.getNetworkId()).c_str()
					));
				logTradeitemContents(to, *content, from);
			}
			else if ((*i) != CachedNetworkId::cms_cachedInvalid)
			{
				LOG("CustomerService", ("Trade:%s received unknown object %s contained in %s from %s", 
					PlayerObject::getAccountDescription(to.getNetworkId()).c_str(), 
					(*i).getValueString().c_str(), 
					ServerObject::getLogDescription(&item).c_str(),
					PlayerObject::getAccountDescription(from.getNetworkId()).c_str()
					));
			}
		}
	}
}

// ----------------------------------------------------------------------

void ServerSecureTrade::exitTrade()
{
	//unlock objects here
	ServerSecureTradeManager::clearTrade(*m_initiator, *m_recipient);
}

// ----------------------------------------------------------------------

/** 
* the initiator money is the amount of money the initiator is putting up for trade
* the recipient money is the amount of money the recipient is putting up for trade
*
* only one transfer results in the end, the difference of the two values
*
* It is important to note that the client displays how much money they are GIVING away regardless of who we are.
* 
*/

void ServerSecureTrade::giveMoney(const CreatureObject & trader, int amount)
{
	if (m_tradeState != TS_Trading)
	{
		DEBUG_REPORT_LOG(true, ("Secure Trade: received money in non-trade state\n"));
		beginTrading();
	}

	if (amount < 0)
	{
		DEBUG_WARNING (true, ("Secure trade: received negative money amount in giveMoney from %s", trader.getNetworkId ().getValueString ().c_str ()));
		return;
	}

	if (&trader == m_initiator)
	{
		m_initiatorMoney = amount;

		Client * const recipientClient = m_recipient->getClient();
		if (recipientClient)
		{
			GiveMoneyMessage const giveMessage(amount);
			recipientClient->send(giveMessage, true);
		}
	}
	else if (&trader == m_recipient)
	{
		m_recipientMoney = amount;

		Client * const initiatorClient = m_initiator->getClient();
		if (initiatorClient)
		{
			GiveMoneyMessage const giveMessage(amount);
			initiatorClient->send(giveMessage, true);
		}
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("Secure Trade: received a modify money from non participant\n"));
	}

	unacceptOffer(*m_initiator);
	unacceptOffer(*m_recipient);
}

// ----------------------------------------------------------------------

void ServerSecureTrade::rejectOffer(const CreatureObject & trader)
{	
	if (m_tradeState != TS_Verifying)
	{
		DEBUG_REPORT_LOG(true, ("Secure Trade: received offer reject non-Verify state\n"));
	}

	DenyTradeMessage const m;
	if (&trader == m_initiator)
	{
		Client * const recipientClient = m_recipient->getClient();
		if (recipientClient)
			recipientClient->send(m, true);
	}
	else if (&trader == m_recipient)
	{
		Client * const initiatorClient = m_initiator->getClient();
		if (initiatorClient)
			initiatorClient->send(m, true);
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("Secure Trade: received reject trade from non participant\n"));
		return;
	}
	beginTrading();
}

// ----------------------------------------------------------------------

void ServerSecureTrade::removeItem(const CreatureObject & trader, const ServerObject & item)
{
	//-- CS requested that removing an item cancels the trade to prevent scamming
	UNREF (item);
	cancelTrade (trader);
}

// ----------------------------------------------------------------------

void ServerSecureTrade::verifyTrade(const CreatureObject & trader)
{
	if (m_tradeState != TS_Verifying)
	{
		DEBUG_REPORT_LOG(true, ("Secure Trade: verifying an offer in wrong state\n"));
		return;
	}

	if (&trader == m_initiator)
	{
		m_initiatorVerify = true;

		Client * const recipientClient = m_recipient->getClient();
		if (recipientClient)
		{
			VerifyTradeMessage const m;
			recipientClient->send(m, true);
		}
	}
	else if (&trader == m_recipient)
	{
		m_recipientVerify = true;

		Client * const initiatorClient = m_initiator->getClient();
		if (initiatorClient)
		{
			VerifyTradeMessage const m;
			initiatorClient->send(m, true);
		}
	}
	else
		DEBUG_REPORT_LOG(true, ("Secure Trade: received a verify from non participant\n"));

	if (m_initiatorVerify && m_recipientVerify)
		completeTrade();

}

// ----------------------------------------------------------------------

void ServerSecureTrade::unacceptOffer(const CreatureObject & trader)
{
	if (m_tradeState != TS_Trading)
	{
		DEBUG_REPORT_LOG(true, ("Secure Trade: accepting an offer in wrong state\n"));
		return;
	}

	if (&trader == m_initiator)
	{
		if (m_initiatorOk)
		{
			m_initiatorOk = false;

			Client * const recipientClient = m_recipient->getClient();
			if (recipientClient)
			{
				UnAcceptTransactionMessage const m;
				recipientClient->send(m, true);
			}
		}
	}
	else if (&trader == m_recipient)
	{
		if (m_recipientOk)
		{
			m_recipientOk = false;

			Client * const initiatorClient = m_initiator->getClient();
			if (initiatorClient)
			{
				UnAcceptTransactionMessage const m;
				initiatorClient->send(m, true);
			}
		}
	}
	else
		DEBUG_REPORT_LOG(true, ("Secure Trade: received an accept from non participant\n"));
}

bool ServerSecureTrade::hasItemInSecureTrade( const CreatureObject &trader, const ServerObject &item ) const
{
	
	// check for a match in intitiator
	std::vector<Watcher<ServerObject> >::iterator i = m_initiatorContents->begin();
	for (; i != m_initiatorContents->end(); ++i)
	{
		ServerObject*  tradeItem = *i;
		if (tradeItem && (tradeItem->getNetworkId() == item.getNetworkId() ) )
		{
			return true;
		}
	}

	// check for match in recipient
	std::vector<Watcher<ServerObject> >::iterator r = m_recipientContents->begin();
	for (; r != m_recipientContents->end(); ++r)
	{
		ServerObject*  tradeItem = *r;
		if (tradeItem && (tradeItem->getNetworkId() == item.getNetworkId()) )
		{
			return true;
		}
	}

	return false;
}
// ----------------------------------------------------------------------

