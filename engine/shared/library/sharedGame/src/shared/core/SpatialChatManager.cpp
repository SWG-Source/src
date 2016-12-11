//======================================================================
//
// SpatialChatManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SpatialChatManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Tag.h"
#include "sharedFile/Iff.h"
#include "UnicodeUtils.h"

#include <map>
#include <set>
#include <vector>

//======================================================================

namespace SpatialChatManagerNamespace
{
	typedef std::map<uint32,std::string> IntStringMap;
	typedef std::map<std::string,uint32> StringIntMap;
	typedef std::map<uint32,uint16>      IntShortMap;
	typedef std::set<uint32>             IntSet;
	typedef std::map<uint32,std::vector<std::string> > IntStringVectorMap;

	IntStringMap s_idToNameMap;
	StringIntMap s_nameToIdMap;
	IntShortMap  s_idToVolumeMap;
	IntSet       s_privateIds;
	IntStringVectorMap s_skillsProviding;

	uint16       s_defaultVolume = 1024;
	std::vector<std::string> s_defaultSkills;

	bool s_installed = false;

	const std::string s_filename = "chat/spatial_chat_types.iff";

	namespace Tags
	{
		const Tag SPCT = TAG (S,P,C,T);
		const Tag TYPS = TAG (T,Y,P,S);
		const Tag VOLT = TAG (V,O,L,T);
		const Tag PRVT = TAG (P,R,V,T);
		const Tag SKCT = TAG (S,K,C,T);
		const Tag VOLS = TAG (V,O,L,S);
		const Tag PRIV = TAG (P,R,I,V);
		const Tag SKCS = TAG (S,K,C,S);
	}
}
using namespace SpatialChatManagerNamespace;

//----------------------------------------------------------------------

uint32 SpatialChatManager::ms_defaultChatType;

//----------------------------------------------------------------------

void SpatialChatManager::install             ()
{
	InstallTimer const installTimer("SpatialChatManager::install");

	DEBUG_FATAL (s_installed, ("installed"));

	s_idToNameMap.clear     ();
	s_nameToIdMap.clear     ();
	s_idToVolumeMap.clear   ();
	s_privateIds.clear      ();
	s_skillsProviding.clear ();

	ms_defaultChatType = 1;

	s_installed = true;

	load (s_filename);
}

//----------------------------------------------------------------------

void SpatialChatManager::remove              ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	s_idToNameMap.clear     ();
	s_nameToIdMap.clear     ();
	s_idToVolumeMap.clear   ();
	s_privateIds.clear      ();
	s_skillsProviding.clear ();

	s_installed = false;
}

//----------------------------------------------------------------------

/**
* @param name must be lowercase canonical name
*/

uint32 SpatialChatManager::getChatTypeByName   (const std::string & name)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const StringIntMap::const_iterator it = s_nameToIdMap.find (name);

	if (it != s_nameToIdMap.end ())
		return (*it).second;

	return 0;
}

//----------------------------------------------------------------------

