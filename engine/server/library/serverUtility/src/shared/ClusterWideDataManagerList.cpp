// ======================================================================
//
// ClusterWideDataManagerList.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/ClusterWideDataManagerList.h"

#include "serverUtility/ClusterWideDataManager.h"
#include "serverUtility/ServerConnection.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedNetworkMessages/ClusterWideDataGetElementMessage.h"
#include "sharedNetworkMessages/ClusterWideDataGetElementResponseMessage.h"
#include "sharedNetworkMessages/ClusterWideDataReleaseLockMessage.h"
#include "sharedNetworkMessages/ClusterWideDataRemoveElementMessage.h"
#include "sharedNetworkMessages/ClusterWideDataUpdateDictionaryMessage.h"
#include "sharedUtility/ValueDictionaryArchive.h"

// ======================================================================

namespace ClusterWideDataManagerListNamespace
{
	struct LockInfo
	{
		LockInfo() : lockKey(0), lockTime(0.0), managerName() {};

		unsigned long lockKey;
		float         lockTime;
		std::string   managerName;
	};

	struct QueuedRequestInfo
	{
		QueuedRequestInfo() : processId(0), requestTime(0.0), request(nullptr), server(nullptr) {};

		unsigned long                      processId;
		float                              requestTime;
		ClusterWideDataGetElementMessage * request;
		ServerConnection *                 server;
	};

	typedef std::map<unsigned long, QueuedRequestInfo>                                QueuedRequestList;

	typedef std::multimap<unsigned long, LockInfo>                                    ServerLockList;
	typedef	std::pair<ServerLockList::const_iterator, ServerLockList::const_iterator> ServerLockListConstRange;
	typedef	std::pair<ServerLockList::iterator, ServerLockList::iterator>             ServerLockListRange;

	typedef std::map<std::string, ClusterWideDataManager *>                           ManagerList;

	typedef std::map<unsigned long, float>											  LockTimeList;

	// tracks requests which are queued because elements are locked
	unsigned long     s_queuedRequestNumber = 0;
	QueuedRequestList s_queuedRequestList;

	// tracks servers and the locks they own
	ServerLockList    s_serverLockList;

	// tracks the different Cluster wide data managers
	ManagerList       s_managerList;

	// tracks when locks are assigned so they can be automatically released
	// after a certain amount of time and the owner hasn't release them yet
	float             s_oldestLockAge = 0.0;
	int               s_lockTimeoutValue = 0;
	LockTimeList      s_lockTimeList;

	// accumulate game time to be used in detecting expired locks
	float             s_totalGameTime = 0.0;

