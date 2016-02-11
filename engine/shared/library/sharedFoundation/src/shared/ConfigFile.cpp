// ======================================================================
//
// ConfigFile.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ConfigFile.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/StringCompare.h"
#include "sharedFoundation/Os.h"

#include <cstdlib>

#include <list>
#include <map>
#include <string>
#include <algorithm> //for find

#define DO_CONFIG_FILE_TEMPLATE_CREATE 0

#if DO_CONFIG_FILE_TEMPLATE_CREATE

static char tagBuffer[6];

#define CONFIG_FILE_TEMPLATE_CREATE_INT(section, key, def)    DEBUG_REPORT_LOG(true, ("cfg: [%s] %s=%d\n", section, key, def))
#define CONFIG_FILE_TEMPLATE_CREATE_BOOL(section, key, def)   DEBUG_REPORT_LOG(true, ("cfg: [%s] %s=%s\n", section, key, def ? "true" : "false"))
#define CONFIG_FILE_TEMPLATE_CREATE_FLOAT(section, key, def)  DEBUG_REPORT_LOG(true, ("cfg: [%s] %s=%3.1f\n", section, key, def))
#define CONFIG_FILE_TEMPLATE_CREATE_STRING(section, key, def) DEBUG_REPORT_LOG(true, ("cfg: [%s] %s=%s\n", section, key, def ? def : "nullptr"))
#define CONFIG_FILE_TEMPLATE_CREATE_TAG(section, key, def)    ConvertTagToString(def, tagBuffer), DEBUG_REPORT_LOG(true, ("cfg: [%s] %s=%d\n", section, key, tagBuffer))

#else

#define CONFIG_FILE_TEMPLATE_CREATE_INT(section, key, def)    NOP
#define CONFIG_FILE_TEMPLATE_CREATE_BOOL(section, key, def)   NOP
#define CONFIG_FILE_TEMPLATE_CREATE_FLOAT(section, key, def)  NOP
#define CONFIG_FILE_TEMPLATE_CREATE_STRING(section, key, def) NOP
#define CONFIG_FILE_TEMPLATE_CREATE_TAG(section, key, def)    NOP

#endif

#define NO_SPACE_SECTION(a) DEBUG_FATAL(strchr(a, ' ') != nullptr, ("No spaces are allowed in config file section names: %s", a))
#define NO_SPACE_KEY(a)     DEBUG_FATAL(strchr(a, ' ') != nullptr, ("No spaces are allowed in config file key names: %s", a))

// ======================================================================

/// Pointer to the current section the parser is in
ConfigFile::Section    *ConfigFile::ms_currentSection;

/// Pointer to a std::<map> of Sections
ConfigFile::SectionMap *ConfigFile::ms_sections;

/// Delimiter string used for include files
const char             *ConfigFile::ms_IncludeDelimiter = ".include ";

bool                    ConfigFile::ms_installed = false;

bool ConfigFile::ms_logConfigSettings = false;

// ======================================================================
/** Install the command-line processor
 *
 *  This routine adds ConfigFile::remove() to the ExitChain.
 *
 *  @see remove()
 */
void ConfigFile::install(void)
{
#if DO_CONFIG_FILE_TEMPLATE_CREATE
	static bool warned = false;
	DEBUG_WARNING(!warned, ("ConfigFile enabled DO_CONFIG_FILE_TEMPLATE_CREATE"));
	warned = true;
#endif

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_logConfigSettings, "SharedFoundation", "logConfigSettings", dump);
#endif

	ms_sections = new ConfigFile::SectionMap;
	ms_currentSection = nullptr;
	ExitChain::add(ConfigFile::remove, "ConfigFile::remove");
	ms_installed = true;
}

// ----------------------------------------------------------------------
/**
 * Check if the config file system is installed.
 *
 * @return true if it is installed, otherwise false.
 */

bool ConfigFile::isInstalled()
{
	return ms_installed;
}

// ----------------------------------------------------------------------
/** Remove the command-line processor
 *
 *
 *  This routine should not be called by the game, but only by the ExitChain.
 *  @see install()
 */
void ConfigFile::remove(void)
{
	for (std::map<const char *, Section *, StringCompare>::iterator it = ms_sections->begin(); it != ms_sections->end(); ++it)
		delete it->second;
	delete ms_sections;
	ms_sections = nullptr;
	ms_installed = false;
}

// ----------------------------------------------------------------------
/**
 * Check if the ConfigFile is completely empty.
 */

bool ConfigFile::isEmpty()
{
	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	return ms_sections->empty();
}

// ----------------------------------------------------------------------
/** Load and parse the given command line buffer into the config file system
 *
 *  This function breaks up the buffer into files and keys and calls the appropriate
 *  functions on them
 *  @param buffer the buffer
 *  @return success or failure
 */
