// ======================================================================
//
// NameManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/NameManager.h"

#include "UnicodeUtils.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverGame/RegexList.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverNetworkMessages/CharacterNamesMessage.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/NameGenerator.h"

#include <algorithm>

// ======================================================================

NameManager *NameManager::ms_instance = nullptr;

// ======================================================================

void NameManager::install()
{
	DEBUG_FATAL(ms_instance,("Installed NameManager twice.\n"));
	ExitChain::add(remove,"NameManager::remove");

	ms_instance = new NameManager;
}

// ----------------------------------------------------------------------

void NameManager::remove()
{
	NOT_NULL(ms_instance);
	delete ms_instance;
	ms_instance = nullptr;
}

// ----------------------------------------------------------------------
	
NameManager::NameManager() :
		m_nameGenerators (new NameGeneratorMapType),
		m_reservedNames  (nullptr),
		m_nameToIdMap    (new NameToIdMapType),
		m_idToCharacterDataMap(new IdToCharacterDataMapType)
{
	DataTable *theTable = DataTableManager::getTable(ConfigServerGame::getReservedNameFile(), true);
	NOT_NULL(theTable);
	m_reservedNames = new RegexList(*theTable);
	DataTableManager::close(ConfigServerGame::getReservedNameFile());
}

// ----------------------------------------------------------------------

NameManager::~NameManager()
{
	NOT_NULL(m_nameGenerators);
	for (NameGeneratorMapType::const_iterator i=m_nameGenerators->begin(); i!=m_nameGenerators->end(); ++i)
	{
		delete i->second;
	}
	delete m_nameGenerators;
	m_nameGenerators = nullptr;
	delete m_reservedNames;
	m_reservedNames = nullptr;
	delete m_nameToIdMap;
	m_nameToIdMap = nullptr;
	delete m_idToCharacterDataMap;
	m_idToCharacterDataMap = nullptr;
}

// ----------------------------------------------------------------------

const NameGenerator & NameManager::getNameGenerator(const std::string &directory, const std::string &nameTable) const
{
	NameGenerator *generator=nullptr;
	NOT_NULL(m_nameGenerators);
	NameGeneratorMapType::const_iterator i=m_nameGenerators->find(NameTableIdentifier(directory,nameTable));
	if (i==m_nameGenerators->end())
	{
		generator=new NameGenerator(nameTable,directory);
		(*m_nameGenerators)[NameTableIdentifier(directory,nameTable)]=generator;
	}
	else
		generator = i->second;
	NOT_NULL(generator);
	return *generator;
}

// ----------------------------------------------------------------------

Unicode::String NameManager::generateRandomName(const std::string &directory, const std::string &nameTable) const
{
	const NameGenerator &nameGenerator = getNameGenerator(directory,nameTable);
	Unicode::String newName(nameGenerator.generateRandomName());
	int failcount = 0;
	UNREF(failcount);
	while (isNameReserved(newName))
	{
		newName = nameGenerator.generateRandomName();
		DEBUG_FATAL(++failcount > 100, ("GenerateRandomName couldn't find a name that passed the obscenity/reserved filter\n"));
	}
	return newName;	
}

// ----------------------------------------------------------------------

NameManager::NameTableIdentifier::NameTableIdentifier(const std::string &directory, const std::string &table) :
		m_directory(directory),
		m_table(table)
{
}

// ----------------------------------------------------------------------

bool NameManager::NameTableIdentifier::operator<(const NameTableIdentifier &rhs) const
{
	if (m_directory == rhs.m_directory)
		return m_table < rhs.m_table;
	else
		return m_directory < rhs.m_directory;
}

// ----------------------------------------------------------------------

/**
 * Check a name against the list of reserved or obscene names
 */
bool NameManager::isNameReserved(const Unicode::String &name, std::string &reasonReserved, std::vector<std::string> &ignoreRules) const
{
	NOT_NULL(m_reservedNames);
	return m_reservedNames->doesStringMatch(name, reasonReserved, ignoreRules);
}

// ----------------------------------------------------------------------

/**
 * Check a name against the list of reserved or obscene names
 */
