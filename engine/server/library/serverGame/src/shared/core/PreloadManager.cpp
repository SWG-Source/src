// PreloadManager.cpp
// copyright 2003 Sony Online Entertainment


#include "serverGame/FirstServerGame.h"
#include "PreloadManager.h"

#include "serverGame/ConfigServerGame.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTableManager.h"

#include <cstdio>
#include <vector>

//------------------------------------------------------------------------------------------

bool PreloadManager::m_installed = false;

//------------------------------------------------------------------------------------------

namespace PreloadManagerNameSpace
{
	std::vector<const ObjectTemplate*> ms_objectTemplateList;
	std::vector<DataTable*> ms_dataTableList;

	void loadObjectTemplates()
	{
		FILE* fp = fopen("objectTemplates.plf","r");
		char buf [256];
		if (fp)
		{
			DEBUG_REPORT_LOG(true, ("Scanning templates"));
			char* line = fgets(buf, 256, fp);
			while (line)
			{
                //strip newline
				int len = strlen(line);
				for (int i = len - 1; i > 0; --i)
				{
					if (buf[i] == '\n')
					{
						buf[i] = 0;
						break;
					}
				}

				char* tmp = strstr(buf, "object/");
				if (tmp)
					line = tmp;

				
				const ObjectTemplate * objectTemplate = ObjectTemplateList::fetch(line);
				if (objectTemplate)
				{
					ms_objectTemplateList.push_back(objectTemplate);
					objectTemplate->preloadAssets();
				}
				DEBUG_REPORT_LOG(true, ("."));
				line = fgets(buf, 256, fp);
			}
			fclose(fp);
			DEBUG_REPORT_LOG(true, ("\n"));
		}
	}

	void loadDataTables()
	{
		FILE* fp = fopen("datatables.plf","r");
		char buf [256];
		if (fp)
		{
			DEBUG_REPORT_LOG(true, ("Scanning tables"));
			char* line = fgets(buf, 256, fp);
			while (line)
			{
				
				//strip newline
				int len = strlen(line);
				for (int i = len - 1; i > 0; --i)
				{
					if (buf[i] == '\n')
					{
						buf[i] = 0;
						break;
					}
				}
				
				char* tmp = strstr(buf, "datatables/");
				if (tmp)
					line = tmp;

				DEBUG_REPORT_LOG(true, ("."));
				line = fgets(buf, 256, fp);
			}
			fclose(fp);
			DEBUG_REPORT_LOG(true, ("\n"));
		}
	}

	void buildDataTableList()
	{
#ifdef LINUX
		system("find ../../data/sku.0/sys.server/compiled/game/datatables/ -name \"*.iff\" > datatables.plf");		
#else
		WARNING(true, ("Building preload lists only works under linux"));
#endif
	}

	void buildObjectTemplateList()
	{
#ifdef LINUX
		system("find ../../data/sku.0/sys.server/compiled/game/object/ -name \"*.iff\" > objectTemplates.plf");
#else
		WARNING(true, ("Building preload lists only works under linux"));
#endif
	}
	
}

using namespace PreloadManagerNameSpace;


//------------------------------------------------------------------------------------------

void PreloadManager::install()
{
	if (m_installed)
	{
		DEBUG_FATAL(true, ("Preload manager already installed"));
		return;
	}
	m_installed = true;

	if (ConfigServerGame::getBuildPreloadLists())
	{
		buildDataTableList();
		buildObjectTemplateList();
	}

	if (ConfigServerGame::getEnablePreload())
	{
		//preloading disabled
		loadDataTables();
		loadObjectTemplates();
	}

	ExitChain::add(remove, "PreloadManager::remove");
}

//------------------------------------------------------------------------------------------

void PreloadManager::remove()
{
	if (!m_installed)
	{
		DEBUG_FATAL(true, ("Preload manager not installed"));
		return;
	}

	for (std::vector<const ObjectTemplate*>::iterator i = ms_objectTemplateList.begin(); i != ms_objectTemplateList.end(); ++i)
	{
		(*i)->releaseReference();
	}

/*	for (std::vector<DataTable*>::iterator j = ms_objectTemplateList.begin(); j != ms_objectTemplateList.end(); ++j)
	{
		DataTableManager::close(*j);
	}
*/
	m_installed = false;
}

//------------------------------------------------------------------------------------------
