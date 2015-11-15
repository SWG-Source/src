// ======================================================================
//
// TaskSetCharacterType.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskUpgradeAccount_H
#define INCLUDED_TaskUpgradeAccount_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class LoginUpgradeAccountMessage;

// ======================================================================

class TaskUpgradeAccount : public DB::TaskRequest
{
  public:
	TaskUpgradeAccount(LoginUpgradeAccountMessage *message, uint32 clusterId);
	~TaskUpgradeAccount();
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

	class SetCharacterTypeQuery : public DB::Query
	{
	public:
		SetCharacterTypeQuery();

		DB::BindableLong         cluster_id; //lint !e1925 // public data member
		DB::BindableLong         station_id; //lint !e1925 // public data member
		DB::BindableNetworkId    character; //lint !e1925 // public data member
		DB::BindableLong         character_type; //lint !e1925 // public data member

		virtual void getSQL                (std::string &sql);
		virtual bool bindParameters        ();
		virtual bool bindColumns           ();
		virtual QueryMode getExecutionMode () const;

	private: //disable
		SetCharacterTypeQuery               (const SetCharacterTypeQuery&);
		SetCharacterTypeQuery &operator=    (const SetCharacterTypeQuery&);
	};

	class QueryJediQuery : public DB::Query
	{
	public:
		QueryJediQuery();

		DB::BindableLong         station_id; //lint !e1925 // public data member
		DB::BindableLong         character_type; //lint !e1925 // public data member
		DB::BindableLong         result; //lint !e1925 // public data member

		virtual void getSQL                (std::string &sql);
		virtual bool bindParameters        ();
		virtual bool bindColumns           ();
		virtual QueryMode getExecutionMode () const;

	private: //disable
		QueryJediQuery               (const QueryJediQuery&);
		QueryJediQuery &operator=    (const QueryJediQuery&);
	};

  private:
	class AddJediQuery : public DB::Query
	{
	  public:
		AddJediQuery();

		DB::BindableLong         cluster_id; //lint !e1925 // public data member
		DB::BindableLong         station_id; //lint !e1925 // public data member

		virtual void getSQL                (std::string &sql);
		virtual bool bindParameters        ();
		virtual bool bindColumns           ();
		virtual QueryMode getExecutionMode () const;

	  private: //disable
		AddJediQuery               (const AddJediQuery&);
		AddJediQuery &operator=    (const AddJediQuery&);
	};

  private:
	TaskUpgradeAccount(); // disable default constructor
	TaskUpgradeAccount(const TaskUpgradeAccount &); // disable copy constructor
	TaskUpgradeAccount & operator = (const TaskUpgradeAccount &); // disable assignment operator
	
	LoginUpgradeAccountMessage *m_message;
	uint32 m_clusterId;
};

// ======================================================================

class TaskOccupyUnlockedSlot : public DB::TaskRequest
{
public:
	TaskOccupyUnlockedSlot(int clusterGroupId, uint32 clusterId, StationId stationId, NetworkId const & characterId, uint32 replyGameServerId);
	~TaskOccupyUnlockedSlot();

public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

private:
	TaskOccupyUnlockedSlot(); // disable default constructor
	TaskOccupyUnlockedSlot(const TaskOccupyUnlockedSlot &); // disable copy constructor
	TaskOccupyUnlockedSlot & operator = (const TaskOccupyUnlockedSlot &); // disable assignment operator

	int m_clusterGroupId;
	uint32 m_clusterId;
	StationId m_stationId;
	NetworkId m_characterId;
	uint32 m_replyGameServerId;
	int m_result;
};

// ======================================================================

class TaskVacateUnlockedSlot : public DB::TaskRequest
{
public:
	TaskVacateUnlockedSlot(int clusterGroupId, uint32 clusterId, StationId stationId, NetworkId const & characterId, uint32 replyGameServerId);
	~TaskVacateUnlockedSlot();

public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

	class GetOnlyOpenCharacterSlotsQuery : public DB::Query
	{
	public:
		GetOnlyOpenCharacterSlotsQuery();

		DB::BindableLong station_id; //lint !e1925 // public data member
		DB::BindableLong cluster_id; //lint !e1925 // public data member
		DB::BindableLong character_type_id; //lint !e1925 // public data member
		DB::BindableLong num_open_slots; //lint !e1925 // public data member

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	private: //disable
		GetOnlyOpenCharacterSlotsQuery(const GetOnlyOpenCharacterSlotsQuery&);
		GetOnlyOpenCharacterSlotsQuery &operator=(const GetOnlyOpenCharacterSlotsQuery&);
	};

private:
	TaskVacateUnlockedSlot(); // disable default constructor
	TaskVacateUnlockedSlot(const TaskVacateUnlockedSlot &); // disable copy constructor
	TaskVacateUnlockedSlot & operator = (const TaskVacateUnlockedSlot &); // disable assignment operator

	int m_clusterGroupId;
	uint32 m_clusterId;
	StationId m_stationId;
	NetworkId m_characterId;
	uint32 m_replyGameServerId;
	int m_result;
};

// ======================================================================

class TaskSwapUnlockedSlot : public DB::TaskRequest
{
public:
	TaskSwapUnlockedSlot(int clusterGroupId, uint32 clusterId, StationId stationId, NetworkId const & sourceCharacterId, NetworkId const & targetCharacterId, uint32 replyGameServerId);
	~TaskSwapUnlockedSlot();

public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

private:
	TaskSwapUnlockedSlot(); // disable default constructor
	TaskSwapUnlockedSlot(const TaskSwapUnlockedSlot &); // disable copy constructor
	TaskSwapUnlockedSlot & operator = (const TaskSwapUnlockedSlot &); // disable assignment operator

	int m_clusterGroupId;
	uint32 m_clusterId;
	StationId m_stationId;
	NetworkId m_sourceCharacterId;
	NetworkId m_targetCharacterId;
	std::string m_targetCharacterName;
	uint32 m_replyGameServerId;
	int m_result;
};

// ======================================================================

#endif
