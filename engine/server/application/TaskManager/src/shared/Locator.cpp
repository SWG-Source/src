// ======================================================================
//
// Locator.cpp
//
// Copyright 2000-04 Sony Online Entertainment
//
// ======================================================================

#include "FirstTaskManager.h"
#include "Locator.h"

#include "ConfigTaskManager.h"
#include "ManagerConnection.h"
#include "TaskManager.h"
#include "serverNetworkMessages/TaskUtilization.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedLog/Log.h"
#include <algorithm>
#include <vector>

// ======================================================================

namespace LocatorNamespace
{
	float getConfigSetting(const char *section, const char *key, const float defaultValue)
	{
		const ConfigFile::Section * sec = ConfigFile::getSection(section);
		if (sec == nullptr)
			return defaultValue;

		const ConfigFile::Key * ky = sec->findKey(key);
		if (ky == nullptr)
			return defaultValue;
		
		return ky->getAsFloat(ky->getCount()-1, defaultValue);
	}

	struct PreferredNode
	{
		std::string m_nodeLabel;
		std::string m_processName;
		std::vector<std::string> m_nodeOptions;

		bool match(std::string const &processName, std::string const &options) const
		{
			if (m_processName != processName)
				return false;
			for (std::vector<std::string>::const_iterator i = m_nodeOptions.begin(); i != m_nodeOptions.end(); ++i)
				if (options.find(*i) == std::string::npos)
					return false;
			return true;
		}
	};

	struct ServerEntry
	{
		ServerEntry(ManagerConnection *conn, std::string const &label, float load);

		bool operator==(std::string const &label) const
		{
			return m_label == label;
		}
		bool operator<(ServerEntry const & rhs) const
		{
			// don't change this; the code requires that the nodes
			// list be ordered by the current load on the node
			return m_load < rhs.m_load;
		}
		bool hasAvailableLoad(float cost) const
		{
			return ((m_load + cost) < m_maximumLoad);
		}
		
		ManagerConnection *m_connection;
		std::string        m_label;
		float              m_load;
		float              m_maximumLoad;
	};

	ServerEntry::ServerEntry(ManagerConnection *conn, std::string const &label, float load) :
		m_connection(conn),
		m_label(label),
		m_load(load),
		m_maximumLoad(0.0f)
	{
		// see if there is a maximum load specified for this particular node
		char key[128];
		snprintf(key, sizeof(key)-1, "maximumLoad.%s", m_label.c_str());
		key[sizeof(key)-1] = '\0';

		m_maximumLoad = getConfigSetting("TaskManager", key, ConfigTaskManager::getMaximumLoad());
	}

	std::vector<ServerEntry>           s_serverList;
	const char                         s_masterNodeLabel[] = "node0";
	float                              s_myLoad;
	std::vector<PreferredNode>         s_preferredNodes;
	std::map<std::string, std::string> s_closedConnections; // to keep track of TaskManager that we have lost connection to which has not yet reconnected

	ManagerConnection *getPreferredServer(std::string const &processName, std::string const &options, float cost);
	ManagerConnection *getUnpreferredServer(float cost);
	void addPreferredNode(std::string const &preferredNodeSpec);
}

using namespace LocatorNamespace;

// ======================================================================

void Locator::install()
{
	int index = 0;
	char const * result = 0;

	do
	{
		result = ConfigFile::getKeyString("TaskManager", "preferredNode", index++, 0);
		if (result)
			addPreferredNode(result);
	}
	while (result);
}

// ----------------------------------------------------------------------

void Locator::closed(std::string const &label, ManagerConnection const *oldConnection)
{
	if (oldConnection)
	{
		std::vector<ServerEntry>::iterator i = std::find(s_serverList.begin(), s_serverList.end(), label);
		if (i != s_serverList.end())
		{
			DEBUG_REPORT_LOG(true, ("Removing connection %s on %s from locator map\n", label.c_str(), oldConnection->getRemoteAddress().c_str()));
			IGNORE_RETURN(s_serverList.erase(i));
			s_closedConnections[label] = oldConnection->getRemoteAddress();
		}
	}
}

// ----------------------------------------------------------------------

ManagerConnection *LocatorNamespace::getPreferredServer(std::string const &processName, std::string const &options, float cost)
{
	// Find a node other than the master node that can afford to run the process
	// and is a preferred node for the process and option set.
	for (std::vector<ServerEntry>::iterator i = s_serverList.begin(); i != s_serverList.end(); ++i)
	{
		ServerEntry &e = *i;
		if ((e.m_label != s_masterNodeLabel) && (e.hasAvailableLoad(cost)))
		{		
			for (std::vector<PreferredNode>::const_iterator j = s_preferredNodes.begin(); j != s_preferredNodes.end(); ++j)
				if ((*j).m_nodeLabel == e.m_label && (*j).match(processName, options))
					return e.m_connection;
		} 
	}	
	return 0;
}

// ----------------------------------------------------------------------

