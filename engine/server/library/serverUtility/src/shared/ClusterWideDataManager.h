// ======================================================================
//
// ClusterWideDataManager.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClusterWideDataManager_H
#define INCLUDED_ClusterWideDataManager_H

// ======================================================================

#include "sharedUtility/ValueDictionary.h"

#include <map>
#include <string>
#include <vector>

// ======================================================================

class ClusterWideDataManager
{
public: //ctor/dtor
	ClusterWideDataManager(std::string const & managerName);
	~ClusterWideDataManager();

public: // methods
	std::string const & getManagerName() const;
	bool                getElement(std::string const & elementNameRegex,
	                               bool lockElements,
	                               std::vector<std::string> & elementNameList,
	                               std::vector<ValueDictionary> & elementDictionaryList,
	                               unsigned long & lockKey);
	int                 releaseLock(unsigned long lockKey);
	int                 removeElement(std::string const & elementNameRegex, unsigned long lockKey);
	int                 removeElementByOwnerId(unsigned long ownerId);
	int                 updateDictionary(std::string const & elementNameRegex, ValueDictionary const & dictionary, unsigned long lockKey);
	int                 replaceDictionary(std::string const & elementNameRegex, ValueDictionary const & dictionary, unsigned long ownerId, unsigned long lockKey);

private: // helper function
	int                 updateDictionaryHelper(std::string const & elementNameRegex, ValueDictionary const & dictionary, bool overwriteDictionary, unsigned long ownerId, unsigned long lockKey);
	static bool         isElementNameRegex(std::string const & elementNameRegex);
	static bool         isMatchElementName(std::string const & elementNameRegex, std::string const & elementName, bool useRegexCompare);

private: // data structures
	struct ElementData
	{
		ElementData() : lockKey(0), ownerId(0) {};

		unsigned long lockKey;
		unsigned long ownerId;
		ValueDictionary dictionary;
	};

	typedef std::map<std::string, ElementData> ElementDataMap;

private:
	std::string const    m_managerName;
	ElementDataMap       m_dictionaryMap;

	// for assigning lock keys
	static unsigned long ms_lockKeyCounter;

	// Disabled.
	ClusterWideDataManager();
	ClusterWideDataManager(ClusterWideDataManager const &);
	ClusterWideDataManager &operator =(ClusterWideDataManager const &);
};

//-----------------------------------------------------------------------

inline std::string const & ClusterWideDataManager::getManagerName() const
{
	return m_managerName;
}

// ======================================================================

#endif
