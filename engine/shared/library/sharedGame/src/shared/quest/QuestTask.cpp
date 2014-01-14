// ======================================================================
//
// QuestTask.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/QuestTask.h"

#include "sharedUtility/DataTable.h"
#include "sharedGame/Quest.h"

// ======================================================================

namespace QuestTask_namespace
{
	std::string const cs_prerequisiteTasksColumn("PREREQUISITE_TASKS");
	std::string const cs_exclusionTasksColumn("EXCLUSION_TASKS");
	std::string const cs_attachScriptColumn("ATTACH_SCRIPT");
	std::string const cs_journalEntryTitleColumn("JOURNAL_ENTRY_TITLE");
	std::string const cs_journalEntryDescriptionColumn("JOURNAL_ENTRY_DESCRIPTION");
	std::string const cs_tasksOnCompleteColumn("TASKS_ON_COMPLETE");
	std::string const cs_tasksOnFailColumn("TASKS_ON_FAIL");
	std::string const cs_targetColumn("TARGET");
	std::string const cs_parameterColumn("PARAMETER");
	std::string const cs_visibleColumn("IS_VISIBLE");
	std::string const cs_showSystemMessagesColumn("SHOW_SYSTEM_MESSAGES");
	std::string const cs_allowRepeatsColumn("ALLOW_REPEATS");
	std::string const cs_musicOnActivateColumn("MUSIC_ON_ACTIVATE");
	std::string const cs_systemMessageOnActivateColumn("SYSTEM_MESSAGE_ON_ACTIVATE");
	std::string const cs_createWaypointColumn("CREATE_WAYPOINT");
	std::string const cs_planetNameColumn("PLANET_NAME");
	std::string const cs_locationXColumn("LOCATION_X");
	std::string const cs_locationYColumn("LOCATION_Y");
	std::string const cs_locationZColumn("LOCATION_Z");
	std::string const cs_waypointBuildingCellName("WAYPOINT_BUILDING_CELL_NAME");
	std::string const cs_entranceCreateWaypointColumn("CREATE_ENTRANCE_WAYPOINT");
	std::string const cs_entranceLocationXColumn("ENTRANCE_LOCATION_X");
	std::string const cs_entranceLocationYColumn("ENTRANCE_LOCATION_Y");
	std::string const cs_entranceLocationZColumn("ENTRANCE_LOCATION_Z");
	std::string const cs_chanceToActivate("CHANCE_TO_ACTIVATE");
	std::string const cs_questControlOnTaskCompletion("QUEST_CONTROL_ON_TASK_COMPLETION");
	std::string const cs_questControlOnTaskFailure("QUEST_CONTROL_ON_TASK_FAILURE");

	std::vector<int> * createIntVector(std::string const &source);
}

using namespace QuestTask_namespace;

// ======================================================================