bool Locator::isMasterNodePreferred(std::string const &processName, std::string const &options, float cost)
{
	if(!ConfigTaskManager::getAllowPreferredServerOnMasterNode())
		return false;

	for (std::vector<PreferredNode>::const_iterator j = s_preferredNodes.begin(); j != s_preferredNodes.end(); ++j)
		if ((*j).m_nodeLabel == s_masterNodeLabel && (*j).match(processName, options))
			return true;
				
	return false;
}

// ----------------------------------------------------------------------

ManagerConnection *LocatorNamespace::getUnpreferredServer(float cost)
{	
	// Find a node other than the master node that can afford to run the process
	// and is a not marked as a preferred node for anything; if more than one
	// node qualifies, return the node with the lowest load
	for (std::vector<ServerEntry>::iterator i = s_serverList.begin(); i != s_serverList.end(); ++i)
	{
		ServerEntry &e = *i;
		if ((e.m_label != s_masterNodeLabel) && (e.hasAvailableLoad(cost)))
		{
			bool foundPreferred = false;
			for (std::vector<PreferredNode>::const_iterator j = s_preferredNodes.begin(); j != s_preferredNodes.end(); ++j)
			{
				if ((*j).m_nodeLabel == e.m_label)
				{
					foundPreferred = true;
					break;
				}
			}

			// s_serverList is ordered by load, so this will
			// return the available node with the lowest load
			if (!foundPreferred)
				return e.m_connection;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------

ManagerConnection *Locator::getBestServer(std::string const &processName, std::string const &options, float cost)
{	
	if (s_serverList.empty())		
		return 0;

	// first, look for a preferred node for this process
	{
		ManagerConnection * const connection = getPreferredServer(processName, options, cost);
		if (connection)
			return connection;
	}

	// there is not a preferred node available, so look for a node which is not preferred in some form
	{
		ManagerConnection * const connection = getUnpreferredServer(cost);
		if (connection)
			return connection;
	}

	// there is not a non-preferred node available, so look for any node other than the command node;
	// If more than one node qualifies, we want to choose the one who currently has the lowest load
	{
		for (std::vector<ServerEntry>::iterator i = s_serverList.begin(); i != s_serverList.end(); ++i)
		{
			ServerEntry &e = *i;
			if ((e.m_label != s_masterNodeLabel) && (e.hasAvailableLoad(cost)))
			{
				// s_serverList is ordered by load, so this will
				// return the available node with the lowest load
				return e.m_connection;
			}
		}

		WARNING(true, ("No hosts are available to spawn a process costing %f without exceeding the host's load limit", cost));
	}
						 
	return 0;
}

// ----------------------------------------------------------------------

ManagerConnection *Locator::getServer(std::string const &label)
{
	for (std::vector<ServerEntry>::const_iterator i = s_serverList.begin(); i != s_serverList.end(); ++i)
		if (i->m_label == label || i->m_connection->getRemoteAddress() == label)
			return i->m_connection;

	return 0;
}

// ----------------------------------------------------------------------

float Locator::getServerLoad(std::string const &label)
{
	std::vector<ServerEntry>::const_iterator i = std::find(s_serverList.begin(), s_serverList.end(), label);
	if (i != s_serverList.end())
		return i->m_load;
	return 0.f;
}

// ----------------------------------------------------------------------

float Locator::getServerMaximumLoad(std::string const &label)
{
	std::vector<ServerEntry>::const_iterator i = std::find(s_serverList.begin(), s_serverList.end(), label);
	if (i != s_serverList.end())
		return i->m_maximumLoad;

	// see if there is a maximum load specified for this particular node
	char key[128];
	snprintf(key, sizeof(key)-1, "maximumLoad.%s", label.c_str());
	key[sizeof(key)-1] = '\0';

	return getConfigSetting("TaskManager", key, ConfigTaskManager::getMaximumLoad());
}

// ----------------------------------------------------------------------

void Locator::incrementMyLoad(float amount)
{
	s_myLoad += amount;
	std::sort(s_serverList.begin(), s_serverList.end());
}

// ----------------------------------------------------------------------

void Locator::decrementMyLoad(float amount)
{
	s_myLoad -= amount;
	if (s_myLoad < 0.0f)
		s_myLoad = 0.0f;
	std::sort(s_serverList.begin(), s_serverList.end());
	updateAllLoads(-amount);
}

// ----------------------------------------------------------------------

void Locator::incrementServerLoad(std::string const &label, float amount)
{
	std::vector<ServerEntry>::iterator i = std::find(s_serverList.begin(), s_serverList.end(), label);
	if (i != s_serverList.end())
	{
		i->m_load += amount;
		std::sort(s_serverList.begin(), s_serverList.end());
	}
}

// ----------------------------------------------------------------------

void Locator::decrementServerLoad(std::string const &label, float amount)
{
	std::vector<ServerEntry>::iterator i = std::find(s_serverList.begin(), s_serverList.end(), label);
	if (i != s_serverList.end())
	{
		i->m_load -= amount;
		if (i->m_load < 0.0f)
		{
			DEBUG_WARNING(true, ("%s dropped less than 0", label.c_str()));
			i->m_load = 0.0f;
		}
		std::sort(s_serverList.begin(), s_serverList.end());
	}
}

// ----------------------------------------------------------------------

float Locator::getMyLoad()
{
	return s_myLoad;
}

// ----------------------------------------------------------------------

float Locator::getMyMaximumLoad()
{
	static float myMaximumLoad = -1.0f;

	// get the maximum load for this particular node
	if (myMaximumLoad <= 0.0f)
	{
		// see if there is a maximum load specified for this particular node
		char key[128];
		snprintf(key, sizeof(key)-1, "maximumLoad.%s", TaskManager::getNodeLabel().c_str());
		key[sizeof(key)-1] = '\0';

		myMaximumLoad = getConfigSetting("TaskManager", key, ConfigTaskManager::getMaximumLoad());
	}

	return myMaximumLoad;
}

// ----------------------------------------------------------------------

void Locator::opened(std::string const &label, ManagerConnection *newConnection)
{
	if (newConnection)
	{
		std::vector<ServerEntry>::const_iterator i = std::find(s_serverList.begin(), s_serverList.end(), label);
		if (i == s_serverList.end())
		{
			ServerEntry s(newConnection, label, 0.0f);
			s_serverList.push_back(s);
			std::sort(s_serverList.begin(), s_serverList.end());
			IGNORE_RETURN(s_closedConnections.erase(label));

			LOG("TaskManagerConnect", ("Connection with remote task manager %s established with label %s, maximum load %f", newConnection->getRemoteAddress().c_str(), label.c_str(), s.m_maximumLoad));
		}
		else
		{
			WARNING_STRICT_FATAL(true, ("Tried to add %s on %s to server list but it already exsists", label.c_str(), newConnection->getRemoteAddress().c_str()));
		}
		DEBUG_REPORT_LOG(true, ("Connection with remote task manager %s established with label %s\n", newConnection->getRemoteAddress().c_str(), label.c_str()));
		TaskManager::runSpawnRequestQueue();
	}
	else
		WARNING_STRICT_FATAL(true, ("Passed nullptr connection to Locator::opened"));
}

// ----------------------------------------------------------------------

void Locator::updateServerLoad(std::string const &label, float load)
{
	std::vector<ServerEntry>::iterator i = std::find(s_serverList.begin(), s_serverList.end(), label);
	if(i != s_serverList.end())
	{
		i->m_load += load;
		std::sort(s_serverList.begin(), s_serverList.end());
		LOG("TaskManager", ("updateServerLoad(%s, %f)", label.c_str(), load));
	}
	else
	{
		DEBUG_WARNING(true, ("Told to update load on %s which didn't exist\n", label.c_str()));
	}
	TaskManager::runSpawnRequestQueue();
}

// ----------------------------------------------------------------------

void Locator::updateAllLoads(float delta)
{
	if (!s_serverList.empty())
	{
		TaskUtilization const util(TaskUtilization::SYSTEM_AVG, delta);
		sendToAllTaskManagers(util);
	}
}

// ----------------------------------------------------------------------

std::map<std::string, std::string> const & Locator::getClosedConnections()
{
	return s_closedConnections;
}

// ----------------------------------------------------------------------

void Locator::sendToAllTaskManagers(GameNetworkMessage const &msg)
{
	if (!s_serverList.empty())
		for (std::vector<ServerEntry>::const_iterator i = s_serverList.begin(); i != s_serverList.end(); ++i)
			i->m_connection->send(msg);
}

// ----------------------------------------------------------------------

void LocatorNamespace::addPreferredNode(std::string const &preferredNodeSpec)
{
	// preferredNodeSpec is "nodeLabel:processName:option=value:option=value:..."

	PreferredNode n;
	std::string::size_type lastPos = 0;
	std::string::size_type pos;

	pos = preferredNodeSpec.find(':', lastPos);
	n.m_nodeLabel = preferredNodeSpec.substr(lastPos, pos-lastPos);
	lastPos = pos == std::string::npos ? preferredNodeSpec.size() : pos+1;

	pos = preferredNodeSpec.find(':', lastPos);
	n.m_processName = preferredNodeSpec.substr(lastPos, pos-lastPos);
	lastPos = pos == std::string::npos ? preferredNodeSpec.size() : pos+1;

	while (lastPos != std::string::npos)
	{
		pos = preferredNodeSpec.find(':', lastPos);
		n.m_nodeOptions.push_back(preferredNodeSpec.substr(lastPos, pos-lastPos));
		lastPos = pos == std::string::npos ? std::string::npos : pos+1;
	}

	if (!n.m_nodeLabel.empty() && !n.m_processName.empty())
		s_preferredNodes.push_back(n);
}

// ======================================================================

