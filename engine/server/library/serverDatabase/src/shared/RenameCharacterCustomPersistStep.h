// ======================================================================
//
// RenameCharacterCustomPersistStep.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_RenameCharacterCustomPersistStep_H
#define INCLUDED_RenameCharacterCustomPersistStep_H

// ======================================================================

#include "serverDatabase/CustomPersistStep.h"
#include "sharedFoundation/NetworkId.h"

class NetworkId;
class TransferCharacterData;

// ======================================================================

class RenameCharacterCustomPersistStep : public CustomPersistStep
{
  public:
	RenameCharacterCustomPersistStep(int8 renameCharacterMessageSource, uint32 stationId, const NetworkId &characterId, const Unicode::String &characterName, const Unicode::String &characterOldName, const NetworkId &requestedBy, const TransferCharacterData * requestData);
	~RenameCharacterCustomPersistStep();
	
	virtual bool beforePersist (DB::Session *session);
	virtual bool afterPersist  (DB::Session *session);
	virtual void onComplete    ();

	static bool hasPendingRenameCharacterCustomPersistStep(const NetworkId &characterId);

  private:
	int8              m_renameCharacterMessageSource;
	uint32            m_stationId;
	NetworkId         m_characterId;
	Unicode::String * m_characterName;
	Unicode::String * m_characterOldName;
	NetworkId         m_requestedBy;
	bool              m_success;
	TransferCharacterData *  m_requestData;
};

// ======================================================================

#endif
