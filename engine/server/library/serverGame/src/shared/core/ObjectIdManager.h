// ======================================================================
//
// ObjectIdManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectIdManager_H
#define INCLUDED_ObjectIdManager_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"
#include "Singleton/Singleton.h"

#include <list>

// ======================================================================

/**
 * Keeps a list of available object id's.  If the list gets
 * too small, requests more from centralServer
 */
class ObjectIdManager : public Singleton<ObjectIdManager>, public MessageDispatch::Receiver
{
  public:
/* 	enum //@todo Should be config file options */
/* 	  { */
/* 		  minReservedIds=500,	 // How small can the freelist get before we ask for more */
/* 		  targetReservedIds=1000 // The maximum number of ID's we'd like to have in the */
/* 								 // freelist.  Whenever we go below minNumIds, we load enough */
/* 								 // to bring it up to this. */
/* 	  }; */

	ObjectIdManager();
	~ObjectIdManager();

	virtual void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	
	static NetworkId const getNewObjectId();
	static bool hasAvailableObjectId();
	
  private:
	NetworkId const getNewObjectIdInternal();

	struct FreeListEntry
	{
		NetworkId::NetworkIdType m_start;
		NetworkId::NetworkIdType m_end;
		FreeListEntry(NetworkId::NetworkIdType start, NetworkId::NetworkIdType end);
		FreeListEntry() {} // to make STL happy
	};
	typedef std::list<FreeListEntry> FreeListType;
	FreeListType m_freeList;

	/** How many ID's are in the freelist
	*/
	int numAvailableIds;
	/**
	 * How many ID's have we already requested.
	 * We use this to avoid sending extra requests
	 * while we're waiting for a response.
	 */
	int numRequestedIds;

	int m_minReservedIds;    // How small can the freelist get before we ask for more
	int m_maxReservedIds;    // The maximum number of ID's we'd like to have in the freelist
	bool m_gotABlock;        // True if we ever received a block (for debugging)

  private:
	friend class ServerObjectLint;
	void requestMoreObjectIds(int howMany, bool logRequest, uint32 processId=0);
	void release();
	
	void addBlock(NetworkId::NetworkIdType start, NetworkId::NetworkIdType end);

	// const std::string debugDumpFreelist();
	ObjectIdManager(const ObjectIdManager&);  //disable
	ObjectIdManager &operator=(const ObjectIdManager&); //disable

};

// ----------------------------------------------------------------------

inline ObjectIdManager::FreeListEntry::FreeListEntry(NetworkId::NetworkIdType start, NetworkId::NetworkIdType end) :
	m_start(start),
	m_end(end)
{
}

// ----------------------------------------------------------------------

inline bool ObjectIdManager::hasAvailableObjectId()
{
	return (getInstance().numAvailableIds > 0);
}

// ======================================================================

#endif
