// ======================================================================
//
// PlayCutSceneMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_PlayCutSceneMessage_H
#define	_PlayCutSceneMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class PlayCutSceneMessage : public GameNetworkMessage
{
public: //ctor/dtor
	explicit PlayCutSceneMessage(std::string const &cutSceneName);
	explicit PlayCutSceneMessage(Archive::ReadIterator &source);

	~PlayCutSceneMessage();

public: // methods

	std::string const &  getCutSceneName() const;

public: // types

private: 
	Archive::AutoVariable<std::string>  m_cutSceneName;
};

// ----------------------------------------------------------------------

#endif // _PlayCutSceneMessage_H
