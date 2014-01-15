//========================================================================
//
// ChunkObjectListMessage.cpp - 
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ChunkObjectListMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------
/**
 * Class constructor.
 *
 * @param process		gameserver id requesting the chunk (from RequestChunkMessage)
 * @param count         number of ids in the chunk
 * @param ids           list of ids
 */
ChunkObjectListMessage::ChunkObjectListMessage(uint32 process, uint16 count,
	const NetworkId* ids) :
	GameNetworkMessage("ChunkObjectListMessage"),
	m_process(process),
	m_ids()
{
	addVariable(m_process);
	addVariable(m_ids);

	if (count > 0)
	{
		NOT_NULL(ids);
		std::vector<NetworkId> v;

		for (size_t i = 0; i < count; ++i)
			v.push_back(ids[i]);
		m_ids.set(v);
	}
}	// RequestChunkMessage::RequestChunkMessage

//-----------------------------------------------------------------------

	ChunkObjectListMessage::ChunkObjectListMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ChunkObjectListMessage"),
m_process(),
m_ids()
{
	addVariable(m_process);
	addVariable(m_ids);
	unpack(source);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
ChunkObjectListMessage::~ChunkObjectListMessage()
{
}	// ChunkObjectListMessage::~ChunkObjectListMessage

//-----------------------------------------------------------------------
