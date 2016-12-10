// ======================================================================
//
// ImmediateDeleteCustomPersistStep.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ImmediateDeleteCustomPersistStep_H
#define INCLUDED_ImmediateDeleteCustomPersistStep_H

// ======================================================================

#include "serverDatabase/CustomPersistStep.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class ImmediateDeleteCustomPersistStep : public CustomPersistStep
{
  public:
	ImmediateDeleteCustomPersistStep();
	~ImmediateDeleteCustomPersistStep();

	virtual bool beforePersist (DB::Session *session);
	virtual bool afterPersist  (DB::Session *session);
	virtual void onComplete    ();

	void addObject             (const NetworkId &deletedObject);
	
  private:
	std::set<NetworkId>     *m_objects; // using set instead of vector to enforce uniqueness

  private:
	ImmediateDeleteCustomPersistStep &operator= (const ImmediateDeleteCustomPersistStep&); //disable
	ImmediateDeleteCustomPersistStep(const ImmediateDeleteCustomPersistStep&); //disable
};

// ======================================================================

#endif