bool ConfigFile::loadFromCommandLine(const char *buffer)
{
	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	while (*buffer)
	{
		//trim off leading whitespace
		while (isspace(*buffer))
		{
			++buffer;
			// advancing buffer, check for nullptr (trailing white space on command line)
			if(! *buffer)
				break;
		}

		if(! *buffer)
			break;

		//option 1: a file name, delimited by a "@"
		if (*buffer == '@')
		{
			const char* fileBegin = buffer+1;
			const char* fileEnd = fileBegin;
			while (*fileEnd && !isspace(*fileEnd))
			{
				++fileEnd;
			}
			const int fileLen = fileEnd - fileBegin;
			char *file = new char[static_cast<unsigned int>(fileLen+1)];
			memcpy(file, fileBegin, static_cast<unsigned int>(fileLen));
			file[fileLen] = '\0';
			IGNORE_RETURN(loadFile(file));
			delete[] file;
			buffer = fileEnd;
		}

		//option 2: a section name, delimited by a "-s "
		else if (*buffer == '-' && *(buffer+1) == 's' && *(buffer+2) == ' ')
		{
			//find the section name in the buffer
			const char* sectionBegin = buffer+3;
			const char* sectionEnd = sectionBegin;
			while (*sectionEnd &&!isspace(*sectionEnd))
			{
				++sectionEnd;
			}
			//make a string out of it
			const int sectionLen = sectionEnd - sectionBegin;
			char *sectionName = new char[static_cast<unsigned int>(sectionLen+1)];
			memcpy(sectionName, sectionBegin, static_cast<unsigned int>(sectionLen));
			sectionName[sectionLen] = '\0';

			//try to get an existing section, otherwise add one
			Section *section = getSection(sectionName);
			if (section)
			{
				//put this back as the current section
				ms_currentSection = section;
			}
			else
			{
				//allocate a new section and set it as the current one
				Section *newSection = createSection(sectionName);
				//note that newSection is memory owned by the created Section object
				//while sectionName is memory allocated to create a nullptr terminated string
				ms_currentSection = newSection;
			}
			delete[] sectionName;
			buffer = sectionEnd;
		}

		//option 3: a key/value pair
		else
		{
			//find the key name in the buffer
			const char *keyBegin = buffer;
			const char *keyEnd = strchr(keyBegin, '=');
			NOT_NULL(keyEnd);
			//make a string out of it
			const int keyLen = keyEnd - keyBegin;
			char *keyName = new char[static_cast<unsigned int>(keyLen+1)];
			memcpy(keyName, keyBegin, static_cast<unsigned int>(keyLen));
			keyName[keyLen] = '\0';

			//find the value name in the buffer
			const char *valueBegin = keyEnd + 1; //+1 to skip the equals sign

			const bool quoted = (*valueBegin == '"');
			if (quoted)
				++valueBegin;

			const char *valueEnd = valueBegin;
			while (*valueEnd)
			{
				if (quoted && *valueEnd == '"')
					break;
				else if (!quoted && isspace(*valueEnd))
					break;

				++valueEnd;
			}
			//make a string out of it
			const int valueLen = valueEnd - valueBegin;

			//-- skip the trailing quote
			if (quoted && *valueEnd == '"')
				++valueEnd;

			char *valueName = new char[static_cast<unsigned int>(valueLen+1)];
			memcpy(valueName, valueBegin, static_cast<unsigned int>(valueLen));
			valueName[valueLen] = '\0';

			Key *key = ms_currentSection->findKey(keyName);
			if (key)
			{
				key->addValue(valueName);
			}
			else
			{
				ms_currentSection->addKey(keyName, valueName);
			}
			
			//add the key to the ConfigFile (you'd better be in a section)
			//ms_currentSection->addKey(keyName, valueName);

			delete[] keyName;
			delete[] valueName;
			buffer = valueEnd;
		}
	}
	return true;
}

// ----------------------------------------------------------------------

bool ConfigFile::loadFromBuffer(char const * const buffer, int const length)
{
	//allocate here to prevent reallocation per loop
	char const * bufferPosition = buffer;
	char *currentLine;
	char const *lineEnd;
	int lineLength;
	while(bufferPosition < buffer + length && *bufferPosition)
	{
		//trim leading whitespace
		while (*bufferPosition && isspace(*bufferPosition))
			++bufferPosition;
		//build and process the next line
		lineEnd = strchr(bufferPosition, '\n');

		if (!lineEnd)
			lineEnd = buffer + length - 1;

		lineLength = lineEnd - bufferPosition;
		currentLine = new char[static_cast<unsigned int>(lineLength+1)];
		strncpy(currentLine, bufferPosition, static_cast<unsigned int>(lineLength));
		currentLine[lineLength] = '\0';
		processLine(currentLine);
		delete[] currentLine;
		bufferPosition += lineLength+1;
		//trim ending whitespace.  Remember that this string is not guaranteed to be nullptr terminated.
		while (*bufferPosition && (bufferPosition < buffer + length) && isspace(static_cast<unsigned char>(*bufferPosition)))
			++bufferPosition;
	}

	return true;
}

