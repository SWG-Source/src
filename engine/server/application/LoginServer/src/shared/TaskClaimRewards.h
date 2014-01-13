// ======================================================================
//
// TaskClaimRewards.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskClaimRewards_H
#define INCLUDED_TaskClaimRewards_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedFoundation/StationId.h"

// ======================================================================

class TaskClaimRewards : public DB::TaskRequest
{
  public:
	TaskClaimRewards(uint32 clusterId, uint32 gameServerId, StationId stationId, NetworkId const & playerId, std::string const & rewardEvent, bool consumeEvent, std::string const & rewardItem, bool consumeItem, uint32 accountFeatureId, bool consumeAccountFeatureId, int previousAccountFeatureIdCount, int currentAccountFeatureIdCount);
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	uint32 const m_clusterId;
	uint32 const m_gameServerId;
	StationId const m_stationId;
	NetworkId const m_playerId;
	std::string const m_rewardEvent;
	bool const m_consumeEvent;
	std::string const m_rewardItem;
	bool const m_consumeItem;
	uint32 const m_accountFeatureId;
	bool const m_consumeAccountFeatureId;
	int m_previousAccountFeatureIdCount;
	int m_currentAccountFeatureIdCount;
	bool m_result;

  private:
	TaskClaimRewards(); // disabled default constructor
};

// ======================================================================

class TaskFeatureIdTransactionRequest : public DB::TaskRequest
{
public:
	TaskFeatureIdTransactionRequest(uint32 clusterId, StationId stationId, NetworkId const & playerId, uint32 gameServerId);

public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

private:
	uint32 const m_clusterId;
	StationId const m_stationId;
	NetworkId const m_playerId;
	uint32 const m_gameServerId;
	std::map<std::string, int> m_transactions;

private:
	TaskFeatureIdTransactionRequest(); // disabled default constructor
};

// ======================================================================

class TaskFeatureIdTransactionSyncUpdate : public DB::TaskRequest
{
public:
	TaskFeatureIdTransactionSyncUpdate(uint32 clusterId, StationId stationId, NetworkId const & playerId, std::string const & itemId, int adjustment);

public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

private:
	uint32 const m_clusterId;
	StationId const m_stationId;
	NetworkId const m_playerId;
	std::string const m_itemId;
	int const m_adjustment;

private:
	TaskFeatureIdTransactionSyncUpdate(); // disabled default constructor
};

// ======================================================================

#endif
