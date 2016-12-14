//========================================================================
//
// ScriptParams.h - varargs substitute for passing params to scripts.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ScriptParams_H
#define _INCLUDED_ScriptParams_H

#include "serverGame/ServerObjectTemplate.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedObject/CachedNetworkId.h"

class ValueDictionary;
struct ObjectMenuRequestData;
class ManufactureObjectInterface;

namespace AttribMod
{
	struct AttribMod;
}

namespace Crafting
{
	struct IngredientSlot;
}

typedef std::vector<ObjectMenuRequestData> MenuDataVector;

struct SlotData
{
	const Crafting::IngredientSlot *   slot;
	const ManufactureObjectInterface * schematic;
	int                                amountRequired;
	const std::string *                appearance;
};

struct Param
{
	enum ParamType
	{
		NONE,
		BOOL,
		BOOL_ARRAY,
		INT,
		INT_ARRAY,
		FLOAT,
		FLOAT_ARRAY,
		STRING,
		STRING_ARRAY,
		UNICODE,
		UNICODE_ARRAY,
		OBJECT_ID,
		OBJECT_ID_ARRAY,
		CACHED_OBJECT_ID_ARRAY,
		STRING_ID,
		STRING_ID_ARRAY,
		ATTRIB_MOD,
		ATTRIB_MOD_ARRAY,
		MENTAL_STATE_MOD,
		MENTAL_STATE_MOD_ARRAY,
		OBJECT_MENU_INFO,
		MANUFACTURE_SCHEMATIC,
		INGREDIENT_SLOT,
		LOCATION,
		LOCATION_ARRAY,
		DICTIONARY,
		DICTIONARY_ARRAY,
		BYTE_ARRAY,
		OBJECT_ID_ARRAY_ARRAY
	};

	ParamType m_type;
	bool m_owned;
	std::string m_name;
	union
	{
		bool bParam;
		const std::deque<bool> * baParam;
		int iParam;
		const std::vector<int> * iaParam;
		float fParam;
		const std::vector<float> * faParam;
		const char * sParam;
		std::vector<const char *> *saParam;
		const Unicode::String * uParam;
		std::vector<const Unicode::String *> * uaParam;
		const NetworkId * oidParam;
		const std::vector<NetworkId> *oidaParam;
		const std::vector<CachedNetworkId> *coidaParam;
		const std::vector<const std::vector<NetworkId> *> *oidaaParam;
		const StringId * sidParam;
		const std::vector<const StringId *> *sidaParam;
		const AttribMod::AttribMod * amParam;
		const std::vector<AttribMod::AttribMod> * amaParam;
		const ServerObjectTemplate::MentalStateMod * msmParam;
		const std::vector<ServerObjectTemplate::MentalStateMod> * amsmParam;
		const MenuDataVector * aomrdParam;
		const ManufactureObjectInterface * msoParam;
		const Vector * lParam;
		const std::vector<const Vector *> * laParam;
		SlotData isParam;
		const ValueDictionary * valueDictionaryParam;
		const std::vector<ValueDictionary> * valueDictionaryArrayParam;
		const std::vector<unsigned char> * byteArrayParam;

	} m_param;
};
class ScriptParams;
namespace Archive
{
	class ByteStream;
	class ReadIterator;
	void get(ReadIterator & source, ScriptParams & target);
	void put(ByteStream & target, const ScriptParams & source);
	void get(ReadIterator & source, std::vector<const std::vector<NetworkId> *> & target);
	void put(ByteStream & target, const std::vector<const std::vector<NetworkId> *> & source);
}

class ScriptParams
{
public:

public:

	ScriptParams(void);
	~ScriptParams(void);

	void clear(void);

