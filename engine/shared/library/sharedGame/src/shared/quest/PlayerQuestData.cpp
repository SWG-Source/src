// ======================================================================
//
// PlayerQuestData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/PlayerQuestData.h"

#include "Archive/Archive.h"
#include "Archive/AutoDeltaPackedMap.h"
#include "sharedFoundation/NetworkIdArchive.h"

#include <list>

// ======================================================================

uint32 PlayerQuestData::ms_lastRelativeAgeIndex = 0;

// ======================================================================

PlayerQuestData::PlayerQuestData () :
		m_questGiver(NetworkId::cms_invalid),
		m_completed(false),
		m_activeTasks(0),
		m_completedTasks(0),
		m_relativeAgeIndex(++ms_lastRelativeAgeIndex),
		m_hasReceivedReward(false)
{
}

// ----------------------------------------------------------------------

PlayerQuestData::PlayerQuestData(bool const completed, bool const receivedReward) :
		m_questGiver(NetworkId::cms_invalid),
		m_completed(completed),
		m_activeTasks(0),
		m_completedTasks(0),
		m_relativeAgeIndex(++ms_lastRelativeAgeIndex),
		m_hasReceivedReward(receivedReward)
{
}

// ----------------------------------------------------------------------

PlayerQuestData::PlayerQuestData(NetworkId const & questGiver) :
		m_questGiver(questGiver),
		m_completed(false),
		m_activeTasks(0),
		m_completedTasks(0),
		m_relativeAgeIndex(++ms_lastRelativeAgeIndex),
		m_hasReceivedReward(false)
{
}

// ----------------------------------------------------------------------

PlayerQuestData::PlayerQuestData(NetworkId const & questGiver, uint16 const activeTasks, uint16 const completedTasks, bool receivedReward) :
		m_questGiver(questGiver),
		m_completed(false),
		m_activeTasks(activeTasks),
		m_completedTasks(completedTasks),
		m_relativeAgeIndex(++ms_lastRelativeAgeIndex),
		m_hasReceivedReward(receivedReward)
{
}

// ----------------------------------------------------------------------

PlayerQuestData::~PlayerQuestData()
{
}

// ----------------------------------------------------------------------

bool PlayerQuestData::operator!=(PlayerQuestData const & rhs) const
{
	return (m_questGiver != rhs.m_questGiver) ||
		(m_activeTasks != rhs.m_activeTasks) || 
		(m_completedTasks != rhs.m_completedTasks) ||
		(m_completed != rhs.m_completed) ||
		(m_hasReceivedReward != rhs.m_hasReceivedReward);
}

// ----------------------------------------------------------------------

NetworkId const & PlayerQuestData::getQuestGiver() const
{
	return m_questGiver;
}

// ----------------------------------------------------------------------

uint16 PlayerQuestData::getActiveTasks() const
{
	return m_activeTasks;
}

// ----------------------------------------------------------------------

uint16 PlayerQuestData::getCompletedTasks() const
{
	return m_completedTasks;
}

// ----------------------------------------------------------------------

bool PlayerQuestData::hasReceivedReward() const
{
	return m_hasReceivedReward;
}

// ----------------------------------------------------------------------

void PlayerQuestData::setHasReceivedReward(bool const hasReceivedReward)
{
	m_hasReceivedReward = hasReceivedReward;
}

// ----------------------------------------------------------------------

bool PlayerQuestData::isTaskActive(int const taskId) const
{
	return (!m_completed && ((m_activeTasks & getTaskMask(taskId)) != 0));
}

// ----------------------------------------------------------------------

bool PlayerQuestData::isTaskCompleted(int const taskId) const
{
	return (m_completed || ((m_completedTasks & getTaskMask(taskId)) != 0));
}

// ----------------------------------------------------------------------

void PlayerQuestData::activateTask(int const taskId)
{
	m_completedTasks &= ~getTaskMask(taskId);
	m_activeTasks |= getTaskMask(taskId);

	// Refresh the relative age of this quest since its data has changed
	m_relativeAgeIndex = (++ms_lastRelativeAgeIndex);
}

// ----------------------------------------------------------------------

void PlayerQuestData::completeTask(int const taskId)
{
	m_activeTasks &= ~getTaskMask(taskId);
	m_completedTasks |= getTaskMask(taskId);

	// Refresh the relative age of this quest since its data has changed
	m_relativeAgeIndex = (++ms_lastRelativeAgeIndex);
}

// ----------------------------------------------------------------------

void PlayerQuestData::clearTask(int const taskId)
{
	m_activeTasks &= ~getTaskMask(taskId);
	m_completedTasks &= ~getTaskMask(taskId);

	// Refresh the relative age of this quest since its data has changed
	m_relativeAgeIndex = (++ms_lastRelativeAgeIndex);
}

// ----------------------------------------------------------------------

uint16 PlayerQuestData::getTaskMask(int const taskId) const
{
	DEBUG_FATAL(taskId < 0 || taskId > static_cast<int>(sizeof(m_activeTasks) * 8),("PlayerQuestData::getTaskMask called with task id %i, which is not in the valid range for a task id.", taskId));
	return static_cast<uint16>(1 << taskId);
}

// ----------------------------------------------------------------------

void PlayerQuestData::complete()
{
	m_completed=true;

	// Refresh the relative age of this quest since its data has changed
	m_relativeAgeIndex = (++ms_lastRelativeAgeIndex);
}

// ----------------------------------------------------------------------

bool PlayerQuestData::isCompleted() const
{
	return m_completed;
}

// ----------------------------------------------------------------------

uint32 PlayerQuestData::getRelativeAgeIndex() const
{
	return m_relativeAgeIndex;
}

