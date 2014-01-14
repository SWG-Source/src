// ======================================================================
//
// QuestManager.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_QuestManager_H
#define INCLUDED_QuestManager_H

// ======================================================================

class Quest;
class CrcString;

// ======================================================================

/**
* Singleton for managing the list of quests
*/
class QuestManager
{
public:
	static void install();
	static void reloadQuests();
	static void setDebugging(bool debugging);
	static bool getDebugging();
	static bool isValidTaskId(int taskId);
	static bool isQuestAbandonable(std::string const & questName);
	static bool isQuestAcceptable(std::string const & questName);
	static int  getMaximumNumberOfQuestsPerPlayer();
	static bool isQuestForceAccepted(std::string const & questName);
	
public:
	static Quest const * getQuest(uint32 questCrc);
	static Quest const * getQuest(std::string const & questName);
	static Quest const * getQuest(CrcString const & fileName);
	static std::string getQuestName(uint32 questCrc);
	
private:
	static void remove();
	static void loadQuests();
	static void unloadQuests();
};

// ======================================================================

#endif
