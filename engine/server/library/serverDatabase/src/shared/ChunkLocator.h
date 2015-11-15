// ======================================================================
//
// ChunkLocator.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ChunkLocator_H
#define INCLUDED_ChunkLocator_H

#include "serverDatabase/ObjectLocator.h"

// ======================================================================

/**
 * Given a sceneID and map coordinates, locates all the objects in that
 * chunk.  Adds them to the list of objects to be loaded by the Snapshot.
 *
 * @todo It will be common to have a bunch of these attached to the same
 * snapshot.  Maybe it would be more efficient to find a way to combine
 * them.
 */
class ChunkLocator : public ObjectLocator
{
  public:
	ChunkLocator(int nodeX, int nodeZ, const std::string &sceneId, uint32 serverId, bool wantChunkCompleteMessage);
	virtual ~ChunkLocator();
	
  public:
	virtual bool locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated);
	virtual void sendPostBaselinesCustomData(GameServerConnection &conn) const;

  private:
	int m_nodeX;
	int m_nodeZ;
	std::string *m_sceneId;
	int m_objectCount;
	int m_queryTime;
	uint32 m_serverId;
	bool m_wantChunkCompleteMessage;

  private:
	ChunkLocator &operator=(const ChunkLocator&); //disable
	ChunkLocator(const ChunkLocator&); //disable
};

// ======================================================================

#endif
