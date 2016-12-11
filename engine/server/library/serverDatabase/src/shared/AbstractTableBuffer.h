// ======================================================================
//
// AbstractTableBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AbstractTableBuffer_H
#define INCLUDED_AbstractTableBuffer_H

// ======================================================================

#include "sharedFoundation/Tag.h"

// ======================================================================

namespace DB
{
	class Session;
}

class NetworkId;

// ======================================================================

namespace DB
{
	typedef std::set<Tag> TagSet;
}

// ======================================================================

/** 
 * Abstract base class for TableBuffer's.  Allows TableBuffers to be
 * put in lists, have virtual functions, etc.
 */

class AbstractTableBuffer
{
  public:
	/**
	 * Load the buffer from the database.
	 * The TagSet identifies which types of objects are being loaded
	 * Some buffers may change how they work or skip loading altogether
	 * based on the contents of this set.
	 */
	virtual bool load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase) =0;
	virtual bool save(DB::Session *session) =0;
	virtual void removeObject(const NetworkId &object) =0;

	static bool hasCommonTags(const DB::TagSet &left, const DB::TagSet &right);
	
  public:
	AbstractTableBuffer();
	virtual ~AbstractTableBuffer();
};

// ======================================================================

#endif
