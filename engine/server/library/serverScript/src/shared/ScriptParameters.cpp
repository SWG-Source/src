//========================================================================
//
// ScriptParams.cpp - varargs substitute for passing params to scripts.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/ScriptParameters.h"

#include "serverScript/JavaLibrary.h"
#include "serverScript/ScriptParamArchive.h"
#include "sharedGame/AttribMod.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/ObjectMenuRequestData.h" //@todo: move this to a different library (sharedUserInterface)
#include "sharedNetworkMessages/ObjectMenuRequestDataArchive.h"
#include "sharedUtility/ValueDictionary.h"
#include <deque>

//========================================================================

/**
 * Class constructor.
 */
ScriptParams::ScriptParams(void) :
m_params()
{
}	// ScriptParams::ScriptParams

/**
 * Class destructor.
 */
ScriptParams::~ScriptParams(void)
{
	// delete data that we own
	for (std::vector<Param>::iterator i = m_params.begin(); i != m_params.end(); ++i)
	{
		Param & p = (*i);
		if (p.m_owned)
		{
			switch(p.m_type)
			{
			case Param::BOOL_ARRAY:
				{
					delete p.m_param.baParam;
				}
				break;
			case Param::INT_ARRAY:
				{
					delete p.m_param.iaParam;
				}
				break;
			case Param::FLOAT_ARRAY:
				{
					delete p.m_param.faParam;
				}
				break;
			case Param::STRING:
				{
					// @todo : what is this bullshit? Can't vector delete a const pointer?
					delete [] const_cast<char *>(p.m_param.sParam);
				}
				break;
			case Param::STRING_ARRAY:
				{
					std::vector<const char *>::const_iterator s;
					for(s = p.m_param.saParam->begin(); s != p.m_param.saParam->end(); ++s)
					{
						const char * c = (*s);
						delete [] const_cast<char *>(c);
					}
					delete p.m_param.saParam;
				}
				break;
			case Param::UNICODE:
				{
					delete p.m_param.uParam;
				}
				break;
			case Param::UNICODE_ARRAY:
				{
					std::vector<const Unicode::String *>::const_iterator s;
					for(s = p.m_param.uaParam->begin(); s != p.m_param.uaParam->end(); ++s)
					{
						const Unicode::String * c = (*s);
						delete const_cast<Unicode::String *>(c);
					}
					delete p.m_param.uaParam;
				}
				break;
			case Param::OBJECT_ID:
				{
					delete p.m_param.oidParam;
				}
				break;
			case Param::OBJECT_ID_ARRAY:
				{
					delete p.m_param.oidaParam;
				}
				break;
			case Param::CACHED_OBJECT_ID_ARRAY:
				{
					delete p.m_param.coidaParam;
				}
				break;
			case Param::OBJECT_ID_ARRAY_ARRAY:
				{
					std::vector<const std::vector<NetworkId> *>::const_iterator iter;
					for (iter = p.m_param.oidaaParam->begin(); 
						iter != p.m_param.oidaaParam->end(); ++iter)
					{
						delete *iter;
					}
					delete p.m_param.oidaaParam;
				}
				break;
			case Param::STRING_ID:
				{
					delete p.m_param.sidParam;
				}
				break;
			case Param::STRING_ID_ARRAY:
				{
					std::vector<const StringId *>::const_iterator s;
					for(s = p.m_param.sidaParam->begin(); s != p.m_param.sidaParam->end(); ++s)
					{
						const StringId * c = (*s);
						delete c;
					}
					delete p.m_param.sidaParam;
				}
				break;
			case Param::ATTRIB_MOD:
				{
					delete const_cast<AttribMod::AttribMod *>(p.m_param.amParam);
				}
				break;
			case Param::ATTRIB_MOD_ARRAY:
				{
					delete p.m_param.amaParam;
				}
				break;
			case Param::MENTAL_STATE_MOD:
				{
					delete const_cast<ServerObjectTemplate::MentalStateMod *>(p.m_param.msmParam);
				}
				break;
			case Param::MENTAL_STATE_MOD_ARRAY:
				{
					delete p.m_param.amsmParam;
				}
				break;
			case Param::OBJECT_MENU_INFO:
				{
					delete p.m_param.aomrdParam;
				}
				break;
			case Param::LOCATION:
				{
					delete const_cast<Vector *>(p.m_param.lParam);
				}
				break;
			case Param::LOCATION_ARRAY:
				{
					std::vector<const Vector *>::const_iterator s;
					for(s = p.m_param.laParam->begin(); s != p.m_param.laParam->end(); ++s)
					{
						const Vector * c = (*s);
						delete const_cast<Vector *>(c);
					}
					delete p.m_param.laParam;
				}
				break;
			case Param::DICTIONARY:
				{
					delete p.m_param.valueDictionaryParam;
				}
				break;
			case Param::DICTIONARY_ARRAY:
				{
					delete p.m_param.valueDictionaryArrayParam;
				}
				break;
			default:
				break;
			}
		}
	}
}	// ScriptParams::~ScriptParams

