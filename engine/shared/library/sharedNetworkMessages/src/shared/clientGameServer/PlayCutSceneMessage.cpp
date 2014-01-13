// ======================================================================
//
// PlayCutSceneMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/PlayCutSceneMessage.h"

// ======================================================================

PlayCutSceneMessage::PlayCutSceneMessage(std::string const &cutSceneName) :
	GameNetworkMessage("PlayCutSceneMessage")
,	m_cutSceneName(cutSceneName)
{
	AutoByteStream::addVariable(m_cutSceneName);
}

// ----------------------------------------------------------------------

PlayCutSceneMessage::PlayCutSceneMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("PlayCutSceneMessage")
,	m_cutSceneName()
{
	AutoByteStream::addVariable(m_cutSceneName);
	unpack(source);
}

// ----------------------------------------------------------------------

PlayCutSceneMessage::~PlayCutSceneMessage()
{
}

// ----------------------------------------------------------------------

const std::string & PlayCutSceneMessage::getCutSceneName() const
{
	return m_cutSceneName.get();
}

// ----------------------------------------------------------------------

// ======================================================================

