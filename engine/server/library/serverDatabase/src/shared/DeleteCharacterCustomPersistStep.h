// ======================================================================
//
// DeleteCharacterCustomPersistStep.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeleteCharacterCustomPersistStep_H
#define INCLUDED_DeleteCharacterCustomPersistStep_H

// ======================================================================

#include <string>

#include "serverDatabase/CustomPersistStep.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

class DeleteCharacterCustomPersistStep : public CustomPersistStep
{
  public:
	DeleteCharacterCustomPersistStep(uint32 stationId, const NetworkId &characterId);
	virtual bool beforePersist (DB::Session *session);
	virtual bool afterPersist  (DB::Session *session);
	virtual void onComplete    ();
		
  private:
	NetworkId        m_characterId;
	uint32           m_stationId;
	uint32           m_resultCode;
	
	class DeleteCharacterQuery : public DB::Query
	{
	  public:
		DeleteCharacterQuery(uint32 stationId, const NetworkId &characterId);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		
	  protected:
		virtual QueryMode getExecutionMode() const;

	  public:
		DB::BindableLong         station_id;
		DB::BindableNetworkId    character_id;
		DB::BindableLong         delete_minutes;
		DB::BindableLong         result;
				
	  private: // disable:
		DeleteCharacterQuery(const DeleteCharacterQuery&);
		DeleteCharacterQuery& operator=(const DeleteCharacterQuery&);
	};
};

// ======================================================================

#endif
