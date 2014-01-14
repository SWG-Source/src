//======================================================================
//
// ProsePackageParticipantArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ProsePackageParticipantArchive.h"
#include "sharedGame/ProsePackageParticipant.h"

#include "Archive/Archive.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

//======================================================================

namespace Archive
{
	//----------------------------------------------------------------------

	void get (Archive::ReadIterator & source, ProsePackageParticipant & target)
	{
		get (source, target.id);
		get (source, target.stringId);
		get (source, target.str);
	}

	//----------------------------------------------------------------------

	void put (ByteStream & target, const ProsePackageParticipant & source)
	{
		put (target, source.id);
		put (target, source.stringId);
		put (target, source.str);
	}
}

//======================================================================
