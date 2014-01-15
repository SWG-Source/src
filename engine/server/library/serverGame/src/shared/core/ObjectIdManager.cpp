// ======================================================================
//
// ObjectIdManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ObjectIdManager.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverNetworkMessages/AddObjectIdBlockMessage.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/RequestObjectIdsMessage.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/Emitter.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

ObjectIdManager::ObjectIdManager() :
		Singleton<ObjectIdManager>(),
		numAvailableIds(0),
		numRequestedIds(0),
		m_minReservedIds(ConfigServerGame::getMinObjectIds()),
		m_maxReservedIds(ConfigServerGame::getReservedObjectIds()),
		m_gotABlock(false)
{
	PROFILER_AUTO_BLOCK_DEFINE("ObjectIdManager::ObjectIdManager");
	DEBUG_FATAL((m_maxReservedIds<=m_minReservedIds),("maxReservedIds must be greater than minReservedIds."));
	
	connectToMessage("AddOIDBlockMessage");
	connectToMessage("CentralGameServerSetProcessId");
}

// ----------------------------------------------------------------------

ObjectIdManager::~ObjectIdManager()
{
//@todo:  release freelist
}

//-----------------------------------------------------------------------

NetworkId const ObjectIdManager::getNewObjectId()
{
	return getInstance().getNewObjectIdInternal();
}

// ----------------------------------------------------------------------

NetworkId const ObjectIdManager::getNewObjectIdInternal()
{
	if (numAvailableIds==0)
	{
		if (m_gotABlock)
			FATAL(true,("Out of available object ID's"));
		else
			FATAL(true,("Requested an object ID before any were received.\n"));
		return NetworkId::cms_invalid;
	}
	
	FreeListType::iterator i=m_freeList.begin();
	DEBUG_FATAL((i==m_freeList.end()),("numAvailableIds was non-zero but freelist was empty."));
	
	NetworkId::NetworkIdType theId=(*i).m_start;
	
	if (++((*i).m_start) > (*i).m_end)
	{ // this block is now empty
		m_freeList.erase(i);
	}
	
	if ((--numAvailableIds+numRequestedIds) < m_minReservedIds)
	{
		requestMoreObjectIds(m_maxReservedIds - numAvailableIds - numRequestedIds, false);
	}
	
	return NetworkId(theId);
}

void ObjectIdManager::requestMoreObjectIds(int howMany, bool logRequest, uint32 processId)
{
	DEBUG_REPORT_LOG(true,("Requesting %i object ID's\n",howMany));

	if (logRequest)
		LOG("ObjectIdManager", ("Requesting %d more object ids for pid %lu, numAvailableIds=%d, numRequestedIds=%d, m_minReservedIds=%d, m_maxReservedIds=%d, m_gotABlock=%s", howMany, ((processId == 0) ? GameServer::getInstance().getProcessId() : processId), numAvailableIds, numRequestedIds, m_minReservedIds, m_maxReservedIds, (m_gotABlock ? "yes" : "no")));

	RequestOIDsMessage const msg(processId==0?GameServer::getInstance().getProcessId():processId, howMany, logRequest);
	GameServer::getInstance().sendToCentralServer(msg);

	numRequestedIds+=howMany;
}

void ObjectIdManager::release()
{
	for (FreeListType::iterator i=m_freeList.begin(); i!=m_freeList.end(); ++i)
	{
		//@todo:  send release message
	}
}

void ObjectIdManager::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);
	if(message.isType("AddOIDBlockMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		AddOIDBlockMessage msg(ri);

		if (msg.getLogRequest())
			LOG("ObjectIdManager", ("Received object ids (%s - %s) for pid %lu", msg.getStart().getValueString().c_str(), msg.getEnd().getValueString().c_str(), msg.getServerId()));

		addBlock(msg.getStart().getValue(), msg.getEnd().getValue());
	}
	if(message.isType("CentralGameServerSetProcessId"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		CentralGameServerSetProcessId m(ri);
		uint32 const processId = m.getProcessId();

		if ((numAvailableIds+numRequestedIds) < m_minReservedIds)
		{
			LOG("ObjectIdManager", ("ObjectIdManager received CentralGameServerSetProcessId message for pid %lu, numAvailableIds=%d, numRequestedIds=%d, m_minReservedIds=%d, m_maxReservedIds=%d, m_gotABlock=%s, requesting more object ids", processId, numAvailableIds, numRequestedIds, m_minReservedIds, m_maxReservedIds, (m_gotABlock ? "yes" : "no")));
			requestMoreObjectIds(m_maxReservedIds - numAvailableIds - numRequestedIds, true, processId);
		}
		else
		{
			LOG("ObjectIdManager", ("ObjectIdManager received CentralGameServerSetProcessId message for pid %lu, numAvailableIds=%d, numRequestedIds=%d, m_minReservedIds=%d, m_maxReservedIds=%d, m_gotABlock=%s, ***NOT*** requesting more object ids?????", processId, numAvailableIds, numRequestedIds, m_minReservedIds, m_maxReservedIds, (m_gotABlock ? "yes" : "no")));
		}
	}
}

void ObjectIdManager::addBlock(NetworkId::NetworkIdType start, NetworkId::NetworkIdType end)
{
	DEBUG_REPORT_LOG(true,("Adding NetworkId block from %s to %s\n",NetworkId(start).getValueString().c_str(),NetworkId(end).getValueString().c_str()));
	m_gotABlock=true;
	m_freeList.push_back(FreeListEntry(start,end));
	numAvailableIds+=static_cast<int>(end-start+1); // No way this should be more than an int's worth of id's
	numRequestedIds-=static_cast<int>(end-start+1);
	if (numRequestedIds<0)
		numRequestedIds=0; // we may recieve more ID's than we asked for.  (This is allowed by design to avoid fragmentation issues.)

	if (m_freeList.size() == 1)
	{
		GameServer::getInstance().onReceivedFirstNetworkIdBlock();
	}
}

#if 0 // code for printing the freelist.  Commented out until we need it.

const std::string ObjectIdManager::debugDumpFreelist()
{
	std::string buffer="Object ID Freelist:\n";
	char temp[100];
	
	FreeListEntry *e=freeList;
	while (e)
	{
		sprintf(temp,"     Block:  %li to %li\n",e->start,e->end);
		buffer+=temp;
		e=e->next;
	}
	return buffer;
}

#endif
