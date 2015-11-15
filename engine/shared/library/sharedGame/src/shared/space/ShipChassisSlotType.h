//======================================================================
//
// ShipChassisSlotType.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipChassisSlotType_H
#define INCLUDED_ShipChassisSlotType_H

//======================================================================

namespace ShipChassisSlotType
{
	// The following must be kept in sync:
	// scst_short_n.stf
	// ShipChassisSlotType::Type (ShipChassisSlotType.h)
	// ShipChassisSlotTypeNamespace::s_slotTypeNames  (ShipChassisSlotType.cpp)
	// ShipChassisSlotTypeNamespace::s_slotTypeComponentTypeMapping (ShipChassisSlotType.cpp)
	// ship_chassis_slot_type (base_class.java)
	// ship_chassis_slot_type::names (base_class.java)
	enum Type
	{
		SCST_first,
		SCST_reactor = SCST_first,
		SCST_engine,
		SCST_shield_0,
		SCST_shield_1,
		SCST_armor_0,
		SCST_armor_1,
		SCST_capacitor,
		SCST_booster,
		SCST_droid_interface,
		SCST_bridge,
		SCST_hangar,
		SCST_targeting_station,
		SCST_cargo_hold,
		SCST_modification_0,
		SCST_modification_1,
		SCST_weapon_first,
		SCST_weapon_0 = SCST_weapon_first,
		SCST_weapon_1,
		SCST_weapon_2,
		SCST_weapon_3,
		SCST_weapon_4,
		SCST_weapon_5,
		SCST_weapon_6,
		SCST_weapon_7,
		SCST_num_explicit_types,
		SCST_weapon_8 = SCST_num_explicit_types,
		SCST_weapon_9,
		SCST_weapon_10,
		SCST_weapon_11,
		SCST_weapon_12,
		SCST_weapon_13,
		SCST_weapon_14,
		SCST_weapon_15,
		SCST_weapon_16,
		SCST_weapon_17,
		SCST_weapon_18,
		SCST_weapon_19,
		SCST_weapon_20,
		SCST_weapon_21,
		SCST_weapon_22,
		SCST_weapon_23,
		SCST_weapon_24,
		SCST_weapon_25,
		SCST_weapon_26,
		SCST_weapon_27,
		SCST_weapon_28,
		SCST_weapon_29,
		SCST_weapon_30,
		SCST_weapon_31,
		SCST_weapon_32,
		SCST_weapon_33,
		SCST_weapon_34,
		SCST_weapon_35,
		SCST_weapon_36,
		SCST_weapon_37,
		SCST_weapon_38,
		SCST_weapon_39,
		SCST_weapon_40,
		SCST_weapon_41,
		SCST_weapon_42,
		SCST_weapon_43,
		SCST_weapon_44,
		SCST_weapon_45,
		SCST_weapon_46,
		SCST_weapon_47,
		SCST_weapon_48,
		SCST_weapon_49,
		SCST_weapon_50,
		SCST_weapon_51,
		SCST_weapon_52,
		SCST_weapon_53,
		SCST_weapon_54,
		SCST_weapon_55,
		SCST_weapon_56,
		SCST_weapon_57,
		SCST_weapon_58,
		SCST_weapon_59,
		SCST_weapon_60,
		SCST_weapon_61,
		SCST_weapon_62,
		SCST_weapon_63,
		SCST_weapon_64,
		SCST_weapon_65,
		SCST_weapon_66,
		SCST_weapon_67,
		SCST_weapon_68,
		SCST_weapon_69,
		SCST_weapon_70,
		SCST_weapon_71,
		SCST_weapon_72,
		SCST_weapon_73,
		SCST_weapon_74,
		SCST_weapon_75,
		SCST_weapon_76,
		SCST_weapon_77,
		SCST_weapon_78,
		SCST_weapon_79,
		SCST_weapon_80,
		SCST_weapon_81,
		SCST_weapon_82,
		SCST_weapon_83,
		SCST_weapon_84,
		SCST_weapon_85,
		SCST_weapon_86,
		SCST_weapon_87,
		SCST_weapon_88,
		SCST_weapon_89,
		SCST_weapon_90,
		SCST_weapon_91,
		SCST_weapon_92,
		SCST_weapon_93,
		SCST_weapon_94,
		SCST_weapon_95,
		SCST_weapon_96,
		SCST_weapon_97,
		SCST_weapon_98,
		SCST_weapon_99,
		SCST_weapon_last = SCST_weapon_99,
		SCST_num_types,
		SCST_invalid = SCST_num_types
	};

	int const cms_numWeaponIndices = (static_cast<int>(SCST_weapon_last) - static_cast<int>(SCST_weapon_0)) + 1;
	int const cms_numExplicitWeaponIndices = (static_cast<int>(SCST_num_explicit_types) - static_cast<int>(SCST_weapon_0)) + 1;

	std::string const & getNameFromType          (Type type);
	Unicode::String const & getShortNameFromType     (Type type);
	Type                getTypeFromName          (std::string const & name);
	int                 getComponentTypeForSlot  (Type slotType);
	bool isWeaponChassisSlot(Type const type);
	bool isWeaponIndex(int const index);
	int getWeaponIndex(Type const type);
}

//======================================================================

#endif
