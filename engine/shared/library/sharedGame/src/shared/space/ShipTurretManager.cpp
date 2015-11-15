// ======================================================================
//
// ShipTurretManager.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipTurretManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedGame/ShipChassis.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedUtility/DataTable.h"
#include <map>

// ======================================================================

namespace ShipTurretManagerNamespace
{

	struct ShipTurretData
	{
		float minYaw;
		float maxYaw;
		float minPitch;
		float maxPitch;
	};

	std::map<uint32, std::map<int, ShipTurretData> > s_shipTurretData;

}

using namespace ShipTurretManagerNamespace;

// ======================================================================

void ShipTurretManager::install() // static
{
	InstallTimer const installTimer("ShipTurretManager::install");

	Iff iff;
	if (iff.open("datatables/space/ship_turret.iff", true))
	{
		DataTable dataTable;
		dataTable.load(iff);
		int numberOfRows = dataTable.getNumRows();
		for (int row = 0; row < numberOfRows; ++row)
		{
			std::string const chassisName(dataTable.getStringValue("chassis", row));
			ShipChassis const * const chassis = ShipChassis::findShipChassisByName(TemporaryCrcString(chassisName.c_str(), false));
			FATAL(!chassis, ("ShipTurretManager::install: no such chassis '%s'", chassisName.c_str()));
			int const weaponIndex = dataTable.getIntValue("weaponIndex", row);
			ShipTurretData &shipTurretData = s_shipTurretData[chassis->getCrc()][weaponIndex];
			shipTurretData.minYaw = dataTable.getFloatValue("minYaw", row) * PI_OVER_180;
			shipTurretData.maxYaw = dataTable.getFloatValue("maxYaw", row) * PI_OVER_180;
			shipTurretData.minPitch = dataTable.getFloatValue("minPitch", row) * PI_OVER_180;
			shipTurretData.maxPitch = dataTable.getFloatValue("maxPitch", row) * PI_OVER_180;
		}
	}
}

// ----------------------------------------------------------------------

bool ShipTurretManager::isTurret(uint32 chassisCrc, int weaponIndex)
{
	std::map<uint32, std::map<int, ShipTurretData> >::const_iterator i = s_shipTurretData.find(chassisCrc);
	if (i == s_shipTurretData.end())
		return false;
	return (*i).second.count(weaponIndex) > 0;
}

// ----------------------------------------------------------------------

float ShipTurretManager::getTurretMinYaw(uint32 chassisCrc, int weaponIndex)
{
	std::map<uint32, std::map<int, ShipTurretData> >::const_iterator i = s_shipTurretData.find(chassisCrc);
	if (i != s_shipTurretData.end())
	{
		std::map<int, ShipTurretData>::const_iterator j = (*i).second.find(weaponIndex);
		if (j != (*i).second.end())
			return (*j).second.minYaw;
	}
	return 0.f;
}

// ----------------------------------------------------------------------

float ShipTurretManager::getTurretMaxYaw(uint32 chassisCrc, int weaponIndex)
{
	std::map<uint32, std::map<int, ShipTurretData> >::const_iterator i = s_shipTurretData.find(chassisCrc);
	if (i != s_shipTurretData.end())
	{
		std::map<int, ShipTurretData>::const_iterator j = (*i).second.find(weaponIndex);
		if (j != (*i).second.end())
			return (*j).second.maxYaw;
	}
	return 0.f;
}

// ----------------------------------------------------------------------

float ShipTurretManager::getTurretMinPitch(uint32 chassisCrc, int weaponIndex)
{
	std::map<uint32, std::map<int, ShipTurretData> >::const_iterator i = s_shipTurretData.find(chassisCrc);
	if (i != s_shipTurretData.end())
	{
		std::map<int, ShipTurretData>::const_iterator j = (*i).second.find(weaponIndex);
		if (j != (*i).second.end())
			return (*j).second.minPitch;
	}
	return 0.f;
}

// ----------------------------------------------------------------------

float ShipTurretManager::getTurretMaxPitch(uint32 chassisCrc, int weaponIndex)
{
	std::map<uint32, std::map<int, ShipTurretData> >::const_iterator i = s_shipTurretData.find(chassisCrc);
	if (i != s_shipTurretData.end())
	{
		std::map<int, ShipTurretData>::const_iterator j = (*i).second.find(weaponIndex);
		if (j != (*i).second.end())
			return (*j).second.maxPitch;
	}
	return 0.f;
}

// ======================================================================
