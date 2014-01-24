// ======================================================================
//
// AbstractTableBuffer.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/AbstractTableBuffer.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <vector>

// ======================================================================

AbstractTableBuffer::AbstractTableBuffer()
{
}

// ----------------------------------------------------------------------

AbstractTableBuffer::~AbstractTableBuffer()
{
}

// ----------------------------------------------------------------------

/**
 * Return TRUE if the two tagsets have at least one tag in common.
 * TODO:  If this starts showing up on profiles, write a more efficient version
 */

bool AbstractTableBuffer::hasCommonTags(const DB::TagSet &left, const DB::TagSet &right)
{
	std::vector<Tag> result;
	IGNORE_RETURN(std::set_intersection(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(result)));
	return  (!result.empty());
}

// ======================================================================
