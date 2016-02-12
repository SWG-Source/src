// ======================================================================
//
// QuestManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/QuestManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcStringTable.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/Quest.h"
#include "sharedUtility/FileName.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

// ======================================================================

namespace QuestManagerNamespace
{
	std::string const s_questListFileName("misc/quest_crc_string_table.iff");
	std::string const cs_summaryTablePrefix("datatables/questlist/");
	std::string const cs_taskTablePrefix("datatables/questtask/");

	std::string const cs_unabandonableQuestFileName("datatables/quest/unabandonable_quests.iff");
	std::string const cs_unacceptableQuestFileName("datatables/quest/unaccepted_quests.iff");
	std::string const s_forceAcceptQuestsFileName("datatables/quest/force_accept_quests.iff");

	typedef std::map<uint32, Quest*> QuestMap;
	QuestMap s_quests;

	typedef std::map<uint32, std::string> QuestNameMap;
	QuestNameMap s_questNames;


	std::set<std::string> s_unabandonableQuests;
	std::set<std::string> s_unacceptableQuests;
	std::set<std::string> s_forceAcceptQuests;

	bool ms_installed = false;
	bool ms_debugging = false;

	//currently turned off
	int const cms_maximumNumberOfQuestsPerPlayer = -1;

#ifdef _DEBUG
	void regressionTest();
#endif
}

using namespace QuestManagerNamespace;

// ======================================================================

void QuestManager::install()
{
	InstallTimer const installTimer("QuestManager::install");

	DEBUG_FATAL(ms_installed, ("QuestManager::install: already installed"));
	ms_installed = true;

	loadQuests();

	//load up the list of unabandonable quests (ones that would break if players abandoned them at the wrong place, or quests that just haven't been tested with abandonment
	s_unabandonableQuests.clear();
	Iff unabandonableQuestsTableIff;
	if(unabandonableQuestsTableIff.open(cs_unabandonableQuestFileName.c_str(), true))
	{
		DataTable unabandonableQuestsTable;
		unabandonableQuestsTable.load(unabandonableQuestsTableIff);
		int const rowCount        = unabandonableQuestsTable.getNumRows();
		int const questNameColumn = unabandonableQuestsTable.findColumnNumber("NAME");
		for(int row = 0; row < rowCount; ++row)
		{
			s_unabandonableQuests.insert(unabandonableQuestsTable.getStringValue(questNameColumn, row));
		}
	}

	//load up the list of unabandonable quests (ones that would break if players abandoned them at the wrong place, or quests that just haven't been tested with abandonment
	s_unacceptableQuests.clear();
	Iff unacceptableQuestsTableIff;
	if(unacceptableQuestsTableIff.open(cs_unacceptableQuestFileName.c_str(), true))
	{
		DataTable unacceptableQuestsTable;
		unacceptableQuestsTable.load(unacceptableQuestsTableIff);
		int const rowCount        = unacceptableQuestsTable.getNumRows();
		int const questNameColumn = unacceptableQuestsTable.findColumnNumber("NAME");
		for(int row = 0; row < rowCount; ++row)
		{
			s_unacceptableQuests.insert(unacceptableQuestsTable.getStringValue(questNameColumn, row));
		}
	}

	//load up the list of unabandonable quests (ones that would break if players abandoned them at the wrong place, or quests that just haven't been tested with abandonment
	s_forceAcceptQuests.clear();
	Iff forceAcceptQuestsTableIff;
	if(forceAcceptQuestsTableIff.open(s_forceAcceptQuestsFileName.c_str(), true))
	{
		DataTable forceAcceptQuestsTable;
		forceAcceptQuestsTable.load(forceAcceptQuestsTableIff);
		int const rowCount        = forceAcceptQuestsTable.getNumRows();
		int const questNameColumn = forceAcceptQuestsTable.findColumnNumber("NAME");
		for(int row = 0; row < rowCount; ++row)
		{
			s_forceAcceptQuests.insert(forceAcceptQuestsTable.getStringValue(questNameColumn, row));
			//force accept quests are also unacceptable quests (so they don't show the completionUI)
			s_unacceptableQuests.insert(forceAcceptQuestsTable.getStringValue(questNameColumn, row));
		}
	}

	ExitChain::add(QuestManager::remove, "QuestManager::install");
}

// ----------------------------------------------------------------------

void QuestManager::loadQuests()
{
	CrcStringTable questList(s_questListFileName.c_str());

	std::vector<const char *> strings;
	questList.getAllStrings(strings);
	for (std::vector<const char *>::const_iterator i=strings.begin(); i!=strings.end(); ++i)
	{
		std::string const & questName = *i;
		s_questNames[Crc::calculate(questName.c_str())] = questName;
	}

#ifdef _DEBUG
	{
		InstallTimer const installTimer("QuestManager::install: quest validation");

		for (int i = 0; i < questList.getNumberOfStrings(); ++i)
		{
			FileName const fileName("datatables/questlist", questList.getString(i).getString(), "iff");
			DEBUG_WARNING(!TreeFile::exists(fileName.getString()), ("Data tables for quest %s were not found", fileName.getString()));
		}

	}

	regressionTest();
#endif
}

