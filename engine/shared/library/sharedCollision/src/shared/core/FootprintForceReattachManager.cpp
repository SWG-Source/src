// ======================================================================
//
// CollisionWorld.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/FootprintForceReattachManager.h"

#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>
#include <vector>

namespace FootprintForceReattachManagerNamespace
{
	typedef std::vector<std::string> StringVector;

	StringVector ms_sceneIds;

	const char * const ms_datatableName = "datatables/planet/footprint_force_reattach.iff";

	bool s_installed = false;
	void install()
	{
		s_installed = true;

		ms_sceneIds.clear();

		DataTable const * const dt = DataTableManager::getTable(ms_datatableName, true);

		if (nullptr == dt)
			WARNING(true, ("FootprintForceReattachManager unable to find [%s]", ms_datatableName));
		else
		{
			int const numRows = dt->getNumRows();

			ms_sceneIds.reserve(numRows);

			{
				for (int row = 0; row < numRows; ++row)
				{
					std::string const & planetName = dt->getStringValue(0, row);
					ms_sceneIds.push_back(planetName);
				}
			}

			//-- sort to allow for binary search
			std::sort(ms_sceneIds.begin(), ms_sceneIds.end());
		}
	}
}

using namespace FootprintForceReattachManagerNamespace;

//----------------------------------------------------------------------

bool FootprintForceReattachManager::isSceneFootprintForceReattach(std::string const & sceneId)
{
	if (!s_installed)
		install();

	bool const found = std::binary_search(ms_sceneIds.begin(), ms_sceneIds.end(), sceneId);
	return found;
}

//----------------------------------------------------------------------
