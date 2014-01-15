//========================================================================
//
// RequestChunkMessage.h - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_RequestChunkMessage_H
#define	_INCLUDED_RequestChunkMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class RequestChunkMessage : public GameNetworkMessage
{
public:
	struct Chunk
	{
		uint32 m_process;
		int m_nodeX;
		int m_nodeZ;

		Chunk();
		Chunk(uint32 process, int nodeX, int nodeZ);
	};
	
public:
	RequestChunkMessage  (std::vector<Chunk> chunks, const std::string &sceneId);
	RequestChunkMessage  (Archive::ReadIterator & source);
	~RequestChunkMessage ();

	std::vector<Chunk> const & getChunks() const;
	std::string getSceneId (void) const;

private:
	Archive::AutoArray<Chunk> m_chunks;
	Archive::AutoVariable<std::string> m_sceneId;

	RequestChunkMessage();
	RequestChunkMessage(const RequestChunkMessage&);
	RequestChunkMessage& operator= (const RequestChunkMessage&);
};

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;
	
	void get(ReadIterator & source, RequestChunkMessage::Chunk & chunk);
	void put(ByteStream & target, const RequestChunkMessage::Chunk & chunk);
}

// ======================================================================

inline std::string RequestChunkMessage::getSceneId (void) const
{
	return m_sceneId.get();
}

// ======================================================================

#endif	// _INCLUDED_RequestChunkMessage_H

