//===================================================================
//
// OptionManager.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/OptionManager.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedUtility/ConfigSharedUtility.h"
#include "UnicodeUtils.h"

#include <algorithm>
#include <vector>
#include <string>

//===================================================================
// OptionManagerNamespace
//===================================================================

namespace OptionManagerNamespace
{
	const Tag TAG_BOOL = TAG(B, O, O, L);
	const Tag TAG_FLT = TAG3(F, L, T);
	const Tag TAG_INT = TAG3(I, N, T);
	const Tag TAG_OPTN = TAG(O, P, T, N);
	const Tag TAG_STDS = TAG(S, T, D, S);
	const Tag TAG_UNIS = TAG(U, N, I, S);

	bool ms_optionManagersEnabled = true;
}

using namespace OptionManagerNamespace;

//===================================================================
// PUBLIC Option
//===================================================================

OptionManager::Option::Option(Option::Type const type) :
	m_version(0),
	m_name(0),
	m_section(0),
	m_void(0),
	m_type(type)
{
}

//-------------------------------------------------------------------

OptionManager::Option::~Option()
{
}

//-------------------------------------------------------------------

OptionManager::Option::Option(Option const & rhs) :
	m_version(rhs.m_version),
	m_name(rhs.m_name),
	m_section(rhs.m_section),
	m_void(rhs.m_void),
	m_type(rhs.m_type)
{
}

//----------------------------------------------------------------------

OptionManager::Option & OptionManager::Option::operator= (Option const & rhs)
{
	m_version = rhs.m_version;
	m_name = rhs.m_name;
	m_section = rhs.m_section;
	m_void = rhs.m_void;
	m_type = rhs.m_type;

	return *this;
}

//-------------------------------------------------------------------

OptionManager::Option::Type OptionManager::Option::getType() const
{
	return m_type;
}

//-------------------------------------------------------------------

void OptionManager::Option::debugDump(char const * operation) const
{
	UNREF(operation);

	switch (m_type)
	{
	case T_bool:
		DEBUG_REPORT_LOG(true, ("OptionManager::Option %s (ver %d) [%s] %s=%s\n", operation, m_version, m_section, m_name, *m_bool ? "true" : "false"));
		break;

	case T_float:
		DEBUG_REPORT_LOG(true, ("OptionManager::Option %s (ver %d) [%s] %s=%1.5f\n", operation, m_version, m_section, m_name, *m_float));
		break;

	case T_int:
		DEBUG_REPORT_LOG(true, ("OptionManager::Option %s (ver %d) [%s] %s=%i\n", operation, m_version, m_section, m_name, *m_int));
		break;

	case T_stdString:
		DEBUG_REPORT_LOG(true, ("OptionManager::Option %s (ver %d) [%s] %s=%s\n", operation, m_version, m_section, m_name, m_stdString->c_str()));
		break;

	case T_unicodeString:
		DEBUG_REPORT_LOG(true, ("OptionManager::Option %s (ver %d) [%s] %s=%s\n", operation, m_version, m_section, m_name, Unicode::wideToNarrow(*m_unicodeString).c_str()));
		break;

	default:
		break;
	}
}

//===================================================================
// STATIC PUBLIC OptionManager
//===================================================================

void OptionManager::setOptionManagersEnabled(bool enabled)
{
	ms_optionManagersEnabled = enabled;
}

//----------------------------------------------------------------------

OptionManager::OptionManager() :
	m_registeredOptionList(new OptionList),
	m_savedOptionList(new OptionList),
	m_stringList(new StringList),
	m_boolList(new BoolList),
	m_floatList(new FloatList),
	m_intList(new IntList),
	m_stdStringList(new StdStringList),
	m_unicodeStringList(new UnicodeStringList)
{
}

//-------------------------------------------------------------------

OptionManager::~OptionManager()
{
	clearSavedLists();

	delete m_registeredOptionList;
	delete m_savedOptionList;
	delete m_stringList;
	delete m_boolList;
	delete m_floatList;
	delete m_intList;
	delete m_stdStringList;
	delete m_unicodeStringList;
}

//-------------------------------------------------------------------

