// ======================================================================
//
// PurgeCompleteCustomPersistStep.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PurgeCompleteCustomPersistStep_H
#define INCLUDED_PurgeCompleteCustomPersistStep_H

// ======================================================================

#include "serverDatabase/CustomPersistStep.h"
#include "sharedFoundation/StationId.h"

// ======================================================================

/**
 * A custom persist step when a step in the purge cycle has been
 * completed for the specified account and saved to the database.
 * Sends an acknowledgement to the Login Server (via Central)
 */
class PurgeCompleteCustomPersistStep : public CustomPersistStep
{
  public:
	explicit PurgeCompleteCustomPersistStep(StationId stationId);
	~PurgeCompleteCustomPersistStep();
	
	virtual bool beforePersist (DB::Session *session);
	virtual bool afterPersist  (DB::Session *session);
	virtual void onComplete    ();

  private:
	StationId m_stationId;

  private:
	PurgeCompleteCustomPersistStep &operator= (const PurgeCompleteCustomPersistStep&); //disable
	PurgeCompleteCustomPersistStep(const PurgeCompleteCustomPersistStep&); //disable
};

// ======================================================================

#endif
