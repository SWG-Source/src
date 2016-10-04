// ======================================================================
//
// LazyDeleter.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/LazyDeleter.h"

#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/LazyDeleteQuery.h"
#include "serverDatabase/Loader.h"
#include "serverDatabase/Persister.h"
#include "sharedDatabaseInterface/DbException.h"
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedSynchronization/Mutex.h"
#include "sharedThread/RunThread.h"
#include "sharedThread/Thread.h"
#include "sharedLog/Log.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include <deque>
#include <vector>

// ======================================================================

LazyDeleter * LazyDeleter::ms_instance=nullptr;

// ======================================================================

/**
 * Add an object to be purged from the database when time allows
 * These objects are put on a separate queue then copied to m_objectsToDelete
 * to avoid having to acquire and release the lock for each object
 */
void LazyDeleter::addObject(const NetworkId &objectId)
{
	m_incomingObjects->push_back(objectId);
	++m_totalObjectCount;
}

// ----------------------------------------------------------------------

void LazyDeleter::update(float updateTime)
{
	UNREF(updateTime);
	
	if (!m_incomingObjects->empty())
	{
		m_objectListLock->enter();
		
		for (std::vector<NetworkId>::iterator i=m_incomingObjects->begin(); i!=m_incomingObjects->end(); ++i)
			m_objectsToDelete->push_back(*i);
		m_incomingObjects->clear();

		m_objectListLock->leave();
	}

	if (Persister::getInstance().isSaveInProgress() || Loader::getInstance().isBacklogged() || (Loader::getInstance().getNumPreloads() != 0))
		m_paused=true;
	else
		m_paused=false;
}

// ----------------------------------------------------------------------

void LazyDeleter::install()
{
	DEBUG_FATAL(ms_instance,("LazyDeleter::install was called twice."));
	ExitChain::add(&remove,"LazyDeleter::remove");
	ms_instance = new LazyDeleter;
}

// ----------------------------------------------------------------------

void LazyDeleter::remove()
{
	NOT_NULL(ms_instance);
	delete ms_instance;
	ms_instance = nullptr;
}

// ----------------------------------------------------------------------

LazyDeleter::LazyDeleter() :
		m_workerThread(nullptr), // Avoid starting the worker thread until the other member variables are initialized
		m_incomingObjects(new std::vector<NetworkId>),
		m_objectsToDelete(new std::deque<NetworkId>),
		m_objectListLock(new Mutex),
		m_shutdown(false),
		m_paused(false),
		m_batchSize(10),
		m_session(DatabaseProcess::getInstance().getDBServer()->getSession()),
		m_totalObjectCount(0)
{
	m_workerThread = new MemberFunctionThreadZero<LazyDeleter>("LazyDeleter", *this, &LazyDeleter::workerThreadLoop);
	ThreadHandle tempThreadHandle(m_workerThread); // for some obscure reason, the thread doesn't run unless you create a handle, but we don't need the handle for anything
	UNREF(tempThreadHandle);
}

// ----------------------------------------------------------------------

LazyDeleter::~LazyDeleter()
{
	m_shutdown = true;
	m_workerThread->wait();
	DatabaseProcess::getInstance().getDBServer()->releaseSession(m_session);
	
	delete m_incomingObjects;
	delete m_objectsToDelete;
	delete m_objectListLock;
	 
	m_workerThread = nullptr;
	m_incomingObjects = nullptr;
	m_objectsToDelete = nullptr;
	m_objectListLock = nullptr;
	m_session = nullptr;
}

// ----------------------------------------------------------------------

void LazyDeleter::workerThreadLoop()
{
	bool done = false;
	int size = 0;
	bool succeeded;
	
	while (!done)
	{
		if ((!m_paused) || m_shutdown)
		{
			m_objectListLock->enter();

			size = m_objectsToDelete->size();
			
			if (size!=0)
			{
				LazyDeleteQuery qry;
				std::vector<NetworkId> batch;
			
				for (int c=0; !m_objectsToDelete->empty() && c < m_batchSize; ++c)
				{
					NetworkId &object = m_objectsToDelete->front();
					batch.push_back(object);
					m_objectsToDelete->pop_front();
				}

				m_objectListLock->leave();

				succeeded = true;
				if (!qry.setupData(m_session))
					succeeded = false;
				else
				{
					for (std::vector<NetworkId>::iterator i=batch.begin(); i!=batch.end(); ++i)
					{
						if (!qry.addData(*i))
						{
							succeeded = false;
							break;
						}
						if (qry.getNumItems() == ConfigServerDatabase::getDefaultLazyDeleteBulkBindSize())
						{	
							if (! (m_session->exec(&qry)))
							{
								succeeded = false;
								break;
							}
							qry.clearData();
							qry.done();
							m_session->commitTransaction();
						}
					}
				}

				if (succeeded && (qry.getNumItems() != 0))
					succeeded = m_session->exec(&qry);
				qry.done();
				qry.freeData();
				if (succeeded)
					m_session->commitTransaction();
				else
				{
					m_session->rollbackTransaction();
					m_session->disconnect();
					while (! (m_session->connect()))
					{
						m_session->disconnect();
						Os::sleep(DB::Server::getDisconnectSleepTime());
					};
					LOG("DatabaseError", ("Database connection reestablished"));
					m_objectListLock->enter();
					m_objectsToDelete->insert(m_objectsToDelete->begin(),batch.begin(),batch.end()); // retry this batch
					m_objectListLock->leave();
				}
			}
			else
			{
				m_objectListLock->leave();
				if (m_shutdown)
					done = true;
			}
		}
		if (!m_shutdown)
		{
			if ( size > (ConfigServerDatabase::getDefaultLazyDeleteBulkBindSize()))
				Os::sleep(ConfigServerDatabase::getDefaultLazyDeleteSleepTime()); // just multi-batch bulk delete so wait a bit to start again
			else
				Os::sleep(10);
			
		}
	}
}

// ----------------------------------------------------------------------

size_t LazyDeleter::getQueueSize() const
{
	size_t result;
	m_objectListLock->enter();
	result = m_objectsToDelete->size();
	m_objectListLock->leave();

	return result;
}

// ======================================================================
