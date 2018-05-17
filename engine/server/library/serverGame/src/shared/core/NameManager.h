// ======================================================================
//
// NameManager.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NameManager_H
#define INCLUDED_NameManager_H

// ======================================================================

class NameGenerator;
class NetworkId;
class RegexList;

// ======================================================================

/**
 * A singleton that organizes random name generators and reserved/obscene
 * name lists.
 *
 * Random name tables are identified by a directory, which indicates the
 * general type of thing to be named, e.g. player or resource, and a
 * table, which indicates the specific type of name, e.g. Bothan or Human.
 */
class NameManager
{
  public:
	static NameManager &  getInstance        ();
	static void           install            ();

  public:
	int                   getTotalPlayerCount  () const;
	void                  addPlayer            (const NetworkId &id, uint32 stationId, const std::string &name, const std::string &fullName, time_t createTime, time_t lastLoginTime, bool notifyOtherServers);
	void                  addPlayers           (const std::vector<NetworkId> &ids, const std::vector<int> &stationIds, const std::vector<std::string> &names, const std::vector<std::string> &fullNames, const std::vector<int> &createTimes, const std::vector<int> &lastLoginTimes);
	void                  renamePlayer         (const NetworkId &id, const Unicode::String &name, const Unicode::String &fullName);
	std::string           debugGetNameList     () const;
	bool                  isPlayer             (NetworkId const & possiblePlayer) const;
	const NetworkId &     getPlayerId          (const std::string &name) const;
	uint32                getPlayerStationId   (const NetworkId &id) const;
	const std::string &   getPlayerName        (const NetworkId &id) const;
	const std::string &   getPlayerFullName    (const NetworkId &id) const;
	int                   getPlayerCreateTime  (const NetworkId &id) const;
	int                   getPlayerLastLoginTime (const NetworkId &id) const;
	void                  getPlayerWithLastLoginTimeAfter (time_t time, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const;
	void                  getPlayerWithLastLoginTimeBefore (time_t time, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const;
	void                  getPlayerWithLastLoginTimeBetween (time_t timeLowerRange, time_t timeUpperRange, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const;
	void                  getPlayerWithLastLoginTimeAfterDistribution (std::map<int, std::pair<std::string, int> > &result) const;
	void                  getPlayerWithCreateTimeAfter (time_t time, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const;
	void                  getPlayerWithCreateTimeBefore (time_t time, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const;
	void                  getPlayerWithCreateTimeBetween (time_t timeLowerRange, time_t timeUpperRange, std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > &result) const;
	void                  getPlayerWithCreateTimeAfterDistribution (std::map<int, std::pair<std::string, int> > &result) const;
	void                  releasePlayerName    (const NetworkId &id);
	Unicode::String       generateRandomName   (const std::string &directory, const std::string &nameTable) const;
	Unicode::String       generateUniqueRandomName (const std::string &directory, const std::string &nameTable) const;
	bool                  isNameAppropriate    (const std::string &directory, const std::string &nameTable, const Unicode::String &name) const;
	bool                  isNameReserved       (const Unicode::String &name) const;
	bool                  isNameReserved       (const Unicode::String &name, std::string &reasonReserved) const;
	bool                  isNameReserved       (const Unicode::String &name, std::string &reasonReserved, std::vector<std::string> &ignoreRules) const;
	void                  sendAllNamesToServer (std::vector<uint32> const & servers) const;
	static std::string    normalizeName        (const std::string &name);
	
  private:
	const NameGenerator & getNameGenerator   (const std::string &directory, const std::string &nameTable) const;
	
  private:
	class NameTableIdentifier
	{
	  public:
		NameTableIdentifier (const std::string &m_directory, const std::string &m_table);
		bool operator<      (const NameTableIdentifier &rhs) const;
		
	  private:
		std::string          m_directory;
		std::string          m_table;
	};

	struct CharacterData
	{
		uint32 stationId;
		std::string characterName;
		std::string characterFullName;
		time_t createTime;
		time_t lastLoginTime;
	};

	typedef std::map<NameTableIdentifier, NameGenerator*> NameGeneratorMapType;
	typedef std::map<NetworkId, CharacterData> IdToCharacterDataMapType;
	typedef std::map<std::string, NetworkId> NameToIdMapType;

  private:
	mutable NameGeneratorMapType * m_nameGenerators;
	RegexList                    * m_reservedNames;
	NameToIdMapType              * m_nameToIdMap;
	IdToCharacterDataMapType     * m_idToCharacterDataMap;
	
  private:
	NameManager();
	~NameManager();
	static void remove();
	static NameManager *ms_instance;
};

// ----------------------------------------------------------------------

inline NameManager &NameManager::getInstance()
{
	NOT_NULL(ms_instance);
	return *ms_instance;
}

// ======================================================================

#endif