bool NameManager::isNameReserved(const Unicode::String &name, std::string &reasonReserved) const
{
	NOT_NULL(m_reservedNames);
	return m_reservedNames->doesStringMatch(name, reasonReserved);
}

// ----------------------------------------------------------------------

/**
 * Check a name against the list of reserved or obscene names
 */
bool NameManager::isNameReserved(const Unicode::String &name) const
{
	NOT_NULL(m_reservedNames);
	return m_reservedNames->doesStringMatch(name);
}

// ----------------------------------------------------------------------

/**
 * Check a name against the "appropriateness" rules of a name generator.
 * @return true if the name is considered "appropriate" for the type of name
 * specified.  What "appropriate" means varies by name generator, but in
 * general it refers to the name length, number of words, etc.
 */
 
bool NameManager::isNameAppropriate(const std::string &directory, const std::string &nameTable, const Unicode::String &name) const
{
	return (getNameGenerator(directory,nameTable).verifyName(name));
}

// ----------------------------------------------------------------------

int NameManager::getTotalPlayerCount() const
{
	return m_idToCharacterDataMap->size();
}

// ----------------------------------------------------------------------

void NameManager::addPlayer(const NetworkId &id, uint32 stationId, const std::string &name, const std::string &fullName, time_t createTime, time_t lastLoginTime, bool notifyOtherServers)
{
	std::string normalizedName(normalizeName(name));
	
	(*m_nameToIdMap)[normalizedName] = id;

	CharacterData characterData;
	characterData.stationId = stationId;
	characterData.characterName = normalizedName;
	characterData.characterFullName = fullName;

	if (createTime == 0)
	{
		createTime = static_cast<time_t>(getPlayerCreateTime(id));
		if (createTime <= 0)
			createTime = ::time(nullptr);
	}
	characterData.createTime = createTime;

	if (lastLoginTime == 0)
	{
		lastLoginTime = static_cast<time_t>(getPlayerLastLoginTime(id));
		if (lastLoginTime <= 0)
			lastLoginTime = ::time(nullptr);
	}
	characterData.lastLoginTime = lastLoginTime;

	(*m_idToCharacterDataMap)[id] = characterData;

	if (notifyOtherServers)
	{
		std::vector<NetworkId> ids;
		std::vector<int> stationIds;
		std::vector<std::string> characterNames;
		std::vector<std::string> characterFullNames;
		std::vector<int> characterCreateTimes;
		std::vector<int> characterLastLoginTimes;

		ids.push_back(id);
		stationIds.push_back(static_cast<int>(stationId));
		characterNames.push_back(normalizedName);
		characterFullNames.push_back(fullName);
		characterCreateTimes.push_back(static_cast<int>(createTime));
		characterLastLoginTimes.push_back(static_cast<int>(lastLoginTime));

		ServerMessageForwarding::beginBroadcast();

		CharacterNamesMessage const characterNamesMessage(ids, stationIds, characterNames, characterFullNames, characterCreateTimes, characterLastLoginTimes);
		ServerMessageForwarding::send(characterNamesMessage);

		ServerMessageForwarding::end();
	}
}

// ----------------------------------------------------------------------

void NameManager::renamePlayer(const NetworkId &id, const Unicode::String &name, const Unicode::String &fullName)
{
	uint32 stationId = 0;
	time_t createTime = 0;
	time_t lastLoginTime = 0;

	IdToCharacterDataMapType::const_iterator i=m_idToCharacterDataMap->find(id);
	if (i != m_idToCharacterDataMap->end())
	{
		stationId = i->second.stationId;
		createTime = i->second.createTime;
		lastLoginTime = i->second.lastLoginTime;
	}

	// Remove the old names
	releasePlayerName(id);

	// Add the new names
	addPlayer(id, stationId, Unicode::wideToNarrow(name), Unicode::wideToNarrow(fullName), createTime, lastLoginTime, false);
}

// ----------------------------------------------------------------------

const NetworkId & NameManager::getPlayerId(const std::string &name) const
{
	NameToIdMapType::const_iterator i=m_nameToIdMap->find(name);
	if (i==m_nameToIdMap->end())
		return NetworkId::cms_invalid;
	else
		return i->second;
}