	void addParam(const Unicode::unicode_char_t *, const std::string & paramName = "", bool owned = false);
	void addParam(bool param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::deque<bool> & param, const std::string & paramName = "", bool owned = false);
	void addParam(int param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::vector<int> & param, const std::string & paramName = "", bool owned = false);
	void addParam(float param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::vector<float> & param, const std::string & paramName = "", bool owned = false);
	void addParam(const char * param, const std::string & paramName = "", bool owned = false);
	void addParam(std::vector<const char *> & param, const std::string & paramName = "", bool owned = false);
	void addParam(const Unicode::String & param, const std::string & paramName = "", bool owned = false);
	void addParam(std::vector<const Unicode::String *> & param, const std::string & paramName = "", bool owned = false);
	void addParam(const NetworkId & param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::vector<NetworkId> & param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::vector<CachedNetworkId> & param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::vector<const std::vector<NetworkId> *> & param, const std::string & paramName = "", bool owned = false);
	void addParam(const StringId & param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::vector<const StringId *> & param, const std::string & paramName = "", bool owned = false);
//	void addParam(SlotData param);
//	void addParam(const std::vector<SlotData> & param);
	void addParam(const AttribMod::AttribMod & param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::vector<AttribMod::AttribMod> & param, const std::string & paramName = "", bool owned = false);
	void addParam(const ServerObjectTemplate::MentalStateMod & param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::vector<ServerObjectTemplate::MentalStateMod> & param, const std::string & paramName = "", bool owned = false);
	void addParam(const ManufactureObjectInterface & param, const std::string & paramName = "", bool owned = false);
	void addParam(const Crafting::IngredientSlot & param, const ManufactureObjectInterface & schematic, int amountRequired, const std::string & appearance, const std::string & paramName = "", bool owned = false);
	void addParam(const Vector &location, const std::string &paramName = "", bool owned = false);
	void addParam(const std::vector<const Vector *> &location, const std::string &paramName = "", bool owned = false);
	void addParam(const ValueDictionary & param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::vector<ValueDictionary> & param, const std::string & paramName = "", bool owned = false);

	void addParam(const MenuDataVector & param, const std::string & paramName = "", bool owned = false);
	void addParam(const std::vector<unsigned char> & param, const std::string & paramName = "", bool owned = false);
	
	bool changeParam(int index, int param, bool owned = false);
	bool changeParam(int index, std::vector<int> & param, bool owned = false);
	bool changeParam(int index, float param, bool owned = false);
	bool changeParam(int index, const StringId & param, bool owned = false);
	bool changeParam(int index, const MenuDataVector & param, bool owned = false);
	bool changeParam(int index, std::vector<const char *> & param, bool owned = false);
	bool changeParam(int index, std::vector<const Unicode::String *> & param, bool owned = false);
	bool changeParam(int index, std::vector<unsigned char> & param, bool owned = false);

