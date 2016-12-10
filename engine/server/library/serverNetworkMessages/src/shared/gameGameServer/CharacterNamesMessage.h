// ======================================================================
//
// CharacterNamesMessage.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CharacterNamesMessage_H
#define INCLUDED_CharacterNamesMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent From:  GameServer or DatabaseServer
 *      To:    GameServer
 * Action:     Tell the GameServer the object ids of a batch of players.
 *             These will be used to check for name uniqueness, etc.
 */
class CharacterNamesMessage : public GameNetworkMessage
{
  public:
	CharacterNamesMessage(const std::vector<NetworkId> &ids, const std::vector<int> &stationIds, const std::vector<std::string> &characterNames, const std::vector<std::string> &characterFullNames, const std::vector<int> &createTimes, const std::vector<int> &loginTimes);
	CharacterNamesMessage(Archive::ReadIterator & source);
	~CharacterNamesMessage();

	const std::vector<NetworkId>   &getIds() const;
	const std::vector<int>         &getStationIds() const;
	const std::vector<std::string> &getNames() const;
	const std::vector<std::string> &getFullNames() const;
	const std::vector<int>         &getCreateTimes() const;
	const std::vector<int>         &getLoginTimes() const;
		
   private:
	Archive::AutoArray<NetworkId>   m_ids;
	Archive::AutoArray<int>         m_stationIds;
	Archive::AutoArray<std::string> m_names;
	Archive::AutoArray<std::string> m_fullNames;
	Archive::AutoArray<int>         m_createTimes;
	Archive::AutoArray<int>         m_loginTimes;
	
  private:
	//disabled functions:
	CharacterNamesMessage            ();
	CharacterNamesMessage            (const CharacterNamesMessage&);
	CharacterNamesMessage& operator= (const CharacterNamesMessage&);

};

// ======================================================================

inline const std::vector<NetworkId> & CharacterNamesMessage::getIds() const
{
	return m_ids.get();
}

// ----------------------------------------------------------------------

inline const std::vector<int> & CharacterNamesMessage::getStationIds() const
{
	return m_stationIds.get();
}

// ----------------------------------------------------------------------

inline const std::vector<std::string> & CharacterNamesMessage::getNames() const
{
	return m_names.get();
}
		
// ----------------------------------------------------------------------

inline const std::vector<std::string> & CharacterNamesMessage::getFullNames() const
{
	return m_fullNames.get();
}

// ----------------------------------------------------------------------

inline const std::vector<int> & CharacterNamesMessage::getCreateTimes() const
{
	return m_createTimes.get();
}

// ----------------------------------------------------------------------

inline const std::vector<int> & CharacterNamesMessage::getLoginTimes() const
{
	return m_loginTimes.get();
}

// ======================================================================

#endif
