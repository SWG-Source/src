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

	typedef std::map<int, StringId>        TypeStringIdMap;
	typedef std::map<int, std::string>     TypeStringMap;
	typedef std::map<std::string, int>     StringTypeMap;
	typedef std::map<int, Unicode::String> TypeUnicodeStringMap;

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
	static std::map<int, std::string> const & getTypesExcludedFromCommodities();

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
