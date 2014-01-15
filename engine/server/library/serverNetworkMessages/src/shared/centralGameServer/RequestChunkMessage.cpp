//========================================================================
//
// RequestChunkMessage.cpp - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RequestChunkMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param process		gameserver id that should receive the loaded objects
 */
RequestChunkMessage::RequestChunkMessage  (std::vector<Chunk> chunks, const std::string &sceneId) :
	GameNetworkMessage("RequestChunkMessage"),
	m_chunks(),
	m_sceneId(sceneId)
{
	m_chunks.set(chunks);
	
	addVariable(m_chunks);
	addVariable(m_sceneId);
}	

//-----------------------------------------------------------------------

RequestChunkMessage::RequestChunkMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("RequestChunkMessage"),
		m_chunks(),
		m_sceneId()
{
	addVariable(m_chunks);
	addVariable(m_sceneId);
	
	unpack(source);
}

//-----------------------------------------------------------------------

RequestChunkMessage::~RequestChunkMessage()
{
}	

// ----------------------------------------------------------------------

std::vector<RequestChunkMessage::Chunk> const & RequestChunkMessage::getChunks() const
{
	return m_chunks.get();
}

// ======================================================================

RequestChunkMessage::Chunk::Chunk() :
		m_process(),
		m_nodeX(),
		m_nodeZ()
{
}

// ----------------------------------------------------------------------

RequestChunkMessage::Chunk::Chunk(uint32 process, int nodeX, int nodeZ) :
		m_process(process),
		m_nodeX(nodeX),
		m_nodeZ(nodeZ)
{
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, RequestChunkMessage::Chunk & chunk)
	{
		get(source,chunk.m_process);
		get(source,chunk.m_nodeX);
		get(source,chunk.m_nodeZ);
	}

	// ----------------------------------------------------------------------

	void put(ByteStream & target, const RequestChunkMessage::Chunk & chunk)
	{
		put(target,chunk.m_process);
		put(target,chunk.m_nodeX);
		put(target,chunk.m_nodeZ);
	}
	
}

// ======================================================================