	std::vector<Param> &                                   getParams();
	const std::vector<Param> &                             getParams() const;
	int                                                       getParamCount(void) const;
	Param::ParamType                                          getParamType(int index) const;
	const std::string &                                       getParamName(int index) const;
	bool                                                      getBoolParam(int index) const;
	const std::deque<bool> &                               getBoolArrayParam(int index) const;
	int                                                       getIntParam(int index) const;
	const std::vector<int> &                               getIntArrayParam(int index) const;
	float                                                     getFloatParam(int index) const;
	const std::vector<float> &                             getFloatArrayParam(int index) const;
	const char *                                              getStringParam(int index) const;
	const std::vector<const char *> &                      getStringArrayParam(int index) const;
	const Unicode::String &                                   getUnicodeParam(int index) const;
	const std::vector<const Unicode::String *> &           getUnicodeArrayParam(int index) const;
	const NetworkId &                                         getObjIdParam(int index) const;
	const std::vector<NetworkId> &                         getObjIdArrayParam(int index) const;
	const std::vector<CachedNetworkId> &                   getCachedObjIdArrayParam(int index) const;
	const std::vector<const std::vector<NetworkId> *> & getObjIdArrayArrayParam(int index) const;
	const Vector &                                            getLocationParam(int index) const;
	const std::vector<const Vector *> &                    getLocationArrayParam(int index) const;
	const StringId &                                          getStringIdParam(int index) const;
	const std::vector<const StringId *> &                  getStringIdArrayParam(int index) const;
	const AttribMod::AttribMod &                              getAttribModParam(int index) const;
	const std::vector<unsigned char> &                     getByteArrayParam(int index) const;
	const std::vector<AttribMod::AttribMod> &              getAttribModArrayParam(int index) const;
	const ServerObjectTemplate::MentalStateMod &                   getMentalStateModParam(int index) const;
	const std::vector<ServerObjectTemplate::MentalStateMod> &   getMentalStateModArrayParam(int index) const;
	const MenuDataVector &                                    getObjectMenuRequestDataArrayParam(int index) const;
	const ManufactureObjectInterface &                        getManufactureSchematicParam(int index) const;
	const Crafting::IngredientSlot &                          getIngredientSlotParam(int index) const;
	const ManufactureObjectInterface &                        getIngredientSlotParamSchematic(int index) const;
	int                                                       getIngredientSlotParamAmountRequired(int index) const;
	const std::string &                                       getIngredientSlotParamAppearance(int index) const;
	const ValueDictionary &                                   getValueDictionaryParam(int index) const;
	const std::vector<ValueDictionary> &                   getValueDictionaryArrayParam(int index) const;

protected:
	friend void Archive::get(Archive::ReadIterator & source, ScriptParams & target);

private:

	std::vector<Param> m_params;
};



inline void ScriptParams::clear(void)
{
	m_params.clear();
}	// ScriptParams::clear

inline std::vector<Param> & ScriptParams::getParams()
{
	return m_params;
}

inline const std::vector<Param> & ScriptParams::getParams() const
{
	return m_params;
}

inline int ScriptParams::getParamCount(void) const
{
	return m_params.size();
}	// ScriptParams::getParamCount

inline Param::ParamType ScriptParams::getParamType(int index) const
{
	return m_params[index].m_type;
}	// ScriptParams::getParamType

inline const std::string & ScriptParams::getParamName(int index) const
{
	return m_params[index].m_name;
}

inline bool ScriptParams::getBoolParam(int index) const
{
	return m_params[index].m_param.bParam;
}	// ScriptParams::getBoolParam

inline const std::deque<bool> & ScriptParams::getBoolArrayParam(int index) const
{
	return *m_params[index].m_param.baParam;
}	// ScriptParams::getBoolArrayParam

inline int ScriptParams::getIntParam(int index) const
{
	return m_params[index].m_param.iParam;
}	// ScriptParams::getIntParam

inline const std::vector<int> & ScriptParams::getIntArrayParam(int index) const
{
	return *m_params[index].m_param.iaParam;
}	// ScriptParams::getIntArrayParam

inline float ScriptParams::getFloatParam(int index) const
{
	return m_params[index].m_param.fParam;
}	// ScriptParams::getFloatParam

inline const std::vector<float> & ScriptParams::getFloatArrayParam(int index) const
{
	return *m_params[index].m_param.faParam;
}	// ScriptParams::getFloatArrayParam

inline const char * ScriptParams::getStringParam(int index) const
{
	return m_params[index].m_param.sParam;
}	// ScriptParams::getStringParam

inline const std::vector<const char *> & ScriptParams::getStringArrayParam(int index) const
{
	return *m_params[index].m_param.saParam;
}	// ScriptParams::getStringArrayParam

inline const Unicode::String & ScriptParams::getUnicodeParam(int index) const
{
	return *m_params[index].m_param.uParam;
}	// ScriptParams::getUnicodeParam

inline const std::vector<const Unicode::String *> & ScriptParams::getUnicodeArrayParam(int index) const
{
	return *m_params[index].m_param.uaParam;
}	// ScriptParams::getUnicodeArrayParam

inline const Vector &ScriptParams::getLocationParam(int index) const
{
	return *m_params[index].m_param.lParam;
}	// ScriptParams::getIntParam

