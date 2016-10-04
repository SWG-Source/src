// ======================================================================
//
// ServerObject_AuthTransfer.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerObject.h"

#include "serverGame/AuthTransferTracker.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PositionUpdateTracker.h"
#include "serverGame/ServerController.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerUIManager.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverNetworkMessages/AuthTransferClientMessage.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/GameServerForceChangeAuthorityMessage.h"
#include "serverNetworkMessages/PlanetRemoveObject.h"
#include "serverNetworkMessages/PlayedTimeAccumMessage.h"
#include "serverNetworkMessages/SetAuthoritativeMessage.h"
#include "serverNetworkMessages/UnloadObjectMessage.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

namespace ServerObject_AuthTransferNamespace
{

	// ----------------------------------------------------------------------

	void getContainedClients(ServerObject &obj, std::vector<Watcher<Client> > &containedClients)
	{
		// find all clients for primary controlled objects in the containment hierarchy under obj (including obj)

		Client * const client = obj.getClient();
		if (client && client->getCharacterObjectId() == obj.getNetworkId())
			containedClients.push_back(Watcher<Client>(client));

		Container * const container = ContainerInterface::getContainer(obj);
		if (container)
		{
			for (ContainerIterator i = container->begin(); i != container->end(); ++i)
			{
				ServerObject * const containedObject = safe_cast<ServerObject *>((*i).getObject());
				if (containedObject)
					getContainedClients(*containedObject, containedClients);
			}
		}
	}

	// ----------------------------------------------------------------------

}
using namespace ServerObject_AuthTransferNamespace;

// ======================================================================

