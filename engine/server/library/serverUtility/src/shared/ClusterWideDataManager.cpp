// ======================================================================
//
// ClusterWideDataManager.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/ClusterWideDataManager.h"

// ======================================================================

unsigned long ClusterWideDataManager::ms_lockKeyCounter = 0;

// ======================================================================

ClusterWideDataManager::ClusterWideDataManager(std::string const & managerName) :
	m_managerName(managerName),
	m_dictionaryMap()
{
}

// ----------------------------------------------------------------------

ClusterWideDataManager::~ClusterWideDataManager()
{
}

// ----------------------------------------------------------------------

bool ClusterWideDataManager::getElement(std::string const & elementNameRegex,
                                        bool const lockElements,
                                        std::vector<std::string> & elementNameList,
                                        std::vector<ValueDictionary> & elementDictionaryList,
                                        unsigned long & lockKey)
{
	bool success = true;

	// allocate a new lock key; if we end up not actually
	// using the new lock key, we will "put it back"
	if (lockElements)
	{
		++ms_lockKeyCounter;

		// handle wraparound
		if (ms_lockKeyCounter == 0)
			ms_lockKeyCounter = 1;
	}

	elementNameList.clear();
	elementDictionaryList.clear();
	lockKey = 0;

	bool useRegexCompare = isElementNameRegex(elementNameRegex);
	std::vector<ElementData *> tempElementDictionaryList;

	for (ElementDataMap::iterator iter = m_dictionaryMap.begin(); iter != m_dictionaryMap.end(); ++iter)
	{
		if (isMatchElementName(elementNameRegex, iter->first, useRegexCompare))
		{
			// if element is locked and request wanted to lock
			// returned elements, don't need to look any further
			if ((lockElements) && ((iter->second).lockKey > 0))
			{
				success = false;
				break;
			}

			// found a match, add the element name to return list
			elementNameList.push_back(iter->first);

			// if request wanted to lock returned elements, don't
			// put the dictionary on the return list yet, because
			// if it's a regular expression search, the request
			// may still fail if a matching element after this
			// is locked, and we will have wasted time putting
			// the dictionary on the returned list, which makes
			// a copy of the dictionary, which could be expensive;
			// so we will put a reference to the dictionary into
			// a temporary list, and at the end, copy them over
			// into the return dictionary list
			if (lockElements)
				tempElementDictionaryList.push_back(&(iter->second));
			else
				elementDictionaryList.push_back((iter->second).dictionary);

			// if not using regular expression, we already found
			// the one match, so don't need to look any further
			if (!useRegexCompare)
				break;
		}
	}

	// if successful, copy matching dictionary from temp list to return list
	if ((success) && (lockElements))
	{
		for (std::vector<ElementData *>::iterator iter2 = tempElementDictionaryList.begin(); iter2 != tempElementDictionaryList.end(); ++iter2)
		{
			// lock the elements in the return list
			(*iter2)->lockKey = ms_lockKeyCounter;

			elementDictionaryList.push_back((*iter2)->dictionary);
		}

		// return the lock key to the caller that can be used
		// later to unlock the elements in the return list
		if (!elementDictionaryList.empty())
			lockKey = ms_lockKeyCounter;
	}

	// if new lock key was not used, "put it back"
	if (lockElements)
	{
		if ((!success) || (elementDictionaryList.empty()))
			--ms_lockKeyCounter;
	}

	// if failure, make sure all return lists are empty
	if (!success)
	{
		elementNameList.clear();
		elementDictionaryList.clear();
	}

	return success;
}

// ----------------------------------------------------------------------

int ClusterWideDataManager::releaseLock(unsigned long const lockKey)
{
	// if no lock specified, no elements will be unlocked
	if (lockKey == 0)
		return 0;

	int numberElementsUnlocked = 0;

	for (ElementDataMap::iterator iter = m_dictionaryMap.begin(); iter != m_dictionaryMap.end(); ++iter)
	{
		if (((iter->second).lockKey > 0) && ((iter->second).lockKey == lockKey))
		{
			(iter->second).lockKey = 0;
			++numberElementsUnlocked;
		}
	}

	return numberElementsUnlocked;
}

// ----------------------------------------------------------------------

