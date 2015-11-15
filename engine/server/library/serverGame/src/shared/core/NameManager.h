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
	void                  addPlayers           (const stdvector<NetworkId>::fwd &ids, const stdvector<int>::fwd &stationIds, const stdvector<std::string>::fwd &names, const stdvector<std::string>::fwd &fullNames, const stdvector<int>::fwd &createTimes, const stdvector<int>::fwd &lastLoginTimes);
	void                  renamePlayer         (const NetworkId &id, const Unicode::String &name, const Unicode::String &fullName);
	std::string           debugGetNameList     () const;
	bool                  isPlayer             (NetworkId const & possiblePlayer) const;
	const NetworkId &     getPlayerId          (const std::string &name) const;
	uint32                getPlayerStationId   (const NetworkId &id) const;
	const std::string &   getPlayerName        (const NetworkId &id) const;
	const std::string &   getPlayerFullName    (const NetworkId &id) const;
	int                   getPlayerCreateTime  (const NetworkId &id) const;
	int                   getPlayerLastLoginTime (const NetworkId &id) const;
	void                  getPlayerWithLastLoginTimeAfter (time_t time, stdmultimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::fwd &result) const;
	void                  getPlayerWithLastLoginTimeBefore (time_t time, stdmultimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::fwd &result) const;
	void                  getPlayerWithLastLoginTimeBetween (time_t timeLowerRange, time_t timeUpperRange, stdmultimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::fwd &result) const;
	void                  getPlayerWithLastLoginTimeAfterDistribution (stdmap<int, std::pair<std::string, int> >::fwd &result) const;
	void                  getPlayerWithCreateTimeAfter (time_t time, stdmultimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::fwd &result) const;
	void                  getPlayerWithCreateTimeBefore (time_t time, stdmultimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::fwd &result) const;
	void                  getPlayerWithCreateTimeBetween (time_t timeLowerRange, time_t timeUpperRange, stdmultimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::fwd &result) const;
	void                  getPlayerWithCreateTimeAfterDistribution (stdmap<int, std::pair<std::string, int> >::fwd &result) const;
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

	typedef stdmap<NameTableIdentifier, NameGenerator*>::fwd NameGeneratorMapType;
	typedef stdmap<NetworkId, CharacterData>::fwd IdToCharacterDataMapType;
	typedef stdmap<std::string, NetworkId>::fwd NameToIdMapType;

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
