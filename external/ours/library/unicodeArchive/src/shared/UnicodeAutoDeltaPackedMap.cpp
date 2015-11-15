// ======================================================================
//
// UnicodeAutoDeltaPackedMap.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "unicodeArchive/FirstUnicodeArchive.h"
#include "unicodeArchive/UnicodeAutoDeltaPackedMap.h"

#include "UnicodeUtils.h"
#include "unicodeArchive/UnicodeArchive.h"
#include <stdio.h>

// ======================================================================

namespace Archive
{
	template <> void AutoDeltaPackedMap<int, Unicode::String>::pack(ByteStream & target, const std::string & buffer)
	{
		char temp[200];
		Command c;

		Archive::put(target, countCharacter(buffer,':'));
		Archive::put(target, static_cast<size_t>(0)); // baselineCommandCount
		
		int tempPos = 0;
		for (std::string::const_iterator i=buffer.begin(); i!=buffer.end(); ++i)
		{
			if (*i==':')
			{
				temp[tempPos]='\0';
				sscanf(temp,"%i",&c.key);
				char const * const valueStart = strchr(temp,' ');
				if (valueStart)
					c.value = Unicode::utf8ToWide(valueStart+1);
				else
					c.value = Unicode::String();
				Archive::put(target, static_cast<unsigned char>(Command::ADD));
				Archive::put(target, c.key);
				Archive::put(target, c.value);
				tempPos=0;
			}
			else
			{
				temp[tempPos++]=*i;
			}
		}
	}

	// ======================================================================

	template <> void AutoDeltaPackedMap<int, Unicode::String>::unpack(ReadIterator & source, std::string & buffer)
	{
		char temp[200];
		
		Command c;
		size_t commandCount;
		size_t baselineCommandCount;

		Archive::get(source, commandCount);
		Archive::get(source, baselineCommandCount);

		if (commandCount==0)
		{
			buffer=' '; // An empty map is represented as a single space, because a completely empty string is used to mean "no change"
		}
		else
		{
			for (size_t i = 0; i < commandCount; ++i)
			{
				Archive::get(source, c.cmd);
				assert(c.cmd == Command::ADD); // only add is valid in unpack
				Archive::get(source, c.key);
				Archive::get(source, c.value);

#ifdef WIN32
				_snprintf(temp, sizeof(temp)-1, "%i %s:", c.key, Unicode::wideToUTF8(c.value).c_str());
#else
				snprintf(temp, sizeof(temp)-1, "%i %s:", c.key, Unicode::wideToUTF8(c.value).c_str());
#endif

				temp[sizeof(temp)-1]='\0';
				buffer+=temp;
			}
		}
	}
}

// ======================================================================