QuestTask::QuestTask(Quest const & owner, DataTable const &sourceTable, int row) :
	m_owner(owner),
	m_id(row),
	m_prerequisiteTasks(createIntVector(sourceTable.getStringValue(cs_prerequisiteTasksColumn, row))),
	m_exclusionTasks(createIntVector(sourceTable.getStringValue(cs_exclusionTasksColumn, row))),
	m_attachScript(sourceTable.getStringValue(cs_attachScriptColumn, row)),
	m_journalEntryTitle(sourceTable.getStringValue(cs_journalEntryTitleColumn, row)),
	m_journalEntryDescription(sourceTable.getStringValue(cs_journalEntryDescriptionColumn, row)),
	m_tasksOnComplete(createIntVector(sourceTable.getStringValue(cs_tasksOnCompleteColumn, row))),
	m_tasksOnFail(createIntVector(sourceTable.getStringValue(cs_tasksOnFailColumn, row))),
	m_target(sourceTable.getStringValue(cs_targetColumn, row)),
	m_parameter(sourceTable.getStringValue(cs_parameterColumn, row)),
	m_visible(sourceTable.getIntValue(cs_visibleColumn, row) != 0),
	m_showSystemMessages(sourceTable.getIntValue(cs_showSystemMessagesColumn, row) != 0),
	m_allowRepeats(sourceTable.getIntValue(cs_allowRepeatsColumn, row) != 0),
	m_musicOnActivate(),
	m_systemMessageOnActivate(),
	m_createWaypoint(false),
	m_waypointPlanetName(),
	m_waypointLocation(),
	m_waypointBuildingCellName(),
	m_isInteriorWaypoint(false),
	m_entranceCreateWaypoint(false),
	m_waypointEntranceLocation(),
	m_chanceToActivate(0),
	m_questControlOnTaskCompletion(QC_none),
	m_questControlOnTaskFailure(QC_none)
{
	if(sourceTable.doesColumnExist(cs_musicOnActivateColumn))
	{
		m_musicOnActivate = sourceTable.getStringValue(cs_musicOnActivateColumn, row);
	}
	if(sourceTable.doesColumnExist(cs_systemMessageOnActivateColumn))
	{
		m_systemMessageOnActivate = StringId(sourceTable.getStringValue(cs_systemMessageOnActivateColumn, row));
	}
	if(sourceTable.doesColumnExist(cs_createWaypointColumn))
	{
		m_createWaypoint = sourceTable.getIntValue(cs_createWaypointColumn, row) != 0;
	}
	if(sourceTable.doesColumnExist(cs_planetNameColumn))
	{
		m_waypointPlanetName = sourceTable.getStringValue(cs_planetNameColumn, row);
	}
	if(sourceTable.doesColumnExist(cs_locationXColumn) && sourceTable.doesColumnExist(cs_locationYColumn) && sourceTable.doesColumnExist(cs_locationZColumn))
	{
		float const x = static_cast<float>(atof(sourceTable.getStringValue(cs_locationXColumn, row)));
		float const y = static_cast<float>(atof(sourceTable.getStringValue(cs_locationYColumn, row)));
		float const z = static_cast<float>(atof(sourceTable.getStringValue(cs_locationZColumn, row)));
		m_waypointLocation.set(x, y, z);
	}
	if(sourceTable.doesColumnExist(cs_waypointBuildingCellName))
	{
		m_waypointBuildingCellName = sourceTable.getStringValue(cs_waypointBuildingCellName, row);
	}
	m_isInteriorWaypoint = !m_waypointBuildingCellName.empty();

	if(sourceTable.doesColumnExist(cs_entranceCreateWaypointColumn))
	{
		m_entranceCreateWaypoint = sourceTable.getIntValue(cs_entranceCreateWaypointColumn, row) != 0;
	}
	if(sourceTable.doesColumnExist(cs_entranceLocationXColumn) && sourceTable.doesColumnExist(cs_entranceLocationYColumn) && sourceTable.doesColumnExist(cs_entranceLocationZColumn))
	{
		float const x = static_cast<float>(atof(sourceTable.getStringValue(cs_entranceLocationXColumn, row)));
		float const y = static_cast<float>(atof(sourceTable.getStringValue(cs_entranceLocationYColumn, row)));
		float const z = static_cast<float>(atof(sourceTable.getStringValue(cs_entranceLocationZColumn, row)));
		m_waypointEntranceLocation.set(x, y, z);
	}
	if (sourceTable.doesColumnExist(cs_chanceToActivate))
	{
		m_chanceToActivate = sourceTable.getIntValue(cs_chanceToActivate, row);
	}
	if (sourceTable.doesColumnExist(cs_questControlOnTaskCompletion))
	{
		m_questControlOnTaskCompletion = static_cast<QuestControl>(sourceTable.getIntValue(cs_questControlOnTaskCompletion, row));
	}
	if (sourceTable.doesColumnExist(cs_questControlOnTaskFailure))
	{
		m_questControlOnTaskFailure = static_cast<QuestControl>(sourceTable.getIntValue(cs_questControlOnTaskFailure, row));
	}
#if _DEBUG
	// Test for bad waypoints.
	if (m_isInteriorWaypoint) 
	{
		// Is an interior waypoint set at 0.0f?
		if (static_cast<int>(m_waypointLocation.y) == 0)
		{
			// The designer is using cell space and not world space for this location.
			DEBUG_REPORT_LOG(true, ("The waypoint for quest %s, task id %d, planet %s, location %4.2f, %4.2f, %4.2f is in an invalid location.\n", owner.getName().getString(), m_id, m_waypointPlanetName.c_str(), m_waypointLocation.x, m_waypointLocation.y, m_waypointLocation.z));
		}
	}
#endif
}

