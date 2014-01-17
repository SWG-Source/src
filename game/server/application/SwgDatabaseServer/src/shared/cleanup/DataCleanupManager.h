// ======================================================================
//
// DataCleanupManager.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DataCleanupManager_H
#define INCLUDED_DataCleanupManager_H

// ======================================================================

namespace DB
{
	class TaskQueue;
}

// ======================================================================

/**
 * Class to run cleanup scripts on the database before starting
 * the cluster.  This class can be deleted when its work is complete.
 */
class DataCleanupManager
{
  public:
	DataCleanupManager();
	~DataCleanupManager();

  public:
	void runDailyCleanup( );

  private:
	DB::TaskQueue *  m_taskQueue;
};

// ======================================================================

#endif