void OptionManager::load(char const * const fileName)
{
	if (!ms_optionManagersEnabled)
		return;

	if (TreeFile::exists(fileName))
	{
		if (Iff::isValid(fileName))
		{
			Iff iff;
			if (iff.open(fileName, true))
			{
				if (iff.getCurrentName() == TAG_OPTN)
				{
					load(iff);
				}
				else
				{
					REPORT_LOG(true, ("OptionManager::load: %s not a valid options Iff\n", fileName));
				}
			}
		}
		else
		{
			REPORT_LOG(true, ("OptionManager::load: %s not a valid Iff\n", fileName));
		}
	}
	else
	{
		REPORT_LOG(true, ("OptionManager::load: %s not found\n", fileName));
	}

	if (ConfigSharedUtility::getLogOptionManager())
	{
		OptionList::iterator iter = m_savedOptionList->begin();
		for (; iter != m_savedOptionList->end(); ++iter)
			iter->debugDump("load");
	}
}

//-------------------------------------------------------------------

void OptionManager::save(char const * const fileName)
{
	if (!ms_optionManagersEnabled)
		return;

	Iff iff(1024);
	save(iff);
	if (!iff.write(fileName, true))
		DEBUG_REPORT_LOG(true, ("OptionManager::save: could not write %s\n", fileName));

	if (ConfigSharedUtility::getLogOptionManager())
	{
		OptionList::iterator iter = m_registeredOptionList->begin();
		for (; iter != m_registeredOptionList->end(); ++iter)
			iter->debugDump("save");
	}
}

//----------------------------------------------------------------------

void OptionManager::clearSavedLists()
{
	m_savedOptionList->clear();

	std::for_each(m_stringList->begin(), m_stringList->end(), ArrayPointerDeleter());
	m_stringList->clear();

	std::for_each(m_boolList->begin(), m_boolList->end(), PointerDeleter());
	m_boolList->clear();

	std::for_each(m_floatList->begin(), m_floatList->end(), PointerDeleter());
	m_floatList->clear();

	std::for_each(m_intList->begin(), m_intList->end(), PointerDeleter());
	m_intList->clear();

	std::for_each(m_stdStringList->begin(), m_stdStringList->end(), PointerDeleter());
	m_stdStringList->clear();

	std::for_each(m_unicodeStringList->begin(), m_unicodeStringList->end(), PointerDeleter());
	m_unicodeStringList->clear();
}

//-------------------------------------------------------------------

void OptionManager::registerOption(bool & variable, char const * const section, char const * const name, const int version)
{
	Option option(Option::T_bool);
	option.m_bool = &variable;
	option.m_section = section;
	option.m_name = name;
	option.m_version = version;
	m_registeredOptionList->push_back(option);

	variable = findBool(section, name, variable, version);
}

//-------------------------------------------------------------------

void OptionManager::registerOption(float & variable, char const * const section, char const * const name, const int version)
{
	Option option(Option::T_float);
	option.m_float = &variable;
	option.m_section = section;
	option.m_name = name;
	option.m_version = version;
	m_registeredOptionList->push_back(option);

	variable = findFloat(section, name, variable, version);
}

//-------------------------------------------------------------------

void OptionManager::registerOption(int & variable, char const * const section, char const * const name, const int version)
{
	Option option(Option::T_int);
	option.m_int = &variable;
	option.m_section = section;
	option.m_name = name;
	option.m_version = version;
	m_registeredOptionList->push_back(option);

	variable = findInt(section, name, variable, version);
}

//----------------------------------------------------------------------

void OptionManager::registerOption(std::string & variable, char const * const section, char const * const name, const int version)
{
	Option option(Option::T_stdString);
	option.m_stdString = &variable;
	option.m_section = section;
	option.m_name = name;
	option.m_version = version;
	m_registeredOptionList->push_back(option);

	variable = findStdString(section, name, variable, version);
}

//----------------------------------------------------------------------

void OptionManager::registerOption(Unicode::String & variable, char const * const section, char const * const name, const int version)
{
	Option option(Option::T_unicodeString);
	option.m_unicodeString = &variable;
	option.m_section = section;
	option.m_name = name;
	option.m_version = version;
	m_registeredOptionList->push_back(option);

	variable = findUnicodeString(section, name, variable, version);
}

//===================================================================
// STATIC PRIVATE OptionManager
//===================================================================