/**
 * Adds a boolean param.
 */
void ScriptParams::addParam(bool param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::BOOL;
	p.m_param.bParam = param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(bool)

/**
 * INTENTIONALLY COMMENTED OUT WHILE LEAVING DECLARATION
 * This call occurs because of a conversion of a datatype passing through this function from std::string to Unicode::String.
 * Basically, most times it is called with std::string the c_str() function is called.  Since this is still valid when the
 * type is changed to Unicode::String and the const Unicode::unicode_char_t * is successfully auto-converted to const char *
 * you wind up having a nullptr string when it get explicitly converted to const char * on the other side.  So, this should never
 * be called except in error.
 *
void ScriptParams::addParam(const Unicode::unicode_char_t * param, const std::string & paramName, bool owned)
{
	DEBUG_FATAL(true, ("ScriptParams::addParam(const Unicode::unicode_char_t *, const std::string &, bool) called when ScriptParams::addParam(const Unicode::String, ...) expected."));
	WARNING(true, ("ScriptParams::addParam(const Unicode::unicode_char_t *, const std::string &, bool) called when ScriptParams::addParam(const Unicode::String, ...) expected."));
}	// ScriptParams::addParam(const Unicode::unicode_char_t *)
*/

/**
 * Adds a boolean array param.
 */
void ScriptParams::addParam(const std::deque<bool> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::BOOL_ARRAY;
	p.m_param.baParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const std::vector<bool> &)

/**
 * Adds an integer param.
 */
void ScriptParams::addParam(int param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::INT;
	p.m_param.iParam = param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(int)

/**
 * Adds an integer array param.
 */
void ScriptParams::addParam(const std::vector<int> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::INT_ARRAY;
	p.m_param.iaParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const std::vector<int> &)

/**
 * Adds a float param.
 */
void ScriptParams::addParam(float param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::FLOAT;
	p.m_param.fParam = param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(float)

/**
 * Adds a float array param.
 */
void ScriptParams::addParam(const std::vector<float> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::FLOAT_ARRAY;
	p.m_param.faParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const std::vector<float> &)

/**
 * Adds a string param.
 */
void ScriptParams::addParam(const char * param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::STRING;
	p.m_param.sParam = param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const char *)

/**
 * Adds a string array param.
 */
void ScriptParams::addParam(std::vector<const char *> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::STRING_ARRAY;
	p.m_param.saParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(std::vector<const char *> &)

/**
 * Adds a unicode param.
 */
void ScriptParams::addParam(const Unicode::String & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::UNICODE;
	p.m_param.uParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const Unicode::String &)

/**
 * Adds a unicode array param.
 */
void ScriptParams::addParam(std::vector<const Unicode::String *> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::UNICODE_ARRAY;
	p.m_param.uaParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(std::vector<Unicode::String *> &)

/**
 * Adds a location param.
 */
void ScriptParams::addParam(const Vector & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::LOCATION;
	p.m_param.lParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const char *)

/**
 * Adds a location array param.
 */
void ScriptParams::addParam(const std::vector<const Vector *> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::LOCATION_ARRAY;
	p.m_param.laParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const std::vector<const Vector *> &)

/**
 * Adds an object id param.
 */
void ScriptParams::addParam(const NetworkId & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::OBJECT_ID;
	p.m_param.oidParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(NetworkId)

/**
 * Adds an object id array param.
 */
void ScriptParams::addParam(const std::vector<NetworkId> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::OBJECT_ID_ARRAY;
	p.m_param.oidaParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const std::vector<NetworkId> &)

/**
 * Adds a cached object id array param.
 */
void ScriptParams::addParam(const std::vector<CachedNetworkId> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::CACHED_OBJECT_ID_ARRAY;
	p.m_param.coidaParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const std::vector<CachedNetworkId> &)

/**
 * Adds an object id array param.
 */
void ScriptParams::addParam(const std::vector<const std::vector<NetworkId> *> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::OBJECT_ID_ARRAY_ARRAY;
	p.m_param.oidaaParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const std::vector<std::vector<NetworkId> > &)

/**
 * Adds a string id param.
 */
void ScriptParams::addParam(const StringId & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::STRING_ID;
	p.m_param.sidParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const StringId &)

/**
 * Adds a string id array param.
 */
void ScriptParams::addParam(const std::vector<const StringId *> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::STRING_ID_ARRAY;
	p.m_param.sidaParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const std::vector<StringId *> &)

/**
 * Adds an attribute mod param.
 */
void ScriptParams::addParam(const AttribMod::AttribMod & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::ATTRIB_MOD;
	p.m_param.amParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const AttribMod::AttribMod &)

/**
 * Adds an attribute mod array param.
 */
void ScriptParams::addParam(const std::vector<AttribMod::AttribMod> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::ATTRIB_MOD_ARRAY;
	p.m_param.amaParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const std::vector<AttribMod::AttribMod *> &)

/**
 * Adds a mental state mod param.
 */
void ScriptParams::addParam(const ServerObjectTemplate::MentalStateMod & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::ATTRIB_MOD;
	p.m_param.msmParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const ServerObjectTemplate::MentalStateMod &)

/**
 * Adds a mental state mod array param.
 */
void ScriptParams::addParam(const std::vector<ServerObjectTemplate::MentalStateMod> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::ATTRIB_MOD_ARRAY;
	p.m_param.amsmParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	//ScriptParams::addParam(const std::vector<ServerObjectTemplate::MentalStateMod> &)

/**
 * Adds an array of menu info data
 */
void ScriptParams::addParam(const MenuDataVector & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::OBJECT_MENU_INFO;
	p.m_param.aomrdParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const MenuDataVector &)

/**
 * Adds a manufacture schematic object param.
 */
void ScriptParams::addParam(const ManufactureObjectInterface & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::MANUFACTURE_SCHEMATIC;
	p.m_param.msoParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;	
}	// ScriptParams::addParam(const ManufactureSchematicObject &)

/**
 * Adds a Crafting::IngredientSlot param.
 */
void ScriptParams::addParam(const Crafting::IngredientSlot & param, 
	const ManufactureObjectInterface & schematic, int amountRequired, 
	const std::string & appearance, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::INGREDIENT_SLOT;
	p.m_param.isParam.slot = &param;
	p.m_param.isParam.schematic = &schematic;
	p.m_param.isParam.amountRequired = amountRequired;
	p.m_param.isParam.appearance = &appearance;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const Crafting::IngredientSlot &)

/**
 * Adds a ValueDictionary param.
 */
void ScriptParams::addParam(const ValueDictionary & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::DICTIONARY;
	p.m_param.valueDictionaryParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}

/**
 * Adds a ValueDictionary array param.
 */
void ScriptParams::addParam(const std::vector<ValueDictionary> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::DICTIONARY_ARRAY;
	p.m_param.valueDictionaryArrayParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}

/**
 * Adds an integer array param.
 */
void ScriptParams::addParam(const std::vector<unsigned char> & param, const std::string & paramName, bool owned)
{
	m_params.push_back(Param());
	Param &p = m_params.back();
	p.m_type = Param::BYTE_ARRAY;
	p.m_param.byteArrayParam = &param;
	p.m_name = paramName;
	p.m_owned = owned;
}	// ScriptParams::addParam(const std::vector<int> &)

/**
 * Changes the value of a parameter.
 *
 * @index		the index of the parameter
 * @param		the new value of the parameter
 *
 * @return true if the value was changed, false if not
 */
bool ScriptParams::changeParam(int index, int param, bool owned)
{
	if (m_params.size() <= static_cast<std::vector<Param>::size_type>(index))
		return false;
	
	Param &p = m_params[index];
	if (p.m_type != Param::INT)
		return false;
	
	p.m_param.iParam = param;
	p.m_owned = owned;
	return true;
}	// ScriptParams::changeParam(int, int)

/**
 * Changes the value of a parameter.
 *
 * @index		the index of the parameter
 * @param		the new value of the parameter
 *
 * @return true if the value was changed, false if not
 */
bool ScriptParams::changeParam(int index, std::vector<int> & param, bool owned)
{
	if (m_params.size() <= static_cast<std::vector<Param>::size_type>(index))
		return false;
	
	Param &p = m_params[index];
	if (p.m_type != Param::INT_ARRAY)
		return false;
	
	if (p.m_owned)
		delete p.m_param.iaParam;

	p.m_param.iaParam = &param;
	p.m_owned = owned;
	return true;
}	// ScriptParams::changeParam(int, const std::vector<int> &)

/**
 * Changes the value of a parameter.
 *
 * @index		the index of the parameter
 * @param		the new value of the parameter
 *
 * @return true if the value was changed, false if not
 */
bool ScriptParams::changeParam(int index, float param, bool owned)
{
	if (m_params.size() <= static_cast<std::vector<Param>::size_type>(index))
		return false;
	
	Param &p = m_params[index];
	if (p.m_type != Param::FLOAT)
		return false;
	
	p.m_param.fParam = param;
	p.m_owned = owned;
	return true;
}	// ScriptParams::changeParam(int, float)

/**
 * Changes the value of a parameter.
 *
 * @index		the index of the parameter
 * @param		the new value of the parameter
 *
 * @return true if the value was changed, false if not
 */
bool ScriptParams::changeParam(int index, const StringId & param, bool owned)
{
	if (m_params.size() <= static_cast<std::vector<Param>::size_type>(index))
		return false;
	
	Param &p = m_params[index];
	if (p.m_type != Param::STRING_ID)
		return false;
	
	if (p.m_param.sidParam == nullptr)
		return false;
	
	if (p.m_owned)
		delete p.m_param.sidParam;

	p.m_param.sidParam = &param;
	p.m_owned = owned;
	return true;
}	// ScriptParams::changeParam(int, const StringId &)

/**
 * Changes the value of a parameter.
 *
 * @index		the index of the parameter
 * @param		the new value of the parameter
 *
 * @return true if the value was changed, false if not
 */
bool ScriptParams::changeParam(int index, const MenuDataVector & param, bool owned)
{
	if (m_params.size() <= static_cast<std::vector<Param>::size_type>(index))
		return false;
	
	Param &p = m_params[index];
	if (p.m_type != Param::OBJECT_MENU_INFO)
		return false;
	
	if (p.m_owned)
		delete p.m_param.aomrdParam;

	p.m_param.aomrdParam = &param;
	p.m_owned = owned;
	return true;
}	// ScriptParams::changeParam(int, const MenuDataVector &)

/**
 * Changes the value of a parameter.
 *
 * @index		the index of the parameter
 * @param		the new value of the parameter
 *
 * @return true if the value was changed, false if not
 */
bool ScriptParams::changeParam(int index, std::vector<const char *> & param, bool owned)
{
	if (m_params.size() <= static_cast<std::vector<Param>::size_type>(index))
		return false;
	
	Param &p = m_params[index];
	if (p.m_type != Param::STRING_ARRAY)
		return false;
	
	if (p.m_owned)
	{
		std::vector<const char *>::const_iterator s;
		for(s = p.m_param.saParam->begin(); s != p.m_param.saParam->end(); ++s)
		{
			const char * c = (*s);
			delete [] const_cast<char *>(c);
		}
		delete p.m_param.saParam;
	}

	p.m_param.saParam = &param;
	p.m_owned = owned;
	return true;
}	// ScriptParams::changeParam(int, const std::vector<const char *> &)

/**
 * Changes the value of a parameter.
 *
 * @index		the index of the parameter
 * @param		the new value of the parameter
 *
 * @return true if the value was changed, false if not
 */
bool ScriptParams::changeParam(int index, std::vector<const Unicode::String *> & param, bool owned)
{
	if (m_params.size() <= static_cast<std::vector<Param>::size_type>(index))
		return false;
	
	Param &p = m_params[index];
	if (p.m_type != Param::UNICODE_ARRAY)
		return false;
	
	if (p.m_owned)
	{
		std::vector<const Unicode::String *>::const_iterator s;
		for(s = p.m_param.uaParam->begin(); s != p.m_param.uaParam->end(); ++s)
		{
			const Unicode::String * c = (*s);
			delete const_cast<Unicode::String *>(c);
		}
		delete p.m_param.uaParam;
	}

	p.m_param.uaParam = &param;
	p.m_owned = owned;
	return true;
}	// ScriptParams::changeParam(int, const std::vector<Unicode::String *> &)

/**
 * Changes the value of a parameter.
 *
 * @index		the index of the parameter
 * @param		the new value of the parameter
 *
 * @return true if the value was changed, false if not
 */
bool ScriptParams::changeParam(int index, std::vector<unsigned char> & param, bool owned)
{
	if (m_params.size() <= static_cast<std::vector<Param>::size_type>(index))
		return false;
	
	Param &p = m_params[index];
	if (p.m_type != Param::BYTE_ARRAY)
		return false;
	
	if (p.m_owned)
		delete p.m_param.byteArrayParam;

	p.m_param.byteArrayParam = &param;
	p.m_owned = owned;
	return true;
}	// ScriptParams::changeParam(int, const std::vector<int> &)

//-----------------------------------------------------------------------

namespace Archive {


//-----------------------------------------------------------------------

void put(ByteStream & target, const ScriptParams & source)
{
	Archive::put(target, source.getParams());
}

//-----------------------------------------------------------------------

void get(ReadIterator & source, ScriptParams & target)
{
	Archive::get(source, target.getParams());
}

//-----------------------------------------------------------------------

}//namespace Archive