// ----------------------------------------------------------------------
/** Load and parse the given file into the config file system
 *
 *  This function may be called recursively if includes are used
 *  @param file the filename string
 *  @return success or failure
 */
bool ConfigFile::loadFile(const char *file)
{
	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	NOT_NULL(file);
	ms_currentSection = nullptr;
	HANDLE handle = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (handle != INVALID_HANDLE_VALUE)
	{
		// get the length of the config file
		const DWORD length = GetFileSize(handle, nullptr);

		if (length != 0xffffffff)
		{
			// create a buffer to load the config file into
			char * const buffer = new char[length+2];

			// make sure the buffer is nullptr terminated
			buffer[length] = '\n';
			buffer[length+1] = '\0';

			// read the config file in
			DWORD readResult;
			const BOOL result = ReadFile(handle, buffer, length, &readResult, nullptr);

			// make sure we read all the correct stuff
			if (result && readResult == length)
				IGNORE_RETURN(loadFromBuffer(buffer, length + 2));

			delete [] buffer;
		}
		// close the file
		static_cast<void>(CloseHandle(handle));
		return true;
	}
	else
	{
		char *lastError = Os::getLastError();
		WARNING (true, ("ConfigFile::loadFile %s: %s", file, lastError));
		delete [] lastError;
		return false;
	}
}

// ----------------------------------------------------------------------
/** Load and parse the given line into the config file system
 *
 *  @param line the line string
 */
void ConfigFile::processLine(const char *line)
{
	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	// validate argument
	DEBUG_FATAL(!line, ("ConfigFile::processLine nullptr"));

	// check for a full line comment
	if (*line == '#' || *line == ';')
		return;

	//check for .include statement (an optional trailing comment is ignored)
	if (line == strstr(line, ms_IncludeDelimiter))
	{
		//advance past the .include
		const char *fileName = line + strlen(ms_IncludeDelimiter);
		if (fileName[0] == '\"')
		{
			//find end of file name
			const char* endQuote = strchr(++fileName, '\"');
			NOT_NULL(endQuote);
			const int len = endQuote - fileName;
			//allocate mem to hold filename
			char *fileStr = new char[static_cast<unsigned int>(len+1)];
			memcpy(fileStr, fileName, static_cast<unsigned int>(len));
			fileStr[len] = '\0';
			//save off the current section, so that we return to it afterwards
			Section *thisSection = ms_currentSection;
			if (!loadFile(fileStr))
				WARNING (true, ("ConfigFile::processLine unable to load included file %s", fileStr));

			//return to current section "scope"
			ms_currentSection = thisSection;
			delete[] fileStr;
		}
		return;
	}

	//look for a section declaration
	// check for a comment
	if (*line == '[')
	{
		const char *beginSection = ++line;
		//find end of section name
		const char* endSection = strchr(line, ']');
		NOT_NULL(endSection);
		const int len = endSection - line;
		//allocate mem to hold section name
		char *sectionName = new char[static_cast<unsigned int>(len+1)];
		memcpy(sectionName, beginSection, static_cast<unsigned int>(len));
		sectionName[len] = '\0';
		//see if we already have this section
		Section *section = getSection(sectionName);
		if (section)
		{
			//put this back as the current section
			ms_currentSection = section;
		}
		else
		{
			//allocate a new section and set it as the current one
			Section *newSection = createSection(sectionName);
			//note that newSection is memory owned by the created Section object
			//while sectionName is memory allocated to create a nullptr terminated string
			ms_currentSection = newSection;
		}
		delete[] sectionName;
		return;
	}
	//if we get here, we should have a 1 or > key assignment(s) with an optional comment on the end
	processKeys(line);
}

// ----------------------------------------------------------------------
/** Load and parse the given string into key/value pairs and enter them into the config file system
 *
 *  @param line a string of one or more key/value pairs (i.e. a=6 & b =  7)
 */
