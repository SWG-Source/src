// ======================================================================
//
// LoaderSnapshotGroup.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LoaderSnapshotGroup_H
#define INCLUDED_LoaderSnapshotGroup_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"

namespace DB
{
	class Session;
}
class Snapshot;
class ObjectLocator;

// ======================================================================

class LoaderSnapshotGroup
{
  public:
	LoaderSnapshotGroup(uint32 requestingProcess, Snapshot *snapshot);
	virtual ~LoaderSnapshotGroup() =0; //insure class gets derived
	
	bool load                      (DB::Session *session);
	bool send                      () const;
	void addLocator                (ObjectLocator *newLocator);
	void addGoldLocator            (ObjectLocator *newLocator);
	int  getLocatorCount           () const;
	void setUniverseAuthHack       ();
	uint32 getRequestingProcessId  () const;
	void setLoadSerialNumber       (int loadSerialNumber);

  private:
	virtual Snapshot *makeGoldSnapshot()=0;
		
  private:
	uint32 m_requestingProcess;

  private:
	Snapshot *m_snapshot;
	Snapshot *m_goldSnapshot;
	int m_loadSerialNumber;
};

// ----------------------------------------------------------------------

inline uint32 LoaderSnapshotGroup::getRequestingProcessId() const
{
	return m_requestingProcess;
}

// ======================================================================

#endif
