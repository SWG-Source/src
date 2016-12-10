//======================================================================
//
// MoodManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/MoodManager.h"

#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "UnicodeUtils.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Tag.h"
#include "sharedObject/World.h"

//======================================================================

namespace
{
	typedef std::map<uint32,std::string> IntStringMap;
	typedef std::map<std::string,uint32> StringIntMap;
	
	IntStringMap s_idToNameMap;
	StringIntMap s_nameToIdMap;
		
	bool s_installed = false;
	
	const std::string s_filename = "chat/mood_types.iff";
	
	namespace Tags
	{
		const Tag MOOD = TAG (M,O,O,D);
		const Tag TYPS = TAG (T,Y,P,S);
	}
}

//-----------------------------------------------------------------

void MoodManager::install ()
{
	InstallTimer const installTimer("MoodManager::install ");

	DEBUG_FATAL (s_installed, ("already installed.\n"));

	s_idToNameMap.clear ();
	s_nameToIdMap.clear ();

	s_installed = true;

	load (s_filename);
}

//-----------------------------------------------------------------

void MoodManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	s_idToNameMap.clear   ();
	s_nameToIdMap.clear   ();

	s_installed = false;
}

//----------------------------------------------------------------------

uint32 MoodManager::getMoodByCanonicalName (const std::string & canonicalName)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const StringIntMap::const_iterator it = s_nameToIdMap.find (canonicalName);

	if (it != s_nameToIdMap.end ())
		return (*it).second;

	return 0;
}

//----------------------------------------------------------------------

bool MoodManager::getCanonicalName       (uint32 id, std::string & name)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const IntStringMap::const_iterator it = s_idToNameMap.find (id);

	if (it != s_idToNameMap.end ())
	{
		name = (*it).second;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void MoodManager::getCanonicalMoods      (StringIntMap & sim)
{
	sim = s_nameToIdMap;
}

//----------------------------------------------------------------------

void MoodManager::load (const std::string & filename)
{
	Iff iff;
	if (!iff.open (filename.c_str (), true))
	{
		WARNING_STRICT_FATAL (true, ("MoodManager No such file: '%s'", filename.c_str ()));
		return;
	}

	if (!iff.enterForm (Tags::MOOD, true))
	{
		WARNING_STRICT_FATAL (true, ("MoodManager file '%s' had no tag MOOD", filename.c_str ()));
		return;
	}

	if (!iff.enterForm (TAG_0000, true))
	{
		WARNING_STRICT_FATAL (true, ("MoodManager file '%s' had no tag 0000", filename.c_str ()));
		return;
	}

	if (!iff.enterChunk (Tags::TYPS, true))
	{
		WARNING_STRICT_FATAL (true, ("MoodManager file '%s' had no tag TYPS", filename.c_str ()));
		return;
	}

	uint32 count = 0;

	//-- 'none' must be zero
	static const std::string s_none = "none";
	s_idToNameMap.insert (std::make_pair (count,  s_none));
	s_nameToIdMap.insert (std::make_pair (s_none, count));

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
}

//======================================================================