void ConfigFile::processKeys(const char *line)
{
	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));

	if (!*line)
		return;

	DEBUG_FATAL(!ms_currentSection, ("ConfigFile::processKey key assigned outside a section"));
	//build the key name
	while (isspace(*line))
		++line;
	const char *beginKey = line;
	//find end of key name
	const char* endKey = strchr(line, '=');
	WARNING_DEBUG_FATAL(!endKey, ("Invalid line in config file. Section = [%s]", ms_currentSection->getName()));
	if (!endKey)
		return;

	const char *beginValue = endKey +1;
	//remove '=' from keyname
	--endKey;
	//trim trailing whitespace from before key name
	while (isspace(*endKey))
		--endKey;
	const int len = endKey - line + 1;

	//allocate mem to hold keyname
	char *keyName = new char[static_cast<unsigned int>(len+1)];
	memcpy(keyName, beginKey, static_cast<unsigned int>(len));
	keyName[len] = '\0';

	const char *endValue = beginValue;
	while (*endValue)
	{
		//build the value
		char *value = nullptr;
		while(isspace(*beginValue))
			++beginValue;
		if (*beginValue != '"')
		{
			//find the end of the key value
			endValue = beginValue;
			while(*endValue &&
					!isspace(*endValue) &&
					(*endValue) != '#' &&
					(*endValue) != '\n' &&
					(*endValue) != '&' &&
					(*endValue) != ';')
				++endValue;
			int valueLength = endValue - beginValue;
			value = new char[static_cast<unsigned int>(valueLength+1)];
			memcpy(value, beginValue, static_cast<unsigned int>(valueLength));
			value[valueLength] = '\0';
		}
		else
		{
			// quoted string: go until we get a matching quote or eol
			endValue = beginValue + 1;
			while(*endValue &&
				  *endValue != '"' &&
				  *endValue != '\n')
				++endValue;
			++beginValue;
			int valueLength = endValue - beginValue;
			value = new char[static_cast<unsigned int>(valueLength+1)];
			if (valueLength > 0)
				memcpy(value, beginValue, static_cast<unsigned int>(valueLength));
			value[valueLength] = '\0';
			if (*endValue == '"')
				++endValue;
		}

		//see if we already have this key
		Key *key = ms_currentSection->findKey(keyName);
		if (key)
		{
			key->addValue(value);
		}
		else
		{
			ms_currentSection->addKey(keyName, value);
		}
		delete[] value;

		//advance and look for another key or value on this line
		while ((*endValue) && isspace(*endValue))
			++endValue;
		if (*endValue == '&' || *endValue == '\0' || *endValue == ';' || *endValue == '#')
		{
			if (*endValue == '&')
				processKeys(++endValue);
			// move endValue to the end of the line
			while (*endValue)
				++endValue;
			delete[] keyName;
			keyName = 0;
		}
		else
			beginValue = endValue;
	}

	delete[] keyName;
	keyName = 0;
}

// ----------------------------------------------------------------------
/** Get a Section pointer from a string
 *
 *  @param section the name of the section
 *  @return the pointer if valid, otherwise nullptr
*/
ConfigFile::Section *ConfigFile::getSection(const char *section)
{
	std::map<const char *, Section *, StringCompare>::const_iterator it = ms_sections->find(section);
	if (it != ms_sections->end())
		return it->second;

	return nullptr;
}

// ----------------------------------------------------------------------
/** Create a new section
 *
 *  @param section the name of the section
 *  @return a pointer to the newly created Section
 */
ConfigFile::Section *ConfigFile::createSection(const char *name)
{
	Section *newSection = new Section(name);
	(*ms_sections)[newSection->getName()] = newSection;
	return newSection;
}

// ----------------------------------------------------------------------
/** Removes a section
 *
 *  @param section the name of the section
 */
void ConfigFile::removeSection(const char *name)
{
	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	NOT_NULL(name);
	NOT_NULL(ms_sections);

	SectionMap::iterator iter = ms_sections->find(name);
	if (iter != ms_sections->end())
	{
		delete iter->second;
		iter->second = nullptr;
		ms_sections->erase(iter);
	}
}

// ----------------------------------------------------------------------
/** Print all the stored config data to stdout
 *
 */
void ConfigFile::dump(void)
{
#ifdef _DEBUG
	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	for (std::map<const char *, Section *, StringCompare>::iterator it = ms_sections->begin(); it != ms_sections->end(); ++it)
	{
		const char *name = it->first;
		DEBUG_REPORT_LOG_PRINT(true, ("[%s]\n", name  ));
		it->second->dump();
	}
	ms_logConfigSettings = false;
#endif
}

// ----------------------------------------------------------------------
/** Gets a value as an int
 *
 *  Since access to this function is usually macroed, provide direct access
 *
 *  @param section the section name string
 *  @param key the key name string
 *  @param index of the entry (since a key may hold > 1 value)
 *  @param defaultValue the value to return if the section, key, or index aren't valid
 *  @return the value
 */
int ConfigFile::getKeyInt(const char *section, const char *key, int index, int defaultValue)
{
	NO_SPACE_SECTION(section);
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_INT(section, key, defaultValue);

	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	//validate arguments
	NOT_NULL(section);
	NOT_NULL(key);
	Section *s = getSection(section);
	if (s)
		return s->getKeyInt(key, index, defaultValue);

	return defaultValue;
}

// ----------------------------------------------------------------------
/** Gets a value as a bool
 *
 *  Since access to this function is usually macroed, provide direct access
 *
 *  @param section the section name string
 *  @param key the key name string
 *  @param index of the entry (since a key may hold > 1 value)
 *  @param defaultValue the value to return if the section, key, or index aren't valid
 *  @return the value
 */
bool ConfigFile::getKeyBool(const char *section, const char *key, int index, bool defaultValue)
{
	NO_SPACE_SECTION(section);
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_BOOL(section, key, defaultValue);

	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	//validate arguments
	NOT_NULL(section);
	NOT_NULL(key);
	Section *s = getSection(section);
	if (s)
		return s->getKeyBool(key, index, defaultValue);

	return defaultValue;
}

