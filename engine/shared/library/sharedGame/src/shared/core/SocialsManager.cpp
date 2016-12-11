//======================================================================
//
// SocialsManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================


#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SocialsManager.h"

#include "UnicodeUtils.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Crc.h"
#include "sharedUtility/DataTable.h"

#include <map>

//======================================================================

namespace
{
	typedef std::map<uint32,std::string> IntStringMap;
	typedef std::map<std::string,uint32> StringIntMap;

	IntStringMap s_idToNameMap;
	StringIntMap s_nameToIdMap;
	IntStringMap s_crcToNameMap;
	
	bool s_installed = false;

	const std::string s_filename = "datatables/chat/social_types.iff";

	enum DtCols
	{
		DC_name
	};
}

//----------------------------------------------------------------------

void SocialsManager::install ()
{
	InstallTimer const installTimer("SocialsManager::install ");

	DEBUG_FATAL (s_installed, ("already installed"));
	load (s_filename);
	s_installed = true;
}

//----------------------------------------------------------------------

void SocialsManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	s_idToNameMap.clear ();
	s_nameToIdMap.clear ();
	s_crcToNameMap.clear();
	s_installed = false;
}

//----------------------------------------------------------------------

uint32 SocialsManager::getSocialTypeByName   (const std::string & name)
{
	if (!s_installed)
		install ();

	const StringIntMap::const_iterator it = s_nameToIdMap.find (name);

	if (it != s_nameToIdMap.end ())
		return (*it).second;

	return 0;
}

//----------------------------------------------------------------------

bool SocialsManager::getSocialNameByType   (uint32 type, std::string & name)
{
	if (!s_installed)
		install ();

	const IntStringMap::const_iterator it = s_idToNameMap.find (type);

	if (it != s_idToNameMap.end ())
	{
		name = (*it).second;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool SocialsManager::getSocialNameByCrc(const uint32 crc, std::string & name)
{
	if (!s_installed)
		install ();

	const IntStringMap::const_iterator it = s_crcToNameMap.find (crc);

	if (it != s_crcToNameMap.end ())
	{
		name = (*it).second;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void SocialsManager::load (const std::string & filename)
{	
	Iff iff;
	
	if (!iff.open (filename.c_str (), true))
		WARNING (true, ("Data file %s not available.", filename.c_str ()));
	else
	{
		DataTable dt;
		dt.load (iff);
		iff.close ();

		const int numRows = dt.getNumRows ();

		uint32 count = 0;
		for (int i = 0; i < numRows; ++i)
		{
			const std::string & name = Unicode::toLower (dt.getStringValue (DC_name, i));
			++count;

			s_idToNameMap.insert (std::make_pair (count, name));
			s_crcToNameMap.insert (std::make_pair (Crc::calculate(name.c_str()), name));

			const std::pair<StringIntMap::const_iterator, bool> retval = s_nameToIdMap.insert (std::make_pair (name, count));

			if (!retval.second)
				WARNING_STRICT_FATAL (true, ("SocialsManager file '%s' duplicate social type '%s'", filename.c_str (), name.c_str ()));
		}
	}
}

//----------------------------------------------------------------------

void SocialsManager::getCanonicalSocials (StringIntMap & sim)
{
	sim = s_nameToIdMap;
}

//======================================================================
