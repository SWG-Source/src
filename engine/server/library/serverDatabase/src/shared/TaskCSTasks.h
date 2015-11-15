// ======================================================================
//
// TaskCSTasks.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskCSTasks_H
#define INCLUDED_TaskCSTasks_H

// ======================================================================

#include <vector>

#include "Unicode.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedFoundation/StationId.h"

//-----------------------------------------------------------------------

class TaskGetCharactersForAccount:public DB::TaskRequest
{
  public:
	explicit TaskGetCharactersForAccount( uint32 accountId, uint32 loginServerId, uint32 toolId );
	virtual ~TaskGetCharactersForAccount();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();


  private:
	class CSGetCharactersQuery : public DB::Query
	{
	  public:
		CSGetCharactersQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;
		
	  public:
		// input:
		DB::BindableLong        station_id; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableNetworkId   character_id; //lint !e1925 public data member Suppresed because it's in a private inner class
		
		//output:
		DB::BindableString<128> character_name; //lint !e1925 public data member Suppresed because it's in a private inner class

	  private:
		CSGetCharactersQuery(const CSGetCharactersQuery&);
		CSGetCharactersQuery& operator=(const CSGetCharactersQuery&);
		
	  public:
		struct CharacterRow
		{
			DB::BindableNetworkId characterId;
			DB::BindableString< 40 > characterName;
		};

	  private:
		std::vector<CharacterRow> m_data;
		
	public:		
		const std::vector<CharacterRow> & getData() const;
				

	};

  private:
	class CharacterRecord
	{
	  public:
		StationId m_suid; //lint !e1925 public data member Suppresed because it's in a private inner class
		NetworkId m_characterId; //lint !e1925 public data member Suppresed because it's in a private inner class
		
		Unicode::String m_characterName; //lint !e1925 public data member Suppresed because it's in a private inner class
	};

	std::vector<CharacterRecord*> m_characters;
	
	uint32 m_loginServerId;
	uint32 m_toolId;
	uint32 m_accountId; // target account ID
	
  private:
	//Disabled:
	TaskGetCharactersForAccount();
	TaskGetCharactersForAccount(const TaskGetCharactersForAccount&);
	TaskGetCharactersForAccount& operator=(const TaskGetCharactersForAccount& rhs);
};

inline const std::vector<TaskGetCharactersForAccount::CSGetCharactersQuery::CharacterRow> & TaskGetCharactersForAccount::CSGetCharactersQuery::getData() const
{
	return m_data;
}
//-----------------------------------------------------------------------

class TaskGetDeletedItems:public DB::TaskRequest
{
  public:
	explicit TaskGetDeletedItems(const NetworkId &characterId, uint32 loginServerId, uint32 toolId, uint32 pageNumber);
	virtual ~TaskGetDeletedItems();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();
	
  private:
	class CSGetDeletedItemsQuery : public DB::Query
	{
	  public:
		CSGetDeletedItemsQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;
		
	  public:
		// input:
		DB::BindableNetworkId        character_id; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableLong	     page_number;

	  private:
		CSGetDeletedItemsQuery(const CSGetDeletedItemsQuery&);
		CSGetDeletedItemsQuery& operator=(const CSGetDeletedItemsQuery&);
		
	  public:
		struct DeletedItemRow
		{
			DB::BindableNetworkId itemId;
			DB::BindableString<501> itemTable;
			DB::BindableString<501> itemName;
			DB::BindableString<129> displayName;
		};

	  private:
		std::vector<DeletedItemRow> m_data;
		
	public:		
		const std::vector<DeletedItemRow> & getData() const;
				

	};
  private:
	class DeletedItemRecord
	{
	  public:
		NetworkId m_itemId; //lint !e1925 public data member Suppresed because it's in a private inner class
		
		Unicode::String m_itemTable;
		Unicode::String m_itemName; //lint !e1925 public data member Suppresed because it's in a private inner class
		Unicode::String m_displayName;
	};

	std::vector<DeletedItemRecord*> m_items;

	
	uint32 m_loginServerId;
	uint32 m_toolId;
	NetworkId m_characterId; // target account ID
	uint32 m_pageNumber; // starting number of list
	
};

