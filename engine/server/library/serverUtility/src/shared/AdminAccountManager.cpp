// AdminAccountManager.cpp
// copyright 2002 Sony Online Entertainment

//-----------------------------------------------------------------------

#include "ConfigServerUtility.h"

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/AdminAccountManager.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "webAPI.h"

#include <string>

//-----------------------------------------------------------------------

DataTable * AdminAccountManager::ms_adminTable = 0;
bool        AdminAccountManager::ms_installed = false;
std::string *AdminAccountManager::ms_dataTableName = nullptr;

//-----------------------------------------------------------------------

void AdminAccountManager::install(const std::string &dataTableName)
{
	DEBUG_FATAL(ms_installed, ("AdminAccountManager already installed"));

	ms_adminTable = DataTableManager::getTable (dataTableName, true);
	DEBUG_FATAL(!ms_adminTable, ("Could not open admin file %s!", dataTableName.c_str()));
	ms_installed = true;

	ms_dataTableName = new std::string(dataTableName);

	ExitChain::add(AdminAccountManager::remove, "DataTableManager::remove");
}

//-----------------------------------------------------------------------

void AdminAccountManager::remove()
{
	DEBUG_FATAL(!ms_installed, ("AdminAccountManager not installed"));
	NOT_NULL(ms_dataTableName);
	DataTableManager::close(*ms_dataTableName);
	ms_installed = false;
	delete ms_dataTableName;
	ms_dataTableName = nullptr;
}

//-----------------------------------------------------------------------

const char *AdminAccountManager::getAdminCommandName()
{
	DEBUG_FATAL(!ms_installed, ("AdminAccountManager not installed"));
	return ms_adminTable->getStringValue("AdminSkill", 0);
}

//-----------------------------------------------------------------------

const std::string & AdminAccountManager::getAdminTagName()
{
	DEBUG_FATAL(!ms_installed, ("AdminAccountManager not installed"));
	static const std::string s("*admin*");
	return s;
}

//-----------------------------------------------------------------------

int AdminAccountManager::getAdminLevel(const std::string & account)
{
	int level = 0;
	DEBUG_FATAL(!ms_installed, ("AdminAccountManager not installed"));

	if(ConfigServerUtility::isExternalAdminLevelsEnabled()){
		std::ostringstream postBuffer;
		postBuffer << "user_name=" << account << "&secretKey=" << ConfigServerUtility::getExternalAdminLevelsSecretKey();
		std::string response = webAPI::simplePost(ConfigServerUtility::getExternalAdminLevelsURL(), std::string(postBuffer.str()), "");
		// aconite 4/3/22
		// stoi inconsistently throws an invalid argument exception from this request 
		// which causes the login and/or game server to crash on an auth or /setGod;
		// this is a temporary patch to safeguard against that until this can be further evaluated
		try
		{
			int newLevel = std::stoi(response);
			if(newLevel != 0)
			{
				level = newLevel;
			}
		}
		catch(std::invalid_argument const& ex) {}
		catch(std::out_of_range const& ex) {}
		return level;
	}

	int columnNumber = ms_adminTable->findColumnNumber("AdminAccounts");
	DEBUG_FATAL(columnNumber == -1, ("Error loading admin table...no account column"));
	int row = ms_adminTable->searchColumnString( columnNumber, account);
	if (row == -1) return 0;

	level = ms_adminTable->getIntValue("AdminLevel", row);
	return level;
}

bool AdminAccountManager::isAdminAccount(const std::string & account, int& level)
{
	level = 0;
	DEBUG_FATAL(!ms_installed, ("AdminAccountManager not installed"));

	int columnNumber = ms_adminTable->findColumnNumber("AdminAccounts");
	DEBUG_FATAL(columnNumber == -1, ("Error loading admin table...no account column"));
	int row = ms_adminTable->searchColumnString( columnNumber, account);
	if (row == -1)
		return false;

	level = ms_adminTable->getIntValue("AdminLevel", row);
	return true;
}

//-----------------------------------------------------------------------

bool AdminAccountManager::isInternalIp(const std::string & addr)
{
	DEBUG_FATAL(!ms_installed, ("AdminAccountManager not installed"));

	std::vector<const char *> ipAddrs;
	ms_adminTable->getStringColumn("AdminIpBlocks", ipAddrs);

	bool retval = false;

	for (std::vector<const char *>::iterator i = ipAddrs.begin(); i != ipAddrs.end(); ++i)
	{
		const std::string ipAddr(*i);

		//Check IP block
		//Look for X in the IP block signifying a wild card.
		size_t xpos = ipAddr.find ('x');

		//if no X is found, do a straight compare
		if ( (xpos == 0 || xpos == std::string::npos)
		   && ipAddr.compare(addr) == 0
			)
		{
			retval = true;
		}

		//we only want to compare up to the X, but if that is bigger that our addr
		//we cannot compare the two, so return false.
		else if (xpos - 1 > addr.size())
			retval = false;

		//compare substring
		else if (ipAddr.compare(0, xpos - 1, addr, 0, xpos - 1) == 0)
		{
			retval =  true;
		}

		if (retval == true)
		{
			return true;
		}
	}


	return retval;
}

//-----------------------------------------------------------------------

void AdminAccountManager::reload()
{
	DEBUG_FATAL(!ms_installed, ("AdminAccountManager not installed"));
	NOT_NULL(ms_dataTableName);
	DataTableManager::reload(*ms_dataTableName);
	ms_adminTable = DataTableManager::getTable (ms_dataTableName->c_str(), true);
}

//-----------------------------------------------------------------------
