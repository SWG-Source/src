// ======================================================================
//
// ConsoleCommandParserObjvar.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserObjvar.h"

#include "UnicodeUtils.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/DynamicVariableListNestedList.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/GameObjectTypes.h"
#include <cstdio>

// ======================================================================

//TODO:  A lot of this file could be replaced by calls to ObjectVariable::debugPrint(), but need to make sure the formatting is consistent whith
// what the god client expects

// value types for set objvar
enum
{
	INT_ARGUMENT,
	REAL_ARGUMENT,
	STRING_ARGUMENT
};

static const CommandParser::CmdInfo cmds[] =
{
	{"set",    3, "<oid> <name> <value>",  "Sets an objvar for an object."},
	{"setlist",2, "<oid> <name>",          "Creates a new objvar list."},
	{"get",    2, "<oid> <name>",          "Prints the value of an objvar."},
	{"list",   0, "",                      "Lists all the objvars on an object."},
	{"listWithPacking", 1, "<oid>",      "Lists all the objvars on an object, showing which are packed into the OBJECTS table in the database."},
	{"remove", 2, "<oid> <name>",          "Removes an objvar from an object."},
	{"", 0, "", ""} // this must be last
};

//-----------------------------------------------------------------

static std::string getDebugString(Transform const &tr)
{
	Vector i(tr.getLocalFrameI_p());
	Vector j(tr.getLocalFrameJ_p());
	Vector k(tr.getLocalFrameK_p());
	Vector p(tr.getPosition_p());

	std::string ret("\n");
	char buf[256];

	snprintf(buf, 256, "[%g %g %g %g]\n", static_cast<double>(i.x), static_cast<double>(j.x), static_cast<double>(k.x), static_cast<double>(p.x));
	ret += buf;
	snprintf(buf, 256, "[%g %g %g %g]\n", static_cast<double>(i.y), static_cast<double>(j.y), static_cast<double>(k.y), static_cast<double>(p.y));
	ret += buf;
	snprintf(buf, 256, "[%g %g %g %g]\n", static_cast<double>(i.z), static_cast<double>(j.z), static_cast<double>(k.z), static_cast<double>(p.z));
	ret += buf;

	return ret;
}

//-----------------------------------------------------------------

static std::string getDebugString(Vector const &v)
{
	char buf[256];
	snprintf(buf, 256, "[%g %g %g]", static_cast<double>(v.x), static_cast<double>(v.y), static_cast<double>(v.z));
	return std::string(buf);
}

//-----------------------------------------------------------------

