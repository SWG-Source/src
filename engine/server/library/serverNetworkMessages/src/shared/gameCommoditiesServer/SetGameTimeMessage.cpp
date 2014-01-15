#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "SetGameTimeMessage.h"

SetGameTimeMessage::SetGameTimeMessage(Archive::ReadIterator & source) :
GameNetworkMessage("SetGameTimeMessage"),
m_responseId(),
m_trackId(),
m_gameTime()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_gameTime);
	unpack(source);
}

SetGameTimeMessage::SetGameTimeMessage(
	int requestId,
	int gameTime
) : 
GameNetworkMessage("SetGameTimeMessage"),
m_responseId(requestId),
m_trackId(0),
m_gameTime(gameTime)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_gameTime);
}

SetGameTimeMessage::~SetGameTimeMessage()
{
}
