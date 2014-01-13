//======================================================================
//
// ShipDamageMessageArchive.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"

#include "sharedNetworkMessages/ShipDamageMessage.h"

//----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator & source, ShipDamageMessage & target)
	{
		target.get(source);
	}
	
	void put(ByteStream & target, ShipDamageMessage const & source)
	{
		source.put(target);
	}

}

//======================================================================

