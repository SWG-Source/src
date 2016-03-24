 // ======================================================================
//
// Quest.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/Quest.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/QuestTask.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <algorithm>
#include <map>

// ======================================================================

namespace Quest_Namespace
{
	std::string const cs_summaryTablePrefix("datatables/questlist/");
	std::string const cs_taskTablePrefix("datatables/questtask/");
	std::string const cs_journalEntryTitleColumn("JOURNAL_ENTRY_TITLE");
	std::string const cs_journalEntryDescriptionColumn("JOURNAL_ENTRY_DESCRIPTION");
	std::string const cs_prerequisiteQuestsColumn("PREREQUISITE_QUESTS");
	std::string const cs_exclusionQuestsColumn("EXCLUSION_QUESTS");
	std::string const cs_allowRepeatsColumn("ALLOW_REPEATS");
	std::string const cs_visibleColumn("VISIBLE");
	std::string const cs_categoryColumn("CATEGORY");
	std::string const cs_completeQuestWhenTasksCompleteColumn("COMPLETE_WHEN_TASKS_COMPLETE");
	std::string const cs_levelColumn("LEVEL");
	std::string const cs_tierColumn("TIER");
	std::string const cs_typeColumn("TYPE");
	std::string const cs_experienceRewardTypeColumn("QUEST_REWARD_EXPERIENCE_TYPE");
	std::string const cs_experienceRewardAmountColumn("QUEST_REWARD_EXPERIENCE_AMOUNT");
	std::string const cs_factionRewardTypeColumn("QUEST_REWARD_FACTION_NAME");
	std::string const cs_factionRewardAmountColumn("QUEST_REWARD_FACTION_AMOUNT");
	std::string const cs_moneyRewardCreditsColumn("QUEST_REWARD_BANK_CREDITS");
	std::string const cs_journalEntryCompetionSummaryColumn("JOURNAL_ENTRY_COMPLETION_SUMMARY");
	std::string const cs_inclusiveLootNameColumn("QUEST_REWARD_LOOT_NAME");
	std::string const cs_inclusiveLootCountColumn("QUEST_REWARD_LOOT_COUNT");
	std::string const cs_inclusiveLootNameColumn2("QUEST_REWARD_LOOT_NAME_2");
	std::string const cs_inclusiveLootCountColumn2("QUEST_REWARD_LOOT_COUNT_2");
	std::string const cs_inclusiveLootNameColumn3("QUEST_REWARD_LOOT_NAME_3");
	std::string const cs_inclusiveLootCountColumn3("QUEST_REWARD_LOOT_COUNT_3");
	std::string const cs_exclusiveLootNameColumn("QUEST_REWARD_EXCLUSIVE_LOOT_NAME");
	std::string const cs_exclusiveLootCountColumn("QUEST_REWARD_EXCLUSIVE_LOOT_COUNT");
	std::string const cs_exclusiveLootNameColumn2("QUEST_REWARD_EXCLUSIVE_LOOT_NAME_2");
	std::string const cs_exclusiveLootCountColumn2("QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_2");
	std::string const cs_exclusiveLootNameColumn3("QUEST_REWARD_EXCLUSIVE_LOOT_NAME_3");
	std::string const cs_exclusiveLootCountColumn3("QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_3");
	std::string const cs_exclusiveLootNameColumn4("QUEST_REWARD_EXCLUSIVE_LOOT_NAME_4");
	std::string const cs_exclusiveLootCountColumn4("QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_4");
	std::string const cs_exclusiveLootNameColumn5("QUEST_REWARD_EXCLUSIVE_LOOT_NAME_5");
	std::string const cs_exclusiveLootCountColumn5("QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_5");
	std::string const cs_exclusiveLootNameColumn6("QUEST_REWARD_EXCLUSIVE_LOOT_NAME_6");
	std::string const cs_exclusiveLootCountColumn6("QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_6");
	std::string const cs_exclusiveLootNameColumn7("QUEST_REWARD_EXCLUSIVE_LOOT_NAME_7");
	std::string const cs_exclusiveLootCountColumn7("QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_7");
	std::string const cs_exclusiveLootNameColumn8("QUEST_REWARD_EXCLUSIVE_LOOT_NAME_8");
	std::string const cs_exclusiveLootCountColumn8("QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_8");
	std::string const cs_exclusiveLootNameColumn9("QUEST_REWARD_EXCLUSIVE_LOOT_NAME_9");
	std::string const cs_exclusiveLootCountColumn9("QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_9");
	std::string const cs_exclusiveLootNameColumn10("QUEST_REWARD_EXCLUSIVE_LOOT_NAME_10");
	std::string const cs_exclusiveLootCountColumn10("QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_10");

