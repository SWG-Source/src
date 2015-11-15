// ======================================================================
//
// PlayMusicMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/PlayMusicMessage.h"

// ======================================================================

PlayMusicMessage::PlayMusicMessage(std::string const &musicName, NetworkId const &sourceObjId, uint32 playType, bool loop) :
	GameNetworkMessage("PlayMusicMessage"),
	m_musicName(musicName),
	m_sourceObjId(sourceObjId),
	m_playType(playType),
	m_loop(loop)
{
	AutoByteStream::addVariable(m_musicName);
	AutoByteStream::addVariable(m_sourceObjId);
	AutoByteStream::addVariable(m_playType);
	AutoByteStream::addVariable(m_loop);
}

// ----------------------------------------------------------------------

PlayMusicMessage::PlayMusicMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("PlayMusicMessage"),
	m_musicName(),
	m_sourceObjId(),
	m_playType(),
	m_loop()
{
	AutoByteStream::addVariable(m_musicName);
	AutoByteStream::addVariable(m_sourceObjId);
	AutoByteStream::addVariable(m_playType);
	AutoByteStream::addVariable(m_loop);
	unpack(source);
}

// ----------------------------------------------------------------------

PlayMusicMessage::~PlayMusicMessage()
{
}

// ======================================================================

