//========================================================================
//
// AttribModNameManager.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AttribModNameManager.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"


namespace AttribModNameManagerNamespace
{
	// some stuf to help track down why we're getting attrib mod name requests
	// for names we don't know about
	std::set<uint32> unknownCrcs;
}

AttribModNameManager * AttribModNameManager::ms_attribModNameManager = nullptr;


//========================================================================

/**
 * Used to sort dotted names. A name that is a sub-dotted-string of the compared
 * name are considered equal.
 */
bool AttribModNameManager::SortDottedNames::operator()(const std::string &a, const std::string &b) const
{
	size_t as = a.size();
	size_t bs = b.size();
	if (as < bs && b[as] == '.')
		return strncmp(a.c_str(), b.c_str(), as) < 0;
	else if (bs < as && a[bs] == '.')
		return strncmp(b.c_str(), a.c_str(), bs) > 0;
	else
		return a < b;
}

//========================================================================

/**
 * Class constructror.
 */
AttribModNameManager::AttribModNameManager() :
	m_names(new Names),
	m_crcMap(new CrcNameMap)
{
}	// AttribModNameManager::AttribModNameManager

// ----------------------------------------------------------------------

/**
 * Class destructror.
 */
AttribModNameManager::~AttribModNameManager()
{
	delete m_names;
	delete m_crcMap;
	m_names = nullptr;
	m_crcMap = nullptr;
}	// AttribModNameManager::~AttribModNameManager

// ----------------------------------------------------------------------

/**
 * Creates the AttribModNameManager.
 */
void AttribModNameManager::install()
{
	if (ms_attribModNameManager == nullptr)
	{
		ms_attribModNameManager = new AttribModNameManager;
		ExitChain::add(AttribModNameManager::remove, "AttribModNameManager::remove");
	}
}	// AttribModNameManager::install

// ----------------------------------------------------------------------

/**
 * Destroys the AttribModNameManager.
 */
void AttribModNameManager::remove()
{
	if (ms_attribModNameManager != nullptr)
	{
		delete ms_attribModNameManager;
		ms_attribModNameManager = nullptr;
	}
}	// AttribModNameManager::remove

// ----------------------------------------------------------------------

/**
 * Adds an attrib mod name to the manager. If the mod name wasn't a duplicate
 * of an already existing mod, we send it to the other game servers.
 *
 * @param name		the attrib mod name
 */
void AttribModNameManager::addAttribModName(const char * name)
{
	bool added = addAttribModNameFromRemote(name);
	if (added)
	{
		DEBUG_REPORT_LOG(true, ("AttribModNameManager sending name %s to other servers\n", name));

		// tell other servers about the mod name

		ServerMessageForwarding::beginBroadcast();

		GenericValueTypeMessage<std::string> const addAttribModNameMessage("AddAttribModName", name);
		ServerMessageForwarding::send(addAttribModNameMessage);

		ServerMessageForwarding::end();
	}
}	// AttribModNameManager::addAttribModName

// ----------------------------------------------------------------------

/**
 * Adds an attrib mod name to the manager.
 *
 * @param name		the attrib mod name
 */
bool AttribModNameManager::addAttribModNameFromRemote(const char * name)
{
	const int BUFSIZE = 1024;
	char buffer[BUFSIZE];
	int nameLen = strlen(name);
	if (nameLen >= BUFSIZE)
	{
		WARNING_STRICT_FATAL(true, ("AttribModNameManager::addAttribModName passed "
			"mod name %s >= %d bytes", name, BUFSIZE));
		return false;
	}
	strcpy(buffer, name);

	// we add the name to our map, plus any base name it derives from
	bool added = false;
	char * bufEnd = &buffer[nameLen];
	while (bufEnd != buffer)
	{
		std::pair<Names::iterator, bool> result = m_names->insert(buffer);
		if (result.second)
		{
			DEBUG_REPORT_LOG(true, ("AttribModNameManager added name %s\n", buffer));
			added = true;
			uint32 crc = Crc::calculate(buffer);
			m_crcMap->insert(std::make_pair(crc, result.first));
			
			// see if our new name is one that we couldn't find before
			if (AttribModNameManagerNamespace::unknownCrcs.find(crc) != 
				AttribModNameManagerNamespace::unknownCrcs.end())
			{
				LOG("unknown_mods", ("AttribModNameManager::addAttribModNameFromRemote "
					"added previously unknown mod name %s(%u) in frame %lu",
					buffer, crc, ServerClock::getInstance().getServerFrame()));
				AttribModNameManagerNamespace::unknownCrcs.erase(crc);
			}
			
			// look for the last '.' in the name, and replace it with 0
			while (--bufEnd != buffer)
			{
				if (*bufEnd == '.')
				{
					*bufEnd = '\0';
					break;
				}
			}
		}
		else
			break;
	}
	return added;
}	// AttribModNameManager::addAttribModNameFromRemote

// ----------------------------------------------------------------------

/**
 * Adds attrib mod names to the manager.
 *
 * @param names		the attrib mod names, separated by spaces
 */
