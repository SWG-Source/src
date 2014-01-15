// ======================================================================
//
// PgcRatingInfo.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PgcRatingInfo.h"

#include "Archive/Archive.h"
#include "sharedFoundation/NetworkIdArchive.h"

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, PgcRatingInfo & target)
	{
		get(source, target.m_chroniclerName);
		get(source, target.m_ratingCount);
		get(source, target.m_ratingTotal);
		get(source, target.m_lastRatingTime);
		get(source, target.m_data);
	}

	void put(ByteStream & target, const PgcRatingInfo & source)
	{
		put(target, source.m_chroniclerName);
		put(target, source.m_ratingCount);
		put(target, source.m_ratingTotal);
		put(target, source.m_lastRatingTime);
		put(target, source.m_data);
	}
}

// ======================================================================

bool PgcRatingInfo::operator== (const PgcRatingInfo& rhs) const
{
	if (this == &rhs)
		return true;

	return ((m_chroniclerName == rhs.m_chroniclerName) &&
		(m_ratingCount == rhs.m_ratingCount) &&
		(m_ratingTotal == rhs.m_ratingTotal) &&
		(m_lastRatingTime == rhs.m_lastRatingTime) &&
		(m_data == rhs.m_data));
}

// ----------------------------------------------------------------------

bool PgcRatingInfo::operator!= (const PgcRatingInfo& rhs) const
{
	return !operator==(rhs);
}

// ======================================================================