	char const * const cs_questExperienceDataTableFile = "datatables/quest/quest_experience.iff";
	char const * const cs_questExperienceDataTableLevelColumn = "LEVEL";
	char const * const cs_questExperienceDataTableTierColumns[] =
	{
		"TIER_1_EXPERIENCE",
		"TIER_2_EXPERIENCE",
		"TIER_3_EXPERIENCE",
		"TIER_4_EXPERIENCE",
		"TIER_5_EXPERIENCE",
		"TIER_6_EXPERIENCE"
	};

	int const cs_minTierValue = -1;
	int const cs_maxTierValue = 6;

	void getCrcVector(std::string const &source, std::vector<uint32> & crcs);
}

using namespace Quest_Namespace;

// ======================================================================

Quest::Quest(CrcString const & name) :
	m_name(name),
	m_journalEntryTitle(),
	m_journalEntryDescription(),
	m_journalEntryCompetionSummary(),
	m_prerequisiteQuests(new std::vector<uint32>),
	m_exclusionQuests(new std::vector<uint32>),
	m_tasks(new std::vector<QuestTask *>),
	m_allowRepeats(false),
	m_visible(true),
	m_category(),
	m_completeQuestWhenTasksComplete(false),
	m_experienceRewardType(),
	m_experienceRewardAmount(0),
	m_factionRewardType(),
	m_factionRewardAmount(0),
	m_moneyRewardCredits(0),
	m_inclusiveLootItemNames(),
	m_inclusiveLootItemCounts(),
	m_exclusiveLootItemNames(),
	m_exclusiveLootItemCounts()
{
	//-- Summary Data

	Iff tableIff;
	if (tableIff.open((cs_summaryTablePrefix + m_name.getString() + ".iff").c_str(), true))
	{
		DataTable summaryTable;
		summaryTable.load(tableIff);
		DEBUG_FATAL(summaryTable.getNumRows()!=1,("Data bug:  Data table %s had %i rows, expected it to have 1.", (cs_summaryTablePrefix + m_name.getString()).c_str(), summaryTable.getNumRows()));
		m_journalEntryTitle = StringId(summaryTable.getStringValue(cs_journalEntryTitleColumn,0));
		m_journalEntryDescription = StringId(summaryTable.getStringValue(cs_journalEntryDescriptionColumn,0));
		getCrcVector(summaryTable.getStringValue(cs_prerequisiteQuestsColumn,0), *m_prerequisiteQuests);
		getCrcVector(summaryTable.getStringValue(cs_exclusionQuestsColumn,0), *m_exclusionQuests);
		m_allowRepeats = (summaryTable.getIntValue(cs_allowRepeatsColumn,0) != 0);
		m_category = StringId(summaryTable.getStringValue(cs_categoryColumn,0));

		if(summaryTable.doesColumnExist(cs_levelColumn))
			m_level = summaryTable.getIntValue(cs_levelColumn,0);
		else
			m_level = 0;

		if(summaryTable.doesColumnExist(cs_tierColumn))
		{
			m_tier = summaryTable.getIntValue(cs_tierColumn,0);
			clamp(cs_minTierValue, m_tier, cs_maxTierValue);
		}
		else
			m_tier = -1;

		if(summaryTable.doesColumnExist(cs_typeColumn))
			m_type = summaryTable.getStringValue(cs_typeColumn,0);
		else
			m_type.clear();

		if(summaryTable.doesColumnExist(cs_visibleColumn))
			m_visible = (summaryTable.getIntValue(cs_visibleColumn,0) != 0);
		else
			m_visible = true;

		if(summaryTable.doesColumnExist(cs_experienceRewardTypeColumn))
			m_experienceRewardType = summaryTable.getStringValue(cs_experienceRewardTypeColumn,0);

		if(summaryTable.doesColumnExist(cs_experienceRewardAmountColumn))
			m_experienceRewardAmount = summaryTable.getIntValue(cs_experienceRewardAmountColumn,0);

		if(summaryTable.doesColumnExist(cs_factionRewardTypeColumn))
			m_factionRewardType = summaryTable.getStringValue(cs_factionRewardTypeColumn,0);

		if(summaryTable.doesColumnExist(cs_factionRewardAmountColumn))
			m_factionRewardAmount = summaryTable.getIntValue(cs_factionRewardAmountColumn,0);

		if(summaryTable.doesColumnExist(cs_moneyRewardCreditsColumn))
			m_moneyRewardCredits = summaryTable.getIntValue(cs_moneyRewardCreditsColumn,0);

		if(summaryTable.doesColumnExist(cs_completeQuestWhenTasksCompleteColumn))
			m_completeQuestWhenTasksComplete = (summaryTable.getIntValue(cs_completeQuestWhenTasksCompleteColumn,0) != 0);

		if(summaryTable.doesColumnExist(cs_journalEntryCompetionSummaryColumn))
			m_journalEntryCompetionSummary = StringId(summaryTable.getStringValue(cs_journalEntryCompetionSummaryColumn,0));

		std::string s;
		if(summaryTable.doesColumnExist(cs_inclusiveLootNameColumn) && summaryTable.doesColumnExist(cs_inclusiveLootCountColumn))
		{
			s = summaryTable.getStringValue(cs_inclusiveLootNameColumn,0);
			if(!s.empty())
			{
				m_inclusiveLootItemNames.push_back(s);
				m_inclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_inclusiveLootCountColumn,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_inclusiveLootNameColumn2) && summaryTable.doesColumnExist(cs_inclusiveLootCountColumn2))
		{
			s = summaryTable.getStringValue(cs_inclusiveLootNameColumn2,0);
			if(!s.empty())
			{
				m_inclusiveLootItemNames.push_back(s);
				m_inclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_inclusiveLootCountColumn2,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_inclusiveLootNameColumn3) && summaryTable.doesColumnExist(cs_inclusiveLootCountColumn3))
		{
			s = summaryTable.getStringValue(cs_inclusiveLootNameColumn3,0);
			if(!s.empty())
			{
				m_inclusiveLootItemNames.push_back(s);
				m_inclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_inclusiveLootCountColumn3,0));
			}
		}

		if(summaryTable.doesColumnExist(cs_exclusiveLootNameColumn) && summaryTable.doesColumnExist(cs_exclusiveLootCountColumn))
		{
			s = summaryTable.getStringValue(cs_exclusiveLootNameColumn,0);
			if(!s.empty())
			{
				m_exclusiveLootItemNames.push_back(s);
				m_exclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_exclusiveLootCountColumn,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_exclusiveLootNameColumn2) && summaryTable.doesColumnExist(cs_exclusiveLootCountColumn2))
		{
			s = summaryTable.getStringValue(cs_exclusiveLootNameColumn2,0);
			if(!s.empty())
			{
				m_exclusiveLootItemNames.push_back(s);
				m_exclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_exclusiveLootCountColumn2,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_exclusiveLootNameColumn3) && summaryTable.doesColumnExist(cs_exclusiveLootCountColumn3))
		{
			s = summaryTable.getStringValue(cs_exclusiveLootNameColumn3,0);
			if(!s.empty())
			{
				m_exclusiveLootItemNames.push_back(s);
				m_exclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_exclusiveLootCountColumn3,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_exclusiveLootNameColumn4) && summaryTable.doesColumnExist(cs_exclusiveLootCountColumn4))
		{
			s = summaryTable.getStringValue(cs_exclusiveLootNameColumn4,0);
			if(!s.empty())
			{
				m_exclusiveLootItemNames.push_back(s);
				m_exclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_exclusiveLootCountColumn4,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_exclusiveLootNameColumn5) && summaryTable.doesColumnExist(cs_exclusiveLootCountColumn5))
		{
			s = summaryTable.getStringValue(cs_exclusiveLootNameColumn5,0);
			if(!s.empty())
			{
				m_exclusiveLootItemNames.push_back(s);
				m_exclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_exclusiveLootCountColumn5,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_exclusiveLootNameColumn6) && summaryTable.doesColumnExist(cs_exclusiveLootCountColumn6))
		{
			s = summaryTable.getStringValue(cs_exclusiveLootNameColumn6,0);
			if(!s.empty())
			{
				m_exclusiveLootItemNames.push_back(s);
				m_exclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_exclusiveLootCountColumn6,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_exclusiveLootNameColumn7) && summaryTable.doesColumnExist(cs_exclusiveLootCountColumn7))
		{
			s = summaryTable.getStringValue(cs_exclusiveLootNameColumn7,0);
			if(!s.empty())
			{
				m_exclusiveLootItemNames.push_back(s);
				m_exclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_exclusiveLootCountColumn7,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_exclusiveLootNameColumn8) && summaryTable.doesColumnExist(cs_exclusiveLootCountColumn8))
		{
			s = summaryTable.getStringValue(cs_exclusiveLootNameColumn8,0);
			if(!s.empty())
			{
				m_exclusiveLootItemNames.push_back(s);
				m_exclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_exclusiveLootCountColumn8,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_exclusiveLootNameColumn9) && summaryTable.doesColumnExist(cs_exclusiveLootCountColumn9))
		{
			s = summaryTable.getStringValue(cs_exclusiveLootNameColumn9,0);
			if(!s.empty())
			{
				m_exclusiveLootItemNames.push_back(s);
				m_exclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_exclusiveLootCountColumn9,0));
			}
		}
		if(summaryTable.doesColumnExist(cs_exclusiveLootNameColumn10) && summaryTable.doesColumnExist(cs_exclusiveLootCountColumn10))
		{
			s = summaryTable.getStringValue(cs_exclusiveLootNameColumn10,0);
			if(!s.empty())
			{
				m_exclusiveLootItemNames.push_back(s);
				m_exclusiveLootItemCounts.push_back(summaryTable.getIntValue(cs_exclusiveLootCountColumn10,0));
			}
		}

		//-- Task list

		Iff taskIff((cs_taskTablePrefix + m_name.getString() + ".iff").c_str());
		DataTable taskList;
		taskList.load(taskIff);

		int const numRows = taskList.getNumRows();
		for (int i=0; i<numRows; ++i)
		{
			QuestTask * const task = new QuestTask(*this, taskList, i);
			m_tasks->push_back(task);
		}
	}
	else
		DEBUG_WARNING(true,("Data tables for quest %s were not found", m_name.getString()));
}

