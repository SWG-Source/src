//======================================================================
//
// ShipChassisSlotType.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipChassisSlotType.h"

#include "sharedGame/ShipComponentType.h"

#include "Unicode.h"

#include <unordered_map>

//======================================================================

namespace ShipChassisSlotTypeNamespace
{
	// The following must be kept in sync:
	// scst_short_n.stf
	// ShipChassisSlotType::Type (ShipChassisSlotType.h)
	// ShipChassisSlotTypeNamespace::s_slotTypeNames  (ShipChassisSlotType.cpp)
	// ShipChassisSlotTypeNamespace::s_slotTypeComponentTypeMapping (ShipChassisSlotType.cpp)
	// ship_chassis_slot_type (base_class.java)
	// ship_chassis_slot_type::names (base_class.java)
	const std::string s_slotTypeNames [ShipChassisSlotType::SCST_num_types + 1] =
	{
		"reactor",
		"engine",
		"shield_0",
		"shield_1",
		"armor_0",
		"armor_1",
		"capacitor",
		"booster",
		"droid_interface",
		"bridge",
		"hangar",
		"targeting_station",
		"cargo_hold",
		"modification_0",
		"modification_1",
		"weapon_0",
		"weapon_1",
		"weapon_2",
		"weapon_3",
		"weapon_4",
		"weapon_5",
		"weapon_6",
		"weapon_7",
		"weapon_8",
		"weapon_9",
		"weapon_10",
		"weapon_11",
		"weapon_12",
		"weapon_13",
		"weapon_14",
		"weapon_15",
		"weapon_16",
		"weapon_17",
		"weapon_18",
		"weapon_19",
		"weapon_20",
		"weapon_21",
		"weapon_22",
		"weapon_23",
		"weapon_24",
		"weapon_25",
		"weapon_26",
		"weapon_27",
		"weapon_28",
		"weapon_29",
		"weapon_30",
		"weapon_31",
		"weapon_32",
		"weapon_33",
		"weapon_34",
		"weapon_35",
		"weapon_36",
		"weapon_37",
		"weapon_38",
		"weapon_39",
		"weapon_40",
		"weapon_41",
		"weapon_42",
		"weapon_43",
		"weapon_44",
		"weapon_45",
		"weapon_46",
		"weapon_47",
		"weapon_48",
		"weapon_49",
		"weapon_50",
		"weapon_51",
		"weapon_52",
		"weapon_53",
		"weapon_54",
		"weapon_55",
		"weapon_56",
		"weapon_57",
		"weapon_58",
		"weapon_59",
		"weapon_60",
		"weapon_61",
		"weapon_62",
		"weapon_63",
		"weapon_64",
		"weapon_65",
		"weapon_66",
		"weapon_67",
		"weapon_68",
		"weapon_69",
		"weapon_70",
		"weapon_71",
		"weapon_72",
		"weapon_73",
		"weapon_74",
		"weapon_75",
		"weapon_76",
		"weapon_77",
		"weapon_78",
		"weapon_79",
		"weapon_80",
		"weapon_81",
		"weapon_82",
		"weapon_83",
		"weapon_84",
		"weapon_85",
		"weapon_86",
		"weapon_87",
		"weapon_88",
		"weapon_89",
		"weapon_90",
		"weapon_91",
		"weapon_92",
		"weapon_93",
		"weapon_94",
		"weapon_95",
		"weapon_96",
		"weapon_97",
		"weapon_98",
		"weapon_99",
		"none",
	};

	Unicode::String s_slotTypeShortNames [ShipChassisSlotType::SCST_num_types];

