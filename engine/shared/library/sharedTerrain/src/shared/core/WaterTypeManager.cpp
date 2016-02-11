// ======================================================================
//
// WaterTypeManager.cpp
//
// copyright 2005, Sony Online Entertainment
//
// ======================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/WaterTypeManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>

// ======================================================================

bool WaterTypeManager::ms_installed = false;
WaterTypeManager::WaterTypeDataType * WaterTypeManager::ms_waterTypeData=nullptr;
WaterTypeManager::CreatureWaterTypeDataType* WaterTypeManager::ms_creatureWaterTypeData=nullptr;

// ======================================================================

WaterTypeManager::WaterTypeDataRecord::WaterTypeDataRecord() :
m_typeId(0),
m_name(),
m_causesDamage(false),
m_damageKills(false),
m_damageInterval(0),
m_damagePerInterval(0),
m_transparent(false)
{
}

// ======================================================================

WaterTypeManager::CreatureWaterTypeDataRecord::CreatureWaterTypeDataRecord() :
m_objectTemplateName(),
m_lavaResistance(0.0f)
{
}

// ======================================================================

void WaterTypeManager::install()
{
	InstallTimer const installTimer("WaterTypeManager::install");

	DEBUG_FATAL(ms_installed,("WaterTypeManager::install was called more than once."));
	ms_waterTypeData = new WaterTypeDataType;
	ms_creatureWaterTypeData = new CreatureWaterTypeDataType;
	ms_installed = true;

	ExitChain::add(&remove,"WaterTypeManager::remove");

	{
		DataTable const * const waterTypeDataTable = DataTableManager::getTable("datatables/terrain/water_values.iff", true);
		FATAL(!waterTypeDataTable,("water_values data table could not be opened."));
		int const numRows = waterTypeDataTable->getNumRows();
		for (int row=0; row<numRows; ++row)
		{
			WaterTypeDataRecord data;
			data.m_typeId = row;
			data.m_name = waterTypeDataTable->getStringValue("water_type",row);
			data.m_causesDamage = (waterTypeDataTable->getIntValue("causes_damage",row) != 0);
			data.m_damageKills = (waterTypeDataTable->getIntValue("damage_kills",row) != 0);
			data.m_damageInterval = waterTypeDataTable->getIntValue("damage_interval_secs", row);
			data.m_damagePerInterval = waterTypeDataTable->getFloatValue("damage_per_interval_percentage",row);
			data.m_transparent = (waterTypeDataTable->getIntValue("transparent",row) != 0);
			(*ms_waterTypeData)[data.m_typeId] = data;
		}
		DataTableManager::close("datatables/terrain/water_values.iff");
	}

	{
		DataTable const* const creatureWaterTypeDataTable = DataTableManager::getTable("datatables/terrain/creature_water_values.iff",true);
		FATAL(!creatureWaterTypeDataTable,("creature_water_values data table could not be opened."));
		int const numRows = creatureWaterTypeDataTable->getNumRows();
		for(int row=0; row < numRows; ++row)
		{
			CreatureWaterTypeDataRecord data;
			
			data.m_objectTemplateName = creatureWaterTypeDataTable->getStringValue("object_template_name",row);
			data.m_lavaResistance = creatureWaterTypeDataTable->getFloatValue("lava_resistance",row);
			if(data.m_lavaResistance > 100.0f)
			{
				data.m_lavaResistance = 100.0f;
			}
			else if(data.m_lavaResistance < 0.0f)
			{
				data.m_lavaResistance = 0.0f;
			}
			(*ms_creatureWaterTypeData)[data.m_objectTemplateName] = data;
		}
		DataTableManager::close("datatables/terrain/creature_water_values.iff");
	}
}

// ----------------------------------------------------------------------

