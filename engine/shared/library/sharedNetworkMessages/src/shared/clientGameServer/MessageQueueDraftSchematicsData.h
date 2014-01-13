//======================================================================
//
// MessageQueueDraftSchematicsData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueDraftSchematicsData_H
#define INCLUDED_MessageQueueDraftSchematicsData_H

//======================================================================

struct MessageQueueDraftSchematicsData
{
	uint32 serverCrc;
	uint32 sharedCrc;
	int    category;
	
	MessageQueueDraftSchematicsData(uint32 _serverCrc, uint32 _sharedCrc, int _category);
};


inline MessageQueueDraftSchematicsData::MessageQueueDraftSchematicsData(
	uint32 _serverCrc, uint32 _sharedCrc, int _category) :
	serverCrc(_serverCrc),
	sharedCrc(_sharedCrc),
	category(_category)
{
}

//======================================================================

#endif