inline const std::vector<TaskGetDeletedItems::CSGetDeletedItemsQuery::DeletedItemRow> & TaskGetDeletedItems::CSGetDeletedItemsQuery::getData() const
{
	return m_data;
}


//-----------------------------------------------------------------------

class TaskGetCharacterId:public DB::TaskRequest
{
  public:
	explicit TaskGetCharacterId( const std::string &characterName, uint32 loginServerId, uint32 toolId );
	virtual ~TaskGetCharacterId();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();
	
	enum
	{
		MAX_NAME_LENGTH = 40
	};
	
  private:
	class CSGetCharacterIdQuery : public DB::Query
	{
	  public:
		CSGetCharacterIdQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;
		
	  public:
		// input:
		DB::BindableString<TaskGetCharacterId::MAX_NAME_LENGTH>        character_name; //lint !e1925 public data member Suppresed because it's in a private inner class

	  private:
		CSGetCharacterIdQuery(const CSGetCharacterIdQuery&);
		CSGetCharacterIdQuery& operator=(const CSGetCharacterIdQuery&);
		
	  public:
		struct CharacterIdRow
		{
			DB::BindableNetworkId characterId;
			DB::BindableLong stationId;
			DB::BindableString<TaskGetCharacterId::MAX_NAME_LENGTH> characterName;
		};

	  private:
		std::vector<CharacterIdRow> m_data;
		
	public:		
		const std::vector<CharacterIdRow> & getData() const;
				

	};
  private:
	class CharacterIdRecord
	{
	  public:
		NetworkId m_characterId; //lint !e1925 public data member Suppresed because it's in a private inner class
		uint32    m_stationId;
	};

	std::vector<CharacterIdRecord*> m_characterIds;

	
	uint32 m_loginServerId;
	uint32 m_toolId;
	std::string m_characterName; // target account ID	
	
};

inline const std::vector<TaskGetCharacterId::CSGetCharacterIdQuery::CharacterIdRow> & TaskGetCharacterId::CSGetCharacterIdQuery::getData() const
{
	return m_data;
}

class TaskMovePlayer : public DB::TaskRequest
{
  public:
	explicit TaskMovePlayer( const NetworkId & id, const std::string & scene, double x, double y, double z );
	virtual ~TaskMovePlayer();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();
	
	enum
	{
		MAX_SCENE_NAME_LENGTH =  40
	};
  private:
	class CSMovePlayerQuery : public DB::Query
	{
		public:
		CSMovePlayerQuery(const NetworkId & characterIdIn, const std::string & scene, const float xIn, const float yIn, const float zIn);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
	
		private:
		DB::BindableNetworkId characterId;
		DB::BindableString<TaskMovePlayer::MAX_SCENE_NAME_LENGTH> newScene;
		DB::BindableDouble x;
		DB::BindableDouble y;
		DB::BindableDouble z;
	
		private:
		CSMovePlayerQuery(const CSMovePlayerQuery &); // disable
		CSMovePlayerQuery & operator=(const CSMovePlayerQuery &); //disable
	};  
  	NetworkId m_characterId;
	std::string m_scene;
	double m_x;
	double m_y;
	double m_z;
		
};
	
//-----------------------------------------------------------------------

class TaskCsUndeleteItem : public DB::TaskRequest
{
  public:
	explicit TaskCsUndeleteItem( const NetworkId & characterId, const NetworkId & itemId, bool move );
	virtual ~TaskCsUndeleteItem();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();
  private:
	class CsUndeleteItemQuery : public DB::Query
	{
		public:
		CsUndeleteItemQuery(const NetworkId & characterIdIn, const NetworkId & itemIdIn, bool moveIn);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
	
		private:
		DB::BindableNetworkId characterId;
		DB::BindableNetworkId itemId;
		DB::BindableLong move;
	
		private:
		CsUndeleteItemQuery(const CsUndeleteItemQuery &); // disable
		CsUndeleteItemQuery & operator=(const CsUndeleteItemQuery &); //disable
	};  
  	NetworkId m_characterId;
	NetworkId m_itemId;
	bool m_move;
		
};

#endif // INCLUDED_TaskCSTasks_H

