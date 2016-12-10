// ======================================================================
//
// Quest.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Quest_H
#define INCLUDED_Quest_H

// ======================================================================

#include "sharedFoundation/PersistentCrcString.h"
#include "StringId.h"

class QuestTask;

// ======================================================================

#include <vector>

// ======================================================================

/**
 * Data associated with a quest.
 */
class Quest
{
public:
	explicit Quest(CrcString const & name);
	~Quest();

public:
	bool isRepeatable() const;
	bool isVisible() const;
	std::vector<uint32> const & getPrerequisiteQuests() const;
	std::vector<uint32> const & getExclusionQuests() const;
	int getNumberOfTasks() const;
	QuestTask const * getTask(int taskId) const;
	CrcString const & getName() const;
	StringId const & getJournalEntryTitle() const;
	StringId const & getJournalEntryDescription() const;
	StringId const & getJournalEntryCompetionSummary() const;
	uint32 getId() const;
	StringId const & getCategory() const;
	bool getCompleteQuestWhenTasksComplete() const;
	int getLevel() const;
	int getTier() const;
	std::string const & getType() const;
	std::string const & getExperienceRewardType() const;
	int getExperienceRewardAmount() const;
	std::string const & getFactionRewardType() const;
	int getFactionRewardAmount() const;
	int getMoneyRewardCredits() const;
	std::vector<std::string> const & getInclusiveStaticLootItemNames() const;
	std::vector<int> const & getInclusiveStaticLootItemCounts() const;
	std::vector<std::string> const & getExclusiveStaticLootItemNames() const;
	std::vector<int> const & getExclusiveStaticLootItemCounts() const;
	bool isAnInclusiveStaticLootReward(std::string const & item) const;
	bool isAnExclusiveStaticLootReward(std::string const & item) const;

private:
	PersistentCrcString const m_name;
	StringId m_journalEntryTitle;
	StringId m_journalEntryDescription;
	StringId m_journalEntryCompetionSummary;
	std::vector<uint32> * const m_prerequisiteQuests;
	std::vector<uint32> * const m_exclusionQuests;
	std::vector<QuestTask *> * const m_tasks;
	bool m_allowRepeats;
	bool m_visible;
	StringId m_category;
	bool m_completeQuestWhenTasksComplete;
	int m_level;
	int m_tier;
	std::string m_type;
	std::string m_experienceRewardType;
	int m_experienceRewardAmount;
	std::string m_factionRewardType;
	int m_factionRewardAmount;
	int m_moneyRewardCredits;
	std::vector<std::string> m_inclusiveLootItemNames;
	std::vector<int> m_inclusiveLootItemCounts;
	std::vector<std::string> m_exclusiveLootItemNames;
	std::vector<int> m_exclusiveLootItemCounts;

private:
	Quest();
	Quest(Quest const &);
	Quest & operator=(Quest const &); //disable
};

// ======================================================================

#endif