// ======================================================================

namespace Archive
{
	// Class to help us sort quest data by age
	class QuestDataToSort
	{
	public:
		QuestDataToSort() :
			m_key(0)
		{}

		QuestDataToSort( uint32 key, PlayerQuestData const& questData ) :
			m_key(key),
			m_questData(questData)
		{}

		bool operator<(QuestDataToSort const & rhs) const
		{
			return (m_questData.getRelativeAgeIndex() < rhs.m_questData.getRelativeAgeIndex());
		}

		uint32                 GetPlayerQuestDataKey() const { return m_key; }
		PlayerQuestData const& GetPlayerQuestData() const    { return m_questData; }

	private:
		uint32          m_key;
		PlayerQuestData m_questData;
	};

	typedef std::list<QuestDataToSort> SortableQuestList;

	//----------------------------------------------------------------------

	class ByteStream;
	class ReadIterator;
	void get(ReadIterator & source, PlayerQuestData & target)
	{
		get(source,target.m_questGiver);
		get(source,target.m_activeTasks);
		get(source,target.m_completedTasks);
		get(source,target.m_completed);
		get(source,target.m_relativeAgeIndex);
		get(source,target.m_hasReceivedReward);
	}
	
	void put(ByteStream & target, const PlayerQuestData & source)
	{
		put(target,source.m_questGiver);
		put(target,source.m_activeTasks);
		put(target,source.m_completedTasks);
		put(target,source.m_completed);
		put(target,source.m_relativeAgeIndex);
		put(target,source.m_hasReceivedReward);
	}

	template<>
	void Archive::AutoDeltaPackedMap<uint32,PlayerQuestData>::unpack(ReadIterator & source, std::string & buffer)
	{
		char temp[200];
		
		Command c;
		size_t commandCount;
		size_t baselineCommandCount; //lint !e578 hides a symbol elsewhere

		Archive::get(source, commandCount);
		Archive::get(source, baselineCommandCount);

		if (commandCount==0)
		{
			buffer=' '; // An empty map is represented as a single space, because a completely empty string is used to mean "no change"
		}
		else
		{
			// We need to sort the quests by age
			SortableQuestList sortedQuests;

			for (size_t i = 0; i < commandCount; ++i)
			{
				Archive::get(source, c.cmd);
				Archive::get(source, c.key);
				Archive::get(source, c.value);

				sortedQuests.push_back(QuestDataToSort(c.key, c.value));
			}

			// Sort the quests
			sortedQuests.sort();

			// Output the quests as strings
			for ( SortableQuestList::const_iterator j = sortedQuests.begin(); j != sortedQuests.end(); ++j )
			{
				QuestDataToSort const& questDataToSort    = (*j);
				uint32 const           playerQuestDataKey = questDataToSort.GetPlayerQuestDataKey();
				PlayerQuestData const& playerQuestData    = questDataToSort.GetPlayerQuestData();

				if (playerQuestData.isCompleted())
				{
					IGNORE_RETURN(snprintf(temp, sizeof(temp)-1, "%lu %hi:", playerQuestDataKey, playerQuestData.hasReceivedReward() ? static_cast<uint16>(1) : static_cast<uint16>(0)));
				}
				else
				{
					IGNORE_RETURN(snprintf(temp, sizeof(temp)-1, "%lu %hi %hi %s:", playerQuestDataKey, playerQuestData.getActiveTasks(), playerQuestData.getCompletedTasks(), playerQuestData.getQuestGiver().getValueString().c_str()));
				}
			
				temp[sizeof(temp)-1]='\0';
				buffer+=temp;
			}
		}
	}

	template<>
	void Archive::AutoDeltaPackedMap<uint32,PlayerQuestData>::pack(ByteStream & target, std::string const & buffer)
	{
		char temp[200];
		AutoDeltaMap<uint32, PlayerQuestData>::Command c;

		Archive::put(target, countCharacter(buffer,':'));
		Archive::put(target, static_cast<size_t>(0)); // baselineCommandCount
		
		int tempPos = 0;
		for (std::string::const_iterator i=buffer.begin(); i!=buffer.end(); ++i)
		{
			if (*i==':')
			{
				uint16 activeTasks = 0;
				uint16 completedTasks = 0;
				char giver[100];
				temp[tempPos]='\0';
				int const numScanned = sscanf(temp,"%lu %hi %hi %s", &c.key, &activeTasks, &completedTasks, giver);
				
				//active quests
				switch (numScanned) {
				  case 4:
					{
				    		// in-progress quest
				    		Archive::put(target, static_cast<unsigned char>(Command::ADD));
				    		Archive::put(target, c.key);
				    		Archive::put(target, PlayerQuestData(NetworkId(giver), activeTasks, completedTasks, false));
					}
				  break;
				  case 1:
					{
				    		// completed quest
				    		Archive::put(target, static_cast<unsigned char>(Command::ADD));
				    		Archive::put(target, c.key);
				    		Archive::put(target, PlayerQuestData(true, true));
					}
				  break;
				  case 2: 
					{
						// completed quest
						Archive::put(target, static_cast<unsigned char>(Command::ADD));
						Archive::put(target, c.key);

						//this is really a flags field
						uint16 const flags = activeTasks;
						bool const hasReceivedReward = (flags != 0);
						Archive::put(target, PlayerQuestData(true, hasReceivedReward));
					}
				  break;
				  default:
					FATAL(true,("Could not parse packed quest data %s,",temp));
				  break;
				}
				
				tempPos=0;
			}
			else
			{
				temp[tempPos++]=*i;
			}
		}
	}
}

// ======================================================================
