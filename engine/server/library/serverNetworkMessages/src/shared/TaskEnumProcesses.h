// TaskEnumProcesses.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskEnumProcesses_H
#define	_INCLUDED_TaskEnumProcesses_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/NetworkMessageFactory.h"

//-----------------------------------------------------------------------

class TaskEnumProcesses : public GameNetworkMessage
{
public:
	TaskEnumProcesses(const std::string & hostAddress, const std::vector<std::string> & commandLines, const std::vector<uint32> & pids, const std::vector<bool> & loadedOnStartup);
	TaskEnumProcesses(Archive::ReadIterator & source);
	~TaskEnumProcesses();

	const std::vector<std::string> &    getCommandLines     () const;
	const std::string  &                getHostAddress      () const;
	const std::vector<bool> &           getLoadedOnStartup  () const;
	const std::vector<uint32> &         getPids             () const;

private:
	TaskEnumProcesses & operator = (const TaskEnumProcesses & rhs);
	TaskEnumProcesses(const TaskEnumProcesses & source);

private:
	Archive::AutoArray<std::string>     commandLines;
	Archive::AutoVariable<std::string>  hostAddress;
	Archive::AutoArray<bool>            loadedOnStartup;
	Archive::AutoArray<uint32>          pids;
};


//-----------------------------------------------------------------------

inline const std::vector<std::string> & TaskEnumProcesses::getCommandLines() const
{
	return commandLines.get();
}

//-----------------------------------------------------------------------

inline const std::string & TaskEnumProcesses::getHostAddress() const
{
	return hostAddress.get();
}

//-----------------------------------------------------------------------

inline const std::vector<bool> & TaskEnumProcesses::getLoadedOnStartup() const
{
	return loadedOnStartup.get();
}

//-----------------------------------------------------------------------

inline const std::vector<uint32> & TaskEnumProcesses::getPids() const
{
	return pids.get();
}
//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskEnumProcesses_H
