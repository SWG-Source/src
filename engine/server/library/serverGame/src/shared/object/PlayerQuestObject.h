// PlayerQuestObject.h
// Copyright 2009, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Boudreaux

#ifndef	_INCLUDED_PlayerQuestObject_H
#define	_INCLUDED_PlayerQuestObject_H

//-----------------------------------------------------------------------

#include "serverGame/CreatureObject.h"
#include "serverGame/TangibleObject.h"

class ServerPlayerQuestObjectTemplate;
//-----------------------------------------------------------------------

class PlayerQuestObject : public TangibleObject
{
public:
	virtual ~PlayerQuestObject  ();

	static PlayerQuestObject *         createPlayerQuestObjectInCreatureDatapad(CreatureObject * creatureWithDatapad);
	static PlayerQuestObject *         createPlayerQuestObjectInCreatureInventory(CreatureObject * creatureWithInventory);

	virtual PlayerQuestObject *        asPlayerQuestObject();
	virtual PlayerQuestObject const *  asPlayerQuestObject() const;

	static void             removeDefaultTemplate(void);

	virtual bool            canTrade() const;
	virtual bool            onContainerAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer);
	virtual bool            onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer);
	virtual int             onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer);

	virtual void            getAttributes       (std::vector<std::pair<std::string, Unicode::String> > &data) const;

	virtual void             onAddedToWorld            ();

	int						 getTotalTasks();
	void					 getTasks(std::vector<Unicode::String> & taskVector );
	void					 getTaskCounters(std::vector<int> & counterVector);
	void					 getTaskStatus(std::vector<int> & taskStatus);
	int						 getTaskStatus(int taskIndex);

	void					 addNewTask(std::string title, std::string description, int counterMax, std::string waypoint);
	void					 setTaskStatus(int taskIndex, int status);

	void					 setQuestTitle(std::string title);
	std::string const &		 getQuestTitle();
	void					 setQuestDescription(std::string description);
	std::string const &		 getQuestDescription();

	void					 setTaskTitle(int index, std::string title);
	std::string const &      getTaskTitle(int index);

	void					 setTaskDescription(int index, std::string description);
	std::string const &      getTaskDescription(int index);

	void					 setQuestDifficulty(int difficulty);
	int						 getQuestDifficulty();

	int						 getTaskCounter(int index);
	void					 setTaskCounter(int index, int value);

	std::string const &      getWaypoint(int index);

	bool					 isCompleted() const;

	void					 setRecipe(bool recipe);
	bool					 isRecipe() const;

	void					 addRecipeTaskData(std::string data);
	void					 setRecipeTaskData(int index, std::string data);
	std::string const 		 getRecipeTaskData(int index);

	void					 setRewards(std::string data);
	std::string const &		 getRewards();

	void					 setCreator(NetworkId const & creator);

	void					 resetAllData();

	virtual void             getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const;
	virtual void             setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source);

protected:
	virtual void            endBaselines        ();
	virtual void            initializeFirstTimeObject();

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;
	friend class ServerPlayerQuestObjectTemplate;
	PlayerQuestObject  (const ServerPlayerQuestObjectTemplate * playerQuestObjectTemplate);

private:
	PlayerQuestObject & operator = (const PlayerQuestObject & rhs);
	PlayerQuestObject(const PlayerQuestObject & source);

	void	readInObjVarData();
	void    saveDataToObjVars();

	void    checkForCompleted();
	void	fixObjVarDelimiter();

	virtual void			onLoadedFromDatabase();

private:

	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

	// Auto deltas.
	Archive::AutoDeltaVector<Unicode::String>					m_tasks;
	Archive::AutoDeltaVector<std::pair<int, int> >				m_taskCounters;
	Archive::AutoDeltaVector<int>								m_taskStatus;
	Archive::AutoDeltaVector<std::string>						m_waypoints;
	Archive::AutoDeltaVariable<std::string>						m_rewards;
	Archive::AutoDeltaVariable<std::string>						m_creatorName;
	Archive::AutoDeltaVariable<bool>							m_completed;
	Archive::AutoDeltaVariable<bool>							m_recipe;
	Archive::AutoDeltaVariable<std::string>					    m_title;
	Archive::AutoDeltaVariable<std::string>						m_description;
	Archive::AutoDeltaVariable<NetworkId>						m_creator;
	Archive::AutoDeltaVariable<int>								m_totalTasks;
	Archive::AutoDeltaVariable<int>								m_difficulty;
	
	// Task Titles and Descriptions.
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle1;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription1;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle2;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription2;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle3;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription3;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle4;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription4;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle5;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription5;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle6;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription6;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle7;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription7;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle8;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription8;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle9;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription9;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle10;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription10;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle11;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription11;
	Archive::AutoDeltaVariable<std::string>					    m_taskTitle12;
	Archive::AutoDeltaVariable<std::string>					    m_taskDescription12;

	// Helper containers to keep access simple.
	std::vector<Archive::AutoDeltaVariable<std::string> *>		m_taskTitles;
	std::vector<Archive::AutoDeltaVariable<std::string> *>		m_taskDescriptions;

private:
	void addMembersToPackages();
};

//-----------------------------------------------------------------------

inline int PlayerQuestObject::getTotalTasks()
{
	return m_totalTasks.get();
}

//-----------------------------------------------------------------------

inline void PlayerQuestObject::setQuestTitle(std::string title)
{
	m_title = title;
}

//-----------------------------------------------------------------------

inline std::string const & PlayerQuestObject::getQuestTitle()
{
	return m_title.get();
}

//-----------------------------------------------------------------------

inline void PlayerQuestObject::setQuestDescription(std::string description)
{
	m_description = description;
}

//-----------------------------------------------------------------------

inline std::string const & PlayerQuestObject::getQuestDescription()
{
	return m_description.get();
}

//-----------------------------------------------------------------------

inline void PlayerQuestObject::setQuestDifficulty(int difficulty)
{
	m_difficulty = difficulty;
}

//-----------------------------------------------------------------------

inline int PlayerQuestObject::getQuestDifficulty()
{
	return m_difficulty.get();
}

//-----------------------------------------------------------------------

inline int PlayerQuestObject::getTaskCounter(int index)
{
	return m_taskCounters.get()[index].first;
}

//-----------------------------------------------------------------------

inline std::string const & PlayerQuestObject::getWaypoint(int index)
{
	return m_waypoints.get()[index];
}

//-----------------------------------------------------------------------

inline bool PlayerQuestObject::isCompleted() const
{
	return m_completed.get();
}

//-----------------------------------------------------------------------

inline bool PlayerQuestObject::isRecipe() const
{
	return m_recipe.get();
}


#endif	// _INCLUDED_PlayerQuestObject_H

