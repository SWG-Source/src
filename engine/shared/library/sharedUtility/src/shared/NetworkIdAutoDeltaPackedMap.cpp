// ======================================================================
//
// NetworkIdAutoDeltaPackedMap.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/NetworkIdAutoDeltaPackedMap.h"

#include "sharedFoundation/NetworkIdArchive.h"

// ======================================================================

namespace Archive
{
	template<>
	void AutoDeltaPackedMap<int, NetworkId>::pack(ByteStream & target, const std::string & buffer)
	{
		char temp[200];
		char value[200];
		Command c;

		Archive::put(target, countCharacter(buffer,':'));
		Archive::put(target, static_cast<size_t>(0)); // baselineCommandCount
		
		int tempPos = 0;
		for (std::string::const_iterator i=buffer.begin(); i!=buffer.end(); ++i)
		{
			if (*i==':')
			{
				temp[tempPos]='\0';				
				sscanf(temp,"%i %s",&c.key, value);
				c.value=NetworkId(std::string(value)); // because there's no constructor that takes a char*
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

	template<>
	void AutoDeltaPackedMap<int, NetworkId>::unpack(ReadIterator & source, std::string & buffer)
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
				Archive::get(source, c.key);
				Archive::get(source, c.value);
				
				snprintf(temp, sizeof(temp)-1, "%i %s:", c.key, c.value.getValueString().c_str());
			
				temp[sizeof(temp)-1]='\0';
				buffer+=temp;
			}
		}
	}

}	

// ======================================================================
