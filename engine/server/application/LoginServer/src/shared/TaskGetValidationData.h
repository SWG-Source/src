// ======================================================================
//
// TaskGetValidationData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskGetValidationData_H
#define INCLUDED_TaskGetValidationData_H

// ======================================================================

#include "serverNetworkMessages/AvatarList.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

class TransferRequestMoveValidation;

// ======================================================================

class TaskGetValidationData : public DB::TaskRequest
{
  public:
	TaskGetValidationData (StationId stationId, int clusterGroupId, uint32 clusterId, unsigned int track, uint32 subscriptionBits);
	TaskGetValidationData (const TransferRequestMoveValidation & request, int clusterGroupId, uint32 clusterId);
	~TaskGetValidationData();	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskGetValidationData(); // disabled default constructor

  private:
	StationId m_stationId;
	int m_clusterGroupId;
	uint32 m_clusterId;
	std::vector<int> m_openCharacterSlots;
	bool m_canSkipTutorial;
	unsigned int  m_track;
	uint32 m_subscriptionBits;
	TransferRequestMoveValidation *  m_transferRequest;
	uint32 m_transferRequestSourceCharacterTemplateId;
	std::vector<std::pair<NetworkId, std::string> > m_consumedRewardEvents;
	std::vector<std::pair<NetworkId, std::string> > m_claimedRewardItems;
	
  private:
	TaskGetValidationData(const TaskGetValidationData&); //disable
	TaskGetValidationData &operator=(const TaskGetValidationData&); //disable
};

// ======================================================================

#endif
