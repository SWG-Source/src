// ======================================================================
//
// ChunkCompleteMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ChunkCompleteMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

ChunkCompleteMessage::ChunkCompleteMessage(std::vector<std::pair<int, int> > const & chunks) :
	GameNetworkMessage("ChunkCompleteMessage"),
	m_chunks()
{
	m_chunks.set(chunks);
	
	addVariable(m_chunks);
}	

// ----------------------------------------------------------------------

ChunkCompleteMessage::ChunkCompleteMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ChunkCompleteMessage"),
		m_chunks()
{
	addVariable(m_chunks);
	
	unpack(source);
}

// ----------------------------------------------------------------------

ChunkCompleteMessage::~ChunkCompleteMessage()
{
}	

// ----------------------------------------------------------------------

std::vector<std::pair<int, int> > const & ChunkCompleteMessage::getChunks() const
{
	return m_chunks.get();
}

// ======================================================================
