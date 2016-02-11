// ======================================================================
//
// DataTableManager.cpp
// 
// copyright 2002 Sony Online Entertainment
//
// ======================================================================


#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/DataTableManager.h"

#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableCell.h"
#include <map>

// ----------------------------------------------------------------------

DataTable * DataTableManager::m_cachedTable = 0;
std::string DataTableManager::m_cachedTableName = "";
bool DataTableManager::m_errorChecking = false;
std::map<std::string, DataTable *> DataTableManager::m_tables;
bool DataTableManager::m_installed = false;
std::multimap<std::string, DataTableReloadCallback> DataTableManager::m_reloadCallbacks;

// ----------------------------------------------------------------------

void DataTableManager::install(bool verboseErrorChecking)
{
	FATAL(m_installed, ("DataTableManager::install: already installed."));
	m_errorChecking = verboseErrorChecking;
	ExitChain::add(DataTableManager::remove, "DataTableManager::remove");

	DataTableCell::install();

	m_installed = true;
}

// ----------------------------------------------------------------------

void DataTableManager::remove()
{
	FATAL(!m_installed, ("DataTableManager::remove: not installed."));
	m_cachedTable = 0;
	m_cachedTableName.clear();

	std::map<std::string, DataTable *>::iterator i;
	for (i=m_tables.begin(); i != m_tables.end(); ++i)
	{
		DataTable * toDelete = i->second;
		delete toDelete;
	}

	m_tables.clear();
	m_reloadCallbacks.clear();

	DataTableCell::remove();

	m_installed = false;
}

// ----------------------------------------------------------------------

DataTable* DataTableManager::open(const std::string& table)
{
	FATAL(!m_installed, ("DataTableManager::open: not installed."));
	DataTable *retVal = getTable(table, false);
	if (retVal)
		return retVal;

	if (!TreeFile::exists(table.c_str()))
	{
		DEBUG_WARNING(true, ("Could not find treefile table for open [%s]", table.c_str()));
		return 0;
	}

	Iff iff(table.c_str(), false);
	retVal = new DataTable;
	retVal->load(iff);

	m_cachedTable = retVal;
	m_cachedTableName = table;
	m_tables[table] = retVal;

	return retVal;
}
// ----------------------------------------------------------------------


void DataTableManager::close(const std::string& table)
{
//	FATAL(!m_installed, ("DataTableManager::close: not installed."));
	std::map<std::string, DataTable *>::iterator i = m_tables.find(table);
	if (i == m_tables.end())
	{
		DEBUG_WARNING(true, ("Could not find loaded table for close [%s]", table.c_str()));
		return;
	}

	if (m_cachedTable == i->second)
	{
		DEBUG_WARNING(m_cachedTableName != table, ("Cached name got out of sync"));
		m_cachedTableName.clear();
		m_cachedTable = 0;
	}

	DataTable * dt = i->second;
	m_tables.erase(i);
	delete dt;
}
// ----------------------------------------------------------------------


DataTable * DataTableManager::getTable(const std::string& table, bool openIfNotFound)
{
//	FATAL(!m_installed, ("DataTableManager::getTable: not installed."));
	if (m_cachedTableName == table)
		return m_cachedTable;

	std::map<std::string, DataTable *>::iterator i = m_tables.find(table);
	if (i == m_tables.end())
	{
		if (openIfNotFound)
		{
			DataTable * dt = open(table);
			if (!dt)
			{
				DEBUG_WARNING(true, ("Could not find table [%s]", table.c_str()));
				return nullptr;
			}
			else
			{
				m_cachedTableName = table;
				m_cachedTable = dt;
				return dt;
			}
		}
		else
		{
			return nullptr;
		}
	}

	m_cachedTableName = table;
	m_cachedTable = i->second;
	return i->second;
}

// ----------------------------------------------------------------------

DataTable*  DataTableManager::reload(const std::string & table)
{
//	FATAL(!m_installed, ("DataTableManager::reload: not installed."));
	close(table);

	DataTable * const dataTable = open(table);

	if (dataTable != nullptr)
	{
		std::multimap<std::string, DataTableReloadCallback>::const_iterator i = m_reloadCallbacks.lower_bound(table);
		for (; i != m_reloadCallbacks.end() && (*i).first == table; ++i)
			(*((*i).second))(*dataTable);
	}

	return dataTable;
}

// ----------------------------------------------------------------------

DataTable*  DataTableManager::reloadIfOpen(const std::string & table)
{
	if(isOpen(table))
		return reload(table);
	else
		return nullptr;
}

// ----------------------------------------------------------------------

void DataTableManager::addReloadCallback(const std::string& table, DataTableReloadCallback callbackFunction)
{
	IGNORE_RETURN(m_reloadCallbacks.insert(std::make_pair(table, callbackFunction)));
}

// ----------------------------------------------------------------------

bool DataTableManager::isOpen(const std::string& table)
{
	std::map<std::string, DataTable *>::const_iterator const i = m_tables.find(table);
	return i != m_tables.end();
}

// ======================================================================

