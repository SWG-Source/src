// ======================================================================
//
// AutoDeltaNetworkIdPackedMap.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AutoDeltaNetworkIdPackedMap_H
#define INCLUDED_AutoDeltaNetworkIdPackedMap_H

// ======================================================================

#include "Archive/AutoDeltaMap.h"
#include <stdio.h>
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "Archive/AutoDeltaPackedMap.h"

// ======================================================================

namespace Archive
{
	/**
	 * An AutoDeltaNetworkIdPackedMap is an AutoDeltaMap that will be packed into
	 * a single value for storage.  It functions as an AutoDeltaMap in
	 * all respects except that packDeltas() will send the entire map
	 * on the network.
	 */
	template<>
	void AutoDeltaPackedMap<NetworkId, int>::pack(ByteStream & target, const std::string & buffer)
	{
		char temp[200];
		AutoDeltaMap<NetworkId, int>::Command c;

		Archive::put(target, countCharacter(buffer,':'));
		Archive::put(target, static_cast<size_t>(0)); // baselineCommandCount
		
		int tempPos = 0;
		for (std::string::const_iterator i=buffer.begin(); i!=buffer.end(); ++i)
		{
			if (*i==':')
			{
				temp[tempPos]='\0';
				sscanf(temp, "%d", &c.value);
				Archive::put(target, static_cast<unsigned char>(AutoDeltaMap<NetworkId, int>::Command::ADD));
				Archive::put(target, c.key);
				Archive::put(target, c.value);
				tempPos=0;
			}
			else if (*i == ' ')
			{
				temp[tempPos] = '\0';
				c.key = NetworkId(temp);
				tempPos = 0;
			}
			else
				temp[tempPos++]=*i;
		}
	}

	template<>
	void AutoDeltaPackedMap<NetworkId, int>::unpack(ReadIterator & source, std::string & buffer)
	{
		char temp[200];
		
		AutoDeltaMap<NetworkId, int>::Command c;
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
				
#ifdef WIN32
				_snprintf(temp, sizeof(temp)-1, "%s %d:", c.key.getValueString().c_str(), c.value);
#else
				snprintf(temp, sizeof(temp)-1, "%s %d:", c.key.getValueString().c_str(), c.value);
#endif
			
				temp[sizeof(temp)-1]='\0';
				buffer+=temp;
			}
		}
	}

} //namespace

// ======================================================================

#endif