bool OptionManager::findBool(char const * const section, char const * const name, bool const defaultValue, const int version)
{
	bool value = defaultValue;

	OptionList::iterator iter = m_savedOptionList->begin();
	for (; iter != m_savedOptionList->end(); ++iter)
	{
		const Option& option = *iter;
		if (section && (option.getType() == Option::T_bool && strcmp(section, option.m_section) == 0 && strcmp(name, option.m_name) == 0 && option.m_version == version))
		{
			value = *option.m_bool;
			break;
		}
	}

	if (section && name && ConfigFile::isInstalled())
		return ConfigFile::getKeyBool(section, name, value, true);

	return value;
}

//----------------------------------------------------------------------

float OptionManager::findFloat(char const * const section, char const * const name, float const defaultValue, const int version)
{
	float value = defaultValue;

	OptionList::iterator iter = m_savedOptionList->begin();
	for (; iter != m_savedOptionList->end(); ++iter)
	{
		const Option& option = *iter;
		if (section && (option.getType() == Option::T_float && strcmp(section, option.m_section) == 0 && strcmp(name, option.m_name) == 0 && option.m_version == version))
		{
			value = *option.m_float;
			break;
		}
	}

	if (section && name && ConfigFile::isInstalled())
		return ConfigFile::getKeyFloat(section, name, value, true);

	return value;
}

//----------------------------------------------------------------------

int OptionManager::findInt(char const * const section, char const * const name, int const defaultValue, const int version)
{
	int value = defaultValue;

	OptionList::iterator iter = m_savedOptionList->begin();
	for (; iter != m_savedOptionList->end(); ++iter)
	{
		const Option& option = *iter;
		if (section && (option.getType() == Option::T_int && strcmp(section, option.m_section) == 0 && strcmp(name, option.m_name) == 0 && option.m_version == version))
		{
			value = *option.m_int;
			break;
		}
	}

	if (section && name && ConfigFile::isInstalled())
		return ConfigFile::getKeyInt(section, name, value, true);

	return value;
}

//----------------------------------------------------------------------

std::string OptionManager::findStdString(char const * const section, char const * const name, std::string const & defaultValue, const int version)
{
	std::string value = defaultValue;

	OptionList::iterator iter = m_savedOptionList->begin();
	for (; iter != m_savedOptionList->end(); ++iter)
	{
		const Option& option = *iter;
		if (section && (option.getType() == Option::T_stdString && strcmp(section, option.m_section) == 0 && strcmp(name, option.m_name) == 0 && option.m_version == version))
		{
			NOT_NULL(option.m_stdString);
			value = *option.m_stdString;
			break;
		}
	}

	if (section && name && ConfigFile::isInstalled())
	{
		const char * const str = ConfigFile::getKeyString(section, name, value.c_str(), true);
		if (str)
			return std::string(str);
	}

	return value;
}

//----------------------------------------------------------------------

Unicode::String OptionManager::findUnicodeString(char const * const section, char const * const name, Unicode::String const & defaultValue, const int version)
{
	Unicode::String value = defaultValue;

	OptionList::iterator iter = m_savedOptionList->begin();
	for (; iter != m_savedOptionList->end(); ++iter)
	{
		const Option& option = *iter;
		if (option.getType() == Option::T_unicodeString && strcmp(section, option.m_section) == 0 && strcmp(name, option.m_name) == 0 && option.m_version == version)
		{
			NOT_NULL(option.m_unicodeString);
			value = *option.m_unicodeString;
			break;
		}
	}

	// No config file search for Unicode::String

	return value;
}

//----------------------------------------------------------------------

void OptionManager::load(Iff & iff)
{
	iff.enterForm(TAG_OPTN);

	int version = 0;

	switch (iff.getCurrentName())
	{
	case TAG_0000:
	case TAG_0001:
		WARNING(true, ("OptionManager::load: detected old version (below 0002), skipping load..."));
		break;

		// loading of version 0002 still supported in order to convert to version 0003 on save
	case TAG_0002:
		version = 2;
		break;

	case TAG_0003:
		version = 3;
		break;

	default:
	{
		char tagBuffer[5];
		ConvertTagToString(iff.getCurrentName(), tagBuffer);

		char buffer[128];
		iff.formatLocation(buffer, sizeof(buffer));
		DEBUG_FATAL(true, ("OptionManager::load - invalid version %s/%s", buffer, tagBuffer));
	}
	break;
	}

	if (version >= 2)
	{
		loadVersion(iff, version);
	}

	iff.exitForm(TAG_OPTN, true);
}