// ----------------------------------------------------------------------
/** Gets a value as an float
 *
 *  Since access to this function is usually macroed, provide direct access
 *
 *  @param section the section name string
 *  @param key the key name string
 *  @param index of the entry (since a key may hold > 1 value)
 *  @param defaultValue the value to return if the section, key, or index aren't valid
 *  @return the value
 */
float ConfigFile::getKeyFloat(const char *section, const char *key, int index, float defaultValue)
{
	NO_SPACE_SECTION(section);
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_FLOAT(section, key, defaultValue);

	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	//validate arguments
	NOT_NULL(section);
	NOT_NULL(key);
	Section *s = getSection(section);
	if (s)
		return s->getKeyFloat(key, index, defaultValue);

	return defaultValue;
}

// ----------------------------------------------------------------------
/** Gets a value as a string (const char *)
 *
 *  Since access to this function is usually macroed, provide direct access
 *
 *  @param section the section name string
 *  @param key the key name string
 *  @param index of the entry (since a key may hold > 1 value)
 *  @param defaultValue the value to return if the section, key, or index aren't valid
 *  @return the value
 */
const char *ConfigFile::getKeyString(const char *section, const char *key, int index, const char *defaultValue)
{
	NO_SPACE_SECTION(section);
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_STRING(section, key, defaultValue);

	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	//validate arguments
	NOT_NULL(section);
	NOT_NULL(key);
	Section *s = getSection(section);
	if (s)
		return s->getKeyString(key, index, defaultValue);

	return defaultValue;
}

// ----------------------------------------------------------------------
/** Gets a value as a Tag
 *
 *  Since access to this function is usually macroed, provide direct access
 *
 *  @param section the section name string
 *  @param key the key name string
 *  @param index of the entry (since a key may hold > 1 value)
 *  @param defaultValue the value to return if the section, key, or index aren't valid
 *  @return the value
 */

Tag ConfigFile::getKeyTag(const char *section, const char *key, int index, Tag defaultValue)
{
	NO_SPACE_SECTION(section);
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_TAG(section, key, defaultValue);

	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	//validate arguments
	NOT_NULL(section);
	NOT_NULL(key);
	Section *s = getSection(section);
	if (s)
		return s->getKeyTag(key, index, defaultValue);

	return defaultValue;
}

// ----------------------------------------------------------------------
/** Gets a value as an int
 *  This function returns the *last* key in the list.  This should be the item entered last
 *  into the system (after basic includes, etc.)
 *
 *  Since access to this function is usually macroed, provide direct access
 *
 *  @param section the section name string
 *  @param key the key name string
 *  @param defaultValue the value to return if the section, or key isn't valid
 *  @param overrideLazyAdd if true, ignores and replaces an existing key that was added lazily. If false, honors any previously-installed value
 *  @return the value
 */
int ConfigFile::getKeyInt(const char *section, const char *key, int defaultValue, bool overrideLazyAdds)
{
	NO_SPACE_SECTION(section);
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_INT(section, key, defaultValue);

	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	//validate arguments
	NOT_NULL(section);
	NOT_NULL(key);
	Section *s = getSection(section);
	if(!s)
	{
		s = createSection(section);
	}

	Key *k = s->findKey(key);

	if (k && k->getLazyAdd() && overrideLazyAdds)
	{
		s->removeKey(key);
		k = 0;
	}

	if(!k)
	{
		s->addKey(key, FormattedString<32>().sprintf("%i", defaultValue), !overrideLazyAdds);
		return defaultValue;
	}

	return s->getKeyInt(key, k->getCount()-1, defaultValue);
}

// ----------------------------------------------------------------------
/** Gets a value as a bool
 *  This function returns the *last* key in the list.  This should be the item entered last
 *  into the system (after basic includes, etc.)
 *
 *  Since access to this function is usually macroed, provide direct access
 *
 *  @param section the section name string
 *  @param key the key name string
 *  @param defaultValue the value to return if the section, or key isn't valid
 *  @param overrideLazyAdd if true, ignores and replaces an existing key that was added lazily. If false, honors any previously-installed value
 *  @return the value
 */
bool ConfigFile::getKeyBool(const char *section, const char *key, bool defaultValue, bool overrideLazyAdds)
{
	NO_SPACE_SECTION(section);
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_BOOL(section, key, defaultValue);

	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	//validate arguments
	NOT_NULL(section);
	NOT_NULL(key);
	Section *s = getSection(section);
	if(!s)
	{
		s = createSection(section);
	}

	Key *k = s->findKey(key);

	if (k && k->getLazyAdd() && overrideLazyAdds)
	{
		s->removeKey(key);
		k = 0;
	}

	if(!k)
	{
		s->addKey(key, FormattedString<32>().sprintf("%s", defaultValue ? "1" : "0"), !overrideLazyAdds);
		return defaultValue;
	}

	return s->getKeyBool(key, k->getCount()-1, defaultValue);
}

