//======================================================================
//
// ShipComponentType.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipComponentType.h"

#include <unordered_map>

//======================================================================

namespace ShipComponentTypeNamespace
{
	const std::string s_componentTypeNames [ShipComponentType::SCT_num_types] =
	{
		"reactor",
		"engine",
		"shield",
		"armor",
		"weapon",
		"capacitor",
		"booster",
		"droid_interface",
		"bridge",
		"hangar",
		"targeting_station",
		"cargo_hold",
		"modification"
	};

	const std::string s_componentTypePrefixes[ShipComponentType::SCT_num_types] =
	{
		"rct_",
		"eng_",
		"shd_",
		"arm_",
		"wpn_",
		"cap_",
		"bst_",
		"ddi_",
		"bdg_",
		"hgr_",
		"tst_",
		"crg_",
		"mod_"
	};

	typedef std::unordered_map<std::string, ShipComponentType::Type> ComponentNameTypeMap;
	ComponentNameTypeMap s_componentNameTypeMap;

	bool s_installed = false;

	void install ()
	{
		if (s_installed)
			return;

		s_installed = true;

		for (int i = 0; i < static_cast<int>(ShipComponentType::SCT_num_types); ++i)
			s_componentNameTypeMap.insert (std::make_pair (s_componentTypeNames [i], static_cast<ShipComponentType::Type>(i)));
	}
}

using namespace ShipComponentTypeNamespace;

//----------------------------------------------------------------------

std::string const & ShipComponentType::getNameFromType (Type type)
{
	const int i_type = static_cast<int>(type);

	if (i_type < SCT_reactor || i_type >= SCT_num_types)
	{
		static const std::string empty;
		return empty;
	}

	return s_componentTypeNames [i_type];
}

//----------------------------------------------------------------------

ShipComponentType::Type ShipComponentType::getTypeFromName (std::string const & name)
{
	if (!s_installed)
		install ();

	const ComponentNameTypeMap::const_iterator it = s_componentNameTypeMap.find (name);
	if (it != s_componentNameTypeMap.end ())
		return (*it).second;
	
	return SCT_num_types;
}

//----------------------------------------------------------------------

std::string const & ShipComponentType::getCanonicalPrefixForType(Type type)
{
	const int i_type = static_cast<int>(type);

	if (i_type < SCT_reactor || i_type >= SCT_num_types)
	{
		static const std::string empty;
		return empty;
	}

	return s_componentTypePrefixes [i_type];
}

//----------------------------------------------------------------------

bool ShipComponentType::isCanonicalPrefix(std::string const & prefix)
{
	for (int i = 0; i < SCT_num_types; ++i)
	{
		if (s_componentTypePrefixes[i] == prefix)
			return true;
	}

	return false;
}

//======================================================================