//-------------------------------------------------------------------

void OptionManager::save(Iff & iff)
{
	iff.insertForm(TAG_OPTN);

	iff.insertForm(TAG_0003);

	OptionList::iterator iter = m_registeredOptionList->begin();
	for (; iter != m_registeredOptionList->end(); ++iter)
	{
		const Option& option = *iter;

		switch (option.getType())
		{
		case Option::T_bool:
		{
			iff.insertChunk(TAG_BOOL);

			iff.insertChunkData(static_cast<int8> (*option.m_bool ? 1 : 0));
			iff.insertChunkString(option.m_section);
			iff.insertChunkString(option.m_name);
			iff.insertChunkData(option.m_version);

			iff.exitChunk(TAG_BOOL);
		}
		break;

		case Option::T_float:
		{
			iff.insertChunk(TAG_FLT);

			iff.insertChunkData(*option.m_float);
			iff.insertChunkString(option.m_section);
			iff.insertChunkString(option.m_name);
			iff.insertChunkData(option.m_version);

			iff.exitChunk(TAG_FLT);
		}
		break;

		case Option::T_int:
		{
			iff.insertChunk(TAG_INT);

			iff.insertChunkData(*option.m_int);
			iff.insertChunkString(option.m_section);
			iff.insertChunkString(option.m_name);
			iff.insertChunkData(option.m_version);

			iff.exitChunk(TAG_INT);
		}
		break;

		case Option::T_stdString:
		{
			iff.insertChunk(TAG_STDS);

			iff.insertChunkString(option.m_stdString->c_str());
			iff.insertChunkString(option.m_section);
			iff.insertChunkString(option.m_name);
			iff.insertChunkData(option.m_version);

			iff.exitChunk(TAG_STDS);
		}
		break;

		case Option::T_unicodeString:
		{
			iff.insertChunk(TAG_UNIS);

			iff.insertChunkString(*option.m_unicodeString);
			iff.insertChunkString(option.m_section);
			iff.insertChunkString(option.m_name);
			iff.insertChunkData(option.m_version);

			iff.exitChunk(TAG_UNIS);
		}
		break;

		default:
			break;
		}
	}

	iff.exitForm(TAG_0003);

	iff.exitForm(TAG_OPTN);
}

//-------------------------------------------------------------------