// ----------------------------------------------------------------------
/** Gets a value as a float
 *  This function returns the *last* key in the list.  This should be the item entered last
 *  into the system (after basic includes, etc.)
 *
 *  Since access to this function is usually macroed, provide direct access
 *
 *  @param section the section name string
 *  @param key the key name string
 *  @param defaultValue the value to return if the section, or key isn't valid
 *  @param overrideLazyAdd if true, ignores and replaces an existing key that was added lazily. If false, honors any previously-installed value
 *  @return the value
 */
float ConfigFile::getKeyFloat(const char *section, const char *key, float defaultValue, bool overrideLazyAdds)
{
	NO_SPACE_SECTION(section);
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_FLOAT(section, key, defaultValue);

	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	//validate arguments
	NOT_NULL(section);
	NOT_NULL(key);
	Section *s = getSection(section);
	if(!s)
	{
		s = createSection(section);
	}

	Key *k = s->findKey(key);

	if (k && k->getLazyAdd() && overrideLazyAdds)
	{
		s->removeKey(key);
		k = 0;
	}

	if(!k)
	{
		s->addKey(key, FormattedString<32>().sprintf("%f", defaultValue), !overrideLazyAdds);
		return defaultValue;
	}

	return s->getKeyFloat(key, k->getCount()-1, defaultValue);
}

// ----------------------------------------------------------------------
/** Gets a value as a string (const char*).
 *  This function returns the *last* key in the list.  This should be the item entered last
 *  into the system (after basic includes, etc.)
 *
 *  Since access to this function is usually macroed, provide direct access
 *
 *  @param section the section name string
 *  @param key the key name string
 *  @param defaultValue the value to return if the section, or key isn't valid
 *  @param overrideLazyAdd if true, ignores and replaces an existing key that was added lazily. If false, honors any previously-installed value
 *  @return the value
 */
const char *ConfigFile::getKeyString(const char *section, const char *key, const char *defaultValue, bool overrideLazyAdds)
{
	NO_SPACE_SECTION(section);
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_STRING(section, key, defaultValue);

	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	//validate arguments
	NOT_NULL(section);
	NOT_NULL(key);
	Section *s = getSection(section);
	if(!s)
		return defaultValue;
	Key *k = s->findKey(key);

	if (k && k->getLazyAdd() && overrideLazyAdds)
	{
		s->removeKey(key);
		k = 0;
	}

	if(!k)
	{
		if (defaultValue != 0)
			s->addKey(key, defaultValue, !overrideLazyAdds);
		return defaultValue;
	}

	return s->getKeyString(key, k->getCount()-1, defaultValue);
}

// ----------------------------------------------------------------------
/** Gets a value as a string (const char*).
 *  This function returns the *last* key in the list.  This should be the item entered last
 *  into the system (after basic includes, etc.)
 *
 *  Since access to this function is usually macroed, provide direct access
 *
 *  @param section the section name string
 *  @param key the key name string
 *  @param defaultValue the value to return if the section, or key isn't valid
 *  @param overrideLazyAdd if true, ignores and replaces an existing key that was added lazily. If false, honors any previously-installed value
 *  @return the value
 */

Tag ConfigFile::getKeyTag(const char *section, const char *key, Tag defaultValue, bool overrideLazyAdds)
{
	NO_SPACE_SECTION(section);
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_TAG(section, key, defaultValue);

	DEBUG_FATAL(!ms_installed, ("ConfigFile not installed"));
	//validate arguments
	NOT_NULL(section);
	NOT_NULL(key);
	Section *s = getSection(section);
	if(!s)
	{
		s = createSection(section);
	}

	Key *k = s->findKey(key);

	if (k && k->getLazyAdd() && overrideLazyAdds)
	{
		s->removeKey(key);
		k = 0;
	}

	if(!k)
	{
		char buffer[6];
		ConvertTagToString(defaultValue, buffer);
		s->addKey(key, buffer, !overrideLazyAdds);
		return defaultValue;
	}

	return s->getKeyTag(key, k->getCount()-1, defaultValue);
}

// ======================================================================
//ConfigFile::Element

ConfigFile::Element::Element(void)
:
	m_entry(nullptr)
{}

// ----------------------------------------------------------------------

ConfigFile::Element::Element(const char *entry)
:
	m_entry(DuplicateString(entry))
{}

// ----------------------------------------------------------------------

ConfigFile::Element::~Element(void)
{
	delete[] m_entry;
}

// ----------------------------------------------------------------------

ConfigFile::Element::Element(const Element &element)
:
	m_entry(element.m_entry)
{}

// ----------------------------------------------------------------------

ConfigFile::Element &ConfigFile::Element::operator =(const Element &element)
{
	if (this != &element)
	{
		delete[] m_entry;
		m_entry = DuplicateString(element.m_entry);
	}
	return *this;
}

