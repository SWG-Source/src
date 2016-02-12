// ======================================================================
//
// DataTableColumnType.cpp
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/DataTableColumnType.h"

#include "UnicodeUtils.h"
#include "sharedFoundation/Crc.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <cstdio>
#include <map>

// ----------------------------------------------------------------------

static std::string chomp(std::string const &str)
{
	std::string result = str;
	while(!result.empty() && result[0] == ' ')
		result.erase(0, 1);
	while(!result.empty() && result[result.size() - 1] == ' ')
		result.erase(result.size() - 1, 1);
	return result;
}

static std::string getDelimStr(std::string const &str, char left, char right)
{
	std::string::size_type lPos = str.find(left);
	std::string::size_type rPos = str.rfind(right);

	if (lPos == std::string::npos || rPos == std::string::npos)
		return "";

	return str.substr(lPos+1, rPos-lPos-1);
}

// ----------------------------------------------------------------------

bool consumePackedObjVarIntField(char const *&s)
{
	while (*s)
	{
		char const c = *s;
		++s;
		if (c == '|')
			return true;
		else if ( c != '-' && !isdigit(c))
			break;
	}
	return false;
}

// ----------------------------------------------------------------------

bool consumePackedObjVarStringField(char const *& s)
{
	while (*s)
	{
		char const c = *s++;
		if (c == '|')
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------

DataTableColumnType::~DataTableColumnType ()
{
	delete m_enumMap;
	m_enumMap = 0;

	delete m_defaultCell;
	m_defaultCell = 0;
}

// ----------------------------------------------------------------------

DataTableColumnType::DataTableColumnType(std::string const &desc) :
	m_typeSpecString(desc),
	m_type(DT_Unknown),
	m_basicType(DT_Unknown),
	m_defaultValue(),
	m_enumMap(0),
	m_defaultCell(0)
{
	// desc may only look like:
	// {ifshbep}[def] or e(x=0,y=1,z=2,...)[def]

	// first, split into type and default value
	int type = tolower(desc[0]);
	m_defaultValue = getDelimStr(desc, '[', ']');

	if (type == 'i')
	{
		m_type = m_basicType = DT_Int;
		if (m_defaultValue.length() == 0)
			m_defaultValue = "0";
	}
	else if (type == 'f')
	{
		m_type = m_basicType = DT_Float;
		if (m_defaultValue.length() == 0)
			m_defaultValue = "0";
	}
	else if (type == 's')
	{
		m_type = m_basicType = DT_String;
	}
	else if (type == 'c')
	{
		m_type = m_basicType = DT_Comment;
	}
	else if (type == 'h')
	{
		m_type = DT_HashString;
		m_basicType = DT_Int;
	}
	else if (type == 'p')
	{
		m_type = DT_PackedObjVars;
		m_basicType = DT_String;
	}
	else if (type == 'b')
	{
		m_type = DT_Bool;
		m_basicType = DT_Int;
		if (m_defaultValue != "1")
			m_defaultValue = "0";
	}
	else if (type == 'e')
	{
		m_enumMap = new StringIntMap;
		m_type = DT_Enum;
		m_basicType = DT_Int;
		// build the enumeration map
		std::string enumList = getDelimStr(desc, '(', ')');
		enumList += ",";
		// enumList looks like "foo=0,bar=1,life=42,"
		std::string::size_type eqPos;
		while ((eqPos = enumList.find('=')) != std::string::npos)
		{
			std::string::size_type endPos = enumList.find(',');
			std::string label = enumList.substr(0, eqPos);
			std::string val = enumList.substr(eqPos+1, endPos-eqPos-1);
			(*m_enumMap)[label] = static_cast<int>(strtol(val.c_str(), nullptr, 0));
			enumList.erase(0, endPos+1);
		}
		// assure the default is a member of the enumeration
		if (m_enumMap->find(m_defaultValue) == m_enumMap->end())
		{
			WARNING(true, ("Default value [%s] is not a member of enumeration", m_defaultValue.c_str()));
			m_basicType = DT_Unknown;
		}
	}
	else if (type == 'v')
	{
		m_enumMap = new StringIntMap;
		m_type = DT_BitVector;
		m_basicType = DT_Int;
		// build the enumeration map
		std::string enumList = getDelimStr(desc, '(', ')');
		enumList += ",";
		// enumList looks like "foo=0,bar=1,life=42,"
		std::string::size_type eqPos;
		while ((eqPos = enumList.find('=')) != std::string::npos)
		{
			std::string::size_type endPos = enumList.find(',');
			std::string label = enumList.substr(0, eqPos);
			std::string val = enumList.substr(eqPos+1, endPos-eqPos-1);
			int bit = static_cast<int>(strtol(val.c_str(), nullptr, 0));
			if((bit < 1) || (bit > 32))
			{
				WARNING(true, ("Flags value [%s] is not a whole number from 1 to 32", label.c_str()));
				m_basicType = DT_Unknown;
			}
			(*m_enumMap)[label] = 1 << (bit - 1);
			enumList.erase(0, endPos+1);
		}
		// assure the default is a member of the enumeration
		if(strcmp(m_defaultValue.c_str(), "NONE") != 0)
		{		
			if (m_enumMap->find(m_defaultValue) == m_enumMap->end())
			{
				WARNING(true, ("Default value [%s] is not a member of enumeration", m_defaultValue.c_str()));
				m_basicType = DT_Unknown;
			}
		}
	}
	else if (type == 'z')
	{
		m_enumMap = new StringIntMap;
		m_type = DT_Enum;
		m_basicType = DT_Int;
		// get the filename
		std::string fileName = getDelimStr(desc, '(', ')');

		DataTable * enumTable = DataTableManager::getTable(fileName, true);
		if (!enumTable)
		{
			m_basicType = DT_Unknown;
			return;
		}
		int enumCount = enumTable->getNumRows();
		int x;
		std::string firstKey;
		for (x=0; x<enumCount; ++x)
		{
			std::string key = enumTable->getStringValue(0, x);
			Unicode::trim(key);
			int value = enumTable->getIntValue(1,x);
			if (x==0)
				firstKey = key;
			(*m_enumMap)[key] = value;
		}

		// assure the default is a member of the enumeration
		if (m_enumMap->find(m_defaultValue) == m_enumMap->end())
		{
			m_defaultValue = firstKey;
		}
	}
	else
		m_basicType = DT_Unknown;

	createDefaultCell();
}

// ----------------------------------------------------------------------

void DataTableColumnType::createDefaultCell()
{
	std::string value;

	IS_NULL(m_defaultCell);

	// If mangleValue does not update the input value string,
	//   then the default cell will have a value of 0 for floats and ints
	//     and a value of empty string for strings.
	IGNORE_RETURN(mangleValue(value));

	switch(m_basicType)
	{
	case DT_Int:
		m_defaultCell = new DataTableCell(static_cast<int>(strtol(value.c_str(), nullptr, 0)));
		break;
	case DT_Float:
		m_defaultCell = new DataTableCell(static_cast<float>(atof(value.c_str())));
		break;
	case DT_String:
	case DT_Comment:
	case DT_Unknown:
	default:
		m_defaultCell = new DataTableCell(value.c_str());
		break;
	}
}

// ----------------------------------------------------------------------

DataTableColumnType::DataTableColumnType(DataTableColumnType const &rhs) :
	m_typeSpecString(rhs.m_typeSpecString),
	m_type(rhs.m_type),
	m_basicType(rhs.m_basicType),
	m_defaultValue(rhs.m_defaultValue),
	m_enumMap(rhs.m_enumMap ? new StringIntMap (*rhs.m_enumMap) : 0),
	m_defaultCell(0)
{
	createDefaultCell();
}

// ----------------------------------------------------------------------

DataTableColumnType& DataTableColumnType::operator=(DataTableColumnType const &rhs)
{
	if (&rhs != this)
	{
		m_typeSpecString = rhs.m_typeSpecString;
		m_type = rhs.m_type;
		m_basicType = rhs.m_basicType;
		m_defaultValue = rhs.m_defaultValue;

		if(rhs.m_enumMap)
		{
			if(!m_enumMap)
				m_enumMap = new StringIntMap;
			*m_enumMap = *rhs.m_enumMap;
		}
		else
		{
			delete m_enumMap;
			m_enumMap = 0;
		}

		delete m_defaultCell;
		m_defaultCell = 0;

		createDefaultCell();
	}
	return *this;
}

// ----------------------------------------------------------------------

std::string const &DataTableColumnType::getTypeSpecString() const
{
	return m_typeSpecString;
}

// ----------------------------------------------------------------------

DataTableColumnType::DataType DataTableColumnType::getType() const
{
	return m_type;
}

// ----------------------------------------------------------------------

DataTableColumnType::DataType DataTableColumnType::getBasicType() const
{
	return m_basicType;
}

// ----------------------------------------------------------------------

bool DataTableColumnType::lookupEnum(std::string const &label, int &result) const
{
	NOT_NULL(m_enumMap);

	std::string localLabel = chomp(label);

	const StringIntMap::const_iterator i = m_enumMap->find(localLabel);
	if (i != m_enumMap->end())
	{
		result = (*i).second;
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------

bool DataTableColumnType::lookupBitVector(std::string const &label, int &result) const
{
	std::string localLabel = label;
	if(strcmp(label.c_str(), "NONE") == 0)
	{
		result = 0;
		return true;
	}
	bool foundAny = false;
	localLabel += ",";
	// enumList looks like "foo,bar,life,"
	std::string::size_type eqPos;	
	while ((eqPos = localLabel.find(',')) != std::string::npos)
	{
		std::string::size_type endPos = localLabel.find(',');
		std::string subLabel = chomp(localLabel.substr(0, eqPos));
		int subResult;
		if(lookupEnum(subLabel, subResult))
		{
			foundAny = true;
			result |= subResult;
		}
		else
		{
			DEBUG_WARNING(true, ("DataTableColumnType::lookupEnumExtended found invalid enum value '%s'", subLabel.c_str()));
		}
		localLabel.erase(0, endPos+1);
	}
				
	return foundAny;
}

// ----------------------------------------------------------------------

bool DataTableColumnType::mangleValue(std::string &value) const
{
	// if the value passed in is empty, we mangle to the default value
	if (value.length() == 0)
	{
		if (m_defaultValue == "required" || m_defaultValue == "unique")
			return false;
		else
			value = m_defaultValue;
	}

	// special validation code for packed objvars
	if (m_type == DT_PackedObjVars)
	{
		// packed objvars are of the form:
		// nameString|typeInt|valueString|nameString|typeInt|valueString|$|
		// where || may be used in the string fields to represent a |
		char const *s = value.c_str();
		while (*s)
		{
			// name
			if (s[0] == '$' && s[1] == '|' && s[2] == '\0')
				break;
			if (!consumePackedObjVarStringField(s))
				return false;
			// type
			if (!consumePackedObjVarIntField(s))
				return false;
			// value
			if (!consumePackedObjVarStringField(s))
				return false;
		}
	}

	// only basic type DT_Int are complex types that use value mangling other
	// than default values
	if (m_basicType != DT_Int || m_type == DT_Int)
		return true;
	// complex type which needs mangling
	switch (m_type)
	{
	case DT_Bool:
		{
			if (value == "0" || value == "1")
				return true;
		}
		break;
	case DT_HashString:
		{
			// conversion to integer crc
			int val;
			if (value.length())
				val = Crc::normalizeAndCalculate(value.c_str());
			else
				val = Crc::crcNull;
			char buf[16];
			sprintf(buf, "%d", val);
			value = buf;
			return true;
		}
		break;
	case DT_Enum:
		{
			// enumeration lookup
			int val = 0;
			if (lookupEnum(value, val))
			{
				char buf[16];
				sprintf(buf, "%d", val);
				value = buf;
				return true;
			}
		}
		break;
	case DT_BitVector:
		{
			// enumeration lookup
			int val = 0;
			if (lookupBitVector(value, val))
			{
				char buf[16];
				sprintf(buf, "%d", val);
				value = buf;
				return true;
			}
		}
		break;
	default:
		break;
	}
	return false;
}

// ----------------------------------------------------------------------

bool DataTableColumnType::areUniqueCellsRequired() const
{
	return m_defaultValue == "unique";
}

// ======================================================================

