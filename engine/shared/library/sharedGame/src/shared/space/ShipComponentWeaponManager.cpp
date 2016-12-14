//======================================================================
//
// ShipComponentWeaponManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipComponentWeaponManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentType.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedFoundation/ExitChain.h"

#include <map>

//======================================================================

namespace ShipComponentWeaponManagerNamespace
{
	enum DatatableColumns
	{
		DC_componentName,
		DC_projectileIndex,
		DC_range,
		DC_speed,
		DC_ammoConsuming,
		DC_missile,
		DC_countermeasure,
		DC_mining,
		DC_tractor,
		DC_beam
	};

	class Data
	{
	public:

		//lint -e1925
		int projectileIndex;
		int8 flags;
		float range;
		float speed;

		Data() :
		projectileIndex(0),
		flags(0),
		range(0.0f),
		speed(0.0f)
		{
		}
	};

	typedef std::map<uint32, Data> CrcIntMap;
	CrcIntMap s_dataMap;

	bool s_installed = false;
}

using namespace ShipComponentWeaponManagerNamespace;

//----------------------------------------------------------------------

void ShipComponentWeaponManager::install()
{
	InstallTimer const installTimer("ShipComponentWeaponManager::install");

	DEBUG_FATAL(s_installed, ("ShipComponentWeaponManager already installed"));
	s_installed = true;

	std::string const & filename = "datatables/space/ship_weapon_components.iff";
	
	DataTable * const dt = DataTableManager::getTable(filename, true);

	if (dt == nullptr)
	{
		WARNING(true, ("ShipComponentWeaponManager no such datatable [%s]", filename.c_str()));
		return;
	}
	
	const int numRows = dt->getNumRows();
	
	for (int row = 0; row < numRows; ++row)
	{
		std::string const & componentName = dt->getStringValue(static_cast<int>(DC_componentName), row);
		ShipComponentDescriptor const * const shipComponentDescriptor = 
			ShipComponentDescriptor::findShipComponentDescriptorByName(TemporaryCrcString(componentName.c_str(), true));
		
		if (shipComponentDescriptor == nullptr)
		{
			WARNING(true, ("getComponentType datatable specified invalid component [%s]", componentName.c_str()));
			continue;
		}
		
		if (shipComponentDescriptor->getComponentType() != ShipComponentType::SCT_weapon)
		{
			WARNING(true, ("ShipComponentWeaponManager specified non-weapon component [%s]", componentName.c_str()));
			continue;
		}
		
		Data data;
		data.projectileIndex = dt->getIntValue(static_cast<int>(DC_projectileIndex), row);
		data.flags |= (dt->getIntValue(static_cast<int>(DC_ammoConsuming), row) != 0) ? F_ammoConsuming : 0;
		data.flags |= (dt->getIntValue(static_cast<int>(DC_missile), row) != 0) ? F_missile : 0;
		data.flags |= (dt->getIntValue(static_cast<int>(DC_countermeasure), row) != 0) ? F_countermeasure : 0;
		data.flags |= (dt->getIntValue(static_cast<int>(DC_mining), row) != 0) ? F_mining : 0;
		data.flags |= (dt->getIntValue(static_cast<int>(DC_tractor), row) != 0) ? F_tractor : 0;
		data.flags |= (dt->getIntValue(static_cast<int>(DC_beam), row) != 0) ? F_beam : 0;

		data.range = dt->getFloatValue(static_cast<int>(DC_range), row);
		data.speed = dt->getFloatValue(static_cast<int>(DC_speed), row);


		DEBUG_FATAL((data.flags & (F_countermeasure | F_missile)) == (F_countermeasure | F_missile),
			("Data bug:  weapon %i in ship_weapon_components is both a missile and a countermeasure.  This is not supported.",row));

		IGNORE_RETURN(s_dataMap.insert(std::make_pair(shipComponentDescriptor->getCrc(), data)));
	}

	ExitChain::add(ShipComponentWeaponManager::remove, "ShipComponentWeaponManager::remove");
}

//----------------------------------------------------------------------

void ShipComponentWeaponManager::remove()
{
	s_installed = false;
	s_dataMap.clear ();
}

//----------------------------------------------------------------------

int ShipComponentWeaponManager::getProjectileIndex(uint32 componentCrc)
{
	CrcIntMap::const_iterator const it = s_dataMap.find(componentCrc);
	if (it != s_dataMap.end())
		return (*it).second.projectileIndex;

	return -1;
}

//----------------------------------------------------------------------

bool ShipComponentWeaponManager::isAmmoConsuming(uint32 componentCrc)
{
	return hasFlags(componentCrc, F_ammoConsuming);
}

//----------------------------------------------------------------------

bool ShipComponentWeaponManager::isMissile(uint32 componentCrc)
{
	return hasFlags(componentCrc, F_missile);
}

//----------------------------------------------------------------------

bool ShipComponentWeaponManager::isCountermeasure(uint32 componentCrc)
{
	return hasFlags(componentCrc, F_countermeasure);
}

//----------------------------------------------------------------------

bool ShipComponentWeaponManager::isMining(uint32 componentCrc)
{
	return hasFlags(componentCrc, F_mining);
}

//----------------------------------------------------------------------

bool ShipComponentWeaponManager::isTractor(uint32 componentCrc)
{
	return hasFlags(componentCrc, F_tractor);
}

//----------------------------------------------------------------------

float ShipComponentWeaponManager::getRange(uint32 componentCrc)
{
	CrcIntMap::const_iterator const it = s_dataMap.find(componentCrc);
	if (it != s_dataMap.end())
		return (*it).second.range;

	return false;
}

//----------------------------------------------------------------------

float ShipComponentWeaponManager::getProjectileSpeed(uint32 componentCrc)
{
	CrcIntMap::const_iterator const it = s_dataMap.find(componentCrc);
	if (it != s_dataMap.end())
		return (*it).second.speed;

	return false;
}

//----------------------------------------------------------------------

bool ShipComponentWeaponManager::hasFlags(uint32 componentCrc, int flags)
{
	CrcIntMap::const_iterator const it = s_dataMap.find(componentCrc);
	if (it != s_dataMap.end())
		return ((*it).second.flags & flags) == flags;

	return false;
}

//======================================================================

