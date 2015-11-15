// ======================================================================
//
// CreateCharacterCustomPersistStep.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CreateCharacterCustomPersistStep_H
#define INCLUDED_CreateCharacterCustomPersistStep_H

// ======================================================================

#include "serverDatabase/CustomPersistStep.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"
#include <string>

class NetworkId;

// ======================================================================

class CreateCharacterCustomPersistStep : public CustomPersistStep
{
  public:
	CreateCharacterCustomPersistStep(uint32 stationId, const NetworkId &characterObject, const Unicode::String &characterName, const std::string &normalizedName, const int templateId, bool special);
	~CreateCharacterCustomPersistStep();
	
	virtual bool beforePersist (DB::Session *session);
	virtual bool afterPersist  (DB::Session *session);
	virtual void onComplete    ();

  private:
	uint32 m_stationId;
	NetworkId m_characterObject;
	Unicode::String m_characterName;
	std::string m_normalizedName;
	int m_templateId;
	std::string m_clusterName;
	bool m_special;

  private:
	CreateCharacterCustomPersistStep &operator= (const CreateCharacterCustomPersistStep&); //disable
	CreateCharacterCustomPersistStep(const CreateCharacterCustomPersistStep&); //disable
};

// ======================================================================

#endif
