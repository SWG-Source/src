//========================================================================
//
// EnableNewJediTrackingMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_EnableNewJediTrackingMessage_H
#define	_INCLUDED_EnableNewJediTrackingMessage_H

//-----------------------------------------------------------------------

#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:  DBProcess
 * Action:  enables/disables tracking of players to see if they can become Jedi
 */
class EnableNewJediTrackingMessage: public GameNetworkMessage
{
public:
	EnableNewJediTrackingMessage(bool enableTracking);
	EnableNewJediTrackingMessage(Archive::ReadIterator & source);
	~EnableNewJediTrackingMessage();

public:
	bool getEnableTracking() const;
	
private:
	Archive::AutoVariable<bool>  m_enableTracking;

private:
	EnableNewJediTrackingMessage(const EnableNewJediTrackingMessage&);
	EnableNewJediTrackingMessage& operator= (const EnableNewJediTrackingMessage&);
};

// ======================================================================

inline bool EnableNewJediTrackingMessage::getEnableTracking() const
{
	return m_enableTracking.get();
}

// ======================================================================

#endif	// _INCLUDED_EnableNewJediTrackingMessage_H

