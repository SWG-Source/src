// ======================================================================
//
// SpeciesRestrictions.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "swgSharedUtility/FirstSwgSharedUtility.h"
#include "swgSharedUtility/SpeciesRestrictions.h"

#include "sharedFoundation/Crc.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <map>
#include <string>

// ======================================================================

namespace SpeciesRestrictionsNamespace
{
	bool s_loaded=false;
	std::string s_dataTableName("datatables/creation/species_account_features_restrictions.iff");

	typedef std::map<uint32, uint32> TemplateCrcToRequiredFeaturesMapType;
	TemplateCrcToRequiredFeaturesMapType s_templateCrcToRequiredFeaturesMap;

	void load();
}

using namespace SpeciesRestrictionsNamespace;

// ======================================================================

/**
 * Check whether someone with the specified game features can
 * create the specified character species
 */
bool SpeciesRestrictions::canCreateCharacter(uint32 const gameFeatures, uint32 const objectTemplateCrc)
{
	if (!s_loaded)
		load();

	TemplateCrcToRequiredFeaturesMapType::const_iterator i=s_templateCrcToRequiredFeaturesMap.find(objectTemplateCrc);
	if (i==s_templateCrcToRequiredFeaturesMap.end())
		return true; // unlisted templates are not restricted
	else
	{
		return ((i->second & gameFeatures) == i->second);
	}
}

// ----------------------------------------------------------------------

/**
 * Load the data table for species restrictions, called automatically
 * the first time it is needed.
 */
void SpeciesRestrictionsNamespace::load()
{
	DataTable * restrictionDataTable = DataTableManager::getTable(s_dataTableName, true);
	if (restrictionDataTable)
	{
		int const numRows = restrictionDataTable->getNumRows();
		for (int row=0; row<numRows; ++row)
		{
			std::string const & templateName = restrictionDataTable->getStringValue("objectTemplate",row);
			uint32 const bits = restrictionDataTable->getIntValue("requiredGameFeatures",row);

			s_templateCrcToRequiredFeaturesMap.insert(std::make_pair(Crc::calculate(templateName.c_str()), bits));
		}
	}
}

// ======================================================================