// ----------------------------------------------------------------------

QuestTask::~QuestTask()
{
	delete m_prerequisiteTasks;
	delete m_exclusionTasks;
	delete m_tasksOnComplete;
	delete m_tasksOnFail;
}

// ----------------------------------------------------------------------

bool QuestTask::isRepeatable() const
{
	return m_allowRepeats;
}

// ----------------------------------------------------------------------

QuestTask::TaskIdList const & QuestTask::getPrerequisiteTasks() const
{
	return *m_prerequisiteTasks;
}

// ----------------------------------------------------------------------

QuestTask::TaskIdList const & QuestTask::getExclusionTasks() const
{
	return *m_exclusionTasks;
}

// ----------------------------------------------------------------------

QuestTask::TaskIdList const & QuestTask::getTasksOnComplete() const
{
	return *m_tasksOnComplete;
}

// ----------------------------------------------------------------------

QuestTask::TaskIdList const & QuestTask::getTasksOnFail() const
{
	return *m_tasksOnFail;
}

// ----------------------------------------------------------------------

int QuestTask::getId() const
{
	return m_id;
}

// ----------------------------------------------------------------------

Quest const & QuestTask::getQuest() const
{
	return m_owner;
}

// ----------------------------------------------------------------------

StringId const & QuestTask::getJournalEntryTitle() const
{
	return m_journalEntryTitle;
}

// ----------------------------------------------------------------------

StringId const & QuestTask::getJournalEntryDescription() const
{
	return m_journalEntryDescription;
}

// ----------------------------------------------------------------------

const std::string & QuestTask::getAttachScript() const
{
	return m_attachScript;
}

// ----------------------------------------------------------------------

std::string const & QuestTask::getMusicOnActivate() const
{
	return m_musicOnActivate;
}

// ----------------------------------------------------------------------

StringId const &  QuestTask::getSystemMessageOnActivate() const
{
	return m_systemMessageOnActivate;
}

// ----------------------------------------------------------------------

bool QuestTask::isVisible() const
{
	return m_visible;
}

// ----------------------------------------------------------------------

Location QuestTask::getWaypointLocation() const
{
	int sceneCrc = Location::getCrcBySceneName(m_waypointPlanetName.c_str());
	return Location(m_waypointLocation, NetworkId::cms_invalid, sceneCrc);
}

// ----------------------------------------------------------------------

Location QuestTask::getWaypointEntranceLocation() const
{
	int sceneCrc = Location::getCrcBySceneName(m_waypointPlanetName.c_str());
	return Location(m_waypointEntranceLocation, NetworkId::cms_invalid, sceneCrc);
}

// ----------------------------------------------------------------------

int QuestTask::getChanceToActivate() const
{
	return m_chanceToActivate;
}

// ----------------------------------------------------------------------

bool QuestTask::questCompletesOnTaskCompletion() const
{
	return m_questControlOnTaskCompletion == QC_complete;
}

// ----------------------------------------------------------------------

bool QuestTask::questClearsOnTaskCompletion() const
{
	return m_questControlOnTaskCompletion == QC_clear;
}

// ----------------------------------------------------------------------

bool QuestTask::questCompletesOnTaskFailure() const
{
	return m_questControlOnTaskFailure == QC_complete;
}

// ----------------------------------------------------------------------

bool QuestTask::questClearsOnTaskFailure() const
{
	return m_questControlOnTaskFailure == QC_clear;
}

// ======================================================================

/**
 * Given a comma-separated list of numbers (in ASCII), allocate a new vector of those numbers
 */
std::vector<int> * QuestTask_namespace::createIntVector(std::string const &source)
{
	std::vector<int> * result = new std::vector<int>;

	size_t pos = 0;
	size_t commaPos = source.find(',', pos);
	while (commaPos != std::string::npos)
	{
		result->push_back(atoi(source.c_str()+pos));
		pos = commaPos+1;
		commaPos = source.find(',', pos);
	}
	if (pos < source.size())
		result->push_back(atoi(source.c_str()+pos));
	
	return result;
}
// ======================================================================