// ----------------------------------------------------------------------

void QuestManager::unloadQuests()
{
	std::for_each(s_quests.begin(), s_quests.end(), PointerDeleterPairSecond());
	s_quests.clear();
	s_questNames.clear();
}

// ----------------------------------------------------------------------

void QuestManager::reloadQuests()
{
	unloadQuests();
	loadQuests();
}

// ----------------------------------------------------------------------

void QuestManager::remove()
{
	unloadQuests();

	s_unabandonableQuests.clear();
	s_unacceptableQuests.clear();
	s_forceAcceptQuests.clear();

	DEBUG_FATAL(!ms_installed, ("QuestManager::remove: not installed"));
	ms_installed = false;
}

// ----------------------------------------------------------------------

Quest const * QuestManager::getQuest(CrcString const & fileName)
{
	Quest const * const quest = getQuest(fileName.getCrc());
	DEBUG_WARNING(quest == nullptr, ("QuestManager::getQuest(%s): FAILED - testquest not found", fileName.getString()));
	return quest;
}

// ----------------------------------------------------------------------

Quest const * QuestManager::getQuest(uint32 const questCrc)
{
	Quest * quest = nullptr;

	//-- Look for the quest in the quest map
	QuestMap::iterator const iter = s_quests.find(questCrc);
	if (iter != s_quests.end())
		quest = iter->second;

	if (!quest) 
	{
		// -- Didn't find it in the map?  Create it.
		QuestNameMap::iterator const itQuestName = s_questNames.find(questCrc);
		if (itQuestName != s_questNames.end()) 
		{
			std::string const & questName = itQuestName->second;

			DataTableManager::reloadIfOpen(cs_summaryTablePrefix + questName + ".iff");
			DataTableManager::reloadIfOpen(cs_taskTablePrefix + questName + ".iff");

			quest = new Quest(ConstCharCrcString(questName.c_str()));
			s_quests[questCrc] = quest;
		}
	}
	
	DEBUG_WARNING(quest == nullptr, ("QuestManager::getQuest: FAILED - testquest not found"));

	return quest;
}

// ----------------------------------------------------------------------

Quest const * QuestManager::getQuest(std::string const & questName)
{
	TemporaryCrcString const fileName(questName.c_str(), true);
	Quest const * const quest = getQuest(fileName);
	DEBUG_WARNING(quest == nullptr, ("QuestManager::getQuest(%s): FAILED - testquest not found", questName.c_str()));
	return quest;
}

// ----------------------------------------------------------------------

std::string QuestManager::getQuestName(uint32 const questCrc)
{
	Quest const * const q = getQuest(questCrc);
	
	if(!q)
		return "";
	else
		return q->getName().getString();
}

// ----------------------------------------------------------------------

void QuestManager::setDebugging(bool debugging)
{
	ms_debugging = debugging;
}

// ----------------------------------------------------------------------

bool QuestManager::getDebugging()
{
	return ms_debugging;
}

// ----------------------------------------------------------------------

bool QuestManager::isValidTaskId(int const taskId)
{
	return (taskId >= 0 && taskId < 16);
}

// ----------------------------------------------------------------------

bool QuestManager::isQuestAbandonable(std::string const & questName)
{
	//can't abandon empty quest name
	if(questName.empty())
		return false;

	//can't abandon a quest with a colon in it (used for quest:taskid)
	if(questName.find(":") != questName.npos)
		return false;

	return s_unabandonableQuests.find(questName) == s_unabandonableQuests.end();
}

// ----------------------------------------------------------------------

bool QuestManager::isQuestAcceptable(std::string const & questName)
{
	//can't accept and empty quest name
	if(questName.empty())
		return false;

	//can't accept invalid quest names
	Quest const * const q = getQuest(questName);
	if(!q)
		return false;
	
	//can't accept invisible quests
	if(!q->isVisible())	
		return false;

	//can't accept a quest with a colon in it (used for quest:taskid)
	if(questName.find(":") != questName.npos)
		return false;

	return s_unacceptableQuests.find(questName) == s_unacceptableQuests.end();
}

// ----------------------------------------------------------------------

bool QuestManager::isQuestForceAccepted(std::string const & questName)
{
	//can't force accept a quest with a colon in it (used for quest:taskid)
	if(questName.find(":") != questName.npos)
		return false;

	return s_forceAcceptQuests.find(questName) != s_forceAcceptQuests.end();
}

// ----------------------------------------------------------------------

int QuestManager::getMaximumNumberOfQuestsPerPlayer()
{
	return cms_maximumNumberOfQuestsPerPlayer;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void QuestManagerNamespace::regressionTest()
{
	DEBUG_FATAL(QuestManager::getQuest("testquest") == 0, ("QuestManagerNamespace::regressionTest: FAILED - testquest not found"));
}

#endif

// ======================================================================