void AttribModNameManager::addAttribModNamesListFromRemote(const char * names)
{
static const int BUFSIZE = 1024;
char buffer[BUFSIZE];

	int i;
	const char * c = names;
	while (*c != '\0')
	{
		for (i = 0; i < BUFSIZE-1 && *c != ' ' && *c != '\0';)
			buffer[i++] = *c++;
		if (*c != ' ' && *c != '\0')
		{
			// the name is longer than our buffer, print an error message
			// and go to the next name
			buffer[BUFSIZE-1] = '\0';
			WARNING(true, ("AttribModNameManager::addAttribModNamesListFromRemote "
				"received a name (%s) that is too long!", buffer));
			while (*c != ' ' && *c != '\0')
				++c;
		}
		else
		{
			buffer[i] = '\0';
			addAttribModNameFromRemote(buffer);
		}
		if (*c == ' ')
			++c;
	}
}	// AttribModNameManager::addAttribModNamesListFromRemote

// ----------------------------------------------------------------------

/**
 * Called when another game server starts and wants the attrib mod names being 
 * used.
 */
void AttribModNameManager::sendAllNamesToServer(std::vector<uint32> const & servers) const
{
	// get the set of non-base names we know about
	std::set<std::string, SortDottedNames> unique;
	for (Names::reverse_iterator i = m_names->rbegin(); i != m_names->rend(); ++i)
		unique.insert(*i);

	if (!unique.empty())
	{
		// concat the names
		std::string msgString;
		for (std::set<std::string, SortDottedNames>::const_iterator j = unique.begin(); j != unique.end(); ++j)
		{
			msgString.append(*j);
			msgString.append(" ");
		}

		// @todo: test if msgString gets too big and break it up if needed

		// send the names to the requesting server

		ServerMessageForwarding::begin(servers);

		GenericValueTypeMessage<std::string> const addAttribModNamesListMessage("AddAttribModNamesList", msgString);
		ServerMessageForwarding::send(addAttribModNamesListMessage);

		ServerMessageForwarding::end();
	}
}

// ----------------------------------------------------------------------

/**
 * Returns the attrib mod name for a given crc.
 *
 * @param crc		the crc value to look up
 *
 * @return the attrib mod name, or nullptr if there was no name for the crc
 */
const char * AttribModNameManager::getAttribModName(uint32 crc) const
{
	if (crc != 0)
	{
		CrcNameMap::const_iterator result = m_crcMap->find(crc);
		if (result != m_crcMap->end())
			return (*(*result).second).c_str();
		LOG("unknown_mods", ("AttribModNameManager::getAttribModName could not find "
			"mod name for crc %u in frame %lu", crc, 
			ServerClock::getInstance().getServerFrame()));
		AttribModNameManagerNamespace::unknownCrcs.insert(crc);
	}
	return nullptr;
}	// AttribModNameManager::getAttribModName

// ----------------------------------------------------------------------

/**
 * For a given attrib mod base name, returns all the mod names derived from it.
 *
 * @param base		the base mod name
 * @param names		list that will be filled in with the derived names
 */
void AttribModNameManager::getAttribModNamesFromBase(const std::string & base, 
	std::vector<const char *> & names) const
{
	size_t baselen = base.size();
	for (Names::const_iterator i = m_names->lower_bound(base); i != m_names->end(); ++i)
	{
		if (strncmp(base.c_str(), (*i).c_str(), baselen) == 0)
		{
			char lastchar = (*i).c_str()[baselen];
			if (lastchar == '\0' || lastchar == '.')
			{
				names.push_back((*i).c_str());
			}
			else
				break;
		}
		else
			break;
	}
}	// AttribModNameManager::getAttribModNamesFromBase

// ----------------------------------------------------------------------

/**
 * For a given attrib mod base name, returns all the mod name crcs derived from it.
 *
 * @param base		the base mod name
 * @param crcs		list that will be filled in with the derived crcs
 */
void AttribModNameManager::getAttribModCrcsFromBase(const std::string & base, 
	std::vector<uint32> & crcs) const
{
	size_t baselen = base.size();
	for (Names::const_iterator i = m_names->lower_bound(base); i != m_names->end(); ++i)
	{
		if (strncmp(base.c_str(), (*i).c_str(), baselen) == 0)
		{
			char lastchar = (*i).c_str()[baselen];
			if (lastchar == '\0' || lastchar == '.')
			{
				crcs.push_back(Crc::calculate((*i).c_str()));
			}
			else
				break;
		}
		else
			break;
	}
}	// AttribModNameManager::getAttribModCrcsFromBase

// ----------------------------------------------------------------------

/**
 * For a given attrib mod base crc, returns all the mod names derived from it.
 *
 * @param base		the base mod name
 * @param names		list that will be filled in with the derived names
 */
void AttribModNameManager::getAttribModNamesFromBase(uint32 base, 
	std::vector<const char *> & names) const
{
	const char * baseName = getAttribModName(base);
	if (baseName != nullptr)
		getAttribModNamesFromBase(baseName, names);
}	// AttribModNameManager::getAttribModNamesFromBase

// ----------------------------------------------------------------------

/**
 * For a given attrib mod base crc, returns all the mod name crcs derived from it.
 *
 * @param base		the base mod name
 * @param crcs		list that will be filled in with the derived crcs
 */
void AttribModNameManager::getAttribModCrcsFromBase(uint32 base, 
	std::vector<uint32> & crcs) const
{
	const char * baseName = getAttribModName(base);
	if (baseName != nullptr)
		getAttribModCrcsFromBase(baseName, crcs);
}	// AttribModNameManager::getAttribModCrcsFromBase