void ServerObject::beginAuthTransfer(uint32 pid) const
{
	AuthTransferTracker::beginAuthTransfer(getNetworkId(), pid, getExposedProxyList());
	Container const * const container = ContainerInterface::getContainer(*this);
	if (container)
	{
		for (ContainerConstIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
		{
			ServerObject const * const containedObject = safe_cast<ServerObject const *>((*containerIterator).getObject());
			if (containedObject)
				containedObject->beginAuthTransfer(pid);
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::transferAuthoritySceneChange(uint32 pid)
{
	std::vector<Watcher<Client> > containedClients;
	getContainedClients(*this, containedClients);

	{
		for (std::vector<Watcher<Client> >::const_iterator i = containedClients.begin(); i != containedClients.end(); ++i)
		{
			Client * const client = (*i).getPointer();
			if (client)
			{
				ServerObject * const characterObject = (*i)->getCharacterObject();
				if (characterObject)
					characterObject->onClientAboutToLoad();
			}
		}
	}

	beginAuthTransfer(pid);

	ProxyList proxyList = getExposedProxyList();

	// Before we send baselines to the new server, we need to clear out the proxy list,
	if (!m_proxyServerProcessIds.empty())
		m_proxyServerProcessIds.clear();

	setAuthServerProcessId(pid);
	sendDeltasForSelfAndContents(&proxyList);

	// get rid of the old proxies now, because we already had to remove them from the proxy list
	if (!proxyList.empty())
	{
		ServerMessageForwarding::begin(std::vector<uint32>(proxyList.begin(), proxyList.end()));

		UnloadObjectMessage const unloadMessage(getNetworkId());
		ServerMessageForwarding::send(unloadMessage);

		ServerMessageForwarding::end();
	}

	// set up message forwarding
	ServerMessageForwarding::begin(pid);

	forwardServerCreateAndBaselines();

	// build transfer authority message
	SetAuthoritativeMessage const sam(getNetworkId(), pid, true, false, NetworkId::cms_invalid, getTransform_o2w(), true);

	releaseAuthority(pid);

	{
		for (std::vector<Watcher<Client> >::const_iterator i = containedClients.begin(); i != containedClients.end(); ++i)
		{
			Client * const client = (*i).getPointer();

			if (client)
			{
				ObserveTracker::onClientAboutToTransferAuthority(*client);
				// transfer the client to the new authoritative server before
				// transfering authority so that when the new authoritative
				// takes authority, it will immediately attach the client
				// to the object, so there's no possibility of the game
				// client missing updates to the object
				ConnectionServerConnection * const connectionServer = client->getConnection();
				if (connectionServer)
				{
					DEBUG_REPORT_LOG(ConfigServerGame::getLogObservers(), ("%s is now sending AuthTransferClientMessage\n", client->getCharacterObjectId().getValueString().c_str()));
					AuthTransferClientMessage const atcm(
						client->getCharacterObjectId(),
						connectionServer->getRemoteAddress(),
						connectionServer->getRemotePort(),
						false,
						client->getAccountName(),
						client->getIpAddress(),
						client->isSecure(),
						client->getStationId(),
						nullptr,
						client->getGameFeatures(),
						client->getSubscriptionFeatures(),
						client->getAccountFeatureIds(),
						client->getEntitlementTotalTime(),
						client->getEntitlementEntitledTime(),
						client->getEntitlementTotalTimeSinceLastLogin(),
						client->getEntitlementEntitledTimeSinceLastLogin(),
						client->getBuddyPoints(),
						GameServer::getInstance().getProcessId(),
						client->getConsumedRewardEvents(),
						client->getClaimedRewardItems(),
						client->isUsingAdminLogin(),
						static_cast<int>(client->getCombatSpamFilter()),
						client->getCombatSpamRangeSquaredFilter(),
						client->getFurnitureRotationDegree(),
						client->getHasUnoccupiedJediSlot(),
						client->getIsJediSlotCharacter());

					ServerMessageForwarding::send(atcm);
				}
				client->clearControlledObjects();
			}
		}
	}

	// transfer authority to the new authoritative server
	ServerMessageForwarding::send(sam);

	// End forwarding
	ServerMessageForwarding::end();
}

// ----------------------------------------------------------------------

void ServerObject::transferAuthorityNoSceneChange(uint32 pid, bool skipLoadScreen, const NetworkId &goalCell, const Transform &goalTransform, bool informPlanetServer)
{
	beginAuthTransfer(pid);

	ProxyList proxyList = getExposedProxyList();
	m_proxyServerProcessIds.insert(GameServer::getInstance().getProcessId());
	bool needCreate = !isProxiedOnServer(pid);
	setAuthServerProcessId(pid);
	sendDeltasForSelfAndContents(&proxyList);

	// Set up message forwarding
	ServerMessageForwarding::begin(pid);

	if (needCreate)
		forwardServerCreateAndBaselines();

	// build transfer authority message
	SetAuthoritativeMessage const sam(getNetworkId(), pid, false, false, goalCell, goalTransform, true);

	std::vector<Watcher<Client> > containedClients;
	getContainedClients(*this, containedClients);

	releaseAuthority(pid);

	for (std::vector<Watcher<Client> >::const_iterator i = containedClients.begin(); i != containedClients.end(); ++i)
	{
		Client * const client = (*i).getPointer();

		if (client)
		{
			// force any UpdatePvpStatusMessage waiting to be sent to this
			// client at the end of the frame to be send now, because the
			// client object is going away, and the receiving game server
			// doesn't have the list of UpdatePvpStatusMessage waiting to
			// be sent to this client at the end of the frame; we only need
			// to do this when there is an authority transfer that doesn't
			// involve a scene change, because with scene change, the game
			// client will unload all objects and get fresh baselines and
			// UpdatePvpStatusMessage for observed objects
			PvpUpdateObserver::sendUpdateAndRemoveClientFromPvpStatusCache(client);

			ObserveTracker::onClientAboutToTransferAuthority(*client);
			// transfer the client to the new authoritative server before
			// transfering authority so that when the new authoritative
			// takes authority, it will immediately attach the client
			// to the object, so there's no possibility of the game
			// client missing updates to the object
			ConnectionServerConnection * const connectionServer = client->getConnection();
			if (connectionServer)
			{
				std::vector<NetworkId> observeListIds;
				if (skipLoadScreen)
				{
					Client::ObservingList const &observeList = client->getObserving();
					for (Client::ObservingList::const_iterator j = observeList.begin(); j != observeList.end(); ++j)
						observeListIds.push_back((*j)->getNetworkId());
				}

				DEBUG_REPORT_LOG(ConfigServerGame::getLogObservers(), ("%s is now sending AuthTransferClientMessage\n", client->getCharacterObjectId().getValueString().c_str()));
				AuthTransferClientMessage const atcm(
					client->getCharacterObjectId(),
					connectionServer->getRemoteAddress(),
					connectionServer->getRemotePort(),
					skipLoadScreen,
					client->getAccountName(),
					client->getIpAddress(),
					client->isSecure(),
					client->getStationId(),
					(observeListIds.empty() ? nullptr : &observeListIds),
					client->getGameFeatures(),
					client->getSubscriptionFeatures(),
					client->getAccountFeatureIds(),
					client->getEntitlementTotalTime(),
					client->getEntitlementEntitledTime(),
					client->getEntitlementTotalTimeSinceLastLogin(),
					client->getEntitlementEntitledTimeSinceLastLogin(),
					client->getBuddyPoints(),
					GameServer::getInstance().getProcessId(),
					client->getConsumedRewardEvents(),
					client->getClaimedRewardItems(),
					client->isUsingAdminLogin(),
					static_cast<int>(client->getCombatSpamFilter()),
					client->getCombatSpamRangeSquaredFilter(),
					client->getFurnitureRotationDegree(),
					client->getHasUnoccupiedJediSlot(),
					client->getIsJediSlotCharacter());

				ServerMessageForwarding::send(atcm);
			}
			client->clearControlledObjects();
		}
	}

	// transfer authority to the new authoritative server
	ServerMessageForwarding::send(sam);

	if (informPlanetServer)
	{
		GameServerForceChangeAuthorityMessage const authMessage(getNetworkId(), GameServer::getInstance().getProcessId(), pid);
		GameServer::getInstance().sendToPlanetServer(authMessage);
	}

	// End forwarding
	ServerMessageForwarding::end();
}

// ----------------------------------------------------------------------

void ServerObject::transferAuthority(uint32 pid, bool skipLoadScreen, bool handlingCrash, bool informPlanetServer)
{
	transferAuthority(pid, skipLoadScreen, handlingCrash, getAttachedTo() ? getAttachedTo()->getNetworkId() : NetworkId::cms_invalid, getTransform_o2p(), informPlanetServer);
}

// ----------------------------------------------------------------------

void ServerObject::transferAuthority(uint32 pid, bool skipLoadScreen, bool handlingCrash, const NetworkId &goalCell, const Transform &goalTransform, bool informPlanetServer)
{
	// If we are handling a crash and being told to become authoritative, we do so directly.
	// If we are not authoritative, but have been set as the auth server process id for this object,
	// then we are completing the transfer process and should set ourselves authoritative.
	// If neither of these is true, then we are beginning the auth transfer process, or should tell
	// the auth object to do so if we aren't it.
	if (   pid == GameServer::getInstance().getProcessId()
	    && (handlingCrash || getAuthServerProcessId() == GameServer::getInstance().getProcessId()))
	{
		// setting authority to this server
		if (!isAuthoritative())
		{
			if (handlingCrash)
			{
				// normally this stuff is done by the server that's giving up authority
				setAuthServerProcessId(pid);
				if (isPlayerControlled())
				{
					std::vector<NetworkId> nothing; //TODO:  do we need to worry about this in the crash recovery case?
					TransferControlMessage const tcm(getNetworkId(), pid, true, nothing);
					GameServer::getInstance().sendToConnectionServers(tcm);
				}
			}

			if (goalCell == NetworkId::cms_invalid)
			{
				safe_cast<ServerController *>(getController())->teleport(goalTransform, 0);
				setAuthority();
			}
			else
			{
				ServerObject * const cell = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(goalCell));
				safe_cast<ServerController *>(getController())->teleport(goalTransform, cell);
				setAuthority();
			}
		}
	}
	else
	{
		// setting authority to a different server
		if (!isAuthoritative())
		{
			sendControllerMessageToAuthServer(CM_transferAuthority, new MessageQueueGenericValueType<std::pair<uint32, bool> >(std::make_pair(pid, skipLoadScreen)));
			return;
		}

		if (GameServer::getInstance().isGameServerConnected(pid))
		{
			if (getSceneId() != ServerWorld::getSceneId() && getSceneId() != "universe")
				transferAuthoritySceneChange(pid);
			else
				transferAuthorityNoSceneChange(pid, skipLoadScreen, goalCell, goalTransform, informPlanetServer);
		}

		else
		{
			WARNING(true, ("GameServer(%d) is not connected to this gameserver! TransferAuthority for (%s : %s) will not continue because it CANNOT send authority transfer network messages to a server that is not running or not connected!", pid, getObjectTemplateName(), getNetworkId().getValueString().c_str()));
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::releaseAuthority(uint32 newProcessId)
{
	DEBUG_REPORT_LOG(ConfigServerGame::getLogObservers(), ("Called releaseAuthority on %s to process %lu\n", getNetworkId().getValueString().c_str(), newProcessId));

	PositionUpdateTracker::flushPositionUpdate(*this);

	// remove our synchronizedUi
	ServerSynchronizedUi * const syncUI = m_synchronizedUi.getPointer();
	if (syncUI)
	{
		DEBUG_WARNING(true, ("Object %s is being set non-authoritative but still has a synchronizedUi", getNetworkId().getValueString().c_str()));
		syncUI->onAuthorityTransferred();
		delete syncUI;
	}

	NetworkController * const controller = safe_cast<NetworkController *>(getController());
	NOT_NULL(controller);

	if (isPlayerControlled())
	{
		//DEBUG_WARNING(true, ("ServerUIManager: ServerObject is calling clientLosingAuthority, network id = %s, newProcessId = %lu", getNetworkId().getValueString().c_str(), newProcessId));
		ServerUIManager::onPlayerAuthorityTransferred(getNetworkId());
	}

	PlayerObject *thisAsPlayer = this->asPlayerObject();
	if(thisAsPlayer)
	{
		PlayedTimeAccumMessage ptam(getNetworkId(), thisAsPlayer->getPlayedTimeAccumOnly());
		ServerMessageForwarding::send(ptam);
	}
	else
	{
		CreatureObject *thisAsCreature = this->asCreatureObject();
		if(thisAsCreature)
		{
			PlayedTimeAccumMessage ptam(getNetworkId(), thisAsCreature->getPseudoPlayedTime());
			ServerMessageForwarding::send(ptam);
		}
	}

	Container * const container = ContainerInterface::getContainer(*this);
	if (container)
	{
		std::vector<CachedNetworkId> idList;
		idList.reserve(static_cast<size_t>(container->getNumberOfItems()));

		for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
			idList.push_back(*containerIterator);

		for (std::vector<CachedNetworkId>::const_iterator i = idList.begin(); i != idList.end(); ++i)
		{
			ServerObject * const containedObject = safe_cast<ServerObject*>(i->getObject());
			if (containedObject)
				containedObject->releaseAuthority(newProcessId);
		}
	}

	// if we're in another scene and losing authority, unload and notify the planet server
	if (   getSceneId() != ServerWorld::getSceneId()
	    && getSceneId() != "universe"
	    && (isPlayerControlled() || ContainerInterface::getTopmostContainer(*this) == this))
	{
		PlanetRemoveObject const removeMessage(getNetworkId());
		GameServer::getInstance().sendToPlanetServer(removeMessage);
		unload();
	}

	controller->setAuthoritative(false);

	if (getScriptObject())
		getScriptObject()->setOwnerIsAuthoritative(false, newProcessId);

	virtualOnReleaseAuthority();
}

// ======================================================================

