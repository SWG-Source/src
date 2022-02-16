// ======================================================================
//
// MoveToPlayerCustomPersistStep.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MoveToPlayerCustomPersistStep_H
#define INCLUDED_MoveToPlayerCustomPersistStep_H

// ======================================================================

#include "Unicode.h"
#include "serverDatabase/CustomPersistStep.h"
#include "sharedDatabaseInterface/DbBindableLong.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedFoundation/NetworkId.h"
#include <string>

class NetworkId;

// ======================================================================

class MoveToPlayerCustomPersistStep : public CustomPersistStep
{
  public:
	MoveToPlayerCustomPersistStep(const NetworkId &objectId, const NetworkId &targetPlayer, int maxDepth, bool useBank, bool useDatapad);
	~MoveToPlayerCustomPersistStep();
	
	virtual bool beforePersist (DB::Session *session);
	virtual bool afterPersist  (DB::Session *session);
	virtual void onComplete    ();

  private:
	NetworkId m_objectId;
	NetworkId m_targetPlayer;
	int m_maxDepth;
	bool m_useBank;
	bool m_useDatapad;

	class MoveToPlayerQuery: public DB::Query
	{
	  public:
		MoveToPlayerQuery(const NetworkId &objectId, const NetworkId &targetPlayer, int maxDepth, bool useBank, bool useDatapad);
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	  private:
		MoveToPlayerQuery(MoveToPlayerQuery const &);
		MoveToPlayerQuery &operator=(MoveToPlayerQuery const &);
		DB::BindableNetworkId object_id;
		DB::BindableNetworkId target_player;
		DB::BindableLong max_depth;
		bool m_useBank;
		bool m_useDatapad;
	};
	
  private:
	MoveToPlayerCustomPersistStep &operator= (const MoveToPlayerCustomPersistStep&); //disable
	MoveToPlayerCustomPersistStep(const MoveToPlayerCustomPersistStep&); //disable
};

// ======================================================================

#endif
