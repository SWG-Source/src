// ======================================================================
//
// ChunkCompleteMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_ChunkCompleteMessage_H
#define	_INCLUDED_ChunkCompleteMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class ChunkCompleteMessage : public GameNetworkMessage
{
public:
	ChunkCompleteMessage  (std::vector<std::pair<int, int> > const & chunks);
	ChunkCompleteMessage  (Archive::ReadIterator & source);
	~ChunkCompleteMessage ();

	std::vector<std::pair<int, int> > const & getChunks() const;

private:
	Archive::AutoArray<std::pair<int, int> > m_chunks;

	ChunkCompleteMessage();
	ChunkCompleteMessage(const ChunkCompleteMessage&);
	ChunkCompleteMessage& operator= (const ChunkCompleteMessage&);
};

// ======================================================================

#endif	// _INCLUDED_ChunkCompleteMessage_H

