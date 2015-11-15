

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "VoiceChatChannelInfo.h"

const char* const VoiceChatChannelInfo::cms_name = "VoiceChatChannelInfo";

VoiceChatChannelInfo::VoiceChatChannelInfo(std::string const & name, std::string const & displayName, std::string const & uri, std::string const & password, std::string const & announceText, uint32 flags)
: GameNetworkMessage(cms_name),
m_flags(flags),
m_channelName(name),
m_displayName(displayName),
m_channelUri(uri),
m_channelPassword(password),
m_announceText(announceText)
{
	addVariable(m_flags);
	addVariable(m_channelName);
	addVariable(m_displayName);
	addVariable(m_channelUri);
	addVariable(m_channelPassword);
	addVariable(m_announceText);
}


VoiceChatChannelInfo::VoiceChatChannelInfo(Archive::ReadIterator & source)
: GameNetworkMessage(cms_name),
  m_flags(CIF_None),
  m_channelName(),
  m_displayName(),
  m_channelUri(),
  m_channelPassword(),
  m_announceText()
{
	addVariable(m_flags);
	addVariable(m_channelName);
	addVariable(m_displayName);
	addVariable(m_channelUri);
	addVariable(m_channelPassword);
	addVariable(m_announceText);
	unpack(source);
}

VoiceChatChannelInfo::~VoiceChatChannelInfo()
{

}
