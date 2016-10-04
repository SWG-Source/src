// ======================================================================
//
// ClusterWideDataClient.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ClusterWideDataClient.h"

#include "serverGame/GameServer.h"
#include "serverGame/ServerObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedNetworkMessages/ClusterWideDataGetElementMessage.h"
#include "sharedNetworkMessages/ClusterWideDataGetElementResponseMessage.h"
#include "sharedNetworkMessages/ClusterWideDataReleaseLockMessage.h"
#include "sharedNetworkMessages/ClusterWideDataRemoveElementMessage.h"
#include "sharedNetworkMessages/ClusterWideDataUpdateDictionaryMessage.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedUtility/ValueDictionaryArchive.h"

// ======================================================================

namespace ClusterWideDataClientNamespace
{
	// tracks the requests that have been sent to the
	// cluster wide data manager, and the object that
	// should be notified when the response comes in
	typedef std::map<unsigned long, NetworkId> CallbackObjectIdList;
	CallbackObjectIdList callbackObjectIdList;
}

// ----------------------------------------------------------------------

void ClusterWideDataClient::install()
{
	ExitChain::add(remove, "ClusterWideDataClient");
}

// ----------------------------------------------------------------------

void ClusterWideDataClient::remove()
{
	// clear up requests list
	ClusterWideDataClientNamespace::callbackObjectIdList.clear();
}

// ----------------------------------------------------------------------

unsigned long ClusterWideDataClient::getClusterWideData(std::string const & managerName, std::string const & elementNameRegex, bool const lockElements, NetworkId const & callbackObjectId)
{
	ClusterWideDataGetElementMessage const msg(managerName, elementNameRegex, lockElements);
	GameServer::getInstance().sendToCentralServer(msg);

	// associate the request Id with the callback object
	ClusterWideDataClientNamespace::callbackObjectIdList[msg.getRequestId()] = callbackObjectId;

	return msg.getRequestId();
}

// ----------------------------------------------------------------------

void ClusterWideDataClient::releaseClusterWideDataLock(std::string const & managerName, unsigned long const lockKey)
{
	ClusterWideDataReleaseLockMessage const msg(managerName, lockKey);
	GameServer::getInstance().sendToCentralServer(msg);
}

// ----------------------------------------------------------------------

void ClusterWideDataClient::removeClusterWideData(std::string const & managerName, std::string const & elementNameRegex, unsigned long const lockKey)
{
	ClusterWideDataRemoveElementMessage const msg(managerName, elementNameRegex, lockKey);
	GameServer::getInstance().sendToCentralServer(msg);
}

// ----------------------------------------------------------------------

void ClusterWideDataClient::updateClusterWideData(std::string const & managerName, std::string const & elementNameRegex, ValueDictionary const & dictionary, unsigned long const lockKey)
{
	ClusterWideDataUpdateDictionaryMessage const msg(managerName, elementNameRegex, dictionary, false, false, lockKey);
	GameServer::getInstance().sendToCentralServer(msg);
}

// ----------------------------------------------------------------------

void ClusterWideDataClient::replaceClusterWideData(std::string const & managerName, std::string const & elementNameRegex, ValueDictionary const & dictionary, bool const autoRemove, unsigned long const lockKey)
{
	ClusterWideDataUpdateDictionaryMessage const msg(managerName, elementNameRegex, dictionary, true, autoRemove, lockKey);
	GameServer::getInstance().sendToCentralServer(msg);
}

// ----------------------------------------------------------------------

void ClusterWideDataClient::registerMessage(MessageDispatch::Receiver & messageReceiver)
{
	messageReceiver.connectToMessage(ClusterWideDataGetElementResponseMessage::ms_messageName.c_str());
}

// ----------------------------------------------------------------------

bool ClusterWideDataClient::handleMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{
	bool handledMessage = false;

	if (message.isType(ClusterWideDataGetElementResponseMessage::ms_messageName.c_str()))
	{
		handledMessage = true;

		Archive::ReadIterator ri = safe_cast<const GameNetworkMessage *>(&message)->getByteStream().begin();

		ClusterWideDataGetElementResponseMessage msg(ri);

		// locate the callback object
		ClusterWideDataClientNamespace::CallbackObjectIdList::iterator iter = ClusterWideDataClientNamespace::callbackObjectIdList.find(msg.getRequestId());
		ServerObject * object = nullptr;

		if (iter != ClusterWideDataClientNamespace::callbackObjectIdList.end())
			object = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(iter->second));

		// invoke callback object with the result
		if ((object) && (object->isAuthoritative()))
		{
			ScriptParams params;
			params.addParam(msg.getManagerName().c_str());
			params.addParam(msg.getElementNameRegex().c_str());
			params.addParam(static_cast<int>(msg.getRequestId()));

			std::vector<const char *> nameList;
			for (std::vector<std::string>::const_iterator i = msg.getElementNameList().begin(); i != msg.getElementNameList().end(); ++i)
				nameList.push_back(i->c_str());

			params.addParam(nameList);

			params.addParam(msg.getElementDictionaryList());
			params.addParam(static_cast<int>(msg.getLockKey()));

			IGNORE_RETURN(object->getScriptObject()->trigAllScripts(Scripting::TRIG_RECEIVE_CLUSTER_WIDE_DATA_RESPONSE, params));
		}
		else
		{
			if (object)
				WARNING(true, ("Non-authoritative object with network id (%s) cannot process cluster wide data response Id (%lu)", ((iter != ClusterWideDataClientNamespace::callbackObjectIdList.end()) ? iter->second.getValueString().c_str() : "none"), msg.getRequestId()));
			else
				WARNING(true, ("Can't locate object with network id (%s) for cluster wide data response Id (%lu)", ((iter != ClusterWideDataClientNamespace::callbackObjectIdList.end()) ? iter->second.getValueString().c_str() : "none"), msg.getRequestId()));

			// unlock any locked elements
			if (msg.getLockKey() > 0)
				releaseClusterWideDataLock(msg.getManagerName(), msg.getLockKey());
		}

		// request has now been serviced
		if (iter != ClusterWideDataClientNamespace::callbackObjectIdList.end())
			ClusterWideDataClientNamespace::callbackObjectIdList.erase(iter);
	}

	return handledMessage;
}

// ======================================================================
