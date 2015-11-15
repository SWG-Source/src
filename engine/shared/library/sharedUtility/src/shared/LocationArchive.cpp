// LocationArchive.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/LocationArchive.h"

#include "Archive/Archive.h"
#include "sharedUtility/Location.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"

//-----------------------------------------------------------------------

namespace Archive
{

	void get (Archive::ReadIterator & source, Location & target)
	{
		get(source, target.m_coordinates);
		get(source, target.m_cell);
		get(source, target.m_sceneIdCrc);
	}

	void put (ByteStream & target, const Location & source)
	{
		put(target, source.getCoordinates());
		put(target, source.getCell());
		put(target, source.getSceneIdCrc());
	}
}