ConsoleCommandParserObjvar::ConsoleCommandParserObjvar (void) :
CommandParser ("objvar", 0, "...", "Objvar related commands.", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------


bool ConsoleCommandParserObjvar::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL (node);
	UNREF (userId);

	UNREF(originalCommand);

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	//-----------------------------------------------------------------

	if (isAbbrev( argv [0], "set"))
	{
		NetworkId oid (Unicode::wideToNarrow (argv[1]));
		ServerObject* object = ServerWorld::findObjectByNetworkId(oid);
		if (object == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}

		const Unicode::NarrowString &objvarName = Unicode::wideToNarrow(argv[2]);

		// can't dork around with the "resourceData" objvar on a resource container
		// or a manufacturing schematic or an incubator, since it contains resource
		// data for "imported" resources that *MUST* remain the same as the
		// corresponding objvar on every other resource container or a manufacturing
		// schematic that references that "imported" resource.
		if (objvarName == "power.resourceData")
		{
			const char *objectTemplateName = object->getObjectTemplateName();
			if (objectTemplateName && (std::string(objectTemplateName) == "object/tangible/crafting/station/incubator_station.iff"))
			{
				result += Unicode::narrowToWide("You cannot set the \"resourceData\" objvar on an incubator\n");
				return true;
			}
		}

		int const gameObjectType = object->getGameObjectType();
		if (GameObjectTypes::isTypeOf(gameObjectType, SharedObjectTemplate::GOT_resource_container) || (gameObjectType == SharedObjectTemplate::GOT_data_manufacturing_schematic))
		{
			std::string::size_type const pos = objvarName.rfind("resourceData");
			if (pos == (objvarName.size() - sizeof("resourceData") + 1))
			{
				result += Unicode::narrowToWide("You cannot set the \"resourceData\" objvar on a resource crate or a manufacturing schematic\n");
				return true;
			}
		}

		if (!object->getObjVars().hasItem(objvarName))
		{
			// create a new objvar, deduce type from argument string
			const Unicode::NarrowString &nString = Unicode::wideToNarrow(argv[3]);
			switch (getArgumentType(nString))
			{
				case INT_ARGUMENT:
					object->setObjVarItem(objvarName, atoi(nString.c_str()));
					break;
				case REAL_ARGUMENT:
					object->setObjVarItem(objvarName, static_cast<real>(atof(nString.c_str())));
					break;
				case STRING_ARGUMENT:
					object->setObjVarItem(objvarName, argv[3]);
					break;
				default:
					break;
			}
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
		{
			// verify that the argument passed in is the same type as the objvar
			switch (object->getObjVars().getType(objvarName))
			{
				case DynamicVariable::INT:
					{
						const Unicode::NarrowString &nString = Unicode::wideToNarrow(argv[3]);
						if (getArgumentType(nString) == INT_ARGUMENT)
						{
							object->setObjVarItem(objvarName, atoi(nString.c_str()));
							result += getErrorMessage(argv[0], ERR_SUCCESS);
						}
						else
							result += getErrorMessage(argv[0], ERR_INVALID_INT_OBJVAR);
					}
					break;
				case DynamicVariable::INT_ARRAY:
					result += getErrorMessage(argv[0], ERR_CMD_NOT_FOUND);
					break;
				case DynamicVariable::REAL:
					{
						const Unicode::NarrowString &nString = Unicode::wideToNarrow(argv[3]);
						if (getArgumentType(nString) == REAL_ARGUMENT)
						{
							object->setObjVarItem(objvarName, static_cast<real>(atof(nString.c_str())));
							result += getErrorMessage(argv[0], ERR_SUCCESS);
						}
						else
							result += getErrorMessage(argv[0], ERR_INVALID_REAL_OBJVAR);
					}
					break;
				case DynamicVariable::REAL_ARRAY:
					result += getErrorMessage(argv[0], ERR_CMD_NOT_FOUND);
					break;
				case DynamicVariable::STRING:
					{
						object->setObjVarItem(objvarName, argv[3]);
						result += getErrorMessage(argv[0], ERR_SUCCESS);
					}
					break;
				case DynamicVariable::STRING_ARRAY:
					result += getErrorMessage(argv[0], ERR_CMD_NOT_FOUND);
					break;
				case DynamicVariable::NETWORK_ID:
				case DynamicVariable::NETWORK_ID_ARRAY:
				case DynamicVariable::LOCATION:
				case DynamicVariable::LOCATION_ARRAY:
				case DynamicVariable::STRING_ID:
				case DynamicVariable::STRING_ID_ARRAY:
				case DynamicVariable::TRANSFORM:
				case DynamicVariable::TRANSFORM_ARRAY:
				case DynamicVariable::VECTOR:
				case DynamicVariable::VECTOR_ARRAY:
					result += getErrorMessage(argv[0], ERR_CMD_NOT_FOUND);
					break;
				case DynamicVariable::LIST:
					result += getErrorMessage(argv[0], ERR_LIST_OBJVAR_NOT_SUPPORTED);
					break;
			}			
		}
	}

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "setlist"))
	{
		// This is now a no-op, but left in to avoid breaking the god client
// 		NetworkId oid (Unicode::wideToNarrow (argv[1]));
// 		ServerObject* object = ServerWorld::findObjectByNetworkId(oid);
// 		if (object == nullptr)
// 		{
// 			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
// 			return true;
// 		}

// 		const Unicode::NarrowString &listName = Unicode::wideToNarrow(argv[2]);
// 		object->addNestedObjVarList(listName);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "get"))
	{
		NetworkId oid (Unicode::wideToNarrow(argv[1]));
		ServerObject* object = ServerWorld::findObjectByNetworkId(oid);
		if (object == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		std::string objvarName(Unicode::wideToNarrow(argv[2]));
		if (object->getObjVars().hasItem(objvarName))
		{
			char buffer[32];
			switch (object->getObjVars().getType(objvarName))
			{
				case DynamicVariable::INT:
					{
						int value = 0;
						object->getObjVars().getItem(objvarName,value);
						IGNORE_RETURN(_itoa(value, buffer, 10));
						result += Unicode::narrowToWide(buffer);
					}
					break;
				case DynamicVariable::INT_ARRAY:
					result += getErrorMessage(argv[0], ERR_CMD_NOT_FOUND);
					break;
				case DynamicVariable::REAL:
					{
						float value = 0;
						object->getObjVars().getItem(objvarName,value);
						sprintf(buffer, "%f", value);
						result += Unicode::narrowToWide(buffer);
					}
					break;
				case DynamicVariable::REAL_ARRAY:
					result += getErrorMessage(argv[0], ERR_CMD_NOT_FOUND);
					break;
				case DynamicVariable::STRING:
					{
						Unicode::String value;
						object->getObjVars().getItem(objvarName,value);
						result += value;
					}
					break;
				case DynamicVariable::STRING_ARRAY:
					result += getErrorMessage(argv[0], ERR_CMD_NOT_FOUND);
					break;
				case DynamicVariable::NETWORK_ID:
				case DynamicVariable::NETWORK_ID_ARRAY:
				case DynamicVariable::LOCATION:
				case DynamicVariable::LOCATION_ARRAY:
				case DynamicVariable::STRING_ID:
				case DynamicVariable::STRING_ID_ARRAY:
				case DynamicVariable::TRANSFORM:
				case DynamicVariable::TRANSFORM_ARRAY:
				case DynamicVariable::VECTOR:
				case DynamicVariable::VECTOR_ARRAY:
					result += getErrorMessage(argv[0], ERR_CMD_NOT_FOUND);
					break;
				case DynamicVariable::LIST:
					 char buf[256]; 
					 sprintf (buf, "objvar list\n");
					 result += Unicode::narrowToWide(buf);
					 listObjvars(DynamicVariableList::NestedList(object->getObjVars(),objvarName),
						result, 0, false);
					break;
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJVAR);
		}
	}

	//-----------------------------------------------------------------

	else if (isAbbrev(argv[0], "list"))
	{
		ServerObject * const userServerObject = ServerWorld::findObjectByNetworkId(userId);

		if (userServerObject != nullptr)
		{
			NetworkId specifiedNetworkId;

			if (argv.size () > 1)
			{
				specifiedNetworkId = NetworkId(Unicode::wideToNarrow(argv[1]));
			}
			else
			{
				CreatureObject * const userCreatureObject = CreatureObject::asCreatureObject(userServerObject);

				if (userCreatureObject != nullptr)
				{
					specifiedNetworkId = userCreatureObject->getLookAtTarget();
				}
			}

			ServerObject * const specifiedServerObject = ServerWorld::findObjectByNetworkId(specifiedNetworkId);

			if (specifiedServerObject != nullptr)
			{
				DynamicVariableList const & objVarList = specifiedServerObject->getObjVars();
				FormattedString<1024> fs;
				result += Unicode::narrowToWide(fs.sprintf("objvar list for object %s, num %d\n", specifiedServerObject->getNetworkId().getValueString().c_str(), objVarList.size()));
				listObjvars(DynamicVariableList::NestedList(objVarList), result, 0, false);
				result += getErrorMessage(argv[0], ERR_SUCCESS);

				return true;
			}
		}

		result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
	}
	// ----------------------------------------------------------------------

	else if (isAbbrev(argv[0], "listWithPacking"))
	{
		NetworkId oid = NetworkId(Unicode::wideToNarrow(argv[1]));
		ServerObject* object = ServerWorld::findObjectByNetworkId(oid);
		if (object == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}

		char buf[256]; 
		sprintf (buf, "objvar list\n");
		result += Unicode::narrowToWide(buf);
		listObjvars(DynamicVariableList::NestedList(object->getObjVars()), result, 0, true);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	
	//-----------------------------------------------------------------

	else if (isAbbrev(argv[0], "remove"))
	{
		NetworkId oid = NetworkId(Unicode::wideToNarrow(argv[1]));
		ServerObject* object = ServerWorld::findObjectByNetworkId(oid);
		if (object == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
		
		const Unicode::NarrowString &objvarName = Unicode::wideToNarrow(argv[2]);

		// can't dork around with the "resourceData" objvar on a resource container
		// or a manufacturing schematic or an incubator, since it contains resource
		// data for "imported" resources that *MUST* remain the same as the
		// corresponding objvar on every other resource container or a manufacturing
		// schematic that references that "imported" resource.
		if (objvarName == "power.resourceData")
		{
			const char *objectTemplateName = object->getObjectTemplateName();
			if (objectTemplateName && (std::string(objectTemplateName) == "object/tangible/crafting/station/incubator_station.iff"))
			{
				result += Unicode::narrowToWide("You cannot remove the \"resourceData\" objvar on an incubator\n");
				return true;
			}
		}

		int const gameObjectType = object->getGameObjectType();
		if (GameObjectTypes::isTypeOf(gameObjectType, SharedObjectTemplate::GOT_resource_container) || (gameObjectType == SharedObjectTemplate::GOT_data_manufacturing_schematic))
		{
			std::string::size_type const pos = objvarName.rfind("resourceData");
			if (pos == (objvarName.size() - sizeof("resourceData") + 1))
			{
				result += Unicode::narrowToWide("You cannot remove the \"resourceData\" objvar on a resource crate or a manufacturing schematic\n");
				return true;
			}
		}

		object->removeObjVarItem(objvarName);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------

	else
	{
		result += getErrorMessage(argv[0], ERR_NO_HANDLER);
	}

	return true;
}	// ConsoleCommandParserObjvar::performParsing

//-----------------------------------------------------------------

/**
 * Determines the data type of an argument string.
 *
 * @param arg		the string to analize
 *
 * @return INT_ARGUMENT if the string is all digits, 
 *         REAL_ARGUMENT if the string is all digits plus one '.',
 *         STRING_ARGUMENT for everthing else
 */
int ConsoleCommandParserObjvar::getArgumentType(const Unicode::NarrowString &arg) const
{
	bool foundDecimal = false;

	size_t const count = arg.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!isdigit(arg[i]))
		{
			if ((i == 0) && (arg[i] == '-') && (count > 1))
				continue;

			if (arg[i] != '.' || foundDecimal)
				return STRING_ARGUMENT;
			foundDecimal = true;
		}
	}
	if (foundDecimal)
		return REAL_ARGUMENT;
	return INT_ARGUMENT;
}	// ConsoleCommandParserObjvar::getArgumentType

//-----------------------------------------------------------------

/**
 * Prints out to a string all the objvars in an objvar list.
 *
 * @param objvarList		the list to print
 * @param result			string to print the list to
 * @param tabCount          how many tabs to print before the data
 */
void ConsoleCommandParserObjvar::listObjvars(const DynamicVariableList::NestedList &objvarList, String_t &result, int tabCount, bool withPackingData)
{
static const String_t space = Unicode::narrowToWide(" ");
static const String_t spaces = Unicode::narrowToWide("  ");
static const String_t newline = Unicode::narrowToWide("\n");
static const String_t tabs = Unicode::narrowToWide("\t");
static const String_t islist = Unicode::narrowToWide("<list>\n");
char buffer[32];

	for (DynamicVariableList::NestedList::const_iterator objVar(objvarList.begin()); objVar!=objvarList.end(); ++objVar)
	{
		// pad the objvar
		for (int j = 0; j < tabCount; ++j)
			result += spaces;

		// print the name
		result += Unicode::narrowToWide(objVar.getName());
		result += tabs;
		if (withPackingData)
		{
			char buffer[100];
			sprintf(buffer,"%i",objVar.getPackedPosition());
			result += Unicode::narrowToWide(std::string(buffer));
			result += tabs;
		}
			
		// print the data
		switch (objVar.getType())
		{
			case DynamicVariable::INT:
				{
					int value=0;
					objVar.getValue(value);
					IGNORE_RETURN(_itoa(value, buffer, 10));
					result += Unicode::narrowToWide(buffer);
					result += newline;
				}
				break;
			case DynamicVariable::INT_ARRAY:
				{
					std::vector<int> value;
					objVar.getValue(value);
					std::string text = "[";
					size_t count = value.size();
					for (size_t j = 0; j < count; ++j)
					{
						IGNORE_RETURN(_itoa(value[j], buffer, 10));
						text += buffer;
						if (j + 1 < count)
							text += ", ";
					}
					text += "]\n";
					result += Unicode::narrowToWide(text);
				}
				break;
			case DynamicVariable::REAL:
				{
					real value = 0;
					objVar.getValue(value);
					sprintf(buffer, "%f\n", value);
					result += Unicode::narrowToWide(buffer);
				}
				break;
			case DynamicVariable::REAL_ARRAY:
				{
					std::vector<real> value;
					objVar.getValue(value);
					std::string text = "[";
					size_t count = value.size();
					for (size_t j = 0; j < count; ++j)
					{
						sprintf(buffer, "%f", value[j]);
						text += buffer;
						if (j + 1 < count)
							text += ", ";
					}
					text += "]\n";
					result += Unicode::narrowToWide(text);
				}
				break;
			case DynamicVariable::STRING:
				{
					Unicode::String value;
					objVar.getValue(value);
					result += value;
					result += newline;
				}
				break;
			case DynamicVariable::STRING_ARRAY:
				{
					std::vector<Unicode::String> value;
					objVar.getValue(value);
					int k;
					int nameLength = objVar.getName().size();
					result += Unicode::narrowToWide("[\n");
					size_t count = value.size();
					for (size_t j = 0; j < count; ++j)
					{
						for (k = 0; k < tabCount; ++k)
							result += spaces;
						for (k = 0; k < nameLength; ++k)
							result += space;
						result += tabs;
						result += value[j];
						result += newline;
					}
					for (k = 0; k < tabCount; ++k)
						result += spaces;
					for (k = 0; k < nameLength; ++k)
						result += space;
					result += tabs;
					result += Unicode::narrowToWide("]\n");
				}
				break;
			case DynamicVariable::NETWORK_ID:
				{
					NetworkId value;
					objVar.getValue(value);
					result += Unicode::narrowToWide("(NetworkId)");
					result += Unicode::narrowToWide(value.getValueString());
					result += newline;
				}
				break;
			case DynamicVariable::NETWORK_ID_ARRAY:
				{
					std::vector<NetworkId> value;
					objVar.getValue(value);
					std::string text = "(NetworkId)[";
					size_t count = value.size();
					for (size_t j = 0; j < count; ++j)
					{
						text += value[j].getValueString();
						if (j + 1 < count)
							text += ", ";
					}
					text += "]\n";
					result += Unicode::narrowToWide(text);
				}
				break;
			case DynamicVariable::LOCATION:
				{
					DynamicVariableLocationData value;
					objVar.getValue(value);
					result += Unicode::narrowToWide("(x=");
					sprintf(buffer, "%f", value.pos.x);
					result += Unicode::narrowToWide(buffer);
					result += Unicode::narrowToWide(", y=");
					sprintf(buffer, "%f", value.pos.y);
					result += Unicode::narrowToWide(buffer);
					result += Unicode::narrowToWide(", z=");
					sprintf(buffer, "%f", value.pos.z);
					result += Unicode::narrowToWide(buffer);
					result += Unicode::narrowToWide(", scene=");
					result += Unicode::narrowToWide(value.scene);
					result += Unicode::narrowToWide(", cell=");
					result += Unicode::narrowToWide(value.cell.getValueString());
					result += Unicode::narrowToWide(")\n");
				}
				break;
			case DynamicVariable::LOCATION_ARRAY:
				{
					std::vector<DynamicVariableLocationData> value;
					objVar.getValue(value);
					int k;
					int nameLength = objVar.getName().size();
					result += Unicode::narrowToWide("[\n");
					size_t count = value.size();
					for (size_t j = 0; j < count; ++j)
					{
						for (k = 0; k < tabCount; ++k)
							result += spaces;
						for (k = 0; k < nameLength; ++k)
							result += space;
						result += tabs;
						result += Unicode::narrowToWide("(x=");
						sprintf(buffer, "%f", value[j].pos.x);
						result += Unicode::narrowToWide(buffer);
						result += Unicode::narrowToWide(", y=");
						sprintf(buffer, "%f", value[j].pos.y);
						result += Unicode::narrowToWide(buffer);
						result += Unicode::narrowToWide(", z=");
						sprintf(buffer, "%f", value[j].pos.z);
						result += Unicode::narrowToWide(buffer);
						result += Unicode::narrowToWide(", scene=");
						result += Unicode::narrowToWide(value[j].scene);
						result += Unicode::narrowToWide(", cell=");
						result += Unicode::narrowToWide(value[j].cell.getValueString());
						result += Unicode::narrowToWide(")\n");
					}
					for (k = 0; k < tabCount; ++k)
						result += spaces;
					for (k = 0; k < nameLength; ++k)
						result += space;
					result += tabs;
					result += Unicode::narrowToWide("]\n");					
				}
				break;
			case DynamicVariable::LIST:
				result += islist;
				listObjvars(objVar.getNestedList(),
					result, tabCount + 1, withPackingData);
				break;
			case DynamicVariable::STRING_ID:
				{
					StringId value;
					objVar.getValue(value);
					result += Unicode::narrowToWide("(StringId)");
					result += Unicode::narrowToWide(value.getDebugString());
					result += newline;
				}
				break;
			case DynamicVariable::STRING_ID_ARRAY:
				{
					std::vector<StringId> value;
					objVar.getValue(value);
					std::string text = "(StringId)[";
					size_t count = value.size();
					for (size_t j = 0; j < count; ++j)
					{
						text += value[j].getDebugString();
						if (j + 1 < count)
							text += ", ";
					}
					text += "]\n";
					result += Unicode::narrowToWide(text);
				}
				break;
			case DynamicVariable::TRANSFORM:
				{
					Transform value;
					objVar.getValue(value);
					result += Unicode::narrowToWide("(Transform)");
					result += Unicode::narrowToWide(getDebugString(value));
					result += newline;
				}
				break;
			case DynamicVariable::TRANSFORM_ARRAY:
				{
					std::vector<Transform> value;
					objVar.getValue(value);
					std::string text = "(Transform)[";
					size_t count = value.size();
					for (size_t j = 0; j < count; ++j)
					{
						text += getDebugString(value[j]);
						if (j + 1 < count)
							text += ", ";
					}
					text += "]\n";
					result += Unicode::narrowToWide(text);
				}
				break;
			case DynamicVariable::VECTOR:
				{
					Vector value;
					objVar.getValue(value);
					result += Unicode::narrowToWide("(Vector)");
					result += Unicode::narrowToWide(getDebugString(value));
					result += newline;
				}
				break;
			case DynamicVariable::VECTOR_ARRAY:
				{
					std::vector<Vector> value;
					objVar.getValue(value);
					std::string text = "(Vector)[";
					size_t count = value.size();
					for (size_t j = 0; j < count; ++j)
					{
						text += getDebugString(value[j]);
						if (j + 1 < count)
							text += ", ";
					}
					text += "]\n";
					result += Unicode::narrowToWide(text);
				}
				break;
		}		
	}		
}	// ConsoleCommandParserObjvar::listObjvars


// ======================================================================
