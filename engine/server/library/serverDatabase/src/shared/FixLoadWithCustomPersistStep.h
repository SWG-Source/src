// ======================================================================
//
// FixLoadWithCustomPersistStep.h
// Copyright (c) 2007 Sony Online Entertainment, LLC
//
// ======================================================================

#ifndef INCLUDED_FixLoadWithCustomPersistStep_H
#define INCLUDED_FixLoadWithCustomPersistStep_H

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

class FixLoadWithCustomPersistStep : public CustomPersistStep
{
  public:
	FixLoadWithCustomPersistStep(const NetworkId &topmostObject, const NetworkId &startingLoadWith, int maxDepth);
	~FixLoadWithCustomPersistStep();
	
	virtual bool beforePersist (DB::Session *session);
	virtual bool afterPersist  (DB::Session *session);
	virtual void onComplete    ();

  private:
	NetworkId m_topmostObject;
	NetworkId m_startingLoadWith;
	int m_maxDepth;

	class FixLoadWithQuery: public DB::Query
	{
	  public:
		FixLoadWithQuery(const NetworkId &topmostObject, const NetworkId &startingLoadWith, int maxDepth);
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	  private:
		FixLoadWithQuery(FixLoadWithQuery const &);
		FixLoadWithQuery &operator=(FixLoadWithQuery const &);
		DB::BindableNetworkId topmost_object;
		DB::BindableNetworkId starting_loadwith;
		DB::BindableLong max_depth;
	};
	
  private:
	FixLoadWithCustomPersistStep &operator= (const FixLoadWithCustomPersistStep&); //disable
	FixLoadWithCustomPersistStep(const FixLoadWithCustomPersistStep&); //disable
};

// ======================================================================

#endif
