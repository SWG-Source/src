// ======================================================================
//
// LazyDeleter.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LazyDeleter_H
#define INCLUDED_LazyDeleter_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"

// ======================================================================

namespace DB
{
	class Session;
}

class Thread;
class Mutex;

// ======================================================================

/**
 * Singleton that purges deleted objects from the database.
 */
class LazyDeleter
{
  public:
	static LazyDeleter &getInstance();
	static void install();

	void addObject(const NetworkId &objectId);
	void update(float updateTime);
	size_t getQueueSize() const;
	int getTotalObjectCount() const;
	
  private:
	void workerThreadLoop();
	
  private:
	static void remove();
	LazyDeleter();
	~LazyDeleter();
	
  private:	
	LazyDeleter(const LazyDeleter&); //disable
	LazyDeleter &operator=(const LazyDeleter&); //disable
	
  private:
	static LazyDeleter *ms_instance;
	Thread *m_workerThread;
	std::vector<NetworkId> *m_incomingObjects;
	std::deque<NetworkId> *m_objectsToDelete;
	Mutex *m_objectListLock;
	bool m_shutdown;
	bool m_paused;
	int m_batchSize;
	DB::Session *m_session;
	int m_totalObjectCount;
};

// ----------------------------------------------------------------------

inline LazyDeleter &LazyDeleter::getInstance()
{
	NOT_NULL(ms_instance);
	return *ms_instance;
}

// ----------------------------------------------------------------------

inline int LazyDeleter::getTotalObjectCount() const
{
	return m_totalObjectCount;
}

// ======================================================================

#endif
