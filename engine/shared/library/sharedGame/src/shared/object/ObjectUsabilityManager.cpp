// ======================================================================
// 
// ObjectUsabilityManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ObjectUsabilityManager.h"

// ======================================================================

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedUtility/DataTable.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

// ----------------------------------------------------------------------

namespace ObjectUsabilityManagerNamespace
{
	bool ms_installed = false;

	//
	const std::string s_sharedPrefix("shared_");
	
	//
	typedef std::set<uint64> WearableObjectPermissionSet;
	WearableObjectPermissionSet s_wearableCrcSet; // can you wear an item?
	
	std::set<uint32> s_isWearableCrcSet;

	//
	struct SpecieGenderColumnInformation
	{
		SpecieGenderColumnInformation() : 
		m_species(0),	
		m_gender(0)
		{
		}

		SpecieGenderColumnInformation(SharedCreatureObjectTemplate::Species const species, SharedCreatureObjectTemplate::Gender const gender) :
		m_species(static_cast<uint16>(species)),
		m_gender(static_cast<uint16>(gender))
		{
		}

		uint16 m_species;
		uint16 m_gender;

		uint32 getKeyValue() const
		{
			uint32 const keyUpper = static_cast<uint32>(m_species) << 16;
			uint32 const keyLower = static_cast<uint32>(m_gender);
			return keyUpper | keyLower;
		}
	};

	//
	typedef std::map<uint32 /*appearance crc*/, SpecieGenderColumnInformation> SpeciesGenderMap;
	SpeciesGenderMap s_speciesGenderMap;
}

using namespace ObjectUsabilityManagerNamespace;	

// ----------------------------------------------------------------------

void ObjectUsabilityManager::install()
{
	InstallTimer const installTimer("ObjectUsabilityManager::install");

	DEBUG_FATAL(ms_installed, ("ObjectUsabilityManagerNamespace::install: already installed"));
	ms_installed = true;

	ExitChain::add(ObjectUsabilityManager::remove, "ObjectUsabilityManagerNamespace::remove");

	//-- Appearance info
	{
		char const * const appearanceInfoFileName = "datatables/appearance/appearance_info_table.iff";
		Iff iffInfo;
		if (iffInfo.open(appearanceInfoFileName, true))
		{
			DataTable appearanceInfoDataTable;
			appearanceInfoDataTable.load(iffInfo);
			
			int const numberOfRows = appearanceInfoDataTable.getNumRows();
			
			for (int row = 0; row < numberOfRows; ++row)
			{
				// Read the template name crc.
				std::string const & speciesName = appearanceInfoDataTable.getStringValue(0, row);
				TemporaryCrcString const tmpCrcSpeciesName(speciesName.c_str(), true);
				uint32 const crcSpeciesName = tmpCrcSpeciesName.getCrc();

				// Get the species value.
				SharedCreatureObjectTemplate::Species const species = static_cast<SharedCreatureObjectTemplate::Species>(appearanceInfoDataTable.getIntValue(1, row));

				// Get the gender value.
				SharedCreatureObjectTemplate::Gender const gender = static_cast<SharedCreatureObjectTemplate::Gender>(appearanceInfoDataTable.getIntValue(2, row));

				s_speciesGenderMap[crcSpeciesName] = SpecieGenderColumnInformation(species, gender);
			}
		}
		else
		{
			WARNING(true, ("ObjectUsabilityManager table missing [%s]", appearanceInfoFileName));
			return;
		}
	}
		
	//-- Appearance table
	{
		char const * const appearanceTableFileName = "datatables/appearance/appearance_table.iff";
		Iff iffAppearance;
		if (iffAppearance.open(appearanceTableFileName, true))
		{
			DataTable appearanceDataTable;
			appearanceDataTable.load(iffAppearance);
			
			int const numberOfColumns = appearanceDataTable.getNumColumns();
			int const numberOfRows = appearanceDataTable.getNumRows();
			
			for (int row = 0; row < numberOfRows; ++row)
			{
				// Read the template name crc.
				std::string const & sourceName = appearanceDataTable.getStringValue(0, row);
				TemporaryCrcString const crcSourceName(sourceName.c_str(), true);
				uint32 const crcValue = crcSourceName.getCrc();
				uint64 const keyValue = static_cast<uint64>(crcValue) << static_cast<uint64>(32);

				IGNORE_RETURN(s_isWearableCrcSet.insert(crcValue));
				
				// Read the species info.
				for (int column = 1; column < numberOfColumns; ++column)
				{
					std::string const & speciesName = appearanceDataTable.getColumnName(column);
					TemporaryCrcString const tmpCrcSpeciesName(speciesName.c_str(), true);
					uint32 const crcSpeciesName = tmpCrcSpeciesName.getCrc();

					std::string const & fileName = appearanceDataTable.getStringValue(column, row);
					if (fileName != ":block") // :block means "you can't use this item" in the appearance.tab
					{
						{
							SpeciesGenderMap::iterator const it = s_speciesGenderMap.find(crcSpeciesName);
							DEBUG_FATAL(it == s_speciesGenderMap.end(), ("ObjectUsabilityManager::install() species (%s) in appearance_table does not have a appearance_info.iff entry.", speciesName.c_str()));
							UNREF(it);
						}

						SpecieGenderColumnInformation const & speciesInfo = s_speciesGenderMap[crcSpeciesName];
						IGNORE_RETURN(s_wearableCrcSet.insert(keyValue | speciesInfo.getKeyValue()));
					}
				}
			}
		}
		else
			WARNING(true, ("ObjectUsabilityManager table missing [%s]", appearanceTableFileName));

	}
}

// ----------------------------------------------------------------------

void ObjectUsabilityManager::remove()
{
	DEBUG_FATAL(!ms_installed, ("ObjectUsabilityManagerNamespace::remove: not installed"));
	ms_installed = false;
	
	s_wearableCrcSet.clear();
	s_isWearableCrcSet.clear();
	s_speciesGenderMap.clear();
}

// ----------------------------------------------------------------------

bool ObjectUsabilityManager::canWear(uint32 const wearableSharedTemplateCrc, SharedCreatureObjectTemplate::Species const species, SharedCreatureObjectTemplate::Gender const gender)
{
	SpecieGenderColumnInformation const info(species, gender);
	uint64 const wearableKeyValue = static_cast<uint64>(wearableSharedTemplateCrc) << static_cast<uint64>(32);
	uint64 const keyValue = wearableKeyValue | info.getKeyValue();
	return s_wearableCrcSet.find(keyValue) != s_wearableCrcSet.end();
}

// ----------------------------------------------------------------------

bool ObjectUsabilityManager::canWear(uint32 const wearableSharedTemplateCrc, int const species, int const gender)
{
	return canWear(wearableSharedTemplateCrc, static_cast<SharedCreatureObjectTemplate::Species>(species), static_cast<SharedCreatureObjectTemplate::Gender>(gender));
}

// ----------------------------------------------------------------------

bool ObjectUsabilityManager::isWearable(uint32 const wearableSharedTemplateCrc)
{
	return s_isWearableCrcSet.find(wearableSharedTemplateCrc) != s_isWearableCrcSet.end();
}

// ======================================================================
