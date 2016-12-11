// ======================================================================
//
// DataTableManager.h
// 
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DataTableManager_H
#define INCLUDED_DataTableManager_H

// ======================================================================

class DataTable;

// ======================================================================

typedef void (*DataTableReloadCallback)(DataTable const &);

class DataTableManager
{
public:
	static void install(bool verboseErrorChecking = true);
	
	static void               close(const std::string& table);
	static DataTable *        getTable(const std::string& table, bool openIfNotFound = false);
	static DataTable *        reload(const std::string& table);
	static DataTable *        reloadIfOpen(const std::string& table);
	static void               addReloadCallback(const std::string& table, DataTableReloadCallback callbackFunction);
	static bool               isOpen(const std::string& table);

private:
	static void remove();

	static DataTable *        open(const std::string& table);

	DataTableManager();
	~DataTableManager();

	static DataTable*                         m_cachedTable;
	static std::string                        m_cachedTableName;
	static bool                               m_errorChecking;
	static std::map<std::string, DataTable *> m_tables;
	static bool                               m_installed;
	static std::multimap<std::string, DataTableReloadCallback> m_reloadCallbacks;
};

// ======================================================================

#endif
