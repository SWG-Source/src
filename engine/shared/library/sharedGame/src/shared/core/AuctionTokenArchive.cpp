//======================================================================
//
// AuctionTokenArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/AuctionTokenArchive.h"

#include "Archive/Archive.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/AuctionToken.h"
#include "unicodeArchive/UnicodeArchive.h"

//======================================================================

namespace Archive
{
	//----------------------------------------------------------------------
	
	void get (Archive::ReadIterator & source, AuctionToken & target)
	{
		int version = 0;
		get (source, version);
		
		switch (version)
		{
		case 0:
			get (source, target.sharedTemplateName);
			get (source, target.customizationData);
			break;
		default:
			WARNING (true, ("invalid AuctionTokenArchive version %d", version));
			ReadException ex("invalid AuctionTokenArchive version");
			throw (ex);
		}

	}

	//----------------------------------------------------------------------

	void put (ByteStream & target, const AuctionToken & source)
	{
		put (target, int(0));

		put (target, source.sharedTemplateName);
		put (target, source.customizationData);
	}
}

//======================================================================
