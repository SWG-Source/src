// ======================================================================
//
// SwgPersister.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgPersister_H
#define INCLUDED_SwgPersister_H

#include "serverDatabase/Persister.h"

// ======================================================================

class SwgPersister : public Persister
{
  public:
	static void install();
	
  private:
	virtual Snapshot * makeSnapshot           (DB::ModeQuery::Mode mode) const;
	virtual Snapshot * makeCommoditiesSnapshot(DB::ModeQuery::Mode mode) const;
	virtual void startLoadCommodities();
	virtual void startSave();
	virtual void restoreHouse(const NetworkId &houseId, const std::string &whoRequested);
	virtual void restoreCharacter(const NetworkId &playerId, const std::string &whoRequested);
	virtual void undeleteItem(const NetworkId &itemId, const std::string &whoRequested);
	virtual void moveToPlayer(const NetworkId &oid, const NetworkId &player, const std::string &whoRequested);
	virtual void getMoneyFromOfflineObject(uint32 replyServer, NetworkId const & sourceObject, int amount, NetworkId const & replyTo, std::string const & successCallback, std::string const & failCallback, std::vector<int8> const & packedDictionary);
		
  private:
	SwgPersister();
	virtual ~SwgPersister();
	SwgPersister(const SwgPersister &);
	SwgPersister & operator = (const SwgPersister &);
};

// ======================================================================

#endif