	ClusterWideDataManager * getClusterWideDataManager(std::string const & managerName, bool const createIfNotExist);
	void processQueuedRequests();
	bool handleClusterWideDataGetElementMessage(ClusterWideDataGetElementMessage const & msg, ServerConnection & server);
	void handleClusterWideDataReleaseLockMessage(ClusterWideDataReleaseLockMessage const & msg, ServerConnection const & server);
	void handleClusterWideDataRemoveElementMessage(ClusterWideDataRemoveElementMessage const & msg, ServerConnection const & server);
	void handleClusterWideDataUpdateDictionaryMessage(ClusterWideDataUpdateDictionaryMessage const & msg, ServerConnection const & server);
	void removeFromLockTimeList(unsigned long const lockKey);
	void removeExpiredLocks();
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerList::install()
{
	ExitChain::add(remove, "ClusterWideDataManagerList");
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerList::remove()
{
	// clear up queued requests
	for (ClusterWideDataManagerListNamespace::QueuedRequestList::iterator iter = ClusterWideDataManagerListNamespace::s_queuedRequestList.begin(); iter != ClusterWideDataManagerListNamespace::s_queuedRequestList.end(); ++iter)
		delete iter->second.request;

	ClusterWideDataManagerListNamespace::s_queuedRequestList.clear();

	// clear up cluster wide data managers
	for (ClusterWideDataManagerListNamespace::ManagerList::iterator iter2 = ClusterWideDataManagerListNamespace::s_managerList.begin(); iter2 != ClusterWideDataManagerListNamespace::s_managerList.end(); ++iter2)
		delete iter2->second;

	ClusterWideDataManagerListNamespace::s_managerList.clear();

	// clear up lock lists
	ClusterWideDataManagerListNamespace::s_serverLockList.clear();
	ClusterWideDataManagerListNamespace::s_lockTimeList.clear();
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerList::update(float time)
{
	// accumulate game time
	ClusterWideDataManagerListNamespace::s_totalGameTime += time;

	// handle and locks that have expired
	ClusterWideDataManagerListNamespace::removeExpiredLocks();
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerList::registerMessage(MessageDispatch::Receiver & messageReceiver)
{
	messageReceiver.connectToMessage(ClusterWideDataGetElementMessage::ms_messageName.c_str());
	messageReceiver.connectToMessage(ClusterWideDataReleaseLockMessage::ms_messageName.c_str());
	messageReceiver.connectToMessage(ClusterWideDataRemoveElementMessage::ms_messageName.c_str());
	messageReceiver.connectToMessage(ClusterWideDataUpdateDictionaryMessage::ms_messageName.c_str());
}

// ----------------------------------------------------------------------

bool ClusterWideDataManagerList::handleMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	bool handledMessage = false;

	if (message.isType(ClusterWideDataGetElementMessage::ms_messageName.c_str()))
	{
		handledMessage = true;

		Archive::ReadIterator ri = safe_cast<const GameNetworkMessage *>(&message)->getByteStream().begin();

		// dynamic allocation because if the request needs to be queued,
		// we will store the message off for later processing
		ClusterWideDataGetElementMessage * msg = new ClusterWideDataGetElementMessage(ri);

		// need to cast away const here because we need to send response
		// back to source and ServerConnection::send() is non-const
		ServerConnection * server = const_cast<ServerConnection *>(dynamic_cast<ServerConnection const *>(&source));
		if (server)
		{
			if (ClusterWideDataManagerListNamespace::handleClusterWideDataGetElementMessage(*msg, *server))
			{
				delete msg;
			}
			else
			{
				// some elements were locked, queue the request for later processing
				LOG("ClusterWideDataManagerList", ("Queuing request from server (%lu), manager name (%s), element name regex (%s), lock elements (%s), request Id (%lu)", server->getProcessId(), msg->getManagerName().c_str(), msg->getElementNameRegex().c_str(), (msg->getLockElements() ? "true" : "false"), msg->getRequestId()));

				ClusterWideDataManagerListNamespace::QueuedRequestInfo info;
				info.processId = server->getProcessId();
				info.requestTime = ClusterWideDataManagerListNamespace::s_totalGameTime;
				info.request = msg;
				info.server = server;

				ClusterWideDataManagerListNamespace::s_queuedRequestList[++ClusterWideDataManagerListNamespace::s_queuedRequestNumber] = info;
			}
		}
		else
		{
			WARNING(true, ("Received ClusterWideDataGetElementMessage but not from a game server - manager name (%s), element name regex (%s), lock elements (%s), request Id (%lu)", msg->getManagerName().c_str(), msg->getElementNameRegex().c_str(), (msg->getLockElements() ? "true" : "false"), msg->getRequestId()));
			delete msg;
		}
	}
	else if (message.isType(ClusterWideDataReleaseLockMessage::ms_messageName.c_str()))
	{
		handledMessage = true;

		Archive::ReadIterator ri = safe_cast<const GameNetworkMessage *>(&message)->getByteStream().begin();
		ClusterWideDataReleaseLockMessage msg(ri);

		ServerConnection const * server = dynamic_cast<ServerConnection const *>(&source);
		if (server)
		{
			ClusterWideDataManagerListNamespace::handleClusterWideDataReleaseLockMessage(msg, *server);
		}
		else
		{
			WARNING(true, ("Received ClusterWideDataReleaseLockMessage but not from a game server - manager name (%s), lock key (%lu)", msg.getManagerName().c_str(), msg.getLockKey()));
		}
	}
	else if (message.isType(ClusterWideDataRemoveElementMessage::ms_messageName.c_str()))
	{
		handledMessage = true;

		Archive::ReadIterator ri = safe_cast<const GameNetworkMessage *>(&message)->getByteStream().begin();
		ClusterWideDataRemoveElementMessage msg(ri);

		ServerConnection const * server = dynamic_cast<ServerConnection const *>(&source);
		if (server)
		{
			ClusterWideDataManagerListNamespace::handleClusterWideDataRemoveElementMessage(msg, *server);
		}
		else
		{
			WARNING(true, ("Received ClusterWideDataRemoveElementMessage but not from a game server - manager name (%s), element name regex (%s)", msg.getManagerName().c_str(), msg.getElementNameRegex().c_str()));
		}
	}
	else if (message.isType(ClusterWideDataUpdateDictionaryMessage::ms_messageName.c_str()))
	{
		handledMessage = true;

		Archive::ReadIterator ri = safe_cast<const GameNetworkMessage *>(&message)->getByteStream().begin();
		ClusterWideDataUpdateDictionaryMessage msg(ri);

		ServerConnection const * server = dynamic_cast<ServerConnection const *>(&source);
		if (server)
		{
			ClusterWideDataManagerListNamespace::handleClusterWideDataUpdateDictionaryMessage(msg, *server);
		}
		else
		{
			WARNING(true, ("Received ClusterWideDataUpdateDictionaryMessage but not from a game server - manager name (%s), element name regex (%s), replace dictionary (%s)", msg.getManagerName().c_str(), msg.getElementNameRegex().c_str(), (msg.getReplaceDictionary() ? "yes" : "no")));
		}
	}

	return handledMessage;
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerList::onGameServerDisconnect(unsigned long const processId)
{
	// remove any queued messages for the disconnected server
	int requestsDeleted = 0;

	for (ClusterWideDataManagerListNamespace::QueuedRequestList::iterator iter = ClusterWideDataManagerListNamespace::s_queuedRequestList.begin(); iter != ClusterWideDataManagerListNamespace::s_queuedRequestList.end();)
	{
		if (iter->second.processId == processId)
		{
			delete iter->second.request;
			ClusterWideDataManagerListNamespace::s_queuedRequestList.erase(iter++);
			++requestsDeleted;
		}
		else
		{
			++iter;
		}
	}

	if (requestsDeleted > 0)
		LOG("ClusterWideDataManagerList", ("Removed (%d) pending requests for disconnected server (%lu)", requestsDeleted, processId));

	// remove any "auto remove" elements owned by the server
	int elementsRemoved = 0;

	for (ClusterWideDataManagerListNamespace::ManagerList::const_iterator iter3 = ClusterWideDataManagerListNamespace::s_managerList.begin(); iter3 != ClusterWideDataManagerListNamespace::s_managerList.end(); ++iter3)
	{
		elementsRemoved += iter3->second->removeElementByOwnerId(processId);
	}

	if (elementsRemoved > 0)
		LOG("ClusterWideDataManagerList", ("Removed (%d) elements for disconnected server (%lu)", elementsRemoved, processId));

	// if server has any outstanding locks, unlock those elements
	int elementsUnlocked = 0;

	ClusterWideDataManagerListNamespace::ServerLockListConstRange range = ClusterWideDataManagerListNamespace::s_serverLockList.equal_range(processId);

	ClusterWideDataManager * manager = nullptr;
	for (ClusterWideDataManagerListNamespace::ServerLockList::const_iterator iter2 = range.first; iter2 != range.second; ++iter2)
	{
		manager = ClusterWideDataManagerListNamespace::getClusterWideDataManager((iter2->second).managerName, false);
		if (manager)
			elementsUnlocked += manager->releaseLock((iter2->second).lockKey);
		else
			WARNING(true, ("ClusterWideDataManager (%s) not found for lock (%lu) owned by disconnected server (%lu)", (iter2->second).managerName.c_str(), (iter2->second).lockKey, processId));

		ClusterWideDataManagerListNamespace::removeFromLockTimeList((iter2->second).lockKey);
	}

	unsigned int locksDeleted = ClusterWideDataManagerListNamespace::s_serverLockList.erase(processId);

	if (locksDeleted > 0)
		LOG("ClusterWideDataManagerList", ("Removed (%d) locks and unlocked (%d) elements for disconnected server (%lu)", locksDeleted, elementsUnlocked, processId));

	// if any locks were released or any elements were removed,
	// then some elements may now be unlocked, so go through
	// the queued requests and process them
	if ((elementsUnlocked > 0) || (elementsRemoved > 0))
		ClusterWideDataManagerListNamespace::processQueuedRequests();
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerList::setLockTimeoutValue(int const timeout)
{
	LOG("ClusterWideDataManagerList", ("Setting lock timeout value to (%d) seconds", timeout));

	ClusterWideDataManagerListNamespace::s_lockTimeoutValue = timeout;
}

// ----------------------------------------------------------------------

int ClusterWideDataManagerList::getNumberOfQueuedRequests()
{
	return ClusterWideDataManagerListNamespace::s_queuedRequestList.size();
}

// ----------------------------------------------------------------------

ClusterWideDataManager * ClusterWideDataManagerListNamespace::getClusterWideDataManager(std::string const & managerName, bool const createIfNotExist)
{
	ManagerList::iterator iter = s_managerList.find(managerName);
	if (iter != s_managerList.end())
		return iter->second;

	if (createIfNotExist)
	{
		ClusterWideDataManager * manager = new ClusterWideDataManager(managerName);
		s_managerList[managerName] = manager;
		return manager;
	}

	return nullptr;
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerListNamespace::processQueuedRequests()
{
	for (QueuedRequestList::iterator iter = s_queuedRequestList.begin(); iter != s_queuedRequestList.end();)
	{
		if (handleClusterWideDataGetElementMessage(*(iter->second.request), *(iter->second.server)))
		{
			delete iter->second.request;
			s_queuedRequestList.erase(iter++);
		}
		else
		{
			++iter;
		}
	}
}

// ----------------------------------------------------------------------

bool ClusterWideDataManagerListNamespace::handleClusterWideDataGetElementMessage(ClusterWideDataGetElementMessage const & msg, ServerConnection & server)
{
	bool success = true;

	std::vector<std::string> elementNameList;
	std::vector<ValueDictionary> elementDictionaryList;
	unsigned long lockKey = 0;

	ClusterWideDataManager * manager = getClusterWideDataManager(msg.getManagerName(), false);
	if (manager)
	{
		success = manager->getElement(msg.getElementNameRegex(), msg.getLockElements(), elementNameList, elementDictionaryList, lockKey);
	}

	if (success)
	{
		// cut down on the log spam; don't log "browse" requests
		if (lockKey > 0)
			LOG("ClusterWideDataManagerList", ("Returned (%d) elements to server (%lu), manager name (%s), element name regex (%s), lock elements (%s), request Id (%lu), lock key (%lu)", elementNameList.size(), server.getProcessId(), msg.getManagerName().c_str(), msg.getElementNameRegex().c_str(), (msg.getLockElements() ? "true" : "false"), msg.getRequestId(), lockKey));

		// send response
		ClusterWideDataGetElementResponseMessage resp(msg.getManagerName(), msg.getElementNameRegex(), elementNameList, elementDictionaryList, msg.getRequestId(), lockKey);
		server.send(resp, true);

		// if a lock was acquired, add it to the lock list so if
		// the server goes down, we can unlock the locked elements
		if (lockKey > 0)
		{
			LockInfo info;
			info.lockKey = lockKey;
			info.lockTime = s_totalGameTime;
			info.managerName = msg.getManagerName();

			IGNORE_RETURN(s_serverLockList.insert(ServerLockList::value_type(server.getProcessId(), info)));

			s_lockTimeList[lockKey] = info.lockTime;

			if (s_lockTimeList.size() == 1)
				s_oldestLockAge = info.lockTime;
		}
	}

	return success;
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerListNamespace::handleClusterWideDataReleaseLockMessage(ClusterWideDataReleaseLockMessage const & msg, ServerConnection const & server)
{
	// remove the lock from the lock list
	bool foundLockToRemove = false;
	ServerLockListRange range = s_serverLockList.equal_range(server.getProcessId());
	ServerLockList::iterator iter;
	for (iter = range.first; iter != range.second; ++iter)
	{
		if ((iter->second).lockKey == msg.getLockKey())
		{
			if ((iter->second).managerName == msg.getManagerName())
				foundLockToRemove = true;
			else
				WARNING(true, ("ClusterWideDataManagerList received request to unlock lock (%lu) with incorrect manager (%s) where correct manager is (%s)", msg.getLockKey(), (msg.getManagerName()).c_str(), ((iter->second).managerName).c_str()));
			break;
		}
	}

	if (foundLockToRemove)
	{
		// if we found a lock to remove in the ServerLockList, we remove it from the manager and the list
		ClusterWideDataManager * manager = getClusterWideDataManager(msg.getManagerName(), false);
		if (manager)
		{
			s_serverLockList.erase(iter);
			ClusterWideDataManagerListNamespace::removeFromLockTimeList(msg.getLockKey());

			int numberElementsUnlocked = manager->releaseLock(msg.getLockKey());

			LOG("ClusterWideDataManagerList", ("Unlocked (%d) elements for server (%lu), manager name (%s), lock key (%lu)", numberElementsUnlocked, server.getProcessId(), msg.getManagerName().c_str(), msg.getLockKey()));

			// if any elements were unlocked, so go through the queued requests and process them
			if (numberElementsUnlocked > 0)
				processQueuedRequests();
		}
		else
			LOG("ClusterWideDataManagerList", ("ClusterWideDataManagerList received request to unlock lock but could not get a ClusterWideDataManager for manager name (%s), lock key (%lu)", msg.getManagerName().c_str(), msg.getLockKey()));
	}
	else if (iter == range.second)
		LOG("ClusterWideDataManagerList", ("ClusterWideDataManagerList received request to unlock lock but could not find any locks with matching lock key: manager name (%s), lock key (%lu)", msg.getManagerName().c_str(), msg.getLockKey()));
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerListNamespace::handleClusterWideDataRemoveElementMessage(ClusterWideDataRemoveElementMessage const & msg, ServerConnection const & server)
{
	ClusterWideDataManager * manager = getClusterWideDataManager(msg.getManagerName(), false);
	if (manager)
	{
		int numberElementsRemoved = manager->removeElement(msg.getElementNameRegex(), msg.getLockKey());

		LOG("ClusterWideDataManagerList", ("Removed (%d) elements for server (%lu), manager name (%s), element name regex (%s), lock key (%lu)", numberElementsRemoved, server.getProcessId(), msg.getManagerName().c_str(), msg.getElementNameRegex().c_str(), msg.getLockKey()));

		// if any elements were removed, so go through the queued requests and process them
		if (numberElementsRemoved > 0)
			processQueuedRequests();
	}
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerListNamespace::handleClusterWideDataUpdateDictionaryMessage(ClusterWideDataUpdateDictionaryMessage const & msg, ServerConnection const & server)
{
	ClusterWideDataManager * manager = getClusterWideDataManager(msg.getManagerName(), true);
	if (manager)
	{
		if (msg.getReplaceDictionary())
		{
			int numberElementsReplaced = manager->replaceDictionary(msg.getElementNameRegex(), msg.getDictionary(), (msg.getAutoRemove() ? server.getProcessId() : 0)  , msg.getLockKey());

			LOG("ClusterWideDataManagerList", ("Replaced (%d) elements for server (%lu), manager name (%s), element name regex (%s), auto remove (%s), lock key (%lu)", numberElementsReplaced, server.getProcessId(), msg.getManagerName().c_str(), msg.getElementNameRegex().c_str(), (msg.getAutoRemove() ? "yes" : "no"), msg.getLockKey()));
		}
		else
		{
			int numberElementsUpdated = manager->updateDictionary(msg.getElementNameRegex(), msg.getDictionary(), msg.getLockKey());

			LOG("ClusterWideDataManagerList", ("Updated (%d) elements for server (%lu), manager name (%s), element name regex (%s), lock key (%lu)", numberElementsUpdated, server.getProcessId(), msg.getManagerName().c_str(), msg.getElementNameRegex().c_str(), msg.getLockKey()));
		}
	}
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerListNamespace::removeFromLockTimeList(unsigned long const lockKey)
{
	IGNORE_RETURN(s_lockTimeList.erase(lockKey));

	if (!ClusterWideDataManagerListNamespace::s_lockTimeList.empty())
		ClusterWideDataManagerListNamespace::s_oldestLockAge = ClusterWideDataManagerListNamespace::s_lockTimeList.begin()->second;
	else
		ClusterWideDataManagerListNamespace::s_oldestLockAge = 0.0;
}

// ----------------------------------------------------------------------

void ClusterWideDataManagerListNamespace::removeExpiredLocks()
{
	if (s_lockTimeoutValue <= 0)
		return;

	if (s_oldestLockAge == 0.0)
		return;

	if ((s_totalGameTime - s_oldestLockAge) < static_cast<float>(s_lockTimeoutValue))
		return;

	if (s_lockTimeList.empty())
	{
		WARNING(true, ("ClusterWideDataManagerList mismatch - age of the oldest lock is (%.2f seconds), but there are no locks in the lock time list", (s_totalGameTime - s_oldestLockAge)));

		s_oldestLockAge = 0.0;

		return;
	}

	// get the lock key that has expired
	unsigned long oldestLockKey = 0;

	// limit the scope of iter
	{
		LockTimeList::iterator iter = s_lockTimeList.begin();
		if (iter->second != s_oldestLockAge) //lint !e777 // we compare these two float values because we expect them to be bitwise identical
		{
			WARNING(true, ("ClusterWideDataManagerList mismatch between perceived age of the oldest lock (%.2f seconds), and the actual age of the oldest lock (%.2f seconds), lock key (%lu)", (s_totalGameTime - s_oldestLockAge), (s_totalGameTime - iter->second), iter->first));

			s_oldestLockAge = iter->second;

			return;
		}

		// remove the lock from the lock time list
		oldestLockKey = iter->first;

		// ***iter is invalid after this call; also the call will change s_oldestLockAge
		// to be the age of the next oldest lock, or 0 if there are no more locks***
		removeFromLockTimeList(oldestLockKey);
	}

	// find the lock info for the expired lock key
	ServerLockList::iterator iter2 = s_serverLockList.begin();
	for (; iter2 != s_serverLockList.end(); ++iter2)
	{
		if ((iter2->second).lockKey == oldestLockKey)
			break;
	}

	if (iter2 == s_serverLockList.end())
	{
		WARNING(true, ("ClusterWideDataManagerList could not locate lock info for expired lock key (%lu)", oldestLockKey));
		return;
	}

	// release the locked elements
	int numberElementsUnlocked = 0;
	ClusterWideDataManager * manager = getClusterWideDataManager((iter2->second).managerName, false);
	if (manager)
	{
		numberElementsUnlocked = manager->releaseLock((iter2->second).lockKey);

		LOG("ClusterWideDataManagerList", ("Unlocked (%d) elements for expired lock key (%lu) owned by server (%lu)", numberElementsUnlocked, (iter2->second).lockKey, iter2->first));
	}
	else
	{
		WARNING(true, ("ClusterWideDataManager (%s) not found for expired lock key (%lu) owned by server (%lu)", (iter2->second).managerName.c_str(), (iter2->second).lockKey, iter2->first));
	}

	// remove the lock from the lock list
	s_serverLockList.erase(iter2);

	// if any elements were unlocked, so go through the queued requests and process them
	if (numberElementsUnlocked > 0)
		processQueuedRequests();
}

// ======================================================================