// ----------------------------------------------------------------------

Quest::~Quest()
{
	delete m_prerequisiteQuests;
	delete m_exclusionQuests;
	std::for_each(m_tasks->begin(), m_tasks->end(), PointerDeleter());
	delete m_tasks;
}

// ----------------------------------------------------------------------

std::vector<uint32> const & Quest::getPrerequisiteQuests() const
{
	return *m_prerequisiteQuests;
}

// ----------------------------------------------------------------------

std::vector<uint32> const & Quest::getExclusionQuests() const
{
	return *m_exclusionQuests;
}

// ----------------------------------------------------------------------

bool Quest::isRepeatable() const
{
	return m_allowRepeats;
}

// ----------------------------------------------------------------------

bool Quest::isVisible() const
{
	return m_visible;
}

// ----------------------------------------------------------------------

int Quest::getNumberOfTasks() const
{
	return static_cast<int>(m_tasks->size());
}

// ----------------------------------------------------------------------

QuestTask const * Quest::getTask(int const taskId) const
{
	if (taskId < 0 || taskId >= getNumberOfTasks())
		return nullptr;
	return (*m_tasks)[static_cast<size_t>(taskId)];
}

// ----------------------------------------------------------------------

CrcString const & Quest::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

uint32 Quest::getId() const
{
	return m_name.getCrc();
}

