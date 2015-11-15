//======================================================================
//
// ProsePackageArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ProsePackageArchive.h"

#include "Archive/Archive.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageParticipantArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

//======================================================================

namespace Archive
{
	//----------------------------------------------------------------------

	void get (Archive::ReadIterator & source, ProsePackage & target)
	{
		get (source, target.stringId);

		get (source, target.actor);
		get (source, target.target);
		get (source, target.other);

		get (source, target.digitInteger);
		get (source, target.digitFloat);

		get (source, target.complexGrammar);
	}

	//----------------------------------------------------------------------

	void put (ByteStream & target, const ProsePackage & source)
	{
		put (target, source.stringId);

		put (target, source.actor);
		put (target, source.target);
		put (target, source.other);
		
		put (target, source.digitInteger);
		put (target, source.digitFloat);

		put (target, source.complexGrammar);
	}
}

//======================================================================
