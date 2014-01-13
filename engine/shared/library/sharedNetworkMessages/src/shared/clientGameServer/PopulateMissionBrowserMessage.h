// ======================================================================
//
// PopulateMissionBrowserMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_PopulateMissionBrowserMessage_H
#define	_PopulateMissionBrowserMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class PopulateMissionBrowserMessage : public GameNetworkMessage
{
public:

	PopulateMissionBrowserMessage(stdvector<NetworkId>::fwd const & missions);
	explicit PopulateMissionBrowserMessage(Archive::ReadIterator &source);

	~PopulateMissionBrowserMessage();

public: // methods

	stdvector<NetworkId>::fwd const &getMissions() const;

public: // types

private: 
	Archive::AutoArray<NetworkId>    m_missions;
};

// ----------------------------------------------------------------------

inline stdvector<NetworkId>::fwd const & PopulateMissionBrowserMessage::getMissions() const
{
	return m_missions.get();
}

// ----------------------------------------------------------------------

#endif // _PopulateMissionBrowserMessage_H

