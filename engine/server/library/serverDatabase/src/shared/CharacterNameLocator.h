// ======================================================================
//
// CharacterNameLocator.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CharacterNameLocator_H
#define INCLUDED_CharacterNameLocator_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "serverDatabase/ObjectLocator.h"
#include <vector>

// ======================================================================

/**
 * Load character names and send them to the game server.
 * (Doesn't add any objects to the load, handles the names as custom data.)
 */
class CharacterNameLocator : public ObjectLocator
{
  public:
	virtual bool locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated);
	virtual void sendPostBaselinesCustomData(GameServerConnection &conn) const;

  private:
	struct CharacterNameRow
	{
		DB::BindableNetworkId   character_id;
		DB::BindableLong        character_station_id;
		DB::BindableString<127> character_name;
		DB::BindableString<127> character_full_name;
		DB::BindableLong        character_create_time;
		DB::BindableLong        character_last_login_time;
	};

	class CharacterNameQuery : public DB::Query
	{		
	  public:
		CharacterNameQuery(const std::string &schema);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		const std::vector<CharacterNameRow> & getData() const;
		
	  private:
		
		std::vector<CharacterNameRow> m_data;
		std::string m_schema;
		
	  private:
		CharacterNameQuery(const CharacterNameQuery&);
		CharacterNameQuery& operator=(const CharacterNameQuery&);
	};
	
  private:
	std::vector<NetworkId>   m_characterIds;
	std::vector<int>         m_stationIds;
	std::vector<std::string> m_characterNames;
	std::vector<std::string> m_characterFullNames;
	std::vector<int>         m_characterCreateTime;
	std::vector<int>         m_characterLastLoginTime;
};

// ======================================================================

inline const std::vector<CharacterNameLocator::CharacterNameRow> & CharacterNameLocator::CharacterNameQuery::getData() const
{
	return m_data;
}

// ======================================================================

#endif
