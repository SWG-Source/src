// ======================================================================
//
// PlayerQuestData.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlayerQuestData_H
#define INCLUDED_PlayerQuestData_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"

// ======================================================================

class PlayerQuestData;

namespace Archive
{
	class ByteStream;
	class ReadIterator;
	void get(ReadIterator & source, PlayerQuestData & target);
	void put(ByteStream & target, const PlayerQuestData & source);
}

// ======================================================================

/**
 * A class that tracks an active quest for a player
 */
class PlayerQuestData
{
  public:
	PlayerQuestData ();
	explicit PlayerQuestData (NetworkId const & questGiver);
	PlayerQuestData (NetworkId const & questGiver, uint16 activeTasks, uint16 completedTasks, bool hasReceivedReward);
	PlayerQuestData (bool completed, bool receivedReward);
	~PlayerQuestData();

	bool operator!=(PlayerQuestData const & rhs) const;
	void packToString(std::string & buffer) const;
	void unpackFromString(std::string const & buffer);

	NetworkId const & getQuestGiver() const;
	uint16 getActiveTasks() const;
	uint16 getCompletedTasks() const;
	bool   hasReceivedReward() const;
	void   setHasReceivedReward(bool hasReceivedReward);

	bool isCompleted() const;
	
	bool isTaskActive(int taskId) const;
	bool isTaskCompleted(int taskId) const;

	void complete();
	void activateTask(int taskId);
	void completeTask(int taskId);
	void clearTask(int taskId);

	uint32 getRelativeAgeIndex() const;

  private:
	uint16 getTaskMask(int taskId) const;
	
  private:
	// We need to keep track of the relative age of Quest data
	static uint32 ms_lastRelativeAgeIndex;

	NetworkId m_questGiver;
	bool   m_completed;
	uint16 m_activeTasks;
	uint16 m_completedTasks;
	uint32 m_relativeAgeIndex;  // This is not meant to be persisted
	bool   m_hasReceivedReward;

	friend void Archive::get(Archive::ReadIterator & source, PlayerQuestData & target);
	friend void Archive::put(Archive::ByteStream & target, const PlayerQuestData & source);
};

// ======================================================================

#endif
