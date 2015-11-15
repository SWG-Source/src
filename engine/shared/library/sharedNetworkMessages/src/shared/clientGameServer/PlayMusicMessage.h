// ======================================================================
//
// PlayMusicMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_PlayMusicMessage_H
#define	_PlayMusicMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class PlayMusicMessage : public GameNetworkMessage
{
public: //ctor/dtor
	PlayMusicMessage(std::string const &musicName, NetworkId const &sourceObjId, uint32 playType, bool loop);
	explicit PlayMusicMessage(Archive::ReadIterator &source);

	~PlayMusicMessage();

public: // methods

	std::string const &  getMusicName() const;
	NetworkId const &    getSourceObjId() const;
	uint32               getPlayType() const;
	bool                 getLoop() const;

public: // types

private: 
	Archive::AutoVariable<std::string>  m_musicName;
	Archive::AutoVariable<NetworkId>    m_sourceObjId;
	Archive::AutoVariable<uint32>       m_playType;
	Archive::AutoVariable<bool>         m_loop;
};

// ----------------------------------------------------------------------

inline std::string const &PlayMusicMessage::getMusicName() const
{
	return m_musicName.get();
}

// ----------------------------------------------------------------------

inline NetworkId const &PlayMusicMessage::getSourceObjId() const
{
	return m_sourceObjId.get();
}

// ----------------------------------------------------------------------

inline uint32 PlayMusicMessage::getPlayType() const
{
	return m_playType.get();
}

// ----------------------------------------------------------------------

inline bool PlayMusicMessage::getLoop() const
{
	return m_loop.get();
}

// ----------------------------------------------------------------------

#endif // _PlayMusicMessage_H

