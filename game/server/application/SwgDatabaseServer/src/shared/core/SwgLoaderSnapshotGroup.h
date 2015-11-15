// ======================================================================
//
// SwgLoaderSnapshotGroup.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgLoaderSnapshotGroup_H
#define INCLUDED_SwgLoaderSnapshotGroup_H

// ======================================================================

#include "serverDatabase/LoaderSnapshotGroup.h"

namespace DB
{
	class Session;
}
class SwgSnapshot;

// ======================================================================

class SwgLoaderSnapshotGroup : public LoaderSnapshotGroup
{
  public:
	SwgLoaderSnapshotGroup(uint32 requestingProcess);
	virtual ~SwgLoaderSnapshotGroup();

  private:
	virtual Snapshot *makeGoldSnapshot();
};

// ======================================================================

#endif
