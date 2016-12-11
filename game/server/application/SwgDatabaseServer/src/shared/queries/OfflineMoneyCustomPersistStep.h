// ======================================================================
//
// OfflineMoneyCustomPersistStep.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_OfflineMoneyCustomPersistStep_H
#define INCLUDED_OfflineMoneyCustomPersistStep_H

// ======================================================================

#include "serverDatabase/CustomPersistStep.h"

// ======================================================================

namespace OfflineMoneyCustomPersistStepNamespace
{
	struct GetMoneyData;
}

// ======================================================================

class OfflineMoneyCustomPersistStep : public CustomPersistStep
{
  public:
	OfflineMoneyCustomPersistStep();
	~OfflineMoneyCustomPersistStep();
	
	void getMoneyFromOfflineObject(uint32 replyServer, NetworkId const & sourceObject, int amount, NetworkId const & replyTo, std::string const & successCallback, std::string const & failCallback, std::vector<int8> const & packedDictionary);
		
	virtual bool beforePersist (DB::Session *session);
	virtual bool afterPersist  (DB::Session *session);
	virtual void onComplete    ();

  private:
	typedef std::vector<OfflineMoneyCustomPersistStepNamespace::GetMoneyData*> GetMoneyRequestsType;
	GetMoneyRequestsType * const m_getMoneyRequests;

  private:
	OfflineMoneyCustomPersistStep(const OfflineMoneyCustomPersistStep &); //disable
	OfflineMoneyCustomPersistStep & operator=(const OfflineMoneyCustomPersistStep &); //disable
};

// ======================================================================

#endif