// ----------------------------------------------------------------------

int ConfigFile::Element::getAsInt(void) const
{
	DEBUG_FATAL(!m_entry, ("ConfigFile::getAsIntno data"));

	if (m_entry[0] == '0' && m_entry[1] == 'x')
	{
		int result = 0;
		for (const char *s = m_entry + 2; *s; ++s)
		{
			const int c = tolower(*s);

			if (c >= '0' && c <= '9')
				result = (result << 4) | (c - '0');
			else
				if (c >= 'a' && c <= 'f')
					result = (result << 4) | (c - 'a' + 10);
				else
					break;
		}

		return result;
	}

	return atoi(m_entry);
}

// ----------------------------------------------------------------------

bool ConfigFile::Element::getAsBool(void) const
{
	DEBUG_FATAL(!m_entry, ("ConfigFile::getAsBool no data"));

	if (_stricmp(m_entry, "true") == 0)
		return true;

	if (_stricmp(m_entry, "false") == 0)
		return false;

	if (_stricmp(m_entry, "yes") == 0)
		return true;

	if (_stricmp(m_entry, "no") == 0)
		return false;

	return getAsInt() != 0;
}

// ----------------------------------------------------------------------

float ConfigFile::Element::getAsFloat(void) const
{
	DEBUG_FATAL(!m_entry, ("ConfigFile::getAsFloatno data"));
	return static_cast<float>(atof(m_entry));
}

// ----------------------------------------------------------------------

const char *ConfigFile::Element::getAsString(void) const
{
	DEBUG_FATAL(!m_entry, ("ConfigFile::getAsString no data"));
	return m_entry;
}

// ----------------------------------------------------------------------

Tag ConfigFile::Element::getAsTag(void) const
{
	DEBUG_FATAL(!m_entry, ("ConfigFile::getAsTag no data"));
	return ConvertStringToTag(m_entry);
}

// ======================================================================
//ConfigFile::Key

ConfigFile::Key::Key(const char *name, const char *value, bool lazyAdd)
:
	m_elements(nullptr),
	m_name(nullptr),
	m_lazyAdd(lazyAdd)
{
	m_name = new char[strlen(name)+1];
	strcpy(m_name, name);
	m_elements = new ElementList;
	addValue(value);
}

// ----------------------------------------------------------------------

ConfigFile::Key::~Key(void)
{
	if(m_elements)
	{
		for (std::list<Element *>::iterator it = m_elements->begin(); it != m_elements->end(); ++it)
			delete (*it);
		delete m_elements;
	}
	delete[] m_name;
}

// ----------------------------------------------------------------------

void ConfigFile::Key::addValue(const char *value)
{
	Element *element = new Element(value);
	m_elements->push_back(element);
}                                                   //lint !e429 custodial pointer not freed or returned, we store it in a list we later clean out, so ignore this

// ----------------------------------------------------------------------

const char *ConfigFile::Key::getName(void) const
{
	return m_name;
}

// ----------------------------------------------------------------------

int ConfigFile::Key::getCount(void) const
{
	return static_cast<int>(m_elements->size());
}

// ----------------------------------------------------------------------

bool ConfigFile::Key::getLazyAdd(void) const
{
	return m_lazyAdd;
}

// ----------------------------------------------------------------------

int ConfigFile::Key::getAsInt(int index, int defaultValue) const
{
	DEBUG_FATAL(index < 0, ("ConfigFile::Key::getAsInt index < 0"));
	int i = 0;
	for (std::list<Element *>::const_iterator it = m_elements->begin(); it != m_elements->end() && i <= index; ++it)
	{
		if (i == index)
			return (*it)->getAsInt();
		++i;
	}
	return defaultValue;
}

// ----------------------------------------------------------------------

bool ConfigFile::Key::getAsBool(int index, bool defaultValue) const
{
	DEBUG_FATAL(index < 0, ("ConfigFile::Key::getAsInt index < 0"));
	int i = 0;
	for (std::list<Element *>::const_iterator it = m_elements->begin(); it != m_elements->end() && i <= index; ++it)
	{
		if (i == index)
			return (*it)->getAsBool();
		++i;
	}
	return defaultValue;
}

// ----------------------------------------------------------------------

float ConfigFile::Key::getAsFloat(int index, float defaultValue) const
{
	DEBUG_FATAL(index < 0, ("ConfigFile::Key::getAsInt index < 0"));
	int i = 0;
	for (std::list<Element *>::const_iterator it = m_elements->begin(); it != m_elements->end() && i <= index; ++it)
	{
		if (i == index)
			return (*it)->getAsFloat();
		++i;
	}
	return defaultValue;
}

// ----------------------------------------------------------------------

const char *ConfigFile::Key::getAsString(int index, const char *defaultValue) const
{
	DEBUG_FATAL(index < 0, ("ConfigFile::Key::getAsInt index < 0"));
	int i = 0;
	for (std::list<Element *>::const_iterator it = m_elements->begin(); it != m_elements->end() && i <= index; ++it)
	{
		if (i == index)
			return (*it)->getAsString();
		++i;
	}
	return defaultValue;
}

