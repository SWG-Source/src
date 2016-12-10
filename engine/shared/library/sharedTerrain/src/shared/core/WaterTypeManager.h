//===================================================================
//
// WaterTypeManager.h
//
// copyright 2005, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_WaterTypeManager_H
#define INCLUDED_WaterTypeManager_H

#include <string>
#include "sharedTerrain/TerrainGeneratorType.def"


//===================================================================

class WaterTypeManager
{
public:
	static void install                       ();
	static void remove                        ();

public:
	static bool getCausesDamage(TerrainGeneratorWaterType type);
	static bool getDamageKills(TerrainGeneratorWaterType type);
	static int getDamageInterval(TerrainGeneratorWaterType type);
	static float getDamagePerInterval(TerrainGeneratorWaterType type);
	static bool getTransparent(TerrainGeneratorWaterType type);
	static float getLavaResistance(const std::string& objectTemplateName);

private:
	struct WaterTypeDataRecord
	{

	public:
		WaterTypeDataRecord();

	public:
		int m_typeId;
		std::string m_name;
		bool m_causesDamage;
		bool m_damageKills;
		int m_damageInterval;
		float m_damagePerInterval;
		bool m_transparent;
	};

	struct CreatureWaterTypeDataRecord
	{
	public:
		CreatureWaterTypeDataRecord();

	public:
		std::string m_objectTemplateName;
		float m_lavaResistance;

	};

	typedef std::map<int, WaterTypeDataRecord> WaterTypeDataType;
	typedef std::map<std::string, CreatureWaterTypeDataRecord> CreatureWaterTypeDataType;
	
	static bool				ms_installed;
	static					WaterTypeDataType * ms_waterTypeData;
	static                  CreatureWaterTypeDataType* ms_creatureWaterTypeData;
};

//===================================================================

#endif

