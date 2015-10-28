// ======================================================================
//
// SwgLoader.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgLoader_H
#define INCLUDED_SwgLoader_H

// ======================================================================

#include "serverDatabase/Loader.h"

// ======================================================================

class TaskVerifyCharacter;
class TransferCharacterData;

// ======================================================================

/**
 * Singleton that organizes and queues requests to load objects from the database.
 */
class SwgLoader : public Loader
{
  public:
	static void install();
	
  public:
//	virtual void getCharacters(uint32 accountNumber);
	virtual void verifyCharacter(StationId suid, const NetworkId &id, const TransferCharacterData *);
	virtual void update(real updateTime);
	void verifyCharacterFinished (TaskVerifyCharacter *task);
	virtual void locateStructure(const NetworkId &structureId, const std::string whoRequested);
		
  private:
	virtual LoaderSnapshotGroup *makeLoaderSnapshotGroup(uint32 processId);
	virtual void loadStartupData();

  private:
	SwgLoader();
	virtual ~SwgLoader();
	SwgLoader(const SwgLoader &);
	SwgLoader &  operator = (const SwgLoader &);

  private:
	TaskVerifyCharacter *m_pendingTaskVerifyCharacter;
	TaskVerifyCharacter *m_loadingTaskVerifyCharacter;

	DB::TaskQueue *m_verifyCharacterTaskQ;
};


// ======================================================================

#endif
