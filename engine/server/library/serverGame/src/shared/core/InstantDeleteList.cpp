// ======================================================================
//
// InstantDeleteList.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/InstantDeleteList.h"

#include "serverGame/ConfigServerGame.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <set>

// ======================================================================

InstantDeleteList::ListType *InstantDeleteList::ms_theList = nullptr;

// ======================================================================

void InstantDeleteList::install()
{
	ExitChain::add(InstantDeleteList::remove, "InstantDeleteList::remove");
	ms_theList = new ListType;

	DataTable * data = DataTableManager::getTable(ConfigServerGame::getInstantDeleteListDataTableName(),true);
	FATAL(!data,("Could not find data table %s, needed for InstantDeleteList",ConfigServerGame::getInstantDeleteListDataTableName()));
			
	int numRows = data->getNumRows();
	for (int row=0; row<numRows; ++row)
	{
		std::string templateName = data->getStringValue("template",row);

		uint32 templateId = ObjectTemplateList::lookUp(templateName.c_str()).getCrc();
		ms_theList->insert(templateId);
	}
}

// ----------------------------------------------------------------------

void InstantDeleteList::remove()
{
	delete ms_theList;
	ms_theList = nullptr;
}

// ----------------------------------------------------------------------

bool InstantDeleteList::isOnList(const ObjectTemplate &theTemplate)
{
	NOT_NULL(ms_theList);
	return (ms_theList->find(theTemplate.getCrcName().getCrc()) != ms_theList->end());
}

// ======================================================================