void WaterTypeManager::remove()
{
	DEBUG_FATAL(!ms_installed,("WaterTypeManager not installed"));
	{
		/*
		// JU_TODO: temp - reference if we need to preload the character or vehicle effects here
		for (WaterTypeManager::WaterTypeDataType::iterator it = ms_waterTypeData->begin(); it != ms_waterTypeData->end(); ++it)
		{
			WaterTypeDataRecord & waterTypeDataRecord = (*it).second;
			waterTypeDataRecord.releaseResources();
		}
		*/
	}

	delete ms_waterTypeData;
	ms_waterTypeData=nullptr;
	delete ms_creatureWaterTypeData;
	ms_creatureWaterTypeData=nullptr;
	ms_installed = false;
}

// ----------------------------------------------------------------------

bool WaterTypeManager::getCausesDamage(TerrainGeneratorWaterType type)
{
	bool ret = false;

	WaterTypeDataType::const_iterator const i = ms_waterTypeData->find(type);
	if (i == ms_waterTypeData->end())
	{
		WARNING(true,("WaterTypeManager::getCausesDamage:  Received request with type id %i, which isn't in the data table.", type));
	}
	else
	{
		WaterTypeDataRecord const & data = i->second;
		ret = data.m_causesDamage;
	}

	return ret;
}

// ----------------------------------------------------------------------

bool WaterTypeManager::getDamageKills(TerrainGeneratorWaterType type)
{
	bool ret = false;

	WaterTypeDataType::const_iterator const i = ms_waterTypeData->find(type);
	if (i == ms_waterTypeData->end())
	{
		WARNING(true,("WaterTypeManager::getDamageKills:  Received request with type id %i, which isn't in the data table.", type));
	}
	else
	{
		WaterTypeDataRecord const & data = i->second;
		ret = data.m_damageKills;
	}

	return ret;
}

// ----------------------------------------------------------------------

int WaterTypeManager::getDamageInterval(TerrainGeneratorWaterType type)
{
	int ret = 0;

	WaterTypeDataType::const_iterator const i = ms_waterTypeData->find(type);
	if (i == ms_waterTypeData->end())
	{
		WARNING(true,("WaterTypeManager::getDamageInterval:  Received request with type id %i, which isn't in the data table.", type));
	}
	else
	{
		WaterTypeDataRecord const & data = i->second;
		ret = data.m_damageInterval;
	}

	return ret;
}

// ----------------------------------------------------------------------

float WaterTypeManager::getDamagePerInterval(TerrainGeneratorWaterType type)
{
	float ret = 0.0f;

	WaterTypeDataType::const_iterator const i = ms_waterTypeData->find(type);
	if (i == ms_waterTypeData->end())
	{
		WARNING(true,("WaterTypeManager::getDamagePerInterval:  Received request with type id %i, which isn't in the data table.", type));
	}
	else
	{
		WaterTypeDataRecord const & data = i->second;
		ret = data.m_damagePerInterval;
	}

	return ret;
}

// ----------------------------------------------------------------------

bool WaterTypeManager::getTransparent(TerrainGeneratorWaterType type)
{
	bool ret = false;

	WaterTypeDataType::const_iterator const i = ms_waterTypeData->find(type);
	if (i == ms_waterTypeData->end())
	{
		WARNING(true,("WaterTypeManager::getTransparent:  Received request with type id %i, which isn't in the data table.", type));
	}
	else
	{
		WaterTypeDataRecord const & data = i->second;
		ret = data.m_transparent;
	}

	return ret;

}

// ----------------------------------------------------------------------

float WaterTypeManager::getLavaResistance(const std::string& objectTemplateName)
{
	float ret = 0.0f;
	CreatureWaterTypeDataType::const_iterator const i = ms_creatureWaterTypeData->find(objectTemplateName);
	if(i != ms_creatureWaterTypeData->end())
	{
		CreatureWaterTypeDataRecord const &data = i->second;
		ret = data.m_lavaResistance;
	}

	return ret;	
}

// ======================================================================







