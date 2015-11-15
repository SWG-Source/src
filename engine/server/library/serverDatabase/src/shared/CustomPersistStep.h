// ======================================================================
//
// CustomPersistStep.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CustomPersistStep_H
#define INCLUDED_CustomPersistStep_H

// ======================================================================

namespace DB
{
	class Session;
}

// ======================================================================

/**
 * CustomPersistStep is an abstract class representing an extra thing
 * to do while saving a snapshot.  (For example, associating one of the
 * saved objects with a player's account.)
 *
 * CustomPersistSteps can do things before the objects are saved or after
 * they are saved, but they always take place within the same database
 * transaction.
 *
 * CustomPersistSteps can also do special things in the main thread
 * after completion, e.g. acknowledging to another server.
 */
class CustomPersistStep
{
  public:
	virtual bool beforePersist (DB::Session *session) =0;
	virtual bool afterPersist  (DB::Session *session) =0;
	virtual void onComplete    ()                     =0;

	virtual ~CustomPersistStep();
};

// ======================================================================

#endif
