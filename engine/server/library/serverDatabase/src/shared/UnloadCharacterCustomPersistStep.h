// ======================================================================
//
// UnloadCharacterCustomPersistStep.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UnloadCharacterCustomPersistStep_H
#define INCLUDED_UnloadCharacterCustomPersistStep_H

// ======================================================================

#include "serverDatabase/CustomPersistStep.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

/**
 * A custom persist step when a charcter logs out.
 * Locks the object to prevent a reload until any pending persisted data
 * has been saved.
 */
class UnloadCharacterCustomPersistStep : public CustomPersistStep
{
  public:
	UnloadCharacterCustomPersistStep(const NetworkId &characterObject, uint32 serverId);
	~UnloadCharacterCustomPersistStep();
	
	virtual bool beforePersist (DB::Session *session);
	virtual bool afterPersist  (DB::Session *session);
	virtual void onComplete    ();

  private:
	NetworkId m_characterObject;
	uint32    m_serverId;

  private:
	UnloadCharacterCustomPersistStep &operator= (const UnloadCharacterCustomPersistStep&); //disable
	UnloadCharacterCustomPersistStep(const UnloadCharacterCustomPersistStep&); //disable
};

// ======================================================================

#endif
