// ======================================================================
//
// ParametersMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ParametersMessage_H
#define INCLUDED_ParametersMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent from:  Game Server
 * Sent to:    Client
 * Action:     Tells the client the value of various server settings
 *             that it needs to know about.  This message is sent whenever
 *             the client connects to a game server, so the client may
 *             receive it more than once.
 *
 *             For now, if it receives it
 *             more than once, it is entitled to ignore the second message.
 *             We may change this rule in the future if different servers are
 *             allowed to have different settings for any of the things in
 *             this message.
 */
class ParametersMessage : public GameNetworkMessage
{
  public:
	ParametersMessage(int weatherUpdateInterval);
	explicit ParametersMessage(Archive::ReadIterator & source);
	virtual ~ParametersMessage();

  public:
	int getWeatherUpdateInterval() const;
	
  private:
	Archive::AutoVariable<int> m_weatherUpdateInterval;

	ParametersMessage();
	ParametersMessage(const ParametersMessage&);
	ParametersMessage& operator= (const ParametersMessage&);
};

// ----------------------------------------------------------------------

inline int ParametersMessage::getWeatherUpdateInterval() const
{
	return m_weatherUpdateInterval.get();
}

// ======================================================================

#endif
