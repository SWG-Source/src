//========================================================================
//
// AttribModNameManager.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef _INCLUDED_AttribModNameManager_H
#define _INCLUDED_AttribModNameManager_H

//#include "sharedFoundation/PersistentCrcString.h"
#include <set>


/**
 * Keeps track of attrib mod names being used in the game. Attrib mods just store
 * the crc value for the name, this class can be used to get back the string 
 * from the crc value. Attrib mod names take the same form as objvar names; a group
 * of names can be collected under a single list name, separated by a '.'.
 */
class AttribModNameManager
{
public:
	static void install();
	static void remove();
	
	static AttribModNameManager & getInstance();


	void addAttribModName(const char * name);
	bool addAttribModNameFromRemote(const char * name);
	void addAttribModNamesListFromRemote(const char * names);
	void sendAllNamesToServer(std::vector<uint32> const & servers) const;

	const char * getAttribModName(uint32 crc) const;
	void         getAttribModNamesFromBase(const std::string & base, std::vector<const char *> & names) const;
	void         getAttribModCrcsFromBase(const std::string & base, std::vector<uint32> & crcs) const;
	void         getAttribModNamesFromBase(uint32 base, std::vector<const char *> & names) const;
	void         getAttribModCrcsFromBase(uint32 base, std::vector<uint32> & crcs) const;

private:
	 AttribModNameManager();
	~AttribModNameManager();

private:
	static AttribModNameManager * ms_attribModNameManager;

	class SortDottedNames
	{
	public:
		bool operator()(const std::string &, const std::string &) const;
	};

	typedef std::set<std::string> Names;
	typedef std::map<uint32, Names::const_iterator> CrcNameMap;

	Names      * m_names;
	CrcNameMap * m_crcMap;
};


//========================================================================

inline AttribModNameManager & AttribModNameManager::getInstance()
{
	NOT_NULL(ms_attribModNameManager);
	return *ms_attribModNameManager;
}


#endif	// _INCLUDED_AttribModNameManager_H