bool SpatialChatManager::getChatNameByType   (uint32 type, std::string & name)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const IntStringMap::const_iterator it = s_idToNameMap.find (type);

	if (it != s_idToNameMap.end ())
	{
		name = (*it).second;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool SpatialChatManager::isPrivate           (uint32 type)
{
	return s_privateIds.find (type) != s_privateIds.end ();
}

//----------------------------------------------------------------------

uint16 SpatialChatManager::getVolume           (uint32 type)
{
	const IntShortMap::const_iterator it = s_idToVolumeMap.find (type);

	if (it != s_idToVolumeMap.end ())
		return (*it).second;

	return s_defaultVolume;
}

//----------------------------------------------------------------------

const std::vector<std::string> &SpatialChatManager::getSkillsProviding(uint32 type)
{
	const IntStringVectorMap::const_iterator it = s_skillsProviding.find(type);
	if (it != s_skillsProviding.end())
		return (*it).second;
	return s_defaultSkills;
}

//----------------------------------------------------------------------

void SpatialChatManager::load (const std::string & filename)
{
	Iff iff;
	if (!iff.open (filename.c_str (), true))
	{
		WARNING_STRICT_FATAL (true, ("SpatialChatManager No such file: '%s'", filename.c_str ()));
		return;
	}

	if (!iff.enterForm (Tags::SPCT, true))
	{
		WARNING_STRICT_FATAL (true, ("SpatialChatManager file '%s' had no tag SPCT", filename.c_str ()));
		return;
	}

	if (!iff.enterForm (TAG_0000, true))
	{
		WARNING_STRICT_FATAL (true, ("SpatialChatManager file '%s' had no tag 0000", filename.c_str ()));
		return;
	}

	if (!iff.enterChunk (Tags::TYPS, true))
	{
		WARNING_STRICT_FATAL (true, ("SpatialChatManager file '%s' had no tag TYPS", filename.c_str ()));
		return;
	}

	uint32 count = 0;
	while (iff.getChunkLengthLeft () > 0)
	{
		const std::string name = Unicode::toLower (iff.read_stdstring ());

		++count;

		s_idToNameMap.insert (std::make_pair (count, name));

		const std::pair<StringIntMap::const_iterator, bool> retval = s_nameToIdMap.insert (std::make_pair (name, count));

		if (!retval.second)
			WARNING_STRICT_FATAL (true, ("SpatialChatManager file '%s' duplicate chat type '%s'", filename.c_str (), name.c_str ()));
	}

	iff.exitChunk (Tags::TYPS);

	if (!iff.enterForm (Tags::VOLS, true))
	{
		WARNING_STRICT_FATAL (true, ("SpatialChatManager file '%s' had no tag VOLS", filename.c_str ()));
		return;
	}

	while (iff.enterChunk (Tags::VOLT, true))
	{
		const std::string name  = Unicode::toLower (iff.read_stdstring ());
		const uint16     volume = iff.read_uint16 ();

		if (name.empty ())
		{
			s_defaultVolume = volume;
		}
		else
		{
			const uint32 id = getChatTypeByName (name);

			if (id)
				s_idToVolumeMap.insert (std::make_pair (id, volume));
			else
				WARNING_STRICT_FATAL (true, ("SpatialChatManager file '%s' invalid chat type '%s' for volume", filename.c_str (), name.c_str ()));
		}

		iff.exitChunk (Tags::VOLT);
	}

	iff.exitForm (Tags::VOLS);

	if (!iff.enterForm (Tags::PRIV, true))
	{
		WARNING_STRICT_FATAL (true, ("SpatialChatManager file '%s' had no tag PRIV", filename.c_str ()));
		return;
	}

	while (iff.enterChunk (Tags::PRVT, true))
	{
		const std::string name = iff.read_stdstring ();
		
		uint32 id = getChatTypeByName (name);

		if (id)
			s_privateIds.insert (id);
		else
		{
			WARNING_STRICT_FATAL (true, ("SpatialChatManager file '%s' invalid chat type '%s' for private", filename.c_str (), name.c_str ()));
		}

		iff.exitChunk (Tags::PRVT);
	}

	iff.exitForm (Tags::PRIV);

	if (!iff.enterForm (Tags::SKCS, true))
	{
		WARNING_STRICT_FATAL (true, ("SpatialChatManager file '%s' had no tag SKCS", filename.c_str ()));
		return;
	}

	while (iff.enterChunk (Tags::SKCT, true))
	{
		const std::string typeName = iff.read_stdstring ();
		const std::string skillName = iff.read_stdstring ();

		uint32 id = getChatTypeByName (typeName);
		if (id)
			s_skillsProviding[id].push_back(skillName);
		else
		{
			WARNING_STRICT_FATAL (true, ("SpatialChatManager file '%s' invalid chat type '%s' for skill", filename.c_str (), typeName.c_str ()));
		}

		iff.exitChunk (Tags::SKCT);
	}

	iff.exitForm (Tags::SKCS);

}

//======================================================================
