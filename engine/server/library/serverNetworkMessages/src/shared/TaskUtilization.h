// TaskUtilization.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskUtilization_H
#define	_INCLUDED_TaskUtilization_H
//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------
      
class TaskUtilization : public GameNetworkMessage
{
public:
	enum UtilTypes
	{
		SYSTEM_CPU,
		SYSTEM_MEMORY,
		SYSTEM_NETWORK_IO,
		SYSTEM_AVG,
		PROCESS_CPU,
		PROCESS_MEMORY,
		PROCESS_NETWORK_IO
	};

	TaskUtilization(const unsigned char utilType, const float utilAmount);
	TaskUtilization(Archive::ReadIterator & source);
	~TaskUtilization();

	const unsigned char  getUtilType() const;
	const float          getUtilAmount() const;

private:
	TaskUtilization & operator = (const TaskUtilization & rhs);
	TaskUtilization(const TaskUtilization & source);

	Archive::AutoVariable<unsigned char>  utilType;
	Archive::AutoVariable<float>          utilAmount;
};

//-----------------------------------------------------------------------

inline const unsigned char TaskUtilization::getUtilType() const
{
	return utilType.get();
}

//-----------------------------------------------------------------------

inline const float TaskUtilization::getUtilAmount() const
{
	return utilAmount.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskUtilization_H