// ----------------------------------------------------------------------

StringId const & Quest::getCategory() const
{
	return m_category;
}

// ----------------------------------------------------------------------

StringId const & Quest::getJournalEntryTitle() const
{
	return m_journalEntryTitle;
}

// ----------------------------------------------------------------------

StringId const & Quest::getJournalEntryDescription() const
{
	return m_journalEntryDescription;
}

// ----------------------------------------------------------------------

StringId const & Quest::getJournalEntryCompetionSummary() const
{
	return m_journalEntryCompetionSummary;
}

// ----------------------------------------------------------------------

bool Quest::getCompleteQuestWhenTasksComplete() const
{
	return m_completeQuestWhenTasksComplete;
}

// ----------------------------------------------------------------------

int Quest::getLevel() const
{
	return m_level;
}

// ----------------------------------------------------------------------

int Quest::getTier() const
{
	return m_tier;
}

// ----------------------------------------------------------------------

std::string const & Quest::getType() const
{
	return m_type;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExperienceRewardType() const
{
	return m_experienceRewardType;
}

// ----------------------------------------------------------------------

int Quest::getExperienceRewardAmount() const
{
	//-- NOTE: Any changes to this function must also be made to the script code used
	//--       to calculate the reward experience given to the player.
	//--       See groundquests.getQuestExperienceReward in script/library.

	DataTable * questExperienceDataTable = DataTableManager::getTable(cs_questExperienceDataTableFile, true);

	if (getLevel() < 1 || getTier() < 0)
		return m_experienceRewardAmount;

	if (getTier() == 0)
		return 0;

	int levelColumn = questExperienceDataTable->findColumnNumber(cs_questExperienceDataTableLevelColumn);
	int levelRow = questExperienceDataTable->searchColumnString(levelColumn,
		FormattedString<32>().sprintf("%d", getLevel()));

	return questExperienceDataTable->getIntValue(cs_questExperienceDataTableTierColumns[getTier()-1], levelRow);
}

// ----------------------------------------------------------------------

std::string const & Quest::getFactionRewardType() const
{
	return m_factionRewardType;
}

// ----------------------------------------------------------------------

int Quest::getFactionRewardAmount() const
{
	return m_factionRewardAmount;
}

// ----------------------------------------------------------------------

int Quest::getMoneyRewardCredits() const
{
	return m_moneyRewardCredits;
}

// ----------------------------------------------------------------------

/**
 * Conversts a comma-delimited list of strings into a vector or the CRCs
 * of those strings.
 */
void Quest_Namespace::getCrcVector(std::string const &source, std::vector<uint32> & crcs)
{
	size_t pos = 0;
	size_t commaPos = source.find(',', pos);
	while (commaPos != std::string::npos)
	{
		crcs.push_back(Crc::calculate(source.c_str()+pos, static_cast<int>(commaPos-pos-1))); //lint !e834 a-b-c is NOT confusing
		pos = commaPos+1;
		commaPos = source.find(',', pos);
	}
	if (pos < source.size())
		crcs.push_back(Crc::calculate(source.c_str()+pos));
}

// ----------------------------------------------------------------------

std::vector<std::string> const & Quest::getInclusiveStaticLootItemNames() const
{
	return m_inclusiveLootItemNames;
}

// ----------------------------------------------------------------------

std::vector<int> const & Quest::getInclusiveStaticLootItemCounts() const
{
	return m_inclusiveLootItemCounts;
}

// ----------------------------------------------------------------------

std::vector<std::string> const & Quest::getExclusiveStaticLootItemNames() const
{
	return m_exclusiveLootItemNames;
}

// ----------------------------------------------------------------------

std::vector<int> const & Quest::getExclusiveStaticLootItemCounts() const
{
	return m_exclusiveLootItemCounts;
}

// ----------------------------------------------------------------------

bool Quest::isAnInclusiveStaticLootReward(std::string const & item) const
{
	return std::find(m_inclusiveLootItemNames.begin(), m_inclusiveLootItemNames.end(), item) != m_inclusiveLootItemNames.end();
}

// ----------------------------------------------------------------------

bool Quest::isAnExclusiveStaticLootReward(std::string const & item) const
{
	return std::find(m_exclusiveLootItemNames.begin(), m_exclusiveLootItemNames.end(), item) != m_exclusiveLootItemNames.end();
}

// ======================================================================