// ----------------------------------------------------------------------

uint32 NameManager::getPlayerStationId(const NetworkId &id) const
{
	IdToCharacterDataMapType::const_iterator i=m_idToCharacterDataMap->find(id);
	if (i==m_idToCharacterDataMap->end())
		return 0;
	else
		return i->second.stationId;
}

// ----------------------------------------------------------------------

const std::string & NameManager::getPlayerName(const NetworkId &id) const
{
	static const std::string emptyString;

	IdToCharacterDataMapType::const_iterator i=m_idToCharacterDataMap->find(id);
	if (i==m_idToCharacterDataMap->end())
		return emptyString;
	else
		return i->second.characterName;
}

// ----------------------------------------------------------------------

const std::string & NameManager::getPlayerFullName(const NetworkId &id) const
{
	static const std::string emptyString;

	IdToCharacterDataMapType::const_iterator i=m_idToCharacterDataMap->find(id);
	if (i==m_idToCharacterDataMap->end())
		return emptyString;
	else
		return i->second.characterFullName;
}

// ----------------------------------------------------------------------

int NameManager::getPlayerCreateTime(const NetworkId &id) const
{
	IdToCharacterDataMapType::const_iterator i=m_idToCharacterDataMap->find(id);
	if (i==m_idToCharacterDataMap->end())
		return -1;
	else
		return static_cast<int>(i->second.createTime);
}

// ----------------------------------------------------------------------

int NameManager::getPlayerLastLoginTime(const NetworkId &id) const
{
	IdToCharacterDataMapType::const_iterator i=m_idToCharacterDataMap->find(id);
	if (i==m_idToCharacterDataMap->end())
		return -1;
	else
		return static_cast<int>(i->second.lastLoginTime);
}

// ----------------------------------------------------------------------

