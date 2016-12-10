//======================================================================
//
// DraftSchematicGroupManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/DraftSchematicGroupManager.h"

#include "UnicodeUtils.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>
#include <map>
#include <vector>

//======================================================================

namespace DraftSchematicGroupManagerNamespace
{
	typedef DraftSchematicGroupManager::StringVector      StringVector;
	typedef DraftSchematicGroupManager::SchematicVector   SchematicVector;
	typedef std::map<std::string, SchematicVector>     GroupMapping;

	GroupMapping       s_groupMapping;
	bool               s_installed = false;

	void install ()
	{
		if (s_installed)
			return;

		s_installed = true;

		const std::string path = ConfigSharedGame::getSchematicGroupDataTable();

		const DataTable * const schematicGroupTable = DataTableManager::getTable (path, true);
		if (!schematicGroupTable)
		{
			WARNING_STRICT_FATAL(true, ("Cannot load schematic group datatable [%s]", path.c_str ()));
			return;
		}

		const int numRows = schematicGroupTable->getNumRows();
		for (int i = 0; i < numRows; ++i)
		{
			const std::string & group     = schematicGroupTable->getStringValue (0, i);
			const std::string & schematic = schematicGroupTable->getStringValue (1, i);
			const std::string::size_type slash = schematic.rfind('/');
			const std::string sharedSchematic(schematic.substr(0, slash + 1) + "shared_" + schematic.substr(slash + 1));
			const uint32 serverCrc = Crc::calculate(schematic.c_str());
			const uint32 sharedCrc = Crc::calculate(sharedSchematic.c_str());
			const std::pair<uint32, uint32> crc(serverCrc, sharedCrc);

			SchematicVector & sv = s_groupMapping [Unicode::toLower (group)];

			if (std::find (sv.begin (), sv.end (), crc) != sv.end ())
				WARNING_STRICT_FATAL (true, ("DraftSchematicGroupManager group [%s] schematic [%s] appears multiple times", group.c_str (), schematic.c_str ()));

			sv.push_back (crc);
		}
	}
}

using namespace DraftSchematicGroupManagerNamespace;

//----------------------------------------------------------------------

void DraftSchematicGroupManager::getGroups            (StringVector & sv)
{
	if (!s_installed)
		install ();

	for (GroupMapping::const_iterator it = s_groupMapping.begin (); it != s_groupMapping.end (); ++it)
	{
		sv.push_back ((*it).first);
	}
}

//----------------------------------------------------------------------

bool DraftSchematicGroupManager::getSchematicsForGroup (const std::string & group, SchematicVector & sv)
{
	if (!s_installed)
		install ();

	const GroupMapping::const_iterator it = s_groupMapping.find (Unicode::toLower (group));
	if (it != s_groupMapping.end ())
	{
		sv = (*it).second;
		return true;
	}

	return false;
}

//======================================================================
