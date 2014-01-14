//======================================================================
//
// GameObjectTypes.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_GameObjectTypes_H
#define INCLUDED_GameObjectTypes_H

//======================================================================

class StringId;

//----------------------------------------------------------------------

class GameObjectTypes
{
public:

	typedef stdmap<int, StringId>::fwd        TypeStringIdMap;
	typedef stdmap<int, std::string>::fwd     TypeStringMap;
	typedef stdmap<std::string, int>::fwd     StringTypeMap;
	typedef stdmap<int, Unicode::String>::fwd TypeUnicodeStringMap;

	static const TypeStringMap &   getTypeStringMap ();

	static const std::string &     getCanonicalName (int type);
	static const StringId &        getStringId      (int type);
	static const Unicode::String & getLocalizedName (int type);
	static int                     getGameObjectType (std::string const & canonicalName);

	static bool                    getTypeByName    (const std::string & name, int & type);

	static bool                    isTypeOf         (int type, int typeToTestAgainst);
	static bool                    isSubType        (int type);

	static int                     getMaskedType    (int type);
	static int                     getSubType       (int type);

	static bool                    doesPowerupApply (int powerupType, int targetType);

	static bool                    isExcludedFromCommodities(int type);
	static stdmap<int, std::string>::fwd const & getTypesExcludedFromCommodities();

	static const int ms_type_mask;
};

//----------------------------------------------------------------------

inline int GameObjectTypes::getMaskedType    (int type)
{
	return (type & ms_type_mask);
}

//----------------------------------------------------------------------

inline int GameObjectTypes::getSubType       (int type)
{
	return (type & ~ms_type_mask);
}

//======================================================================

#endif