void NameManager::getPlayerWithLastLoginTimeAfter(time_t const time, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const
{
	result.clear();

	for (IdToCharacterDataMapType::const_iterator i = m_idToCharacterDataMap->begin(); i != m_idToCharacterDataMap->end(); ++i)
	{
		if (i->second.lastLoginTime > time)
			IGNORE_RETURN(result.insert(std::make_pair(i->second.lastLoginTime, std::make_pair(std::make_pair(i->first, i->second.stationId), i->second.characterName))));
	}
}

// ----------------------------------------------------------------------

void NameManager::getPlayerWithLastLoginTimeBefore(time_t const time, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const
{
	result.clear();

	for (IdToCharacterDataMapType::const_iterator i = m_idToCharacterDataMap->begin(); i != m_idToCharacterDataMap->end(); ++i)
	{
		if (i->second.lastLoginTime < time)
			IGNORE_RETURN(result.insert(std::make_pair(i->second.lastLoginTime, std::make_pair(std::make_pair(i->first, i->second.stationId), i->second.characterName))));
	}
}

// ----------------------------------------------------------------------

void NameManager::getPlayerWithLastLoginTimeBetween(time_t const timeLowerRange, time_t const timeUpperRange, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const
{
	result.clear();

	for (IdToCharacterDataMapType::const_iterator i = m_idToCharacterDataMap->begin(); i != m_idToCharacterDataMap->end(); ++i)
	{
		if ((i->second.lastLoginTime > timeLowerRange) && (i->second.lastLoginTime < timeUpperRange))
			IGNORE_RETURN(result.insert(std::make_pair(i->second.lastLoginTime, std::make_pair(std::make_pair(i->first, i->second.stationId), i->second.characterName))));
	}
}

// ----------------------------------------------------------------------
// return the distribution of characters' last login time
void NameManager::getPlayerWithLastLoginTimeAfterDistribution(std::map<int, std::pair<std::string, int> > &result) const
{
	for (std::map<int, std::pair<std::string, int> >::iterator iter = result.begin(); iter != result.end(); ++iter)
		iter->second.second = 0;

	int const timeNow = static_cast<int>(::time(nullptr));
	for (IdToCharacterDataMapType::const_iterator i = m_idToCharacterDataMap->begin(); i != m_idToCharacterDataMap->end(); ++i)
	{
		int const lastLoginTimeSecondsAgo = std::max(static_cast<int>(0), static_cast<int>(timeNow - static_cast<int>(i->second.lastLoginTime)));

		std::map<int, std::pair<std::string, int> >::iterator iterFind = result.lower_bound(lastLoginTimeSecondsAgo);
		if (iterFind != result.end())
			++(iterFind->second.second);
	}
}

// ----------------------------------------------------------------------

void NameManager::getPlayerWithCreateTimeAfter(time_t const time, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const
{
	result.clear();

	for (IdToCharacterDataMapType::const_iterator i = m_idToCharacterDataMap->begin(); i != m_idToCharacterDataMap->end(); ++i)
	{
		if (i->second.createTime > time)
			IGNORE_RETURN(result.insert(std::make_pair(i->second.createTime, std::make_pair(std::make_pair(i->first, i->second.stationId), i->second.characterName))));
	}
}

// ----------------------------------------------------------------------

void NameManager::getPlayerWithCreateTimeBefore(time_t const time, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const
{
	result.clear();

	for (IdToCharacterDataMapType::const_iterator i = m_idToCharacterDataMap->begin(); i != m_idToCharacterDataMap->end(); ++i)
	{
		if (i->second.createTime < time)
			IGNORE_RETURN(result.insert(std::make_pair(i->second.createTime, std::make_pair(std::make_pair(i->first, i->second.stationId), i->second.characterName))));
	}
}

// ----------------------------------------------------------------------

void NameManager::getPlayerWithCreateTimeBetween(time_t const timeLowerRange, time_t const timeUpperRange, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const
{
	result.clear();

	for (IdToCharacterDataMapType::const_iterator i = m_idToCharacterDataMap->begin(); i != m_idToCharacterDataMap->end(); ++i)
	{
		if ((i->second.createTime > timeLowerRange) && (i->second.createTime < timeUpperRange))
			IGNORE_RETURN(result.insert(std::make_pair(i->second.createTime, std::make_pair(std::make_pair(i->first, i->second.stationId), i->second.characterName))));
	}
}

// ----------------------------------------------------------------------
// return the distribution of characters' create time
void NameManager::getPlayerWithCreateTimeAfterDistribution(std::map<int, std::pair<std::string, int> > &result) const
{
	for (std::map<int, std::pair<std::string, int> >::iterator iter = result.begin(); iter != result.end(); ++iter)
		iter->second.second = 0;

	int const timeNow = static_cast<int>(::time(nullptr));
	for (IdToCharacterDataMapType::const_iterator i = m_idToCharacterDataMap->begin(); i != m_idToCharacterDataMap->end(); ++i)
	{
		int const createTimeSecondsAgo = std::max(static_cast<int>(0), static_cast<int>(timeNow - static_cast<int>(i->second.createTime)));

		std::map<int, std::pair<std::string, int> >::iterator iterFind = result.lower_bound(createTimeSecondsAgo);
		if (iterFind != result.end())
			++(iterFind->second.second);
	}
}

// ----------------------------------------------------------------------

void NameManager::releasePlayerName(const NetworkId &id)
{
	IdToCharacterDataMapType::iterator i=m_idToCharacterDataMap->find(id);
	if (i!=m_idToCharacterDataMap->end())
	{
		m_nameToIdMap->erase(i->second.characterName);
		m_idToCharacterDataMap->erase(i);
	}
}

// ----------------------------------------------------------------------

void NameManager::addPlayers(const std::vector<NetworkId> &ids, const std::vector<int> &stationIds, const std::vector<std::string> &names, const std::vector<std::string> &fullNames, const std::vector<int> &createTimes, const std::vector<int> &lastLoginTimes)
{
	DEBUG_FATAL(ids.size() != stationIds.size(),("Programmer bug:  Vectors ids and stationIds must be the same size.\n"));
	DEBUG_FATAL(ids.size() != names.size(),("Programmer bug:  Vectors ids and names must be the same size.\n"));
	DEBUG_FATAL(ids.size() != fullNames.size(),("Programmer bug:  Vectors ids and fullNames must be the same size.\n"));
	DEBUG_FATAL(ids.size() != createTimes.size(),("Programmer bug:  Vectors ids and createTimes must be the same size.\n"));
	DEBUG_FATAL(ids.size() != lastLoginTimes.size(),("Programmer bug:  Vectors ids and lastLoginTimes must be the same size.\n"));

	for (size_t i=0; i<ids.size(); ++i)
	{
		addPlayer(ids[i], static_cast<uint32>(stationIds[i]), names[i], fullNames[i], static_cast<time_t>(createTimes[i]), static_cast<time_t>(lastLoginTimes[i]), false);
	}
}

// ----------------------------------------------------------------------

void NameManager::sendAllNamesToServer (std::vector<uint32> const & servers) const
{
	std::vector<NetworkId> ids;
	std::vector<int> stationIds;
	std::vector<std::string> characterNames;
	std::vector<std::string> characterFullNames;
	std::vector<int> characterCreateTimes;
	std::vector<int> characterLastLoginTimes;

	for (IdToCharacterDataMapType::const_iterator i=m_idToCharacterDataMap->begin(); i!=m_idToCharacterDataMap->end(); ++i)
	{
		ids.push_back(i->first);
		stationIds.push_back(static_cast<int>(i->second.stationId));
		characterNames.push_back(i->second.characterName);
		characterFullNames.push_back(i->second.characterFullName);
		characterCreateTimes.push_back(static_cast<int>(i->second.createTime));
		characterLastLoginTimes.push_back(static_cast<int>(i->second.lastLoginTime));
	}

	ServerMessageForwarding::begin(servers);

	CharacterNamesMessage const characterNamesMessage(ids, stationIds, characterNames, characterFullNames, characterCreateTimes, characterLastLoginTimes);
	ServerMessageForwarding::send(characterNamesMessage);

	ServerMessageForwarding::end();
}

// ----------------------------------------------------------------------

std::string NameManager::debugGetNameList() const
{
	std::string result;
	
	for (IdToCharacterDataMapType::const_iterator i=m_idToCharacterDataMap->begin(); i!=m_idToCharacterDataMap->end(); ++i)
		result += i->first.getValueString() + " : " + FormattedString<16>().sprintf("%lu", i->second.stationId) + " : " + i->second.characterName + " : " + i->second.characterFullName + " : " + CalendarTime::convertEpochToTimeStringLocal(i->second.createTime) + " : " + CalendarTime::convertEpochToTimeStringLocal(i->second.lastLoginTime) + '\n';

	result += FormattedString<128>().sprintf("(%d total characters)\n", m_idToCharacterDataMap->size());

	return result;
}

// ----------------------------------------------------------------------

std::string NameManager::normalizeName(const std::string &name)
{
	std::string firstName(name, 0, name.find(' '));
	return Unicode::toLower(firstName);
}

// ----------------------------------------------------------------------

/**
 * Generate a random name that is not already used by another player.
 * Technically, the name is not reserved until sent to the database.  However,
 * the odds of two game servers generating the same random name at the same
 * time are very low.  The worst case is a player might be given a random name,
 * then be told to pick another when they finish character creation.
 */
Unicode::String NameManager::generateUniqueRandomName(const std::string &directory, const std::string &nameTable) const
{
	int failcount = 0;
	Unicode::String name = generateRandomName(directory,nameTable);
	while (getPlayerId(normalizeName(Unicode::wideToNarrow(name))) != NetworkId::cms_invalid)
	{
		++failcount;
		WARNING_STRICT_FATAL(failcount > 100, ("GenerateUniqueRandomName couldn't find a name that wasn't already in use.  This may indicate that the random name data tables don't have enough possibilities.  Directory:  %s  name table:  %s", directory.c_str(), nameTable.c_str()));
		if (failcount > 100)
			break;

		name = generateRandomName(directory,nameTable);
	}
	return name;
}

// ----------------------------------------------------------------------

bool NameManager::isPlayer(NetworkId const & possiblePlayer) const
{
	IdToCharacterDataMapType::const_iterator i=m_idToCharacterDataMap->find(possiblePlayer);
	return (i!=m_idToCharacterDataMap->end());
}

// ======================================================================