inline const std::vector<const Vector *> & ScriptParams::getLocationArrayParam(int index) const
{
	return *m_params[index].m_param.laParam;
}	// ScriptParams::getIntArrayParam

inline const NetworkId & ScriptParams::getObjIdParam(int index) const
{
	if (m_params[index].m_param.oidParam != nullptr)
		return *m_params[index].m_param.oidParam;
	return NetworkId::cms_invalid;
}	// ScriptParams::getObjIdParam

inline const std::vector<NetworkId> & ScriptParams::getObjIdArrayParam(int index) const
{
	return *m_params[index].m_param.oidaParam;
}	// ScriptParams::getObjIdArrayParam

inline const std::vector<CachedNetworkId> & ScriptParams::getCachedObjIdArrayParam(int index) const
{
	return *m_params[index].m_param.coidaParam;
}	// ScriptParams::getCachedObjIdArrayParam

inline const std::vector<const std::vector<NetworkId> *> & ScriptParams::getObjIdArrayArrayParam(int index) const
{
	return *m_params[index].m_param.oidaaParam;
}

inline const StringId & ScriptParams::getStringIdParam(int index) const
{
	return *m_params[index].m_param.sidParam;
}	// ScriptParams::getStringIdParam

inline const std::vector<const StringId *> & ScriptParams::getStringIdArrayParam(int index) const
{
	return *m_params[index].m_param.sidaParam;
}	// ScriptParams::getStringIdArrayParam

inline const AttribMod::AttribMod & ScriptParams::getAttribModParam(int index) const
{
	return *m_params[index].m_param.amParam;
}	// ScriptParams::getAttribModParam

inline const std::vector<AttribMod::AttribMod> & ScriptParams::getAttribModArrayParam(int index) const
{
	return *m_params[index].m_param.amaParam;
}	// ScriptParams::getAttribModArrayParam

inline const ServerObjectTemplate::MentalStateMod & ScriptParams::getMentalStateModParam(int index) const
{
	return *m_params[index].m_param.msmParam;
}	// ScriptParams::getMentalStateModParam

inline const std::vector<ServerObjectTemplate::MentalStateMod> & ScriptParams::getMentalStateModArrayParam(int index) const
{
	return *m_params[index].m_param.amsmParam;
}	// ScriptParams::getMentalStateModArrayParam

inline const MenuDataVector &  ScriptParams::getObjectMenuRequestDataArrayParam(int index) const
{
	return *m_params[index].m_param.aomrdParam;
}

inline const ManufactureObjectInterface & ScriptParams::getManufactureSchematicParam(int index) const
{
	return *m_params[index].m_param.msoParam;
}

inline const Crafting::IngredientSlot & ScriptParams::getIngredientSlotParam(int index) const
{
	return *m_params[index].m_param.isParam.slot;
}

inline const ManufactureObjectInterface & ScriptParams::getIngredientSlotParamSchematic(int index) const
{
	return *m_params[index].m_param.isParam.schematic;
}

inline int ScriptParams::getIngredientSlotParamAmountRequired(int index) const
{
	return m_params[index].m_param.isParam.amountRequired;
}

inline const std::string & ScriptParams::getIngredientSlotParamAppearance(int index) const
{
	return *m_params[index].m_param.isParam.appearance;
}

inline const ValueDictionary & ScriptParams::getValueDictionaryParam(int index) const
{
	return *m_params[index].m_param.valueDictionaryParam;
}

inline const std::vector<ValueDictionary> & ScriptParams::getValueDictionaryArrayParam(int index) const
{
	return *m_params[index].m_param.valueDictionaryArrayParam;
}

inline const std::vector<unsigned char> & ScriptParams::getByteArrayParam(int index) const
{
	return *m_params[index].m_param.byteArrayParam;
}	// ScriptParams::getIntArrayParam

#endif	// _INCLUDED_ScriptParams_H
