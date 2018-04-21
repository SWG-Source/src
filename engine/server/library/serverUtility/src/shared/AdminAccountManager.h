// AdminAccountManager.h
// copyright 2002 Sony Online Entertainment
// Author: Justin Randall

#ifndef	_AdminAccountManager_H
#define	_AdminAccountManager_H

//-----------------------------------------------------------------------

class DataTable;

class AdminAccountManager
{

public:	
	static void install(const std::string &dataTableName);
	static void remove();

	static const char *getAdminCommandName();
	static const std::string & getAdminTagName();
	
	static int  getAdminLevel(const std::string & account);
	static bool isAdminAccount(const std::string & account, int& level);
	static bool isInternalIp(const std::string & addr);
	static void reload();

	
private:
	AdminAccountManager();
	~AdminAccountManager();

	static DataTable *   ms_adminTable;
	static bool	         ms_installed;
	static std::string * ms_dataTableName;

};

//-----------------------------------------------------------------------


#endif
