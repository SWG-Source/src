// ======================================================================
//
// QuestTask.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_QuestTask_H
#define INCLUDED_QuestTask_H

// ======================================================================

#include "StringId.h"
#include "sharedUtility/Location.h"

class DataTable;
class Quest;

// ======================================================================

/**
 * A subtask that is part of a quest
 */
class QuestTask
{
public:
	typedef std::vector<int> TaskIdList;

	enum QuestControl
	{
		QC_none = 0,
		QC_complete = 1,
		QC_clear = 2
	};
	
public:
	QuestTask(Quest const & owner, DataTable const &sourceTable, int row);
	~QuestTask();
	bool isRepeatable() const;
	TaskIdList const & getPrerequisiteTasks() const;
	TaskIdList const & getExclusionTasks() const;
	TaskIdList const & getTasksOnComplete() const;
	TaskIdList const & getTasksOnFail() const;
	Quest const & getQuest() const;
	StringId const & getJournalEntryTitle() const;
	StringId const & getJournalEntryDescription() const;
	const std::string &  getAttachScript() const;
	int getId() const;
	std::string const & getMusicOnActivate() const;
	StringId const & getSystemMessageOnActivate() const;
	bool isVisible() const;
	Location getWaypointLocation() const;
	bool hasWaypoint() const;
	bool hasEntranceWaypoint() const;
	bool isInteriorWaypoint() const;
	std::string const & getWaypointBuildingCellName() const;
	Location getWaypointEntranceLocation() const;
	int getChanceToActivate() const;
	bool questCompletesOnTaskCompletion() const;
	bool questClearsOnTaskCompletion() const;
	bool questCompletesOnTaskFailure() const;
	bool questClearsOnTaskFailure() const;

  private:
	Quest const & m_owner;
	int const m_id;
	TaskIdList * const m_prerequisiteTasks;
	TaskIdList * const m_exclusionTasks;
	std::string const m_attachScript;
	StringId const m_journalEntryTitle;
	StringId const m_journalEntryDescription;
	TaskIdList * const m_tasksOnComplete;
	TaskIdList * const m_tasksOnFail;
	std::string const m_target;
	std::string const m_parameter;
	bool const m_visible;
	bool const m_showSystemMessages;
	bool const m_allowRepeats;
	std::string m_musicOnActivate;
	StringId m_systemMessageOnActivate;
	bool m_createWaypoint;
	std::string m_waypointPlanetName;
	Vector m_waypointLocation;
	std::string m_waypointBuildingCellName;
	bool m_isInteriorWaypoint;
	bool m_entranceCreateWaypoint;
	Vector m_waypointEntranceLocation;
	int m_chanceToActivate;
	QuestControl m_questControlOnTaskCompletion;
	QuestControl m_questControlOnTaskFailure;

  private:
	QuestTask();
	QuestTask(QuestTask const &);
	QuestTask & operator=(QuestTask const &);
};

// ----------------------------------------------------------------------

inline bool QuestTask::hasWaypoint() const
{
	return m_createWaypoint;
}

// ----------------------------------------------------------------------

inline bool QuestTask::hasEntranceWaypoint() const
{
	return m_entranceCreateWaypoint;
}

// ----------------------------------------------------------------------
inline bool QuestTask::isInteriorWaypoint() const
{
	return m_isInteriorWaypoint;
}

// ----------------------------------------------------------------------

inline std::string const & QuestTask::getWaypointBuildingCellName() const
{
	return m_waypointBuildingCellName;
}

// ======================================================================

#endif