// ----------------------------------------------------------------------

Tag ConfigFile::Key::getAsTag(int index, Tag defaultValue) const
{
	DEBUG_FATAL(index < 0, ("ConfigFile::Key::getAsInt index < 0"));
	int i = 0;
	for (std::list<Element *>::const_iterator it = m_elements->begin(); it != m_elements->end() && i <= index; ++it)
	{
		if (i == index)
			return (*it)->getAsTag();
		++i;
	}
	return defaultValue;
}

// ----------------------------------------------------------------------

void ConfigFile::Key::dump(const char *keyName) const
{
	UNREF(keyName);
#ifdef _DEBUG
	for (std::list<Element *>::const_iterator it = m_elements->begin(); it != m_elements->end(); ++it)
		DEBUG_REPORT_LOG_PRINT(true, ("\t%s=%s\n", keyName, (*it)->getAsString()));
#endif
}

// ======================================================================
//ConfigFile::Section

ConfigFile::Section::Section(const char *name)
:
	m_keys(nullptr),
	m_name(nullptr)
{
	m_name = new char[strlen(name)+1];
	strcpy(m_name, name);
	m_keys = new KeyMap;
}

// ----------------------------------------------------------------------

ConfigFile::Section::~Section(void)
{
	if(m_keys)
	{
		for (KeyMap::iterator it = m_keys->begin(); it != m_keys->end(); ++it)
			delete it->second;
		delete m_keys;
	}
	delete[] m_name;
}

// ----------------------------------------------------------------------

const char *ConfigFile::Section::getName(void) const
{
	return m_name;
}

// ----------------------------------------------------------------------

ConfigFile::Key *ConfigFile::Section::findKey(const char *key) const
{
	KeyMap::const_iterator it = m_keys->find(key);
	if (it != m_keys->end())
		return it->second;

	return nullptr;
}

// ----------------------------------------------------------------------

void ConfigFile::Section::addKey(const char *name, const char *value, bool lazyAdd)
{
	Key *newKey = new Key(name, value, lazyAdd);
	(*m_keys)[newKey->getName()] = newKey;
}

// ----------------------------------------------------------------------

void ConfigFile::Section::removeKey(const char *name)
{
	KeyMap::iterator it = m_keys->find(name);
	if (it != m_keys->end())
	{
		delete it->second;
		m_keys->erase(it);
	}
}

// ----------------------------------------------------------------------

int ConfigFile::Section::getKeyInt(const char *key, int index, int defaultValue) const
{
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_INT(m_name, key, defaultValue);

	KeyMap::const_iterator it = m_keys->find(key);
	if (it != m_keys->end())
		return it->second->getAsInt(index, defaultValue);

	return defaultValue;
}

// ----------------------------------------------------------------------

bool ConfigFile::Section::getKeyBool(const char *key, int index, bool defaultValue) const
{
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_BOOL(m_name, key, defaultValue);

	KeyMap::const_iterator it = m_keys->find(key);
	if (it != m_keys->end())
		return it->second->getAsBool(index, defaultValue);

	return defaultValue;
}

// ----------------------------------------------------------------------

float ConfigFile::Section::getKeyFloat(const char *key, int index, float defaultValue) const
{
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_FLOAT(m_name, key, defaultValue);

	KeyMap::const_iterator it = m_keys->find(key);
	if (it != m_keys->end())
		return it->second->getAsFloat(index, defaultValue);

	return defaultValue;
}

// ----------------------------------------------------------------------

const char *ConfigFile::Section::getKeyString(const char *key, int index, const char *defaultValue) const
{
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_STRING(m_name, key, defaultValue);

	KeyMap::const_iterator it = m_keys->find(key);
	if (it != m_keys->end())
		return it->second->getAsString(index, defaultValue);

	return defaultValue;
}

// ----------------------------------------------------------------------

Tag ConfigFile::Section::getKeyTag(const char *key, int index, Tag defaultValue) const
{
	NO_SPACE_KEY(key);

	CONFIG_FILE_TEMPLATE_CREATE_TAG(m_name, key, defaultValue);

	KeyMap::const_iterator it = m_keys->find(key);
	if (it != m_keys->end())
		return it->second->getAsTag(index, defaultValue);

	return defaultValue;
}

// ----------------------------------------------------------------------

bool ConfigFile::Section::getKeyExists(const char *key) const
{
	NO_SPACE_KEY(key);

	KeyMap::const_iterator it = m_keys->find(key);
	if (it != m_keys->end())
		return true;

	return false;
}

// ----------------------------------------------------------------------

void ConfigFile::Section::dump(void) const
{
#ifdef _DEBUG
	for (KeyMap::const_iterator it = m_keys->begin(); it != m_keys->end(); ++it)
		it->second->dump(it->first);
#endif
}

// ======================================================================