	// The following must be kept in sync:
	// scst_short_n.stf
	// ShipChassisSlotType::Type (ShipChassisSlotType.h)
	// ShipChassisSlotTypeNamespace::s_slotTypeNames  (ShipChassisSlotType.cpp)
	// ShipChassisSlotTypeNamespace::s_slotTypeComponentTypeMapping (ShipChassisSlotType.cpp)
	// ship_chassis_slot_type (base_class.java)
	// ship_chassis_slot_type::names (base_class.java)
	const int s_slotTypeComponentTypeMapping [static_cast<int>(ShipChassisSlotType::SCST_num_types)] =
	{
		static_cast<int>(ShipComponentType::SCT_reactor),          // SCST_reactor
		static_cast<int>(ShipComponentType::SCT_engine),           // SCST_engine
		static_cast<int>(ShipComponentType::SCT_shield),           // SCST_shield_0
		static_cast<int>(ShipComponentType::SCT_shield),           // SCST_shield_1
		static_cast<int>(ShipComponentType::SCT_armor),            // SCST_armor_0
		static_cast<int>(ShipComponentType::SCT_armor),            // SCST_armor_1
		static_cast<int>(ShipComponentType::SCT_capacitor),        // SCST_capacitor
		static_cast<int>(ShipComponentType::SCT_booster),          // SCST_booster
		static_cast<int>(ShipComponentType::SCT_droid_interface),  // SCST_droid_interface
		static_cast<int>(ShipComponentType::SCT_bridge),           // SCST_bridge
		static_cast<int>(ShipComponentType::SCT_hangar),           // SCST_hangar
		static_cast<int>(ShipComponentType::SCT_targeting_station),// SCST_targeting_station
		static_cast<int>(ShipComponentType::SCT_cargo_hold),       // SCST_cargo_hold
		static_cast<int>(ShipComponentType::SCT_modification),     // SCST_modification_0
		static_cast<int>(ShipComponentType::SCT_modification),     // SCST_modification_1
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_0
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_1
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_2
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_3
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_4
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_5
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_6
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_7
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_8
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_9
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_10
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_11
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_12
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_13
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_14
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_15
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_16
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_17
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_18
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_19
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_20
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_21
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_22
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_23
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_24
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_25
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_26
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_27
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_28
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_29
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_30
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_31
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_32
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_33
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_34
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_35
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_36
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_37
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_38
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_39
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_40
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_41
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_42
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_43
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_44
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_45
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_46
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_47
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_48
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_49
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_50
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_51
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_52
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_53
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_54
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_55
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_56
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_57
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_58
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_59
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_60
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_61
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_62
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_63
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_64
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_65
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_66
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_67
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_68
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_69
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_70
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_71
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_72
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_73
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_74
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_75
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_76
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_77
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_78
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_79
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_80
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_81
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_82
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_83
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_84
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_85
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_86
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_87
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_88
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_89
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_90
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_91
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_92
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_93
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_94
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_95
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_96
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_97
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_98
		static_cast<int>(ShipComponentType::SCT_weapon),           // SCST_weapon_99
	};

	typedef std::unordered_map<std::string, ShipChassisSlotType::Type> SlotNameTypeMap;
	SlotNameTypeMap s_slotNameTypeMap;

	bool s_installed = false;

	void install ()
	{
		if (s_installed)
			return;

		s_installed = true;

		for (int i = 0; i < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++i)
		{
			IGNORE_RETURN (s_slotNameTypeMap.insert (std::make_pair (s_slotTypeNames [i], static_cast<ShipChassisSlotType::Type>(i))));

			s_slotTypeShortNames [i] = StringId("space/scst_short_n", s_slotTypeNames [i]).localize();
		}
	}
}

using namespace ShipChassisSlotTypeNamespace;

//----------------------------------------------------------------------

std::string const & ShipChassisSlotType::getNameFromType (Type type)
{ //lint !e1929 // ref
	if (!s_installed)
		install ();

	const int i_type = static_cast<int>(type);

	if (i_type < SCST_first || i_type > SCST_num_types)
	{
		static const std::string empty;
		return empty;
	}

	return s_slotTypeNames [i_type];
}

//----------------------------------------------------------------------

ShipChassisSlotType::Type  ShipChassisSlotType::getTypeFromName (std::string const & name)
{
	if (!s_installed)
		install ();

	const SlotNameTypeMap::const_iterator it = s_slotNameTypeMap.find (name);
	if (it != s_slotNameTypeMap.end ())
		return (*it).second;

	return SCST_num_types;
}

//----------------------------------------------------------------------

int ShipChassisSlotType::getComponentTypeForSlot (ShipChassisSlotType::Type slotType)
{
	if (slotType == static_cast<int>(ShipChassisSlotType::SCST_num_types))
		return static_cast<int>(ShipChassisSlotType::SCST_num_types);

	return static_cast<int>(s_slotTypeComponentTypeMapping [static_cast<int>(slotType)]);
}

//----------------------------------------------------------------------

Unicode::String const & ShipChassisSlotType::getShortNameFromType (Type type)
{
	if (!s_installed)
		install ();

	if (type < SCST_first || type >= SCST_num_types)
	{
		return Unicode::emptyString;
	}
	
	return s_slotTypeShortNames[type];
}

//----------------------------------------------------------------------

bool ShipChassisSlotType::isWeaponChassisSlot(Type const type)
{
	return ((type >= ShipChassisSlotType::SCST_weapon_first) && (type <= ShipChassisSlotType::SCST_weapon_last));
}

//----------------------------------------------------------------------

int ShipChassisSlotType::getWeaponIndex(Type const type)
{
	DEBUG_FATAL(!isWeaponChassisSlot(type), ("Non-weapon chassis slot specified(%s)", getNameFromType(type).c_str()));

	return static_cast<int>(type - ShipChassisSlotType::SCST_weapon_first);
}

//----------------------------------------------------------------------

bool ShipChassisSlotType::isWeaponIndex(int const index)
{
	return (index >= 0) && (index < cms_numWeaponIndices);
}

//======================================================================
