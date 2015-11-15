// TaskEnumCluster.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskEnumCluster_H
#define	_INCLUDED_TaskEnumCluster_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class TaskEnumCluster : public GameNetworkMessage
{
public:
	explicit TaskEnumCluster(const std::vector<std::string> & clusterNames);
	explicit TaskEnumCluster(Archive::ReadIterator & s);
	~TaskEnumCluster();

	const std::vector<std::string> & getClusterNames() const;

private:
	TaskEnumCluster & operator = (const TaskEnumCluster & rhs);
	TaskEnumCluster(const TaskEnumCluster & source);

private:
	Archive::AutoArray<std::string>  clusterNames;
};

//-----------------------------------------------------------------------

inline const std::vector<std::string> & TaskEnumCluster::getClusterNames() const
{
	return clusterNames.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskEnumCluster_H