int ClusterWideDataManager::removeElement(std::string const & elementNameRegex, unsigned long const lockKey)
{
	// if no lock specified, no elements will be removed
	if (lockKey == 0)
		return 0;

	int numberElementsRemoved = 0;
	bool useRegexCompare = isElementNameRegex(elementNameRegex);

	for (ElementDataMap::iterator iter = m_dictionaryMap.begin(); iter != m_dictionaryMap.end();)
	{
		if (isMatchElementName(elementNameRegex, iter->first, useRegexCompare))
		{
			if (((iter->second).lockKey > 0) && ((iter->second).lockKey == lockKey))
			{
				m_dictionaryMap.erase(iter++);
				++numberElementsRemoved;
			}
			else
			{
				++iter;
			}

			// if not using regular expression, we already found
			// the one match, so don't need to look any further
			if (!useRegexCompare)
				break;
		}
		else
		{
			++iter;
		}
	}

	return numberElementsRemoved;
}

// ----------------------------------------------------------------------

int ClusterWideDataManager::removeElementByOwnerId(unsigned long const ownerId)
{
	// if no ownerId specified, no elements will be removed
	if (ownerId == 0)
		return 0;

	// remove elements regardless of whether they are locked or not
	int numberElementsRemoved = 0;

	for (ElementDataMap::iterator iter = m_dictionaryMap.begin(); iter != m_dictionaryMap.end();)
	{
		if ((iter->second).ownerId == ownerId)
		{
			m_dictionaryMap.erase(iter++);
			++numberElementsRemoved;
		}
		else
		{
			++iter;
		}
	}

	return numberElementsRemoved;
}

// ----------------------------------------------------------------------

int ClusterWideDataManager::updateDictionary(std::string const & elementNameRegex, ValueDictionary const & dictionary, unsigned long const lockKey)
{
	return updateDictionaryHelper(elementNameRegex, dictionary, false, 0, lockKey);
}

// ----------------------------------------------------------------------

int ClusterWideDataManager::replaceDictionary(std::string const & elementNameRegex, ValueDictionary const & dictionary, unsigned long const ownerId, unsigned long const lockKey)
{
	return updateDictionaryHelper(elementNameRegex, dictionary, true, ownerId, lockKey);
}

// ----------------------------------------------------------------------

int ClusterWideDataManager::updateDictionaryHelper(std::string const & elementNameRegex, ValueDictionary const & dictionary, bool const overwriteDictionary, unsigned long const ownerId, unsigned long const lockKey)
{
	int numberElementsUpdated = 0;
	bool foundMatch = false;
	bool useRegexCompare = isElementNameRegex(elementNameRegex);

	for (ElementDataMap::iterator iter = m_dictionaryMap.begin(); iter != m_dictionaryMap.end(); ++iter)
	{
		if (isMatchElementName(elementNameRegex, iter->first, useRegexCompare))
		{
			foundMatch = true;

			if (((iter->second).lockKey > 0) && ((iter->second).lockKey == lockKey))
			{
				if (overwriteDictionary)
				{
					(iter->second).ownerId = ownerId;
					(iter->second).dictionary = dictionary;
				}
				else
				{
					(iter->second).dictionary.insert(dictionary);
				}

				++numberElementsUpdated;
			}

			// if not using regular expression, we already found
			// the one match, so don't need to look any further
			if (!useRegexCompare)
				break;
		}
	}

	// if no match was found, and a single element was specified,
	// and overwrite was specified, insert it
	if ((!useRegexCompare) && (!foundMatch) && (overwriteDictionary))
	{
		ElementData element;
		element.lockKey = 0;
		element.ownerId = ownerId;
		element.dictionary = dictionary;

		m_dictionaryMap[elementNameRegex] = element;

		++numberElementsUpdated;
	}

	return numberElementsUpdated;
}

// ----------------------------------------------------------------------

bool ClusterWideDataManager::isElementNameRegex(std::string const & elementNameRegex)
{
	// check for regular expression; currently we
	// only support a single * at the end of the string
	return ((elementNameRegex.length() > 0) && (elementNameRegex.rfind('*') == (elementNameRegex.length() - 1)));
}

// ----------------------------------------------------------------------

bool ClusterWideDataManager::isMatchElementName(std::string const & elementNameRegex, std::string const & elementName, bool const useRegexCompare)
{
	if (useRegexCompare)
	{
		if (elementNameRegex.length() == 1)
		{
			return true;
		}
		else
		{
			// currently, the only regular expression we support
			// is a single * at the end of the string
			return (strncmp(elementNameRegex.c_str(), elementName.c_str(), elementNameRegex.length() - 1) == 0);
		}
	}
	else
	{
		return (elementNameRegex == elementName);
	}
}

// ======================================================================
