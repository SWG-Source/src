// ======================================================================
//
// ObjectLocator.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================
#include "sharedDatabaseInterface/DbSession.h"

#ifndef INCLUDED_ObjectLocator_H
#define INCLUDED_ObjectLocator_H

class GameServerConnection;

// ======================================================================

/** 
 * Abstract base class for classes that can locate objects in the database.
 *
 * The job of classes derived from ObjectLocator is to get a list of
 * objectID's matching particular criteria.  This is used by Snapshot to
 * identify which objects to load.
 *
 * The list of objects is left in a temporary table on the database.
 *
 * For example, a derived ObjectLocator might find all the objects in a
 * particular spatial chunk.  If an instance of that ObjectLocator is
 * attached to a Snapshot, then the snapshot will load all the objects
 * in a chunk.
 *
 * This class allows Snapshots to be used for different types of loads
 * without deriving different types of snapshots, and it allows a single
 * Snapshot to be used for several logically distinct load operations.
 */
class ObjectLocator
{
  public:
	virtual bool locateObjects               (DB::Session *session, const std::string &schema, int &objectsLocated) =0;
	virtual void sendPreBaselinesCustomData  (GameServerConnection &conn) const;
	virtual void sendPostBaselinesCustomData (GameServerConnection &conn) const;

  public:
	ObjectLocator();
	virtual ~ObjectLocator();
};

// ======================================================================

#endif
