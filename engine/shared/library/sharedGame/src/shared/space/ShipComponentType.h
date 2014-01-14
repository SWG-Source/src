//======================================================================
//
// ShipComponentType.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentType_H
#define INCLUDED_ShipComponentType_H

//======================================================================

namespace ShipComponentType
{
	enum Type
	{
		SCT_reactor,
		SCT_engine,
		SCT_shield,
		SCT_armor,
		SCT_weapon,
		SCT_capacitor,
		SCT_booster,
		SCT_droid_interface,
		SCT_bridge,
		SCT_hangar,
		SCT_targeting_station,
		SCT_cargo_hold,
		SCT_modification,
		SCT_num_types
	};

	std::string const & getNameFromType (Type type);
	Type                getTypeFromName (std::string const & name);
	std::string const & getCanonicalPrefixForType(Type type);
	bool isCanonicalPrefix(std::string const &);

}

//======================================================================

#endif