void OptionManager::loadVersion(Iff & iff, const int version)
{
	clearSavedLists();

	Tag versionTag = iff.getCurrentName();

	iff.enterForm(versionTag);

	while (iff.getNumberOfBlocksLeft())
	{
		switch (iff.getCurrentName())
		{
		case TAG_BOOL:
		{
			iff.enterChunk(TAG_BOOL);

			Option option(Option::T_bool);
			option.m_bool = new bool;
			*option.m_bool = iff.read_bool8();
			option.m_section = iff.read_string();
			option.m_name = iff.read_string();
			if (version > 2)
			{
				option.m_version = iff.read_int32();
			}
			m_savedOptionList->push_back(option);

			//-- save these for deleting later
			m_stringList->push_back(const_cast<char*> (option.m_section));
			m_stringList->push_back(const_cast<char*> (option.m_name));
			m_boolList->push_back(option.m_bool);

			iff.exitChunk(TAG_BOOL);
		}
		break;

		case TAG_FLT:
		{
			iff.enterChunk(TAG_FLT);

			Option option(Option::T_float);
			option.m_float = new float;
			*option.m_float = iff.read_float();
			option.m_section = iff.read_string();
			option.m_name = iff.read_string();
			if (version > 2)
			{
				option.m_version = iff.read_int32();
			}
			m_savedOptionList->push_back(option);

			//-- save these for deleting later
			m_stringList->push_back(const_cast<char*> (option.m_section));
			m_stringList->push_back(const_cast<char*> (option.m_name));
			m_floatList->push_back(option.m_float);

			iff.exitChunk(TAG_FLT);
		}
		break;

		case TAG_INT:
		{
			iff.enterChunk(TAG_INT);

			Option option(Option::T_int);
			option.m_int = new int;
			*option.m_int = iff.read_int32();
			option.m_section = iff.read_string();
			option.m_name = iff.read_string();
			if (version > 2)
			{
				option.m_version = iff.read_int32();
			}
			m_savedOptionList->push_back(option);

			//-- save these for deleting later
			m_stringList->push_back(const_cast<char*> (option.m_section));
			m_stringList->push_back(const_cast<char*> (option.m_name));
			m_intList->push_back(option.m_int);

			iff.exitChunk(TAG_INT);
		}
		break;

		case TAG_STDS:
		{
			iff.enterChunk(TAG_STDS);

			Option option(Option::T_stdString);
			option.m_stdString = new std::string;
			*option.m_stdString = iff.read_stdstring();
			option.m_section = iff.read_string();
			option.m_name = iff.read_string();
			if (version > 2)
			{
				option.m_version = iff.read_int32();
			}
			m_savedOptionList->push_back(option);

			//-- save these for deleting later
			m_stringList->push_back(const_cast<char*> (option.m_section));
			m_stringList->push_back(const_cast<char*> (option.m_name));
			m_stdStringList->push_back(option.m_stdString);

			iff.exitChunk(TAG_STDS);
		}
		break;

		case TAG_UNIS:
		{
			iff.enterChunk(TAG_UNIS);

			Option option(Option::T_unicodeString);
			option.m_unicodeString = new Unicode::String;
			*option.m_unicodeString = iff.read_unicodeString();
			option.m_section = iff.read_string();
			option.m_name = iff.read_string();
			if (version > 2)
			{
				option.m_version = iff.read_int32();
			}
			m_savedOptionList->push_back(option);

			//-- save these for deleting later
			m_stringList->push_back(const_cast<char*> (option.m_section));
			m_stringList->push_back(const_cast<char*> (option.m_name));
			m_unicodeStringList->push_back(option.m_unicodeString);

			iff.exitChunk(TAG_UNIS);
		}
		break;

		default:
		{
			iff.enterChunk();
			iff.exitChunk(true);
		}
		}
	}

	iff.exitForm(versionTag);

	//-- copy the loaded values into any registered values

	copyOptionListIntersection(*m_savedOptionList, *m_registeredOptionList);
}

//----------------------------------------------------------------------

	/*
	* Copy the values of options that are shared by both lists.
	* The size of dst is unchanged.
	*/

void OptionManager::copyOptionListIntersection(const OptionList & src, OptionList & dst)
{
	for (OptionList::const_iterator it = src.begin(); it != src.end(); ++it)
	{
		const Option & sopt = *it;

		for (OptionList::iterator rit = dst.begin(); rit != dst.end(); ++rit)
		{
			const Option & dopt = *rit;

			if (dopt.m_name    && sopt.m_name && !strcmp(dopt.m_name, sopt.m_name) &&
				dopt.m_section && sopt.m_section && !strcmp(dopt.m_section, sopt.m_section) &&
				dopt.m_type == sopt.m_type &&
				dopt.m_version == sopt.m_version) // default value takes precedence when version changes
			{
				switch (dopt.m_type)
				{
				case Option::T_bool:
					NOT_NULL(dopt.m_bool);
					NOT_NULL(sopt.m_bool);
					*dopt.m_bool = *sopt.m_bool;
					break;
				case Option::T_float:
					NOT_NULL(dopt.m_float);
					NOT_NULL(sopt.m_float);
					*dopt.m_float = *sopt.m_float;
					break;
				case Option::T_int:
					NOT_NULL(dopt.m_int);
					NOT_NULL(sopt.m_int);
					*dopt.m_int = *sopt.m_int;
					break;
				case Option::T_stdString:
					NOT_NULL(dopt.m_stdString);
					NOT_NULL(sopt.m_stdString);
					*dopt.m_stdString = *sopt.m_stdString;
					break;
				case Option::T_unicodeString:
					NOT_NULL(dopt.m_unicodeString);
					NOT_NULL(sopt.m_unicodeString);
					*dopt.m_unicodeString = *sopt.m_unicodeString;
					break;
				}
			}
		}
	}
}
//===================================================================