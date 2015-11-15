//========================================================================
//
// ChunkObjectListMessage.h - tells Centralserver all the object ids in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ChunkObjectListMessage_H
#define	_INCLUDED_ChunkObjectListMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <vector>

/** @todo: right now this will every object in the game, we need to modify to
 * give a chunk id list
 */


//-----------------------------------------------------------------------

class ChunkObjectListMessage : public GameNetworkMessage
{
public:
	ChunkObjectListMessage  (uint32 process, uint16 count, const NetworkId* ids);
	ChunkObjectListMessage  (Archive::ReadIterator & source);
	~ChunkObjectListMessage ();

	const uint32             getProcess(void) const;
	const uint16             getCount(void) const;
	const std::vector<NetworkId> & getIds(void) const;

private:
	Archive::AutoVariable<uint32> m_process;		// gameserver id requesting the chunk (from RequestChunkMessage)
	//uint16 m_count;			// number of ids in the chunk
	//NetworkId*   m_ids;			// list of ids
	Archive::AutoArray<NetworkId> 	m_ids;

	ChunkObjectListMessage();
	ChunkObjectListMessage(const ChunkObjectListMessage&);
	ChunkObjectListMessage& operator= (const ChunkObjectListMessage&);
};


//-----------------------------------------------------------------------

inline const uint32 ChunkObjectListMessage::getProcess(void) const
{
	return m_process.get();
}	// ChunkObjectListMessage::getProcess

//-----------------------------------------------------------------------

inline const std::vector<NetworkId> & ChunkObjectListMessage::getIds(void) const
{
	return m_ids.get();
}	// ChunkObjectListMessage::getIds

//-----------------------------------------------------------------------

inline const uint16 ChunkObjectListMessage::getCount() const
{
	// cast back to a 16 bit int to save space over the network
	return static_cast<uint16>(m_ids.get().size());
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChunkObjectListMessage_H
